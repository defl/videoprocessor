/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <stdafx.h>

#include <atlstr.h>
#include <algorithm>
#include <vector>
#include <dxva.h>
#include <propvarutil.h>

#include <version.h>
#include <cie.h>
#include <resource.h>
#include <VideoProcessorApp.h>
#include <microsoft_directshow/CVideoInfo1DirectShowRenderer.h>
#include <microsoft_directshow/CVideoInfo2DirectShowRenderer.h>
#include <microsoft_directshow/DirectShowDefines.h>
#include <guid.h>

#include "VideoProcessorDlg.h"


const static UINT_PTR TIMER_ID_1SECOND = 1;


BEGIN_MESSAGE_MAP(CVideoProcessorDlg, CDialog)

	// Pre-baked callbacks
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_QUERYDRAGICON()
	ON_WM_GETMINMAXINFO()
	ON_WM_SETFOCUS()
	ON_WM_CLOSE()
	ON_WM_TIMER()

	// Custom messages
	ON_MESSAGE(WM_MESSAGE_CAPTURE_DEVICE_FOUND, &CVideoProcessorDlg::OnMessageCaptureDeviceFound)
	ON_MESSAGE(WM_MESSAGE_CAPTURE_DEVICE_LOST, &CVideoProcessorDlg::OnMessageCaptureDeviceLost)
	ON_MESSAGE(WM_MESSAGE_CAPTURE_DEVICE_STATE_CHANGE, &CVideoProcessorDlg::OnMessageCaptureDeviceStateChange)
	ON_MESSAGE(WM_MESSAGE_CAPTURE_DEVICE_CARD_STATE_CHANGE, &CVideoProcessorDlg::OnMessageCaptureDeviceCardStateChange)
	ON_MESSAGE(WM_MESSAGE_CAPTURE_DEVICE_VIDEO_STATE_CHANGE, &CVideoProcessorDlg::OnMessageCaptureDeviceVideoStateChange)
	ON_MESSAGE(WM_MESSAGE_DIRECTSHOW_NOTIFICATION, &CVideoProcessorDlg::OnMessageDirectShowNotification)
	ON_MESSAGE(WM_MESSAGE_RENDERER_STATE_CHANGE, &CVideoProcessorDlg::OnMessageRendererStateChange)

	// UI element messages
	ON_CBN_SELCHANGE(IDC_CAPTURE_DEVICE_COMBO, &CVideoProcessorDlg::OnCaptureDeviceSelected)
	ON_CBN_SELCHANGE(IDC_CAPTURE_INPUT_COMBO, &CVideoProcessorDlg::OnCaptureInputSelected)
	ON_CBN_SELCHANGE(IDC_RENDERER_COMBO, &CVideoProcessorDlg::OnRendererSelected)
	ON_CBN_SELCHANGE(IDC_RENDERER_TIMESTAMP_COMBO, &CVideoProcessorDlg::OnRendererTimestampSelected)
	ON_CBN_SELCHANGE(IDC_RENDERER_NOMINAL_RANGE_COMBO, &CVideoProcessorDlg::OnRendererNominalRangeSelected)
	ON_CBN_SELCHANGE(IDC_RENDERER_TRANSFER_FUNCTION_COMBO, &CVideoProcessorDlg::OnRendererTransferFunctionSelected)
	ON_CBN_SELCHANGE(IDC_RENDERER_TRANSFER_MATRIX_COMBO, &CVideoProcessorDlg::OnRendererTransferMatrixSelected)
	ON_CBN_SELCHANGE(IDC_RENDERER_PRIMARIES_COMBO, &CVideoProcessorDlg::OnRendererPrimariesSelected)
	ON_BN_CLICKED(IDC_FULL_SCREEN_BUTTON, &CVideoProcessorDlg::OnBnClickedFullScreenButton)
	ON_BN_CLICKED(IDC_TIMING_CLOCK_FRAME_OFFSET_AUTO_CHECK, &CVideoProcessorDlg::OnBnClickedTimingClockFrameOffsetAutoCheck)
	ON_BN_CLICKED(IDC_RENDERER_RESTART_BUTTON, &CVideoProcessorDlg::OnBnClickedRendererRestart)
	ON_BN_CLICKED(IDC_RENDERER_RESET_BUTTON, &CVideoProcessorDlg::OnBnClickedRendererReset)
	ON_BN_CLICKED(IDC_RENDERER_RESET_AUTO_CHECK, &CVideoProcessorDlg::OnBnClickedRendererResetAutoCheck)
	ON_BN_CLICKED(IDC_RENDERER_VIDEO_FRAME_USE_QUEUE_CHECK, &CVideoProcessorDlg::OnBnClickedRendererVideoFrameUseQueueCheck)

	// Command handlers
	ON_COMMAND(ID_COMMAND_FULLSCREEN_TOGGLE, &CVideoProcessorDlg::OnCommandFullScreenToggle)
	ON_COMMAND(ID_COMMAND_FULLSCREEN_EXIT, &CVideoProcessorDlg::OnCommandFullScreenExit)
END_MESSAGE_MAP()


static const std::vector<std::pair<LPCTSTR, RendererTimestamp>> RENDERER_TIMESTAMP_OPTIONS =
{
	std::make_pair(TEXT("Clock-clock"), RendererTimestamp::RENDERER_TIMESTAMP_CLOCK_CLOCK),
	std::make_pair(TEXT("Clock+Theo"),  RendererTimestamp::RENDERER_TIMESTAMP_CLOCK_THEO),
	std::make_pair(TEXT("Theoretical"), RendererTimestamp::RENDERER_TIMESTAMP_THEORETICAL),
	std::make_pair(TEXT("None"),        RendererTimestamp::RENDERER_TIMESTAMP_NONE),
};


static const std::vector<std::pair<LPCTSTR, DXVA_NominalRange>> DIRECTSHOW_NOMINAL_RANGE_OPTIONS =
{
	std::make_pair(TEXT("Auto"),             DXVA_NominalRange_Unknown),
	std::make_pair(TEXT("Full (0-255)"),     DXVA_NominalRange_0_255),
	std::make_pair(TEXT("Limited (16-235)"), DXVA_NominalRange_16_235),
	std::make_pair(TEXT("Small (48-208)"),   DXVA_NominalRange_48_208)
};


static const std::vector<std::pair<LPCTSTR, DXVA_VideoTransferFunction>> DIRECTSHOW_TRANSFER_FUNCTION_OPTIONS =
{
	std::make_pair(TEXT("Auto"),                      DXVA_VideoTransFunc_Unknown),
	std::make_pair(TEXT("PQ"),                        DIRECTSHOW_VIDEOTRANSFUNC_2084),
	std::make_pair(TEXT("Rec 709 (γ=2.2)"),           DXVA_VideoTransFunc_22_709),
	std::make_pair(TEXT("Bt.2020 constant"),          DIRECTSHOW_VIDEOTRANSFUNC_2020_const),

	std::make_pair(TEXT("True gamma 1.8"),            DXVA_VideoTransFunc_18),
	std::make_pair(TEXT("True gamma 2.0"),            DXVA_VideoTransFunc_20),
	std::make_pair(TEXT("True gamma 2.2"),            DXVA_VideoTransFunc_22),
	std::make_pair(TEXT("True gamma 2.6"),            DIRECTSHOW_VIDEOTRANSFUNC_26),
	std::make_pair(TEXT("True gamma 2.8"),            DXVA_VideoTransFunc_28),

	std::make_pair(TEXT("Linear RGB (γ=1.0)"),        DXVA_VideoTransFunc_10),
	std::make_pair(TEXT("204M (γ=2.2)"),              DXVA_VideoTransFunc_22_240M),
	std::make_pair(TEXT("8-bit gamma 2.2"),           DXVA_VideoTransFunc_22_8bit_sRGB),
	std::make_pair(TEXT("Log 100:1 H.264"),           DIRECTSHOW_VIDEOTRANSFUNC_Log_100),
	std::make_pair(TEXT("Log 316:1 H.264"),           DIRECTSHOW_VIDEOTRANSFUNC_Log_316),
	std::make_pair(TEXT("Rec 709 (γ=2.2) symmetric"), DIRECTSHOW_VIDEOTRANSFUNC_709_sym),
	std::make_pair(TEXT("Bt.2020 non-const"),         DIRECTSHOW_VIDEOTRANSFUNC_2020),
	std::make_pair(TEXT("Hybrid log"),                DIRECTSHOW_VIDEOTRANSFUNC_HLG)
};


