#include <iostream>

#include "region.hpp"
#include "../shader.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

static void load_texture ( unsigned int* texID, const char* name )
{
	int texWidth, texHeight, n;
	unsigned char* bitmap = stbi_load( name, &texWidth, &texHeight, &n, 4 );

	if ( bitmap )
	{
		if ( *texID == 0 ) { glGenTextures( 1, texID ); GLCALL; }

		glBindTexture( GL_TEXTURE_2D, *texID ); GLCALL;
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap ); GLCALL;
		
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

/////////////////////////
// SIMULATION THREAD:
void Region::simulate()
{
	auto execute_command = [&]( std::vector<Command>& commandQue )
	{
		for ( auto& com : commandQue )
		{
			if ( regionDataGenerated )
			{	
				// Simulation commands:
				switch ( com.type )
				{
					default: break;
				}
			}
			else 
			{
				// Pre-Simulation commands:
				switch ( com.type )
				{
					case Command_Type::GENERATE_REGION_DATA:
						generate();
						break;
					
					default: break;
				}
			}
		}
		commandQue.clear();
	};

	if ( simulationUsingCommandQue_1 == false )
	{
		renderingUsingCommandQue_1 = true;
		if ( simulationUsingCommandQue_1 == false )
		{
			execute_command( commandQue_1 );
			renderingUsingCommandQue_1 = false;
		} else { renderingUsingCommandQue_1 = false; }
	}
	
	if ( simulationUsingCommandQue_2 == false )
	{
		renderingUsingCommandQue_2 = true;
		if ( simulationUsingCommandQue_2 == false )
		{
			execute_command( commandQue_2 );
			renderingUsingCommandQue_2 = false;
		} else { renderingUsingCommandQue_2 = false; }
	}

	if ( !simulationPaused )
	{
		if ( regionDataGenerated )
		{
			// TEMP(Xavier): (2017.11.30)
			// This is here only to test the simulation passing data
			// to the render thread.
			// for ( unsigned int i = 0; i < ceil(height/chunk_height); ++i )
			// 	for ( unsigned int j = 0; j < ceil(width/chunk_width); ++j )
			// 		for ( unsigned int k = 0; k < ceil(length/chunk_length); ++k )
			// 			meshesNeedingUpdate_floor.push_back( vec3(i,j,k) );
			
			// build_floor_mesh();
			build_wall_mesh();
			// build_water_mesh();
			// build_object_mesh();
		}
	}
}

Floor Region::get_floor ( int x, int y, int z )
{
	if ( x >= 0 && x < length && y >= 0 && y < width && z >= 0 && z < height )
		return tiles.floor[ z*width*length + y*length + x ];
	return Floor::NONE;
}

Wall Region::get_wall ( int x, int y, int z )
{
	if ( x >= 0 && x < length && y >= 0 && y < width && z >= 0 && z < height )
		return tiles.wall[ z*width*length + y*length + x ];
	return Wall::NONE;
}

unsigned char Region::get_water ( int x, int y, int z )
{
	if ( x >= 0 && x < length && y >= 0 && y < width && z >= 0 && z < height )
		return tiles.water[ z*width*length + y*length + x ];
	return 0;
}

Object Region::get_object ( int x, int y, int z )
{
	if ( x >= 0 && x < length && y >= 0 && y < width && z >= 0 && z < height )
		return tiles.object[ z*width*length + y*length + x ];
	return Object::NONE;
}

Direction Region::get_direction ( int x, int y, int z )
{
	if ( x >= 0 && x < length && y >= 0 && y < width && z >= 0 && z < height )
		return tiles.direction[ z*width*length + y*length + x ];
	return Direction::NONE;
}

static std::size_t buildAgeIdentifier_floor = 0;
void Region::build_floor_mesh ()
{
	if ( meshesNeedingUpdate_floor.size() > 0 )
	{
		for ( auto p : meshesNeedingUpdate_floor )
		{
			// TODO(Xavier): (2017.11.30) Build floor mesh ...
			// TEMP(Xavier): (2017.11.30)
			// This is here only to test the simulation passing data
			// to the render thread.
			float offsetX = rand()%300;
			float offsetY = rand()%300;
			std::vector<float> verts = 
			{
				100+offsetX, 100+offsetY, 0,
				100+offsetX, 200+offsetY, 0,
				200+offsetX, 100+offsetY, 0,
			};
			std::vector<unsigned int> indices = 
			{
				0, 1, 2,
			};

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
					simulationUsingUploadQue_1 = false;
				} else { simulationUsingUploadQue_1 = false; }
			}
		}
		meshesNeedingUpdate_floor.clear();
	}
}

