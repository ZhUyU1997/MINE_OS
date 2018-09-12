#include <sys/types.h>
#include <s3c24xx.h>
#include <timer.h>
#include "ff.h"

struct Button {
	int x;
	int y;
	int w;
	int h;
	U32 color;
	char *text;
	int key;
};

struct Button btns[10];
char btn_text[10][10] = {
	"LED",
	"BUTTON",
	"LCD",
	"TS",
	"I2C",
	"SPI",
	"Res",
	"PHT11",
	"DS18",
	"HS0038",
};
#define min(x,y) (((x)>(y))?(y):(x))
#define max(x,y) (((x)>(y))?(x):(y))
void button_create(struct Button *btn) {
	//lcd_set_background_color(btn->color);
	//DrawFillRect(btn->x, btn->y, btn->w, btn->h, btn->color);
	//lcd_putstr(max(btn->x + 10, btn->x + btn->w / 2 - strlen(btn->text) * 4), btn->y + btn->h / 2 - 4, btn->text, 0xff0000);
}

int get_button_key(struct Button *btns, int count) {
	int x, y, pressure;
	while (1) {
		if (ts_read(&x, &y, &pressure))
			continue;
		if (!pressure)
			continue;
		for (int i = 0; i < count; i++) {
			if ((x >= btns[i].x)
					&& (y >= btns[i].y)
					&& (x < (btns[i].x + btns[i].w))
					&& (y < (btns[i].y + btns[i].h))) {
				return btns[i].key;
			}
		}
	}
}

int get_button_key_asyn(struct Button *btns, int count) {
	int x, y, pressure;
	if (ts_read_asyn(&x, &y, &pressure))
		return -1;
	if (!pressure)
		return -1;
	for (int i = 0; i < count; i++) {
		if ((x >= btns[i].x)
				&& (y >= btns[i].y)
				&& (x <= (btns[i].x + btns[i].w))
				&& (y <= (btns[i].y + btns[i].h))) {
			return btns[i].key;
		}
	}
	return -1;
}

int get_button_key_timeout(struct Button *btns, int count, int timeout) {
	int x, y, pressure;
	long long pre = get_system_time_us();
	while((get_system_time_us() - pre) < timeout * 1000){
		if (ts_read_asyn(&x, &y, &pressure))
			continue;
		if (!pressure)
			return -1;
		for (int i = 0; i < count; i++) {
			if ((x >= btns[i].x)
					&& (y >= btns[i].y)
					&& (x <= (btns[i].x + btns[i].w))
					&& (y <= (btns[i].y + btns[i].h))) {
				return btns[i].key;
			}
		}
	}
	return -1;
}

int get_button_key_by_xy(int x, int y, struct Button *btns, int count) {
	for (int i = 0; i < count; i++) {
		if ((x >= btns[i].x)
				&& (y >= btns[i].y)
				&& (x <= (btns[i].x + btns[i].w))
				&& (y <= (btns[i].y + btns[i].h))) {
			return btns[i].key;
		}
	}
	return -1;
}

void ts_cal() {
	double a[6];
	FIL f;
	FRESULT res;
	UINT bw;

	init_Ts();
	drawImage("00.bmp");
	ts_calibrate();
	get_calibrate_params(a);
	printf("step 0\n");
	if ((res = f_open(&f, "cal", FA_WRITE | FA_CREATE_ALWAYS)) != FR_OK) {
		printf("再见1, res = %d\n", res);
		return;
	}
	printf("step 1\n");
	res = f_write(&f, a, sizeof(a), &bw);
	if (( res!= FR_OK) || bw != sizeof(a)) {
		printf("再见2, res = %d\n", res);
		f_close(&f);
		return;
	}
	f_close(&f);
}

