/*
 * libx/print_hex.c
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>

void print_hex(char *data, int len){
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

		printf("   | ");

		for (int j = 0; j < 16; j++) {
			/* 后打印字符 */
			unsigned char c = data[i * 16 + j];
			if ((i * 16 + j) < len){
				if (!isprint(c))
					putchar('.');
				else
					putchar(c);
			}
		}
		printf("\n");
	}
}
