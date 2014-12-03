#define VIBRA_MINOR 88 

#define STARTUP_VIBRA_DELAY 150 

static unsigned int onkey_stat; 
static struct timer_list vibra_timer; 

static void vibra_stop_vibration(unsigned long data) 
{        
    struct micco_vibra *vibra = (void *)data; 
    struct vibra_control *control = vibra->vibra_ctl; 
    control->level = 0x0; 
    micco_write(MICCO_VIBRA_CONTROL, control->level); 
} 

static int raw_vibrate(int level) //驱动马达，level控制转速 
{ 
    u8 val; 
    val = level & 0x00fe; 
    DMSG("+/-vibra_ioctl/n"); 
    micco_write(MICCO_VIBRA_CONTROL, val); 
    return 0; 

} 

static int adv_vibrate(struct vibra_control *ctl) //驱动马达，level控制转速 
{ 
    u8 val; 
    val = ctl->level & 0x00fe; 
    micco_write(MICCO_VIBRA_CONTROL, val); 
    mod_timer(&vibra_timer, jiffies + msecs_to_jiffies(ctl->m_time)); 

    return 0; 
} 



static int vibra_ioctl(struct inode *inode, struct file *file, 
    unsigned int cmd, unsigned long arg) //用户空间通过ioctl操作马达 
{ 
    int ret; 
    unsigned int level; 
    u8 val; 
    unsigned int onkey_val; 
    struct vibra_control ctl; 

    switch(cmd) { 
    case VIBRA_RAW_VIBTATION: 
        ret = copy_from_user(&level, (void *)arg, sizeof(unsigned int)); 
        ret = raw_vibrate(level); 
        break; 

    case VIBRA_ADV_VIBTATION: 
        ret = copy_from_user(&ctl, (void *)arg, sizeof(struct vibra_control)); 
        ret = adv_vibrate(&ctl);        
        break; 

    case VIBRA_POWERON_STAT_GET: 
        ret = copy_to_user((void *)arg, &onkey_stat, sizeof(unsigned int)); 
        break; 

    case VIBRA_ONKEY_STAT_GET: 
        ret = micco_read(MICCO_STATUS_A, &val); 

        if (ret < 0) { 
            printk("read  MICCO_STATUS_A failed/n"); 
            onkey_val = 0; 
        } 

        onkey_val = !(val & MICCO_STATUS_A_ONKEY); 

        ret = copy_to_user((void *)arg, &onkey_val, sizeof(unsigned int)); 

        break; 

    default: 
        if (cmd < 0xFF) 
            ret = raw_vibrate(cmd); 
        else 
            ret = -ENOIOCTLCMD; 
    } 

    return ret; 
} 


/*用户空间读取马达运转时间*/ 
static ssize_t vibrator_mtime_show(struct device *dev, 
                       struct device_attribute *attr, char *buf) 
{ 
    struct micco_vibra        *vibra = dev_get_drvdata(dev); 
    struct vibra_control     *control = vibra->vibra_ctl; 

    return sprintf(buf, "%u/n", control->m_time); 
} 


/*用户空间修改马达运转时间*/ 
static ssize_t vibrator_mtime_store(struct device *dev, 
                    struct device_attribute *attr, 
                    const char *buf, size_t count) 
{ 
    struct micco_vibra        *vibra = dev_get_drvdata(dev); 
    struct vibra_control     *control = vibra->vibra_ctl; 
    char *endp; 

    control->m_time = simple_strtoul(buf, &endp, 10); 
    DMSG("vibrator_mtime_store: m_tme = %d/n",control->m_time ); 
    adv_vibrate(control); 
    return count; 
} 


/*属性设置及读写属性注册*/ 
static DEVICE_ATTR(mtime, 0664, vibrator_mtime_show, 
           vibrator_mtime_store); 

/*用户空间读马达转速*/ 
static ssize_t vibrator_level_show(struct device *dev, 
                       struct device_attribute *attr, char *buf) 
{ 
    struct micco_vibra        *vibra = dev_get_drvdata(dev); 
    struct vibra_control     *control = vibra->vibra_ctl; 

    return sprintf(buf, "%u/n", control->level); 
} 

/*用户空间调整马达转速*/ 
static ssize_t vibrator_level_store(struct device *dev, 
                    struct device_attribute *attr, 
                    const char *buf, size_t count) 
{ 
    struct micco_vibra        *vibra = dev_get_drvdata(dev); 
    struct vibra_control     *control = vibra->vibra_ctl; 
    char *endp; 

    control->level = simple_strtoul(buf, &endp, 10); 
    DMSG("vibrator_level_store:raw_vibrate(%x)", control->level); 
    adv_vibrate(control); 
    return count; 
} 


/*属性设置及读写属性注册*/ 
static DEVICE_ATTR(level, 0664, vibrator_level_show, 
           vibrator_level_store); 

static struct file_operations vibra_fops = { 
    .owner        = THIS_MODULE,  
    .ioctl      = vibra_ioctl, 
}; 

