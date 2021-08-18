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


/**
 * Renderer callback
 */
struct IRendererCallback
{
	// Note that this will be synchronous with calls to the renderer. No external thread
	// will cause calls. Most likely this will be a result of OnWindowsEvent() and Stop() handling.
	virtual void OnRendererState(RendererState rendererState) = 0;

	// The renderer can report a human-readable string to say what it's doing
	// No need to do anything but just display.
	virtual void OnRendererDetailString(const CString& details) = 0;
};


/**
 * Video renderer interface
 */
class IVideoRenderer
{
public:

	virtual ~IVideoRenderer() {}

	//
	// Runtime
	//

	// Update the video information.
	// The renderer can decide to stop after this and it will signal so by returning false,
	// a return of true means the new state was accepted.
	// ! Only can be called if Start() exectued correctly and before Stop() is called
	virtual bool OnVideoState(VideoStateComPtr&) = 0;

	// Draw the current buffer as frame
	// VideoFrames can be buffered and they can be internally refcounted, hence non-constant
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

	// Reset the internal state and the video stream.
	virtual void Reset() = 0;

	//
	// GUI
	//

	// Window got resized
	virtual void OnSize() = 0;

	// Window needs repainting
	virtual void OnPaint() = 0;

	//
	// Queues
	//

	// Set the video frame queue max size.
	// Only valid te be called if the RendererState called back RENDERSTATE_RENDERING
	// Queues might not be implemented by all renderers, this will throw if it cannot be set
	virtual void SetFrameQueueMaxSize(size_t) = 0;

	// Get the current frame queue size, negative means no queue
	// Only valid te be called if the RendererState called back RENDERSTATE_RENDERING
	// Queues might not be implemented by all renderers, this will return 0 if there is no queueing possible.
	virtual size_t GetFrameQueueSize() = 0;

	//
	// Metrics
	//

	// Get the time it took in milliseconds from the frame's capture timestamp to the entry of the renderer
	// as measured by the given clock.
	// This value does not need to be per frame, sampling is fine
	// Only valid te be called if the RendererState called back RENDERSTATE_RENDERING
	virtual double EntryLatencyMs() const = 0;

	// Get the time it took in milliseconds from the frame's capture timestamp to the point where we hand it
	// over to the bit which puts the image on the wire. It's the furthest possible timestamp we can take.
	// This value does not need to be per frame, sampling is fine
	// Only valid te be called if the RendererState called back RENDERSTATE_RENDERING
	virtual double ExitLatencyMs() const = 0;

	// Get the amount of dropped frames due to queue actions
	virtual uint64_t DroppedFrameCount() const = 0;
};