static const std::vector<std::pair<LPCTSTR, DXVA_VideoTransferMatrix>> DIRECTSHOW_TRANSFER_MATRIX_OPTIONS =
{
	std::make_pair(TEXT("Auto"),       DXVA_VideoTransferMatrix_Unknown),
	std::make_pair(TEXT("BT.2020 10"), DIRECTSHOW_VIDEOTRANSFERMATRIX_BT2020_10),
	std::make_pair(TEXT("BT.2020 12"), DIRECTSHOW_VIDEOTRANSFERMATRIX_BT2020_12),
	std::make_pair(TEXT("BT.709"),     DXVA_VideoTransferMatrix_BT709),
	std::make_pair(TEXT("BT.601"),     DXVA_VideoTransferMatrix_BT601),
	std::make_pair(TEXT("240M"),       DXVA_VideoTransferMatrix_SMPTE240M),
	std::make_pair(TEXT("FCC"),        DIRECTSHOW_VIDEOTRANSFERMATRIX_FCC),
	std::make_pair(TEXT("YCgCo"),      DIRECTSHOW_VIDEOTRANSFERMATRIX_YCgCo)
};


static const std::vector<std::pair<LPCTSTR, DXVA_VideoPrimaries>> DIRECTSHOW_PRIMARIES_OPTIONS =
{
	std::make_pair(TEXT("Auto"),         DXVA_VideoPrimaries_Unknown),
	std::make_pair(TEXT("BT.2020"),      DIRECTSHOW_VIDEOPRIMARIES_BT2020),
	std::make_pair(TEXT("DCI-P3"),       DIRECTSHOW_VIDEOPRIMARIES_DCI_P3),
	std::make_pair(TEXT("BT.709"),       DXVA_VideoPrimaries_BT709),

	std::make_pair(TEXT("NTSC SysM"),    DXVA_VideoPrimaries_BT470_2_SysM),
	std::make_pair(TEXT("NTSC SysBG"),   DXVA_VideoPrimaries_BT470_2_SysBG),
	std::make_pair(TEXT("CIE 1931 XYZ"), DIRECTSHOW_VIDEOPRIMARIES_XYZ),
	std::make_pair(TEXT("ACES"),         DIRECTSHOW_VIDEOPRIMARIES_ACES),
};


//
// Constructor/destructor
//


CVideoProcessorDlg::CVideoProcessorDlg(bool startstartFullscreen):
	CDialog(CVideoProcessorDlg::IDD, nullptr),
	m_rendererfullScreen(startstartFullscreen)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_blackMagicDeviceDiscoverer = new BlackMagicDeckLinkCaptureDeviceDiscoverer(*this);
}


CVideoProcessorDlg::~CVideoProcessorDlg()
{
	for (auto& captureDevice : m_captureDevices)
		(*captureDevice).Release();
}


//
// UI-related handlers
//


void CVideoProcessorDlg::OnCaptureDeviceSelected()
{
	const int captureDeviceIndex = m_captureDeviceCombo.GetCurSel();
	if (captureDeviceIndex < 0)
		return;

	// Find input device based on IDeckLink* object
	for (auto& captureDevice : m_captureDevices)
	{
		if (m_captureDeviceCombo.GetItemDataPtr(captureDeviceIndex) == captureDevice.p)
		{
			m_desiredCaptureDevice = captureDevice;
			break;
		}
	}

	UpdateState();
}


void CVideoProcessorDlg::OnCaptureInputSelected()
{
	assert(m_captureDevice);

	const int captureInputIndex = m_captureInputCombo.GetCurSel();
	if (captureInputIndex < 0)
		return;

	const CaptureInputId selectedCaptureInputId = (CaptureInputId)m_captureInputCombo.GetItemData(captureInputIndex);
	assert(selectedCaptureInputId != INVALID_CAPTURE_INPUT_ID);

	m_desiredCaptureInputId = selectedCaptureInputId;

	UpdateState();
}


void CVideoProcessorDlg::OnRendererSelected()
{
	OnBnClickedRendererRestart();
}


void CVideoProcessorDlg::OnRendererTimestampSelected()
{
	OnBnClickedRendererRestart();
}


void CVideoProcessorDlg::OnRendererNominalRangeSelected()
{
	OnBnClickedRendererRestart();
}


void CVideoProcessorDlg::OnRendererTransferFunctionSelected()
{
	OnBnClickedRendererRestart();
}


void CVideoProcessorDlg::OnRendererTransferMatrixSelected()
{
	OnBnClickedRendererRestart();
}


void CVideoProcessorDlg::OnRendererPrimariesSelected()
{
	OnBnClickedRendererRestart();
}


void CVideoProcessorDlg::OnClose()
{
	DbgLog((LOG_TRACE, 1, TEXT("CVideoProcessorDlg::OnClose()")));

	if (m_wantToTerminate)
		return;

	// Set intent first, stopping the discoverer will lead to state update calls
	m_desiredCaptureDevice = nullptr;
	m_wantToTerminate = true;

	// Stop discovery
	if (m_blackMagicDeviceDiscoverer)
	{
		m_blackMagicDeviceDiscoverer->Stop();
		m_blackMagicDeviceDiscoverer.Release();
	}

	UpdateState();

	// Remove all renderers
	ClearRendererCombo();
}


void CVideoProcessorDlg::OnTimer(UINT_PTR nIDEvent)
{
	CString cstring;

	if (m_rendererState == RendererState::RENDERSTATE_RENDERING)
	{
		cstring.Format(_T("%lu"), m_renderer->GetFrameQueueSize());
		m_rendererVideoFrameQueueSizeText.SetWindowText(cstring);

		cstring.Format(_T("%.01f"), m_renderer->EntryLatencyMs());
		m_latencyToVPRendererText.SetWindowText(cstring);

		cstring.Format(_T("%.01f"), m_renderer->ExitLatencyMs());
		m_latencyToDSRendererText.SetWindowText(cstring);

		cstring.Format(_T("%lu"), m_renderer->DroppedFrameCount());
		m_rendererDroppedFrameCountText.SetWindowText(cstring);
	}
	else
	{
		m_rendererVideoFrameQueueSizeText.SetWindowText(_T(""));
		m_latencyToVPRendererText.SetWindowText(_T(""));
		m_latencyToDSRendererText.SetWindowText(_T(""));
		m_rendererDroppedFrameCountText.SetWindowText(TEXT(""));
	}

	if (m_captureDeviceState == CaptureDeviceState::CAPTUREDEVICESTATE_CAPTURING)
	{
		cstring.Format(_T("%lu"), m_captureDevice->VideoFrameCapturedCount());
		m_inputVideoFrameCountText.SetWindowText(cstring);

		cstring.Format(_T("%lu"), m_captureDevice->VideoFrameMissedCount());
		m_inputVideoFrameMissedText.SetWindowText(cstring);

		cstring.Format(_T("%.01f"), m_captureDevice->HardwareLatencyMs());
		m_latencyCaptureText.SetWindowText(cstring);
	}
	else
	{
		m_inputVideoFrameCountText.SetWindowText(TEXT(""));
		m_inputVideoFrameMissedText.SetWindowText(TEXT(""));

		m_latencyCaptureText.SetWindowText(_T(""));
	}

	// Prevent screensaver, this should be called "periodically" for whatever that means
	if (m_timerSeconds % 60 == 0)
	{
		SetThreadExecutionState(ES_DISPLAY_REQUIRED);
	}


	// Auto adjust
	if (m_timerSeconds % 5 == 0 &&
		m_rendererState == RendererState::RENDERSTATE_RENDERING)
	{
		assert(m_renderer);
		assert(m_captureDevice);

		bool queueOk = true;

		// Auto-click reset on renderer if requested
		const bool rendererResetAuto = m_rendererResetAutoCheck.GetCheck();
		if(rendererResetAuto)
		{
			const bool needsReset = m_renderer->GetFrameQueueSize() >= 3;
			queueOk = !needsReset;

			if (needsReset)
			{
				DbgLog((LOG_TRACE, 1, TEXT("CVideoProcessorDlg::OnTimer(): Resetting renderer")));
				m_renderer->Reset();
			}
		}

		// Auto update the clock frame offset to get just over zero
		// only do this if the queue is ok as it will have a major impact on the offset(or unmanaged)
		const bool timingClockFrameOffsetAuto = m_timingClockFrameOffsetAutoCheck.GetCheck();
		if (queueOk && timingClockFrameOffsetAuto)
		{
			const double videoFrameLead = -(m_renderer->ExitLatencyMs());

			const bool needsAdjusting =
				videoFrameLead < 0 ||
				videoFrameLead > (m_captureDeviceVideoState->displayMode->FrameDurationMs() * 2);

			if (needsAdjusting)
			{
				DbgLog((LOG_TRACE, 1, TEXT("CVideoProcessorDlg::OnTimer(): Adjusting clock frame offset + reset")));

				const int delta = -videoFrameLead;
				const int newOffset = GetTimingClockFrameOffsetMs() + delta;

				SetTimingClockFrameOffsetMs(newOffset);
				UpdateTimingClockFrameOffset();
			}
		}
	}

	++m_timerSeconds;
}


