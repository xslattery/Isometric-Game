
#include "region.hpp"
#include "../shader.hpp"

/////////////////////////
// SIMULATION THREAD:
void Region::simulate()
{
	// build_floor_mesh();
	build_wall_mesh();
	// build_water_mesh();
	// build_object_mesh();
}

// TODO(Xavier): (2017.11.29)
// These need to be properly implemented.
Floor Region::get_floor ( int x, int y, int z ) { return Floor::NONE; }
Wall Region::get_wall ( int x, int y, int z ) { return Wall::NONE; }
unsigned char Region::get_water ( int x, int y, int z ) { return 0; }
Object Region::get_object ( int x, int y, int z ) { return Object::NONE; }
Direction Region::get_direction ( int x, int y, int z ) { return Direction::NONE; }

void Region::build_floor_mesh()
{
	for ( auto p : meshesNeedingUpdate_floor )
	{
	}
	meshesNeedingUpdate_floor.clear();
}

void Region::build_wall_mesh()
{
	for ( auto p : meshesNeedingUpdate_wall )
	{
	}
	meshesNeedingUpdate_wall.clear();
}

void Region::build_water_mesh()
{
	for ( auto p : meshesNeedingUpdate_water )
	{
	}
	meshesNeedingUpdate_water.clear();
}

void Region::build_object_mesh()
{
	for ( auto p : meshesNeedingUpdate_object )
	{
	}
	meshesNeedingUpdate_object.clear();
}


///////////////////////////
// SHARED:
void Region::init( unsigned int l, unsigned int w, unsigned int h )
{
	length = l;
	width = w;
	height = h;

	simulationUsingUploadQue_1 = false;
	renderingUsingUploadQue_1 = false;
	simulationUsingUploadQue_2 = false;
	renderingUsingUploadQue_2 = false;
	simulationUsingCommandQue_1 = false;
	renderingUsingCommandQue_1 = false;
	simulationUsingCommandQue_2 = false;
	renderingUsingCommandQue_2 = false;
	simulationWritingToSelection = false;
	renderingWritingToSelection = false;
	renderingReadingFromSelection = false;
	selectionActive = false;
	
	selection.type = Selection_Type::NONE;
	selection.info.position = vec3(-1); // Set to unused value.
	
	shader = 0;
	// TODO(Xavier): (2017.11.29) Load the shader.
	// shader = LoadShader( /* Shader source */);
}

void Region::generate()
{
	tiles.floor = static_cast<Floor*>(malloc(length*width*height * sizeof(Floor)));
	tiles.wall = static_cast<Wall*>(malloc(length*width*height * sizeof(Wall)));
	tiles.water = static_cast<unsigned char*>(malloc(length*width*height * sizeof(unsigned char)));
	tiles.object = static_cast<Object*>(malloc(length*width*height * sizeof(Object)));
	tiles.direction = static_cast<Direction*>(malloc(length*width*height * sizeof(Direction)));

	for ( unsigned int i = 0; i < height; ++i )
	{
		for ( unsigned int j = 0; j < width; ++j )
		{
			for ( unsigned int k = 0; k < length; ++k )
			{
				tiles.floor[ i*width*length + j*length + k ] = Floor::NONE;
				tiles.wall[ i*width*length + j*length + k ] = Wall::NONE;
				tiles.water[ i*width*length + j*length + k ] = 0;
				tiles.object[ i*width*length + j*length + k ] = Object::NONE;
				tiles.direction[ i*width*length + j*length + k ] = Direction::NONE;
			}	
		}
	}

	// meshesNeedingUpdate_floor.push_back( vec3(0) );
	meshesNeedingUpdate_wall.push_back( vec3(0) );
	// meshesNeedingUpdate_water.push_back( vec3(0) );
	// meshesNeedingUpdate_object.push_back( vec3(0) );

	// build_floor_mesh();
	build_wall_mesh();
	// build_water_mesh();
	// build_object_mesh();
}

