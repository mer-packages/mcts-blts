/* test_blitter.c -- Graphics benchmarks

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
#include "ogles2_helper.h"
#include "test_blitter.h"
#include "test_common.h"

const char* data_path = "/usr/share/blts-opengles2-tests/";

/* May be needed with emulation sdks */
//#define USE_ALL_TEXTURE_UNITS 1

static const char vertex_shader_proj[] =
	"attribute vec4 a_position;\n"
	"attribute vec2 a_texCoord;\n"
	"varying vec2 v_texCoord;\n"
	"uniform mediump mat4 u_mvmatrix;\n"
	"uniform mediump mat4 u_pmatrix;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = u_pmatrix * u_mvmatrix * a_position;\n"
	"	v_texCoord = a_texCoord;\n"
	"}\n";

static const char frag_shader_simple[] =
	"varying mediump vec2 v_texCoord;\n"
	"uniform sampler2D s_texture;\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = texture2D(s_texture, v_texCoord);\n"
	"}\n";

static const char vertex_shader_particle[] =
	"attribute vec4 a_position;\n"
	"varying mediump vec2 screen_pos;\n"
	"varying mediump float radius;\n"
	"varying lowp float decay;\n"
	"uniform mediump vec4 u_window_size;\n"
	"uniform mediump mat4 u_mvmatrix;\n"
	"uniform mediump mat4 u_pmatrix;\n"
	"void main()\n"
	"{\n"
	"	gl_PointSize = 20.0;\n"
	"	mediump vec4 pos = a_position;\n"
	"	decay = -pos.z;\n"
	"	gl_Position = u_pmatrix * u_mvmatrix * pos;\n"
	"	mediump vec2 halfsize = vec2(u_window_size.x * 0.5, u_window_size.y * 0.5);\n"
	"	screen_pos = halfsize + ((gl_Position.xy / gl_Position.w) * halfsize);\n"
	"	radius = gl_PointSize * 0.5;\n"
	"}\n";

static const char frag_shader_particle[] =
	"varying lowp float decay;\n"
	"varying mediump vec2 screen_pos;\n"
	"varying mediump float radius;\n"
	"void main()\n"
	"{\n"
	"	mediump vec4 color = vec4(1.0, 1.0, 1.0, 1.0);\n"
	"	mediump float dist = distance(gl_FragCoord.xy, screen_pos);\n"
	"	if(dist > radius ) discard;\n"
	"	color *= decay / (dist / radius);\n"
	"	color.y = 1.0 / decay;\n"
	"	gl_FragColor = color;\n"
	"}\n";

static const char frag_shader_opaque[] =
	"varying mediump vec2 v_texCoord;\n"
	"uniform sampler2D s_texture;\n"
	"uniform lowp float u_opacity;\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = texture2D(s_texture, v_texCoord) * u_opacity;\n"
	"}\n";

/* simple averaging filter */
static const char frag_shader_opaque_blurred[] =
	"uniform mediump float u_tex_size;\n"
	"varying mediump vec2 v_texCoord;\n"
	"uniform sampler2D s_texture;\n"
	"uniform lowp float u_opacity;\n"
	"void main()\n"
	"{\n"
	"	mediump float blur_size = 1.0 / u_tex_size;\n"
	"	mediump vec4 sum = texture2D(s_texture, vec2(v_texCoord.x, v_texCoord.y));\n"
	"	sum += texture2D(s_texture, vec2(v_texCoord.x, v_texCoord.y));"
	"	sum += texture2D(s_texture, vec2(v_texCoord.x - blur_size, v_texCoord.y));"
	"	sum += texture2D(s_texture, vec2(v_texCoord.x + blur_size, v_texCoord.y));"
	"	sum += texture2D(s_texture, vec2(v_texCoord.x, v_texCoord.y - blur_size));"
	"	sum += texture2D(s_texture, vec2(v_texCoord.x - blur_size, v_texCoord.y - blur_size));"
	"	sum += texture2D(s_texture, vec2(v_texCoord.x + blur_size, v_texCoord.y - blur_size));"
	"	sum += texture2D(s_texture, vec2(v_texCoord.x, v_texCoord.y + 1.0 * blur_size));"
	"	sum += texture2D(s_texture, vec2(v_texCoord.x - blur_size, v_texCoord.y + blur_size));"
	"	sum += texture2D(s_texture, vec2(v_texCoord.x + blur_size, v_texCoord.y + blur_size));"
	"	gl_FragColor = sum / 9.0 * u_opacity;\n"
	"}\n";

