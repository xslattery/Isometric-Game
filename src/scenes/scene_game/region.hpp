#ifndef _REGION_HPP_
#define _REGION_HPP_

#include <vector>
#include <mutex>
#include <cstdint>
#include "../../platform/platform.h"
#include "../../platform/opengl.hpp"
#include "../../math/math.hpp"

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
	ADD_WATER_WAVE = 4,
};

struct Region_Command
{
	uint32_t type;
};

struct Chunk_Data
{
	uint32_t *floor = nullptr;
	uint32_t floorBegin, floorEnd;
	uint32_t floorNonHiddenBegin, floorNonHiddenEnd;

	uint32_t *wall = nullptr;
	uint32_t wallBegin, wallEnd;
	uint32_t wallNonHiddenBegin, wallNonHiddenEnd;

	uint8_t *water = nullptr;
	uint32_t waterBegin, waterEnd;
	uint32_t waterNonHiddenBegin, waterNonHiddenEnd;
};

enum Chunk_Mesh_Data_Type
{
	FLOOR = 0x1 << 0,
	WALL = 0x1 << 1,
	WATER = 0x1 << 2,
};

struct Chunk_Mesh_Data
{
	uint32_t type;
	vec3 position;
	size_t age;
	std::vector<float> vertexData;
	std::vector<uint32_t> indexData;
	std::vector<uint32_t> layeredIndexCount;
};

struct Chunk_Mesh
{
	struct Sub_Mesh
	{
		uint32_t vao = 0;
		uint32_t vbo = 0;
		uint32_t ibo = 0;
		uint32_t indexCount = 0;
		std::vector<uint32_t> layeredIndexCount;

		~Sub_Mesh()
		{
			if ( vao != 0 ) { glDeleteVertexArrays( 1, &vao ); GLCALL; }
			if ( vbo != 0 ) { glDeleteBuffers( 1, &vbo ); GLCALL; }
			if ( ibo != 0 ) { glDeleteBuffers( 1, &ibo ); GLCALL; }
		}
	};

	size_t floorMeshAge = 0;
	Sub_Mesh floorMesh;

	size_t wallMeshAge = 0;
	Sub_Mesh wallMesh;

	size_t waterMeshAge = 0;
	Sub_Mesh waterMesh;
};

struct Region
{
	// SIMULATION THREAD:
	Chunk_Data *chunks = nullptr;
	uint32_t length, width, height;
	uint32_t chunkLength, chunkWidth, chunkHeight;
	uint32_t worldLength, worldWidth, worldHeight;

	std::mutex chunksNeedingMeshUpdate_mutex;
	uint32_t *chunksNeedingMeshUpdate = nullptr;

	std::vector<vec4> waterThatNeedsUpdate;
	std::vector<bool> updatedWaterBitset;

	// GENERATION THREAD:
	std::atomic<uint32_t> ageIncrementerFloor;
	std::atomic<uint32_t> ageIncrementerWall;
	std::atomic<uint32_t> ageIncrementerWater;
	std::atomic<uint32_t> generationNextChunk;

	// MAIN, SIMULATION & GENERATION THREADS:
	std::atomic_bool simulationPaused;
	std::atomic<uint32_t> simulationDeltaTime;
	std::atomic<uint32_t> generationDeltaTime;
	std::atomic_bool chunkDataGenerated;
	std::atomic<uint32_t> numberOfWaterBeingUpdated;

	std::atomic<uint32_t> viewDirection;

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

	uint32_t shader;
	uint32_t chunkMeshTexture;
	uint32_t chunkMeshTexture_halfHeight;
	float projectionScale;
	mat4 projection;
	mat4 camera;
	int viewHeight;
	int viewDepth;
	bool halfHeight;
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
void region_init ( const WindowInfo& window, Region *region, uint32_t cl, uint32_t cw, uint32_t ch, uint32_t wl, uint32_t ww, uint32_t wh );
void region_cleanup ( Region *region );
void region_render ( const WindowInfo& window, Region *region );
void region_resize_viewport ( const WindowInfo& window, Region *region );
void region_upload_new_meshes ( Region *region );
void region_issue_command ( Region *region, Region_Command command );

#endif