// TODO(Xavier): (2017.11.29)
// Loading and saving needs to be implemeneted.
void Region::load() { }
void Region::save() { }

void Region::cleanup()
{
	free( tiles.floor );
	free( tiles.wall );
	free( tiles.water );
	free( tiles.object );
	free( tiles.direction );
}


//////////////////////////////
// RENDERING THREAD:
void Region::render()
{
	// NOTE(Xavier): (2017.11.29)
	// I thought of a potential issue here.
	// If the simulation if faster then the rendering then
	// it is possiable that an new mesh will be uploaded
	// then it is replaced by an old one.
	// to FIX this try using a counter to store the mesh build count
	// if it is less then the current one the mesh will not be uploaded
	// other wise the counter will be set to the new value.

	if ( simulationUsingUploadQue_1 == false )
	{
		renderingUsingUploadQue_1 = true;
		if ( simulationUsingUploadQue_1 == false )
		{
			if ( chunkMeshesToBeUploaded_1.size() > 0 )
			{
				for ( auto& m : chunkMeshesToBeUploaded_1 )
				{
					if ( m.floorData.size() > 0 ) upload_floor_mesh( m );
					if ( m.wallData.size() > 0 ) upload_wall_mesh( m );
					if ( m.waterData.size() > 0 ) upload_water_mesh( m );
					if ( m.objectData.size() > 0 ) upload_object_mesh( m );
				}
				chunkMeshesToBeUploaded_1.clear();
			}
			renderingUsingUploadQue_1 = false;
		} else { renderingUsingUploadQue_1 = false; }
	}

	if ( simulationUsingUploadQue_2 == false )
	{
		renderingUsingUploadQue_2 = true;
		if ( simulationUsingUploadQue_2 == false )
		{
			if ( chunkMeshesToBeUploaded_2.size() > 0 )
			{
				for ( auto& m : chunkMeshesToBeUploaded_2 )
				{
					if ( m.floorData.size() > 0 ) upload_floor_mesh( m );
					if ( m.wallData.size() > 0 ) upload_wall_mesh( m );
					if ( m.waterData.size() > 0 ) upload_water_mesh( m );
					if ( m.objectData.size() > 0 ) upload_object_mesh( m );
				}
				chunkMeshesToBeUploaded_2.clear();
			}
			renderingUsingUploadQue_2 = false;
		} else { renderingUsingUploadQue_2 = false; }
	}
}

void Region::issue_command()
{
}

void Region::select_position( int x, int y, int z )
{
}

void Region::select_area( int x, int y, int z )
{
}

void Region::cancel_selection()
{
}

void Region::upload_floor_mesh( Chunk_Mesh_Data& meshData )
{
	// 1. Find the correct chunk mesh to update based on the position of the meshData
	// 2. Check the age of the data againt the current age.
	// 3. using opengl calls upload the meshData to the graphics card.
	// 4. store the vao, vbo & ibo to the chunkMesh.
}

void Region::upload_wall_mesh( Chunk_Mesh_Data& meshData )
{
	// 1. Find the correct chunk mesh to update based on the position of the meshData
	// 2. Check the age of the data againt the current age.
	// 3. using opengl calls upload the meshData to the graphics card.
	// 4. store the vao, vbo & ibo to the chunkMesh.
}

void Region::upload_water_mesh( Chunk_Mesh_Data& meshData )
{
	// 1. Find the correct chunk mesh to update based on the position of the meshData
	// 2. Check the age of the data againt the current age.
	// 3. using opengl calls upload the meshData to the graphics card.
	// 4. store the vao, vbo & ibo to the chunkMesh.
}

void Region::upload_object_mesh( Chunk_Mesh_Data& meshData )
{
	// 1. Find the correct chunk mesh to update based on the position of the meshData
	// 2. Check the age of the data againt the current age.
	// 3. using opengl calls upload the meshData to the graphics card.
	// 4. store the vao, vbo & ibo to the chunkMesh.
}
