#include <stdio.h>
#include <s3c24xx.h>
#include <interrupt.h>
//lwip
#include "lwip/opt.h"
#include "lwip/api.h"
#include "lwip/tcpip.h"
#include "lwip/init.h"
#include "lwip/stats.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/pbuf.h"
#include "lwip/netif.h"
#include "lwip/sockets.h"
#include "lwip/ip.h"
#include "lwip/raw.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/autoip.h"
#include "lwip/igmp.h"
#include "lwip/dns.h"
#include "netif/etharp.h"

//dm9000 driver
#include "dm9000.h"

struct netif dm9000a_netif;
void LWIP_Module_Init(void);
extern err_t ethernetif_init(struct netif *netif);
extern void  ethernetif_input(struct netif *netif, void *ptr);

void dm9000_input(void  *q_msg) {
	ethernetif_input(&dm9000a_netif, q_msg);
}

void EINT_DM9000_Init(void) {
	//GPF7=EINT7
	GPFCON &=  ~(3 << 14);
	GPFCON |= (2 << 14);

	//TODO:可能非必要
	//GPF0-7禁止上拉电阻
	GPFUP  = 0xff;
	//EINT7高电平触发
	EXTINT0 &= ~(7 << 28);
	EXTINT0 |= (1 << 28);

	EINTMASK &= ~(1 << EINT7);
	request_irq(EINT4_7, DM9000A_Rx);
}

//lwip module init
void LWIP_Module_Init(void) {
	struct ip_addr ipaddr, netmask, gateway;

	//lwip_init();
	tcpip_init(NULL, NULL);
	IP4_ADDR(&ipaddr , 192, 168,  11,   3);
	IP4_ADDR(&netmask, 255, 255, 255,   0);
	IP4_ADDR(&gateway, 192, 168,  11,   1);

	netif_add(&dm9000a_netif, &ipaddr, &netmask, &gateway,
			  (void *)NULL,
			  ethernetif_init,
			  tcpip_input);
	netif_set_default(&dm9000a_netif);
	netif_set_up(&dm9000a_netif);
}

void net_init() {
	printf("初始化lwip...\n");
	LWIP_Module_Init();
	EINT_DM9000_Init();

	printf("初始化http server...\n");
	httpserver_init();
}

/*
//handle the dm9000a mac frame,actived by dm9000a ISR
void Task0(void *pdata) {
	INT8U err;
	void  *q_msg;
	DM_RXPKT *tmp;

	pdata = pdata;
	printf("任务0启动...\n");
	while (1) {
		q_msg = OSQPend(rxpkt_q, 0, &err);
		//tmp =(DM_RXPKT *)q_msg;
		if (err == OS_NO_ERR) {
			ethernetif_input(&dm9000a_netif, q_msg);
		}
		OSMemPut(rxpkt_q_mem, q_msg);
	}
}
*/
