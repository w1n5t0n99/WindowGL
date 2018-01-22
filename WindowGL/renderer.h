#pragma once

#include "opengl.h"
#include "scene.h"
#include "packed_freelist.h"
#include "shader_system.h"

#include <chrono>
#include <cinttypes>

class Renderer
{
public:
	void Init(Scene* scene, int width, int height);
	void Resize(int width, int height);
	void Paint();

	int GetWidth();
	int GetHeight();

	int GetElapsedTime();

private:
	gourd::ShaderSystem shader_system_;
	Scene* scene_;
	int window_height, window_width;

	GLuint color_ubo;
	GLuint transform_ubo;
	GLuint camera_ubo;
	const GLuint* scene_shader;
	
	std::chrono::time_point<std::chrono::system_clock> start_render_time;
};

