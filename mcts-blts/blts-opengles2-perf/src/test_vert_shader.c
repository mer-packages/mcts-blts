/* test_vert_shader.c -- Vertex shader performance test

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
	"uniform highp mat4 mv_matrix;\n"
	"uniform highp mat4 p_matrix;\n"
	"uniform highp float bend;\n"
	"uniform highp vec3 lightDir;\n"
	"uniform highp vec3 eyeDir;\n"
	"varying lowp vec3 diffuseIntensity;\n"
	"varying lowp vec3 specularIntensity;\n"
	"float calc_wave(vec4 pos)\n"
	"{\n"
	"	highp float ret = pos.z;\n"
	"	ret += (sin(bend + pos.x * 5.0) + cos(bend + pos.y * 7.0)  + 1.0) / 5.0;\n"
	"	ret += (cos(bend + pos.y * 9.0) + sin(bend + pos.x * 13.0) + 1.0) / 5.0;\n"
	"	ret -= sqrt((pos.x * pos.x + pos.y * pos.y)) / 1.0;\n"
	"	return ret;\n"
	"}\n"
	"vec3 estimate_normal(vec4 pos)\n"
	"{\n"
	"	highp vec3 norm;\n"
	"	highp vec4 pos2;\n"
	"	pos2 = pos;\n"
	"	pos2.x += 2.0;\n"
	"	pos2.z += calc_wave(pos2);\n"
	"	norm.x = length(pos - pos2);\n"
	"	pos2 = pos;\n"
	"	pos2.y += 2.0;\n"
	"	pos2.z += calc_wave(pos2);\n"
	"	norm.y = length(pos - pos2);\n"
	"	pos2 = pos;\n"
	"	pos2.z += 2.0;\n"
	"	pos2.z += calc_wave(pos2);\n"
	"	norm.z = length(pos - pos2);\n"
	"	return normalize(norm);\n"
	"}\n"
	"void light(highp vec3 normal, highp vec3 eyeDir, highp vec3 lightDir, lowp vec3 lightColor)\n"
	"{\n"
	"	lowp float NdotL = max(dot(normal, lightDir), 0.0);\n"
	"	diffuseIntensity += NdotL * lightColor;\n"
	"	highp vec3 halfVector = normalize(lightDir + eyeDir);\n"
	"	highp float NdotH = max(dot(normal, halfVector), 0.0);\n"
	"	highp float specular = pow(NdotH, 0.5);\n"
	"	specularIntensity += specular * lightColor;\n"
	"}\n"
	"void main()\n"
	"{\n"
	"	highp vec4 pos = a_position;\n"
	"	highp vec4 pos2;\n"
	"	highp vec3 norm;\n"
	"	int cnt = 10;\n"
	"	while(cnt-- > 0)\n"
	"	{\n"
	"		pos.z += calc_wave(pos) / float(10);\n"
	"		norm = estimate_normal(pos);\n"
	"	}\n"
	"	diffuseIntensity = vec3(0.0);\n"
	"	specularIntensity = vec3(0.0);\n"
	"	light(norm, normalize(eyeDir), normalize(lightDir), vec3(0.3, 0.5, 0.3));\n"
	"	gl_Position = p_matrix * mv_matrix * pos;\n"
	"}\n";

static const char frag_shader[] =
	"varying lowp vec3  diffuseIntensity;\n"
	"varying lowp vec3  specularIntensity;\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = vec4(diffuseIntensity + specularIntensity, 1.0);\n"
	"}\n";

typedef struct
{
	int position_loc;
	int mv_matrix_loc;
	int p_matrix_loc;
	int bend_loc;

	int	light_dir_loc;
	int	eye_dir_loc;

	float bend;
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
	data->mv_matrix_loc = glGetUniformLocation(data->shader_program,
		"mv_matrix");
	data->p_matrix_loc = glGetUniformLocation(data->shader_program,
		"p_matrix");
	data->bend_loc = glGetUniformLocation(data->shader_program,
		"bend");
	data->light_dir_loc = glGetUniformLocation(data->shader_program,
		"lightDir");
	data->eye_dir_loc = glGetUniformLocation(data->shader_program,
		"eyeDir");

	glesh_init_object(&object);
	glesh_generate_plane(5.0f, 60, &object);
	glesh_translate(&object.modelview, 0.0f, 0.0f, -3.50f);
	glesh_add_object(context, &object);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glesh_set_to_identity(&context->perspective_mat);
	glesh_perspective(&context->perspective_mat, 60.0f,
		(GLfloat)context->width / (GLfloat)context->height, 1.0f, 20.0f);

	glesh_set_to_identity(&context->mvp_mat);
	glesh_multiply(&context->mvp_mat, &context->objects[0].modelview,
		&context->perspective_mat);

	glUseProgram(data->shader_program);

	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	glViewport(0, 0, context->width, context->height);

	glesh_vector3 lightDir;
	lightDir.v[0] = 0.0f;
	lightDir.v[1] = 1.0f;
	lightDir.v[2] = 1.0f;
	glUniform3fv(data->light_dir_loc, 1, lightDir.v);

	glesh_vector3 eyeDir;
	eyeDir.v[0] = 0.0f;
	eyeDir.v[1] = 0.0f;
	eyeDir.v[2] = 1.0f;
	glUniform3fv(data->eye_dir_loc, 1, eyeDir.v);

	data->bend = 1.0;

	glUniformMatrix4fv(data->p_matrix_loc, 1, GL_FALSE,
		(GLfloat*)&context->perspective_mat);
	glUniformMatrix4fv(data->mv_matrix_loc, 1, GL_FALSE,
		(GLfloat*)&context->objects[0].modelview);
	glVertexAttribPointer(data->position_loc, 3, GL_FLOAT, GL_FALSE, 0,
		context->objects[0].vertices);
	glEnableVertexAttribArray(data->position_loc);

	return 1;
}

static int draw(glesh_context* context, void* user_ptr)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	s_test_data* data = (s_test_data*)user_ptr;
	data->bend += 0.1f;
	glUniform1f(data->bend_loc, data->bend);

	glDrawElements(GL_TRIANGLES, context->objects[0].num_indices,
		GL_UNSIGNED_INT, context->objects[0].indices);
	eglSwapBuffers(context->egl_display, context->egl_surface);

	return 1;
}

int test_vert_shader(test_execution_params* params)
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

