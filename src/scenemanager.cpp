
#include "scenemanager.hpp"
#include "scenes/scene_mainmenu.hpp"
#include "scenes/scene_game.hpp"


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
std::atomic<bool> Scene_Manager::simulationShouldUpdate;
std::atomic<bool> Scene_Manager::simulationStoppedUpdating;
std::atomic<bool> Scene_Manager::generationShouldUpdate;
std::atomic<bool> Scene_Manager::generationStoppedUpdating;


/////////////////////////////////
// Main Thread Methods:
void Scene_Manager::init( const WindowInfo& window )
{
	// Load the main scene:
	mainScene = load_main_scene( window );
	simulationShouldUpdate = false;
	generationShouldUpdate = false;
	simulationStoppedUpdating = false;
	generationStoppedUpdating = false;
}

void Scene_Manager::exit()
{
	disable_updating();
	while ( !simulationStoppedUpdating ); // Wait until the scene is no longer being simulated.
	while ( !generationStoppedUpdating ); // Wait until the scene is no longer being generated.
	
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
	while ( !simulationStoppedUpdating ); // Wait until the scene is no longer being simulated.
	while ( !generationStoppedUpdating ); // Wait until the scene is no longer being generated.
	
	if ( activeScene != nullptr ) delete activeScene;
	activeScene = nullptr;

	if ( scene == SceneType::MainMenu ) { /* Do nothing because the main menu will become active by default */}
	else if ( scene == SceneType::Game )
	{
		activeScene = load_game_scene( window );
		simulationShouldUpdate = true;
		generationShouldUpdate = true;
	}
}

bool Scene_Manager::is_simulation_updating () { return simulationShouldUpdate; }
bool Scene_Manager::is_generation_updating () { return generationShouldUpdate; }
void Scene_Manager::disable_updating () { simulationShouldUpdate = false; generationShouldUpdate = false; }	
void Scene_Manager::enable_updating () { simulationShouldUpdate = true; generationShouldUpdate = true; }

///////////////////////////////////////
// Simulation Thread:
void Scene_Manager::simulate_scene()
{
	if ( simulationShouldUpdate )
	{
		simulationStoppedUpdating = false;
		if ( activeScene != nullptr ) { activeScene->simulate(); }
		else
		{
			// NOTE(Xavier): (2017.12.4)
			// Because there is no acive scene this medhod
			// does not need to do anything.
		}
	}
	else
	{
		simulationStoppedUpdating = true;
	}
}

///////////////////////////////////////
// Generation Thread:
bool Scene_Manager::generate_scene()
{
	if ( generationShouldUpdate )
	{
		generationStoppedUpdating = false;
		if ( activeScene != nullptr )
		{ 
			return activeScene->generate();
		}
	}
	else
	{
		generationStoppedUpdating = true;
	}

	return false;
}