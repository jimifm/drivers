#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/kernel.h>   /* printk() */
#include <linux/slab.h>     /* kmalloc() */
#include <linux/fs.h>       /* everything... */
#include <linux/errno.h>    /* error codes */
#include <linux/types.h>    /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>    /* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>
#include <linux/ioport.h>

#include <asm/system.h>     /* cli(), *_flags */
#include <asm/uaccess.h>    /* copy_*_user */
#include <asm/io.h>

#define SWITCOLOR_MAJOR 255
static int switcolor_major = SWITCOLOR_MAJOR;


//#define DEBE0_PHYS_START 0x01e60000
#define DEBE_PHYS_OFFSET 0x09d0//

#define DEBE0_PHYS_START (0x01e60000 + DEBE_PHYS_OFFSET)
#define DEBE0_PHYS_END 0x01e609fc//
#define DEBE0_PHYS_LEN (DEBE0_PHYS_END - DEBE0_PHYS_START + 1)
//#define DEBE1_PHYS_START 0x01e40000
#define DEBE1_PHYS_START (0x01e40000 + DEBE_PHYS_OFFSET)
#define DEBE1_PHYS_END 0x01e409fc
#define DEBE1_PHYS_LEN (DEBE1_PHYS_END - DEBE1_PHYS_START + 1)

//define offset
#define debe_get_relative_offset(offset) \
    (offset - DEBE_PHYS_OFFSET)

#define DEBE_OCRCOEF_REG_OFFSET debe_get_relative_offset(0x9d0)//4c*3
#define DEBE_OCRCONS_REG_OFFSET debe_get_relative_offset(0x9dc)//4c*1
#define DEBE_OCGCOEF_REG_OFFSET debe_get_relative_offset(0x9e0)//4c*3
#define DEBE_OCGCONS_REG_OFFSET debe_get_relative_offset(0x9ec)//4c*1
#define DEBE_OCBCOEF_REG_OFFSET debe_get_relative_offset(0x9f0)//4c*3
#define DEBE_OCBCONS_REG_OFFSET debe_get_relative_offset(0x9fc)//4c*1

struct debe_channel{
    void __iomem *base;
};

struct switcolor_dev
{
    struct cdev dev;
    //unsigned long start;
    struct debe_channel debe_channel[2];
    //unsigned long len;
};

struct switcolor_dev switcolor;
//dev_t dev = 0;
struct class *switcolor_class;

int switcolor_open(struct inode *inode, struct file *filp)
{
    struct switcolor_dev *switcolor; /* device information */

    switcolor = container_of(inode->i_cdev, struct switcolor_dev, dev);
    filp->private_data = switcolor; /* for other methods */

    return 0;          /* success */
}

int switcolor_release(struct inode *inode, struct file *filp)
{
    return 0;
}

ssize_t switcolor_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    return 0;
}

ssize_t switcolor_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    char data;
    struct switcolor_dev *switcolor;
    //u32 value;
    printk(KERN_INFO "debug by zhanghong: switcolor dev write\n");
                        
    switcolor = (struct switcolor_dev *)filp->private_data;
    
    if(copy_from_user(&data,buf,count))
    {
        printk(KERN_ERR "can't copy data from usr\n");
        return -EFAULT;
    }
   
    printk(KERN_INFO "debug by zhanghong: switcolor write data %c\n",data);     
    //value = ioread32(switcolor->base); 
    //analyzing.....
/*
#define DEBE_OCRCOEF_REG_OFFSET debe_get_relative_offset(0x9d0)//4c*3
#define DEBE_OCRCONS_REG_OFFSET debe_get_relative_offset(0x9dc)//4c*1
#define DEBE_OCGCOEF_REG_OFFSET debe_get_relative_offset(0x9e0)//4c*3
#define DEBE_OCGCONS_REG_OFFSET debe_get_relative_offset(0x9ec)//4c*1
#define DEBE_OCBCOEF_REG_OFFSET debe_get_relative_offset(0x9f0)//4c*3
#define DEBE_OCBCONS_REG_OFFSET debe_get_relative_offset(0x9fc)//4c*1
*/  if(data == '0'){  
        iowrite32( 0x88 , switcolor->debe_channel[0].base + DEBE_OCRCOEF_REG_OFFSET/*+offset*/);    
        iowrite32( 0x88 , switcolor->debe_channel[0].base + DEBE_OCRCOEF_REG_OFFSET + 4/*+offset*/);    
        iowrite32( 0x88 , switcolor->debe_channel[0].base + DEBE_OCRCOEF_REG_OFFSET + 8/*+offset*/);    

        iowrite32( 0x88 , switcolor->debe_channel[0].base + DEBE_OCRCONS_REG_OFFSET/*+offset*/); 
    }
    else
    {
        iowrite32( 0x88 , switcolor->debe_channel[1].base + DEBE_OCRCOEF_REG_OFFSET/*+offset*/);    
        iowrite32( 0x88 , switcolor->debe_channel[1].base + DEBE_OCRCOEF_REG_OFFSET + 4/*+offset*/);    
        iowrite32( 0x88 , switcolor->debe_channel[1].base + DEBE_OCRCOEF_REG_OFFSET + 8/*+offset*/);    

        iowrite32( 0x88 , switcolor->debe_channel[1].base + DEBE_OCRCONS_REG_OFFSET/*+offset*/); 
    }
    //iowrite32( 0x88 , switcolor->debe_channel[0].base + DEBE_OCRCOEF_REG_OFFSET/*+offset*/);                                                                          
    //iowrite32( 0x88 , switcolor->debe_channel[0].base + DEBE_OCRCOEF_REG_OFFSET/*+offset*/);                                                                          
    //iowrite32( 0x88 , switcolor->debe_channel[0].base + DEBE_OCRCOEF_REG_OFFSET/*+offset*/);                                                                          
    //iowrite32( 0x88 , switcolor->debe_channel[0].base + DEBE_OCRCOEF_REG_OFFSET/*+offset*/); 
    return count;
}

