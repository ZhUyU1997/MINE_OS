#ifndef __24XUSB_H__
#define __24XUSB_H__
#include <types.h>
#include <s3c24x0.h>
#include <usb/ch9.h>
/*
 * USB definitons
 */

/* Endpoint*/
enum ENDPOINT
{
	EP0 = 0,
	EP1,
	EP2,
	EP3,
	EP4,
};

enum USB_DEV_STATE{
	USBD_STATE_POWERED,
	USBD_STATE_DEFAULT,
	USBD_STATE_ADDRESS,
	USBD_STATE_CONFIGURED,
	USBD_STATE_SUSPENDED,
};

enum EP0_STATE {
	EP0_STATE_INIT,
	EP0_STATE_TRANSMIT,
};

/* usb buf */
struct usb_buf {
	u8_t *buf;
	u32_t size;
	u32_t count;
};


struct usbdev_struct {
	struct usb_device_descriptor *dev_desc;
	struct usb_config_descriptor *config_desc;
	struct usb_interface_descriptor *interface_desc;
	struct usb_hid_descriptor *hid_desc;
	struct usb_endpoint_descriptor *endpoint_desc[4];
	char *string_desc[4];
	u32_t string_desc_size[4];
	u8_t *config_all;
	u32_t config_all_size;
	u8_t *report_desc;
	u32_t report_desc_size;
	void (*ep_handler[4])();
	void (*reset)();
	void (*handle_class)(struct usb_ctrlrequest ctrlreq);
};

#ifndef S32
typedef int S32;
#endif

#ifndef S16
typedef short int S16;
#endif

#ifndef S8
typedef char S8;
#endif

#define TRUE 						(1)
#define FALSE 						(0)

/* Feature Selectors */
#define DEVICE_REMOTE_WAKEUP        (1)
#define EP_STALL                    (0)

/* Power Management Register */
#define DISABLE_SUSPEND				0x00
#define ENABLE_SUSPEND				0x01
#define SUSPEND_MODE				0x02
#define MCU_RESUME					0x04
#define ISO_UPDATE		 			(1<<7)

/* MAXP Register */
#define FIFO_SIZE_0              	0x00  /* 0x00 * 8 = 0  */
#define FIFO_SIZE_8              	0x01  /* 0x01 * 8 = 8  */
#define FIFO_SIZE_16             	0x02  /* 0x02 * 8 = 16 */
#define FIFO_SIZE_32             	0x04  /* 0x04 * 8 = 32 */
#define FIFO_SIZE_64             	0x08  /* 0x08 * 8 = 64 */

/* ENDPOINT0 CSR (Control Status Register) : Mapped to IN CSR1 */
#define EP0_OUT_PKT_READY        	0x01  /* USB sets, MCU clears by setting SERVICED_OUT_PKT_RDY */
#define EP0_IN_PKT_READY         	0x02  /* MCU sets, USB clears after sending FIFO */
#define EP0_SENT_STALL           	0x04  /* USB sets */
#define EP0_DATA_END            	0x08  /* MCU sets */
#define EP0_SETUP_END            	0x10  /* USB sets, MCU clears by setting SERVICED_SETUP_END */
#define EP0_SEND_STALL           	0x20  /* MCU sets */
#define EP0_SERVICED_OUT_PKT_RDY 	0x40  /* MCU writes 1 to clear OUT_PKT_READY */
#define EP0_SERVICED_SETUP_END 		0x80  /* MCU writes 1 to clear SETUP_END */

#define EP0_WR_BITS    				0xc0

//EP_INT_REG / EP_INT_EN_REG
#define EP0_INT             		0x01  // Endpoint 0
#define EP1_INT						0x02  // Endpoint 1
#define EP2_INT						0x04  // Endpoint 2 
#define EP3_INT			 			0x08  // Endpoint 3 
#define EP4_INT			 			0x10  // Endpoint 4

//USB_INT_REG / USB_INT_EN_REG
#define SUSPEND_INT            	 	0x01
#define RESUME_INT               	0x02
#define RESET_INT                	0x04

//IN_CSR1
#define EPI_IN_PKT_READY         	0x01
#define EPI_UNDER_RUN		 		0x04
#define EPI_FIFO_FLUSH		 		0x08
#define EPI_SEND_STALL         		0x10
#define EPI_SENT_STALL         		0x20
#define EPI_CDT			 			0x40
#define EPI_WR_BITS              	(EPI_FIFO_FLUSH|EPI_IN_PKT_READY|EPI_CDT)
//(EPI_FIFO_FLUSH) is preferred  (???)
//IN_CSR2
#define EPI_IN_DMA_INT_MASK			(1<<4)
#define EPI_MODE_IN					(1<<5)
#define EPI_MODE_OUT				(0<<5)
#define EPI_ISO						(1<<6)
#define EPI_BULK					(0<<6)
#define EPI_AUTO_SET				(1<<7)

