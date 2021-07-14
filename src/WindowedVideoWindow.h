/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once

#define BLACK RGB(0, 0, 0)
#define WHITE RGB(255, 255, 255)


/**
 * This is where in windowed mode the renderer will draw to
 */
class WindowedVideoWindow:
	public CStatic
{
public:

	WindowedVideoWindow();
	virtual ~WindowedVideoWindow();

	// If true will show the logo
	void ShowLogo(bool show);

protected:

	HBITMAP m_logoBmp = nullptr;
	CBrush m_brush;
	bool m_showLogo = true;

	// Handlers for ON_WM_* messages
	afx_msg void OnPaint();
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);

	DECLARE_MESSAGE_MAP()
};
