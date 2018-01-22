#pragma once
#include "opengl.h"
#include <string>
#include <vector>
#include <map>

/*
Shader system for managing  openGL shaders designed by nlguillemot cuz I
don't know what I'm doing!
*/
namespace gourd
{

	class ShaderSystem
	{
	public:
		struct ShaderSource
		{
			std::string name;
			GLenum type;
			bool operator<(const ShaderSource& rhs) const { return type < rhs.type; }
		};

		ShaderSystem();
		~ShaderSystem();

		void SetVersion(std::string version);
		void SetPreamble(std::string name);

		const GLuint* const AddProgram(std::initializer_list<ShaderSource> ls);
		void InitPrograms();

	private:
		std::string ShaderSystem::CreateShaderStringFromFile(const std::string& source_file);

		struct Shader
		{
			uint32_t hash_name = 0;
			GLuint id = 0; 
		};

		struct Program
		{
			GLuint public_id = 0; //set to 0 if program doesnt link so not to add errors
			GLuint inner_id = 0;		
		};

		struct ShaderPair
		{
			typedef std::map<ShaderSource, Shader>::iterator Iterator;
			Iterator iterator;

			ShaderPair(Iterator iter) : iterator(iter) {};
			
			const ShaderSource& GetSource() { return iterator->first; }
			Shader& GetShader() { return iterator->second; }
			const Shader& GetShader() const { return iterator->second; }

			bool operator<(const ShaderPair& rhs) const { return iterator->first.type < rhs.iterator->first.type; }

		};

		struct ProgramPair
		{
			typedef std::map<std::vector<ShaderPair>, Program >::iterator Iterator;
			Iterator iterator;

			ProgramPair(Iterator iter) : iterator(iter) {};

			const std::vector<ShaderPair>& GetVec() { return iterator->first; }
			Program& GetProgram() { return iterator->second; }
			const Program& GetProgram() const { return iterator->second; }
		};

		std::map<ShaderSource, Shader> shaders_;
		std::map<std::vector<ShaderPair>, Program> programs_;

		std::string version_;
		std::string preamble_;
	};

}