/*
* Format: %d %d %s %d
* texture2Ds, divisor
*/
static const char convolution_template[] =
	"uniform mediump float u_tex_size;\n"
	"varying mediump vec2 v_texCoord;\n"
	"uniform sampler2D s_texture;\n"
	"uniform lowp float u_opacity;\n"
	"void main()\n"
	"{\n"
	"	mediump float blur_size = 1.0 / u_tex_size;\n"
	"	mediump vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);\n"
	"%s"
	"	gl_FragColor = sum / %.2f * u_opacity;\n"
	"}\n";

static char* frag_shader_convolution;

typedef struct
{
	glesh_vector3* pos;
	float x_vec;
	float y_vec;
} s_particle;

typedef struct
{
	glesh_object* obj;
	float rel_pos_x;
	float rel_pos_y;
	int num_particles;
	int video_offset;
	float video_time;
	glesh_object* particle_obj;
	glesh_texture video_texture;
	s_particle particles[MAX_PARTICLES];
} s_widget;

typedef struct
{
	glesh_object* obj;
	int num_widgets;
	s_widget widgets[MAX_WIDGETS];
} s_desktop;

typedef struct
{
	int num_desktops;
	s_desktop desktops[MAX_DESKTOPS];
} s_scene;

typedef struct
{
	GLuint prog;
	int position_loc;
	int sampler_loc;
	int texcrd_loc;
	int mvmatrix_loc;
	int pmatrix_loc;
	int opacity_loc;
	int texsize_loc;
	int wsize_loc;
} s_shader_program;

typedef struct
{
	float rot_angle;
	float zoom_angle;
	float scroll_angle;
	s_shader_program base_shader;
	s_shader_program particle_shader;
	s_scene scenes[MAX_SCENES];
	unsigned char* video_images[NUM_VIDEO_IMAGES];
	int num_scenes;
	int flags;
	test_configuration_file_params* test_config;
} s_test_data;

static int get_new_video_texture(s_test_data* data, int tex_id,
	int offset, const GLenum format);

static int generate_widget(glesh_context* context, s_test_data* data,
	s_desktop* desktop, float posx, float posy, float timestamp)
{
	glesh_object object;
	glesh_texture* tex;
	int t;
	char filename[PATH_MAX];
	int img_id = (desktop->num_widgets + 1) % MAX_WIDGET_IMAGES + 1;

	glesh_init_object(&object);
	glesh_generate_rectangle_strip(0.3f, 0.3f, &object);

	if(!(data->flags & T_FLAG_VIDEO_WIDGETS))
	{
		sprintf(filename, "%s/images/widgets/image%d.bmp", data_path, img_id);
		tex = glesh_texture_from_bmp_file(context, GL_RGBA, filename, filename,
			0, 0);
		if(!tex)
		{
			BLTS_ERROR("Failed to load texture\n");
			return 0;
		}

		glesh_attach_texture(&object, tex);
	}
	else
	{
		desktop->widgets[desktop->num_widgets].video_texture.tex_id =
			glesh_get_texture_from_pool(context);
		desktop->widgets[desktop->num_widgets].video_offset = rand();
		desktop->widgets[desktop->num_widgets].video_time = timestamp;
		get_new_video_texture(data,
			desktop->widgets[desktop->num_widgets].video_texture.tex_id,
			desktop->widgets[desktop->num_widgets].video_offset, GL_RGBA);
	}

	desktop->widgets[desktop->num_widgets].obj =
		glesh_add_object(context, &object);
	desktop->widgets[desktop->num_widgets].rel_pos_x = posx;
	desktop->widgets[desktop->num_widgets].rel_pos_y = posy;

	if(data->flags & T_FLAG_PARTICLES)
	{
		GLfloat* vec;
		s_widget* widget = &desktop->widgets[desktop->num_widgets];
		glesh_init_object(&object);
		widget->particle_obj = glesh_add_object(context, &object);
		vec = glesh_add_vertices(widget->particle_obj,
			data->test_config->particle_count);

		for(t = 0; t < data->test_config->particle_count; t++)
		{
			widget->particles[widget->num_particles].pos =
				(glesh_vector3*)&vec[t * 3];
			widget->num_particles++;
		}
	}

	desktop->num_widgets++;

	return 1;
}

