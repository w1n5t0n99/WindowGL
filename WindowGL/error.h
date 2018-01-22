#pragma once
#include <iostream>
#include <string>
#include "opengl.h"

namespace gourd
{
	//=================================
	// Check OpenGL error state
	//===================================
	inline GLenum GLCheckError(const char *file, int line)
	{
		GLenum errorCode;
		while ((errorCode = glGetError()) != GL_NO_ERROR)
		{
			std::string error;
			switch (errorCode)
			{
			case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
			case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
			case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
			case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
			case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
			case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
			}
			std::cerr << error << " | " << file << " (" << line << ")" << std::endl;
		}
		return errorCode;
	}

	//==============================================================
	// OpenGL debug output callback
	//===============================================================
	inline void APIENTRY GLDebugOutput(GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar *message,
		const void *userParam)
	{
		// ignore non-significant error/warning codes
		if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

		std::cerr << "---------------" << std::endl;
		std::cerr << "Debug message (" << id << "): " << message << std::endl;

		switch (source)
		{
		case GL_DEBUG_SOURCE_API:             std::cerr << "Source: API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cerr << "Source: Window System"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cerr << "Source: Shader Compiler"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cerr << "Source: Third Party"; break;
		case GL_DEBUG_SOURCE_APPLICATION:     std::cerr << "Source: Application"; break;
		case GL_DEBUG_SOURCE_OTHER:           std::cerr << "Source: Other"; break;
		} std::cerr << std::endl;

		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR:               std::cerr << "Type: Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cerr << "Type: Deprecated Behaviour"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cerr << "Type: Undefined Behaviour"; break;
		case GL_DEBUG_TYPE_PORTABILITY:         std::cerr << "Type: Portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:         std::cerr << "Type: Performance"; break;
		case GL_DEBUG_TYPE_MARKER:              std::cerr << "Type: Marker"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          std::cerr << "Type: Push Group"; break;
		case GL_DEBUG_TYPE_POP_GROUP:           std::cerr << "Type: Pop Group"; break;
		case GL_DEBUG_TYPE_OTHER:               std::cerr << "Type: Other"; break;
		} std::cerr << std::endl;

		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:         std::cerr << "Severity: high"; break;
		case GL_DEBUG_SEVERITY_MEDIUM:       std::cerr << "Severity: medium"; break;
		case GL_DEBUG_SEVERITY_LOW:          std::cerr << "Severity: low"; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: std::cerr << "Severity: notification"; break;
		} std::cerr << std::endl;
		std::cerr << std::endl;
	}

	//===================================================
	// Simple stack based error context
	//=====================================================
	class ErrorContext;
	static thread_local ErrorContext* KHead = nullptr;

	class ErrorContext
	{
	public:
		// ErrorContext shouldn't go out of scope before the values it holds does
		ErrorContext(const char* description, const char* value, const char* file, int line) : description_(description),
			value_(value), file_(file), line_(line), prev_(KHead)
		{
			KHead = this;
		}

		~ErrorContext()
		{
			KHead = prev_;
		}

	private:
		// Prevent heap allocation
		void * operator new (size_t);
		void * operator new[](size_t);
		void   operator delete (void *);
		void   operator delete[](void*);

		const char* description_;
		const char* value_;
		const char* file_;
		int line_;
		ErrorContext* prev_;

		friend void CheckErrorContext();
	};

	//======================================
	// Output all error contexts
	//=======================================
	inline void CheckErrorContext()
	{
		auto cur = KHead;
		while (cur != nullptr)
		{
			fprintf(stderr, "%s: %s \"%s (%d)\"\n", cur->description_, cur->value_, cur->file_, cur->line_);
			cur = cur->prev_;
		}
	}

#if defined(DEVELOPMENT)
#define Gourd_ErrorContext(description, value)  gourd::ErrorContext error_context(description, value, __FILE__, __LINE__)
#else
#define Gourd_ErrorContext(description, value) ((void)0)
#endif

#if defined(DEVELOPMENT)
#define Gourd_CheckErrorContext() gourd::CheckErrorContext()
#else
#define Gourd_CheckErrorContext() ((void)0)
#endif

#if defined(DEVELOPMENT)
#define Gourd_GL_CheckError() gourd::GLCheckError(__FILE__, __LINE__) 
#else
#define Gourd_GL_CheckError() ((void)0)
#endif

}

