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

#include <cie.h>
#include <resource.h>
#include <VideoProcessorApp.h>
#include <microsoft_directshow/madvr_renderer/DirectShowMadVRRenderer.h>

#include "VideoProcessorDlg.h"


BEGIN_MESSAGE_MAP(CVideoProcessorDlg, CDialog)

	// Pre-baked callbacks
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
	ON_WM_QUERYDRAGICON()
	ON_WM_GETMINMAXINFO()
	ON_WM_CLOSE()

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
	ON_CBN_SELCHANGE(IDC_RENDERER_PIXEL_VALUE_RANGE_COMBO, &CVideoProcessorDlg::OnRendererPixelValueRangeSelected)
	ON_BN_CLICKED(IDC_FULL_SCREEN_BUTTON, &CVideoProcessorDlg::OnBnClickedFullScreenButton)
	ON_BN_CLICKED(IDC_RENDERER_RESTART_BUTTON, &CVideoProcessorDlg::OnBnClickedRendererRestart)

	// Command handlers
	ON_COMMAND(ID_COMMAND_FULLSCREEN, &CVideoProcessorDlg::OnCommandFullScreen)
END_MESSAGE_MAP()


//
// Constructor/destructor
//


CVideoProcessorDlg::CVideoProcessorDlg(CWnd* pParent) :
	CDialog(CVideoProcessorDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
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


void CVideoProcessorDlg::OnRendererPixelValueRangeSelected()
{
	const int rendererPixelValueRangeIndex = m_rendererPixelValueRangeCombo.GetCurSel();
	if (rendererPixelValueRangeIndex < 0)
		return;

	const PixelValueRange pixelValueRange = (PixelValueRange)m_rendererPixelValueRangeCombo.GetItemData(rendererPixelValueRangeIndex);

	m_desiredRendererPixelValueRange = pixelValueRange;

	UpdateState();
}


void CVideoProcessorDlg::OnClose()
{
	if (m_wantToTerminate)
		return;

	// Stop discovery
	if (m_blackMagicDeviceDiscoverer)
	{
		m_blackMagicDeviceDiscoverer->Stop();
		m_blackMagicDeviceDiscoverer.Release();
	}

	m_desiredCaptureDevice = nullptr;
	m_wantToTerminate = true;
	UpdateState();
}


void CVideoProcessorDlg::OnBnClickedFullScreenButton()
{
	if (m_renderer)
		m_renderer->GoFullScreen(true);
}


void CVideoProcessorDlg::OnBnClickedRendererRestart()
{
	if (m_rendererState == RendererState::RENDERSTATE_FAILED)
		m_rendererState = RendererState::RENDERSTATE_UNKNOWN;

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

	m_captureDevices.insert(captureDevice);

	RefreshCaptureDeviceList();

	return 0;
}


LRESULT	CVideoProcessorDlg::OnMessageCaptureDeviceLost(WPARAM wParam, LPARAM lParam)
{
	ACaptureDeviceComPtr captureDevice;
	captureDevice.Attach((ACaptureDevice*)wParam);

	auto it = m_captureDevices.find(captureDevice);
	if (it == m_captureDevices.end())
		throw std::exception("Removing unknown capture device?");

	// Device being removed is the one we're using
	if (m_captureDevice == captureDevice)
	{
		RemoveCapture();
	}

	m_captureDevices.erase(it);

	RefreshCaptureDeviceList();

	return 0;
}


LRESULT	CVideoProcessorDlg::OnMessageCaptureDeviceStateChange(WPARAM wParam, LPARAM lParam)
{
	const CaptureDeviceState newState = (CaptureDeviceState)wParam;

	if (!m_captureDevice)
	{
		return 0;
	}

	assert(newState != m_captureDeviceState);

	m_captureDeviceState = newState;

	switch (newState)
	{
	case CaptureDeviceState::CAPTUREDEVICESTATE_READY:
		m_captureDeviceStateText.SetWindowText(TEXT("Ready"));
		break;
	case CaptureDeviceState::CAPTUREDEVICESTATE_CAPTURING:
		m_captureDeviceStateText.SetWindowText(TEXT("Capturing"));
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
			_T("%.03f,%.03f"),
			hdrData.displayPrimaryRedX,
			hdrData.displayPrimaryRedY);
		m_hdrDpRed.SetWindowText(cstring);

		cstring.Format(
			_T("%.03f,%.03f"),
			hdrData.displayPrimaryGreenX,
			hdrData.displayPrimaryGreenY);
		m_hdrDpGreen.SetWindowText(cstring);

		cstring.Format(
			_T("%.03f,%.03f"),
			hdrData.displayPrimaryBlueX,
			hdrData.displayPrimaryBlueY);
		m_hdrDpBlue.SetWindowText(cstring);

		// Whitepoint (name known ones)
		// https://en.wikipedia.org/wiki/Standard_illuminant
		cstring.Format(
			_T("%.03f,%.03f"),
			hdrData.whitePointX,
			hdrData.whitePointY);

		if (CieEquals(0.34567, hdrData.whitePointX) &&
			CieEquals(0.35850, hdrData.whitePointY))
			cstring = TEXT(" (D50)");
		if (CieEquals(0.33242, hdrData.whitePointX) &&
			CieEquals(0.34743, hdrData.whitePointY))
			cstring = TEXT(" (D55)");
		if (CieEquals(0.31271, hdrData.whitePointX) &&
			CieEquals(0.32902, hdrData.whitePointY))
			cstring = TEXT(" (D65)");
		if (CieEquals(0.29902, hdrData.whitePointX) &&
			CieEquals(0.31485, hdrData.whitePointY))
			cstring = TEXT(" (D75)");

		m_hdrWhitePoint.SetWindowText(cstring);

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
		m_hdrDpRed.SetWindowText(_T(""));
		m_hdrDpGreen.SetWindowText(_T(""));
		m_hdrDpBlue.SetWindowText(_T(""));
		m_hdrWhitePoint.SetWindowText(_T(""));
		m_hdrDml.SetWindowText(_T(""));
		m_hdrMaxCll.SetWindowText(_T(""));
		m_hdrMaxFall.SetWindowText(_T(""));
	}

	// If the renderer did not accept the new state we need to restart the renderer
	// TODO: This causes an interesting loop and double lock but is vital to auto switching...
	// 	     Maybe post the updatestate so it gets decoupled?
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
	// TODO: Renderer lock?

	if (m_renderer)
		if (FAILED(m_renderer->OnWindowsEvent(wParam, lParam)))
			throw std::runtime_error("Failed to handle windows event in renderer");

	return 0;
}


