#include <s3c24x0.h>
#include <assert.h>
#include "2440usb.h"
#include "usb.h"
void send_ep1_data(U8 *buf, U32 size) {
	STORE_INDEX(1);
	while (usbdevregs->IN_CSR1_REG & EP0_IN_PKT_READY);
	WrPktEp1(buf, size);
	SET_EPX_IN_PKT_READY();
	RESTORE_INDEX();
}
void Ep1Handler(void) {
	return;
}
