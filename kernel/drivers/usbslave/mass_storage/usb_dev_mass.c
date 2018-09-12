#include <s3c24x0.h>
#include <sys/types.h>
#include <assert.h>
#include <usb/ch9.h>
#include <usb/2440usb.h>

struct g_config_desc_to_send 
{
	struct usb_config_descriptor g_config_desc;
	struct usb_interface_descriptor g_interface_desc;
	struct usb_endpoint_descriptor g_endpoint_desc;
	struct usb_endpoint_descriptor g_endpoint_desc1;
	struct usb_endpoint_descriptor g_endpoint_desc2;
} __attribute__ ((packed));

static struct usb_device_descriptor g_usb_dev_desc = {
	.bLength 			= USB_DT_DEVICE_SIZE,
	.bDescriptorType 	= USB_DT_DEVICE,
	.bcdUSB 			= 0x0110,
	.bDeviceClass 		= 0,
	.bDeviceSubClass  	= 0,
	.bDeviceProtocol 	= 0,
	.bMaxPacketSize0    = 0x08,
	.idVendor			= 0x7777,
	.idProduct			= 0x6666,
	.iManufacturer		= 1,
	.iProduct			= 2,
	.iSerialNumber		= 3,
	.bNumConfigurations	= 1
};
static struct g_config_desc_to_send g_config_all = {
	.g_config_desc =
	{
		.bLength			= USB_DT_CONFIG_SIZE,
		.bDescriptorType	= USB_DT_CONFIG,

		.wTotalLength		= sizeof(struct usb_config_descriptor)
							+ sizeof(struct usb_interface_descriptor)
							+ sizeof(struct usb_endpoint_descriptor) * 3,
		.bNumInterfaces		= 1,
		.bConfigurationValue = 1,
		.iConfiguration		= 0,
		.bmAttributes		= 0xc0,
		.bMaxPower			= 0x31,
	},
	.g_interface_desc =
	{
		.bLength			= USB_DT_INTERFACE_SIZE,
		.bDescriptorType	= USB_DT_INTERFACE,

		.bInterfaceNumber	= 0,
		.bAlternateSetting	= 0,
		.bNumEndpoints		= 3,
		.bInterfaceClass	= USB_CLASS_MASS_STORAGE,
		.bInterfaceSubClass	= 0x06,
		.bInterfaceProtocol	= 0x50,
		.iInterface			= 0,
	},
	.g_endpoint_desc =
	{
		.bLength			= USB_DT_ENDPOINT_SIZE,
		.bDescriptorType	= USB_DT_ENDPOINT,

		.bEndpointAddress	= 0x81,
		.bmAttributes		= USB_ENDPOINT_XFER_INT,
		.wMaxPacketSize		= 0x20,
		.bInterval			= 0x0A,

	},
	.g_endpoint_desc1 =
	{
		.bLength			= USB_DT_ENDPOINT_SIZE,
		.bDescriptorType	= USB_DT_ENDPOINT,

		.bEndpointAddress	= 0x82,
		.bmAttributes		= USB_ENDPOINT_XFER_BULK,
		.wMaxPacketSize		= 0x40,
		.bInterval			= 0x0A,

	},
	.g_endpoint_desc2 =
	{
		.bLength			= USB_DT_ENDPOINT_SIZE,
		.bDescriptorType	= USB_DT_ENDPOINT,

		.bEndpointAddress	= 0x03,
		.bmAttributes		= USB_ENDPOINT_XFER_BULK,
		.wMaxPacketSize		= 0x40,
		.bInterval			= 0x0A,
	},
};
static char g_string_desc0[4] = {
	4,
	USB_DT_STRING,
	0x09,
	0x04
};
static char g_string_desc1[20] = {
	20,
	USB_DT_STRING,
	'T', 0x00,
	'E', 0x00,
	'S', 0x00,
	'T', 0x00,
	'V', 0x00,
	'E', 0x00,
	'N', 0x00,
	'D', 0x00,
	' ', 0x00,
};
static char g_string_desc2[20] = {
	20,
	USB_DT_STRING,
	'U', 0x00,
	'S', 0x00,
	'B', 0x00,
	'M', 0x00,
	'A', 0x00,
	'S', 0x00,
	'S', 0x00,
	'S', 0x00,
	' ', 0x00,
};
static char g_string_desc3[20] = {
	20,
	USB_DT_STRING,
	'U', 0x00,
	'S', 0x00,
	'B', 0x00,
	'M', 0x00,
	'A', 0x00,
	'S', 0x00,
	'S', 0x00,
	'S', 0x00,
	' ', 0x00,
};
static void usbdev_mass_storage_reset(void) {
	usbdevregs->EP_INT_EN_REG = 0;
	usbdevregs->USB_INT_EN_REG = 0;
	/* 禁止挂起模式 */
	usbdevregs->PWR_REG = PWR_REG_DEFAULT_VALUE;	//disable suspend mode
	usbdevregs->FUNC_ADDR_REG = 0x80;
	/* 端点0 */
	SET_INDEX(0);
	usbdevregs->INDEX_REG = 0;
	usbdevregs->MAXP_REG = FIFO_SIZE_8;   	//EP0 max packit size = 8
	FLUSH_EP0_FIFO();
	/* 端点1 */
	SET_INDEX(1);
	usbdevregs->MAXP_REG = FIFO_SIZE_64;
	/* 端点2 */
	SET_INDEX(2);
	usbdevregs->MAXP_REG = FIFO_SIZE_64;
	/* 端点3 */
	SET_INDEX(3);
	usbdevregs->IN_CSR2_REG = EPI_MODE_OUT;
	usbdevregs->MAXP_REG = FIFO_SIZE_64;

	usbdevregs->EP_INT_REG = EP0_INT | EP1_INT | EP2_INT | EP3_INT | EP4_INT;
	usbdevregs->USB_INT_REG = RESET_INT | SUSPEND_INT | RESUME_INT;

	usbdevregs->EP_INT_EN_REG = EP0_INT | EP1_INT | EP2_INT | EP3_INT;
	usbdevregs->USB_INT_EN_REG = RESET_INT;

	ep0State = EP0_STATE_INIT;
}

