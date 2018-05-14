#include <s3c24x0.h>
#include <assert.h>
#include <usb/ch9.h>
#include "2440usb.h"

#define DES_TX_COUNT (usb_buf_count(&ub)/EP_PKT_SIZE(0))
#define USB_BUF_INIT(x) usb_buf_init(&ub, &(x), sizeof(x));ep0State = EP0_STATE_TRANSMIT

//TODO:SUSPENDED状态未处理
enum USB_DEV_STATE usbd_state;

enum EP0_STATE ep0State = EP0_STATE_INIT;

static struct usb_buf ub;

struct status {
	struct USB_CONFIGURATION_GET {
		U8 ConfigurationValue;
	} ConfigGet;
	struct USB_INTERFACE_GET {
		U8 AlternateSetting;
	} InterfaceGet;

	union {
		struct {
			U8 self_powered: 1;
			U8 remote_wakeup: 1;
		};
		U16 device;
	};

	U16 interface;
	U16 endpoint0;
	U16 endpoint1;
};
struct status g_status = {
	.remote_wakeup = 1,
	.self_powered = 1,
	.device = 3,
	.interface = 0,
	.endpoint0 = 0,
	.endpoint1 = 0
};
volatile int isUsbdSetConfiguration;

void handle_standard_input(struct usb_ctrlrequest ctrlreq) {
	switch (ctrlreq.bRequest) {
		case USB_REQ_GET_STATUS:
			DbgPrintf("[获取状态]");
			switch (ctrlreq.bRequestType & USB_RECIP_MASK) {
				case USB_RECIP_DEVICE:
					DbgPrintf("[设备]");
					USB_BUF_INIT(g_status.device);
					break;
				case USB_RECIP_INTERFACE:
					DbgPrintf("[接口]");
					g_status.interface = 0;
					USB_BUF_INIT(g_status.interface);
					break;
				case USB_RECIP_ENDPOINT:
					if (((ctrlreq.wIndex & 0xff) & 0x7f) == 0x00) {
						DbgPrintf("[端点0]");
						USB_BUF_INIT(g_status.endpoint0);
					} else if (((ctrlreq.wIndex & 0xff) & 0x8f) == 0x81) {       // IN  Endpoint 1
						DbgPrintf("[端点1]");
						USB_BUF_INIT(g_status.endpoint1);
					} else {
						DbgPrintf("[ERROR04]");
					}
					break;
				case USB_RECIP_OTHER:
				default:
					DbgPrintf("[ERROR03]");
					break;
			}
			CLR_EP0_OUT_PKT_RDY();
			break;
		case USB_REQ_GET_DESCRIPTOR:
			switch (ctrlreq.wValue >> 8) {
				case USB_DT_DEVICE:
					DbgPrintf("[获取设备描述符]");
					USB_BUF_INIT(g_usb_dev_desc);
					break;
				case USB_DT_CONFIG:
					DbgPrintf("[获取配置描述符]");
					if (ctrlreq.wLength > 0x9) {
						USB_BUF_INIT(g_config_all);
					} else {
						USB_BUF_INIT(g_config_all.g_config_desc);
					}
					break;
				case USB_DT_STRING:
					DbgPrintf("[获取字串描述符]");
					switch (ctrlreq.wValue & 0xff) {
						case 0:
							USB_BUF_INIT(g_string_desc0);
							break;
						case 1:
							USB_BUF_INIT(g_string_desc1);
							break;
						case 2:
							USB_BUF_INIT(g_string_desc2);
							break;
						case 3:
							USB_BUF_INIT(g_string_desc3);
							break;
						default:
							DbgPrintf("[UE:STRI?]");
							break;
					}
					break;
				case USB_DT_INTERFACE:
					DbgPrintf("[获取接口描述符]");
					USB_BUF_INIT(g_config_all.g_interface_desc);
					break;
				case USB_DT_ENDPOINT:
					DbgPrintf("[获取端点描述符]");
					//TODO:考虑方向
					switch (ctrlreq.wValue & 0xff) {
						case 0x81:
							USB_BUF_INIT(g_config_all.g_endpoint_desc);
							break;
						default:
							DbgPrintf("[未定义端点]");
							break;
					}
					break;
				case USB_DT_RPIPE:
					DbgPrintf("[获取报告描述符]");
					USB_BUF_INIT(gHIDReportDescriptor);
					break;
				default:
					DbgPrintf("[未定义描述符]");
					break;
			}
			CLR_EP0_OUT_PKT_RDY();
			break;
		case USB_REQ_GET_CONFIGURATION:
			DbgPrintf("[获取配置]");
			CLR_EP0_OUT_PKT_RDY();
			USB_BUF_INIT(g_status.ConfigGet);
			break;
		case USB_REQ_GET_INTERFACE:
			DbgPrintf("[获取接口]");
			CLR_EP0_OUT_PKT_RDY();
			USB_BUF_INIT(g_status.InterfaceGet);
			break;
		case USB_REQ_SYNCH_FRAME:
			DbgPrintf("[同步帧]");
			CLR_EP0_OUT_PKT_RDY();
			ep0State = EP0_STATE_INIT;
			break;
		default:
			DbgPrintf("[未定义的标准输入请求 SETUP=%x]", ctrlreq.bRequest);
			CLR_EP0_OUTPKTRDY_DATAEND(); //Because of no data control transfers.
			ep0State = EP0_STATE_INIT;
			break;
	}
}