void CVideoProcessorDlg::OnBnClickedFullScreenButton()
{
	DbgLog((LOG_TRACE, 1, TEXT("CVideoProcessorDlg::OnBnClickedFullScreenButton()")));

	assert(!m_rendererfullScreen);  // Can happen in debug mode where the UI remains visible
	m_rendererfullScreen = true;

	m_wantToRestartRenderer = true;
	UpdateState();
}


void CVideoProcessorDlg::OnBnClickedTimingClockFrameOffsetAutoCheck()
{
	const bool checked = m_timingClockFrameOffsetAutoCheck.GetCheck();

	m_timingClockFrameOffsetEdit.EnableWindow(!checked);
}


void CVideoProcessorDlg::OnBnClickedRendererRestart()
{
	DbgLog((LOG_TRACE, 1, TEXT("CVideoProcessorDlg::OnBnClickedRendererRestart()")));

	if (m_rendererState == RendererState::RENDERSTATE_FAILED)
		m_rendererState = RendererState::RENDERSTATE_UNKNOWN;

	m_wantToRestartRenderer = true;
	UpdateState();
}


void CVideoProcessorDlg::OnBnClickedRendererReset()
{
	DbgLog((LOG_TRACE, 1, TEXT("CVideoProcessorDlg::OnBnClickedRendererReset()")));

	if (!m_renderer)
		return;

	m_renderer->Reset();
}


void CVideoProcessorDlg::OnBnClickedRendererResetAutoCheck()
{
	const bool checked = m_rendererResetAutoCheck.GetCheck();
}


void CVideoProcessorDlg::OnBnClickedRendererVideoFrameUseQueueCheck()
{
	DbgLog((LOG_TRACE, 1, TEXT("CVideoProcessorDlg::OnBnClickedRendererVideoFrameUseQueueCheck()")));

	const bool useQueue = m_rendererVideoFrameUseQeueueCheck.GetCheck();

	m_wantToRestartRenderer = true;
	UpdateState();
}


//
// Custom message handlers
//


LRESULT CVideoProcessorDlg::OnMessageCaptureDeviceFound(WPARAM wParam, LPARAM lParam)
{
	ACaptureDeviceComPtr captureDevice;
	captureDevice.Attach((ACaptureDevice*)wParam);

	DbgLog((LOG_TRACE, 1, TEXT("CVideoProcessorDlg::OnMessageCaptureDeviceFound(): %s"), captureDevice->GetName()));

	m_captureDevices.insert(captureDevice);

	RefreshCaptureDeviceList();

	return 0;
}


LRESULT	CVideoProcessorDlg::OnMessageCaptureDeviceLost(WPARAM wParam, LPARAM lParam)
{
	ACaptureDeviceComPtr captureDevice;
	captureDevice.Attach((ACaptureDevice*)wParam);

	DbgLog((LOG_TRACE, 1, TEXT("CVideoProcessorDlg::OnMessageCaptureDeviceLost(): %s"), captureDevice->GetName()));

	auto it = m_captureDevices.find(captureDevice);
	if (it == m_captureDevices.end())
		throw std::runtime_error("Removing unknown capture device?");

	// Device being removed is the one we're using, let's stop using it
	if (m_captureDevice == captureDevice)
	{
		m_desiredCaptureDevice = nullptr;
		UpdateState();
	}

	m_captureDevices.erase(it);

	RefreshCaptureDeviceList();

	return 0;
}


LRESULT	CVideoProcessorDlg::OnMessageCaptureDeviceStateChange(WPARAM wParam, LPARAM lParam)
{
	const CaptureDeviceState newState = (CaptureDeviceState)wParam;

	DbgLog((LOG_TRACE, 1, TEXT("CVideoProcessorDlg::OnMessageCaptureDeviceStateChange(): %s"), ToString(newState)));

	if (!m_captureDevice)
		return 0;

	assert(newState != m_captureDeviceState);

	m_captureDeviceState = newState;

	switch (newState)
	{
	case CaptureDeviceState::CAPTUREDEVICESTATE_READY:
		m_captureDeviceStateText.SetWindowText(TEXT("Ready"));
		break;

	case CaptureDeviceState::CAPTUREDEVICESTATE_CAPTURING:
		m_captureDeviceStateText.SetWindowText(TEXT("Capturing"));
		m_timingClockDescriptionText.SetWindowText(m_captureDevice->GetTimingClock()->TimingClockDescription());
		break;

	default:
		throw std::runtime_error("Unexpected state received from capture device");
	}

	UpdateState();

	return 0;
}


LRESULT CVideoProcessorDlg::OnMessageCaptureDeviceCardStateChange(WPARAM wParam, LPARAM lParam)
{
	CaptureDeviceCardStateComPtr cardState;
	cardState.Attach((CaptureDeviceCardState*)wParam);
	assert(cardState);

	DbgLog((LOG_TRACE, 1,
		TEXT("CVideoProcessorDlg::OnMessageCaptureDeviceStateChange(): Lock=%d"),
		cardState->inputLocked));

	// Input fields
	m_inputLockedText.SetWindowText(ToString(cardState->inputLocked));

	if (cardState->inputDisplayMode)
		m_inputDisplayModeText.SetWindowText(cardState->inputDisplayMode->ToString());
	else
		m_inputDisplayModeText.SetWindowText(TEXT(""));

	if (cardState->inputEncoding != Encoding::UNKNOWN)
		m_inputEncodingText.SetWindowText(ToString(cardState->inputEncoding));
	else
		m_inputEncodingText.SetWindowText(TEXT(""));

	if (cardState->inputBitDepth != BitDepth::UNKNOWN)
		m_inputBitDepthText.SetWindowText(ToString(cardState->inputBitDepth));
	else
		m_inputBitDepthText.SetWindowText(TEXT(""));

	// Other
	m_captureDeviceOtherList.ResetContent();
	for (auto& str : cardState->other)
	{
		m_captureDeviceOtherList.AddString(str);
	}

	UpdateState();

	return 0;
}


LRESULT CVideoProcessorDlg::OnMessageCaptureDeviceVideoStateChange(WPARAM wParam, LPARAM lParam)
{
	VideoStateComPtr videoState;
	videoState.Attach((VideoState*)wParam);

	DbgLog((LOG_TRACE, 1,
		TEXT("CVideoProcessorDlg::OnMessageCaptureDeviceVideoStateChange(): Valid=%d"),
		videoState->valid));

	const bool rendererAcceptedState = (bool)lParam;

	m_captureDeviceVideoState = videoState;

	assert(videoState);
	assert(m_captureDevice);

	if (videoState->valid)
	{
		m_videoValidText.SetWindowText(_T("Yes"));
		m_videoDisplayModeText.SetWindowText(videoState->displayMode->ToString());
		m_videoPixelFormatText.SetWindowText(ToString(videoState->pixelFormat));
		m_videoEotfText.SetWindowText(ToString(videoState->eotf));
		m_videoColorSpaceText.SetWindowText(ToString(videoState->colorspace));
	}
	else
	{
		m_videoValidText.SetWindowText(_T("No"));
		m_videoDisplayModeText.SetWindowText(_T(""));
		m_videoPixelFormatText.SetWindowText(_T(""));
		m_videoEotfText.SetWindowText(_T(""));
		m_videoColorSpaceText.SetWindowText(_T(""));
	}

	if (videoState->valid)
	{
		m_colorspaceCie1931xy.SetColorSpace(videoState->colorspace);
	}
	else
	{
		m_colorspaceCie1931xy.SetColorSpace(ColorSpace::UNKNOWN);
	}

	m_colorspaceCie1931xy.SetHDRData(videoState->hdrData);

	if (videoState->valid && videoState->hdrData)
	{
		const HDRData hdrData = *(videoState->hdrData);

		CString cstring;
		cstring.Format(
			_T("%.03f-%.01f"),
			hdrData.masteringDisplayMinLuminance,
			hdrData.masteringDisplayMaxLuminance);
		m_hdrDml.SetWindowText(cstring);

		cstring.Format(_T("%.01f"), hdrData.maxCll);
		m_hdrMaxCll.SetWindowText(cstring);

		cstring.Format(_T("%.01f"), hdrData.maxFall);
		m_hdrMaxFall.SetWindowText(cstring);
	}
	else
	{
		m_hdrDml.SetWindowText(_T(""));
		m_hdrMaxCll.SetWindowText(_T(""));
		m_hdrMaxFall.SetWindowText(_T(""));
	}

	// If the renderer did not accept the new state we need to restart the renderer
	if (!rendererAcceptedState)
	{
		m_wantToRestartRenderer = true;
	}

	// New round, new chances, reset state here
	if (m_rendererState == RendererState::RENDERSTATE_FAILED)
	{
		m_rendererState = RendererState::RENDERSTATE_UNKNOWN;
	}

	UpdateState();

	return 0;
}


