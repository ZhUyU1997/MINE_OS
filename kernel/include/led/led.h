#ifndef __LED_H__
#define __LED_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <core/device.h>

struct led_t
{
	/* The LED name */
	char * name;

	/* Set LED's brightness (0 ~ CONFIG_MAX_BRIGHTNESS) */
	void (*set)(struct led_t * led, int brightness);

	/* Get LED's brightness */
	int (*get)(struct led_t * led);

	/* Private data */
	void * priv;
};

struct led_t * search_led(const char * name);
bool_t register_led(struct device_t ** device, struct led_t * led);
bool_t unregister_led(struct led_t * led);

void led_set_brightness(struct led_t * led, int brightness);
int led_get_brightness(struct led_t * led);

#ifdef __cplusplus
}
#endif

#endif /* __LED_H__ */
