/*
 * xvmalloc memory allocator
 *
 * Copyright (C) 2008, 2009  Nitin Gupta
 *
 * This code is released using a dual license strategy: BSD/GPL
 * You can choose the licence that better fits your requirements.
 *
 * Released under the terms of 3-clause BSD License
 * Released under the terms of GNU General Public License Version 2.0
 */

#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/highmem.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/slab.h>

#include "xvmalloc.h"
#include "xvmalloc_int.h"
#include "kmap_types.h"

static void stat_inc(u64 *value)
{
	*value = *value + 1;
}

static void stat_dec(u64 *value)
{
	*value = *value - 1;
}

static int test_flag(struct block_header *block, enum blockflags flag)
{
	return block->prev & BIT(flag);
}

static void set_flag(struct block_header *block, enum blockflags flag)
{
	block->prev |= BIT(flag);
}

static void clear_flag(struct block_header *block, enum blockflags flag)
{
	block->prev &= ~BIT(flag);
}

/*
 * Given <page, offset> pair, provide a derefrencable pointer.
 * This is called from xv_malloc/xv_free path, so it
 * needs to be fast.
 */
static void *get_ptr_atomic(struct page *page, u16 offset, enum km_type type)
{
	unsigned char *base;

	base = kmap_atomic(page);
	return base + offset;
}

static void put_ptr_atomic(void *ptr, enum km_type type)
{
	kunmap_atomic(ptr);
}

static u32 get_blockprev(struct block_header *block)
{
	return block->prev & PREV_MASK;
}

static void set_blockprev(struct block_header *block, u16 new_offset)
{
	block->prev = new_offset | (block->prev & FLAGS_MASK);
}

static struct block_header *BLOCK_NEXT(struct block_header *block)
{
	return (struct block_header *)
		((char *)block + block->size + XV_ALIGN);
}

/*
 * Get index of free list containing blocks of maximum size
 * which is less than or equal to given size.
 */
static u32 get_index_for_insert(u32 size)
{
	if (unlikely(size > XV_MAX_ALLOC_SIZE))
		size = XV_MAX_ALLOC_SIZE;
	size &= ~FL_DELTA_MASK;
	return (size - XV_MIN_ALLOC_SIZE) >> FL_DELTA_SHIFT;
}

/*
 * Get index of free list having blocks of size greater than
 * or equal to requested size.
 */
static u32 get_index(u32 size)
{
	if (unlikely(size < XV_MIN_ALLOC_SIZE))
		size = XV_MIN_ALLOC_SIZE;
	size = ALIGN(size, FL_DELTA);
	return (size - XV_MIN_ALLOC_SIZE) >> FL_DELTA_SHIFT;
}

/**
 * find_block - find block of at least given size
 * @pool: memory pool to search from
 * @size: size of block required
 * @page: page containing required block
 * @offset: offset within the page where block is located.
 *
 * Searches two level bitmap to locate block of at least
 * the given size. If such a block is found, it provides
 * <page, offset> to identify this block and returns index
 * in freelist where we found this block.
 * Otherwise, returns 0 and <page, offset> params are not touched.
 */
static u32 find_block(struct xv_pool *pool, u32 size,
			struct page **page, u32 *offset)
{
	//printk("5.1------------------------------------------------\n");
	ulong flbitmap, slbitmap;
	u32 flindex, slindex, slbitstart;
	//printk("5.2------------------------------------------------\n");
	/* There are no free blocks in this pool */
	if (!pool->flbitmap)
		return 0;
	//printk("5.3------------------------------------------------\n");
	/* Get freelist index correspoding to this size */
	slindex = get_index(size);
	slbitmap = pool->slbitmap[slindex / BITS_PER_LONG];
	slbitstart = slindex % BITS_PER_LONG;
	//printk("5.4------------------------------------------------\n");
	/*
	 * If freelist is not empty at this index, we found the
	 * block - head of this list. This is approximate best-fit match.
	 */
	if (test_bit(slbitstart, &slbitmap)) {
		//printk("5.5------------------------------------------------\n");
		*page = pool->freelist[slindex].page;
		*offset = pool->freelist[slindex].offset;
		return slindex;
	}
	//printk("5.6------------------------------------------------\n");
	/*
	 * No best-fit found. Search a bit further in bitmap for a free block.
	 * Second level bitmap consists of series of 32-bit chunks. Search
	 * further in the chunk where we expected a best-fit, starting from
	 * index location found above.
	 */
	slbitstart++;
	slbitmap >>= slbitstart;
	//printk("5.7------------------------------------------------\n");
	/* Skip this search if we were already at end of this bitmap chunk */
	if ((slbitstart != BITS_PER_LONG) && slbitmap) {
		//printk("5.8------------------------------------------------\n");
		slindex += __ffs(slbitmap) + 1;
		*page = pool->freelist[slindex].page;
		*offset = pool->freelist[slindex].offset;
		return slindex;
	}
	//printk("5.9------------------------------------------------\n");
	/* Now do a full two-level bitmap search to find next nearest fit */
	flindex = slindex / BITS_PER_LONG;

