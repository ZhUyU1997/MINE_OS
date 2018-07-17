#include <stdio.h>
#include <assert.h>
#include <framebuffer.h>
#include <s3c24xx.h>
#include "ff.h"


/* 位图文件头主要是对位图文件的一些描述 位图信息头主要是对位图图像方面信息的描述 */
/*************************** 位图标准信息(54字节) ******************************************/
/* 位图文件头 （位图头的字节数 = 位图文件字节数 - 位图图像数据字节数）*/
typedef struct BMP_FILE_HEADER {
	U16 bType;			// 文件标识符
	U32 bSize;			// 文件的大小
	U16 bReserved1;		// 保留值,必须设置为0
	U16 bReserved2;		// 保留值,必须设置为0
	U32 bOffset;		// 文件头的最后到图像数据位开始的偏移量
} __attribute__((packed)) BMPFILEHEADER;	// 14 字节

/* 位图信息头 */
typedef struct BMP_INFO {
	U32 bInfoSize;			// 信息头的大小
	U32 bWidth;				// 图像的宽度
	U32 bHeight;			// 图像的高度
	U16 bPlanes;			// 图像的位面数
	U16 bBitCount;			// 每个像素的位数
	U32 bCompression;		// 压缩类型
	U32 bmpImageSize;		// 图像的大小,以字节为单位
	U32 bXPelsPerMeter;		// 水平分辨率
	U32 bYPelsPerMeter;		// 垂直分辨率
	U32 bClrUsed;			// 使用的色彩数
	U32 bClrImportant;		// 重要的颜色数
} __attribute__((packed)) BMPINF;	// 40 字节

/* 彩色表:调色板 */
typedef struct RGB {
	U8 b;    // 蓝色强度
	U8 g;    // 绿色强度
	U8 r;    // 红色强度
	U8 rgbReversed; // 保留值
} RGB;


int drawImage(char *name) {
	BMPFILEHEADER bmpFileHeader;  // 定义一个 BMP 文件头的结构体
	BMPINF bmpInfo;               // 定义一个 BMP 文件信息结构体
	FIL f;
	FRESULT res;
	UINT br;
	if ((res = f_open(&f, name, FA_READ)) != FR_OK) {
		printf("再见\n");
		return 1;
	}

	if ((res = f_read(&f, &bmpFileHeader, sizeof(bmpFileHeader), &br)) != FR_OK)
		goto exit;
	if ((res = f_read(&f, &bmpInfo, sizeof(bmpInfo), &br)) != FR_OK)
		goto exit;
	/*
	// 输出BMP文件的位图文件头的所有信息
	printf("位图文件头主要是对位图文件的一些描述:BMPFileHeader\n\n");
	printf("文件标识符 = %#X\n", bmpFileHeader.bType);
	printf("BMP 文件大小 = %d 字节\n", bmpFileHeader.bSize);
	printf("保留值1 = %d \n", bmpFileHeader.bReserved1);
	printf("保留值2 = %d \n", bmpFileHeader.bReserved2);
	printf("文件头的最后到图像数据位开始的偏移量 = %d 字节\n", bmpFileHeader.bOffset);

	// 输出BMP文件的位图信息头的所有信息
	printf("\n\n位图信息头主要是对位图图像方面信息的描述:BMPInfo\n\n");
	printf("信息头的大小 = %d 字节\n", bmpInfo.bInfoSize);
	printf("位图的高度 = %d \n", bmpInfo.bHeight);
	printf("位图的宽度 = %d \n", bmpInfo.bWidth);
	printf("图像的位面数(位面数是调色板的数量,默认为1个调色板) = %d \n", bmpInfo.bPlanes);
	printf("每个像素的位数 = %d 位\n", bmpInfo.bBitCount);
	printf("压缩类型 = %d \n", bmpInfo.bCompression);
	printf("图像的大小 = %d 字节\n", bmpInfo.bmpImageSize);
	printf("水平分辨率 = %d \n", bmpInfo.bXPelsPerMeter);
	printf("垂直分辨率 = %d \n", bmpInfo.bYPelsPerMeter);
	printf("使用的色彩数 = %d \n", bmpInfo.bClrUsed);
	printf("重要的色彩数 = %d \n", bmpInfo.bClrImportant);

	printf("\n\n\n压缩说明：有0（不压缩），1（RLE 8，8位RLE压缩），2（RLE 4，4位RLE压缩，3（Bitfields，位域存放）");
	*/
	if (bmpInfo.bBitCount != 24)
		goto exit;
	int totalSize = (bmpInfo.bWidth * bmpInfo.bBitCount / 8 + 3) / 4 * 4 * bmpInfo.bHeight; //计算总字节数（4字节对齐）
	RGB rgb;
	char buf[3 * 480];

	for (int y = 0; y < bmpInfo.bHeight; y++) {
		int count;
		for (int x = 0; x < bmpInfo.bWidth; x += count) {
			if (bmpInfo.bWidth - x >= (sizeof(buf) / 3)) {
				count = (sizeof(buf) / 3);
			} else {
				count = bmpInfo.bWidth - x;
			}
			res = f_read(&f, buf, 3 * count, &br);
			if ((res != FR_OK) || (br == 0))
				goto exit;
			for (int i = 0; i < count; i++) {
				assert(x + i < 480);
				rgb.b = buf[3 * i + 0];
				rgb.g = buf[3 * i + 1];
				rgb.r = buf[3 * i + 2];
				PutPixel(x + i, bmpInfo.bHeight - y - 1, convert888_565(*(U32 *)&rgb));
			}
		}
		if (bmpInfo.bWidth % 4 > 0) {
			res = f_read(&f, &rgb, 4 - bmpInfo.bWidth % 4, &br);
			if ((res != FR_OK) || (br == 0))
				goto exit;
		}
	}

exit:
	f_close(&f);
	return 0;
}

