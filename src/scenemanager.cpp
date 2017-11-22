
#include "scenemanager.hpp"
#include "scenes/mainmenu_scene.hpp"
#include "scenes/game_scene.hpp"


///////////////////////////
// Helper functions:
Scene* load_main_scene( const WindowInfo& window )
{
	Scene *result = new MainMenu_Scene;
	result->init( window );
	return result;
}

Scene* load_game_scene( const WindowInfo& window )
{
	Scene *result = new Game_Scene;
	result->init( window );
	return result;
}


Scene *Scene_Manager::mainScene = nullptr;
Scene *Scene_Manager::activeScene = nullptr;
std::atomic<bool> Scene_Manager::shouldUpdate;
std::atomic<int> Scene_Manager::updateRate;


/////////////////////////////////
// Main Thread Methods:
void Scene_Manager::init( const WindowInfo& window )
{
	// Load the main scene:
	mainScene = load_main_scene( window );
	shouldUpdate = false;
	updateRate = 0;
}

void Scene_Manager::exit()
{
	// Cleanup and release all data:
	if ( activeScene != nullptr ) delete activeScene;
	activeScene = nullptr;
	delete mainScene;
	mainScene = nullptr;
}

void Scene_Manager::render_scene( const WindowInfo& window )
{
	if ( activeScene != nullptr ) { activeScene->render( window ); }
	else
	{
		// Because there is no acive scene the main scene is implied to be active:
		if ( mainScene == nullptr ) { /* ERROR: There should be a main scene */ }
		else { mainScene->render( window ); }
	}
}

void Scene_Manager::resize_scene ( const WindowInfo& window )
{
	if ( activeScene != nullptr ) { activeScene->resize( window ); }
	else
	{
		// Because there is no acive scene the main scene is implied to be active:
		if ( mainScene == nullptr ) { /* ERROR: There should be a main scene */ }
		else { mainScene->resize( window ); }
	}
}

void Scene_Manager::input_scene( const WindowInfo& window, InputInfo* input )
{
	if ( activeScene != nullptr ) { activeScene->input( window, input ); }
	else
	{
		// Because there is no acive scene the main scene is implied to be active:
		if ( mainScene == nullptr ) { /* ERROR: There should be a main scene */ }
		else { mainScene->input( window, input ); }
	}
}


////////////////////////////////
// Shared Methods:
void Scene_Manager::change_scene( SceneType scene, const WindowInfo& window )
{
	disable_updating();
	while ( is_updating() ); // Wait until the scene is no longer being updated.
	
	if ( activeScene != nullptr ) delete activeScene;
	activeScene = nullptr;

	if ( scene == SceneType::MainMenu ) { /* Do nothing because the main menu will become active by default */}
	else if ( scene == SceneType::Game )
	{
		activeScene = load_game_scene( window );
		shouldUpdate = true;
		updateRate = 1;
	}
}

bool Scene_Manager::is_updating () { return shouldUpdate; }
void Scene_Manager::disable_updating () { shouldUpdate = false; }	
void Scene_Manager::enable_updating () { shouldUpdate = true; }
void Scene_Manager::set_update_rate ( const int& rate ) { updateRate = rate; }


///////////////////////////////////////
// Logic Thread Methods & Data:
void Scene_Manager::simulate_scene()
{
	if ( shouldUpdate )
	{
		if ( activeScene != nullptr ) { activeScene->simulate(); }
		else
		{
			// Because there is no acive scene this medhod
			// does not need to do anything.
		}
	}
}