//OUT_CSR1
#define EPO_OUT_PKT_READY        	0x01
#define EPO_OVER_RUN		 		0x04
#define EPO_DATA_ERROR		 		0x08
#define EPO_FIFO_FLUSH		 		0x10
#define EPO_SEND_STALL           	0x20
#define EPO_SENT_STALL           	0x40
#define EPO_CDT			 			0x80
#define EPO_WR_BITS              	(EPO_FIFO_FLUSH|EPO_SEND_STALL|EPO_CDT)
//(EPO_FIFO_FLUSH) is preferred (???)

//OUT_CSR2
#define EPO_OUT_DMA_INT_MASK		(1<<5)
#define EPO_ISO		 				(1<<6)
#define EPO_BULK	 				(0<<6)
#define EPO_AUTO_CLR				(1<<7)

//USB DMA control register
#define UDMA_IN_RUN_OB				(1<<7)
#define UDMA_IGNORE_TTC				(1<<7)
#define UDMA_DEMAND_MODE			(1<<3)
#define UDMA_OUT_RUN_OB				(1<<2)
#define UDMA_OUT_DMA_RUN			(1<<2)
#define UDMA_IN_DMA_RUN				(1<<1)
#define UDMA_DMA_MODE_EN			(1<<0)

#define rEP1_DMA_TTC				(usbdevregs->ep1.EP_DMA_TTC_L+(usbdevregs->ep1.EP_DMA_TTC_M<<8)+(usbdevregs->ep1.EP_DMA_TTC_H<<16))
#define rEP2_DMA_TTC				(usbdevregs->ep2.EP_DMA_TTC_L+(usbdevregs->ep2.EP_DMA_TTC_M<<8)+(usbdevregs->ep2.EP_DMA_TTC_H<<16))
#define rEP3_DMA_TTC				(usbdevregs->ep3.EP_DMA_TTC_L+(usbdevregs->ep3.EP_DMA_TTC_M<<8)+(usbdevregs->ep3.EP_DMA_TTC_H<<16))
#define rEP4_DMA_TTC				(usbdevregs->ep4.EP_DMA_TTC_L+(usbdevregs->ep4.EP_DMA_TTC_M<<8)+(usbdevregs->ep4.EP_DMA_TTC_H<<16))

//If you chane the packet size, the source code should be changed!!!
//USB Device Options
#define USBDMA						FALSE
#define USBDMA_DEMAND				FALSE
#define EP_CAT(x,y)					EP_CAT_I(x,y)
#define EP_CAT_I(x,y)				x##y
#define EP_PKT_SIZE(ep)				EP_CAT(EP_CAT(EP,ep),_PKT_SIZE)
#define EP0_PKT_SIZE				8
#define EP1_PKT_SIZE				64
#define EP2_PKT_SIZE				64
#define EP3_PKT_SIZE				64
#define EP4_PKT_SIZE				64

#define ADDR_EP_FIFO(ep)			(&(usbdevregs->fifo[ep].EP_FIFO_REG))
#define STORE_INDEX(ep)				int index = usbdevregs->INDEX_REG;SET_INDEX(ep)
#define RESTORE_INDEX()				usbdevregs->INDEX_REG= index
#define SET_INDEX(ep)				usbdevregs->INDEX_REG = ep
#define PWR_REG_DEFAULT_VALUE		(DISABLE_SUSPEND)


