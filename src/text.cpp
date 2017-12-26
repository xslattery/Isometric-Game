
#include "platform/opengl.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <cstddef>
#include <map>
#include <vector>
#include "shader.hpp"

#include "text.hpp"


void create_packed_glyph_texture ( Packed_Glyph_Texture &pgt, const char* filename, FT_Library freeType, unsigned int filter )
{
	// NOTE(Xavier): The max size will be 200 pixels aka 100pt
	if ( pgt.fontsize > 200 ) pgt.fontsize = 200; 
	
	FT_Face face;
	if ( FT_New_Face(freeType, filename, 0, &face) ) {
		std::cout << "FREETYPE: Failed to load font.\n";
		return;
	}
	
	FT_Set_Pixel_Sizes( face, 0, pgt.fontsize );
	if ( FT_Load_Char(face, 'X', FT_LOAD_RENDER) ) {
		std::cout << "FREETYTPE: Failed to load Glyph\n";
		return;
	}
	
	struct Temp_Character {
		vec2 size; 
		vec2 bearing;	
		unsigned int advance;	
		unsigned char *bitmap;
		~Temp_Character() { delete [] bitmap; }
	};
	std::map< unsigned char, Temp_Character > tempGlyphs;

	unsigned char startCharacter = 0;
	unsigned char endCharacter = 128;

	for ( unsigned char c = startCharacter; c <= endCharacter; ++c ) {
		if ( FT_Load_Char(face, c, FT_LOAD_RENDER) ) {
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph";
			continue;
		}

		tempGlyphs.insert( std::pair<unsigned char, Temp_Character>( c, Temp_Character() ) );
		
		tempGlyphs[c].size = vec2( face->glyph->bitmap.width, face->glyph->bitmap.rows );
		tempGlyphs[c].bearing = vec2( face->glyph->bitmap_left, face->glyph->bitmap_top );
		tempGlyphs[c].advance = (unsigned int)face->glyph->advance.x;
		
		unsigned int dimensions = (unsigned int)face->glyph->bitmap.width * (unsigned int)face->glyph->bitmap.rows;
		tempGlyphs[c].bitmap = new unsigned char [ dimensions ]; // TODO(Xavier): (2017.11.20) Allocation may fail, handle this.
		
		memcpy( tempGlyphs[c].bitmap, face->glyph->bitmap.buffer, dimensions );

		pgt.glyphs[c].size = vec2( face->glyph->bitmap.width, face->glyph->bitmap.rows );
		pgt.glyphs[c].bearing = vec2( face->glyph->bitmap_left, face->glyph->bitmap_top );
		pgt.glyphs[c].advance = (unsigned int)face->glyph->advance.x;
	}

	FT_Done_Face( face );

	unsigned int combinedCharacterArea = 0;
	for ( std::size_t i = startCharacter; i <= endCharacter; i++ ) { 
		combinedCharacterArea += tempGlyphs[i].size.x * tempGlyphs[i].size.y; 
	}

	unsigned int recmDim = sqrt( combinedCharacterArea ) * 1.5;
	recmDim = recmDim + (4 - (recmDim%4));
	pgt.width = recmDim;
	pgt.height = recmDim;
	unsigned char *combinedBitmap = new unsigned char [ recmDim * recmDim ](); // TODO(Xavier): (2017.11.20) Allocation may fail, handle this.
	
	unsigned int xx = 0; unsigned int yy = 0;
	for ( std::size_t ch = startCharacter; ch <= endCharacter; ++ch ) {
		if ( tempGlyphs[ch].size.x > 0 ) { 
			if ( xx + tempGlyphs[ch].size.x + 1 > recmDim ) { yy += pgt.fontsize; xx = 0; }
			if ( yy + tempGlyphs[ch].size.y + 1 > recmDim ) { break; }
			
			pgt.glyphs[ch].position = vec2( xx, yy );
			
			for ( std::size_t y = yy; y < yy+tempGlyphs[ch].size.y; y++ ) {
				for ( std::size_t x = xx; x < xx+tempGlyphs[ch].size.x; x++ ) {
					combinedBitmap[ recmDim*y + x ] = tempGlyphs[ch].bitmap[ (std::size_t)tempGlyphs[ch].size.x*(y-yy) + x-xx ];
				}
			}
			xx += tempGlyphs[ch].size.x + 1;
		}
	}	    

	unsigned int tex_id;
	glGenTextures(1, &tex_id);

	glBindTexture( GL_TEXTURE_2D, tex_id ); GLCALL;
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ); GLCALL;
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, recmDim, recmDim, 0, GL_RED, GL_UNSIGNED_BYTE, combinedBitmap ); GLCALL;
	
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ); GLCALL;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE ); GLCALL;
	
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter ); GLCALL;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter ); GLCALL;

	delete [] combinedBitmap;

	pgt.id = tex_id;

}

