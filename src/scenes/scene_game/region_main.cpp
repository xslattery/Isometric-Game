
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>
#include <memory.h>
#include "../../shader.hpp"

#include "region.hpp"


//////////////////////////////////
// This function will load a texture
// and return the opengl texture id
// by argument reference.
static void load_texture ( uint32_t* texID, const char* name )
{
	int texWidth, texHeight, n;
	uint8_t* bitmap = stbi_load( name, &texWidth, &texHeight, &n, 4 );

	if ( bitmap ) {
		if ( *texID == 0 ) { glGenTextures( 1, texID ); GLCALL; }

		glBindTexture( GL_TEXTURE_2D, *texID ); GLCALL;
		glTexImage2D( GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap ); GLCALL;
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); GLCALL;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); GLCALL;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); GLCALL;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); GLCALL;

		stbi_image_free( bitmap );
	}
	else {
		*texID = 0;
	}
}


//////////////////////////////////
// This function is responsiable
// for initilizing everything about
// the region. This can include
// information that relates to other threads.
void region_init ( const WindowInfo& window, Region *region, uint32_t cl, uint32_t cw, uint32_t ch, uint32_t wl, uint32_t ww, uint32_t wh )
{
	region->chunkLength = cl;
	region->chunkWidth = cw;
	region->chunkHeight = ch;
	region->length = wl;
	region->width = ww;
	region->height = wh;
	region->worldLength = cl * wl;
	region->worldWidth = cw * ww;
	region->worldHeight = ch * wh;

	region->viewDirection = Direction::D_NORTH;

	region->chunks = new Chunk_Data [wl*ww*wh];
	region->chunksNeedingMeshUpdate = new uint32_t [wl*ww*wh];
	region->chunkMeshes = new Chunk_Mesh [wl*ww*wh];

	for ( uint32_t i = 0; i < wl*ww*wh; ++i ) {
		region->chunks[i].floor = new uint32_t [cl*cw*ch];
		region->chunks[i].wall = new uint32_t [cl*cw*ch];
		region->chunks[i].water = new uint8_t [cl*cw*ch];
	}

	region->chunksNeedingMeshUpdate_mutex.lock();
	for ( uint32_t i = 0; i < wl*ww*wh; ++i ) {
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
	region->viewDepth = 72;
	region->halfHeight = false;

	region->updatedWaterBitset = std::vector<bool>( region->worldLength*region->worldWidth*region->worldHeight, false );

	region->shader = load_shader(
		R"(
			#version 330 core

			layout(location = 0) in vec3 position;
			layout(location = 1) in vec2 textcoord;

			uniform mat4 projection;
			uniform mat4 view;
			uniform mat4 model;

			out vec2 passTexCoord;

			void main () {
			    gl_Position = projection * view * model * vec4(position, 1.0);
			    passTexCoord = textcoord;
			}
		)",
		R"(
			#version 330 core

			layout(location = 0) out vec4 Color;

			uniform sampler2D ourTexture;

			in vec2 passTexCoord;

			void main () {
				if ( texture(ourTexture, passTexCoord).w == 0 ) discard;
			    Color = texture(ourTexture, passTexCoord);
			}
		)"
	);

	region->chunkMeshTexture = 0;
	load_texture( &region->chunkMeshTexture, "res/TileMap.png" );

	region->chunkMeshTexture_halfHeight = 0;
	load_texture( &region->chunkMeshTexture_halfHeight, "res/TileMapHalfHeight.png" );

	region->projectionScale = 1.0f;
	region->projection = orthographic_projection( -window.height/2*region->projectionScale, window.height/2*region->projectionScale, -window.width/2*region->projectionScale, window.width/2*region->projectionScale, 0.1f, 5000.0f );
	region->camera = translate( mat4(1), -vec3(0, 0, 500) );
}


//////////////////////////////////
// This function is responsiable 
// for cleaning up when the region
// is removed.
void region_cleanup ( Region *region )
{
	region->simulationPaused = true;
	region->chunkDataGenerated = false;

	for ( uint32_t i = 0; i < region->length*region->width*region->height; ++i ) {
		delete [] region->chunks[i].floor;
		delete [] region->chunks[i].wall;
		delete [] region->chunks[i].water;
	}
	delete [] region->chunks;
	delete [] region->chunksNeedingMeshUpdate;
	delete [] region->chunkMeshes;
}


