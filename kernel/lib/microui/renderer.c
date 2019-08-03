#include <assert.h>
#include <time/timer.h>
#include <framebuffer/framebuffer.h>
#include <core/event.h>
#include <time/delay.h>

#include "renderer.h"
#include "atlas.inl"

#define BUFFER_SIZE 16384

static mu_Rect tex_buf[BUFFER_SIZE];
static mu_Rect vert_buf[BUFFER_SIZE];
static mu_Color color_buf[BUFFER_SIZE];

static struct framebuffer_t * fb;
static struct render_t * render;
static int width;
static int height;
static int bpp;
static int buf_idx;


void r_init(void) {
	fb = search_framebuffer(CONFIG_FB_DEV);
	render = framebuffer_create_render(fb);
	width = framebuffer_get_width(fb);
	height = framebuffer_get_height(fb);
	bpp = framebuffer_get_bpp(fb);
	
	framebuffer_set_backlight(fb, CONFIG_MAX_BRIGHTNESS);
}
#define BIT_GET(x,p)	(((char *)(x))[(p) / (sizeof(char) * 8)] & (1<<((p) % (sizeof(char) * 8))))

static void disp_flush(mu_Rect dst, mu_Rect src, mu_Color color)
{
    int32_t x;
    int32_t y;
	mu_Color old,new;
	float factor_x = ((float)src.w) / (dst.w);
	float factor_y = ((float)src.h) / (dst.h);
	int sx = dst.x < 0 ? 0 : dst.x >= width ? width - 1:dst.x;
	int sy = dst.y < 0 ? 0 : dst.y >= height ? height - 1:dst.y;
	int ex = dst.x + dst.w < 0 ? 0 : dst.x + dst.w >= width ? width - 1: dst.x + dst.w;
	int ey = dst.y + dst.h < 0 ? 0 : dst.y + dst.h >= height ? height - 1: dst.y + dst.h;

    for(y = sy; y < ey; y++) {
        for(x = sx; x < ex; x++) {
			int src_x = src.x + (x - dst.x) * factor_x;
			int src_y = src.y + (y - dst.y) * factor_y;
			old = *(mu_Color *)&((u32_t *)render->pixels)[y * width + x];
			float a = color.a * atlas_texture[src_y * ATLAS_HEIGHT + src_x] / (255.0 * 255.0);
			new.r = a * (color.r - old.r) + old.r;
			new.g = a * (color.g - old.g) + old.g;
			new.b = a * (color.b - old.b) + old.b;
			((u32_t *)render->pixels)[y * width + x] = *(int *)&new;
        }
    }
}

static void disp_clear(mu_Color color)
{
    int32_t x;
    int32_t y;
    for(y = 0; y < height; y++) {
        for(x = 0; x < width; x++) {
			((u32_t *)render->pixels)[y * width + x] = *(int *)&color;
        }
    }
}

static void flush(void) {
  if (buf_idx == 0) { return; }

  for(int i = 0; i < buf_idx; i++){
	  disp_flush(vert_buf[i], tex_buf[i], color_buf[i]);
  }
  buf_idx = 0;
}


static void push_quad(mu_Rect dst, mu_Rect src, mu_Color color) {
  if (buf_idx == BUFFER_SIZE) { flush(); }
  /* update texture buffer */
  tex_buf[buf_idx] = src;
  /* update vertex buffer */
  vert_buf[buf_idx] = dst;
  /* update color buffer */
  color_buf[buf_idx] = color;
  buf_idx++;
}


void r_draw_rect(mu_Rect rect, mu_Color color) {
  push_quad(rect, atlas[ATLAS_WHITE], color);
}


void r_draw_text(const char *text, mu_Vec2 pos, mu_Color color) {
  mu_Rect dst = { pos.x, pos.y, 0, 0 };
  for (const char *p = text; *p; p++) {
    if ((*p & 0xc0) == 0x80) { continue; }
    int chr = mu_min((unsigned char) *p, 127);
    mu_Rect src = atlas[ATLAS_FONT + chr];
    dst.w = src.w;
    dst.h = src.h;
    push_quad(dst, src, color);
    dst.x += dst.w;
  }
}


void r_draw_icon(int id, mu_Rect rect, mu_Color color) {
  mu_Rect src = atlas[id];
  int x = rect.x + (rect.w - src.w) / 2;
  int y = rect.y + (rect.h - src.h) / 2;
  push_quad(mu_rect(x, y, src.w, src.h), src, color);
}


int r_get_text_width(const char *text, int len) {
  int res = 0;
  for (const char *p = text; *p && len--; p++) {
    if ((*p & 0xc0) == 0x80) { continue; }
    int chr = mu_min((unsigned char) *p, 127);
    res += atlas[ATLAS_FONT + chr].w;
  }
  return res;
}


int r_get_text_height(void) {
  return 18;
}


void r_set_clip_rect(mu_Rect rect) {
  flush();
  //glScissor(rect.x, height - (rect.y + rect.h), rect.w, rect.h);
}


void r_clear(mu_Color clr) {
  flush();
  disp_clear(clr);
}


void r_present(void) {
  flush();
  framebuffer_present_render(fb, render, NULL, 0);
}
