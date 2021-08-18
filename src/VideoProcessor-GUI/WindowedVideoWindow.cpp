/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <pch.h>

#include <resource.h>

#include "WindowedVideoWindow.h"


BEGIN_MESSAGE_MAP(WindowedVideoWindow, CStatic)
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_PAINT()
END_MESSAGE_MAP()


WindowedVideoWindow::WindowedVideoWindow()
{
	m_brush.CreateSolidBrush(BLACK);

	m_logoBmp = (HBITMAP)LoadImage(
		GetModuleHandle(nullptr), MAKEINTRESOURCE(IDB_LOGO),
		IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);

	if (!m_logoBmp)
		throw std::runtime_error("Failed to load logo bitmap");
}


WindowedVideoWindow::~WindowedVideoWindow()
{
	m_brush.DeleteObject();

	DeleteObject(m_logoBmp);
}


void WindowedVideoWindow::ShowLogo(bool show)
{
	m_showLogo = show;
	Invalidate();
}



void WindowedVideoWindow::OnPaint()
{
	if (m_showLogo)
	{
		//
		// Figure out usable rectangle and where to draw
		//

		BITMAP bitmap;
		if (!::GetObject(m_logoBmp, sizeof(bitmap), &bitmap))
			throw std::runtime_error("Failed to get bitmap header from logo bitmap");

		CRect rect;
		GetClientRect(&rect);

		LONG canvasHeight = rect.bottom - rect.top;
		LONG canvasWidth = rect.right - rect.left;

		assert(canvasHeight > bitmap.bmHeight);
		assert(canvasWidth > bitmap.bmWidth);
		LONG bmpXOffset = (canvasWidth - bitmap.bmWidth) / 2;
		LONG bmpYOffset = (canvasHeight - bitmap.bmHeight) / 2;

		//
		// Draw
		//

		PAINTSTRUCT ps;
		HDC hdc = ::BeginPaint(GetSafeHwnd(), &ps);

		// Paint background
		::FillRect(hdc, &rect, m_brush);

		// Paint logo
		HDC hdcMem = ::CreateCompatibleDC(hdc);
		HGDIOBJ oldBitmap = ::SelectObject(hdcMem, m_logoBmp);
		::BitBlt(
			hdc, bmpXOffset, bmpYOffset, bitmap.bmWidth, bitmap.bmHeight,
			hdcMem, 0, 0,
			SRCCOPY);
		::SelectObject(hdcMem, oldBitmap);
		::DeleteDC(hdcMem);

		// Done
		::EndPaint(GetSafeHwnd(), &ps);
	}

	CStatic::OnPaint();
}


HBRUSH WindowedVideoWindow::CtlColor(CDC* pDC, UINT nCtlColor)
{
	pDC->SetBkColor(BLACK);
	pDC->SetTextColor(WHITE);

	return m_brush;
}
