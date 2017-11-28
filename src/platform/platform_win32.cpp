#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <Windows.h>
#include <windowsx.h>
#include <gl\GL.h>

// Global Variables:
TCHAR szTitle[] = TEXT("GL");				// The title bar text
TCHAR szWindowClass[] = TEXT("GLTitle");	// the main window class name

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void Init()
{
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
}

void Paint()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

int APIENTRY WinMain (HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	// Initialize global strings
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = szWindowClass;

	RegisterClass(&wc);

	HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	MSG msg;
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}



HGLRC SetupGLContext(HWND hWnd, HDC hdc)
{
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
		PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
		32,                       //Colordepth of the framebuffer.
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,                       //Number of bits for the depthbuffer
		8,                        //Number of bits for the stencilbuffer
		0,                        //Number of Aux buffers in the framebuffer.
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};
	int nFormat = ChoosePixelFormat(hdc, &pfd);
	SetPixelFormat(hdc, nFormat, &pfd);
	HGLRC hGLRC = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hGLRC);
	SetWindowTextA(hWnd, (char *)glGetString(GL_VERSION));
	Init();
	SelectObject(hdc, GetStockObject(SYSTEM_FONT));
	wglUseFontBitmaps(hdc, 0, 255, 1000);
	wglMakeCurrent(NULL, NULL);
	return hGLRC;
}


void OnMouseWheel(HWND hWnd, int xPos, int yPos, int zDelta, UINT)
{
	
}

void OnLButtonDown(HWND hWnd, bool bDoubleClicked, int xPos, int yPos, UINT keyFlags)
{

}

void OnMouseMove(HWND hWnd, int x, int y, UINT keyFlags)
{
	
}


void OnRButtonDown(HWND hWnd, bool, int x, int y, UINT)
{
	
}

void OnRButtonUp(HWND hWnd, int x, int y, UINT)
{
	
}

void OnLButtonDblClk(HWND hWnd, bool, int x, int y, UINT)
{

}

void OnChar(HWND hWnd, TCHAR ch, int)
{
	switch (ch)
	{
	case 'q':
	case 'Q':
		PostMessage(hWnd, WM_CLOSE, 0, 0);
		break;
	default:
		break;
	}

}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HGLRC hGLRC;
	static HDC hdc;

	switch (message)
	{
		case WM_CREATE:
			hdc = GetDC(hWnd);
			hGLRC = SetupGLContext(hWnd, hdc);
			break;
		case WM_PAINT:
			wglMakeCurrent(hdc, hGLRC);
			Paint();
			SwapBuffers(hdc);
			wglMakeCurrent(NULL, NULL);
			ValidateRect(hWnd, NULL);
			break;
		case WM_ERASEBKGND:
			break;
		case WM_SIZE:
			wglMakeCurrent(hdc, hGLRC);
			glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
			wglMakeCurrent(NULL, NULL);
			break;
		case WM_LBUTTONDOWN:
			HANDLE_WM_LBUTTONDOWN(hWnd, wParam, lParam, OnLButtonDown);
			break;
		case WM_MOUSEMOVE:
			HANDLE_WM_MOUSEMOVE(hWnd, wParam, lParam, OnMouseMove);
			break;
		case WM_MOUSEWHEEL:
			HANDLE_WM_MOUSEWHEEL(hWnd, wParam, lParam, OnMouseWheel);
			break;
		case WM_RBUTTONDOWN:
			HANDLE_WM_RBUTTONDOWN(hWnd, wParam, lParam, OnRButtonDown);
			break;
		case WM_RBUTTONUP:
			HANDLE_WM_RBUTTONUP(hWnd, wParam, lParam, OnRButtonUp);
			break;
		case WM_LBUTTONDBLCLK:
			HANDLE_WM_LBUTTONDBLCLK(hWnd, wParam, lParam, OnLButtonDblClk);
			break;
		case WM_CHAR:
			HANDLE_WM_CHAR(hWnd, wParam, lParam, OnChar);
			break;
		case WM_DESTROY:
			wglDeleteContext(hGLRC);
			ReleaseDC(hWnd, hdc);
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}