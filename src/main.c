#include <stdio.h>
#include <s3c24xx.h>
#include "serial.h"
#include "command.h"
#include "interrupt.h"

//ucosii
#include "ucos_ii.h"
#include "app_cfg.h"

//lcd driver
#include "lcddrv.h"
#include "framebuffer.h"

//ucgui
#include "GUI.h"

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

//fs
#include "ff.h"
struct netif dm9000a_netif;

OS_STK  MainTaskStk[MainTaskStkLengh];
OS_STK	Task0Stk[Task0StkLengh];       // Define the Task0 stack

void OSTickISR();
void LWIP_Module_Init(void);
extern err_t ethernetif_init(struct netif *netif);
extern void  ethernetif_input(struct netif *netif, void *ptr);


FATFS fatworkarea;         // Work area (file system object) for logical drives 

void show_bss_info(){
	extern char __bss_start, __bss_end;
    unsigned int size = &__bss_end - &__bss_start;
	printf("BSS大小为：%uKB, %uMB\n", size/1024, size/1024/1024);
}
int main() {
	irq_init();
	Port_Init();
	uart0_init();
	//uart0_interrupt_init();
	printf("\n\n************************************************\n");
	show_bss_info();
	printf("初始化MMU...\n");
	mmu_init();

	printf("初始化TIMER...\n");
	timer_init();

	printf("初始化LCD...\n");
	Lcd_Port_Init();						// 设置LCD引脚
	Tft_Lcd_Init(MODE_TFT_16BIT_480272);	// 初始化LCD控制
	Lcd_PowerEnable(0, 1);					// 设置LCD_PWREN有效，它用于打开LCD的电源
	Lcd_EnvidOnOff(1);						// 使能LCD控制器输出信号
	ClearScr(0x0);							// 清屏

	printf("初始化SD控制器...\n");
	SDI_init();
	printf("初始化fatfs...\n");
	f_mount(0,&fatworkarea);
	usb_init_slave();
	enable_irq();
	cmd_loop();
	/*TEST_SD();*/
	printf("初始化uC/OS...\n");
	OSInit ();

	printf("初始化系统时钟...\n");
	OSTimeSet(0);

	printf("创建系统初始任务...\n");
	OSTaskCreate (OSMainTask, (void *)0, &MainTaskStk[MainTaskStkLengh - 1], MainTaskPrio);
	INT8U  err;
	OSTaskNameSet(MainTaskPrio, (INT8U *)"MainTask", &err);

	printf("启动uC/OS...\n");
	OSStart ();
	while (1);
}

void OSMainTask(void *pdata) {
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
	OS_CPU_SR  cpu_sr;
#endif

	enable_irq();
	OS_ENTER_CRITICAL();
	//initial timer for ucos time tick
	init_tick(1000, OSTimeTick);
	OS_EXIT_CRITICAL();

#if (OS_TASK_STAT_EN > 0)
	OSStatInit();		//----统计任务初始化函数
#endif


	printf("初始化lwip...\n");
	LWIP_Module_Init();
	EINT_DM9000_Init();

	printf("创建任务0...\n");
	OSTaskCreate (Task0, (void *)0, &Task0Stk[Task0StkLengh - 1], Task0Prio);
	INT8U  err;
	OSTaskNameSet(Task0Prio, (INT8U *)"Task0", &err);

	printf("初始化http server...\n");
	httpserver_init();

	while (0) {
		//printf("OSMainTask Delay\n");
		OSTimeDly(OS_TICKS_PER_SEC);
	}
	//InfoNES_Main();
	init_Ts();
	MainTask1();
	while (1);
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

void dm9000_input(void  *q_msg){
	ethernetif_input(&dm9000a_netif, q_msg);
}

void Task0(void *pdata) {
	printf("任务0启动...\n");
	while (1) {
		cmd_loop();
	}
}


void EINT_DM9000_Init(void) {
	//GPF7=EINT7
	GPFCON &=  ~(3 << 14);
	GPFCON |=  (2 << 14);

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
