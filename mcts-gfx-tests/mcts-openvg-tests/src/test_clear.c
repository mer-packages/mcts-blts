#include "test_common.h"
#include "openvg_helper.h"

static const VGfloat red_color[4] = { 1.0, 0.0, 0.0, 1.0 };
static const VGfloat blue_color[4] = { 0.0, 0.0, 1.0, 1.0 };
static const VGfloat white_color[4] = { 1.0, 1.0, 1.0, 1.0 };

static int draw(vgh_context * context, redraw_context * rectx)
{
	vgSetfv(VG_CLEAR_COLOR, 4, red_color);
	vgClear(0, 0, context->width, context->height);
	vgSetfv(VG_CLEAR_COLOR, 4, white_color);
	vgClear(0, 0, (context->width) * 2 / 3, context->height);
	vgSetfv(VG_CLEAR_COLOR, 4, blue_color);
	vgClear(0, 0, context->width / 3, context->height);
	return 1;
}

int test_clear(test_execution_params * params)
{
	vgh_context *pcontext, context;
	redraw_context *prectx, rectx;
	pcontext = &context;
	prectx = &rectx;
	memset(prectx, 0, sizeof(redraw_context));
	memset(pcontext, 0, sizeof(vgh_context));
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
	if (!vgh_execute_main_loop
	    (pcontext, draw, prectx, params->execution_time)) {
		LOGERR("vgh_execute_main_loop failed!\n");
	}

	vgh_destroy_context(pcontext);
	return 0;

}
