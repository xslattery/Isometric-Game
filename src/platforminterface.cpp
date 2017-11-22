#include "opengl.hpp"
#include <thread>
#include <chrono>

#include "platform/platform.h"
#include "globals.hpp"
#include "scenemanager.hpp"

static std::atomic<bool> terminateLogicThread;
static void logic_thread_entry ()
{
	while ( !terminateLogicThread )
	{
		Scene_Manager::update_scene();
		std::this_thread::sleep_for( std::chrono::milliseconds(1000) );
	}
	
	std::cout << "Closing Logic Thread." << std::endl;
}

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

    Globals::init();
	Scene_Manager::init( window );

	std::thread logicThread = std::thread( logic_thread_entry );
	logicThread.detach();
}

void input ( const WindowInfo& window, const InputInfo& input )
{
	Scene_Manager::input_scene( window, input );
}

void render ( const WindowInfo& window )
{
	Scene_Manager::render_scene( window );
}

void resize ( const WindowInfo& window )
{
	glViewport( 0, 0, window.hidpi_width, window.hidpi_height ); GLCALL;
	Scene_Manager::resize_scene( window );
}

void cleanup ( const WindowInfo& window )
{
	// NOTE(Xavier): This will be called when the window is closing.
	Scene_Manager::exit();
	terminateLogicThread = true;
}
