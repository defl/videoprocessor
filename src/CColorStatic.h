/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include <atlstr.h>


/**
 * Version of a CStatic with color abilities
 * https://stackoverflow.com/questions/1636590/mfc-change-text-color-of-a-cstatic-text-control
 */
class CColorStatic:
    public CStatic
{
protected:
    DECLARE_MESSAGE_MAP()

public:

    const static COLORREF WHITE  = RGB(255, 255, 255);
    const static COLORREF BLACK  = RGB(0, 0, 0);
    const static COLORREF GREEN  = RGB(0, 140, 63);
    const static COLORREF ORANGE = RGB(255, 174, 0);
    const static COLORREF RED    = RGB(255, 0, 0);
    const static COLORREF WINDOWS_APP_BACKGROUND = RGB(240, 240, 240);

    void SetTextColor(COLORREF);

protected:
    HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);

private:

    COLORREF m_textColor = BLACK;
};
