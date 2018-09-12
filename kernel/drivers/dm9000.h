#ifndef __DM9000_H__
#define __DM9000_H__

#include <sys/types.h>
#include <ucos_ii.h>
/*function list*/
U8   DM9000A_ReadReg(U32 reg);
void DM9000A_WriteReg(U32 reg, U8 val);
U16  PHY_ReadReg(U32 phy_reg);
void PHY_WriteReg(U32 phy_reg, U16 phy_val);

U32  DM9000A_Info(void);
void DM9000A_Probe(void);
void DM9000A_Reset(void);
void PHY_SetMode(U32 mode);

void DM9000_outblk_16bit(volatile void *data_ptr, int count);
void DM9000_inblk_16bit (void *data_ptr, int count);
void DM9000_rx_status_16bit(U16 *pRxStatus, U16 *pRxLen);

U8   DM9000A_Tx(volatile void *packet, int length);
void DM9000A_Halt(void);
U8   DM9000A_Rx(void);
void DM9000A_Init(void);

void NetReceive(void *pdata, unsigned short len);

typedef struct dm_rxpkt {
	U32  length;
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
