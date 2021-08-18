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
#include <guiddef.h>
#include <video_frame_formatter/IVideoFrameFormatter.h>
#include <microsoft_directshow/DirectShowRendererStartStopTimeMethod.h>


static const GUID IID_ILiveSource =
{ 0x77f36c8d, 0xdab1, 0x4356, { 0x82, 0x73, 0x3f, 0xbe, 0x60, 0x63, 0xf2, 0x57 } };

static const GUID CLSID_CLiveSource =
{ 0x63ca2ee3, 0x6460, 0x4098, { 0x9b, 0x74, 0x7, 0x9c, 0xe7, 0x40, 0xe4, 0xb5 } };


DECLARE_INTERFACE_(ILiveSource, IUnknown)
{
	// Initialize, can only be called once
	STDMETHOD(Initialize)(
		IVideoFrameFormatter* videoFrameFormatter,
		const AM_MEDIA_TYPE& mediaSubType,
		timestamp_t frameDuration,
		ITimingClock * timingClock,
		DirectShowStartStopTimeMethod timestamp,
		bool useFrameQueue,
		size_t frameQueueMaxSize) PURE;

	// Destroy, can only be called once
	STDMETHOD(Destroy)(void) PURE;

	// HDR data can change dynamically on a frame-by-frame basis. If you call
	// this then the next frame sent through OnVideoFrame() will carry the information
	STDMETHOD(OnHDRData)(HDRDataSharedPtr&) PURE;

	// New video frame to send out.
	// OnVideoState() has to have been called before this
	STDMETHOD(OnVideoFrame)(VideoFrame&) PURE;

	// Set the video frame queue max size.
	// Only valid te be called if the RendererState called back RENDERSTATE_RENDERING
	// Queues might not be implemented by all renderers, this will throw if it cannot be set
	STDMETHOD(SetFrameQueueMaxSize)(size_t) PURE;

	// Reset the internal state and the video stream.
	STDMETHOD(Reset)(void) PURE;
};