static void Ep1Handler(void) {
	return;
}

static void Ep2Handler(){
	SET_INDEX(2);
	if (epx_status_common(USB_DIR_IN) < 0)
		return;
	if (usb_send_is_empty(EP2)) {
		DbgPrintf("[EMPTY]");
		handle_mass_bulk_in();
	} else {
		usb_send_message(EP2);
	}
}

static void Ep3Handler(){
	SET_INDEX(3);
	if (epx_status_common(USB_DIR_OUT) < 0)
		return;
	handle_mass_bulk_out();
}

extern void handle_mass_class(struct usb_ctrlrequest ctrlreq);

struct usbdev_struct usbdev = {
	.dev_desc = &g_usb_dev_desc,
	.config_desc = &g_config_all.g_config_desc,
	.interface_desc = &g_config_all.g_interface_desc,
	.hid_desc = NULL,
	.endpoint_desc[0] = &g_config_all.g_endpoint_desc,
	.endpoint_desc[1] = &g_config_all.g_endpoint_desc1,
	.endpoint_desc[2] = &g_config_all.g_endpoint_desc2,
	.endpoint_desc[3] = NULL,
	.string_desc[0] = &g_string_desc0,
	.string_desc[1] = &g_string_desc1,
	.string_desc[2] = &g_string_desc2,
	.string_desc[3] = &g_string_desc3,
	.string_desc_size[0] = sizeof(g_string_desc0),
	.string_desc_size[1] = sizeof(g_string_desc1),
	.string_desc_size[2] = sizeof(g_string_desc2),
	.string_desc_size[3] = sizeof(g_string_desc3),
	.config_all = &g_config_all,
	.config_all_size = sizeof(g_config_all),
	.report_desc = NULL,
	.report_desc_size = 0,
	.ep_handler[0] = Ep1Handler,
	.ep_handler[1] = Ep2Handler,
	.ep_handler[2] = Ep3Handler,
	.ep_handler[3] = NULL,
	.reset = usbdev_mass_storage_reset,
	.handle_class = handle_mass_class,
};
