#include <stdio.h>
#include <ctype.h>
#include <GL/glut.h>
#include "ui.h"
#include "cgmath/cgmath.h"


#define TEXT_SCALE	0.14
#define TEXT_HEIGHT	20


static void draw_nk(void);
static float text_width(nk_handle nk, float h, const char *str, int len);

static struct nk_context nk;
static struct nk_user_font font;
static struct nk_image nkdevimg;

int init_ui(void)
{
	struct nk_rect sub;

	font.height = TEXT_HEIGHT * 0.75;
	font.width = text_width;

	nk_init_default(&nk, &font);
	nk_style_set_font(&nk, &font);

	sub = nk_recti(devimg.xoffs, devimg.yoffs, devimg.width, devimg.height);
	nkdevimg = nk_subimage_id(devimg.tex, 2048, 1024, sub);

	nk_input_begin(&nk);
	return 0;
}

void cleanup_ui(void)
{
	nk_free(&nk);
}

void draw_ui(void)
{
	nk_input_end(&nk);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, virt_width, VIRT_HEIGHT, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);

	nk_begin(&nk, "Device info", nk_rect(0, 0, 400, 200), NK_WINDOW_BORDER | NK_WINDOW_TITLE);
	nk_layout_row_dynamic(&nk, 0, 1);
	nk_labelf(&nk, NK_TEXT_LEFT, "Name: %s", devinf.name);
	nk_labelf(&nk, NK_TEXT_LEFT, "Path: %s", devinf.path);
	nk_labelf(&nk, NK_TEXT_LEFT, "Axes: %d", devinf.naxes);
	nk_labelf(&nk, NK_TEXT_LEFT, "Buttons: %d", devinf.nbuttons);
	nk_end(&nk);

	nk_begin(&nk, "Device picture", nk_rect(400, 0, 200, 200), NK_WINDOW_BORDER);
	nk_layout_row_dynamic(&nk, 175, 1);
	nk_image(&nk, nkdevimg);
	nk_end(&nk);

	nk_begin(&nk, "Axes", nk_rect(0, 200, 600, 200), NK_WINDOW_BORDER | NK_WINDOW_TITLE);
	nk_layout_row_dynamic(&nk, 0, 3);
	nk_label(&nk, "Sensitivity", NK_TEXT_RIGHT);
	sensitivity = nk_slide_float(&nk, 0, sensitivity, 6, 0.1);
	if((swap_yz = nk_check_label(&nk, "Swap Y-Z Axis", swap_yz))) {
	}
	nk_end(&nk);

	/*
	nk_begin(&nk, "foo", nk_rect(200, 80, 200, 200), NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_MOVABLE);
	nk_layout_row_dynamic(&nk, 30, 2);
	if(nk_button_label(&nk, "red")) glClearColor(1, 0, 0, 1);
	if(nk_button_label(&nk, "black")) glClearColor(0, 0, 0, 1);
	nk_layout_row_dynamic(&nk, 30, 2);
	foo1 = nk_option_label(&nk, "foo1", foo1);
	foo2 = nk_option_label(&nk, "foo2", foo2);
	nk_end(&nk);
	*/

	draw_nk();
	nk_clear(&nk);

	nk_input_begin(&nk);
}

void reshape_ui(int x, int y)
{
	float aspect = (float)x / (float)y;
	win_width = x;
	win_height = y;

	virt_width = aspect * VIRT_HEIGHT;
}

void ui_input_key(int key, int press, unsigned int modkeys)
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

	glutPostRedisplay();
}

void ui_input_special(int key, int press, unsigned int modkeys)
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
		return;
	}

	glutPostRedisplay();
}

