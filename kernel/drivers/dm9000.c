#include <timer.h>
#include "dm9000.h"
#include "ucos_ii.h"

#define DM9000A_BASE      0x20000000
#define DM9000_IO       DM9000A_BASE
#define DM9000_DATA  (DM9000A_BASE+4)

/*DEFINES*/
#define DM9000A_10MHD              0
#define DM9000A_100MHD             1
#define DM9000A_10MFD              4
#define DM9000A_100MFD             5
#define DM9000A_AUTO               8
#define DM9000A_1M_HPNA           16

#define DM9000A_ID	     0x90000A46L
#define DM9000A_PKT_MAX	        1536   // Received packet max size                                 
#define DM9000A_PKT_RDY	        0x01   // Packet ready to receive                                  

/*DM9000A REGISTER ADDRESS DEFINES*/
#define DM9000A_NCR             0x00   // Network Control Register                               
#define DM9000A_NSR             0x01   // Network Status  Register                               
#define DM9000A_TCR             0x02   // TX Control Register                                   
#define DM9000A_TSR1            0x03   // TX Status  Register 1                                 
#define DM9000A_TSR2            0x04   // TX Status  Register 2                                   
#define DM9000A_RCR             0x05   // RX Control Register                                    
#define DM9000A_RSR             0x06   // RX Status  Register                               
#define DM9000A_ROCR            0x07   // Receive Overflow Counter Register                    
#define DM9000A_BPTR            0x08   // Back Pressure Threshold Register   
#define DM9000A_FCTR            0x09   // Flow Control  Threshold Resgister 
#define DM9000A_FCR             0x0A   // RX Flow Control Register          

#define DM9000A_EPCR            0x0B   // EEPROM & PHY Control Register        
#define DM9000A_EPAR            0x0C   // EEPROM & PHY Address Register        
#define DM9000A_EPDRL           0x0D   // EEPROM & PHY Low  Byte Data Register 
#define DM9000A_EPDRH           0x0E   // EEPROM & PHY High Byte Data Register 

#define DM9000A_WCR             0x0F   // Wake Up Control Register(8-bit)  

#define DM9000A_PAR0            0x10   // Physical  Address Register, Byte 0 
#define DM9000A_PAR1            0x11   //    "         "        "   , Byte 1 
#define DM9000A_PAR2            0x12   //    "         "        "   , Byte 1 
#define DM9000A_PAR3            0x13   //    "         "        "   , Byte 1  
#define DM9000A_PAR4            0x14   //    "         "        "   , Byte 1  
#define DM9000A_PAR5            0x15   //    "         "        "   , Byte 1 
#define DM9000A_MAR0            0x16   // Multicast Address Register, Byte 0 
#define DM9000A_MAR1            0x17   //    "         "        "   , Byte 1  
#define DM9000A_MAR2            0x18   //    "         "        "   , Byte 2  
#define DM9000A_MAR3            0x19   //    "         "        "   , Byte 3  
#define DM9000A_MAR4            0x1A   //    "         "        "   , Byte 4   
#define DM9000A_MAR5            0x1B   //    "         "        "   , Byte 5 
#define DM9000A_MAR6            0x1C   //    "         "        "   , Byte 6  
#define DM9000A_MAR7            0x1D   //    "         "        "   , Byte 7  

#define DM9000A_GPCR	        0x1E   // General Purpose Control Register(8-bit)  
#define DM9000A_GPR             0x1F   // General Purpose Register                                
#define DM9000A_TRPAL           0x22   // TX SRAM Read  Pointer Address Low  Byte                 
#define DM9000A_TRPAH           0x23   // TX SRAM Read  Pointer Address High Byte                  
#define DM9000A_RWPAL           0x24   // RX SRAM Write Pointer Address Low  Byte                 
#define DM9000A_RWPAH           0x25   // RX SRAM Write Pointer Address High Byte                 

#define DM9000A_VIDL            0x28   // Vendor  ID Low  Byte                                     
#define DM9000A_VIDH            0x29   // Vendor  ID High Byte                                     
#define DM9000A_PIDL            0x2A   // Product ID Low  Byte                                     
#define DM9000A_PIDH            0x2B   // Product ID High Byte                                    
#define DM9000A_CHIPR           0x2C   // Chip Revision                                           

