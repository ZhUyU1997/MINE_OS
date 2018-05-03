#include <sys/types.h>
#include "mp3_buf.h"
unsigned short MP3_Data[FRAMEBUF_NUM][OUTPUT_BUFFER_SIZE];

struct MP3_FRAMEBUF_LINK {
	unsigned short *data;
	struct MP3_FRAMEBUF_LINK *next;
};
static struct MP3_FRAMEBUF_LINK MP3_LINK[FRAMEBUF_NUM];

static struct MP3_FRAMEBUF_LINK * volatile LINK = NULL;
static struct MP3_FRAMEBUF_LINK * volatile DMA_LINK = NULL;

static volatile int run_status = 0;
int handle_dma_finish() {
	if (run_status == 0)
		return 0;
	if (DMA_LINK->next == LINK) {
		DMA_LINK = DMA_LINK->next;
		run_status = 0;
		return 0;
	}
	DMA_LINK = DMA_LINK->next;
	dma_set(DMA_LINK->data, sizeof(MP3_Data[0]));
	dma_start();
	return 1;
}
void framebuf_init() {
	for (int i = 0; i < FRAMEBUF_NUM; i++) {
		MP3_LINK[i].data = MP3_Data[i];
		MP3_LINK[i].next = &MP3_LINK[(i + 1) % (FRAMEBUF_NUM)];
	}
	LINK = NULL;
	DMA_LINK = NULL;
	run_status = 0;
}
void framebuf_exit() {
	run_status = 0;
}
void framebuf_start() {
	if (run_status == 0) {
		dma_init();
		set_dma_end_func(handle_dma_finish);
		dma_set(DMA_LINK->data, sizeof(MP3_Data[0]));
		dma_start();
		run_status = 1;
	}
}
int get_framebuf_status() {
	if (LINK->next == DMA_LINK) {
		return 1;
	} else {
		return 0;
	}
}
unsigned short *get_next_framebuf() {
	if (LINK == NULL) {
		LINK = MP3_LINK;
		return LINK->data;
	}
	//TODO:
	while (get_framebuf_status());

	if (run_status == 0) {
		if (DMA_LINK == NULL)
			DMA_LINK = MP3_LINK;
		framebuf_start();
	}


	LINK = LINK->next;
	return LINK->data;
}
unsigned short *get_current_framebuf() {
	if (LINK == NULL) {
		return NULL;
	}
	return LINK->data;
}


