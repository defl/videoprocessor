/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <stdafx.h>

#include <resource.h>
#include <cie.h>

#include "CCie1931Control.h"



BEGIN_MESSAGE_MAP(CCie1931Control, CStatic)
    ON_WM_PAINT()
END_MESSAGE_MAP()


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
    // TODO: This is using the GDI, probably also possible though MFC

    // Load bitmap on first use
    // TODO: Figure how to get OnCreate() called here. Apparently not happening because this is a custom widget which
    //       is hook up to the resource generated magic something. Destructors also not allowed. So nevermind and just leak
    //       for now until I can be arsed to figure it out.
    //       details: https://social.msdn.microsoft.com/Forums/en-US/c659741c-5330-406d-92c7-c089a48872ff/ccustomcontroloncreate-not-called?forum=vcgeneral
    if (!m_cie1931xyBmp)
    {
        m_cie1931xyBmp = (HBITMAP)LoadImage(
            GetModuleHandle(nullptr), MAKEINTRESOURCE(IDB_CIE1931XY),
            IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);

        if (!m_cie1931xyBmp)
            throw std::runtime_error("Failed to load CIE1931XY bitmap");
    }

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
        scaledBitmapHeight = (int)std::floor(bitmap.bmHeight * ((double)canvasWidth / (double)bitmap.bmWidth));
        scaledBitmapWidth = canvasWidth;
    }
    // Will use max canvas height, and less with
    else
    {
        scaledBitmapHeight = canvasHeight;
        scaledBitmapWidth = (int)std::floor(bitmap.bmWidth * (double)canvasHeight / (double)bitmap.bmHeight);
    }

    // X,Y coordinates of the zero point and maximum limits in the CIE1931 image (these are image-dependent)
    // All these are in the graph's coordinate system, we'll translate them to Windows for you
    static const double xZeroStartsAtFractionOfImage = 0.11;  // fraction of image from left to get to X=0 vertical axis
    static const double yZeroStartsAtFractionOfImage = 0.08;  // fraction of image from bottom to get to Y=0 horizontal axis
    static const double x80PctEndsAtFractionOfImage = 0.95;   // fraction of image from left to get to X=0.8 vertical axis
    static const double y90PctEndsAtFractionOfImage = 0.9;    // fraction of image from bottom to get to Y=0.9 horizontal axis

    // coordinates for 0,0 and 1,1
    int zeroX = (int)std::round(xZeroStartsAtFractionOfImage * (double)scaledBitmapWidth);
    int zeroY = (int)std::round(scaledBitmapHeight - (yZeroStartsAtFractionOfImage * (double)scaledBitmapHeight));
    int oneX = (int)std::round((x80PctEndsAtFractionOfImage / 80.0 * 100.0) * (double)scaledBitmapWidth);
    int oneY = (int)std::round(scaledBitmapWidth - ((y90PctEndsAtFractionOfImage / 90.0 * 100.0) * (double)scaledBitmapHeight));  // Can be negative

    // Coordinate helpers
#define X_cie_to_pixel(cie_x) (int)std::round((zeroX + (oneX - zeroX) * (double)cie_x))
#define Y_cie_to_pixel(cie_y) (int)std::round((zeroY - (zeroY - oneY) * (double)cie_y))

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
        double redX, redY, greenX, greenY, blueX, blueY;

        switch (m_colorSpace)
        {
            // Coordinates from https://en.wikipedia.org/wiki/Rec._2020
        case ColorSpace::BT_2020:
            redX = 0.708;
            redY = 0.292;
            greenX = 0.17;
            greenY = 0.797;
            blueX = 0.131;
            blueY = 0.046;
            break;

            // Coordinates from https://en.wikipedia.org/wiki/Rec._709
        case ColorSpace::REC_709:
            redX = 0.64;
            redY = 0.33;
            greenX = 0.30;
            greenY = 0.60;
            blueX = 0.15;
            blueY = 0.06;
            break;

            // Coordinates from https://en.wikipedia.org/wiki/Rec._601
        case ColorSpace::REC_601_525:
            redX = 0.630;
            redY = 0.340;
            greenX = 0.310;
            greenY = 0.595;
            blueX = 0.155;
            blueY = 0.070;
            break;
        case ColorSpace::REC_601_625:
            redX = 0.640;
            redY = 0.330;
            greenX = 0.290;
            greenY = 0.600;
            blueX = 0.150;
            blueY = 0.060;
            break;

        default:
            throw std::runtime_error("Unknown colorspace");
        }

        // Render lines
        hp = ::CreatePen(0, 2, RGB(0, 0, 0));
        ::SelectObject(hdc, hp);

        ::MoveToEx(hdc, X_cie_to_pixel(redX), Y_cie_to_pixel(redY), nullptr);
        ::LineTo(hdc, X_cie_to_pixel(greenX), Y_cie_to_pixel(greenY));
        ::LineTo(hdc, X_cie_to_pixel(blueX), Y_cie_to_pixel(blueY));
        ::LineTo(hdc, X_cie_to_pixel(redX), Y_cie_to_pixel(redY));

        // Render text
        ::SetBkColor(hdc, RGB(0, 0, 0));
        ::SetBkMode(hdc, OPAQUE);

        ::SetTextColor(hdc, RGB(255, 0, 0));
        const CString redString = CieXYToString(redX, redY);
        ::TextOut(hdc,
            X_cie_to_pixel(redX) - 50,
            Y_cie_to_pixel(redY) - 20,
            redString, redString.GetLength());

        ::SetTextColor(hdc, RGB(0, 255, 0));
        const CString greenString = CieXYToString(greenX, greenY);
        ::TextOut(hdc,
            X_cie_to_pixel(greenX) - 20,
            Y_cie_to_pixel(greenY) - 20,
            greenString, greenString.GetLength());

        ::SetTextColor(hdc, RGB(100, 100, 255));
        const CString blueString = CieXYToString(blueX, blueY);
        ::TextOut(hdc,
            X_cie_to_pixel(blueX) - 20,
            Y_cie_to_pixel(blueY) + 6,
            blueString, blueString.GetLength());
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

        // Render text
        ::SetTextColor(hdc, RGB(0, 0, 0));
        ::SetBkColor(hdc, RGB(255, 255, 255));
        ::SetBkMode(hdc, OPAQUE);
        const CString whitePointString = CieXYToString(m_hdrData->whitePointX, m_hdrData->whitePointY);
        ::TextOut(hdc,
            X_cie_to_pixel(m_hdrData->whitePointX),
            Y_cie_to_pixel(m_hdrData->whitePointY) + 6,
            whitePointString, whitePointString.GetLength());
    }

    ::EndPaint(GetSafeHwnd(), &ps);

    CStatic::OnPaint();
}
