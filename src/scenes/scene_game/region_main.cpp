
#include "region.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>
#include <memory.h>

#include "../../shader.hpp"

static bool debugGridMeshGenerated = false;
static unsigned int debugGrid_vao = 0;
static unsigned int debugGrid_vbo = 0;
static unsigned int debugGrid_ibo = 0;
static unsigned int debugGrid_indexCount = 0;
static unsigned int debugGrid_shader = 0;
static void generate_debug_grid_mesh ( Region *region );
static void render_debug_grid_mesh ( float verticalOffset );

static void load_texture ( unsigned int* texID, const char* name )
{
	int texWidth, texHeight, n;
	unsigned char* bitmap = stbi_load( name, &texWidth, &texHeight, &n, 4 );

	if ( bitmap )
	{
		if ( *texID == 0 ) { glGenTextures( 1, texID ); GLCALL; }

		glBindTexture( GL_TEXTURE_2D, *texID ); GLCALL;
		glTexImage2D( GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap ); GLCALL;
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); GLCALL;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); GLCALL;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); GLCALL;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); GLCALL;

		stbi_image_free( bitmap );
	}
	else
	{
		*texID = 0;
	}
}

void region_init ( const WindowInfo& window, Region *region, unsigned int cl, unsigned int cw, unsigned int ch, 
															 unsigned int wl, unsigned int ww, unsigned int wh ) 
{
	region->chunkLength = cl;
	region->chunkWidth = cw;
	region->chunkHeight = ch;
	region->length = wl;
	region->width = ww;
	region->height = wh;

	region->viewDirection = Direction::D_NORTH;

	region->chunks = new Chunk_Data [wl*ww*wh];
	region->chunksNeedingMeshUpdate = new unsigned int [wl*ww*wh];
	region->chunkMeshes = new Chunk_Mesh [wl*ww*wh];

	for ( unsigned int i = 0; i < wl*ww*wh; ++i )
	{
		region->chunks[i].floor = new unsigned int [cl*cw*ch];
		region->chunks[i].wall = new unsigned int [cl*cw*ch];
		region->chunks[i].water = new unsigned int [cl*cw*ch];
	}

	region->chunksNeedingMeshUpdate_mutex.lock();
	for ( unsigned int i = 0; i < wl*ww*wh; ++i )
	{
		region->chunksNeedingMeshUpdate[i] = Chunk_Mesh_Data_Type::FLOOR | Chunk_Mesh_Data_Type::WALL | Chunk_Mesh_Data_Type::WATER;
	}
	region->chunksNeedingMeshUpdate_mutex.unlock();

	region->simulationPaused = false;
	region->simulationDeltaTime = 0;
	region->chunkDataGenerated = false;

	region->ageIncrementerFloor = 0;
	region->ageIncrementerWall = 0;
	region->ageIncrementerWater = 0;
	region->generationNextChunk = 0;

	region->viewHeight = ch * wh;

	region->updatedWaterBitset = std::vector<bool>( region->length*region->chunkLength*region->width*region->chunkWidth*region->height*region->chunkHeight, false );

	region->shader = load_shader(
		R"(
			#version 330 core

			layout(location = 0) in vec3 position;
			layout(location = 1) in vec2 textcoord;

			uniform mat4 projection;
			uniform mat4 view;
			uniform mat4 model;

			out vec2 passTexCoord;

			void main ()
			{
			    gl_Position = projection * view * model * vec4(position, 1.0);
			    passTexCoord = textcoord;
			}
		)",
		R"(
			#version 330 core

			layout(location = 0) out vec4 Color;

			uniform sampler2D ourTexture;

			in vec2 passTexCoord;

			void main ()
			{
				if ( texture(ourTexture, passTexCoord).w == 0 ) discard;
			    Color = texture(ourTexture, passTexCoord);
			}
		)"
	);

	region->chunkMeshTexture = 0;
	load_texture( &region->chunkMeshTexture, "res/TileMap.png" );

	region->projectionScale = 1.0f;
	region->projection = orthographic_projection( -window.height/2*region->projectionScale, window.height/2*region->projectionScale, -window.width/2*region->projectionScale, window.width/2*region->projectionScale, 0.1f, 5000.0f );
	region->camera = translate( mat4(1), -vec3(0, 0, 500) );

	// NOTE(Xavier): (2017.12.24) Debug.
	region->debug_drawDebugGrid = true;
	debugGrid_shader = load_shader(
		R"(
			#version 330 core

			layout(location = 0) in vec2 position;

			uniform mat4 projection;
			uniform mat4 view;
			uniform mat4 model;

			void main ()
			{
			    gl_Position = projection * view * model * vec4(position, 0.0, 1.0);
			}
		)",
		R"(
			#version 330 core

			uniform vec4 color;

			layout(location = 0) out vec4 Color;

			void main ()
			{
			    Color = color;
			}
		)"
	);
}