struct file_operations switcolor_fops = {    
    .owner =    THIS_MODULE,
    .read =     switcolor_read,
    .write =    switcolor_write,
    //.ioctl =    switcolor_ioctl,
    .open =     switcolor_open,    
    .release =  switcolor_release,
};

static ssize_t switcolor_show(struct device *dev,
    struct device_attribute *attr, char *buf) 
{     
    return sprintf(buf,"test pass\n");
} 


static ssize_t switcolor_store(struct device *dev,                   
    struct device_attribute *attr,
    const char *buf, size_t count) 
{ 
    printk(KERN_INFO "store :%s\n",buf);
    //mainkey
    //R(R G B)
    //if(strstr)
    iowrite32( 0x88 , switcolor.debe_channel[0].base + DEBE_OCRCOEF_REG_OFFSET/*+offset*/);    
    iowrite32( 0x88 , switcolor.debe_channel[0].base + DEBE_OCRCOEF_REG_OFFSET + 4/*+offset*/);    
    iowrite32( 0x88 , switcolor.debe_channel[0].base + DEBE_OCRCOEF_REG_OFFSET + 8/*+offset*/);    

    iowrite32( 0x88 , switcolor.debe_channel[0].base + DEBE_OCRCONS_REG_OFFSET/*+offset*/); 
    //G(R G B)
    //
    //B(R G B)
    //
    return count;
} 

static DEVICE_ATTR(switcolor,0664, switcolor_show, switcolor_store);     
    
struct device *switcolor_dev;
static int __init switcolor_init(void)
{   
    int result;

    dev_t devno = MKDEV(switcolor_major, 0);
    if(switcolor_major)
    {
        result = register_chrdev_region(devno, 1,"SWITCOLOR");
    }
    else
    {
        result = alloc_chrdev_region(&devno, 0, 1,"SWITCOLOR");
        switcolor_major = MAJOR(devno);
    }
    
    if (result < 0) {
        printk(KERN_ERR "SWITCOLOR: can't get major %d\n", MAJOR(devno));
        return result;
    }
    
    switcolor_class = class_create(THIS_MODULE,"switcolor_class");  
    if(IS_ERR(switcolor_class)){  
        printk("%s create class error\n",__func__);  
        return -1;                              
    }  
    
    switcolor_dev = device_create(switcolor_class, NULL, devno, NULL, "switcolor");
    if(IS_ERR(switcolor_dev))
    {
        printk(KERN_ERR "can't create device\n");
        return -1;
    }
    request_mem_region(DEBE0_PHYS_START,DEBE0_PHYS_LEN,"switcolor");
    switcolor.debe_channel[0].base = ioremap(DEBE0_PHYS_START,DEBE0_PHYS_LEN);
    
    request_mem_region(DEBE1_PHYS_START,DEBE1_PHYS_LEN,"switcolor");
    switcolor.debe_channel[1].base = ioremap(DEBE1_PHYS_START,DEBE1_PHYS_LEN);
    
    cdev_init( &switcolor.dev, &switcolor_fops);
    
    switcolor.dev.owner = THIS_MODULE;
    
    //switcolor.dev.ops = &switcolor_fops;
    result = cdev_add(&switcolor.dev,devno,1);
    if(result < 0)    
    {
        printk(KERN_ERR "SWITCOLOR: can't add switcolor\n");
        return result;
    }

    if (device_create_file(switcolor_dev, &dev_attr_switcolor) < 0)
    {
        printk(KERN_ERR "SWITCOLOR: can't create sysfs");
        return -1;
    }

    printk(KERN_INFO "init SWITCOLOR OK, devno is %d major %d!\n",devno,MAJOR(devno));
   
    return 0;
}

static void __exit switcolor_exit(void)
{
    release_mem_region(DEBE0_PHYS_START,DEBE0_PHYS_LEN);
    iounmap(switcolor.debe_channel[0].base);
    
    release_mem_region(DEBE1_PHYS_START,DEBE1_PHYS_LEN);
    iounmap(switcolor.debe_channel[1].base);
    
    cdev_del(&switcolor.dev);    
    device_destroy(switcolor_class,MKDEV(switcolor_major,0));
    class_destroy(switcolor_class);  
    unregister_chrdev_region(MKDEV(switcolor_major,0), 1);   
}
module_init(switcolor_init);
module_exit(switcolor_exit);
MODULE_AUTHOR("ZhangHong");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple SWITCOLOR Driver");
