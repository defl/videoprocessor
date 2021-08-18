/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <pch.h>

#include <dvdmedia.h>

#include <guid.h>
#include <microsoft_directshow/live_source_filter/CLiveSource.h>
#include <microsoft_directshow/DIrectShowTranslations.h>

#include "DirectShowVideoRenderer.h"


DirectShowVideoRenderer::DirectShowVideoRenderer(
	IRendererCallback& callback,
	HWND videoHwnd,
	HWND eventHwnd,
	UINT eventMsg,
	ITimingClock* timingClock,
	DirectShowStartStopTimeMethod timestamp,
	bool useFrameQueue,
	size_t frameQueueMaxSize,
	VideoConversionOverride videoConversionOverride):
	m_callback(callback),
	m_videoHwnd(videoHwnd),
	m_eventHwnd(eventHwnd),
	m_eventMsg(eventMsg),
	m_timingClock(timingClock),
	m_timestamp(timestamp),
	m_useFrameQueue(useFrameQueue),
	m_frameQueueMaxSize(frameQueueMaxSize),
	m_videoConversionOverride(videoConversionOverride)
{
	if (!videoHwnd)
		throw std::runtime_error("Invalid videoHwnd");
	if (!eventHwnd)
		throw std::runtime_error("Invalid eventHwnd");
	if (!eventMsg)
		throw std::runtime_error("Invalid eventMsg");

	if (timingClock && timingClock->TimingClockTicksPerSecond() < 1000LL)
		throw std::runtime_error("TimingClock needs resolution of at least millisecond level");

	if (!useFrameQueue && timestamp == DirectShowStartStopTimeMethod::DS_SSTM_CLOCK_CLOCK)
		throw std::runtime_error("No queue cannot be used with clock-clock, pick another mode and restart");

	ZeroMemory(&m_pmt, sizeof(AM_MEDIA_TYPE));
}


DirectShowVideoRenderer::~DirectShowVideoRenderer()
{
	GraphTeardown();
}


bool DirectShowVideoRenderer::OnVideoState(VideoStateComPtr& videoState)
{
	if (!videoState)
		throw std::runtime_error("null video state is invalid");

	if (m_videoState)
	{
		// Unacceptable changes to this renderer, return false and get cleaned up
		if (videoState->valid == false ||
			videoState->colorspace != m_videoState->colorspace ||
			videoState->eotf != m_videoState->eotf ||
			*(videoState->displayMode) != *(m_videoState->displayMode) ||
			videoState->videoFrameEncoding != m_videoState->videoFrameEncoding)
		{
			return false;
		}
	}
	else
	{
		// No video state yet, initialize
		m_videoState = videoState;
	}

	// All good, continue
	return true;
}


void DirectShowVideoRenderer::OnVideoFrame(VideoFrame& videoFrame)
{
	// Called from some unknown thread, but with promise that Start() has completed

	assert(m_state == RendererState::RENDERSTATE_RENDERING);
	assert(m_videoState);
	assert(videoFrame.GetTimingTimestamp() > 0);

	// Get delay until now once in a while
	if (m_frameCounter % 20 == 0)
	{
		const timingclocktime_t frameTime = videoFrame.GetTimingTimestamp();
		const timingclocktime_t clockTime = m_timingClock->TimingClockNow();

		m_frameLatencyEntry = TimingClockDiffMs(frameTime, clockTime, m_timingClock->TimingClockTicksPerSecond());
	}

	if (FAILED(m_liveSource->OnVideoFrame(videoFrame)))
	{
		DbgLog((LOG_TRACE, 1, TEXT("DirectShowVideoRenderer::OnVideoFrame(): Failed to deliver frame #%I64u"), m_frameCounter));
	}

	++m_frameCounter;
}


HRESULT DirectShowVideoRenderer::OnWindowsEvent(LONG_PTR, LONG_PTR)
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


void DirectShowVideoRenderer::Build()
{
	GraphBuild();
}


void DirectShowVideoRenderer::Start()
{
	GraphRun();
}


void DirectShowVideoRenderer::Stop()
{
	GraphStop();
}


void DirectShowVideoRenderer::Reset()
{
	// Stop directshow graph
	if (FAILED(m_pControl->Stop()))
		throw std::runtime_error("Failed to Stop() graph");

	m_liveSource->Reset();

	m_frameCounter = 0;

	// Run directshow graph again
	if (FAILED(m_pControl->Run()))
		throw std::runtime_error("Failed to Run() graph");
}


void DirectShowVideoRenderer::OnSize()
{
	if (!m_videoWindow)
		return;

	// Get window size
	RECT rectWindow;
	if (!GetWindowRect(m_videoHwnd, &rectWindow))
		throw std::runtime_error("Failed to get window rectangle");

	m_renderBoxWidth = rectWindow.right - rectWindow.left;
	m_renderBoxHeight = rectWindow.bottom - rectWindow.top;

	// TODO: Saw this blow up when resizing when the renderer is changing
	if (FAILED(m_videoWindow->SetWindowPosition(0, 0, m_renderBoxWidth, m_renderBoxHeight)))
		throw std::runtime_error("Failed to SetWindowPosition");
}