void region_cleanup ( Region *region )
{
	region->simulationPaused = true;
	region->chunkDataGenerated = false;

	for ( unsigned int i = 0; i < region->length*region->width*region->height; ++i )
	{
		delete [] region->chunks[i].floor;
		delete [] region->chunks[i].wall;
		delete [] region->chunks[i].water;
	}
	delete [] region->chunks;
	delete [] region->chunksNeedingMeshUpdate;
	delete [] region->chunkMeshes;
}



void region_render ( const WindowInfo& window, Region *region )
{
	region_upload_new_meshes( region );

	if ( region->viewHeight < 0 ) region->viewHeight = 0;
	if ( region->viewHeight > region->height*region->chunkHeight ) region->viewHeight = region->height*region->chunkHeight;

	glUseProgram( region->shader ); GLCALL;
	set_uniform_mat4( region->shader, "projection", &region->projection );
	set_uniform_mat4( region->shader, "view", &region->camera );
	
	for ( unsigned int i = 0; i < region->length*region->width*region->height; ++i )
	{
		auto& cm = region->chunkMeshes[i];

		if ( i/(region->length*region->width)*region->chunkHeight > region->viewHeight ) continue;
		unsigned int indexOffset = region->viewHeight - i/(region->length*region->width)*region->chunkHeight;
		if ( indexOffset > region->chunkHeight - 1 ) indexOffset = region->chunkHeight - 1;

		if ( cm.floorMesh.vao != 0 && cm.floorMesh.indexCount != 0 )
		{
			auto mtx = translate(mat4(1), vec3(0));
			set_uniform_mat4( region->shader, "model", &mtx );
			glBindVertexArray( cm.floorMesh.vao ); GLCALL;
			glBindTexture( GL_TEXTURE_2D, region->chunkMeshTexture ); GLCALL;
			glDrawElements( GL_TRIANGLES, cm.floorMesh.layeredIndexCount[indexOffset], GL_UNSIGNED_INT, 0 ); GLCALL;
		}

		if ( cm.wallMesh.vao != 0 && cm.wallMesh.indexCount != 0 )
		{
			auto mtx = translate(mat4(1), vec3(0));
			set_uniform_mat4( region->shader, "model", &mtx );
			glBindVertexArray( cm.wallMesh.vao ); GLCALL;
			glBindTexture( GL_TEXTURE_2D, region->chunkMeshTexture ); GLCALL;
			glDrawElements( GL_TRIANGLES, cm.wallMesh.layeredIndexCount[indexOffset], GL_UNSIGNED_INT, 0 ); GLCALL;
		}

		if ( cm.waterMesh.vao != 0 && cm.waterMesh.indexCount != 0 )
		{
			auto mtx = translate(mat4(1), vec3(0));
			set_uniform_mat4( region->shader, "model", &mtx );
			glBindVertexArray( cm.waterMesh.vao ); GLCALL;
			glBindTexture( GL_TEXTURE_2D, region->chunkMeshTexture ); GLCALL;
			glDrawElements( GL_TRIANGLES, cm.waterMesh.layeredIndexCount[indexOffset], GL_UNSIGNED_INT, 0 ); GLCALL;
		}
	}

	glClear( GL_DEPTH_BUFFER_BIT ); GLCALL;
	glUseProgram( debugGrid_shader ); GLCALL;
	set_uniform_mat4( debugGrid_shader, "projection", &region->projection );
	set_uniform_mat4( debugGrid_shader, "view", &region->camera );
	if ( region->debug_drawDebugGrid ) 
	{
		if ( debugGridMeshGenerated )
		{
			render_debug_grid_mesh( region->viewHeight );
		}
		else
		{
			generate_debug_grid_mesh( region );
			debugGridMeshGenerated = true;
		}
	}
}



