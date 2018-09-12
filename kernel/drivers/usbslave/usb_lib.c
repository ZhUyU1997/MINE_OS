#include <stdio.h>
#include <assert.h>
#include <s3c24x0.h>
#include <interrupt.h>
#include <timer.h>
#include <usb/2440usb.h>
struct usb_buf ub[4] = {0};

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
int check_ep(enum ENDPOINT ep) {
	switch (ep) {
		case EP0:
		case EP1:
		case EP2:
		case EP3:
		case EP4:
			return 1;
		default:
			assert(0);
			return 0;
	}
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

U32 ep_rx_fifo(enum ENDPOINT ep, U8 *buf, U32 size) {
	assert(buf && size);
	if (check_fifo_size(ep, size)) {
		STORE_INDEX(ep);
		U32 real_size = usbdevregs->OUT_FIFO_CNT1_REG
						| (usbdevregs->OUT_FIFO_CNT2_REG << 8);
		if (real_size > size)
			return 0;
		else
			size = real_size;
		for (U32 i = 0; i < size; i++)
			buf[i] = usbdevregs->fifo[ep].EP_FIFO_REG;
		RESTORE_INDEX();
	}
	//print_pkt(buf, size);
	return size;
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
void print_pkt(U8 *buf, U32 size) {
	DbgPrintf("[RCV:%d]\n", size);
	for (U32 i = 0; i < size; i++){
		DbgPrintf("%02x ", buf[i]);
		if(i%16==15)
			DbgPrintf("\n");
	}
	DbgPrintf("\n[END]\n");
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
/*
 * 每次发送完成，mcu 置位 IN_PKT_RDY(packet ready)
 * USB发送完成时,USB将IN_PKT_RDY清零
 * 如果是最后一个数据包，IN_PKT_RDY置位的同时还要将 DATA_END 置位
 */
void usb_buf_ep0_tx(struct usb_buf *ub) {
	U32 size = get_ep_fifo_size(EP0);
	U32 reamin_size = usb_buf_remain(ub);
	if(reamin_size > 0)
		usb_buf_tx(ub, EP0);
	if (reamin_size > size) {
		SET_EP0_IN_PKT_RDY();
	} else if (reamin_size == size) {
		SET_EP0_IN_PKT_RDY();
	} else if (reamin_size > 0) {
		ep0State = EP0_STATE_INIT;
		SET_EP0_INPKTRDY_DATAEND();
	} else {
		//如果描述符为EP0_PKT_SIZE整数倍，在发完所有数据后需要再发一个空数据包
		DbgPrintf("[NULL]");
		ep0State = EP0_STATE_INIT;
		SET_EP0_INPKTRDY_DATAEND();
	}
}
void usb_buf_epx_tx(enum ENDPOINT ep, struct usb_buf *ub) {
	U32 size = get_ep_fifo_size(ep);
	U32 reamin_size = usb_buf_remain(ub);
	if(reamin_size > 0)
		usb_buf_tx(ub, ep);
	if (reamin_size > 0 && reamin_size < size) {
		//ep0State = EP0_STATE_INIT;
	} else if (reamin_size <= 0) {
		//如果描述符为EP0_PKT_SIZE整数倍，在发完所有数据后需要再发一个空数据包
		DbgPrintf("[NULL]");
		//ep0State = EP0_STATE_INIT;
	}
	SET_EPX_IN_PKT_RDY();
}
void usb_send_init(enum ENDPOINT ep, U8 *buf, U32 size) {
	if(!check_ep(ep))
		return;
	usb_buf_init(&ub[ep], buf, size);
}
int usb_send_is_empty(enum ENDPOINT ep) {
	//TODO
	if(!check_ep(ep))
		return 1;
	return !usb_buf_remain(&ub[ep]);
}
void usb_send_message(enum ENDPOINT ep) {
	if(!check_ep(ep))
		return;
	STORE_INDEX(ep);
	switch (ep) {
		case EP0:
			WAIT_EP0_IN_PKT();
			usb_buf_ep0_tx(&ub[0]);
			break;
		case EP1:
		case EP2:
		case EP3:
		case EP4:
			WAIT_EPX_IN_PKT();
			usb_buf_epx_tx(ep, &ub[ep]);
			break;
		default:
			assert(0);
	}
	RESTORE_INDEX();
}
//TODO:
void usb_receive_message(enum ENDPOINT ep, U8 *buf, U32 size) {
	ep_rx_fifo(ep, buf, size);
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
