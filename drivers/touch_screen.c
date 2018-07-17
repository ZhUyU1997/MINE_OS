#include <stdio.h>
#include <interrupt.h>
#include <timer.h>
#include "s3c24xx.h"
#include "serial.h"

// ADCCON寄存器
#define PRESCALE_DIS        (0 << 14)
#define PRESCALE_EN         (1 << 14)
#define PRSCVL(x)           ((x) << 6)
#define ADC_INPUT(x)        ((x) << 3)
#define ADC_START           (1 << 0)
#define ADC_ENDCVT          (1 << 15)

// ADCTSC寄存器
#define UD_SEN          (1 << 8)
#define DOWN_INT        (UD_SEN*0)
#define UP_INT          (UD_SEN*1)
#define YM_SEN          (1 << 7)
#define YM_HIZ          (YM_SEN*0)
#define YM_GND          (YM_SEN*1)
#define YP_SEN          (1 << 6)
#define YP_EXTVLT       (YP_SEN*0)
#define YP_AIN          (YP_SEN*1)
#define XM_SEN          (1 << 5)
#define XM_HIZ          (XM_SEN*0)
#define XM_GND          (XM_SEN*1)
#define XP_SEN          (1 << 4)
#define XP_EXTVLT       (XP_SEN*0)
#define XP_AIN          (XP_SEN*1)
#define XP_PULL_UP      (1 << 3)
#define XP_PULL_UP_EN   (XP_PULL_UP*0)
#define XP_PULL_UP_DIS  (XP_PULL_UP*1)
#define AUTO_PST        (1 << 2)
#define CONVERT_MAN     (AUTO_PST*0)
#define CONVERT_AUTO    (AUTO_PST*1)
#define XP_PST(x)       (x << 0)

#define NOP_MODE        0
#define X_AXIS_MODE     1
#define Y_AXIS_MODE     2
#define WAIT_INT_MODE   3


/* 设置进入等待中断模式，XP_PU,XP_Dis,XM_Dis,YP_Dis,YM_En
 * (1)对于S3C2410，位[8]只能为0，所以只能使用下面的wait_down_int，
 *    它既等待Pen Down中断，也等待Pen Up中断
 * (2)对于S3C2440，位[8]为0、1时分别表示等待Pen Down中断或Pen Up中断
 */
/* 进入"等待中断模式"，等待触摸屏被按下 */
void wait_down_int() {
	ADCTSC = DOWN_INT | XP_PULL_UP_EN | XP_AIN | XM_HIZ | YP_AIN | YM_GND |  XP_PST(WAIT_INT_MODE);
}
/* 进入"等待中断模式"，等待触摸屏被松开 */
void wait_up_int()   {
	ADCTSC = UP_INT | XP_PULL_UP_EN | XP_AIN | XM_HIZ | YP_AIN | YM_GND | XP_PST(WAIT_INT_MODE);
}

/* 进入自动(连续) X/Y轴坐标转换模式 */
void mode_auto_xy() {
	ADCTSC = CONVERT_AUTO | XP_PULL_UP_DIS | XP_PST(NOP_MODE);
}


static int g_ts_x = 0;
static int g_ts_y = 0;
static int g_ts_pressure = 0;
volatile int g_ts_data_valid = 0;
static int test_x_array[16];
static int test_y_array[16];


int get_touch_x() {
	return g_ts_x;
}
int get_touch_y() {
	return g_ts_y;
}

void report_ts_xy(int x, int y, int pressure) {
	//printf("x = %08d, y = %08d\n", x, y);
	if (g_ts_data_valid == 0) {
		g_ts_x = x;
		g_ts_y = y;
		g_ts_pressure = pressure;
		g_ts_data_valid = 1;
		//TODO:
		//GUI_TOUCH_Exec();
		//GUI_TOUCH_Exec();
	}
}

void ts_read_raw(int *px, int *py, int *ppressure) {
	while (g_ts_data_valid == 0);
	*px = g_ts_x;
	*py = g_ts_y;
	*ppressure = g_ts_pressure;
	g_ts_data_valid = 0;
}

int ts_read_raw_asyn(int *px, int *py, int *ppressure) {
	if (g_ts_data_valid == 0)
		return -1;
	*px = g_ts_x;
	*py = g_ts_y;
	*ppressure = g_ts_pressure;
	g_ts_data_valid = 0;
	return 0;
}

/*
 * INT_TC的中断服务程序
 * 当触摸屏被按下时，进入自动(连续) X/Y轴坐标转换模式；
 * 当触摸屏被松开时，进入等待中断模式，再次等待INT_TC中断
 */
