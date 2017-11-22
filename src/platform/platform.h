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

////////////////////////////
enum Keys
{
	Key_A = 0,
	Key_B = 11,
	Key_C = 8,
	Key_D = 2,
	Key_E = 14,
	Key_F = 3,
	Key_G = 5,
	Key_H = 4,
	Key_I = 34,
	Key_J = 38,
	Key_K = 40,
	Key_L = 37,
	Key_M = 46,
	Key_N = 45,
	Key_O = 31,
	Key_P = 35,
	Key_Q = 12,
	Key_R = 15,
	Key_S = 1,
	Key_T = 17,
	Key_U = 32,
	Key_V = 9,
	Key_W = 13,
	Key_X = 7,
	Key_Y = 16,
	Key_Z = 6,
	
	Key_0 = 29,
	Key_1 = 18,
	Key_2 = 19,
	Key_3 = 20,
	Key_4 = 21,
	Key_5 = 23,
	Key_6 = 22,
	Key_7 = 26,
	Key_8 = 28,
	Key_9 = 25,

	Key_SPACE = 49,
	Key_COMMAND = 55,
	Key_OPTION = 58,
	Key_CONTROL = 59,
	Key_SHIFT = 56,
	Key_RETURN = 36,
	Key_DELETE = 51,
	Key_TAB = 48,
};

#endif