// This is a handler for the DirectShow graph in the renderer,
// it works by using the GUI's message queue.
LRESULT CVideoProcessorDlg::OnMessageDirectShowNotification(WPARAM wParam, LPARAM lParam)
{
	if (m_renderer)
		if (FAILED(m_renderer->OnWindowsEvent(wParam, lParam)))
			throw std::runtime_error("Failed to handle windows event in renderer");

	return 0;
}


LRESULT CVideoProcessorDlg::OnMessageRendererStateChange(WPARAM wParam, LPARAM lParam)
{
	const RendererState newRendererState = (RendererState)wParam;

	DbgLog((LOG_TRACE, 1, TEXT("CVideoProcessorDlg::OnMessageRendererStateChange(): %s"), ToString(newRendererState)));

	assert(newRendererState != RendererState::RENDERSTATE_UNKNOWN);

	bool enableButtons = false;

	assert(m_renderer);
	assert(m_rendererState != newRendererState);
	m_rendererState = newRendererState;

	switch (m_rendererState)
	{
		// Renderer ready, can be started if wanted
	case RendererState::RENDERSTATE_READY:
		m_rendererStateText.SetWindowText(TEXT("Ready"));
		break;

		// Renderer running, ready for frames
	case RendererState::RENDERSTATE_RENDERING:
		m_deliverCaptureDataToRenderer.store(true, std::memory_order_release);
		enableButtons = true;
		m_rendererStateText.SetWindowText(TEXT("Rendering"));
		break;

		// Stopped rendering, can be cleaned up
	case RendererState::RENDERSTATE_STOPPED:
		RenderRemove();
		RenderGUIClear();
		m_rendererStateText.SetWindowText(TEXT("Stopped"));
		break;

	default:
		throw std::runtime_error("Unknown renderer state");
	}

	m_rendererFullscreenButton.EnableWindow(enableButtons);
	m_rendererRestartButton.EnableWindow(enableButtons);
	m_rendererResetButton.EnableWindow(enableButtons);

	UpdateState();

	return 0;
}


//
// Command handlers
//


void CVideoProcessorDlg::OnCommandFullScreenToggle()
{
	m_rendererfullScreen = !m_rendererfullScreen;

	m_wantToRestartRenderer = true;
	UpdateState();
}


void CVideoProcessorDlg::OnCommandFullScreenExit()
{
	// If fullscreen toggle off, else do nothing
	if (m_rendererfullScreen)
		OnCommandFullScreenToggle();
}



//
// ICaptureDeviceDiscovererCallback
//


void CVideoProcessorDlg::OnCaptureDeviceFound(ACaptureDeviceComPtr& captureDevice)
{
	// WARNING: Most likely to be called from some internal capture card thread!

	assert(captureDevice);

	PostMessage(
		WM_MESSAGE_CAPTURE_DEVICE_FOUND,
		(WPARAM)captureDevice.Detach(),
		0);
}


void CVideoProcessorDlg::OnCaptureDeviceLost(ACaptureDeviceComPtr& captureDevice)
{
	// WARNING: Most likely to be called from some internal capture card thread!

	assert(captureDevice);

	PostMessage(
		WM_MESSAGE_CAPTURE_DEVICE_LOST,
		(WPARAM)captureDevice.Detach(),
		0);
}


//
// ICaptureDeviceCallback
//


void CVideoProcessorDlg::OnCaptureDeviceState(CaptureDeviceState state)
{
	// WARNING: Often, but not always, called from some internal capture card thread!

	PostMessage(
		WM_MESSAGE_CAPTURE_DEVICE_STATE_CHANGE,
		state,
		0);
}


void CVideoProcessorDlg::OnCaptureDeviceCardStateChange(CaptureDeviceCardStateComPtr cardState)
{
	// WARNING: Most likely to be called from some internal capture card thread!

	assert(cardState);

	PostMessage(
		WM_MESSAGE_CAPTURE_DEVICE_CARD_STATE_CHANGE,
		(WPARAM)cardState.Detach(),
		0);
}


void CVideoProcessorDlg::OnCaptureDeviceVideoStateChange(VideoStateComPtr videoState)
{
	// WARNING: Most likely to be called from some internal capture card thread!

	assert(videoState);
	bool rendererAcceptedState = true;  // If no renderer it'll have no problems with the new state

	// This is an atomic bool which is set by the main thread and used in context of the
	// capture thread which will deliver frames.
	if (m_deliverCaptureDataToRenderer.load(std::memory_order_acquire))
	{
		assert(m_captureDevice);
		assert(m_captureDeviceState == CaptureDeviceState::CAPTUREDEVICESTATE_CAPTURING);
		assert(m_renderer);
		assert(m_rendererState == RendererState::RENDERSTATE_RENDERING);

		rendererAcceptedState = m_renderer->OnVideoState(videoState);
	}

	PostMessage(
		WM_MESSAGE_CAPTURE_DEVICE_VIDEO_STATE_CHANGE,
		(WPARAM)videoState.Detach(),
		rendererAcceptedState);
}


void CVideoProcessorDlg::OnCaptureDeviceVideoFrame(VideoFrame& videoFrame)
{
	// WARNING: Most likely to be called from some internal capture card thread!

	// This is an atomic bool which is set by the main thread and used in context of the
	// capture thread which will deliver frames.
	if (m_deliverCaptureDataToRenderer.load(std::memory_order_acquire))
	{
		assert(m_captureDevice);
		assert(m_captureDeviceState == CaptureDeviceState::CAPTUREDEVICESTATE_CAPTURING);
		assert(m_renderer);
		assert(m_rendererState == RendererState::RENDERSTATE_RENDERING);

		m_renderer->OnVideoFrame(videoFrame);
	}
}


void CVideoProcessorDlg::OnRendererState(RendererState rendererState)
{
	// Will be called synchronous as a response to our calls and hence does
	// not need posting messages, we still do so to de-couple actions.

	PostMessage(
		WM_MESSAGE_RENDERER_STATE_CHANGE,
		rendererState,
		0);
}