static std::size_t buildAgeIdentifier_wall = 0;
void Region::build_wall_mesh ()
{
	if ( meshesNeedingUpdate_wall.size() > 0 )
	{
		for ( auto p : meshesNeedingUpdate_wall )
		{
			std::vector<float> verts;
			std::vector<unsigned int> indices;
			float ox = p.x;
			float oy = p.y;
			float oz = p.z;
			vec2 xDir { -1, 18.0f/27.0f };
			vec2 yDir {  1, 18.0f/27.0f };

			vec2 tl { 0, 			1.0f-1.0f/512*68 };
			vec2 tr { 1.0f/512*54, 	1.0f-1.0f/512*68 };
			vec2 bl { 0, 			1.0f };
			vec2 br { 1.0f/512*54, 	1.0f };

			for ( float zz = 0; zz < chunk_height; ++zz )
			{
				for ( float yy = 0; yy < chunk_width; ++yy )
				{
					for ( float xx = 0; xx < chunk_length; ++xx )
					{
						if ( get_wall( ox+xx, oy+yy, oz+zz ) != Wall::NONE )
						{
							unsigned int idxP = verts.size()/5;
							unsigned int tempIndices [6] =
							{
								idxP+0, idxP+1, idxP+2,
								idxP+2, idxP+1, idxP+3
							};
							indices.insert( indices.end(), tempIndices, tempIndices+6 );

							vec2 pos = ( (xx+ox*chunk_length)*xDir + (yy+oy*chunk_width)*yDir ) * 27;
							pos += vec2{ 0, 32 } * (zz+oz*chunk_height);
							float zPos = -(xx+ox + yy+oy) + (zz+oz)*2;

							float tempVerts [20] =
							{
								-27.0f+pos.x, 68.0f+pos.y, zPos,		tl.x, tl.y,
								-27.0f+pos.x,  0.0f+pos.y, zPos,		bl.x, bl.y,
								 27.0f+pos.x, 68.0f+pos.y, zPos,		tr.x, tr.y,
								 27.0f+pos.x,  0.0f+pos.y, zPos,		br.x, br.y,
							};
							verts.insert( verts.end(), tempVerts, tempVerts+20 );
						}
					}
				}
			}

			// Pass to shared mesh data ...
			if ( renderingUsingUploadQue_2 == false )
			{
				simulationUsingUploadQue_2 = true;
				if ( renderingUsingUploadQue_2 == false )
				{
					chunkMeshesToBeUploaded_2.emplace_back( p );
					auto& cm = chunkMeshesToBeUploaded_2.back();
					buildAgeIdentifier_wall++;
					cm.ageIdentifier_wall = buildAgeIdentifier_wall;
					cm.wallVertData = std::move( verts );
					cm.wallIndexData = std::move( indices );
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
					buildAgeIdentifier_wall++;
					cm.ageIdentifier_wall = buildAgeIdentifier_wall;
					cm.wallVertData = std::move( verts );
					cm.wallIndexData = std::move( indices );
					simulationUsingUploadQue_1 = false;
				} else { simulationUsingUploadQue_1 = false; }
			}
		}
		meshesNeedingUpdate_wall.clear();
	}
}

static std::size_t buildAgeIdentifier_water = 0;
void Region::build_water_mesh ()
{
	if ( meshesNeedingUpdate_water.size() > 0 )
	{
		for ( auto p : meshesNeedingUpdate_water )
		{
			// TODO(Xavier): (2017.11.30) Build water mesh ...
			// TEMP(Xavier): (2017.11.30)
			// This is here only to test the simulation passing data
			// to the render thread.
			float offset = rand()%300;
			std::vector<float> verts = 
			{
				100+offset, 100, 0,
				100+offset, 200, 0,
				200+offset, 100, 0,
			};
			std::vector<unsigned int> indices = 
			{
				0, 1, 2,
			};

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
					simulationUsingUploadQue_1 = false;
				} else { simulationUsingUploadQue_1 = false; }
			}
		}
		meshesNeedingUpdate_water.clear();
	}
}