//////////////////////////////////
// This function uploads then
// renders the regions meshes.
void region_render ( const WindowInfo& window, Region *region )
{
	region_upload_new_meshes( region );

	if ( region->viewHeight < 0 ) region->viewHeight = 0;
	if ( region->viewHeight > region->worldHeight-1 ) region->viewHeight = region->worldHeight-1;

	if ( region->viewDepth < 1 ) region->viewDepth = 1;
	if ( region->viewDepth > region->worldHeight ) region->viewDepth = region->worldHeight;

	glUseProgram( region->shader ); GLCALL;
	set_uniform_mat4( region->shader, "projection", &region->projection );
	set_uniform_mat4( region->shader, "view", &region->camera );
	
	for ( uint32_t i = 0; i < region->length*region->width*region->height; ++i ) {
		if ( i/(region->length*region->width)*region->chunkHeight > region->viewHeight ) continue;
		if ( (int)(i/(region->length*region->width)*region->chunkHeight + region->chunkHeight-1) <= (int)region->viewHeight - (int)region->viewDepth ) continue;
		
		uint32_t indexOffsetTop = region->viewHeight - i/(region->length*region->width)*region->chunkHeight;
		if ( indexOffsetTop > region->chunkHeight - 1 ) indexOffsetTop = region->chunkHeight - 1;
		
		int indexOffsetBottom = 0;
		if ( i/(region->length*region->width)*region->chunkHeight >= region->viewHeight - ((region->viewHeight)%region->chunkHeight) ) {
			indexOffsetBottom = indexOffsetTop - region->viewDepth;
		}
		else {
			int chunkDifference = ((region->viewHeight - ((region->viewHeight)%region->chunkHeight) - i/(region->length*region->width)*region->chunkHeight) / region->chunkHeight) - 1;
			int tmp = (region->viewDepth - (region->viewHeight%region->chunkHeight) - 1) - chunkDifference*region->chunkHeight;
			if ( tmp <= 0 ) tmp = 10;
			indexOffsetBottom = indexOffsetTop - tmp;
		}

		auto& cm = region->chunkMeshes[i];
		if ( cm.floorMesh.vao != 0 && cm.floorMesh.indexCount != 0 ) {
			auto mtx = translate(mat4(1), vec3(0));
			set_uniform_mat4( region->shader, "model", &mtx );
			glBindTexture( GL_TEXTURE_2D, region->chunkMeshTexture ); GLCALL;
			glBindVertexArray( cm.floorMesh.vao ); GLCALL;
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, cm.floorMesh.ibo ); GLCALL;

			uint32_t indexStart = 0;
			uint32_t indexEnd = cm.floorMesh.layeredIndexCount[indexOffsetTop];
			if ( indexOffsetBottom >= 0 ) {
				indexStart = cm.floorMesh.layeredIndexCount[indexOffsetBottom];
				indexEnd = cm.floorMesh.layeredIndexCount[indexOffsetTop] - indexStart;
			}

			if ( indexEnd+indexStart > cm.floorMesh.indexCount ) indexEnd = cm.floorMesh.indexCount-indexStart;

			glDrawElements( GL_TRIANGLES, indexEnd, GL_UNSIGNED_INT, (void*)(indexStart*sizeof(uint32_t)) ); GLCALL;
			glBindVertexArray( 0 ); GLCALL;
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ); GLCALL;
		}

		if ( cm.wallMesh.vao != 0 && cm.wallMesh.indexCount != 0 ) {
			auto mtx = translate(mat4(1), vec3(0));
			set_uniform_mat4( region->shader, "model", &mtx );
			glBindTexture( GL_TEXTURE_2D, region->chunkMeshTexture ); GLCALL;
			glBindVertexArray( cm.wallMesh.vao ); GLCALL;
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, cm.wallMesh.ibo ); GLCALL;

			uint32_t indexStart = 0;
			uint32_t indexEnd = cm.wallMesh.layeredIndexCount[indexOffsetTop];
			if ( indexOffsetBottom >= 0 ) {
				indexStart = cm.wallMesh.layeredIndexCount[indexOffsetBottom];
				indexEnd = cm.wallMesh.layeredIndexCount[indexOffsetTop] - indexStart;
			}

			if ( indexEnd+indexStart > cm.wallMesh.indexCount ) indexEnd = cm.wallMesh.indexCount-indexStart;

			glDrawElements( GL_TRIANGLES, indexEnd, GL_UNSIGNED_INT, (void*)(indexStart*sizeof(uint32_t)) ); GLCALL;
			glBindVertexArray( 0 ); GLCALL;
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ); GLCALL;
		}

		if ( cm.waterMesh.vao != 0 && cm.waterMesh.indexCount != 0 ) {
			auto mtx = translate(mat4(1), vec3(0));
			set_uniform_mat4( region->shader, "model", &mtx );
			glBindTexture( GL_TEXTURE_2D, region->chunkMeshTexture ); GLCALL;
			glBindVertexArray( cm.waterMesh.vao ); GLCALL;
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, cm.waterMesh.ibo ); GLCALL;

			uint32_t indexStart = 0;
			uint32_t indexEnd = cm.waterMesh.layeredIndexCount[indexOffsetTop];
			if ( indexOffsetBottom >= 0 ) {
				indexStart = cm.waterMesh.layeredIndexCount[indexOffsetBottom];
				indexEnd = cm.waterMesh.layeredIndexCount[indexOffsetTop] - indexStart;
			}

			if ( indexEnd+indexStart > cm.waterMesh.indexCount ) indexEnd = cm.waterMesh.indexCount-indexStart;

			glDrawElements( GL_TRIANGLES, indexEnd, GL_UNSIGNED_INT, (void*)(indexStart*sizeof(uint32_t)) ); GLCALL;
			glBindVertexArray( 0 ); GLCALL;
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ); GLCALL;
		}
	}
}


