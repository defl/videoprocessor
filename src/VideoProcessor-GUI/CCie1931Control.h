/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include <afxwin.h>

#include <ColorSpace.h>
#include <HDRData.h>


/**
 * Win32 gui control which draws the CIE1931 XY chart and can plot HDR and colorspace on it
 */
class CCie1931Control:
	public CStatic
{
public:

	CCie1931Control();
	virtual ~CCie1931Control();

	// Set colorspace
	void SetColorSpace(ColorSpace);

	// Set HDR data
	void SetHDRData(std::shared_ptr<HDRData>);

protected:

	// Handlers for ON_WM_* messages
	void OnPaint();

private:

	HBITMAP m_cie1931xyBmp = nullptr;
	ColorSpace m_colorSpace = ColorSpace::UNKNOWN;
	std::shared_ptr<HDRData> m_hdrData = nullptr;

	DECLARE_MESSAGE_MAP()
};