void create_text_mesh( const char* text, Text_Mesh &tm, Packed_Glyph_Texture &pgt, unsigned int shader )
{

	float scaleFactor = (float)pgt.fontsize / (float)tm.fontsize;

	int lineSpacing = 0;

	std::vector<float> verts;
	std::vector<unsigned char> colors;
	std::vector<unsigned int> indis;

	unsigned int vert_ofst = 0;
	float xx = 0;
	float yy = pgt.glyphs[ '`' ].bearing.y / scaleFactor; // TODO(Xavier): (2017.11.19) make it so it chooses the max bearing of all pgt.glyphs.
	for ( std::size_t i = 0; i < CHARACTER_COUNT; ++i )
		if ( yy < pgt.glyphs[i].bearing.y/scaleFactor )
			yy = pgt.glyphs[i].bearing.y/scaleFactor;

	
	for ( std::size_t i = 0; i < strlen( text ); ++i ) {
		#define TAB_WIDTH 4 // NOTE(Xavier): (2017.11.20) This should probably be customizable.

		unsigned char ch = text[i];
		if ( ch == '\n') { xx = 0; yy += tm.fontsize + lineSpacing; continue;}
		if ( ch == '\t') { xx += TAB_WIDTH * ((int)(pgt.glyphs[' '].advance / scaleFactor) >> 6) - pgt.glyphs[' '].bearing.x / scaleFactor; continue; }
		if ( ch == ' ') { xx += ((int)(pgt.glyphs[' '].advance / scaleFactor) >> 6) - pgt.glyphs[' '].bearing.x / scaleFactor; continue; }

		float xd = pgt.glyphs[ch].size.x / scaleFactor;
		float yd = pgt.glyphs[ch].size.y / scaleFactor;
		float yo = pgt.glyphs[ch].bearing.y / scaleFactor;

		float uv_x = ( 1.0f / pgt.width ) * pgt.glyphs[ ch ].position.x;
		float uv_y = ( 1.0f / pgt.height ) * pgt.glyphs[ ch ].position.y;
		float uv_xd = ( 1.0f / pgt.width ) * ( pgt.glyphs[ ch ].position.x + pgt.glyphs[ ch ].size.x );
		float uv_yd = ( 1.0f / pgt.height ) * ( pgt.glyphs[ ch ].position.y + pgt.glyphs[ ch ].size.y );

		float tmpVertArray[ 20 ] = { 
			xx, 	yy-yo, 		0,		uv_x, 		uv_y,
			xx, 	yy+yd-yo, 	0,		uv_x, 		uv_yd,
			xx+xd, 	yy+yd-yo, 	0,		uv_xd, 		uv_yd,
			xx+xd, 	yy-yo, 		0,		uv_xd, 		uv_y
		};
		verts.insert( verts.end(), tmpVertArray, tmpVertArray + 20 );

		// NOTE(Xavier): This is for in the future when
		// individual pgt.glyphs can have their own colors.

		unsigned char tmpColorArray[ 16 ] = {
			255, 255, 255, 255,
			255, 255, 255, 255,
			255, 255, 255, 255,
			255, 255, 255, 255,
		};
		colors.insert( colors.end(), tmpColorArray, tmpColorArray + 16 );

		// TODO(Xavier): (2017.11.20) Support should be added to choose the winding direction.
		unsigned int tmp_indx_array[ 6 ] = {
			vert_ofst, vert_ofst+1, vert_ofst+2, vert_ofst, vert_ofst+2, vert_ofst+3
		};
		indis.insert( indis.end(),  tmp_indx_array, tmp_indx_array+6 );
		vert_ofst += 4;
		
		xx += ( (int)(pgt.glyphs[ ch ].advance / scaleFactor) >> 6 ) - pgt.glyphs[ ch ].bearing.x / scaleFactor;
	}

	if ( verts.size() > 0 ) {	
		tm.num_indices = indis.size();

		if ( tm.vao == 0 ) glGenVertexArrays( 1, &tm.vao );
		if ( tm.vbo_vertices == 0 ) glGenBuffers( 1, &tm.vbo_vertices );
		if ( tm.vbo_colors == 0 ) glGenBuffers( 1, &tm.vbo_colors );
		if ( tm.ebo == 0 ) glGenBuffers( 1, &tm.ebo );

		glBindVertexArray( tm.vao ); GLCALL;
		
			glBindBuffer( GL_ARRAY_BUFFER, tm.vbo_vertices ); GLCALL;
				glBufferData( GL_ARRAY_BUFFER, verts.size() * sizeof( float ), verts.data(), GL_DYNAMIC_DRAW ); GLCALL;
			
				GLint posAttrib = glGetAttribLocation( shader, "position" ); GLCALL;
				glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0 ); GLCALL;
				glEnableVertexAttribArray( posAttrib ); GLCALL;
				
				GLint texAttrib = glGetAttribLocation( shader, "texcoord" ); GLCALL;
				glVertexAttribPointer( texAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)) ); GLCALL;
				glEnableVertexAttribArray( texAttrib ); GLCALL;
			
			glBindBuffer( GL_ARRAY_BUFFER, tm.vbo_colors ); GLCALL;
				glBufferData( GL_ARRAY_BUFFER, colors.size() * sizeof(unsigned char), colors.data(), GL_DYNAMIC_DRAW ); GLCALL;
				
				GLint colorAttrib = glGetAttribLocation( shader, "color" ); GLCALL;
				glVertexAttribPointer( colorAttrib, 4, GL_UNSIGNED_BYTE, GL_TRUE, 4 * sizeof(unsigned char), (void*)0 ); GLCALL;
				glEnableVertexAttribArray( colorAttrib ); GLCALL;

			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, tm.ebo ); GLCALL;
				glBufferData( GL_ELEMENT_ARRAY_BUFFER, indis.size() * sizeof(unsigned int), indis.data(), GL_DYNAMIC_DRAW ); GLCALL;
		
		glBindVertexArray( 0 ); GLCALL;

		tm.texture_id = pgt.id;
	}

}

void render_text_mesh( Text_Mesh &tm, unsigned int shader )
{
	set_uniform_mat4( shader, "model", &tm.transform );
	glBindTexture( GL_TEXTURE_2D, tm.texture_id ); GLCALL;
	glBindVertexArray( tm.vao ); GLCALL;
	glDrawElements( GL_TRIANGLES, tm.num_indices, GL_UNSIGNED_INT, 0 ); GLCALL;
}

Packed_Glyph_Texture::~Packed_Glyph_Texture()
{
	if ( id != 0 ) glDeleteTextures( 1, &id );
}

Text_Mesh::~Text_Mesh()
{
	if ( vao != 0 ) glDeleteVertexArrays( 1, &vao );
	if ( vbo_vertices != 0 ) glDeleteBuffers( 1, &vbo_vertices );
	if ( vbo_colors != 0 ) glDeleteBuffers( 1, &vbo_colors );
	if ( ebo != 0 ) glDeleteBuffers( 1, &ebo );
}