in vec2 TexCoord;

out vec4 color;

layout(binding = UNIFORM_BLOCK_COLOR, std140) uniform ColorBlock
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

layout(binding = UNIFORM_TEXTURE_0) uniform sampler2D texture0;


void main()
{

	color = texture(texture0, TexCoord);
} 