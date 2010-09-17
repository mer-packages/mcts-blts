/* test_frag_shader.c -- Fragment shader performance test

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
	"uniform highp float ripple;\n"
	"uniform highp float scale_x;\n"
	"uniform highp float scale_y;\n"
	"void main()\n"
	"{\n"
	"	highp vec2 pos;\n"
	"	highp vec4 color;\n"
	"	highp vec2 c;\n"
	"   highp vec2 z;\n"
	"   highp vec2 z_square;\n"
	"	highp int n = 0;\n"
	"	highp int iterations = 20;\n"
	"	pos.x = gl_FragCoord.x / scale_x;\n"
	"	pos.y = gl_FragCoord.y / scale_y;\n"
	"	pos.x += ( ( sin(gl_FragCoord.x / 60.0 + ripple) ) / 50.0 );\n"
	"	pos.y += ( ( cos(gl_FragCoord.y / 60.0 + ripple) ) / 50.0 );\n"
	"	c.x = (pos.x - 0.7) * 2.8;\n"
	"	c.y = (pos.y - 0.5) * 2.8;\n"
	"	z = c;\n"
	"	z_square = z * z;\n"
	"	while(n++ < iterations && z_square.x + z_square.y < 4.0)\n"
	"	{\n"
	"		z.y = 2.0 * z.x * z.y + c.y;\n"
	"		z.x = z_square.x - z_square.y + c.x;\n"
	"		z_square = z * z;\n"
	"	}\n"
	"	color = vec4(0.0, 0.0, 0.0, 0.0);\n"
	"	if(n < iterations / 2)\n"
	"	{\n"
	"		color.x = float(n) * 2.0 / float(iterations);\n"
	"		color.y = color.x;\n"
	"	}\n"
	"	else if(n < iterations)\n"
	"	{\n"
	"		color.y = float(n) / float(iterations);\n"
	"		color.z = color.y;\n"
	"	}\n"
	"	gl_FragColor = color;\n"
	"}\n";

typedef struct
{
	int position_loc;
	int scale_x_loc;
	int scale_y_loc;
	int ripple_loc;
	GLfloat ripple;
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
	data->scale_x_loc = glGetUniformLocation(data->shader_program, "scale_x");
	data->scale_y_loc = glGetUniformLocation(data->shader_program, "scale_y");
	data->ripple_loc = glGetUniformLocation(data->shader_program, "ripple");

	glesh_init_object(&object);
	glesh_generate_rectangle_strip(2.0f, 2.0f, &object);
	glesh_add_object(context, &object);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glUseProgram(data->shader_program);

	glVertexAttribPointer(data->position_loc, 3, GL_FLOAT, GL_FALSE, 0,
		context->objects[0].vertices);
	glEnableVertexAttribArray(data->position_loc);

	data->ripple = 0.0f;
	glUniform1f(data->scale_x_loc, (GLfloat)context->width);
	glUniform1f(data->scale_y_loc, (GLfloat)context->height);

	glViewport(0, 0, context->width, context->height);

	return 1;
}

static int draw(glesh_context* context, void* user_ptr)
{
	s_test_data* data = (s_test_data*)user_ptr;
	data->ripple += 0.1f;
	glUniform1f(data->ripple_loc, data->ripple);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	eglSwapBuffers(context->egl_display, context->egl_surface);
	return 1;
}

int test_frag_shader(test_execution_params* params)
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

	glesh_destroy_context(&context);

	return 0;
}

