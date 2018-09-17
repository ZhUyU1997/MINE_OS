#include <sys/types.h>
#include <assert.h>
#include <timer.h>
#include <usb/2440usb.h>
#include "command.h"
#include <memory.h>
#include "ff.h"

#define CMD_MAX_CMD_NUM 50
#define CMD_MAXARGS 10
extern cmd_table *ct_list[];

DIR  dirobj;               // current work dir fof cd
void FileAttr(BYTE attr, char *p) {
	if ((attr & 0x10) == 0x10) {
		sprintf(p, "%5s", "dir :");
	} else {
		sprintf(p, "%5s", "file:");
	}

}
CMD_DEFINE(ls, "ls", "ls") {
	char p_cmd[16], p_arg[32];
	char *p_path, *pfname;
	FRESULT read_res, dir_res;
	DIR  tempdir;
	FILINFO tempfinfo;
	char fdesp[8];
	FRESULT res1 = f_opendir(&dirobj, "/");

	if (res1 == FR_OK)
		if (argc == 1) {
			p_path = "/";
		} else if (argc == 2) {
			p_path = argv[1];
		} else {
			return 1;
		}
	dir_res = f_opendir(&tempdir, p_path);
	if (dir_res != FR_OK) {
		printf("f_opendir failed,path:%s does not exist\n\r", p_path);
		return 1;
	}
	for (;;) {
		read_res = f_readdir(&tempdir, &tempfinfo);
		if ((read_res != FR_OK) || (tempfinfo.fname[0] == 0)) {
			break;
		} else if (tempfinfo.fname[0] == '.') {
			continue;
		} else {
			pfname = tempfinfo.fname;
			FileAttr((tempfinfo.fattrib), fdesp);
			printf("%s   %-15s  %8dbyte\n\r", fdesp, pfname, tempfinfo.fsize);
		}
	}
	return 0;
}
CMD_DEFINE(wav, "wav", "wav") {
	if (argc != 2)
		return 1;
	read_wav_file(argv[1]);
	return 0;
}
CMD_DEFINE(usbdebug, "usbdebug", "usbdebug") {
#if USB_DEBUG == 1
	DbgPrintf("show");
#endif
	return 0;
}
CMD_DEFINE(usbtest, "usbtest", "usbtest") {
	printf("USB slave 测试\n");
	usb_init_slave();
	return 0;
}
CMD_DEFINE(backtrace, "backtrace", "backtrace") {
	printf("backtrace测试\n");
	char s[128];
	for (int i = 0; i < 128 / 3; i++) {
		*(volatile int *)(s + 3 * i) = 0;
	}
	return 0;
}
CMD_DEFINE(ts_test, "ts_test", "ts_test") {
	ts_test_view();
	return 0;
}
CMD_DEFINE(test, "test", "test") {
	ten_test_view();
	return 0;
}
CMD_DEFINE(lcd_test, "lcd_test", "lcd_test") {
	lcd_putstr(0, 0, "11113145623vdfhigaeruirh4uifthv89y9q3ry478h7f@#$%^^!@#$%^&*((*)_+-=\":>?<{}|;'][]\./,./");
	return 0;
}
CMD_DEFINE(usbmouse, "usbmouse", "usbmouse") {
#if 1
	while (1) {
		U8 Buf[4] = {0, 0, 0, 0};
		switch (getc()) {
			case 'a':
				Buf[1] = -1;	//这里一次往左移动一个单位。
				break;
			case 'd':
				Buf[1] = 1;		//这里一次往右移动一个单位。
				break;
			case 'w':
				Buf[2] = -1;	//这里一次往上移动一个单位。
				break;
			case 's':
				Buf[2] = 1;		//这里一次往下移动一个单位。
				break;
			case 'j':
				Buf[0] |= 0x01;	//D0为鼠标左键
				break;
			case 'k':
				Buf[0] |= 0x02;	//D1为鼠标右键
				break;
			case 'q':
			case 'Q':
				return 0;
				break;
			default:
				break;
		}
		usb_send_init(EP1, Buf, sizeof(Buf));
		usb_send_message(EP1);
	}
#endif
	return 0;
}
CMD_DEFINE(delay_u, "delay_u", "delay_u") {
	if (argc != 2)
		return 1;
	int time = simple_strtoul(argv[1], NULL, 10);
	run_command("RTC");
	for (int i = time; i > 0; i -= 60000)
		delay_u((time > 60000) ? 60000 : time);
	run_command("RTC");
	return 0;
}
CMD_DEFINE(udelay, "udelay", "udelay") {
	run_command("RTC");
	if (argc != 2)
		return 1;
	udelay(simple_strtoul(argv[1], NULL, 10));
	run_command("RTC");
	return 0;
}
CMD_DEFINE(wr_at24xx, "wr_at24xx", "wr_at24xx") {
	if (argc != 3)
		return 1;
	int err;

	/* 获得地址 */
	unsigned int addr = simple_strtoul(argv[1], NULL, 10);
	if (addr > 256) {
		printf("address > 256, error!\n");
		return;
	}
	printf("[write:%d][%s]\n", addr, argv[2]);
	err = at24cxx_write(addr, argv[2], strlen(argv[2]) + 1);
	if (err)
		printf("[error]\n");
	return 0;
}

