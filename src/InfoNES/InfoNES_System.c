/*-------------------------------------------------------------------*/
/*  Include files                                                    */
/*-------------------------------------------------------------------*/
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include "ff.h"
#include "InfoNES.h"

/*-------------------------------------------------------------------*/
/*  Palette data                                                     */
/*-------------------------------------------------------------------*/
WORD NesPalette[64] = {
	0x39ce, 0x1071, 0x0015, 0x2013, 0x440e, 0x5402, 0x5000, 0x3c20,
	0x20a0, 0x0100, 0x0140, 0x00e2, 0x0ceb, 0x0000, 0x0000, 0x0000,
	0x5ef7, 0x01dd, 0x10fd, 0x401e, 0x5c17, 0x700b, 0x6ca0, 0x6521,
	0x45c0, 0x0240, 0x02a0, 0x0247, 0x0211, 0x0000, 0x0000, 0x0000,
	0x7fff, 0x1eff, 0x2e5f, 0x223f, 0x79ff, 0x7dd6, 0x7dcc, 0x7e67,
	0x7ae7, 0x4342, 0x2769, 0x2ff3, 0x03bb, 0x0000, 0x0000, 0x0000,
	0x7fff, 0x579f, 0x635f, 0x6b3f, 0x7f1f, 0x7f1b, 0x7ef6, 0x7f75,
	0x7f94, 0x73f4, 0x57d7, 0x5bf9, 0x4ffe, 0x0000, 0x0000, 0x0000
};
char buffer[256 * 1024];
BYTE RomBuf[256 * 1024];
BYTE VROM_Buf[256 * 1024];

U32 ChColor(U16 color) {
	return (color >> 3) << 4 | (color & 0x001f);
}

/*-------------------------------------------------------------------*/
/*  Function prototypes                                              */
/*-------------------------------------------------------------------*/

/* Menu screen */
int InfoNES_Menu() {
	static int is_first = 1;
	if (is_first) {
		is_first = 0;
		return 0;
	}
	printf("要退出吗?[Y/N]:");
	switch (getc()) {
		case 'Y':
		case 'y':
			is_first = 1;
			return -1;
			break;
		default:
			break;
	}
	printf("\n");
	return 0;
}

/* Read ROM image file */
int InfoNES_ReadRom(const char *pszFileName) {
	int i, j, size;
	int res;
	unsigned int state;
	FIL  file;
	char *buf;
	res = f_open(&file, pszFileName, FA_READ) ;
	if (FR_OK == res) {
		printf("\nNES File Open OK!!!");
		printf("\nNES File Length= %d", file.fsize);
	} else {
		printf("\nFile Open Error!!! %d", res);
		f_close(&file);
		return -1;
	}
	res = f_read(&file, buffer, file.fsize, &state); //  读文件数据，长度file.fsize，
	if (FR_OK != res) {
		printf("\nFile Read Error!!!%d", res);
		f_close(&file);
		return -1;
	}

	i = 0;

	buf = buffer;
	/* Read ROM Header */
	NesHeader.byID[0] = buf[i++];
	NesHeader.byID[1] = buf[i++];
	NesHeader.byID[2] = buf[i++];
	NesHeader.byID[3] = buf[i++];
	NesHeader.byRomSize = buf[i++];
	NesHeader.byVRomSize = buf[i++];
	NesHeader.byInfo1 = buf[i++];
	NesHeader.byInfo2 = buf[i++];
	for (j = 0; j < 8; j++) NesHeader.byReserve[j] = buf[i++];

	printf("\n");
	printf("%c", NesHeader.byID[0]);
	printf("%c", NesHeader.byID[1]);
	printf("%c", NesHeader.byID[2]);
	printf("\n");

	printf("NesHeader.byInfo1    =%8x\n", NesHeader.byInfo1);
	printf("NesHeader.byInfo2    =%8x\n", NesHeader.byInfo2);
	if ((NesHeader.byID[0] != 0x4e) || (NesHeader.byID[1] != 0x45) || (NesHeader.byID[2] != 0x53) || (NesHeader.byID[3] != 0x1a)) {
		printf("\033[41;36m 未发现 NES 文件标识 \033[0m");
		return -1;
	}
	memset(SRAM, 0, SRAM_SIZE);

	/* If trainer presents Read Triner at 0x7000-0x71ff */
	if (NesHeader.byInfo1 & 4) {
		for (j = 0; j < 512; j++) SRAM[0x1000 + j] = buf[i++];
	}

	/* Allocate Memory for ROM Image */
	size = NesHeader.byRomSize * 0x4000 ;
	printf("需要 %dKB ROM\n", size / 1024);
	ROM = RomBuf;

	/* Read ROM Image */
	for (j = 0; j < size; j++) ROM[j] = buf[i++];

	if (NesHeader.byVRomSize > 0) {
		/* Allocate Memory for VROM Image */
		size = NesHeader.byVRomSize * 0x2000 ;
		printf("需要 %dKB VROM\n", size / 1024);
		VROM = VROM_Buf;
		/* Read VROM Image */
		for (j = 0; j < size; j++) VROM[j] = buf[i++];
	}
	/* Successful */
	return 0;
}

