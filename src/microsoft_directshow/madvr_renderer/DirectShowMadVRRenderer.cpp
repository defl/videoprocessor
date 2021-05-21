/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include "stdafx.h"

#include <d3d9helper.h> // madVR requirement
#include <mvrInterfaces.h>
#include <dvdmedia.h>
#include <dxva.h>
#include <gl/gl.h>
#include <windef.h>

#include <guid.h>
#include <microsoft_directshow/live_source_filter/CLiveSource.h>
#include <microsoft_directshow/DIrectShowTranslations.h>

#include "DirectShowMadVRRenderer.h"


DirectShowMadVRRenderer::DirectShowMadVRRenderer(
	IRendererCallback& callback,
	HWND videoHwnd,
	HWND eventHwnd,
	UINT eventMsg,
	VideoStateComPtr& videoState,
	DXVA_NominalRange forceNominalRange,
	DXVA_VideoTransferFunction forceVideoTransferFunction,
	DXVA_VideoTransferMatrix forceVideoTransferMatrix,
	DXVA_VideoPrimaries forceVideoPrimaries):
	m_callback(callback),
	m_videoHwnd(videoHwnd),
	m_eventHwnd(eventHwnd),
	m_eventMsg(eventMsg),
	m_videoState(videoState),
	m_forceNominalRange(forceNominalRange),
	m_forceVideoTransferFunction(forceVideoTransferFunction),
	m_forceVideoTransferMatrix(forceVideoTransferMatrix),
	m_forceVideoPrimaries(forceVideoPrimaries)
{
	if (!videoHwnd)
		throw std::runtime_error("Invalid videoHwnd");
	if (!eventHwnd)
		throw std::runtime_error("Invalid eventHwnd");
	if (!eventMsg)
		throw std::runtime_error("Invalid eventMsg");
	if (!videoState)
		throw std::runtime_error("Invalid videoState object");

	GraphBuild();
}


DirectShowMadVRRenderer::~DirectShowMadVRRenderer()
{
	GraphTeardown();
}


bool DirectShowMadVRRenderer::OnVideoState(VideoStateComPtr& videoState)
{
	if (!videoState)
		throw std::runtime_error("null video state is invalid");

	// Unacceptable changes to this renderer, return false and get cleaned up
	if (videoState->valid == false ||
		videoState->colorspace != m_videoState->colorspace ||
		videoState->eotf != m_videoState->eotf ||
		*(videoState->displayMode) != *(m_videoState->displayMode) ||
		videoState->pixelFormat != m_videoState->pixelFormat)
	{
		return false;
	}

	if (videoState->hdrData)
	{
		if (!m_videoState->hdrData ||
			*(videoState->hdrData) != *(m_videoState->hdrData))
		{
			if (FAILED(m_LiveSource->OnHDRData(videoState->hdrData)))
				throw std::runtime_error("Failed to set HDR data");

			// TODO: This updating of the video state is not very nice
			m_videoState->hdrData = videoState->hdrData;
		}
	}

	// All good, continue
	return true;
}


void DirectShowMadVRRenderer::OnVideoFrame(VideoFrame& videoFrame)
{
	if (m_state != RendererState::RENDERSTATE_RENDERING)
		return;

	if (FAILED(m_LiveSource->OnVideoFrame(videoFrame)))
		throw std::runtime_error("Failed to add frame");
}


HRESULT DirectShowMadVRRenderer::OnWindowsEvent(LONG_PTR, LONG_PTR)
{
	// ! Do not tear down graph here

	if (!m_pEvent)
		throw std::runtime_error("No pevent");

	long evCode = 0;
	LONG_PTR param1 = 0, param2 = 0;

	HRESULT hr = S_OK;

	// Get the events from the queue.
	while (SUCCEEDED(m_pEvent->GetEvent(&evCode, &param1, &param2, 0)))
	{
		// Invoke the callback.
		OnGraphEvent(evCode, param1, param2);

		// Free the event data.
		hr = m_pEvent->FreeEventParams(evCode, param1, param2);
		if (FAILED(hr))
		{
			break;
		}
	}

	return hr;
}