void region_resize_viewport ( const WindowInfo& window, Region *region )
{
	region->projection = orthographic_projection( -window.height/2*region->projectionScale, window.height/2*region->projectionScale, -window.width/2*region->projectionScale, window.width/2*region->projectionScale, 0.1f, 5000.0f );
}



static void upload_mesh ( Region *region, Chunk_Mesh_Data *meshData )
{
	auto index = static_cast<unsigned int>(meshData->position.x + meshData->position.y*region->length + meshData->position.z*region->length*region->width);
	auto& cm = region->chunkMeshes[ index ];

	if ( meshData->type == Chunk_Mesh_Data_Type::FLOOR )
	{
		if ( cm.floorMeshAge <= meshData->age )
		{
			cm.floorMeshAge = meshData->age;

			if ( cm.floorMesh.vao == 0 ) { glGenVertexArrays( 1, &cm.floorMesh.vao ); GLCALL; }
			if ( cm.floorMesh.vbo == 0 ) { glGenBuffers( 1, &cm.floorMesh.vbo ); GLCALL; }
			if ( cm.floorMesh.ibo == 0 ) { glGenBuffers( 1, &cm.floorMesh.ibo ); GLCALL; }

			glBindVertexArray( cm.floorMesh.vao ); GLCALL;
			
				glBindBuffer( GL_ARRAY_BUFFER, cm.floorMesh.vbo ); GLCALL;
					glBufferData( GL_ARRAY_BUFFER, meshData->vertexData.size() * sizeof( float ), meshData->vertexData.data(), GL_DYNAMIC_DRAW ); GLCALL;
				
					GLint posAttrib = glGetAttribLocation( region->shader, "position" ); GLCALL;
					glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0 ); GLCALL;
					glEnableVertexAttribArray( posAttrib ); GLCALL;

					GLint texAttrib = glGetAttribLocation( region->shader, "textcoord" ); GLCALL;
					glVertexAttribPointer( texAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)12 ); GLCALL;
					glEnableVertexAttribArray( texAttrib ); GLCALL;

				glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, cm.floorMesh.ibo ); GLCALL;
					glBufferData( GL_ELEMENT_ARRAY_BUFFER, meshData->indexData.size() * sizeof(unsigned int), meshData->indexData.data(), GL_DYNAMIC_DRAW ); GLCALL;
			
			cm.floorMesh.indexCount = meshData->indexData.size();
			cm.floorMesh.layeredIndexCount = std::move( meshData->layeredIndexCount );

			glBindVertexArray( 0 ); GLCALL;
		}
	}
	else if ( meshData->type == Chunk_Mesh_Data_Type::WALL )
	{
		if ( cm.wallMeshAge <= meshData->age )
		{
			cm.wallMeshAge = meshData->age;

			if ( cm.wallMesh.vao == 0 ) { glGenVertexArrays( 1, &cm.wallMesh.vao ); GLCALL; }
			if ( cm.wallMesh.vbo == 0 ) { glGenBuffers( 1, &cm.wallMesh.vbo ); GLCALL; }
			if ( cm.wallMesh.ibo == 0 ) { glGenBuffers( 1, &cm.wallMesh.ibo ); GLCALL; }

			glBindVertexArray( cm.wallMesh.vao ); GLCALL;
			
				glBindBuffer( GL_ARRAY_BUFFER, cm.wallMesh.vbo ); GLCALL;
					glBufferData( GL_ARRAY_BUFFER, meshData->vertexData.size() * sizeof( float ), meshData->vertexData.data(), GL_DYNAMIC_DRAW ); GLCALL;
				
					GLint posAttrib = glGetAttribLocation( region->shader, "position" ); GLCALL;
					glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0 ); GLCALL;
					glEnableVertexAttribArray( posAttrib ); GLCALL;

					GLint texAttrib = glGetAttribLocation( region->shader, "textcoord" ); GLCALL;
					glVertexAttribPointer( texAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)12 ); GLCALL;
					glEnableVertexAttribArray( texAttrib ); GLCALL;

				glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, cm.wallMesh.ibo ); GLCALL;
					glBufferData( GL_ELEMENT_ARRAY_BUFFER, meshData->indexData.size() * sizeof(unsigned int), meshData->indexData.data(), GL_DYNAMIC_DRAW ); GLCALL;
			
			cm.wallMesh.indexCount = meshData->indexData.size();
			cm.wallMesh.layeredIndexCount = std::move( meshData->layeredIndexCount );

			glBindVertexArray( 0 ); GLCALL;
		}
	}
	else if ( meshData->type == Chunk_Mesh_Data_Type::WATER )
	{
		if ( cm.waterMeshAge <= meshData->age )
		{
			cm.waterMeshAge = meshData->age;

			if ( cm.waterMesh.vao == 0 ) { glGenVertexArrays( 1, &cm.waterMesh.vao ); GLCALL; }
			if ( cm.waterMesh.vbo == 0 ) { glGenBuffers( 1, &cm.waterMesh.vbo ); GLCALL; }
			if ( cm.waterMesh.ibo == 0 ) { glGenBuffers( 1, &cm.waterMesh.ibo ); GLCALL; }

			glBindVertexArray( cm.waterMesh.vao ); GLCALL;
			
				glBindBuffer( GL_ARRAY_BUFFER, cm.waterMesh.vbo ); GLCALL;
					glBufferData( GL_ARRAY_BUFFER, meshData->vertexData.size() * sizeof( float ), meshData->vertexData.data(), GL_DYNAMIC_DRAW ); GLCALL;
				
					GLint posAttrib = glGetAttribLocation( region->shader, "position" ); GLCALL;
					glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0 ); GLCALL;
					glEnableVertexAttribArray( posAttrib ); GLCALL;

					GLint texAttrib = glGetAttribLocation( region->shader, "textcoord" ); GLCALL;
					glVertexAttribPointer( texAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)12 ); GLCALL;
					glEnableVertexAttribArray( texAttrib ); GLCALL;

				glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, cm.waterMesh.ibo ); GLCALL;
					glBufferData( GL_ELEMENT_ARRAY_BUFFER, meshData->indexData.size() * sizeof(unsigned int), meshData->indexData.data(), GL_DYNAMIC_DRAW ); GLCALL;
			
			cm.waterMesh.indexCount = meshData->indexData.size();
			cm.waterMesh.layeredIndexCount = std::move( meshData->layeredIndexCount );

			glBindVertexArray( 0 ); GLCALL;
		}
	}
	else
	{
		std::cout << "ERROR: Couldn't upload to opengl.	" << meshData->type << "\n";
	}
}



