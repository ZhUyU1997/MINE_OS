#include <s3c24x0.h>
#include <assert.h>
#include <usb/ch9.h>
#include <usb/2440usb.h>

//TODO:端点处理不够通用

#define DES_TX_COUNT (usb_buf_count(&ub[0])/EP_PKT_SIZE(0))
#define USB_BUF_INIT(x) usb_buf_init(&ub[0], (x), sizeof(*(x)));ep0State = EP0_STATE_TRANSMIT
//TODO：注意ctrlreq->wLength可能小于发送的数据
#define USB_BUF_INIT_TRUNCATION(x) usb_buf_init(&ub[0], (x), ctrlreq.wLength > sizeof(*(x)) ? sizeof(*(x)) : ctrlreq.wLength);ep0State = EP0_STATE_TRANSMIT
#define USB_BUF_INIT_SIZE(x,size) usb_buf_init(&ub[0], (x), size);ep0State = EP0_STATE_TRANSMIT
#define USB_BUF_INIT_TRUNCATION_SIZE(x,size) usb_buf_init(&ub[0], (x), ctrlreq.wLength > (size) ? (size) : ctrlreq.wLength);ep0State = EP0_STATE_TRANSMIT

//TODO:SUSPENDED状态未处理
enum USB_DEV_STATE usbd_state = USBD_STATE_POWERED;

enum EP0_STATE ep0State = EP0_STATE_INIT;

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
	U16 endpoint[4];
};
struct status g_status = {
	.remote_wakeup = 1,
	.self_powered = 1,
	.device = 3,
	.interface = 0,
	.endpoint[0] = 0,
	.endpoint[1] = 0,
	.endpoint[2] = 0,
	.endpoint[3] = 0,
};

void handle_standard_input(struct usb_ctrlrequest ctrlreq) {
	switch (ctrlreq.bRequest) {
		case USB_REQ_GET_STATUS:
			DbgPrintf("[获取状态]");
			switch (ctrlreq.bRequestType & USB_RECIP_MASK) {
				case USB_RECIP_DEVICE:
					DbgPrintf("[设备]");
					USB_BUF_INIT(&g_status.device);
					break;
				case USB_RECIP_INTERFACE:
					DbgPrintf("[接口]");
					g_status.interface = 0;
					USB_BUF_INIT(&g_status.interface);
					break;
				case USB_RECIP_ENDPOINT:
					switch (ctrlreq.wIndex & 0xff) {
						case 0x00:
							DbgPrintf("[端点0]");
							USB_BUF_INIT(&g_status.endpoint[0]);
							break;
						case 0x81:
							DbgPrintf("[端点1]");
							assert(usbdev.endpoint_desc[1-1]);
							USB_BUF_INIT(&g_status.endpoint[1]);
							break;
						case 0x82:
							DbgPrintf("[端点2]");
							assert(usbdev.endpoint_desc[2-1]);
							USB_BUF_INIT(&g_status.endpoint[2]);
							break;
						case 0x03:
							DbgPrintf("[端点3]");
							assert(usbdev.endpoint_desc[3-1]);
							USB_BUF_INIT(&g_status.endpoint[3]);
							break;
						default:
							DbgPrintf("[未定义端点]");
							break;
					}
					break;
				case USB_RECIP_OTHER:
				default:
					DbgPrintf("[ERROR]");
					break;
			}
			CLR_EP0_OUT_PKT_RDY();
			break;
		case USB_REQ_GET_DESCRIPTOR:
			CLR_EP0_OUT_PKT_RDY();
			switch (ctrlreq.wValue >> 8) {
				case USB_DT_DEVICE:
					DbgPrintf("[获取设备描述符]");
					if(usbd_state == USBD_STATE_DEFAULT){
						ep_tx_fifo(EP0, usbdev.dev_desc, 8);
						SET_EP0_INPKTRDY_DATAEND();
					}else{
						USB_BUF_INIT(usbdev.dev_desc);
					}
					break;
				case USB_DT_CONFIG:
					DbgPrintf("[获取配置描述符]");
					if (ctrlreq.wLength > 0x9) {
						USB_BUF_INIT_SIZE(usbdev.config_all, usbdev.config_all_size);
					} else {
						USB_BUF_INIT(usbdev.config_desc);
					}
					break;
				case USB_DT_STRING:
					DbgPrintf("[获取字串描述符]");
					switch (ctrlreq.wValue & 0xff) {
						case 0:
						case 1:
						case 2:
						case 3:
						{
							int index = ctrlreq.wValue & 0xff;
							assert(usbdev.string_desc[index]);
							USB_BUF_INIT_TRUNCATION_SIZE(usbdev.string_desc[index],usbdev.string_desc_size[index]);
							break;
						}
						default:
							DbgPrintf("[UE:STRI?]");
							ep0State = EP0_STATE_INIT;
							break;
					}
					break;
				case USB_DT_INTERFACE:
					DbgPrintf("[获取接口描述符]");
					USB_BUF_INIT(usbdev.interface_desc);
					break;
				case USB_DT_ENDPOINT:
					DbgPrintf("[获取端点描述符]");
					//TODO:考虑方向
					switch (ctrlreq.wValue & 0xff) {
						case 0x81:
							assert(usbdev.endpoint_desc[1-1]);
							USB_BUF_INIT(usbdev.endpoint_desc[1-1]);
							break;
						case 0x82:
							assert(usbdev.endpoint_desc[2-1]);
							USB_BUF_INIT(usbdev.endpoint_desc[2-1]);
							break;
						case 0x03:
							assert(usbdev.endpoint_desc[3-1]);
							USB_BUF_INIT(usbdev.endpoint_desc[3-1]);
							break;
						default:
							DbgPrintf("[未定义端点]");
							break;
					}
					break;
				case USB_DT_RPIPE:
					DbgPrintf("[获取报告描述符]");
					assert(usbdev.report_desc);
					USB_BUF_INIT_SIZE(usbdev.report_desc, usbdev.report_desc_size);
					break;
				default:
					DbgPrintf("[未定义描述符]");
					break;
			}
			break;
		case USB_REQ_GET_CONFIGURATION:
			DbgPrintf("[获取配置]");
			CLR_EP0_OUT_PKT_RDY();
			USB_BUF_INIT(&g_status.ConfigGet);
			break;
		case USB_REQ_GET_INTERFACE:
			DbgPrintf("[获取接口]");
			CLR_EP0_OUT_PKT_RDY();
			USB_BUF_INIT(&g_status.InterfaceGet);
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
						switch (ctrlreq.wIndex & 0xff) {
							case 0x00:
								g_status.endpoint[0] = 0;
								break;
							case 0x81:
								assert(usbdev.endpoint_desc[1-1]);
								g_status.endpoint[1] = 0;
								break;
							case 0x83:
								assert(usbdev.endpoint_desc[2-1]);
								g_status.endpoint[2] = 0;
								break;
							case 0x03:
								assert(usbdev.endpoint_desc[3-1]);
								g_status.endpoint[3] = 0;
								break;
							default:
								DbgPrintf("[ERROR]");
								break;
						}
					}
					break;
				case USB_RECIP_INTERFACE:
				case USB_RECIP_OTHER:
				default:
					DbgPrintf("[ERROR]");
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
						switch (ctrlreq.wIndex & 0xff) {
							case 0x00:
								g_status.endpoint[0] = 1;
								break;
							case 0x81:
								assert(usbdev.endpoint_desc[1-1]);
								g_status.endpoint[1] = 1;
								break;
							case 0x83:
								assert(usbdev.endpoint_desc[2-1]);
								g_status.endpoint[2] = 1;
								break;
							case 0x03:
								assert(usbdev.endpoint_desc[3-1]);
								g_status.endpoint[3] = 1;
								break;
							default:
								DbgPrintf("[ERROR]");
								break;
						}
					}
					break;
				default:
					DbgPrintf("[ERROR]");
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

