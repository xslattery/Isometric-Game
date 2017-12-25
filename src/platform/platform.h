#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "../input.hpp"

////////////////////////////
// NOTE(Xavier): (2017.11.15) This is the window info struct. Its purpose 
// is to be passed by argument to the crossplatform layer from the platform
// layer, providing information about platform speciffic information.
struct WindowInfo
{
	float width;
	float height;
	float hidpi_width;
	float hidpi_height;

	float deltaTime;
};

////////////////////////////
// NOTE(Xavier): (2017.11.15) These are calls the
// platform layer makes to the crossplatform layer:
void init ( const WindowInfo& window );
void input_and_render ( const WindowInfo& window, InputInfo *input );
void resize ( const WindowInfo& window );
void cleanup ( const WindowInfo& window );

////////////////////////////
// NOTE(Xavier): (2017.11.15) Thses are calls the 
// crossplatform layer makes to the platform layer:
void close_window ();

#endif