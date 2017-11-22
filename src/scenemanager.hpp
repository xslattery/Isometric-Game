#ifndef _SCENE_MANAGER_HPP_
#define _SCENE_MANAGER_HPP_

#include <atomic>
#include "scenes/scene.hpp"

enum class SceneType
{
	MainMenu,
	Game
};


//////////////////////////
// Helper functions:
Scene* load_main_scene( const WindowInfo& window );
Scene* load_game_scene( const WindowInfo& window );


class Scene_Manager
{
public:
	// Shared Data:
	static Scene *mainScene;
	static Scene *activeScene; // IF nullprt mainScene as the acive scene is implied.
	static std::atomic<bool> shouldUpdate;
	static std::atomic<int> updateRate;

	////////////////////////////////////////////////////////////
	// Main Thread Methods:
	static void init ( const WindowInfo& window );
	static void exit ();
	static void render_scene ( const WindowInfo& window );
	static void resize_scene ( const WindowInfo& window );
	static void input_scene ( const WindowInfo& window, const InputInfo& input );

	////////////////////////////////////////////////////////////
	// Shared Methods:
	static void change_scene ( SceneType scene, const WindowInfo& window );
	static bool is_updating ();
	static void disable_updating ();
	static void enable_updating ();
	static void set_update_rate ( const int& rate );

	////////////////////////////////////////////////////////////
	// Logic Thread Methods:
	static void update_scene ();

};

#endif