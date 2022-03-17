#ifndef SPNAVCFG_H_
#define SPNAVCFG_H_

struct device_info {
	char *name;
	char *path;
	int naxes, nbuttons;
	int type;
};

struct device_image {
	int devtype;
	unsigned int tex;
	int width, height;
	int xoffs, yoffs;
};

int win_width, win_height;

#define MAX_BUTTONS		64

struct device_info devinf;
struct device_image devimg;

float sensitivity;
float sens_axis[6];
int invert;
int map_axis[6];
int map_bn[MAX_BUTTONS];
int dead_thres[6];
int led, grab;
/*int repeat_msec;*/

#endif	/* SPNAVCFG_H_ */
