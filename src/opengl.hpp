#ifndef _GL_DEBUG_HPP_
#define _GL_DEBUG_HPP_

#include <OpenGL/gl3.h>
#include <iostream>

#ifndef DEBUG
#define DEBUG 1
#endif

#if DEBUG
#define GLCALL { while ( GLenum error = glGetError() ) \
{ std::cout << "OpenGL Error: F:" << __FILE__ << " L:" << __LINE__ << " : " << error << '\n'; } }
#else
#define GLCALL
#endif

#endif