#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <core/device.h>

struct console_t
{
	/* The console name */
	char * name;

	/* Read console */
	ssize_t (*read)(struct console_t * console, unsigned char * buf, size_t count);

	/* Write console */
	ssize_t (*write)(struct console_t * console, const unsigned char * buf, size_t count);

	/* Private data */
	void * priv;
};

struct console_t * search_console(const char * name);
struct console_t * search_first_console(void);
bool_t register_console(struct device_t ** device, struct console_t * console);
bool_t unregister_console(struct console_t * console);

struct console_t * console_get(void);
bool_t console_set(const char * name);
ssize_t console_stdin_read(unsigned char * buf, size_t count);
ssize_t console_stdout_write(const unsigned char * buf, size_t count);
ssize_t console_stderr_write(const unsigned char * buf, size_t count);

#ifdef __cplusplus
}
#endif

#endif /* __CONSOLE_H__ */
