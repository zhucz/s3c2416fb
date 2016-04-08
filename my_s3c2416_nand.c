/*
 * (C) Copyright 2006 OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#include <nand.h>
#include <asm/arch/s3c24xx_cpu.h>
#include <asm/io.h>

#define S3C2416_NFCONF_EN          (1<<15)
#define S3C2416_NFCONF_512BYTE     (1<<14)
#define S3C2416_NFCONF_4STEP       (1<<13)
#define S3C2416_NFCONF_INITECC     (1<<12)


#define S3C2416_NFCONT_nFCE        (1<<2)
#define S3C2416_NFCONF_TACLS(x)    ((x)<<12)
#define S3C2416_NFCONF_TWRPH0(x)   ((x)<<8)
#define S3C2416_NFCONF_TWRPH1(x)   ((x)<<4)

#define S3C2416_ADDR_NALE 4
#define S3C2416_ADDR_NCLE 8



static void s3c24x0_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct s3c24x0_nand *nand = s3c24x0_get_base_nand();

	debug("hwcontrol(): 0x%02x 0x%02x\n", cmd, ctrl);

	if(ctrl & NAND_CTRL_CHANGE){
		if (ctrl & NAND_NCE)
			writel(readl(&nand->nfcont) & ~S3C2416_NFCONT_nFCE,&nand->nfcont);//使能 NAND FLASH
		else
			writel(readl(&nand->nfcont) |  S3C2416_NFCONT_nFCE,&nand->nfcont);//关闭 NAND FLASH
	}
	
	if(cmd != NAND_CMD_NONE){
		if(ctrl & NAND_CLE){
			writeb(cmd,&nand->nfcmmd);
		}else if(ctrl & NAND_ALE){
			writeb(cmd,&nand->nfaddr);
		}
	}

#if 0
	if (ctrl & NAND_CTRL_CHANGE) {
		ulong IO_ADDR_W = (ulong)nand;

		if (!(ctrl & NAND_CLE))
			IO_ADDR_W |= S3C2416_ADDR_NCLE;//要写的是命令
		if (!(ctrl & NAND_ALE))
			IO_ADDR_W |= S3C2416_ADDR_NALE;//要写的是地址

		chip->IO_ADDR_W = (void *)IO_ADDR_W;

		if (ctrl & NAND_NCE)
			writel(readl(&nand->nfcont) & ~S3C2416_NFCONT_nFCE,&nand->nfcont);//使能 NAND FLASH
		else
			writel(readl(&nand->nfcont) |  S3C2416_NFCONT_nFCE,&nand->nfcont);//关闭 NAND FLASH
	}

	if (cmd != NAND_CMD_NONE)
		writeb(cmd, chip->IO_ADDR_W);
#endif

}

static int s3c24x0_dev_ready(struct mtd_info *mtd)
{
	struct s3c24x0_nand *nand = s3c24x0_get_base_nand();
	debug("dev_ready\n");
	return readl(&nand->nfstat) & 0x01;
}


int board_nand_init(struct nand_chip *nand)
{
	u_int32_t cfg;
	u_int8_t tacls, twrph0, twrph1;
	struct s3c24x0_nand *nand_reg = s3c24x0_get_base_nand();

	uint8_t id[6] = {'\0'};

	debug("board_nand_init()\n");



	/* initialize hardware */

	writel(~(1<< 1), &nand_reg->nfcont);//enable nand flash
	cfg = readl(&nand_reg->nfconf);
	cfg &= ~(1 << 0);//8 bit bus
	cfg |= (1 << 1); //5 address cycle
	cfg &= (3 << 2); //large size 2048
	tacls = 2;
	twrph0 = 2;
	twrph1 = 1;
	cfg |= (tacls  << 12);
	cfg |= (twrph0 << 8);
	cfg |= (twrph1 << 4);
	writel(cfg, &nand_reg->nfconf);

	writeb(0x90,&nand_reg->nfcmmd);
	writeb(0x00,&nand_reg->nfaddr);

	while(!(readl(&nand_reg->nfstat) & 0x01));


	id[0] = readb(&nand_reg->nfdata);
	id[1] = readb(&nand_reg->nfdata);
	id[2] = readb(&nand_reg->nfdata);
	id[3] = readb(&nand_reg->nfdata);
	id[4] = readb(&nand_reg->nfdata);

	debug("%x,%x,%x,%x,%x\n",id[0],id[1],id[2],id[3],id[4]);


	/* initialize nand_chip data structure */
	nand->IO_ADDR_R = (void *)&nand_reg->nfdata;
	nand->IO_ADDR_W = (void *)&nand_reg->nfdata;

	nand->select_chip = NULL;

	/* read_buf and write_buf are default */
	/* read_byte and write_byte are default */

	/* hwcontrol always must be implemented */
	nand->cmd_ctrl = s3c24x0_hwcontrol;
	nand->dev_ready = s3c24x0_dev_ready;
	nand->ecc.mode = NAND_ECC_NONE;


	debug("end of nand_init\n");

	return 0;
}
