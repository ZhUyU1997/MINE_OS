/****************************************************************
 NAME: usb.h
 DESC: definitions(USB data structure) for USB setup operation.
       Because It's h/w independent file, it may be used without any change in future.
       Reuse the source of S3C2400X u24xmon
 HISTORY:
 Apr.07.2000:purnnamu: first release.
 ****************************************************************/
#ifndef __USB_H__
#define __USB_H__

/* Endpoint 0 */

/* Standard bmRequestTyje (Direction) */
#define HOST_TO_DEVICE              (0x00)
#define DEVICE_TO_HOST              (0x80)

/* Standard bmRequestType (Type) */
#define STANDARD_TYPE               (0x00)
#define CLASS_TYPE                  (0x20)
#define VENDOR_TYPE                 (0x40)
#define RESERVED_TYPE               (0x60)

/* Standard bmRequestType (Recipient) */
#define DEVICE_RECIPIENT            (0)
#define INTERFACE_RECIPIENT         (1)
#define ENDPOINT_RECIPIENT          (2)
#define OTHER_RECIPIENT             (3)

/* Feature Selectors */
#define DEVICE_REMOTE_WAKEUP        (1)
#define EP_STALL                    (0)

/* Standard Request Codes */
#define GET_STATUS                  (0)
#define CLEAR_FEATURE               (1)
#define SET_FEATURE                 (3)
#define SET_ADDRESS                 (5)
#define GET_DESCRIPTOR              (6)
#define SET_DESCRIPTOR              (7)
#define GET_CONFIGURATION           (8)
#define SET_CONFIGURATION           (9)
#define GET_INTERFACE               (10)
#define SET_INTERFACE               (11)
#define SYNCH_FRAME                 (12)

/* Class-specific Request Codes */
#define GET_DEVICE_ID               (0)
#define GET_PORT_STATUS             (1)
#define SOFT_RESET                  (2)

/* Descriptor Types */
#define DEVICE_TYPE                 (1)
#define CONFIGURATION_TYPE          (2)
#define STRING_TYPE                 (3)
#define INTERFACE_TYPE              (4)
#define ENDPOINT_TYPE               (5)

/* configuration descriptor: bmAttributes */
#define CONF_ATTR_DEFAULT	    	(0x80) /* Spec 1.0 it was BUSPOWERED bit. */
#define CONF_ATTR_REMOTE_WAKEUP		(0x20)
#define CONF_ATTR_SELFPOWERED		(0x40)

/* endpoint descriptor */
#define EP_ADDR_IN					(0x80)
#define EP_ADDR_OUT					(0x00)

#define EP_ATTR_CONTROL		    	(0x0)
#define EP_ATTR_ISOCHRONOUS	    	(0x1)
#define EP_ATTR_BULK		    	(0x2)
#define EP_ATTR_INTERRUPT	    	(0x3)


/* string descriptor */
#define LANGID_US_L 		    	(0x09)
#define LANGID_US_H 		    	(0x04)


struct USB_SETUP_DATA {
	u8_t bmRequestType;
	u8_t bRequest;
	u8_t bValueL;
	u8_t bValueH;
	u8_t bIndexL;
	u8_t bIndexH;
	u8_t bLengthL;
	u8_t bLengthH;
};


struct USB_DEVICE_DESCRIPTOR {
	u8_t bLength;
	u8_t bDescriptorType;
	u8_t bcdUSBL;
	u8_t bcdUSBH;
	u8_t bDeviceClass;
	u8_t bDeviceSubClass;
	u8_t bDeviceProtocol;
	u8_t bMaxPacketSize0;
	u8_t idVendorL;
	u8_t idVendorH;
	u8_t idProductL;
	u8_t idProductH;
	u8_t bcdDeviceL;
	u8_t bcdDeviceH;
	u8_t iManufacturer;
	u8_t iProduct;
	u8_t iSerialNumber;
	u8_t bNumConfigurations;
};


struct USB_CONFIGURATION_DESCRIPTOR {
	u8_t bLength;
	u8_t bDescriptorType;
	u8_t wTotalLengthL;
	u8_t wTotalLengthH;
	u8_t bNumInterfaces;
	u8_t bConfigurationValue;
	u8_t iConfiguration;
	u8_t bmAttributes;
	u8_t maxPower;
};


struct USB_INTERFACE_DESCRIPTOR {
	u8_t bLength;
	u8_t bDescriptorType;
	u8_t bInterfaceNumber;
	u8_t bAlternateSetting;
	u8_t bNumEndpoints;
	u8_t bInterfaceClass;
	u8_t bInterfaceSubClass;
	u8_t bInterfaceProtocol;
	u8_t iInterface;
};


struct USB_ENDPOINT_DESCRIPTOR {
	u8_t bLength;
	u8_t bDescriptorType;
	u8_t bEndpointAddress;
	u8_t bmAttributes;
	u8_t wMaxPacketSizeL;
	u8_t wMaxPacketSizeH;
	u8_t bInterval;
};

struct USB_CONFIGURATION_SET {
	u8_t ConfigurationValue;
};

struct USB_GET_STATUS {
	u8_t Device;
	u8_t Interface;
	u8_t Endpoint0;
	u8_t Endpoint1;
	u8_t Endpoint3;
};

struct USB_INTERFACE_GET {
	u8_t AlternateSetting;
};

#endif /*__USB_H__*/
