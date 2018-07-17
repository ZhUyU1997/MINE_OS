#include <s3c24x0.h>
#include <sys/types.h>
#include <assert.h>
#include <usb/ch9.h>
#include <usb/2440usb.h>
struct g_config_desc_to_send 
{
	struct usb_config_descriptor g_config_desc;
	struct usb_interface_descriptor g_interface_desc;
	struct usb_hid_descriptor g_hid_desc;
	struct usb_endpoint_descriptor g_endpoint_desc;
} __attribute__ ((packed));

/*
 * USB报告描述符的定义
 * 返回的输入报告具有4字节。
 * 第一字节的低3位用来表示按键是否按下的，高5位为常数0，无用。
 * 第二字节表示X轴改的变量，第三字节表示Y轴的改变量，第四字节表示
 * 滚轮的改变量。我们在中断端点1中应该要按照上面的格式返回实际的
 * 鼠标数据。
 */
static const char gHIDReportDescriptor[52] = {
	//每行开始的第一字节为该条目的前缀，前缀的格式为：
	//D7~D4：bTag。D3~D2：bType；D1~D0：bSize。以下分别对每个条目注释。

	//这是一个全局（bType为1）条目，选择用途页为普通桌面Generic Desktop Page(0x01)
	//后面跟一字节数据（bSize为1），后面的字节数就不注释了，
	//自己根据bSize来判断。
	0x05U, 0x01U,                   /* Usage Page (Generic Desktop)         */

	//这是一个局部（bType为2）条目，说明接下来的应用集合用途用于鼠标
	0x09U, 0x02U,                   /* Usage (Mouse)                        */

	//这是一个主条目（bType为0）条目，开集合，后面跟的数据0x01表示
	//该集合是一个应用集合。它的性质在前面由用途页和用途定义为
	//普通桌面用的鼠标。
	0xA1U, 0x01U,                   /* Collection (Application)             */

	//这是一个局部条目。说明用途为指针集合
	0x09U, 0x01U,                   /*   Usage (Pointer)                    */

	//这是一个主条目，开集合，后面跟的数据0x00表示该集合是一个
	//物理集合，用途由前面的局部条目定义为指针集合。
	0xA1U, 0x00U,                   /*   Collection (Physical)              */

	//这是一个全局条目，选择用途页为按键（Button Page(0x09)）
	0x05U, 0x09U,                   /*     Usage Page (Buttons)             */

	//这是一个局部条目，说明用途的最小值为1。实际上是鼠标左键。
	0x19U, 0x01U,                   /*     Usage Minimum (01)               */

	//这是一个局部条目，说明用途的最大值为3。实际上是鼠标中键。
	0x29U, 0x03U,                   /*     Usage Maximum (03)               */

	//这是一个全局条目，说明返回的数据的逻辑值（就是我们返回的数据域的值啦）
	//最小为0。因为我们这里用Bit来表示一个数据域，因此最小为0，最大为1。
	0x15U, 0x00U,                   /*     Logical Minimum (0)              */

	//这是一个全局条目，说明逻辑值最大为1。
	0x25U, 0x01U,                   /*     Logical Maximum (1)              */

	//这是一个全局条目，说明数据域的数量为三个。
	0x95U, 0x03U,                   /*     Report Count (3)                 */

	//这是一个全局条目，说明每个数据域的长度为1个bit。
	0x75U, 0x01U,                   /*     Report Size (1)                  */

	//这是一个主条目，说明有3个长度为1bit的数据域（数量和长度
	//由前面的两个全局条目所定义）用来做为输入，
	//属性为：Data,Var,Abs。Data表示这些数据可以变动，Var表示
	//这些数据域是独立的，每个域表示一个意思。Abs表示绝对值。
	//这样定义的结果就是，第一个数据域bit0表示按键1（左键）是否按下，
	//第二个数据域bit1表示按键2（右键）是否按下，第三个数据域bit2表示
	//按键3（中键）是否按下。
	0x81U, 0x02U,                   /*     Input (Data, Variable, Absolute) */

	//这是一个全局条目，说明数据域数量为1个
	0x95U, 0x01U,                   /*     Report Count (1)                 */

	//这是一个全局条目，说明每个数据域的长度为5bit。
	0x75U, 0x05U,                   /*     Report Size (5)                  */

	//这是一个主条目，输入用，由前面两个全局条目可知，长度为5bit，
	//数量为1个。它的属性为常量（即返回的数据一直是0）。
	//这个只是为了凑齐一个字节（前面用了3个bit）而填充的一些数据
	//而已，所以它是没有实际用途的。
	0x81U, 0x01U,                   /*     Input (Constant) for padding     */

	//这是一个全局条目，选择用途页为普通桌面Generic Desktop Page(0x01)
	0x05U, 0x01U,                   /*     Usage Page (Generic Desktop)     */

	//这是一个局部条目，说明用途为X轴
	0x09U, 0x30U,                   /*     Usage (X)                        */

	//这是一个局部条目，说明用途为Y轴
	0x09U, 0x31U,                   /*     Usage (Y)                        */

	//这是一个局部条目，说明用途为滚轮
	0x09U, 0x38U,                   /*     Usage (Z)                        */

	//下面两个为全局条目，说明返回的逻辑最小和最大值。
	//因为鼠标指针移动时，通常是用相对值来表示的，
	//相对值的意思就是，当指针移动时，只发送移动量。
	//往右移动时，X值为正；往下移动时，Y值为正。
	//对于滚轮，当滚轮往上滚时，值为正。
	0x15U, 0x81U,                   /*     Logical Minimum (-127)           */
	0x25U, 0x7FU,                   /*     Logical Maximum (127)            */

	//这是一个全局条目，说明数据域的长度为8bit。
	0x75U, 0x08U,                   /*     Report Size (8)                  */

	//这是一个全局条目，说明数据域的个数为3个。
	0x95U, 0x03U,                   /*     Report Count (3)                 */

	//这是一个主条目。它说明这三个8bit的数据域是输入用的，
	//属性为：Data,Var,Rel。Data说明数据是可以变的，Var说明
	//这些数据域是独立的，即第一个8bit表示X轴，第二个8bit表示
	//Y轴，第三个8bit表示滚轮。Rel表示这些值是相对值。
	0x81U, 0x06U,                   /*     Input (Data, Variable, Relative) */

	//下面这两个主条目用来关闭前面的集合用。
	//我们开了两个集合，所以要关两次。bSize为0，所以后面没数据。
	0xC0U,                          /*   End Collection (Physical)          */
	0xC0U                           /* End Collection (Application)         */
};

