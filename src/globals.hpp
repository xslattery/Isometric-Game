#ifndef _GLOBALS_HPP_
#define _GLOBALS_HPP_

#include <ft2build.h>
#include FT_FREETYPE_H

struct Globals
{
	static void init ();

	static FT_Library freeType;
	
	// TEMP(Xavier): (2017.12.1)
	// This is here for testing the region rendering
	// this info should be passes by function argument instead of being
	// global to the entire program.
	static unsigned int window_width;
	static unsigned int window_height;
};

#endif
