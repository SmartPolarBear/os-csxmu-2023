/* Hello from Kernel! */
#include <linux/module.h>

MODULE_LICENSE("GPL");

/*id*/
static char* id = "22920202204622";
module_param(id, charp, 0644);
MODULE_PARM_DESC(id, "char* param\n");

/*name*/
static char* name = "KeZheng Xiong";
module_param(name, charp, 0644);
MODULE_PARM_DESC(name, "char* param\n");

/*age*/
static int age = 21;
module_param(age, int, 0644);
MODULE_PARM_DESC(age, "int param\n");


/*--------------begin-请在此处定义函数 hello student(...)----------*/



void hello_student(int id,char *name,int age)
{
    printk("My name is %s, student id is %d. I am %d years old.",name,id,age);
}


/*----------------------end-----------------*/



/*--------------begin--请在此处定义函数 my magic number(.)---------*/


void my_magic_number(int id,int age)
{
    int ret=0;
    while(id){
        ret+=id%10;
        id/=10;
    }
    ret+=age;
    printk("My magic number is %d.",ret);
} 


/*----------------------end-----------------*/


int __init hello_init(void)
{
	printk(KERN_ALERT "Init module.\n");
	hello_student(id, name,age);
	my_magic_number(id,age);
	return 0;
}


void __exit hello_exit(void)
{
	printk(KERN_ALERT "Exit module.\n");
}
module_init(hello_init);
module_exit(hello_exit);