#define DM9000A_TCR2            0x2D   // TX Control Register 2                                  
#define DM9000A_OCR             0x2E   // Operation Control Register                             
#define DM9000A_SMCR            0x2F   // Special Mode Control Register                           
#define DM9000A_ETXCSR          0x30   // Early Transmit Control/Status Register                 
#define DM9000A_TCSCR           0x31   // Transmit Check Sum Control Register                    
#define DM9000A_RCSCSR          0x32   // Receive  Check Sum Control Status Register              
#define DM9000A_MPAR            0x33   // MII PHY Address Register    
#define DM9000A_LEDCR           0x34   // LED Pin Control Register                          
#define DM9000A_BUSCR           0x38   // Processor Bus Control Register                          
#define DM9000A_INTCR           0x39   // INT Pin Control Register                                

#define DM9000A_PHY             0x40

#define DM9000A_SCCR            0x50   // System Clock Turn ON Control Register                   
#define DM9000A_RSCCR           0x51   // Resume System Clock  Control Register                    

#define DM9000A_MRCMDX          0xF0   // Memory Data Pre-Fetch Rd Cmd w/o Address Increment Reg.  
#define DM9000A_MRCMDX1         0xF1   // Memory Data Read  Command with   Address Increment Reg.  
#define DM9000A_MRCMD           0xF2   // Memory Data Read  Command with   Address Increment Reg.  
#define DM9000A_MRRH            0xF4   // Memory Data Read  Address Register Low  Byte             
#define DM9000A_MRRL            0xF5   // Memory Data Read  Address Register High Byte             
#define DM9000A_MWCMDX	        0xF6   // Memory Data Write Command w/o    Address Increment Reg.  
#define DM9000A_MWCMD           0xF8   // Memory Data Write Command with   Address Increment Reg.  
#define DM9000A_MWRL            0xFA   // Memory Data Write Address Register Low  Byte             
#define DM9000A_MWRH            0xFB   // Memory Data Write Address Register High Byte             

#define DM9000A_TXPLL           0xFC   // TX Packet Length Low  Byte Register                      
#define DM9000A_TXPLH           0xFD   // TX Packet Length High Byte Register                      

#define DM9000A_ISR             0xFE   // Interrupt Status Register                               
#define DM9000A_IMR             0xFF   // Interrupt Mask   Register                               

/*DM9000A PHY REGISTER ADDRESS DEFINES & REGISTER VALUE DEFINES*/
#define DM9000A_PHY_BMCR        0x00
#define DM9000A_PHY_BMSR        0x01
#define DM9000A_PHY_PHYID1      0x02
#define DM9000A_PHY_PHYID2      0x02
#define DM9000A_PHY_ANAR        0x04
#define DM9000A_PHY_ANLPAR      0x05
#define DM9000A_PHY_ANER        0x06
#define DM9000A_PHY_DSCR        0x10
#define DM9000A_PHY_DSCSR       0x11
#define DM9000A_PHY_10BTCSR     0x12
#define DM9000A_PHY_PWDOR       0x13
#define DM9000A_PHY_SPECCFG     0x17

/* Defines for PHY register [0x00]  */
#define PHY_BMCR_RESET       (1 << 15)
#define PHY_BMCR_LBK_EN      (1 << 14)
#define PHY_BMCR_100MBPS     (1 << 13)
#define PHY_BMCR_10MBPS      (0 << 13)
#define PHY_BMCR_AN_EN       (1 << 12)
#define PHY_BMCR_PWR_DN      (1 << 11)
#define PHY_BMCR_ISOLATE     (1 << 10)
#define PHY_BMCR_AN_RESTART  (1 <<  9)
#define PHY_BMCR_FDX         (1 <<  8)
#define PHY_BMCR_COLL_EN     (1 <<  7)

/* Defines for PHY register [0x01]  */
#define PHY_BMSR_T4          (1 << 15)
#define PHY_BMSR_TX_FDX      (1 << 14)
#define PHY_BMSR_TX_HDX      (1 << 13)
#define PHY_BMSR_10_FDX      (1 << 12)
#define PHY_BMSR_10_HDX      (1 << 11)
#define PHY_BMSR_MF          (1 <<  6)
#define PHY_BMSR_AN_COMP     (1 <<  5)
#define PHY_BMSR_FAULT       (1 <<  4)
#define PHY_BMSR_AN_ABLE     (1 <<  3)
#define PHY_BMSR_LINK        (1 <<  2)
#define PHY_BMSR_JABBER      (1 <<  1)
#define PHY_BMSR_EXT         (1 <<  0)

