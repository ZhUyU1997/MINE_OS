#include <string.h>
#include <usb/ch9.h>
#include <usb/2440usb.h>
#include "usb_mass.h"

static u8_t *ramdisk = (u8_t *)MASS_STORAGE_ADDR;

void usb_read_ramdisk(int ep, u32_t addr, u32_t size) {
	DbgPrintf("[READ BLOCK:%d,SIZE:%d]", addr / MASS_STORAGE_BLOCK_SIZE, size / MASS_STORAGE_BLOCK_SIZE);
	usb_send_init(ep, ramdisk + addr, size);
	usb_send_message(ep);
}
void usb_write_ramdisk(u32_t addr, u8_t *buf, u32_t size) {
	DbgPrintf("[WRITE BLOCK:%d OFFSET:%d]", addr / MASS_STORAGE_BLOCK_SIZE, addr % MASS_STORAGE_BLOCK_SIZE);
	memcpy(ramdisk + addr, buf, size);
}
