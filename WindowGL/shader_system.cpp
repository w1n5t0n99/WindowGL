#include "shader_system.h"
#include <functional>
#include <fstream>
#include <functional>
#include <iostream>

namespace gourd
{

	//=====================================
	// Ctor
	//=====================================
	ShaderSystem::ShaderSystem()
	{

	}

	//======================================
	//  Dtor
	//=======================================
	ShaderSystem::~ShaderSystem()
	{
		for (auto it = shaders_.begin(); it != shaders_.end(); ++it)
		{
			ShaderPair sp(it);
			glDeleteShader(sp.GetShader().id);
		}

		for (auto it = programs_.begin(); it != programs_.end(); ++it)
		{
			ProgramPair pp(it);
			glDeleteProgram(pp.GetProgram().inner_id);
		}

	}

	//========================================
	// Set version to append to shader source
	//==========================================
	void ShaderSystem::SetVersion(std::string version)
	{
		version_ = std::move(version);
	}

	//=============================================
	// Set preamble to append to shader source
	//==============================================
	void ShaderSystem::SetPreamble(std::string preamble)
	{
		preamble_ = CreateShaderStringFromFile(preamble);
	}

	//=============================================
	// Add create shader program and return id
	//===============================================
	const GLuint* const ShaderSystem::AddProgram(std::initializer_list<ShaderSource> ls)
	{
		Shader shader_temp;
		std::vector<ShaderPair> shaders_for_program;

		for (auto src : ls)
		{
			ShaderPair sp = shaders_.emplace(std::make_pair(src, shader_temp)).first;
			// if hasn't been added yet
			if (!sp.GetShader().id)
			{
				sp.GetShader().id = glCreateShader(sp.GetSource().type);
				sp.GetShader().hash_name = std::hash<std::string>{} (sp.GetSource().name);
			}

			shaders_for_program.push_back(sp);
		}

		ProgramPair pp = programs_.emplace(std::make_pair(shaders_for_program, Program{})).first;
		if (!pp.GetProgram().inner_id)
		{
			// public id left alone until successful linking
			pp.GetProgram().inner_id = glCreateProgram();
			for (auto sp : pp.GetVec())
			{
				glAttachShader(pp.GetProgram().inner_id, sp.GetShader().id);
			}
		}

		return &pp.GetProgram().public_id;
	}

