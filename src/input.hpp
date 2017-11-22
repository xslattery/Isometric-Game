#ifndef _INPUT_HPP_
#define _INPUT_HPP_

enum class Key
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

enum class Mouse
{
	Left = 0,
	Right = 1,
	Middle = 2,
};

struct InputInfo
{
	#define KEY_COUNT 128
	bool activeKeys[KEY_COUNT];
	bool downKeys[KEY_COUNT];
	bool upKeys[KEY_COUNT];

	#define MOUSE_BUTTON_COUNT 3
	bool activeMouseButtons[KEY_COUNT];
	bool downMouseButtons[KEY_COUNT];
	bool upMouseButtons[KEY_COUNT];

	float mouseX = 0;
	float mouseY = 0;
	float mouseScrollDeltaX = 0;
	float mouseScrollDeltaY = 0;
};

inline bool get_key ( InputInfo *input, Key key )
{
	if ( (unsigned int)key > KEY_COUNT ) return false;
	return input->activeKeys[ (unsigned int)key ];
}

inline bool get_key_down ( InputInfo *input, Key key )
{
	if ( (unsigned int)key > KEY_COUNT ) return false;
	return input->downKeys[ (unsigned int)key ];
}

inline bool get_key_up ( InputInfo *input, Key key )
{
	if ( (unsigned int)key > KEY_COUNT ) return false;
	return input->upKeys[ (unsigned int)key ];
}

inline bool get_mouse ( InputInfo *input, Mouse mouse )
{
	if ( (unsigned int)mouse > MOUSE_BUTTON_COUNT ) return false;
	return input->activeMouseButtons[ (unsigned int)mouse ];
}

inline bool get_mouse_down ( InputInfo *input, Mouse mouse )
{
	if ( (unsigned int)mouse > MOUSE_BUTTON_COUNT ) return false;
	return input->downMouseButtons[ (unsigned int)mouse ];
}

inline bool get_mouse_up ( InputInfo *input, Mouse mouse )
{
	if ( (unsigned int)mouse > MOUSE_BUTTON_COUNT ) return false;
	return input->upMouseButtons[ (unsigned int)mouse ];
}

#endif