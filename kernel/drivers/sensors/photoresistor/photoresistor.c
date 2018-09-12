
void photoresistor_get_vol(int *m, int *n, int c){
	double vol, vol0;
	int val, val0;
	val = adc_read(c);
	vol = (double)val / 1023 * 3.3; /* 1023----3.3v */
	*m = (int)vol;	/* 3.01, m = 3 */
	vol = vol - *m;	/* 小数部分: 0.01 */
	*n = vol * 1000;  /* 10 */
}

void photoresistor_test(void) {
	int m, m0; /* 整数部分 */
	int n, n0; /* 小数部分 */

	while (!serial_getc_async()) {
		photoresistor_get_vol(&m0, &n0, 0);
		photoresistor_get_vol(&m, &n, 1);
		printf("photoresistor vol: %d.%03dv, compare to threshold %d.%03dv\r", m, n, m0, n0);  /* 3.010v */
	}
}