LRESULT CVideoProcessorDlg::OnMessageRendererStateChange(WPARAM wParam, LPARAM lParam)
{
	const RendererState newRendererState = (RendererState)wParam;
	assert(newRendererState != RendererState::RENDERSTATE_UNKNOWN);

	{
		std::lock_guard<std::mutex> lock(m_rendererMutex);

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
			m_deliverFramesToRenderer = true;
			m_rendererStateText.SetWindowText(TEXT("Rendering"));
			break;

		// Stopped rendering, can be cleaned up
		case RendererState::RENDERSTATE_STOPPED:
			RemoveRenderLocked();
			m_rendererStateText.SetWindowText(TEXT("Stopped"));
			break;

		default:
			throw std::runtime_error("Unknown renderer state");
		}
	}

	UpdateState();

	return 0;
}


//
// Command handlers
//


void CVideoProcessorDlg::OnCommandFullScreen()
{
	if (m_renderer)
		m_renderer->GoFullScreen(true);
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

	// Push to renderer first
	{
		std::lock_guard<std::mutex> lock(m_rendererMutex);

		if (m_renderer)
		{
			rendererAcceptedState = m_renderer->OnVideoState(videoState);
		}
	}

	PostMessage(
		WM_MESSAGE_CAPTURE_DEVICE_VIDEO_STATE_CHANGE,
		(WPARAM)videoState.Detach(),
		rendererAcceptedState);
}


void CVideoProcessorDlg::OnCaptureDeviceVideoFrame(VideoFrame& videoFrame)
{
	// WARNING: Most likely to be called from different thread, ensure proper locking

	std::lock_guard<std::mutex> lock(m_rendererMutex);

	assert(m_captureDevice);

	if (m_renderer && m_deliverFramesToRenderer)
		m_renderer->OnVideoFrame(videoFrame);
}