static std::size_t buildAgeIdentifier_object = 0;
void Region::build_object_mesh ()
{
	if ( meshesNeedingUpdate_object.size() > 0 )
	{
		for ( auto p : meshesNeedingUpdate_object )
		{
			// TODO(Xavier): (2017.11.30) Build object mesh ...
			// TEMP(Xavier): (2017.11.30)
			// This is here only to test the simulation passing data
			// to the render thread.
			float offset = rand()%300;
			std::vector<float> verts = 
			{
				100+offset, 100, 0,
				100+offset, 200, 0,
				200+offset, 100, 0,
			};
			std::vector<unsigned int> indices = 
			{
				0, 1, 2,
			};

			// Pass to shared mesh data ...
			if ( renderingUsingUploadQue_2 == false )
			{
				simulationUsingUploadQue_2 = true;
				if ( renderingUsingUploadQue_2 == false )
				{
					chunkMeshesToBeUploaded_2.emplace_back( p );
					auto& cm = chunkMeshesToBeUploaded_2.back();
					buildAgeIdentifier_object++;
					cm.ageIdentifier_object = buildAgeIdentifier_object;
					cm.objectVertData = std::move( verts );
					cm.objectIndexData = std::move( indices );
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
					buildAgeIdentifier_object++;
					cm.ageIdentifier_object = buildAgeIdentifier_object;
					cm.objectVertData = std::move( verts );
					cm.objectIndexData = std::move( indices );
					simulationUsingUploadQue_1 = false;
				} else { simulationUsingUploadQue_1 = false; }
			}
		}
		meshesNeedingUpdate_object.clear();
	}
}

void Region::generate ()
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
				tiles.floor[ i*width*length + j*length + k ] = Floor::STONE;
				tiles.wall[ i*width*length + j*length + k ] = Wall::STONE;
				tiles.water[ i*width*length + j*length + k ] = 0;
				tiles.object[ i*width*length + j*length + k ] = Object::NONE;
				tiles.direction[ i*width*length + j*length + k ] = Direction::NONE;
			}	
		}
	}

	for ( unsigned int i = 0; i < ceil(height/chunk_height); ++i )
	{
		for ( unsigned int j = 0; j < ceil(width/chunk_width); ++j )
		{
			for ( unsigned int k = 0; k < ceil(length/chunk_length); ++k )
			{
				// meshesNeedingUpdate_floor.push_back( vec3(i,j,k) );
				meshesNeedingUpdate_wall.push_back( vec3(i,j,k) );
				// meshesNeedingUpdate_water.push_back( vec3(i,j,k) );
				// meshesNeedingUpdate_object.push_back( vec3(i,j,k) );
			}
		}
	}

	// meshesNeedingUpdate_wall.push_back( vec3(0,0,0) );

	regionDataGenerated = true;
}

// TODO(Xavier): (2017.11.29)
// Loading and saving needs to be implemeneted.
void Region::load () { }
void Region::save () { }