void ui_input_mbutton(int bn, int press, int x, int y)
{
	long tm;
	static long last_click;
	x = PX_TO_VX(x);
	y = PY_TO_VY(y);

	if(bn == 0) {
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

	nk_input_button(&nk, bn, x, y, press);
	glutPostRedisplay();
}

void ui_input_mmotion(int x, int y)
{
	nk_input_motion(&nk, PX_TO_VX(x), PY_TO_VY(y));
	glutPostRedisplay();
}

/* -------- nuklear drawing command handling -------- */

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

#define setcolor(c)	glColor4ubv(&(c).r)
#define draw_line(x0, y0, x1, y1)	(glVertex2i(x0, y0), glVertex2i(x1, y1))

static void draw_arc(float cx, float cy, float r, float a0, float a1, int nseg)
{
	int i;
	float da = (a1 - a0) / (nseg - 1);
	float theta = a0;
	for(i=0; i<nseg; i++) {
		glVertex2f(cx + cos(theta) * r, cy + sin(theta) * r);
		theta += da;
	}
}

static void draw_rect(float x, float y, float w, float h)
{
	glVertex2f(x, y);
	glVertex2f(x + w, y);
	glVertex2f(x + w, y + h);
	glVertex2f(x, y + h);
}


void nkgfx_clip(struct nk_command_scissor *cmd)
{
	cmd->x = VX_TO_PX(cmd->x);
	cmd->y = VY_TO_PY(cmd->y);
	cmd->w = VX_TO_PX(cmd->w);
	cmd->h = VY_TO_PY(cmd->h);
	cmd->y = win_height - cmd->y - cmd->h;
	glScissor(cmd->x, cmd->y, cmd->w, cmd->h);
}

void nkgfx_line(struct nk_command_line *cmd)
{
	glLineWidth(cmd->line_thickness);
	glBegin(GL_LINES);
	setcolor(cmd->color);
	glVertex2i(cmd->begin.x, cmd->begin.y);
	glVertex2i(cmd->end.x, cmd->end.y);
	glEnd();
}

void nkgfx_curve(struct nk_command_curve *cmd)
{
	int i;
	float x, y, t;

	glLineWidth(cmd->line_thickness);
	glBegin(GL_LINE_STRIP);
	setcolor(cmd->color);
	for(i=0; i<8; i++) {
		t = (float)i / 7.0f;
		x = cgm_bezier(cmd->begin.x, cmd->ctrl[0].x, cmd->ctrl[1].x, cmd->end.x, t);
		y = cgm_bezier(cmd->begin.y, cmd->ctrl[0].y, cmd->ctrl[1].y, cmd->end.y, t);
		glVertex2f(x, y);
	}
	glEnd();
}

void nkgfx_rect(struct nk_command_rect *cmd)
{
	int rnd = cmd->rounding;

	glLineWidth(cmd->line_thickness);

	glBegin(GL_LINE_LOOP);
	setcolor(cmd->color);

	if(rnd) {
		draw_arc(cmd->x + cmd->w - rnd, cmd->y + cmd->h - rnd, rnd, 0, M_PI / 2.0f, 8);
		draw_arc(cmd->x + rnd, cmd->y + cmd->h - rnd, rnd, M_PI / 2.0f, M_PI, 8);
		draw_arc(cmd->x + rnd, cmd->y + rnd, rnd, M_PI, 3.0f * M_PI / 2.0f, 8);
		draw_arc(cmd->x + cmd->w - rnd, cmd->y + rnd, rnd, 3.0f * M_PI / 2.0f, M_PI * 2.0f, 8);
	} else {
		glVertex2i(cmd->x, cmd->y);
		glVertex2i(cmd->x + cmd->w, cmd->y);
		glVertex2i(cmd->x + cmd->w, cmd->y + cmd->h);
		glVertex2i(cmd->x, cmd->y + cmd->h);
	}
	glEnd();
}

void nkgfx_fillrect(struct nk_command_rect_filled *cmd)
{
	int rnd = cmd->rounding;

	setcolor(cmd->color);

	if(rnd) {
		glBegin(GL_QUADS);
		draw_rect(cmd->x + rnd, cmd->y, cmd->w - rnd * 2, cmd->h);
		draw_rect(cmd->x, cmd->y + rnd, rnd, cmd->h - rnd * 2);
		draw_rect(cmd->x + cmd->w - rnd, cmd->y + rnd, rnd, cmd->h - rnd * 2);
		glEnd();

		glBegin(GL_TRIANGLE_FAN);
		glVertex2i(cmd->x + cmd->w - rnd, cmd->y + cmd->h - rnd);
		draw_arc(cmd->x + cmd->w - rnd, cmd->y + cmd->h - rnd, rnd, 0, M_PI / 2.0f, 8);
		glEnd();
		glBegin(GL_TRIANGLE_FAN);
		glVertex2i(cmd->x + rnd, cmd->y + cmd->h - rnd);
		draw_arc(cmd->x + rnd, cmd->y + cmd->h - rnd, rnd, M_PI / 2.0f, M_PI, 8);
		glEnd();
		glBegin(GL_TRIANGLE_FAN);
		glVertex2i(cmd->x + rnd, cmd->y + rnd);
		draw_arc(cmd->x + rnd, cmd->y + rnd, rnd, M_PI, 3.0f * M_PI / 2.0f, 8);
		glEnd();
		glBegin(GL_TRIANGLE_FAN);
		glVertex2i(cmd->x + cmd->w - rnd, cmd->y + rnd);
		draw_arc(cmd->x + cmd->w - rnd, cmd->y + rnd, rnd, 3.0f * M_PI / 2.0f, M_PI * 2.0f, 8);
		glEnd();
	} else {
		glBegin(GL_QUADS);
		draw_rect(cmd->x, cmd->y, cmd->w, cmd->h);
		glEnd();
	}
}

void nkgfx_colrect(struct nk_command_rect_multi_color *cmd)
{
	int r = ((int)cmd->left.r + cmd->top.r + cmd->bottom.r + cmd->right.r) >> 2;
	int g = ((int)cmd->left.g + cmd->top.g + cmd->bottom.g + cmd->right.g) >> 2;
	int b = ((int)cmd->left.b + cmd->top.b + cmd->bottom.b + cmd->right.b) >> 2;
	int a = ((int)cmd->left.a + cmd->top.a + cmd->bottom.a + cmd->right.a) >> 2;
	int mx = cmd->x + (cmd->w >> 1);
	int my = cmd->y + (cmd->h >> 1);

	glBegin(GL_TRIANGLES);
	setcolor(cmd->left);
	glVertex2i(cmd->x, cmd->y);
	setcolor(cmd->top);
	glVertex2i(cmd->x + cmd->w, cmd->y);
	glColor4ub(r, g, b, a);
	glVertex2i(mx, my);

	setcolor(cmd->top);
	glVertex2i(cmd->x + cmd->w, cmd->y);
	setcolor(cmd->bottom);
	glVertex2i(cmd->x + cmd->w, cmd->y + cmd->h);
	glColor4ub(r, g, b, a);
	glVertex2i(mx, my);

	setcolor(cmd->bottom);
	glVertex2i(cmd->x + cmd->w, cmd->y + cmd->h);
	setcolor(cmd->right);
	glVertex2i(cmd->x, cmd->y + cmd->h);
	glColor4ub(r, g, b, a);
	glVertex2i(mx, my);

	setcolor(cmd->right);
	glVertex2i(cmd->x, cmd->y + cmd->h);
	setcolor(cmd->left);
	glVertex2i(cmd->x, cmd->y);
	glColor4ub(r, g, b, a);
	glVertex2i(mx, my);
	glEnd();
}

void nkgfx_circle(struct nk_command_circle *cmd)
{
	glLineWidth(cmd->line_thickness);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(cmd->x + cmd->w / 2.0f, cmd->y + cmd->h / 2.0f, 0);
	glScalef(cmd->w / 2.0f, cmd->h / 2.0f, 1);

	glBegin(GL_LINE_LOOP);
	setcolor(cmd->color);
	draw_arc(0, 0, 1, 0, M_PI * 2.0f, 32);
	glEnd();

	glPopMatrix();
}

void nkgfx_fillcircle(struct nk_command_circle_filled *cmd)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(cmd->x + cmd->w / 2.0f, cmd->y + cmd->h / 2.0f, 0);
	glScalef(cmd->w / 2.0f, cmd->h / 2.0f, 1);

	glBegin(GL_TRIANGLE_FAN);
	setcolor(cmd->color);
	glVertex2i(0, 0);
	draw_arc(0, 0, 1, 0, M_PI * 2.0f, 32);
	glEnd();

	glPopMatrix();
}

