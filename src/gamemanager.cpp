#include "opengl.hpp"
#include <cstddef>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "platform/platform.h"
#include "math.hpp"
#include "text.hpp"
#include "shader.hpp"


static FT_Library freeType;
static Packed_Glyph_Texture packedGlyphTexture;
static Text_Mesh textMesh = { 0 };
static unsigned int shader;
static mat4 projection;
static mat4 camera;


void init ( const WindowInfo& window )
{
	std::cout << "Initialize\n";
	std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << '\n'; GLCALL;
	std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << '\n'; GLCALL;
	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << '\n'; GLCALL;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n'; GLCALL;

	glEnable( GL_FRAMEBUFFER_SRGB ); GLCALL;
	
	glEnable( GL_DEPTH_TEST ); GLCALL;
	
	glEnable( GL_CULL_FACE ); GLCALL;
	glCullFace( GL_BACK ); GLCALL;
	
	glEnable( GL_BLEND); GLCALL;
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); GLCALL;
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO); GLCALL;
	
	glViewport( 0, 0, window.hidpi_width, window.hidpi_height ); GLCALL;
    
    glClearColor( 0.5f, 0.6f, 0.7f, 1.0f ); GLCALL;

	shader = load_shader(
		R"(
			#version 330 core

			layout(location = 0) in vec3 position;
			layout(location = 1) in vec2 texcoord;
			layout(location = 2) in vec4 color;

			uniform mat4 model;
			uniform mat4 view;
			uniform mat4 projection;

			out vec2 TexCoord;
			out vec4 inColor;

			void main ()
			{
			    gl_Position = projection * view * model * vec4(position, 1.0);
			    TexCoord = texcoord;
			    inColor = color;
			}
		)",
		R"(
			#version 330 core

			in vec2 TexCoord;
			in vec4 inColor;

			uniform sampler2D ourTexture;
			uniform vec4 overlayColor;

			layout(location = 0) out vec4 Color;

			void main ()
			{
			    if ( texture(ourTexture, TexCoord).r == 0 ) discard;
			    Color = vec4( inColor.xyz, texture(ourTexture, TexCoord).r );
			    // Color = vec4( 1, 0, 0, 1 );
			}
		)"
	);

	projection = orthographic_projection( window.height, 0, 0, window.width, 0.1f, 100.0f );
	camera = translate( camera, -vec3(0, 0, 10) );

	FT_Init_FreeType( &freeType );

	packedGlyphTexture.fontsize = 64;
	create_packed_glyph_texture( packedGlyphTexture, "res/Menlo-Regular.ttf", freeType );

	textMesh.position = vec3( 10, 10, 0 );
	textMesh.transform = translate( mat4(1), textMesh.position );	
	textMesh.fontsize = 32;

	create_text_mesh( "Hello World!", textMesh, packedGlyphTexture, shader );
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
	// NOTE(Xavier): (2017.11.19) This was done to fix the OpenGL error 1286 when
	// the window is resizing.
	if ( glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE )
	{
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); GLCALL;

		glUseProgram( shader ); GLCALL;
		
		set_uniform_mat4( shader, "projection", &projection );
		set_uniform_mat4( shader, "view", &camera );
		render_text_mesh( textMesh, shader );
	}
}

void resize ( const WindowInfo& window )
{
	glViewport( 0, 0, window.hidpi_width, window.hidpi_height ); GLCALL;
	projection = orthographic_projection( window.height, 0, 0, window.width, 0.1f, 100.0f );
}

void cleanup ( const WindowInfo& window )
{
	// NOTE(Xavier): This will be called when the window is closing.
	std::cout << "Cleanup\n";
}
