
#ifndef COMMON_H
#define COMMON_H

#define GPB4_OUT (0x1<<(2*4))
#define GPB3_OUT (0x1<<(2*3))
#define GPB2_OUT (0x1<<(2*2))

#define GPB4_PULL_UP (~(0x1<<4))
#define GPB3_PULL_UP (~(0x1<<3))
#define GPB2_PULL_UP (~(0x1<<2))

#define GPE0_I2SLRCK (0x2<<(2*0))
#define GPE1_I2SSCKL (0X2<<(2*1))
#define GPE2_CDCLK   (0X2<<(2*2))
#define GPE3_I2SDI   (0x2<<(2*3))
#define GPE4_I2SDO   (0x2<<(2*4))

#define GPE0_NOT_PULL_UP ((0x1<<0))
#define GPE1_NOT_PULL_UP ((0x1<<1))
#define GPE2_NOT_PULL_UP ((0x1<<2))
#define GPE3_NOT_PULL_UP ((0x1<<3))
#define GPE4_NOT_PULL_UP ((0x1<<4))

#define IISCON (*(volatile unsigned int *)0x55000000)
#define IISCON_PRESCALER_ENABLE (0x1<<1)
#define IISCON_INTERFACE_ENABLE (0x1<<0)
#define IISCON_INTERFACE_DISABLE (0x0<<0)
#define IISCON_RX_IDLE (0x1<<2)
#define IISCON_TX_IDLE (0x1<<3)
#define IISCON_TX_DMA (0x1<<5)
#define IISCON_ENABLE_IIS (0x1<<0)

#define IISMOD (*(volatile unsigned int *)0x55000004)
#define IISMOD_SCLK_32FS (0x1<<0)
#define IISMOD_MCLK_384FS (0x1<<2)
#define IISMOD_MCLK_256FS (0x0<<2)
#define IISMOD_SERIAL_BIT_PER_CH_16 (0x1<<3)
#define IISMOD_MSB_FORMAT (0x1<<4)
#define IISMOD_LOW_FOR_LEFT_CH (~(0x0<<5))
#define IISMOD_TXMOD (0x2<<6)

#define DISRCC2 (*(volatile unsigned int *)0x4B0000C4)
#define DIDST2 (*(volatile unsigned int *)0x4B000088) 
#define DIDSTC2 (*(volatile unsigned int *)0x4B00008C)
#define DCON2 (*(volatile unsigned int *)0x4B000090)
#define DMASKTRIG2 (*(volatile unsigned int *)0x4B0000A0) 


#define IISFCON (*(volatile unsigned int *)0x5500000C)
#define IISFIFO (*(volatile unsigned int *)0x55000010)
#define IISPSR  (*(volatile unsigned int *)0x55000008)
//#define INTMSK (*volatile unsigned int *)0X4A000008)

#define IISFCON_TX_ENABLE (0x1<<13)
#define IISFCON_RX_FIFO_DMA (0x1<<14)
#define IISFCON_TX_FIFO_DMA (0x1<<14)

#define GPB_X_WRITE(x) (1<<x)
#define GPX_X_CLEAR(x) (~(1<<x))

#endif
