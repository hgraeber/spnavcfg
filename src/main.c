/*
spnavcfg - an interactive GUI configurator for the spacenavd daemon.
Copyright (C) 2007-2022 John Tsiombikas <nuclear@siggraph.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <GL/glut.h>
#include <spnav.h>
#include "ui.h"

#define VIRT_HEIGHT	600
static int virt_width;

#define PX_TO_VX(x)		((x) * virt_width / win_width)
#define PY_TO_VY(y)		((y) * VIRT_HEIGHT / win_height)

static int init(void);
static void cleanup(void);
static void display(void);
static void draw_nk(void);
static float text_width(nk_handle nk, float h, const char *str, int len);
static void reshape(int x, int y);
static void keypress(unsigned char key, int x, int y);
static void keyrelease(unsigned char key, int x, int y);
static void skeypress(int key, int x, int y);
static void skeyrelease(int key, int x, int y);
static void mouse(int bn, int st, int x, int y);
static void motion(int x, int y);

static void errorbox(const char *msg);

static int win_width, win_height;
static unsigned int modkeys;
static struct nk_context nk;
static struct nk_user_font font;

static int fd;

#define MAX_BUTTONS		64

static float sensitivity;
static float sens_axis[6];
static int invert;
static int map_axis[6];
static int map_bn[MAX_BUTTONS];
static int dead_thres[6];
static int led, grab;
/*static int repeat_msec;*/


int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(800, 600);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow("spacenav configuration");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keypress);
	glutKeyboardUpFunc(keyrelease);
	glutSpecialFunc(skeypress);
	glutSpecialUpFunc(skeyrelease);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(motion);

	if(init() == -1) {
		return 1;
	}
	atexit(cleanup);

	nk_input_begin(&nk);
	glutMainLoop();
	return 0;
}

int init(void)
{
	int i;

	glEnable(GL_CULL_FACE);

	font.height = 16;
	font.width = text_width;

	nk_init_default(&nk, &font);
	nk_style_set_font(&nk, &font);

	if((fd = spnav_open()) == -1) {
		errorbox("Failed to connect to spacenavd!");
		return -1;
	}
	if(spnav_protocol() < 1) {
		errorbox("Currently running version of spacenavd is too old for this version of the configuration tool.\n"
				"\nEither update to a recent version of spacenavd (v0.9 or later), or downgrade to spnavcfg v0.3.1.");
		return -1;
	}
	spnav_client_name("spnavcfg");

	printf("Device: %s\n", spnav_dev_name(0, 0));
	printf("Path: %s\n", spnav_dev_path(0, 0));
	printf("Buttons: %d\n", spnav_dev_buttons());
	printf("Axes: %d\n", spnav_dev_axes());

	sensitivity = spnav_cfg_get_sens();
	spnav_cfg_get_axis_sens(sens_axis);
	invert = spnav_cfg_get_invert();
	led = spnav_cfg_get_led();
	grab = spnav_cfg_get_grab();

	for(i=0; i<6; i++) {
		map_axis[i] = spnav_cfg_get_axismap(i);
		dead_thres[i] = spnav_cfg_get_deadzone(i);
	}
	for(i=0; i<MAX_BUTTONS; i++) {
		map_bn[i] = spnav_cfg_get_bnmap(i);
	}
	return 0;
}

static void cleanup(void)
{
	spnav_close();
}

static void display(void)
{
	static int foo1, foo2;

	nk_input_end(&nk);

	glClear(GL_COLOR_BUFFER_BIT);

	nk_begin(&nk, "foo", nk_rect(20, 20, 200, 200), NK_WINDOW_BORDER);
	nk_layout_row_dynamic(&nk, 30, 2);
	if(nk_button_label(&nk, "red")) glClearColor(1, 0, 0, 1);
	if(nk_button_label(&nk, "black")) glClearColor(0, 0, 0, 1);
	nk_layout_row_dynamic(&nk, 30, 2);
	if(nk_option_label(&nk, "foo1", foo1)) foo1 = 1;
	if(nk_option_label(&nk, "foo2", foo2)) foo2 = 1;
	nk_end(&nk);

	draw_nk();

	glutSwapBuffers();
	assert(glGetError() == GL_NO_ERROR);

	nk_input_begin(&nk);
}

static void draw_nk(void)
{
	const struct nk_command *cmd = 0;

	glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);
	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	nk_foreach(cmd, &nk) {
		switch(cmd->type) {
		case NK_COMMAND_SCISSOR:
			nkgfx_clip((struct nk_command_scissor*)cmd);
			break;
		case NK_COMMAND_LINE:
			nkgfx_line((struct nk_command_line*)cmd);
			break;
		case NK_COMMAND_CURVE:
			nkgfx_curve((struct nk_command_curve*)cmd);
			break;
		case NK_COMMAND_RECT:
			nkgfx_rect((struct nk_command_rect*)cmd);
			break;
		case NK_COMMAND_RECT_FILLED:
			nkgfx_fillrect((struct nk_command_rect_filled*)cmd);
			break;
		case NK_COMMAND_RECT_MULTI_COLOR:
			nkgfx_colrect((struct nk_command_rect_multi_color*)cmd);
			break;
		case NK_COMMAND_CIRCLE:
			nkgfx_circle((struct nk_command_circle*)cmd);
			break;
		case NK_COMMAND_CIRCLE_FILLED:
			nkgfx_fillcircle((struct nk_command_circle_filled*)cmd);
			break;
		case NK_COMMAND_ARC:
			nkgfx_arc((struct nk_command_arc*)cmd);
			break;
		case NK_COMMAND_ARC_FILLED:
			nkgfx_fillarc((struct nk_command_arc_filled*)cmd);
			break;
		case NK_COMMAND_TRIANGLE:
			nkgfx_tri((struct nk_command_triangle*)cmd);
			break;
		case NK_COMMAND_TRIANGLE_FILLED:
			nkgfx_filltri((struct nk_command_triangle_filled*)cmd);
			break;
		case NK_COMMAND_POLYGON:
			nkgfx_poly((struct nk_command_polygon*)cmd);
			break;
		case NK_COMMAND_POLYGON_FILLED:
			nkgfx_fillpoly((struct nk_command_polygon_filled*)cmd);
			break;
		case NK_COMMAND_POLYLINE:
			nkgfx_polyline((struct nk_command_polyline*)cmd);
			break;
		case NK_COMMAND_TEXT:
			nkgfx_text((struct nk_command_text*)cmd);
			break;
		case NK_COMMAND_IMAGE:
			nkgfx_image((struct nk_command_image*)cmd);
			break;
		default:
			break;
		}
	}
	glPopAttrib();
}

