#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <s3c24xx.h>
#include <timer.h>
#include "sdi.h"

/********************************************************
 宏定义
********************************************************/
#define __SD_MMC_DEBUG__ 0

#if __SD_MMC_DEBUG__
#define SD_DEBUG(fmt,args...) printf(fmt,##args)
#else
#define SD_DEBUG(fmt,args...)
#endif

volatile u16_t SD_RCA;
volatile u32_t LBA_OFFSET;
volatile u32_t TOTAL_SECTOR;
volatile u32_t TOTAL_SIZE; //(MB)

#define SDCard_BlockSize	9
#define SDCARD_BUFF_SIZE	512
/********************************************************
 变量定义
********************************************************/
SD_STRUCT SDCard;
u8_t cTxBuffer[SDCARD_BUFF_SIZE * 2];
u8_t cRxBuffer[SDCARD_BUFF_SIZE * 2];


void TEST_SD() {
	printf("test begin\n");
	if (Read_Block(1, 2, cRxBuffer)) {
		for (int i = 0; i < SDCARD_BUFF_SIZE * 2; i++) {
			printf("%02X ", cRxBuffer[i] & 0xff);
			if (i % 16 == 15)
				printf("\n");
			if (i == SDCARD_BUFF_SIZE - 1)
				printf("next:\n");
		}
		for (int i = 0; i < SDCARD_BUFF_SIZE * 2; i++)
			cRxBuffer[i] += 1;
		printf("***write:\n");
		if (Write_Block(1, 2, cRxBuffer)) {
			if (Read_Block(1, 2, cRxBuffer)) {
				for (int i = 0; i < SDCARD_BUFF_SIZE * 2; i++) {
					printf("%02X ", cRxBuffer[i] & 0xff);
					if (i % 16 == 15)
						printf("\n");
					if (i == SDCARD_BUFF_SIZE - 1)
						printf("next:\n");
				}
			} else {
				printf("read error!\n");
			}
		} else {
			printf("write error!\n");
		}
	} else {
		printf("read error!\n");
	}
	printf("test end\n");
}
/**********************************************
功能：检查SDIO命令发送，接收是否结束
入口：cmd:命令 be_resp：=1有应答 =0无应答
出口：=0应答超时 =1执行成功
说明：无
**********************************************/
int Chk_CMD_End(int cmd, int be_resp) {
	int finish0;

	if (!be_resp) { // No response
		finish0 = rSDICSTA;
		// Check cmd end
		while (!(finish0 & 0x800))
			finish0 = rSDICSTA;
		// Clear cmd end state
		rSDICSTA = finish0;

		return 1;
	} else {// With response
		finish0 = rSDICSTA;
		// Check cmd/rsp end
		while (!(finish0 & (0x200 + 0x400)))
			finish0 = rSDICSTA;
		//TODO:有的版本代码认为CMD9 CRC no check
		if ((cmd == 1) || (cmd == 41)) {// CRC no check, CMD9 is a long Resp. command.
			if ((finish0 & 0xf00) != 0xa00) {// Check error
				// Clear error state
				rSDICSTA = finish0;
				if (finish0 & 0x400)
					return 0;// Timeout error
			}
			// Clear cmd & rsp end state
			rSDICSTA = finish0;
		} else {// CRC check
			if ((finish0 & 0x1f00) != 0xa00) {// Check error
				SD_DEBUG("CMD%d:rSDICSTA=0x%x, rSDIRSP0=0x%x\n", cmd, finish0, rSDIRSP0);
				// Clear error state
				rSDICSTA = finish0;

				if (((finish0 & 0x400) == 0x400))
					return 0;// Timeout error
			}
			rSDICSTA = finish0;
		}
		return 1;
	}
}