	flbitmap = (pool->flbitmap) >> (flindex + 1);
	//printk("5.10------------------------------------------------\n");
	if (!flbitmap)
		return 0;
	//printk("5.11------------------------------------------------\n");
	flindex += __ffs(flbitmap) + 1;
	slbitmap = pool->slbitmap[flindex];
	slindex = (flindex * BITS_PER_LONG) + __ffs(slbitmap);
	*page = pool->freelist[slindex].page;
	*offset = pool->freelist[slindex].offset;
	//printk("5.12------------------------------------------------\n");
	return slindex;
}

/*
 * Insert block at <page, offset> in freelist of given pool.
 * freelist used depends on block size.
 */
static void insert_block(struct xv_pool *pool, struct page *page, u32 offset,
			struct block_header *block)
{
	u32 flindex, slindex;
	struct block_header *nextblock;

	slindex = get_index_for_insert(block->size);
	flindex = slindex / BITS_PER_LONG;

	block->link.prev_page = 0;
	block->link.prev_offset = 0;
	block->link.next_page = pool->freelist[slindex].page;
	block->link.next_offset = pool->freelist[slindex].offset;
	pool->freelist[slindex].page = page;
	pool->freelist[slindex].offset = offset;

	if (block->link.next_page) {
		nextblock = get_ptr_atomic(block->link.next_page,
					block->link.next_offset, KM_USER1);
		nextblock->link.prev_page = page;
		nextblock->link.prev_offset = offset;
		put_ptr_atomic(nextblock, KM_USER1);
	}

	__set_bit(slindex % BITS_PER_LONG, &pool->slbitmap[flindex]);
	__set_bit(flindex, &pool->flbitmap);
}

/*
 * Remove block from head of freelist. Index 'slindex' identifies the freelist.
 */
static void remove_block_head(struct xv_pool *pool,
			struct block_header *block, u32 slindex)
{
	struct block_header *tmpblock;
	u32 flindex = slindex / BITS_PER_LONG;

	pool->freelist[slindex].page = block->link.next_page;
	pool->freelist[slindex].offset = block->link.next_offset;
	block->link.prev_page = 0;
	block->link.prev_offset = 0;

	if (!pool->freelist[slindex].page) {
		__clear_bit(slindex % BITS_PER_LONG, &pool->slbitmap[flindex]);
		if (!pool->slbitmap[flindex])
			__clear_bit(flindex, &pool->flbitmap);
	} else {
		/*
		 * DEBUG ONLY: We need not reinitialize freelist head previous
		 * pointer to 0 - we never depend on its value. But just for
		 * sanity, lets do it.
		 */
		tmpblock = get_ptr_atomic(pool->freelist[slindex].page,
				pool->freelist[slindex].offset, KM_USER1);
		tmpblock->link.prev_page = 0;
		tmpblock->link.prev_offset = 0;
		put_ptr_atomic(tmpblock, KM_USER1);
	}
}

/*
 * Remove block from freelist. Index 'slindex' identifies the freelist.
 */
static void remove_block(struct xv_pool *pool, struct page *page, u32 offset,
			struct block_header *block, u32 slindex)
{
	u32 flindex;
	struct block_header *tmpblock;

	if (pool->freelist[slindex].page == page
	   && pool->freelist[slindex].offset == offset) {
		remove_block_head(pool, block, slindex);
		return;
	}

	flindex = slindex / BITS_PER_LONG;

	if (block->link.prev_page) {
		tmpblock = get_ptr_atomic(block->link.prev_page,
				block->link.prev_offset, KM_USER1);
		tmpblock->link.next_page = block->link.next_page;
		tmpblock->link.next_offset = block->link.next_offset;
		put_ptr_atomic(tmpblock, KM_USER1);
	}

