/* minimal_compositor.c -- Simple composite window manager using GLES2

   Copyright (C) 2000-2010, Nokia Corporation.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <memory.h>
#include <poll.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/shape.h>
#include "ogles2_helper.h"

typedef struct
{
	GLuint prog;
	int position_loc;
	int sampler_loc;
	int texcrd_loc;
	int mvmatrix_loc;
	int pmatrix_loc;
} s_shader_program;

typedef struct _x_window
{
	struct _x_window* next;
	Window id;
	Pixmap pixmap;
	XWindowAttributes a;
	int damaged;
	Damage damage;
	glesh_object* obj;
} x_window;

static const char vertex_shader_proj[] =
	"attribute vec4 a_position;\n"
	"attribute vec2 a_texCoord;\n"
	"varying vec2 v_texCoord;\n"
	"uniform mediump mat4 u_mvmatrix;\n"
	"uniform mediump mat4 u_pmatrix;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = u_mvmatrix * a_position;\n"
	"	v_texCoord = a_texCoord;\n"
	"}\n";

// TODO: We could convert the pixels from gbr<->rgb in shader
static const char frag_shader_simple[] =
	"varying mediump vec2 v_texCoord;\n"
	"uniform sampler2D s_texture;\n"
	"void main()\n"
	"{\n"
	"	mediump vec4 col = texture2D(s_texture, v_texCoord);\n"
	"	gl_FragColor = col;\n"
	"}\n";

static const EGLint pixmap_config[] =
{
	EGL_SURFACE_TYPE, EGL_PIXMAP_BIT,
	EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
	EGL_DEPTH_SIZE, 0,
	EGL_BIND_TO_TEXTURE_RGB,EGL_TRUE,
	EGL_NONE
};

static const EGLint pixmap_attribs[] =
{
	EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
	EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGB,
	EGL_NONE
};

static s_shader_program shader;
static glesh_context context;
static x_window* list;
static Window root;
static Window xoverlay;
static int dpy_damaged;
static int damage_event;
static int custom_tfp;
static EGLint ecfgs;
static EGLConfig configs[20];

static x_window* find_win(Window id)
{
	x_window* w;

	for(w = list; w; w = w->next)
	{
		if(w->id == id) return w;
	}
	return NULL;
}

static Status get_drawable_size(Display *dpy, Drawable d, GLuint *width,
	GLuint *height)
{
	Window root;
	Status stat;
	int xpos, ypos;
	unsigned int w, h, bw, depth;
	stat = XGetGeometry(dpy, d, &root, &xpos, &ypos, &w, &h, &bw, &depth);
	*width = w;
	*height = h;
	return stat;
}

static int texture_from_pixmap(Display* dpy, Pixmap pixmap,
	glesh_texture* texture)
{
	GLuint width, height;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->tex_id);

	get_drawable_size(dpy, pixmap, &width, &height);

	if(custom_tfp)
	{
		XImage* xi = XGetImage(dpy, pixmap, 0, 0, width, height, ~0L, ZPixmap);
		if(!xi || !xi->data)
		{
			LOG("XGetImage failed\n");
			return 1;
		}

		if(xi->depth == 32)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, (void*)&(xi->data[0]));
		}
		else if(xi->depth == 16)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
				GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (void*)&(xi->data[0]));
		}
		XDestroyImage(xi);
	}
	else
	{
		if(eglReleaseTexImage(context.egl_display, texture->eglpixmap,
			EGL_BACK_BUFFER) == EGL_FALSE)
		{
			LOG("eglReleaseTexImage failed\n");
		}
		if(eglBindTexImage(context.egl_display, texture->eglpixmap,
			EGL_BACK_BUFFER) == EGL_FALSE)
		{
			LOG("eglBindTexImage failed\n");
		}
	}

	texture->width = width;
	texture->height = height;
	return 0;
}

static float width_to_gl(float width)
{
	return 2.0f / context.width * width;
}

static float height_to_gl(float height)
{
	return 2.0f / context.height * height;
}

static void resize_win_object(glesh_object* object, int width, int height)
{
	if(object->vertices)
	{
		free(object->vertices);
		free(object->normals);
		free(object->texcoords);
		free(object->indices);
	}
	glesh_generate_rectangle_strip(width_to_gl(width),
		height_to_gl(height), object);
}

static glesh_object* create_window_object(int width, int height)
{
	glesh_object object;

	glesh_init_object(&object);
	object.tex = NULL;
	resize_win_object(&object, width, height);
	return glesh_add_object(&context, &object);
}

static int update_object_texture(Display* dpy, Pixmap pixmap,
	glesh_object* object)
{
	return texture_from_pixmap(dpy, pixmap, object->tex);
}

static void clear_window_texture(Display* dpy, x_window* win)
{
	if(win->obj)
	{
		if(win->obj->tex)
		{
			glBindTexture(GL_TEXTURE_2D, win->obj->tex->tex_id);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 0, GL_RGBA,
				GL_UNSIGNED_BYTE, 0);
			win->obj->tex->width = 0;
			win->obj->tex->height = 0;
			if(win->obj->tex->eglpixmap)
			{
				eglReleaseTexImage(dpy, win->obj->tex->eglpixmap,
					EGL_BACK_BUFFER);
			}
		}
	}

	if(win->pixmap)
	{
		XFreePixmap(dpy, win->pixmap);
		win->pixmap = None;
	}
}

static int draw_object(glesh_object* object)
{
	if(!object->tex)
	{
		LOG("no texture for object, not drawing\n");
		return 1;
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, object->tex->tex_id);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// TODO: Flip the vertices around...
	glesh_rotate(&object->modelview, 180.0f, 0, 1.0f, 0.0);

	glUniformMatrix4fv(shader.mvmatrix_loc, 1, GL_FALSE,
		(GLfloat*)&object->modelview);
	glVertexAttribPointer(shader.position_loc, 3, GL_FLOAT, GL_FALSE, 0,
		object->vertices);
	glVertexAttribPointer(shader.texcrd_loc, 2, GL_FLOAT, GL_FALSE, 0,
		object->texcoords);
	glEnableVertexAttribArray(shader.position_loc);
	glEnableVertexAttribArray(shader.texcrd_loc);
	glUniform1i(shader.sampler_loc, object->tex->tex_id);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	return 0;
}

static void generate_window_object(Display* dpy, x_window* win)
{
	LOG("Creating window object %p. %d,%d,%d,%d,%d\n",
		win, win->a.x, win->a.y, win->a.width, win->a.height, win->a.depth);

	if(!(win->obj = create_window_object(win->a.width + win->a.border_width * 2,
		win->a.height + win->a.border_width * 2)))
	{
		return;
	}
	glesh_texture* tex = malloc(sizeof(glesh_texture));
	memset(tex, 0, sizeof(glesh_texture));
	tex->tex_id = glesh_get_texture_from_pool(&context);
	glesh_attach_texture(win->obj, tex);

	win->pixmap = XCompositeNameWindowPixmap(dpy, win->id);

	if(!custom_tfp)
	{
		EGLint i;
		for(i = 0; i < ecfgs; i++)
		{
			win->obj->tex->eglpixmap = eglCreatePixmapSurface(
				context.egl_display, configs[i],
				(EGLNativePixmapType)win->pixmap, pixmap_attribs);
			if(win->obj->tex->eglpixmap != EGL_NO_SURFACE)
			{
				break;
			}
		}
	}

	LOG("Window object created\n");
}

static int generate_paint_list(x_window* w_list[])
{
	x_window* w;
	int t = 99;
	for(w = list; w; w = w->next)
	{
		if( w->a.map_state != IsViewable ||
			w->a.x + w->a.width < 1 || w->a.y + w->a.height < 1 ||
			w->a.x >= context.width || w->a.y >= context.height)
		{
			/* Not visible */
			LOG("skipping window %p. %d,%d,%d,%d,%d\n",
				w, w->a.x, w->a.y, w->a.width, w->a.height, w->a.depth);
			continue;
		}
		w_list[t--] = w;
	}
	return t + 1;
}