void region_upload_new_meshes ( Region *region )
{
	if ( region->chunkMeshData_mutex_1.try_lock() )
	{
		for ( auto& meshData : region->chunkMeshData_1 )
		{
			upload_mesh( region, &meshData );
		}
		region->chunkMeshData_1.clear();
		region->chunkMeshData_mutex_1.unlock();
	}
	
	if ( region->chunkMeshData_mutex_2.try_lock() )
	{
		for ( auto& meshData : region->chunkMeshData_2 )
		{
			upload_mesh( region, &meshData );
		}
		region->chunkMeshData_2.clear();
		region->chunkMeshData_mutex_2.unlock();
	}
}



void region_issue_command ( Region *region, Region_Command command )
{
	if ( region->commandQue_mutex_1.try_lock() )
	{
		region->commandQue_1.push_back( command );
		region->commandQue_mutex_1.unlock();
	}
	else if ( region->commandQue_mutex_2.try_lock() )
	{
		region->commandQue_2.push_back( command );
		region->commandQue_mutex_2.unlock();
	}
	else
	{
		std::cout << "ERROR: Region Command was not able to be issued.\n";
	}
}



static void generate_debug_grid_mesh ( Region *region )
{
	std::vector<vec2> vertexData;
	std::vector<unsigned int> indexData;

	const vec2 xDir { -1, 18.0f/27.0f };
	const vec2 yDir {  1, 18.0f/27.0f };

	unsigned int curIndex = 0;
	for ( int i = 0; i <= region->length*region->chunkLength; ++i )
	{
		vertexData.emplace_back( xDir*i*27 );
		vertexData.emplace_back( xDir*i*27 + yDir*region->width*region->chunkWidth*27 );

		indexData.emplace_back( curIndex );
		indexData.emplace_back( curIndex + 1 );
		curIndex += 2;
	}
	for ( int i = 0; i <= region->width*region->chunkWidth; ++i )
	{
		vertexData.emplace_back( yDir*i*27 );
		vertexData.emplace_back( yDir*i*27 + xDir*region->length*region->chunkLength*27 );

		indexData.emplace_back( curIndex );
		indexData.emplace_back( curIndex + 1 );
		curIndex += 2;
	}

	if ( debugGrid_vao == 0 ) { glGenVertexArrays( 1, &debugGrid_vao ); GLCALL; }
	if ( debugGrid_vbo == 0 ) { glGenBuffers( 1, &debugGrid_vbo ); GLCALL; }
	if ( debugGrid_ibo == 0 ) { glGenBuffers( 1, &debugGrid_ibo ); GLCALL; }

	glBindVertexArray( debugGrid_vao ); GLCALL;
		
			glBindBuffer( GL_ARRAY_BUFFER, debugGrid_vbo ); GLCALL;
				glBufferData( GL_ARRAY_BUFFER, vertexData.size() * sizeof( vec2 ), vertexData.data(), GL_STATIC_DRAW ); GLCALL;
			
				GLint posAttrib = glGetAttribLocation( region->shader, "position" ); GLCALL;
				glVertexAttribPointer( posAttrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0 ); GLCALL;
				glEnableVertexAttribArray( posAttrib ); GLCALL;

			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, debugGrid_ibo ); GLCALL;
				glBufferData( GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(unsigned int), indexData.data(), GL_STATIC_DRAW ); GLCALL;
		
		debugGrid_indexCount = indexData.size();

	glBindVertexArray( 0 ); GLCALL;
}

