#include <stdio.h>
#include "ff.h"
typedef unsigned short WORD;
struct RIFF_BLOCK {
	char riff[4];
	DWORD riffSize;
	char wave[4];
};
struct FMT_BLOCK {
	char fmt[4];
	DWORD fmtSize;
	WORD formatTag;
	WORD channels;
	DWORD samplesPerSec;
	DWORD bytesPerSec;
	WORD blockAlign;
	WORD bitsPerSample;
};
struct FACT_BLOCK {
	char fact[4];
	DWORD factSize;
	DWORD factData;
};
struct DATA_BLOCK {
	char data[4];
	DWORD dataSize;
};
struct FILE_HEADER {
	struct RIFF_BLOCK riff_block;
	struct FMT_BLOCK fmt_block;
	struct FACT_BLOCK fact_block;
	struct DATA_BLOCK data_block;
};
#define BUF_SIZE (1024*500)
static short buf[BUF_SIZE / 2];
static short sbuf[2][BUF_SIZE];
int read_wav_file(const char *filename) {
	FIL fp;
	struct FILE_HEADER file_header;
	UINT br;
	FRESULT rst = f_open(
					  &fp,			/* Pointer to the blank file object */
					  filename,	/* Pointer to the file name */
					  FA_READ			/* Access mode and file open mode flags */
				  );
	if (rst != FR_OK) {
		printf("\033[31m文件打开失败\033[0m\n");
		return 1;
	}
	if (fp.fsize <= sizeof(struct FILE_HEADER)) {
		printf("\033[31m文件尺寸过小\033[0m\n");
		f_close(&fp);
		return 1;
	}

	rst = f_read(
			  &fp, 		/* Pointer to the file object */
			  &file_header,		/* Pointer to data buffer */
			  sizeof(struct FILE_HEADER),		/* Number of bytes to read */
			  &br		/* Pointer to number of bytes read */
		  );
	if (rst != FR_OK) {
		printf("\033[31m文件读取失败\033[0m\n");
		f_close(&fp);
		return 1;
	}
	if (strncmp(file_header.riff_block.riff, "RIFF", 4)) {
		printf("\033[31mRIFF文件头校验失败\033[0m\n");
		f_close(&fp);
		return 1;
	}
#define PRINT(x) printf(#x" = %d\n", x);
	PRINT(file_header.fmt_block.fmtSize);
	PRINT(file_header.fmt_block.samplesPerSec);
	PRINT(file_header.fmt_block.channels);
	int channels = file_header.fmt_block.channels;
	sound_init();
	open_sound();
	dma_init();
	printf("[volume: %03d]", wm8976_get_volume());
	for (int index = 0; fp.fsize != fp.fptr; index = (index + 1) % 2) {
		rst = f_read(
				  &fp, 		/* Pointer to the file object */
				  (channels == 2) ? sbuf[index] : buf,		/* Pointer to data buffer */
				  (channels == 2) ? sizeof(sbuf[index]) : (sizeof(buf)),		/* Number of bytes to read */
				  &br		/* Pointer to number of bytes read */
			  );
		if (rst != FR_OK) {
			printf("\033[31m文件读取失败\033[0m\n");
			close_sound();
			f_close(&fp);
			return 1;
		}
		//printf("br = %d\n", br);
		if (channels == 1) {
			for (int j = 0; j < br / 2; j++) {
				sbuf[index][2 * j] = buf[j];
				sbuf[index][2 * j + 1] = buf[j];
			}
		}
		switch (serial_getc_async()) {
			case 'w':
				wm8976_up_volume();
				printf("\r[volume: %03d]", wm8976_get_volume());
				break;
			case 'e':
				wm8976_down_volume();
				printf("\r[volume: %03d]", wm8976_get_volume());
				break;
			case 'q':
				goto exit;
				break;
			default:
				break;
		}
		while (!dma_can_run());
		dma_set(sbuf[index], (channels == 2) ? br : (br * 2));
		dma_start();
	}

exit:
	//如果在DMA传输之前关闭IIS,DSTAT清零会失败
	while (!dma_can_run());
	close_sound();
	f_close(&fp);
	return 0;
}




