/* ogles2_helper.h -- Various helper functions and definitions for GLES2

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

#ifndef OGLES2_HELPER_H
#define OGLES2_HELPER_H

#include <X11/Xlib.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <blts_log.h>
#include <blts_timing.h>
#include <errno.h>
#include <math.h>

#define GLESH_MAX_OBJECTS 2000
#define GLESH_MAX_TEXTURES 64
#define GLESH_PI (3.14159265f)
#define GLESH_COS_SIN_TABLE_SIZE (1<<12)
#define GLESH_COS_SIN_TABLE_MASK (GLESH_COS_SIN_TABLE_SIZE-1)

#define DRAW_FUNCTION int (*drawFunc)(glesh_context* c, void* u)

#define UNUSED_PARAM(a) (void)(a);

#define GLESH_MAX(a,b) (((a) > (b)) ? (a) : (b))
#define GLESH_MIN(a,b) (((a) < (b)) ? (a) : (b))

#define ARRAY_SIZE(a) (sizeof(a) / sizeof *(a))

typedef struct
{
	char name[64];
	GLuint tex_id;
	GLuint width;
	GLuint height;
	EGLSurface eglpixmap;
} glesh_texture;

typedef struct
{
	GLfloat m[4][4];
} glesh_matrix;

typedef struct
{
	GLfloat v[3];
} glesh_vector3;

typedef struct
{
	GLfloat v[4];
} glesh_vector4;

typedef struct
{
	unsigned int frames_rendered;
	double total_time_elapsed;
	double fps;
	double cpu_usage;
	double total_load;
} glesh_perf_data;

typedef struct
{
	int num_indices;
	int num_triangles;
	int num_vertices;
	GLfloat* vertices;
	GLfloat* normals;
	GLfloat* texcoords;
	GLuint* indices;
	glesh_matrix modelview;
	glesh_texture* tex;
} glesh_object;

typedef struct
{
	GLint width; /* Render window width in pixels */
	GLint height; /* Render window height in pixels */
	GLint depth; /* Render window depth in bpp */

	EGLDisplay egl_display;
	EGLContext egl_context;
	EGLSurface egl_surface;
	Display* x11_display;
	Window x11_window;
	Colormap x11_colormap;

	GLuint num_textures;
	glesh_texture textures[GLESH_MAX_TEXTURES];

	GLuint num_objects;
	glesh_object objects[GLESH_MAX_OBJECTS];

	glesh_matrix mvp_mat;
	glesh_matrix perspective_mat;

	float* sin_table;
	float* cos_table;

	GLuint texture_pool[GLESH_MAX_TEXTURES];
	int next_texture;

	glesh_perf_data perf_data;
} glesh_context;

typedef struct
{
	int				biSize;
	int				biWidth;
	int				biHeight;
	short			biPlanes;
	short			biBitCount;
	int				biCompression;
	int				biSizeImage;
	int				biXPelsPerMeter;
	int				biYPelsPerMeter;
	int				biClrUsed;
	int				biClrImportant;
} glesh_bitmap_header;

/* Init/uninit */
int glesh_create_context(glesh_context* context, const EGLint attribList[],
	int window_width, int window_height, int depth);
int glesh_execute_main_loop(glesh_context* context, DRAW_FUNCTION,
	void* user_ptr, double runtime);
int glesh_load_program (const char *vertex_shader_src,
	const char *fragment_shader_src);
int glesh_destroy_context(glesh_context* context);

/* Textures */
glesh_texture* glesh_texture_from_bmp_file(glesh_context* context,
	const GLenum format, const char* texture_name, const char* filename,
	int scale_w, int scale_h);
glesh_texture* glesh_bitmap_to_texture(glesh_context* context,
	const GLenum format, const char* texture_name, unsigned char* data,
	glesh_bitmap_header* header);
glesh_texture* glesh_generate_texture(glesh_context* context,
	const GLenum format, const int width, const int height,
	const char* texture_name);
glesh_texture* glest_texture_by_name(glesh_context* context,
	const char* texture_name);
GLuint glesh_get_texture_from_pool(glesh_context* context);

/* Primitives */
int glesh_generate_sphere(int numSlices, float radius, glesh_object* object);
int glesh_generate_cube(float scale, glesh_object* object);
int glesh_generate_rectangle_strip(float scalex, float scaley,
	glesh_object* object);
int glesh_generate_triangle_strip(float scale, glesh_object* object);
int glesh_generate_plane(float scale, int numSlices, glesh_object* object);

/* Objects */
int glesh_init_object(glesh_object* object);
int glesh_destroy_object(glesh_object* object);
int glesh_attach_texture(glesh_object* object, glesh_texture* tex);
glesh_object* glesh_add_object(glesh_context* context, glesh_object* object);
GLfloat* glesh_add_vertices(glesh_object* object, int count);

/* Matrix, vectors */
void glesh_multiply(glesh_matrix* result, glesh_matrix* srcA,
	glesh_matrix* srcB);
void glesh_rotate(glesh_matrix* mat, GLfloat angle, GLfloat x, GLfloat y,
	GLfloat z);
void glesh_generate_rotation_matrix(glesh_matrix* mat, GLfloat angle,
	GLfloat x, GLfloat y, GLfloat z);
void glesh_set_to_identity(glesh_matrix* mat);
void glesh_perspective(glesh_matrix* result, float fovy, float aspect,
	float nearZ, float farZ);
void glesh_translate(glesh_matrix* result, GLfloat tx, GLfloat ty, GLfloat tz);

/* Misc */
void glesh_report_eglerror(const char* location);
const char* glesh_egl_error_to_string(EGLint err);
GLuint glesh_context_triangle_count(glesh_context* context);
unsigned char* glesh_read_bitmap(const char* filename,
	glesh_bitmap_header* header, int bgr, int scale_w, int scale_h);
double glesh_time_step();
unsigned char* glesh_generate_pattern(const int width, const int height,
	const int offset, const GLenum format);

#endif // OGLES2_HELPER

