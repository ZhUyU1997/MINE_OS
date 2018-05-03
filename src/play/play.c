#include "ff.h"
#include "play.h"
#include "mp3common.h"
#include "mp3.h"
#include "mp3_buf.h"
#define READBUF_SIZE		1024*12	//4000//4096//4000       // Value must min be 2xMAINBUF_SIZE = 2x1940 = 3880bytes
unsigned char readBuf[READBUF_SIZE];	// Read buffer where data from SD card is read to

int AudioDecode(char *Name) {
	FIL F;
	FRESULT fres = f_open(&F, Name, FA_READ);
	if (fres != FR_OK)
		return 1;

	printf("begin to decode %s\n", Name);
	int Status = MpegAudioDecoder(&F);
	printf("end decode %s\n", Name);
	if (Status) {
		printf("an error occurred during decoding %s.\n", Name);
	}
	f_close(&F);
	return (Status);
}

int MpegAudioDecoder(FIL *InputFp) {
	// Content is the output from MP3GetLastFrameInfo,
	// we only read this once, and conclude it will be the same in all frames
	// Maybe this needs to be changed, for different requirements.
	MP3FrameInfo mp3FrameInfo;
	int bytesLeft;					// Saves how many bytes left in readbuf
	unsigned char *readPtr;			// Pointer to the next new data
	int offset;						// Used to save the offset to the next frame
	UINT br, frame_count;
	int ret = 1;
	//检查MP3文件格式
	MP3CTRL mp3ctrl;	//mp3控制结构体

	u8 rst = mp3_get_info(InputFp, readBuf, READBUF_SIZE, &mp3ctrl);
	if (rst)
		return 1;

	/* Decode stdin to stdout. */
	printf("title:%s\n", mp3ctrl.title);
	printf("artist:%s\n", mp3ctrl.artist);
	printf("bitrate:%dbps\n", mp3ctrl.bitrate);
	printf("samplerate:%d\n", mp3ctrl.samplerate);
	printf("totalsec:%d\n", mp3ctrl.totsec);

	f_lseek(InputFp, mp3ctrl.datastart);	//跳过文件头中tag信息


	/* Initilizes the MP3 Library */
	// hMP3Decoder: Content is the pointers to all buffers and information for the MP3 Library
	HMP3Decoder hMP3Decoder = MP3InitDecoder();
	if (hMP3Decoder == 0) {
		// 这意味着存储器分配失败。这通常在堆存储空间不足时发生。
		// 请使用其他堆存储空间重新编译代码。
		printf("\033[31mMP3 Decoder 初始化失败\033[0m\n");
		return 1;
	}

	frame_count = 0;

	sound_init();
	open_sound();
	framebuf_init();
	printf("[volume: %03d]", wm8976_get_volume());
	while (1) {

		bytesLeft = 0;
		readPtr = readBuf;

		FRESULT fres = f_read(InputFp, readBuf, READBUF_SIZE, &br);
		if (fres != FR_OK) {
			printf("文件读取失败\n");
			ret = 1;
			goto exit;
		}

		if (br == 0) {
			ret = 0;
			goto exit;
		}

		bytesLeft += br;
		//printf("1 readBuf = %X,readPtr = %X, bytesLeft %d\n", readBuf, readPtr, bytesLeft);

		while (1) {
			/* find start of next MP3 frame - assume EOF if no sync found */
			int offset = MP3FindSyncWord(readPtr, bytesLeft);
			if (offset < 0) {
				printf("MP3FindSyncWord failed!!\n");
				break;
			}

			readPtr += offset;		//data start point
			bytesLeft -= offset;	//in buffer
			int errs = MP3Decode(hMP3Decoder, &readPtr, &bytesLeft, get_next_framebuf(), 0);


			if (errs != ERR_MP3_NONE) {
				printf("err code %d ,readBuf = %X,readPtr = %X, bytesLeft %d\n", errs, readBuf, readPtr, bytesLeft);
				switch (errs) {
					case ERR_MP3_INVALID_FRAMEHEADER:
						printf("INVALID_FRAMEHEADER\n");
						//bytesLeft = 0;
						//readPtr = readBuf;
						//continue;
						goto exit;
						break;
					case ERR_MP3_INDATA_UNDERFLOW:
						printf("INDATA_UNDERFLOW\n");
						goto exit;
						break;
					case ERR_MP3_MAINDATA_UNDERFLOW:
						printf("MAINDATA_UNDERFLOW\n");
						//bytesLeft = READBUF_SIZE;
						//readPtr = readBuf;
						//continue;
						goto exit;
						break;
					case ERR_MP3_FREE_BITRATE_SYNC:
						printf("FREE_BITRATE_SYNC\n");
						goto exit;
						break;
					default:
						printf("ERR\n");
						goto exit;
						break;
				}
			}
			MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);

			if (bytesLeft < MAINBUF_SIZE * 2) {
				memmove(readBuf, readPtr, bytesLeft);
				fres = f_read(InputFp, readBuf + bytesLeft, READBUF_SIZE - bytesLeft, &br);
				if ((fres != FR_OK) || (br == 0)) {
					ret = 1;
					goto exit;
				}

				if (br < READBUF_SIZE - bytesLeft)
					memset(readBuf + bytesLeft + br, 0, READBUF_SIZE - bytesLeft - br);
				bytesLeft = READBUF_SIZE;
				readPtr = readBuf;
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
		}
	}

exit:
	//如果在DMA传输之前关闭IIS,DSTAT清零会失败
	framebuf_exit();
	close_sound();
	MP3FreeDecoder(hMP3Decoder);
	return 0;
}
