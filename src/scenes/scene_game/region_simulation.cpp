
#include "../../math/perlin.hpp"
#ifdef PLATFORM_OSX
#include <mach/mach_time.h>
#endif

#include "region.hpp"


static void process_commands ( Region *region );
static float generate_height_data ( float xx, float yy, float scale, int octaves, float persistance, float lacunarity, bool power );
static void simulate_water ( Region *region, std::vector<uint32_t> &newChunksThatNeedUpdate );

// HELPER FUNCTIONS:
inline Chunk_Data* region_get_chunk ( Region *region, int x, int y, int z );
inline uint32_t region_get_floor ( Region *region, int x, int y, int z );
inline uint32_t region_get_wall ( Region *region, int x, int y, int z );
inline uint32_t region_get_water ( Region *region, int x, int y, int z );
inline void region_set_floor ( Region *region, int x, int y, int z, uint32_t floor );
inline void region_set_wall ( Region *region, int x, int y, int z, uint32_t wall );
inline void region_set_water ( Region *region, int x, int y, int z, uint32_t water );


//////////////////////////////////
// This function is responsiable 
// for simulating the region.
void region_simulate ( Region *region )
{
	process_commands( region );

	if ( region->chunkDataGenerated ) {
		if ( !region->simulationPaused ) {	

			std::vector<uint32_t> newChunksThatNeedUpdate( region->length*region->width*region->height, 0 );
			
			simulate_water( region, newChunksThatNeedUpdate );

			region->chunksNeedingMeshUpdate_mutex.lock();

			for ( uint32_t i = 0; i < newChunksThatNeedUpdate.size(); ++i ) {
				region->chunksNeedingMeshUpdate[ i ] |= newChunksThatNeedUpdate[ i ];
			}

			region->chunksNeedingMeshUpdate_mutex.unlock();
		}
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


//////////////////////////////////
// This function handles the inter-
// thread communication and executes
// commands recieved form the main thread.
static void process_commands ( Region *region )
{
	auto execute_command = [&]( Region_Command& command ) {
		switch ( command.type ) {
			case Region_Command_Type::GENERATE_DATA:
				region_generate( region );
				break;

			case Region_Command_Type::ROTATE_LEFT:
				region->viewDirection++;
				if ( region->viewDirection > 4 ) region->viewDirection = 1;
				region->chunksNeedingMeshUpdate_mutex.lock();
				for ( uint32_t i = 0; i < region->length*region->width*region->height; ++i ) {
					region->chunksNeedingMeshUpdate[i] = Chunk_Mesh_Data_Type::FLOOR | Chunk_Mesh_Data_Type::WALL | Chunk_Mesh_Data_Type::WATER;
				}
				region->chunksNeedingMeshUpdate_mutex.unlock();
				break;

			case Region_Command_Type::ROTATE_RIGHT:
				region->viewDirection--;
				if ( region->viewDirection < 1 ) region->viewDirection = 4;	
				region->chunksNeedingMeshUpdate_mutex.lock();
				for ( uint32_t i = 0; i < region->length*region->width*region->height; ++i ) {
					region->chunksNeedingMeshUpdate[i] = Chunk_Mesh_Data_Type::FLOOR | Chunk_Mesh_Data_Type::WALL | Chunk_Mesh_Data_Type::WATER;
				}
				region->chunksNeedingMeshUpdate_mutex.unlock();
				break;

			case Region_Command_Type::ADD_WATER_WAVE:
				for ( int y = 0; y < region->width; ++y ) {
					for ( int x = 0; x < region->length; ++x ) {
						Chunk_Data *chunk = region_get_chunk( region, x, y, region->height-1 );
						for ( int cy = 0; cy < region->chunkWidth; ++cy ) {
							for ( int cx = 0; cx < region->chunkLength; ++cx ) {
								uint8_t *water = &chunk->water[ cx + cy*region->chunkLength + (region->chunkHeight-1)*region->chunkLength*region->chunkWidth ];
								*water = 255; //rand()%256;
								if ( *water > 0 ) region->waterThatNeedsUpdate.emplace_back( cx+(x*region->chunkLength), cy+(y*region->chunkWidth), (region->chunkHeight-1)+((region->height-1)*region->chunkHeight), x + y*region->length + (region->height-1)*region->length*region->width );
							}
						}
					}
				}
				break;

			default: break;
		}
	};

	if ( region->commandQue_mutex_1.try_lock() ) {
		for ( auto& command : region->commandQue_1 ) {
			execute_command( command );
		}
		region->commandQue_1.clear();
		region->commandQue_mutex_1.unlock();
	}
	
	if ( region->commandQue_mutex_2.try_lock() ) {
		for ( auto& command : region->commandQue_2 ) {
			execute_command( command );
		}
		region->commandQue_2.clear();
		region->commandQue_mutex_2.unlock();
	}
}


//////////////////////////////////
// This function simulated the
// the water for a single time-step
// in the region.
static void simulate_water ( Region *region, std::vector<uint32_t> &newChunksThatNeedUpdate )
{
	if ( region->waterThatNeedsUpdate.size() > 0 ) {
		std::vector<vec4> newWaterThatNeedsUpdate;

		static uint32_t lowestWater = 0;
		static uint32_t highestWater = region->worldLength*region->worldWidth*region->worldHeight;
		for ( uint32_t i = lowestWater; i < highestWater; ++i ) {
			region->updatedWaterBitset[ i ] = false;
		}
		
		uint32_t newLowestWater = region->worldLength*region->worldWidth*region->worldHeight;
		uint32_t newHighestWater = 0;

		for ( auto p : region->waterThatNeedsUpdate ) {
			uint32_t bitPos = p.x + p.y*region->worldLength + p.z*region->worldLength*region->worldWidth;
			if ( region->updatedWaterBitset[ bitPos ] == true ) continue;
			region->updatedWaterBitset[ bitPos ] = true;
			if ( bitPos < newLowestWater ) newLowestWater = bitPos;
			if ( bitPos+1 > newHighestWater ) newHighestWater = bitPos+1;

			int sameDepth = region_get_water( region, p.x, p.y, p.z ) & 0xFF;
			if ( sameDepth == 0 ) continue;

			if ( region_get_floor(region, p.x, p.y, p.z) == Floor::FLOOR_NONE && region_get_wall(region, p.x, p.y, p.z-1) == Wall::WALL_NONE && region_get_water(region, p.x, p.y, p.z-1) < 255 ) {
				int belowDepth = region_get_water( region, p.x, p.y, p.z-1 ) & 0xFF;
				
				belowDepth += sameDepth;
				sameDepth = belowDepth - 255;
				if ( sameDepth < 0 ) sameDepth = 0;
				belowDepth -= sameDepth;

				region_set_water( region, p.x, p.y, p.z, sameDepth );
				region_set_water( region, p.x, p.y, p.z-1, belowDepth );

				uint32_t cx = p.x / region->chunkLength;
				uint32_t cy = p.y / region->chunkWidth;
				uint32_t cz = (p.z-1) / region->chunkHeight;
				uint32_t newChunkIndex = cx + cy*region->length + cz*region->length*region->width;
				newChunksThatNeedUpdate[ newChunkIndex ] |= Chunk_Mesh_Data_Type::WATER;
				newChunksThatNeedUpdate[ p.w ] |= Chunk_Mesh_Data_Type::WATER;
				
				if ( sameDepth > 0 ) newWaterThatNeedsUpdate.emplace_back( p );
				newWaterThatNeedsUpdate.emplace_back( p.x, p.y, p.z-1, newChunkIndex );
			}

			if ( sameDepth > 0 ) {
				int sides = 1;
				int xpw = region_get_wall(region, p.x+1, p.y, p.z ); if ( p.x+1 == region->worldLength ) xpw = Wall::WALL_STONE;
				int xnw = region_get_wall(region, p.x-1, p.y, p.z ); if ( p.x-1 < 0 ) xnw = Wall::WALL_STONE;
				int ypw = region_get_wall(region, p.x, p.y+1, p.z ); if ( p.y+1 == region->worldWidth ) ypw = Wall::WALL_STONE;
				int ynw = region_get_wall(region, p.x, p.y-1, p.z ); if ( p.y-1 < 0) ynw = Wall::WALL_STONE;
				if ( xpw == Wall::WALL_NONE ) sides++;
				if ( xnw == Wall::WALL_NONE ) sides++;
				if ( ypw == Wall::WALL_NONE ) sides++;
				if ( ynw == Wall::WALL_NONE ) sides++;
				int xp = region_get_water( region, p.x+1, p.y, p.z ) & 0xFF;
				int xn = region_get_water( region, p.x-1, p.y, p.z ) & 0xFF;
				int yp = region_get_water( region, p.x, p.y+1, p.z ) & 0xFF;
				int yn = region_get_water( region, p.x, p.y-1, p.z ) & 0xFF;
				int average = (sameDepth + xp + xn + yp + yn) / sides;

				if ( average == sameDepth ) continue;

				if ( sides > 1 ) {
					if ( (region_get_water(region, p.x, p.y, p.z+1 ) & 0xFF) > 0 ) {
						uint32_t cx = p.x / region->chunkLength;
						uint32_t cy = p.y / region->chunkWidth;
						uint32_t cz = (p.z+1) / region->chunkHeight;
						uint32_t newChunkIndex = cx + cy*region->length + cz*region->length*region->width;
						newChunksThatNeedUpdate[ newChunkIndex ] |= Chunk_Mesh_Data_Type::WATER;
						newWaterThatNeedsUpdate.emplace_back( p.x, p.y, p.z+1, newChunkIndex );
					}
				}
				
				int mod = (sameDepth + xp + xn + yp + yn) % sides;

				// NOTE(Xavier): This is to help the system get into a steady state.
				if ( xp && xn && yp && yn && mod != 0 && rand()%100 == 1 ) mod = 0;
				else if ( mod != 0 && rand()%500 == 1 ) mod = 0;

				int modxp = 0;
				int modxn = 0;
				int modyp = 0;
				int modyn = 0;

				switch ( mod ) {
					case 1:
						switch ( sides ) {
							case 2:
								if ( !xpw ) modxp = 1;
								if ( !xnw ) modxn = 1;
								if ( !ypw ) modyp = 1;
								if ( !ynw ) modyn = 1;
								break;
							case 3:
								if ( !xpw && !xnw ) {
									switch (rand()%2) {
										case 0: modxp = 1; break;
										case 1: modxn = 1; break;
									}
								}
								else if ( !xpw && !ynw ) {
									switch (rand()%2) {
										case 0: modxp = 1; break;
										case 1: modyn = 1; break;
									}
								}
								else if ( !xpw && !ypw ) {
									switch (rand()%2) {
										case 0: modxp = 1; break;
										case 1: modyp = 1; break;
									}
								}
								else if ( !xnw && !ypw ) {
									switch (rand()%2) {
										case 0: modxn = 1; break;
										case 1: modyp = 1; break;
									}
								}
								else if ( !xnw && !ynw ) {
									switch (rand()%2) {
										case 0: modxn = 1; break;
										case 1: modyn = 1; break;
									}
								}
								else if ( !ynw && !ypw ) {
									switch (rand()%2) {
										case 0: modyn = 1; break;
										case 1: modyp = 1; break;
									}
								}
								break;
							case 4:
								if ( !xpw && !xnw && !ypw ) {
									switch (rand()%3) {
										case 0: modxp = 0; modxn = 0; modyp = 1; break;
										case 1: modxp = 1; modxn = 0; modyp = 0; break;
										case 2: modxp = 0; modxn = 1; modyp = 0; break;
									}
								}
								else if ( !xpw && !xnw && !ynw ) {
									switch (rand()%3) {
										case 0: modxp = 0; modxn = 0; modyn = 1; break;
										case 1: modxp = 1; modxn = 0; modyn = 0; break;
										case 2: modxp = 0; modxn = 1; modyn = 0; break;
									}
								}
								else if ( !xpw && !ynw && !ypw ) {
									switch (rand()%3) {
										case 0: modxp = 0; modyn = 0; modyp = 1; break;
										case 1: modxp = 1; modyn = 0; modyp = 0; break;
										case 2: modxp = 0; modyn = 1; modyp = 0; break;
									}
								}
								else if ( !ynw && !xnw && !ypw ) {
									switch (rand()%3) {
										case 0: modyn = 0; modxn = 0; modyp = 1; break;
										case 1: modyn = 1; modxn = 0; modyp = 0; break;
										case 2: modyn = 0; modxn = 1; modyp = 0; break;
									}
								}
								break;
							case 5:
								switch (rand()%4) {
									case 0: modxp = 1; break;
									case 1: modxn = 1; break;
									case 2: modyp = 1; break;
									case 3: modyn = 1; break;
								}
								break;
						}
						break;
					case 2:
						switch ( sides ) {
							case 3:
								if ( !xpw && !xnw ) modxp = 1; modxn = 1;
								if ( !xpw && !ynw ) modxp = 1; modyn = 1;
								if ( !xpw && !ypw ) modxp = 1; modyp = 1;
								if ( !xnw && !ypw ) modxn = 1; modyp = 1;
								if ( !xnw && !ynw ) modxn = 1; modyn = 1;
								if ( !ynw && !ypw ) modyn = 1; modyp = 1;
								break;
							case 4:
								if ( !xpw && !xnw && !ypw ) {
									switch (rand()%3) {
										case 0: modxp = 0; modxn = 1; modyp = 1; break;
										case 1: modxp = 1; modxn = 0; modyp = 1; break;
										case 2: modxp = 1; modxn = 1; modyp = 0; break;
									}
								}
								if ( !xpw && !xnw && !ynw ) {
									switch (rand()%3) {
										case 0: modxp = 0; modxn = 1; modyn = 1; break;
										case 1: modxp = 1; modxn = 0; modyn = 1; break;
										case 2: modxp = 1; modxn = 1; modyn = 0; break;
									}
								}
								if ( !xpw && !ynw && !ypw ) {
									switch (rand()%3) {
										case 0: modxp = 0; modyn = 1; modyp = 1; break;
										case 1: modxp = 1; modyn = 0; modyp = 1; break;
										case 2: modxp = 1; modyn = 1; modyp = 0; break;
									}
								}
								if ( !ynw && !xnw && !ypw ) {
									switch (rand()%3) {
										case 0: modyn = 0; modxn = 1; modyp = 1; break;
										case 1: modyn = 1; modxn = 0; modyp = 1; break;
										case 2: modyn = 1; modxn = 1; modyp = 0; break;
									}
								}
								break;
							case 5:
								switch (rand()%2) {
									case 0: modxp = 1; modxn = 1; break;
									case 1: modyp = 1; modyn = 1; break;
								}
								break;
						}
						break;
					case 3:
						if ( sides == 4 ) {
							if ( !xpw && !xnw && !ypw ) modxp = 1; modxn = 1; modyp = 1;
							if ( !xpw && !xnw && !ynw ) modxp = 1; modxn = 1; modyn = 1;
							if ( !xpw && !ynw && !ypw ) modxp = 1; modyn = 1; modyp = 1;
							if ( !ynw && !xnw && !ypw ) modyn = 1; modxn = 1; modyp = 1;
						}
						else if ( sides == 5 ) {
							switch (rand()%4) {
								case 0: modxp = 1; modxn = 1; modyp = 1; break;
								case 1: modxp = 1; modxn = 1; modyn = 1; break;
								case 2: modyp = 1; modxp = 1; modyp = 1; break;
								case 3: modyn = 1; modyp = 1; modxn = 1; break;
							}
						}
						break;
					case 4:
						modxp = 1;
						modxn = 1;
						modyp = 1;
						modyn = 1;
						break;
				}

				if ( xpw == Wall::WALL_NONE ) {
					region_set_water( region, p.x+1, p.y, p.z, average + modxp );

					uint32_t cx = (p.x+1) / region->chunkLength;
					uint32_t cy = p.y / region->chunkWidth;
					uint32_t cz = p.z / region->chunkHeight;
					uint32_t newChunkIndex = cx + cy*region->length + cz*region->length*region->width;
					newChunksThatNeedUpdate[ newChunkIndex ] |= Chunk_Mesh_Data_Type::WATER;
					newWaterThatNeedsUpdate.emplace_back( p.x+1, p.y, p.z, newChunkIndex );
				}

				if ( xnw == Wall::WALL_NONE ) {
					region_set_water( region, p.x-1, p.y, p.z, average + modxn );

					uint32_t cx = (p.x-1) / region->chunkLength;
					uint32_t cy = p.y / region->chunkWidth;
					uint32_t cz = p.z / region->chunkHeight;
					uint32_t newChunkIndex = cx + cy*region->length + cz*region->length*region->width;
					newChunksThatNeedUpdate[ newChunkIndex ] |= Chunk_Mesh_Data_Type::WATER;
					newWaterThatNeedsUpdate.emplace_back( p.x-1, p.y, p.z, newChunkIndex );
				}

				if ( ypw == Wall::WALL_NONE ) {
					region_set_water( region, p.x, p.y+1, p.z, average + modyp );

					uint32_t cx = p.x / region->chunkLength;
					uint32_t cy = (p.y+1) / region->chunkWidth;
					uint32_t cz = p.z / region->chunkHeight;
					uint32_t newChunkIndex = cx + cy*region->length + cz*region->length*region->width;
					newChunksThatNeedUpdate[ newChunkIndex ] |= Chunk_Mesh_Data_Type::WATER;
					newWaterThatNeedsUpdate.emplace_back( p.x, p.y+1, p.z, newChunkIndex );
				}

				if ( ynw == Wall::WALL_NONE ) {
					region_set_water( region, p.x, p.y-1, p.z, average + modyn );

					uint32_t cx = p.x / region->chunkLength;
					uint32_t cy = (p.y-1) / region->chunkWidth;
					uint32_t cz = p.z / region->chunkHeight;
					uint32_t newChunkIndex = cx + cy*region->length + cz*region->length*region->width;
					newChunksThatNeedUpdate[ newChunkIndex ] |= Chunk_Mesh_Data_Type::WATER;
					newWaterThatNeedsUpdate.emplace_back( p.x, p.y-1, p.z, newChunkIndex );
				}

				region_set_water( region, p.x, p.y, p.z, average );
				newChunksThatNeedUpdate[ p.w ] |= Chunk_Mesh_Data_Type::WATER;
			}
		}

		lowestWater = newLowestWater;
		highestWater = newHighestWater;

		region->waterThatNeedsUpdate = std::move( newWaterThatNeedsUpdate );
		region->numberOfWaterBeingUpdated = region->waterThatNeedsUpdate.size();
	}
}


//////////////////////////////////
// This function generates the 
// height data using perlin noise.
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

	for ( int i = 0; i < octaves; ++i ) {
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


//////////////////////////////////
// This function generates the 
// regions data.
void region_generate ( Region *region )
{
	// Generate Data:
	for ( int z = 0; z < region->height; ++z ) {
		for ( int y = 0; y < region->width; ++y ) {
			for ( int x = 0; x < region->length; ++x ) {
				Chunk_Data *chunk = region_get_chunk( region, x, y, z );

				for ( int cz = 0; cz < region->chunkHeight; ++cz ) {
					for ( int cy = 0; cy < region->chunkWidth; ++cy ) {
						for ( int cx = 0; cx < region->chunkLength; ++cx ) {
							uint32_t *floor = &chunk->floor[ cx + cy*region->chunkLength + cz*region->chunkLength*region->chunkWidth ];
							uint32_t *wall = &chunk->wall[ cx + cy*region->chunkLength + cz*region->chunkLength*region->chunkWidth ];
							uint8_t *water = &chunk->water[ cx + cy*region->chunkLength + cz*region->chunkLength*region->chunkWidth ];
							
							*floor = Floor::FLOOR_NONE;
							*wall = Wall::WALL_NONE;
							*water = 0;

							int genHeight = static_cast<int>(generate_height_data( cx+(x*region->chunkLength), cy+(y*region->chunkWidth), 350, 4, 0.5f, 2.5f, 1 ) * 25.0f ) + 64;
							if ( genHeight > cz+(z*region->chunkHeight) ) *floor = Floor::FLOOR_STONE;
							if ( genHeight-1 > cz+(z*region->chunkHeight) ) *wall = Wall::WALL_STONE;
							
							if ( cz+(z*region->chunkHeight) == region->worldHeight-1 ) *water = 255; //rand()%256;
							if ( *water > 0 )region->waterThatNeedsUpdate.emplace_back( cx+(x*region->chunkLength), cy+(y*region->chunkWidth), cz+(z*region->chunkHeight), x + y*region->length + z*region->length*region->width );
						}
					}
				}
			}
		}
	}

	// Generate Occlusion:
	for ( int z = 0; z < region->height; ++z ) {
		for ( int y = 0; y < region->width; ++y ) {
			for ( int x = 0; x < region->length; ++x ) {
				Chunk_Data *chunk = region_get_chunk( region, x, y, z );

				for ( int cz = 0; cz < region->chunkHeight; ++cz ) {
					for ( int cy = 0; cy < region->chunkWidth; ++cy ) {
						for ( int cx = 0; cx < region->chunkLength; ++cx ) {
							uint32_t* floor = &chunk->floor[ cx + cy*region->chunkLength + cz*region->chunkLength*region->chunkWidth ];
							uint32_t* wall = &chunk->wall[ cx + cy*region->chunkLength + cz*region->chunkLength*region->chunkWidth ];
							
							int xx = cx+(x*region->chunkLength);
							int yy = cy+(y*region->chunkWidth);
							int zz = cz+(z*region->chunkHeight);

							// TODO(Xavier): (2017.12.25)
							// Cleanup this occlusion code.
							if ( *floor != Floor::FLOOR_NONE ) {
								if ( region_get_floor(region, xx-1, yy, zz) != Floor::FLOOR_NONE && region_get_floor(region, xx, yy-1, zz) != Floor::FLOOR_NONE && region_get_wall(region, xx, yy, zz) != Wall::WALL_NONE ) {
									if ( region_get_floor(region, xx+1, yy, zz) != Floor::FLOOR_NONE && region_get_floor(region, xx, yy-1, zz) != Floor::FLOOR_NONE && region_get_wall(region, xx, yy, zz) != Wall::WALL_NONE ) {
										if ( region_get_floor(region, xx+1, yy, zz) != Floor::FLOOR_NONE && region_get_floor(region, xx, yy+1, zz) != Floor::FLOOR_NONE && region_get_wall(region, xx, yy, zz) != Wall::WALL_NONE ) {
											if ( region_get_floor(region, xx-1, yy, zz) != Floor::FLOOR_NONE && region_get_floor(region, xx, yy+1, zz) != Floor::FLOOR_NONE && region_get_wall(region, xx, yy, zz) != Wall::WALL_NONE ) {
												*floor |= Occlusion::N_HIDDEN;
												*floor |= Occlusion::E_HIDDEN;
												*floor |= Occlusion::S_HIDDEN;
												*floor |= Occlusion::W_HIDDEN;
											}
										}
									}
								}
							}

							if ( *wall != Wall::WALL_NONE ) {
								if ( region_get_wall(region, xx-1, yy, zz) != Wall::WALL_NONE && region_get_wall(region, xx, yy-1, zz) != Wall::WALL_NONE && region_get_floor(region, xx, yy, zz+1) != Floor::FLOOR_NONE ) {
									if ( region_get_wall(region, xx+1, yy, zz) != Wall::WALL_NONE && region_get_wall(region, xx, yy-1, zz) != Wall::WALL_NONE && region_get_floor(region, xx, yy, zz+1) != Floor::FLOOR_NONE ) {
										if ( region_get_wall(region, xx+1, yy, zz) != Wall::WALL_NONE && region_get_wall(region, xx, yy+1, zz) != Wall::WALL_NONE && region_get_floor(region, xx, yy, zz+1) != Floor::FLOOR_NONE ) {
											if ( region_get_wall(region, xx-1, yy, zz) != Wall::WALL_NONE && region_get_wall(region, xx, yy+1, zz) != Wall::WALL_NONE && region_get_floor(region, xx, yy, zz+1) != Floor::FLOOR_NONE ) {
												*wall |= Occlusion::N_HIDDEN;
												*wall |= Occlusion::E_HIDDEN;
												*wall |= Occlusion::S_HIDDEN;
												*wall |= Occlusion::W_HIDDEN;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	region->chunksNeedingMeshUpdate_mutex.lock();
	
	for ( uint32_t i = 0; i < region->length*region->width*region->height; ++i ) {
		region->chunksNeedingMeshUpdate[i] = Chunk_Mesh_Data_Type::FLOOR | Chunk_Mesh_Data_Type::WALL | Chunk_Mesh_Data_Type::WATER;
	}
	
	region->chunksNeedingMeshUpdate_mutex.unlock();

	region->chunkDataGenerated = true;
}


//////////////////////////////////
// This function saves the region
// to file.
void region_save ( Region *region )
{
	// TODO(Xavier): (2017.12.7)
	// Implement this function.
}


//////////////////////////////////
// This function loads the region
// from file.
void region_load ( Region *region )
{
	// TODO(Xavier): (2017.12.7)
	// Implement this function.

	region->chunkDataGenerated = true;
}




//////////////////////
// HELPER FUNCTIONS //
//////////////////////


//////////////////////////////////
// This fucntion returns the chunk
// at the specified loaction.
inline Chunk_Data* region_get_chunk ( Region *region, int x, int y, int z )
{
	if ( x < 0 || x >= region->length || y < 0 || y >= region->width || z < 0 || z >= region->height ) return nullptr;
	return &region->chunks[ x + y*region->length + z*region->length*region->width ];
}

//////////////////////////////////
// This function returns the value
// of a floor at a location.
inline uint32_t region_get_floor ( Region *region, int x, int y, int z )
{
	if ( x < 0 || x >= region->worldLength || y < 0 || y >= region->worldWidth || z < 0 || z >= region->worldHeight ) return 0;
	uint32_t cx = x / region->chunkLength;
	uint32_t cy = y / region->chunkWidth;
	uint32_t cz = z / region->chunkHeight;
	uint32_t lx = x % region->chunkLength;
	uint32_t ly = y % region->chunkWidth;
	uint32_t lz = z % region->chunkHeight;
	return region_get_chunk( region, cx, cy, cz )->floor[ lx + ly*region->chunkLength + lz*region->chunkLength*region->chunkWidth ];
}

//////////////////////////////////
// This function returns the value
// of a wall at a loaction.
inline uint32_t region_get_wall ( Region *region, int x, int y, int z )
{
	if ( x < 0 || x >= region->worldLength || y < 0 || y >= region->worldWidth || z < 0 || z >= region->worldHeight ) return 0;
	uint32_t cx = x / region->chunkLength;
	uint32_t cy = y / region->chunkWidth;
	uint32_t cz = z / region->chunkHeight;
	uint32_t lx = x % region->chunkLength;
	uint32_t ly = y % region->chunkWidth;
	uint32_t lz = z % region->chunkHeight;
	return region_get_chunk( region, cx, cy, cz )->wall[ lx + ly*region->chunkLength + lz*region->chunkLength*region->chunkWidth ];
}

//////////////////////////////////
// This function returns the value
// of water at a loaction.
inline uint32_t region_get_water ( Region *region, int x, int y, int z )
{
	if ( x < 0 || x >= region->worldLength || y < 0 || y >= region->worldWidth || z < 0 || z >= region->worldHeight ) return 0;
	uint32_t cx = x / region->chunkLength;
	uint32_t cy = y / region->chunkWidth;
	uint32_t cz = z / region->chunkHeight;
	uint32_t lx = x % region->chunkLength;
	uint32_t ly = y % region->chunkWidth;
	uint32_t lz = z % region->chunkHeight;
	return region_get_chunk( region, cx, cy, cz )->water[ lx + ly*region->chunkLength + lz*region->chunkLength*region->chunkWidth ];
}

//////////////////////////////////
// This function sets the value
// of a floor at a loaction.
inline void region_set_floor ( Region *region, int x, int y, int z, uint32_t floor )
{
	if ( x < 0 || x >= region->worldLength || y < 0 || y >= region->worldWidth || z < 0 || z >= region->worldHeight ) return;
	uint32_t cx = x / region->chunkLength;
	uint32_t cy = y / region->chunkWidth;
	uint32_t cz = z / region->chunkHeight;
	uint32_t lx = x % region->chunkLength;
	uint32_t ly = y % region->chunkWidth;
	uint32_t lz = z % region->chunkHeight;
	region_get_chunk( region, cx, cy, cz )->floor[ lx + ly*region->chunkLength + lz*region->chunkLength*region->chunkWidth ] = floor;
}

//////////////////////////////////
// This function sets the value
// of a wall at a loaction.
inline void region_set_wall ( Region *region, int x, int y, int z, uint32_t wall )
{
	if ( x < 0 || x >= region->worldLength || y < 0 || y >= region->worldWidth || z < 0 || z >= region->worldHeight ) return;
	uint32_t cx = x / region->chunkLength;
	uint32_t cy = y / region->chunkWidth;
	uint32_t cz = z / region->chunkHeight;
	uint32_t lx = x % region->chunkLength;
	uint32_t ly = y % region->chunkWidth;
	uint32_t lz = z % region->chunkHeight;
	region_get_chunk( region, cx, cy, cz )->wall[ lx + ly*region->chunkLength + lz*region->chunkLength*region->chunkWidth ] = wall;
}

//////////////////////////////////
// This function sets the value
// of water at a loaction.
inline void region_set_water ( Region *region, int x, int y, int z, uint32_t water )
{
	if ( x < 0 || x >= region->worldLength || y < 0 || y >= region->worldWidth || z < 0 || z >= region->worldHeight ) return;
	uint32_t cx = x / region->chunkLength;
	uint32_t cy = y / region->chunkWidth;
	uint32_t cz = z / region->chunkHeight;
	uint32_t lx = x % region->chunkLength;
	uint32_t ly = y % region->chunkWidth;
	uint32_t lz = z % region->chunkHeight;
	region_get_chunk( region, cx, cy, cz )->water[ lx + ly*region->chunkLength + lz*region->chunkLength*region->chunkWidth ] = water;
}