	//=======================================================
	// Compile and link shaders if needed
	//==========================================================
	void ShaderSystem::InitPrograms()
	{

		std::string preamble_hash = std::to_string(static_cast<uint32_t>(std::hash<std::string>{}("preamble")));
		// compile all shaders
		for (auto it = shaders_.begin(); it != shaders_.end(); ++it)
		{
			ShaderPair sp(it);
			// the #line prefix ensures error messages have the right line number for their file
			// the #line directive also allows specifying a "file name" number, which makes it possible to identify which file the error came from.
			std::string version = "#version " + version_ + "\n";

			std::string defines;
			switch (sp.GetSource().type)
			{
			case GL_VERTEX_SHADER:          defines += "#define VERTEX_SHADER\n";             break;
			case GL_FRAGMENT_SHADER:        defines += "#define FRAGMENT_SHADER\n";           break;
			case GL_GEOMETRY_SHADER:        defines += "#define GEOMETRY_SHADER\n";           break;
			case GL_TESS_CONTROL_SHADER:    defines += "#define TESS_CONTROL_SHADER\n";       break;
			case GL_TESS_EVALUATION_SHADER: defines += "#define TESS_EVALUATION_SHADER\n";    break;
			case GL_COMPUTE_SHADER:         defines += "#define COMPUTE_SHADER\n";            break;
			}

			std::string preamble = "#line 1 " + preamble_hash + "\n" + preamble_ + "\n";

			std::string source_hash = std::to_string(sp.GetShader().hash_name);
			std::string source = "#line 1 " + source_hash + "\n" + CreateShaderStringFromFile(sp.GetSource().name) + "\n";

			const char* strings[] =
			{
				version.c_str(),
				defines.c_str(),
				preamble.c_str(),
				source.c_str()
			};
			GLint lengths[] =
			{
				(GLint)version.length(),
				(GLint)defines.length(),
				(GLint)preamble.length(),
				(GLint)source.length()
			};

			glShaderSource(sp.GetShader().id, sizeof(strings) / sizeof(*strings), strings, lengths);
			glCompileShader(sp.GetShader().id);

			GLint status;
			glGetShaderiv(sp.GetShader().id, GL_COMPILE_STATUS, &status);
			if (!status)
			{
				GLint logLength;
				glGetShaderiv(sp.GetShader().id, GL_INFO_LOG_LENGTH, &logLength);
				std::vector<char> log(logLength + 1);
				glGetShaderInfoLog(sp.GetShader().id, logLength, NULL, log.data());

				std::string log_s = log.data();

				// replace all filename hashes in the error messages with actual filenames
				for (size_t found_preamble; (found_preamble = log_s.find(preamble_hash)) != std::string::npos;) 
				{
					log_s.replace(found_preamble, preamble_hash.size(), "preamble");
				}
				for (size_t found_source; (found_source = log_s.find(source_hash)) != std::string::npos;)
				{
					log_s.replace(found_source, source_hash.size(), sp.GetSource().name);
				}

				fprintf(stderr, "Error compiling %s:\n%s\n", sp.GetSource().name.c_str(), log_s.c_str());
			}

		}

		// link all programs 
		for (auto it = programs_.begin(); it != programs_.end(); ++it)
		{
			ProgramPair pp(it);
			glLinkProgram(pp.GetProgram().inner_id);
			
			GLint logLength;
			glGetProgramiv(pp.GetProgram().inner_id, GL_INFO_LOG_LENGTH, &logLength);
			std::vector<char> log(logLength + 1);
			glGetProgramInfoLog(pp.GetProgram().inner_id, logLength, NULL, log.data());

			std::string log_s = log.data();
			// replace all filename hashes in the error messages with actual filenames
			for (size_t found_preamble; (found_preamble = log_s.find(preamble_hash)) != std::string::npos;)
			{
				log_s.replace(found_preamble, preamble_hash.size(), "preamble");
			}
			for (auto sp : pp.GetVec())
			{
				std::string source_hash = std::to_string(sp.GetShader().hash_name);
				for (size_t found_source; (found_source = log_s.find(source_hash)) != std::string::npos;)
				{
					log_s.replace(found_source, source_hash.size(), sp.GetSource().name);
				}
			}

			GLint status;
			glGetProgramiv(pp.GetProgram().inner_id, GL_LINK_STATUS, &status);

			if (!status)
			{
				fprintf(stderr, "Error linking");
				pp.GetProgram().public_id = 0;
			}
			else
			{
				fprintf(stderr, "Successfully linked");
				pp.GetProgram().public_id = pp.GetProgram().inner_id;
			}

			fprintf(stderr, " program (");
			int index = 0;
			for (auto sp : pp.GetVec())
			{
				if (index > 0)
				{
					fprintf(stderr, ", ");
				}

				fprintf(stderr, "%s", sp.GetSource().name.c_str());
				++index;
			}
			fprintf(stderr, ")");
			if (log[0] != '\0')
			{
				fprintf(stderr, ":\n%s\n", log_s.c_str());
			}
			else
			{
				fprintf(stderr, "\n");
			}

		}
	}

	//=================================================================
	// Create finalized shader source code string
	//===================================================================
	std::string ShaderSystem::CreateShaderStringFromFile(const std::string& source_file)
	{
		std::ifstream file_stream(source_file);
		//copy preamble
		if (file_stream)
		{
			std::string s(
				std::istreambuf_iterator<char>{file_stream},
				std::istreambuf_iterator<char>{});

			return s;
		}

		return "";
	}

}