static struct usb_device_descriptor g_usb_dev_desc = {
	.bLength 			= USB_DT_DEVICE_SIZE,
	.bDescriptorType 	= USB_DT_DEVICE,
	.bcdUSB 			= 0x0110,
	.bDeviceClass 		= 0,
	.bDeviceSubClass  	= 0,
	.bDeviceProtocol 	= 0,
	.bMaxPacketSize0    = 0x08,
	.idVendor			= 0x8888,
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
							+ sizeof(struct usb_endpoint_descriptor)
							+ sizeof(struct usb_hid_descriptor),
		.bNumInterfaces		= 1,	//配置数量
		.bConfigurationValue = 1,
		.iConfiguration		= 0,
		.bmAttributes		= USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,	//自己供电
		.bMaxPower			= 0x32, //设置为100mA
	},
	.g_interface_desc =
	{
		.bLength			= USB_DT_INTERFACE_SIZE,
		.bDescriptorType	= USB_DT_INTERFACE,

		.bInterfaceNumber	= 0,
		.bAlternateSetting	= 0,	//接口备用编号，一般不用，设为0
		.bNumEndpoints		= 1,	//接口使用的端点数，不包括0端点
		.bInterfaceClass	= USB_CLASS_HID,	//该接口使用的类,HID类
		.bInterfaceSubClass	= 1,	//该接口使用的子类,支持BIOS引导启动的子类
		.bInterfaceProtocol	= 2,	//该接口使用的协议,键盘代码为0x01，鼠标代码为0x02
		.iInterface			= 0,	//描述该接口的字符串的索引值，0表示没有
	},
	.g_hid_desc =
	{
		.bLength			= sizeof(struct usb_hid_descriptor),
		.bDescriptorType	= 0x21,
		.bcdHID				= 0x0110,
		.bCountryCode		= 0x33,
		.bNumDescriptors	= 1,
		.bDescriptorType0	= 0x22,
		.wDescriptorLength	= sizeof(gHIDReportDescriptor),
	},
	.g_endpoint_desc =
	{
		.bLength			= USB_DT_ENDPOINT_SIZE,
		.bDescriptorType	= USB_DT_ENDPOINT,

		.bEndpointAddress	= USB_DIR_IN | EP1,	//endpoint 1 is IN endpoint.
		.bmAttributes		= USB_ENDPOINT_XFER_INT,
		.wMaxPacketSize		= EP1_PKT_SIZE,	//32
		.bInterval			= 0x0A,			//端点查询的时间
	},
};

