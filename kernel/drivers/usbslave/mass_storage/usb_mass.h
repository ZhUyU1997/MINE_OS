#ifndef __USB_MASS_H__
#define __USB_MASS_H__
#include <usb/ch9.h>
extern void handle_class(struct usb_ctrlrequest ctrlreq);
extern int handle_mass_bulk_in(void);
extern void handle_mass_bulk_out(void);
#define CBWSignature 0x43425355

struct cbw {
	unsigned int dCBWSignature;
	unsigned int dCBWTag;
	unsigned int dCBWDataTransferLength;
	unsigned char bmCBWFlags;
	unsigned char dCBWLUN;
	unsigned char dCBWLength;
	unsigned char CBWCB[16];
} __attribute__((packed));

struct CBDescriptor {
	unsigned char op_code;
	unsigned char LUN;
	unsigned int logic_address;
	char res0;
	short parameter;
	unsigned char res1[3];
} __attribute__((packed));

#define TestUnitRdy     0x00
#define FormatUnit 		0x04
#define Inquiry    		0x12
#define ReadFormatCap 	0x23
#define ReadCap    		0x25
#define Read		    0x28
#define Write			0x2A
#define ReqSense		0x03
#define ModeSense       0x1A
#define Verify			0x2F
#define Medium_remove   0x1E
#define CSWSignature	0x53425355

struct csw {
	int dCSWSignature;
	int dCSWTag;
	int dCSWDataResidue;
	char dCSWStatus;
} __attribute__((packed));

/*GET LUN*/
#define MASS_STORAGE_MAX_LUN                  0
/*GET CAP*/
//512*24 = 12KB
#define MASS_STORAGE_BLOCK_SIZE               (512U)
#define MASS_STORAGE_BLOCK_NUM                (80)
#define MASS_STORAGE_ADDR                     (0x30A00000)
#endif

