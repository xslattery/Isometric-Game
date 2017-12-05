#ifndef _REGION_HPP_
#define _REGION_HPP_

#include <atomic>
#include <vector>
#include "../../math.hpp"
#include "../../platform/opengl.hpp"
#include "../../platform/platform.h"

// TODO(Xavier): (2017.12.5)
// Change the region class to be a
// data struct with external functions that
// modify it. These functions will be passed the region data.
// Testing should be done to see if this results in a performance
// improvement or hinderence.

struct Chunk_Textured_Mesh
{
	// TODO(Xavier): (2017.11.30)
	// This should be moved to its own file.
	unsigned int vao = 0;
	unsigned int vbo = 0;
	unsigned int ibo = 0;
	unsigned int numIndices = 0;

	~Chunk_Textured_Mesh()
	{
		if ( vao != 0 ) { glDeleteVertexArrays( 1, &vao ); GLCALL; }
		if ( vbo != 0 ) { glDeleteBuffers( 1, &vbo ); GLCALL; }
		if ( ibo != 0 ) { glDeleteBuffers( 1, &ibo ); GLCALL; }
	}
};

enum class Floor
{
	NONE = 0,
	STONE = 1,
};

enum class Wall
{
	NONE = 0,
	STONE = 1,
};

enum class Object
{
	NONE = 0,
};

enum class Direction
{
	NONE = 0,
	N = 1,
	S = 2,
	E = 3,
	W = 4
};

enum class Selection_Type
{
	NONE,
	SINGLE_TILE, 
	MULTI_TILE, 
	ENTITY,
};

enum class Command_Type
{
	NONE,
	GENERATE_REGION_DATA,
	// LOAD_REGION_DATA,
	// SAVE_REGION_DATA,
};

struct Region
{
	///////////////////////////////
	// SIMULATION DATA & METHODS
	struct
	{
		Floor *floor;
		Wall *wall;
		unsigned char *water;
		Object *object;
		Direction *direction;
	} tiles;

	std::vector<vec3> meshesNeedingUpdate_floor;
	std::vector<vec3> meshesNeedingUpdate_wall;
	std::vector<vec3> meshesNeedingUpdate_water;
	std::vector<vec3> meshesNeedingUpdate_object;

	void simulate ();
	void generate ();
	void load ();
	void save ();

	Floor get_floor ( int x, int y, int z );
	Wall get_wall ( int x, int y, int z );
	unsigned char get_water ( int x, int y, int z );
	Object get_object ( int x, int y, int z );
	Direction get_direction ( int x, int y, int z );

	void build_floor_mesh ();
	void build_wall_mesh ();
	void build_water_mesh ();
	void build_object_mesh ();



	///////////////////////////
	// SHARED DATA
	std::atomic<bool> regionDataGenerated;
	std::atomic<bool> simulationPaused;

	struct Chunk_Mesh_Data
	{
		Chunk_Mesh_Data ( vec3 p )
		: position(p), ageIdentifier_floor(0), ageIdentifier_wall(0), ageIdentifier_water(0), ageIdentifier_object(0) {}
		
		vec3 position;

		std::vector<float> floorVertData;
		std::vector<unsigned int> floorIndexData;
		std::size_t ageIdentifier_floor;
		
		std::vector<float> wallVertData;
		std::vector<unsigned int> wallIndexData;
		std::size_t ageIdentifier_wall;
		
		std::vector<float> waterVertData;
		std::vector<unsigned int> waterIndexData;
		std::size_t ageIdentifier_water;

		std::vector<float> objectVertData;
		std::vector<unsigned int> objectIndexData;
		std::size_t ageIdentifier_object;
	};
	std::atomic<bool> simulationUsingUploadQue_1;
	std::atomic<bool> renderingUsingUploadQue_1;
	std::vector<Chunk_Mesh_Data> chunkMeshesToBeUploaded_1;
	std::atomic<bool> simulationUsingUploadQue_2;
	std::atomic<bool> renderingUsingUploadQue_2;
	std::vector<Chunk_Mesh_Data> chunkMeshesToBeUploaded_2;

	struct Command
	{
		// TODO(Xavier): (2017.11.29)
		// This needs more details.
		Command_Type type;
	};
	std::atomic<bool> simulationUsingCommandQue_1;
	std::atomic<bool> renderingUsingCommandQue_1;
	std::vector<Command> commandQue_1;
	std::atomic<bool> simulationUsingCommandQue_2;
	std::atomic<bool> renderingUsingCommandQue_2;
	std::vector<Command> commandQue_2;

	std::atomic<bool> simulationWritingToSelection;
	std::atomic<bool> renderingWritingToSelection;
	std::atomic<bool> renderingReadingFromSelection;
	std::atomic<bool> selectionActive;
	struct
	{
		Selection_Type type;
		struct
		{
			vec3 position;
		} info;
	} selection;

	unsigned int length;
	unsigned int width;
	unsigned int height;
	
	// NOTE(Xavier): (2017.11.30)
	// IF the chunk diemensions do not divide evenly into the regions dimensions
	// then the chunks will extend outside the region boundries. However the extended
	// section of the chunk will be empty.
	float chunk_length;
	float chunk_width;
	float chunk_height;



	//////////////////////////////
	// RENDERING DATA & METHODS
	struct Chunk_Mesh
	{
		Chunk_Mesh ( vec3 p )
		: position(p), ageIdentifier_floor(0), ageIdentifier_wall(0), ageIdentifier_water(0), ageIdentifier_object(0) {}
		vec3 position;
		Chunk_Textured_Mesh floor;
		std::size_t ageIdentifier_floor;
		Chunk_Textured_Mesh wall;
		std::size_t ageIdentifier_wall;
		Chunk_Textured_Mesh water;
		std::size_t ageIdentifier_water;
		Chunk_Textured_Mesh object;
		std::size_t ageIdentifier_object;
	};
	std::vector<Chunk_Mesh> chunkMeshes;

	unsigned int shader;
	unsigned int chunkMeshTexture;
	float projectionScale;
	mat4 projection;
	mat4 camera;

	int viewHeight;

	void render ();
	void issue_command ( Command_Type command );
	void pause_simulation( bool state );
	
		// These methods will build their own selection meshes.
		void select_position ( int x, int y, int z );
		void select_area ( int x, int y, int z );
		void cancel_selection ();
		
	void upload_floor_mesh ( Chunk_Mesh_Data& meshData );
	void upload_wall_mesh ( Chunk_Mesh_Data& meshData );
	void upload_water_mesh ( Chunk_Mesh_Data& meshData );
	void upload_object_mesh ( Chunk_Mesh_Data& meshData );

	void resize( const WindowInfo& window );
	
	// NOTE(Xavier): (2017.11.29)
	// These methods are a special case as they will be 
	// called from the main thread but access data for the simulation thread.
	void init ( const WindowInfo& window, unsigned int l, unsigned int w, unsigned int h );
	void cleanup ();
};

#endif
