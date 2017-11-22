#include "opengl.hpp"
#include <thread>
#include <chrono>

#include "platform/platform.h"
#include "input.hpp"
#include "globals.hpp"
#include "scenemanager.hpp"


///////////////////////////////////
// Simulation Threaad:
static std::atomic<bool> terminateSimulationThread;
static void simulation_thread_entry ()
{
	while ( !terminateSimulationThread )
	{
		Scene_Manager::simulate_scene();
		std::this_thread::sleep_for( std::chrono::milliseconds(1000) );
	}
	
	#if DEBUG
		std::cout << "Exited Logic Thread." << std::endl;
	#endif
}


//////////////////////////////
// Main Thread:
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

	// Begin the simulation thread:
	std::thread simulationThread = std::thread( simulation_thread_entry );
	simulationThread.detach();
}

void input_and_render ( const WindowInfo& window, InputInfo *input )
{
	Scene_Manager::input_scene( window, input );
	Scene_Manager::render_scene( window );
}

void resize ( const WindowInfo& window )
{
	glViewport( 0, 0, window.hidpi_width, window.hidpi_height ); GLCALL;
	Scene_Manager::resize_scene( window );
}

void cleanup ( const WindowInfo& window )
{
	Scene_Manager::exit();
	terminateSimulationThread = true;
}
