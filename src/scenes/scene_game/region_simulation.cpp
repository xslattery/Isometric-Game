
#ifdef PLATFORM_OSX
#include <mach/mach_time.h>
#endif
#include "region.hpp"
#include <cmath>

#include "../../perlin.hpp"

inline Chunk_Data* region_get_chunk ( Region *region, int x, int y, int z );
inline unsigned int region_get_floor ( Region *region, int x, int y, int z );
inline unsigned int region_get_wall ( Region *region, int x, int y, int z );
inline unsigned int region_get_water ( Region *region, int x, int y, int z );
inline void region_set_floor ( Region *region, int x, int y, int z, unsigned int floor );
inline void region_set_wall ( Region *region, int x, int y, int z, unsigned int wall );
inline void region_set_water ( Region *region, int x, int y, int z, unsigned int water );

static float generate_height_data ( float xx, float yy, float scale, int octaves, float persistance, float lacunarity, bool power )
{
	if ( scale <= 0 ) scale = 0.0001f;
	if ( octaves < 1 ) octaves = 1;
	if ( persistance > 1 ) persistance = 1;
	if ( persistance < 0 ) persistance = 0;
	if ( lacunarity < 1 ) lacunarity = 1;

	float amplitude = 1.0f;
	float frequency = 1.0f;
	float noiseValue = 0.0f;

	for ( int i = 0; i < octaves; ++i )
	{
		float sampleX = xx / scale * frequency;
		float sampleZ = yy / scale * frequency;

		float nv = noise_2d(sampleX, sampleZ);

		noiseValue += nv * amplitude;

		amplitude *= persistance;
		frequency *= lacunarity;
	}

	if ( power ) noiseValue = pow(2.71828182845904523536, noiseValue);

	return noiseValue; 
}



void region_generate ( Region *region )
{
	for ( int z = 0; z < region->height; ++z )
	{
		for ( int y = 0; y < region->width; ++y )
		{
			for ( int x = 0; x < region->length; ++x )
			{
				Chunk_Data *chunk = region_get_chunk( region, x, y, z );

				for ( int cz = 0; cz < region->chunkHeight; ++cz )
				{
					for ( int cy = 0; cy < region->chunkWidth; ++cy )
					{
						for ( int cx = 0; cx < region->chunkLength; ++cx )
						{
							unsigned int* floor = &chunk->floor[ cx + cy*region->chunkLength + cz*region->chunkLength*region->chunkWidth ];
							unsigned int* wall = &chunk->wall[ cx + cy*region->chunkLength + cz*region->chunkLength*region->chunkWidth ];
							unsigned int* water = &chunk->water[ cx + cy*region->chunkLength + cz*region->chunkLength*region->chunkWidth ];
							
							*floor = Floor::FLOOR_NONE;
							*wall = Wall::WALL_NONE;
							*water = 0;

							int genHeight = static_cast<int>(generate_height_data( cx+(x*region->chunkLength), cy+(y*region->chunkWidth), 350, 4, 0.5f, 2.5f, 1 ) * 25.0f ) + 64;
							if ( genHeight > cz+(z*region->chunkHeight) ) *floor = Floor::FLOOR_STONE;
							if ( genHeight-1 > cz+(z*region->chunkHeight) ) *wall = Wall::WALL_STONE;
							
							if ( cz+(z*region->chunkHeight) == region->height*region->chunkHeight-1 ) *water = rand()%5;
						}
					}
				}
			}
		}
	}

	region->chunksNeedingMeshUpdate_mutex.lock();
	
	for ( unsigned int i = 0; i < region->length*region->width*region->height; ++i )
	{
		region->chunksNeedingMeshUpdate[i] = Chunk_Mesh_Data_Type::FLOOR | Chunk_Mesh_Data_Type::WALL | Chunk_Mesh_Data_Type::WATER;
	}
	
	region->chunksNeedingMeshUpdate_mutex.unlock();

	region->chunkDataGenerated = true;
}



void region_save ( Region *region )
{
	// TODO(Xavier): (2017.12.7)
	// Implement this function.
}



void region_load ( Region *region )
{
	// TODO(Xavier): (2017.12.7)
	// Implement this function.

	region->chunkDataGenerated = true;
}



static void process_commands ( Region *region )
{
	auto execute_command = [&]( Region_Command& command )
	{
		switch ( command.type )
		{
			case Region_Command_Type::GENERATE_DATA:
				region_generate( region );
				break;

			case Region_Command_Type::ROTATE_LEFT:
				// TODO(Xavier): (2017.12.7)
				// Handle the command.
				break;

			case Region_Command_Type::ROTATE_RIGHT:
				// TODO(Xavier): (2017.12.7)
				// Handle the command.
				break;

			default: break;
		}
	};

	if ( region->commandQue_mutex_1.try_lock() )
	{
		for ( auto& command : region->commandQue_1 )
		{
			execute_command( command );
		}
		region->commandQue_1.clear();
		region->commandQue_mutex_1.unlock();
	}
	
	if ( region->commandQue_mutex_2.try_lock() )
	{
		for ( auto& command : region->commandQue_2 )
		{
			execute_command( command );
		}
		region->commandQue_2.clear();
		region->commandQue_mutex_2.unlock();
	}
}



