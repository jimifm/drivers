#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#define colorspace_print(x,arg...) printk(KERN_INFO"[COLORSPACE_INFO]"x,##arg)
#define colorspace_error(x,arg...) printk(KERN_ERR"[COLORSPACE_ERR]"x,##arg)
static int __init colorspace_init(void)
{

}

static void __exit colorspace_exit(void)
{

}
module_init(colorspace_init);
module_exit(colorspace_exit);

MODULE_AUTHOR("Zhang Hong");
MODULE_DESCRIPTION("colorspace control driver");
MODULE_LICENSE("GPL v2");
