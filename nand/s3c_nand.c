#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/clk.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>

#include <asm/io.h>

#include <asm/arch/regs-nand.h>
#include <asm/arch/nand.h>

#define	TACLS	0
#define	TWRPH0	1
#define	TWRPH1	0

struct nand_regs
{
	unsigned long nfconf;// 0x4E000000 ? W R/W NAND flash configuration
	unsigned long nfcont;// 0x4E000004 NAND flash control
	unsigned long nfcmd;// 0x4E000008 NAND flash command
	unsigned long nfaddr;// 0x4E00000C NAND flash address
	unsigned long nfdata;// 0x4E000010 NAND flash data
	unsigned long nfmecc0;// 0x4E000014 NAND flash main area ECC0/1
	unsigned long nfmecc1;// 0x4E000018 NAND flash main area ECC2/3
	unsigned long nfsecc;// 0x4E00001C NAND flash spare area ECC
	unsigned long nfstat;// 0x4E000020 NAND flash operation status
	unsigned long nfestat0;// 0x4E000024 NAND flash ECC status for I/O[7:0]
	unsigned long nfestat1;// 0x4E000028 NAND flash ECC status for I/O[15:8]
	unsigned long nfmecc0_r;// 0x4E00002C R NAND flash main area ECC0 status
	unsigned long nfmecc1_r;// 0x4E000030 NAND flash main area ECC1 status
	unsigned long nfsecc_r;// 0x4E000034 NAND flash spare area ECC status
	unsigned long nfsblk;// 0x4E000038 R/W NAND flash start block address
	unsigned long nfeblk;// 0x4E00003C NAND flash end block address
};

static struct mtd_partition s3c_partitions[] =
{
	[0] = {
		.name		= "bootloader",
		.size		= 0x00040000,
		.offset		= 0x00000000,
		//.mask_flags	= MTD_WRITEABLE
	},
	[1] = {
		.name		= "bootloader params",
		.size		= 0x00020000,
		.offset		= MTDPART_OFS_APPEND,
		//.mask_flags	= MTD_WRITEABLE
	},
	[2] = {
		.name		= "kernel",
		.size		= 0x00200000,
		.offset		= MTDPART_OFS_APPEND,
		//.mask_flags	= MTD_WRITEABLE
	},
	[3] = {
		.name		= "root",
		.size		= MTDPART_SIZ_FULL,
		.offset		= MTDPART_OFS_APPEND
	}
};

static struct nand_regs *s3c_nand_regs;
static struct mtd_info *s3c_mtd;
static struct nand_chip *s3c_nand_chip;

static void s3c_nand_cmd_ctrl(struct mtd_info *mtd, int dat, unsigned int ctrl)
{
	if (dat == NAND_CMD_NONE)
		return;

	if (ctrl & NAND_CLE)
		//writeb(dat, host->io_base + (1 << host->board->cle));
		s3c_nand_regs->nfcmd = dat;
	else
		//writeb(dat, host->io_base + (1 << host->board->ale));
		s3c_nand_regs->nfaddr = dat;
}

void s3c_nand_select_chip(struct mtd_info *mtd, int chip)
{
	if (chip == -1) {	//unselect
		s3c_nand_regs->nfcont |= (1<<1);
	} else {
		s3c_nand_regs->nfcont &= ~(1<<1);
	}
}

static int s3c_nand_device_ready(struct mtd_info *mtd)
{
	return (s3c_nand_regs->nfstat & S3C2410_NFSTAT_BUSY);
}

static int __init s3c_nand_probe(struct platform_device *pdev)
{
	struct clk *s3c_clk;
	int res = 0;

	/* Allocate memory for the device structure (and zero it) */
	s3c_mtd = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
	if (!s3c_mtd) {
		printk(KERN_ERR "s3c_nand: failed to allocate mtd_info structure.\n");
		return -ENOMEM;
	}

	s3c_nand_chip = kzalloc(sizeof(struct nand_chip), GFP_KERNEL);

	//address remap
	s3c_nand_regs = ioremap(0x4E000000, sizeof(struct nand_regs));

#if 1
	//hardward init
	/* get the clock source and enable it */
	s3c_clk = clk_get(NULL, "nand");
	clk_enable(s3c_clk);
#endif

	s3c_nand_regs->nfconf = (TACLS<<12) | (TWRPH0<<8) | (TWRPH1<<4);

	/* NFCONT: 
	 * BIT1-设为1, 取消片选 
	 * BIT0-设为1, 使能NAND FLASH控制器
	 */
	s3c_nand_regs->nfcont = (1<<1) | (1<<0);

	/* Set address of NAND IO lines */
	s3c_nand_chip->IO_ADDR_R = &s3c_nand_regs->nfdata;
	s3c_nand_chip->IO_ADDR_W = &s3c_nand_regs->nfdata;
	s3c_nand_chip->cmd_ctrl = s3c_nand_cmd_ctrl;
	s3c_nand_chip->select_chip = s3c_nand_select_chip;
	//s3c_nand_chip->cmdfunc = s3c_nand_cmdfunc;
	s3c_nand_chip->dev_ready = s3c_nand_device_ready;
	s3c_nand_chip->ecc.mode = NAND_ECC_SOFT;	/* enable ECC */
	//s3c_nand_chip->chip_delay = 20;		/* 20us command delay time */

	s3c_mtd->priv = s3c_nand_chip;
	s3c_mtd->owner = THIS_MODULE;

	/* Scan to find existance of the device */
	nand_scan(s3c_mtd, 1);

	res = add_mtd_partitions(s3c_mtd, s3c_partitions, 4);
	//res = add_mtd_device(s3c_mtd);

	return res;
}

static int __devexit s3c_nand_remove(struct platform_device *pdev)
{
	nand_release(s3c_mtd);

	//at91_nand_disable(host);

	iounmap(s3c_nand_regs);
	kfree(s3c_mtd);
	kfree(s3c_nand_chip);

	return 0;
}
static struct platform_driver s3c_nand_driver = {
	.probe		= s3c_nand_probe,
	.remove		= s3c_nand_remove,
	.driver		= {
		.name	= "s3c2440-nand",
		.owner	= THIS_MODULE,
	},
};
static int __init s3c_nand_init(void)
{
	return platform_driver_register(&s3c_nand_driver);
	return 0;
}

static void __exit s3c_nand_exit(void)
{
	platform_driver_unregister(&s3c_nand_driver);
}

module_init(s3c_nand_init);
module_exit(s3c_nand_exit);
MODULE_LICENSE("GPL");