//////////////////////////////////
// This function handles what should
// happen when the window is resized.
void region_resize_viewport ( const WindowInfo& window, Region *region )
{
	region->projection = orthographic_projection( -window.height/2*region->projectionScale, window.height/2*region->projectionScale, -window.width/2*region->projectionScale, window.width/2*region->projectionScale, 0.1f, 5000.0f );
}


//////////////////////////////////
// This function is responsiable
// for uploading mesh data to the 
// opengl driver.
static void upload_mesh ( Region *region, Chunk_Mesh_Data *meshData )
{
	auto index = static_cast<uint32_t>(meshData->position.x + meshData->position.y*region->length + meshData->position.z*region->length*region->width);
	auto& cm = region->chunkMeshes[ index ];

	if ( meshData->type == Chunk_Mesh_Data_Type::FLOOR ) {
		if ( cm.floorMeshAge <= meshData->age ) {
			cm.floorMeshAge = meshData->age;

			if ( cm.floorMesh.vao == 0 ) { glGenVertexArrays( 1, &cm.floorMesh.vao ); GLCALL; }
			if ( cm.floorMesh.vbo == 0 ) { glGenBuffers( 1, &cm.floorMesh.vbo ); GLCALL; }
			if ( cm.floorMesh.ibo == 0 ) { glGenBuffers( 1, &cm.floorMesh.ibo ); GLCALL; }

			glBindVertexArray( cm.floorMesh.vao ); GLCALL;
			
				glBindBuffer( GL_ARRAY_BUFFER, cm.floorMesh.vbo ); GLCALL;
					glBufferData( GL_ARRAY_BUFFER, meshData->vertexData.size() * sizeof( float ), meshData->vertexData.data(), GL_DYNAMIC_DRAW ); GLCALL;
				
					GLint posAttrib = glGetAttribLocation( region->shader, "position" ); GLCALL;
					glEnableVertexAttribArray( posAttrib ); GLCALL;
					glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0 ); GLCALL;

					GLint texAttrib = glGetAttribLocation( region->shader, "textcoord" ); GLCALL;
					glEnableVertexAttribArray( texAttrib ); GLCALL;
					glVertexAttribPointer( texAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)12 ); GLCALL;

				glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, cm.floorMesh.ibo ); GLCALL;
					glBufferData( GL_ELEMENT_ARRAY_BUFFER, meshData->indexData.size() * sizeof(uint32_t), meshData->indexData.data(), GL_DYNAMIC_DRAW ); GLCALL;
			
			cm.floorMesh.indexCount = meshData->indexData.size();
			cm.floorMesh.layeredIndexCount = std::move( meshData->layeredIndexCount );

			glBindVertexArray( 0 ); GLCALL;
			glBindBuffer( GL_ARRAY_BUFFER, 0 ); GLCALL;
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ); GLCALL;
		}
	}
	else if ( meshData->type == Chunk_Mesh_Data_Type::WALL ) {
		if ( cm.wallMeshAge <= meshData->age ) {
			cm.wallMeshAge = meshData->age;

			if ( cm.wallMesh.vao == 0 ) { glGenVertexArrays( 1, &cm.wallMesh.vao ); GLCALL; }
			if ( cm.wallMesh.vbo == 0 ) { glGenBuffers( 1, &cm.wallMesh.vbo ); GLCALL; }
			if ( cm.wallMesh.ibo == 0 ) { glGenBuffers( 1, &cm.wallMesh.ibo ); GLCALL; }

			glBindVertexArray( cm.wallMesh.vao ); GLCALL;
			
				glBindBuffer( GL_ARRAY_BUFFER, cm.wallMesh.vbo ); GLCALL;
					glBufferData( GL_ARRAY_BUFFER, meshData->vertexData.size() * sizeof( float ), meshData->vertexData.data(), GL_DYNAMIC_DRAW ); GLCALL;
				
					GLint posAttrib = glGetAttribLocation( region->shader, "position" ); GLCALL;
					glEnableVertexAttribArray( posAttrib ); GLCALL;
					glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0 ); GLCALL;

					GLint texAttrib = glGetAttribLocation( region->shader, "textcoord" ); GLCALL;
					glEnableVertexAttribArray( texAttrib ); GLCALL;
					glVertexAttribPointer( texAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)12 ); GLCALL;

				glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, cm.wallMesh.ibo ); GLCALL;
					glBufferData( GL_ELEMENT_ARRAY_BUFFER, meshData->indexData.size() * sizeof(uint32_t), meshData->indexData.data(), GL_DYNAMIC_DRAW ); GLCALL;

			cm.wallMesh.indexCount = meshData->indexData.size();
			cm.wallMesh.layeredIndexCount = std::move( meshData->layeredIndexCount );

			glBindVertexArray( 0 ); GLCALL;
			glBindBuffer( GL_ARRAY_BUFFER, 0 ); GLCALL;
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ); GLCALL;
		}
	}
	else if ( meshData->type == Chunk_Mesh_Data_Type::WATER ) {
		if ( cm.waterMeshAge <= meshData->age ) {
			cm.waterMeshAge = meshData->age;

			if ( cm.waterMesh.vao == 0 ) { glGenVertexArrays( 1, &cm.waterMesh.vao ); GLCALL; }
			if ( cm.waterMesh.vbo == 0 ) { glGenBuffers( 1, &cm.waterMesh.vbo ); GLCALL; }
			if ( cm.waterMesh.ibo == 0 ) { glGenBuffers( 1, &cm.waterMesh.ibo ); GLCALL; }

			glBindVertexArray( cm.waterMesh.vao ); GLCALL;
			
				glBindBuffer( GL_ARRAY_BUFFER, cm.waterMesh.vbo ); GLCALL;
					glBufferData( GL_ARRAY_BUFFER, meshData->vertexData.size() * sizeof( float ), meshData->vertexData.data(), GL_DYNAMIC_DRAW ); GLCALL;
				
					GLint posAttrib = glGetAttribLocation( region->shader, "position" ); GLCALL;
					glEnableVertexAttribArray( posAttrib ); GLCALL;
					glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0 ); GLCALL;

					GLint texAttrib = glGetAttribLocation( region->shader, "textcoord" ); GLCALL;
					glEnableVertexAttribArray( texAttrib ); GLCALL;
					glVertexAttribPointer( texAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)12 ); GLCALL;

				glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, cm.waterMesh.ibo ); GLCALL;
					glBufferData( GL_ELEMENT_ARRAY_BUFFER, meshData->indexData.size() * sizeof(uint32_t), meshData->indexData.data(), GL_DYNAMIC_DRAW ); GLCALL;
			
			cm.waterMesh.indexCount = meshData->indexData.size();
			cm.waterMesh.layeredIndexCount = std::move( meshData->layeredIndexCount );

			glBindVertexArray( 0 ); GLCALL;
			glBindBuffer( GL_ARRAY_BUFFER, 0 ); GLCALL;
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ); GLCALL;
		}
	}
	else {
		std::cout << "ERROR: Couldn't upload to opengl.	" << meshData->type << "\n";
	}
}


