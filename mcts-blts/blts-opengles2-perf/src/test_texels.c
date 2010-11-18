/* test_texels.c -- Texels-per-second test

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
#include "ogles2_helper.h"
#include "test_common.h"

static const char vertex_shader[] =
	"attribute vec4 a_position;\n"
	"attribute vec2 a_texCoord;\n"
	"varying vec2 v_texCoord;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = a_position;\n"
	"	v_texCoord = a_texCoord;\n"
	"}\n";

static const char frag_shader[] =
	"varying mediump vec2 v_texCoord;\n"
	"uniform sampler2D s_texture;\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = texture2D( s_texture, v_texCoord );\n"
	"}\n";

typedef struct
{
	int position_loc;
	int sampler_loc;
	int texcrd_loc;
	GLuint shader_program;
} s_test_data;

static int init(glesh_context* context, s_test_data* data)
{
	glesh_object object;
	glesh_texture* tex;

	data->shader_program = glesh_load_program(vertex_shader, frag_shader);
	if(!data->shader_program)
	{
		BLTS_ERROR("Failed to load shader program\n");
		return 0;
	}

	data->position_loc = glGetAttribLocation(data->shader_program,
		"a_position");
	data->texcrd_loc = glGetAttribLocation(data->shader_program,
		"a_texCoord");
	data->sampler_loc = glGetUniformLocation(data->shader_program,
		"s_texture");

	tex = glesh_generate_texture(context, GL_RGBA, 1024, 1024, "test");
	if(!tex)
	{
		BLTS_ERROR("Failed to generate texture\n");
		return 0;
	}

	glesh_init_object(&object);
	glesh_generate_rectangle_strip(2.0f, 2.0f, &object);
	glesh_attach_texture(&object, tex);
	glesh_add_object(context, &object);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glUseProgram(data->shader_program);

	glActiveTexture(GL_TEXTURE0 + context->objects[0].tex->tex_id);
	glBindTexture(GL_TEXTURE_2D, context->objects[0].tex->tex_id);

	glVertexAttribPointer(data->position_loc, 3, GL_FLOAT, GL_FALSE, 0,
		context->objects[0].vertices);
	glVertexAttribPointer(data->texcrd_loc, 2, GL_FLOAT, GL_FALSE, 0,
		context->objects[0].texcoords);
	glEnableVertexAttribArray(data->position_loc);
	glEnableVertexAttribArray(data->texcrd_loc);

	glUniform1i(data->sampler_loc, context->objects[0].tex->tex_id);

	glViewport(0, 0, context->width, context->height);

	return 1;
}

static int draw(glesh_context* context, void* user_ptr)
{
	int t;
	UNUSED_PARAM(user_ptr)
	glClear(GL_COLOR_BUFFER_BIT);

	for(t = 0; t < 8; t++)
	{
		context->objects[0].texcoords[t] += 0.001f;
	}

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	eglSwapBuffers(context->egl_display, context->egl_surface);
	return 1;
}


int test_texels(test_execution_params* params)
{
	glesh_context context;
	s_test_data data;

	if(!glesh_create_context(&context, NULL, params->w, params->h, params->d))
	{
		BLTS_ERROR("glesh_create_context failed!\n");
		return -1;
	}

	if(!init(&context, &data))
	{
		BLTS_ERROR("init failed!\n");
		return -1;
	}

	if(!glesh_execute_main_loop(&context, draw, &data, params->execution_time))
	{
		BLTS_ERROR("glesh_execute_main_loop failed!\n");
	}

	BLTS_DEBUG("Texels per second: %lf\n", context.width * context.height *
		context.perf_data.frames_rendered /
		context.perf_data.total_time_elapsed);

	glesh_destroy_context(&context);

	return 0;
}

