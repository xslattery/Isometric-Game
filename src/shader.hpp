#ifndef _SHADER_HPP_
#define _SHADER_HPP_

#include "math.hpp"

unsigned int load_shader ( const std::string& vertexShader, const std::string& fragementShader );
void delete_shader( unsigned int shader );
void set_uniform_mat4 ( const unsigned int shader, const char *name, mat4 *matrix );

#endif