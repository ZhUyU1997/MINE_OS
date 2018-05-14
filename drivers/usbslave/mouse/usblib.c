#include <s3c24x0.h>
#include <interrupt.h>
#include <timer.h>
#include <assert.h>
#include "usb.h"
#include "2440usb.h"
int check_fifo_size(enum ENDPOINT ep, U32 size) {
	switch (ep) {
		case EP0:
			assert(size <= EP0_PKT_SIZE);
			break;
		case EP1:
			assert(size <= EP1_PKT_SIZE);
			break;
		case EP2:
			assert(size <= EP2_PKT_SIZE);
			break;
		case EP3:
			assert(size <= EP3_PKT_SIZE);
			break;
		case EP4:
			assert(size <= EP4_PKT_SIZE);
			break;
		default:
			assert(0);
			return 0;
			break;
	}
	return 1;
}
void ep_tx_fifo(enum ENDPOINT ep, U8 *buf, U32 size) {
	assert(buf && size);
	if (check_fifo_size(ep, size)) {
		STORE_INDEX(ep);
		for (U32 i = 0; i < size; i++)
			usbdevregs->fifo[ep].EP_FIFO_REG = buf[i];
		RESTORE_INDEX();
	}
}

void ep_rx_fifo(enum ENDPOINT ep, U8 *buf, U32 size) {
	assert(buf && size);
	if (check_fifo_size(ep, size)) {
		STORE_INDEX(ep);
		for (U32 i = 0; i < size; i++)
			buf[i] = usbdevregs->fifo[ep].EP_FIFO_REG;
		RESTORE_INDEX();
	}
}

U32 get_ep_fifo_size(enum ENDPOINT ep) {
	switch (ep) {
		case EP0:
			return EP0_PKT_SIZE;
		case EP1:
			return EP1_PKT_SIZE;
		case EP2:
			return EP2_PKT_SIZE;
		case EP3:
			return EP3_PKT_SIZE;
		case EP4:
			return EP4_PKT_SIZE;
		default:
			assert(0);
	}
}
void PrintPkt(U8 *buf, U32 size) {
	DbgPrintf("[RCV:");
	for (U32 i = 0; i < size; i++)
		DbgPrintf("%02x,", buf[i]);
	DbgPrintf("]");
}

void RdPktEp0(U8 *buf, U32 size) {
	ep_rx_fifo(EP0, buf, size);
}
void WrPktEp0(U8 *buf, U32 size) {
	ep_tx_fifo(EP0, buf, size);
}
void WrPktEp1(U8 *buf, U32 size) {
	ep_tx_fifo(EP1, buf, size);
}
U32 usb_buf_tx(struct usb_buf *ub, enum ENDPOINT ep) {
	STORE_INDEX(ep);
	U32 size = get_ep_fifo_size(ep);
	U32 reamin_size = usb_buf_remain(ub);
	U32 ret = 0;
	if (reamin_size > 0)
		ret = usb_buf_read_const(ub, ADDR_EP_FIFO(ep), (reamin_size >= size) ? size : reamin_size);
	RESTORE_INDEX();
	return ret;
}

#if USB_DEBUG == 1
void DbgPrintf(char *fmt, ...) {
	static char str[1024 * 24];
	static char *p = str;
	if (strcmp(fmt, "show") == 0) {
		for (int i = 0; i < strlen(str); i++)
			putc(str[i]);
		return;
	}
	if (p > &str[1024 * 24])
		return 0;
	va_list ap;
	va_start(ap, fmt);
	p += vsprintf(p, fmt, ap);
	va_end(ap);
}
#endif
