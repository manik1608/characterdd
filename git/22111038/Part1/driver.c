#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include<linux/sysfs.h> 
#include<linux/kobject.h> 
#include <linux/err.h>

#define DEVNAME "cs614_device"

//values to read
#define PID 		0
#define	STATIC_PRIO 	1
#define	COMM 		2
#define PPID		3
#define NVCSW		4

// major number -> specific no. assigned to a dd module
// minor number is for the instances of the devices
static int major;
atomic_t device_opened;
static struct class *demo_class;
struct device *demo_device;
static int command;

static char *d_buf = NULL;

/*
** Function Prototypes
*/
static int      __init cs614_driver_init(void);
static void     __exit cs614_driver_exit(void);

static int my_open(struct inode *inode, struct file *file){
    atomic_inc(&device_opened);
    try_module_get(THIS_MODULE);
    printk(KERN_INFO "Device opened successfully\n");
    return 0;
}

static int my_release(struct inode *inode, struct file *file){
    atomic_dec(&device_opened);
    module_put(THIS_MODULE);
    printk(KERN_INFO "Device closed successfully\n");
    return 0;
}

static ssize_t my_read(struct file *filp, char *ubuf, size_t length, loff_t * offset){ 
//	if(length < 16)
//		return -EINVAL;
    printk(KERN_INFO "In read\n");
    if(command == 0){
		int pid = current->pid;
		sprintf(d_buf, "%d", pid);
	}
	
	else if(command == 1){
		int static_priority = current->static_prio;
		sprintf(d_buf, "%d", static_priority);
	}
	
	else if(command == 2){
		sprintf(d_buf, "%s", current->comm);
	}
	
	else if(command == 3){
		int ppid = current->parent->pid;
		sprintf(d_buf, "%d", ppid);
	}
	
	else if(command == 4){
		int req = current->nvcsw;
		sprintf(d_buf, "%d", req);
	}
	if(copy_to_user(ubuf, d_buf, 16))
		return -EINVAL;
	return sizeof(d_buf);
}

static ssize_t my_write(struct file *filp, const char *buff, size_t len, loff_t * off){
    if(copy_from_user(d_buf, buff, len))
	    return -EINVAL;
    printk(KERN_INFO "In write\n");
    return 8;
}

static char *demo_devnode(struct device *dev, umode_t *mode)
{
        if (mode && dev->devt == MKDEV(major, 0))
                *mode = 0666;
        return NULL;
}

static struct file_operations fops = {
    .read = my_read,
    .write = my_write,
    .open = my_open,
    .release = my_release,
};


/*static ssize_t cs614_sysfs_read(struct kobject *kobj,
                                  struct kobj_attribute *attr, char *buf)
{
		//int enabled = 0;
        return;
}*/

static ssize_t cs614_sysfs_write(struct kobject *kobj,
                                   struct kobj_attribute *attr,
                                   const char *buf, size_t count)
{
    int newval;
    int err = kstrtoint(buf, 10, &newval);
    if (err || newval < 0 || newval > 4 )
            return -EINVAL;
	
	command = newval;
   
    return strlen(buf);
}

static struct kobj_attribute cs614_sysfs_attribute = __ATTR(cs614_value, 0660, NULL, cs614_sysfs_write);
static struct attribute *cs614_sysfs_attrs[] = {
        &cs614_sysfs_attribute.attr,
        NULL,
};
static struct attribute_group cs614_sysfs_group = {
        .attrs = cs614_sysfs_attrs,
        .name = "cs614_sysfs",
};

/*
** Module Init function
*/
static int __init cs614_driver_init(void)
{
	//creating device
	int err, ret;
	printk(KERN_INFO "Assignment part 1 started\n");
            
    major = register_chrdev(0, DEVNAME, &fops);
    err = major;
    if (err < 0) {      
         printk(KERN_ALERT "Registering char device failed with %d\n", major);   
         goto error_regdev;
    }                 
        
    demo_class = class_create(THIS_MODULE, DEVNAME);
    err = PTR_ERR(demo_class);
    if (IS_ERR(demo_class))
            goto error_class;

    demo_class->devnode = demo_devnode;

    demo_device = device_create(demo_class, NULL, MKDEV(major, 0), NULL, DEVNAME);
    err = PTR_ERR(demo_device);
    if (IS_ERR(demo_device))
            goto error_device;

    d_buf = kzalloc(16, GFP_KERNEL);
    printk(KERN_INFO "I was assigned major number %d. To talk to\n", major);                                                              
    atomic_set(&device_opened, 0);
    
    //creating sysfs directory
    ret = sysfs_create_group (kernel_kobj, &cs614_sysfs_group);
    if(unlikely(ret))
        printk(KERN_INFO "demo: can't create sysfs\n");
        
    pr_info("Device Driver Insert...Done!!!\n");
	return 0;
    

error_device:
     class_destroy(demo_class);
error_class:
        unregister_chrdev(major, DEVNAME);
error_regdev:
        return  err;	
}

/*
** Module exit function
*/
static void __exit cs614_driver_exit(void)
{
	//destrying buffer memory and destroying created device
	kfree(d_buf);
    device_destroy(demo_class, MKDEV(major, 0));
    class_destroy(demo_class);
    unregister_chrdev(major, DEVNAME);
    
    //removing sysfs
    sysfs_remove_group (kernel_kobj, &cs614_sysfs_group);
    
    pr_info("Device Driver Remove...Done!!!\n");
}
 
module_init(cs614_driver_init);
module_exit(cs614_driver_exit);
 
MODULE_LICENSE("GPL");