void DirectShowMadVRRenderer::Start()
{
	GraphRun();
}



void DirectShowMadVRRenderer::Stop()
{
	GraphStop();
}


void DirectShowMadVRRenderer::OnPaint()
{
	if (m_pMVR)
	{
		CComQIPtr<IMadVRCommand> pMVRC = m_pMVR;
		pMVRC->SendCommand("redraw");
	}
}


void DirectShowMadVRRenderer::OnSize()
{
	// Get window size
	RECT rectWindow;
	if (!GetWindowRect(m_videoHwnd, &rectWindow))
		throw std::runtime_error("Failed to get window rectangle");
	m_renderBoxWidth = rectWindow.right - rectWindow.left;
	m_renderBoxHeight = rectWindow.bottom - rectWindow.top;

	if (FAILED(m_videoWindow->SetWindowPosition(0, 0, m_renderBoxWidth, m_renderBoxHeight)))
		throw std::runtime_error("Failed to SetWindowPosition");
}


void DirectShowMadVRRenderer::GoFullScreen(bool fullScreen)
{
	// Read current state
	LONG lMode;
	if (FAILED((m_videoWindow->get_FullScreenMode(&lMode))))
		throw std::runtime_error("Failed to get_FullScreenMode");

	// Go to fullscreen
	if (lMode == OAFALSE && fullScreen)
	{
		if(FAILED(m_videoWindow->put_FullScreenMode(OATRUE)))
			throw std::runtime_error("Failed to put_FullScreenMode true");
	}

	// Go to windowed
	else if (lMode == OATRUE && !fullScreen)
	{
		if (FAILED(m_videoWindow->put_FullScreenMode(OAFALSE)))
			throw std::runtime_error("Failed to put_FullScreenMode false");
	}
}


void DirectShowMadVRRenderer::OnGraphEvent(long evCode, LONG_PTR param1, LONG_PTR param2)
{
	// ! Do not tear down graph here
	// https://docs.microsoft.com/en-us/windows/win32/directshow/responding-to-events

	switch (evCode)
	{
	// Seen, unhandled
	case EC_VIDEO_SIZE_CHANGED:
	case EC_CLOCK_CHANGED:
	case EC_PAUSED:
		break;
	case 64023:  // TODO: WTF is this?
		break;

	// Aborted, happens when exiting from full-screen
	case EC_USERABORT:
	case EC_ERRORABORT:
		GraphStop();
		break;

	// Complete is called by renderer as a response to an end-of-stream which we trigger
	// when we want to stop rendering.
	case EC_COMPLETE:
		GraphStop();
		break;

	// Catch for unknowns
	default:
		assert(false);
	}
}


void DirectShowMadVRRenderer::SetState(RendererState state)
{
	assert(state != RendererState::RENDERSTATE_UNKNOWN);
	assert(m_state != state);

	m_state = state;
	m_callback.OnRendererState(state);
}