///////////////////////////
// SHARED:
void Region::init ( const WindowInfo& window, unsigned int l, unsigned int w, unsigned int h )
{
	// TEMP(Xavier): (2017.11.30)
	// This is here only to test the simulation passing data
	// to the render thread.
	length = l;
	width = w;
	height = h;
	chunk_length = 16;
	chunk_width = 16;
	chunk_height = 16;

	simulationPaused = false;
	regionDataGenerated = false;
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
	selection.info.position = vec3( -1 ); // Set to unused value.
	
	shader = load_shader(
		R"(
			#version 330 core

			layout(location = 0) in vec3 position;
			layout(location = 1) in vec2 textcoord;

			uniform mat4 model;
			uniform mat4 view;
			uniform mat4 projection;

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

	chunkMeshTexture = 0;
	load_texture( &chunkMeshTexture, "res/TileMap.png" );

	projection = orthographic_projection( -window.height/0.2, window.height/0.2, -window.width/0.2, window.width/0.2, 0.1f, 5000.0f );
	camera = translate( mat4(1), -vec3(0, 200, 500) );

	for ( unsigned int i = 0; i < ceil(height/chunk_height); ++i )
	{
		for ( unsigned int j = 0; j < ceil(width/chunk_width); ++j )
		{
			for ( unsigned int k = 0; k < ceil(length/chunk_length); ++k )
			{
				chunkMeshes.emplace_back( vec3(i,j,k) );
			}	
		}
	}
}

void Region::cleanup ()
{
	free( tiles.floor );
	free( tiles.wall );
	free( tiles.water );
	free( tiles.object );
	free( tiles.direction );
}


//////////////////////////////
// RENDERING THREAD:
void Region::render ()
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
					if ( m.floorVertData.size() > 0 ) upload_floor_mesh( m );
					if ( m.wallVertData.size() > 0 ) upload_wall_mesh( m );
					if ( m.waterVertData.size() > 0 ) upload_water_mesh( m );
					if ( m.objectVertData.size() > 0 ) upload_object_mesh( m );
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
					if ( m.floorVertData.size() > 0 ) upload_floor_mesh( m );
					if ( m.wallVertData.size() > 0 ) upload_wall_mesh( m );
					if ( m.waterVertData.size() > 0 ) upload_water_mesh( m );
					if ( m.objectVertData.size() > 0 ) upload_object_mesh( m );
				}
				chunkMeshesToBeUploaded_2.clear();
			}
			renderingUsingUploadQue_2 = false;
		} else { renderingUsingUploadQue_2 = false; }
	}

	// Draw all the chunks to the screen.
	glUseProgram( shader ); GLCALL;
	set_uniform_mat4( shader, "projection", &projection );
	set_uniform_mat4( shader, "view", &camera );
	for ( auto& cm : chunkMeshes )
	{
		if ( cm.floor.vao != 0 )
		{
			auto mtx = translate(mat4(1), vec3(0));
			set_uniform_mat4( shader, "model", &mtx );
			glBindVertexArray( cm.floor.vao ); GLCALL;
			glBindTexture( GL_TEXTURE_2D, chunkMeshTexture ); GLCALL;
			glDrawElements( GL_TRIANGLES, cm.floor.numIndices, GL_UNSIGNED_INT, 0 ); GLCALL;
		}

		if ( cm.wall.vao != 0 )
		{
			auto mtx = translate(mat4(1), vec3(0));
			set_uniform_mat4( shader, "model", &mtx );
			glBindVertexArray( cm.wall.vao ); GLCALL;
			glBindTexture( GL_TEXTURE_2D, chunkMeshTexture ); GLCALL;
			glDrawElements( GL_TRIANGLES, cm.wall.numIndices, GL_UNSIGNED_INT, 0 ); GLCALL;
		}

		if ( cm.water.vao != 0 )
		{
			auto mtx = translate(mat4(1), vec3(0));
			set_uniform_mat4( shader, "model", &mtx );
			glBindVertexArray( cm.water.vao ); GLCALL;
			glBindTexture( GL_TEXTURE_2D, chunkMeshTexture ); GLCALL;
			glDrawElements( GL_TRIANGLES, cm.water.numIndices, GL_UNSIGNED_INT, 0 ); GLCALL;
		}
		
		if ( cm.object.vao != 0 )
		{
			auto mtx = translate(mat4(1), vec3(0));
			set_uniform_mat4( shader, "model", &mtx );
			glBindVertexArray( cm.object.vao ); GLCALL;
			glBindTexture( GL_TEXTURE_2D, chunkMeshTexture ); GLCALL;
			glDrawElements( GL_TRIANGLES, cm.object.numIndices, GL_UNSIGNED_INT, 0 ); GLCALL;
		}
	}
}

void Region::issue_command ( Command_Type command )
{
	if ( simulationUsingCommandQue_1 == false )
	{
		renderingUsingCommandQue_1 = true;
		if ( simulationUsingCommandQue_1 == false )
		{
			commandQue_1.push_back( {command} );
			renderingUsingCommandQue_1 = false;
		} else { renderingUsingCommandQue_1 = false; }
	}
	else if ( simulationUsingCommandQue_2 == false )
	{
		renderingUsingCommandQue_2 = true;
		if ( simulationUsingCommandQue_2 == false )
		{
			commandQue_2.push_back( {command} );
			renderingUsingCommandQue_2 = false;
		} else { renderingUsingCommandQue_2 = false; }
	}
}

