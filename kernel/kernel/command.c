#include <sys/types.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <timer.h>
#include <usb/2440usb.h>
#include <memory.h>
#include "command.h"
#include "fcntl.h"
#include "sys/dirent.h"
#include "vfs.h"

#define CMD_MAX_CMD_NUM 50
#define CMD_MAXARGS 10

extern cmd_table *ct_list[];
int run_command(char *cmd);

CMD_DEFINE(ls, "ls", "ls") {
	struct dirent *dir;
	int len = 0;
	int fd = sys_open("/",O_DIRECTORY);

	if(fd >= 0)
		dir = kmalloc(256, 0);
	else
		return 1;
	
	while(1){
		memset(dir,0,256);
		len = sys_getdents(fd,dir,256);
		if(len <= 0)
			break;
		printf("%s\n", dir->d_name);
	}

	sys_close(fd);
	return 0;
}
void show_dentry(struct dir_entry *dir,int deep){
	struct List *i;
	list_for_each(i,&dir->subdirs_list){
		for(int i=0;i<deep;i++){printf("-");}
		printf(">");
		struct dir_entry *temp = container_of(i,struct dir_entry,child_node);
		printf("%s\n", temp->name);
		mdelay(1000);
		show_dentry(temp,deep+1);
	}
}
CMD_DEFINE(lsdentry, "ls", "ls") {
	struct dir_entry *dir = root_sb->root;
	show_dentry(dir,0);
	return 0;
}
CMD_DEFINE(usbdebug, "usbdebug", "usbdebug") {
#if USB_DEBUG == 1
	DbgPrintf("show");
#endif
	return 0;
}
CMD_DEFINE(usbtest, "usbtest", "usbtest") {
#if 0
	printf("USB slave 测试\n");
	usb_init_slave();
#endif
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
CMD_DEFINE(usbmouse, "usbmouse", "usbmouse") {
#if 0
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

CMD_DEFINE(RTC, "RTC", "RTC") {
#if 0
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
#endif
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
	UINT16 *data, *data2;
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
#if 0
	terminal_init();
	setRotation(0);
	vt100_init(vt100_response);
	fillRect(0, 0, 480, 272, 0);
	vt100_puts("\e[?7l");
	test_colors();
	mdelay(2000); 
	test_cursor();
	mdelay(2000);
	test_edit();
	mdelay(2000);
	test_scroll();
	mdelay(2000);
#if 0
	while(1){
		unsigned int data = getc();
		if(data == 'T'){ // ´ key on my kb
			break;
		}
		vt100_putc(data);
	}
#endif
#endif
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
	CMD_ENTRY(lsdentry),
	CMD_ENTRY(usbslave),
	CMD_ENTRY(delay_u),
	CMD_ENTRY(udelay),
	CMD_ENTRY(usbdebug),
	CMD_ENTRY(usbmouse),
	CMD_ENTRY(usbtest),
	CMD_ENTRY(RTC),
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
	return 0;
}