	if (block->link.next_page) {
		tmpblock = get_ptr_atomic(block->link.next_page,
				block->link.next_offset, KM_USER1);
		tmpblock->link.prev_page = block->link.prev_page;
		tmpblock->link.prev_offset = block->link.prev_offset;
		put_ptr_atomic(tmpblock, KM_USER1);
	}
}

/*
 * Allocate a page and add it freelist of given pool.
 */
static int grow_pool(struct xv_pool *pool, gfp_t flags)
{
	struct page *page;
	struct block_header *block;
	page = alloc_page(flags);
	if (unlikely(!page))
		return -ENOMEM;

	stat_inc(&pool->total_pages);

	spin_lock(&pool->lock);
	block = get_ptr_atomic(page, 0, KM_USER0);

	block->size = PAGE_SIZE - XV_ALIGN;
	set_flag(block, BLOCK_FREE);
	clear_flag(block, PREV_FREE);
	set_blockprev(block, 0);

	insert_block(pool, page, 0, block);

	put_ptr_atomic(block, KM_USER0);
	spin_unlock(&pool->lock);

	return 0;
}

/*
 * Create a memory pool. Allocates freelist, bitmaps and other
 * per-pool metadata.
 */
struct xv_pool *xv_create_pool(void)
{
	
	u32 ovhd_size;
	struct xv_pool *pool;

	ovhd_size = roundup(sizeof(*pool), PAGE_SIZE);
	pool = kzalloc(ovhd_size, GFP_KERNEL);
	if (!pool)
		return NULL;

	spin_lock_init(&pool->lock);
	//printk("0------------------------------------------------\n");
	return pool;
}

void xv_destroy_pool(struct xv_pool *pool)
{
	kfree(pool);
}

/**
 * xv_malloc - Allocate block of given size from pool.
 * @pool: pool to allocate from
 * @size: size of block to allocate
 * @page: page no. that holds the object
 * @offset: location of object within page
 *
 * On success, <page, offset> identifies block allocated
 * and 0 is returned. On failure, <page, offset> is set to
 * 0 and -ENOMEM is returned.
 *
 * Allocation requests with size > XV_MAX_ALLOC_SIZE will fail.
 */
int xv_malloc(struct xv_pool *pool, u32 size, struct page **page,
		u32 *offset, gfp_t flags)
{
	//printk("1------------------------------------------------\n");
	int error;
	u32 index, tmpsize, origsize, tmpoffset;
	struct block_header *block, *tmpblock;
	//printk("2------------------------------------------------\n");
	//printk("333------------------------------------------------\n");
	*page = NULL;
	//printk("2.1------------------------------------------------\n");
	*offset = 0;
	//printk("2.2------------------------------------------------\n");
	origsize = size;
	//printk("3------------------------------------------------\n");
	if (unlikely(!size || size > XV_MAX_ALLOC_SIZE))
		return -ENOMEM;
	//printk("3------------------------------------------------\n");
	size = ALIGN(size, XV_ALIGN);
	//printk("4------------------------------------------------\n");
	spin_lock(&pool->lock);
	//printk("5------------------------------------------------\n");
	index = find_block(pool, size, page, offset);
	//printk("6------------------------------------------------\n");
	if (!*page) {
		//printk("7------------------------------------------------\n");
		spin_unlock(&pool->lock);
		//printk("8------------------------------------------------\n");
		if (flags & GFP_NOWAIT)
		{
			//printk("8.1------------------------------------------------\n");
			return -ENOMEM;
		}
		//printk("9------------------------------------------------\n");
		error = grow_pool(pool, flags);
		//printk("10------------------------------------------------\n");
		if (unlikely(error))
			return error;
	    //printk("11------------------------------------------------\n");
		spin_lock(&pool->lock);
		//printk("12------------------------------------------------\n");
		index = find_block(pool, size, page, offset);
		//printk("13------------------------------------------------\n");
	}
	//printk("14------------------------------------------------\n");
	if (!*page) {
		//printk("15------------------------------------------------\n");
		spin_unlock(&pool->lock);
		return -ENOMEM;
	}
	//printk("16------------------------------------------------\n");
	block = get_ptr_atomic(*page, *offset, KM_USER0);
	//printk("17------------------------------------------------\n");
	remove_block_head(pool, block, index);
	//printk("18------------------------------------------------\n");		
	/* Split the block if required */
	tmpoffset = *offset + size + XV_ALIGN;
	tmpsize = block->size - size;
	tmpblock = (struct block_header *)((char *)block + size + XV_ALIGN);
	//printk("18------------------------------------------------\n");
	if (tmpsize) {
		//printk("18------------------------------------------------\n");
		tmpblock->size = tmpsize - XV_ALIGN;
		set_flag(tmpblock, BLOCK_FREE);
		clear_flag(tmpblock, PREV_FREE);
		//printk("19------------------------------------------------\n");
		set_blockprev(tmpblock, *offset);
		if (tmpblock->size >= XV_MIN_ALLOC_SIZE)
			insert_block(pool, *page, tmpoffset, tmpblock);
		//printk("20------------------------------------------------\n");
		if (tmpoffset + XV_ALIGN + tmpblock->size != PAGE_SIZE) {
			//printk("21------------------------------------------------\n");
			tmpblock = BLOCK_NEXT(tmpblock);
			set_blockprev(tmpblock, tmpoffset);
		}
		//printk("22------------------------------------------------\n");
	} else {
		/* This block is exact fit */
		//printk("23------------------------------------------------\n");
		if (tmpoffset != PAGE_SIZE)
			clear_flag(tmpblock, PREV_FREE);
		//printk("24------------------------------------------------\n");
	}
	//printk("25------------------------------------------------\n");
	block->size = origsize;
	//printk("26------------------------------------------------\n");
	clear_flag(block, BLOCK_FREE);
	//printk("27------------------------------------------------\n");
	put_ptr_atomic(block, KM_USER0);
	//printk("28------------------------------------------------\n");
	spin_unlock(&pool->lock);
	//printk("29------------------------------------------------\n");
	*offset += XV_ALIGN;
	//printk("30------------------------------------------------\n");
	return 0;
}