static void paint_windows(Display *dpy)
{
	x_window *w;
	x_window* w_list[100];
	int t;

	LOG("Painting...\n");

	glClear(GL_COLOR_BUFFER_BIT);

	memset(w_list, 0, sizeof(w_list));
	t = generate_paint_list(w_list);

	while(t < 100)
	{
		w = w_list[t++];
		if(!w->obj)
		{
			generate_window_object(dpy, w);
		}
		if(w->obj)
		{
			if(w->damaged)
			{
				if(w->pixmap) XFreePixmap(dpy, w->pixmap);
				w->pixmap = XCompositeNameWindowPixmap(dpy, w->id);
				if(w->pixmap)
				{
					LOG("Update texture for window %p. %d,%d,%d,%d,%d\n",
						w, w->a.x, w->a.y, w->a.width, w->a.height, w->a.depth);
					update_object_texture(dpy, w->pixmap, w->obj);
				}
				else
				{
					clear_window_texture(dpy, w);
				}
				w->damaged = 0;
			}
			if(w->obj->tex && w->obj->tex->width)
			{
				float cx, cy;
				cx = width_to_gl(w->obj->tex->width) / 2.0f - 1.0f;
				cy = height_to_gl(w->obj->tex->height) / 2.0f -1.0f;
				glesh_set_to_identity(&w->obj->modelview);
				glesh_translate(&w->obj->modelview,
					cx + (2.0f / context.width * w->a.x),
					-(cy + (2.0f / context.height * w->a.y)),
					0.0f);
				draw_object(w->obj);
			}
		}
	}
	eglSwapBuffers(context.egl_display, context.egl_surface);
	LOG("Done\n");
}

