
#include "../platform/platform.h"
#include "../globals.hpp"
#include "../shader.hpp"
#include "../scenemanager.hpp"

#include "game_scene.hpp"

//////////////////////////////////////
// Main Thread - Methods & Data:
void Game_Scene::init( const WindowInfo& window )
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

	textMesh.position = vec3( 100, 100, 0 );
	textMesh.transform = translate( mat4(1), textMesh.position );	
	textMesh.fontsize = 32;

	create_text_mesh( "Game Scene", textMesh, packedGlyphTexture, shader );


	region.init( 32, 32, 32 );
	region.generate();
	// TODO(Xavier): (2017.11.29)
	// This needs to be setup so the user can 
	// specify the save file to load from.
	// region.load();
}

void Game_Scene::render ( const WindowInfo& window )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); GLCALL;

	region.render();

	glUseProgram( shader ); GLCALL;
	set_uniform_mat4( shader, "projection", &projection );
	set_uniform_mat4( shader, "view", &camera );
	render_text_mesh( textMesh, shader );
}

void Game_Scene::resize ( const WindowInfo& window )
{
	projection = orthographic_projection( window.height, 0, 0, window.width, 0.1f, 100.0f );
}

void Game_Scene::input ( const WindowInfo& window, InputInfo* input )
{
	if ( get_key_down( input, Key::Key_SPACE ) )
	{
		Scene_Manager::change_scene( SceneType::MainMenu, window );
	}
}

//////////////////////////////////////
// Logic Thread - Methods & Data:
void Game_Scene::simulate ()
{
	std::cout << "Game Scene\n";

	region.simulate();
}

//////////////////////////////////////
// Destructor: ( Shouldn't care about threads )
Game_Scene::~Game_Scene ()
{
	delete_shader( shader );

	region.cleanup();
}