static int generate_desktop(glesh_context* context, s_test_data* data,
	s_scene* scene, int num_widgets)
{
	glesh_object object;
	glesh_texture* tex;
	int t, x, y;
	char filename[PATH_MAX];

	sprintf(filename, "%s/images/image%d.bmp", data_path,
		scene->num_desktops + 1);
	if(data->test_config->scale_images_to_window)
	{
		tex = glesh_texture_from_bmp_file(context, GL_RGBA, filename, filename,
			context->width, context->height);
	}
	else
	{
		tex = glesh_texture_from_bmp_file(context, GL_RGBA, filename, filename,
			0, 0);
	}
	if(!tex)
	{
		BLTS_ERROR("Failed to load texture\n");
		return 0;
	}

	glesh_init_object(&object);
	glesh_generate_rectangle_strip(2.0f, 2.0f, &object);
	glesh_attach_texture(&object, tex);
	scene->desktops[scene->num_desktops++].obj =
		glesh_add_object(context, &object);

	t = 0;
	for(y = 0; y < 4; y++)
	{
		for(x = 0; x < 4; x++)
		{
			if(t++ >= num_widgets) break;
			generate_widget(context, data,
				&scene->desktops[scene->num_desktops - 1],
				(float)x / 2.0f - 0.8f, (float)-y / 2.0f + 0.7f,
				(float)data->test_config->video_widget_generation_freq /
				(float)num_widgets * t / 1000.0f);
		}
	}

	return 1;
}

static int load_shaders(s_test_data* data)
{
	const char* vshp = vertex_shader_proj;
	const char* fshp = NULL;

	if((data->flags & T_FLAG_BLUR) && data->flags & (T_FLAG_BLEND))
	{
		if(data->flags & T_FLAG_CONVOLUTION)
		{
			fshp = frag_shader_convolution;
		}
		else
		{
			fshp = frag_shader_opaque_blurred;
		}
	}
	else if(data->flags & T_FLAG_BLEND)
	{
		fshp = frag_shader_opaque;
	}
	else
	{
		fshp = frag_shader_simple;
	}

	data->base_shader.prog = glesh_load_program(vshp, fshp);
	if(!data->base_shader.prog)
	{
		BLTS_ERROR("Failed to load shader program\n");
		return 0;
	}

	if(data->flags & T_FLAG_PARTICLES)
	{
		data->particle_shader.prog = glesh_load_program(vertex_shader_particle,
			frag_shader_particle);
		if(!data->particle_shader.prog)
		{
			BLTS_ERROR("Failed to load shader program\n");
			return 0;
		}
	}

	return 1;
}

static int get_shader_locs(s_test_data* data, s_shader_program* prog)
{
	prog->position_loc = glGetAttribLocation(prog->prog, "a_position");
	prog->texcrd_loc = glGetAttribLocation(prog->prog, "a_texCoord");
	prog->sampler_loc = glGetUniformLocation(prog->prog, "s_texture");
	prog->mvmatrix_loc = glGetUniformLocation(prog->prog, "u_mvmatrix");
	prog->pmatrix_loc = glGetUniformLocation(prog->prog, "u_pmatrix");

	if(data->flags & T_FLAG_BLEND)
	{
		prog->opacity_loc = glGetUniformLocation(prog->prog, "u_opacity");
	}

	if(data->flags & T_FLAG_PARTICLES)
	{
		prog->wsize_loc = glGetUniformLocation(prog->prog, "u_window_size");
	}

	if(data->flags & T_FLAG_BLUR)
	{
		prog->texsize_loc = glGetUniformLocation(prog->prog, "u_tex_size");
	}

	return 1;
}

