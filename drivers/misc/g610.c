#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/notifier.h>
#include <linux/ctype.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <mach/regs-gpio.h>
#include <mach/gpio-bank.h>
#include <plat/gpio-cfg.h>
struct G610_device 
{
	int power;
	struct device dev;
};

#define to_G610_device(obj) container_of(obj, struct G610_device, dev)

static ssize_t G610_show_power(struct device *dev,
		struct device_attribute *attr,char *buf)
{
	struct G610_device *bd = to_G610_device(dev);
	return sprintf(buf, "%d\n", bd->power);
}

static ssize_t G610_store_power(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int rc;
	struct G610_device *bd = to_G610_device(dev);
	unsigned long power;
		
	rc = strict_strtoul(buf, 0, &power);
	if (rc)
		return rc;

	rc = -ENXIO;

//	printk("G610: set power to %lu\n", power);
	if (bd->power != power) 
	{
		bd->power = power;
		if(power)
		{	
			s3c_gpio_setpin(S5PV210_GPJ2(6), 1);	
		}
		else
		{
			s3c_gpio_setpin(S5PV210_GPJ2(6), 0);			
		}	
	}
	rc = count;

	return rc;
}


static struct class *G610_class;

static int G610_suspend(struct device *dev, pm_message_t state)
{
	return 0;
}

static int G610_resume(struct device *dev)
{
	return 0;
}

static void bl_device_release(struct device *dev)
{
	struct G610_device *bd = to_G610_device(dev);
	kfree(bd);
}


static struct device_attribute bl_device_attributes[] = {
	__ATTR(G610_power, 0666, G610_show_power, G610_store_power),
	__ATTR_NULL,
};


void G610_device_register(void)
{
	struct G610_device *new_bd;
	int rc;

//	printk("G610_device_register: name=%s\n", "G610_power");

	new_bd = kzalloc(sizeof(struct G610_device), GFP_KERNEL);
	if (!new_bd)
		return ;

	new_bd->power = 0;
	new_bd->dev.class = G610_class;
	new_bd->dev.parent = NULL;
	new_bd->dev.release = bl_device_release;
	dev_set_name(&new_bd->dev, "G610_power");
	dev_set_drvdata(&new_bd->dev, NULL);

	rc = device_register(&new_bd->dev);
	if (rc) {
		kfree(new_bd);
		return ;
	}
	

	rc = gpio_request(S5PV210_GPJ2(6), "GPJ2_6");
	if (rc)
		printk(KERN_ERR "#### failed to request GPJ2_6 \n");
	s3c_gpio_cfgpin(S5PV210_GPJ2(6), S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(S5PV210_GPJ2(6), S3C_GPIO_PULL_UP);
	s3c_gpio_setpin(S5PV210_GPJ2(6), 0);
}


void G610_device_unregister(struct G610_device *bd)
{
	if (!bd)
		return;
		
	device_unregister(&bd->dev);
	gpio_free(S5PV210_GPH2(7));	
}


static void __exit G610_class_exit(void)
{
	class_destroy(G610_class);
}

static int __init G610_class_init(void)
{
	G610_class = class_create(THIS_MODULE, "G610_power");
	if (IS_ERR(G610_class)) {
		printk(KERN_WARNING "Unable to create G610 class; errno = %ld\n",
				PTR_ERR(G610_class));
		return PTR_ERR(G610_class);
	}

	G610_class->dev_attrs = bl_device_attributes;
	G610_class->suspend = G610_suspend;
	G610_class->resume = G610_resume;
	
	G610_device_register();
	return 0;
}

postcore_initcall(G610_class_init);
module_exit(G610_class_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lockey");
MODULE_DESCRIPTION("G610 Lowlevel Control Abstraction");
