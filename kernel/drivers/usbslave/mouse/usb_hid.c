#include <s3c24x0.h>
#include <assert.h>
#include <usb/ch9.h>
#include <usb/2440usb.h>

/* Class-specific Request Codes */
#define GET_DEVICE_ID               (0)
#define GET_PORT_STATUS             (1)
#define SOFT_RESET                  (2)

#define GET_REPORT       0x01
#define GET_IDLE         0x02
#define GET_PROTOCAL     0x03
#define SET_REPORT       0x09
#define SET_IDLE         0x0A
#define SET_PROTOCAL     0x0B

#define INPUT			 1
#define OUTPUT			 2
#define FEATURE			 3

void handle_hid_class(struct usb_ctrlrequest ctrlreq) {
	switch (ctrlreq.bRequest) {
		case GET_REPORT:
			switch ((ctrlreq.wValue & 0xff)) {
				case INPUT:
					DbgPrintf("[input]");
					CLR_EP0_OUTPKTRDY_DATAEND();
					//usb_send_message(EP0,(char *)&l_tSampleInputReport[0],4);
					break;
				case OUTPUT:
					DbgPrintf("[output]");
					break;
				case FEATURE:
					DbgPrintf("[feature]");
					break;
			}
			break;
		case GET_IDLE:
			DbgPrintf("[get idle]");
			break;
		case GET_PROTOCAL:
			DbgPrintf("[get protocal]");
			break;
		case SET_IDLE:
			DbgPrintf("[set idle]");
			CLR_EP0_OUTPKTRDY_DATAEND();
			break;
		case SET_REPORT:
			DbgPrintf("[set report]");
			CLR_EP0_OUTPKTRDY_DATAEND();
			break;
		case SET_PROTOCAL:
			DbgPrintf("[set protocol]");
			CLR_EP0_OUTPKTRDY_DATAEND();
			break;
	}
}
