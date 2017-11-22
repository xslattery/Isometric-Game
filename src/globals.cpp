
#include "globals.hpp"

void Globals::init ()
{
	FT_Init_FreeType( &freeType );
}

FT_Library Globals::freeType;