#define CLR_EP0_OUT_PKT_RDY() 		usbdevregs->EP0_CSR =(((usbdevregs->EP0_CSR) & (~EP0_WR_BITS)) | EP0_SERVICED_OUT_PKT_RDY)
#define CLR_EP0_OUTPKTRDY_DATAEND() usbdevregs->EP0_CSR =(((usbdevregs->EP0_CSR) & (~EP0_WR_BITS)) | (EP0_SERVICED_OUT_PKT_RDY|EP0_DATA_END))
#define SET_EP0_IN_PKT_RDY() 		usbdevregs->EP0_CSR =(((usbdevregs->EP0_CSR) & (~EP0_WR_BITS)) | (EP0_IN_PKT_READY))
#define SET_EP0_INPKTRDY_DATAEND() 	usbdevregs->EP0_CSR =(((usbdevregs->EP0_CSR) & (~EP0_WR_BITS)) | (EP0_IN_PKT_READY|EP0_DATA_END))
#define CLR_EP0_SETUP_END() 		usbdevregs->EP0_CSR =(((usbdevregs->EP0_CSR) & (~EP0_WR_BITS)) | (EP0_SERVICED_SETUP_END))
#define CLR_EP0_SENT_STALL() 		usbdevregs->EP0_CSR =(((usbdevregs->EP0_CSR) & (~EP0_WR_BITS)) & (~EP0_SENT_STALL))
#define FLUSH_EP0_FIFO() 			{while(usbdevregs->OUT_FIFO_CNT1_REG) usbdevregs->fifo[0].EP_FIFO_REG;}
#define WAIT_EP0_IN_PKT()			while (usbdevregs->EP0_CSR & EP0_IN_PKT_READY);

#define SET_EPX_IN_PKT_RDY()		usbdevregs->IN_CSR1_REG = (((usbdevregs->IN_CSR1_REG) & (~EPI_WR_BITS)) | EPI_IN_PKT_READY)
#define SET_EPX_IN_SEND_STALL()		usbdevregs->IN_CSR1_REG = (((usbdevregs->IN_CSR1_REG) & (~EPI_WR_BITS)) | EPI_SEND_STALL)
#define CLR_EPX_IN_SENT_STALL()		usbdevregs->IN_CSR1_REG = (((usbdevregs->IN_CSR1_REG) & (~EPI_WR_BITS)) & (~EPI_SENT_STALL))
#define FLUSH_EPX_IN_FIFO()			usbdevregs->IN_CSR1_REG = (((usbdevregs->IN_CSR1_REG) & (~EPI_WR_BITS)) | EPI_FIFO_FLUSH)
#define WAIT_EPX_IN_PKT()			while (usbdevregs->IN_CSR1_REG & EPI_IN_PKT_READY);

#define CLR_EPX_OUT_PKT_RDY() 		usbdevregs->OUT_CSR1_REG = (((usbdevregs->OUT_CSR1_REG) & (~EPO_WR_BITS)) & (~EPO_OUT_PKT_READY))
#define SET_EPX_OUT_SEND_STALL()	usbdevregs->OUT_CSR1_REG = (((usbdevregs->OUT_CSR1_REG) & (~EPO_WR_BITS)) | EPO_SEND_STALL)
#define CLR_EPX_OUT_SENT_STALL()	usbdevregs->OUT_CSR1_REG = (((usbdevregs->OUT_CSR1_REG) & (~EPO_WR_BITS)) & (~EPO_SENT_STALL))
#define FLUSH_EPX_OUT_FIFO()		usbdevregs->OUT_CSR1_REG = (((usbdevregs->OUT_CSR1_REG) & (~EPO_WR_BITS)) | EPO_FIFO_FLUSH) 

u32_t get_ep_fifo_size(enum ENDPOINT ep);
u32_t usb_buf_remain(struct usb_buf *ub);
u32_t usb_buf_count(struct usb_buf *ub);
void usb_buf_init(struct usb_buf *ub, u8_t *buf, u32_t size);

u32_t usb_buf_tx(struct usb_buf *ub, enum ENDPOINT ep);
void usb_buf_ep0_tx(struct usb_buf *ub);

void ep_tx_fifo(enum ENDPOINT ep, u8_t *buf, u32_t size);
u32_t ep_rx_fifo(enum ENDPOINT ep, u8_t *buf, u32_t size);

void usb_send_init(enum ENDPOINT ep, u8_t *buf, u32_t size);
void usb_send_message(enum ENDPOINT ep);
void usb_receive_message(enum ENDPOINT ep, u8_t *buf, u32_t size);

void Ep0Handler(void);

void IsrUsbd(unsigned long nr, unsigned long parameter);

#define USB_DEBUG 0
#if USB_DEBUG == 1
void DbgPrintf(char *fmt, ...);
#elif USB_DEBUG == 2
#define DbgPrintf printf
#else
#define DbgPrintf(fmt, args...)
#endif

extern struct usb_buf ub[4];

extern S3C24X0_GPIO * gpioregs;
extern S3C24X0_USB_DEVICE * usbdevregs;
extern enum USB_DEV_STATE usbd_state;
extern enum EP0_STATE ep0State;

extern struct usbdev_struct usbdev;
#endif //__24XUSB_H__
