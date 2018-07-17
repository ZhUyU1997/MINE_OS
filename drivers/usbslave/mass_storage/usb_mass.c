#include <usb/ch9.h>
#include <assert.h>
#include <usb/2440usb.h>
#include "usb_mass.h"

//**********************************************************************/
//usb mass class request
//
/***********************************************************************/
#define RESET       0xFF
#define MAX_LUN     0xFE
void handle_mass_class(struct usb_ctrlrequest ctrlreq) {
	switch (ctrlreq.bRequest) {
		case MAX_LUN:
		{
			DbgPrintf("[MAX_LUN]");
			char lun = MASS_STORAGE_MAX_LUN;
			CLR_EP0_OUT_PKT_RDY();
			usb_send_init(EP0, &lun, 1);
			ep0State = EP0_STATE_TRANSMIT;
			break;
		}
		case RESET:
			DbgPrintf("[RESET]");
			//TODO
			//send empty data
			usb_send_init(EP0, NULL, 0);
			ep0State = EP0_STATE_TRANSMIT;
			break;
	}
}
void do_error() {
	return;
}
//-------------------------------------------------------------------
char ModeSense_rpy[4] = {
	0x03,
	0x00,
	0x00,
	0x00,
};
char Inquiry_rpy[36] = {
	0x00, 0x80,
	0x02, 0x02,
	0x1f, 0x00,
	0x00, 0x00,
	'M', 'a', 's', 's', 't', 'e', 's', 't',
	'c', 'h', 'i', 'n', 'e', 's', 'e', 'c', 'h', 'i', 'n', 'e', 's', 'e', 'e', 'e',
	'1', '.', '0', '0'
};
char ReadFmtCap_rpy[] = {
	0x00, 0x00,
	0x00, 0x08,
	(char)((MASS_STORAGE_BLOCK_NUM) >> 24), (char)((MASS_STORAGE_BLOCK_NUM) >> 16),
	(char)((MASS_STORAGE_BLOCK_NUM) >> 8), (char)(MASS_STORAGE_BLOCK_NUM),
	0x02,
	(char)(MASS_STORAGE_BLOCK_SIZE >> 24), (char)(MASS_STORAGE_BLOCK_SIZE >> 16),
	(char)(MASS_STORAGE_BLOCK_SIZE >> 8) , (char)(MASS_STORAGE_BLOCK_SIZE),
};
char ReadCap_rpy[] = {
	(char)((MASS_STORAGE_BLOCK_NUM - 1) >> 24), (char)((MASS_STORAGE_BLOCK_NUM - 1) >> 16),
	(char)((MASS_STORAGE_BLOCK_NUM - 1) >> 8), (char)(MASS_STORAGE_BLOCK_NUM - 1),

	(char)(MASS_STORAGE_BLOCK_SIZE >> 24), (char)(MASS_STORAGE_BLOCK_SIZE >> 16),
	(char)(MASS_STORAGE_BLOCK_SIZE >> 8) , (char)(MASS_STORAGE_BLOCK_SIZE),
};
struct mass_data_write_struct {
	U32 spread_size;
	U32 recvd_size;
	U32 block_addr;
} write_req = {0};
struct mass_req_fsm {
	struct cbw tcbw;
	struct csw tcsw;
	int flg_req;
	int exec_status;
};
struct mass_req_fsm g_mass_req = {0};

//**********************************************************************/
//usb mass in bulk endpoint
//macro defintion