/* Defines for PHY register [0x04]  */
#define PHY_ANAR_NP          (1 << 15)
#define PHY_ANAR_ACK         (1 << 14)
#define PHY_ANAR_RF          (1 << 13)
#define PHY_ANAR_FCS         (1 << 10)
#define PHY_ANAR_T4          (1 <<  9)
#define PHY_ANAR_TX_FDX      (1 <<  8)
#define PHY_ANAR_TX_HDX      (1 <<  7)
#define PHY_ANAR_10_FDX      (1 <<  6)
#define PHY_ANAR_10_HDX      (1 <<  5)

/*DM9000A REGISTER VALUE DEFINES     */
/* Defines for register [0x00]       */
#define NCR_WAKEEN           (1 << 6)
#define NCR_FCOL             (1 << 4)
#define NCR_FDX              (1 << 3)
#define NCR_LBK_PHY          (2 << 1)
#define NCR_LBK_MAC          (1 << 1)
#define NCR_LBK_NORMAL       (0 << 1)
#define NCR_RST              (1 << 0)

/* Defines for register [0x01]       */
#define NSR_SPEED            (1 << 7)
#define NSR_LINKST           (1 << 6)
#define NSR_WAKEST           (1 << 5)
#define NSR_TX2END           (1 << 3)
#define NSR_TX1END           (1 << 2)
#define NSR_RXOV             (1 << 1)

/* Defines for register [0x02]      */
#define TCR_TJDIS            (1 << 6)
#define TCR_EXCECM           (1 << 5)
#define TCR_PAD_DIS2         (1 << 4)
#define TCR_CRC_DIS2         (1 << 3)
#define TCR_PAD_DIS1         (1 << 2)
#define TCR_CRC_DIS1         (1 << 1)
#define TCR_TXREQ            (1 << 0)

/* Defines for register [0x03]      */
/* Defines for register [0x04]      */
#define TSR_TJTO             (1 << 7)
#define TSR_LC               (1 << 6)
#define TSR_NC               (1 << 5)
#define TSR_LCOL             (1 << 4)
#define TSR_COL              (1 << 3)
#define TSR_EC               (1 << 2)

/* Defines for register [0x05]      */
#define RCR_WTDIS            (1 << 6)
#define RCR_DIS_LONG         (1 << 5)
#define RCR_DIS_CRC          (1 << 4)
#define RCR_ALL              (1 << 3)
#define RCR_RUNT             (1 << 2)
#define RCR_PRMSC            (1 << 1)
#define RCR_RXEN             (1 << 0)

/* Defines for register [0x06]      */
#define RSR_RF               (1 << 7)
#define RSR_MF               (1 << 6)
#define RSR_LCS              (1 << 5)
#define RSR_RWTO             (1 << 4)
#define RSR_PLE              (1 << 3)
#define RSR_AE               (1 << 2)
#define RSR_CE               (1 << 1)
#define RSR_FOE              (1 << 0)
#define RSR_ERRORS             (0xBF)

/* Defines for register [0x08]      */
#define BPTR_BPHW(x)       (((x) & 0x0F) << 4)
#define BPTR_JPT_5u            (0x00)
#define BPTR_JPT_10u           (0x01)
#define BPTR_JPT_15u           (0x02)
#define BPTR_JPT_25u           (0x03)
#define BPTR_JPT_50u           (0x04)
#define BPTR_JPT_100u          (0x05)
#define BPTR_JPT_150u          (0x06)
#define BPTR_JPT_200u          (0x07)
#define BPTR_JPT_250u          (0x08)
#define BPTR_JPT_300u          (0x09)
#define BPTR_JPT_350u          (0x0A)
#define BPTR_JPT_400u          (0x0B)
#define BPTR_JPT_450u          (0x0C)
#define BPTR_JPT_500u          (0x0D)
#define BPTR_JPT_550u          (0x0E)
#define BPTR_JPT_600u          (0x0F)

/* Defines for register [0x09]        */
#define FCTR_HWOT(x)       ( (x & 0x0F) << 4)
#define FCTR_LWOT(x)         (x & 0x0F)