/*
 * Free block identified with <page, offset>
 */
void xv_free(struct xv_pool *pool, struct page *page, u32 offset)
{
	void *page_start;
	struct block_header *block, *tmpblock;

	offset -= XV_ALIGN;

	spin_lock(&pool->lock);

	page_start = get_ptr_atomic(page, 0, KM_USER0);
	block = (struct block_header *)((char *)page_start + offset);

	/* Catch double free bugs */
	BUG_ON(test_flag(block, BLOCK_FREE));

	block->size = ALIGN(block->size, XV_ALIGN);

	tmpblock = BLOCK_NEXT(block);
	if (offset + block->size + XV_ALIGN == PAGE_SIZE)
		tmpblock = NULL;

	/* Merge next block if its free */
	if (tmpblock && test_flag(tmpblock, BLOCK_FREE)) {
		/*
		 * Blocks smaller than XV_MIN_ALLOC_SIZE
		 * are not inserted in any free list.
		 */
		if (tmpblock->size >= XV_MIN_ALLOC_SIZE) {
			remove_block(pool, page,
				    offset + block->size + XV_ALIGN, tmpblock,
				    get_index_for_insert(tmpblock->size));
		}
		block->size += tmpblock->size + XV_ALIGN;
	}

	/* Merge previous block if its free */
	if (test_flag(block, PREV_FREE)) {
		tmpblock = (struct block_header *)((char *)(page_start) +
						get_blockprev(block));
		offset = offset - tmpblock->size - XV_ALIGN;

		if (tmpblock->size >= XV_MIN_ALLOC_SIZE)
			remove_block(pool, page, offset, tmpblock,
				    get_index_for_insert(tmpblock->size));

		tmpblock->size += block->size + XV_ALIGN;
		block = tmpblock;
	}

	/* No used objects in this page. Free it. */
	if (block->size == PAGE_SIZE - XV_ALIGN) {
		put_ptr_atomic(page_start, KM_USER0);
		spin_unlock(&pool->lock);

		__free_page(page);
		stat_dec(&pool->total_pages);
		return;
	}

	set_flag(block, BLOCK_FREE);
	if (block->size >= XV_MIN_ALLOC_SIZE)
		insert_block(pool, page, offset, block);

	if (offset + block->size + XV_ALIGN != PAGE_SIZE) {
		tmpblock = BLOCK_NEXT(block);
		set_flag(tmpblock, PREV_FREE);
		set_blockprev(tmpblock, offset);
	}

	put_ptr_atomic(page_start, KM_USER0);
	spin_unlock(&pool->lock);
}

u32 xv_get_object_size(void *obj)
{
	struct block_header *blk;

	blk = (struct block_header *)((char *)(obj) - XV_ALIGN);
	return blk->size;
}

/*
 * Returns total memory used by allocator (userdata + metadata)
 */
u64 xv_get_total_size_bytes(struct xv_pool *pool)
{
	return pool->total_pages << PAGE_SHIFT;
}