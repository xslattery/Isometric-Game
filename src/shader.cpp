#include "opengl.hpp"

#include "shader.hpp"

unsigned int compile_shader ( unsigned int type, const std::string& source )
{
	unsigned int shader = GLCALL( glCreateShader( type ) );
	const char *src = source.c_str();
	GLCALL( glShaderSource( shader, 1, &src, nullptr ) );
	GLCALL( glCompileShader( shader ) );

	int result;
	GLCALL( glGetShaderiv( shader, GL_COMPILE_STATUS, &result ) );
	if ( result == GL_FALSE )
	{
		int length;
		GLCALL( glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &length ) );
		char* message = (char*)alloca( length * sizeof(char) );
		GLCALL( glGetShaderInfoLog( shader, length, &length, message ) );
		std::cout << "Shader Failed to Compile: " << '\n';
		std::cout << message << '\n';
	}

	return shader;
}

unsigned int load_shader ( const std::string& vertexShader, const std::string& fragementShader )
{
	unsigned int program = GLCALL( glCreateProgram() );
	unsigned int vs = compile_shader( GL_VERTEX_SHADER, vertexShader );
	unsigned int fs = compile_shader( GL_FRAGMENT_SHADER, fragementShader );

	GLCALL( glAttachShader( program, vs) );
	GLCALL( glAttachShader( program, fs) );
	GLCALL( glLinkProgram( program ) );
	GLCALL( glValidateProgram( program ) );

	GLCALL( glDetachShader( program, vs ) );
	GLCALL( glDetachShader( program, fs ) );

	GLCALL( glDeleteShader(vs) );
	GLCALL( glDeleteShader(fs) );

	return program;
}

void set_uniform_mat4 ( const unsigned int shader, const char *name, mat4 *matrix )
{
    glUniformMatrix4fv( glGetUniformLocation( shader, name ), 1, GL_FALSE, (float*)matrix );
}