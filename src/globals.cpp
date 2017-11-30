
#include "globals.hpp"

void Globals::init ()
{
	FT_Init_FreeType( &freeType );
}

FT_Library Globals::freeType;
unsigned int Globals::window_width;
unsigned int Globals::window_height;