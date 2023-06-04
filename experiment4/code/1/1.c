#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");

static void* kmallocmem1;
static void* kmallocmem2;
static void* kmallocmem3;
static void* kmallocmem4;

static int __init kmalloc_module_init(void)
{
    printk(KERN_INFO "kmalloc module loaded\n");
    kmallocmem1 = kmalloc(1024, GFP_KERNEL);
    if (!kmallocmem1)
        printk(KERN_INFO "Failed to allocate kmallocmem1!\n");
    else
        printk(KERN_INFO "kmallocmem1: %p\n", kmallocmem1);
    
    kmallocmem2 = kmalloc(8192, GFP_KERNEL);
    if (!kmallocmem2)
        printk(KERN_INFO "Failed to allocate kmallocmem2!\n");
    else
        printk(KERN_INFO "kmallocmem2: %p\n", kmallocmem2);
        
    kmallocmem3 = kmalloc(10485760, GFP_KERNEL);
    if (!kmallocmem3)
        printk(KERN_INFO "Failed to allocate kmallocmem3!\n");
    else
        printk(KERN_INFO "kmallocmem3: %p\n", kmallocmem3);
    
    kmallocmem4 = kmalloc(10500000, GFP_KERNEL);
    if (!kmallocmem4)
        printk(KERN_INFO "Failed to allocate kmallocmem4!\n");
    else
        printk(KERN_INFO "kmallocmem4: %p\n", kmallocmem4);
        
    return 0;
}

static void __exit kmalloc_module_exit(void)
{
    printk(KERN_INFO "kmalloc module unloaded\n");
    kfree(kmallocmem1);
    kfree(kmallocmem2);
    kfree(kmallocmem3);
    kfree(kmallocmem4);
}

module_init(kmalloc_module_init);
module_exit(kmalloc_module_exit);