void view_hex(char *data, int len){
	for (int i = 0; i < (len + 15) / 16; i++) {
		/* 每行打印16个数据 */
		for (int j = 0; j < 16; j++) {
			/* 先打印数值 */
			unsigned char c = data[i * 16 + j];
			if ((i * 16 + j) < len)
				printf("%02x ", c);
			else
				printf("   ");
		}

		printf("   ; ");

		for (int j = 0; j < 16; j++) {
			/* 后打印字符 */
			unsigned char c = data[i * 16 + j];
			if ((i * 16 + j) < len){
				if (c < 0x20 || c > 0x7e)  /* 不可视字符 */
					putchar('.');
				else
					putchar(c);
			}
				
		}
		printf("\n");
	}
}
CMD_DEFINE(rd_at24xx, "rd_at24xx", "rd_at24xx addr len") {
	unsigned char data[100];
	unsigned char str[16];
	int err;
	int cnt = 0;

	if (argc != 3)
		return 1;
	/* 获得地址 */
	unsigned int addr = simple_strtoul(argv[1], NULL, 10);

	if (addr > 256) {
		printf("address > 256, error!\n");
		return;
	}

	/* 获得长度 */
	int len = simple_strtoul(argv[2], NULL, 10);
	err = at24cxx_read(addr, data, len);
	if (err)
		printf("[error]\n");
	printf("[data view]\n");
	view_hex(data, len);
	return 0;
}
CMD_DEFINE(i2c_init, "i2c_init", "i2c_init") {
	i2c_init();
	return 0;
}
CMD_DEFINE(i2c_test, "i2c_test", "i2c_test") {
	run_command("i2c_init");
	run_command("wr_at24xx 0 helloworld!");
	run_command("rd_at24xx 0 20");
	return 0;
}
CMD_DEFINE(adc_test, "adc_test", "adc_test") {
	int vol0, vol1;
	int t0, t1;
	printf("Measuring the voltage of AIN0 and AIN1, press any key to exit\n");
	while (!serial_getc_async()) {  // 串口无输入，则不断测试
		get_adc(&vol0, &t0, 0);
		get_adc(&vol1, &t1, 2);
		printf("AIN0 = %d.%-3dV    AIN2 = %d.%-3dV\r", (int)vol0, t0, (int)vol1, t1);
	}
	return 0;
}
CMD_DEFINE(res_test, "res_test", "res_test") {
	photoresistor_test();
	return 0;
}
CMD_DEFINE(dh_test, "dh_test", "dh_test") {
	dht11_test();
	return 0;
}
CMD_DEFINE(ds_test, "ds_test", "ds_test") {
	ds18b20_test();
	return 0;
}
CMD_DEFINE(irda_raw, "irda_raw", "irda_raw") {
	irda_raw_test();
	return 0;
}
CMD_DEFINE(irda_nec, "irda_nec", "irda_nec") {
	irda_nec_test();
	return 0;
}
CMD_DEFINE(bmp_test, "bmp_test", "bmp_test") {
	drawImage("xx01.bmp");
	return 0;
}
CMD_DEFINE(RTC, "RTC", "RTC") {
	char data[7] = {0};
	char *week_str[7] = {"一", "二", "三", "四", "五", "六", "日"};
	char *week;
	if (argc == 1) {
		RTC_Read(&data[0], &data[1], &data[2], &data[3], &data[4], &data[5], &data[6]);

		if (data[3] >= 1 && data[3] <= 7) {
			week = week_str[data[3] - 1];
			printf("%d年,%d月,%d日,星期%s,%d点,%d分,%d秒\n", 2000 + data[0],
				   data[1], data[2], week, data[4], data[5], data[6]);
		} else {
			printf("error!\n");
			return 1;
		}
	} else if (argc == 8) {
		for (int i = 0; i < 7; i++) {
			data[i] = simple_strtoul(argv[i + 1], NULL, 10);
		}
		//year:0-99
		RTC_Set(data[0], data[1], data[2], data[3], data[4], data[5], data[6]);
		if (data[3] >= 1 && data[3] <= 7) {
			week = week_str[data[3] - 1];
			printf("%d年,%d月,%d日,星期%s,%d点,%d分,%d秒\n", 2000 + data[0],
				   data[1], data[2], week, data[4], data[5], data[6]);
			printf("设置成功\n");
		} else {
			printf("error!\n");
			return 1;
		}
	} else {
		printf("error!参数数量异常\n");
		return 1;
	}
	return 0;
}
CMD_DEFINE(spi_test, "spi_test", "spi_test") {
	int vol0, vol1;
	int t0, t1;
	char str[200];
	unsigned int mid, pid;
	SPIInit();
	OLEDInit();
	OLEDPrint(0, 0, "www.100ask.net");

	SPIFlashReadID(&mid, &pid);
	printf("SPI Flash : MID = 0x%02x, PID = 0x%02x\n", mid, pid);

	sprintf(str, "SPI : %02x, %02x", mid, pid);
	OLEDPrint(2, 0, str);

	SPIFlashInit();

	SPIFlashEraseSector(4096);
	SPIFlashProgram(4096, "100ask", 7);
	SPIFlashRead(4096, str, 7);
	printf("SPI Flash read from 4096: %s\n", str);
	OLEDPrint(4, 0, str);

	i2c_init();

	OLEDClearPage(2);
	OLEDClearPage(3);

	printf("Measuring the voltage of AIN0 and AIN1, press any key to exit\n");
	while (!serial_getc_async()) {  // 串口无输入，则不断测试
		get_adc(&vol0, &t0, 0);
		get_adc(&vol1, &t1, 2);
		//printf("AIN0 = %d.%-3dV    AIN2 = %d.%-3dV\r", (int)vol0, t0, (int)vol1, t1);
		sprintf(str, "ADC: %d.%-3d, %d.%-3d", (int)vol0, t0, (int)vol1, t1);
		OLEDPrint(6, 0, str);
	}
	return 0;
}
CMD_DEFINE(usbslave,
		   "usbslave - get file from host(PC)",
		   "[loadAddress] [wait] \n"
		   "\"wait\" is 0 or 1, 0 means for return immediately, not waits for the finish of transferring") {
#if 0
	//TODO:最好将文件下载到文件系统中
	extern int download_run;
	extern volatile U32 dwUSBBufBase;
	extern volatile U32 dwUSBBufSize;

	int wait = 1;
#define BUF_SIZE (1024*1024)
	/* download_run为1时表示将文件保存在USB Host发送工具dnw指定的位置
	 * download_run为0时表示将文件保存在参数argv[2]指定的位置
	 * 要下载程序到内存，然后直接运行时，要设置download_run=1，这也是这个参数名字的来由
	 */
	//由于0x3000000存放了页表，必须download_run = 0确保下载地址正确，即不采用上位机设置的地址
	download_run = 0;//默认由下位机决定地址和大小
	if (argc == 2) {
		//dwUSBBufBase = kmalloc(BUF_SIZE);
		dwUSBBufBase = 0x30a00000;
		if (!dwUSBBufBase) {
			printf("malloc memory error!\n");
			return 1;
		}
		wait = (int)simple_strtoul(argv[1], NULL, 16);
		dwUSBBufSize = BUF_SIZE;
	} else {
		return 1;
	}
	usb_init_slave();
	int size = usb_receive(dwUSBBufBase, dwUSBBufSize, wait);
	assert(size > 0 && size <= BUF_SIZE);
#endif
	return 0;
}
CMD_DEFINE(mmtest, "mmtest", "mmtest") {
	UINT16 *data,data2;
	data = kmalloc(7680, 0);
	printf("kamlloc size = %d addr = %X\n", 7680, data);
	kfree(data);
	data2 = kmalloc(7680, 0);
	printf("kamlloc size = %d addr = %X\n", 7680, data2);
	kfree(data);
	
	return 0;
}
CMD_DEFINE(analyse_keycode, "analyse_keycode", "analyse_keycode") {
	while(1){
		unsigned int c1 = getc();
		//unsigned int c2 = serial_getc_async();
		printf("%02X ", c1);
		//if(c2!=0){
		//	printf("%02X ", c2);
		//}
		//	
	}
	return 0;
}
static void vt100_response(char *str){
	printf(str);
}
CMD_DEFINE(vt100, "vt100", "vt100") {
	ili9340_init();
	ili9340_setRotation(0);
	vt100_init(vt100_response);
	ili9340_fillRect(0, 0, 480, 272, 0);
	vt100_puts("\e[?7l");
	while(1){
		unsigned int data = getc();
		if(data == 'T'){ // ´ key on my kb
			test_colors();
			mdelay(2000); 
			test_cursor();
			mdelay(2000);
			test_edit();
			mdelay(2000);
			test_scroll();
			mdelay(2000);
		}else
			vt100_putc(data);
	}
	return 0;
}
CMD_DEFINE(panic, "panic", "panic") {
	panic();
	return 0;
}
CMD_DEFINE(help, "help", "help") {
	for (int i = 0; ct_list[i] != NULL; i++) {
		printf("%-20s:\t-%s\n", ct_list[i]->name, ct_list[i]->usage);
	}
	return 0;
}
#define CMD_ENTRY(x) & ct_##x
cmd_table *ct_list[] = {
	CMD_ENTRY(help),
	CMD_ENTRY(ls),
	CMD_ENTRY(wav),
	CMD_ENTRY(usbslave),
	CMD_ENTRY(delay_u),
	CMD_ENTRY(udelay),
	CMD_ENTRY(usbdebug),
	CMD_ENTRY(usbmouse),
	CMD_ENTRY(usbtest),
	CMD_ENTRY(ts_test),
	CMD_ENTRY(test),
	CMD_ENTRY(lcd_test),
	CMD_ENTRY(wr_at24xx),
	CMD_ENTRY(rd_at24xx),
	CMD_ENTRY(i2c_init),
	CMD_ENTRY(i2c_test),
	CMD_ENTRY(adc_test),
	CMD_ENTRY(spi_test),
	CMD_ENTRY(res_test),
	CMD_ENTRY(dh_test),
	CMD_ENTRY(ds_test),
	CMD_ENTRY(RTC),
	CMD_ENTRY(irda_raw),
	CMD_ENTRY(irda_nec),
	CMD_ENTRY(bmp_test),
	CMD_ENTRY(backtrace),
	CMD_ENTRY(mmtest),
	CMD_ENTRY(analyse_keycode),
	CMD_ENTRY(vt100),
	CMD_ENTRY(panic),
	NULL
};
cmd_table *search_cmd(char *name) {
	for (int i = 0; ct_list[i] != NULL; i++) {
		if (strcmp(ct_list[i]->name, name) == 0) {
			return ct_list[i];
		}
	}
	return NULL;
}
int run_command(char *cmd) {
	char str[256] = {
		[255] = 0
	};
	strncpy(str, cmd, 255);

	char *argv[CMD_MAXARGS + 1] = {0};	/* NULL terminated	*/
	int argc = 0;
	int cmdlen = strlen(cmd);

	for (int i = 0; i < cmdlen; i++) {
		if (str[i] != ' ' && i != 0) {
			continue;
		} else {
			while (str[i] == ' ') {
				str[i] = '\0';
				i++;
			}
			if (i < cmdlen) {
				argv[argc] = &str[i];
				argc++;
				if (argc == CMD_MAXARGS + 1)
					return -1;
			} else
				break;
		}
	}
	cmd_table *pct = search_cmd(argv[0]);
	if (pct) {
		pct->cmd(pct, argc, argv);
	} else {
		printf("%s:command not found\n", argv[0]);
		return 0;
	}
	return 1;
}
static int get_str(char *buf, int len) {
	int i;
	for (i = 0; i < len - 1; i++) {
		char c = getc();
		if (c == '\r') {
			if (i == 0) {
				return -1;
			} else {
				printf("\n");
				buf[i] = '\0';
				break;
			}
		} else if (c == '\b') {
			if (i > 0) { //前面有字符
				putc(c);
				i = i - 2;
			} else { //前面没有字符
				i = i - 1;
			}
		} else {
			putc(c);
			buf[i] = c;
		}
	}
	return 1;
}
int cmd_loop() {
	char buf[100] = {0};
	while (1) {
		printf("\nOS>");
		if (get_str(buf, 100) == -1)
			continue;
		run_command(buf);
	}
}