#define LANGID_US_L 		    	(0x09)
#define LANGID_US_H 		    	(0x04)
static char g_string_desc0[4] = {
	4,
	USB_DT_STRING,
	LANGID_US_L,
	LANGID_US_H
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
	'O', 0x00,
	'U', 0x00,
	'S', 0x00,
	'E', 0x00,
	' ', 0x00,
};
static char g_string_desc3[20] = {
	20,
	USB_DT_STRING,
	'U', 0x00,
	'S', 0x00,
	'B', 0x00,
	'M', 0x00,
	'O', 0x00,
	'U', 0x00,
	'S', 0x00,
	'E', 0x00,
	' ', 0x00,
};

static void usbdev_mouse_reset(void) {
	usbdevregs->EP_INT_EN_REG = 0;
	usbdevregs->USB_INT_EN_REG = 0;
	/* 禁止挂起模式 */
	usbdevregs->PWR_REG = PWR_REG_DEFAULT_VALUE;	//disable suspend mode
	usbdevregs->FUNC_ADDR_REG = 0x80;
	/* 端点0 */
	SET_INDEX(0);
	usbdevregs->INDEX_REG = 0;
	usbdevregs->MAXP_REG = FIFO_SIZE_8;   	//EP0 max packit size = 8
	usbdevregs->EP0_CSR = EP0_SERVICED_OUT_PKT_RDY | EP0_SERVICED_SETUP_END;
	FLUSH_EP0_FIFO();
	/* 端点1 */
	SET_INDEX(1);
	usbdevregs->MAXP_REG = FIFO_SIZE_64;
	usbdevregs->IN_CSR1_REG = EPI_FIFO_FLUSH | EPI_CDT;
	//usbdevregs->IN_CSR2_REG = EPI_MODE_IN | EPI_IN_DMA_INT_MASK | EPO_ISO; //IN mode, IN_DMA_INT=masked
	//TODO:EPO_ISO需去除，原因未知
	usbdevregs->IN_CSR2_REG = EPI_MODE_IN | EPI_IN_DMA_INT_MASK; //IN mode, IN_DMA_INT=masked
	usbdevregs->OUT_CSR1_REG = EPO_CDT;
	usbdevregs->OUT_CSR2_REG = EPO_ISO | EPO_OUT_DMA_INT_MASK;

	usbdevregs->EP_INT_REG = EP0_INT | EP1_INT | EP2_INT | EP3_INT | EP4_INT;
	usbdevregs->USB_INT_REG = RESET_INT | SUSPEND_INT | RESUME_INT;

	usbdevregs->EP_INT_EN_REG = EP0_INT | EP1_INT;
	usbdevregs->USB_INT_EN_REG = RESET_INT;

	ep0State = EP0_STATE_INIT;
}

static void Ep1Handler(void) {
	return;
}

extern void handle_hid_class(struct usb_ctrlrequest ctrlreq);

struct usbdev_struct usbdev = {
	.dev_desc = &g_usb_dev_desc,
	.config_desc = &g_config_all.g_config_desc,
	.interface_desc = &g_config_all.g_interface_desc,
	.hid_desc = &g_config_all.g_hid_desc,
	.endpoint_desc[0] = &g_config_all.g_endpoint_desc,
	.endpoint_desc[1] = NULL,
	.endpoint_desc[2] = NULL,
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
	.report_desc = &gHIDReportDescriptor,
	.report_desc_size = sizeof(gHIDReportDescriptor),
	.ep_handler[0] = Ep1Handler,
	.ep_handler[1] = NULL,
	.ep_handler[2] = NULL,
	.ep_handler[3] = NULL,
	.reset = usbdev_mouse_reset,
	.handle_class = handle_hid_class,
};
