#include <stdio.h>
#include <GL/glut.h>
#include "ui.h"
#include "cgmath/cgmath.h"


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
	glLineWidth(1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(cmd->x, cmd->y + 16, 0);
	glScalef(0.14, -0.14, 0.14);

	setcolor(cmd->foreground);
	for(i=0; i<cmd->length; i++) {
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *ptr++);
	}

	glPopMatrix();
	glPopAttrib();
}

void nkgfx_image(struct nk_command_image *cmd)
{
}