void nkgfx_arc(struct nk_command_arc *cmd)
{
	glLineWidth(cmd->line_thickness);

	glBegin(GL_LINE_STRIP);
	setcolor(cmd->color);
	draw_arc(cmd->cx, cmd->cy, cmd->r, cmd->a[0], cmd->a[1], 24);
	glEnd();
}

void nkgfx_fillarc(struct nk_command_arc_filled *cmd)
{
	glBegin(GL_TRIANGLE_FAN);
	setcolor(cmd->color);
	glVertex2i(cmd->cx, cmd->cy);
	draw_arc(cmd->cx, cmd->cy, cmd->r, cmd->a[0], cmd->a[1], 24);
	glEnd();
}

void nkgfx_tri(struct nk_command_triangle *cmd)
{
	glLineWidth(cmd->line_thickness);

	glBegin(GL_LINE_LOOP);
	setcolor(cmd->color);
	glVertex2i(cmd->a.x, cmd->a.y);
	glVertex2i(cmd->b.x, cmd->b.y);
	glVertex2i(cmd->c.x, cmd->c.y);
	glEnd();
}

void nkgfx_filltri(struct nk_command_triangle_filled *cmd)
{
	glBegin(GL_TRIANGLES);
	setcolor(cmd->color);
	glVertex2i(cmd->a.x, cmd->a.y);
	glVertex2i(cmd->b.x, cmd->b.y);
	glVertex2i(cmd->c.x, cmd->c.y);
	glEnd();
}