static float text_width(nk_handle nk, float h, const char *str, int len)
{
	int i, res = 0;
	for(i=0; i<len; i++) {
		res += glutStrokeWidth(GLUT_STROKE_ROMAN, *str++);
	}
	return PX_TO_VX(res);
}

static void reshape(int x, int y)
{
	float aspect = (float)x / (float)y;
	win_width = x;
	win_height = y;

	virt_width = aspect * VIRT_HEIGHT;

	glViewport(0, 0, x, y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, virt_width, 0, VIRT_HEIGHT, -1, 1);
}

static void handle_key(int key, int press)
{
	switch(key) {
	case 'c':
		if(modkeys & GLUT_ACTIVE_CTRL) {
			nk_input_key(&nk, NK_KEY_COPY, press);
		}
		break;
	case 'v':
		if(modkeys & GLUT_ACTIVE_CTRL) {
			nk_input_key(&nk, NK_KEY_PASTE, press);
		}
		break;
	case 'x':
		if(modkeys & GLUT_ACTIVE_CTRL) {
			nk_input_key(&nk, NK_KEY_TEXT_START, press);
			nk_input_key(&nk, NK_KEY_SCROLL_START, press);
		}
		break;
	case 'z':
		if(modkeys & GLUT_ACTIVE_CTRL) {
			nk_input_key(&nk, NK_KEY_TEXT_UNDO, press);
		}
		break;
	case 'r':
		if(modkeys & GLUT_ACTIVE_CTRL) {
			nk_input_key(&nk, NK_KEY_TEXT_REDO, press);
		}
		break;
	case 127:
		nk_input_key(&nk, NK_KEY_DEL, press);
		break;
	case '\b':
		nk_input_key(&nk, NK_KEY_BACKSPACE, press);
		break;
	case '\n':
	case '\r':
		nk_input_key(&nk, NK_KEY_ENTER, press);
		break;
	default:
		if(isalpha(key)) {
			if(modkeys & GLUT_ACTIVE_SHIFT) {
				nk_input_key(&nk, toupper(key), press);
			} else {
				nk_input_key(&nk, key, press);
			}
		}
		break;
	}
}

static void handle_skey(int key, int press)
{
	switch(key) {
	case GLUT_KEY_LEFT:
		nk_input_key(&nk, NK_KEY_LEFT, press);
		break;
	case GLUT_KEY_UP:
		nk_input_key(&nk, NK_KEY_UP, press);
		break;
	case GLUT_KEY_RIGHT:
		nk_input_key(&nk, NK_KEY_RIGHT, press);
		break;
	case GLUT_KEY_DOWN:
		nk_input_key(&nk, NK_KEY_DOWN, press);
		break;
	case GLUT_KEY_PAGE_UP:
		nk_input_key(&nk, NK_KEY_SCROLL_UP, press);
		break;
	case GLUT_KEY_PAGE_DOWN:
		nk_input_key(&nk, NK_KEY_SCROLL_DOWN, press);
		break;
	case GLUT_KEY_HOME:
		nk_input_key(&nk, NK_KEY_SCROLL_START, press);
		break;
	case GLUT_KEY_END:
		nk_input_key(&nk, NK_KEY_SCROLL_END, press);
		break;
	default:
		break;
	}
}

static void keypress(unsigned char key, int x, int y)
{
	if(key == 27) exit(0);

	modkeys = glutGetModifiers();
	handle_key(key, 1);
}

static void keyrelease(unsigned char key, int x, int y)
{
	modkeys = glutGetModifiers();
	handle_key(key, 0);
}

static void skeypress(int key, int x, int y)
{
	modkeys = glutGetModifiers();
	handle_skey(key, 1);
}

static void skeyrelease(int key, int x, int y)
{
	modkeys = glutGetModifiers();
	handle_skey(key, 0);
}

static void mouse(int bn, int st, int x, int y)
{
	long tm;
	static long last_click;
	int press = st == GLUT_DOWN;
	int bidx = bn - GLUT_LEFT_BUTTON;
	x = PX_TO_VX(x);
	y = PY_TO_VY(y);

	if(bn == GLUT_LEFT_BUTTON) {
		if(press) {
			tm = glutGet(GLUT_ELAPSED_TIME);
			if(tm - last_click > 20 && tm - last_click < 200) {
				nk_input_button(&nk, NK_BUTTON_DOUBLE, x, y, 1);
			}
			last_click = tm;
		} else {
			nk_input_button(&nk, NK_BUTTON_DOUBLE, x, y, 0);
		}
	}

	nk_input_button(&nk, bidx, x, y, press);
}

static void motion(int x, int y)
{
	nk_input_motion(&nk, PX_TO_VX(x), PY_TO_VY(y));
}

static void errorbox(const char *msg)
{
	fprintf(stderr, "error: %s\n", msg);
}