void CVideoProcessorDlg::UpdateState()
{
	DbgLog((LOG_TRACE, 1, TEXT("CVideoProcessorDlg::UpdateState()")));

	// Want to change cards
	if (m_desiredCaptureDevice != m_captureDevice)
	{
		m_captureInputCombo.EnableWindow(FALSE);

		// Have a render and it's rendering, stop it
		if (m_renderer &&
			m_rendererState == RendererState::RENDERSTATE_RENDERING)
			RenderStop();

		// Waiting for render to go away
		// (This has to come before stopping the capture as the renderer might be
		//  using the capture card as a clock.)
		if (m_renderer)
			return;

		// Have a capture and it's capturing, stop it
		if (m_captureDevice &&
			m_captureDeviceState == CaptureDeviceState::CAPTUREDEVICESTATE_CAPTURING)
			CaptureStop();

		// If capture device is stopped we're happy to remove it
		if (m_captureDevice && m_captureDeviceState == CaptureDeviceState::CAPTUREDEVICESTATE_READY)
			CaptureRemove();

		// We're waiting for the capture device to go away
		if (m_captureDevice)
			return;

		// From this point on we should be clean, set up new card if so desired
		assert(!m_renderer);
		assert(!m_captureDevice);
		assert(m_rendererState == RendererState::RENDERSTATE_UNKNOWN);
		assert(m_captureDeviceState == CaptureDeviceState::CAPTUREDEVICESTATE_UNKNOWN);

		if (m_desiredCaptureDevice)
		{
			m_captureDevice = m_desiredCaptureDevice;
			m_captureDevice->SetCallbackHandler(this);
			m_captureDevice->SetFrameOffsetMs(GetTimingClockFrameOffsetMs());

			RefreshInputConnectionCombo();

			m_captureInputCombo.EnableWindow(TRUE);
		}
	}

	// Capture card gone, but still have renderer, can't live for much longer
	if (!m_captureDevice && m_renderer)
	{
		assert(m_rendererState != RendererState::RENDERSTATE_RENDERING);
		return;
	}

	// If we want to terminate at this point we should be good to do so
	if (m_wantToTerminate)
	{
		assert(!m_captureDevice);
		assert(!m_desiredCaptureDevice);
		assert(!m_renderer);
		assert(m_captureDeviceState == CaptureDeviceState::CAPTUREDEVICESTATE_UNKNOWN);
		assert(
			m_rendererState == RendererState::RENDERSTATE_UNKNOWN ||
			m_rendererState == RendererState::RENDERSTATE_FAILED ||
			m_rendererState == RendererState::RENDERSTATE_STOPPED);

		CDialog::EndDialog(S_OK);
		return;
	}

	// If we don't have a capture card here we don't want to.
	if (!m_captureDevice)
	{
		assert(!m_desiredCaptureDevice);
		return;
	}

	assert(m_desiredCaptureDevice == m_captureDevice);

	// Capture device still starting up
	if (m_captureDeviceState == CaptureDeviceState::CAPTUREDEVICESTATE_UNKNOWN)
		return;

	assert(m_captureDeviceState != CaptureDeviceState::CAPTUREDEVICESTATE_UNKNOWN);

	// Have the right capture device, but want different input
	if (m_desiredCaptureInputId != INVALID_CAPTURE_INPUT_ID &&
		m_desiredCaptureInputId != m_currentCaptureInputId)
	{
		// Have a render and it's rendering, stop it
		if (m_renderer &&
			m_rendererState == RendererState::RENDERSTATE_RENDERING)
			RenderStop();

		// Waiting for render to go away
		// (This has to come before stopping the capture as the renderer might be
		//  using the capture card as a clock.)
		if (m_renderer)
			return;

		// Have a capture and it's capturing, stop it
		if (m_captureDevice &&
			m_captureDeviceState == CaptureDeviceState::CAPTUREDEVICESTATE_CAPTURING)
			CaptureStop();

		// Waiting for the capture to be stopped
		if (m_captureDeviceState != CaptureDeviceState::CAPTUREDEVICESTATE_READY)
			return;

		// From this point on we should be clean, set up new card
		assert(!m_renderer);
		assert(m_rendererState == RendererState::RENDERSTATE_UNKNOWN);
		assert(m_captureDeviceState == CaptureDeviceState::CAPTUREDEVICESTATE_READY);

		m_captureDevice->SetCaptureInput(m_desiredCaptureInputId);
		m_currentCaptureInputId = m_desiredCaptureInputId;

		CaptureStart();
		return;
	}

	// Have the right card and the right input
	assert(m_captureDevice);
	assert(m_desiredCaptureDevice == m_captureDevice);
	assert(m_desiredCaptureInputId == m_currentCaptureInputId);

	// Only continue if we're actually capturing
	if (m_captureDeviceState != CaptureDeviceState::CAPTUREDEVICESTATE_CAPTURING)
		return;

	//
	// Capture is good from here on out
	//

	// No render, start one if the current state is not failed and we have a valid video state
	if (!m_renderer)
	{
		// If we still have a full screen window and don't want to be full screen anymore clean it up
		if (!m_rendererfullScreen && m_fullScreenRenderWindow)
		{
			FullScreenWindowDestroy();

			m_fullScreenRenderWindow = nullptr;
		}

		// If the renderer failed we don't auto-start it again but wait for something to happen
		if (m_rendererState == RendererState::RENDERSTATE_FAILED)
			return;

		if (m_captureDeviceVideoState &&
			m_captureDeviceVideoState->valid)
			RenderStart();

		return;
	}

	assert(m_renderer);

	// If we have a renderer but the video state is invalid stop if rendering
	if (m_rendererState == RendererState::RENDERSTATE_RENDERING &&
		(!m_captureDeviceVideoState ||
	  	 !m_captureDeviceVideoState->valid))
	{
		RenderStop();
		return;
	}

	// Somebody wants to restart rendering, allrighty then
	if (m_rendererState == RendererState::RENDERSTATE_RENDERING &&
		m_wantToRestartRenderer)
	{
		m_wantToRestartRenderer = false;

		RenderStop();
		return;
	}

	// We have a renderer and a valid video state, relax and enjoy the show
	assert(m_captureDeviceVideoState);
	//assert(m_captureDeviceVideoState->valid);
}


//
// Helpers
//


void CVideoProcessorDlg::RefreshCaptureDeviceList()
{
	// Rebuild combo box with all devices which can capture
	m_captureDeviceCombo.ResetContent();

	for (auto& captureDevice : m_captureDevices)
	{
		if (!captureDevice->CanCapture())
			continue;

		const int index = m_captureDeviceCombo.AddString(captureDevice->GetName());
		m_captureDeviceCombo.SetItemDataPtr(index, (void*)captureDevice.p);

		// Retain selected device even if combo box position has changed
		if (captureDevice == m_captureDevice)
			m_captureDeviceCombo.SetCurSel(index);
	}

	if (m_captureDeviceCombo.GetCount() > 0)
	{
		m_captureDeviceCombo.EnableWindow(TRUE);

		// Select first capture device if none is selected yet
		const int index = m_captureDeviceCombo.GetCurSel();
		if (index == CB_ERR)
		{
			m_captureDeviceCombo.SetCurSel(0);
			OnCaptureDeviceSelected();
		}
	}
	else
	{
		m_captureDeviceCombo.EnableWindow(FALSE);

		if (m_captureDevice)
		{
			m_desiredCaptureDevice = nullptr;
			UpdateState();
		}
	}
}


void CVideoProcessorDlg::RefreshInputConnectionCombo()
{
	assert(m_captureDevice);

	const CaptureInputs captureInputs = m_captureDevice->SupportedCaptureInputs();
	const CaptureInputId currentCaptureInputId = m_captureDevice->CurrentCaptureInputId();

	m_captureInputCombo.ResetContent();

	int index;
	for (auto& captureInput : captureInputs)
	{
		index = m_captureInputCombo.AddString(captureInput.name);
		m_captureInputCombo.SetItemData(index, captureInput.id);

		// If we're in a known state keep current selection
		if(m_captureDeviceState != CaptureDeviceState::CAPTUREDEVICESTATE_UNKNOWN &&
			captureInput.id == currentCaptureInputId)
		{
			m_captureInputCombo.SetCurSel(index);
			OnCaptureInputSelected();
		}
	}

	// If no input connection has been selected, select first index
	if (m_captureInputCombo.GetCount() > 0)
	{
		index = m_captureInputCombo.GetCurSel();

		// Nothing selected yet
		if (index == CB_ERR)
		{
			// Iterate all options
			bool found = false;
			for (int i = 0; i < m_captureInputCombo.GetCount(); i++)
			{
				const int n = m_captureInputCombo.GetLBTextLen(i);

				CString str;
				m_captureInputCombo.GetLBText(i, str.GetBuffer(n));
				str.ReleaseBuffer();

				if (str == TEXT("HDMI"))
				{
					m_captureInputCombo.SetCurSel(i);
					found = true;
					break;
				}
			}

			// Nothing found, just take first
			if(!found)
				m_captureInputCombo.SetCurSel(0);

			OnCaptureInputSelected();
		}
	}
}


void CVideoProcessorDlg::CaptureStart()
{
	DbgLog((LOG_TRACE, 1, TEXT("CVideoProcessorDlg::CaptureStart()")));

	assert(m_captureDevice);
	assert(m_captureDeviceState == CaptureDeviceState::CAPTUREDEVICESTATE_READY);
	assert(!m_renderer);
	assert(m_rendererState == RendererState::RENDERSTATE_UNKNOWN);

	// Update internal state before call to StartCapture as that might be synchronous
	m_captureDeviceState = CaptureDeviceState::CAPTUREDEVICESTATE_STARTING;

	m_captureDevice->StartCapture();

	// Update GUI
	m_captureDeviceStateText.SetWindowText(TEXT("Starting"));
}


void CVideoProcessorDlg::CaptureStop()
{
	DbgLog((LOG_TRACE, 1, TEXT("CVideoProcessorDlg::CaptureStop(): Begin")));

	assert(m_captureDevice);
	assert(m_captureDeviceState == CaptureDeviceState::CAPTUREDEVICESTATE_CAPTURING);
	assert(!m_renderer);
	assert(m_rendererState == RendererState::RENDERSTATE_UNKNOWN);

	// Update internal state before call to StartCapture as that might be synchronous
	m_captureDeviceState = CaptureDeviceState::CAPTUREDEVICESTATE_STOPPING;

	m_captureDevice->StopCapture();

	m_captureDeviceVideoState = nullptr;

	// Update GUI
	CaptureGUIClear();
	m_captureDeviceStateText.SetWindowText(TEXT("Stopping"));

	DbgLog((LOG_TRACE, 1, TEXT("CVideoProcessorDlg::CaptureStop(): End")));
}


