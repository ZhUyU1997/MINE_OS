
static double g_kx;
static double g_ky;

static int g_ts_xc, g_ts_yc;
static int g_lcd_xc, g_lcd_yc;
static int g_ts_xy_swap = 0;


static unsigned int fb_base;
static int xres, yres, bpp;

int get_lcd_x_frm_ts_x(int ts_x) {
	return g_kx * (ts_x - g_ts_xc) + g_lcd_xc;
}

int get_lcd_y_frm_ts_y(int ts_y) {
	return g_ky * (ts_y - g_ts_yc) + g_lcd_yc;
}


void get_calibrate_point_data(int lcd_x, int lcd_y, int *px, int *py) {
	int pressure;
	int x, y;
	int sum_x = 0, sum_y = 0;
	int cnt = 0;

	DispCross(lcd_x, lcd_y, 0xffffff);
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
	DispCross(lcd_x, lcd_y, 0);
}


int is_ts_xy_swap(int a_ts_x, int a_ts_y, int b_ts_x, int b_ts_y) {
	int dx = b_ts_x - a_ts_x;
	int dy = b_ts_y - a_ts_y;

	if (dx < 0)
		dx = 0 - dx;
	if (dy < 0)
		dy = 0 - dy;

	if (dx > dy)
		return 0; /* xy没有反转 */
	else
		return 1; /* xy反了 */
}

void swap_xy(int *px, int *py) {
	int tmp = *px;
	*px = *py;
	*py = tmp;
}

/*
----------------------------
|                          |
|  +(A)              (B)+  |
|                          |
|                          |
|                          |
|            +(E)          |
|                          |
|                          |
|                          |
|  +(D)              (C)+  |
|                          |
----------------------------

*/

void ts_calibrate(void) {

	int a_ts_x, a_ts_y;
	int b_ts_x, b_ts_y;
	int c_ts_x, c_ts_y;
	int d_ts_x, d_ts_y;
	int e_ts_x, e_ts_y;

	/* X轴方向 */
	int ts_s1, ts_s2;
	int lcd_s;

	/* Y轴方向 */
	int ts_d1, ts_d2;
	int lcd_d;

	/* 获得LCD的参数: fb_base, xres, yres, bpp */
	get_lcd_params(&fb_base, &xres, &yres, &bpp);
	printf("xres = %d, yres = %d\n", xres, yres);
	/* 对于ABCDE, 循环: 显示"+"、点击、读ts原始值 */
	/* A(50, 50) */
	get_calibrate_point_data(50, 50, &a_ts_x, &a_ts_y);

	/* B(xres-50, 50) */
	get_calibrate_point_data(xres - 50, 50, &b_ts_x, &b_ts_y);

	/* C(xres-50, yres-50) */
	get_calibrate_point_data(xres - 50, yres - 50, &c_ts_x, &c_ts_y);

	/* D(50, yres-50) */
	get_calibrate_point_data(50, yres - 50, &d_ts_x, &d_ts_y);

	/* E(xres/2, yres/2) */
	get_calibrate_point_data(xres / 2, yres / 2, &e_ts_x, &e_ts_y);

	/* 确定触摸屏数据XY是否反转 */
	g_ts_xy_swap = is_ts_xy_swap(a_ts_x, a_ts_y, b_ts_x, b_ts_y);

	if (g_ts_xy_swap) {
		/* 对调所有点的XY坐标 */
		swap_xy(&a_ts_x, &a_ts_y);
		swap_xy(&b_ts_x, &b_ts_y);
		swap_xy(&c_ts_x, &c_ts_y);
		swap_xy(&d_ts_x, &d_ts_y);
		swap_xy(&e_ts_x, &e_ts_y);
	}

	/* 确定公式的参数并保存 */
	ts_s1 = b_ts_x - a_ts_x;
	ts_s2 = c_ts_x - d_ts_x;
	lcd_s = xres - 50 - 50;

	ts_d1 = d_ts_y - a_ts_y;
	ts_d2 = c_ts_y - b_ts_y;
	lcd_d = yres - 50 - 50;

	g_kx = ((double)(2 * lcd_s)) / (ts_s1 + ts_s2);
	g_ky = ((double)(2 * lcd_d)) / (ts_d1 + ts_d2);

	g_ts_xc = e_ts_x;
	g_ts_yc = e_ts_y;

	g_lcd_xc = xres / 2;
	g_lcd_yc = yres / 2;

	printf("A lcd_x = %08d, lcd_y = %08d\n", get_lcd_x_frm_ts_x(a_ts_x), get_lcd_y_frm_ts_y(a_ts_y));
	printf("B lcd_x = %08d, lcd_y = %08d\n", get_lcd_x_frm_ts_x(b_ts_x), get_lcd_y_frm_ts_y(b_ts_y));
	printf("C lcd_x = %08d, lcd_y = %08d\n", get_lcd_x_frm_ts_x(c_ts_x), get_lcd_y_frm_ts_y(c_ts_y));
	printf("D lcd_x = %08d, lcd_y = %08d\n", get_lcd_x_frm_ts_x(d_ts_x), get_lcd_y_frm_ts_y(d_ts_y));
	printf("E lcd_x = %08d, lcd_y = %08d\n", get_lcd_x_frm_ts_x(e_ts_x), get_lcd_y_frm_ts_y(e_ts_y));
}

/*
 * 读TS原始数据, 转换为LCD坐标
 */
int ts_read(int *lcd_x, int *lcd_y, int *lcd_pressure) {
	int ts_x, ts_y, ts_pressure;
	int tmp_x, tmp_y;

	ts_read_raw(&ts_x, &ts_y, &ts_pressure);

	if (g_ts_xy_swap) {
		swap_xy(&ts_x, &ts_y);
	}

	/* 使用公式计算 */
	tmp_x = g_kx * (ts_x - g_ts_xc) + g_lcd_xc;
	tmp_y = g_ky * (ts_y - g_ts_yc) + g_lcd_yc;

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

	if (g_ts_xy_swap) {
		swap_xy(&ts_x, &ts_y);
	}

	/* 使用公式计算 */
	tmp_x = g_kx * (ts_x - g_ts_xc) + g_lcd_xc;
	tmp_y = g_ky * (ts_y - g_ts_yc) + g_lcd_yc;

	if (tmp_x < 0 || tmp_x >= xres || tmp_y < 0 || tmp_y >= yres)
		return -1;

	*lcd_x = tmp_x;
	*lcd_y = tmp_y;
	*lcd_pressure = ts_pressure;
	return 0;
}
