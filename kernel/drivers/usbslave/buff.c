#include <sys/types.h>
#include <assert.h>
#include <usb/2440usb.h>

void usb_buf_init(struct usb_buf *ub, U8 *buf, U32 size) {
	assert(ub && buf && size);
	//buf不为空，size则必须大于0
	assert((!buf) || size);
	ub->buf = buf;
	ub->size = size;
	ub->count = 0;
}
static inline U32 usb_buf_read_single(struct usb_buf *ub, U8 *buf) {
	if (ub->size > ub->count) {
		buf[0] = ub->buf[ub->count++];
		return 1;
	} else {
		return 0;
	}
}
//目的地址增加
U32 usb_buf_read_inc(struct usb_buf *ub, U8 *buf, U32 size) {
	assert(ub && buf && size);
	for (U32 i = 0; i < size; i++) {
		if (!usb_buf_read_single(ub, buf++))
			return i;
	}
	return size;
}
//目的地址不变
U32 usb_buf_read_const(struct usb_buf *ub, U8 *buf, U32 size) {
	assert(ub && buf && size);
	for (U32 i = 0; i < size; i++) {
		if (!usb_buf_read_single(ub, buf))
			return i;
	}
	return size;
}

U32 usb_buf_remain(struct usb_buf *ub) {
	assert(ub);
	return ub->size - ub->count;
}

U32 usb_buf_count(struct usb_buf *ub) {
	assert(ub);
	return ub->count;
}