static int build_convolution_filter(float* mat, int size, float divisor)
{
	int x, y, size_x, size_y;
	int i = 0;
	const int max_line_len = 128;
	char line[max_line_len];
	char x_str[64];
	char y_str[64];
	float sum_weight = 0.0f;
	char* tex2d_sums = malloc(size * max_line_len);
	if(!tex2d_sums)
	{
		BLTS_LOGGED_PERROR("malloc");
		return 0;
	}

	frag_shader_convolution = malloc(sizeof(convolution_template) +
		size * max_line_len);
	if(!frag_shader_convolution)
	{
		BLTS_LOGGED_PERROR("malloc");
		free(tex2d_sums);
		return 0;
	}

	size_x = size_y = (int)sqrt(size);
	memset(tex2d_sums, 0, size * max_line_len);
	for(y = -(size_y/2); y <= (size_y/2); y++)
	{
		for(x = -(size_x/2); x <= (size_x/2); x++)
		{
			if(mat[i] != 0.0f)
			{
				if(x == 0)
				{
					x_str[0] = 0;
				}
				else if(x == 1)
				{
					sprintf(x_str, " + blur_size");
				}
				else if(x == -1)
				{
					sprintf(x_str, " - blur_size");
				}
				else
				{
					sprintf(x_str, " + blur_size * %d.0", x);
				}

				if(y == 0)
				{
					y_str[0] = 0;
				}
				else if(y == 1)
				{
					sprintf(y_str, " + blur_size");
				}
				else if(y == -1)
				{
					sprintf(y_str, " - blur_size");
				}
				else
				{
					sprintf(y_str, " + blur_size * %d.0", y);
				}


				if(mat[i] == 1.0f)
				{
					sprintf(line,
						"\tsum += texture2D(s_texture, "
						"vec2(v_texCoord.x%s, "
						"v_texCoord.y%s));\n",
						x_str, y_str);
				}
				else if(mat[i] == -1.0f)
				{
					sprintf(line,
						"\tsum -= texture2D(s_texture, "
						"vec2(v_texCoord.x%s, "
						"v_texCoord.y%s));\n",
						x_str, y_str);
				}
				else
				{
					sprintf(line,
						"\tsum += texture2D(s_texture, "
						"vec2(v_texCoord.x%s, "
						"v_texCoord.y%s)) * %.2f;\n",
						x_str, y_str, mat[i]);
				}
				strcat(tex2d_sums, line);
				sum_weight += 1;//mat[i];
			}
			i++;
		}
	}

	if(divisor != 0.0f)
	{
		sprintf(frag_shader_convolution, convolution_template, tex2d_sums,
			divisor);
	}
	else
	{
		sprintf(frag_shader_convolution, convolution_template, tex2d_sums,
			sum_weight);
	}

	BLTS_DEBUG("Used shader:\n%s\n", frag_shader_convolution);

	free(tex2d_sums);
	return 1;
}

static int init(glesh_context* context, s_test_data* data)
{
	int t, i;
	int scenecount = 1;
	int widgetcount = 0;

	if(data->flags & T_FLAG_CONVOLUTION)
	{
		if(!build_convolution_filter(data->test_config->convolution_mat,
			data->test_config->convolution_mat_size,
			data->test_config->convolution_mat_divisor))
		{
			return 0;
		}
		data->flags |= T_FLAG_BLUR;
	}

	if(!load_shaders(data))
	{
		return 0;
	}

	get_shader_locs(data, &data->base_shader);

	if(data->flags & T_FLAG_PARTICLES)
	{
		get_shader_locs(data, &data->particle_shader);
	}

	if(data->flags & T_FLAG_WIDGETS)
	{
		widgetcount = data->test_config->widget_count;
	}

	if(data->flags & T_FLAG_BLEND)
	{
		scenecount = data->test_config->layer_count;
	}

	if(data->flags & T_FLAG_VIDEO_WIDGETS)
	{
		for(t = 0; t < NUM_VIDEO_IMAGES; t++)
		{
			data->video_images[t] = glesh_generate_pattern(
				data->test_config->video_widget_tex_width,
				data->test_config->video_widget_tex_height,
				t * 256 / NUM_VIDEO_IMAGES, GL_RGBA);
			if(!data->video_images[t]) return 0;
		}
	}

	for(i = 0; i < scenecount; i++)
	{
		for(t = 0; t < data->test_config->desktop_count; t++)
		{
			/* Widgets only on topmost desktop */
			if(i == 0)
			{
				generate_desktop(context, data,
					&data->scenes[data->num_scenes], widgetcount);
			}
			else
			{
				generate_desktop(context, data,
					&data->scenes[data->num_scenes], 0);
			}
		}
		data->num_scenes++;
	}

	glUseProgram(data->base_shader.prog);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glViewport(0, 0, context->width, context->height);

	if(data->flags & T_FLAG_ZOOM)
	{
		glesh_set_to_identity(&context->perspective_mat);
		glesh_perspective(&context->perspective_mat, 60.0f,
			(GLfloat)context->width / (GLfloat)context->height, 1.0f, 20.0f);

		glDepthFunc(GL_ALWAYS);
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		glesh_set_to_identity(&context->perspective_mat);
	}

	data->scroll_angle = 0.0f;
	data->rot_angle = 0.0f;
	data->zoom_angle = 0.0f;

	return 1;
}

