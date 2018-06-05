#include <timer.h>
#include "irda_raw.h"

#define NEXT_PLACE(i) ((i+1)&0x3FF)

static irda_raw_event g_events[1024];
static int g_r = 0;
static int g_w = 0;

static int is_ir_event_buf_empty(void) {
	return g_r == g_w;
}

static int is_ir_event_buf_full(void) {
	return NEXT_PLACE(g_w) == g_r;
}

int ir_event_put(p_irda_raw_event pd) {
	if (is_ir_event_buf_full())
		return -1;
	g_events[g_w] = *pd;
	g_w = NEXT_PLACE(g_w);
	return 0;
}

int ir_event_get(p_irda_raw_event pd) {
	if (is_ir_event_buf_empty())
		return -1;
	*pd = g_events[g_r];
	g_r = NEXT_PLACE(g_r);
	return 0;
}


int ir_event_get_timeout(p_irda_raw_event pd, int timeout_us) {
	unsigned long long start = get_system_time_us();

	while (get_system_time_us() - start < timeout_us) {
		if (!is_ir_event_buf_empty()) {
			*pd = g_events[g_r];
			g_r = NEXT_PLACE(g_r);
			return 0;
		}
	}
	return -1;
}