void CVideoProcessorDlg::CaptureRemove()
{
	DbgLog((LOG_TRACE, 1, TEXT("CVideoProcessorDlg::CaptureRemove(): Begin")));

	assert(m_captureDevice);
	assert(m_captureDeviceState == CaptureDeviceState::CAPTUREDEVICESTATE_READY);
	assert(!m_renderer);
	assert(m_rendererState == RendererState::RENDERSTATE_UNKNOWN);

	m_captureDeviceState = CaptureDeviceState::CAPTUREDEVICESTATE_UNKNOWN;
	m_captureDevice->SetCallbackHandler(nullptr);
	m_captureDevice.Release();
	m_captureDevice = nullptr;

	m_desiredCaptureInputId = INVALID_CAPTURE_INPUT_ID;
	m_currentCaptureInputId = INVALID_CAPTURE_INPUT_ID;

	DbgLog((LOG_TRACE, 1, TEXT("CVideoProcessorDlg::CaptureRemove(): End")));
}


void CVideoProcessorDlg::CaptureGUIClear()
{
	// Capture device group
	m_captureDeviceStateText.SetWindowText(TEXT(""));
	m_captureDeviceOtherList.ResetContent();

	// Input group
	m_inputLockedText.SetWindowText(TEXT(""));
	m_inputDisplayModeText.SetWindowText(TEXT(""));
	m_inputEncodingText.SetWindowText(TEXT(""));
	m_inputBitDepthText.SetWindowText(TEXT(""));
	m_inputVideoFrameCountText.SetWindowText(TEXT(""));
	m_inputVideoFrameMissedText.SetWindowText(TEXT(""));

	// Captured video group
	m_videoValidText.SetWindowText(TEXT(""));
	m_videoDisplayModeText.SetWindowText(TEXT(""));
	m_videoPixelFormatText.SetWindowText(TEXT(""));
	m_videoEotfText.SetWindowText(TEXT(""));
	m_videoColorSpaceText.SetWindowText(TEXT(""));

	// Timing clock
	m_timingClockDescriptionText.SetWindowText(TEXT(""));

	// ColorSpace group
	m_colorspaceCie1931xy.SetColorSpace(ColorSpace::UNKNOWN);
	m_colorspaceCie1931xy.SetHDRData(nullptr);

	// HDR group
	m_hdrDml.SetWindowText(TEXT(""));
	m_hdrMaxCll.SetWindowText(TEXT(""));
	m_hdrMaxFall.SetWindowText(TEXT(""));
}


void CVideoProcessorDlg::RenderStart()
{
	DbgLog((LOG_TRACE, 1, TEXT("CVideoProcessorDlg::RenderStart(): Begin")));

	assert(!m_renderer);
	assert(m_rendererState == RendererState::RENDERSTATE_UNKNOWN ||
		   m_rendererState == RendererState::RENDERSTATE_STOPPED);

	assert(m_captureDevice);
	assert(m_captureDeviceState == CaptureDeviceState::CAPTUREDEVICESTATE_CAPTURING);

	int i;

	i = m_rendererCombo.GetCurSel();

	// No renderer picked yet, ignore
	if (i < 0)
		return;

	GUID* rendererClSID= (GUID*)m_rendererCombo.GetItemData(i);

	// Get timestamp
	i = m_rendererTimestampCombo.GetCurSel();
	assert(i >= 0);
	RendererTimestamp rendererTimestamp = (RendererTimestamp)m_rendererTimestampCombo.GetItemData(i);

	// Capture card always provides the clock
	ITimingClock* timingClock = m_captureDevice->GetTimingClock();
	if (!timingClock)
		throw std::runtime_error("Failed to get timing clock from capture card");

	m_rendererBox.SetWindowTextW(TEXT("Starting..."));
	m_rendererState = RendererState::RENDERSTATE_STARTING;

	// Try to construct and start a VideoInfo2 renderer
	try
	{
		i = m_rendererNominalRangeCombo.GetCurSel();
		assert(i >= 0);
		DXVA_NominalRange forceNominalRange = (DXVA_NominalRange)m_rendererNominalRangeCombo.GetItemData(i);

		i = m_rendererTransferFunctionCombo.GetCurSel();
		assert(i >= 0);
		DXVA_VideoTransferFunction forceVideoTransferFunction = (DXVA_VideoTransferFunction)m_rendererTransferFunctionCombo.GetItemData(i);

		i = m_rendererTransferMatrixCombo.GetCurSel();
		assert(i >= 0);
		DXVA_VideoTransferMatrix forceVideoTransferMatrix = (DXVA_VideoTransferMatrix)m_rendererTransferMatrixCombo.GetItemData(i);

		i = m_rendererPrimariesCombo.GetCurSel();
		assert(i >= 0);
		DXVA_VideoPrimaries forceVideoPrimaries = (DXVA_VideoPrimaries)m_rendererPrimariesCombo.GetItemData(i);

		m_renderer = new CVideoInfo2DirectShowRenderer(
			*rendererClSID,
			*this,
			GetRenderWindow(),
			this->GetSafeHwnd(),
			WM_MESSAGE_DIRECTSHOW_NOTIFICATION,
			timingClock,
			m_captureDeviceVideoState,
			rendererTimestamp,
			GetRendererVideoFrameUseQueue(),
			GetRendererVideoFrameQueueSizeMax(),
			forceNominalRange,
			forceVideoTransferFunction,
			forceVideoTransferMatrix,
			forceVideoPrimaries);

		if (!m_renderer)
			throw std::runtime_error("Failed to build CVideoInfo2DirectShowRenderer");

		m_renderer->Build();
		m_renderer->Start();
	}
	catch (std::runtime_error e)
	{
		if (m_renderer)
		{
			delete m_renderer;
			m_renderer = nullptr;
		}

		// That didn't work, try the fallback to VideoInfo1 renderer
		try
		{
			m_renderer = new CVideoInfo1DirectShowRenderer(
				*rendererClSID,
				*this,
				GetRenderWindow(),
				this->GetSafeHwnd(),
				WM_MESSAGE_DIRECTSHOW_NOTIFICATION,
				timingClock,
				m_captureDeviceVideoState,
				rendererTimestamp,
				GetRendererVideoFrameUseQueue(),
				GetRendererVideoFrameQueueSizeMax());

			if (!m_renderer)
				throw std::runtime_error("Failed to build CVideoInfo1DirectShowRenderer");

			m_renderer->Build();
			m_renderer->Start();
		}
		catch (std::runtime_error e)
		{
			if (m_renderer)
			{
				delete m_renderer;
				m_renderer = nullptr;
			}

			m_rendererState = RendererState::RENDERSTATE_FAILED;
			m_rendererStateText.SetWindowText(TEXT("Failed"));

			// Show error in renderer box
			// TODO: Move this code to some sort of reusable function
			size_t size = strlen(e.what()) + 1;
			wchar_t* wtext = new wchar_t[size];
			size_t outSize;
			mbstowcs_s(&outSize, wtext, size, e.what(), size - 1);
			m_rendererBox.SetWindowText(wtext);
			delete[] wtext;

			// Ensure we're not full screen anymore and update state
			m_rendererfullScreen = false;
			UpdateState();

			// Give the user a chance to try again
			m_rendererRestartButton.EnableWindow(true);
		}
	}

	m_rendererStateText.SetWindowText(TEXT("Started, waiting for image..."));

	DbgLog((LOG_TRACE, 1, TEXT("CVideoProcessorDlg::RenderStart(): End")));
}


void CVideoProcessorDlg::RenderStop()
{
	DbgLog((LOG_TRACE, 1, TEXT("CVideoProcessorDlg::RenderStop(): Begin")));

	assert(m_captureDevice);
	assert(m_captureDeviceState == CaptureDeviceState::CAPTUREDEVICESTATE_CAPTURING);

	assert(m_renderer);
	assert(m_rendererState == RendererState::RENDERSTATE_RENDERING);
	assert(m_deliverCaptureDataToRenderer.load(std::memory_order_acquire));

	// After this call no frames will ever go through to the renderer
	m_deliverCaptureDataToRenderer.store(false, std::memory_order_release);

	// Update internal state before call to StartCapture as that might be synchronous
	m_rendererState = RendererState::RENDERSTATE_STOPPING;

	m_renderer->Stop();

	m_rendererStateText.SetWindowText(TEXT("Stopping"));

	DbgLog((LOG_TRACE, 1, TEXT("CVideoProcessorDlg::RenderStop(): End")));
}


void CVideoProcessorDlg::RenderRemove()
{
	DbgLog((LOG_TRACE, 1, TEXT("CVideoProcessorDlg::RenderRemove(): Begin")));

	assert(m_renderer);
	assert(m_rendererState == RendererState::RENDERSTATE_STOPPED);
	assert(!m_deliverCaptureDataToRenderer);

	delete m_renderer;
	m_renderer = nullptr;

	m_rendererState = RendererState::RENDERSTATE_UNKNOWN;

	DbgLog((LOG_TRACE, 1, TEXT("CVideoProcessorDlg::RenderRemove(): End")));
}