void DirectShowVideoRenderer::SetFrameQueueMaxSize(size_t frameMaxQueueSize)
{
	m_liveSource->SetFrameQueueMaxSize(frameMaxQueueSize);
}


size_t DirectShowVideoRenderer::GetFrameQueueSize()
{
	if (m_state != RendererState::RENDERSTATE_RENDERING)
		throw std::runtime_error("Invalid state, can only be called while rendering");

	return m_liveSource->GetFrameQueueSize();
}


double DirectShowVideoRenderer::EntryLatencyMs() const
{
	if (m_state != RendererState::RENDERSTATE_RENDERING)
		throw std::runtime_error("Invalid state, can only be called while rendering");

	return m_frameLatencyEntry;
}


double DirectShowVideoRenderer::ExitLatencyMs() const
{
	if (m_state != RendererState::RENDERSTATE_RENDERING)
		throw std::runtime_error("Invalid state, can only be called while rendering");

	return m_liveSource->ExitLatencyMs();
}


uint64_t DirectShowVideoRenderer::DroppedFrameCount() const
{
	if (m_state != RendererState::RENDERSTATE_RENDERING)
		throw std::runtime_error("Invalid state, can only be called while rendering");

	return m_liveSource->DroppedFrameCount();
}


void DirectShowVideoRenderer::OnGraphEvent(long evCode, LONG_PTR param1, LONG_PTR param2)
{
	// ! Do not tear down graph here
	// https://docs.microsoft.com/en-us/windows/win32/directshow/responding-to-events

	switch (evCode)
	{
	case EC_USERABORT:
	case EC_ERRORABORT:
	case EC_COMPLETE:
		if (m_state == RendererState::RENDERSTATE_RENDERING)
		{
			GraphStop();
		}
		break;
	}
}


void DirectShowVideoRenderer::SetState(RendererState state)
{
	DbgLog((LOG_TRACE, 1, TEXT("DirectShowVideoRenderer::SetState(): %s"), ToString(state)));

	assert(state != RendererState::RENDERSTATE_UNKNOWN);
	assert(m_state != state);

	m_state = state;
	m_callback.OnRendererState(state);
}


void DirectShowVideoRenderer::GraphBuild()
{
	DbgLog((LOG_TRACE, 1, TEXT("DirectShowVideoRenderer::GraphBuild(): Begin")));

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
	//

	FilterGraphBuild();

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
	// Build conversion dependent stuff and media type
	//

	MediaTypeGenerate();

	//
	// Live source filter
	//

	LiveSourceBuildAndConnect();

	//
	// Renderer
	//

	RendererBuild();

	if (!m_pRenderer)
		throw std::runtime_error("Created renderer instance wes nullptr");

	RendererConnect();

	//
	// Window setup
	//

	WindowSetup();

	//
	// Set up event notification.
	//

	if (FAILED(m_pEvent->SetNotifyWindow((OAHWND)m_eventHwnd, m_eventMsg, NULL)))
		throw std::runtime_error("Failed to setup event notification");

	SetState(RendererState::RENDERSTATE_READY);

	DbgLog((LOG_TRACE, 1, TEXT("DirectShowVideoRenderer::GraphBuild(): End")));
}


void DirectShowVideoRenderer::GraphTeardown()
{
	// Details of how to clean up here https://docs.microsoft.com/en-us/windows/win32/directshow/using-windowed-mode

	DbgLog((LOG_TRACE, 1, TEXT("DirectShowVideoRenderer::GraphTeardown(): Begin")));

	//
	// Stop sending event notifications
	//
	// https://docs.microsoft.com/en-us/windows/win32/directshow/responding-to-events for notes on cleanup
	if (m_pEvent)
	{
		m_pEvent->SetNotifyWindow((OAHWND)nullptr, NULL, NULL);
		m_pEvent->Release();
		m_pEvent = nullptr;
	}

	//
	// Teardown window
	//

	WindowTeardown();

	//
	// Disonnect
	//

	LiveSourceDisconnect();

	//
	// Free
	//

	FilterGraphDestroy();

	if (m_referenceClock)
	{
		m_referenceClock->Release();
		m_referenceClock = nullptr;
	}

	LiveSourceDestroy();

	RendererDestroy();

	if (m_videoFramFormatter)
	{
		delete m_videoFramFormatter;
		m_videoFramFormatter = nullptr;
	}

	if (m_pmt.pbFormat)
	{
		CoTaskMemFree(m_pmt.pbFormat);
		m_pmt.pbFormat = nullptr;
	}

	DbgLog((LOG_TRACE, 1, TEXT("DirectShowVideoRenderer::GraphTeardown(): End")));
}


void DirectShowVideoRenderer::GraphRun()
{
	DbgLog((LOG_TRACE, 1, TEXT("DirectShowVideoRenderer::GraphRun()")));

	assert(m_pGraph);
	assert(m_pControl);

	if (FAILED(m_pControl->Run()))
		throw std::runtime_error("Failed to Run() graph");

	SetState(RendererState::RENDERSTATE_RENDERING);
}


