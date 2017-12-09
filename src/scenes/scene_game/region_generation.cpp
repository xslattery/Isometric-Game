
#include "region.hpp"
#ifdef PLATFORM_OSX
#include <mach/mach_time.h>
#endif

static void build_floor_mesh( Region *region, unsigned int chunk );
static void build_wall_mesh( Region *region, unsigned int chunk );
static void build_water_mesh( Region *region, unsigned int chunk );

bool region_build_new_meshes ( Region *region )
{
	bool didWork = false;
	if ( region->chunkDataGenerated )
	{
		std::vector<unsigned int> chunksToBeUpdated;
		std::vector<unsigned int> chunksToBeUpdatedInfo;
		// unsigned int chunkToBeUpdated = 0;
		// unsigned int chunkToBeUpdatedInfo = 0;
		
		region->chunksNeedingMeshUpdate_mutex.lock();
		
		for ( unsigned int i = region->generationNextChunk; i < region->length*region->width*region->height; ++i )
		{
			if ( region->chunksNeedingMeshUpdate[i] != 0 )
			{
				// region->generationNextChunk = i + 1;
				// if ( region->generationNextChunk >= region->length*region->width*region->height ) region->generationNextChunk = 0;

				chunksToBeUpdated.push_back( i );
				chunksToBeUpdatedInfo.push_back( region->chunksNeedingMeshUpdate[i] );

				// chunkToBeUpdated = i;
				// chunkToBeUpdatedInfo = region->chunksNeedingMeshUpdate[i];
				region->chunksNeedingMeshUpdate[i] = 0;
				// break;
			}
		}
		
		region->chunksNeedingMeshUpdate_mutex.unlock();

		// if ( chunkToBeUpdatedInfo != 0 )
		// {
		// 	if ( chunkToBeUpdatedInfo & Chunk_Mesh_Data_Type::FLOOR ) build_floor_mesh( region, chunkToBeUpdated );
		// 	if ( chunkToBeUpdatedInfo & Chunk_Mesh_Data_Type::WALL ) build_wall_mesh( region, chunkToBeUpdated );
		// 	if ( chunkToBeUpdatedInfo & Chunk_Mesh_Data_Type::WATER ) build_water_mesh( region, chunkToBeUpdated );

		// 	didWork = true;
		// }

		for ( unsigned int i = 0; i < chunksToBeUpdated.size(); ++i )
		{
			if ( chunksToBeUpdatedInfo[i] & Chunk_Mesh_Data_Type::FLOOR ) build_floor_mesh( region, chunksToBeUpdated[i] );
			if ( chunksToBeUpdatedInfo[i] & Chunk_Mesh_Data_Type::WALL ) build_wall_mesh( region, chunksToBeUpdated[i] );
			if ( chunksToBeUpdatedInfo[i] & Chunk_Mesh_Data_Type::WATER ) build_water_mesh( region, chunksToBeUpdated[i] );
		}
		if ( chunksToBeUpdated.size() > 0 ) didWork = true;
	}

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

static void build_floor_mesh( Region *region, unsigned int chunk )
{
	std::vector<float> verts;
	std::vector<unsigned int> indices;
	std::vector<unsigned int> indexCount;

	const unsigned int cz = chunk/(region->length*region->width);
	const unsigned int temp = chunk - cz * region->length * region->width;
	const unsigned int cy = temp / region->length;
	const unsigned int cx = temp % region->length;
	const float ox = cx * region->chunkLength;
	const float oy = cy * region->chunkWidth;
	const float oz = cz * region->chunkHeight;
	const float wLength = region->length * region->chunkLength;
	const float wWidth = region->width * region->chunkWidth;
	
	const vec2 xDir { -1, 18.0f/27.0f };
	const vec2 yDir {  1, 18.0f/27.0f };

	vec2 tl { 0, 			1.0f-1.0f/512*68*2 };
	vec2 br { 1.0f/512*54, 	1.0f-1.0f/512*68 };

	unsigned int *chunkDataFloor = region->chunks[chunk].floor;

	for ( unsigned int i = 0; i < region->chunkLength*region->chunkWidth*region->chunkHeight; ++i )
	{
		if ( i > 0 && i % (region->chunkLength*region->chunkWidth) == 0 )
			indexCount.push_back( indices.size() );

		if ( chunkDataFloor[i] != Floor::FLOOR_NONE )
		{
			float zz = i / (region->chunkLength*region->chunkWidth);
			unsigned int iTemp = i - zz * region->chunkLength * region->chunkWidth;
			float yy = iTemp / region->chunkLength;
			float xx = iTemp % region->chunkLength;

			vec2 pos;
			float zPos = 0;

			if ( region->viewDirection == Direction::D_NORTH )
			{
				if ( (chunkDataFloor[i] & Occlusion::N_HIDDEN) != 0 ) continue;

				pos = ( (xx+ox)*xDir + (yy+oy)*yDir ) * 27;
				zPos = -(xx+ox + yy+oy) + (zz+oz)*2;
			}
			else if ( region->viewDirection == Direction::D_WEST )
			{
				if ( (chunkDataFloor[i] & Occlusion::W_HIDDEN) != 0 ) continue;

				pos = ( (xx+ox)*yDir + (wWidth-1-(yy+oy))*xDir ) * 27;
				zPos = -(xx+ox + wWidth-1-(yy+oy)) + (zz+oz)*2;	
			}
			else if ( region->viewDirection == Direction::D_SOUTH )
			{
				if ( (chunkDataFloor[i] & Occlusion::S_HIDDEN) != 0 ) continue;

				pos = ( (wLength-1-(xx+ox))*xDir + (wWidth-1-(yy+oy))*yDir ) * 27;
				zPos = -(wLength-1-(xx+ox) + wWidth-1-(yy+oy)) + (zz+oz)*2;
			}
			else if ( region->viewDirection == Direction::D_EAST )
			{
				if ( (chunkDataFloor[i] & Occlusion::E_HIDDEN) != 0 ) continue;

				pos = ( (wLength-1-(xx+ox))*yDir + (yy+oy)*xDir ) * 27;
				zPos = -(wLength-1-(xx+ox) + (yy+oy)) + (zz+oz)*2;
			}
			pos += vec2{ 0, 30 } * (zz+oz);

			unsigned int idxP = verts.size()/5;
			unsigned int tempIndices [6] =
			{
				idxP+0, idxP+1, idxP+2,
				idxP+2, idxP+1, idxP+3
			};
			indices.insert( indices.end(), tempIndices, tempIndices+6 );

			float tempVerts [20] =
			{
				-27.0f+pos.x, 68.0f+pos.y, zPos,		tl.x, tl.y,
				-27.0f+pos.x,  0.0f+pos.y, zPos,		tl.x, br.y,
				 27.0f+pos.x, 68.0f+pos.y, zPos,		br.x, tl.y,
				 27.0f+pos.x,  0.0f+pos.y, zPos,		br.x, br.y,
			};
			verts.insert( verts.end(), tempVerts, tempVerts+20 );
		}
	}

	indexCount.push_back( indices.size() );

	if ( region->chunkMeshData_mutex_2.try_lock() )
	{
		region->chunkMeshData_2.emplace_back();
		region->chunkMeshData_2.back().type = Chunk_Mesh_Data_Type::FLOOR;
		region->chunkMeshData_2.back().position = vec3( cx, cy, cz );
		region->chunkMeshData_2.back().age = ++region->ageIncrementerFloor;
		region->chunkMeshData_2.back().vertexData = std::move( verts );
		region->chunkMeshData_2.back().indexData = std::move( indices );
		region->chunkMeshData_2.back().layeredIndexCount = std::move( indexCount );

		region->chunkMeshData_mutex_2.unlock();
	}
	else if ( region->chunkMeshData_mutex_1.try_lock() )
	{
		region->chunkMeshData_1.emplace_back();
		region->chunkMeshData_1.back().type = Chunk_Mesh_Data_Type::FLOOR;
		region->chunkMeshData_1.back().position = vec3( cx, cy, cz );
		region->chunkMeshData_1.back().age = ++region->ageIncrementerFloor;
		region->chunkMeshData_1.back().vertexData = std::move( verts );
		region->chunkMeshData_1.back().indexData = std::move( indices );
		region->chunkMeshData_1.back().layeredIndexCount = std::move( indexCount );

		region->chunkMeshData_mutex_1.unlock();
	}
	else
	{
		std::cout << "ERROR: Couldn't upload.\n";
	}
}



static void build_wall_mesh( Region *region, unsigned int chunk )
{
	std::vector<float> verts;
	std::vector<unsigned int> indices;
	std::vector<unsigned int> indexCount;

	const unsigned int cz = chunk/(region->length*region->width);
	const unsigned int temp = chunk - cz * region->length * region->width;
	const unsigned int cy = temp / region->length;
	const unsigned int cx = temp % region->length;
	const float ox = cx * region->chunkLength;
	const float oy = cy * region->chunkWidth;
	const float oz = cz * region->chunkHeight;
	const float wLength = region->length * region->chunkLength;
	const float wWidth = region->width * region->chunkWidth;
	
	const vec2 xDir { -1, 18.0f/27.0f };
	const vec2 yDir {  1, 18.0f/27.0f };

	vec2 tl { 0, 			1.0f-1.0f/512*68 };
	vec2 br { 1.0f/512*54, 	1.0f };

	unsigned int *chunkDataWall = region->chunks[chunk].wall;

	for ( unsigned int i = 0; i < region->chunkLength*region->chunkWidth*region->chunkHeight; ++i )
	{
		if ( i > 0 && i % (region->chunkLength*region->chunkWidth) == 0 )
			indexCount.push_back( indices.size() );

		if ( chunkDataWall[i] != Wall::WALL_NONE )
		{
			float zz = i / (region->chunkLength*region->chunkWidth);
			unsigned int iTemp = i - zz * region->chunkLength * region->chunkWidth;
			float yy = iTemp / region->chunkLength;
			float xx = iTemp % region->chunkLength;

			vec2 pos;
			float zPos = 0;

			if ( region->viewDirection == Direction::D_NORTH )
			{
				if ( (chunkDataWall[i] & Occlusion::N_HIDDEN) != 0 ) continue;

				pos = ( (xx+ox)*xDir + (yy+oy)*yDir ) * 27;
				zPos = -(xx+ox + yy+oy) + (zz+oz)*2 + 0.1f;
			}
			else if ( region->viewDirection == Direction::D_WEST )
			{
				if ( (chunkDataWall[i] & Occlusion::W_HIDDEN) != 0 ) continue;

				pos = ( (xx+ox)*yDir + (wWidth-1-(yy+oy))*xDir ) * 27;
				zPos = -(xx+ox + wWidth-1-(yy+oy)) + (zz+oz)*2 + 0.1f;	
			}
			else if ( region->viewDirection == Direction::D_SOUTH )
			{
				if ( (chunkDataWall[i] & Occlusion::S_HIDDEN) != 0 ) continue;

				pos = ( (wLength-1-(xx+ox))*xDir + (wWidth-1-(yy+oy))*yDir ) * 27;
				zPos = -(wLength-1-(xx+ox) + wWidth-1-(yy+oy)) + (zz+oz)*2 + 0.1f;
			}
			else if ( region->viewDirection == Direction::D_EAST )
			{
				if ( (chunkDataWall[i] & Occlusion::E_HIDDEN) != 0 ) continue;

				pos = ( (wLength-1-(xx+ox))*yDir + (yy+oy)*xDir ) * 27;
				zPos = -(wLength-1-(xx+ox) + (yy+oy)) + (zz+oz)*2 + 0.1f;
			}
			pos += vec2{ 0, 30 } * (zz+oz);

			unsigned int idxP = verts.size()/5;
			unsigned int tempIndices [6] =
			{
				idxP+0, idxP+1, idxP+2,
				idxP+2, idxP+1, idxP+3
			};
			indices.insert( indices.end(), tempIndices, tempIndices+6 );

			float tempVerts [20] =
			{
				-27.0f+pos.x, 68.0f+pos.y, zPos,		tl.x, tl.y,
				-27.0f+pos.x,  0.0f+pos.y, zPos,		tl.x, br.y,
				 27.0f+pos.x, 68.0f+pos.y, zPos,		br.x, tl.y,
				 27.0f+pos.x,  0.0f+pos.y, zPos,		br.x, br.y,
			};
			verts.insert( verts.end(), tempVerts, tempVerts+20 );
		}
	}

	indexCount.push_back( indices.size() );

	if ( region->chunkMeshData_mutex_2.try_lock() )
	{
		region->chunkMeshData_2.emplace_back();
		region->chunkMeshData_2.back().type = Chunk_Mesh_Data_Type::WALL;
		region->chunkMeshData_2.back().position = vec3( cx, cy, cz );
		region->chunkMeshData_2.back().age = ++region->ageIncrementerWall;
		region->chunkMeshData_2.back().vertexData = std::move( verts );
		region->chunkMeshData_2.back().indexData = std::move( indices );
		region->chunkMeshData_2.back().layeredIndexCount = std::move( indexCount );

		region->chunkMeshData_mutex_2.unlock();
	}
	else if ( region->chunkMeshData_mutex_1.try_lock() )
	{
		region->chunkMeshData_1.emplace_back();
		region->chunkMeshData_1.back().type = Chunk_Mesh_Data_Type::WALL;
		region->chunkMeshData_1.back().position = vec3( cx, cy, cz );
		region->chunkMeshData_1.back().age = ++region->ageIncrementerWall;
		region->chunkMeshData_1.back().vertexData = std::move( verts );
		region->chunkMeshData_1.back().indexData = std::move( indices );
		region->chunkMeshData_1.back().layeredIndexCount = std::move( indexCount );

		region->chunkMeshData_mutex_1.unlock();
	}
	else
	{
		std::cout << "ERROR: Couldn't upload.\n";
	}
}



static void build_water_mesh( Region *region, unsigned int chunk )
{
	std::vector<float> verts;
	std::vector<unsigned int> indices;
	std::vector<unsigned int> indexCount;

	const unsigned int cz = chunk/(region->length*region->width);
	const unsigned int temp = chunk - cz * region->length * region->width;
	const unsigned int cy = temp / region->length;
	const unsigned int cx = temp % region->length;
	const float ox = cx * region->chunkLength;
	const float oy = cy * region->chunkWidth;
	const float oz = cz * region->chunkHeight;
	const float wLength = region->length * region->chunkLength;
	const float wWidth = region->width * region->chunkWidth;
	
	const vec2 xDir { -1, 18.0f/27.0f };
	const vec2 yDir {  1, 18.0f/27.0f };

	unsigned int *chunkDataWater = region->chunks[chunk].water;

	for ( unsigned int i = 0; i < region->chunkLength*region->chunkWidth*region->chunkHeight; ++i )
	{
		if ( i > 0 && i % (region->chunkLength*region->chunkWidth) == 0 )
			indexCount.push_back( indices.size() );

		if ( (chunkDataWater[i] & 0xFFFF) > 0 )
		{
			vec2 tl, br;
			if ( (chunkDataWater[i] & 0xFFFF) == 1 )
			{
				tl = { 0, 			1.0f-1.0f/512*68*3 };
				br = { 1.0f/512*54,	1.0f-1.0f/512*68*2 };
			}
			else if ( (chunkDataWater[i] & 0xFFFF) == 2 )
			{
				tl = { 1.0f/512*54, 	1.0f-1.0f/512*68*3 };
				br = { 1.0f/512*54*2,	1.0f-1.0f/512*68*2 };
			}
			else if ( (chunkDataWater[i] & 0xFFFF) == 3 )
			{
				tl = { 1.0f/512*54*2, 	1.0f-1.0f/512*68*3 };
				br = { 1.0f/512*54*3,	1.0f-1.0f/512*68*2 };
			}
			else if ( (chunkDataWater[i] & 0xFFFF) == 4 )
			{
				tl = { 1.0f/512*54*3, 	1.0f-1.0f/512*68*3 };
				br = { 1.0f/512*54*4,	1.0f-1.0f/512*68*2 };
			}

			float zz = i / (region->chunkLength*region->chunkWidth);
			unsigned int iTemp = i - zz * region->chunkLength * region->chunkWidth;
			float yy = iTemp / region->chunkLength;
			float xx = iTemp % region->chunkLength;

			vec2 pos;
			float zPos = 0;

			if ( region->viewDirection == Direction::D_NORTH )
			{
				if ( (chunkDataWater[i] & Occlusion::N_HIDDEN) != 0 ) continue;

				pos = ( (xx+ox)*xDir + (yy+oy)*yDir ) * 27;
				zPos = -(xx+ox + yy+oy) + (zz+oz)*2 + 0.1f;
			}
			else if ( region->viewDirection == Direction::D_WEST )
			{
				if ( (chunkDataWater[i] & Occlusion::W_HIDDEN) != 0 ) continue;

				pos = ( (xx+ox)*yDir + (wWidth-1-(yy+oy))*xDir ) * 27;
				zPos = -(xx+ox + wWidth-1-(yy+oy)) + (zz+oz)*2 + 0.1f;	
			}
			else if ( region->viewDirection == Direction::D_SOUTH )
			{
				if ( (chunkDataWater[i] & Occlusion::S_HIDDEN) != 0 ) continue;

				pos = ( (wLength-1-(xx+ox))*xDir + (wWidth-1-(yy+oy))*yDir ) * 27;
				zPos = -(wLength-1-(xx+ox) + wWidth-1-(yy+oy)) + (zz+oz)*2 + 0.1f;
			}
			else if ( region->viewDirection == Direction::D_EAST )
			{
				if ( (chunkDataWater[i] & Occlusion::E_HIDDEN) != 0 ) continue;

				pos = ( (wLength-1-(xx+ox))*yDir + (yy+oy)*xDir ) * 27;
				zPos = -(wLength-1-(xx+ox) + (yy+oy)) + (zz+oz)*2 + 0.1f;
			}
			pos += vec2{ 0, 30 } * (zz+oz);

			unsigned int idxP = verts.size()/5;
			unsigned int tempIndices [6] =
			{
				idxP+0, idxP+1, idxP+2,
				idxP+2, idxP+1, idxP+3
			};
			indices.insert( indices.end(), tempIndices, tempIndices+6 );

			float tempVerts [20] =
			{
				-27.0f+pos.x, 68.0f+pos.y, zPos,		tl.x, tl.y,
				-27.0f+pos.x,  0.0f+pos.y, zPos,		tl.x, br.y,
				 27.0f+pos.x, 68.0f+pos.y, zPos,		br.x, tl.y,
				 27.0f+pos.x,  0.0f+pos.y, zPos,		br.x, br.y,
			};
			verts.insert( verts.end(), tempVerts, tempVerts+20 );
		}
	}

	indexCount.push_back( indices.size() );

	if ( region->chunkMeshData_mutex_2.try_lock() )
	{
		region->chunkMeshData_2.emplace_back();
		region->chunkMeshData_2.back().type = Chunk_Mesh_Data_Type::WATER;
		region->chunkMeshData_2.back().position = vec3( cx, cy, cz );
		region->chunkMeshData_2.back().age = ++region->ageIncrementerWater;
		region->chunkMeshData_2.back().vertexData = std::move( verts );
		region->chunkMeshData_2.back().indexData = std::move( indices );
		region->chunkMeshData_2.back().layeredIndexCount = std::move( indexCount );

		region->chunkMeshData_mutex_2.unlock();
	}
	else if ( region->chunkMeshData_mutex_1.try_lock() )
	{
		region->chunkMeshData_1.emplace_back();
		region->chunkMeshData_1.back().type = Chunk_Mesh_Data_Type::WATER;
		region->chunkMeshData_1.back().position = vec3( cx, cy, cz );
		region->chunkMeshData_1.back().age = ++region->ageIncrementerWater;
		region->chunkMeshData_1.back().vertexData = std::move( verts );
		region->chunkMeshData_1.back().indexData = std::move( indices );
		region->chunkMeshData_1.back().layeredIndexCount = std::move( indexCount );

		region->chunkMeshData_mutex_1.unlock();
	}
	else
	{
		std::cout << "ERROR: Couldn't upload.\n";
	}
}

/*

std::vector<float> verts;
std::vector<unsigned int> indices;
std::vector<unsigned int> indexCount;

// verts.reserve( chunk_length*chunk_width*chunk_height * 20 );
// indices.reserve( chunk_length*chunk_width*chunk_height * 6 );
// indexCount.reserve( chunk_height );

const float ox = p.x * chunk_length;
const float oy = p.y * chunk_width;
const float oz = p.z * chunk_height;
const vec2 xDir { -1, 18.0f/27.0f };
const vec2 yDir {  1, 18.0f/27.0f };

for ( float zz = 0; zz < chunk_height; ++zz )
{
	for ( float yy = 0; yy < chunk_width; ++yy )
	{
		for ( float xx = 0; xx < chunk_length; ++xx )
		{
			auto waterLevel = get_water(ox+xx, oy+yy, oz+zz);
			if ( waterLevel > 0 && waterLevel < 5 )
			{
				vec2 tl, br;
				if ( waterLevel == 1 )
				{
					tl = { 0, 			1.0f-1.0f/512*68*3 };
					br = { 1.0f/512*54,	1.0f-1.0f/512*68*2 };
				}
				else if ( waterLevel == 2 )
				{
					tl = { 1.0f/512*54, 	1.0f-1.0f/512*68*3 };
					br = { 1.0f/512*54*2,	1.0f-1.0f/512*68*2 };
				}
				else if ( waterLevel == 3 )
				{
					tl = { 1.0f/512*54*2, 	1.0f-1.0f/512*68*3 };
					br = { 1.0f/512*54*3,	1.0f-1.0f/512*68*2 };
				}
				else if ( waterLevel == 4 )
				{
					tl = { 1.0f/512*54*3, 	1.0f-1.0f/512*68*3 };
					br = { 1.0f/512*54*4,	1.0f-1.0f/512*68*2 };
				}

				// TODO(Xavier): (2017.12.7)
				// For correctness checking against sdjacent water also needs to be done.

				vec2 pos;
				float zPos = 0;
				if ( viewDirection == Direction::N )
				{
					if ( get_wall(ox+xx-1, oy+yy, oz+zz) != Wall::NONE && get_wall(ox+xx, oy+yy-1, oz+zz) != Wall::NONE && get_floor(ox+xx, oy+yy, oz+zz+1) != Floor::NONE )
						continue;

					pos = ( (xx+ox)*xDir + (yy+oy)*yDir ) * 27;
					zPos = -(xx+ox + yy+oy) + (zz+oz)*2 + 0.1f;
				}
				else if ( viewDirection == Direction::W )
				{
					if ( get_wall(ox+xx-1, oy+yy, oz+zz) != Wall::NONE && get_wall(ox+xx, oy+yy+1, oz+zz) != Wall::NONE && get_floor(ox+xx, oy+yy, oz+zz+1) != Floor::NONE )
						continue;

					pos = ( (xx+ox)*yDir + (width-1-(yy+oy))*xDir ) * 27;
					zPos = -(xx+ox + width-1-(yy+oy)) + (zz+oz)*2 + 0.1f;
				}
				else if ( viewDirection == Direction::S )
				{
					if ( get_wall(ox+xx+1, oy+yy, oz+zz) != Wall::NONE && get_wall(ox+xx, oy+yy+1, oz+zz) != Wall::NONE && get_floor(ox+xx, oy+yy, oz+zz+1) != Floor::NONE )
						continue;

					pos = ( (length-1-(xx+ox))*xDir + (width-1-(yy+oy))*yDir ) * 27;
					zPos = -(length-1-(xx+ox) + width-1-(yy+oy)) + (zz+oz)*2 + 0.1f;
				}
				else if ( viewDirection == Direction::E )
				{
					if ( get_wall(ox+xx+1, oy+yy, oz+zz) != Wall::NONE && get_wall(ox+xx, oy+yy-1, oz+zz) != Wall::NONE && get_floor(ox+xx, oy+yy, oz+zz+1) != Floor::NONE )
						continue;

					pos = ( (length-1-(xx+ox))*yDir + (yy+oy)*xDir ) * 27;
					zPos = -(length-1-(xx+ox) + (yy+oy)) + (zz+oz)*2 + 0.1f;
				}
				pos += vec2{ 0, 30 } * (zz+oz);

				unsigned int idxP = verts.size()/5;
				unsigned int tempIndices [6] =
				{
					idxP+0, idxP+1, idxP+2,
					idxP+2, idxP+1, idxP+3
				};
				indices.insert( indices.end(), tempIndices, tempIndices+6 );

				float tempVerts [20] =
				{
					-27.0f+pos.x, 68.0f+pos.y, zPos,		tl.x, tl.y,
					-27.0f+pos.x,  0.0f+pos.y, zPos,		tl.x, br.y,
					 27.0f+pos.x, 68.0f+pos.y, zPos,		br.x, tl.y,
					 27.0f+pos.x,  0.0f+pos.y, zPos,		br.x, br.y,
				};
				verts.insert( verts.end(), tempVerts, tempVerts+20 );

				// TODO(Xavier): (2017.12.7)
				// For correctness testing against adjacent floors needs to be done.
				if ( get_floor( ox+xx, oy+yy, oz+zz ) == Floor::NONE )
				{
					tl = { 0, 			1.0f-1.0f/512*68*4 };
					br = { 1.0f/512*54,	1.0f-1.0f/512*68*3 };

					unsigned int idxP = verts.size()/5;
					unsigned int tempIndices [6] =
					{
						idxP+0, idxP+1, idxP+2,
						idxP+2, idxP+1, idxP+3
					};
					indices.insert( indices.end(), tempIndices, tempIndices+6 );

					float tempVerts [20] =
					{
						-27.0f+pos.x, 68.0f+pos.y, zPos,		tl.x, tl.y,
						-27.0f+pos.x,  0.0f+pos.y, zPos,		tl.x, br.y,
						 27.0f+pos.x, 68.0f+pos.y, zPos,		br.x, tl.y,
						 27.0f+pos.x,  0.0f+pos.y, zPos,		br.x, br.y,
					};
					verts.insert( verts.end(), tempVerts, tempVerts+20 );
				}
			}
		}
	}
	indexCount.push_back( indices.size() );
}

// verts.shrink_to_fit();
// indices.shrink_to_fit();
// indexCount.shrink_to_fit();

// Pass to shared mesh data ...
if ( renderingUsingUploadQue_2 == false )
{
	simulationUsingUploadQue_2 = true;
	if ( renderingUsingUploadQue_2 == false )
	{
		chunkMeshesToBeUploaded_2.emplace_back( p );
		auto& cm = chunkMeshesToBeUploaded_2.back();
		buildAgeIdentifier_water++;
		cm.ageIdentifier_water = buildAgeIdentifier_water;
		cm.waterVertData = std::move( verts );
		cm.waterIndexData = std::move( indices );
		cm.waterIndexCount = std::move( indexCount );
		simulationUsingUploadQue_2 = false;
	} else { simulationUsingUploadQue_2 = false; }
}
else if ( renderingUsingUploadQue_1 == false )
{
	simulationUsingUploadQue_1 = true;
	if ( renderingUsingUploadQue_1 == false )
	{
		chunkMeshesToBeUploaded_1.emplace_back( p );
		auto& cm = chunkMeshesToBeUploaded_1.back();
		buildAgeIdentifier_water++;
		cm.ageIdentifier_water = buildAgeIdentifier_water;
		cm.waterVertData = std::move( verts );
		cm.waterIndexData = std::move( indices );
		cm.waterIndexCount = std::move( indexCount );
		simulationUsingUploadQue_1 = false;
	} else { simulationUsingUploadQue_1 = false; }
}

*/