/***********************************************************************/
int handle_mass_bulk_in() {
	if (g_mass_req.flg_req) {
		DbgPrintf("[RPY]");
		g_mass_req.tcsw.dCSWSignature = CSWSignature;
		g_mass_req.tcsw.dCSWTag = g_mass_req.tcbw.dCBWTag;
		g_mass_req.tcsw.dCSWDataResidue = 0;
		g_mass_req.tcsw.dCSWStatus = g_mass_req.exec_status;
		usb_send_init(EP2, &g_mass_req.tcsw, sizeof(struct csw));
		usb_send_message(EP2);
		g_mass_req.flg_req = 0;
		g_mass_req.exec_status = 0;
		memset(&g_mass_req.tcbw, 0, sizeof(struct cbw));
		memset(&g_mass_req.tcsw, 0, sizeof(struct csw));
		return 0;
	}
	return -1;
}
//**********************************************************************/
//usb mass out bulk endpoint
//macro defintion
#define MASS_OUT_CMD          0
#define MASS_OUT_DATA         1
int mass_out_fsm = MASS_OUT_CMD;
/***********************************************************************/
void handle_mass_bulk_cmd(char *buf, int size) {
	struct cbw* pcbw;
	struct CBDescriptor * pcbd;
	if (size != sizeof(struct cbw))
		goto error;
	pcbw = (struct cbw*)buf;

	if (pcbw->dCBWSignature != CBWSignature)
		goto error;
	memcpy((char *)&g_mass_req.tcbw, buf, size);
	g_mass_req.flg_req = 1;
	g_mass_req.exec_status = 0;
	pcbd = (struct CBDescriptor *)pcbw->CBWCB;
	switch (pcbd->op_code) {
		case TestUnitRdy:
			DbgPrintf("[TSTRDY:%08X]",g_mass_req.tcbw.dCBWTag);
			handle_mass_bulk_in();
			break;
		case Inquiry:
			DbgPrintf("[INQUIRY]");
			//in
			if (pcbw->bmCBWFlags & (1 << 7)) {
				usb_send_init(EP2, (char *)Inquiry_rpy, sizeof(Inquiry_rpy));
				usb_send_message(EP2);
			}
			break;
		case ReadFormatCap:
			DbgPrintf("[READ FORMAT CAP]");
			if (pcbw->bmCBWFlags & (1 << 7)) {
				//	unsigned int t_size = (((char *)&pcbd->parameter)[0]<<8) | ((char *)&pcbd->parameter)[1];
				//	DbgPrintf("size:%d\n",t_size);
				usb_send_init(EP2, (char *)ReadFmtCap_rpy, sizeof(ReadFmtCap_rpy));
				usb_send_message(EP2);
			}
			break;
		case ReadCap:
			DbgPrintf("[READ CAP]");
			if (pcbw->bmCBWFlags & (1 << 7)) {
				usb_send_init(EP2, (char *)ReadCap_rpy, sizeof(ReadCap_rpy));
				usb_send_message(EP2);
			}
			break;
		case Read:
			DbgPrintf("[READ]");
			if (pcbw->bmCBWFlags & (1 << 7)) {
				U32 block_addr = (((U8 *)&pcbd->logic_address)[0] << 24)
								|(((U8 *)&pcbd->logic_address)[1] << 16)
								|(((U8 *)&pcbd->logic_address)[2] <<  8)
								|(((U8 *)&pcbd->logic_address)[3] <<  0);
				U32 t_size = (((char *)&pcbd->parameter)[0] << 8) | ((char *)&pcbd->parameter)[1];

				t_size *= MASS_STORAGE_BLOCK_SIZE;
				if (pcbw->dCBWDataTransferLength == 0) {
					g_mass_req.exec_status = 2;
					handle_mass_bulk_in();
				}
				if (pcbw->dCBWDataTransferLength > t_size) {
					g_mass_req.exec_status = 1;
					handle_mass_bulk_in();
				} else if (pcbw->dCBWDataTransferLength < t_size) {
					g_mass_req.exec_status = 2;
					handle_mass_bulk_in();
				} else {
					usb_read_ramdisk(EP2, block_addr * MASS_STORAGE_BLOCK_SIZE, t_size);
				}
			}
			break;
		case Write:
			if (!(pcbw->bmCBWFlags & (1 << 7))) {
				DbgPrintf("[WRITE]");
				U32 block_addr = (((U8 *)&pcbd->logic_address)[0] << 24)
								|(((U8 *)&pcbd->logic_address)[1] << 16)
								|(((U8 *)&pcbd->logic_address)[2] <<  8)
								|(((U8 *)&pcbd->logic_address)[3] <<  0);
				write_req.spread_size = pcbw->dCBWDataTransferLength;
				write_req.block_addr = 	block_addr * MASS_STORAGE_BLOCK_SIZE;
				write_req.recvd_size = 0;
				mass_out_fsm = MASS_OUT_DATA;
			}
			break;
		case ReqSense:
			DbgPrintf("[REQSENSE]");
			break;
		case ModeSense:
			DbgPrintf("[MODESENSE %d]", pcbw->dCBWDataTransferLength);
			if (pcbw->bmCBWFlags & (1 << 7)) {
				usb_send_init(EP2, (char *)ModeSense_rpy, pcbw->dCBWDataTransferLength);
				usb_send_message(EP2);
			}
			break;
		case FormatUnit:
			DbgPrintf("[FORMATUNIT]");
			break;
		case Verify:
			DbgPrintf("[Verify]");
			break;
		case Medium_remove:
			DbgPrintf("[Medium_remove]");
			handle_mass_bulk_in();
			break;
		default:
			DbgPrintf("[DEFAULT CMD:%p]",pcbd->op_code);
			while(1);
			handle_mass_bulk_in();
	}
	return;
error:
	DbgPrintf("[ERROR]");
	do_error();
	return;
}

void handle_mass_bulk_out() {
	U8 buf[64];
	U8 out_csr = usbdevregs->OUT_CSR1_REG;
	U32 size;
	if (out_csr & EPO_OUT_PKT_READY){
		size = ep_rx_fifo(EP3, buf, sizeof(buf));
	}else{
		DbgPrintf("[ERROR]");
		assert(0);
		return;
	}
	switch (mass_out_fsm) {
		case MASS_OUT_CMD:
			DbgPrintf("[CMD]");
			handle_mass_bulk_cmd(buf, size);
			break;
		case MASS_OUT_DATA:
			DbgPrintf("[DATA]");
			write_req.spread_size -= size;
			usb_write_ramdisk(write_req.block_addr + write_req.recvd_size, buf, size);
			if (write_req.spread_size == 0) {
				DbgPrintf("[change state]");
				handle_mass_bulk_in();
				mass_out_fsm = MASS_OUT_CMD;
				write_req.block_addr = 0;
				write_req.recvd_size = 0;
			} else {
				write_req.recvd_size += size;
			}
			break;
	}
	//TODO：打开调试输出时，如果在ep_rx_fifo()之后插入CLR_EPX_OUT_PKT_RDY(),会出现问题。
	//关掉调试则正常，因此为了统一将CLR_EPX_OUT_PKT_RDY放在后面（可能会影响UDC速度）
	//初步猜测可能串口打印时间过长导致超时的缘故
	CLR_EPX_OUT_PKT_RDY();
}
