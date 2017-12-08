#ifndef _MAIN_MENU_SCENE_HPP_
#define _MAIN_MENU_SCENE_HPP_

#include "../text.hpp"
#include "../math.hpp"

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
	// Logic Thread Methods:
	void simulate() override;

	////////////////
	// Destructor:
	~MainMenu_Scene() override;
};

#endif