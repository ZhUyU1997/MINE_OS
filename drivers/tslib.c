
static double a1, b1, c1;
static double a2, b2, c2;

static unsigned int fb_base;
static int xres, yres, bpp;

int get_lcd_x(int ts_x, int ts_y) {
	return a1 + b1 * ts_x + c1 * ts_y;
}

int get_lcd_y(int ts_x, int ts_y) {
	return a2 + b2 * ts_x + c2 * ts_y;
}
static double avg1(int *a, int len) {
	double  res = 0.0;
	for (int i = 0; i < len; i++) {
		res += a[i];
	}
	return res / len;
}
static double avg2(int *a, int len) {
	double  res = 0.0;
	for (int i = 0; i < len; i++) {
		res += a[i] * a[i];
	}
	return res / len;
}

static double L(int *a, int *b, int len) {
	double  res = 0.0;
	double avg_a = avg1(a, len);
	double avg_b = avg1(b, len);
	for (int i = 0; i < len; i++) {
		res += (a[i] - avg_a) * (b[i] - avg_b);
	}
	return res;
}

static void calc(int *x1, int *x2, int *y, int len, double *a0, double *a1, double *a2) {
	double temp = (L(x1, x1, len) * L(x2, x2, len) - L(x1, x2, len) * L(x1, x2, len));
	*a1 = (L(x1, y, len) * L(x2, x2, len) - L(x2, y, len) * L(x1, x2, len)) / temp;
	*a2 = (L(x2, y, len) * L(x1, x1, len) - L(x1, y, len) * L(x1, x2, len)) / temp;
	*a0 = avg1(y, len) - (*a1) * avg1(x1, len) - (*a2) * avg1(x2, len);
}

void get_calibrate_point_data(int lcd_x, int lcd_y, int *px, int *py) {
	int pressure;
	int x, y;
	int sum_x = 0, sum_y = 0;
	int cnt = 0;

	DispCross(lcd_x, lcd_y, 0xff0000);
	printf("lcd_x = %d, lcd_y = %d\n", lcd_x, lcd_y);
	/* 等待点击 */

	do {
		ts_read_raw(&x, &y, &pressure);
	} while (pressure == 0);


	do {
		if (cnt < 128) {
			sum_x += x;
			sum_y += y;
			cnt++;
		}
		ts_read_raw(&x, &y, &pressure);
		printf("x = %08d, y = %08d, cnt = %d\n", x, y, cnt);
	} while (pressure);

	*px = sum_x / cnt;
	*py = sum_y / cnt;

	printf("return raw data: x = %08d, y = %08d\n", *px, *py);

	/* 直到松开才返回 */
	DispCross(lcd_x, lcd_y, 0xd4d4d4);
}

void ts_calibrate(void) {
#define POINT_NUM 5
	get_lcd_params(&fb_base, &xres, &yres, &bpp);
	int ts_x[POINT_NUM], ts_y[POINT_NUM];
	int lcd_x[POINT_NUM] = {50,	xres - 50,	xres - 50,	50,			xres / 2};
	int lcd_y[POINT_NUM] = {50,	50,			yres - 50,	yres - 50,	yres / 2};


	/* 获得LCD的参数: fb_base, xres, yres, bpp */

	printf("xres = %d, yres = %d\n", xres, yres);

	for (int i = 0; i < POINT_NUM; i++) {
		get_calibrate_point_data(lcd_x[i], lcd_y[i], &ts_x[i], &ts_y[i]);
	}

	calc(ts_x, ts_y, lcd_x, POINT_NUM, &a1, &b1, &c1);
	calc(ts_x, ts_y, lcd_y, POINT_NUM, &a2, &b2, &c2);
	
#define F(x) (int)(x), (int)(((int)((x)*1000) - ((int)(x))*1000))
	printf("a1 = %d.%d, b1 = %d.%d, c1 = %d.%d\n", F(a1), F(b1), F(c1));
	printf("a2 = %d.%d, b2 = %d.%d, c2 = %d.%d\n", F(a2), F(b2), F(c2));
}

/*
 * 读TS原始数据, 转换为LCD坐标
 */
int ts_read(int *lcd_x, int *lcd_y, int *lcd_pressure) {
	int ts_x, ts_y, ts_pressure;
	int tmp_x, tmp_y;

	ts_read_raw(&ts_x, &ts_y, &ts_pressure);

	/* 使用公式计算 */
	tmp_x = get_lcd_x(ts_x, ts_y);
	tmp_y = get_lcd_y(ts_x, ts_y);
	//printf("x = %d,y = %d\n", tmp_x, tmp_y);
	if (tmp_x < 0 || tmp_x >= xres || tmp_y < 0 || tmp_y >= yres)
		return 0;

	*lcd_x = tmp_x;
	*lcd_y = tmp_y;
	*lcd_pressure = ts_pressure;
	return 0;
}

int ts_read_asyn(int *lcd_x, int *lcd_y, int *lcd_pressure) {
	int ts_x, ts_y, ts_pressure;
	int tmp_x, tmp_y;

	if (ts_read_raw_asyn(&ts_x, &ts_y, &ts_pressure))
		return -1;

	/* 使用公式计算 */
	tmp_x = get_lcd_x(ts_x, ts_y);
	tmp_y = get_lcd_y(ts_x, ts_y);

	if (tmp_x < 0 || tmp_x >= xres || tmp_y < 0 || tmp_y >= yres)
		return -1;

	*lcd_x = tmp_x;
	*lcd_y = tmp_y;
	*lcd_pressure = ts_pressure;
	return 0;
}

void get_calibrate_params(double *pa){
	pa[0] = a1;
	pa[1] = b1;
	pa[2] = c1;
	pa[3] = a2;
	pa[4] = b2;
	pa[5] = c2;
}

void set_calibrate_params(double *pa){
	get_lcd_params(&fb_base, &xres, &yres, &bpp);
	a1 = pa[0];
	b1 = pa[1];
	c1 = pa[2];
	a2 = pa[3];
	b2 = pa[4];
	c2 = pa[5];
}