static int get_new_video_texture(s_test_data* data, int tex_id,
	int offset, const GLenum format)
{
#ifdef USE_ALL_TEXTURE_UNITS
	glActiveTexture(GL_TEXTURE0 + tex_id);
#else
	glActiveTexture(GL_TEXTURE0);
#endif
	glBindTexture(GL_TEXTURE_2D, tex_id);
	if(format == GL_RGBA)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, format,
			data->test_config->video_widget_tex_width,
			data->test_config->video_widget_tex_height, 0, format,
			GL_UNSIGNED_BYTE, data->video_images[offset%NUM_VIDEO_IMAGES]);
	}
	else if(format == GL_RGB)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, format,
			data->test_config->video_widget_tex_width,
			data->test_config->video_widget_tex_height, 0,
			format, GL_UNSIGNED_SHORT_5_6_5,
			data->video_images[offset%NUM_VIDEO_IMAGES]);
	}
	else
	{
		return 0;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return 1;
}

static int draw_object(glesh_context* context, s_test_data* data,
	glesh_object* object, s_shader_program* prog)
{
#ifdef USE_ALL_TEXTURE_UNITS
	glActiveTexture(GL_TEXTURE0 + object->tex->tex_id);
#else
	glActiveTexture(GL_TEXTURE0);
#endif
	glBindTexture(GL_TEXTURE_2D, object->tex->tex_id);

	glUniformMatrix4fv(prog->pmatrix_loc, 1, GL_FALSE,
		(GLfloat*)&context->perspective_mat);

	if(data->flags & T_FLAG_BLUR)
	{
		glUniform1f(prog->texsize_loc, (float)object->tex->width / 2.0f);
	}

	glUniformMatrix4fv(prog->mvmatrix_loc, 1, GL_FALSE,
		(GLfloat*)&object->modelview);
	glVertexAttribPointer(prog->position_loc, 3, GL_FLOAT, GL_FALSE, 0,
		object->vertices);
	glVertexAttribPointer(prog->texcrd_loc, 2, GL_FLOAT, GL_FALSE, 0,
		object->texcoords);
	glEnableVertexAttribArray(prog->position_loc);
	glEnableVertexAttribArray(prog->texcrd_loc);
	glUniform1i(prog->sampler_loc, object->tex->tex_id);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glesh_set_to_identity(&object->modelview);

	return 1;
}

static int draw_widget_shadow(glesh_context* context, s_test_data* data,
	s_widget* widget, float pos)
{
	if(data->flags & T_FLAG_BLEND)
	{
		glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
		glUniform1f(data->base_shader.opacity_loc, (GLfloat)0.5f);
	}

	if(data->flags & T_FLAG_ZOOM)
	{
		glesh_translate(&widget->obj->modelview, 0, 0, data->zoom_angle);
	}

	if(data->flags & T_FLAG_ROTATE)
	{
		glesh_rotate(&widget->obj->modelview, data->rot_angle, 0, 0, 1.0f);
	}
	glesh_translate(&widget->obj->modelview, pos + widget->rel_pos_x + 0.1f,
		widget->rel_pos_y - 0.1f, 0);

	draw_object(context, data, widget->obj, &data->base_shader);

	return 1;
}

static int update_particle(glesh_context* context, s_particle* particle)
{
	float tick = glesh_time_step();

	particle->pos->v[2] += tick;
	if(particle->pos->v[2] >= 0.0f)
	{
		/* Decayed, regenerate particle */
		float p_spread = (float)(rand()%100)/5000.0f;
		int p_angle = rand() & GLESH_COS_SIN_TABLE_MASK;
		particle->x_vec = context->cos_table[p_angle] * p_spread;
		particle->y_vec = context->sin_table[p_angle] * p_spread;
		particle->pos->v[0] = 0.0f;
		particle->pos->v[1] = 0.0f;
		particle->pos->v[2] = -(float)(rand()%100)/100.0f;
	}

	/* Update position */
	particle->pos->v[0] += particle->x_vec;
	particle->pos->v[1] += particle->y_vec;
	particle->y_vec -= tick / 100.0f;

	return 1;
}

