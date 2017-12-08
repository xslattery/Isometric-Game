
#include "region.hpp"

static void build_floor_mesh( Region *region, unsigned int chunk );
static void build_wall_mesh( Region *region, unsigned int chunk );
static void build_water_mesh( Region *region, unsigned int chunk );

void region_build_new_meshes ( Region *region )
{
	unsigned int chunkToBeUpdated = 0;
	unsigned int chunkToBeUpdatedInfo = 0;
	
	region->chunksNeedingMeshUpdate_mutex.lock();
	
	for ( unsigned int i = 0; i < region->length*region->width*region->height; ++i )
	{
		if ( region->chunksNeedingMeshUpdate[i] != 0 )
		{
			chunkToBeUpdated = i;
			chunkToBeUpdatedInfo = region->chunksNeedingMeshUpdate[i];
			region->chunksNeedingMeshUpdate[i] = 0;
			break;
		}
	}
	
	region->chunksNeedingMeshUpdate_mutex.unlock();

	if ( chunkToBeUpdatedInfo != 0 )
	{
		if ( chunkToBeUpdatedInfo & Chunk_Mesh_Data_Type::FLOOR ) build_floor_mesh( region, chunkToBeUpdated );
		if ( chunkToBeUpdatedInfo & Chunk_Mesh_Data_Type::WALL ) build_wall_mesh( region, chunkToBeUpdated );
		if ( chunkToBeUpdatedInfo & Chunk_Mesh_Data_Type::WATER ) build_water_mesh( region, chunkToBeUpdated );
	}
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
				pos = ( (xx+ox)*xDir + (yy+oy)*yDir ) * 27;
				zPos = -(xx+ox + yy+oy) + (zz+oz)*2;
			}
			else if ( region->viewDirection == Direction::D_WEST )
			{
				pos = ( (xx+ox)*yDir + (wWidth-1-(yy+oy))*xDir ) * 27;
				zPos = -(xx+ox + wWidth-1-(yy+oy)) + (zz+oz)*2;	
			}
			else if ( region->viewDirection == Direction::D_SOUTH )
			{
				pos = ( (wLength-1-(xx+ox))*xDir + (wWidth-1-(yy+oy))*yDir ) * 27;
				zPos = -(wLength-1-(xx+ox) + wWidth-1-(yy+oy)) + (zz+oz)*2;
			}
			else if ( region->viewDirection == Direction::D_EAST )
			{
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

	if ( verts.size() > 0 )
	{
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
			std::cout << "Couldn't upload.\n";
		}
	}
}

/*if ( meshesNeedingUpdate_floor.size() > 0 )
{
	for ( auto p : meshesNeedingUpdate_floor )
	{
		std::vector<float> verts;
		std::vector<unsigned int> indices;
		std::vector<unsigned int> indexCount;

		const float ox = p.x * chunk_length;
		const float oy = p.y * chunk_width;
		const float oz = p.z * chunk_height;
		const vec2 xDir { -1, 18.0f/27.0f };
		const vec2 yDir {  1, 18.0f/27.0f };

		vec2 tl { 0, 			1.0f-1.0f/512*68*2 };
		vec2 br { 1.0f/512*54, 	1.0f-1.0f/512*68 };

		for ( float zz = 0; zz < chunk_height; ++zz )
		{
			for ( float yy = 0; yy < chunk_width; ++yy )
			{
				for ( float xx = 0; xx < chunk_length; ++xx )
				{
					if ( get_floor(ox+xx, oy+yy, oz+zz) != Floor::NONE )
					{	
						vec2 pos;
						float zPos = 0;
						if ( viewDirection == Direction::N )
						{
							if ( get_floor(ox+xx-1, oy+yy, oz+zz) != Floor::NONE && get_floor(ox+xx, oy+yy-1, oz+zz) != Floor::NONE && get_wall(ox+xx, oy+yy, oz+zz) != Wall::NONE )
								continue;

							pos = ( (xx+ox)*xDir + (yy+oy)*yDir ) * 27;
							zPos = -(xx+ox + yy+oy) + (zz+oz)*2;
						}
						else if ( viewDirection == Direction::W )
						{
							if ( get_floor(ox+xx-1, oy+yy, oz+zz) != Floor::NONE && get_floor(ox+xx, oy+yy+1, oz+zz) != Floor::NONE && get_wall(ox+xx, oy+yy, oz+zz) != Wall::NONE )
								continue;

							pos = ( (xx+ox)*yDir + (width-1-(yy+oy))*xDir ) * 27;
							zPos = -(xx+ox + width-1-(yy+oy)) + (zz+oz)*2;	
						}
						else if ( viewDirection == Direction::S )
						{
							if ( get_floor(ox+xx+1, oy+yy, oz+zz) != Floor::NONE && get_floor(ox+xx, oy+yy+1, oz+zz) != Floor::NONE && get_wall(ox+xx, oy+yy, oz+zz) != Wall::NONE )
								continue;

							pos = ( (length-1-(xx+ox))*xDir + (width-1-(yy+oy))*yDir ) * 27;
							zPos = -(length-1-(xx+ox) + width-1-(yy+oy)) + (zz+oz)*2;
						}
						else if ( viewDirection == Direction::E )
						{
							if ( get_floor(ox+xx+1, oy+yy, oz+zz) != Floor::NONE && get_floor(ox+xx, oy+yy-1, oz+zz) != Floor::NONE && get_wall(ox+xx, oy+yy, oz+zz) != Wall::NONE )
								continue;

							pos = ( (length-1-(xx+ox))*yDir + (yy+oy)*xDir ) * 27;
							zPos = -(length-1-(xx+ox) + (yy+oy)) + (zz+oz)*2;
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
			}
			indexCount.push_back( indices.size() );
		}

		// Pass to shared mesh data ...
		if ( renderingUsingUploadQue_2 == false )
		{
			simulationUsingUploadQue_2 = true;
			if ( renderingUsingUploadQue_2 == false )
			{
				chunkMeshesToBeUploaded_2.emplace_back( p );
				auto& cm = chunkMeshesToBeUploaded_2.back();
				buildAgeIdentifier_floor++;
				cm.ageIdentifier_floor = buildAgeIdentifier_floor;
				cm.floorVertData = std::move( verts );
				cm.floorIndexData = std::move( indices );
				cm.floorIndexCount = std::move( indexCount );
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
				buildAgeIdentifier_floor++;
				cm.ageIdentifier_floor = buildAgeIdentifier_floor;
				cm.floorVertData = std::move( verts );
				cm.floorIndexData = std::move( indices );
				cm.floorIndexCount = std::move( indexCount );
				simulationUsingUploadQue_1 = false;
			} else { simulationUsingUploadQue_1 = false; }
		}
	}
	meshesNeedingUpdate_floor.clear();
}*/

static void build_wall_mesh( Region *region, unsigned int chunk )
{

}

static void build_water_mesh( Region *region, unsigned int chunk )
{

}