/* Defines for register [0x0A]        */
#define FCR_TXP0             (1 << 7)
#define FCR_TXPF             (1 << 6)
#define FCR_TXPEN            (1 << 5)
#define FCR_BKPA             (1 << 4)
#define FCR_BKPM             (1 << 3)
#define FCR_RXPS             (1 << 2)
#define FCR_RXPCS            (1 << 1)
#define FCR_FLCE             (1 << 0)

/* Defines for register [0x0B]        */
#define EPCR_REEP            (1 << 5)
#define EPCR_WEB             (1 << 4)
#define EPCR_EPOS            (1 << 3)
#define EPCR_ERPRR           (1 << 2)
#define EPCR_ERPRW           (1 << 1)
#define EPCR_ERRE            (1 << 0)

/* Defines for register [0x1F]        */
#define GPR_PHYPD            (1 << 0)
#define GPR_PHYPU            (0 << 0)

/* Defines for register [0x39]        */
#define INTCR_INT_TYPE       (1 << 1)
#define INTCR_INT_POL        (1 << 0)

/* Defines for register [0xFE]        */
#define IMR_PAR              (1 << 7)
#define IMR_LNKCHGI          (1 << 5)
#define IMR_UDRUNI           (1 << 4)
#define IMR_ROOI             (1 << 3)
#define IMR_ROI              (1 << 2)
#define IMR_PTI              (1 << 1)
#define IMR_PRI              (1 << 0)
#define IMR_ALL                (0x3F)

/* Defines for register [0xFF]        */
#define ISR_IOMODE           (1 << 7)
#define ISR_LNKCHG           (1 << 5)
#define ISR_UDRUN            (1 << 4)
#define ISR_ROO              (1 << 3)
#define ISR_ROS              (1 << 2)
#define ISR_PT               (1 << 1)
#define ISR_PR               (1 << 0)
#define ISR_ALL                (0x3F)

/*********************************************************************************************
GLOBAL VARIABLES DEFINE
**********************************************************************************************/
char dm9000a_eth_addr[6] = {0x00,0x0c,0x29,0x4d,0xe4,0xf4};
void *NetRxPackets[10];
//ECB for Queue
//OS_EVENT *rxpkt_q; 
//void     *rxpkt_q_tbl[RXPKT_Q_SIZE];

OS_MEM   *rxpkt_q_mem;
char     rxpkt_pool[ RXPKT_Q_SIZE * sizeof(DM_RXPKT) ];


#define DM9000A_TX_TIMEOUT  500			//ms

/*********************************************************************************************
MARCOS DEFINE
**********************************************************************************************/
#define DM9000_outb(data,reg) ( *(volatile U8  *)reg = data )
#define DM9000_outw(data,reg) ( *(volatile U16 *)reg = data )
#define DM9000_outl(data,reg) ( *(volatile U32 *)reg = data )
#define DM9000_inb(reg) (*(volatile U8  *)reg)
#define DM9000_inw(reg) (*(volatile U16 *)reg)
#define DM9000_inl(reg) (*(volatile U32 *)reg)


/*********************************************************************************************
CONFIG DM9000 DEBUG
**********************************************************************************************/
#define DM9000A_DEBUG(x,fmt,arg...)  printf(fmt,##arg)
#define DM9000A_DEBUG_COM 1

/*********************************************************************************************
函数名  : NetDly_us()
输入    : null
输出    : 1:succeed 0:fail
功能描述:
**********************************************************************************************/

void NetDly_us(U32 usec)
{
    udelay(usec);
}
/*********************************************************************************************
函数名  : NetDly_ms()
输入    : null
输出    : 1:succeed 0:fail
功能描述:
**********************************************************************************************/
/*
void NetDly_ms(U32 ms)
{
    Delay(ms);
}
*/
/*********************************************************************************************
函数名  : DM9000A_(void)
输入    : null
输出    : 1:succeed 0:fail
功能描述: 8bit return
**********************************************************************************************/
U8 DM9000A_ReadReg(U32 reg) {
	DM9000_outb(reg, DM9000_IO);
	return DM9000_inb(DM9000_DATA);
}

