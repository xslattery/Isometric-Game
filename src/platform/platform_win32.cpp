#define WIN32_LEAN_AND_MEAN // Exclude rarely-used items from the windows headers
#include <Windows.h>
#include <windowsx.h>
#include "opengl.hpp"

#include "platform.h"

static WindowInfo windowInfo;
static InputInfo inputInfo;
static bool running;

static HGLRC openglContext;
static HDC deviceContextHandle;

void init_win32 ( HWND windowHandle )
{
	RECT rect;
	GetClientRect( windowHandle, &rect );
	windowInfo.width = rect.right;
	windowInfo.height = rect.bottom;
	windowInfo.hidpi_width = rect.right;
	windowInfo.hidpi_height = rect.bottom;
	windowInfo.deltaTime = 0;
	init( windowInfo );
}

LRESULT CALLBACK WndProc ( HWND, UINT, WPARAM, LPARAM );

int APIENTRY WinMain ( HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow )
{
	printf( "Hello World\n");

	// Main window class name:
	LPCTSTR szWindowClass = "GLTitle";

	WNDCLASS windowClass;
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
	windowClass.lpfnWndProc = WndProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = hInstance;
	windowClass.hIcon = LoadIcon( hInstance, MAKEINTRESOURCE(IDI_APPLICATION) );
	windowClass.hCursor = LoadCursor( NULL, IDC_ARROW );
	windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = szWindowClass;

	RegisterClass( &windowClass );

	// Title bar text:
	LPCTSTR szTitle = "Win32 OpenGL Window";

	HWND windowHandle = CreateWindow( szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 400, NULL, NULL, hInstance, NULL );
	if ( !windowHandle ) return false;

	ShowWindow( windowHandle, nCmdShow );
	UpdateWindow( windowHandle );

	deviceContextHandle = GetDC( windowHandle );

	
	// Main Loop:
	running = true;
	while ( running ) {
		// Message loop:
		MSG message = { };
		while ( PeekMessage(&message, 0, 0, 0, PM_REMOVE) ) {
			if ( message.message == WM_QUIT || message.message == WM_CLOSE ) {
				running = false;
				// MessageBox( NULL, "Hi", "There", MB_OK );
			}
			TranslateMessage( &message );
			DispatchMessage( &message );
		}

		wglMakeCurrent( deviceContextHandle, openglContext );
		input_and_render ( windowInfo, &inputInfo );
		SwapBuffers( deviceContextHandle );
		wglMakeCurrent( NULL, NULL );
	}


	return 0;
}

HGLRC SetupGLContext ( HWND windowHandle, HDC localDeviceContextHandle )
{
	
	PIXELFORMATDESCRIPTOR desiredPixelFormat = {
		sizeof(PIXELFORMATDESCRIPTOR),								// Size
		1,															// Version
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,	// Flags
		PFD_TYPE_RGBA, 												// Framebuffer Kind
		32, 														// Color depth
		0, 0, 0, 0, 0, 0, 0, 0,										// Color channels bits & shifts
		0, 															// Accum bits
		0, 0, 0, 0,													// Accum channels bits
		24,															// Depthbuffer
		8,															// Stencilbuffer
		0,															// Aux buffers
		PFD_MAIN_PLANE,												// Layer type
		0,															// Reserved
		0, 0, 0														// Layer, Visible & Damage masks
	};

	int pixelFormat = ChoosePixelFormat( localDeviceContextHandle, &desiredPixelFormat );
	SetPixelFormat( localDeviceContextHandle, pixelFormat, &desiredPixelFormat );
	
	HGLRC tempOpenglContext = wglCreateContext( localDeviceContextHandle );
	wglMakeCurrent( localDeviceContextHandle, tempOpenglContext );

	// TODO(Xavier): (2017.11.29)
	// Error detection needs to be setup
	// for example glewInit() may fail int addition to
	// some other setup function calls.
	glewInit();

	int attributes[] =  {
	    WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
	    WGL_CONTEXT_MINOR_VERSION_ARB, 3,
	    WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
	    0
	};
	
	HGLRC localOpenglContext = wglCreateContextAttribsARB( localDeviceContextHandle, NULL, attributes );
	wglMakeCurrent( NULL, NULL );
	wglDeleteContext( tempOpenglContext );
	wglMakeCurrent( localDeviceContextHandle, localOpenglContext );

	// Set VSync On(1) / Off(0):
	wglSwapIntervalEXT( 1 );

	wglMakeCurrent( NULL, NULL );
	
	return localOpenglContext;
}

