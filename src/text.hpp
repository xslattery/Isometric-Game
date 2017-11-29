#ifndef _TEXT_HPP_
#define _TEXT_HPP_

#include "platform/opengl.hpp"
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
	unsigned int id = 0;

	#define CHARACTER_COUNT 256
	Glyph glyphs[ CHARACTER_COUNT ];

	~Packed_Glyph_Texture();
};

struct Text_Mesh
{
	int crop_width;
	int crop_height;
	int fontsize;

	vec3 position;
	mat4 transform;

	unsigned int texture_id = 0;
	unsigned int vao = 0;
	unsigned int vbo_vertices = 0;
	unsigned int vbo_colors = 0;
	unsigned int ebo = 0;
	unsigned int num_indices = 0;

	~Text_Mesh();
};

void create_packed_glyph_texture( Packed_Glyph_Texture &pgt, const char* filename, FT_Library freeType, unsigned int filter = GL_LINEAR );
void create_text_mesh( const char* text, Text_Mesh &tm, Packed_Glyph_Texture &pgt, unsigned int shader_id );
void render_text_mesh( Text_Mesh &tm, unsigned int shader_id );

#endif