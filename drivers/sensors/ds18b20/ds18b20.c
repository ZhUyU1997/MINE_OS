#include <s3c24xx.h>
#include <timer.h>
/* 使用GPG6作用ds18b20的DATA引脚 */


/* rom commands */
#define SEARCH_ROM    0xF0
#define READ_ROM      0x33
#define MATCH_ROM     0x55
#define SKIP_ROM      0xCC
#define ALARM_ROM     0xEC

/* functions commands */
#define CONVERT_TEAMPERATURE 0x44
#define WRITE_SCRATCHPAD     0x4E
#define READ_SCRATCHPAD      0xBE
#define COPY_SCRATCHPAD      0x48
#define RECALL_EEPROM        0xB8
#define READ_POWER_SUPPLY    0xB4


/* 先实现GPIO的基本操作 */
static void ds18b20_data_cfg_as_output(void) {
	GPGCON &= ~(3 << 12);
	GPGCON |= (1 << 12);
}

static void ds18b20_data_cfg_as_input(void) {
	GPGCON &= ~(3 << 12);
}

static void ds18b20_data_set(int val) {
	if (val)
		GPGDAT |= (1 << 6);
	else
		GPGDAT &= ~(1 << 6);
}

static int ds18b20_data_get(void) {
	if (GPGDAT & (1 << 6))
		return 1;
	else
		return 0;
}

static void ds18b20_data_set_val_for_time(int val, int us) {
	ds18b20_data_cfg_as_output();
	ds18b20_data_set(val);
	udelay(us);
}

static void ds18b20_data_release(void) {
	ds18b20_data_cfg_as_input();
}

/* ds18b20的代码 */
static int ds18b20_initialization(void) {
	int val;

	ds18b20_data_set_val_for_time(0, 500);
	ds18b20_data_release();
	udelay(80);

	val = ds18b20_data_get();
	udelay(250);
	return val;
}

static void ds18b20_write_bit(int val) {
	if (0 == val) {
		ds18b20_data_set_val_for_time(0, 60);
		ds18b20_data_release();
		udelay(2);
	} else {
		ds18b20_data_set_val_for_time(0, 2);
		ds18b20_data_release();
		udelay(60);
	}
}

static int ds18b20_read_bit(void) {
	int val;

	ds18b20_data_set_val_for_time(0, 2);
	ds18b20_data_release();
	udelay(10);
	val = ds18b20_data_get();
	udelay(50);
	return val;
}

static void ds18b20_write_byte(unsigned char data) {
	int i;
	for (i = 0; i < 8; i++) {

		ds18b20_write_bit(data & (1 << i));
	}
}

static unsigned char ds18b20_read_byte(void) {
	int i;
	unsigned char data = 0;

	for (i = 0; i < 8; i++) {
		if (ds18b20_read_bit() == 1)
			data |= (1 << i);
	}

	return data;
}

static void ds18b20_write_rom_cmd(unsigned char cmd) {
	ds18b20_write_byte(cmd);
}

static void ds18b20_write_function_cmd(unsigned char cmd) {
	ds18b20_write_byte(cmd);
}

/* 实际操作函数 */
int ds18b20_read_rom(unsigned char rom[]) {
	int i;

	if (ds18b20_initialization() != 0) {
		printf("ds18b20_initialization err!\n");
		return -1;
	}

	ds18b20_write_rom_cmd(READ_ROM);

	for (i = 0; i < 8; i++) {
		rom[i] = ds18b20_read_byte();
	}

	return 0;
}

int ds18b20_wait_when_processing(int timeout_us) {
	while (timeout_us--) {
		if (ds18b20_read_bit() == 1)
			return 0;  /* ok */
		udelay(1);
	}
	return -1;
}

int ds18b20_start_convert(void) {
	if (ds18b20_initialization() != 0) {
		printf("ds18b20_initialization err!\n");
		return -1;
	}

	ds18b20_write_rom_cmd(SKIP_ROM);
	ds18b20_write_function_cmd(CONVERT_TEAMPERATURE);

	/* 等待/判断转换成功 */
	if (0 != ds18b20_wait_when_processing(1000000)) {
		printf("ds18b20_wait_when_processing err!\n");
		return -1;
	}

	return 0;
}

int ds18b20_read_ram(unsigned char ram[]) {
	int i;

	if (ds18b20_initialization() != 0) {
		printf("ds18b20_initialization err!\n");
		return -1;
	}

	ds18b20_write_rom_cmd(SKIP_ROM);
	ds18b20_write_function_cmd(READ_SCRATCHPAD);

	for (i = 0; i < 9; i++) {
		ram[i] = ds18b20_read_byte();
	}

	return 0;
}


int ds18b20_read_temperature(double *temp) {
	int err;
	unsigned char ram[9];
	double val[] = {0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64};
	double sum = 0;
	int i;

	err = ds18b20_start_convert();
	if (err)
		return err;

	err = ds18b20_read_ram(ram);
	if (err)
		return err;

	/* 计算温度 */

	/* 先判断精度 */
	if (ram[4] & (3 << 5) == 0) /* 精度: 9bit */
		i = 3;
	else if (ram[4] & (3 << 5) == (1 << 5)) /* 精度: 10bit */
		i = 2;
	else if (ram[4] & (3 << 5) == (2 << 5)) /* 精度: 11bit */
		i = 1;
	else
		/* 精度是 12 bit */
		i = 0;

	for (; i < 8; i++) {
		if (ram[0] & (1 << i))
			sum += val[i];
	}

	for (i = 0; i < 3; i++) {
		if (ram[1] & (1 << i))
			sum += val[8 + i];
	}

	if (ram[1] & (1 << 3))
		sum = 0 - sum;

	*temp = sum;
	return 0;
}

void ds18b20_init_state(void) {
	ds18b20_data_release();
}

void ds18b20_test(void) {
	unsigned char rom[8];
	int i;
	double temp;
	int m, n;

	ds18b20_init_state();

	//while (1)
	{
		if (ds18b20_read_rom(rom) == 0) {
			printf("ds18b20 rom: ");
			for (i = 0; i < 8; i++) {
				printf("%02x ", rom[i]);
			}
			printf("\n");
		}
	}

	while (!serial_getc_async()) {
		if (0 == ds18b20_read_temperature(&temp)) {
			m = (int)temp;	/* 3.01, m = 3 */
			temp = temp - m;	/* 小数部分: 0.01 */
			n = temp * 10000;  /* 10 */

			/* 在串口上打印 */
			printf("ds18b20 temperature: %d.%04d\n", m, n);  /* 3.010v */
		}
	}
}

