/*
 * driver/console/console-virt.c
 *
 * Copyright(c) 2007-2019 Jianjun Jiang <8192542@qq.com>
 * Official site: http://xboot.org
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <core/initcall.h>
#include <console/console.h>
#include <framebuffer/framebuffer.h>
#include <malloc.h>
#include "vt100.h"
#include "font5x8.h"

struct console_virt_pdata_t {
	struct framebuffer_t * fb;
	struct render_t * render;
	struct vt100 * term;
	int width;
	int height;
	int bpp;
};

static void send_response(char *str)
{
}

static void put_pixel(struct render_t * render, uint16_t x, uint16_t y, uint32_t color)
{
	uint32_t * data = render->pixels;
	data[y * render->width + x] = color;
}

static uint32_t get_pixel(struct render_t * render, uint16_t x, uint16_t y)
{
	uint32_t * data = render->pixels;
	return data[y * render->width + x];
}

static void sync(struct vt100 *term)
{
	struct console_t * console = term->console;
	struct console_virt_pdata_t * pdat = (struct console_virt_pdata_t *)console->priv;
	struct framebuffer_t * fb = pdat->fb;
	struct render_t * render = pdat->render;
	framebuffer_present_render(fb, render, NULL, 0);
}

static void move_char(struct vt100 *term, uint16_t dx, uint16_t dy, uint16_t sx, uint16_t sy)
{
	struct console_t * console = term->console;
	struct console_virt_pdata_t * pdat = (struct console_virt_pdata_t *)console->priv;
	struct framebuffer_t * fb = pdat->fb;
	struct render_t * render = pdat->render;

	dx = dx * term->char_width;
	dy = dy * term->char_height;
	sx = sx * term->char_width;
	sy = sy * term->char_height;

	for (int b = 0; b < term->char_height; b++)
	{
		for (int j = 0; j < term->char_width; j++)
		{
			put_pixel(render, dx + j, dy + b, get_pixel(render, sx + j, sy + b));
		}
	}
	//framebuffer_present_render(fb, render, &(struct dirty_rect_t) {dx, dy, term->char_width, term->char_height}, 1);
}

static void _draw_char(struct vt100 *term, uint16_t x, uint16_t y, uint8_t ch, bool_t bsync)
{
	struct console_t * console = term->console;
	struct console_virt_pdata_t * pdat = (struct console_virt_pdata_t *)console->priv;
	struct render_t * render = pdat->render;
	struct framebuffer_t * fb = pdat->fb;

	uint32_t bg = term->back_color;
	uint32_t fg = term->front_color;

	x = x * term->char_width;
	y = y * term->char_height;

	// character glyph buffer
	const unsigned char *_buf = &font[ch * 5];

	for (int b = 0; b < term->char_height; b++)
	{
		for (int j = 0; j < term->char_width; j++)
		{
			put_pixel(render, x + j, y + b, bg);
		}
	}

	for (int b = 0; b < VT100_FONT_HEIGHT; b++)
	{
		// draw 5 pixels for each column of the glyph
		for (int j = 0; j < VT100_FONT_WIDTH; j++)
		{
			uint32_t pix = bg;
			// j == t->char_width - 1 : draw one more separator pixel
			int rb = b * (8.0 / VT100_FONT_HEIGHT);
			int rj = j * (5.0 / VT100_FONT_WIDTH);
			if (_buf[rj] & (1 << (rb)))
				pix = fg;
			put_pixel(render, x + j, y + b, pix);
		}
	}
	if(bsync == TRUE)
	{
		framebuffer_present_render(fb, render, &(struct dirty_rect_t) {x, y, term->char_width, term->char_height}, 1);
		//sync(term);
	}
}

static void draw_char(struct vt100 *term, uint16_t x, uint16_t y, uint8_t ch)
{
	_draw_char(term, x, y, ch, TRUE);
}

static void fill_rect(struct vt100 *term, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	for (int i = 0; i < w; i++)
	{
		for (int j = 0; j < h; j++)
		{
			_draw_char(term, x + i, y + j, '\0', FALSE);
		}
	}
	sync(term);
}

static void scroll(struct vt100 *term, int lines)
{
	uint16_t top = term->scroll_start_row;
	uint16_t bottom = term->scroll_end_row;
	int height = bottom - top + 1;

	if (lines > 0)
		for (int i = top; i <= bottom - lines; i++)
			for (int j = 0; j < term->width; j++)
				move_char(term, j, i, j, i + lines);
	else if (lines < 0)
		for (int i = bottom; i >= top - lines; i--)
			for (int j = 0; j < term->width; j++)
				move_char(term, j, i, j, i + lines);

	if (lines < 0) {
		term->fill_rect(term, 0, top, term->width, -lines);
	} else if (lines > 0) {
		term->fill_rect(term, 0, bottom - lines + 1, term->width, lines);
	}
}

static void terminal_init(struct console_t * console)
{
	struct console_virt_pdata_t * pdat = (struct console_virt_pdata_t *)console->priv;
	struct vt100 *term = pdat->term;
	term->char_width = VT100_CHAR_WIDTH;
	term->char_height = VT100_CHAR_HEIGHT;
	term->width = pdat->width / VT100_CHAR_WIDTH;
	term->height = pdat->height / VT100_CHAR_HEIGHT;
	term->draw_char = draw_char;
	term->fill_rect = fill_rect;
	term->scroll = scroll;
	term->send_response = send_response;
	term->console = console;
	vt100_init(term);
	//sync(term);
}

static ssize_t console_virt_read(struct console_t * console, unsigned char * buf, size_t count)
{
	struct console_virt_pdata_t * pdat = (struct console_virt_pdata_t *)console->priv;
	return count;
}

static ssize_t console_virt_write(struct console_t * console, const unsigned char * buf, size_t count)
{
	struct console_virt_pdata_t * pdat = (struct console_virt_pdata_t *)console->priv;
	vt100_puts(pdat->term, buf, count);
	//sync(pdat);
	return count;
}

static struct device_t * console_virt_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct console_virt_pdata_t * pdat;
	struct console_t * console;
	struct device_t * dev;
	struct framebuffer_t * fb = search_framebuffer(dt_read_string(n, "framebuffer", NULL));

	if(!fb)
		return NULL;

	pdat = malloc(sizeof(struct console_virt_pdata_t));
	if(!pdat)
		return NULL;

	console = malloc(sizeof(struct console_t));
	if(!console)
	{
		free(pdat);
		return NULL;
	}

	pdat->term = malloc(sizeof(struct vt100));
	if(!pdat->term)
	{
		free(pdat);
		free(console);
		return NULL;
	}

	pdat->fb = fb;
	pdat->render = framebuffer_create_render(fb);
	pdat->width = framebuffer_get_width(fb);
	pdat->height = framebuffer_get_height(fb);
	pdat->bpp = framebuffer_get_bpp(fb);

	framebuffer_set_backlight(fb, CONFIG_MAX_BRIGHTNESS);

	console->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	console->read = console_virt_read;
	console->write = console_virt_write;
	console->priv = pdat;

	terminal_init(console);

	if(!register_console(&dev, console))
	{
		free_device_name(console->name);
		free(console->priv);
		free(console);
		return NULL;
	}
	dev->driver = drv;

	//test_vt100(console, pdat->term);
	return dev;
}

static void console_virt_remove(struct device_t * dev)
{
	struct console_t * console = (struct console_t *)dev->priv;

	if(console && unregister_console(console))
	{
		free_device_name(console->name);
		free(console->priv);
		free(console);
	}
}

static void console_virt_suspend(struct device_t * dev)
{
}

static void console_virt_resume(struct device_t * dev)
{
}

static struct driver_t console_virt = {
	.name		= "console-virt",
	.probe		= console_virt_probe,
	.remove		= console_virt_remove,
	.suspend	= console_virt_suspend,
	.resume		= console_virt_resume,
};

static __init void console_virt_driver_init(void)
{
	register_driver(&console_virt);
}

static __exit void console_virt_driver_exit(void)
{
	unregister_driver(&console_virt);
}

driver_initcall(console_virt_driver_init);
driver_exitcall(console_virt_driver_exit);