/*********************************************************************************************
函数名  : DM9000A_(void)
输入    : null
输出    : 1:succeed 0:fail
功能描述: 8bit write
**********************************************************************************************/
void DM9000A_WriteReg(U32 reg , U8 val) {
	DM9000_outb(reg, DM9000_IO);
	DM9000_outb(val, DM9000_DATA);
}
/*********************************************************************************************
函数名  : PHY_ReadReg()
输入    :
输出    :
功能描述: 16bit return
**********************************************************************************************/
U16 PHY_ReadReg(U32 phy_reg) {
	U16 val;

	DM9000A_WriteReg(DM9000A_EPAR, DM9000A_PHY | phy_reg);
	DM9000A_WriteReg(DM9000A_EPCR, EPCR_ERPRR | EPCR_EPOS); //0xc
	NetDly_us(100);
	DM9000A_WriteReg(DM9000A_EPCR, EPCR_EPOS); //0x8 or 0x0(uboot)
	val  = DM9000A_ReadReg(DM9000A_EPDRL);
	val |= (DM9000A_ReadReg(DM9000A_EPDRH) << 8);
	return val;
}



/*********************************************************************************************
函数名  : DM9000A_()
输入    :
输出    : null
功能描述:
**********************************************************************************************/
void PHY_WriteReg(U32 phy_reg, U16 phy_val) {
	U8 val_h, val_l;

	val_h = (U8)((phy_val >> 8) & 0xff);
	val_l = (U8)(phy_val & 0xff);
	DM9000A_WriteReg(DM9000A_EPAR , DM9000A_PHY | phy_reg);
	DM9000A_WriteReg(DM9000A_EPDRH, val_h);
	DM9000A_WriteReg(DM9000A_EPDRL, val_l);
	DM9000A_WriteReg(DM9000A_EPCR , EPCR_ERPRW | EPCR_EPOS); //0xa
	NetDly_us(500);  //500us(uboot)
	DM9000A_WriteReg(DM9000A_EPCR , EPCR_EPOS); //0x8 or 0x0(uboot)
}



/*********************************************************************************************
函数名  : DM9000A_Info(void)
输入    : null
输出    : U32 id 0x90000a46
功能描述:
**********************************************************************************************/
U32 DM9000A_Info(void) {
	U32 id;

	id  =  DM9000A_ReadReg(DM9000A_VIDL);
	id |= (DM9000A_ReadReg(DM9000A_VIDH) << 8);
	id |= (DM9000A_ReadReg(DM9000A_PIDL) << 16);
	id |= (DM9000A_ReadReg(DM9000A_PIDH) << 24);
	return (id);
}


/*********************************************************************************************
函数名  : DM9000A_(void)
输入    : null
输出    : null
功能描述:
**********************************************************************************************/
void DM9000A_Probe(void) {
	U32 id_val;
	id_val = DM9000A_Info();
	if (id_val == DM9000A_ID) {
		DM9000A_DEBUG(DM9000A_DEBUG_COM, "DM9000_address=0x%x,DM9000_id=0x%x\n", DM9000A_BASE, id_val);
	} else {
		DM9000A_DEBUG(DM9000A_DEBUG_COM, "ERROR:dm9000a is not found\n");
	}
}
/*********************************************************************************************
函数名  : DM9000A_(void)
输入    : null
输出    : null
功能描述:
**********************************************************************************************/
void DM9000A_Reset(void) {
	U8 check_pidl, check_pidh;

	DM9000A_WriteReg(DM9000A_GPR, 0); /*Power internal PHY by writting 0 */

	DM9000A_WriteReg(DM9000A_NCR, NCR_RST | NCR_LBK_MAC); /* 3 */
	do {
		NetDly_us(30);  /* Wait at least 20 us*/
	} while ( (DM9000A_ReadReg(DM9000A_NCR) & 0x01) );

	DM9000A_WriteReg(DM9000A_NCR, 0);

	DM9000A_WriteReg(DM9000A_NCR, NCR_RST | NCR_LBK_MAC); /* Issue a second reset */
	do {
		NetDly_us(30);
	} while ( (DM9000A_ReadReg(DM9000A_NCR) & 0x01) );

	check_pidl = DM9000A_ReadReg(DM9000A_PIDL);
	check_pidh = DM9000A_ReadReg(DM9000A_PIDH);
	if ( (check_pidl != 0x0) || (check_pidh != 0x90 ) ) {
		DM9000A_DEBUG(DM9000A_DEBUG_COM, "ERROR:resetting DM9000a not responding!!\n");
		DM9000A_DEBUG(DM9000A_DEBUG_COM, "      PIDH=%02x,PIDL=%02x\n", check_pidh, check_pidl);
	}

}


