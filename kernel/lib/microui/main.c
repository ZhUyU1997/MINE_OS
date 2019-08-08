#include <stdio.h>
#include <time/timer.h>
#include <framebuffer/framebuffer.h>
#include <core/event.h>
#include <time/delay.h>
#include <clocksource/clocksource.h>

#include "renderer.h"
#include "microui.h"


static char logbuf[64000];
static int logbuf_updated = 0;
static float bg[3] = { 90, 95, 100 };


static void write_log(const char *text) {
	if (logbuf[0]) { strcat(logbuf, "\n"); }
	strcat(logbuf, text);
	logbuf_updated = 1;
}


static void test_window(mu_Context *ctx) {
	static mu_Container window;

	/* init window manually so we can set its position and size */
	if (!window.inited) {
		mu_init_window(ctx, &window, 0);
		window.rect = mu_rect(40, 40, 300, 450);
	}

	/* limit window to minimum size */
	window.rect.w = mu_max(window.rect.w, 240);
	window.rect.h = mu_max(window.rect.h, 300);


	/* do window */
	if (mu_begin_window(ctx, &window, "Demo Window")) {

		/* window info */
		static int show_info = 1;
		if (mu_header(ctx, &show_info, "Window Info")) {
			char buf[64];
			mu_layout_row(ctx, 2, (int[]) { 54, -1 }, 0);
			mu_label(ctx,"Position:");
			sprintf(buf, "%d, %d", ctx->mouse_pos.x, ctx->mouse_pos.y); mu_label(ctx, buf);
			mu_label(ctx, "Size:");
			sprintf(buf, "%d, %d", window.rect.w, window.rect.h); mu_label(ctx, buf);
		}

		/* labels + buttons */
		static int show_buttons = 1;
		if (mu_header(ctx, &show_buttons, "Test Buttons")) {
			mu_layout_row(ctx, 3, (int[]) { 86, -110, -1 }, 0);
			mu_label(ctx, "Test buttons 1:");
			if (mu_button(ctx, "Button 1")) { write_log("Pressed button 1"); }
			if (mu_button(ctx, "Button 2")) { write_log("Pressed button 2"); }
			mu_label(ctx, "Test buttons 2:");
			if (mu_button(ctx, "Button 3")) { write_log("Pressed button 3"); }
			if (mu_button(ctx, "Button 4")) { write_log("Pressed button 4"); }
		}

		/* tree */
		static int show_tree = 1;
		if (mu_header(ctx, &show_tree, "Tree and Text")) {
			mu_layout_row(ctx, 2, (int[]) { 140, -1 }, 0);
			mu_layout_begin_column(ctx);
			static int states[8];
			if (mu_begin_treenode(ctx, &states[0], "Test 1")) {
				if (mu_begin_treenode(ctx, &states[1], "Test 1a")) {
					mu_label(ctx, "Hello");
					mu_label(ctx, "world");
					mu_end_treenode(ctx);
				}
				if (mu_begin_treenode(ctx, &states[2], "Test 1b")) {
					if (mu_button(ctx, "Button 1")) { write_log("Pressed button 1"); }
					if (mu_button(ctx, "Button 2")) { write_log("Pressed button 2"); }
					mu_end_treenode(ctx);
				}
				mu_end_treenode(ctx);
			}
			if (mu_begin_treenode(ctx, &states[3], "Test 2")) {
				mu_layout_row(ctx, 2, (int[]) { 54, 54 }, 0);
				if (mu_button(ctx, "Button 3")) { write_log("Pressed button 3"); }
				if (mu_button(ctx, "Button 4")) { write_log("Pressed button 4"); }
				if (mu_button(ctx, "Button 5")) { write_log("Pressed button 5"); }
				if (mu_button(ctx, "Button 6")) { write_log("Pressed button 6"); }
				mu_end_treenode(ctx);
			}
			if (mu_begin_treenode(ctx, &states[4], "Test 3")) {
				static int checks[3] = { 1, 0, 1 };
				mu_checkbox(ctx, &checks[0], "Checkbox 1");
				mu_checkbox(ctx, &checks[1], "Checkbox 2");
				mu_checkbox(ctx, &checks[2], "Checkbox 3");
				mu_end_treenode(ctx);
			}
			mu_layout_end_column(ctx);

			mu_layout_begin_column(ctx);
			mu_layout_row(ctx, 1, (int[]) { -1 }, 0);
			mu_text(ctx, "Lorem ipsum dolor sit amet, consectetur adipiscing "
				"elit. Maecenas lacinia, sem eu lacinia molestie, mi risus faucibus "
				"ipsum, eu varius magna felis a nulla.");
			mu_layout_end_column(ctx);
		}

		/* background color sliders */
		static int show_sliders = 1;
		if (mu_header(ctx, &show_sliders, "Background Color")) {
			mu_layout_row(ctx, 2, (int[]) { -78, -1 }, 74);
			/* sliders */
			mu_layout_begin_column(ctx);
			mu_layout_row(ctx, 2, (int[]) { 46, -1 }, 0);
			mu_label(ctx, "Red:");   mu_slider(ctx, &bg[0], 0, 255);
			mu_label(ctx, "Green:"); mu_slider(ctx, &bg[1], 0, 255);
			mu_label(ctx, "Blue:");  mu_slider(ctx, &bg[2], 0, 255);
			mu_layout_end_column(ctx);
			/* color preview */
			mu_Rect r = mu_layout_next(ctx);
			mu_draw_rect(ctx, r, mu_color(bg[0], bg[1], bg[2], 255));
			char buf[32];
			sprintf(buf, "#%02X%02X%02X", (int) bg[0], (int) bg[1], (int) bg[2]);
			mu_draw_control_text(ctx, buf, r, MU_COLOR_TEXT, MU_OPT_ALIGNCENTER);
		}

		mu_end_window(ctx);
	}
}