void handle_standard_output(struct usb_ctrlrequest ctrlreq) {
	switch (ctrlreq.bRequest) {
		case USB_REQ_CLEAR_FEATURE:
			//TODO:
			DbgPrintf("[清除特性]");
			switch (ctrlreq.bRequestType & USB_RECIP_MASK) {
				case USB_RECIP_DEVICE:
					if ((ctrlreq.wIndex & 0xff) == 1)
						g_status.remote_wakeup = FALSE;
					break;
				case USB_RECIP_ENDPOINT:
					if ((ctrlreq.wValue & 0xff) == 0) {
						if (((ctrlreq.wIndex & 0xff) & 0x7f) == 0x00) {
							g_status.endpoint0 = 0;
						} else if (((ctrlreq.wIndex & 0xff) & 0x8f) == 0x81) {       // IN  Endpoint 1
							g_status.endpoint1 = 0;
						} else {
							DbgPrintf("[ERROR01]");
						}
					}
					break;
				case USB_RECIP_INTERFACE:
				case USB_RECIP_OTHER:
				default:
					DbgPrintf("[ERROR01]");
					break;
			}
			break;
		case USB_REQ_SET_FEATURE:
			DbgPrintf("[设置特性]");
			switch (ctrlreq.bRequestType & USB_RECIP_MASK) {
				case USB_RECIP_DEVICE:
					if ((ctrlreq.wValue & 0xff) == 1)
						g_status.remote_wakeup = TRUE;
					break;
				case USB_RECIP_ENDPOINT:
					if ((ctrlreq.wValue & 0xff) == 0) {
						if (((ctrlreq.wIndex & 0xff) & 0x7f) == 0x00) {
							g_status.endpoint0 = 1;
						}
						if (((ctrlreq.wIndex & 0xff) & 0x8f) == 0x81) {
							g_status.endpoint1 = 1;
						} else {
							DbgPrintf("[ERROR01]");
						}
					}
					break;
				default:
					DbgPrintf("[ERROR02]");
					break;
			}
			break;
		case USB_REQ_SET_ADDRESS:
			DbgPrintf("[设置地址:%d]", ctrlreq.wValue & 0xff);
			usbdevregs->FUNC_ADDR_REG = (ctrlreq.wValue & 0xff) | 0x80;
			usbd_state = USBD_STATE_ADDRESS;
			break;
		case USB_REQ_SET_DESCRIPTOR:
			DbgPrintf("[设置描述符]");
			break;
		case USB_REQ_SET_CONFIGURATION:
			DbgPrintf("[设置配置]");
			g_status.ConfigGet.ConfigurationValue = ctrlreq.wValue & 0xff;
			usbd_state = USBD_STATE_CONFIGURED;
			break;
		case USB_REQ_SET_INTERFACE:
			DbgPrintf("[设置接口]");
			g_status.InterfaceGet.AlternateSetting = ctrlreq.wValue & 0xff;
			break;
		default:
			DbgPrintf("[未定义的标准输出请求 SETUP=%x]", ctrlreq.bRequest);
			break;
	}
	CLR_EP0_OUTPKTRDY_DATAEND();
	ep0State = EP0_STATE_INIT;
}