/*********************************************************************************************
函数名  : DM9000A_(void)
输入    : null
输出    : null
功能描述:
**********************************************************************************************/
void PHY_SetMode(U32 mode) {
	DM9000A_WriteReg(DM9000A_GPR, GPR_PHYPD);
	DM9000A_WriteReg(DM9000A_GPR, GPR_PHYPU);

	switch (mode) {
		case DM9000A_10MHD:
			PHY_WriteReg(DM9000A_PHY_ANAR, 0x0021);
			PHY_WriteReg(DM9000A_PHY_BMCR, 0x0000);
			break;
		case DM9000A_10MFD:
			PHY_WriteReg(DM9000A_PHY_ANAR, 0x0041);
			PHY_WriteReg(DM9000A_PHY_BMCR, 0x1100);
			break;
		case DM9000A_100MHD:
			PHY_WriteReg(DM9000A_PHY_ANAR, 0x0081);
			PHY_WriteReg(DM9000A_PHY_BMCR, 0x2000);
			break;
		case DM9000A_100MFD:
			PHY_WriteReg(DM9000A_PHY_ANAR, 0x0101);
			PHY_WriteReg(DM9000A_PHY_BMCR, 0x3100);
			break;
		case DM9000A_AUTO:
			PHY_WriteReg(DM9000A_PHY_ANAR, 0x01e1);
			PHY_WriteReg(DM9000A_PHY_BMCR, 0x1200);
			break;
	}

	DM9000A_WriteReg(DM9000A_GPR, 0x00); /*Enable PHY*/
}


/*********************************************************************************************
函数名  : DM9000_outblk_16bit()--Tx
输入    :
输出    : null
功能描述:
**********************************************************************************************/
void DM9000_outblk_16bit(volatile void *data_ptr, int count) {
	int i;
	U32 tmplen = (count + 1) / 2;

	for (i = 0; i < tmplen; i++) {
		DM9000_outw( ((U16 *)data_ptr)[i], DM9000_DATA) ;
	}

}


/*********************************************************************************************
函数名  : DM9000_inblk_16bit()--Rx
输入    :
输出    : null
功能描述:
**********************************************************************************************/
void DM9000_inblk_16bit(void *data_ptr, int count) {
	int i;
	U32 tmplen = (count + 1) / 2;

	for (i = 0; i < tmplen; i++) {
		((U16 *)data_ptr)[i] = DM9000_inw(DM9000_DATA);
	}
}


/*********************************************************************************************
函数名  : DM9000_inblk_16bit()--Rx
输入    :
输出    : null
功能描述:
**********************************************************************************************/
void DM9000_rx_status_16bit(U16 *pRxStatus, U16 *pRxLen) {
	DM9000_outb(DM9000A_MRCMD, DM9000_IO);

	*pRxStatus = DM9000_inw(DM9000_DATA);
	*pRxLen = DM9000_inw(DM9000_DATA);
}


/*********************************************************************************************
函数名  : DM9000A_Tx()
输入    : null
输出    : err:1=ok,0=tx_fail
功能描述:
**********************************************************************************************/
U8 DM9000A_Tx(volatile void *packet, int length) {
	U32 timeout;
	U8  err = 1;
	//printf("DM9000A_Tx\n");
	//NetReceive(packet, length);
	DM9000A_WriteReg(DM9000A_ISR, ISR_PT);

	DM9000_outb(DM9000A_MWCMD, DM9000_IO); /* Move data to DM9000 TX RAM */

	DM9000_outblk_16bit(packet, length); /* push the data to the TX-fifo */

	DM9000A_WriteReg(DM9000A_TXPLL,  length & 0xff );
	DM9000A_WriteReg(DM9000A_TXPLH, (length >> 8) & 0xff ); /* Set TX length to DM9000 */

	DM9000A_WriteReg(DM9000A_TCR, TCR_TXREQ); /* Cleared after TX complete(polling) */

	timeout = OSTimeGet() + DM9000A_TX_TIMEOUT / 5; /*RTC_GetSec()+5*/
	while ( !(DM9000A_ReadReg(DM9000A_NSR) & (NSR_TX1END | NSR_TX2END)) ||
	        !(DM9000A_ReadReg(DM9000A_ISR) &  ISR_PT)  ) {
		if (/*RTC_GetSec()*/OSTimeGet() >= timeout ) {
			//DM9000A_DEBUG(DM9000A_DEBUG_COM,"ERROR:dm9000a transmission timeout \n");
			//err=0;
			err = 1;
			break;
		}
		OSTimeDly(1);
	}

	DM9000A_WriteReg(DM9000A_ISR, ISR_PT);/* clear tx bit in ISR*/
	//printf("transmit done\n");
	return (err);
}

