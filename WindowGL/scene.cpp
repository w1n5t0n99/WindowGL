#include "scene.h"
#include "preamble.glsl"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

//=================================================
// Initialize scene data
//================================================
void Scene::Init()
{
	Sprite test_quad =
	{
		" ",
		0, 0, 0, 0, 0,
		0
	};

	GLuint mesh_vao = 0;
	GLuint index_bo = 0;
	GLuint position_bo = 0;
	GLuint normal_bo = 0;
	GLuint tex_coord_bo = 0;

	GLfloat cube_vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	GLuint indices[] =
	{
		0, 1, 3,
		1, 2, 3
	};

	test_quad.name = "test_quad";
	// Create openGL objects
	test_quad.draw_command = { 36, 1, 0, 0 };
	glCreateVertexArrays(1, &mesh_vao);
	glCreateBuffers(1, &index_bo);
	glCreateBuffers(1, &position_bo);
	glCreateBuffers(1, &normal_bo);
	glCreateBuffers(1, &tex_coord_bo);

	test_quad.mesh_vao = mesh_vao;
	test_quad.index_bo = index_bo;
	test_quad.position_bo = position_bo;
	test_quad.normal_bo = normal_bo;
	test_quad.tex_coord_bo = tex_coord_bo;

	// Load buffers and assign to binding points
	glNamedBufferData(test_quad.position_bo, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
	glVertexArrayVertexBuffer(test_quad.mesh_vao, static_cast<GLuint>(BindingPoint::KAttribute_Vertex), test_quad.position_bo, 0, sizeof(GL_FLOAT) * 5);
	glVertexArrayVertexBuffer(test_quad.mesh_vao, static_cast<GLuint>(BindingPoint::KAttribute_Tex_Coord), test_quad.position_bo, sizeof(GL_FLOAT) * 3, sizeof(GL_FLOAT) * 5);
	// Position attribute
	glEnableVertexArrayAttrib(test_quad.mesh_vao, ATTRIB_LOCATION_POS);
	glVertexArrayAttribFormat(test_quad.mesh_vao, ATTRIB_LOCATION_POS, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(test_quad.mesh_vao, ATTRIB_LOCATION_POS, static_cast<GLuint>(BindingPoint::KAttribute_Vertex));
	// Texture attribute
	glEnableVertexArrayAttrib(test_quad.mesh_vao, ATTRIB_LOCATION_TEX);
	glVertexArrayAttribFormat(test_quad.mesh_vao, ATTRIB_LOCATION_TEX, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(test_quad.mesh_vao, ATTRIB_LOCATION_TEX, static_cast<GLuint>(BindingPoint::KAttribute_Tex_Coord));

		// load texture
	Material mat_vin = { "wallpaper.png",{ 0.4f, 0.4f, 0.4f },{ 0.4f, 0.4f, 0.4f },{ 0.4f, 0.4f, 0.4f }, 0.4f, 1 };
	Material mat_ryan = { "ryan.png",{ 0.3f, 0.3f, 0.3f },{ 0.4f, 0.4f, 0.4f },{ 0.4f, 0.4f, 0.4f }, 0.4f, 1 };
	Material mat_brad = { "brad.png",{ 0.5f, 0.5f, 0.5f },{ 0.4f, 0.4f, 0.4f },{ 0.4f, 0.4f, 0.4f }, 0.4f, 1 };
	Material mat_jeff = { "jeff.png",{ 0.4f, 0.4f, 0.4f },{ 0.0f, 0.4f, 0.4f },{ 1.0f, 1.0f, 1.0f }, 0.4f, 1 };
	Material mat_drew = { "drew.png",{ 0.5f, 0.5f, 0.5f },{ 0.4f, 0.4f, 0.4f },{ 0.4f, 0.4f, 0.4f }, 0.4f, 1 };
	Material mat_alex = { "alex.png",{ 0.4f, 0.4f, 0.4f },{ 1.0f, 1.0f, 1.0f },{ 0.4f, 0.4f, 0.4f }, 0.4f, 1 };

	auto alex_mat_id = materials_.Add(mat_alex);
	auto brad_mat_id = materials_.Add(mat_brad);
	auto drew_mat_id = materials_.Add(mat_drew);
	auto jeff_mat_id = materials_.Add(mat_jeff);
	auto ryan_mat_id = materials_.Add(mat_ryan);
	auto vin_mat_id = materials_.Add(mat_vin);

	Sprite vin_mesh, ryan_mesh, brad_mesh, jeff_mesh, alex_mesh, drew_mesh;

	vin_mesh = test_quad;
	vin_mesh.material_id = vin_mat_id;

	ryan_mesh = test_quad;
	ryan_mesh.material_id = ryan_mat_id;

	brad_mesh = test_quad;
	brad_mesh.material_id = brad_mat_id;

	jeff_mesh = test_quad;
	jeff_mesh.material_id = jeff_mat_id;

	alex_mesh = test_quad;
	alex_mesh.material_id = alex_mat_id;

	drew_mesh = test_quad;
	drew_mesh.material_id = drew_mat_id;

	auto v_mesh_id = meshes_.Add(vin_mesh);
	auto r_mesh_id = meshes_.Add(ryan_mesh);
	auto b_mesh_id = meshes_.Add(brad_mesh);
	auto j_mesh_id = meshes_.Add(jeff_mesh);
	auto a_mesh_id = meshes_.Add(alex_mesh);
	auto d_mesh_id = meshes_.Add(drew_mesh);

	int w, h, n;
	for (auto it = materials_.Begin(); it != materials_.End(); ++it)
	{
		stbi_set_flip_vertically_on_load(1);
		auto data = stbi_load((*it).name.c_str(), &w, &h, &n, 4);
		stbi_set_flip_vertically_on_load(0);
		if (data == nullptr)
			std::cerr << (*it).name.c_str() << " failed to load" << std::endl;

		GLuint tex_id;
		glCreateTextures(GL_TEXTURE_2D, 1, &tex_id);

		glTextureParameteri(tex_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(tex_id, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(tex_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(tex_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureStorage2D(tex_id, 1, GL_SRGB8_ALPHA8, w, h);
		glTextureSubImage2D(tex_id, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateTextureMipmap(tex_id);

		(*it).diffuse_map_id = tex_id;
	}
	
	test_mesh_ = v_mesh_id;

	// load transforms
	Transform t_vin, t_ryan, t_brad, t_jeff, t_drew, t_alex;
	t_vin.translation = glm::vec3(0.0f, 0.0f, 0.0f);
	t_ryan.translation = glm::vec3(2.0f, 5.0f, -15.0f);
	t_brad.translation = glm::vec3(-1.5f, -2.2f, -2.5f);
	t_jeff.translation = glm::vec3(-3.8f, -2.0f, -12.3f);
	t_drew.translation = glm::vec3(2.4f, -0.4f, -3.5f);
	t_alex.translation = glm::vec3(-1.7f, 3.0f, -7.5f);

	auto vin_tran_id = transforms_.Add(t_vin);
	auto ryan_tran_id = transforms_.Add(t_ryan);
	auto brad_tran_id = transforms_.Add(t_brad);
	auto jeff_tran_id = transforms_.Add(t_jeff);
	auto drew_tran_id = transforms_.Add(t_drew);
	auto alex_tran_id = transforms_.Add(t_alex);

	instances_.Add({ v_mesh_id, vin_tran_id });
	instances_.Add({ r_mesh_id, ryan_tran_id });
	instances_.Add({ b_mesh_id, brad_tran_id });
	instances_.Add({ j_mesh_id, jeff_tran_id });
	instances_.Add({ d_mesh_id, drew_tran_id });
	instances_.Add({ a_mesh_id, alex_tran_id });

}

