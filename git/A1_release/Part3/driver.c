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
#include <linux/vmalloc.h>
#include <linux/fdtable.h>
 

//values to read
#define PID 		0
#define	STATIC_PRIO 	1
#define	COMM 		2
#define PPID            3
#define NVCSW           4
#define NUM_THREADS	5
#define NUM_FILES_OPEN	6
#define STACK_SIZE	7



/*
** Function Prototypes
*/
static int      __init cs614_driver_init(void);
static void     __exit cs614_driver_exit(void);
 
/*
** Module Init function
*/
static int __init cs614_driver_init(void)
{
        pr_info("Device Driver Insert...Done!!!\n");
        return 0;
}

/*
** Module exit function
*/
static void __exit cs614_driver_exit(void)
{
        pr_info("Device Driver Remove...Done!!!\n");
}
 
module_init(cs614_driver_init);
module_exit(cs614_driver_exit);
 
MODULE_LICENSE("GPL");
