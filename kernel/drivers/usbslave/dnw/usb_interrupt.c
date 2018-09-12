#include <interrupt.h>
#include <s3c24x0.h>
#include "usbmain.h"
S3C24X0_INTERRUPT * intregs;

extern void IsrUsbd(void);
extern void IsrDma2(void);

void USB_ISR_Init(void) {
	intregs = S3C24X0_GetBase_INTERRUPT();
	set_irq_handler(INT_USBD, IsrUsbd);
	ClearPending(INT_USBD);
	
	set_irq_handler(INT_DMA2, IsrDma2);
	ClearPending(INT_DMA2);
	
}


