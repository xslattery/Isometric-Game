#ifndef _SHADER_HPP_
#define _SHADER_HPP_

#include "math.hpp"

unsigned int compile_shader ( unsigned int type, const std::string& source );
unsigned int load_shader ( const std::string& vertexShader, const std::string& fragementShader );
void set_uniform_mat4 ( const unsigned int shader, const char *name, mat4 *matrix );

#endif