/* Release a memory for ROM */
void InfoNES_ReleaseRom() {
}

/* Transfer the contents of work frame on the screen */
void InfoNES_LoadFrame() {
	for (UINT32 y = 0; y < 240; y++) {
		for (UINT32 x = 0; x < 256; x++) {
			//printf("color:%X\n",ChColor(WorkFrame[x*256+y]));
			PutPixel(x, y, ChColor(WorkFrame[y * 256 + x]));
		}
	}
}

/* Get a joypad state */
void InfoNES_PadState(DWORD *pdwPad1, DWORD *pdwPad2, DWORD *pdwSystem) {
	unsigned char c = serial_getc_async();
	static int count[9] = {0};
	int index;
	*pdwPad1 = 0;
	*pdwPad2 = 0;
	
	for (int i = 0; i < 9; i++) {
		if (count[i] != 0) {
			if (i == 8) {
				*pdwSystem = PAD_SYS_QUIT;
				count[i] = 0;
			} else {
				*pdwPad1 |= 1 << i;
				count[i]--;
			}
		}
	}
	switch (c) {
		case '\0':
			return 0;
			break;
		case 'K'://A
		case 'k':
			index = 0;
			break;
		case 'J'://B
		case 'j':
			index = 1;
			break;
		case 'H'://ST
		case 'h':
			index = 2;
			break;
		case 'F'://SE
		case 'f':
			index = 3;
			break;
		case 'W'://上
		case 'w':
			index = 4;
			break;
		case 'S'://下
		case 's':
			index = 5;
			break;
		case 'A'://左
		case 'a':
			index = 6;
			break;
		case 'D'://右
		case 'd':
			index = 7;
			break;
		case 'Q'://右
		case 'q':
			index = 8;
			break;
		default:
			return;
			break;
	}
	count[index] = 20;
	if(index == 0)
		count[index] = 40;
}

/* memcpy */
void *InfoNES_MemoryCopy(void *dest, const void *src, int count) {
	for (int i = 0; i < count; i++)((U8 *)dest)[i] = ((U8 *)src)[i];
	return 0;
}

/* memset */
void *InfoNES_MemorySet(void *dest, int c, int count) {
	for (int i = 0; i < count; i++)((U8 *)dest)[i] = c;
	return 0;
}

/* Print debug message */
void InfoNES_DebugPrint(char *pszMsg) {
	printf(pszMsg);
}

/* Wait */
void InfoNES_Wait() {
}

/* Sound Initialize */
void InfoNES_SoundInit(void) {
	sound_init();
}

short sound_buf[1024*2];
/* Sound Open */
int InfoNES_SoundOpen(int samples_per_sync, int sample_rate) {
	printf("samples_per_sync = %d, sample_rate = %d\n", samples_per_sync, sample_rate);
	assert(2 * samples_per_sync <= (sizeof(sound_buf)/2));
	open_sound();
	dma_init();
	return 1;
}

/* Sound Close */
void InfoNES_SoundClose(void) {
	while (!dma_can_run());
	close_sound();
}

/* Sound Output 5 Waves - 2 Pulse, 1 Triangle, 1 Noise, 1 DPCM */
void InfoNES_SoundOutput(int samples, BYTE *wave1, BYTE *wave2, BYTE *wave3, BYTE *wave4, BYTE *wave5) {
	assert(2 * samples <= (sizeof(sound_buf)/2));
	for (int i = 0; i < samples; i++) {
		sound_buf[2 * i] = ( wave1[i] + wave2[i]  + wave4[i] + wave5[i] ) *10;
		sound_buf[2 * i + 1] = sound_buf[2 * i];
	}
	while (!dma_can_run());
	dma_set(sound_buf, samples*2*2);
	dma_start();

}

/* Print system message */
/*void InfoNES_MessageBox( char *pszMsg, ... );

{
	printf(pszMsg);
}
*/
