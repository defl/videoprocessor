/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include <dshow.h>

#include <IRenderer.h>
#include <PixelValueRange.h>
#include <VideoState.h>
#include <microsoft_directshow/live_source_filter/CLiveSource.h>


/**
 * This renderer renders using madVR.
 * For that to work it has to create a DirectShow graph.
 */
class DirectShowMadVRRenderer:
	public IRenderer
{
public:

	DirectShowMadVRRenderer(
		IRendererCallback& callback,
		HWND videoHwnd,
		HWND eventHwnd,
		UINT eventMsg,
		VideoStateComPtr& videoState,
		PixelValueRange forcePixelValueRange);

	virtual ~DirectShowMadVRRenderer();

	// IRenderer
	bool OnVideoState(VideoStateComPtr&) override;
	void OnVideoFrame(VideoFrame& videoFrame) override;
	HRESULT OnWindowsEvent(LONG_PTR param1, LONG_PTR param2) override;
	void Start() override;
	void Stop() override;
	void OnPaint() override;
	void OnSize() override;
	void GoFullScreen(bool fullScreen) override;

private:
	IRendererCallback& m_callback;
	HWND m_videoHwnd;
	HWND m_eventHwnd;
	UINT m_eventMsg;
	VideoStateComPtr m_videoState;
	PixelValueRange m_forcePixelValueRange;

	LONG m_renderBoxWidth = 0;
	LONG m_renderBoxHeight = 0;

	RendererState m_state = RendererState::RENDERSTATE_UNKNOWN;

	IGraphBuilder* m_pGraph = NULL;
	IMediaControl* m_pControl = NULL;
	IMediaEventEx* m_pEvent = NULL;
	IVideoWindow* m_videoWindow = NULL;
	IFilterGraph2* m_pGraph2 = NULL;

	CLiveSource* m_LiveSource = NULL;
	IBaseFilter* m_pMVR = NULL;

	// Handle Directshow graph events
	void OnGraphEvent(long evCode, LONG_PTR param1, LONG_PTR param2);

	// Helper for state setting and callbacks
	void SetState(RendererState state);

	// Graph management functions.
	void GraphBuild();
	void GraphTeardown();
	void GraphRun();
	void GraphStop();
};
