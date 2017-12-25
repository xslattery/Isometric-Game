#ifndef _SCENE_GAME_HPP_
#define _SCENE_GAME_HPP_

#include "../text.hpp"
#include "../math/math.hpp"

#include "scene.hpp"
#include "scene_game/region.hpp"

class Game_Scene : public Scene
{
private:
	///////////////////////////
	// Main Thread Data:
	Packed_Glyph_Texture packedGlyphTexture;
	Text_Mesh textMesh = { 0 };
	Text_Mesh generatingTextMesh = { 0 };
	unsigned int shader;
	mat4 projection;
	mat4 camera;

	Region region;

public:
	//////////////////////////
	// Main Thread Methods:
	void init( const WindowInfo& window ) override;
	void render( const WindowInfo& window ) override;
	void resize ( const WindowInfo& window ) override;
	void input( const WindowInfo& window, InputInfo* input ) override;

	////////////////////////////
	// Simulation Thread Methods:
	void simulate() override;

	////////////////////////////
	// Generation Thread Methods:
	bool generate() override;

	////////////////
	// Destructor:
	~Game_Scene() override;
};

#endif