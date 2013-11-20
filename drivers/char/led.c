/*
 * =====================================================================================
 *
 *       Filename:  led.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/21/2013 
 *       Revision:  none
 *       Compiler:  arm-linux-gcc 
 *
 *         Author:  Alexlee sinlinx@gmail.com
 *        Company:  Baoding Sinlinx Electronics Technology Co., LTD
 *
 * =====================================================================================
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/gpio.h>

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/usb/ch9.h>
#include <linux/pwm_backlight.h>
#include <linux/spi/spi.h>
#include <linux/gpio_keys.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/setup.h>
#include <asm/mach-types.h>

#include <mach/map.h>
#include <mach/regs-mem.h>
#include <mach/regs-gpio.h>
#include <mach/gpio-bank.h>
#include <plat/s5pv210.h>
#include <plat/gpio-cfg.h>
#include <mach/gpio-bank-b.h>

#define LED_ON	0
#define LED_OFF 1

static unsigned long led_table [] = {
	S5PV210_GPJ4(2),	
	S5PV210_GPJ4(1),
	S5PV210_GPJ4(0),
	S5PV210_GPJ3(7),
};

static unsigned long led_cfg_table [] = {
	S3C_GPIO_OUTPUT,
	S3C_GPIO_OUTPUT,
	S3C_GPIO_OUTPUT,
	S3C_GPIO_OUTPUT,
};

static int led_open(struct inode *inode, struct file *file)
{
	int i;
	for(i = 0; i < 4; i++)
	{
		s3c_gpio_cfgpin(led_table[i], led_cfg_table[i]);
	}

	return 0;
}

static int led_ioctl(struct inode *indoe, struct file *file,
		unsigned int cmd, unsigned long arg)
{
	if (arg > 4)
		return -EINVAL;

	switch(cmd)
	{
		case LED_ON:
			s3c_gpio_setpin(led_table[arg], 0);
			printk("led[%d] on\r\n", arg);
			break;
		case LED_OFF:
			s3c_gpio_setpin(led_table[arg], 1);
			printk("led[%d] off\r\n", arg);
			break;
		default:
			break;
	}

	return 0;
}

static struct file_operations leds_fops = {
	.owner 	= THIS_MODULE,
	.open	= led_open,
	.ioctl	= led_ioctl,
};

static int __init leds_init(void)
{

	struct cdev *led_cdev;
	struct class *led_class;

	led_cdev = cdev_alloc();
	cdev_init(led_cdev, &leds_fops);
	led_cdev->owner = THIS_MODULE;
	
	dev_t devid;
	alloc_chrdev_region(&devid, 0, 1, "led");
	cdev_add(led_cdev,devid,1);
	
	led_class = class_create(THIS_MODULE,"led_class");
	device_create(led_class, NULL, devid, NULL,"led");

	printk("LEDS initialized.\n");
}


module_init(leds_init);

MODULE_AUTHOR("alexlee");
MODULE_DESCRIPTION("LEDS DRIVER");
MODULE_LICENSE("GPL");