static int draw_widget(glesh_context* context, s_test_data* data,
	s_widget* widget, float pos)
{
	int t;

	if(data->flags & T_FLAG_BLEND)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUniform1f(data->base_shader.opacity_loc, 1.0f);
	}

	if(data->flags & T_FLAG_ZOOM)
	{
		glesh_translate(&widget->obj->modelview, 0, 0, data->zoom_angle);
	}

	if(data->flags & T_FLAG_ROTATE)
	{
		glesh_rotate(&widget->obj->modelview, data->rot_angle, 0, 0, 1.0f);
	}

	glesh_translate(&widget->obj->modelview, pos + widget->rel_pos_x,
		widget->rel_pos_y, 0);
	draw_object(context, data, widget->obj, &data->base_shader);

	if(data->flags & T_FLAG_PARTICLES)
	{
		glUseProgram(data->particle_shader.prog);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if(data->flags & T_FLAG_ZOOM)
		{
			glesh_translate(&widget->particle_obj->modelview, 0, 0,
				data->zoom_angle);
		}

		if(data->flags & T_FLAG_ROTATE)
		{
			glesh_rotate(&widget->particle_obj->modelview, data->rot_angle,
				0, 0, 1.0f);
		}

		glesh_translate(&widget->particle_obj->modelview,
			pos + widget->rel_pos_x + 0.1f, widget->rel_pos_y - 0.1f, 0);

		for(t = 0; t < widget->num_particles; t++)
		{
			update_particle(context, &widget->particles[t]);
		}

		glUniformMatrix4fv(data->particle_shader.pmatrix_loc, 1, GL_FALSE,
			(GLfloat*)&context->perspective_mat);
		glUniformMatrix4fv(data->particle_shader.mvmatrix_loc, 1, GL_FALSE,
			(GLfloat*)&widget->particle_obj->modelview);

		glUniform4f(data->particle_shader.wsize_loc, context->width,
			context->height, 0.0f, 0.0f);
		glVertexAttribPointer(data->particle_shader.position_loc, 3, GL_FLOAT,
			GL_FALSE, 0, widget->particle_obj->vertices);
		glEnableVertexAttribArray(data->particle_shader.position_loc);
		glDrawArrays(GL_POINTS, 0, widget->particle_obj->num_vertices);
		glesh_set_to_identity(&widget->particle_obj->modelview);

		glUseProgram(data->base_shader.prog);
	}

	return 1;
}

static int draw_widgets(glesh_context* context, s_test_data* data,
	s_desktop* desktop, float pos)
{
	int t;

	for(t = 0; t < desktop->num_widgets; t++)
	{
		if(data->flags & T_FLAG_VIDEO_WIDGETS)
		{
			desktop->widgets[t].video_time += glesh_time_step();
			if(desktop->widgets[t].video_time >=
				(float)data->test_config->video_widget_generation_freq / 1000.0f)
			{
				desktop->widgets[t].video_time = 0;
				get_new_video_texture(data,
					desktop->widgets[t].video_texture.tex_id,
					++desktop->widgets[t].video_offset, GL_RGBA);
			}
			glesh_attach_texture(desktop->widgets[t].obj,
				&desktop->widgets[t].video_texture);
		}

		if(data->flags & T_FLAG_WIDGET_SHADOWS)
		{
			draw_widget_shadow(context, data, &desktop->widgets[t], pos);
		}

		draw_widget(context, data, &desktop->widgets[t], pos);
	}

	return 1;
}

static int draw_desktop(glesh_context* context, s_test_data* data,
	s_desktop* desktop, float pos)
{
	if(data->flags & T_FLAG_BLEND)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	}

	if(data->flags & T_FLAG_ZOOM)
	{
		glesh_translate(&desktop->obj->modelview, 0, 0, data->zoom_angle);
	}

	if(data->flags & T_FLAG_ROTATE)
	{
		glesh_rotate(&desktop->obj->modelview, data->rot_angle, 0, 0, 1.0f);
	}

	glesh_translate(&desktop->obj->modelview, pos, 0, 0);
	draw_object(context, data, desktop->obj, &data->base_shader);
	draw_widgets(context, data, desktop, pos);

	return 1;
}