void DirectShowMadVRRenderer::GraphBuild()
{
	assert(m_videoState);

	//
	// Window setup
	//

	// Get window size
	RECT rectWindow;
	if (!GetWindowRect(m_videoHwnd, &rectWindow))
		throw std::runtime_error("Failed to get window rectangle");
	m_renderBoxWidth = rectWindow.right - rectWindow.left;
	m_renderBoxHeight = rectWindow.bottom - rectWindow.top;

	//
	// Directshow graph
	// Good tips: https://www.codeproject.com/Articles/158053/DirectShow-Filters-Development-Part-2-Live-Source
	//

	if (FAILED(CoCreateInstance(
		CLSID_FilterGraph,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder,
		(void**)&m_pGraph)))
		throw std::runtime_error("Failed to CoCreateInstance CLSID_FilterGraph");

	// Query for graph interfaces.
	if (FAILED(m_pGraph->QueryInterface(IID_IMediaControl, (void**)&m_pControl)))
		throw std::runtime_error("Failed to get IID_IMediaControl interface");

	if (FAILED(m_pGraph->QueryInterface(IID_IMediaEventEx, (void**)&m_pEvent)))
		throw std::runtime_error("Failed to get IID_IMediaEventEx interface");

	if (FAILED(m_pGraph->QueryInterface(IID_IVideoWindow, (void**)&m_videoWindow)))
		throw std::runtime_error("Failed to get IID_IVideoWindow interface");

	if (FAILED(m_pGraph->QueryInterface(IID_IFilterGraph2, (void**)&m_pGraph2)))
		throw std::runtime_error("Failed to get IID_IFilterGraph2 interface");

	//
	// Live source filter
	//
	m_LiveSource = dynamic_cast<CLiveSource*>(CLiveSource::CreateInstance(nullptr, nullptr));
	if (m_LiveSource == nullptr)
		throw std::runtime_error("Failed to build a CLiveSource");

	m_LiveSource->OnVideoState(m_videoState);

	m_LiveSource->AddRef();

	if (m_pGraph->AddFilter(m_LiveSource, L"LiveSource") != S_OK)
	{
		m_LiveSource->Release();
		throw std::runtime_error("Failed to add LiveSource to the graph");
	}

	//
	// madVR render filter
	//
	if (FAILED(CoCreateInstance(
		CLSID_madVR,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IBaseFilter,
		(void**)&m_pMVR)))
		throw std::runtime_error("Failed to create madVR instance");

	if (!m_pMVR)
		throw std::runtime_error("Created madVR instance wes nullptr");

	if (FAILED(m_pGraph->AddFilter(m_pMVR, L"madVR")))
		throw std::runtime_error("Failed to add madVR to the graph");

	//
	// Build media type for connect
	//

	AM_MEDIA_TYPE pmt;
	ZeroMemory(&pmt, sizeof(AM_MEDIA_TYPE));

	pmt.formattype = FORMAT_VIDEOINFO2;
	pmt.cbFormat = sizeof(VIDEOINFOHEADER2);
	pmt.majortype = MEDIATYPE_Video;
	pmt.subtype = TranslateToMediaSubType(m_videoState->pixelFormat);
	pmt.bFixedSizeSamples = TRUE;
	pmt.bTemporalCompression = FALSE;

	pmt.pbFormat = (BYTE*)CoTaskMemAlloc(sizeof(VIDEOINFOHEADER2));
	if (!pmt.pbFormat)
		throw std::runtime_error("Out of mem");

	VIDEOINFOHEADER2* pvi2 = (VIDEOINFOHEADER2*)pmt.pbFormat;
	ZeroMemory(pvi2, sizeof(VIDEOINFOHEADER2));

	// Figure out pixel format and send as directshow type

	pvi2->bmiHeader.biSizeImage = m_videoState->BytesPerFrame();
	pvi2->bmiHeader.biBitCount = PixelFormatBitsPerPixel(m_videoState->pixelFormat);
	pvi2->bmiHeader.biCompression = PixelFormatFourCC(m_videoState->pixelFormat);
	pvi2->bmiHeader.biWidth = m_videoState->displayMode->FrameWidth();
	pvi2->bmiHeader.biHeight = m_videoState->displayMode->FrameHeight();
	pvi2->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pvi2->bmiHeader.biPlanes = 1;
	pvi2->bmiHeader.biClrImportant = 0;
	pvi2->bmiHeader.biClrUsed = 0;

	pvi2->AvgTimePerFrame = (REFERENCE_TIME)(UNITS / m_videoState->displayMode->RefreshRateHz());

	//// dwControlFlags is a 32bit int. With AMCONTROL_COLORINFO_PRESENT the upper 24 bits are used by DXVA_ExtendedFormat.
	//// That struct is 32 bits so it's lower member (SampleFormat) is actually overbooked with the value of dwConotrolFlags
	//// so can't be used. LAV has defined some out-of-spec but compatile with madVR values for the more modern formats,
	//// which we use as well see
	//// https://github.com/Nevcairiel/LAVFilters/blob/ddef56ae155d436f4301346408f4fdba755197d6/decoder/LAVVideo/Media.cpp

	DXVA_ExtendedFormat* colorimetry = (DXVA_ExtendedFormat*)&(pvi2->dwControlFlags);

	colorimetry->VideoPrimaries =
		(m_forceVideoPrimaries != DXVA_VideoPrimaries::DXVA_VideoPrimaries_Unknown) ?
		m_forceVideoPrimaries :
		TranslateVideoPrimaries(m_videoState->colorspace);

	colorimetry->VideoTransferMatrix =
		(m_forceVideoTransferMatrix != DXVA_VideoTransferMatrix::DXVA_VideoTransferMatrix_Unknown) ?
		m_forceVideoTransferMatrix :
		TranslateVideoTransferMatrix(m_videoState->colorspace);

	colorimetry->VideoTransferFunction =
		(m_forceVideoTransferFunction != DXVA_VideoTransferFunction::DXVA_VideoTransFunc_Unknown) ?
		m_forceVideoTransferFunction :
		TranslateVideoTranferFunction(m_videoState->eotf, m_videoState->colorspace);

	colorimetry->NominalRange =
		(m_forceNominalRange != DXVA_NominalRange::DXVA_NominalRange_Unknown) ?
		m_forceNominalRange :
		DXVA_NominalRange::DXVA_NominalRange_Unknown;  // = Let madVR guess

	pvi2->dwControlFlags += AMCONTROL_USED;
	pvi2->dwControlFlags += AMCONTROL_COLORINFO_PRESENT;

	pmt.lSampleSize = DIBSIZE(pvi2->bmiHeader);

	CoTaskMemFree(pmt.pbFormat);

	//
	// Connect pins (both only have 1)
	//

	IEnumPins* pEnum = NULL;
	IPin* pLiveSourceOutputPin = NULL;
	IPin* pMadVRInputPin = NULL;

	if (FAILED(m_LiveSource->EnumPins(&pEnum)))
		throw std::runtime_error("Failed to get livesource pin enumerator");

	if (pEnum->Next(1, &pLiveSourceOutputPin, NULL) != S_OK)
		throw std::runtime_error("Failed to run next on livesource pin");

	pEnum->Release();

	if (FAILED(m_pMVR->EnumPins(&pEnum)))
		throw std::runtime_error("Failed to get livesource pin enumerator");

	if (pEnum->Next(1, &pMadVRInputPin, NULL) != S_OK)
		throw std::runtime_error("Failed to get livesource pin enumerator");

	pEnum->Release();

	if (FAILED(m_pGraph->ConnectDirect(pLiveSourceOutputPin, pMadVRInputPin, &pmt)))
		throw std::runtime_error("Failed to connect pins");

	pLiveSourceOutputPin->Release();
	pMadVRInputPin->Release();

	//
	// Set up window
	// https://docs.microsoft.com/en-us/windows/win32/directshow/using-windowed-mode
	//
	if (FAILED(m_videoWindow->put_Owner((OAHWND)m_videoHwnd)))
		throw std::runtime_error("Failed to set window owner");

	if (FAILED(m_videoWindow->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS)))
		throw std::runtime_error("Failed to set window style");

	if (FAILED(m_videoWindow->SetWindowPosition(0, 0, m_renderBoxWidth, m_renderBoxHeight)))
		throw std::runtime_error("Failed to SetWindowPosition");

	//
	// Set up event notification.
	//

	if (FAILED(m_pEvent->SetNotifyWindow((OAHWND)m_eventHwnd, m_eventMsg, NULL)))
		throw std::runtime_error("Failed to setup event notification");

	SetState(RendererState::RENDERSTATE_READY);
}