static struct miscdevice vibra_miscdev = { 
    .minor        = VIBRA_MINOR, 
    .name        = "micco_vibra", 
    .fops        = &vibra_fops, 
}; 

static int __devinit vibra_init(struct micco_vibra *vibra) 
{ 
    struct vibra_control *ctl; 
    int ret; 
    u8 val; 

    ctl = kzalloc(sizeof(*ctl), GFP_KERNEL); 
    if (ctl == NULL) 
        return -ENOMEM; 
    vibra->vibra_ctl = ctl; 
    ctl->level=0; 
    ctl->m_time=0; 


    vibra_timer.function = vibra_stop_vibration;  
    vibra_timer.data = (unsigned long) vibra; 
    init_timer(&vibra_timer); 

   /*注册vibra_miscdev设备*/ 
   ret = misc_register(&vibra_miscdev);    
   if (ret < 0) { 
        dev_err(&vibra->pdev->dev, "can't register vibra device/n"); 
        goto err2; 
  } 



    printk("/nVIBRA Probe vibra_miscdev->minor=%d/n",vibra_miscdev.minor); 
    ret = micco_read(MICCO_FAULT_LOG, &val); 
    printk("Fault Log value is 0x%x---------->/n", val); 
    ret = micco_read(MICCO_STATUS_A, &val); 
    if (ret < 0) { 
        printk("read  MICCO_STATUS_A failed/n"); 
        onkey_stat = 0; 
        return 0; 
    } 


    onkey_stat = !(val & MICCO_STATUS_A_ONKEY); 
    if (onkey_stat) { 
        raw_vibrate(0x7e); 
        mdelay(100); 
        raw_vibrate(0x0); 
    }  

   /*创建sysfs文件系统节点*/ 
   if (device_create_file(&vibra->pdev->dev, &dev_attr_level) < 0) 
        goto err3; 
   if (device_create_file(&vibra->pdev->dev, &dev_attr_mtime) < 0) 
        goto err4; 

   return 0; 
/* 
err5: 
    device_remove_file(&vibra->pdev->dev, &dev_attr_vibrator_mtime); 
*/ 
err4: 
    device_remove_file(&vibra->pdev->dev, &dev_attr_level); 
err3: 
    misc_deregister(&vibra_miscdev); 
err2: 
    kfree(ctl); 
    return ret; 
} 

static void __devexit vibra_exit(struct micco_vibra *vibra) 
{ 
    device_remove_file(&vibra->pdev->dev, &dev_attr_mtime); 
    device_remove_file(&vibra->pdev->dev, &dev_attr_level); 

    misc_deregister(&vibra_miscdev); 
} 

/**************************************************************************** 
* Initialization / Registeration / Removal 
***************************************************************************/ 
static int vibra_probe(struct platform_device *pdev) 
{ 
    struct micco_vibra            *vibra; 
    int r = -ENODEV; 

    vibra = kzalloc(sizeof(*vibra), GFP_KERNEL); 
    if (vibra == NULL) 
        return -ENOMEM; 

    /*注册驱动数据 */ 
    dev_set_drvdata(&pdev->dev, vibra); 

    vibra->pdev = pdev; 

    r = vibra_init(vibra); 
    if (r) 
        goto err1; 

    return 0; 
err1: 
    kfree(vibra); 
    return r; 

} 

static int vibra_remove(struct platform_device *pdev) 
{ 
    struct micco_vibra *vibra = dev_get_drvdata(&pdev->dev); 

    dev_dbg(&vibra->pdev->dev, "%s/n", __FUNCTION__); 
    vibra_exit(vibra); 
    kfree(vibra); 
    return 0; 
} 

static struct platform_device vibra_plat_device = { 
    .name        = "micco-vibra",  //sysfs设备目录“/sys/devices/platform/micco-vibra.0” 
    .id        = 0, 
}; 

static struct platform_driver vibra_plat_driver = { 
    .driver = { 
       .name     = "micco-vibra", //和vibra_plat_device 的name名字相同 
        .owner = THIS_MODULE, 
    }, 
    .probe        = vibra_probe, 
    .remove        = vibra_remove, 
}; 

static int __init plat_vibra_init(void) 
{ 
    int rc; 
    printk("vibra driver initializing/n"); 
   rc = platform_device_register(&vibra_plat_device); 
    if (rc < 0) 
        return rc; 
   rc = platform_driver_register(&vibra_plat_driver); 
    if (rc < 0) 
        goto err_register_vibra_dev; 
    return rc; 

err_register_vibra_dev: 
    platform_device_unregister (&vibra_plat_device); 
    return rc; 
} 
static void __exit plat_vibra_exit(void) 
{ 
    platform_driver_unregister(&vibra_plat_driver); 
    platform_device_unregister (&vibra_plat_device); 
} 

module_init(plat_vibra_init); 
module_exit(plat_vibra_exit);