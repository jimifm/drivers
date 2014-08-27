#include <linux/module.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/i2c.h>

/******/
struct ik_platform_data{

    u16 page_size;
};

static struct ik_platform_data i2c_kb = {
    .page_size =16,
};

static struct i2c_board_info kb_i2c_devs[] __initdata = {

    {
        I2C_BOARD_INFO("lala",0x50),
        .platform_data = &i2c_kb,
    },
};

//i2c_register_board_info(0, kb_i2c_devs, ARRAY_SIZE(kb_i2c_devs)); 
/******/

static const struct i2c_device_id i2c_keyborad_ids[] = {
    {"lala", 0},
    {/*END OF LIST*/}
};

MODULE_DEVICE_TABLE(i2c, i2c_keyborad_ids);

static int i2c_keyborad_probe(struct i2c_client *client,
    const struct i2c_device_id *id)
{
    //struct i2c_adapter *adap = client->adapter;
    //struct ik_platform_data *pdata = client->dev.platform_data;
    //int ret = -ENODEV;
	//if (!i2c_check_functionality(adap, I2C_FUNC_SMBUS_BYTE_DATA))
		//goto err;
//err:
 //   return ret;
    int ret = 0;
    if(!i2c_check_functionality(client->adapter,I2C_FUNC_I2C)){
        
        ret = -EINVAL;
        goto err;
    }
    printk("i2c_keyborad_probe okay..\n");
    //return 0;

err:
    return ret;
    
}

static int i2c_keborad_remove(struct i2c_client *client)
{
    printk("i2c_keborad_remove okay..\n");
    return 0;
}

static struct i2c_driver keyborad_driver = {

    .driver = {
        .name = "i2c_keyborad",
        .owner = THIS_MODULE,
    },
    .probe = i2c_keyborad_probe,
    .remove = i2c_keborad_remove,
    .id_table = i2c_keyborad_ids, 
};

static int __init keyborad_init(void)
{
    return i2c_add_driver(&keyborad_driver);
}

static void __exit keyborad_exit(void)
{
    i2c_del_driver(&keyborad_driver);
}
module_init(keyborad_init);
module_exit(keyborad_exit);

MODULE_AUTHOR("Zhang Hong");
MODULE_DESCRIPTION("i2c keyborad driver");
MODULE_LICENSE("GPL v2");
