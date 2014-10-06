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
	{ 0x78c14ddb, "module_layout" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0xf34131ee, "__register_chrdev" },
	{ 0x362ef408, "_copy_from_user" },
	{ 0xd0d8621b, "strlen" },
	{ 0x37a0cba, "kfree" },
	{ 0x3f9b9190, "kmem_cache_alloc_trace" },
	{ 0x794a2c29, "kmalloc_caches" },
	{ 0x2f287f0d, "copy_to_user" },
	{ 0x50eedeb8, "printk" },
	{ 0xb4390f9a, "mcount" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "51A8C8D02AE4691A2088FC0");