static void log_window(mu_Context *ctx) {
	static mu_Container window;

	/* init window manually so we can set its position and size */
	if (!window.inited) {
		mu_init_window(ctx, &window, 0);
		window.rect = mu_rect(350, 40, 300, 200);
	}

	if (mu_begin_window(ctx, &window, "Log Window")) {

		/* output text panel */
		static mu_Container panel;
		mu_layout_row(ctx, 1, (int[]) { -1 }, -28);
		mu_begin_panel(ctx, &panel);
		mu_layout_row(ctx, 1, (int[]) { -1 }, -1);
		mu_text(ctx, logbuf);
		mu_end_panel(ctx);
		if (logbuf_updated) {
			panel.scroll.y = panel.content_size.y;
			logbuf_updated = 0;
		}

		/* input textbox + submit button */
		static char buf[128];
		int submitted = 0;
		mu_layout_row(ctx, 2, (int[]) { -70, -1 }, 0);
		if (mu_textbox(ctx, buf, sizeof(buf)) & MU_RES_SUBMIT) {
			mu_set_focus(ctx, ctx->last_id);
			submitted = 1;
		}
		if (mu_button(ctx, "Submit")) { submitted = 1; }
		if (submitted) {
			write_log(buf);
			buf[0] = '\0';
		}

		mu_end_window(ctx);
	}
}


static int uint8_slider(mu_Context *ctx, unsigned char *value, int low, int high) {
	static float tmp;
	mu_push_id(ctx, &value, sizeof(value));
	tmp = *value;
	int res = mu_slider_ex(ctx, &tmp, low, high, 0, "%.0f", MU_OPT_ALIGNCENTER);
	*value = tmp;
	mu_pop_id(ctx);
	return res;
}


static void style_window(mu_Context *ctx) {
	static mu_Container window;

	/* init window manually so we can set its position and size */
	if (!window.inited) {
		mu_init_window(ctx, &window, 0);
		window.rect = mu_rect(350, 250, 300, 240);
	}

	static struct { const char *label; int idx; } colors[] = {
		{ "text:",         MU_COLOR_TEXT        },
		{ "border:",       MU_COLOR_BORDER      },
		{ "windowbg:",     MU_COLOR_WINDOWBG    },
		{ "titlebg:",      MU_COLOR_TITLEBG     },
		{ "titletext:",    MU_COLOR_TITLETEXT   },
		{ "panelbg:",      MU_COLOR_PANELBG     },
		{ "button:",       MU_COLOR_BUTTON      },
		{ "buttonhover:",  MU_COLOR_BUTTONHOVER },
		{ "buttonfocus:",  MU_COLOR_BUTTONFOCUS },
		{ "base:",         MU_COLOR_BASE        },
		{ "basehover:",    MU_COLOR_BASEHOVER   },
		{ "basefocus:",    MU_COLOR_BASEFOCUS   },
		{ "scrollbase:",   MU_COLOR_SCROLLBASE  },
		{ "scrollthumb:",  MU_COLOR_SCROLLTHUMB },
		{ NULL }
	};

	if (mu_begin_window(ctx, &window, "Style Editor")) {
		int sw = mu_get_container(ctx)->body.w * 0.14;
		mu_layout_row(ctx, 6, (int[]) { 80, sw, sw, sw, sw, -1 }, 0);
		for (int i = 0; colors[i].label; i++) {
			mu_label(ctx, colors[i].label);
			uint8_slider(ctx, &ctx->style->colors[i].r, 0, 255);
			uint8_slider(ctx, &ctx->style->colors[i].g, 0, 255);
			uint8_slider(ctx, &ctx->style->colors[i].b, 0, 255);
			uint8_slider(ctx, &ctx->style->colors[i].a, 0, 255);
			mu_draw_rect(ctx, mu_layout_next(ctx), ctx->style->colors[i]);
		}
		mu_end_window(ctx);
	}
}


