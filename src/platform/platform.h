#ifndef _GAME_MANAGER_H_
#define _GAME_MANAGER_H_

////////////////////////////
struct WindowInfo;
struct InputInfo;
enum class InputType;
enum class MouseButton;


////////////////////////////
// NOTE(Xavier): (2017.11.15) These are calls the
// platform layer makes to the crossplatform layer:
void init ( const WindowInfo& window );
void input ( const WindowInfo& window, const InputInfo& input );
void render ( const WindowInfo& window );
void resize ( const WindowInfo& window );
void cleanup ( const WindowInfo& window );


////////////////////////////
// NOTE(Xavier): (2017.11.15) Thses are calls the 
// crossplatform layer makes to the platform layer:
void close_window ();


////////////////////////////
// NOTE(Xavier): (2017.11.15) This is the window info struct. Its purpose 
// is to be passed by argument to the crossplatform layer from the platform
// layer, providing information about platform speciffic information.
struct WindowInfo
{
	float width;
	float height;
	float hidpi_width;
	float hidpi_height;

	float deltaTime;
};

////////////////////////////
enum class InputType
{
	None,	
	KeyDown,
	KeyUp,
	MouseMove,
	MouseDrag,
	MouseScroll,
	MouseDown,
	MouseUp,
	MouseEnter,
	MouseExit,
};

////////////////////////////
enum class MouseButton
{
	None,
	Left,
	Right,
	Middle,
};

////////////////////////////
struct InputInfo
{
	InputType type = InputType::None;
	MouseButton mouseButton = MouseButton::None;
	float mouseX = 0;
	float mouseY = 0;
	float mouseScrollDeltaX = 0;
	float mouseScrollDeltaY = 0;
	unsigned short keyCode = 0; // TODO(Xavier): (2017.11.15) This will probably have to change because '0' is 'a'.
};

#endif