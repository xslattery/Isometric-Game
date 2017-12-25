
#include <vector>
#include "shader.hpp"

#include "debug.hpp"

namespace Debug
{
	static unsigned int debugLine_shader = 0;
	static void load_debug_line_shader ();

	static bool debugGridMeshGenerated = false;
	static unsigned int debugGrid_vao = 0;
	static unsigned int debugGrid_vbo = 0;
	static unsigned int debugGrid_ibo = 0;
	static unsigned int debugGrid_indexCount = 0;
	static void generate_debug_grid_mesh ( Region *region );
	static void render_debug_grid_mesh ( float verticalOffset );

	static bool debugChunkOutlineMeshGenerated = false;
	static unsigned int debugChunkOutline_vao = 0;
	static unsigned int debugChunkOutline_vbo = 0;
	static unsigned int debugChunkOutline_ibo = 0;
	static unsigned int debugChunkOutline_indexCount = 0;
	static void generate_debug_chunk_outline_mesh ( Region *region );
	static void render_debug_chunk_outline_mesh ();

	void draw_region_layer_grid ( Region *region )
	{
		if ( debugLine_shader == 0 ) load_debug_line_shader();

		glDisable( GL_DEPTH_TEST ); GLCALL;
		glUseProgram( debugLine_shader ); GLCALL;
		set_uniform_mat4( debugLine_shader, "projection", &region->projection );
		set_uniform_mat4( debugLine_shader, "view", &region->camera );
		
		if ( debugGridMeshGenerated )
		{
			render_debug_grid_mesh( region->viewHeight );
		}
		else
		{
			generate_debug_grid_mesh( region );
			debugGridMeshGenerated = true;
		}

		glEnable( GL_DEPTH_TEST );
	}

	void draw_region_chunk_grid ( Region *region )
	{
		if ( debugLine_shader == 0 ) load_debug_line_shader();

		glDisable( GL_DEPTH_TEST ); GLCALL;
		glUseProgram( debugLine_shader ); GLCALL;
		set_uniform_mat4( debugLine_shader, "projection", &region->projection );
		set_uniform_mat4( debugLine_shader, "view", &region->camera );

		if ( debugChunkOutlineMeshGenerated )
		{
			render_debug_chunk_outline_mesh();
		}
		else
		{
			generate_debug_chunk_outline_mesh( region );
			debugChunkOutlineMeshGenerated = true;
		}

		glEnable( GL_DEPTH_TEST );
	}

