#ifndef __SDI_H__
#define __SDI_H__
#include <sys/types.h>

//#define SD_INIT_CLK   400000    //400khz
//#define SD_NORM_CLK   20000000  //24Mhz
//#define SD_BUS_WIDTH_TYPE 1     //0:1bit  1:4bit

extern volatile U16 SD_RCA;
extern volatile U32 LBA_OFFSET;
extern volatile U32 TOTAL_SECTOR;
extern volatile U32 TOTAL_SIZE; //(MB)

#define	INICLK		300000
#define	SDCLK		24000000	//PCLK=49.392MHz
#define	MMCCLK		15000000	//PCLK=49.392MHz



/********************************************************
 宏定义
********************************************************/
#define INVALID_CARD	0
#define MMC_CARD		1
#define SD_V1X_CARD		2
#define SDHC_V20_CARD	3

/* SD卡信息结构体定义 */
/* the information structure variable of SD Card*/
typedef struct SD_STRUCT {
	U8 cCardType;				/* 卡类型 */
	U8 cBlockSize;				// 卡的块长度，以为2的2次方表示，如512字节为2的9次方，则该值为9
	U8 cCardCID[16];			// 卡的CID信息
	U16 iCardRCA;				/* 卡逻辑地址 */
	U32 lCardCSD[4];			// 卡的CSD信息
	U32 lBlockNum;				/* 卡中块的数量 */
	U32 lBlockLen;				/* 卡的块长度(单位:字节) */
	U32 lSectorSize;			/* 一次可擦除的块个数 */
	U32 lCardSize;				//卡容量(单位:字节)


	U32 timeout_read;			/* 读块超时时间(单位: 8 SPI clock) */
	U32 timeout_write;			/* 写块超时时间(单位: 8 SPI clock) */
	U32 timeout_erase;			/* 擦块超时时间(单位: 8 SPI clock) */
} SD_STRUCT;


typedef enum _enum_sd_bus_width {
    ENUM_SD_BUS_1_BIT = 0,
    ENUM_SD_BUS_4_BIT = 1
} enum_sd_bus_width;

typedef enum _enum_card_type {
    ENUM_CARD_TYPE_MMC = 0,
    ENUM_CARD_TYPE_SD  = 1
} enum_card_type;

typedef enum _enum_card_capacity_status {
    ENUM_High_Capacity 		= 0,
    ENUM_Standard_Capacity  = 1
} enum_card_capacity_status;

typedef struct _sd_control_desc {
	enum_sd_bus_width Wide;
	int 	reserved[4];
} sd_control_desc;

typedef struct _card_desc {
	enum_card_type				Card_Type;
	enum_card_capacity_status	Card_Capacity_Stat;
	int  						Maker_ID;
	char 						Product_Name[8];
	int  						Serial_Num;
	int							Manufacturing_Date[2];
	int							RCA;
	int							Device_Size;
	int							Reserved[3];
} card_desc;

#define POL	0
#define INT	1
#define DMA	2


int Chk_CMD_End(int cmd, int be_resp);
void CMD0(void);
U8 CMD1(void);
U8 CMD8(void);
U8 CMD55(U16 iRCA);
U8 ACMD41(U16 iRCA)	;
U8 CMD2(U8 *cCID_Info);
U8 CMD3(U16 iCardType, U16 *iRCA);
U8 CMD7(U8 cSorD, U16 iRCA);
U16 CMD13(U16 iRCA)	;
U8 ACMD6(U8 BusWidth, U16 iRCA);
U8 CMD9(U16 iRCA, U32 *lCSD)	;
U8 CMD17(U32 Addr);
U8 CMD18(U32 Addr);
U8 CMD12(void);
U8 CMD24(U32 Addr);
U8 CMD25(U32 Addr);
U8 CMD32(U32 Addr);
U8 CMD33(U32 Addr);
U8 CMD38(void);

U8 Card_sel_desel(U8 cSelDesel, U16 iCardRCA);
U8 Set_bus_Width(U8 cCardType, U8 cBusWidth, U16 iRCA);
U8 SDI_init(void);
U8 Read_Mult_Block(U32 Addr, U32 DatSize, U8* RxBuffer);
U8 Write_Mult_Block(U32 Addr, U32 DatSize, U8* TxBuffer);
U8 Erase_Block(U32 StartAddr, U32 EndAddr);
U8 Read_Block(U32 Addr, U32 count, U8* RxBuffer);
U8 Write_Block(U32 Addr, U32 count, U8* TxBuffer);
#endif

