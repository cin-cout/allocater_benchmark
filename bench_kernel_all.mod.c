#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0xdd8f8694, "module_layout" },
	{ 0x7e322334, "alloc_pages_current" },
	{ 0x4ddb27b7, "cdev_alloc" },
	{ 0x22b90774, "cdev_del" },
	{ 0xc69b7ee5, "zs_destroy_pool" },
	{ 0xb29533ee, "zs_malloc" },
	{ 0xb43f9365, "ktime_get" },
	{ 0x22e92418, "device_destroy" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x97651e6c, "vmemmap_base" },
	{ 0x5ab904eb, "pv_ops" },
	{ 0x958df3ac, "zs_free" },
	{ 0xfb578fc5, "memset" },
	{ 0x56b1771b, "current_task" },
	{ 0xc5850110, "printk" },
	{ 0xbcf1f0e6, "zs_create_pool" },
	{ 0x7749276a, "device_create" },
	{ 0xfda9581f, "prandom_u32" },
	{ 0xc4952f09, "cdev_add" },
	{ 0x7cd8d75e, "page_offset_base" },
	{ 0xb1058e35, "__free_pages" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0xb8b9f817, "kmalloc_order_trace" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0xdbf17652, "_raw_spin_lock" },
	{ 0x37a0cba, "kfree" },
	{ 0xb65e5a32, "class_destroy" },
	{ 0x2871e975, "__class_create" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "E91E46F1B84DA7AA6F2C206");
