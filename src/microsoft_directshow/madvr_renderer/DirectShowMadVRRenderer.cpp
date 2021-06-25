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
#include <guiddef.h>

#include <guid.h>
#include <microsoft_directshow/live_source_filter/CLiveSource.h>
#include <microsoft_directshow/live_source_filter/CLiveSourceVideoOutputPin.h>
#include <microsoft_directshow/DIrectShowTranslations.h>
#include <CNoopVideoFrameFormatter.h>
#include <ffmpeg/CFFMpegDecoderVideoFrameFormatter.h>

#include "DirectShowMadVRRenderer.h"


DirectShowMadVRRenderer::DirectShowMadVRRenderer(
	IRendererCallback& callback,
	HWND videoHwnd,
	HWND eventHwnd,
	UINT eventMsg,
	ITimingClock* timingClock,
	VideoStateComPtr& videoState,
	RendererTimestamp timestamp,
	size_t frameQueueMaxSize,
	int frameClockOffsetMs,
	DXVA_NominalRange forceNominalRange,
	DXVA_VideoTransferFunction forceVideoTransferFunction,
	DXVA_VideoTransferMatrix forceVideoTransferMatrix,
	DXVA_VideoPrimaries forceVideoPrimaries):
	m_callback(callback),
	m_videoHwnd(videoHwnd),
	m_eventHwnd(eventHwnd),
	m_eventMsg(eventMsg),
	m_timingClock(timingClock),
	m_videoState(videoState),
	m_timestamp(timestamp),
	m_frameClockOffsetMs(frameClockOffsetMs),
	m_frameQueueMaxSize(frameQueueMaxSize),
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

	if (timingClock && timingClock->GetTimingClockTicksPerSecond() < 1000LL)
		throw std::runtime_error("TimingClock needs resolution of at least millisecond level");
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
			if (FAILED(m_liveSource->OnHDRData(videoState->hdrData)))
				throw std::runtime_error("Failed to set HDR data");

			// Update the HDR in the videostate
			m_videoState->hdrData = videoState->hdrData;
		}
	}

	// All good, continue
	return true;
}


