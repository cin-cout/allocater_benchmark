#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/uaccess.h>

#include <linux/zsmalloc.h>
#include "xvmalloc.h"

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("National Cheng Kung University, Taiwan");
MODULE_DESCRIPTION("allocator benchmark driver");
MODULE_VERSION("0.1");

#define DEV_BENCH_NAME "bench"
#define MAX_LENGTH 5056

static dev_t bench_dev = 0;
static struct cdev *bench_cdev;
static struct class *bench_class;
struct zs_pool *zpool;
struct xv_pool *xpool;

static size_t get_random_block_size(size_t blk_min, size_t blk_max)
{
    if (blk_max > blk_min)
        return blk_min + ((size_t) prandom_u32() % (blk_max - blk_min));
    return blk_min;
}

static void run_zsmalloc_benchmark(size_t loops,
                                size_t blk_min,
                                size_t blk_max,
                                unsigned long *handle_array,
                                size_t num_blks)
{

    while (loops--) {
        size_t next_idx = (size_t) prandom_u32() % num_blks;
        size_t blk_size = get_random_block_size(blk_min, blk_max);
        

        if (handle_array[next_idx]) {
			//zs_unmap_object(zpool, handle_array[i]);
            zs_free(zpool, handle_array[next_idx]);
            /* Insert the newly alloced block into the array at a random
             * point.
             */
            handle_array[next_idx] = zs_malloc(zpool, blk_size, GFP_KERNEL);
            //zs_map_object(zpool, handle_array[next_idx], ZS_MM_RW);
			//zs_unmap_object(zpool, handle_array[next_idx]);
        } else {
            /* Insert the newly alloced block into the array at a random point.
             */
            handle_array[next_idx] = zs_malloc(zpool, blk_size, GFP_KERNEL);
            //zs_map_object(zpool, handle_array[next_idx], ZS_MM_RW);
			//zs_unmap_object(zpool, handle_array[next_idx]);
        }
    }

    /* Free up all allocated blocks. */
    for (size_t i = 0; i < num_blks; i++) {
        if (handle_array[i]){
            //zs_unmap_object(zpool, handle_array[i]);
            zs_free(zpool, handle_array[i]);
        }
    }
}

static void run_xvmalloc_benchmark(size_t loops,
                                size_t blk_min,
                                size_t blk_max,
                                struct page **page_array,
                                u32 *offset_array,
                                size_t num_blks)
{

    while (loops--) {
        size_t next_idx = (size_t) prandom_u32() % num_blks;
        size_t blk_size = get_random_block_size(blk_min, blk_max);
        

        if (page_array[next_idx]) {
            xv_free(xpool, page_array[next_idx], offset_array[next_idx]);
            /* Insert the newly alloced block into the array at a random
             * point.
             */
            xv_malloc(xpool, blk_size, &page_array[next_idx], &offset_array[next_idx], __GFP_ZERO);
        } else {
            /* Insert the newly alloced block into the array at a random point.
             */
            xv_malloc(xpool, blk_size, &page_array[next_idx], &offset_array[next_idx], __GFP_ZERO);
        }
    }

    /* Free up all allocated blocks. */
    for (size_t i = 0; i < num_blks; i++) {
        if (page_array[i])
            xv_free(xpool, page_array[i], offset_array[i]);
    }
}

