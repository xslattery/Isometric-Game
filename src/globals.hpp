#ifndef _GLOBALS_HPP_
#define _GLOBALS_HPP_

#include <ft2build.h>
#include FT_FREETYPE_H

struct Globals
{
	static void init ();

	static FT_Library freeType;
};

#endif
