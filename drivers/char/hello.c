/*
 * =====================================================================================
 *
 *       Filename:  hello.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12/02/2011 11:19:22 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Gavin.S (), zdshuwei@gmail.com
 *        Company:  Zhejiang University
 *
 * =====================================================================================
 */

#include <linux/init.h>
#include <linux/module.h>

static int __init hello_init(void)
{
	printk("RealArm:Hello World enter.\n");
	return 0;
}

static void __exit hello_exit(void)
{
	printk("RealArm:Hello World exit.\n");
}

module_init(hello_init);
module_exit(hello_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A simple module");
