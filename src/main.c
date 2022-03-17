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
#include <assert.h>
#include <GL/glut.h>
#include <spnav.h>
#include <imago2.h>
#include "spnavcfg.h"
#include "ui.h"

static int init(void);
static void cleanup(void);
static void display(void);
static void reshape(int x, int y);
static void keypress(unsigned char key, int x, int y);
static void keyrelease(unsigned char key, int x, int y);
static void skeypress(int key, int x, int y);
static void skeyrelease(int key, int x, int y);
static void mouse(int bn, int st, int x, int y);
static void motion(int x, int y);

static void errorbox(const char *msg);
static size_t imgread(void *buf, size_t bytes, void *uptr);
static long imgseek(long offs, int whence, void *uptr);

static unsigned int modkeys;

static int fd;

static struct device_image devimglist[] = {
	{SPNAV_DEV_UNKNOWN,		0, 256, 256, 0, 0},
	{SPNAV_DEV_SB2003,		0, 256, 256, 1, 0},
	{SPNAV_DEV_SB3003,		0, 256, 256, 2, 0},
	{SPNAV_DEV_SB4000,		0, 256, 256, 3, 0},
	{SPNAV_DEV_SM,			0, 256, 256, 5, 0},
	{SPNAV_DEV_SM5000,		0, 256, 256, 2, 0},
	{SPNAV_DEV_SMCADMAN,	0, 256, 256, 6, 0},
	{SPNAV_DEV_PLUSXT,		0, 256, 256, 5, 0},
	{SPNAV_DEV_CADMAN,		0, 256, 256, 6, 0},
	{SPNAV_DEV_SMCLASSIC,	0, 256, 256, 4, 0},
	{SPNAV_DEV_SB5000,		0, 256, 256, 3, 0},
	{SPNAV_DEV_STRAVEL,		0, 256, 256, 2, 1},
	{SPNAV_DEV_SPILOT,		0, 256, 256, 3, 1},
	{SPNAV_DEV_SNAV,		0, 256, 256, 0, 1},
	{SPNAV_DEV_SEXP,		0, 256, 256, 4, 1},
	{SPNAV_DEV_SNAVNB,		0, 256, 256, 1, 1},
	{SPNAV_DEV_SPILOTPRO,	0, 256, 256, 5, 1},
	{SPNAV_DEV_SMPRO,		0, 256, 256, 6, 1},
	{SPNAV_DEV_NULOOQ,		0, 256, 256, 7, 0},
	{SPNAV_DEV_SMW,			0, 256, 256, 1, 2},
	{SPNAV_DEV_SMPROW,		0, 256, 256, 6, 1},
	{SPNAV_DEV_SMENT,		0, 256, 256, 7, 1},
	{SPNAV_DEV_SMCOMP,		0, 256, 256, 0, 2},
	{SPNAV_DEV_SMMOD,		0, 256, 256, 0, 0},
	{-1}
};


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

	glutMainLoop();
	return 0;
}

int init(void)
{
	int i;
	struct img_pixmap img;
	struct img_io io = {0, imgread, 0, imgseek};

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

	devinf.name = strdup(spnav_dev_name(0, 0));
	devinf.path = strdup(spnav_dev_path(0, 0));
	devinf.type = spnav_dev_type();
	devinf.nbuttons = spnav_dev_buttons();
	devinf.naxes = spnav_dev_axes();

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

	img_init(&img);
	if(img_read(&img, &io) != -1) {
		int i, ncol, nrow;
		devimg = devimglist[0];
		for(i=0; devimglist[i].devtype != -1; i++) {
			if(devimglist[i].devtype == devinf.type) {
				devimg = devimglist[i];
				break;
			}
		}
		devimg.tex = img_gltexture(&img);
		ncol = img.width / devimg.width;
		nrow = img.height / devimg.height;
		devimg.xoffs = devimg.xoffs * img.width / ncol;
		devimg.yoffs = devimg.yoffs * img.height / nrow;
		img_destroy(&img);
	} else {
		fprintf(stderr, "failed to decode device images\n");
	}

	if(init_ui() == -1) {
		fprintf(stderr, "failed to initialize user interface\n");
		return -1;
	}

	return 0;
}

static void cleanup(void)
{
	spnav_close();
	cleanup_ui();
}

static void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	draw_ui();

	glutSwapBuffers();
	assert(glGetError() == GL_NO_ERROR);
}

static void reshape(int x, int y)
{
	glViewport(0, 0, x, y);
	reshape_ui(x, y);
}


static void keypress(unsigned char key, int x, int y)
{
	if(key == 27) exit(0);

	modkeys = glutGetModifiers();
	ui_input_key(key, 1, modkeys);
}

static void keyrelease(unsigned char key, int x, int y)
{
	modkeys = glutGetModifiers();
	ui_input_key(key, 0, modkeys);
}

static void skeypress(int key, int x, int y)
{
	modkeys = glutGetModifiers();
	ui_input_special(key, 1, modkeys);
}

static void skeyrelease(int key, int x, int y)
{
	modkeys = glutGetModifiers();
	ui_input_special(key, 0, modkeys);
}

static void mouse(int bn, int st, int x, int y)
{
	int press = st == GLUT_DOWN;
	int bidx = bn - GLUT_LEFT_BUTTON;

	ui_input_mbutton(bidx, press, x, y);
}

static void motion(int x, int y)
{
	ui_input_mmotion(x, y);
}

static void errorbox(const char *msg)
{
	fprintf(stderr, "error: %s\n", msg);

}

extern char devices_png[];
extern int devices_png_size;
static int cur_offs;

static size_t imgread(void *buf, size_t bytes, void *uptr)
{
	long sz, rem = devices_png_size - cur_offs;

	if((sz = bytes < rem ? bytes : rem) <= 0) return 0;

	memcpy(buf, devices_png + cur_offs, sz);
	cur_offs += sz;
	return sz;
}

static long imgseek(long offs, int whence, void *uptr)
{
	switch(whence) {
	case SEEK_SET:
		cur_offs = offs;
		break;
	case SEEK_END:
		cur_offs = devices_png_size - offs;
		break;
	case SEEK_CUR:
		cur_offs += offs;
	default:
		break;
	}
	return cur_offs;
}
