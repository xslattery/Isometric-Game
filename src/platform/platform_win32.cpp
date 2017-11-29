#define WIN32_LEAN_AND_MEAN // Exclude rarely-used items from the windows headers
#include <Windows.h>
#include <windowsx.h> // NOTE(Xavier): (2017.11.28) This may nto be needed.
#include <gl\GL.h> // NOTE(Xavier): (2017.11.28) This should be updated to opengl 3.3+

// http://dantefalcone.name/tutorials/1a-windows-win32-window-and-3d-context-creation/

void Init()
{
	glEnable( GL_DEPTH_TEST );
	glShadeModel( GL_SMOOTH );
	glClearColor( 0.0f, 1.0f, 1.0f, 1.0f );
}

void Paint()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}


LRESULT CALLBACK WndProc ( HWND, UINT, WPARAM, LPARAM );

int APIENTRY WinMain ( HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow )
{
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
	LPCTSTR szTitle = "GL";

	HWND windowHandle = CreateWindow( szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL );
	if ( !windowHandle ) return false;

	ShowWindow( windowHandle, nCmdShow );
	UpdateWindow( windowHandle );

	// Message loop:
	MSG message = { };
	while ( GetMessage(&message, NULL, 0, 0) )
	{
		TranslateMessage( &message );
		DispatchMessage( &message );
	}

	return static_cast<int>(message.wParam);
}

HGLRC SetupGLContext ( HWND windowHandle, HDC deviceContextHandle )
{
	PIXELFORMATDESCRIPTOR desiredPixelFormat =
	{
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

	int pixelFormat = ChoosePixelFormat( deviceContextHandle, &desiredPixelFormat );
	SetPixelFormat( deviceContextHandle, pixelFormat, &desiredPixelFormat );

	// TODO(Xavier): (2017.11.28)
	// Set the OpenGL context to 3.3+
	
	HGLRC openglContext = wglCreateContext( deviceContextHandle );
	wglMakeCurrent( deviceContextHandle, openglContext );
	
	// NOTE(Xavier): (2017.11.28)
	// This may not be necessary:
	SetWindowTextA( windowHandle, static_cast<char *>(glGetString(GL_VERSION)) );
	
	Init();
	
	// NOTE(Xavier): (2017.11.28)
	// This may not be necessary:
	SelectObject( deviceContextHandle, GetStockObject(SYSTEM_FONT) );
	wglUseFontBitmaps( deviceContextHandle, 0, 255, 1000 );
	
	wglMakeCurrent( NULL, NULL );
	
	return openglContext;
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
	switch ( character )
	{
		case 'q': case 'Q':
			PostMessage(windowHandle, WM_CLOSE, 0, 0);
			break;
		
		default: break;
	}
}

LRESULT CALLBACK WndProc ( HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam )
{
	static HGLRC openglContext;
	static HDC deviceContextHandle;

	switch ( message )
	{
		case WM_CREATE:
			deviceContextHandle = GetDC( windowHandle );
			openglContext = SetupGLContext( windowHandle, deviceContextHandle );
			break;
		
		case WM_DESTROY:
			wglDeleteContext( openglContext );
			ReleaseDC( windowHandle, deviceContextHandle );
			PostQuitMessage( 0 );
			break;
		
		case WM_PAINT:
			wglMakeCurrent( deviceContextHandle, openglContext );
			Paint();
			SwapBuffers( deviceContextHandle );
			wglMakeCurrent( NULL, NULL );
			ValidateRect( windowHandle, NULL );
			break;

		case WM_SIZE:
			wglMakeCurrent( deviceContextHandle, openglContext );
			glViewport( 0, 0, LOWORD(lParam), HIWORD(lParam) );
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