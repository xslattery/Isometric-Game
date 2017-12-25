#ifndef _SCENE_MAIN_MENU_HPP_
#define _SCENE_MAIN_MENU_HPP_

#include "../text.hpp"
#include "../math/math.hpp"

#include "scene.hpp"

class MainMenu_Scene : public Scene
{
private:
	///////////////////////////
	// Main Thread Data:
	Packed_Glyph_Texture packedGlyphTexture;
	Text_Mesh textMesh = { 0 };
	unsigned int shader;
	mat4 projection;
	mat4 camera;

public:
	/////////////////////////
	// Main Thread Methods:
	void init( const WindowInfo& window ) override;
	void render( const WindowInfo& window ) override;
	void resize ( const WindowInfo& window ) override;
	void input( const WindowInfo& window, InputInfo* input ) override;

	///////////////////////////
	// Simulation Thread Methods:
	void simulate() override;

	///////////////////////////
	// Generation Thread Methods:
	bool generate() override;

	////////////////
	// Destructor:
	~MainMenu_Scene() override;
};

#endif