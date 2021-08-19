/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include <vector>
#include <memory>
#include <atlstr.h>
#include <atomic>

#include <EOTF.h>
#include <ColorSpace.h>
#include <DisplayMode.h>
#include <InputLocked.h>
#include <ColorFormat.h>
#include <BitDepth.h>
#include <HDRData.h>
#include <CaptureInput.h>
#include <VideoFrame.h>
#include <VideoState.h>


typedef std::vector<CaptureInput> CaptureInputs;


/**
 * State of the capture card.
 *
 * This is everything besides the video state. The video state might not change on this changing but that will
 * be a separate message.
 */
class CaptureDeviceCardState:
	public IUnknown
{
public:

	virtual ~CaptureDeviceCardState();

	//
	// Input data
	// This is the data arriving at the capture card from the outside, it's the
	// format on the wire.
	//

	InputLocked inputLocked = InputLocked::UNKNOWN;
	DisplayModeSharedPtr inputDisplayMode = nullptr;
	ColorFormat inputEncoding = ColorFormat::UNKNOWN;
	BitDepth inputBitDepth = BitDepth::UNKNOWN;

	//
	// Random other things represented as strings.
	//  Not important for rendering but might help the user.
	//

	std::vector<CString> other;

	// IUnknown
	HRESULT	QueryInterface(REFIID iid, LPVOID* ppv) override;
	ULONG AddRef() override;
	ULONG Release() override;

private:

	std::atomic<ULONG> m_refCount;
};


typedef CComPtr<CaptureDeviceCardState> CaptureDeviceCardStateComPtr;


// Different states a capture card can be in
enum CaptureDeviceState
{
	// Card can start capturing, but is doing nothing
	CAPTUREDEVICESTATE_READY,

	// Card is capturting
	CAPTUREDEVICESTATE_CAPTURING,

	// States which will not be sent by the card but which can be used
	// by clients when they are expecting a callback for example
	CAPTUREDEVICESTATE_UNKNOWN,
	CAPTUREDEVICESTATE_STARTING,
	CAPTUREDEVICESTATE_STOPPING,
	CAPTUREDEVICESTATE_FAILED
};


const TCHAR* ToString(const CaptureDeviceState eotf);


/**
 * Callbacks from a CaptureDevice
 */
class ICaptureDeviceCallback
{
public:

	// Capture card state
	// WARNING: Often, but not always, called from some internal capture card thread!
	virtual void OnCaptureDeviceState(CaptureDeviceState) = 0;

	// Capture device is signalling that the card state changed. This does not need to impact video.
	// WARNING: Most likely to be called from some internal capture card thread!
	virtual void OnCaptureDeviceCardStateChange(CaptureDeviceCardStateComPtr) = 0;

	// Capture device is signalling that the available video state changed
	// WARNING: Most likely to be called from some internal capture card thread!
	virtual void OnCaptureDeviceVideoStateChange(VideoStateComPtr) = 0;

	// Frame has arrived
	// This is guaranteed to be called after OnCaptureDeviceVideoStateChange() so that the renderer
	// will know what to do with the data.
	// WARNING: Most likely to be called from some internal capture card thread!
	virtual void OnCaptureDeviceVideoFrame(VideoFrame&) = 0;

	// Error occurred, the capture will be stopped
	virtual void OnCaptureDeviceError(const CString&) = 0;
};


/**
 * Abstract Capture Device
 *
 * Represents a device which can capture video
 */
class ACaptureDevice:
	public IUnknown
{
public:

	// Set callback handler.
	virtual void SetCallbackHandler(ICaptureDeviceCallback*) = 0;

	// Get device name (only available after init)
	virtual CString GetName() = 0;

	//
	// Capture
	//

	// Returns true is this device can actually capture. There are cards by capture
	// card manufacturers which cannot capture or are not compatible with what we want.
	virtual bool CanCapture() = 0;

	// Start capturing
	// Will throw if CanCapture() is false
	virtual void StartCapture() = 0;

	// Stop capturing
	// Idempotent
	virtual void StopCapture() = 0;

	//
	// Inputs
	//

	// Get the current capture input id
	virtual CaptureInputId CurrentCaptureInputId() = 0;

	// Get all the supported capture inputs
	virtual CaptureInputs SupportedCaptureInputs() = 0;

	// Set which capture Input to use
	virtual void SetCaptureInput(const CaptureInputId) = 0;

	//
	// Clock
	//

	// Get the timing clock used to time the frames and to get a current timestamp from
	// This clock might not always be available. It is required to be available when the
	// state is CAPTUREDEVICESTATE_CAPTURING. If not available this will return a nullptr.
	virtual ITimingClock* GetTimingClock() = 0;

	// Offset the timestamp used for timestamping the frames. This is useful becasue
	// with capture the timestamp will always be old and hence renderers will always
	// render immeidately rather than exact, this introduces system jitter into the
	// pipeline. With this you can static-offset the timestamp to be more in the future
	// if postitive (or more in the past, but that makes little sense).
	// Value in whole milliseconds
	virtual void SetFrameOffsetMs(int) = 0;

	//
	// Metrics
	//

	// Get the time it took in milliseconds from the capture timestamp to the start
	// of the capture function. This is the hardware delay.
	// This value does not need to be per frame, sampling is fine
	virtual double HardwareLatencyMs() const = 0;

	// Get the amount of (successfully) captured video frames
	virtual uint64_t VideoFrameCapturedCount() const = 0;

	// Get the amount of missed video frames, if possible.
	// Some hardware supports this, sometimes directly and sometimes it can be derived from
	// gaps in the the hardware clock timestamps
	virtual uint64_t VideoFrameMissedCount() const = 0;
};


typedef CComPtr<ACaptureDevice> ACaptureDeviceComPtr;
