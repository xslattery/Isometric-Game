
#ifdef PLATFORM_OSX
#include <mach/mach_time.h>
#endif

#include "region.hpp"


static void build_floor_mesh( Region *region, uint32_t chunk, bool full );
static void build_wall_mesh( Region *region, uint32_t chunk, bool full );
static void build_water_mesh( Region *region, uint32_t chunk, bool full );


//////////////////////////////////
// This function will test to see
// if a chunks mesh needs to be updated.
// If so it will build the mesh.
bool region_build_new_meshes ( Region *region )
{
	bool didWork = false;
	if ( region->chunkDataGenerated ) {
		int chunkToBeUpdated = -1;
		uint32_t chunkToBeUpdatedInfo = 0;
		
		region->chunksNeedingMeshUpdate_mutex.lock();
		
			uint32_t ii = region->generationNextChunk;
			uint32_t counter = 0;
			while ( counter < region->length*region->width*region->height ) {
				
				if ( ii >= region->length*region->width*region->height ) ii = 0;

				if ( region->chunksNeedingMeshUpdate[ii] != 0 ) {

					if ( ii/(region->length*region->width)*region->chunkHeight <= region->viewHeight ) {
						if ( (int)(ii/(region->length*region->width)*region->chunkHeight + region->chunkHeight-1) > (int)region->viewHeight - (int)region->viewDepth ) {

							region->generationNextChunk = ii + 1;
							if ( region->generationNextChunk >= region->length*region->width*region->height ) region->generationNextChunk = 0;

							chunkToBeUpdated = ii;
							chunkToBeUpdatedInfo = region->chunksNeedingMeshUpdate[ii];
							region->chunksNeedingMeshUpdate[ii] = 0;
							break;

						}
					}
				}
				
				ii++;
				counter++;
			}
		
		region->chunksNeedingMeshUpdate_mutex.unlock();

		if ( chunkToBeUpdated != -1 )
		{
			if ( chunkToBeUpdatedInfo & Chunk_Mesh_Data_Type::FLOOR ) build_floor_mesh( region, chunkToBeUpdated, false );
			if ( chunkToBeUpdatedInfo & Chunk_Mesh_Data_Type::WALL ) build_wall_mesh( region, chunkToBeUpdated, false );
			if ( chunkToBeUpdatedInfo & Chunk_Mesh_Data_Type::WATER ) build_water_mesh( region, chunkToBeUpdated, false );

			if ( chunkToBeUpdatedInfo & Chunk_Mesh_Data_Type::FLOOR ) build_floor_mesh( region, chunkToBeUpdated, true );
			if ( chunkToBeUpdatedInfo & Chunk_Mesh_Data_Type::WALL ) build_wall_mesh( region, chunkToBeUpdated, true );
			if ( chunkToBeUpdatedInfo & Chunk_Mesh_Data_Type::WATER ) build_water_mesh( region, chunkToBeUpdated, true );

			didWork = true;
		}
	}

	// TODO(Xavier): (2017.12.29)
	// Abstract this away.
#ifdef PLATFORM_OSX
	mach_timebase_info_data_t timingInfoSimulation;
	if ( mach_timebase_info (&timingInfoSimulation) != KERN_SUCCESS )
		printf ("ERROR: mach_timebase_info failed\n");

	static std::size_t startTime;
	std::size_t endTime = mach_absolute_time();
	std::size_t elapsedTime = endTime - startTime;
	startTime = mach_absolute_time();
	region->generationDeltaTime = (elapsedTime * timingInfoSimulation.numer / timingInfoSimulation.denom) / 1000;
#endif

	return didWork;
}


