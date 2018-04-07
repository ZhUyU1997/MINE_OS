/*
 * FILE: lcddrv.h
 * 操作LCD控制器、调色板等的底层函数接口
 */

#ifndef __LCDDRV_H__
#define __LCDDRV_H__

#include <sys/types.h>

#define LOWER21BITS(n)  ((n) & 0x1fffff)

#define BPPMODE_1BPP    0x8
#define BPPMODE_2BPP    0x9
#define BPPMODE_4BPP    0xA
#define BPPMODE_8BPP    0xB
#define BPPMODE_16BPP   0xC
#define BPPMODE_24BPP   0xD

#define LCDTYPE_TFT     0x3

#define ENVID_DISABLE   0
#define ENVID_ENABLE    1

#define FORMAT8BPP_5551 0
#define FORMAT8BPP_565  1

#define HSYNC_NORM      0
#define HSYNC_INV       1

#define VSYNC_NORM      0
#define VSYNC_INV       1

#define VDEN_NORM		0
#define VDEN_INV		1

#define BSWP            1
#define HWSWP           1

#define MODE_TFT_8BIT_480272 	 (0x410c)
#define MODE_TFT_16BIT_480272    (0x4110)

#define LCDFRAMEBUFFER 0x33c00000

/*
 * 初始化用于LCD的引脚
 */
void Lcd_Port_Init(void);


void Tft_Lcd_Init(int type);

/*
 * 设置调色板
 */
void Lcd_Palette8Bit_Init(void);

/*
 * 设置LCD控制器是否输出信号
 * 输入参数：
 * onoff: 
 *      0 : 关闭
 *      1 : 打开
 */
void Lcd_EnvidOnOff(int onoff);

/*
 * 设置是否输出LCD电源开关信号LCD_PWREN
 * 输入参数：
 *     invpwren: 0 - LCD_PWREN有效时为正常极性
 *               1 - LCD_PWREN有效时为反转极性
 *     pwren:    0 - LCD_PWREN输出有效
 *               1 - LCD_PWREN输出无效
 */
void Lcd_PowerEnable(int invpwren, int pwren);

/*
 * 使用临时调色板寄存器输出单色图像
 * 输入参数：
 *     color: 颜色值，格式为0xRRGGBB
 */
void ClearScrWithTmpPlt(UINT32 color);

/*
 * 停止使用临时调色板寄存器
 */
void DisableTmpPlt(void);

/*
 * 改变调色板为一种颜色
 * 输入参数：
 *     color: 颜色值，格式为0xRRGGBB
 */
void ChangePalette(UINT32 color);


#endif /*__LCDDRV_H__*/
