#ifndef _SHADER_HPP_
#define _SHADER_HPP_

#include "math/math.hpp"

unsigned int load_shader ( const std::string& vertexShader, const std::string& fragementShader );
void delete_shader( unsigned int shader );

void set_uniform_float ( const unsigned int shader, const char *name, float value );
void set_uniform_float_array ( const unsigned int shader, const char *name, float *value, int count );
void set_uniform_int ( const unsigned int shader, const char *name, int value );
void set_uniform_int_array ( const unsigned int shader, const char *name, int *value, int count );
void set_uniform_vec2 ( const unsigned int shader, const char *name, vec4 *vector );
void set_uniform_vec3 ( const unsigned int shader, const char *name, vec4 *vector );
void set_uniform_vec4 ( const unsigned int shader, const char *name, vec4 *vector );
void set_uniform_mat4 ( const unsigned int shader, const char *name, mat4 *matrix );

void set_uniform_vec2 ( const unsigned int shader, const char *name, vec4 vector );
void set_uniform_vec3 ( const unsigned int shader, const char *name, vec4 vector );
void set_uniform_vec4 ( const unsigned int shader, const char *name, vec4 vector );
void set_uniform_mat4 ( const unsigned int shader, const char *name, mat4 matrix );

#endif