/**********************************************
功能：使卡进入IDEL状态
入口：无
出口：无
说明：无
**********************************************/
void CMD0(void) {
	rSDICARG = 0x0;
	rSDICCON = (1 << 8) | 0x40;		// No_resp, start
	Chk_CMD_End(0, 0);
}
/**********************************************
功能：MMC卡检测
入口：无
出口：=1:MMC卡 =0:非MMC卡
说明：无
**********************************************/
u8_t CMD1(void) {
	rSDICARG = 0xff8000;					//(SD OCR:2.7V~3.6V)
	rSDICCON = (0x1 << 9) | (0x1 << 8) | 0x41; 		//sht_resp, wait_resp, start,

	if (Chk_CMD_End(1, 1)) {	//[31]:Card Power up status bit (busy)
		if ((rSDIRSP0 >> 16) == 0x80ff) {
			return 1;			// Success
		} else
			return 0;
	}
	return 0;
}
/**********************************************
功能：检测卡类型、固件版本、工作电压状况
入口：无
出口：
 =1：SD V1.X或MMC
 =2：标准SD卡或SDHC V2.0
 =0：无效卡
说明：无
**********************************************/
u8_t CMD8(void) {
	rSDICARG = 0x000001AA;
	rSDICCON = (0x1 << 9) | (0x1 << 8) | 0x48;	//sht_resp, wait_resp, start

	if (!Chk_CMD_End(8, 1))
		return 1;
	if ((rSDIRSP0 & 0x1aa) == 0x1aa)
		return 2;
	else
		return 0;
}
/**********************************************
功能：检测卡是否插入
入口：iRCA:RCA
出口：=0 失败 =1 成功
说明：无
**********************************************/
u8_t CMD55(u16_t iRCA) {
	rSDICARG = iRCA << 16;
	rSDICCON = (0x1 << 9) | (0x1 << 8) | 0x77;	//sht_resp, wait_resp, start
	return Chk_CMD_End(55, 1);
}
/**********************************************
功能：检测SD卡上电状态
入口：iRCA:RCA
出口：
 =0应答错误或者卡正忙
 =1标准SD卡
 =2SDHC V2.0
说明：无
**********************************************/
u8_t ACMD41(u16_t iRCA) {
	u8_t cReturn;

	if (!CMD55(iRCA))
		return 0;

	rSDICARG = 0x40ff8000;	//ACMD41(SD OCR:2.7V~3.6V CCS:1)
	//rSDICARG=0xffc000;	//ACMD41(MMC OCR:2.6V~3.6V)
	rSDICCON = (0x1 << 9) | (0x1 << 8) | 0x69; //sht_resp, wait_resp, start, ACMD41

	if (Chk_CMD_End(41, 1)) {
		if (rSDIRSP0 == 0xc0ff8000)
			cReturn = 2;	//SDHC
		else if (rSDIRSP0 == 0x80ff8000)
			cReturn = 1;	//标准SD
		else
			cReturn = 0;	//应答错误
		return cReturn;	// Success
	}
	return 0;
}
/**********************************************
功能：获取CID卡识别寄存器的数据
入口：无
出口：=0失败 =1成功
说明：无
**********************************************/
u8_t CMD2(u8_t *cCID_Info) {
	rSDICARG = 0x0;
	rSDICCON = (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | 0x42; //lng_resp, wait_resp, start

	if (!Chk_CMD_End(2, 1))
		return 0;

	*(cCID_Info + 0) = rSDIRSP0 >> 24;
	*(cCID_Info + 1) = rSDIRSP0 >> 16;
	*(cCID_Info + 2) = rSDIRSP0 >> 8;
	*(cCID_Info + 3) = rSDIRSP0;
	*(cCID_Info + 4) = rSDIRSP1 >> 24;
	*(cCID_Info + 5) = rSDIRSP1 >> 16;
	*(cCID_Info + 6) = rSDIRSP1 >> 8;
	*(cCID_Info + 7) = rSDIRSP1;
	*(cCID_Info + 8) = rSDIRSP2 >> 24;
	*(cCID_Info + 9) = rSDIRSP2 >> 16;
	*(cCID_Info + 10) = rSDIRSP2 >> 8;
	*(cCID_Info + 11) = rSDIRSP2;
	*(cCID_Info + 12) = rSDIRSP3 >> 24;
	*(cCID_Info + 13) = rSDIRSP3 >> 16;
	*(cCID_Info + 14) = rSDIRSP3 >> 8;
	*(cCID_Info + 15) = rSDIRSP3;

	return 1;
}
/**********************************************
功能：给SD卡设定一个地址(RCA)
入口：
 iCardType = 0:SD卡，=1:MMC卡
出口：=0 失败 =1 成功
说明：无
**********************************************/
u8_t CMD3(u16_t iCardType, u16_t *iRCA) {
	rSDICARG = iCardType << 16;					// (MMC:Set RCA, SD:Ask RCA-->SBZ)
	rSDICCON = (0x1 << 9) | (0x1 << 8) | 0x43;	// sht_resp, wait_resp, start

	if (!Chk_CMD_End(3, 1))
		return 0;

	if (iCardType) {
		*iRCA = 1;
	} else {
		*iRCA = (rSDIRSP0 & 0xffff0000) >> 16;
	}

	return 1;
}
/**********************************************
功能：让卡进入选中状态
入口：cSorD = 0:不需要应答 = 1：要应答
出口：无
说明：无
**********************************************/
u8_t CMD7(u8_t cSorD, u16_t iRCA) {
	if (cSorD) {
		rSDICARG = iRCA << 16;				// (RCA,stuff bit)
		rSDICCON = (0x1 << 9) | (0x1 << 8) | 0x47; // sht_resp, wait_resp, start

		if (!Chk_CMD_End(7, 1))
			return 0;
		return 1;
	} else {
		rSDICARG = 0 << 16;		//(RCA,stuff bit)
		rSDICCON = (0x1 << 8) | 0x47;	//no_resp, start
		if (!Chk_CMD_End(7, 0))
			return 0;
		return 1;
	}
}
/**********************************************
功能：获取卡内状态
入口：无
出口：卡状态非0值
说明：无
**********************************************/
u16_t CMD13(u16_t iRCA) {
	rSDICARG = iRCA << 16;				// (RCA,stuff bit)
	rSDICCON = (0x1 << 9) | (0x1 << 8) | 0x4d;	// sht_resp, wait_resp, start

	if (!Chk_CMD_End(13, 1))
		return 0;
	return rSDIRSP0;
}
/**********************************************
功能：设定数据总线位宽
入口：
 BusWidth =0：1bit =1：4bit
 iRCA:RCA
出口：=0：失败 =1：成功
说明：无
**********************************************/
u8_t ACMD6(u8_t BusWidth, u16_t iRCA) {
	if (!CMD55(iRCA))
		return 0;
	rSDICARG = BusWidth << 1;					//Wide 0: 1bit, 1: 4bit
	rSDICCON = (0x1 << 9) | (0x1 << 8) | 0x46;	//sht_resp, wait_resp, start
	return Chk_CMD_End(6, 1);
}
/**********************************************
功能：获取卡的CSD寄存器的值
入口：
 iRCA:卡的RCA
 lCSD：读取的CSD缓存
出口：=0失败 =1成功
说明：无
**********************************************/
u8_t CMD9(u16_t iRCA, u32_t *lCSD) {
	rSDICARG = iRCA << 16;			// (RCA,stuff bit)
	rSDICCON = (0x1 << 10) | (0x1 << 9) | (0x1 << 8) | 0x49;	// long_resp, wait_resp, start

	if (!Chk_CMD_End(9, 1))
		return 0;

	*(lCSD + 0) = rSDIRSP0;
	*(lCSD + 1) = rSDIRSP1;
	*(lCSD + 2) = rSDIRSP2;
	*(lCSD + 3) = rSDIRSP3;
	return 1;
}
/**********************************************
功能：读取一个数据块
入口：起始地址
出口：=1：成功 =0：失败
说明：无
**********************************************/
u8_t CMD17(u32_t Addr) {
	//STEP1:发送指令
	rSDICARG = Addr;				//设定指令参数
	rSDICCON = (1 << 9) | (1 << 8) | 0X51;	//发送CMD17指令
	return Chk_CMD_End(17, 1);
}
/**********************************************
功能：读取多个数据块
入口：起始地址
出口：=1：成功 =0：失败
说明：无
**********************************************/
u8_t CMD18(u32_t Addr) {
	//STEP1:发送指令
	rSDICARG = Addr;				//设定指令参数
	rSDICCON = (1 << 9) | (1 << 8) | 0X52;	//发送CMD17指令
	return Chk_CMD_End(18, 1);
}
/**********************************************
功能：停止数据传输
入口：无
出口：=1：成功 =0：失败
说明：无
**********************************************/
u8_t CMD12(void) {
	rSDICARG = 0x0;
	rSDICCON = (0x1 << 9) | (0x1 << 8) | 0x4c;	//sht_resp, wait_resp, start,
	return Chk_CMD_End(12, 1);
}
/**********************************************
功能：写入一个数据块
入口：无
出口：=1：成功 =0：失败
说明：无
**********************************************/
u8_t CMD24(u32_t Addr) {
	//STEP1:发送指令
	rSDICARG = Addr;				//设定指令参数
	rSDICCON = (1 << 9) | (1 << 8) | 0x58;	//发送CMD24指令
	return Chk_CMD_End(24, 1);
}
/**********************************************
功能：写入多个数据块
入口：无
出口：=1：成功 =0：失败
说明：无
**********************************************/
u8_t CMD25(u32_t Addr) {
	//STEP1:发送指令
	rSDICARG = Addr;				//设定指令参数
	rSDICCON = (1 << 9) | (1 << 8) | 0x59;	//发送CMD25指令
	return Chk_CMD_End(25, 1);
}
/**********************************************
功能：设置擦除起始地址
入口：无
出口：=1：成功 =0：失败
说明：无
**********************************************/
u8_t CMD32(u32_t Addr) {
	//STEP1:发送指令
	rSDICARG = Addr;				//设定指令参数
	rSDICCON = (1 << 9) | (1 << 8) | 0x60;	//发送CMD32指令
	return Chk_CMD_End(32, 1);
}
/**********************************************
功能：设置擦除终止地址
入口：无
出口：=1：成功 =0：失败
说明：无
**********************************************/
u8_t CMD33(u32_t Addr) {
	//STEP1:发送指令
	rSDICARG = Addr;				//设定指令参数
	rSDICCON = (1 << 9) | (1 << 8) | 0x61;	//发送CMD33指令
	return Chk_CMD_End(33, 1);
}
/**********************************************
功能：擦除卡内指定区域的数据
入口：无
出口：=1：成功 =0：失败
说明：无
**********************************************/
u8_t CMD38(void) {
	//STEP1:发送指令
	rSDICARG = 0;					//设定指令参数
	rSDICCON = (1 << 9) | (1 << 8) | 0x66;	//发送CMD38指令
	return Chk_CMD_End(38, 1);
}

/**********************************************
功能：锁定或者解锁SD卡
入口：
 cSelDesel = 1:锁定 =0解锁
 iCardRCA: CARD RCA
出口：=1：成功 =0：失败
说明：无
**********************************************/
u8_t Card_sel_desel(u8_t cSelDesel, u16_t iCardRCA) {
	return CMD7(cSelDesel, iCardRCA);
}

/**********************************************
功能：设置卡通信宽度
入口：
 cCardType 卡类型
 cBusWidth =0：1bit =1：4bit
出口：
 =1：成功 =0：失败
说明：无
**********************************************/
u8_t Set_bus_Width(u8_t cCardType, u8_t cBusWidth, u16_t iRCA) {
	if (cCardType == MMC_CARD)
		return 0;
	return ACMD6(cBusWidth, iRCA);
}

u8_t SDI_CheckDATend(void) {
	int finish;
	finish = rSDIDSTA;
	while (!((finish & 0x10) || (finish & 0x20))) {  // Chek timeout or data end
		finish = rSDIDSTA;
	}//one of the DatFin and DatTout occur
	if ((finish & 0x2) || (finish & 0x1)) {
		udelay(200);
	}
	if ((finish & 0xfc) != 0x10) {
		printf("receive data(or receive busy) time out!!\n\r");
		rSDIDSTA = 0xf8; // Clear all error state
		return 1;
	}
	rSDIDSTA = 0xf8; // Clear all error state
	return 1;
}

/**********************************************************************************
功 能：该函数用于从 SD 卡中读出指定块起始地址和数据块数目的多个连续数据块，当要读取的
数据量满足时则停止读取。
参 数：
 U32 Addr 被读块的起始地址
 U32 count 期待被读出的块数目
 U32* RxBuffer 用于接收读出数据的缓冲区
返回值：
 0 读块操作不成功
 1 读块操作成功
举 例：
在主调函数中定义一个数组作为接收缓冲区，如 U32 Rx_buffer[BufferSize];
然后开始调用 Read_Mult_Block(addr,BufferSize,Rx_buffer);
**********************************************************************************/
u8_t Read_Mult_Block(u32_t Addr, u32_t count, u8_t* RxBuffer) {
	u32_t i = 0;
	u32_t status = 0;
	rSDIFSTA = rSDIFSTA | (1 << 16); // FIFO reset
	rSDIDCON = (count) | (2 << 12) | (1 << 14) | (1 << 16) | (1 << 17) | (1 << 19) | (2 << 22);
	//TODO:sd v2.0 中 Addr为扇区号，因此代码不兼容sd v1.0
	if (count == 1)
		while (CMD17(Addr) != 1);		//发送读单个块指令
	else
		while (CMD18(Addr) != 1); //发送读多块操作指令
	rSDIDSTA = 0xf8;// Clear all flags
	for (int i = 0; i < count; i++) {
		for (int j = 0; j < 128;) {
			//接收数据到缓冲区
			if (rSDIDSTA & 0x20) {
				//检查是否超时
				rSDIDSTA = (0x1 << 0x5); //清除超时标志
				return 0;
			}
			status = rSDIFSTA;
			if ((status & 0x1000) == 0x1000) {
				//如果接收 FIFO 中有数据
				u32_t temp = rSDIDAT;
				RxBuffer[0] = ((u8_t *)&temp)[0];
				RxBuffer[1] = ((u8_t *)&temp)[1];
				RxBuffer[2] = ((u8_t *)&temp)[2];
				RxBuffer[3] = ((u8_t *)&temp)[3];
				RxBuffer += 4;
				j++;
			}
		}
		if ((rSDIDSTA & (1 << 6))) {  // Check data-reccive CRC for every Block
			printf("SDI_ReadBlock failed:data-receive CRC error\n\r");
			rSDIDSTA = (1 << 6);	// Clear data CRC error flag
			return 0;
		}
	}
	if (!SDI_CheckDATend()) {	//*** remain count is not zero
		rSDIDCON  =  0x0;		//DataCon clear
		rSDIFSTA  = (1 << 9);	//clear Rx FIFO Last data Ready
		if (count > 1)			//CMD12 is needed after CMD18 to change SD states to tran
			while (CMD12() != 1); //发送结束指令
		return 0;
	} else {					//*** remain count is zero
		rSDIDCON  =  0x0;		//DataCon clear
		rSDIFSTA  = (1 << 9);	//clear Rx FIFO Last data Ready
		//CMD17 is now in tran(opertion complete) while CMD18 in data
		if (count > 1)			//CMD12 is needed after CMD18 to change SD states to tran
			while (CMD12() != 1); //发送结束指令
		return 1;
	}
}

/**********************************************************************************
功 能：该函数用于向 SD 卡的多个数据块写入数据
参 数：
 U32 Addr 被写块的起始地址
 U32 count 期待被写入块数目
 U32* TxBuffer 待发送数据的缓冲区
返回值：
 0 数据写入操作失败
 1 数据写入操作成功
举 例：
在主调函数中定义一个数组作为发送缓冲区，如 U32 Tx_buffer[BlockSize];
然后开始调用 Write_Mult_Block(addr,DatSize,Tx_buffer);
**********************************************************************************/
u8_t Write_Mult_Block(u32_t Addr, u32_t count, u8_t* TxBuffer) {
	u16_t i = 0;
	u32_t status = 0;
	rSDIFSTA = rSDIFSTA | (1 << 16);	// FIFO reset
	rSDIDCON = (count) | (3 << 12) | (1 << 14) | (1 << 16) | (1 << 17) | (1 << 20) | (2 << 22);
	//TODO:sd v2.0 中 Addr为扇区号，因此代码不兼容sd v1.0
	if (count == 1)
		while (CMD24(Addr) != 1);	//发送写单块操作指令
	else
		while (CMD25(Addr) != 1);	//发送写多块操作指令
	for (int i = 0; i < count; i++) {
		for (int j = 0; j < 128;) {
			//开始传递数据到缓冲区
			status = rSDIFSTA;
			if ((status & 0x2000)) {
				//如果发送 FIFO 可用，即 FIFO 未满
				u32_t temp;
				((u8_t *)&temp)[0] = TxBuffer[0];
				((u8_t *)&temp)[1] = TxBuffer[1];
				((u8_t *)&temp)[2] = TxBuffer[2];
				((u8_t *)&temp)[3] = TxBuffer[3];
				rSDIDAT = temp;
				TxBuffer += 4;
				j++;
			}
		}
		do {
			while (!CMD13(SDCard.iCardRCA));	//Poll the state of SD card
		} while ((rSDIRSP0 & (1 << 8)) == 0);			//if SD card is not ready for new data
		//wait until SD card is ready for new data

		if (rSDIDSTA & (1 << 7)) { //Check send-data CRC for every block
			printf("SDI_WriteBlock failed:data-send CRC error\n\r");
			rSDIDSTA = (1 << 7);    //Clear send-data CRC error flag
			return 0;               //if Block-CRC error has occured,state of CMD24 is tran.
			//but state of CMD25 is rcv
		}
	}
	if (!SDI_CheckDATend()) {	//*** remain counter is not zero
		rSDIDCON  =  0x0;		//DataCon clear
		if (count > 1)			//CMD12 is needed after CMD25 to change SD states to tran
			while (CMD12() != 1); //发送结束指令
		return 0;
	} else {					//*** remain counter became zero
		rSDIDCON  =  0x0; 		//DataCon clear

		if (count > 1)			//CMD12 is needed after CMD25 to change SD states to tran
			while (CMD12() != 1);	//发送结束指令
		do {
			while (!CMD13(SDCard.iCardRCA));	//Poll the state of SD card
		} while ((rSDIRSP0 & (0x1f00)) != 0x900);	//wait for programing flash
		//SD card return to tran
		return 1;
	}
}

/**********************************************************************************
功 能：该函数用于擦除指定地址区间的数据
参 数：
 U32 StartAddr 擦除的起始地址
 U32 EndAddr 擦除的结束地址
返回值：
 0 擦除操作成功
 1 擦除操作失败
注 意：
 起始和结束地址最好与扇区对齐，擦除的最小单位是扇区，如果起始于结束地址差一个扇区的大
 小，但由于没有与扇区对齐，从而使起始地址和结束地址跨度为两个扇区，那么这两个扇区将会
 被擦除。
**********************************************************************************/
u8_t Erase_Block(u32_t StartAddr, u32_t EndAddr) {
	if (CMD32(StartAddr) != 1)
		return 0;
	if (CMD33(EndAddr) != 1)
		return 0;
	if (CMD38() != 1)
		return 0;
	return 1;
}

/**********************************************
功能：SD卡初始化
入口：无
出口：=0失败 =1成功
说明：无
**********************************************/
u8_t SDI_init(void) {
	int i;
	u8_t  MBR[512];

	udelay(500000);	//当板子重新上电时需要延时
	//printf("GPEUP=%X,GPECON=%X,GPGDAT=%X,GPGCON=%X,GPGUP=%X\n",rGPEUP,rGPECON,rGPGDAT,rGPGCON,rGPGUP);
	rGPEUP  &= ~((1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));   // SDCMD ,SDDATA[3:0] =>PULL UP
	rGPECON = ((rGPECON & ~(0xfff << 10)) | (0xaaa << 10));  //GPE5 GPE6 GPE7 GPE8 GPE9 GPE 10 => SDI

	rSDIPRE = PCLK / (INICLK) - 1;	// 400KHz
	rSDICON = (0 << 4) | 1;			// Type A, clk enable
	rSDIFSTA = rSDIFSTA | (1 << 16);	// FIFO reset
	rSDIBSIZE = 0x200;		// 512byte(128word)
	rSDIDTIMER = 0x7fffff;

	for (i = 0; i < 0x1000; i++);  	// Wait 74SDCLK for card

	CMD0();
	if (CMD1()) {
		SDCard.cCardType = MMC_CARD;
	} else {
		switch (CMD8()) {
			case 0://卡固件无效
				SDCard.cCardType = INVALID_CARD;
				SD_DEBUG("卡固件无效\n");
				break;
			case 1://非 SD2.0 卡
				SDCard.cCardType = SD_V1X_CARD;
				SD_DEBUG("非 SD2.0 卡n");
				break;
			case 2://SD2.0 卡
				SDCard.cCardType = SDHC_V20_CARD;
				SD_DEBUG("SD2.0 卡\n");
				break;
		}
	}

	SDCard.iCardRCA = 0;
	for (int j = 0; j < 20; j++) {
		for (i = 0; i < 100; i++) {
			if (ACMD41(SDCard.iCardRCA))
				break;
			udelay(2000);
		}
		if (i < 100)
			break;
		CMD1();
	}


	if (i == 100) {
		printf("Initialize fail\nNo Card assertion\n");
		return 0;
	} else {
		SD_DEBUG("SD is ready\n");
	}

	if (CMD2(SDCard.cCardCID)) {
		SD_DEBUG("CID\n");
		SD_DEBUG("MID = %d\n", SDCard.cCardCID[0]);
		SD_DEBUG("OLD = %d\n", (SDCard.cCardCID[1] * 0X100) + SDCard.cCardCID[2]);
		SD_DEBUG("生产厂家:%s\n", (SDCard.cCardCID + 3));
		SD_DEBUG("生产日期:20%d,%d\n", ((SDCard.cCardCID[13] & 0x0f) << 4) + ((SDCard.cCardCID[14] & 0xf0) >> 4), (SDCard.cCardCID[14] & 0x0f));
	} else {
		printf("Read Card CID is fail!\n");
		return 0;
	}

	//RCA
	if (SDCard.cCardType == MMC_CARD) {
		if (CMD3(1, &SDCard.iCardRCA)) {
			SDCard.iCardRCA = 1;
			rSDIPRE = (PCLK / MMCCLK) - 1;
			SD_DEBUG("MMC Frequency is %dHz\n", (PCLK / (rSDIPRE + 1)));
		} else {
			printf("Read MMC RCA is fail!\n");
			return 0;
		}
	} else {
		if (CMD3(0, &SDCard.iCardRCA)) {
			rSDIPRE = PCLK / (SDCLK) - 1;	// Normal clock=25MHz
			SD_DEBUG("SD Card RCA = 0x%x\n", SDCard.iCardRCA);
			SD_DEBUG("SD Frequency is %dHz\n", (PCLK / (rSDIPRE + 1)));
		} else {
			printf("Read SD RCA is fail!\n");
			return 0;
		}
	}

	//CSD
	if (CMD9(SDCard.iCardRCA, SDCard.lCardCSD)) {
		SDCard.lCardSize = (((SDCard.lCardCSD[1] & 0x0000003f) << 16) + ((SDCard.lCardCSD[2] & 0xffff0000) >> 16) + 1) * 512;
		SDCard.lSectorSize = ((SDCard.lCardCSD[2] >> 6) & 0x0000007f) + 1;

		SD_DEBUG("Read Card CSD OK!\n");
		SD_DEBUG("0x%08x\n", SDCard.lCardCSD[0]);
		SD_DEBUG("0x%08x\n", SDCard.lCardCSD[1]);
		SD_DEBUG("0x%08x\n", SDCard.lCardCSD[2]);
		SD_DEBUG("0x%08x\n", SDCard.lCardCSD[3]);
		printf("卡容量为:%dGB,%dMB\n", SDCard.lCardSize / 1024 / 1024, SDCard.lCardSize / 1024);
	} else {
		printf("Read Card CSD Fail!\n");
		return 0;
	}


	if (Card_sel_desel(1, SDCard.iCardRCA)) {
		SD_DEBUG("Card sel desel OK!\n");
	} else {
		printf("Card sel desel fail!\n");
		return 0;
	}

	//cmd13
	//iTemp = CMD13(SDCard.iCardRCA);
	//if (iTemp) {
	//	printf("Card Status is 0x%x\n", iTemp);
	//}

	if (Set_bus_Width(SDCard.cCardType, 1, SDCard.iCardRCA)) {
		SD_DEBUG("Bus Width is 4bit\n");
	} else {
		SD_DEBUG("Bus Width is 1bit\n");
		return 0;
	}

	if (Read_Block(0, 1, MBR)) {
		LBA_OFFSET   = ((MBR[457] << 24) + (MBR[456] << 16) + (MBR[455] << 8) + MBR[454]);
		TOTAL_SECTOR = ((MBR[461] << 24) + (MBR[460] << 16) + (MBR[459] << 8) + MBR[458]);
		TOTAL_SIZE   = (TOTAL_SECTOR / 2 / 1024);
		SD_DEBUG("MBR Boot OK!LBA_OFFSET=%4d,SD_TOTAL_SIZE=%4dMB\n\r", LBA_OFFSET, TOTAL_SIZE);
		SD_DEBUG("SDI Init OK\n\r");
		return 1;
	} else {
		printf("SDI Read MBR error!!Cannot boot for SD card!!\r\n");
		return 0;
	}
	return 1;
}



u8_t Read_Block(u32_t Addr, u32_t count, u8_t* RxBuffer) {
	assert((count) && (RxBuffer));
	if (count == 0) {
		return 0;
	} else if (count < 4096) {
		SD_DEBUG("read: addr:%X size:%d buffer:%X\n", Addr, count, RxBuffer);
		return Read_Mult_Block(Addr, count, RxBuffer);
	} else {
		//TODO
		assert(0);
		return 0;
	}
}
u8_t Write_Block(u32_t Addr, u32_t count, u8_t* TxBuffer) {
	assert((count) && (TxBuffer));
	if (count == 0) {
		return 0;
	} else if (count < 4096) {
		SD_DEBUG("write: addr:%X size:%d buffer:%X\n", Addr, count, TxBuffer);
		return Write_Mult_Block(Addr, count, TxBuffer);
	} else {
		//TODO
		assert(0);
		return 0;
	}
}
