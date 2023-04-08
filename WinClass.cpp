/*!
\file		WinClass.cpp
\author 	Darrell Lek (100%)
\par    	email: d.lek@digipen.edu
\date   	September 27, 2022
\brief		Function definition of WinClass class

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#include "WinClass.h"
#include <iostream>
#include <glad/glad.h>

LRESULT CALLBACK Win32GLWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	/* Handle window events here */
	/* Refer to https://learn.microsoft.com/en-us/windows/win32/learnwin32/writing-the-window-procedure for more information */

	switch (message)
	{
	case WM_SIZE:
	{
		Win32GLWindow* window = (Win32GLWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (window != nullptr)
		{
			/* Handle Resize Events */
			// if the window is not minimized, resize the viewport
			if (wParam != SIZE_MINIMIZED)
			{
				int x = (short)LOWORD(lParam);
				int y = (short)HIWORD(lParam);
				window->ResizeCallback(x, y);

				/* You can update the viewport here */
			}
		}
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return NULL;
}

Win32GLWindow::Win32GLWindow()
{
	m_hWnd = NULL;
	m_hDC = NULL;
	m_hRC = NULL;

	m_isFullscreen = false;
	m_isCursorVisible = true;
	m_isCursorLocked = false;
	m_isVSync = false;

	m_width = 0;
	m_height = 0;
}

Win32GLWindow::~Win32GLWindow()
{
	Destroy();
}

bool Win32GLWindow::Create(const char* title, int width, int height, bool fullscreen)
{
	Destroy();

	m_width = width;
	m_height = height;

	// register window class
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"Win32GLWindow";
	wc.hIconSm = wc.hIcon;

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Failed to register window class", L"Error", MB_OK | MB_ICONERROR);
		exit(0);
	}

	// create window
	DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD dwStyle = WS_OVERLAPPEDWINDOW;

	if (fullscreen)
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = width;
		dmScreenSettings.dmPelsHeight = height;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			MessageBox(NULL, L"The requested fullscreen mode is not supported by\nyour video card. Use windowed mode instead?", L"Error", MB_OK | MB_ICONERROR);
			exit(0);
		}

		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP;

		m_isFullscreen = true;
	}

	RECT windowRect = { 0, 0, width, height };
	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	int len = MultiByteToWideChar(CP_ACP, 0, title, -1, NULL, 0);
	wchar_t* wtitle = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, title, -1, wtitle, len);

	m_hWnd = CreateWindowEx(dwExStyle, L"Win32GLWindow", wtitle, dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, NULL, NULL, GetModuleHandle(NULL), this);
	if (!m_hWnd)
	{
		MessageBox(NULL, L"Failed to create window", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	delete[] wtitle;

	// create opengl context
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;

	m_hDC = GetDC(m_hWnd);
	if (!m_hDC)
	{
		MessageBox(NULL, L"Failed to create device context", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	int pixelFormat = ChoosePixelFormat(m_hDC, &pfd);
	if (!pixelFormat)
	{
		MessageBox(NULL, L"Failed to find a suitable pixel format", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	if (!SetPixelFormat(m_hDC, pixelFormat, &pfd))
	{
		MessageBox(NULL, L"Failed to set pixel format", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	m_hRC = wglCreateContext(m_hDC);
	if (!m_hRC)
	{
		MessageBox(NULL, L"Failed to create opengl context", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	if (!wglMakeCurrent(m_hDC, m_hRC))
	{
		MessageBox(NULL, L"Failed to activate opengl context", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	// initialize glad
	if (!gladLoadGL())
	{
		MessageBox(NULL, L"Failed to initialize glad", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	// show window
	ShowWindow(m_hWnd, SW_SHOW);
	SetForegroundWindow(m_hWnd);
	SetFocus(m_hWnd);

	// set vsync
	SetVSync(m_isVSync);

	// Set window long ptr
	SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);

	return true;
}

void Win32GLWindow::Destroy()
{
	if (m_hRC)
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(m_hRC);
		m_hRC = NULL;
	}

	if (m_hDC)
	{
		ReleaseDC(m_hWnd, m_hDC);
		m_hDC = NULL;
	}

	if (m_hWnd)
	{
		DestroyWindow(m_hWnd);
		m_hWnd = NULL;
	}

	if (m_isFullscreen)
	{
		ChangeDisplaySettings(NULL, 0);
		m_isFullscreen = false;
	}
}

void Win32GLWindow::SetVSync(bool vsync)
{
	typedef BOOL(APIENTRY* PFNWGLSWAPINTERVALPROC)(int);
	PFNWGLSWAPINTERVALPROC wglSwapIntervalEXT = 0;
	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALPROC)wglGetProcAddress("wglSwapIntervalEXT");

	m_isVSync = vsync;
	if (m_isVSync)
	{
		wglSwapIntervalEXT(1);
	}
	else
	{
		wglSwapIntervalEXT(0);
	}
}

void Win32GLWindow::SetFullscreen(bool fullscreen)
{
	if (m_isFullscreen == fullscreen)
	{
		return;
	}

	if (fullscreen)
	{
		// get current display mode
		DEVMODE dm;
		memset(&dm, 0, sizeof(dm));
		dm.dmSize = sizeof(dm);
		EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);

		// save current window position and size
		m_windowedRect.left = m_windowedRect.top = 0;
		m_windowedRect.right = m_width;
		m_windowedRect.bottom = m_height;
		GetWindowRect(m_hWnd, &m_windowedRect);

		// switch to fullscreen
		ChangeDisplaySettings(&dm, CDS_FULLSCREEN);
		SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
		SetWindowPos(m_hWnd, HWND_TOP, 0, 0, dm.dmPelsWidth, dm.dmPelsHeight, SWP_FRAMECHANGED);

		m_isFullscreen = true;
	}
	else
	{
		// switch to windowed mode
		ChangeDisplaySettings(NULL, 0);
		SetWindowLong(m_hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
		SetWindowPos(m_hWnd, HWND_NOTOPMOST, m_windowedRect.left, m_windowedRect.top, m_windowedRect.right - m_windowedRect.left, m_windowedRect.bottom - m_windowedRect.top, SWP_FRAMECHANGED);
		m_isFullscreen = false;
	}
}

void Win32GLWindow::SetCursorVisible(bool visible)
{
	if (visible)
	{
		ShowCursor(true);
	}
	else
	{
		ShowCursor(false);
	}
}

void Win32GLWindow::SetCursorLocked(bool locked)
{
	if (locked)
	{
		RECT rect;
		GetClientRect(m_hWnd, &rect);
		MapWindowPoints(m_hWnd, NULL, (LPPOINT)&rect, 2);
		ClipCursor(&rect);
	}
	else
	{
		ClipCursor(NULL);
	}
}

void Win32GLWindow::SetTitle(const char* title)
{
	// convert title to LPCWSTR
	int len = MultiByteToWideChar(CP_ACP, 0, title, -1, NULL, 0);
	wchar_t* wtitle = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, title, -1, wtitle, len);

	SetWindowText(m_hWnd, wtitle);

	delete[] wtitle;
}

void Win32GLWindow::SetSize(int width, int height)
{
	m_width = width;
	m_height = height;

	if (m_isFullscreen)
	{
		SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP);
		SetWindowPos(m_hWnd, HWND_TOP, 0, 0, m_width, m_height, SWP_FRAMECHANGED);
	}
	else
	{
		SetWindowLong(m_hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		SetWindowPos(m_hWnd, HWND_TOP, 0, 0, m_width, m_height, SWP_FRAMECHANGED);
	}
}

void Win32GLWindow::Show()
{
	ShowWindow(m_hWnd, SW_SHOW);
}

void Win32GLWindow::Hide()
{
	ShowWindow(m_hWnd, SW_HIDE);
}

bool Win32GLWindow::IsFullscreen() const
{
	return m_isFullscreen;
}

bool Win32GLWindow::IsVSync() const
{
	return m_isVSync;
}

bool Win32GLWindow::IsCursorVisible() const
{
	return m_isCursorVisible;
}

bool Win32GLWindow::IsCursorLocked() const
{
	return m_isCursorLocked;
}

int Win32GLWindow::GetWidth() const
{
	return m_width;
}

int Win32GLWindow::GetHeight() const
{
	return m_height;
}

bool Win32GLWindow::ProcessMessages()
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return true;
}

void Win32GLWindow::ResizeCallback(int width, int height)
{
	m_width = width;
	m_height = height;
}