void nkgfx_poly(struct nk_command_polygon *cmd)
{
	int i;
	glLineWidth(cmd->line_thickness);

	glBegin(GL_LINE_LOOP);
	setcolor(cmd->color);
	for(i=0; i<cmd->point_count; i++) {
		glVertex2i(cmd->points[i].x, cmd->points[i].y);
	}
	glEnd();
}

void nkgfx_fillpoly(struct nk_command_polygon_filled *cmd)
{
	int i;
	glBegin(GL_TRIANGLE_FAN);
	setcolor(cmd->color);
	for(i=0; i<cmd->point_count; i++) {
		glVertex2i(cmd->points[i].x, cmd->points[i].y);
	}
	glEnd();
}

void nkgfx_polyline(struct nk_command_polyline *cmd)
{
	int i;
	glLineWidth(cmd->line_thickness);

	glBegin(GL_LINE_STRIP);
	setcolor(cmd->color);
	for(i=0; i<cmd->point_count; i++) {
		glVertex2i(cmd->points[i].x, cmd->points[i].y);
	}
	glEnd();
}

void nkgfx_text(struct nk_command_text *cmd)
{
	int i;
	char *ptr = cmd->string;

	glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(1.5);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(cmd->x, cmd->y + 3 * TEXT_HEIGHT / 4, 0);
	glScalef(TEXT_SCALE, -TEXT_SCALE, TEXT_SCALE);

	setcolor(cmd->foreground);
	for(i=0; i<cmd->length; i++) {
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *ptr++);
	}

	glPopMatrix();
	glPopAttrib();
}

static float text_width(nk_handle nk, float h, const char *str, int len)
{
	int i, res = 0;
	for(i=0; i<len; i++) {
		res += glutStrokeWidth(GLUT_STROKE_ROMAN, *str++);
	}
	return res * TEXT_SCALE;
}


void nkgfx_image(struct nk_command_image *cmd)
{
	float u, v, du, dv;

	u = (float)cmd->img.region[0] / cmd->img.w;
	v = (float)cmd->img.region[1] / cmd->img.h;
	du = (float)cmd->img.region[2] / cmd->img.w;
	dv = (float)cmd->img.region[3] / cmd->img.h;

	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, cmd->img.handle.id);

	glBegin(GL_QUADS);
	setcolor(cmd->col);
	glTexCoord2f(u, v);
	glVertex2i(cmd->x, cmd->y);
	glTexCoord2f(u + du, v);
	glVertex2i(cmd->x + cmd->w, cmd->y);
	glTexCoord2f(u + du, v + dv);
	glVertex2i(cmd->x + cmd->w, cmd->y + cmd->h);
	glTexCoord2f(u, v + dv);
	glVertex2i(cmd->x, cmd->y + cmd->h);
	glEnd();

	glPopAttrib();
}