void OnLButtonDown ( HWND windowHandle, bool bDoubleClicked, int xPos, int yPos, UINT keyFlags )
{
}

void OnLButtonUp ( HWND windowHandle, int x, int y, UINT )
{
}

void OnLButtonDblClk ( HWND windowHandle, bool, int x, int y, UINT )
{
}

void OnRButtonDown ( HWND windowHandle, bool, int x, int y, UINT )
{
}

void OnRButtonUp ( HWND windowHandle, int x, int y, UINT )
{
}

void OnMouseMove ( HWND windowHandle, int x, int y, UINT keyFlags )
{
}

void OnMouseWheel ( HWND windowHandle, int xPos, int yPos, int zDelta, UINT )
{
}

void OnChar ( HWND windowHandle, TCHAR character, int )
{
	switch ( character ) {
		case 'q': case 'Q':
			PostMessage(windowHandle, WM_CLOSE, 0, 0);
			break;
		
		default: break;
	}
}

LRESULT CALLBACK WndProc ( HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch ( message ) {
		case WM_CREATE:
			deviceContextHandle = GetDC( windowHandle );
			openglContext = SetupGLContext( windowHandle, deviceContextHandle );
			wglMakeCurrent( deviceContextHandle, openglContext );
			init_win32( windowHandle );
			wglMakeCurrent( NULL, NULL );
			break;
		
		case WM_QUIT: case WM_DESTROY:
			running = false;
			cleanup( windowInfo );
			wglDeleteContext( openglContext );
			ReleaseDC( windowHandle, deviceContextHandle );
			PostQuitMessage( 0 );
			break;
		
		case WM_PAINT:
			wglMakeCurrent( deviceContextHandle, openglContext );
			input_and_render ( windowInfo, &inputInfo );
			SwapBuffers( deviceContextHandle );
			wglMakeCurrent( NULL, NULL );
			ValidateRect( windowHandle, NULL );
			break;

		case WM_SIZE:
			wglMakeCurrent( deviceContextHandle, openglContext );
			RECT rect;
			GetClientRect( windowHandle, &rect );
			windowInfo.width = rect.right;
			windowInfo.height = rect.bottom;
			windowInfo.hidpi_width = rect.right;
			windowInfo.hidpi_height = rect.bottom;
			resize ( windowInfo );
			wglMakeCurrent( NULL, NULL );
			break;

		case WM_ERASEBKGND: break;

		case WM_LBUTTONDOWN:
			HANDLE_WM_LBUTTONDOWN( windowHandle, wParam, lParam, OnLButtonDown );
			break;

		case WM_LBUTTONUP:
			HANDLE_WM_LBUTTONUP( windowHandle, wParam, lParam, OnLButtonUp );
			break;
		
		case WM_LBUTTONDBLCLK:
			HANDLE_WM_LBUTTONDBLCLK( windowHandle, wParam, lParam, OnLButtonDblClk );
			break;

		case WM_RBUTTONDOWN:
			HANDLE_WM_RBUTTONDOWN( windowHandle, wParam, lParam, OnRButtonDown );
			break;

		case WM_RBUTTONUP:
			HANDLE_WM_RBUTTONUP( windowHandle, wParam, lParam, OnRButtonUp );
			break;

		case WM_MOUSEMOVE:
			HANDLE_WM_MOUSEMOVE( windowHandle, wParam, lParam, OnMouseMove );
			break;

		case WM_MOUSEWHEEL:
			HANDLE_WM_MOUSEWHEEL( windowHandle, wParam, lParam, OnMouseWheel );
			break;

		case WM_CHAR:
			HANDLE_WM_CHAR( windowHandle, wParam, lParam, OnChar );
			break;

		default: return DefWindowProc( windowHandle, message, wParam, lParam );
	}

	return 0;
}