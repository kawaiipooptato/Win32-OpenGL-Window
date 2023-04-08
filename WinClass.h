/*!
\file		WinClass.h
\author 	Darrell Lek (100%)
\par    	email: d.lek@digipen.edu
\date   	September 27, 2022
\brief		Function declaration of WinClass class

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

class Win32GLWindow
{
public:
	Win32GLWindow();
	~Win32GLWindow();

	bool Create(const char* title, int width, int height, bool fullscreen);
	void Destroy();

	void Show();
	void Hide();

	void SetTitle(const char* title);
	void SetSize(int width, int height);
	void SetFullscreen(bool fullscreen);

	void SetCursorVisible(bool visible);
	void SetCursorLocked(bool locked);

	void SetVSync(bool vsync);

	void SwapBuffers()
	{
		::SwapBuffers(m_hDC);
	}

	bool IsFullscreen() const;
	bool IsCursorVisible() const;
	bool IsCursorLocked() const;
	bool IsVSync() const;

	int GetWidth() const;
	int GetHeight() const;

	HWND GetHandle() const
	{
		return m_hWnd;
	}

	bool ProcessMessages();

	void ResizeCallback(int width, int height);

private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	HWND m_hWnd;
	HDC m_hDC;
	HGLRC m_hRC;

	RECT m_windowedRect; // used to store the windowed rect when switching to fullscreen

	bool m_isFullscreen;
	bool m_isCursorVisible;
	bool m_isCursorLocked;
	bool m_isVSync;

	int m_width;
	int m_height;
};