int ten_test_view() {
	unsigned int fb_base;
	int xres, yres, bpp;
	double a[6];
	FIL f;
	FRESULT res;
	UINT br;
	init_Ts();
	get_lcd_params(&fb_base, &xres, &yres, &bpp);
	if ((res = f_open(&f, "cal", FA_READ)) != FR_OK) {
		printf("cal文件打开失败, res = %d\n", res);
		ts_cal();
	}else{
		res = f_read(&f, a, sizeof(a), &br);
		if (( res!= FR_OK) || br != sizeof(a)) {
			printf("文件读取失败, res = %d\n", res);
			f_close(&f);
			ts_cal();
		}else{
			f_close(&f);
			set_calibrate_params(a);
		}
	}
	int x[10] = {18,110,205,299,394,18,110,205,299,394};
	int y[10] = {70,70,70,70,70,157,157,157,157,157};
	for (int i = 0; i < 10; i++) {
		btns[i].x = x[i];
		btns[i].y = y[i];
		btns[i].w = 72;
		btns[i].h = 73;
		btns[i].color = 0x888888;
		btns[i].key = i;
		btns[i].text = btn_text[i];
	}


	while (1) {
		drawImage("xx01.bmp");
		set_text_color(0x888888);
		for (int i = 0; i < 10; i++)
			button_create(&btns[i]);
		int key = get_button_key(btns, 10);
		switch (key) {
			case 0:
				led_test_view();
				break;
			case 1:
				button_test_view();
				break;
			case 2:
				lcd_test_view();
				break;
			case 3:
				ts_test_view();
				break;
			case 4:
				iic_test_view();
				break;
			case 5:
				spi_test_view();
				break;
			case 6:
				photoresistor_test_view();
				break;
			case 7:
				dht11_test_view();
				break;
			case 8:
				ds18b20_test_view();
				break;
			case 9:
				irda_test_view();
				break;
			default:
				break;
		}
	}
}

struct Button ret_btn = {
	.x = 16,
	.y = 16,
	.w = 45,
	.h = 45,
	.color = 0x888888,
	.key = 0,
	.text = "返回",
};