void CVideoProcessorDlg::OnRendererState(RendererState rendererState)
{
	// Will be called synchronous as a response to our calls and hence does not need posting messages.
	// we still do so to de-couple actions.

	PostMessage(
		WM_MESSAGE_RENDERER_STATE_CHANGE,
		rendererState,
		0);
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
			RemoveCapture();
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

		// If input connection is the active connection set combo to this item
		if (captureInput.id == currentCaptureInputId)
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


void CVideoProcessorDlg::StartCapture()
{
	assert(m_captureDevice);
	assert(m_captureDeviceState == CaptureDeviceState::CAPTUREDEVICESTATE_READY);

	// Update internal state before call to StartCapture as that might be synchronous
	m_captureDeviceState = CaptureDeviceState::CAPTUREDEVICESTATE_STARTING;

	// TODO: Return boolean and upon failure to start call StopCapture()
	// TODO: Check if safe to be called from other thread
	m_captureDevice->StartCapture();

	// Update GUI
	m_captureDeviceStateText.SetWindowText(TEXT("Starting"));
}


void CVideoProcessorDlg::StopCapture()
{
	assert(m_captureDevice);
	assert(m_captureDeviceState == CaptureDeviceState::CAPTUREDEVICESTATE_CAPTURING);

	// Update internal state before call to StartCapture as that might be synchronous
	m_captureDeviceState = CaptureDeviceState::CAPTUREDEVICESTATE_STOPPING;

	// TODO: Return boolean and upon failure and do something?
	// TODO: Check if safe to be called from other thread
	m_captureDevice->StopCapture();

	// Update GUI
	m_captureDeviceStateText.SetWindowText(TEXT("Stopping"));
}


void CVideoProcessorDlg::RemoveCapture()
{
	m_captureDevice->SetCallbackHandler(nullptr);

	m_captureDeviceState = CaptureDeviceState::CAPTUREDEVICESTATE_UNKNOWN;
	m_captureDevice.Release();
	m_captureDevice = nullptr;

	m_desiredCaptureInputId = INVALID_CAPTURE_INPUT_ID;
	m_currentCaptureInputId = INVALID_CAPTURE_INPUT_ID;
}


void CVideoProcessorDlg::StartRender()
{
	assert(!m_renderer);
	assert(m_rendererState == RendererState::RENDERSTATE_UNKNOWN);

	// Update internal state before call to StartCapture as that might be synchronous
	// TODO: Call should always be posted and never sync
	m_rendererState = RendererState::RENDERSTATE_STARTING;


	// Renderer
	// TODO: Should always be called from OnMessageCaptureDeviceVideoStateChange so that it can be constructed with the right info
	try
	{
		m_renderer = new DirectShowMadVRRenderer(
			*this,
			m_rendererBox,
			this->GetSafeHwnd(),
			WM_MESSAGE_DIRECTSHOW_NOTIFICATION,
			m_captureDeviceVideoState,
			m_desiredRendererPixelValueRange);

		if (!m_renderer)
			throw std::runtime_error("Failed to build DirectShowMadVRRenderer");

		m_renderer->Start();

		m_rendererStateText.SetWindowText(TEXT("Starting"));
	}
	catch (std::runtime_error e)
	{
		m_rendererState = RendererState::RENDERSTATE_FAILED;
		m_rendererStateText.SetWindowText(TEXT("Failed"));

		size_t size = strlen(e.what()) + 1;
		wchar_t* wtext = new wchar_t[size];
		size_t outSize;
		mbstowcs_s(&outSize, wtext, size, e.what(), size-1);
		m_rendererBox.SetWindowText(wtext);

		delete wtext;
	}
	catch (...)
	{
		m_rendererState = RendererState::RENDERSTATE_FAILED;
		m_rendererStateText.SetWindowText(TEXT("Failed"));
	}
}


void CVideoProcessorDlg::StopRender()
{
	{
		std::lock_guard<std::mutex> lock(m_rendererMutex);

		assert(m_renderer);
		assert(m_rendererState == RendererState::RENDERSTATE_RENDERING);
		assert(m_deliverFramesToRenderer);

		m_deliverFramesToRenderer = false;

		// Update internal state before call to StartCapture as that might be synchronous
		// TODO: Call should always be posted and never sync
		m_rendererState = RendererState::RENDERSTATE_STOPPING;

		m_renderer->Stop();
	}

	m_rendererStateText.SetWindowText(TEXT("Stopping"));
}


void CVideoProcessorDlg::RemoveRender()
{
	{
		std::lock_guard<std::mutex> lock(m_rendererMutex);
		RemoveRenderLocked();
	}
}


void CVideoProcessorDlg::RemoveRenderLocked()
{
	// WARNING: You need to own the m_rendererMutex at this point

	assert(m_renderer);
	assert(m_rendererState == RendererState::RENDERSTATE_STOPPED);
	assert(!m_deliverFramesToRenderer);

	delete m_renderer;
	m_renderer = nullptr;

	m_rendererState = RendererState::RENDERSTATE_UNKNOWN;
}


void CVideoProcessorDlg::UpdateState()
{
	// Want to change cards
	if (m_desiredCaptureDevice != m_captureDevice)
	{
		// TODO: m_rendererMutex??

		m_captureInputCombo.EnableWindow(FALSE);

		// Have a render and it's rendering, stop it
		if (m_renderer &&
			m_rendererState == RendererState::RENDERSTATE_RENDERING)
			StopRender();

		// Have a capture and it's capturing, stop it
		if (m_captureDevice &&
			m_captureDeviceState == CaptureDeviceState::CAPTUREDEVICESTATE_CAPTURING)
			StopCapture();

		// Waiting for render to go away
		if (m_renderer)
			return;

		// If capture device is stopped we're happy to remove it
		if (m_captureDevice && m_captureDeviceState == CaptureDeviceState::CAPTUREDEVICESTATE_READY)
			RemoveCapture();

		// We're waiting for the capture device to go away
		if (m_captureDevice)
			return;

		// From this point on we should be clean, set up new card if so desired
		assert(!m_renderer);
		assert(!m_captureDevice);

		if (m_desiredCaptureDevice)
		{
			assert(m_captureDeviceState == CaptureDeviceState::CAPTUREDEVICESTATE_UNKNOWN);

			m_captureDevice = m_desiredCaptureDevice;
			m_captureDevice->SetCallbackHandler(this);

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
		assert(m_rendererState == RendererState::RENDERSTATE_UNKNOWN);

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
			StopRender();

		// Have a capture and it's capturing, stop it
		if (m_captureDevice &&
			m_captureDeviceState == CaptureDeviceState::CAPTUREDEVICESTATE_CAPTURING)
			StopCapture();

		// Waiting for render to go away and for the capture to be stopped
		if (m_renderer ||
			m_captureDeviceState != CaptureDeviceState::CAPTUREDEVICESTATE_READY)
			return;

		// From this point on we should be clean, set up new card
		assert(!m_renderer);

		m_captureDevice->SetCaptureInput(m_desiredCaptureInputId);
		m_currentCaptureInputId = m_desiredCaptureInputId;

		StartCapture();
		return;
	}

	// Have the right card and the right input
	assert(m_captureDevice);
	assert(m_desiredCaptureDevice == m_captureDevice);
	assert(m_desiredCaptureInputId == m_currentCaptureInputId);

	// No render yet, start one if the current state is not failed and we have a valid video state
	if (!m_renderer)
	{
		// If the renderer failed we don't auto-start it again but wait for something to happen
		if (m_rendererState == RendererState::RENDERSTATE_FAILED)
			return;

		if (m_captureDeviceVideoState &&
			m_captureDeviceVideoState->valid)
			StartRender();

		return;
	}

	assert(m_renderer);

	// If we have a renderer but the video state is invalid stop if rendering
	if (m_rendererState == RendererState::RENDERSTATE_RENDERING &&
		(!m_captureDeviceVideoState ||
		 !m_captureDeviceVideoState->valid))
	{
		StopRender();
		return;
	}

	// Somebody wants to restart rendering, allrighty then
	if (m_rendererState == RendererState::RENDERSTATE_RENDERING &&
		m_wantToRestartRenderer)
	{
		m_wantToRestartRenderer = false;

		StopRender();
		return;
	}

	// We have a renderer and a valid video state, relax and enjoy the show
	assert(m_captureDeviceVideoState);
	//assert(m_captureDeviceVideoState->valid);
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

	// Captured video group
	DDX_Control(pDX, IDC_VIDEO_VALID_STATIC, m_videoValidText);
	DDX_Control(pDX, IDC_VIDEO_DISPLAY_MODE_STATIC, m_videoDisplayModeText);
	DDX_Control(pDX, IDC_VIDEO_PIXEL_FORMAT_STATIC, m_videoPixelFormatText);
	DDX_Control(pDX, IDC_VIDEO_EOTF_STATIC, m_videoEotfText);
	DDX_Control(pDX, IDC_VIDEO_COLORSPACE_STATIC, m_videoColorSpaceText);

	// ColorSpace group
	DDX_Control(pDX, IDC_CIE1931XY_GRAPH, m_colorspaceCie1931xy);

	//  HDR group
	DDX_Control(pDX, IDC_HDR_DP_RED_STATIC, m_hdrDpRed);
	DDX_Control(pDX, IDC_HDR_DP_GREEN_STATIC, m_hdrDpGreen);
	DDX_Control(pDX, IDC_HDR_DP_BLUE_STATIC, m_hdrDpBlue);
	DDX_Control(pDX, IDC_HDR_WHITE_POINT_STATIC, m_hdrWhitePoint);
	DDX_Control(pDX, IDC_HDR_DML_STATIC, m_hdrDml);
	DDX_Control(pDX, IDC_HDR_MAX_CLL_STATIC, m_hdrMaxCll);
	DDX_Control(pDX, IDC_HDR_MAX_FALL_STATIC, m_hdrMaxFall);

	// Renderer group
	DDX_Control(pDX, IDC_RENDERER_STATE_STATIC, m_rendererStateText);
	DDX_Control(pDX, IDC_RENDERER_PIXEL_VALUE_RANGE_COMBO, m_rendererPixelValueRangeCombo);
	DDX_Control(pDX, IDC_RENDERER_BOX, m_rendererBox);
}


//
// Generated message map functions
//


// Called when the dialog box is initialized
BOOL CVideoProcessorDlg::OnInitDialog()
{
	if (!CDialog::OnInitDialog())
		return FALSE;

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

	{
		int index = m_rendererPixelValueRangeCombo.AddString(TEXT("Let renderer decide"));
		m_rendererPixelValueRangeCombo.SetItemData(index, PixelValueRange::PIXELVALUERANGE_UNKNOWN);
		m_rendererPixelValueRangeCombo.SetCurSel(0);  // Select this by default

		index = m_rendererPixelValueRangeCombo.AddString(TEXT("Force full (0-255)"));
		m_rendererPixelValueRangeCombo.SetItemData(index, PixelValueRange::PIXELVALUERANGE_0_255);

		index = m_rendererPixelValueRangeCombo.AddString(TEXT("Force limited (16-235)"));
		m_rendererPixelValueRangeCombo.SetItemData(index, PixelValueRange::PIXELVALUERANGE_16_235);
	}

	// Start discovery services
	m_blackMagicDeviceDiscoverer = new BlackMagicDeckLinkCaptureDeviceDiscoverer(*this);
	m_blackMagicDeviceDiscoverer->Start();

	m_accelerator = LoadAccelerators(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR1));
	if (!m_accelerator)
		throw std::runtime_error("Failed to load accelerator");

	return TRUE;
}


BOOL CVideoProcessorDlg::PreTranslateMessage(MSG* pMsg)
{
	if (m_accelerator)
	{
		if (::TranslateAccelerator(m_hWnd, m_accelerator, pMsg))
			return(TRUE);
	}

	return CDialog::PreTranslateMessage(pMsg);
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
		// TODO: Lock access?
		if (m_renderer)
			m_renderer->OnPaint();

		CDialog::OnPaint();
	}
}


void CVideoProcessorDlg::OnSize(UINT nType, int cx, int cy)
{
	if (m_renderer)
		m_renderer->OnSize();

	CDialog::OnSize(nType, cx, cy);
}


// The system calls this function to obtain the cursor to display while the user drags
// the minimized window.
HCURSOR CVideoProcessorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


// Required to ensure minimum size of dialog
void CVideoProcessorDlg::OnGetMinMaxInfo(MINMAXINFO* minMaxInfo)
{
	CDialog::OnGetMinMaxInfo(minMaxInfo);

	minMaxInfo->ptMinTrackSize.x = std::max(minMaxInfo->ptMinTrackSize.x, m_minDialogSize.cx);
	minMaxInfo->ptMinTrackSize.y = std::max(minMaxInfo->ptMinTrackSize.y, m_minDialogSize.cy);
}
