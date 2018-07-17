#include <s3c24xx.h>
#include <timer.h>
#include <interrupt.h>
#include "i2c_controller.h"

static p_i2c_msg p_cur_msg;

int isLastData(void) {
	if (p_cur_msg->cnt_transferred == p_cur_msg->len - 1)
		return 1;  /* 正要开始传输最后一个数据 */
	else
		return 0;
}

void resume_iic_with_ack(void) {
	unsigned int iiccon = IICCON;
	iiccon |= (1 << 7); /* 回应ACK */
	iiccon &= ~(1 << 4); /* 恢复IIC操作 */
	IICCON =  iiccon;
}

void resume_iic_without_ack(void) {
	unsigned int iiccon = IICCON;
	iiccon &= ~((1 << 7) | (1 << 4)); /* 不回应ACK, 恢复IIC操作 */
	IICCON =  iiccon;
}


void i2c_interrupt_func(int irq) {
	int index;
	unsigned int iicstat = IICSTAT;
	unsigned int iiccon;

	//printf("i2c_int %s\n", p_cur_msg->flags ? "read" : "write");

	p_cur_msg->cnt_transferred++;

	/* 每传输完一个数据将产生一个中断 */

	/* 对于每次传输, 第1个中断是"已经发出了设备地址" */

	if (p_cur_msg->flags == 0) {	/* write */
		/* 对于第1个中断, 它是发送出设备地址后产生的
		 * 需要判断是否有ACK
		 * 有ACK : 设备存在
		 * 无ACK : 无设备, 出错, 直接结束传输
		 */
		if (p_cur_msg->cnt_transferred == 0) { /* 第1次中断 */
			if (iicstat & (1 << 0)) {
				/* no ack */
				/* 停止传输 */
				IICSTAT = 0xd0;
				IICCON &= ~(1 << 4);
				p_cur_msg->err = -1;
				printf("tx err, no ack\n");
				return;
			}
		}

		if (p_cur_msg->cnt_transferred < p_cur_msg->len) {
			/* 对于其他中断, 要继续发送下一个数据
			 */
			//printf("data = %c\n", p_cur_msg->buf[p_cur_msg->cnt_transferred]);
			IICDS = p_cur_msg->buf[p_cur_msg->cnt_transferred];
			IICCON &= ~(1 << 4);
		} else {
			/* 停止传输 */
			IICSTAT = 0xd0;
			IICCON &= ~(1 << 4);
		}
	} else { /* read */
		/* 对于第1个中断, 它是发送出设备地址后产生的
		 * 需要判断是否有ACK
		 * 有ACK : 设备存在, 恢复I2C传输, 这样在下一个中断才可以得到第1个数据
		 * 无ACK : 无设备, 出错, 直接结束传输
		 */
		if (p_cur_msg->cnt_transferred == 0) { /* 第1次中断 */
			if (iicstat & (1 << 0)) {
				/* no ack */
				/* 停止传输 */
				IICSTAT = 0x90;
				IICCON &= ~(1 << 4);
				p_cur_msg->err = -1;
				printf("rx err, no ack\n\r");
				return;
			} else { /* ack */
				/* 如果是最后一个数据, 启动传输时要设置为不回应ACK */
				/* 恢复I2C传输 */
				if (isLastData()) {
					resume_iic_without_ack();
				} else {
					resume_iic_with_ack();
				}
				return;
			}
		}

		/* 非第1个中断, 表示得到了一个新数据
		 * 从IICDS读出、保存
		 */
		if (p_cur_msg->cnt_transferred < p_cur_msg->len) {
			index = p_cur_msg->cnt_transferred - 1;
			p_cur_msg->buf[index] = IICDS;

			/* 如果是最后一个数据, 启动传输时要设置为不回应ACK */
			/* 恢复I2C传输 */
			if (isLastData()) {
				resume_iic_without_ack();
			} else {
				resume_iic_with_ack();
			}
		} else {
			/* 发出停止信号 */
			IICSTAT = 0x90;
			IICCON &= ~(1 << 4);
		}
	}
}


void s3c2440_i2c_con_init(void) {
	/* 配置引脚用于I2C*/
	GPECON &= ~((3 << 28) | (3 << 30));
	GPECON |= ((2 << 28) | (2 << 30));

	/* 设置时钟 */
	/* [7] : IIC-bus acknowledge enable bit, 1-enable in rx mode
	 * [6] : 时钟源, 0: IICCLK = fPCLK /16; 1: IICCLK = fPCLK /512
	 * [5] : 1-enable interrupt
	 * [4] : 读出为1时表示中断发生了, 写入0来清除并恢复I2C操作
	 * [3:0] : Tx clock = IICCLK/(IICCON[3:0]+1).
	 * Tx Clock = 100khz = 50Mhz/16/(IICCON[3:0]+1)
	 */
	IICCON = (1 << 7) | (0 << 6) | (1 << 5) | (30 << 0);

	/* 注册中断处理函数 */
	request_irq(INT_IIC, i2c_interrupt_func);
}

int do_master_tx(p_i2c_msg msg) {
	p_cur_msg = msg;

	msg->cnt_transferred = -1;
	msg->err = 0;

	/* 设置寄存器启动传输 */
	/* 1. 配置为 master tx mode */
	IICCON |= (1 << 7); /* TX mode, 在ACK周期释放SDA */
	IICSTAT = (1 << 4);

	/* 2. 把从设备地址写入IICDS */
	IICDS = msg->addr << 1;

	/* 3. IICSTAT = 0xf0 , 数据即被发送出去, 将导致中断产生 */
	IICSTAT = 0xf0;


	/* 后续的传输由中断驱动 */

	/* 循环等待中断处理完毕 */
	while (!msg->err && msg->cnt_transferred != msg->len);
	udelay(1000);
	if (msg->err)
		return -1;
	else
		return 0;
}

int do_master_rx(p_i2c_msg msg) {
	p_cur_msg = msg;

	msg->cnt_transferred = -1;
	msg->err = 0;

	/* 设置寄存器启动传输 */
	/* 1. 配置为 Master Rx mode */
	IICCON |= (1 << 7); /* RX mode, 在ACK周期回应ACK */
	IICSTAT = (1 << 4);

	/* 2. 把从设备地址写入IICDS */
	IICDS = (msg->addr << 1) | (1 << 0);

	/* 3. IICSTAT = 0xb0 , 从设备地址即被发送出去, 将导致中断产生 */
	IICSTAT = 0xb0;


	/* 后续的传输由中断驱动 */

	/* 循环等待中断处理完毕 */
	while (!msg->err && msg->cnt_transferred != msg->len);
	udelay(1000);
	if (msg->err)
		return -1;
	else
		return 0;
}

int s3c2440_master_xfer(p_i2c_msg msgs, int num) {
	int i;
	int err;

	for (i = 0; i < num; i++) {
		if (msgs[i].flags == 0)/* write */
			err = do_master_tx(&msgs[i]);
		else
			err = do_master_rx(&msgs[i]);
		if (err)
			return err;
	}
	return 0;
}

/* 实现i2c_controller
          .init
          .master_xfer
          .name
 */

static i2c_controller s3c2440_i2c_con = {
	.name = "s3c2440",
	.init = s3c2440_i2c_con_init,
	.master_xfer = s3c2440_master_xfer,
};

void s3c2440_i2c_con_add(void) {
	register_i2c_controller(&s3c2440_i2c_con);
}

