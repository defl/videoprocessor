/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include <VideoFrame.h>
#include <VideoState.h>


enum RendererState
{
	RENDERSTATE_READY,
	RENDERSTATE_RENDERING,
	RENDERSTATE_STOPPED,

	// States which will not be sent by the card but which can be used
	// by clients when they are expecting a callback for example
	RENDERSTATE_UNKNOWN,
	RENDERSTATE_STOPPING,
	RENDERSTATE_STARTING,
	RENDERSTATE_FAILED
};


const TCHAR* ToString(const RendererState rendererState);


enum RendererTimestamp
{
	// Use the given clock
	RENDERER_TIMESTAMP_CLOCK,

	// Theoretical timestamp based on frame
	RENDERER_TIMESTAMP_THEORETICAL,

	// Don't set timestamps (and use clock)
	RENDERER_TIMESTAMP_NONE
};


const TCHAR* ToString(const RendererTimestamp rendererTimestamp);


/**
 * Renderer callback
 */
struct IRendererCallback
{
	// Note that this will be synchronous with calls to the renderer. No external thread
	// will cause calls. Most likely this will be a result of OnWindowsEvent() and Stop() handling.
	virtual void OnRendererState(RendererState rendererState) = 0;
};


/**
 * Renderer interface
 */
class IRenderer
{
public:

	virtual ~IRenderer() {}

	// Update the video information.
	// The renderer can decide to stop after this and it will signal so by returning false,
	// a return of true means the new state was accepted.
	// ! Only can be called if Start() exectued correctly and before Stop() is called
	virtual bool OnVideoState(VideoStateComPtr&) = 0;

	// Draw the current buffer as frame
	// ! Only can be called if Start() exectued correctly and before Stop() is called
	virtual void OnVideoFrame(VideoFrame&) = 0;

	// Handler for windows events for the graph's pEvent
	virtual HRESULT OnWindowsEvent(LONG_PTR param1, LONG_PTR param2) = 0;

	// Construct the graph
	virtual void Build() = 0;

	// Ask the renderer to start, this can take some time and you'll get notified
	// through the IRendererCallback
	virtual void Start() = 0;

	// Ask the renderer to stop, this can take some time and you'll get notified
	// through the IRendererCallback
	virtual void Stop() = 0;

	// Handle onpaint event
	virtual void OnPaint() = 0;

	// Handle window resize event
	virtual void OnSize() = 0;

	// Get the current frame queue size, negative means no queue
	// Only valid te be called if the RendererState called back RENDERSTATE_RENDERING
	virtual int GetFrameQueueSize() = 0;

	// Get the current "video lead" in milliseconds
	// Video lead is how many ms the last frame start is ahead of the clock.
	// - postive means frame to be rendered in the future, which is what we need
	// - negative means the frame is late, will be rendered immediately
	// Only valid te be called if the RendererState called back RENDERSTATE_RENDERING
	virtual double GetFrameVideoLeadMs() = 0;
};