void DirectShowMadVRRenderer::OnVideoFrame(VideoFrame& videoFrame)
{
	// Called from some unknown thread, but with promize that Start() has completed

	assert(m_state == RendererState::RENDERSTATE_RENDERING);
	assert(m_videoState);

#ifdef _DEBUG

	if (m_timingClock)
	{
		const timingclocktime_t frameTime = videoFrame.GetTimingTimestamp();
		const timingclocktime_t clockTime = m_timingClock->GetTimingClockTime();

		// Time must go forward
		assert(clockTime >= frameTime);

		// Get frame-gap
		assert(m_previousFrameTime < frameTime);
		const timingclocktime_t previousFrameDiff = frameTime - m_previousFrameTime;
		double previousFrameDiffMs = previousFrameDiff / (double)m_timingClock->GetTimingClockTicksPerSecond() * 1000.0;
		m_previousFrameTime = frameTime;

		// Get latency from start to here
		timingclocktime_t captureNowTimeDiff = clockTime - frameTime;
		double timeDeltaMs = captureNowTimeDiff / (double)m_timingClock->GetTimingClockTicksPerSecond() * 1000.0;

		// Clock should be within 20s or so, captures both wrong clocks but also massive buffering
		assert(captureNowTimeDiff < 20 * m_timingClock->GetTimingClockTicksPerSecond());

		// Display once in a while
		if (m_frameCounter % 100 == 0)
		{
			DbgLog((LOG_TRACE, 1, TEXT(
				"DirectShowMadVRRenderer::OnVideoFrame(#%I64u): Capture->now: %.03f ms, Prev-current: %.03f ms"),
				m_frameCounter,
				timeDeltaMs,
				previousFrameDiffMs));
		}
	}
#endif  // _DEBUG

	if (FAILED(m_liveSource->OnVideoFrame(videoFrame)))
	{
		DbgLog((LOG_TRACE, 1, TEXT("DirectShowMadVRRenderer::OnVideoFrame(): Failed to deliver frame #%I64u"), m_frameCounter));
	}

	++m_frameCounter;
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


void DirectShowMadVRRenderer::Build()
{
	GraphBuild();
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


int DirectShowMadVRRenderer::GetFrameQueueSize()
{
	if (m_state != RendererState::RENDERSTATE_RENDERING)
		throw std::runtime_error("Invalid state, can only be called while rendering");

	return m_liveSource->GetFrameQueueSize();
}


double DirectShowMadVRRenderer::GetFrameVideoLeadMs()
{
	if (m_state != RendererState::RENDERSTATE_RENDERING)
		throw std::runtime_error("Invalid state, can only be called while rendering");

	return m_liveSource->GetFrameVideoLeadMs();
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
	case 64023:  // TODO: What is this?
		break;

	case EC_USERABORT:
	case EC_ERRORABORT:
	case EC_COMPLETE:
		if (m_state == RendererState::RENDERSTATE_RENDERING)
		{
			GraphStop();
		}
		break;

	// Catch for unknowns in debug
	default:
		assert(false);
	}
}


void DirectShowMadVRRenderer::SetState(RendererState state)
{
	DbgLog((LOG_TRACE, 1, TEXT("DirectShowMadVRRenderer::SetState(): %s"), ToString(state)));

	assert(state != RendererState::RENDERSTATE_UNKNOWN);
	if (m_state != state)
	{
		m_state = state;
		m_callback.OnRendererState(state);
	}
	else
	{
		// This is an interesting breakpoint moment
		int a = 1;
	}
}


void DirectShowMadVRRenderer::GraphBuild()
{
	DbgLog((LOG_TRACE, 1, TEXT("DirectShowMadVRRenderer::GraphBuild(): Begin")));

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
	// Directshow graph, note that we're not using a capture graph but DIY one
	// - https://docs.microsoft.com/en-us/windows/win32/directshow/about-the-capture-graph-builder
	// - https://www.codeproject.com/Articles/158053/DirectShow-Filters-Development-Part-2-Live-Source
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

	if (FAILED(m_pGraph->QueryInterface(IID_IMediaFilter, (void**)&m_mediaFilter)))
		throw std::runtime_error("Failed to get IID_IMediaFilter interface");

	if (FAILED(m_pGraph->QueryInterface(IID_IAMGraphStreams, (void**)&m_amGraphStreams)))
		throw std::runtime_error("Failed to get IID_IAMGraphStreams interface");

	//
	// Clock
	//

	if (m_timingClock)
	{
		m_referenceClock = new DirectShowTimingClock(*m_timingClock);
		m_referenceClock->AddRef();
		if (FAILED(m_mediaFilter->SetSyncSource(m_referenceClock)))
			throw std::runtime_error("Failed to set sync source to our reference clock");

		if (FAILED(m_amGraphStreams->SyncUsingStreamOffset(TRUE)))
			throw std::runtime_error("Failed to call SyncUsingStreamOffset");
	}

	//
	// Build conversion dependent stuff
	//

	GUID mediaSubType;
	int bitCount;

	switch (m_videoState->pixelFormat)
	{

	// r210 to RGB48
	case PixelFormat::R210:

		assert(m_videoState->invertedVertical);

		mediaSubType = MEDIASUBTYPE_RGB48LE;
		bitCount = 48;

		m_videoFramFormatter = new CFFMpegDecoderVideoFrameFormatter(
			AV_CODEC_ID_R210,
			AV_PIX_FMT_RGB48LE);
		break;

	// RGB 12-bit to RGB48
	case PixelFormat::R12B:

		assert(m_videoState->invertedVertical);

		mediaSubType = MEDIASUBTYPE_RGB48LE;
		bitCount = 48;

		m_videoFramFormatter = new CFFMpegDecoderVideoFrameFormatter(
			AV_CODEC_ID_R12B,
			AV_PIX_FMT_RGB48LE);
		break;

	// No conversion needed
	default:
		mediaSubType = TranslateToMediaSubType(m_videoState->pixelFormat);
		bitCount = PixelFormatBitsPerPixel(m_videoState->pixelFormat);;

		m_videoFramFormatter = new CNoopVideoFrameFormatter();
	}

	m_videoFramFormatter->OnVideoState(m_videoState);

	//
	// Live source filter
	//
	m_liveSource = dynamic_cast<CLiveSource*>(CLiveSource::CreateInstance(nullptr, nullptr));
	if (m_liveSource == nullptr)
		throw std::runtime_error("Failed to build a CLiveSource");

	m_liveSource->AddRef();

	m_liveSource->Initialize(
		m_videoFramFormatter,
		mediaSubType,
		m_videoState->displayMode->FrameDuration(),
		m_timingClock,
		m_timestamp,
		m_frameQueueMaxSize,
		m_frameClockOffsetMs);

	if (m_videoState->hdrData)
		m_liveSource->OnHDRData(m_videoState->hdrData);

	if (m_pGraph->AddFilter(m_liveSource, L"LiveSource") != S_OK)
	{
		m_liveSource->Release();
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

	// TODO: Setting minimal queue sizes will always fail as they fall outside of the valid range for madVR
	//       Should talk to madshi to get lower values accepted.
	//
	//CComQIPtr<IMadVRSettings> pMVRSettings = m_pMVR;
	//if(!pMVRSettings)
	//	throw std::runtime_error("Failed to get MadVR IMadVRSettings");
	//
	//pMVRSettings->SettingsSetInteger(L"cpuQueueSize", 1);
	//pMVRSettings->SettingsSetInteger(L"gpuQueueSize", 1);
	//pMVRSettings->SettingsSetInteger(L"preRenderFrames", 3);

	//
	// Build media type for connect
	//

	AM_MEDIA_TYPE pmt;
	ZeroMemory(&pmt, sizeof(AM_MEDIA_TYPE));

	pmt.formattype = FORMAT_VIDEOINFO2;
	pmt.cbFormat = sizeof(VIDEOINFOHEADER2);
	pmt.majortype = MEDIATYPE_Video;
	pmt.subtype = mediaSubType;
	pmt.bFixedSizeSamples = TRUE;
	pmt.bTemporalCompression = FALSE;

	pmt.pbFormat = (BYTE*)CoTaskMemAlloc(sizeof(VIDEOINFOHEADER2));
	if (!pmt.pbFormat)
		throw std::runtime_error("Out of mem");

	VIDEOINFOHEADER2* pvi2 = (VIDEOINFOHEADER2*)pmt.pbFormat;
	ZeroMemory(pvi2, sizeof(VIDEOINFOHEADER2));

	// Populate bitmap info header
	// https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader

	pvi2->bmiHeader.biSizeImage = m_videoFramFormatter->GetOutFrameSize();
	pvi2->bmiHeader.biBitCount = bitCount;
	pvi2->bmiHeader.biCompression = pmt.subtype.Data1;
	pvi2->bmiHeader.biWidth = m_videoState->displayMode->FrameWidth();
	pvi2->bmiHeader.biHeight = ((long)m_videoState->displayMode->FrameHeight()) *
		                       (m_videoState->invertedVertical ? -1 : 1);
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

	if (FAILED(m_liveSource->EnumPins(&pEnum)))
		throw std::runtime_error("Failed to get livesource pin enumerator");

	if (pEnum->Next(1, &pLiveSourceOutputPin, NULL) != S_OK)
	{
		pEnum->Release();

		throw std::runtime_error("Failed to run next on livesource pin");
	}

	pEnum->Release();
	pEnum = nullptr;

	if (FAILED(m_pMVR->EnumPins(&pEnum)))
	{
		pLiveSourceOutputPin->Release();
		pMadVRInputPin->Release();

		throw std::runtime_error("Failed to get livesource pin enumerator");
	}

	if (pEnum->Next(1, &pMadVRInputPin, NULL) != S_OK)
	{
		pLiveSourceOutputPin->Release();
		pMadVRInputPin->Release();
		pEnum->Release();

		throw std::runtime_error("Failed to get livesource pin enumerator");
	}

	pEnum->Release();
	pEnum = nullptr;

	// TODO: This increases the ref on the live filter by 3, of which 2 I cannot explain
	if (FAILED(m_pGraph->ConnectDirect(pLiveSourceOutputPin, pMadVRInputPin, &pmt)))
	{
		pLiveSourceOutputPin->Release();
		pMadVRInputPin->Release();

		throw std::runtime_error("Failed to connect pins");
	}

	pLiveSourceOutputPin->Release();
	pMadVRInputPin->Release();

	//
	// Set up window
	// https://docs.microsoft.com/en-us/windows/win32/directshow/using-windowed-mode
	//

	if (FAILED(m_videoWindow->put_Owner((OAHWND)m_videoHwnd)))
		throw std::runtime_error("Failed to set owner of video window");

	if (FAILED(m_videoWindow->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS)))
		throw std::runtime_error("Failed to set window style in video window");

	if (FAILED(m_videoWindow->SetWindowPosition(0, 0, m_renderBoxWidth, m_renderBoxHeight)))
		throw std::runtime_error("Failed to SetWindowPosition in video window");

	if (FAILED(m_videoWindow->HideCursor(OATRUE)))
		throw std::runtime_error("Failed to HideCursor in video window");

	//
	// Set up event notification.
	//

	if (FAILED(m_pEvent->SetNotifyWindow((OAHWND)m_eventHwnd, m_eventMsg, NULL)))
		throw std::runtime_error("Failed to setup event notification");

	SetState(RendererState::RENDERSTATE_READY);

	DbgLog((LOG_TRACE, 1, TEXT("DirectShowMadVRRenderer::GraphBuild(): End")));
}


void DirectShowMadVRRenderer::GraphTeardown()
{
	// Details of how to clean up here https://docs.microsoft.com/en-us/windows/win32/directshow/using-windowed-mode

	DbgLog((LOG_TRACE, 1, TEXT("DirectShowMadVRRenderer::GraphTeardown(): Begin")));

	//
	// Stop sending event notifications
	//
	// https://docs.microsoft.com/en-us/windows/win32/directshow/responding-to-events for notes on cleanup
	if (m_pEvent)
	{
		m_pEvent->SetNotifyWindow((OAHWND)NULL, NULL, NULL);
		m_pEvent->Release();
		m_pEvent = nullptr;
	}

	//
	// Disable output window
	// These can fail if we are terminating and there is no visible window anymore, no problem
	//
	if (m_videoWindow)
	{
		m_videoWindow->put_Visible(OAFALSE);
		m_videoWindow->put_Owner(NULL);
		m_videoWindow->HideCursor(OAFALSE);
	}

	//
	// Disonnect
	//

	if (m_liveSource)
	{
		IEnumPins* pEnum = nullptr;
		IPin* pLiveSourceOutputPin = nullptr;

		if (FAILED(m_liveSource->EnumPins(&pEnum)))
			throw std::runtime_error("Failed to get livesource pin enumerator");

		if (pEnum->Next(1, &pLiveSourceOutputPin, NULL) != S_OK)
			throw std::runtime_error("Failed to run next on livesource pin");

		pEnum->Release();
		pEnum = nullptr;

		if (FAILED(m_pGraph->Disconnect(pLiveSourceOutputPin)))
			throw std::runtime_error("Failed to disconnect pins");

		pLiveSourceOutputPin->Release();
	}

	//
	// Free
	//

	if (m_pGraph)
	{
		m_pGraph->Release();
		m_pGraph = nullptr;
	}
	if (m_pControl)
	{
		m_pControl->Release();
		m_pControl = nullptr;
	}
	if (m_pEvent)
	{
		m_pEvent->Release();
		m_pEvent = nullptr;
	}
	if (m_videoWindow)
	{
		m_videoWindow->Release();
		m_videoWindow = nullptr;
	}
	if (m_pGraph2)
	{
		m_pGraph2->Release();
		m_pGraph2 = nullptr;
	}
	if (m_mediaFilter)
	{
		m_mediaFilter->Release();
		m_mediaFilter = nullptr;
	}
	if (m_amGraphStreams)
	{
		m_amGraphStreams->Release();
		m_amGraphStreams = nullptr;
	}

	if (m_referenceClock)
	{
		m_referenceClock->Release();
		m_referenceClock = nullptr;
	}

	if (m_liveSource)
	{
		// TODO: This has 2 refs too many
		m_liveSource->Release();
		m_liveSource = nullptr;
	}

	if (m_pMVR)
	{
		m_pMVR->Release();
		m_pMVR = nullptr;
	}

	if (m_videoFramFormatter)
	{
		delete m_videoFramFormatter;
		m_videoFramFormatter = nullptr;
	}

	DbgLog((LOG_TRACE, 1, TEXT("DirectShowMadVRRenderer::GraphTeardown(): End")));
}


void DirectShowMadVRRenderer::GraphRun()
{
	DbgLog((LOG_TRACE, 1, TEXT("DirectShowMadVRRenderer::GraphRun()")));

	assert(m_pGraph);
	assert(m_pControl);

	if (FAILED(m_pControl->Run()))
		throw std::runtime_error("Failed to Run() graph");

	SetState(RendererState::RENDERSTATE_RENDERING);
}


void DirectShowMadVRRenderer::GraphStop()
{
	DbgLog((LOG_TRACE, 1, TEXT("DirectShowMadVRRenderer::GraphStop()")));

	assert(m_pGraph);
	assert(m_pControl);

	// This is not sent to the outside world but it's used internally to guarantee that we're not
	// mis-handling events coming out of the DirectShow framework
	m_state = RendererState::RENDERSTATE_STOPPING;

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

	// Check if filter really stoppedd
	OAFilterState filterState = -1;  // Known invalid state
	if (FAILED(m_pControl->GetState(500, &filterState)))
		throw std::runtime_error("Failed to get filter state");

	if((FILTER_STATE)filterState != FILTER_STATE::State_Stopped)
		throw std::runtime_error("Filter graph was not stopped");

	assert(m_liveSource->GetFrameQueueSize() == 0);

	SetState(RendererState::RENDERSTATE_STOPPED);
}
