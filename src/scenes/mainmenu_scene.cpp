
#include "../platform/platform.h"
#include "../globals.hpp"
#include "../shader.hpp"
#include "../scenemanager.hpp"

#include "mainmenu_scene.hpp"

//////////////////////////////////////
// Main Thread - Methods & Data:
void MainMenu_Scene::init( const WindowInfo& window )
{
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

	packedGlyphTexture.fontsize = 64;
	create_packed_glyph_texture( packedGlyphTexture, "res/Menlo-Regular.ttf", Globals::freeType );

	textMesh.position = vec3( 10, 10, 0 );
	textMesh.transform = translate( mat4(1), textMesh.position );	
	textMesh.fontsize = 32;

	create_text_mesh( "Main Menu Scene", textMesh, packedGlyphTexture, shader );
}

void MainMenu_Scene::render( const WindowInfo& window )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); GLCALL;

	glUseProgram( shader ); GLCALL;
	
	set_uniform_mat4( shader, "projection", &projection );
	set_uniform_mat4( shader, "view", &camera );
	render_text_mesh( textMesh, shader );
}

void MainMenu_Scene::resize ( const WindowInfo& window )
{
	projection = orthographic_projection( window.height, 0, 0, window.width, 0.1f, 100.0f );
}

void MainMenu_Scene::input( const WindowInfo& window, InputInfo* input )
{
	if ( get_key_down( input, Key::Key_SPACE ) )
	{
		Scene_Manager::change_scene( SceneType::Game, window );
	}
}

//////////////////////////////////////
// Logic Thread - Methods & Data:
void MainMenu_Scene::simulate()
{
	// This will never be called ( idealy ).
}

//////////////////////////////////////
// Destructor: ( Shouldn't care about threads )
MainMenu_Scene::~MainMenu_Scene()
{
	delete_shader( shader );
}