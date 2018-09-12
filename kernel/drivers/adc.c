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
static void adc_init(int channel) {
	/* [15] : ECFLG,  1 = End of A/D conversion
	 * [14] : PRSCEN, 1 = A/D converter prescaler enable
	 * [13:6]: PRSCVL, adc clk = PCLK / (PRSCVL + 1)
	 * [5:3] : SEL_MUX, 000 = AIN 0
	 * [2]   : STDBM
	 * [0]   : 1 = A/D conversion starts and this bit is cleared after the startup.
	 */
	// 选择模拟通道，使能预分频功能，设置A/D转换器的时钟 = PCLK/(49+1)
	ADCCON = PRESCALE_EN | PRSCVL(49) | ADC_INPUT(channel);
	ADCDLY = 0xff;
	// 清除位[2]，设为普通转换模式
	ADCTSC &= ~(1 << 2);
}

int adc_read(int channel) {
	adc_init(channel);
	// 设置位[0]为1，启动A/D转换
	ADCCON |= ADC_START;
	// 当A/D转换真正开始时，位[0]会自动清0
	while (ADCCON & ADC_START);
	// 检测位[15]，当它为1时表示转换结束
	while (!(ADCCON & ADC_ENDCVT));
	// 读取数据
	return (ADCDAT0 & 0x3ff);
}

/*
 * 通过A/D转换，测量可变电阻器的电压值
 */
void get_adc(int *vol, int *t, int ch) {
	float v = ((float)adc_read(ch) * 3.3) / 1024.0; // 计算电压值
	*vol = v;
	*t = (v - (int)v) * 1000;   // 计算小数部分, 本代码中的printf无法打印浮点数
}