void DirectShowMadVRRenderer::GraphTeardown()
{
	// Details of how to clean up here https://docs.microsoft.com/en-us/windows/win32/directshow/using-windowed-mode

	//
	// Disable output window
	//
	CComPtr<IVideoWindow> m_pVideoWindow;
	if (FAILED(m_pGraph->QueryInterface(&m_pVideoWindow)))
		throw std::runtime_error("Failed to query video window");

	if (!m_pVideoWindow)
		throw std::runtime_error("Video window was null");

	// These can fail if we are terminating and there is no visible window anymore, no problem
	m_pVideoWindow->put_Visible(OAFALSE);
	m_pVideoWindow->put_Owner(NULL);

	// Stop sending event messages
	// https://docs.microsoft.com/en-us/windows/win32/directshow/responding-to-events for notes on cleanup
	if (m_pEvent)
	{
		m_pEvent->SetNotifyWindow((OAHWND)NULL, NULL, NULL);
		m_pEvent->Release();
		m_pEvent = NULL;
	}

	//
	// Disonnect
	//

	IEnumPins* pEnum = NULL;
	IPin* pLiveSourceOutputPin = NULL;

	if (FAILED(m_LiveSource->EnumPins(&pEnum)))
		throw std::runtime_error("Failed to get livesource pin enumerator");

	if (pEnum->Next(1, &pLiveSourceOutputPin, NULL) != S_OK)
		throw std::runtime_error("Failed to run next on livesource pin");

	pEnum->Release();

	if (FAILED(m_pGraph->Disconnect(pLiveSourceOutputPin)))
		throw std::runtime_error("Failed to disconnect pins");

	pLiveSourceOutputPin->Release();

	//
	// Free
	//

	if (m_pGraph)
		m_pGraph->Release();
	if (m_pControl)
		m_pControl->Release();
	if (m_pEvent)
		m_pEvent->Release();
	if (m_pGraph2)
		m_pGraph2->Release();
	if (m_videoWindow)
		m_videoWindow->Release();

	assert(m_LiveSource);
	m_LiveSource->Release();

	assert(m_pMVR);
	m_pMVR->Release();
}