/*********************************************************************************************
函数名  : DM9000A_Halt()
输入    : null
输出    : null
功能描述:
**********************************************************************************************/
void DM9000A_Halt(void) {
	DM9000A_DEBUG(DM9000A_DEBUG_COM, "eth halt\n");

	PHY_WriteReg(0, 0x8000);   //PHY reset
	DM9000A_WriteReg(DM9000A_GPR, 0x01); //power-down PHY
	DM9000A_WriteReg(DM9000A_IMR, 0x80); //disable all int
	DM9000A_WriteReg(DM9000A_RCR, 0x00); //disable Rx
}

/*********************************************************************************************
函数名  : DM9000A_Rx(void)
输入    : null
输出    : null
功能描述:
**********************************************************************************************/
U8 DM9000A_Rx(void) {
	DM_RXPKT *rx_memblk;
	INT8U    err;
	char rxbyte, *data_ptr;
	U16  RxStatus, RxLen = 0;
	//printf("*****Rx\n");
	if ( !( DM9000A_ReadReg(DM9000A_ISR) & 0x01  ) ) { //check packet ready or not
		return 0 ;
	}

	DM9000A_WriteReg(DM9000A_ISR, 0x01); //clear PRI status latched in bit0

	for (;;) { /* There is _at least_ 1 package in the fifo, read them all */

		//TODO:用于内存不足导致读取失败时，恢复读取之前的状态
		//DM9000A_ReadReg(DM9000A_MRRH);
		//DM9000A_ReadReg(DM9000A_MRRL);

		DM9000A_ReadReg(DM9000A_MRCMDX);//dummy read

		rxbyte = ( DM9000_inb(DM9000_DATA) & 0x03 );

		if ( rxbyte > DM9000A_PKT_RDY ) { //Status check: this byte must be 0 or 1
			DM9000A_WriteReg(DM9000A_RCR, 0x00); // stop device
			DM9000A_WriteReg(DM9000A_ISR, 0x80);	/* Stop INT request */
			DM9000A_DEBUG(DM9000A_DEBUG_COM, "ERROR:dm9000 rx-packet is not ready:0x%x\n", rxbyte);
			return 0;
		}

		if ( rxbyte != DM9000A_PKT_RDY ) { //rxbyte=0,
			return 0;
		}

		rx_memblk = OSMemGet(rxpkt_q_mem, &err); //allocat the memblk for rx
		data_ptr = &(rx_memblk->rxcontext[0]);

		DM9000_rx_status_16bit(&RxStatus, &RxLen);
		DM9000_inblk_16bit(data_ptr, RxLen);

		if ( (RxStatus & 0xbf00) || (RxLen < 0x40) || (RxLen > DM9000A_PKT_MAX) ) {
			if (RxStatus & 0x100 ) {
				DM9000A_DEBUG(DM9000A_DEBUG_COM, "ERROR:dm9000a rxstate fifo error\n");
			}
			if (RxStatus & 0x200 ) {
				DM9000A_DEBUG(DM9000A_DEBUG_COM, "ERROR:dm9000a rxstate crc error\n");
			}
			if (RxStatus & 0x8000) {
				DM9000A_DEBUG(DM9000A_DEBUG_COM, "ERROR:dm9000a rx length too small\n");
			}
			if (RxLen > DM9000A_PKT_MAX) {
				DM9000A_DEBUG(DM9000A_DEBUG_COM, "ERROR:dm9000a rx length too big\n");
				DM9000A_Reset();
			}

			err = OSMemPut(rxpkt_q_mem, rx_memblk);//free the memblk because of error

		} else {
			//printf("pack received\n");
			//NetReceive(NetRxPackets[0],RxLen);//+++++passing packet to upper layer
			rx_memblk->length = RxLen;
			dm9000_input(rx_memblk);
			OSMemPut(rxpkt_q_mem, rx_memblk);
			//err = OSQPost(rxpkt_q, (void *)rx_memblk);
		}
	}

	return 0;
}