void DirectShowVideoRenderer::FilterGraphBuild()
{
	//
	// Directshow graph, note that we're not using a capture graph but DIY one
	// - https://docs.microsoft.com/en-us/windows/win32/directshow/about-the-capture-graph-builder
	// - https://www.codeproject.com/Articles/158053/DirectShow-Filters-Development-Part-2-Live-Source
	//

	if (FAILED(CoCreateInstance(
		CLSID_FilterGraph,
		nullptr,
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
}


void DirectShowVideoRenderer::FilterGraphDestroy()
{
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

	if (m_pGraph)
	{
		m_pGraph->Release();
		m_pGraph = nullptr;
	}
}


void DirectShowVideoRenderer::GraphStop()
{
	DbgLog((LOG_TRACE, 1, TEXT("DirectShowVideoRenderer::GraphStop()")));

	assert(m_pGraph);
	assert(m_pControl);

	// This is not sent to the outside world but it's used internally to guarantee that we're not
	// mis-handling events coming out of the DirectShow framework
	m_state = RendererState::RENDERSTATE_STOPPING;

	// Stop directshow graph
	if (FAILED(m_pControl->Stop()))
		throw std::runtime_error("Failed to Stop() graph");

	m_liveSource->Reset();

	// Check if filter really stopped
	OAFilterState filterState = -1;  // Known invalid state
	if (FAILED(m_pControl->GetState(500, &filterState)))
		throw std::runtime_error("Failed to get filter state");

	if((FILTER_STATE)filterState != FILTER_STATE::State_Stopped)
		throw std::runtime_error("Filter graph was not stopped");

	assert(m_liveSource->GetFrameQueueSize() == 0);

	SetState(RendererState::RENDERSTATE_STOPPED);
}


void DirectShowVideoRenderer::WindowSetup()
{
	// https://docs.microsoft.com/en-us/windows/win32/directshow/using-windowed-mode

	assert(m_videoHwnd);
	assert(m_renderBoxWidth > 0);
	assert(m_renderBoxHeight > 0);

	if (FAILED(m_videoWindow->put_Owner((OAHWND)m_videoHwnd)))
		throw std::runtime_error("Failed to set owner of video window");

	if (FAILED(m_videoWindow->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS)))
		throw std::runtime_error("Failed to set window style in video window");

	if (FAILED(m_videoWindow->SetWindowPosition(0, 0, m_renderBoxWidth, m_renderBoxHeight)))
		throw std::runtime_error("Failed to SetWindowPosition in video window");

	if (FAILED(m_videoWindow->HideCursor(OATRUE)))
		throw std::runtime_error("Failed to HideCursor in video window");
}


void DirectShowVideoRenderer::WindowTeardown()
{
	// These can fail if we are terminating and there is no visible window anymore, no problem
	if (m_videoWindow)
	{
		m_videoWindow->put_Visible(OAFALSE);
		m_videoWindow->put_Owner(NULL);
		m_videoWindow->HideCursor(OAFALSE);
	}
}


void DirectShowVideoRenderer::LiveSourceBuildAndConnect()
{
	assert(!m_liveSource);

	m_liveSource = dynamic_cast<CLiveSource*>(CLiveSource::CreateInstance(nullptr, nullptr));
	if (!m_liveSource)
		throw std::runtime_error("Failed to build a CLiveSource");

	m_liveSource->AddRef();

	const timestamp_t frameDuration100ns =
		(timestamp_t)round((1.0 / m_videoState->displayMode->RefreshRateHz()) * UNITS);

	m_liveSource->Initialize(
		m_videoFramFormatter,
		m_pmt,
		frameDuration100ns,
		m_timingClock,
		m_timestamp,
		m_useFrameQueue,
		m_frameQueueMaxSize);

	if (m_pGraph->AddFilter(m_liveSource, L"LiveSource") != S_OK)
	{
		m_liveSource->Release();
		throw std::runtime_error("Failed to add LiveSource to the graph");
	}
}


void DirectShowVideoRenderer::LiveSourceDisconnect()
{
	if (m_liveSource)
	{
		IEnumPins* pEnum = nullptr;
		IPin* pLiveSourceOutputPin = nullptr;

		if (FAILED(m_liveSource->EnumPins(&pEnum)))
			throw std::runtime_error("Failed to get livesource pin enumerator");

		if (pEnum->Next(1, &pLiveSourceOutputPin, nullptr) != S_OK)
			throw std::runtime_error("Failed to run next on livesource pin");

		pEnum->Release();
		pEnum = nullptr;

		if (FAILED(m_pGraph->Disconnect(pLiveSourceOutputPin)))
			throw std::runtime_error("Failed to disconnect pins");

		pLiveSourceOutputPin->Release();
	}
}


void DirectShowVideoRenderer::LiveSourceDestroy()
{
	if (m_liveSource)
	{
		m_liveSource->Destroy();
		m_liveSource->Release();
		m_liveSource = nullptr;
	}
}


void DirectShowVideoRenderer::RendererDestroy()
{
	if (m_pRenderer)
	{
		m_pRenderer->Release();
		m_pRenderer = nullptr;
	}
}
