#pragma once

#include "opengl.h"
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "packed_freelist.h"

#include <array>
#include <vector>
#include <memory>
#include <string>
#include <map>

constexpr size_t Max_Components = 2000;
constexpr size_t Max_Cameras = 100;

enum class BindingPoint : GLuint
{
	KUniform_Block_Transform = 0,
	KUniform_Block_Camera = 1,
	KUniform_Block_Color = 2,

	KAttribute_Vertex = 0,
	KAttribute_Tex_Coord = 1,
	KAttribute_Normal = 2,
};

/*
// According to glDrawElementsIndirect documentation
typedef struct GLDrawElementsIndirectCommand
{
	GLuint count;
	GLuint prim_count;
	GLuint first__index;
	GLint base_vertex;
	GLuint base_instance;
} GLDrawElementsIndirectCommand;

// According to glDrawArraysIndirect documentation
typedef struct GLDrawArraysIndirectCommand
{
	GLuint count;
	GLuint prim_count;
	GLuint first;
	GLuint base_instance;
} GLDrawArraysIndirectCommand;

*/

struct DiffuseMap
{
	GLuint diffuse_map_to;
};

struct Material
{
	std::string name;

	float ambient[3];
	float diffuse[3];
	float specular[3];
	float shininess;
	uint32_t diffuse_map_id;
};

struct Mesh
{
	std::string name;

	GLuint mesh_vao;
	GLuint position_bo;
	GLuint tex_coord_bo;
	GLuint normal_bo;
	GLuint index_bo;

	GLuint index_count;
	GLuint vertext_count;

	std::vector<GLDrawElementsIndirectCommand> draw_commands;
	std::vector<uint32_t> material_ids;
};

struct Sprite
{
	std::string name;

	GLuint mesh_vao;
	GLuint position_bo;
	GLuint tex_coord_bo;
	GLuint normal_bo;
	GLuint index_bo;

	GLDrawArraysIndirectCommand draw_command;
	uint32_t material_id;
};


struct Transform
{
	glm::vec3 scale;
	glm::vec3 rotation_origin;
	glm::quat rotation;
	glm::vec3 translation;
};

struct Instance
{
	uint32_t mesh_id;
	uint32_t transform_id;
};

struct Camera
{
	// View
	glm::vec3 eye;
	glm::vec3 target;
	glm::vec3 up;

	// Projection
	float fov_y;
	float aspect;
	float z_near;
};

class Scene
{
public:
	PackedFreelist<Transform, Max_Components> transforms_;
	PackedFreelist<Sprite, Max_Components> meshes_;
	PackedFreelist<Material, Max_Components> materials_;
	PackedFreelist<Instance, Max_Components> instances_;
	PackedFreelist<Camera, Max_Cameras> cameras_;

	uint32_t main_camera_id_;
	uint32_t test_mesh_;

	void Init();

};