void CVideoProcessorDlg::RenderGUIClear()
{
	m_rendererVideoFrameQueueSizeText.SetWindowText(TEXT(""));
	m_rendererDroppedFrameCountText.SetWindowText(TEXT(""));
}


void CVideoProcessorDlg::FullScreenWindowConstruct()
{
	assert(!m_fullScreenRenderWindow);

	HMONITOR hmon = MonitorFromWindow(this->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);

	m_fullScreenRenderWindow = new FullscreenWindow();
	if (!m_fullScreenRenderWindow)
		throw std::runtime_error("Failed to create full screen renderer window");

	m_fullScreenRenderWindow->Create(hmon, this->GetSafeHwnd());
}


void CVideoProcessorDlg::FullScreenWindowDestroy()
{
	assert(m_fullScreenRenderWindow);
	delete m_fullScreenRenderWindow;
	m_fullScreenRenderWindow = nullptr;
}


HWND CVideoProcessorDlg::GetRenderWindow()
{
	if (m_rendererfullScreen)
	{
		// If we don't have a full screen window yet make one
		if (!m_fullScreenRenderWindow)
			FullScreenWindowConstruct();

		assert(IsWindow(m_fullScreenRenderWindow->GetHWND()));
		return m_fullScreenRenderWindow->GetHWND();
	}

	assert(!m_fullScreenRenderWindow);
	assert(IsWindow(m_rendererBox));
	return m_rendererBox;
}


size_t CVideoProcessorDlg::GetRendererVideoFrameQueueSizeMax()
{
	// Note that this field is marked as numbers only so guaranteed to convert corrrectly

	CString text;
	m_rendererVideoFrameQueueSizeMaxEdit.GetWindowText(text);
	return _ttoi(text);
}


bool CVideoProcessorDlg::GetRendererVideoFrameUseQueue()
{
	return m_rendererVideoFrameUseQeueueCheck.GetCheck();
}


int CVideoProcessorDlg::GetTimingClockFrameOffsetMs()
{
	CString text;
	m_timingClockFrameOffsetEdit.GetWindowText(text);

	// ttoi throws non-parsed stuff away so in case there is crap set the output to the
	// used value, this way the user always knows what's going on.
	const int frameOffsetMs = _ttoi(text);
	SetTimingClockFrameOffsetMs(frameOffsetMs);

	return frameOffsetMs;
}


void CVideoProcessorDlg::SetTimingClockFrameOffsetMs(int timingClockFrameOffsetMs)
{
	CString cstring;
	cstring.Format(_T("%i"), timingClockFrameOffsetMs);
	m_timingClockFrameOffsetEdit.SetWindowText(cstring);
}


void CVideoProcessorDlg::UpdateTimingClockFrameOffset()
{
	if (m_captureDevice)
		m_captureDevice->SetFrameOffsetMs(GetTimingClockFrameOffsetMs());

	if (m_renderer)
		m_renderer->Reset();
}


void CVideoProcessorDlg::RebuildRendererCombo()
{
	ClearRendererCombo();

	struct RendererEntry
	{
		CString name;
		GUID guid;

		bool operator< (const RendererEntry& other) const {
			return name < other.name;
		}
	};
	std::vector<RendererEntry> rendererEntries;

	//
	// Iterate all renderers
	// https://docs.microsoft.com/en-us/windows/win32/directshow/using-the-filter-mapper
	// TODO: Move this to the directshow dir and make a nice clean abstraction to use here
	//
	{
		IFilterMapper2* pMapper = nullptr;
		IEnumMoniker* pEnum = nullptr;
		HRESULT hr;

		hr = CoCreateInstance(CLSID_FilterMapper2,
			nullptr, CLSCTX_INPROC, IID_IFilterMapper2,
			(void**)&pMapper);

		if (FAILED(hr))
			throw std::runtime_error("Failed to instantiate the filter mapper");

		GUID arrayInTypes[2];
		arrayInTypes[0] = MEDIATYPE_Video;
		arrayInTypes[1] = GUID_NULL;

		hr = pMapper->EnumMatchingFilters(
			&pEnum,
			0,                  // Reserved.
			TRUE,               // Use exact match?
			MERIT_DO_NOT_USE,   // Minimum merit.
			TRUE,               // At least one input pin?
			1,                  // Number of major type/subtype pairs for input.
			arrayInTypes,       // Array of major type/subtype pairs for input.
			nullptr,               // Input medium.
			nullptr,               // Input pin category.
			FALSE,              // Must be a renderer?
			FALSE,              // At least one output pin?
			0,                  // Number of major type/subtype pairs for output.
			nullptr,               // Array of major type/subtype pairs for output.
			nullptr,               // Output medium.
			nullptr);              // Output pin category.

		// Enumerate the monikers.
		IMoniker* pMoniker;
		ULONG cFetched;
		while (pEnum->Next(1, &pMoniker, &cFetched) == S_OK)
		{
			IPropertyBag* pPropBag = nullptr;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);

			if (SUCCEEDED(hr))
			{
				VARIANT nameVariant;
				VARIANT clsidVariant;
				VariantInit(&nameVariant);
				VariantInit(&clsidVariant);

				HRESULT nameHr = pPropBag->Read(L"FriendlyName", &nameVariant, 0);
				HRESULT clsidHr = pPropBag->Read(L"CLSID", &clsidVariant, 0);
				if (SUCCEEDED(nameHr) && SUCCEEDED(clsidHr))
				{
					CString name = nameVariant.bstrVal;

					// Filter by name.
					// Unforuntately renderes don't actually tell us if they can render
					// so we need a hand-maintained list here.
					// VR = Abbreviation of Video Renderer
					if (((name.Find(TEXT("Video")) >= 0) && (name.Find(TEXT("Render")) >= 0)) ||
						(name.Find(TEXT("VR")) >= 0)
						)
					{
						CString rendererEntityName;
						rendererEntityName.Format(_T("DirectShow - %s"), name);

						RendererEntry rendererEntry;
						rendererEntry.name = rendererEntityName;

						hr = VariantToGUID(clsidVariant, &(rendererEntry.guid));
						if (FAILED(hr))
							throw std::runtime_error("Failed to convert veriant to GUID");

						rendererEntries.push_back(rendererEntry);
					}
				}

				VariantClear(&nameVariant);
				VariantClear(&clsidVariant);

				pPropBag->Release();
			}
			pMoniker->Release();
		}

		pMapper->Release();
		pEnum->Release();
	}

	//
	// Populate selection box, sorted
	//

	std::sort(rendererEntries.begin(), rendererEntries.end());
	for (const auto& rendererEntry : rendererEntries)
	{
		GUID* clsid = new GUID(rendererEntry.guid);

		int comboIndex = m_rendererCombo.AddString(rendererEntry.name);
		m_rendererCombo.SetItemData(comboIndex, (DWORD_PTR)clsid);
	}
}


void CVideoProcessorDlg::ClearRendererCombo()
{
	for (int i = 0; i < m_rendererCombo.GetCount(); i++)
	{
		delete (void *)m_rendererCombo.GetItemData(i);
	}

	m_rendererCombo.ResetContent();
}


//
// CDialog
//


void CVideoProcessorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	// Capture device group
	DDX_Control(pDX, IDC_CAPTURE_DEVICE_COMBO, m_captureDeviceCombo);
	DDX_Control(pDX, IDC_CAPTURE_INPUT_COMBO, m_captureInputCombo);
	DDX_Control(pDX, IDC_CAPTURE_STATE_STATIC, m_captureDeviceStateText);
	DDX_Control(pDX, IDC_CAPTURE_DEVICE_OTHER_LIST, m_captureDeviceOtherList);

	// Input group
	DDX_Control(pDX, IDC_INPUT_LOCKED_STATIC, m_inputLockedText);
	DDX_Control(pDX, IDC_INPUT_DISPLAY_MODE_STATIC, m_inputDisplayModeText);
	DDX_Control(pDX, IDC_INPUT_ENCODING_STATIC, m_inputEncodingText);
	DDX_Control(pDX, IDC_INPUT_BIT_DEPTH_STATIC, m_inputBitDepthText);
	DDX_Control(pDX, IDC_INPUT_VIDEO_FRAME_COUNT_STATIC, m_inputVideoFrameCountText);
	DDX_Control(pDX, IDC_INPUT_VIDEO_FRAME_MISSED_STATIC, m_inputVideoFrameMissedText);

	// Captured video group
	DDX_Control(pDX, IDC_VIDEO_VALID_STATIC, m_videoValidText);
	DDX_Control(pDX, IDC_VIDEO_DISPLAY_MODE_STATIC, m_videoDisplayModeText);
	DDX_Control(pDX, IDC_VIDEO_PIXEL_FORMAT_STATIC, m_videoPixelFormatText);
	DDX_Control(pDX, IDC_VIDEO_EOTF_STATIC, m_videoEotfText);
	DDX_Control(pDX, IDC_VIDEO_COLORSPACE_STATIC, m_videoColorSpaceText);

	// Timing clock group
	DDX_Control(pDX, IDC_TIMING_CLOCK_DESCRIPTION_STATIC, m_timingClockDescriptionText);
	DDX_Control(pDX, IDC_TIMING_CLOCK_FRAME_OFFSET_EDIT, m_timingClockFrameOffsetEdit);
	DDX_Control(pDX, IDC_TIMING_CLOCK_FRAME_OFFSET_AUTO_CHECK, m_timingClockFrameOffsetAutoCheck);

	// ColorSpace group
	DDX_Control(pDX, IDC_CIE1931XY_GRAPH, m_colorspaceCie1931xy);

	//  HDR group
	DDX_Control(pDX, IDC_HDR_DML_STATIC, m_hdrDml);
	DDX_Control(pDX, IDC_HDR_MAX_CLL_STATIC, m_hdrMaxCll);
	DDX_Control(pDX, IDC_HDR_MAX_FALL_STATIC, m_hdrMaxFall);

	// Latency group
	DDX_Control(pDX, IDC_LATENCY_CAPTURE_STATIC, m_latencyCaptureText);
	DDX_Control(pDX, IDC_LATENCY_TO_VP_RENDERER_STATIC, m_latencyToVPRendererText);
	DDX_Control(pDX, IDC_LATENCY_TO_DS_RENDERER_STATIC, m_latencyToDSRendererText);

	// Renderer group
	DDX_Control(pDX, IDC_RENDERER_COMBO, m_rendererCombo);
	DDX_Control(pDX, IDC_RENDERER_TIMESTAMP_COMBO, m_rendererTimestampCombo);
	DDX_Control(pDX, IDC_RENDERER_NOMINAL_RANGE_COMBO, m_rendererNominalRangeCombo);
	DDX_Control(pDX, IDC_RENDERER_TRANSFER_FUNCTION_COMBO, m_rendererTransferFunctionCombo);
	DDX_Control(pDX, IDC_RENDERER_TRANSFER_MATRIX_COMBO, m_rendererTransferMatrixCombo);
	DDX_Control(pDX, IDC_RENDERER_PRIMARIES_COMBO, m_rendererPrimariesCombo);
	DDX_Control(pDX, IDC_FULL_SCREEN_BUTTON, m_rendererFullscreenButton);
	DDX_Control(pDX, IDC_RENDERER_RESTART_BUTTON, m_rendererRestartButton);
	DDX_Control(pDX, IDC_RENDERER_RESET_BUTTON, m_rendererResetButton);
	DDX_Control(pDX, IDC_RENDERER_RESET_AUTO_CHECK, m_rendererResetAutoCheck);
	DDX_Control(pDX, IDC_RENDERER_STATE_STATIC, m_rendererStateText);
	DDX_Control(pDX, IDC_RENDERER_VIDEO_FRAME_USE_QUEUE_CHECK, m_rendererVideoFrameUseQeueueCheck);
	DDX_Control(pDX, IDC_RENDERER_VIDEO_FRAME_QUEUE_SIZE_STATIC, m_rendererVideoFrameQueueSizeText);
	DDX_Control(pDX, IDC_RENDERER_VIDEO_FRAME_QUEUE_SIZE_MAX_EDIT, m_rendererVideoFrameQueueSizeMaxEdit);
	DDX_Control(pDX, IDC_RENDERER_DROPPED_FRAME_COUNT_STATIC, m_rendererDroppedFrameCountText);
	DDX_Control(pDX, IDC_RENDERER_BOX, m_rendererBox);
}


// Called when the dialog box is initialized
BOOL CVideoProcessorDlg::OnInitDialog()
{
	if (!CDialog::OnInitDialog())
		return FALSE;

	CString title;
	title.Format(_T("VideoProcessor (%s)"), VERSION_DESCRIBE);
	SetWindowText(title.GetBuffer());

	SetIcon(m_hIcon, FALSE);

	// Set initial dialog size as minimum size
	CRect rectWindow;
	GetWindowRect(rectWindow);
	m_minDialogSize = rectWindow.Size();

	// Empty popup menus
	m_captureDeviceCombo.ResetContent();

	// Disable the interface
	m_captureDeviceCombo.EnableWindow(FALSE);
	m_captureInputCombo.EnableWindow(FALSE);

	// Get all renderers
	RebuildRendererCombo();

	// Fill renderer selection boxes
	for (const auto& p : RENDERER_TIMESTAMP_OPTIONS)
	{
		int index = m_rendererTimestampCombo.AddString(p.first);
		m_rendererTimestampCombo.SetItemData(index, (int)p.second);
	}
	m_rendererTimestampCombo.SetCurSel(0);

	for (const auto& p : DIRECTSHOW_NOMINAL_RANGE_OPTIONS)
	{
		int index = m_rendererNominalRangeCombo.AddString(p.first);
		m_rendererNominalRangeCombo.SetItemData(index, p.second);
	}
	m_rendererNominalRangeCombo.SetCurSel(0);

	for (const auto& p : DIRECTSHOW_TRANSFER_FUNCTION_OPTIONS)
	{
		int index = m_rendererTransferFunctionCombo.AddString(p.first);
		m_rendererTransferFunctionCombo.SetItemData(index, (int)p.second);
	}
	m_rendererTransferFunctionCombo.SetCurSel(0);

	for (const auto& p : DIRECTSHOW_TRANSFER_MATRIX_OPTIONS)
	{
		int index = m_rendererTransferMatrixCombo.AddString(p.first);
		m_rendererTransferMatrixCombo.SetItemData(index, (int)p.second);
	}
	m_rendererTransferMatrixCombo.SetCurSel(0);

	for (const auto& p : DIRECTSHOW_PRIMARIES_OPTIONS)
	{
		int index = m_rendererPrimariesCombo.AddString(p.first);
		m_rendererPrimariesCombo.SetItemData(index, (int)p.second);
	}
	m_rendererPrimariesCombo.SetCurSel(0);

	// Start discovery services
	m_blackMagicDeviceDiscoverer->Start();

	m_accelerator = LoadAccelerators(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR1));
	if (!m_accelerator)
		throw std::runtime_error("Failed to load accelerator");

	CaptureGUIClear();
	RenderGUIClear();

	m_rendererVideoFrameQueueSizeMaxEdit.SetWindowText(TEXT("32"));
	m_timingClockFrameOffsetEdit.SetWindowText(TEXT("90"));
	m_rendererVideoFrameUseQeueueCheck.SetCheck(true);
	m_rendererResetAutoCheck.SetCheck(true);

	// Start timers
	SetTimer(TIMER_ID_1SECOND, 1000, nullptr);

	return TRUE;
}


BOOL CVideoProcessorDlg::PreTranslateMessage(MSG* pMsg)
{
	// Handle accelerator combinations
	if (m_accelerator)
	{
		if (::TranslateAccelerator(m_hWnd, m_accelerator, pMsg))
			return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CVideoProcessorDlg::OnOK()
{
	// Called if the user presses enter somewhere

	CWnd* pwndCtrl = GetFocus();
	int ctrl_ID = pwndCtrl->GetDlgCtrlID();

	switch (ctrl_ID)
	{
		case IDC_TIMING_CLOCK_FRAME_OFFSET_EDIT:
			UpdateTimingClockFrameOffset();
			break;

		case IDC_RENDERER_VIDEO_FRAME_QUEUE_SIZE_MAX_EDIT:
			if (m_renderer)
			{
				m_renderer->SetFrameQueueMaxSize(GetRendererVideoFrameQueueSizeMax());
			}
			break;

		default:
			m_wantToRestartRenderer = true;
			UpdateState();
	}

	// Don't call the super implmenetation as that will close the window
}


void CVideoProcessorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}


void CVideoProcessorDlg::OnSize(UINT nType, int cx, int cy)
{
	if (m_renderer)
		m_renderer->OnSize();

	CDialog::OnSize(nType, cx, cy);
}


void CVideoProcessorDlg::OnSetFocus(CWnd* pOldWnd)
{
	CDialog::OnSetFocus(pOldWnd);
}


HCURSOR CVideoProcessorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CVideoProcessorDlg::OnGetMinMaxInfo(MINMAXINFO* minMaxInfo)
{
	CDialog::OnGetMinMaxInfo(minMaxInfo);

	// Guarantee minimum size of window
	minMaxInfo->ptMinTrackSize.x = std::max(minMaxInfo->ptMinTrackSize.x, m_minDialogSize.cx);
	minMaxInfo->ptMinTrackSize.y = std::max(minMaxInfo->ptMinTrackSize.y, m_minDialogSize.cy);
}
