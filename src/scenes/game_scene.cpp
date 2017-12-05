
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

	region.init( window, 32, 32, 32 );
	region.issue_command( Command_Type::GENERATE_REGION_DATA );
}

void Game_Scene::render ( const WindowInfo& window )
{
	create_text_mesh( 
		(
			"DT:"+std::to_string(window.deltaTime) +
			"\nDIM:"+std::to_string((int)window.hidpi_width)+"x"+std::to_string((int)window.hidpi_height)
		).c_str(), 
		textMesh, packedGlyphTexture, shader );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); GLCALL;

	region.render();

	glClear( GL_DEPTH_BUFFER_BIT ); GLCALL;

	glUseProgram( shader ); GLCALL;
	set_uniform_mat4( shader, "projection", &projection );
	set_uniform_mat4( shader, "view", &camera );
	render_text_mesh( textMesh, shader );
}

void Game_Scene::resize ( const WindowInfo& window )
{
	projection = orthographic_projection( window.height, 0, 0, window.width, 0.1f, 100.0f );

	region.resize( window );
}

void Game_Scene::input ( const WindowInfo& window, InputInfo* input )
{	
	if ( get_key_down( input, Key::Key_SPACE ) )
		Scene_Manager::change_scene( SceneType::MainMenu, window );

	if ( get_key_down( input, Key::Key_P ) )
		region.simulationPaused = !region.simulationPaused;

	float speed = 1000;
	// if ( get_key( input, Key::Key_W ) )
	// 	region.camera = translate( region.camera, -vec3(0,1,0)*window.deltaTime*speed );
	// if ( get_key( input, Key::Key_S ) )
	// 	region.camera = translate( region.camera, -vec3(0,-1,0)*window.deltaTime*speed );
	// if ( get_key( input, Key::Key_A ) )
	// 	region.camera = translate( region.camera, -vec3(-1,0,0)*window.deltaTime*speed );
	// if ( get_key( input, Key::Key_D ) )
	// 	region.camera = translate( region.camera, -vec3(1,0,0)*window.deltaTime*speed );

	static float dir = 1;
	static int count = 0;
	if(dir > 0) count++;
	if(dir < 0) count--;
	if(count > 60) { dir = -dir; }
	if(count < -60) { dir = -dir; }
	region.camera = translate( region.camera, -vec3(dir,0,0)*window.deltaTime*speed );
}

//////////////////////////////////////
// Logic Thread - Methods & Data:
void Game_Scene::simulate ()
{
	region.simulate();
}

//////////////////////////////////////
// Destructor: ( Shouldn't care about threads )
Game_Scene::~Game_Scene ()
{
	delete_shader( shader );

	region.cleanup();
}

// #include <mach/mach_time.h>
// static std::size_t startTime;
// startTime = mach_absolute_time();
// std::size_t endTime = mach_absolute_time();
// std::size_t elapsedTime = endTime - startTime;
// static mach_timebase_info_data_t timingInfo;
// if ( mach_timebase_info (&timingInfo) != KERN_SUCCESS ) { printf ("mach_timebase_info failed\n"); }
// float millisecs = (elapsedTime * timingInfo.numer / timingInfo.denom) / 1000000;
// std::cout << millisecs << "ms\n";