	static void load_debug_line_shader ()
	{
		debugLine_shader = load_shader(
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

	static void generate_debug_grid_mesh ( Region *region )
	{
		std::vector<vec2> vertexData;
		std::vector<unsigned int> indexData;

		const vec2 xDir { -1, 18.0f/27.0f };
		const vec2 yDir {  1, 18.0f/27.0f };

		unsigned int curIndex = 0;
		for ( int i = 0; i <= region->worldLength; ++i )
		{
			vertexData.emplace_back( xDir*i*27 );
			vertexData.emplace_back( xDir*i*27 + yDir*region->worldWidth*27 );

			indexData.emplace_back( curIndex );
			indexData.emplace_back( curIndex + 1 );
			curIndex += 2;
		}
		for ( int i = 0; i <= region->worldWidth; ++i )
		{
			vertexData.emplace_back( yDir*i*27 );
			vertexData.emplace_back( yDir*i*27 + xDir*region->worldLength*27 );

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
		set_uniform_mat4( debugLine_shader, "model", &mtx );
		set_uniform_vec4( debugLine_shader, "color", vec4(1, 0, 0, 1) );
		glBindVertexArray( debugGrid_vao ); GLCALL;
		glDrawElements( GL_LINES, debugGrid_indexCount, GL_UNSIGNED_INT, 0 ); GLCALL;
		mtx = translate(mat4(1), vec3(0, verticalOffset*30+7, 0));
		set_uniform_mat4( debugLine_shader, "model", &mtx );
		set_uniform_vec4( debugLine_shader, "color", vec4(1, 1, 0, 1) );
		glDrawElements( GL_LINES, debugGrid_indexCount, GL_UNSIGNED_INT, 0 ); GLCALL;
		mtx = translate(mat4(1), vec3(0, verticalOffset*30+30, 0));
		set_uniform_mat4( debugLine_shader, "model", &mtx );
		set_uniform_vec4( debugLine_shader, "color", vec4(1, 0, 1, 1) );
		glDrawElements( GL_LINES, debugGrid_indexCount, GL_UNSIGNED_INT, 0 ); GLCALL;
	}

	static void generate_debug_chunk_outline_mesh ( Region *region )
	{
		std::vector<vec2> vertexData;
		std::vector<unsigned int> indexData;

		const vec2 xDir { -1, 18.0f/27.0f };
		const vec2 yDir {  1, 18.0f/27.0f };

		unsigned int curIndex = 0;
		for ( int i = 0; i < region->length; ++i )
		{
			for ( int j = 0; j < region->width; ++j )
			{
				for ( int k = 0; k < region->height; ++k )
				{
					vertexData.emplace_back( yDir*region->chunkWidth*j*27 + xDir*region->chunkWidth*i*27 + vec2(0, region->chunkHeight*k*30) );
					vertexData.emplace_back( xDir*region->chunkLength*27 + yDir*region->chunkWidth*j*27 + xDir*region->chunkWidth*i*27 + vec2(0, region->chunkHeight*k*30) );
					vertexData.emplace_back( yDir*region->chunkWidth*27 + yDir*region->chunkWidth*j*27 + xDir*region->chunkWidth*i*27 + vec2(0, region->chunkHeight*k*30) );
					vertexData.emplace_back( xDir*region->chunkLength*27 + yDir*region->chunkWidth*27 + yDir*region->chunkWidth*j*27 + xDir*region->chunkWidth*i*27 + vec2(0, region->chunkHeight*k*30) );
					vertexData.emplace_back( vec2(0, region->chunkHeight*30) + yDir*region->chunkWidth*j*27 + xDir*region->chunkWidth*i*27 + vec2(0, region->chunkHeight*k*30) );
					vertexData.emplace_back( xDir*region->chunkLength*27 + vec2(0, region->chunkHeight*30) + yDir*region->chunkWidth*j*27 + xDir*region->chunkWidth*i*27 + vec2(0, region->chunkHeight*k*30) );
					vertexData.emplace_back( yDir*region->chunkWidth*27 + vec2(0, region->chunkHeight*30) + yDir*region->chunkWidth*j*27 + xDir*region->chunkWidth*i*27 + vec2(0, region->chunkHeight*k*30) );
					vertexData.emplace_back( xDir*region->chunkLength*27 + yDir*region->chunkWidth*27 + vec2(0, region->chunkHeight*30) + yDir*region->chunkWidth*j*27 + xDir*region->chunkWidth*i*27 + vec2(0, region->chunkHeight*k*30) );

					indexData.emplace_back( curIndex );
					indexData.emplace_back( curIndex + 1 );
					indexData.emplace_back( curIndex );
					indexData.emplace_back( curIndex + 2 );
					indexData.emplace_back( curIndex + 1 );
					indexData.emplace_back( curIndex + 3 );
					indexData.emplace_back( curIndex + 2 );
					indexData.emplace_back( curIndex + 3 );
					indexData.emplace_back( curIndex + 4 );
					indexData.emplace_back( curIndex + 5 );
					indexData.emplace_back( curIndex + 4);
					indexData.emplace_back( curIndex + 6 );
					indexData.emplace_back( curIndex + 5 );
					indexData.emplace_back( curIndex + 7 );
					indexData.emplace_back( curIndex + 6 );
					indexData.emplace_back( curIndex + 7 );

					indexData.emplace_back( curIndex );
					indexData.emplace_back( curIndex + 4 );
					indexData.emplace_back( curIndex + 1 );
					indexData.emplace_back( curIndex + 5 );
					indexData.emplace_back( curIndex + 2 );
					indexData.emplace_back( curIndex + 6 );
					indexData.emplace_back( curIndex + 3 );
					indexData.emplace_back( curIndex + 7 );

					curIndex += 8;
				}
			}
		}

		if ( debugChunkOutline_vao == 0 ) { glGenVertexArrays( 1, &debugChunkOutline_vao ); GLCALL; }
		if ( debugChunkOutline_vbo == 0 ) { glGenBuffers( 1, &debugChunkOutline_vbo ); GLCALL; }
		if ( debugChunkOutline_ibo == 0 ) { glGenBuffers( 1, &debugChunkOutline_ibo ); GLCALL; }

		glBindVertexArray( debugChunkOutline_vao ); GLCALL;
			
				glBindBuffer( GL_ARRAY_BUFFER, debugChunkOutline_vbo ); GLCALL;
					glBufferData( GL_ARRAY_BUFFER, vertexData.size() * sizeof( vec2 ), vertexData.data(), GL_STATIC_DRAW ); GLCALL;
				
					GLint posAttrib = glGetAttribLocation( region->shader, "position" ); GLCALL;
					glVertexAttribPointer( posAttrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0 ); GLCALL;
					glEnableVertexAttribArray( posAttrib ); GLCALL;

				glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, debugChunkOutline_ibo ); GLCALL;
					glBufferData( GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(unsigned int), indexData.data(), GL_STATIC_DRAW ); GLCALL;
			
			debugChunkOutline_indexCount = indexData.size();

		glBindVertexArray( 0 ); GLCALL;
	}

	static void render_debug_chunk_outline_mesh ()
	{
		auto mtx = translate(mat4(1), vec3(0, 0, 0));
		set_uniform_mat4( debugLine_shader, "model", &mtx );
		set_uniform_vec4( debugLine_shader, "color", vec4(1, 0, 0, 1) );
		glBindVertexArray( debugChunkOutline_vao ); GLCALL;
		glDrawElements( GL_LINES, debugChunkOutline_indexCount, GL_UNSIGNED_INT, 0 ); GLCALL;
	}
}