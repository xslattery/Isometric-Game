
#include "../platform/platform.h"
#include "../globals.hpp"
#include "../shader.hpp"
#include "../scenemanager.hpp"

#include "scene_game.hpp"

//////////////////////////////////////
// Main Thread - Methods:
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

	packedGlyphTexture.fontsize = 32;
	create_packed_glyph_texture( packedGlyphTexture, "res/Menlo-Regular.ttf", Globals::freeType );

	textMesh.position = vec3( 10, 10, 0 );
	textMesh.transform = translate( mat4(1), textMesh.position );	
	textMesh.fontsize = 16;
	create_text_mesh( "Game Scene", textMesh, packedGlyphTexture, shader );

	generatingTextMesh.position = vec3( 10, 200, 0 );
	generatingTextMesh.transform = translate( mat4(1), generatingTextMesh.position );	
	generatingTextMesh.fontsize = 16;
	create_text_mesh( "Generating region...", generatingTextMesh, packedGlyphTexture, shader );

	region_init( window, &region, 32, 32, 32, 2, 2, 6 );
	region_issue_command( &region, {Region_Command_Type::GENERATE_DATA} );
}

void Game_Scene::render ( const WindowInfo& window )
{
	create_text_mesh( (

		"DT: " + std::to_string(window.deltaTime) + "s" +
		"\nSDT: " + std::to_string(region.simulationDeltaTime) + "us" +
		"\nGDT: " + std::to_string(region.generationDeltaTime) + "us" +
		"\nDIM: " + std::to_string((int)window.hidpi_width) + "x" + std::to_string((int)window.hidpi_height) +
		"\nS: " + std::to_string(region.projectionScale) + 
		"\nL: " + std::to_string(region.length) + " W: " + std::to_string(region.width) + " H: " + std::to_string(region.height) +
		"\nCL: " + std::to_string((int)region.chunkLength) + " CW: " + std::to_string((int)region.chunkWidth) + " CH: " + std::to_string((int)region.chunkHeight) +
		"\nWBU: " + std::to_string(region.numberOfWaterBeingUpdated)
		
		).c_str(), textMesh, packedGlyphTexture, shader );

	static bool callOnce_0 = true;
	if ( region.chunkDataGenerated && callOnce_0 )
	{
		callOnce_0 = false;
		create_text_mesh( "Done.", generatingTextMesh, packedGlyphTexture, shader );
	}
	else if ( callOnce_0 )
	{
		static unsigned int wheelCounter = 0;
		if ( wheelCounter % 10 == 0 )
		{
			static unsigned int wheelPos = 0;
			char wheel[8] = { '|', '/', '-', '\\', '|', '/', '-', '\\', };
			create_text_mesh( (std::string("Generating region: ") + wheel[wheelPos]).c_str(), generatingTextMesh, packedGlyphTexture, shader );
			wheelPos++;
			if ( wheelPos >= 8 ) wheelPos = 0;
		}
		wheelCounter++;
	}

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); GLCALL;

	region_render( window, &region );

	glClear( GL_DEPTH_BUFFER_BIT ); GLCALL;

	glUseProgram( shader ); GLCALL;
	set_uniform_mat4( shader, "projection", &projection );
	set_uniform_mat4( shader, "view", &camera );
	render_text_mesh( textMesh, shader );
	render_text_mesh( generatingTextMesh, shader );
}

void Game_Scene::resize ( const WindowInfo& window )
{
	projection = orthographic_projection( window.height, 0, 0, window.width, 0.1f, 100.0f );

	region_resize_viewport( window, &region );
}

void Game_Scene::input ( const WindowInfo& window, InputInfo* input )
{
	if ( get_key_down( input, Key::Key_SPACE ) )
		Scene_Manager::change_scene( SceneType::MainMenu, window );

	if ( get_key_down( input, Key::Key_P ) )
		region.simulationPaused = !region.simulationPaused;

	float speed = 3000;
	if ( get_key( input, Key::Key_RETURN ) )
		speed = 500;
	if ( get_key( input, Key::Key_W ) )
		region.camera = translate( region.camera, -vec3(0,1,0)*window.deltaTime*speed );
	if ( get_key( input, Key::Key_S ) )
		region.camera = translate( region.camera, -vec3(0,-1,0)*window.deltaTime*speed );
	if ( get_key( input, Key::Key_A ) )
		region.camera = translate( region.camera, -vec3(-1,0,0)*window.deltaTime*speed );
	if ( get_key( input, Key::Key_D ) )
		region.camera = translate( region.camera, -vec3(1,0,0)*window.deltaTime*speed );
	
	if ( input->mouseScrollDeltaY != 0 )
	{
		region.projectionScale = region.projectionScale + input->mouseScrollDeltaY*window.deltaTime;
		if ( region.projectionScale <= 0.2 ) region.projectionScale = 0.2f;
		region.projection = orthographic_projection( -window.height/2*region.projectionScale, window.height/2*region.projectionScale, -window.width/2*region.projectionScale, window.width/2*region.projectionScale, 0.1f, 5000.0f );
	}

	if ( get_key( input, Key::Key_Z ) )
	{
		if ( get_key( input, Key::Key_Q ) )
			region.viewHeight--;
		if ( get_key( input, Key::Key_E ) )
			region.viewHeight++;
	} else
	{
		if ( get_key_down( input, Key::Key_Q ) )
			region.viewHeight--;
		if ( get_key_down( input, Key::Key_E ) )
			region.viewHeight++;
	}

	if ( get_key_down( input, Key::Key_J ) )
		region_issue_command( &region, {Region_Command_Type::ROTATE_RIGHT} );
	if ( get_key_down( input, Key::Key_H ) )
		region_issue_command( &region, {Region_Command_Type::ROTATE_LEFT} );

	if ( get_key_down( input, Key::Key_M ) )
		region_issue_command( &region, {Region_Command_Type::ADD_WATER_WAVE} );
}

//////////////////////////////////////
// Simulation Thread - Methods:
void Game_Scene::simulate ()
{
	region_simulate( &region );
}

//////////////////////////////////////
// Generation Thread - Methods:
bool Game_Scene::generate ()
{
	return region_build_new_meshes( &region );
}

//////////////////////////////////////
// Destructor: ( Shouldn't care about threads )
Game_Scene::~Game_Scene ()
{
	delete_shader( shader );

	region_cleanup( &region );
}