static void process_frame(mu_Context *ctx) {
	mu_begin(ctx);
	test_window(ctx);
	log_window(ctx);
	style_window(ctx);
	mu_end(ctx);
}


static int text_width(mu_Font font, const char *text, int len) {
	if (len == -1) { len = strlen(text); }
	return r_get_text_width(text, len);
}

static int text_height(mu_Font font) {
	return r_get_text_height();
}

static const char button_map[256] = {
	[ MOUSE_BUTTON_LEFT   & 0xff ] =  MU_MOUSE_LEFT,
	[ MOUSE_BUTTON_RIGHT  & 0xff ] =  MU_MOUSE_RIGHT,
	[ MOUSE_BUTTON_MIDDLE & 0xff ] =  MU_MOUSE_MIDDLE,
};

static bool_t ex_tp_read(mu_Context *ctx)
{
	struct event_t e;
	if(!pump_event(&e))
		return FALSE;

	switch(e.type)
	{
		case EVENT_TYPE_MOUSE_DOWN:
			mu_input_mousedown(ctx, e.e.mouse_down.x, e.e.mouse_down.y, button_map[e.e.mouse_down.button]);
			break;
		case EVENT_TYPE_MOUSE_UP:
			mu_input_mouseup(ctx, e.e.mouse_up.x, e.e.mouse_up.y, button_map[e.e.mouse_up.button]);
			break;
		case EVENT_TYPE_MOUSE_WHEEL:
			mu_input_scroll(ctx, 0, e.e.mouse_wheel.dy * -30);
			break;
		case EVENT_TYPE_MOUSE_MOVE:
			mu_input_mousemove(ctx, e.e.mouse_move.x, e.e.mouse_move.y);
			break;
		case EVENT_TYPE_TOUCH_BEGIN:
			mu_input_mousedown(ctx, e.e.touch_begin.x, e.e.touch_begin.y, MU_MOUSE_LEFT);
			break;
		case EVENT_TYPE_TOUCH_MOVE:
			mu_input_mousemove(ctx, e.e.touch_begin.x, e.e.touch_begin.y);
			break;
		case EVENT_TYPE_TOUCH_END:
			mu_input_mouseup(ctx, e.e.touch_begin.x, e.e.touch_begin.y, MU_MOUSE_LEFT);
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

int microui() {
	r_init();

	/* init microui */
	mu_Context *ctx = malloc(sizeof(mu_Context));
	mu_init(ctx);
	ctx->text_width = text_width;
	ctx->text_height = text_height;
	char buf[20];
	uint64_t now,before = ktime_to_us(ktime_get());
	/* main loop */
	for (;;) {
		/* handle events */
		while (ex_tp_read(ctx));
		/* process frame */
		process_frame(ctx);
		/* render */
		r_clear(mu_color(bg[0], bg[1], bg[2], 255));
		mu_Command *cmd = NULL;
		while (cmd = mu_next_command(ctx, cmd)) {
			switch (cmd->type) {
				case MU_COMMAND_TEXT: r_draw_text(cmd->text.str, cmd->text.pos, cmd->text.color); break;
				case MU_COMMAND_RECT: r_draw_rect(cmd->rect.rect, cmd->rect.color); break;
				case MU_COMMAND_ICON: r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color); break;
				case MU_COMMAND_CLIP: r_set_clip_rect(cmd->clip.rect); break;
			}
		}
		mu_Rect rect = {
			.x = ctx->mouse_pos.x - 8,
			.y = ctx->mouse_pos.y - 8,
			.w = 16,
			.h = 16,
		};
		mu_Vec2 pos = {0,0};
		mu_Color color = {255,255,255,255};
		now = ktime_to_us(ktime_get());
		sprintf(buf, "%.2f", 1000000.0 / (now - before));
		before = ktime_to_us(ktime_get());
		r_draw_text(buf, pos, color);
		r_draw_icon(MU_ICON_CLOSE, rect, color);
		r_present();
	}

	return 0;
}