//////////////////////////////////
// This function builds the chunk
// mesh for the floors.
static void build_floor_mesh( Region *region, uint32_t chunk, bool full )
{
	std::vector<float> verts;
	std::vector<uint32_t> indices;
	std::vector<uint32_t> indexCount;

	const uint32_t cz = chunk/(region->length*region->width);
	const uint32_t temp = chunk - cz * region->length * region->width;
	const uint32_t cy = temp / region->length;
	const uint32_t cx = temp % region->length;
	const float ox = cx * region->chunkLength;
	const float oy = cy * region->chunkWidth;
	const float oz = cz * region->chunkHeight;
	const float wLength = region->length * region->chunkLength;
	const float wWidth = region->width * region->chunkWidth;
	
	const vec2 xDir { -1, 18.0f/27.0f };
	const vec2 yDir {  1, 18.0f/27.0f };

	vec2 tl { 0, 			1.0f-1.0f/512*68*2 };
	vec2 br { 1.0f/512*54, 	1.0f-1.0f/512*68 };

	uint32_t *chunkDataFloor = region->chunks[chunk].floor;

	for ( uint32_t i = 0; i < region->chunkLength*region->chunkWidth*region->chunkHeight; ++i ) {
		if ( i > 0 && i % (region->chunkLength*region->chunkWidth) == 0 )
			indexCount.push_back( indices.size() );

		if ( (chunkDataFloor[i] & 0xFFFFFF) != Floor::FLOOR_NONE ) {
			if ( (chunkDataFloor[i] & OCCLUSION_BIT) != 0 && !full ) continue;

			float zz = i / (region->chunkLength*region->chunkWidth);
			uint32_t iTemp = i - zz * region->chunkLength * region->chunkWidth;
			float yy = iTemp / region->chunkLength;
			float xx = iTemp % region->chunkLength;

			vec2 pos;
			float zPos = 0;

			if ( region->viewDirection == Direction::D_NORTH ) {
				pos = ( (xx+ox)*xDir + (yy+oy)*yDir ) * 27;
				zPos = -(xx+ox + yy+oy) + (zz+oz)*2;
			}
			else if ( region->viewDirection == Direction::D_WEST ) {
				pos = ( (xx+ox)*yDir + (wWidth-1-(yy+oy))*xDir ) * 27;
				zPos = -(xx+ox + wWidth-1-(yy+oy)) + (zz+oz)*2;	
			}
			else if ( region->viewDirection == Direction::D_SOUTH ) {
				pos = ( (wLength-1-(xx+ox))*xDir + (wWidth-1-(yy+oy))*yDir ) * 27;
				zPos = -(wLength-1-(xx+ox) + wWidth-1-(yy+oy)) + (zz+oz)*2;
			}
			else if ( region->viewDirection == Direction::D_EAST ) {
				pos = ( (wLength-1-(xx+ox))*yDir + (yy+oy)*xDir ) * 27;
				zPos = -(wLength-1-(xx+ox) + (yy+oy)) + (zz+oz)*2;
			}

			pos += vec2{ 0, 30 } * (zz+oz);

			uint32_t idxP = verts.size()/5;
			uint32_t tempIndices [6] = {
				idxP+0, idxP+1, idxP+2,
				idxP+2, idxP+1, idxP+3
			};
			indices.insert( indices.end(), tempIndices, tempIndices+6 );

			float tempVerts [20] = {
				-27.0f+pos.x, 68.0f+pos.y, zPos,		tl.x, tl.y,
				-27.0f+pos.x,  0.0f+pos.y, zPos,		tl.x, br.y,
				 27.0f+pos.x, 68.0f+pos.y, zPos,		br.x, tl.y,
				 27.0f+pos.x,  0.0f+pos.y, zPos,		br.x, br.y,
			};
			verts.insert( verts.end(), tempVerts, tempVerts+20 );
		}
	}

	indexCount.push_back( indices.size() );

	if ( region->chunkMeshData_mutex_2.try_lock() ) {
		region->chunkMeshData_2.emplace_back();
		if ( !full ) region->chunkMeshData_2.back().type = Chunk_Mesh_Data_Type::FLOOR;
		else region->chunkMeshData_2.back().type = Chunk_Mesh_Data_Type::FLOOR_FULL;
		region->chunkMeshData_2.back().position = vec3( cx, cy, cz );
		region->chunkMeshData_2.back().age = ++region->ageIncrementerFloor;
		region->chunkMeshData_2.back().vertexData = std::move( verts );
		region->chunkMeshData_2.back().indexData = std::move( indices );
		region->chunkMeshData_2.back().layeredIndexCount = std::move( indexCount );

		region->chunkMeshData_mutex_2.unlock();
	}
	else if ( region->chunkMeshData_mutex_1.try_lock() ) {
		region->chunkMeshData_1.emplace_back();
		if ( !full ) region->chunkMeshData_1.back().type = Chunk_Mesh_Data_Type::FLOOR;
		else region->chunkMeshData_1.back().type = Chunk_Mesh_Data_Type::FLOOR_FULL;
		region->chunkMeshData_1.back().position = vec3( cx, cy, cz );
		region->chunkMeshData_1.back().age = ++region->ageIncrementerFloor;
		region->chunkMeshData_1.back().vertexData = std::move( verts );
		region->chunkMeshData_1.back().indexData = std::move( indices );
		region->chunkMeshData_1.back().layeredIndexCount = std::move( indexCount );

		region->chunkMeshData_mutex_1.unlock();
	}
	else {
		std::cout << "ERROR: Couldn't upload.\n";
	}
}