struct Button led_btn[4] = {
	[0] =
	{
		.x = 16,
		.y = 16,
		.w = 45,
		.h = 45,
		.color = 0x888888,
		.key = 0,
		.text = "左",
	},
	[1] =
	{
		.x = 140,
		.y = 118,
		.w = 60,
		.h = 37,
		.key = 1,
		.text = "左",
	},
	[2] =
	{
		.x = 210,
		.y = 118,
		.w = 60,
		.h = 37,
		.key = 2,
		.text = "右",
	},
	[3] =
	{
		.x = 280,
		.y = 118,
		.w = 60,
		.h = 37,
		.color = 0x888888,
		.key = 3,
		.text = "闪",
	},
};
void led_ctl(int n, int s) {
	switch (n) {
		case 0:
		case 1:
		case 2:
			if (!s)
				GPFDAT |= (1 << (4 + n));
			else
				GPFDAT &= ~(1 << (4 + n));
			break;
		default:
			break;
	}
}
void led_test_view() {
	int status = 0;
	int i = 0 ,j = 0;
	drawImage("01.bmp");
	button_create(&led_btn);
	GPFCON &= ~((3 << 8) | (3 << 10) | (3 << 12));
	GPFCON |= ((1 << 8) | (1 << 10) | (1 << 12));
	for (;!serial_getc_async();) {
		if(status < 2){
			led_ctl(i % 3, 1);
			led_ctl((i + 1) % 3, 0);
			led_ctl((i + 2) % 3, 0);
			udelay(1000 * 300);
		}else{
			j++;
			led_ctl(0, j%2);
			led_ctl(1, j%2);
			led_ctl(2, j%2);
			udelay(1000 * 30);
		}
		
		int key = get_button_key_asyn(led_btn, 4);
		switch (key) {
			case 0:
				led_ctl(0, 1);
				led_ctl(1, 1);
				led_ctl(2, 1);
				return;
			case 1:
				status = 0;
				break;
			case 2:
				status = 1;
				break;
			case 3:
				status = 2;
				break;
			default:
				break;
		}
		if(status < 2)
			i = (i + 1 + status) % 3;
	}
}
void button_test_view() {
	drawImage("02.bmp");
	button_create(&ret_btn);
	/* 设置GPFCON让GPF4/5/6配置为输出引脚 */
	GPFCON &= ~((3 << 8) | (3 << 10) | (3 << 12));
	GPFCON |= ((1 << 8) | (1 << 10) | (1 << 12));

	/* 配置3个按键引脚为输入引脚:
	 * GPF0(S2),GPF2(S3),GPG3(S4)
	 */
	GPFCON &= ~((3 << 0) | (3 << 4)); /* gpf0,2 */
	GPGCON &= ~((3 << 6)); /* gpg3 */
	/* 循环点亮 */
	while (!serial_getc_async()) {
		int val1 = GPFDAT;
		int val2 = GPGDAT;
		led_ctl(2, !(val1 & (1 << 0)));
		led_ctl(1, !(val1 & (1 << 2)));
		led_ctl(0, !(val2 & (1 << 3)));
		int key = get_button_key_asyn(&ret_btn, 1);
		if (key == 0) {
			led_ctl(0, 1);
			led_ctl(1, 1);
			led_ctl(2, 1);
			return;
		}
	}
}
void lcd_test_view(void) {
	unsigned int fb_base;
	int xres, yres, bpp;
	int x, y, pressure;
	/* 获得LCD的参数: fb_base, xres, yres, bpp */
	get_lcd_params(&fb_base, &xres, &yres, &bpp);

	ClearScr(0xff0000);
	mdelay(1000);
	ClearScr(0x00ff00);
	mdelay(1000);
	ClearScr(0x0000ff);
	mdelay(1000);
	ClearScr(0x000000);
	/* 画线 */
	DrawLine(0, 0, xres - 1, 0, 0x23ff77);
	DrawLine(xres - 1, 0, xres - 1, yres - 1, 0xffff);
	DrawLine(0, yres - 1, xres - 1, yres - 1, 0xff00aa);
	DrawLine(0, 0, 0, yres - 1, 0xff00ef);
	DrawLine(0, 0, xres - 1, yres - 1, 0xff45);
	DrawLine(xres - 1, 0, 0, yres - 1, 0xff0780);

	mdelay(1000);

	/* 画圆 */
	DrawCircle(xres/2, yres/2, yres/4, 0xff);

	/* 输出文字 */
	set_text_color(0xff,0x0);
	lcd_putstr(10, 10, "www.100ask.net\n\r100ask.taobao.com");
	ts_read_asyn(&x, &y, &pressure);
	mdelay(3000);
}
void ts_test_view() {
	int x, y, pressure;

	ts_cal();
	drawImage("04.bmp");
	while (!serial_getc_async()) {
		if (!ts_read(&x, &y, &pressure)) {
			int key = get_button_key_by_xy(x, y, &ret_btn, 1);
			if (key == 0)
				return;
			if (pressure) {
				PutPixel(x, y, 0xf800);
			}
		}
	}
}
void iic_test_view() {
	unsigned char data[128];
	char buf[100];
	drawImage("05.bmp");
	set_text_color(0x0,0x5677fc);
	struct Button iic_btn[129];
	iic_btn[0] = ret_btn;
	for(int i=0;i<8;i++){
		for(int j=0;j<16;j++){
			iic_btn[i*16+j+1].x = 16+28*j;
			iic_btn[i*16+j+1].y = 76+23*i;
			iic_btn[i*16+j+1].w = 28;
			iic_btn[i*16+j+1].h = 23;
			iic_btn[i*16+j+1].key = i*16+j+1;
		}
	}
	i2c_init();
	while (!serial_getc_async()) {
		int err = at24cxx_read(0, data, 128);
		if (err){
			printf("[error]\n");
			return;
		}
		for(int i=0;i<8;i++){
			for(int j=0;j<16;j++){
				sprintf(buf, "%02X", data[i*16+j]&0xff);
				lcd_putstr(iic_btn[i*16+j+1].x+5,iic_btn[i*16+j+1].y+3,buf);
			}
		}
		int key = get_button_key(iic_btn, 129);
		if(key==0)
			return;
		else if(key>=1&&key<=128){
			data[key-1]++;
		}
		err = at24cxx_write(0, data, 128);
		if (err){
			printf("[error]\n");
			return;
		}
		
	}
}
void spi_test_view(void) {
	int x, y, pressure;
	drawImage("06.bmp");
	SPIInit();
	OLEDInit();
	set_text_color(0x888888,0xffffff);
	char buf[8][16][8]={0};
	while (!serial_getc_async()) {
		if (!ts_read(&x, &y, &pressure)) {
			int key = get_button_key_by_xy(x, y, &ret_btn, 1);
			if (key == 0)
				return;
			if (pressure && x >=176 && y >= 104 && x <= 303 && y <= 167) {
				PutPixel(x, y, 0x001f);
				int j=x-176;
				int i=y-104;
				buf[i/8][j/8][j%8] |= 1<<i%8;
				OLEDPutImage(buf);
			}
		}
	}
}
void photoresistor_test_view(void) {
	int m; /* 整数部分 */
	int n; /* 小数部分 */
	char buf[100];

	drawImage("07.bmp");
	set_text_color(0x888888,0xffffff);
	while (!serial_getc_async()) {
		photoresistor_get_vol(&m, &n, 1);
		sprintf(buf, "%d.%03d    ", m, n);
		lcd_putstr(247,130,buf);
		init_Ts();
		int key = get_button_key_timeout(&ret_btn, 1, 100);
		if (key == 0)
			return;
	}
}
void dht11_test_view(void) {
	int hum, temp;
	char buf[100];
	drawImage("08.bmp");
	set_text_color(0x888888,0xffffff);
	dht11_init();
	while (!serial_getc_async()) {
		if (dht11_read(&hum, &temp)) {
			dht11_init();
		} else {
			sprintf(buf, "%d   ", temp);
			lcd_putstr(247,125,buf);
			sprintf(buf, "%d   ", hum);
			lcd_putstr(247,175,buf);
		}
		int key = get_button_key_timeout(&ret_btn, 1, 100);
		if (key == 0)
			return;
	}
}
void ds18b20_test_view(void) {
	int m; /* 整数部分 */
	int n; /* 小数部分 */
	char buf[100];
	double temp;
	drawImage("09.bmp");
	set_text_color(0x888888,0xffffff);
	ds18b20_init_state();
	while (!serial_getc_async()) {
		if (!ds18b20_read_temperature(&temp)) {
			m = (int)temp;	/* 3.01, m = 3 */
			temp = temp - m;	/* 小数部分: 0.01 */
			n = temp * 10000;  /* 10 */

			/* 在串口上打印 */
			sprintf(buf, "%d.%04d    ", m, n);  /* 3.010v */
			lcd_putstr(247,130,buf);
		}
		int key = get_button_key_timeout(&ret_btn, 1, 100);
		if (key == 0)
			return;
	}
}
void irda_test_view(void) {
	int address, data;
	char buf[100];
	drawImage("10.bmp");
	set_text_color(0x888888,0xffffff);
	irda_init();
	while (!serial_getc_async()) {
		if (irda_nec_read(&address, &data)==0) {
			sprintf(buf, "%d         ", address);
			lcd_putstr(247,125,buf);
			sprintf(buf, "%d         ", data);
			lcd_putstr(247,175,buf);
		}else if(irda_nec_read(&address, &data)==1){
			sprintf(buf, "%d repeat  ", address);
			lcd_putstr(247,125,buf);
			sprintf(buf, "%d repeat  ", data);
			lcd_putstr(247,175,buf);
		}
		int key = get_button_key_asyn(&ret_btn, 1);
		if (key == 0)
			break;
	}
	irda_exit();
}
