/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


const wchar_t FULLSCREEN_WINDOW_CLASS_NAME[] = L"Fullscreen Window";


/**
 * Window class which can be used for drawing on full screen
 */
class FullscreenVideoWindow
{
public:

    FullscreenVideoWindow();
    ~FullscreenVideoWindow();

    // Static window procedure
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // Register and create this window
    void Create(HMONITOR hmon, HWND parentWindow);

    // Get the window handler
    HWND GetHWND() const { return m_hwnd; }

protected:

    LRESULT __forceinline HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND m_hwnd = nullptr;
};
