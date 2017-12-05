#include "platform/opengl.hpp"
#include <thread>
#include <chrono>

#include "platform/platform.h"
#include "input.hpp"
#include "globals.hpp"
#include "scenemanager.hpp"

#ifdef PLATFORM_OSX
#include <mach/mach_time.h>
#endif

///////////////////////////////////
// Simulation Threaad:
static std::atomic<bool> terminateSimulationThread;
static void simulation_thread_entry ()
{
	// NOTE(Xavier): (2017.12.5)
	// This needs to be made crossplatform.
	#ifdef PLATFORM_OSX
		mach_timebase_info_data_t timingInfoSimulation;
		if ( mach_timebase_info (&timingInfoSimulation) != KERN_SUCCESS )
			printf ("ERROR: mach_timebase_info failed\n");
		std::size_t startTime;
	#endif

	std::size_t delta = 0;
	while ( !terminateSimulationThread )
	{
		#ifdef PLATFORM_OSX
			startTime = mach_absolute_time();
		#endif
		
		Scene_Manager::simulate_scene();
	
		#ifdef PLATFORM_OSX
			std::size_t endTime = mach_absolute_time();
			std::size_t elapsedTime = endTime - startTime;
			delta = (elapsedTime * timingInfoSimulation.numer / timingInfoSimulation.denom) / 1000000;
		#endif

		std::this_thread::sleep_for( std::chrono::milliseconds(100-delta) );
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
	
	// NOTE(Xavier): (2017.11.29) This was done to fix the OpenGL error 1286 when
	// the window is resizing.
	if ( glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE )
	{
		Scene_Manager::render_scene( window );
	}
}

void resize ( const WindowInfo& window )
{
	glViewport( 0, 0, window.hidpi_width, window.hidpi_height ); GLCALL;
	Scene_Manager::resize_scene( window );
}

void cleanup ( const WindowInfo& window )
{
	terminateSimulationThread = true;
	Scene_Manager::exit();
}
