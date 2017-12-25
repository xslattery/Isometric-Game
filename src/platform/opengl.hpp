#ifndef _OPENGL_HPP_
#define _OPENGL_HPP_

#ifdef PLATFORM_OSX
	#include <OpenGL/gl3.h>
#endif

#ifdef PLATFORM_WIN32
	#define GLEW_STATIC
	#include <glew/glew.h>
	#include <glew/wglew.h>
	#include <GL/gl.h>
#endif

#include <iostream>

#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG
#define GLCALL { while ( GLenum error = glGetError() ) \
{ std::cout << "OpenGL Error: F:" << __FILE__ << " L:" << __LINE__ << " : " << error << '\n'; } }
#else
#define GLCALL
#endif

#endif