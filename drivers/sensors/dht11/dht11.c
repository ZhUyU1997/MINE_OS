#include <s3c24xx.h>
#include <timer.h>
/* 使用GPG5作用dht11的DATA引脚 */

/* 控制GPIO读取DHT11的数据
 * 1. 主机发出至少18MS的低脉冲: start信号
 * 2. start信号变为高, 20-40us之后, dht11会拉低总线维持80us
      然后拉高80us: 回应信号
 * 3. 之后就是数据, 逐位发送
 *    bit0 : 50us低脉冲, 26-28us高脉冲
 *    bit1 : 50us低脉冲, 70us高脉冲
 * 4. 数据有40bit: 8bit湿度整数数据+8bit湿度小数数据
                   +8bit温度整数数据+8bit温度小数数据
                   +8bit校验和
 */

static int dht11_wait_for_val(int val, int timeout_us);

/* 先实现GPIO的基本操作 */
static void dht11_data_cfg_as_output(void) {
	GPGCON &= ~(3 << 10);
	GPGCON |= (1 << 10);
}

static void dht11_data_cfg_as_input(void) {
	GPGCON &= ~(3 << 10);
}

static void dht11_data_set(int val) {
	if (val)
		GPGDAT |= (1 << 5);
	else
		GPGDAT &= ~(1 << 5);
}

static int dht11_data_get(void) {
	if (GPGDAT & (1 << 5))
		return 1;
	else
		return 0;
}


/* 再来实现DHT11的读操作 */

void dht11_init(void) {
	dht11_data_cfg_as_output();
	dht11_data_set(1);
	mdelay(2000);
}

static void dht11_start(void) {
	dht11_data_set(0);
	mdelay(20);
	dht11_data_cfg_as_input();
}

static int dht11_wait_ack(void) {
	udelay(60);
	return dht11_data_get();
}

static int dht11_recv_byte(void) {
	int i;
	int data = 0;

	for (i = 0; i < 8; i++) {
		if (dht11_wait_for_val(1, 1000)) {
			printf("dht11 wait for high data err!\n");
			return -1;
		}
		udelay(40);
		data <<= 1;
		if (dht11_data_get() == 1)
			data |= 1;

		if (dht11_wait_for_val(0, 1000)) {
			printf("dht11 wait for low data err!\n");
			return -1;
		}
	}

	return data;
}

static int dht11_wait_for_val(int val, int timeout_us) {
	while (timeout_us--) {
		if (dht11_data_get() == val)
			return 0; /* ok */
		udelay(1);
	}
	return -1; /* err */
}


int dht11_read(int *hum, int *temp) {
	unsigned char hum_m, hum_n;
	unsigned char temp_m, temp_n;
	unsigned char check;

	dht11_start();

	if (0 != dht11_wait_ack()) {
		printf("dht11 not ack, err!\n");
		return -1;
	}

	if (0 != dht11_wait_for_val(1, 1000)) { /* 等待ACK变为高电平, 超时时间是1000us */
		printf("dht11 wait for ack high err!\n");
		return -1;
	}

	if (0 != dht11_wait_for_val(0, 1000)) { /* 数据阶段: 等待低电平, 超时时间是1000us */
		printf("dht11 wait for data low err!\n");
		return -1;
	}

	hum_m  = dht11_recv_byte();
	hum_n  = dht11_recv_byte();
	temp_m = dht11_recv_byte();
	temp_n = dht11_recv_byte();
	check  = dht11_recv_byte();

	dht11_data_cfg_as_output();
	dht11_data_set(1);

	if (hum_m + hum_n + temp_m + temp_n == check) {
		*hum  = hum_m;
		*temp = temp_m;
		mdelay(2000);  /* 读取周期是2S, 不能读太频繁 */
		return 0;
	} else {
		printf("dht11 checksum err!\n");
		return -1;
	}

}


void dht11_test(void) {
	int hum, temp;

	dht11_init();

	while (!serial_getc_async()) {
		if (dht11_read(&hum, &temp) != 0) {
			printf("dht11 read err!\n");
			dht11_init();
		} else {
			printf("DHT11 : %d humidity, %d temperature\n", hum, temp);
		}
	}
}


