#include <linux/module.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/delay.h>
//#include <linux/interrupt.h>
#include <linux/workqueue.h>
//#include <asm/uaccess.h>
//#include <linux/rwsem.h>
#define keyboard_print(x,arg...) printk(KERN_INFO"[VFE_DEV_I2C]"x,##arg)
#define keyboard_err(x,arg...) printk(KERN_ERR"[VFE_DEV_I2C_ERR]"x,##arg)
struct i2c_devinfo {
	struct list_head	list;
	int			busnum;
	struct i2c_board_info	board_info;
};
#define TIME_PERIOD_MS 200//ms
#define I2C_KB_KEYNUM 19
/******/
struct ik_platform_data{

    u32 cycle_time;//us
};

static struct ik_platform_data i2c_kb = {
    .cycle_time = 15000,//15ms
};

static struct i2c_board_info kb_i2c_devs[] __initdata = {

    {
        I2C_BOARD_INFO("swit",0x28),
        .platform_data = &i2c_kb,
    },
};

//i2c_register_board_info(1, kb_i2c_devs, ARRAY_SIZE(kb_i2c_devs)); 
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
#if 0
static void max7359_build_keycode(struct max7359_keypad *keypad,
				const struct matrix_keymap_data *keymap_data)
{
	struct input_dev *input_dev = keypad->input_dev;
	int i;

	for (i = 0; i < keymap_data->keymap_size; i++) {
		unsigned int key = keymap_data->keymap[i];
		unsigned int row = KEY_ROW(key);
		unsigned int col = KEY_COL(key);
		unsigned int scancode = MATRIX_SCAN_CODE(row, col,
						MAX7359_ROW_SHIFT);
		unsigned short keycode = KEY_VAL(key);

		keypad->keycodes[scancode] = keycode;
		__set_bit(keycode, input_dev->keybit);
	}
	__clear_bit(KEY_RESERVED, input_dev->keybit);
}
#endif

void i2ckb_build_keymap()
{

	struct input_dev *input_dev = keypad->input_dev;
	int i;
    for(i = 0; i< I2C_KB_KEYNUM; i++)
    {
        keypad->keycodes[i] = 102 + i;
        __set_bit(102 + i, input_dev->keybit);
    }
    __clear_bit(KEY_RESERVED, input_dev->keybit);
}

int i2ckb_read_reg(struct i2c_client *client, unsigned char addr,
    unsigned char *value)
{
  unsigned char data[2];
  struct i2c_msg msg[2];
  int ret;
  
  data[0] = addr;
  data[1] = 0xee;
  /*
   * Send out the register address...
   */
  msg[0].addr = client->addr;
  msg[0].flags = 0;
  msg[0].len = 1;
  msg[0].buf = &data[0];
  /*
   * ...then read back the result.
   */
  msg[1].addr = client->addr;
  msg[1].flags = I2C_M_RD;
  msg[1].len = 1;
  msg[1].buf = &data[1];
  
  ret = i2c_transfer(client->adapter, msg, 2);
  if (ret >= 0) {
    *value = data[1];
    ret = 0;
  } else {
    keyboard_err("%s error! slave = 0x%x, addr = 0x%2x, value = 0x%2x\n ",__func__, client->addr, addr,*value);
  }
  return ret;
}
//EXPORT_SYMBOL_GPL(cci_read_a8_d8);

int i2ckb_write_reg(struct i2c_client *client, unsigned char addr,
    unsigned char value)
{
  struct i2c_msg msg;
  unsigned char data[2];
  int ret;
  
  data[0] = addr;
  data[1] = value;
  
  msg.addr = client->addr;
  msg.flags = 0;
  msg.len = 2;
  msg.buf = data;

  ret = i2c_transfer(client->adapter, &msg, 1);
  if (ret >= 0) {
    ret = 0;
  } else {
    keyboard_err("%s error! slave = 0x%x, addr = 0x%2x, value = 0x%2x\n ",__func__, client->addr, addr,value);
  }
  return ret;
}
//EXPORT_SYMBOL_GPL(cci_write_a8_d8);
/*static int i2ckb_read_reg(struct i2c_client *client, uint8_t reg, uint8_t *data)
{
    //int ret = i2c_smbus_read_byte_data(client ,reg);
    int ret = i2c_master_send(client, &reg, 1);
    if(ret < 0){
        dev_err(&client->dev, "%s[send]: reg 0x%x, err %d\n",
            __func__, reg, ret);
        return ret;
    }
    msleep(10);

    printk("master send ok!!\n");

    if(1 != i2c_master_recv(client, data, 1))
    {
        dev_err(&client->dev, "%s[recv]: reg 0x%x, err %d\n",
            __func__, reg, ret);
    }

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
*/
static inline void i2ckb_catnap(struct i2c_client *client)
{
    i2ckb_write_reg(client, 2/*command*/, 6/*value*/);
}