void NetReceive(void *pdata, U16 len) {
	U16 i = 0;
	printf("ethernet2.0 mac frame start:\n");
	while (i < len) {
		printf("%02x ", ((char *)pdata)[i]&0xff);
		i++;
		if ( ((i % 16) == 0) && (i != 0) ) {
			printf("\n");
		}
	}
	printf("\nethernet2.0 mac frame end \n");
}

/*********************************************************************************************
函数名  : DM9000A_Init(void)
输入    : null
输出    : 1:succeed 0:fail
功能描述:
**********************************************************************************************/
void DM9000A_Init(void) {
	U8    readback[6];
	U32   offset, link, i;
	INT8U err;

	DM9000A_Reset();
	DM9000A_Probe();

	//DM9000A_WriteReg(DM9000A_NCR,0x00);//normal mode

	DM9000A_WriteReg(DM9000A_GPR, 0x00); //pre-activate PHY
	PHY_SetMode(DM9000A_AUTO);
	DM9000A_WriteReg(DM9000A_NCR, 0x00); //only internal phy supported by now
	DM9000A_WriteReg(DM9000A_TCR, 0x00); //Tx polling clear
	DM9000A_WriteReg(DM9000A_BPTR, 0x37); //less 3kb,200us(0x3f uboot)
	DM9000A_WriteReg(DM9000A_FCTR, FCTR_HWOT(3) | FCTR_LWOT(8)); //high/low water
	DM9000A_WriteReg(DM9000A_FCR, 0x00); //
	DM9000A_WriteReg(DM9000A_SMCR, 0x00); //special mode
	DM9000A_WriteReg(DM9000A_NSR, NSR_WAKEST | NSR_TX2END | NSR_TX1END); //clear tx
	DM9000A_WriteReg(DM9000A_ISR, 0x0f); //clear int

	for (i = 0, offset = 0x10; i < 6; i++, offset++) {
		DM9000A_WriteReg(offset, dm9000a_eth_addr[i]);
	}

	for (i = 0, offset = 0x16; i < 8; i++, offset++) {
		DM9000A_WriteReg(offset, 0xff);
	}

	for (i = 0, offset = 0x10; i < 6; i++, offset++) { //read back mac, just to be sure
		readback[i] = DM9000A_ReadReg(offset);
	}

	DM9000A_DEBUG(DM9000A_DEBUG_COM, "Ethaddr=%02x:%02x:%02x:%02x:%02x:%02x\n",
	              readback[0], readback[1],
	              readback[2], readback[3],
	              readback[4], readback[5]);


	DM9000A_WriteReg(DM9000A_RCR, RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN); // RX enable
	DM9000A_WriteReg(DM9000A_IMR, IMR_PAR | IMR_PRI); // Enable TX/RX pointer automatically return

//功能是屏蔽无用的语句。其实被屏蔽的语句是MII接口用的。
#if 0
	i = 0;
	while ( !(PHY_ReadReg(DM9000A_PHY_BMSR) & 0x20) ) {
		NetDly_us(1000);
		i++;
		if (i == 5000) {
			DM9000A_DEBUG(DM9000A_DEBUG_COM, "ERROR:could not establish link.\n");
			break;
		}
	}

	link = ( PHY_ReadReg(17) >> 12);
	switch (link) {
		case 1:
			DM9000A_DEBUG(DM9000A_DEBUG_COM, "10M half duplex\n");
			break;
		case 2:
			DM9000A_DEBUG(DM9000A_DEBUG_COM, "10M full duplex\n");
			break;
		case 4:
			DM9000A_DEBUG(DM9000A_DEBUG_COM, "100M half duplex\n");
			break;
		case 8:
			DM9000A_DEBUG(DM9000A_DEBUG_COM, "100M full duplex\n");
			break;
		default:
			DM9000A_DEBUG(DM9000A_DEBUG_COM, "defualt link type:%d\n", link);

	}
#endif
	rxpkt_q_mem = OSMemCreate( (void *)rxpkt_pool, RXPKT_Q_SIZE, sizeof(DM_RXPKT), &err );//create mem partion
	if ( err != OS_NO_ERR ) {
		DM9000A_DEBUG(DM9000A_DEBUG_COM, "ERROR:dm9000a mem init failed\n");
	}

	//rxpkt_q = OSQCreate( (void *)rxpkt_q_tbl , RXPKT_Q_SIZE);
	//if ( !rxpkt_q ) {
	//	DM9000A_DEBUG(DM9000A_DEBUG_COM, "ERROR:dm9000a queue init failed\n");
	//}

}

















