
/* Transformation function and perf testing
 * Zheng Kui <kui.zheng@intel.com>
 */

#include "test_common.h"
#include "openvg_helper.h"

int elps = 0;
static const VGfloat white_color[4] = { 1.0, 1.0, 1.0, 1.0 };

static void ellipse(VGPath vgPath, VGfloat rx, VGfloat ry, VGfloat angle)
{
	static const VGubyte cmd[] =
	    { VG_MOVE_TO_ABS, VG_SCCWARC_TO_REL, VG_SCCWARC_TO_REL,
		VG_CLOSE_PATH
	};

	VGfloat val[12];
	VGfloat c = cos(angle) * rx;
	VGfloat s = sin(angle) * rx;

	val[0] = c;
	val[1] = s;
	val[2] = rx;
	val[3] = ry;
	val[4] = angle;
	val[5] = -2.0f * c;
	val[6] = -2.0f * s;
	val[7] = rx;
	val[8] = ry;
	val[9] = angle;
	val[10] = 2.0f * c;
	val[11] = 2.0f * s;

	vgClearPath(vgPath, VG_PATH_CAPABILITY_ALL);
	vgAppendPathData(vgPath, sizeof(cmd), cmd, val);
	vgDrawPath(vgPath, VG_FILL_PATH | VG_STROKE_PATH);
}

static void init(vgh_context * context)
{
	vgSetfv(VG_CLEAR_COLOR, 4, white_color);
	context->vgPath = vgCreatePath(VG_PATH_FORMAT_STANDARD,
				       VG_PATH_DATATYPE_F, 1.0f, 0.0f, 0, 0,
				       VG_PATH_CAPABILITY_ALL);

	context->vgPaint = vgCreatePaint();
	vgSetParameteri(context->vgPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
	vgSetColor(context->vgPaint, 0x00ff00ff);
	vgSetPaint(context->vgPaint, VG_FILL_PATH);

	vgSeti(VG_RENDERING_QUALITY, VG_RENDERING_QUALITY_NONANTIALIASED);
	vgSeti(VG_BLEND_MODE, VG_BLEND_SRC_OVER);
	vgSetf(VG_STROKE_LINE_WIDTH, 2.0f);
	vgSeti(VG_STROKE_CAP_STYLE, VG_CAP_SQUARE);
	vgSeti(VG_STROKE_JOIN_STYLE, VG_JOIN_MITER);
	vgSetf(VG_STROKE_MITER_LIMIT, 4.0f);
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
}

static void transf(redraw_context * rectx)
{
	unsigned int counter = rectx->counter;
	VGfloat sx = counter % rectx->transformation.sx_step * 0.1f;
	VGfloat sy = counter % rectx->transformation.sy_step * 0.1f;
	switch (elps) {
	case 0:
		vgRotate((rectx->transformation.angle_step) * counter);
		break;
	case 1:
		vgScale(sx, sy);
		break;
	case 2:
		vgShear(rectx->transformation.shx_step,
			rectx->transformation.shy_step);
		break;
	case 3:
		vgTranslate((rectx->transformation.tx_step) * counter,
			    (rectx->transformation.ty_step) * counter);
		break;
	default:
		break;
	}
}

static int draw(vgh_context * context, redraw_context * rectx)
{
	VGfloat width, height;
	width = context->width;
	height = context->height;
	vgClear(0, 0, width, height);
	elps = 0;
	int i;
	vgLoadIdentity();
	vgTranslate(width / 3.0f, height / 3.0f);

	for (i = 0; i < FEA_NUM; i++) {
		if (rectx->redraw[i])
			(rectx->redraw[i]) (rectx);
	}
	ellipse(context->vgPath, width / 6.0f, height / 6.0f, 0.0f);

	elps = 1;
	vgLoadIdentity();
	vgTranslate(width * 2.0f / 3.0f, height / 3.0f);
	vgScale(0.5, 0.5f);
	for (i = 0; i < FEA_NUM; i++) {
		if (rectx->redraw[i])
			(rectx->redraw[i]) (rectx);
	}
	ellipse(context->vgPath, width / 6.0f, height / 6.0f, 0.0f);

	elps = 2;
	vgLoadIdentity();
	vgTranslate(width / 3.0f, height * 2.0f / 3.0f);
	vgScale(1.5, 1.5f);
	for (i = 0; i < FEA_NUM; i++) {
		if (rectx->redraw[i])
			(rectx->redraw[i]) (rectx);
	}
	ellipse(context->vgPath, width / 6.0f, height / 6.0f, 0.0f);

	elps = 3;
	vgLoadIdentity();
	vgTranslate(width * 2.0f / 3.0f, height * 2.0f / 3.0f);
	vgScale(0.8f, 0.8f);
	for (i = 0; i < FEA_NUM; i++) {
		if (rectx->redraw[i])
			(rectx->redraw[i]) (rectx);
	}
	ellipse(context->vgPath, width / 6.0f, height / 6.0f, 0.0f);
	return 1;
}

int test_transf(test_execution_params * params)
{
	vgh_context *pcontext, context;
	redraw_context *prectx, rectx;
	pcontext = &context;
	prectx = &rectx;
	memset(prectx, 0, sizeof(redraw_context));
	memset(pcontext, 0, sizeof(vgh_context));
	prectx->redraw[0] = (void *)&transf;
	prectx->transformation.tx_step = 0.1f;
	prectx->transformation.ty_step = 0.1f;
	prectx->transformation.sx_step = 15;
	prectx->transformation.sy_step = 15;
	prectx->transformation.shx_step = 1.5f;
	prectx->transformation.shy_step = 1.0f;
	prectx->transformation.angle_step = 1.0f;
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
