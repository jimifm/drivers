#include <linux/module.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/delay.h>
//#include <asm/uaccess.h>
//#include <linux/rwsem.h>
struct i2c_devinfo {
	struct list_head	list;
	int			busnum;
	struct i2c_board_info	board_info;
};
#define TIME_PERIOD_MS 50//ms
#define I2C_KB_KEYNUM 8
/******/
struct ik_platform_data{

    u32 cycle_time;//us
};

static struct ik_platform_data i2c_hdmi = {
    .cycle_time = 15000,//15ms
};

static struct i2c_board_info kb_i2c_devs[] __initdata = {

    {
        I2C_BOARD_INFO("swithdmi",0x90),
        .platform_data = &i2c_hdmi,
    },
};

//i2c_register_board_info(1, kb_i2c_devs, ARRAY_SIZE(kb_i2c_devs)); 
/******/

static const struct i2c_device_id i2c_hdmi_ids[] = {
    {"swithdmi", 0},
    {/*END OF LIST*/}
};

MODULE_DEVICE_TABLE(i2c, i2c_hdmi_ids);

static int i2ckb_write_reg(struct i2c_client *client, u8 reg, u8 val) 
{
    int ret = i2c_smbus_write_byte_data(client, reg, val);
    
    if(0 > ret)
        dev_err(&client->dev, "%s: reg 0x%x, val 0x%x, err %d\n",
            __func__, reg, val, ret);
    return ret;
}

static inline void i2ckb_catnap(struct i2c_client *client)
{
    i2ckb_write_reg(client, 2/*command*/, 6/*value*/);
}

static inline void i2ckb_fall_deepsleep(struct i2c_client *client)
{
    i2ckb_write_reg(client, 3, 4);
}


static int i2c_hdmi_probe(struct i2c_client *client,
    const struct i2c_device_id *id)
{
    struct i2c_adapter *adap = client->adapter;
    const struct ik_platform_data *pdata = client->dev.platform_data;

    printk("i2c_hdmi_probe okay..\n");
    return 0;
    
}

static int i2c_hdmi_remove(struct i2c_client *client)
{
    printk("i2c_hdmi_remove okay..\n");
    return 0;
}

static struct i2c_driver keyborad_driver = {

    .driver = {
        .name = "i2c_hdmi",
        .owner = THIS_MODULE,
    },
    .probe = i2c_hdmi_probe,
    .remove = i2c_hdmi_remove,
    .id_table = i2c_hdmi_ids, 
};

static int __init i2chdmi_init(void)
{
    //i2c_register_board_info(0, kb_i2c_devs, ARRAY_SIZE(kb_i2c_devs)); 
    return i2c_add_driver(&keyborad_driver);
}

static void __exit i2chdmi_exit(void)
{
    i2c_del_driver(&keyborad_driver);
}
module_init(i2chdmi_init);
module_exit(i2chdmi_exit);

MODULE_AUTHOR("Zhang Hong");
MODULE_DESCRIPTION("i2c hdmi driver");
MODULE_LICENSE("GPL v2");
