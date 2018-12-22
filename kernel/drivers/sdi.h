#ifndef __SDI_H__
#define __SDI_H__
#include <sys/types.h>

//#define SD_INIT_CLK   400000    //400khz
//#define SD_NORM_CLK   20000000  //24Mhz
//#define SD_BUS_WIDTH_TYPE 1     //0:1bit  1:4bit

extern volatile u16_t SD_RCA;
extern volatile u32_t LBA_OFFSET;
extern volatile u32_t TOTAL_SECTOR;
extern volatile u32_t TOTAL_SIZE; //(MB)

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
	u8_t cCardType;				/* 卡类型 */
	u8_t cBlockSize;				// 卡的块长度，以为2的2次方表示，如512字节为2的9次方，则该值为9
	u8_t cCardCID[16];			// 卡的CID信息
	u16_t iCardRCA;				/* 卡逻辑地址 */
	u32_t lCardCSD[4];			// 卡的CSD信息
	u32_t lBlockNum;				/* 卡中块的数量 */
	u32_t lBlockLen;				/* 卡的块长度(单位:字节) */
	u32_t lSectorSize;			/* 一次可擦除的块个数 */
	u32_t lCardSize;				//卡容量(单位:字节)


	u32_t timeout_read;			/* 读块超时时间(单位: 8 SPI clock) */
	u32_t timeout_write;			/* 写块超时时间(单位: 8 SPI clock) */
	u32_t timeout_erase;			/* 擦块超时时间(单位: 8 SPI clock) */
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
u8_t CMD1(void);
u8_t CMD8(void);
u8_t CMD55(u16_t iRCA);
u8_t ACMD41(u16_t iRCA)	;
u8_t CMD2(u8_t *cCID_Info);
u8_t CMD3(u16_t iCardType, u16_t *iRCA);
u8_t CMD7(u8_t cSorD, u16_t iRCA);
u16_t CMD13(u16_t iRCA)	;
u8_t ACMD6(u8_t BusWidth, u16_t iRCA);
u8_t CMD9(u16_t iRCA, u32_t *lCSD)	;
u8_t CMD17(u32_t Addr);
u8_t CMD18(u32_t Addr);
u8_t CMD12(void);
u8_t CMD24(u32_t Addr);
u8_t CMD25(u32_t Addr);
u8_t CMD32(u32_t Addr);
u8_t CMD33(u32_t Addr);
u8_t CMD38(void);

u8_t Card_sel_desel(u8_t cSelDesel, u16_t iCardRCA);
u8_t Set_bus_Width(u8_t cCardType, u8_t cBusWidth, u16_t iRCA);
u8_t SDI_init(void);
u8_t Read_Mult_Block(u32_t Addr, u32_t DatSize, u8_t* RxBuffer);
u8_t Write_Mult_Block(u32_t Addr, u32_t DatSize, u8_t* TxBuffer);
u8_t Erase_Block(u32_t StartAddr, u32_t EndAddr);
u8_t Read_Block(u32_t Addr, u32_t count, u8_t* RxBuffer);
u8_t Write_Block(u32_t Addr, u32_t count, u8_t* TxBuffer);
#endif

