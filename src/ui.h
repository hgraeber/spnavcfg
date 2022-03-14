#ifndef UI_H_
#define UI_H_

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

#include "nuklear.h"

#define VIRT_HEIGHT	600
int virt_width;
int win_width, win_height;

#define PX_TO_VX(x)		((float)(x) * virt_width / win_width)
#define PY_TO_VY(y)		((float)(y) * VIRT_HEIGHT / win_height)

#define VX_TO_PX(x)		((float)(x) * win_width / virt_width)
#define VY_TO_PY(y)		((float)(y) * win_height / VIRT_HEIGHT)

void nkgfx_clip(struct nk_command_scissor *cmd);
void nkgfx_line(struct nk_command_line *cmd);
void nkgfx_curve(struct nk_command_curve *cmd);
void nkgfx_rect(struct nk_command_rect *cmd);
void nkgfx_fillrect(struct nk_command_rect_filled *cmd);
void nkgfx_colrect(struct nk_command_rect_multi_color *cmd);
void nkgfx_circle(struct nk_command_circle *cmd);
void nkgfx_fillcircle(struct nk_command_circle_filled *cmd);
void nkgfx_arc(struct nk_command_arc *cmd);
void nkgfx_fillarc(struct nk_command_arc_filled *cmd);
void nkgfx_tri(struct nk_command_triangle *cmd);
void nkgfx_filltri(struct nk_command_triangle_filled *cmd);
void nkgfx_poly(struct nk_command_polygon *cmd);
void nkgfx_fillpoly(struct nk_command_polygon_filled *cmd);
void nkgfx_polyline(struct nk_command_polyline *cmd);
void nkgfx_text(struct nk_command_text *cmd);
void nkgfx_image(struct nk_command_image *cmd);

#endif	/* UI_H_ */
