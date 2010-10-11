/* test_fillrate.c -- Fillrate test

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
	"void main()\n"
	"{\n"
	"	gl_Position = a_position;\n"
	"}\n";

static const char frag_shader[] =
	"uniform mediump float u_color;\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = vec4(0.0, u_color, 0.4, 0.0);\n"
	"}\n";

typedef struct
{
	int position_loc;
	int color_loc;
	GLfloat color;
	GLuint shader_program;
} s_test_data;

static int init(glesh_context* context, s_test_data* data)
{
	glesh_object object;

	data->shader_program = glesh_load_program(vertex_shader, frag_shader);
	if(!data->shader_program)
	{
		LOGERR("Failed to load shader program\n");
		return 0;
	}

	data->position_loc = glGetAttribLocation(data->shader_program,
		"a_position");
	data->color_loc = glGetUniformLocation(data->shader_program, "u_color");

	glesh_init_object(&object);
	glesh_generate_rectangle_strip(2.0f, 2.0f, &object);
	glesh_add_object(context, &object);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glUseProgram(data->shader_program);

	glVertexAttribPointer(data->position_loc, 3, GL_FLOAT, GL_FALSE, 0,
		context->objects[0].vertices);
	glEnableVertexAttribArray(data->position_loc);

	glViewport(0, 0, context->width, context->height);

	data->color = 0.0f;

	return 1;
}

static int draw(glesh_context* context, void* user_ptr)
{
	s_test_data* data = (s_test_data*)user_ptr;
	data->color += 0.01f;
	if(data->color >= 1.0f) data->color = 0.0f;
	glUniform1f(data->color_loc, data->color);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	eglSwapBuffers(context->egl_display, context->egl_surface);
	return 1;
}


int test_fillrate(test_execution_params* params)
{
	glesh_context context;
	s_test_data data;

	if(!glesh_create_context(&context, NULL, params->w, params->h, params->d))
	{
		LOGERR("glesh_create_context failed!\n");
		return -1;
	}

	if(!init(&context, &data))
	{
		LOGERR("init failed!\n");
		return -1;
	}

	if(!glesh_execute_main_loop(&context, draw, &data, params->execution_time))
	{
		LOGERR("glesh_execute_main_loop failed!\n");
	}

	LOG("Pixels per second: %lf\n", context.width * context.height *
		context.perf_data.frames_rendered /
		context.perf_data.total_time_elapsed);

	glesh_destroy_context(&context);

	return 0;
}