void DirectShowMadVRRenderer::GraphRun()
{
	assert(m_pGraph);
	assert(m_pControl);

	if (FAILED(m_pControl->Run()))
		throw std::runtime_error("Failed to Run() graph");

	SetState(RendererState::RENDERSTATE_RENDERING);
}


void DirectShowMadVRRenderer::GraphStop()
{
	assert(m_pGraph);
	assert(m_pControl);

	// Stop directshow graph
	if (FAILED(m_pControl->Stop()))
		throw std::runtime_error("Failed to Stop() graph");

	// Check if madVR is stopped
	CComQIPtr<IMadVRInfo> pMVRI = m_pMVR;
	if (!pMVRI)
		throw std::runtime_error("Failed to get IMadVRInfo");

	int madVrPlaybastState = -1;  // Invalid state we hope
	if (FAILED(pMVRI->GetInt("playbackState", &madVrPlaybastState)))
		throw std::runtime_error("Failed to get filter state");

	if ((FILTER_STATE)madVrPlaybastState != FILTER_STATE::State_Stopped)
		throw std::runtime_error("Madvr was not stopped");

	// Wait-lock until state is ok
	// TODO: There still is some sort os async component to stopping but no callback handler. No idea how to check if the graph has really stopped
	OAFilterState filterState = -1; // Invalid state
	if (FAILED(m_pControl->GetState(500, &filterState)))
		throw std::runtime_error("Failed to get filter state");

	if((FILTER_STATE)filterState != FILTER_STATE::State_Stopped)
		throw std::runtime_error("Filter graph was not stopped");

	SetState(RendererState::RENDERSTATE_STOPPED);
}