static void render_debug_grid_mesh ( float verticalOffset )
{
	auto mtx = translate(mat4(1), vec3(0, verticalOffset*30, 0));
	set_uniform_mat4( debugGrid_shader, "model", &mtx );
	set_uniform_vec4( debugGrid_shader, "color", vec4(1, 0, 0, 1) );
	glBindVertexArray( debugGrid_vao ); GLCALL;
	glDrawElements( GL_LINES, debugGrid_indexCount, GL_UNSIGNED_INT, 0 ); GLCALL;
	mtx = translate(mat4(1), vec3(0, verticalOffset*30+7, 0));
	set_uniform_mat4( debugGrid_shader, "model", &mtx );
	set_uniform_vec4( debugGrid_shader, "color", vec4(1, 1, 0, 1) );
	glDrawElements( GL_LINES, debugGrid_indexCount, GL_UNSIGNED_INT, 0 ); GLCALL;
	mtx = translate(mat4(1), vec3(0, verticalOffset*30+30, 0));
	set_uniform_mat4( debugGrid_shader, "model", &mtx );
	set_uniform_vec4( debugGrid_shader, "color", vec4(1, 0, 1, 1) );
	glDrawElements( GL_LINES, debugGrid_indexCount, GL_UNSIGNED_INT, 0 ); GLCALL;
}
