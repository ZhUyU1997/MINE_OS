#include <sys/types.h>
#include <assert.h>
#include "command.h"

#include "GUI.H"
#include "math.h"
#include "GUI_Protected.h"
#include "WM.h"
#include "Dialog.h"
#include "LISTBOX.h"
#include "EDIT.h"
#include "SLIDER.h"
#include "FRAMEWIN.h"

#define CMD_MAX_CMD_NUM 50
#define CMD_MAXARGS 10
extern cmd_table *ct_list[];
CMD_DEFINE(drawline, "drawline", "drawline") {
	if (argc != 5)
		return 1;
	int x0 = simple_strtoul(argv[1], NULL, 10);
	int y0 = simple_strtoul(argv[2], NULL, 10);
	int x1 = simple_strtoul(argv[3], NULL, 10);
	int y1 = simple_strtoul(argv[4], NULL, 10);
	GUI_DrawLine(x0, y0, x1, y1); //非抗锯齿函数显示正常
	return 0;
}
CMD_DEFINE(setcolor, "setcolor", "setcolor") {
	if (argc != 2)
		return 1;
	GUI_SetColor(simple_strtoul(argv[1], NULL, 16));
	return 0;
}
CMD_DEFINE(drawstr, "drawstr", "drawstr") {
	if (argc != 2)
		return 1;
	GUI_DispString(argv[1]);
	return 0;
}
CMD_DEFINE(drawcc, "drawcc", "drawcc") {
	for (int i = 10; i < 50; i++)
		GUI_DrawCircle(120, 60, i);
	return 0;
}
CMD_DEFINE(drawfc, "drawfc", "drawfc") {
	if (argc != 4)
		return 1;
	int x0 = simple_strtoul(argv[1], NULL, 10);
	int y0 = simple_strtoul(argv[2], NULL, 10);
	int r = simple_strtoul(argv[3], NULL, 10);
	GUI_FillCircle(x0, y0, r);
	return 0;
}
CMD_DEFINE(drawe, "drawe", "drawe") {
	if (argc != 5)
		return 1;
	int x0 = simple_strtoul(argv[1], NULL, 10);
	int y0 = simple_strtoul(argv[2], NULL, 10);
	int rx = simple_strtoul(argv[3], NULL, 10);
	int ry = simple_strtoul(argv[4], NULL, 10);
	GUI_DrawEllipse(x0, y0, rx, ry);
	return 0;
}
CMD_DEFINE(drawfe, "drawe", "drawe") {
	if (argc != 5)
		return 1;
	int x0 = simple_strtoul(argv[1], NULL, 10);
	int y0 = simple_strtoul(argv[2], NULL, 10);
	int rx = simple_strtoul(argv[3], NULL, 10);
	int ry = simple_strtoul(argv[4], NULL, 10);
	GUI_FillEllipse(x0, y0, rx, ry);
	return 0;
}
CMD_DEFINE(maintask, "maintask", "maintask") {
	MainTask();
	return 0;
}
CMD_DEFINE(initts, "initts", "initts") {
	init_Ts();
	return 0;
}
CMD_DEFINE(closets, "closets", "closets") {
	close_Ts();
	return 0;
}
CMD_DEFINE(enable_irq, "enable_irq", "enable_irq") {
	enable_irq();
	return 0;
}
CMD_DEFINE(disable_irq, "disable_irq", "disable_irq") {
	disable_irq();
	return 0;
}
CMD_DEFINE(help, "help", "help") {
	printf("cmd name:%s\n", ct->name);
	for (int i = 0; i < argc; i++) {
		printf("argv[%d]:%s\n", i, argv[i]);
	}
	for (int i = 0; ct_list[i] != NULL; i++) {
		printf("%s:\t-%s\n", ct_list[i]->name, ct_list[i]->usage);
	}
	return 0;
}
#define CMD_P(x) & ct_##x
cmd_table *ct_list[] = {
	CMD_P(help),
	CMD_P(drawline),
	CMD_P(setcolor),
	CMD_P(drawstr),
	CMD_P(drawcc),
	CMD_P(drawfc),
	CMD_P(drawe),
	CMD_P(drawfe),
	CMD_P(maintask),
	CMD_P(initts),
	CMD_P(closets),
	CMD_P(enable_irq),
	CMD_P(disable_irq),
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
int run_command (char *cmd, int flag) {
	char *str = cmd;
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
		//xshell 回车产生\r\n
		if (c == '\r') {
			getc();

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
		run_command (buf, 1);
	}
}