//////////////////////////////////
// This function builds the chunk
// mesh for walls.
static void build_wall_mesh( Region *region, uint32_t chunk, bool full )
{
	std::vector<float> verts;
	std::vector<uint32_t> indices;
	std::vector<uint32_t> indexCount;

	const uint32_t cz = chunk/(region->length*region->width);
	const uint32_t temp = chunk - cz * region->length * region->width;
	const uint32_t cy = temp / region->length;
	const uint32_t cx = temp % region->length;
	const float ox = cx * region->chunkLength;
	const float oy = cy * region->chunkWidth;
	const float oz = cz * region->chunkHeight;
	const float wLength = region->length * region->chunkLength;
	const float wWidth = region->width * region->chunkWidth;
	
	const vec2 xDir { -1, 18.0f/27.0f };
	const vec2 yDir {  1, 18.0f/27.0f };

	vec2 tl { 0, 			1.0f-1.0f/512*68 };
	vec2 br { 1.0f/512*54, 	1.0f };

	uint32_t *chunkDataWall = region->chunks[chunk].wall;

	for ( uint32_t i = 0; i < region->chunkLength*region->chunkWidth*region->chunkHeight; ++i ) {
		if ( i > 0 && i % (region->chunkLength*region->chunkWidth) == 0 )
			indexCount.push_back( indices.size() );

		if ( (chunkDataWall[i] & 0xFFFFFF) != Wall::WALL_NONE ) {
			if ( (chunkDataWall[i] & OCCLUSION_BIT) != 0 && !full ) continue;

			float zz = i / (region->chunkLength*region->chunkWidth);
			uint32_t iTemp = i - zz * region->chunkLength * region->chunkWidth;
			float yy = iTemp / region->chunkLength;
			float xx = iTemp % region->chunkLength;

			vec2 pos;
			float zPos = 0;

			if ( region->viewDirection == Direction::D_NORTH ) {
				pos = ( (xx+ox)*xDir + (yy+oy)*yDir ) * 27;
				zPos = -(xx+ox + yy+oy) + (zz+oz)*2 + 0.1f;
			}
			else if ( region->viewDirection == Direction::D_WEST ) {
				pos = ( (xx+ox)*yDir + (wWidth-1-(yy+oy))*xDir ) * 27;
				zPos = -(xx+ox + wWidth-1-(yy+oy)) + (zz+oz)*2 + 0.1f;	
			}
			else if ( region->viewDirection == Direction::D_SOUTH ) {
				pos = ( (wLength-1-(xx+ox))*xDir + (wWidth-1-(yy+oy))*yDir ) * 27;
				zPos = -(wLength-1-(xx+ox) + wWidth-1-(yy+oy)) + (zz+oz)*2 + 0.1f;
			}
			else if ( region->viewDirection == Direction::D_EAST ) {
				pos = ( (wLength-1-(xx+ox))*yDir + (yy+oy)*xDir ) * 27;
				zPos = -(wLength-1-(xx+ox) + (yy+oy)) + (zz+oz)*2 + 0.1f;
			}

			pos += vec2{ 0, 30 } * (zz+oz);

			uint32_t idxP = verts.size()/5;
			uint32_t tempIndices [6] = {
				idxP+0, idxP+1, idxP+2,
				idxP+2, idxP+1, idxP+3
			};
			indices.insert( indices.end(), tempIndices, tempIndices+6 );

			float tempVerts [20] = {
				-27.0f+pos.x, 68.0f+pos.y, zPos,		tl.x, tl.y,
				-27.0f+pos.x,  0.0f+pos.y, zPos,		tl.x, br.y,
				 27.0f+pos.x, 68.0f+pos.y, zPos,		br.x, tl.y,
				 27.0f+pos.x,  0.0f+pos.y, zPos,		br.x, br.y,
			};
			verts.insert( verts.end(), tempVerts, tempVerts+20 );
		}
	}

	indexCount.push_back( indices.size() );

	if ( region->chunkMeshData_mutex_2.try_lock() ) {
		region->chunkMeshData_2.emplace_back();
		if ( !full ) region->chunkMeshData_2.back().type = Chunk_Mesh_Data_Type::WALL;
		else region->chunkMeshData_2.back().type = Chunk_Mesh_Data_Type::WALL_FULL;
		region->chunkMeshData_2.back().position = vec3( cx, cy, cz );
		region->chunkMeshData_2.back().age = ++region->ageIncrementerWall;
		region->chunkMeshData_2.back().vertexData = std::move( verts );
		region->chunkMeshData_2.back().indexData = std::move( indices );
		region->chunkMeshData_2.back().layeredIndexCount = std::move( indexCount );

		region->chunkMeshData_mutex_2.unlock();
	}
	else if ( region->chunkMeshData_mutex_1.try_lock() ) {
		region->chunkMeshData_1.emplace_back();
		if ( !full ) region->chunkMeshData_1.back().type = Chunk_Mesh_Data_Type::WALL;
		else region->chunkMeshData_1.back().type = Chunk_Mesh_Data_Type::WALL_FULL;
		region->chunkMeshData_1.back().position = vec3( cx, cy, cz );
		region->chunkMeshData_1.back().age = ++region->ageIncrementerWall;
		region->chunkMeshData_1.back().vertexData = std::move( verts );
		region->chunkMeshData_1.back().indexData = std::move( indices );
		region->chunkMeshData_1.back().layeredIndexCount = std::move( indexCount );

		region->chunkMeshData_mutex_1.unlock();
	}
	else {
		std::cout << "ERROR: Couldn't upload.\n";
	}
}


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
// This function builds the chunk
// mesh for water.
static void build_water_mesh( Region *region, uint32_t chunk, bool full )
{
	std::vector<float> verts;
	std::vector<uint32_t> indices;
	std::vector<uint32_t> indexCount;

	const uint32_t cz = chunk/(region->length*region->width);
	const uint32_t temp = chunk - cz * region->length * region->width;
	const uint32_t cy = temp / region->length;
	const uint32_t cx = temp % region->length;
	const float ox = cx * region->chunkLength;
	const float oy = cy * region->chunkWidth;
	const float oz = cz * region->chunkHeight;
	const float wLength = region->length * region->chunkLength;
	const float wWidth = region->width * region->chunkWidth;
	
	const vec2 xDir { -1, 18.0f/27.0f };
	const vec2 yDir {  1, 18.0f/27.0f };

	const vec2 wtl = { 0, 			1.0f-1.0f/512*68*4 };
	const vec2 wbr = { 1.0f/512*54,	1.0f-1.0f/512*68*3 };

	uint8_t *chunkDataWater = region->chunks[chunk].water;

	for ( uint32_t i = 0; i < region->chunkLength*region->chunkWidth*region->chunkHeight; ++i ) {
		if ( i > 0 && i % (region->chunkLength*region->chunkWidth) == 0 )
			indexCount.push_back( indices.size() );

		if ( (chunkDataWater[i] & 0xFF) > 0 ) {
			float zz = i / (region->chunkLength*region->chunkWidth);
			uint32_t iTemp = i - zz * region->chunkLength * region->chunkWidth;
			float yy = iTemp / region->chunkLength;
			float xx = iTemp % region->chunkLength;

			if ( xx+ox != 0 && xx+ox != region->worldLength-1 && yy+oy != 0 && yy+oy != region->worldWidth-1 && !full ) {
				if ( i + region->chunkLength*region->chunkWidth < region->chunkLength*region->chunkWidth*region->chunkHeight ) {
					if ( chunkDataWater[ i + region->chunkLength*region->chunkWidth ] > 0 ) {
						continue;
					}
				}
				else {
					if ( region_get_water(region, xx+ox, yy+oy, zz+oz+1) > 0 ) {
						continue;
					}
				}
			}

			vec2 tl, br;
			if ( (chunkDataWater[i] & 0xFF) <= 64 ) {
				tl = { 0, 			1.0f-1.0f/512*68*3 };
				br = { 1.0f/512*54,	1.0f-1.0f/512*68*2 };
			}
			else if ( (chunkDataWater[i] & 0xFF) <= 128 ) {
				tl = { 1.0f/512*54, 	1.0f-1.0f/512*68*3 };
				br = { 1.0f/512*54*2,	1.0f-1.0f/512*68*2 };
			}
			else if ( (chunkDataWater[i] & 0xFF) <= 192 ) {
				tl = { 1.0f/512*54*2, 	1.0f-1.0f/512*68*3 };
				br = { 1.0f/512*54*3,	1.0f-1.0f/512*68*2 };
			}
			else if ( (chunkDataWater[i] & 0xFF) <= 255 ) {
				tl = { 1.0f/512*54*3, 	1.0f-1.0f/512*68*3 };
				br = { 1.0f/512*54*4,	1.0f-1.0f/512*68*2 };
			}

			vec2 pos;
			float zPos = 0;

			if ( region->viewDirection == Direction::D_NORTH ) {
				pos = ( (xx+ox)*xDir + (yy+oy)*yDir ) * 27;
				zPos = -(xx+ox + yy+oy) + (zz+oz)*2 + 0.1f;
			}
			else if ( region->viewDirection == Direction::D_WEST ) {
				pos = ( (xx+ox)*yDir + (wWidth-1-(yy+oy))*xDir ) * 27;
				zPos = -(xx+ox + wWidth-1-(yy+oy)) + (zz+oz)*2 + 0.1f;	
			}
			else if ( region->viewDirection == Direction::D_SOUTH ) {
				pos = ( (wLength-1-(xx+ox))*xDir + (wWidth-1-(yy+oy))*yDir ) * 27;
				zPos = -(wLength-1-(xx+ox) + wWidth-1-(yy+oy)) + (zz+oz)*2 + 0.1f;
			}
			else if ( region->viewDirection == Direction::D_EAST ) {
				pos = ( (wLength-1-(xx+ox))*yDir + (yy+oy)*xDir ) * 27;
				zPos = -(wLength-1-(xx+ox) + (yy+oy)) + (zz+oz)*2 + 0.1f;
			}
			pos += vec2{ 0, 30 } * (zz+oz);

			uint32_t idxP = verts.size()/5;
			uint32_t tempIndices [6] = {
				idxP+0, idxP+1, idxP+2,
				idxP+2, idxP+1, idxP+3
			};
			indices.insert( indices.end(), tempIndices, tempIndices+6 );

			float tempVerts [20] = {
				-27.0f+pos.x, 68.0f+pos.y, zPos,		tl.x, tl.y,
				-27.0f+pos.x,  0.0f+pos.y, zPos,		tl.x, br.y,
				 27.0f+pos.x, 68.0f+pos.y, zPos,		br.x, tl.y,
				 27.0f+pos.x,  0.0f+pos.y, zPos,		br.x, br.y,
			};
			verts.insert( verts.end(), tempVerts, tempVerts+20 );

			if ( (region_get_floor(region, xx+ox, yy+oy, zz+oz) & 0xFFFFFF) == Floor::FLOOR_NONE && region_get_water(region, xx+ox, yy+oy, zz+oz-1 ) > 0 ) {
				zPos -= 0.1f;

				uint32_t idxP = verts.size()/5;
				uint32_t tempIndices [6] = {
					idxP+0, idxP+1, idxP+2,
					idxP+2, idxP+1, idxP+3
				};
				indices.insert( indices.end(), tempIndices, tempIndices+6 );

				float tempVerts [20] = {
					-27.0f+pos.x, 68.0f+pos.y, zPos,		wtl.x, wtl.y,
					-27.0f+pos.x,  0.0f+pos.y, zPos,		wtl.x, wbr.y,
					 27.0f+pos.x, 68.0f+pos.y, zPos,		wbr.x, wtl.y,
					 27.0f+pos.x,  0.0f+pos.y, zPos,		wbr.x, wbr.y,
				};
				verts.insert( verts.end(), tempVerts, tempVerts+20 );
			}
		}
	}

	indexCount.push_back( indices.size() );

	if ( region->chunkMeshData_mutex_2.try_lock() ) {
		region->chunkMeshData_2.emplace_back();
		if ( !full ) region->chunkMeshData_2.back().type = Chunk_Mesh_Data_Type::WATER;
		else region->chunkMeshData_2.back().type = Chunk_Mesh_Data_Type::WATER_FULL;
		region->chunkMeshData_2.back().position = vec3( cx, cy, cz );
		region->chunkMeshData_2.back().age = ++region->ageIncrementerWater;
		region->chunkMeshData_2.back().vertexData = std::move( verts );
		region->chunkMeshData_2.back().indexData = std::move( indices );
		region->chunkMeshData_2.back().layeredIndexCount = std::move( indexCount );

		region->chunkMeshData_mutex_2.unlock();
	}
	else if ( region->chunkMeshData_mutex_1.try_lock() ) {
		region->chunkMeshData_1.emplace_back();
		if ( !full ) region->chunkMeshData_1.back().type = Chunk_Mesh_Data_Type::WATER;
		else region->chunkMeshData_1.back().type = Chunk_Mesh_Data_Type::WATER_FULL;
		region->chunkMeshData_1.back().position = vec3( cx, cy, cz );
		region->chunkMeshData_1.back().age = ++region->ageIncrementerWater;
		region->chunkMeshData_1.back().vertexData = std::move( verts );
		region->chunkMeshData_1.back().indexData = std::move( indices );
		region->chunkMeshData_1.back().layeredIndexCount = std::move( indexCount );

		region->chunkMeshData_mutex_1.unlock();
	}
	else {
		std::cout << "ERROR: Couldn't upload.\n";
	}
}
