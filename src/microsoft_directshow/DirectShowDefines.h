/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

// These are extended defines, not found in the original DirectShow

#pragma once


#include <dxva.h>


// MFVideoTransferMatrix
// Win10 SDK mfobjects.h
// https://docs.microsoft.com/en-us/windows/win32/api/mfobjects/ne-mfobjects-mfvideotransfermatrix
static const DXVA_VideoTransferMatrix DIRECTSHOW_VIDEOTRANSFERMATRIX_BT2020_10 = (DXVA_VideoTransferMatrix)4;
static const DXVA_VideoTransferMatrix DIRECTSHOW_VIDEOTRANSFERMATRIX_BT2020_12 = (DXVA_VideoTransferMatrix)5;

// non-standard values (found in MPC-BE)
static const DXVA_VideoTransferMatrix DIRECTSHOW_VIDEOTRANSFERMATRIX_FCC = (DXVA_VideoTransferMatrix)6;
static const DXVA_VideoTransferMatrix DIRECTSHOW_VIDEOTRANSFERMATRIX_YCgCo = (DXVA_VideoTransferMatrix)7;

// MFVideoPrimaries
// Win10 SDK mfobjects.h
// https://docs.microsoft.com/en-us/windows/win32/api/mfobjects/ne-mfobjects-mfvideoprimaries
static const DXVA_VideoPrimaries DIRECTSHOW_VIDEOPRIMARIES_BT2020 = (DXVA_VideoPrimaries)9;
static const DXVA_VideoPrimaries DIRECTSHOW_VIDEOPRIMARIES_XYZ = (DXVA_VideoPrimaries)10;
static const DXVA_VideoPrimaries DIRECTSHOW_VIDEOPRIMARIES_DCI_P3 = (DXVA_VideoPrimaries)11;
static const DXVA_VideoPrimaries DIRECTSHOW_VIDEOPRIMARIES_ACES = (DXVA_VideoPrimaries)12;

// MFVideoTransferFunction
// Win10 SDK mfobjects.h
// https://docs.microsoft.com/en-us/windows/win32/api/mfobjects/ne-mfobjects-mfvideotransferfunction
static const DXVA_VideoTransferFunction DIRECTSHOW_VIDEOTRANSFUNC_Log_100 = (DXVA_VideoTransferFunction)9;
static const DXVA_VideoTransferFunction DIRECTSHOW_VIDEOTRANSFUNC_Log_316 = (DXVA_VideoTransferFunction)10;
static const DXVA_VideoTransferFunction DIRECTSHOW_VIDEOTRANSFUNC_709_sym = (DXVA_VideoTransferFunction)11;
static const DXVA_VideoTransferFunction DIRECTSHOW_VIDEOTRANSFUNC_2020_const = (DXVA_VideoTransferFunction)12;
static const DXVA_VideoTransferFunction DIRECTSHOW_VIDEOTRANSFUNC_2020 = (DXVA_VideoTransferFunction)13;
static const DXVA_VideoTransferFunction DIRECTSHOW_VIDEOTRANSFUNC_26 = (DXVA_VideoTransferFunction)14;
static const DXVA_VideoTransferFunction DIRECTSHOW_VIDEOTRANSFUNC_2084 = (DXVA_VideoTransferFunction)15;  // PQ
static const DXVA_VideoTransferFunction DIRECTSHOW_VIDEOTRANSFUNC_HLG = (DXVA_VideoTransferFunction)16;
static const DXVA_VideoTransferFunction DIRECTSHOW_VIDEOTRANSFUNC_10_rel = (DXVA_VideoTransferFunction)17;

// Timestamps
#define US_TO_DSTS(us) ((REFERENCE_TIME)us * 10LL)
#define MS_TO_DSTS(ms) (US_TO_DSTS(ms) * 1000LL)
#define S_TO_DSTS(s) (MS_TO_DSTS(s) * 1000LL)

// Reference time
const static REFERENCE_TIME REFERENCE_TIME_INVALID = -1;
