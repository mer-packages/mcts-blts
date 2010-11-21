/*
 * Stroking function and perf testing
 * kui.zheng@intel.com
 */

#include "test_common.h"
#include "openvg_helper.h"

static VGfloat white_color[4] = { 1.0, 1.0, 1.0, 1.0 };
static VGfloat red_color[4] = { 1.0, 0.0, 0.0, 1.0 };
static VGfloat blue_color[4] = { 0.0, 0.0, 1.0, 1.0 };
static VGfloat green_color[4] = { 0.0, 1.0, 0.0, 1.0 };
static VGfloat black_color[4] = { 0.0, 0.0, 0.0, 1.0 };

VGfloat dash_pattern[DASH_PATTERN_COUNT];
VGfloat line_width;

static void init(vgh_context * context)
{
	VGfloat w = context->width;
	VGfloat h = context->height;
	static const VGubyte cmds[] = { VG_LINE_TO_ABS,
		VG_LINE_TO_ABS,
		VG_LINE_TO_ABS,
		VG_LINE_TO_ABS,
	};
	VGfloat coords[] = { w / 3, h / 4,
		0.0f, h / 2,
		w / 3, h * 3 / 4,
		0.0f, h,
	};

	context->vgPath =
	    vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1, 0, 0,
			 0, VG_PATH_CAPABILITY_APPEND_TO);
	vgAppendPathData(context->vgPath, 4, cmds, coords);
	context->vgPaint = vgCreatePaint();
	vgSetPaint(context->vgPaint, VG_FILL_PATH);
	vgSetfv(VG_CLEAR_COLOR, 4, white_color);
}

static void stroke_set(redraw_context * rectx)
{
	VGuint counter = rectx->counter;
	line_width =
	    COUNTER_TO_COLOR(line_width, counter,
			     rectx->stroke.width_step) * 30;
	dash_pattern[0] =
	    COUNTER_TO_COLOR(dash_pattern[0], counter,
			     rectx->stroke.dpattern_step[0]) * 50;
	dash_pattern[1] =
	    COUNTER_TO_COLOR(dash_pattern[1], counter,
			     rectx->stroke.dpattern_step[1]) * 50;
}

static void
stroke_draw(VGPath path, VGPaint paint, VGCapStyle cap,
	    VGJoinStyle join, VGfloat line_width,
	    VGfloat color[], VGfloat dash_pattern[])
{
	vgSetParameterfv(paint, VG_PAINT_COLOR, 4, color);
	vgSetf(VG_STROKE_LINE_WIDTH, line_width);
	vgSeti(VG_STROKE_CAP_STYLE, cap);
	vgSeti(VG_STROKE_JOIN_STYLE, join);
	vgSetfv(VG_STROKE_DASH_PATTERN, DASH_PATTERN_COUNT, dash_pattern);
	vgSetf(VG_STROKE_DASH_PHASE, 0.0f);
	vgDrawPath(path, VG_STROKE_PATH);
}

static int draw(vgh_context * context, redraw_context * rectx)
{
	dash_pattern[0] = 20.0f;
	dash_pattern[1] = 10.0f;
	line_width = 20.0f;
	vgClear(0, 0, context->width, context->height);

	//VG_CAP_BUTT, VG_JOIN_MITER, red dash line
	vgLoadIdentity();
	for (int i = 0; i < FEA_NUM; i++) {
		if (rectx->redraw[i])
			(rectx->redraw[i]) (rectx);
	}
	stroke_draw(context->vgPath, context->vgPaint, VG_CAP_BUTT,
		    VG_JOIN_MITER, line_width, red_color, dash_pattern);

	//VG_CAP_BUTT, VG_JOIN_ROUND, black dash line
	vgLoadIdentity();
	vgTranslate(context->width / 3, context->height);
	vgRotate(180);
	for (int i = 0; i < FEA_NUM; i++) {
		if (rectx->redraw[i])
			(rectx->redraw[i]) (rectx);
	}
	stroke_draw(context->vgPath, context->vgPaint, VG_CAP_BUTT,
		    VG_JOIN_ROUND, line_width, black_color, dash_pattern);

	//VG_CAP_ROUND, VG_JOIN_ROUND, green dash line
	vgLoadIdentity();
	vgTranslate(context->width / 3, 0.0f);
	for (int i = 0; i < FEA_NUM; i++) {
		if (rectx->redraw[i])
			(rectx->redraw[i]) (rectx);
	}
	stroke_draw(context->vgPath, context->vgPaint, VG_CAP_ROUND,
		    VG_JOIN_ROUND, line_width, green_color, dash_pattern);

	//VG_CAP_ROUND, VG_JOIN_BEVEL, black dash line
	vgLoadIdentity();
	vgTranslate(context->width * 2 / 3, context->height);
	vgRotate(180);
	for (int i = 0; i < FEA_NUM; i++) {
		if (rectx->redraw[i])
			(rectx->redraw[i]) (rectx);
	}
	stroke_draw(context->vgPath, context->vgPaint, VG_CAP_ROUND,
		    VG_JOIN_BEVEL, line_width, black_color, dash_pattern);

	//VG_CAP_SQUARE, VG_JOIN_BEVEL, blue dash line
	vgLoadIdentity();
	vgTranslate(context->width * 2 / 3, 0.0f);
	for (int i = 0; i < FEA_NUM; i++) {
		if (rectx->redraw[i])
			(rectx->redraw[i]) (rectx);
	}
	stroke_draw(context->vgPath, context->vgPaint, VG_CAP_SQUARE,
		    VG_JOIN_BEVEL, line_width, blue_color, dash_pattern);

	//VG_CAP_SQUARE, VG_JOIN_MITER, black dash line
	vgLoadIdentity();
	vgTranslate(context->width, context->height);
	vgRotate(180);
	for (int i = 0; i < FEA_NUM; i++) {
		if (rectx->redraw[i])
			(rectx->redraw[i]) (rectx);
	}
	stroke_draw(context->vgPath, context->vgPaint, VG_CAP_SQUARE,
		    VG_JOIN_MITER, line_width, black_color, dash_pattern);
	return 1;
}

int test_stroke(test_execution_params * params)
{
	vgh_context *pcontext, context;
	redraw_context *prectx, rectx;
	pcontext = &context;
	prectx = &rectx;
	memset(prectx, 0, sizeof(redraw_context));
	memset(pcontext, 0, sizeof(vgh_context));
	prectx->redraw[3] = (void *)&stroke_set;
	prectx->stroke.dpattern_step[0] = 0.1f;
	prectx->stroke.dpattern_step[1] = 0.2f;
	prectx->stroke.width_step = 0.01f;
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
