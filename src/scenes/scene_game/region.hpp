#ifndef __REGION_HPP_
#define __REGION_HPP_

#include <vector>
#include <mutex>

#include "../../platform/platform.h"
#include "../../platform/opengl.hpp"
#include "../../math.hpp"

enum Occlusion
{
	N_HIDDEN = 0x1 << 31,
	E_HIDDEN = 0x1 << 30,
	S_HIDDEN = 0x1 << 29,
	W_HIDDEN = 0x1 << 28,
};

enum Direction
{
	D_NONE = 0,
	D_NORTH = 1,
	D_EAST = 2,
	D_SOUTH = 3,
	D_WEST = 4,
};

enum Floor
{
	FLOOR_NONE = 0,
	FLOOR_STONE = 1,
};

enum Wall
{
	WALL_NONE = 0,
	WALL_STONE = 1,
};

enum Region_Command_Type
{
	GENERATE_DATA = 1,
	ROTATE_RIGHT = 2,
	ROTATE_LEFT = 3,
};

struct Region_Command
{
	unsigned int type;
};

struct Chunk_Data
{
	unsigned int *floor = nullptr;
	unsigned int floorBegin, floorEnd;
	unsigned int floorNonHiddenBegin, floorNonHiddenEnd;

	unsigned int *wall = nullptr;
	unsigned int wallBegin, wallEnd;
	unsigned int wallNonHiddenBegin, wallNonHiddenEnd;

	unsigned int *water = nullptr;
	unsigned int waterBegin, waterEnd;
	unsigned int waterNonHiddenBegin, waterNonHiddenEnd;
};

enum Chunk_Mesh_Data_Type
{
	FLOOR = 0x1 << 0,
	WALL = 0x1 << 1,
	WATER = 0x1 << 2,
};

struct Chunk_Mesh_Data
{
	unsigned int type;
	vec3 position;
	std::size_t age;
	std::vector<float> vertexData;
	std::vector<unsigned int> indexData;
	std::vector<unsigned int> layeredIndexCount;
};

struct Chunk_Mesh
{
	struct Sub_Mesh
	{
		unsigned int vao = 0;
		unsigned int vbo = 0;
		unsigned int ibo = 0;
		unsigned int indexCount = 0;
		std::vector<unsigned int> layeredIndexCount;

		~Sub_Mesh()
		{
			if ( vao != 0 ) { glDeleteVertexArrays( 1, &vao ); GLCALL; }
			if ( vbo != 0 ) { glDeleteBuffers( 1, &vbo ); GLCALL; }
			if ( ibo != 0 ) { glDeleteBuffers( 1, &ibo ); GLCALL; }
		}
	};

	std::size_t floorMeshAge = 0;
	Sub_Mesh floorMesh;

	std::size_t wallMeshAge = 0;
	Sub_Mesh wallMesh;

	std::size_t waterMeshAge = 0;
	Sub_Mesh waterMesh;
};

struct Region
{
	// SIMULATION THREAD:
	Chunk_Data *chunks = nullptr;
	unsigned int length, width, height;
	unsigned int chunkLength, chunkWidth, chunkHeight;

	std::mutex chunksNeedingMeshUpdate_mutex;
	unsigned int *chunksNeedingMeshUpdate = nullptr;

	std::atomic<unsigned int> ageIncrementerFloor;
	std::atomic<unsigned int> ageIncrementerWall;
	std::atomic<unsigned int> ageIncrementerWater;

	// MAIN & SIMULATION THREADS:
	std::atomic_bool simulationPaused;
	std::atomic<unsigned int> simulationDeltaTime;
	std::atomic<unsigned int> generationDeltaTime;
	std::atomic_bool chunkDataGenerated;

	std::atomic<unsigned int> viewDirection;

	std::mutex chunkMeshData_mutex_1;
	std::vector<Chunk_Mesh_Data> chunkMeshData_1;
	std::mutex chunkMeshData_mutex_2;
	std::vector<Chunk_Mesh_Data> chunkMeshData_2;

	std::mutex commandQue_mutex_1;
	std::vector<Region_Command> commandQue_1;
	std::mutex commandQue_mutex_2;
	std::vector<Region_Command> commandQue_2;

	// MAIN THREAD:
	Chunk_Mesh* chunkMeshes;

	unsigned int shader;
	unsigned int chunkMeshTexture;
	float projectionScale;
	mat4 projection;
	mat4 camera;
	int viewHeight;
};

///////////////////
// EXTRA THREADS:
bool region_build_new_meshes ( Region *region );

///////////////////////
// SIMULATION THREAD:
void region_generate ( Region *region );
void region_save ( Region *region );
void region_load ( Region *region );
void region_simulate ( Region *region );

/////////////////
// MAIN THREAD:
void region_init ( const WindowInfo& window, Region *region, unsigned int cl, unsigned int cw, unsigned int ch, unsigned int wl, unsigned int ww, unsigned int wh );
void region_cleanup ( Region *region );
void region_render ( const WindowInfo& window, Region *region );
void region_resize_viewport ( const WindowInfo& window, Region *region );
void region_upload_new_meshes ( Region *region );
void region_issue_command ( Region *region, Region_Command command );

#endif