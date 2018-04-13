#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__



/* 临界区的保护 */
#define SYS_LIGHTWEIGHT_PROT         1
	 
/* 有系统 */  
#define NO_SYS                       1

#define LWIP_ARP                     1
#define LWIP_SOCKET                  0
#define LWIP_NETCONN                 0
/* 4字节对齐 */
#define MEM_ALIGNMENT                4
	
/* 4M的内存 */  
#define MEM_SIZE                     1024*1024*1

/* pad size */
#define ETH_PAD_SIZE				 0 

//#define LWIP_DEBUG 
#ifdef LWIP_DEBUG
#define ETHARP_DEBUG LWIP_DBG_ON
#define ICMP_DEBUG LWIP_DBG_ON
#define NETIF_DEBUG LWIP_DBG_ON
#endif
#endif /* __LWIPOPTS_H__ */

