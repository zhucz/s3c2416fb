/* linux/drivers/video/s3c2416fb.c
 *	Copyright (c) 2016
 *
 * S3C2416 LCD Framebuffer Driver
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 * Driver based on skeletonfb.c, sa1100fb.c and others.
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>
#include <mach/regs-lcd.h>
#include "s3c2416fb.h"

#define CONFIG_FB_S3C2416_DEBUG	

/* Debugging stufff*/
#ifdef CONFIG_FB_S3C2416_DEBUG
	#define dprintk(fmt, ...) \
		do { \
			printk(KERN_NOTICE fmt, ##__VA_ARGS__);	\
		} while (0)
#else
	#define dprintk(fmt, ...)	
#endif

static int s3c2416fb_probe(struct platform_device *pdev)
{
	struct s3c2416fb_info *info = NULL;
	struct resource * res = NULL;
	int ret = 0;
	int size = 0;		
	uint32_t lcdcon1 = 0;

	dprintk("------------------s3c2416-lcd%d init---------------\n",  ret);
	
	/* get platform info */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(res == NULL){
		dprintk("lcd drivers can't get platform resource\n");
		ret = -ENXIO;
	}else{
		dprintk("lcd drivers name  = %s \n", res->name);
	}

	/* request memory region */
	size = res->end - res->start + 1;
	info->mem = request_mem_region(res->start, size, pdev->name);
	if(info->mem == NULL){
		dprintk("can't alloc mem region\n");
		ret = -ENOMEM;
	}
#if 0
	info->io = ioremap(res->start, size);	
	if(info->io == NULL){
		dprintk("ioremap() of registers failded\n");
		ret = -ENXIO;
		goto release_mem;
	}
#endif
	dprintk("lcd dev register init \n");
//	lcdcon1 = readl(info->io + S3C2410_LCDCON1);
	printk("lcdcon1 = %x \n", lcdcon1);

	return ret;

release_mem:
	release_mem_region(res->start, size);
	return ret;
}

static int s3c2416fb_remove(struct platform_device *pdev)
{

	return 0;
}

#if defined(CONFIG_PM)
#define s3c2416fb_suspend 	NULL
#define s3c2416fb_resume	NULL
#else
#define s3c2416fb_suspend 	NULL
#define s3c2416fb_resume	NULL
#endif

static struct platform_driver s3c2416fb_driver = {
	.probe		= s3c2416fb_probe,
	.remove 	= s3c2416fb_remove,
	.suspend    = s3c2416fb_suspend,
	.resume		= s3c2416fb_resume,
	.driver		= {
		.name 	= "s3c2416-lcd",
		.owner	= THIS_MODULE,
	},
};


int __init s3c2416fb_init(void)
{
	int ret = 0 ;

	ret =	platform_driver_register(&s3c2416fb_driver);

	return ret;
}

static void __exit s3c2416fb_exit(void)
{
	platform_driver_unregister(&s3c2416fb_driver);
}

module_init(s3c2416fb_init);
module_exit(s3c2416fb_exit);

MODULE_AUTHOR("mogran <lumingliang.github@gmail.com>");
MODULE_DESCRIPTION("Framebuffer driver for the s3c2416");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:s3c2416-lcd");
