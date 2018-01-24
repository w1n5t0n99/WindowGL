#include "renderer.h"
#include "preamble.glsl"

#define GLM_FORCE_RADIANS 
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>



//=====================================================
// Init render data
//=====================================================
void Renderer::Init(Scene* scene, int width, int height)
{
	window_width = width;
	window_height = height;
	scene_ = scene;

	start_render_time = std::chrono::system_clock::now();

	// Define the viewport dimensions
	glViewport(0, 0, window_width, window_height);
	
	// init shaders
	shader_system_.SetVersion("440");
	shader_system_.SetPreamble("preamble.glsl");

	scene_shader = shader_system_.AddProgram({ { "scene.vert", GL_VERTEX_SHADER },{ "scene.frag", GL_FRAGMENT_SHADER } });
	shader_system_.InitPrograms();
	// init ubos
	glCreateBuffers(1, &color_ubo);
	glNamedBufferData(color_ubo, sizeof(float) * 10, nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BLOCK_COLOR, color_ubo);

	auto first_mat = scene_->materials_.Begin();
	// update ubo
	glNamedBufferSubData(color_ubo, 0, sizeof(float) * 3, (*first_mat).ambient);
	glNamedBufferSubData(color_ubo, sizeof(float) * 3, sizeof(float) * 3, (*first_mat).diffuse);
	glNamedBufferSubData(color_ubo, sizeof(float) * 6, sizeof(float) * 3, (*first_mat).specular);
	glNamedBufferSubData(color_ubo, sizeof(float) * 9, sizeof(float), &(*first_mat).shininess);

	//glUniformBlockBinding(*scene_shader, ubo_index, static_cast<GLuint>(BindingPoint::KUniform_Block_Color));

	glCreateBuffers(1, &camera_ubo);
	glNamedBufferData(camera_ubo, sizeof(glm::mat4) * 2, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BLOCK_CAMERA, camera_ubo);

	glCreateBuffers(1, &transform_ubo);
	glNamedBufferData(transform_ubo, sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BLOCK_TRANSFORM, transform_ubo);

	glm::mat4 view;
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), static_cast<float>(window_width) / static_cast<float>(window_height), 0.1f, 100.0f);

	glNamedBufferSubData(camera_ubo, 0, sizeof(glm::mat4), glm::value_ptr(view));
	glNamedBufferSubData(camera_ubo, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));

}

//=================================================================
// Resizing - reinitialize framebuffers, textures, .etc
//=================================================================
void Renderer::Resize(int width, int height)
{
	window_height = height;
	window_width = width;

	glViewport(0, 0, window_width, window_height);
	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), static_cast<float>(window_width) / static_cast<float>(window_height), 0.1f, 100.0f);
	glNamedBufferSubData(camera_ubo, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));

}

//======================================================================
// Get elapsed time since renderer initialization in milliseconds
//=======================================================================
int Renderer::GetElapsedTime()
{
	auto end = std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(end - start_render_time).count();
}

//======================================
// Render scene
//========================================
void Renderer::Paint()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS);

	// Clear the colorbuffer
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(*scene_shader);

	auto& test_mesh = scene_->meshes_[scene_->test_mesh_];

	glBindTextureUnit(UNIFORM_TEXTURE_0, scene_->materials_[test_mesh.material_id].diffuse_map_id);

	glm::mat4 model;
	model = glm::rotate(model, glm::radians(static_cast<float>(GetElapsedTime() * 0.01)), glm::vec3(0.5f, 1.0f, 0.0f));
	//glm::translate(model, glm::vec3(0.5f, 1.0f, 0.0f));
	glNamedBufferSubData(transform_ubo, 0, sizeof(glm::mat4), glm::value_ptr(model));


	glBindVertexArray(test_mesh.mesh_vao);

	auto& draw_command = test_mesh.draw_command;
	/*
	glDrawElementsInstancedBaseVertexBaseInstance(
		GL_TRIANGLES,
		draw_command.count,
		GL_UNSIGNED_INT, 0,
		draw_command.prim_count,
		draw_command.base_vertex,
		draw_command.base_instance);
		*/

	glDrawArraysInstanced(
		GL_TRIANGLES,
		draw_command.first,
		draw_command.count,
		draw_command.primCount);

	glBindVertexArray(0);

}

//===============================
// Return height
//================================
int Renderer::GetHeight()
{
	return window_height;
}

//===============================
// Return width
//================================
int Renderer::GetWidth()
{
	return window_width;
}