int epx_status_common(int direction) {
	int status;
	if (direction == USB_DIR_OUT) {
		status = usbdevregs->OUT_CSR1_REG;
		if (status & EPO_SENT_STALL) {
			DbgPrintf("[SENT STALL]");
			CLR_EPX_OUT_SENT_STALL();
			return -1;
		}
	} else if (direction == USB_DIR_IN) {
		status = usbdevregs->IN_CSR1_REG;
		if (status & EPI_SENT_STALL) {
			DbgPrintf("[SENT STALL]");
			CLR_EPX_IN_SENT_STALL();
			return -1;
		}
	}
	return 0;
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
		usb_receive_message(EP0, &ctrlreq, sizeof(struct usb_ctrlrequest));
		//判断请求类型
		if ((ctrlreq.bRequestType & 0x80) == USB_DIR_OUT) {
			switch ((ctrlreq.bRequestType) & USB_TYPE_MASK) {
				case USB_TYPE_STANDARD:
					handle_standard_output(ctrlreq);
					break;
				case USB_TYPE_CLASS:
					DbgPrintf("[类输出请求]");
					assert(usbdev.handle_class);
					usbdev.handle_class(ctrlreq);
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
					DbgPrintf("[类输入请求]");
					assert(usbdev.handle_class);
					usbdev.handle_class(ctrlreq);
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
			usb_send_message(EP0);
			break;
		default:
			DbgPrintf("[未定义状态]");
			break;
	}
}

void handle_epx_irq(U8 epIntpnd){
	if (epIntpnd & EP1_INT) {
		usbdevregs->EP_INT_REG = EP1_INT;
		DbgPrintf("[EP1]");
		SET_INDEX(1);
		assert(usbdev.ep_handler[1-1]);
		usbdev.ep_handler[1-1]();
	}else if (epIntpnd & EP2_INT) {
		usbdevregs->EP_INT_REG = EP2_INT;
		DbgPrintf("[EP2]");
		SET_INDEX(2);
		assert(usbdev.ep_handler[2-1]);
		usbdev.ep_handler[2-1]();
	}else if (epIntpnd & EP3_INT) {
		usbdevregs->EP_INT_REG = EP3_INT;
		DbgPrintf("[EP3]");
		SET_INDEX(3);
		assert(usbdev.ep_handler[3-1]);
		usbdev.ep_handler[3-1]();
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
		assert(usbdev.reset);
		usbdev.reset();
		usbd_state = USBD_STATE_DEFAULT;
		//RESET_INT should be cleared after ResetUsbd().
		usbdevregs->USB_INT_REG = RESET_INT;
	}
	if (epIntpnd & EP0_INT) {
		usbdevregs->EP_INT_REG = EP0_INT;
		SET_INDEX(0);
		DbgPrintf("[EP0]");
		Ep0Handler();
	}
	//TODO
	if (epIntpnd & (EP1_INT | EP2_INT | EP3_INT)) {
		handle_epx_irq(epIntpnd);
	}
	DbgPrintf("\n");
}