static void map_win(Display *dpy, Window id)
{
	x_window* w = find_win(id);
	if(!w) return;
	w->a.map_state = IsViewable;
	XMapWindow(dpy, id);
	XSelectInput(dpy, id, PropertyChangeMask);
	dpy_damaged = 1;
	w->damaged = 1;
}

static void unmap_win(Display *dpy, Window id)
{
	x_window *w = find_win(id);
	if(!w) return;
	w->a.map_state = IsUnmapped;
	w->damaged = 0;
	clear_window_texture(dpy, w);
	XSelectInput(dpy, w->id, 0);
}

static void add_win(Display *dpy, Window id, Window prev)
{
	x_window* new;
	x_window** p;

	if(id == context.x11_window || id == xoverlay)
	{
		LOG("Not adding our own window\n");
		return;
	}

	new = malloc(sizeof(x_window));
	if(!new)
	{
		LOG("malloc failed\n");
		return;
	}
	memset(new, 0, sizeof(x_window));
	if(prev)
	{
		for(p = &list; *p; p = &(*p)->next)
		{
			if((*p)->id == prev) break;
		}
	}
	else
	{
		p = &list;
	}
	new->id = id;

	if(!XGetWindowAttributes(dpy, id, &new->a))
	{
		LOG("XGetWindowAttributes failed\n");
		free(new);
		return;
	}

	LOG("New window %p. %d,%d,%d,%d,%d\n",
		new, new->a.x, new->a.y, new->a.width, new->a.height, new->a.depth);

	dpy_damaged = 1;
	new->damaged = 1;
	if(new->a.class == InputOnly)
	{
		LOG("Input only window, no damage\n");
		new->damage = None;
	}
	else
	{
		new->damage = XDamageCreate(dpy, id, XDamageReportNonEmpty);
	}

	XCompositeRedirectWindow(dpy, id, CompositeRedirectAutomatic);

	new->next = *p;
	*p = new;
	if(new->a.map_state == IsViewable)
	{
		map_win(dpy, id);
	}
}

static void configure_win(Display* dpy, XConfigureEvent *ce)
{
	x_window* w = find_win(ce->window);
	if(!w)
	{
		if(ce->window == root)
		{
			LOGERR("TODO: Root window size change not supported\n");
		}
		return;
	}

	w->a.x = ce->x;
	w->a.y = ce->y;
	if(w->a.width != ce->width || w->a.height != ce->height)
	{
		clear_window_texture(dpy, w);
	}

	w->a.width = ce->width;
	w->a.height = ce->height;
	w->a.border_width = ce->border_width;
	w->a.override_redirect = ce->override_redirect;
	dpy_damaged = 1;
}

static void destroy_win(Display *dpy, Window id, int gone)
{
	x_window **prev, *w;

	for(prev = &list; (w = *prev); prev = &w->next)
	{
		if(w->id == id)
		{
			LOG("Destroy window %p. %d,%d,%d,%d,%d\n",
				w, w->a.x, w->a.y, w->a.width, w->a.height, w->a.depth);
			if(gone) unmap_win(dpy, w->id);
			*prev = w->next;
			free (w);
			break;
		}
	}
}

static int x_error(Display *dpy, XErrorEvent *ev)
{
	const char *name = NULL;
	static char buffer[256];

	buffer[0] = 0;
	XGetErrorText(dpy, ev->error_code, buffer, sizeof (buffer));
	name = buffer;

	LOG("error %d: %s request %d minor %d serial %lu\n",
		ev->error_code, (strlen (name) > 0) ? name : "unknown",
		ev->request_code, ev->minor_code, ev->serial);

	return 0;
}

