/*
 * Lnear Gradient function and perf Testing with thtree SPREAD models (PAD/REPEAT/REFLECT)
 * kui.zheng@intel.com
 */

#include "test_common.h"
#include "openvg_helper.h"

static const VGfloat linrampStop[4][5] = { {0.0f, 1.0f, 1.0f, 1.0f, 1.0f},
{0.33f, 1.0f, 0.0f, 0.0f, 1.0f},
{0.66f, 0.0f, 1.0f, 0.0f, 1.0f},
{1.00f, 0.0f, 0.0f, 1.0f, 1.0f}
};
static const VGfloat white_color[4] = { 1.0, 1.0, 1.0, 1.0 };
VGfloat lrmpstp[4][5] = { };

static void init(vgh_context * context)
{
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 5; j++)
			lrmpstp[i][j] = linrampStop[i][j];
	VGubyte sqrCmds[5] =
	    { VG_MOVE_TO_ABS, VG_HLINE_TO_ABS, VG_VLINE_TO_ABS, VG_HLINE_TO_ABS,
		VG_CLOSE_PATH
	};
	VGfloat sqrCoords[5] =
	    { 0.0f, 0.0f, context->width, context->height, 0.0f };
	VGfloat linearGradient[4] =
	    { 0.0f, 0.0f, context->width, context->height };
	context->vgPath =
	    vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1, 0, 0,
			 0, VG_PATH_CAPABILITY_APPEND_TO);
	vgAppendPathData(context->vgPath, 5, sqrCmds, sqrCoords);
	context->vgPaint = vgCreatePaint();
	vgSetPaint(context->vgPaint, VG_FILL_PATH);

	vgSetParameteri(context->vgPaint, VG_PAINT_TYPE,
			VG_PAINT_TYPE_LINEAR_GRADIENT);
	vgSetParameterfv(context->vgPaint, VG_PAINT_LINEAR_GRADIENT, 4,
			 linearGradient);
	vgSetfv(VG_CLEAR_COLOR, 4, white_color);
}

static void lin_rampstop(redraw_context * rectx)
{
	unsigned int counter = rectx->counter;
	for (int i = 0; i < 4; i++)
		for (int j = 1; j < 5; j++)
			lrmpstp[i][j] =
			    COUNTER_TO_COLOR(linrampStop[i][j], counter,
					     rectx->rampStop_step[i][j]);
}

static int draw(vgh_context * context, redraw_context * rectx)
{
	vgClear(0, 0, context->width, context->height);
	for (int i = 0; i < FEA_NUM; i++) {
		if (rectx->redraw[i])
			(rectx->redraw[i]) (rectx);
	}

	vgSetParameterfv(context->vgPaint, VG_PAINT_COLOR_RAMP_STOPS, 20,
			 (VGfloat *) lrmpstp);
	vgDrawPath(context->vgPath, VG_FILL_PATH);
	return 1;
}

int test_lingrad(test_execution_params * params)
{
	vgh_context *pcontext, context;
	redraw_context rectx;
	pcontext = &context;
	redraw_context *prectx = &rectx;
	memset(prectx, 0, sizeof(redraw_context));
	memset(pcontext, 0, sizeof(vgh_context));
	prectx->redraw[2] = (void *)&lin_rampstop;
	for (int i = 0; i < 4; i++)
		prectx->rampStop_step[i][1] = 0.001f;	//set a simple rampStop step
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
	vgSetParameteri(pcontext->vgPaint, VG_PAINT_COLOR_RAMP_SPREAD_MODE,
			VG_COLOR_RAMP_SPREAD_PAD);
	if (!vgh_execute_main_loop
	    (pcontext, draw, prectx, params->execution_time)) {
		LOGERR("SPREAD_PAD: vgh_execute_main_loop failed!\n");
	}
	vgSetParameteri(pcontext->vgPaint, VG_PAINT_COLOR_RAMP_SPREAD_MODE,
			VG_COLOR_RAMP_SPREAD_REPEAT);
	if (!vgh_execute_main_loop
	    (pcontext, draw, prectx, params->execution_time)) {
		LOGERR("SPREAD_REPEAT: vgh_execute_main_loop failed!\n");
	}
	vgSetParameteri(pcontext->vgPaint, VG_PAINT_COLOR_RAMP_SPREAD_MODE,
			VG_COLOR_RAMP_SPREAD_REFLECT);
	if (!vgh_execute_main_loop
	    (pcontext, draw, prectx, params->execution_time)) {
		LOGERR("SPREAD_REFLECT: vgh_execute_main_loop failed!\n");
	}
	vgh_destroy_context(pcontext);
	return 0;

}