static inline void i2ckb_fall_deepsleep(struct i2c_client *client)
{
    i2ckb_write_reg(client, 3, 4);
}
static unsigned char cur_code = 0;
static unsigned char cur_encoder = 0;
void i2ckb_wq_fun(unsigned long data)
{

    uint8_t scancode = 0;
    //tasklet_schedule(&i2ckb_tasklet);
    int val = i2ckb_read_reg(keypad->client, 0x35, &scancode/*I2C_COMMAND*/);//°´¼ü¶ÁÈ¡
    //printk(" val %d\n",val); 
    if(val == 0){
        if(scancode != 0){
            input_event(keypad->input_dev, EV_MSC, MSC_SCAN, scancode + 101);
            input_report_key(keypad->input_dev,keypad->keycodes[scancode], 1);
            input_sync(keypad->input_dev);
            cur_code = scancode;
            printk("key down: %d\n",scancode);
        }
        else
        {
            if(cur_code != 0)
            {
                input_event(keypad->input_dev, EV_MSC, MSC_SCAN, cur_code + 101);
                input_report_key(keypad->input_dev,keypad->keycodes[cur_code], 0);
                input_sync(keypad->input_dev);
                printk("key release: %d\n",cur_code);
                cur_code = 0;
            }
        }
    }

    val = i2ckb_read_reg(keypad->client, 0x36, &scancode);//Ðý×ª±àÂëÆ÷¼üÖµ×ª»»

    if(val == 0){
        if(scancode != 0){
            input_event(keypad->input_dev, EV_MSC, MSC_SCAN, scancode + 12);
            input_report_key(keypad->input_dev,keypad->keycodes[scancode], 1);
            input_sync(keypad->input_dev);
            //cur_encoder = scancode;
            printk("key down: %d\n",scancode);
            input_event(keypad->input_dev, EV_MSC, MSC_SCAN, scancode + 12);
            input_report_key(keypad->input_dev,keypad->keycodes[scancode], 0);
            input_sync(keypad->input_dev);
            printk("key release: %d\n",scancode);
            //cur_encoder = 0;
        }
       /* else
        {
            if(cur_encoder != 0)
            {
                input_event(keypad->input_dev, EV_MSC, MSC_SCAN, cur_code + 12);
                input_report_key(keypad->input_dev,keypad->keycodes[cur_code], 0);
                input_sync(keypad->input_dev);
                printk("key release: %d\n",cur_encoder);
                cur_encoder = 0;
            }
        }*/
    }

}

struct work_struct i2ckb_wq;

static void i2c_timer_handle(unsigned long arg)
{
    //input_event...
    //
    //uint8_t reg = 0x35;
    //uint8_t scancode = 0;
    schedule_work(&i2ckb_wq);
    //int val = i2ckb_read_reg(keypad->client, 0x35, &scancode/*I2C_COMMAND*/);
    //printk("key value %d\n",scancode);
   // printk("schedule tasklet_schedule\n");
    /*if(val = 0){
        //input_event();
        //input_report_key();
        input_sync(keypad->input_dev);
    }*/
    mod_timer(&keypad->s_timer, jiffies + msecs_to_jiffies(TIME_PERIOD_MS));
}

static int i2c_kb_open(struct input_dev *dev)
{
   struct i2c_kb *keypad = input_get_drvdata(dev);

   //i2ckb_catnap(keypad->client);

    INIT_WORK(&i2ckb_wq, (void (*)(void *))i2ckb_wq_fun);
    
    init_timer(&keypad->s_timer);
    keypad->s_timer.function = &i2c_timer_handle;
    keypad->s_timer.expires = jiffies + msecs_to_jiffies(TIME_PERIOD_MS);
    
    //i2c_timer = &keypad->s_timer;
    add_timer(&keypad->s_timer);
    
    /*uint8_t reg = 0x35;
    int ret = i2c_master_send(keypad->client, &reg, 1);
    if(ret < 0){
        dev_err(&keypad->client->dev, "%s[send]: reg 0x%x, err %d\n",
            __func__, reg, ret);
        return ret;
    }*/
   //unsigned char data;
   //i2ckb_read_reg(keypad->client, 0x35,&data);
   // printk("i2c master read : %d!\n",data);
   return 0;
}

static void i2c_kb_close(struct input_dev *dev)
{
    //struct i2c_kb *keypad = input_get_drvdata(dev);
    printk("usr space close fd!\n");
   del_timer_sync(&keypad->s_timer);
    //i2ckb_fall_deepsleep(keypad->client);
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

    i2ckb_build_keymap();
    //
    error = input_register_device(input_dev);
    if(error){
    
        dev_err(&client->dev, "failed to register input device\n");
        goto failed_free_mem;
    }
    //init

    
    //i2c_timer = &keypad->s_timer;
    i2c_set_clientdata(client, keypad);
    device_init_wakeup(&client->dev, 1);

    
    printk("i2c_keyborad_probe okay..\n");
    return 0;

failed_free_mem:
    input_free_device(input_dev);
    //del_timer(&keypad->s_timer);
    kfree(keypad);
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