static int x11_event_handler_loop(Display* dpy)
{
	struct pollfd ufd;
	XEvent ev;

	ufd.fd = ConnectionNumber(dpy);
	ufd.events = POLLIN;
	paint_windows(dpy);

	timing_start();

	// TODO: Temporary solution (we need pipe or something to control the compositor)
	while(1)
	{
		if(!QLength(dpy))
		{
			/* No events in queue, wait for them */
			if(poll(&ufd, 1, 1000) == 0) continue;
		}

		do
		{
			XNextEvent (dpy, &ev);
			switch(ev.type)
			{
			case CreateNotify:
				LOG("CreateNotify\n");
				add_win(dpy, ev.xcreatewindow.window, 0);
				break;
			case ConfigureNotify:
				LOG("ConfigureNotify\n");
				configure_win(dpy, &ev.xconfigure);
				break;
			case DestroyNotify:
				LOG("DestroyNotify\n");
				destroy_win(dpy, ev.xdestroywindow.window, 1);
				break;
			case MapNotify:
				LOG("MapNotify\n");
				map_win(dpy, ev.xmap.window);
				break;
			case UnmapNotify:
				LOG("UnmapNotify\n");
				unmap_win(dpy, ev.xunmap.window);
				break;
			case ReparentNotify:
				LOG("ReparentNotify\n");
				if(ev.xreparent.parent == root)
				{
					add_win(dpy, ev.xreparent.window, 0);
				}
				else
				{
					destroy_win(dpy, ev.xreparent.window, 0);
				}
				break;
			case CirculateNotify:
				LOG("CirculateNotify\n");
				break;
			case Expose:
				LOG("Expose\n");
				dpy_damaged = 1;
				break;
			case PropertyNotify:
				LOG("PropertyNotify\n");
				break;
			default:
				LOG("Event: %d\n", ev.type);
				if(ev.type == damage_event + XDamageNotify)
				{
					x_window* w = find_win(((XDamageNotifyEvent *)&ev)->drawable);
					if(w)
					{
						XDamageSubtract(dpy, w->damage, None, None);
						dpy_damaged = 1;
						w->damaged = 1;
					}
				}
				break;
			}
		} while(QLength(dpy));

		if(dpy_damaged)
		{
			paint_windows(dpy);
			XSync(dpy, False);
			dpy_damaged = 0;
		}
	}

	timing_stop();

	return 0;
}

static int setup_composite_manager(int scr)
{
	Window win;
	Atom a;
	static char net_wm_cm[] = "_NET_WM_CM_Sxx";

	snprintf (net_wm_cm, sizeof (net_wm_cm), "_NET_WM_CM_S%d", scr);
	a = XInternAtom (context.x11_display, net_wm_cm, False);

	win = XGetSelectionOwner(context.x11_display, a);
	if(win != None)
	{
		XTextProperty tp;
		char **strs;
		int count;
		Atom winNameAtom = XInternAtom(context.x11_display, "_NET_WM_NAME",
			False);

		if(!XGetTextProperty(context.x11_display, win, &tp, winNameAtom) &&
			!XGetTextProperty(context.x11_display, win, &tp, XA_WM_NAME))
		{
			LOGERR("Composite manager is already running (0x%lx)\n",
				(unsigned long) win);
			return 1;
		}
		if(XmbTextPropertyToTextList (context.x11_display, &tp, &strs,
			&count) == Success)
		{
			LOGERR("Composite manager is already running (%s)\n", strs[0]);
			XFreeStringList(strs);
		}

		XFree(tp.value);

		return 1;
	}

	win = XCreateSimpleWindow(context.x11_display, root, 0, 0, 1, 1, 0,
		None, None);
	Xutf8SetWMProperties(context.x11_display, win, "blts_mcm", "blts_mcm",
		NULL, 0, NULL, NULL, NULL);
	XSetSelectionOwner(context.x11_display, a, win, 0);

	shader.prog = glesh_load_program(vertex_shader_proj, frag_shader_simple);
	if(!shader.prog)
	{
		LOGERR("Failed to load shader program\n");
		return 1;
	}

	xoverlay = XCompositeGetOverlayWindow(context.x11_display, root);
	XReparentWindow(context.x11_display, context.x11_window, xoverlay, 0, 0);

	XMapWindow(context.x11_display, xoverlay);
	XSync(context.x11_display, False);

	XFixesSetWindowShapeRegion(context.x11_display, context.x11_window,
		ShapeBounding, 0, 0, 0);
	XFixesSetWindowShapeRegion(context.x11_display, context.x11_window,
		ShapeInput, 0, 0, 0);
	XFixesSetWindowShapeRegion(context.x11_display, xoverlay,
		ShapeBounding, 0, 0, 0);
	XFixesSetWindowShapeRegion(context.x11_display, xoverlay, ShapeInput,
		0, 0, 0);

	shader.position_loc = glGetAttribLocation(shader.prog, "a_position");
	shader.texcrd_loc = glGetAttribLocation(shader.prog, "a_texCoord");
	shader.sampler_loc = glGetUniformLocation(shader.prog, "s_texture");
	shader.mvmatrix_loc = glGetUniformLocation(shader.prog, "u_mvmatrix");
	shader.pmatrix_loc = glGetUniformLocation(shader.prog, "u_pmatrix");

	glUseProgram(shader.prog);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glViewport(0, 0, context.width, context.height);
	glesh_set_to_identity(&context.perspective_mat);

	return 0;
}

