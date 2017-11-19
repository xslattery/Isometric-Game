#include <OpenGL/gl3.h>
#include <cstddef>
#include <iostream>
#include <mach/mach_time.h>

#include "platform/platform.h"
#include "math.hpp"

// NOTE(Xavier): (2017.11.15) This should be defined at compile 
// time by argument, so it can be changed depending on the type of build.
#define DEBUG 1

#if DEBUG
#define GLCALL(x) x; check_opengl_errors( __LINE__ );
#else
#define GLCALL(x) x;
#endif

static void check_opengl_errors ( size_t line )
{
	while ( GLenum error = glGetError() )
	{
		std::cout << "OpenGL Error: L:" << line << " : " << error << '\n';
	}
}

static unsigned int compile_shader ( unsigned int type, const std::string& source )
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

static unsigned int load_shader ( const std::string& vertexShader, const std::string& fragementShader )
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

void set_uniform_mat4 ( const GLint programID, const GLchar* name, mat4* matrix )
{
    glUniformMatrix4fv( glGetUniformLocation( programID, name ), 1, GL_FALSE, (float*)matrix );
}

void init ( const WindowInfo& window )
{
	std::cout << "Initialize\n";
	std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << '\n';
	std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << '\n';
	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << '\n';
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';

	GLCALL( glEnable( GL_FRAMEBUFFER_SRGB ) );
	GLCALL( glEnable( GL_DEPTH_TEST ) );
	GLCALL( glEnable( GL_CULL_FACE ) );
	GLCALL( glCullFace( GL_BACK ) );
	
	GLCALL( glViewport( 0, 0, window.hidpi_width, window.hidpi_height ) );
    
    GLCALL( glClearColor( 0.5f, 0.6f, 0.7f, 1.0f ) );

	unsigned int shader = load_shader(
		R"(
			#version 330 core

			layout(location = 0) in vec4 position;
			layout(location = 1) in vec4 color;

			uniform mat4 projection;
			uniform mat4 view;

			out vec4 vertColor;

			void main ()
			{
				gl_Position = projection * view * position;
				vertColor = color;
			}
		)",
		R"(
			#version 330 core

			in vec4 vertColor;

			out vec4 color;

			void main ()
			{
				color = vertColor;
			}
		)"
	);

	GLCALL( glUseProgram( shader ) );

	mat4 projection = orthographic_projection( -window.height/2, window.height/2, -window.width/2, window.width/2, 0, 100 );
	set_uniform_mat4( shader, "projection", &projection );

	mat4 camera;
	camera = scale( camera, vec3(100, 100, 1) );
	camera = rotate( camera, vec3(0, 0, 1), 2);
	camera = translate( camera, vec3(1, 0, 0)*100 );
	GLCALL( set_uniform_mat4( 1, "view", &camera ) );

	float verts [] =
	{
		-0.5f, -0.5f, 	1.0f, 0.0f, 0.0f, 1.0f,
		0.5f, -0.5f, 	0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 0.5f, 	0.0f, 0.0f, 1.0f, 1.0f
	};

	unsigned int vao;
	GLCALL( glGenVertexArrays( 1, &vao ) );
	GLCALL( glBindVertexArray( vao ) );

	unsigned int vbo;
	GLCALL( glGenBuffers( 1, &vbo ) );
	GLCALL( glBindBuffer( GL_ARRAY_BUFFER, vbo ) );
	GLCALL( glBufferData( GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW ) );

	GLCALL( glEnableVertexAttribArray( 0 ) );
	GLCALL( glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*6, 0 ) );

	GLCALL( glEnableVertexAttribArray( 1 ) );
	GLCALL( glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, sizeof(float)*6, (void*)(sizeof(float)*2) ) );
}

void input ( const WindowInfo& window, const InputInfo& input )
{
	switch ( input.type )
	{
		case InputType::KeyDown:
		{
		} break;

		case InputType::KeyUp:
		{
		} break;

		case InputType::MouseMove:
		{
		} break;

		case InputType::MouseDrag:
		{
		} break;

		case InputType::MouseScroll:
		{
		} break;

		case InputType::MouseDown:
		{
		} break;

		case InputType::MouseUp:
		{
		} break;

		case InputType::MouseEnter:
		{
		} break;

		case InputType::MouseExit:
		{
		} break;
	
		default: { } break;
	}
}

void render ( const WindowInfo& window )
{
	GLCALL( glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) );

	GLCALL( glDrawArrays( GL_TRIANGLES, 0, 3 ) );

	// static uint64_t startTime = mach_absolute_time();
	// uint64_t endTime = mach_absolute_time();
	// uint64_t elapsedTime = endTime - startTime;
	// startTime = mach_absolute_time();
	// mach_timebase_info_data_t info;
	// if (mach_timebase_info (&info) != KERN_SUCCESS) { printf ("mach_timebase_info failed\n"); }
	// uint64_t nanosecs = elapsedTime * info.numer / info.denom;
	// uint64_t millisecs = nanosecs / 1000000;
	// std::cout << millisecs << "ms\n";
}

void resize ( const WindowInfo& window )
{
	GLCALL( glViewport( 0, 0, window.hidpi_width, window.hidpi_height ) );
}

void cleanup ( const WindowInfo& window )
{
	// NOTE(Xavier): This will be called when the window is closing.
	std::cout << "Cleanup\n";
}
