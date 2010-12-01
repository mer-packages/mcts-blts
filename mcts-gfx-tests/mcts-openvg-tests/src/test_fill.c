/*
 *  Filling function and perf test with two models (VG_NON_ZERO/VG_EVEN_ODD).
 *  Zheng Kui <kui.zheng@intel.com>
 */
#include "test_common.h"
#include "openvg_helper.h"

VGint stars = 0;
static const VGfloat blue_color[4] = { 0.0, 0.0, 1.0, 1.0 };
static const VGfloat green_color[4] = { 0.0, 1.0, 0.0, 1.0 };
static const VGfloat red_color[4] = { 1.0, 0.0, 0.0, 1.0 };

VGfloat fill_color[4] = { 0.0, 0.0f, 0.0f, 1.0f };

static void init(vgh_context * context)
{
	VGubyte star_cmds[6] =
	    { VG_MOVE_TO_ABS, VG_LINE_TO_ABS, VG_LINE_TO_ABS, VG_LINE_TO_ABS,
		VG_LINE_TO_ABS, VG_CLOSE_PATH
	};
	static const VGfloat coords[] = { 0, 280,
		420, 280,
		70, 0,
		210, 420,
		350, 0
	};
	context->vgPath =
	    vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1, 0, 0,
			 0, VG_PATH_CAPABILITY_APPEND_TO);
	vgAppendPathData(context->vgPath, 6, star_cmds, coords);
	context->vgPaint = vgCreatePaint();
	vgSetPaint(context->vgPaint, VG_FILL_PATH);
	vgSetParameterfv(context->vgPaint, VG_PAINT_COLOR, 4, fill_color);
	vgSetfv(VG_CLEAR_COLOR, 4, blue_color);
}

static void color(redraw_context * rectx)
{
	unsigned int counter = rectx->counter;
	int i;
	switch (stars) {
	case 0:
		for (i = 0; i < 3; i++) {
			fill_color[i] =
			    COUNTER_TO_COLOR(green_color[i], counter,
					     rectx->color_step[i]);
		}
		break;
	case 1:
		for (i = 0; i < 3; i++) {
			fill_color[i] =
			    COUNTER_TO_COLOR(red_color[i], counter,
					     rectx->color_step[i]);
		}
		break;
	default:
		break;
	}
}

static void star(VGPath path, VGPaint paint, VGFillRule fillrule)
{
	vgSeti(VG_FILL_RULE, fillrule);
	vgSetParameterfv(paint, VG_PAINT_COLOR, 4, fill_color);
	vgDrawPath(path, VG_FILL_PATH | VG_STROKE_PATH);
}

static int draw(vgh_context * context, redraw_context * rectx)
{
	VGint w = context->width;
	VGint h = context->height;
	int i;
	vgClear(0, 0, w, h);

	stars = 0;
	vgLoadIdentity();
	vgTranslate(6.0f, 30.0f);
	for (i = 0; i < FEA_NUM; i++) {
		if (rectx->redraw[i])
			(rectx->redraw[i]) (rectx);
	}
	star(context->vgPath, context->vgPaint, VG_NON_ZERO);

	stars = 1;
	vgLoadIdentity();
	vgTranslate(438.0f, 30.0f);
	for (i = 0; i < FEA_NUM; i++) {
		if (rectx->redraw[i])
			(rectx->redraw[i]) (rectx);
	}
	star(context->vgPath, context->vgPaint, VG_EVEN_ODD);

	return 1;
}

int test_fill(test_execution_params * params)
{
	vgh_context *pcontext, context;
	redraw_context rectx;
	pcontext = &context;
	redraw_context *prectx = &rectx;
	memset(prectx, 0, sizeof(redraw_context));
	memset(pcontext, 0, sizeof(vgh_context));
	prectx->redraw[1] = (void *)&color;
	prectx->transformation.angle_step = 1.0f;
	prectx->color_step[0] = 0.0f;	//red
	prectx->color_step[1] = 0.0f;	//green
	prectx->color_step[2] = 0.03f;	//blue
	prectx->color_step[3] = 0.3f;	//alpha
	if (!vgh_create_context
	    (&context, NULL, params->w, params->h, params->d)) {
		LOGERR("vgh_create_context failed!\n");
		return -1;
	}

	XMapWindow(context.x11_display, context.x11_window);
	if (!eglMakeCurrent
	    (context.egl_display, context.egl_surface, context.egl_surface,
	     context.egl_context)) {
		vgh_report_eglerror("eglMakeCurrent");
		vgh_destroy_context(pcontext);
		return -1;
	}
	init(pcontext);
	if (!vgh_execute_main_loop
	    (pcontext, draw, prectx, params->execution_time)) {
		LOGERR("vgh_execute_main_loop failed!\n");
	}

	vgh_destroy_context(pcontext);
	return 0;

}