static void Isr_Tc(void) {
	if (ADCDAT0 & 0x8000) {
		//printf("Stylus Up\n");
		close_timer();
		report_ts_xy(-1, -1, 0);
		wait_down_int();    /* 进入"等待中断模式"，等待触摸屏被按下 */
	} else {
		//printf("Stylus Down\n");

		mode_auto_xy();     /* 进入自动(连续) X/Y轴坐标转换模式 */

		/* 设置位[0]为1，启动A/D转换
		 * 注意：ADCDLY为50000，PCLK = 50MHz，
		 *       要经过(1/50MHz)*50000=1ms之后才开始转换X坐标
		 *       再经过1ms之后才开始转换Y坐标
		 */
		ADCCON |= ADC_START;
	}
}
static void timer_handle() {
	mode_auto_xy();
	ADCCON |= ADC_START;
}

#define ABS(x) (((x)>0)?(x):(-(x)))
/*
 * INT_ADC的中断服务程序
 * A/D转换结束时发生此中断
 * 先读取X、Y坐标值，再进入等待中断模式
 */
static void Isr_Adc(void) {
	int x = ADCDAT0 & 0x3ff;
	int y = ADCDAT1 & 0x3ff;

	static int adc_cnt = 0;
	static int adc_x = 0;
	static int adc_y = 0;
	static int pre_adc_x = 0;
	static int pre_adc_y = 0;
	/* 进入ADC中断时, TS处于"自动测量模式" */

	/* 只有在"等待中断模式"下才可以使用ADCDAT0'BIT 15来判断触摸笔状态 */
	wait_up_int();      /* 进入"等待中断模式"，等待触摸屏被松开 */

	//TODO
	udelay(10);
	if (ADCDAT0 & 0x8000) {
		adc_cnt = 0;
		adc_x = 0;
		adc_y = 0;
		close_timer();
		report_ts_xy(-1, -1, 0);
		wait_down_int();
		return;
	}
#if 0
	//abs > 1000 ,如果太小会死循环，可以设个计数器解决
	if ((adc_cnt > 0)
			&& ((ABS(adc_x / adc_cnt - x) > 1000) || (ABS(adc_y / adc_cnt - y) > 1000))) {
		mode_auto_xy();/* 进入"自动测量"模式 */
		ADCCON |= (1 << 0);/* 启动ADC */
		return;
	}
#endif
	/* 第1次启动ADC后:
	 *   a. 要连续启动N次, 获得N个数据, 求平均值并上报
	 *   b. 得到N次数据后, 再启动TIMER
	 */

	adc_x += x;
	adc_y += y;

	test_x_array[adc_cnt] = pre_adc_x = x;
	test_y_array[adc_cnt] = pre_adc_y = y;

	adc_cnt++;

	if (adc_cnt == 16) {
		adc_x >>= 4;
		adc_y >>= 4;
		report_ts_xy(adc_x, adc_y, 1);

		adc_cnt = 0;
		adc_x = 0;
		adc_y = 0;

		/* 启动定时器以再次读取数据 */
		/* 先设置TS进入"等待中断模式" */
		wait_up_int();
		set_timer(10, timer_handle);
	} else {
		mode_auto_xy();/* 进入"自动测量"模式 */
		ADCCON |= (1 << 0);/* 启动ADC */
	}
}

/*
 * ADC、触摸屏的中断服务程序
 * 对于INT_TC、INT_ADC中断，分别调用它们的处理程序
 */
void AdcTsIntHandle(void) {
	if (get_subsrcpnd() & (1 << INT_TC)) {
		Isr_Tc();
	}
	if (get_subsrcpnd() & (1 << INT_ADC_S)) {
		Isr_Adc();
	}
}

/*
 * 初始化触摸屏
 */
void init_Ts(void) {
	// 使能预分频功能，设置A/D转换器的时钟 = PCLK/(49+1)
	/* [15] : ECFLG,  1 = End of A/D conversion
	 * [14] : PRSCEN, 1 = A/D converter prescaler enable
	 * [13:6]: PRSCVL, adc clk = PCLK / (PRSCVL + 1)
	 * [5:3] : SEL_MUX, 000 = AIN 0
	 * [2]   : STDBM
	 * [0]   : 1 = A/D conversion starts and this bit is cleared after the startup.
	 */
	ADCCON = PRESCALE_EN | PRSCVL(49);

	/*  按下触摸屏, 延时一会再发出TC中断
	 *  延时时间 = ADCDLY * 晶振周期 = ADCDLY * 1 / 12000000 = 5ms
	 */
	ADCDLY = 60000;
	set_subint(INT_TC);// 开启INT_TC中断，即触摸屏被按下或松开时产生中断
	set_subint(INT_ADC_S);// 开启INT_ADC中断，即A/D转换结束时产生中断
	request_irq(INT_ADC, AdcTsIntHandle); // 设置ADC中断服务程序
	wait_down_int();    /* 进入"等待中断模式"，等待触摸屏被按下 */
}
void close_Ts(void) {
	clr_subint(INT_TC);
	clr_subint(INT_ADC_S);
	free_irq(INT_ADC);
}