static int bench_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int bench_release(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t bench_read(struct file *file,
                        char *buf,
                        size_t size,
                        loff_t *offset)
{
    return 0;
}

static ssize_t bench_write(struct file *file,
                         const char *buf,
                         size_t size,
                         loff_t *offset)
{

    ktime_t kt;
    size_t num_blks = 10000;
    size_t loops = 10000000;

    static unsigned long handle_array[10000];
    memset(handle_array, 0, sizeof(handle_array));

    
    static struct page *page_array[10000];
    memset(page_array, 0, sizeof(page_array));
    static u32 offset_array[10000];
    memset(offset_array, 0, sizeof(offset_array));
    

    switch (size) {
    case 0:
        kt = ktime_get();
        zpool = zs_create_pool("mypool");
        run_zsmalloc_benchmark(loops, *offset, *offset+32, handle_array, num_blks);
        zs_destroy_pool(zpool);
        kt = ktime_sub(ktime_get(), kt);
        break;
    case 1:
        kt = ktime_get();
        xpool = xv_create_pool();
        run_xvmalloc_benchmark(loops, *offset, *offset+32, page_array, offset_array, num_blks);
        xv_destroy_pool(xpool);
        kt = ktime_sub(ktime_get(), kt);
        break;
    case 2:
        kt = ktime_get();
        zpool = zs_create_pool("mypool");
        run_zsmalloc_benchmark(loops, *offset, *offset+4096, handle_array, num_blks);
        zs_destroy_pool(zpool);
        kt = ktime_sub(ktime_get(), kt);
        break;
    case 3:
        kt = ktime_get();
        xpool = xv_create_pool();
        run_xvmalloc_benchmark(loops, *offset, *offset+4096, page_array, offset_array, num_blks);
        xv_destroy_pool(xpool);
        kt = ktime_sub(ktime_get(), kt);
        break;
    case 4:
        kt = ktime_get();
        zpool = zs_create_pool("mypool");
        run_zsmalloc_benchmark(loops, *offset, *offset+2048, handle_array, num_blks);
        zs_destroy_pool(zpool);
        kt = ktime_sub(ktime_get(), kt);
        break;
    case 5:
        kt = ktime_get();
        xpool = xv_create_pool();
        run_xvmalloc_benchmark(loops, *offset, *offset+2048, page_array, offset_array, num_blks);
        xv_destroy_pool(xpool);
        kt = ktime_sub(ktime_get(), kt);
        break;
    }

    return (ssize_t) ktime_to_ns(kt);
}

static loff_t bench_device_lseek(struct file *file, loff_t offset, int orig)
{
    loff_t new_pos = 0;
    switch (orig) {
    case 0: /* SEEK_SET: */
        new_pos = offset;
        break;
    case 1: /* SEEK_CUR: */
        new_pos = file->f_pos + offset;
        break;
    case 2: /* SEEK_END: */
        new_pos = MAX_LENGTH - offset;
        break;
    }

    if (new_pos > MAX_LENGTH)
        new_pos = MAX_LENGTH;  // max case
    if (new_pos < 0)
        new_pos = 0;        // min case
    file->f_pos = new_pos;  // This is what we'll use now
    return new_pos;
}

const struct file_operations bench_fops = {
    .owner = THIS_MODULE,
    .read = bench_read,
    .write = bench_write,
    .open = bench_open,
    .release = bench_release,
    .llseek = bench_device_lseek,
};

static int __init init_bench_dev(void)
{
    int rc = 0;

    // Let's register the device
    // This will dynamically allocate the major number
    rc = alloc_chrdev_region(&bench_dev, 0, 1, DEV_BENCH_NAME);

    if (rc < 0) {
        printk(KERN_ALERT
               "Failed to register the fibonacci char device. rc = %i",
               rc);
        return rc;
    }

    bench_cdev = cdev_alloc();
    if (bench_cdev == NULL) {
        printk(KERN_ALERT "Failed to alloc cdev");
        rc = -1;
        goto failed_cdev;
    }
    bench_cdev->ops = &bench_fops;
    rc = cdev_add(bench_cdev, bench_dev, 1);

    if (rc < 0) {
        printk(KERN_ALERT "Failed to add cdev");
        rc = -2;
        goto failed_cdev;
    }

    bench_class = class_create(THIS_MODULE, DEV_BENCH_NAME);

    if (!bench_class) {
        printk(KERN_ALERT "Failed to create device class");
        rc = -3;
        goto failed_class_create;
    }

    if (!device_create(bench_class, NULL, bench_dev, NULL, DEV_BENCH_NAME)) {
        printk(KERN_ALERT "Failed to create device");
        rc = -4;
        goto failed_device_create;
    }
    return rc;
failed_device_create:
    class_destroy(bench_class);
failed_class_create:
    cdev_del(bench_cdev);
failed_cdev:
    unregister_chrdev_region(bench_dev, 1);
    return rc;
}

static void __exit exit_bench_dev(void)
{
    device_destroy(bench_class, bench_dev);
    class_destroy(bench_class);
    cdev_del(bench_cdev);
    unregister_chrdev_region(bench_dev, 1);
}

module_init(init_bench_dev);
module_exit(exit_bench_dev);