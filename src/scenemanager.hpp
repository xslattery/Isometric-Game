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
	static Scene *activeScene; // If nullprt mainScene is implied to be the acive scene.
	static std::atomic<bool> simulationShouldUpdate;
	static std::atomic<bool> simulationStoppedUpdating;
	static std::atomic<bool> generationShouldUpdate;
	static std::atomic<bool> generationStoppedUpdating;

	////////////////////////////////////////////////////////////
	// Main Thread Methods:
	static void init ( const WindowInfo& window );
	static void exit ();
	static void render_scene ( const WindowInfo& window );
	static void resize_scene ( const WindowInfo& window );
	static void input_scene ( const WindowInfo& window, InputInfo* input );

	////////////////////////////////////////////////////////////
	// Shared Methods:
	static void change_scene ( SceneType scene, const WindowInfo& window );
	static bool is_simulation_updating ();
	static bool is_generation_updating ();
	static void disable_updating ();
	static void enable_updating ();
	static void set_update_rate ( const int& rate );

	////////////////////////////////////////////////////////////
	// Simulation Thread Methods:
	static void simulate_scene ();

	////////////////////////////////////////////////////////////
	// Generation Thread Methods:
	static bool generate_scene ();

};

#endif