static int draw(glesh_context* context, void* user_ptr)
{
	int t, i;
	s_test_data* data = (s_test_data*)user_ptr;
	float pos;

	glClear(GL_COLOR_BUFFER_BIT);

	data->scroll_angle += glesh_time_step() * GLESH_COS_SIN_TABLE_SIZE /
		(float)data->test_config->scroll_speed;
	pos = context->cos_table[((int)data->scroll_angle)&GLESH_COS_SIN_TABLE_MASK] *
		(data->test_config->desktop_count - 1.0f) + 1.0f;

	if(data->flags & T_FLAG_ROTATE)
	{
		data->rot_angle += 50.0f * glesh_time_step();
	}

	if(data->flags & T_FLAG_ZOOM)
	{
		glClear(GL_DEPTH_BUFFER_BIT);
		data->zoom_angle = context->sin_table[
			((int)data->scroll_angle)&GLESH_COS_SIN_TABLE_MASK] - 2.0f;
	}

	for(t = data->num_scenes - 1; t >= 0; t--)
	{
		if(data->flags & T_FLAG_BLEND)
		{
			glUniform1f(data->base_shader.opacity_loc, 1.0f / (float)(t + 1) / 2.0f);
		}

		for(i = 0; i < data->scenes[t].num_desktops; i++)
		{
			draw_desktop(context, data, &data->scenes[t].desktops[i],
				(pos / (t + 1)) + (i - data->test_config->desktop_count / 2.0f) * 2.0f);
		}
	}

	eglSwapBuffers(context->egl_display, context->egl_surface);
	return 1;
}


int test_blitter(test_execution_params* params)
{
	glesh_context* context = NULL;
	s_test_data* data = NULL;
	int ret = -1;

	data = malloc(sizeof(s_test_data));
	if(!data)
	{
		BLTS_LOGGED_PERROR("malloc");
		goto cleanup;
	}
	memset(data, 0, sizeof(s_test_data));

	data->flags = params->flag;
	data->test_config = &params->config;

	context = malloc(sizeof(glesh_context));
	if(!context)
	{
		BLTS_LOGGED_PERROR("malloc");
		goto cleanup;
	}
	memset(context, 0, sizeof(glesh_context));

	BLTS_DEBUG("Running with features:\n");
	BLTS_DEBUG("- %d desktops\n", data->test_config->desktop_count);

	if(data->flags & T_FLAG_BLEND)
	{
		BLTS_DEBUG("- Blended background (%d layers)\n",
			data->test_config->layer_count);
	}

	if(data->flags & T_FLAG_BLUR)
	{
		BLTS_DEBUG("- Entire scene blurred\n");
	}

	if(data->flags & T_FLAG_WIDGETS)
	{
		BLTS_DEBUG("- Widgets on desktop (%d widgets per desktop)\n",
			data->test_config->widget_count);
	}

	if(data->flags & T_FLAG_WIDGET_SHADOWS)
	{
		BLTS_DEBUG("- Widgets with shadows\n");
	}

	if(data->flags & T_FLAG_ROTATE)
	{
		BLTS_DEBUG("- Rotate\n");
	}

	if(data->flags & T_FLAG_ZOOM)
	{
		BLTS_DEBUG("- Zoom in/out\n");
	}

	if(data->flags & T_FLAG_PARTICLES)
	{
		BLTS_DEBUG("- Particles (%d per widget)\n",
			data->test_config->particle_count);
	}

	if(data->flags & T_FLAG_VIDEO_WIDGETS)
	{
		BLTS_DEBUG("- Video thumbnails (texture size: %d x %d)\n",
			data->test_config->video_widget_tex_width,
			data->test_config->video_widget_tex_height);
	}

	if(data->flags & T_FLAG_CONVOLUTION)
	{
		BLTS_DEBUG("- Convolution filter (%d x %d)\n",
			(int)sqrt(params->config.convolution_mat_size),
			(int)sqrt(params->config.convolution_mat_size));
	}

	if(!glesh_create_context(context, NULL, params->w, params->h, params->d))
	{
		BLTS_ERROR("glesh_create_context failed!\n");
		goto cleanup;
	}

	if(!init(context, data))
	{
		BLTS_ERROR("init failed!\n");
		goto cleanup;
	}

	if(!glesh_execute_main_loop(context, draw, data, params->execution_time))
	{
		BLTS_ERROR("glesh_execute_main_loop failed!\n");
		goto cleanup;
	}

	ret = 0;

cleanup:

	if(data)
	{
		free(data);
	}

	if(context)
	{
		glesh_destroy_context(context);
		free(context);
	}

	return ret;
}