void Region::pause_simulation( bool state )
{
	simulationPaused = state;
}

void Region::select_position ( int x, int y, int z )
{
}

void Region::select_area ( int x, int y, int z )
{
}

void Region::cancel_selection ()
{
}

void Region::upload_floor_mesh ( Chunk_Mesh_Data& meshData )
{
	int cl =  ceil(length/chunk_length);
	int cw =  ceil(width/chunk_width);
	auto index = static_cast<unsigned int>(meshData.position.x + meshData.position.y*cl + meshData.position.z*cl*cw);
	auto& cm = chunkMeshes[ index ];

	if ( cm.ageIdentifier_floor <= meshData.ageIdentifier_floor )
	{
		cm.ageIdentifier_floor = meshData.ageIdentifier_floor;
		cm.position = meshData.position;

		if ( cm.floor.vao == 0 ) { glGenVertexArrays( 1, &cm.floor.vao ); GLCALL; }
		if ( cm.floor.vbo == 0 ) { glGenBuffers( 1, &cm.floor.vbo ); GLCALL; }
		if ( cm.floor.ibo == 0 ) { glGenBuffers( 1, &cm.floor.ibo ); GLCALL; }

		glBindVertexArray( cm.floor.vao ); GLCALL;
		
			glBindBuffer( GL_ARRAY_BUFFER, cm.floor.vbo ); GLCALL;
				glBufferData( GL_ARRAY_BUFFER, meshData.floorVertData.size() * sizeof( float ), meshData.floorVertData.data(), GL_DYNAMIC_DRAW ); GLCALL;
			
				GLint posAttrib = glGetAttribLocation( shader, "position" ); GLCALL;
				glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0 ); GLCALL;
				glEnableVertexAttribArray( posAttrib ); GLCALL;

			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, cm.floor.ibo ); GLCALL;
				glBufferData( GL_ELEMENT_ARRAY_BUFFER, meshData.floorIndexData.size() * sizeof(unsigned int), meshData.floorIndexData.data(), GL_DYNAMIC_DRAW ); GLCALL;
		
		cm.floor.numIndices = meshData.floorIndexData.size();

		glBindVertexArray( 0 ); GLCALL;
	}
}

void Region::upload_wall_mesh ( Chunk_Mesh_Data& meshData )
{
	int cl =  ceil(length/chunk_length);
	int cw =  ceil(width/chunk_width);
	auto index = static_cast<unsigned int>(meshData.position.x + meshData.position.y*cl + meshData.position.z*cl*cw);
	auto& cm = chunkMeshes[ index ];

	if ( cm.ageIdentifier_wall <= meshData.ageIdentifier_wall )
	{
		cm.ageIdentifier_wall = meshData.ageIdentifier_wall;
		cm.position = meshData.position;

		if ( cm.wall.vao == 0 ) { glGenVertexArrays( 1, &cm.wall.vao ); GLCALL; }
		if ( cm.wall.vbo == 0 ){  glGenBuffers( 1, &cm.wall.vbo ); GLCALL; }
		if ( cm.wall.ibo == 0 ){  glGenBuffers( 1, &cm.wall.ibo ); GLCALL; }

		glBindVertexArray( cm.wall.vao ); GLCALL;
		
			glBindBuffer( GL_ARRAY_BUFFER, cm.wall.vbo ); GLCALL;
				glBufferData( GL_ARRAY_BUFFER, meshData.wallVertData.size() * sizeof( float ), meshData.wallVertData.data(), GL_DYNAMIC_DRAW ); GLCALL;
			
				GLint posAttrib = glGetAttribLocation( shader, "position" ); GLCALL;
				glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0 ); GLCALL;
				glEnableVertexAttribArray( posAttrib ); GLCALL;

				GLint texAttrib = glGetAttribLocation( shader, "textcoord" ); GLCALL;
				glVertexAttribPointer( texAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)12 ); GLCALL;
				glEnableVertexAttribArray( texAttrib ); GLCALL;

			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, cm.wall.ibo ); GLCALL;
				glBufferData( GL_ELEMENT_ARRAY_BUFFER, meshData.wallIndexData.size() * sizeof(unsigned int), meshData.wallIndexData.data(), GL_DYNAMIC_DRAW ); GLCALL;
		
		cm.wall.numIndices = meshData.wallIndexData.size();

		glBindVertexArray( 0 ); GLCALL;
	}
}

