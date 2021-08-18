/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <pch.h>

#include <resource.h>
#include <cie.h>

#include "CCie1931Control.h"



BEGIN_MESSAGE_MAP(CCie1931Control, CStatic)
    ON_WM_PAINT()
END_MESSAGE_MAP()


CCie1931Control::CCie1931Control()
{
    m_cie1931xyBmp = (HBITMAP)LoadImage(
        GetModuleHandle(nullptr), MAKEINTRESOURCE(IDB_CIE1931XY),
        IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);

    if (!m_cie1931xyBmp)
        throw std::runtime_error("Failed to load CIE1931XY bitmap");
}


CCie1931Control::~CCie1931Control()
{
    DeleteObject(m_cie1931xyBmp);
}


void CCie1931Control::SetColorSpace(ColorSpace colorSpace)
{
    m_colorSpace = colorSpace;
    InvalidateRect(nullptr);
}


void CCie1931Control::SetHDRData(std::shared_ptr<HDRData> hdrData)
{
    m_hdrData = hdrData;
    InvalidateRect(nullptr);
}


void CCie1931Control::OnPaint(void)
{
    //
    // Figure out usable rectangle and where to draw
    //

    BITMAP bitmap;
    if (!::GetObject(m_cie1931xyBmp, sizeof(bitmap), &bitmap))
        throw std::runtime_error("Failed to get bitmap header from m_cie1931xyBmp");

    CRect rect;
    GetClientRect(&rect);

    LONG canvasHeight = rect.bottom - rect.top;
    LONG canvasWidth = rect.right - rect.left;
    double bitmapAspectRatio = (double)bitmap.bmHeight / (double)bitmap.bmWidth;
    double canvasAspectRatio = (double)canvasHeight / (double)canvasWidth;

    int scaledBitmapHeight;
    int scaledBitmapWidth;

    // Will use max canvas width and less height
    if (bitmapAspectRatio < canvasAspectRatio)
    {
        scaledBitmapHeight = (int)floor(bitmap.bmHeight * ((double)canvasWidth / (double)bitmap.bmWidth));
        scaledBitmapWidth = canvasWidth;
    }
    // Will use max canvas height, and less with
    else
    {
        scaledBitmapHeight = canvasHeight;
        scaledBitmapWidth = (int)floor(bitmap.bmWidth * (double)canvasHeight / (double)bitmap.bmHeight);
    }

    // X,Y coordinates of the zero point and maximum limits in the CIE1931 image (these are image-dependent)
    // All these are in the graph's coordinate system, we'll translate them to Windows for you
    static const double xZeroStartsAtFractionOfImage = 0.11;  // fraction of image from left to get to X=0 vertical axis
    static const double yZeroStartsAtFractionOfImage = 0.08;  // fraction of image from bottom to get to Y=0 horizontal axis
    static const double x80PctEndsAtFractionOfImage = 0.95;   // fraction of image from left to get to X=0.8 vertical axis
    static const double y90PctEndsAtFractionOfImage = 0.9;    // fraction of image from bottom to get to Y=0.9 horizontal axis

    // coordinates for 0,0 and 1,1
    int zeroX = (int)round(xZeroStartsAtFractionOfImage * (double)scaledBitmapWidth);
    int zeroY = (int)round(scaledBitmapHeight - (yZeroStartsAtFractionOfImage * (double)scaledBitmapHeight));
    int oneX = (int)round((x80PctEndsAtFractionOfImage / 80.0 * 100.0) * (double)scaledBitmapWidth);
    int oneY = (int)round(scaledBitmapWidth - ((y90PctEndsAtFractionOfImage / 90.0 * 100.0) * (double)scaledBitmapHeight));  // Can be negative

    // Coordinate helpers
#define X_cie_to_pixel(cie_x) (int)round((zeroX + (oneX - zeroX) * (double)cie_x))
#define Y_cie_to_pixel(cie_y) (int)round((zeroY - (zeroY - oneY) * (double)cie_y))

//
// Draw
//

    PAINTSTRUCT ps;
    HDC hdc = ::BeginPaint(GetSafeHwnd(), &ps);

    // Paint BMP
    HDC hdcMem = ::CreateCompatibleDC(hdc);
    HGDIOBJ oldBitmap = ::SelectObject(hdcMem, m_cie1931xyBmp);

    ::SetStretchBltMode(hdc, HALFTONE);

    ::StretchBlt(
        hdc, 0, 0, scaledBitmapWidth, scaledBitmapHeight,
        hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight,
        SRCCOPY);
    ::SelectObject(hdcMem, oldBitmap);
    ::DeleteDC(hdcMem);

    HPEN hp;

#ifdef _DEBUG

    // Draw the outer lines in the colorspace to visually see if our point-location math is correct

    // X=0 axis
    hp = ::CreatePen(0, 1, RGB(0, 255, 0));
    ::SelectObject(hdc, hp);
    ::MoveToEx(hdc, X_cie_to_pixel(0), Y_cie_to_pixel(0), nullptr);
    ::LineTo(hdc, X_cie_to_pixel(0), Y_cie_to_pixel(0.9));

    // X=0.8 axis
    hp = ::CreatePen(0, 1, RGB(255, 0, 0));
    ::SelectObject(hdc, hp);
    ::MoveToEx(hdc, X_cie_to_pixel(0.8), Y_cie_to_pixel(0), nullptr);
    ::LineTo(hdc, X_cie_to_pixel(0.8), Y_cie_to_pixel(0.9));

    // Y=0 axis
    hp = ::CreatePen(0, 1, RGB(0, 0, 255));
    ::SelectObject(hdc, hp);
    ::MoveToEx(hdc, X_cie_to_pixel(0), Y_cie_to_pixel(0), nullptr);
    ::LineTo(hdc, X_cie_to_pixel(0.8), Y_cie_to_pixel(0));

    // Y=0.9 axis
    hp = ::CreatePen(0, 1, RGB(0, 0, 0));
    ::SelectObject(hdc, hp);
    ::MoveToEx(hdc, X_cie_to_pixel(0), Y_cie_to_pixel(0.9), nullptr);
    ::LineTo(hdc, X_cie_to_pixel(0.8), Y_cie_to_pixel(0.9));

#endif // _DEBUG

    // Colorspace
    if (m_colorSpace != ColorSpace::UNKNOWN)
    {
        const double redX = ColorSpaceToCie1931RedX(m_colorSpace);
        const double redY = ColorSpaceToCie1931RedY(m_colorSpace);
        const double greenX = ColorSpaceToCie1931GreenX(m_colorSpace);
        const double greenY = ColorSpaceToCie1931GreenY(m_colorSpace);
        const double blueX = ColorSpaceToCie1931BlueX(m_colorSpace);
        const double blueY = ColorSpaceToCie1931BlueY(m_colorSpace);

        // Render lines
        hp = ::CreatePen(0, 2, RGB(0, 0, 0));
        ::SelectObject(hdc, hp);

        ::MoveToEx(hdc, X_cie_to_pixel(redX), Y_cie_to_pixel(redY), nullptr);
        ::LineTo(hdc, X_cie_to_pixel(greenX), Y_cie_to_pixel(greenY));
        ::LineTo(hdc, X_cie_to_pixel(blueX), Y_cie_to_pixel(blueY));
        ::LineTo(hdc, X_cie_to_pixel(redX), Y_cie_to_pixel(redY));
    }

    // HDR data
    if (m_hdrData)
    {
        // Primaries
        hp = ::CreatePen(0, 2, RGB(255, 255, 255));
        ::SelectObject(hdc, hp);

        ::MoveToEx(hdc, X_cie_to_pixel(m_hdrData->displayPrimaryRedX), Y_cie_to_pixel(m_hdrData->displayPrimaryRedY), nullptr);
        ::LineTo(hdc, X_cie_to_pixel(m_hdrData->displayPrimaryGreenX), Y_cie_to_pixel(m_hdrData->displayPrimaryGreenY));
        ::LineTo(hdc, X_cie_to_pixel(m_hdrData->displayPrimaryBlueX), Y_cie_to_pixel(m_hdrData->displayPrimaryBlueY));
        ::LineTo(hdc, X_cie_to_pixel(m_hdrData->displayPrimaryRedX), Y_cie_to_pixel(m_hdrData->displayPrimaryRedY));

        // Whitepoint dot
        hp = ::CreatePen(0, 2, RGB(255, 255, 255));
        ::SelectObject(hdc, hp);

        ::Ellipse(hdc,
            X_cie_to_pixel(m_hdrData->whitePointX) - 3,
            Y_cie_to_pixel(m_hdrData->whitePointY) + 3,
            X_cie_to_pixel(m_hdrData->whitePointX) + 3,
            Y_cie_to_pixel(m_hdrData->whitePointY) - 3);
    }

    ::EndPaint(GetSafeHwnd(), &ps);

    CStatic::OnPaint();
}