void region_simulate ( Region *region )
{
	process_commands( region );

	if ( region->chunkDataGenerated )
	{
		if ( !region->simulationPaused )
		{
			// TODO(Xavier): (2017.12.7)
			// Implement the simulation here.
		}

		// NOTE(Xavier): (2017.12.8)
		// This Should me moved to its own thread.
		region_build_new_meshes( region );
	}

#ifdef PLATFORM_OSX
	mach_timebase_info_data_t timingInfoSimulation;
	if ( mach_timebase_info (&timingInfoSimulation) != KERN_SUCCESS )
		printf ("ERROR: mach_timebase_info failed\n");

	static std::size_t startTime;
	std::size_t endTime = mach_absolute_time();
	std::size_t elapsedTime = endTime - startTime;
	startTime = mach_absolute_time();
	region->simulationDeltaTime = (elapsedTime * timingInfoSimulation.numer / timingInfoSimulation.denom) / 1000;
#endif
}



inline Chunk_Data* region_get_chunk ( Region *region, int x, int y, int z )
{
	if ( x < 0 || x >= region->length || y < 0 || y >= region->width || z < 0 || z >= region->height ) return nullptr;
	return &region->chunks[ x + y*region->length + z*region->length*region->width ];
}

inline unsigned int region_get_floor ( Region *region, int x, int y, int z )
{
	if ( x < 0 || x >= region->chunkLength*region->length || y < 0 || y >= region->chunkWidth*region->width || z < 0 || z >= region->chunkHeight*region->height ) return 0;
	unsigned int cx = x / region->chunkLength;
	unsigned int cy = y / region->chunkWidth;
	unsigned int cz = z / region->chunkHeight;
	unsigned int lx = x % region->chunkLength;
	unsigned int ly = y % region->chunkWidth;
	unsigned int lz = z % region->chunkHeight;
	return region_get_chunk( region, cx, cy, cz )->floor[ lx + ly*region->chunkLength + lz*region->chunkLength*region->chunkWidth ];
}

inline unsigned int region_get_wall ( Region *region, int x, int y, int z )
{
	if ( x < 0 || x >= region->chunkLength*region->length || y < 0 || y >= region->chunkWidth*region->width || z < 0 || z >= region->chunkHeight*region->height ) return 0;
	unsigned int cx = x / region->chunkLength;
	unsigned int cy = y / region->chunkWidth;
	unsigned int cz = z / region->chunkHeight;
	unsigned int lx = x % region->chunkLength;
	unsigned int ly = y % region->chunkWidth;
	unsigned int lz = z % region->chunkHeight;
	return region_get_chunk( region, cx, cy, cz )->wall[ lx + ly*region->chunkLength + lz*region->chunkLength*region->chunkWidth ];
}

inline unsigned int region_get_water ( Region *region, int x, int y, int z )
{
	if ( x < 0 || x >= region->chunkLength*region->length || y < 0 || y >= region->chunkWidth*region->width || z < 0 || z >= region->chunkHeight*region->height ) return 0;
	unsigned int cx = x / region->chunkLength;
	unsigned int cy = y / region->chunkWidth;
	unsigned int cz = z / region->chunkHeight;
	unsigned int lx = x % region->chunkLength;
	unsigned int ly = y % region->chunkWidth;
	unsigned int lz = z % region->chunkHeight;
	return region_get_chunk( region, cx, cy, cz )->water[ lx + ly*region->chunkLength + lz*region->chunkLength*region->chunkWidth ];
}

inline void region_set_floor ( Region *region, int x, int y, int z, unsigned int floor )
{
	unsigned int cx = x / region->chunkLength;
	unsigned int cy = y / region->chunkWidth;
	unsigned int cz = z / region->chunkHeight;
	unsigned int lx = x % region->chunkLength;
	unsigned int ly = y % region->chunkWidth;
	unsigned int lz = z % region->chunkHeight;
	region_get_chunk( region, cx, cy, cz )->floor[ lx + ly*region->chunkLength + lz*region->chunkLength*region->chunkWidth ] = floor;
}

inline void region_set_wall ( Region *region, int x, int y, int z, unsigned int wall )
{
	unsigned int cx = x / region->chunkLength;
	unsigned int cy = y / region->chunkWidth;
	unsigned int cz = z / region->chunkHeight;
	unsigned int lx = x % region->chunkLength;
	unsigned int ly = y % region->chunkWidth;
	unsigned int lz = z % region->chunkHeight;
	region_get_chunk( region, cx, cy, cz )->wall[ lx + ly*region->chunkLength + lz*region->chunkLength*region->chunkWidth ] = wall;
}

inline void region_set_water ( Region *region, int x, int y, int z, unsigned int water )
{
	unsigned int cx = x / region->chunkLength;
	unsigned int cy = y / region->chunkWidth;
	unsigned int cz = z / region->chunkHeight;
	unsigned int lx = x % region->chunkLength;
	unsigned int ly = y % region->chunkWidth;
	unsigned int lz = z % region->chunkHeight;
	region_get_chunk( region, cx, cy, cz )->water[ lx + ly*region->chunkLength + lz*region->chunkLength*region->chunkWidth ] = water;
}
