#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");

static void* vmallocmem1;
static void* vmallocmem2;
static void* vmallocmem3;
static void* vmallocmem4;

static int __init vmalloc_module_init(void)
{
    printk(KERN_INFO "Start vmalloc!\n");
    vmallocmem1 = vmalloc(8192);
    if (!vmallocmem1)
        printk(KERN_INFO "Failed to allocate vmallocmem1!\n");
    else
        printk(KERN_INFO "vmallocmem1 addr = %p\n", vmallocmem1);
    
    vmallocmem2 = vmalloc(1048576);
    if (!vmallocmem2)
        printk(KERN_INFO "Failed to allocate vmallocmem2!\n");
    else
        printk(KERN_INFO "vmallocmem2 addr = %p\n", vmallocmem2);
    
    vmallocmem3 = vmalloc(67108864);
    if (!vmallocmem3)
        printk(KERN_INFO "Failed to allocate vmallocmem3!\n");
    else
        printk(KERN_INFO "vmallocmem3 addr = %p\n", vmallocmem3);
    
    vmallocmem4 = vmalloc(10485761);
    if (!vmallocmem4)
        printk(KERN_INFO "Failed to allocate vmallocmem4!\n");
    else
        printk(KERN_INFO "vmallocmem4 addr = %p\n", vmallocmem4);
        
    return 0;
}

static void __exit vmalloc_module_exit(void)
{
    printk(KERN_INFO "Exit vmalloc!\n");
    vfree(vmallocmem1);
    vfree(vmallocmem2);
    vfree(vmallocmem3);
    vfree(vmallocmem4);
}

module_init(vmalloc_module_init);
module_exit(vmalloc_module_exit);