#ifndef _GL_DEBUG_HPP_
#define _GL_DEBUG_HPP_

#include <OpenGL/gl3.h>
#include <iostream>

// NOTE(Xavier): (2017.11.15) This should be defined at compile 
// time by argument, so it can be changed depending on the type of build.
#define DEBUG 1

#if DEBUG
#define GLCALL(x) x; while ( GLenum error = glGetError() ) { std::cout << "OpenGL Error: L:" << __LINE__ << " : " << error << '\n'; }
#else
#define GLCALL(x) x;
#endif

#endif