void Region::upload_water_mesh ( Chunk_Mesh_Data& meshData )
{
	int cl =  ceil(length/chunk_length);
	int cw =  ceil(width/chunk_width);
	auto index = static_cast<unsigned int>(meshData.position.x + meshData.position.y*cl + meshData.position.z*cl*cw);
	auto& cm = chunkMeshes[ index ];

	if ( cm.ageIdentifier_water <= meshData.ageIdentifier_water )
	{
		cm.ageIdentifier_water = meshData.ageIdentifier_water;
		cm.position = meshData.position;

		if ( cm.water.vao == 0 ) { glGenVertexArrays( 1, &cm.water.vao ); GLCALL; }
		if ( cm.water.vbo == 0 ) { glGenBuffers( 1, &cm.water.vbo ); GLCALL; }
		if ( cm.water.ibo == 0 ) { glGenBuffers( 1, &cm.water.ibo ); GLCALL; }

		glBindVertexArray( cm.water.vao ); GLCALL;
		
			glBindBuffer( GL_ARRAY_BUFFER, cm.water.vbo ); GLCALL;
				glBufferData( GL_ARRAY_BUFFER, meshData.waterVertData.size() * sizeof( float ), meshData.waterVertData.data(), GL_DYNAMIC_DRAW ); GLCALL;
			
				GLint posAttrib = glGetAttribLocation( shader, "position" ); GLCALL;
				glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0 ); GLCALL;
				glEnableVertexAttribArray( posAttrib ); GLCALL;

			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, cm.water.ibo ); GLCALL;
				glBufferData( GL_ELEMENT_ARRAY_BUFFER, meshData.waterIndexData.size() * sizeof(unsigned int), meshData.waterIndexData.data(), GL_DYNAMIC_DRAW ); GLCALL;
		
		cm.water.numIndices = meshData.waterIndexData.size();

		glBindVertexArray( 0 ); GLCALL;
	}
}

void Region::upload_object_mesh ( Chunk_Mesh_Data& meshData )
{
	int cl =  ceil(length/chunk_length);
	int cw =  ceil(width/chunk_width);
	auto index = static_cast<unsigned int>(meshData.position.x + meshData.position.y*cl + meshData.position.z*cl*cw);
	auto& cm = chunkMeshes[ index ];

	if ( cm.ageIdentifier_object <= meshData.ageIdentifier_object )
	{
		cm.ageIdentifier_object = meshData.ageIdentifier_object;
		cm.position = meshData.position;

		if ( cm.object.vao == 0 ) { glGenVertexArrays( 1, &cm.object.vao ); GLCALL; }
		if ( cm.object.vbo == 0 ) { glGenBuffers( 1, &cm.object.vbo ); GLCALL; }
		if ( cm.object.ibo == 0 ) { glGenBuffers( 1, &cm.object.ibo ); GLCALL; }

		glBindVertexArray( cm.object.vao ); GLCALL;
		
			glBindBuffer( GL_ARRAY_BUFFER, cm.object.vbo ); GLCALL;
				glBufferData( GL_ARRAY_BUFFER, meshData.objectVertData.size() * sizeof( float ), meshData.objectVertData.data(), GL_DYNAMIC_DRAW ); GLCALL;
			
				GLint posAttrib = glGetAttribLocation( shader, "position" ); GLCALL;
				glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0 ); GLCALL;
				glEnableVertexAttribArray( posAttrib ); GLCALL;

			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, cm.object.ibo ); GLCALL;
				glBufferData( GL_ELEMENT_ARRAY_BUFFER, meshData.objectIndexData.size() * sizeof(unsigned int), meshData.objectIndexData.data(), GL_DYNAMIC_DRAW ); GLCALL;
		
		cm.object.numIndices = meshData.objectIndexData.size();

		glBindVertexArray( 0 ); GLCALL;
	}
}

void Region::resize( const WindowInfo& window )
{
	projection = orthographic_projection( -window.height/0.2, window.height/0.2, -window.width/0.2, window.width/0.2, 0.1f, 5000.0f );
	camera = translate( mat4(1), -vec3(0, 200, 500) );
}
