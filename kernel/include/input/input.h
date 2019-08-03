#ifndef __INPUT_H__
#define __INPUT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <core/device.h>

enum input_type_t {
	INPUT_TYPE_KEYBOARD		= 1,
	INPUT_TYPE_ROTARY		= 2,
	INPUT_TYPE_MOUSE		= 3,
	INPUT_TYPE_TOUCHSCREEN	= 4,
	INPUT_TYPE_JOYSTICK		= 5,
};

enum {
	INPUT_IOCTL_TOUCHSCEEN_SET_CALIBRATION	= 0x00000001,
};

struct input_t
{
	/* The input name */
	char * name;

	/* Input type */
	enum input_type_t type;

	/* Ioctl interface */
	int (*ioctl)(struct input_t * input, int cmd, void * arg);

	/* Private data */
	void * priv;
};

struct input_t * search_input(const char * name);
bool_t register_input(struct device_t ** device, struct input_t * input);
bool_t unregister_input(struct input_t * input);
int input_ioctl(struct input_t * input, int cmd, void * arg);

#ifdef __cplusplus
}
#endif

#endif /* __INPUT_H__ */
