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

#define DEVICE_NAME     "gprs_io"  /* 加载模式后，执行”cat /proc/devices”命令看到的设备名称 */
#define LED_MAJOR       231     /* 主设备号 */

/* 应用程序执行ioctl(fd, cmd, arg)时的第2个参数 */
#define IOCTL_LED_ON    0
#define IOCTL_LED_OFF   1

/* 用来指定LED所用的GPIO引脚 */
static unsigned long led_table [] = {
    S5PV210_GPH0(6),
	S5PV210_GPH0(7),
    
};

/* 用来指定GPIO引脚的功能：输出 */
static unsigned int led_cfg_table [] = {
	S3C_GPIO_OUTPUT,
	S3C_GPIO_OUTPUT,
};

/* 应用程序对设备文件/dev/leds执行open(...)时，
 * 就会调用s3c24xx_leds_open函数
 */
static int s3c24xx_leds_open(struct inode *inode, struct file *file)
{
    int i;
    
    for (i = 0; i < 1; i++) {
        // 设置GPIO引脚的功能：本驱动中LED所涉及的GPIO引脚设为输出功能
        s3c_gpio_cfgpin(led_table[i], S3C_GPIO_OUTPUT);
    }
    
 //   s3c_gpio_setpull(S5PV210_GPH0(7), S3C_GPIO_PULL_UP);
    
    printk("open.\n");
    return 0;
}

/* 应用程序对设备文件/dev/leds执行ioclt(...)时，
 * 就会调用s3c24xx_leds_ioctl函数
 */
static int s3c24xx_leds_ioctl(
    struct inode *inode, 
    struct file *file, 
    unsigned int cmd, 
    unsigned long arg)
{
    if (arg > 4) {
        return -EINVAL;
    }
    
    switch(cmd) {
    case IOCTL_LED_ON:
        // 设置指定引脚的输出电平为0
        printk("%d \n",arg);
    //    s3c_gpio_setpull(led_table[arg], S3C_GPIO_PULL_DOWN);
    	s3c_gpio_setpin(led_table[arg],1);
        return 0;

    case IOCTL_LED_OFF:
        // 设置指定引脚的输出电平为1
        printk("led_1 \n");
     //    s3c_gpio_setpull(led_table[arg], S3C_GPIO_PULL_UP);
         s3c_gpio_setpin(led_table[arg],0);
        return 0;

    default:
        return -EINVAL;
    }
}

/* 这个结构是字符设备驱动程序的核心
 * 当应用程序操作设备文件时所调用的open、read、write等函数，
 * 最终会调用这个结构中指定的对应函数
 */
static struct file_operations s3c24xx_leds_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   s3c24xx_leds_open,     
    .ioctl  =   s3c24xx_leds_ioctl,
};

/*
 * 执行“insmod s3c24xx_leds.ko”命令时就会调用这个函数
 */
static int __init s3c24xx_leds_init(void)
{
    int ret;

    /* 注册字符设备驱动程序
     * 参数为主设备号、设备名字、file_operations结构；
     * 这样，主设备号就和具体的file_operations结构联系起来了，
     * 操作主设备为LED_MAJOR的设备文件时，就会调用s3c24xx_leds_fops中的相关成员函数
     * LED_MAJOR可以设为0，表示由内核自动分配主设备号
     */
	struct cdev *led_cdev;
	struct class *led_class;
	led_cdev = cdev_alloc();
	cdev_init(led_cdev, &s3c24xx_leds_fops);
	led_cdev->owner = THIS_MODULE;

	dev_t devid;
	alloc_chrdev_region(&devid,66,1,"led");
	int led_major = MAJOR(devid);
	int led_minor = MINOR(devid);

	int err = cdev_add(led_cdev,devid,1);

	led_class = class_create(THIS_MODULE,"led_class");
	device_create(led_class, NULL, devid,NULL,"led");
	
/*     ret = register_chrdev(LED_MAJOR, DEVICE_NAME, &s3c24xx_leds_fops);
    if (ret < 0) {
      printk(DEVICE_NAME " can't register major number\n");
      return ret;
    }
    */
    printk(DEVICE_NAME " initialized\n");
    return 0;
}

/*
 * 执行”rmmod s3c24xx_leds.ko”命令时就会调用这个函数 
 */
static void __exit s3c24xx_leds_exit(void)
{
    /* 卸载驱动程序 */
    unregister_chrdev(LED_MAJOR, DEVICE_NAME);
}

/* 这两行指定驱动程序的初始化函数和卸载函数 */
module_init(s3c24xx_leds_init);
module_exit(s3c24xx_leds_exit);

/* 描述驱动程序的一些信息，不是必须的 */
MODULE_AUTHOR("http://www.sinlinx.com");             // 驱动程序的作者
MODULE_DESCRIPTION("S3C2410/S3C2440 LED Driver");   // 一些描述信息
MODULE_LICENSE("GPL");                              // 遵循的协议

