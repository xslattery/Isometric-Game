#ifndef _SCENE_HPP_
#define _SCENE_HPP_

#include "../platform/platform.h"

class Scene
{
public:
	// Main Thread Methods & Data:
	virtual void init( const WindowInfo& window ) = 0;
	virtual void render( const WindowInfo& window ) = 0;
	virtual void resize ( const WindowInfo& window ) = 0;
	virtual void input( const WindowInfo& window, const InputInfo& input ) = 0;

	// Logic Thread Methods & Data:
	virtual void update() = 0;

	// Destructor:
	virtual ~Scene() {}
};

#endif