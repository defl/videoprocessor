/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <pch.h>

#include <winnt.h>
extern "C" {
#include <libavutil/log.h>
}

#include <VideoProcessorDlg.h>
#include <VideoConversionOverride.h>


#include "VideoProcessorApp.h"


BEGIN_MESSAGE_MAP(CVideoProcessorApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


CVideoProcessorApp videoProcessorApp;


void av_log_callback(void* ptr, int level, const char* fmt, va_list vargs)
{
	vprintf(fmt, vargs);
}


BOOL CVideoProcessorApp::InitInstance()
{
	// Setup ffmpeg logging
	av_log_set_callback(av_log_callback);
#ifdef _DEBUG
	av_log_set_level(AV_LOG_TRACE);
#endif

	CVideoProcessorDlg dlg;
	m_pMainWnd = &dlg;

	try
	{
		if (!CWinAppEx::InitInstance())
			throw std::runtime_error("Failed to initialize VideoProcessorApp");

		// COINIT_MULTITHREADED was used in the Blackmagic SDK examples,
		// using that without further investigation
		if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
			throw std::runtime_error("Failed to initialize com objects");

		// Parse command line
		// https://docs.microsoft.com/en-us/cpp/c-runtime-library/argc-argv-wargv
		int iNumOfArgs;
		LPWSTR* pArgs = CommandLineToArgvW(GetCommandLine(), &iNumOfArgs);
		for (int i = 1; i < iNumOfArgs; i++)
		{
			// /fullscreen
			if (wcscmp(pArgs[i], L"/fullscreen") == 0)
			{
				dlg.StartFullScreen();
			}

			// /renderer "name"
			if (wcscmp(pArgs[i], L"/renderer") == 0 && (i + 1) < iNumOfArgs)
			{
				dlg.DefaultRendererName(pArgs[i + 1]);
			}

			// /frame_offset [value|"auto"]
			if (wcscmp(pArgs[i], L"/frame_offset") == 0 && (i + 1) < iNumOfArgs)
			{
				if (wcscmp(pArgs[i + 1], L"auto") == 0)
				{
					dlg.StartFrameOffsetAuto();
				}
				else
				{
					dlg.StartFrameOffset(pArgs[i + 1]);
				}
			}

			// Set video conversion overrides
			if (wcscmp(pArgs[i], L"/video_conversion") == 0 && (i + 1) < iNumOfArgs)
			{
				VideoConversionOverride videoConversionOverride = VideoConversionOverride::VIDEOCONVERSION_NONE;

				if (wcscmp(pArgs[i + 1], L"V210_TO_P010") == 0)
				{
					videoConversionOverride = VideoConversionOverride::VIDEOCONVERSION_V210_TO_P010;
				}
				else
				{
					throw std::runtime_error("Invalid option for /video_conversion");
				}

				dlg.DefaultVideoConversionOverride(videoConversionOverride);
			}

			// Set container colorspace
			if (wcscmp(pArgs[i], L"/container_colorspace") == 0 && (i + 1) < iNumOfArgs)
			{
				ColorSpace colorSpaceOption;

				if (wcscmp(pArgs[i + 1], L"BT2020") == 0)
				{
					colorSpaceOption = ColorSpace::BT_2020;
				}
				else if (wcscmp(pArgs[i + 1], L"P3_D65") == 0)
				{
					colorSpaceOption = ColorSpace::P3_D65;
				}
				else if (wcscmp(pArgs[i + 1], L"P3_DCI") == 0)
				{
					colorSpaceOption = ColorSpace::P3_DCI;
				}
				else if (wcscmp(pArgs[i + 1], L"P3_D60") == 0)
				{
					colorSpaceOption = ColorSpace::P3_D60;
				}
				else if (wcscmp(pArgs[i + 1], L"REC709") == 0)
				{
					colorSpaceOption = ColorSpace::REC_709;
				}
				else if (wcscmp(pArgs[i + 1], L"REC601_525") == 0)
				{
					colorSpaceOption = ColorSpace::REC_601_525;
				}
				else if (wcscmp(pArgs[i + 1], L"REC601_625") == 0)
				{
					colorSpaceOption = ColorSpace::REC_601_625;
				}
				else
				{
					throw std::runtime_error("Invalid option for /colorspace");
				}

				dlg.DefaultContainerColorSpace(colorSpaceOption);
			}

			// Set HDR color space
			if (wcscmp(pArgs[i], L"/hdr_colorspace") == 0 && (i + 1) < iNumOfArgs)
			{
				HdrColorspaceOptions hdrColorSpaceOption;

				if (wcscmp(pArgs[i + 1], L"FOLLOW_INPUT") == 0)
				{
					hdrColorSpaceOption = HdrColorspaceOptions::HDR_COLORSPACE_FOLLOW_INPUT;
				}
				else if (wcscmp(pArgs[i + 1], L"FOLLOW_INPUT_LLDV") == 0)
				{
					hdrColorSpaceOption = HdrColorspaceOptions::HDR_COLORSPACE_FOLLOW_INPUT_LLDV;
				}
				else if (wcscmp(pArgs[i + 1], L"FOLLOW_CONTAINER") == 0)
				{
					hdrColorSpaceOption = HdrColorspaceOptions::HDR_COLORSPACE_FOLLOW_CONTAINER;
				}
				else if (wcscmp(pArgs[i + 1], L"BT2020") == 0)
				{
					hdrColorSpaceOption = HdrColorspaceOptions::HDR_COLORSPACE_BT2020;
				}
				else if (wcscmp(pArgs[i + 1], L"P3") == 0)
				{
					hdrColorSpaceOption = HdrColorspaceOptions::HDR_COLORSPACE_P3;
				}
				else if (wcscmp(pArgs[i + 1], L"REC709") == 0)
				{
					hdrColorSpaceOption = HdrColorspaceOptions::HDR_COLORSPACE_REC709;
				}
				else
				{
					throw std::runtime_error("Invalid option for /hdr_colorspace");
				}

				dlg.DefaultHDRColorSpace(hdrColorSpaceOption);
			}

			// Set HDR luminance
			if (wcscmp(pArgs[i], L"/hdr_luminance") == 0 && (i + 1) < iNumOfArgs)
			{
				HdrLuminanceOptions hdrLuminanceOption;

				if (wcscmp(pArgs[i + 1], L"FOLLOW_INPUT") == 0)
				{
					hdrLuminanceOption = HdrLuminanceOptions::HDR_LUMINANCE_FOLLOW_INPUT;
				}
				else if (wcscmp(pArgs[i + 1], L"FOLLOW_INPUT_LLDV") == 0)
				{
					hdrLuminanceOption = HdrLuminanceOptions::HDR_LUMINANCE_FOLLOW_INPUT_LLDV;
				}
				else if (wcscmp(pArgs[i + 1], L"HDR_LUMINANCE_USER") == 0)
				{
					hdrLuminanceOption = HdrLuminanceOptions::HDR_LUMINANCE_USER;
				}
				else
				{
					throw std::runtime_error("Invalid option for /hdr_luminance");
				}

				dlg.DefaultHDRLuminance(hdrLuminanceOption);
			}

			// Set renderer start-stop time method
			if (wcscmp(pArgs[i], L"/renderer_start_stop_time_method") == 0 && (i + 1) < iNumOfArgs)
			{
				DirectShowStartStopTimeMethod dsssTimeMethod;

				if (wcscmp(pArgs[i + 1], L"CLOCK_SMART") == 0)
				{
					dsssTimeMethod = DirectShowStartStopTimeMethod::DS_SSTM_CLOCK_SMART;
				}
				else if (wcscmp(pArgs[i + 1], L"CLOCK_THEO") == 0)
				{
					dsssTimeMethod = DirectShowStartStopTimeMethod::DS_SSTM_CLOCK_THEO;
				}
				else if (wcscmp(pArgs[i + 1], L"CLOCK_CLOCK") == 0)
				{
					dsssTimeMethod = DirectShowStartStopTimeMethod::DS_SSTM_CLOCK_CLOCK;
				}
				else if (wcscmp(pArgs[i + 1], L"THEO_THEO") == 0)
				{
					dsssTimeMethod = DirectShowStartStopTimeMethod::DS_SSTM_THEO_THEO;
				}
				else if (wcscmp(pArgs[i + 1], L"CLOCK_NONE") == 0)
				{
					dsssTimeMethod = DirectShowStartStopTimeMethod::DS_SSTM_CLOCK_NONE;
				}
				else if (wcscmp(pArgs[i + 1], L"THEO_NONE") == 0)
				{
					dsssTimeMethod = DirectShowStartStopTimeMethod::DS_SSTM_THEO_NONE;
				}
				else
				{
					throw std::runtime_error("Invalid option for /renderer_start_stop_time_method");
				}

				dlg.DefaultRendererStartStopTimeMethod(dsssTimeMethod);
			}

			// Set nominal range
			if (wcscmp(pArgs[i], L"/renderer_nominal_range") == 0 && (i + 1) < iNumOfArgs)
			{
				DXVA_NominalRange nominalRange;

				if (wcscmp(pArgs[i + 1], L"FULL") == 0)
				{
					nominalRange = DXVA_NominalRange::DXVA_NominalRange_0_255;
				}
				else if (wcscmp(pArgs[i + 1], L"LIMITED") == 0)
				{
					nominalRange = DXVA_NominalRange::DXVA_NominalRange_16_235;
				}
				else if (wcscmp(pArgs[i + 1], L"SMALL") == 0)
				{
					nominalRange = DXVA_NominalRange::DXVA_NominalRange_48_208;
				}
				else
				{
					throw std::runtime_error("Invalid option for /renderer_nominal_range");
				}

				dlg.DefaultRendererNominalRange(nominalRange);
			}

			// Set transfer function
			if (wcscmp(pArgs[i], L"/renderer_transfer_function") == 0 && (i + 1) < iNumOfArgs)
			{
				DXVA_VideoTransferFunction transferFunction = DXVA_VideoTransferFunction::DXVA_VideoTransFunc_Unknown;

				if (wcscmp(pArgs[i + 1], L"AUTO") == 0)
				{
					transferFunction = DXVA_VideoTransferFunction::DXVA_VideoTransFunc_Unknown;
				}
				else if (wcscmp(pArgs[i + 1], L"PQ") == 0)
				{
					transferFunction = DIRECTSHOW_VIDEOTRANSFUNC_2084;
				}
				else if (wcscmp(pArgs[i + 1], L"REC709") == 0)
				{
					transferFunction = DXVA_VideoTransferFunction::DXVA_VideoTransFunc_22_709;
				}
				else if (wcscmp(pArgs[i + 1], L"BT2020_CONST") == 0)
				{
					transferFunction = DIRECTSHOW_VIDEOTRANSFUNC_2020_const;
				}

				else if (wcscmp(pArgs[i + 1], L"GAMMA_1.8") == 0)
				{
					transferFunction = DXVA_VideoTransferFunction::DXVA_VideoTransFunc_18;
				}
				else if (wcscmp(pArgs[i + 1], L"GAMMA_2.0") == 0)
				{
					transferFunction = DXVA_VideoTransferFunction::DXVA_VideoTransFunc_20;
				}
				else if (wcscmp(pArgs[i + 1], L"GAMMA_2.2") == 0)
				{
					transferFunction = DXVA_VideoTransferFunction::DXVA_VideoTransFunc_22;
				}
				else if (wcscmp(pArgs[i + 1], L"GAMMA_2.6") == 0)
				{
					transferFunction = DIRECTSHOW_VIDEOTRANSFUNC_26;
				}
				else if (wcscmp(pArgs[i + 1], L"GAMMA_2.8") == 0)
				{
					transferFunction = DXVA_VideoTransferFunction::DXVA_VideoTransFunc_28;
				}

				else if (wcscmp(pArgs[i + 1], L"LINEAR_RGB") == 0)
				{
					transferFunction = DXVA_VideoTransferFunction::DXVA_VideoTransFunc_10;
				}
				else if (wcscmp(pArgs[i + 1], L"204M") == 0)
				{
					transferFunction = DXVA_VideoTransferFunction::DXVA_VideoTransFunc_22_240M;
				}
				else if (wcscmp(pArgs[i + 1], L"8BIT_GAMMA_2.2") == 0)
				{
					transferFunction = DXVA_VideoTransferFunction::DXVA_VideoTransFunc_22_8bit_sRGB;
				}
				else if (wcscmp(pArgs[i + 1], L"LOG_100_1") == 0)
				{
					transferFunction = DIRECTSHOW_VIDEOTRANSFUNC_Log_100;
				}
				else if (wcscmp(pArgs[i + 1], L"LOG_316_1") == 0)
				{
					transferFunction = DIRECTSHOW_VIDEOTRANSFUNC_Log_316;
				}
				else if (wcscmp(pArgs[i + 1], L"REC709") == 0)
				{
					transferFunction = DIRECTSHOW_VIDEOTRANSFUNC_709_sym;
				}
				else if (wcscmp(pArgs[i + 1], L"BT2020") == 0)
				{
					transferFunction = DIRECTSHOW_VIDEOTRANSFUNC_2020;
				}
				else if (wcscmp(pArgs[i + 1], L"HYBRID_LOG_GAMMA") == 0)
				{
					transferFunction = DIRECTSHOW_VIDEOTRANSFUNC_HLG;
				}
				else
				{
					throw std::runtime_error("Invalid option for /renderer_transfer_function");
				}

				dlg.DefaultRendererTransferFunction(transferFunction);
			}

			// Set transfer matrix
			if (wcscmp(pArgs[i], L"/renderer_transfer_matrix") == 0 && (i + 1) < iNumOfArgs)
			{
				DXVA_VideoTransferMatrix transferMatrix = DXVA_VideoTransferMatrix::DXVA_VideoTransferMatrix_Unknown;

				if (wcscmp(pArgs[i + 1], L"AUTO") == 0)
				{
					transferMatrix = DXVA_VideoTransferMatrix::DXVA_VideoTransferMatrix_Unknown;
				}
				else if (wcscmp(pArgs[i + 1], L"BT2020_10") == 0)
				{
					transferMatrix = DIRECTSHOW_VIDEOTRANSFERMATRIX_BT2020_10;
				}
				else if (wcscmp(pArgs[i + 1], L"BT2020_12") == 0)
				{
					transferMatrix = DIRECTSHOW_VIDEOTRANSFERMATRIX_BT2020_12;
				}
				else if (wcscmp(pArgs[i + 1], L"BT709") == 0)
				{
					transferMatrix = DXVA_VideoTransferMatrix::DXVA_VideoTransferMatrix_BT709;
				}
				else if (wcscmp(pArgs[i + 1], L"BT601") == 0)
				{
					transferMatrix = DXVA_VideoTransferMatrix::DXVA_VideoTransferMatrix_BT601;
				}
				else if (wcscmp(pArgs[i + 1], L"240M") == 0)
				{
					transferMatrix = DXVA_VideoTransferMatrix::DXVA_VideoTransferMatrix_SMPTE240M;
				}
				else if (wcscmp(pArgs[i + 1], L"FCC") == 0)
				{
					transferMatrix = DIRECTSHOW_VIDEOTRANSFERMATRIX_FCC;
				}
				else if (wcscmp(pArgs[i + 1], L"YCgCo") == 0)
				{
					transferMatrix = DIRECTSHOW_VIDEOTRANSFERMATRIX_YCgCo;
				}
				else
				{
					throw std::runtime_error("Invalid option for /renderer_transfer_matrix");
				}

				dlg.DefaultRendererTransferMatrix(transferMatrix);
			}

			// Set primaries
			if (wcscmp(pArgs[i], L"/renderer_primaries") == 0 && (i + 1) < iNumOfArgs)
			{
				DXVA_VideoPrimaries primaries = DXVA_VideoPrimaries::DXVA_VideoPrimaries_Unknown;

				if (wcscmp(pArgs[i + 1], L"AUTO") == 0)
				{
					primaries = DXVA_VideoPrimaries::DXVA_VideoPrimaries_Unknown;
				}
				else if (wcscmp(pArgs[i + 1], L"BT2020") == 0)
				{
					primaries = DIRECTSHOW_VIDEOPRIMARIES_BT2020;
				}
				else if (wcscmp(pArgs[i + 1], L"DCI-P3") == 0)
				{
					primaries = DIRECTSHOW_VIDEOPRIMARIES_DCI_P3;
				}
				else if (wcscmp(pArgs[i + 1], L"BT709") == 0)
				{
					primaries = DXVA_VideoPrimaries::DXVA_VideoPrimaries_BT709;
				}
				else if (wcscmp(pArgs[i + 1], L"NTSC_SYSM") == 0)
				{
					primaries = DXVA_VideoPrimaries::DXVA_VideoPrimaries_BT470_2_SysM;
				}
				else if (wcscmp(pArgs[i + 1], L"NYSC_SYSBG") == 0)
				{
					primaries = DXVA_VideoPrimaries::DXVA_VideoPrimaries_BT470_2_SysBG;
				}
				else if (wcscmp(pArgs[i + 1], L"CIE1931_ZYX") == 0)
				{
					primaries = DIRECTSHOW_VIDEOPRIMARIES_XYZ;
				}
				else if (wcscmp(pArgs[i + 1], L"ACES") == 0)
				{
					primaries = DIRECTSHOW_VIDEOPRIMARIES_ACES;
				}
				else
				{
					throw std::runtime_error("Invalid option for /renderer_primaries");
				}

				dlg.DefaultRendererPrimaries(primaries);
			}
		}

		// Set set ourselves to high prio.
		if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS))
			throw std::runtime_error("Failed to set process priority");

		dlg.DoModal();
	}
	catch (std::runtime_error& e)
	{
		dlg.EndDialog(IDABORT);

		size_t size = strlen(e.what()) + 1;
		wchar_t* wtext = new wchar_t[size];
		size_t outSize;
		mbstowcs_s(&outSize, wtext, size, e.what(), size - 1);

		MessageBox(nullptr, wtext, TEXT("Fatal error"), MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);

		delete[] wtext;
	}

	CoUninitialize();

	return FALSE;
}


// Only here for debugging purposes where the application is compiled as a console application.
int main() {
	return _tWinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOW);
}
