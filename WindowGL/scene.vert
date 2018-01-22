layout (location = ATTRIB_LOCATION_POS) in vec3 position;
//layout (location = ATTRIB_LOCATION_COLOR) in vec3 color;
layout (location = ATTRIB_LOCATION_TEX) in vec2 texCoord;

layout(binding = UNIFORM_BLOCK_CAMERA, std140) uniform CameraBlock
{
	mat4 view;
	mat4 projection;
};

layout(binding = UNIFORM_BLOCK_TRANSFORM, std140) uniform TransformBlock
{
	mat4 model;
};

out vec2 TexCoord;

void main()
{
    // Note that we read the multiplication from right to left
    gl_Position = projection * view * model * vec4(position, 1.0f);
	TexCoord = texCoord;
}