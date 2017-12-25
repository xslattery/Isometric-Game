#ifndef _DEBUG_HPP_
#define _DEBUG_HPP_

#include "scenes/scene_game/region.hpp"

namespace Debug
{
	// TODO(Xavier): (2017.25.17)
	// Implement these functions.
	// void log ();
	// void warning ();
	// void error ();
	// void export_log_to_file ();

	void draw_region_layer_grid ( Region *region );
	void draw_region_chunk_grid ( Region *region );
}

#endif