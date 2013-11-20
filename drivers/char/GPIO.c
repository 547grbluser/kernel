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

#define DEVICE_NAME     "gprs_io"  /* ����ģʽ��ִ�С�cat /proc/devices����������豸���� */
#define LED_MAJOR       231     /* ���豸�� */

/* Ӧ�ó���ִ��ioctl(fd, cmd, arg)ʱ�ĵ�2������ */
#define IOCTL_LED_ON    0
#define IOCTL_LED_OFF   1

/* ����ָ��LED���õ�GPIO���� */
static unsigned long led_table [] = {
    S5PV210_GPH0(6),
	S5PV210_GPH0(7),
    
};

/* ����ָ��GPIO���ŵĹ��ܣ���� */
static unsigned int led_cfg_table [] = {
	S3C_GPIO_OUTPUT,
	S3C_GPIO_OUTPUT,
};

/* Ӧ�ó�����豸�ļ�/dev/ledsִ��open(...)ʱ��
 * �ͻ����s3c24xx_leds_open����
 */
static int s3c24xx_leds_open(struct inode *inode, struct file *file)
{
    int i;
    
    for (i = 0; i < 1; i++) {
        // ����GPIO���ŵĹ��ܣ���������LED���漰��GPIO������Ϊ�������
        s3c_gpio_cfgpin(led_table[i], S3C_GPIO_OUTPUT);
    }
    
 //   s3c_gpio_setpull(S5PV210_GPH0(7), S3C_GPIO_PULL_UP);
    
    printk("open.\n");
    return 0;
}

/* Ӧ�ó�����豸�ļ�/dev/ledsִ��ioclt(...)ʱ��
 * �ͻ����s3c24xx_leds_ioctl����
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
        // ����ָ�����ŵ������ƽΪ0
        printk("%d \n",arg);
    //    s3c_gpio_setpull(led_table[arg], S3C_GPIO_PULL_DOWN);
    	s3c_gpio_setpin(led_table[arg],1);
        return 0;

    case IOCTL_LED_OFF:
        // ����ָ�����ŵ������ƽΪ1
        printk("led_1 \n");
     //    s3c_gpio_setpull(led_table[arg], S3C_GPIO_PULL_UP);
         s3c_gpio_setpin(led_table[arg],0);
        return 0;

    default:
        return -EINVAL;
    }
}

/* ����ṹ���ַ��豸��������ĺ���
 * ��Ӧ�ó�������豸�ļ�ʱ�����õ�open��read��write�Ⱥ�����
 * ���ջ��������ṹ��ָ���Ķ�Ӧ����
 */
static struct file_operations s3c24xx_leds_fops = {
    .owner  =   THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
    .open   =   s3c24xx_leds_open,     
    .ioctl  =   s3c24xx_leds_ioctl,
};

/*
 * ִ�С�insmod s3c24xx_leds.ko������ʱ�ͻ�����������
 */
static int __init s3c24xx_leds_init(void)
{
    int ret;

    /* ע���ַ��豸��������
     * ����Ϊ���豸�š��豸���֡�file_operations�ṹ��
     * ���������豸�žͺ;����file_operations�ṹ��ϵ�����ˣ�
     * �������豸ΪLED_MAJOR���豸�ļ�ʱ���ͻ����s3c24xx_leds_fops�е���س�Ա����
     * LED_MAJOR������Ϊ0����ʾ���ں��Զ��������豸��
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
 * ִ�С�rmmod s3c24xx_leds.ko������ʱ�ͻ����������� 
 */
static void __exit s3c24xx_leds_exit(void)
{
    /* ж���������� */
    unregister_chrdev(LED_MAJOR, DEVICE_NAME);
}

/* ������ָ����������ĳ�ʼ��������ж�غ��� */
module_init(s3c24xx_leds_init);
module_exit(s3c24xx_leds_exit);

/* �������������һЩ��Ϣ�����Ǳ���� */
MODULE_AUTHOR("http://www.sinlinx.com");             // �������������
MODULE_DESCRIPTION("S3C2410/S3C2440 LED Driver");   // һЩ������Ϣ
MODULE_LICENSE("GPL");                              // ��ѭ��Э��

