#ifndef _SCENE_HPP_
#define _SCENE_HPP_

#include "../platform/platform.h"

class Scene
{
public:
	/////////////////////////////////
	// Main Thread Methods:
	virtual void init( const WindowInfo& window ) = 0;
	virtual void render( const WindowInfo& window ) = 0;
	virtual void resize ( const WindowInfo& window ) = 0;
	virtual void input( const WindowInfo& window, InputInfo* input ) = 0;

	//////////////////////////////////
	// Simulation Thread Methods:
	virtual void simulate() = 0;

	//////////////////////////////////
	// Generation Thread Methods:
	virtual bool generate() = 0;

	/////////////////
	// Destructor:
	virtual ~Scene() {}
};

#endif