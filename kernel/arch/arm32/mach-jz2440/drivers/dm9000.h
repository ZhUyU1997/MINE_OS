#ifndef __DM9000_H__
#define __DM9000_H__

#include <sys/types.h>
#include <ucos_ii.h>
/*function list*/
u8_t   DM9000A_ReadReg(u32_t reg);
void DM9000A_WriteReg(u32_t reg, u8_t val);
u16_t  PHY_ReadReg(u32_t phy_reg);
void PHY_WriteReg(u32_t phy_reg, u16_t phy_val);

u32_t  DM9000A_Info(void);
void DM9000A_Probe(void);
void DM9000A_Reset(void);
void PHY_SetMode(u32_t mode);

void DM9000_outblk_16bit(volatile void *data_ptr, int count);
void DM9000_inblk_16bit (void *data_ptr, int count);
void DM9000_rx_status_16bit(u16_t *pRxStatus, u16_t *pRxLen);

u8_t   DM9000A_Tx(volatile void *packet, int length);
void DM9000A_Halt(void);
u8_t   DM9000A_Rx(void);
void DM9000A_Init(void);

void NetReceive(void *pdata, unsigned short len);

typedef struct dm_rxpkt {
	u32_t  length;
	char rxcontext[1600];
} DM_RXPKT;

#define RXPKT_Q_SIZE 100  //max 10 entries means at least 8 memblks(for context of pkt) need to create

extern char     dm9000a_eth_addr[6];
//extern OS_EVENT *rxpkt_q;  //ECB for Queue 
//extern void     *rxpkt_q_tbl[RXPKT_Q_SIZE];

extern OS_MEM   *rxpkt_q_mem;
extern char     rxpkt_pool[ RXPKT_Q_SIZE * sizeof(DM_RXPKT) ];

void dm9000_input(void  *q_msg);
#endif
