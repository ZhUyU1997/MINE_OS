#include <sys/types.h>
#include <s3c24xx.h>
#include <timer.h>
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
	lcd_set_background_color(btn->color);
	DrawFillRect(btn->x, btn->y, btn->w, btn->h, btn->color);
	lcd_putstr(max(btn->x + 10, btn->x + btn->w / 2 - strlen(btn->text) * 4), btn->y + btn->h / 2 - 4, btn->text, 0xff0000);
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
					&& (x <= (btns[i].x + btns[i].w))
					&& (y <= (btns[i].y + btns[i].h))) {
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

void ts_cal() {
	unsigned int fb_base;
	int xres, yres, bpp;

	int x, y, pressure;

	/* 获得LCD的参数: fb_base, xres, yres, bpp */
	get_lcd_params(&fb_base, &xres, &yres, &bpp);

	init_Ts();

	/* 清屏 */
	ClearScr(0);
	lcd_set_background_color(0);
	/* 显示文字提示较准 */
	lcd_putstr(70, 70, "Touch cross to calibrate touchscreen", 0xff0000);
	ts_calibrate();
#if 0
	/* 显示文字提示绘画 */
	lcd_putstr(70, yres - 70, "OK! To draw!", 0xffffff);

	while (1) {
		if (!ts_read(&x, &y, &pressure)) {
			printf(" x = %d, y = %d\n", x, y);

			if (pressure) {
				PutPixel(x, y, 0xff00);
			}
		}
	}
#endif
}
int ten_test() {
	unsigned int fb_base;
	int xres, yres, bpp;
	get_lcd_params(&fb_base, &xres, &yres, &bpp);
	ts_cal();
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 5; j++) {
			btns[i * 5 + j].x = 10 + (200 + 10) * i;
			btns[i * 5 + j].y = 10 + (yres / 7 + 10) * j;
			btns[i * 5 + j].w = 200;
			btns[i * 5 + j].h = yres / 7;
			btns[i * 5 + j].color = 0x888888;
			btns[i * 5 + j].key = i * 5 + j;
			btns[i * 5 + j].text = btn_text[i * 5 + j];
		}
	}
	while (1) {
		ClearScr(0xffffff);
		lcd_set_background_color(0x888888);
		for (int i = 0; i < 10; i++)
			button_create(&btns[i]);
		int key = get_button_key(btns, 10);
		char buf[10];
		sprintf(buf, "%d", key);
		lcd_set_background_color(0xffffff);
		lcd_putstr(10, yres - 20, buf, 0x0);
		switch (key) {
			case 0:
				led_test();
				break;
			case 1:
				button_test();
				break;
			case 2:
				break;
			case 3:
				break;
			case 4:
				break;
			case 5:
				break;
			case 6:
				break;
			case 7:
				break;
			case 8:
				break;
			case 9:
				break;
			default:
				break;
		}
	}
}
struct Button ret_btn = {
	.x = 10,
	.y = 10,
	.w = 70,
	.h = 30,
	.color = 0x888888,
	.key = 0,
	.text = "RETURN",
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
void led_test() {
	ClearScr(0xffffff);
	lcd_set_background_color(0x888888);
	button_create(&ret_btn);
	GPFCON &= ~((3 << 8) | (3 << 10) | (3 << 12));
	GPFCON |= ((1 << 8) | (1 << 10) | (1 << 12));
	for (int i = 0;; i = (i + 1) % 3) {
		led_ctl(i % 3, 1);
		led_ctl((i + 1) % 3, 0);
		led_ctl((i + 2) % 3, 0);
		udelay(1000 * 300);
		int key = get_button_key_asyn(&ret_btn, 1);
		if (key == 0) {
			led_ctl(0, 1);
			led_ctl(1, 1);
			led_ctl(2, 1);
			return;
		}
	}
}

void button_test() {
	ClearScr(0xffffff);
	lcd_set_background_color(0x888888);
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
	while (1) {
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
