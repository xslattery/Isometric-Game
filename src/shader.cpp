#include "opengl.hpp"

#include "shader.hpp"

static unsigned int compile_shader ( unsigned int type, const std::string& source )
{
	unsigned int shader = glCreateShader( type ); GLCALL;
	const char *src = source.c_str();
	glShaderSource( shader, 1, &src, nullptr ); GLCALL;
	glCompileShader( shader ); GLCALL;

	int result;
	glGetShaderiv( shader, GL_COMPILE_STATUS, &result ); GLCALL;
	if ( result == GL_FALSE )
	{
		int length;
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &length ); GLCALL;
		
		char *message = (char*)alloca( length * sizeof(char) );
		glGetShaderInfoLog( shader, length, &length, message ); GLCALL;
		
		std::cout << "Shader Failed to Compile: " << '\n';
		std::cout << message << '\n';
	}

	return shader;
}

unsigned int load_shader ( const std::string& vertexShader, const std::string& fragementShader )
{
	unsigned int program = glCreateProgram(); GLCALL;
	unsigned int vs = compile_shader( GL_VERTEX_SHADER, vertexShader );
	unsigned int fs = compile_shader( GL_FRAGMENT_SHADER, fragementShader );

	glAttachShader( program, vs); GLCALL;
	glAttachShader( program, fs); GLCALL;
	glLinkProgram( program ); GLCALL;
	glValidateProgram( program ); GLCALL;

	glDetachShader( program, vs ); GLCALL;
	glDetachShader( program, fs ); GLCALL;

	glDeleteShader(vs); GLCALL;
	glDeleteShader(fs); GLCALL;

	return program;
}

void delete_shader( unsigned int shader )
{
	glDeleteProgram( shader );
}

void set_uniform_float ( const unsigned int shader, const char *name, float value )
{
    glUniform1f( glGetUniformLocation(shader, name), value );  GLCALL;
}

void set_uniform_float_array ( const unsigned int shader, const char *name, float *value, int count )
{
    glUniform1fv( glGetUniformLocation(shader, name), count, value );  GLCALL;
}

void set_uniform_int ( const unsigned int shader, const char *name, int value )
{
    glUniform1i( glGetUniformLocation(shader, name), value );  GLCALL;
}

void set_uniform_int_array ( const unsigned int shader, const char *name, int *value, int count )
{
    glUniform1iv( glGetUniformLocation(shader, name), count, value );  GLCALL;
}

void set_uniform_vec2 ( const unsigned int shader, const char *name, vec4 *vector )
{
    glUniform2f( glGetUniformLocation(shader, name), vector->x, vector->y ); GLCALL;
}

void set_uniform_vec3 ( const unsigned int shader, const char *name, vec4 *vector )
{
    glUniform3f( glGetUniformLocation(shader, name), vector->x, vector->y, vector->z ); GLCALL;
}

void set_uniform_vec4 ( const unsigned int shader, const char *name, vec4 *vector )
{
    glUniform4f( glGetUniformLocation(shader, name), vector->x, vector->y, vector->z, vector->w ); GLCALL;
}

void set_uniform_mat4 ( const unsigned int shader, const char *name, mat4 *matrix )
{
    glUniformMatrix4fv( glGetUniformLocation( shader, name ), 1, GL_FALSE, (float*)matrix ); GLCALL;
}