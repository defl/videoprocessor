/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <stdafx.h>

#include "FullscreenWindow.h"


FullscreenWindow::FullscreenWindow()
{
}


FullscreenWindow::~FullscreenWindow()
{
    if (m_hwnd)
    {
        ::DestroyWindow(m_hwnd);
        ::UnregisterClassW(FULLSCREEN_WINDOW_CLASS_NAME, GetModuleHandle(nullptr));
    }
}


LRESULT CALLBACK FullscreenWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    FullscreenWindow* pThis = nullptr;

    if (uMsg == WM_NCCREATE)
    {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pThis = (FullscreenWindow*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

        pThis->m_hwnd = hwnd;
    }
    else
    {
        pThis = (FullscreenWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (pThis)
        return pThis->HandleMessage(uMsg, wParam, lParam);

    // Return default action
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


void FullscreenWindow::Create(HMONITOR hmon, HWND parentWindow)
{
    //
    // Register the window class
    //

    WNDCLASS wc = { 0 };

    wc.lpfnWndProc = FullscreenWindow::WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = FULLSCREEN_WINDOW_CLASS_NAME;

    RegisterClass(&wc);

    //
    // Create the window
    //

    MONITORINFO mi = { sizeof(mi) };
    if (!GetMonitorInfo(hmon, &mi))
        throw std::runtime_error("Failed to get monitor info");

    // When debugging it's REALLY handy to not cover the whole screen and capture key inputs...
    LONG width = mi.rcMonitor.right - mi.rcMonitor.left;
#ifdef _DEBUG
    width = width  / 5;
#endif

    m_hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_ACCEPTFILES | WS_EX_NOPARENTNOTIFY,
        FULLSCREEN_WINDOW_CLASS_NAME,
        TEXT("Waiting for renderer to start."),
        WS_POPUP | WS_VISIBLE,
        mi.rcMonitor.left,
        mi.rcMonitor.top,
        width,
        mi.rcMonitor.bottom - mi.rcMonitor.top,
        parentWindow,
        nullptr,  // hMenu
        GetModuleHandle(nullptr),  // Parent process
        this);

    if(!m_hwnd)
        throw std::runtime_error("Failed to create window");
}


LRESULT __forceinline FullscreenWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {

    case WM_DESTROY:
        return 0;  // no PostQuitMessage, not it's own thread

    }

    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}
