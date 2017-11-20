#ifndef _TEXT_HPP_
#define _TEXT_HPP_

#include "opengl.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H

#include "math.hpp"

struct Glyph 
{
	vec2 position;
	vec2 size;
	vec2 bearing;
	unsigned int advance;
};

struct Packed_Glyph_Texture
{
	int width;
	int height;
	int fontsize;
	unsigned int id;

	#define CHARACTER_COUNT 256
	Glyph glyphs[ CHARACTER_COUNT ];
};

struct Text_Mesh
{
	int crop_width;
	int crop_height;
	int fontsize;

	vec3 position;
	mat4 transform;

	unsigned int texture_id;
	unsigned int vao;
	unsigned int vbo_vertices;
	unsigned int vbo_colors;
	unsigned int ebo;
	unsigned int num_indices;
};

void create_packed_glyph_texture( Packed_Glyph_Texture &pgt, const char* filename, FT_Library freeType, unsigned int filter = GL_LINEAR );
void create_text_mesh( const char* text, Text_Mesh &tm, Packed_Glyph_Texture &pgt, unsigned int shader_id );
void render_text_mesh( Text_Mesh &tm, unsigned int shader_id );

#endif