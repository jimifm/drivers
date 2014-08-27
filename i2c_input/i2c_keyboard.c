#include <linux/module.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/input.h>
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

static struct ik_platform_data i2c_kb = {
    .cycle_time = 15000,//15ms
};

static struct i2c_board_info kb_i2c_devs[] __initdata = {

    {
        I2C_BOARD_INFO("swit",0x50),
        .platform_data = &i2c_kb,
    },
};

//i2c_register_board_info(0, kb_i2c_devs, ARRAY_SIZE(kb_i2c_devs)); 
/******/

static const struct i2c_device_id i2c_keyborad_ids[] = {
    {"swit", 0},
    {/*END OF LIST*/}
};

MODULE_DEVICE_TABLE(i2c, i2c_keyborad_ids);
struct i2c_kb{

    unsigned short keycodes[I2C_KB_KEYNUM];
    struct input_dev* input_dev;
    struct i2c_client *client;
    //struct delayed_work period_work;
    struct timer_list s_timer;
};


static struct i2c_kb *keypad;

static int i2ckb_read_reg(struct i2c_client *client, int reg)
{
    int ret = i2c_smbus_read_byte_data(client ,reg);

    if(ret < 0)
        dev_err(&client->dev, "%s: reg 0x%x, err %d\n",
            __func__, reg, ret);
    return ret;
}

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

static void i2c_timer_handle(unsigned long arg)
{
    //input_event...
    //
    int val = i2ckb_read_reg(keypad->client, 8/*I2C_COMMAND*/);
    
    //input_event();
    //input_report_key();
    input_sync(keypad->input_dev);
    
    mod_timer(&keypad->s_timer, jiffies + msecs_to_jiffies(TIME_PERIOD_MS));
}

static int i2c_kb_open(struct input_dev *dev)
{
   struct i2c_kb *keypad = input_get_drvdata(dev);

   i2ckb_catnap(keypad->client);

   return 0;
}

static void i2c_kb_close(struct input_dev *dev)
{
    struct i2c_kb *keypad = input_get_drvdata(dev);

    i2ckb_fall_deepsleep(keypad->client);
}

static int i2c_keyborad_probe(struct i2c_client *client,
    const struct i2c_device_id *id)
{
    struct i2c_adapter *adap = client->adapter;
    const struct ik_platform_data *pdata = client->dev.platform_data;
    struct input_dev *input_dev = NULL;
    int error;
    //int ret = -ENODEV;
	//if (!i2c_check_functionality(adap, I2C_FUNC_SMBUS_BYTE_DATA))
		//goto err;
//err:
 //   return ret;
    //int ret = 0;
    if(!i2c_check_functionality(client->adapter,I2C_FUNC_I2C)){
        
        error = -EINVAL;
        goto err_check;
    }

    keypad = kzalloc(sizeof(struct i2c_kb) ,GFP_KERNEL);

    input_dev = input_allocate_device();
    if(!keypad || !input_dev){
    
        dev_err(&client->dev ,"failed to allocate memory\n");
        error = -ENOMEM;
        goto failed_free_mem;
    }

    keypad->client = client;
    keypad->input_dev = input_dev;

    input_dev->name = client->name;
    input_dev->id.bustype = BUS_I2C;
    input_dev->open = i2c_kb_open;
    input_dev->close = i2c_kb_close;
    input_dev->dev.parent = &client->dev;

    input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP);
    input_dev->keycodesize = sizeof(keypad->keycodes[0]);
    input_dev->keycodemax = ARRAY_SIZE(keypad->keycodes);
    input_dev->keycode = keypad->keycodes;

    //need check!?
    input_set_capability(input_dev, EV_MSC, MSC_SCAN);
    
    input_set_drvdata(input_dev, keypad);
    //i2c_kb_build_keycode();
    //
    //timer interrupt
    init_timer(&keypad->s_timer);
    keypad->s_timer.function = &i2c_timer_handle;
    keypad->s_timer.expires = jiffies + msecs_to_jiffies(TIME_PERIOD_MS);
    
    //i2c_timer = &keypad->s_timer;
    add_timer(&keypad->s_timer);

    //
    error = input_register_device(input_dev);
    if(error){
    
        dev_err(&client->dev, "failed to register input device\n");
        goto failed_free_mem;
    }
    //init

    i2c_set_clientdata(client, keypad);
    device_init_wakeup(&client->dev, 1);

    printk("i2c_keyborad_probe okay..\n");
    return 0;

failed_free_mem:
    input_free_device(input_dev);
    kfree(keypad);
    del_timer(&keypad->s_timer);
err_check:
    return error;
    
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
    //i2c_register_board_info(0, kb_i2c_devs, ARRAY_SIZE(kb_i2c_devs)); 
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
