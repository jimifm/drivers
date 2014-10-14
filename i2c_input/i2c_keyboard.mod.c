#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x4c8a6e0a, "module_layout" },
	{ 0xf50b6707, "i2c_del_driver" },
	{ 0xfa81a8f0, "i2c_register_driver" },
	{ 0xea5536ee, "input_event" },
	{ 0xcb4d8a, "i2c_transfer" },
	{ 0xa2d87cb0, "malloc_sizes" },
	{ 0x37a0cba, "kfree" },
	{ 0xe063ab88, "input_free_device" },
	{ 0x393f00cc, "device_init_wakeup" },
	{ 0x4c242d7, "input_register_device" },
	{ 0xbfafec6e, "dev_set_drvdata" },
	{ 0xcf3ad463, "input_set_capability" },
	{ 0xce118570, "dev_err" },
	{ 0xb0d34eaf, "input_allocate_device" },
	{ 0x3d22b4f3, "kmem_cache_alloc_trace" },
	{ 0xc8fd727e, "mod_timer" },
	{ 0x8949858b, "schedule_work" },
	{ 0x7d11c268, "jiffies" },
	{ 0xd9605d4c, "add_timer" },
	{ 0x3bd1b1f6, "msecs_to_jiffies" },
	{ 0x132a7a5b, "init_timer_key" },
	{ 0x55968d76, "dev_get_drvdata" },
	{ 0x6f0036d9, "del_timer_sync" },
	{ 0x27e1a049, "printk" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("i2c:swit");

MODULE_INFO(srcversion, "CD7733C874D9F6DEDD83CA8");
