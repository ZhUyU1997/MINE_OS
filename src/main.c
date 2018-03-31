#include <stdio.h>
#include <s3c24xx.h>
#include "ucos_ii.h"
#include "serial.h"


#include "lcddrv.h"
#include "framebuffer.h"

#include "app_cfg.h"
#include "interrupt.h"

#include "GUI.H"
#include "math.h"
#include "GUI_Protected.h"
#include "WM.h"
#include "Dialog.h"
#include "LISTBOX.h"
#include "EDIT.h"
#include "SLIDER.h"
#include "FRAMEWIN.h" 


OS_STK  MainTaskStk[MainTaskStkLengh];
OS_STK	Task0Stk[Task0StkLengh];       // Define the Task0 stack

void OSTickISR();
void timer_init() {
	int delay_time = 10000;
	//定时器配制寄存器 0 （TCFG0）
	TCFG0 |= (24); //定时器 0，1 的预分频值
	//定时器控制寄存器 1 （TCON）
	TCON &= (~(15 << 8)); //清空8~11位
	TCON |= (1 << 11); //定时器 1间隙模式（自动重载）
	TCON |= (1 << 9); //定时器 1手动更新 TCNTB1
	//TCONB1:定时器 1  计数缓冲寄存器
	TCNTB1 = delay_time;

	TCON |= (1 << 8); //启动
	TCON &= ~(1 << 9); //定时器 1 取消手动更新
	
	set_irq_handler(INT_TIMER1, OSTickISR);
	INTMS_set(INT_TIMER1);
}


int main() {
	uart0_init();
	
/* 	//初始化uC/OS
	OSInit ();
	//初始化系统时基
	OSTimeSet(0);
	//创建系统初始任务
	OSTaskCreate (MainTask, (void *)0, &MainTaskStk[MainTaskStkLengh - 1], MainTaskPrio);
	INT8U  err;
	OSTaskNameSet(MainTaskPrio, (INT8U *)"MainTask", &err);
	OSStart (); */
	
	Lcd_Port_Init();						// 设置LCD引脚
	Tft_Lcd_Init(MODE_TFT_16BIT_480272);	// 初始化LCD控制器
	Lcd_PowerEnable(0, 1);					// 设置LCD_PWREN有效，它用于打开LCD的电源
	Lcd_EnvidOnOff(1);						// 使能LCD控制器输出信号
	
	ClearScr(0xf);						// 清屏
	lcd_set_background_color(0xf);

	printf("\n\n************************************************\n");
	printf("正在初始化MMU...\n");
	
	printf("正在初始化LCD...\n");
	mmu_init();
	
	printf("正在初始化GUI...\n");
	GUI_Init();
	
	GUI_Clear(); 
	GUI_SetPenSize(10); 
	GUI_SetColor(0xff);
	printf("GUI_DrawLine\n");	
	GUI_DrawLine(10, 11, 24, 100); //非抗锯齿函数显示正常
	GUI_DispString("Hello World");  
	while(1);
	return 0;
}

void MainTask(void *pdata) { //Main Task create taks0 and task1

#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
	OS_CPU_SR  cpu_sr;
#endif
	printf("\nMainTask\n");
	enable_irq();
	OS_ENTER_CRITICAL();
	timer_init();//initial timer0 for ucos time tick
	OS_EXIT_CRITICAL();
	printf("1\n");
	OSStatInit();
	printf("2\n");
	OSTaskCreate (Task0, (void *)0, &Task0Stk[Task0StkLengh - 1], Task0Prio);
	INT8U  err;
	OSTaskNameSet(Task0Prio, (INT8U *)"Task0Prio", &err);
	printf("3\n");
	while (1) {
		printf("\nMain Task\n");
		OSTimeDly(OS_TICKS_PER_SEC*10);
	}
}

void Task0(void *pdata) { //Init lwip and create task2
	char *s = "msg";
	char i;
	printf("Enter Task0\n");
	while (1) {
		printf("\rCPU Usage: %d%%", OSCPUUsage); //cpu usage XX%
		OSTimeDly(OS_TICKS_PER_SEC/10);
	}
}
