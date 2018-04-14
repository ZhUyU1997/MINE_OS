#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

/* 临界区的保护 */
#define SYS_LIGHTWEIGHT_PROT         1

#define NO_SYS                       0/* 有系统 */ 
#define LWIP_SOCKET                  1
#define LWIP_NETCONN                 1
#define MEM_ALIGNMENT                4/* 4字节对齐 */	
#define MEM_SIZE                     1024*1024*1/* 1M的内存 */  
#define ETH_PAD_SIZE				 0 /* pad size */

//#define LWIP_DEBUG 
#ifdef LWIP_DEBUG
#define ETHARP_DEBUG LWIP_DBG_ON
#define ICMP_DEBUG LWIP_DBG_ON
#define NETIF_DEBUG LWIP_DBG_ON
#endif
#endif /* __LWIPOPTS_H__ */