static void usage_msg()
{
	const char* usage_msg =
	{
		"USAGE: blts-minimal-compositor [-l logfile] [-e] [-notfp] [-d depth]\n"
		"-l: Used logfile (default: ./blts_mcm.txt)\n"
		"-e: Enable logging\n"
		"-notfp: Do not use eglBindTexImage\n"
		"-d: Depth in bpp\n"
	};

	fprintf(stdout, "%s\n", usage_msg);
}

int main(int argc, char *argv[])
{
	Window root_return, parent_return;
	Window* children = NULL;
	unsigned int nchildren = 0;
	int tmp1, tmp2, tmp3, scr, i;
	char* logfile = "blts_mcm.txt";
	int depth = 0;
	int log_enabled = 0;

	dpy_damaged = 0;
	custom_tfp = 0;
	ecfgs = 0;

	for(i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "-l") == 0)
		{
			if(++i >= argc)
			{
				usage_msg();
				return 1;
			}
			logfile = argv[i];
		}
		else if(strcmp(argv[i], "-?") == 0)
		{
			usage_msg();
			return 1;
		}
		else if(strcmp(argv[i], "-notfp") == 0)
		{
			custom_tfp = 1;
		}
		else if(strcmp(argv[i], "-e") == 0)
		{
			log_enabled = 1;
		}
		else if(strcmp(argv[i], "-d") == 0)
		{
			if(++i >= argc)
			{
				usage_msg();
				return 1;
			}
			depth = atoi(argv[i]);
		}
		else
		{
			usage_msg();
			return 1;
		}
	}

	if(log_enabled) log_open(logfile, 0);

	if(!glesh_create_context(&context, NULL, 0, 0, depth))
	{
		return 1;
	}

	if(!custom_tfp)
	{
		if(!eglChooseConfig(context.egl_display, pixmap_config, configs,
			20, &ecfgs) || !ecfgs)
		{
			LOG("No EGL configuration for texture from pixmap. "
				"Using slow 'copy from pixmap.'\n");
			custom_tfp = 1;
		}
	}

	XSetErrorHandler(x_error);

	if(!XQueryExtension(context.x11_display, COMPOSITE_NAME, &tmp1, &tmp2, &tmp3))
	{
		LOGERR("No composite extension\n");
		exit (1);
	}

	if(!XDamageQueryExtension(context.x11_display, &damage_event, &tmp2))
	{
		LOGERR("No damage extension\n");
		exit (1);
	}

	if(!XFixesQueryExtension(context.x11_display, &tmp1, &tmp2))
	{
		LOGERR("No XFixes extension\n");
		exit (1);
	}

	scr = DefaultScreen(context.x11_display);
	root = RootWindow(context.x11_display, scr);

	if(setup_composite_manager(scr))
	{
		exit(1);
	}

	XSelectInput(context.x11_display, root,SubstructureNotifyMask|ExposureMask|
		StructureNotifyMask|PropertyChangeMask);

	XQueryTree(context.x11_display, root, &root_return, &parent_return,
		&children, &nchildren);
	for(i = 0; i < (int)nchildren; i++)
	{
		add_win(context.x11_display, children[i], i ? children[i-1] : None);
	}
	if(children) XFree(children);

	i = x11_event_handler_loop(context.x11_display);

	glesh_destroy_context(&context);
	log_close();

	return i;
}