//////////////////////////////////
// This function handles the inter-
// thread communication for uploading
// new meshes to the opengl driver.
void region_upload_new_meshes ( Region *region )
{
	if ( region->chunkMeshData_mutex_1.try_lock() ) {
		for ( auto& meshData : region->chunkMeshData_1 ) {
			upload_mesh( region, &meshData );
		}
		region->chunkMeshData_1.clear();
		region->chunkMeshData_mutex_1.unlock();
	}
	
	if ( region->chunkMeshData_mutex_2.try_lock() ) {
		for ( auto& meshData : region->chunkMeshData_2 ) {
			upload_mesh( region, &meshData );
		}
		region->chunkMeshData_2.clear();
		region->chunkMeshData_mutex_2.unlock();
	}
}


//////////////////////////////////
// This function handles the inter-
// thread communication and sends a 
// command to the simulation thread.
void region_issue_command ( Region *region, Region_Command command )
{
	if ( region->commandQue_mutex_1.try_lock() ) {
		region->commandQue_1.push_back( command );
		region->commandQue_mutex_1.unlock();
	}
	else if ( region->commandQue_mutex_2.try_lock() ) {
		region->commandQue_2.push_back( command );
		region->commandQue_mutex_2.unlock();
	}
	else {
		std::cout << "ERROR: Region Command was not able to be issued.\n";
	}
}
