
/* 启动ADC, 读出AIN1电压值
 *
 */


void photoresistor_test(void) {
	int val, val0;
	double vol, vol0;
	int m, m0; /* 整数部分 */
	int n, n0; /* 小数部分 */

	//adc_init();

	while (!serial_getc_async()) {
		val = adc_read(1);
		vol = (double)val / 1023 * 3.3; /* 1023----3.3v */
		m = (int)vol;	/* 3.01, m = 3 */
		vol = vol - m;	/* 小数部分: 0.01 */
		n = vol * 1000;  /* 10 */

		val0 = adc_read(0);
		vol0 = (double)val0 / 1023 * 3.3; /* 1023----3.3v */
		m0 = (int)vol0;	/* 3.01, m = 3 */
		vol0 = vol0 - m0;	/* 小数部分: 0.01 */
		n0 = vol0 * 1000;  /* 10 */

		/* 在串口上打印 */
		printf("photoresistor vol: %d.%03dv, compare to threshold %d.%03dv\r", m, n, m0, n0);  /* 3.010v */

		/* 在LCD上打印 */
		//fb_print_string();
	}

}