void Ep0Handler(void) {
	SET_INDEX(0);
	U8 ep0_csr = usbdevregs->EP0_CSR;
	struct usb_ctrlrequest ctrlreq;
	if (ep0_csr & EP0_SENT_STALL) {
		DbgPrintf("[STALL]");
		FLUSH_EP0_FIFO();
		CLR_EP0_SENT_STALL();
		CLR_EP0_OUT_PKT_RDY();
		ep0State = EP0_STATE_INIT;
		return;
	} else if (ep0_csr & EP0_SETUP_END) {
		// Host may end GET_DESCRIPTOR operation without completing the IN data stage.
		// If host does that, SETUP_END bit will be set.
		// OUT_PKT_RDY has to be also cleared because status stage sets OUT_PKT_RDY to 1.
		DbgPrintf("[SETUP END]");
		FLUSH_EP0_FIFO();
		CLR_EP0_SETUP_END();
		CLR_EP0_OUT_PKT_RDY();
		ep0State = EP0_STATE_INIT;
		return;
	}

	if ((ep0_csr & EP0_OUT_PKT_READY)) {
		RdPktEp0((U8 *)&ctrlreq, sizeof(struct usb_ctrlrequest));
		//PrintEp0Pkt((U8 *)(&ctrlreq));
		//判断请求类型
		if ((ctrlreq.bRequestType & 0x80) == USB_DIR_OUT) {
			switch ((ctrlreq.bRequestType) & USB_TYPE_MASK) {
				case USB_TYPE_STANDARD:
					handle_standard_output(ctrlreq);
					break;
				case USB_TYPE_CLASS:
					handle_hid_class(ctrlreq);
					break;
				case USB_TYPE_VENDOR:
					DbgPrintf("[USB厂商输出请求]");
					break;
				default:
					DbgPrintf("[错误：未定义的输出请求]");
					break;
			}
		} else {
			switch ((ctrlreq.bRequestType) & USB_TYPE_MASK) {
				case USB_TYPE_STANDARD:
					handle_standard_input(ctrlreq);
					break;
				case USB_TYPE_CLASS:
					handle_hid_class(ctrlreq);
					break;
				case USB_TYPE_VENDOR:
					DbgPrintf("[USB厂商输入请求]");
					break;
				default:
					DbgPrintf("[错误：未定义的输入请求]");
					break;
			}
		}
	}
	switch (ep0State) {
		case EP0_STATE_INIT:
			break;
		case EP0_STATE_TRANSMIT:
			DbgPrintf("[获取(%d)]", DES_TX_COUNT);
			usb_buf_ep0_tx(&ub);
			break;
		default:
			DbgPrintf("[未定义状态]");
			break;
	}
}
/*
 * 每次发送完成，mcu 置位 IN_PKT_RDY(packet ready)
 * USB发送完成时,USB将IN_PKT_RDY清零
 * 如果是最后一个数据包，IN_PKT_RDY置位的同时还要将 DATA_END 置位
 */
void usb_buf_ep0_tx(struct usb_buf *ub) {
	U32 size = get_ep_fifo_size(EP0);
	U32 reamin_size = usb_buf_remain(ub);
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

void IsrUsbd(void) {
	U8 usbdIntpnd = usbdevregs->USB_INT_REG;
	U8 epIntpnd = usbdevregs->EP_INT_REG;
	DbgPrintf("#", epIntpnd, usbdIntpnd);
	if (usbdIntpnd & SUSPEND_INT) {
		usbdevregs->USB_INT_REG = SUSPEND_INT;
		DbgPrintf("[SUS]");
	}
	if (usbdIntpnd & RESUME_INT) {
		usbdevregs->USB_INT_REG = RESUME_INT;
		DbgPrintf("[RSM]");
	}
	if (usbdIntpnd & RESET_INT) {
		DbgPrintf("[RST]");
		ReconfigUsbd();
		usbd_state = USBD_STATE_DEFAULT;
		//RESET_INT should be cleared after ResetUsbd().
		usbdevregs->USB_INT_REG = RESET_INT;
	}
	if (epIntpnd & EP0_INT) {
		usbdevregs->EP_INT_REG = EP0_INT;
		SET_INDEX(0);
		Ep0Handler();
	}
	if (epIntpnd & EP1_INT) {
		usbdevregs->EP_INT_REG = EP1_INT;
		DbgPrintf("[INT:EP1]");
		SET_INDEX(1);
		Ep1Handler();
	}
	DbgPrintf("\n");
}
