/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include <vector>
#include <atomic>

#include <DeckLinkAPI_h.h>

#include <VideoFrame.h>
#include <ACaptureDevice.h>
#include <ITimingClock.h>


typedef CComPtr<IDeckLink> IDeckLinkComPtr;


// Known invalid values
#define BMD_PIXEL_FORMAT_INVALID (BMDPixelFormat)0
#define BMD_DISPLAY_MODE_INVALID (BMDDisplayMode)0
#define BMD_TIME_SCALE_INVALID (BMDTimeScale)0
#define BMD_EOTF_INVALID -1
#define BMD_COLOR_SPACE_INVALID -1


/**
 * BlackMagic DeckLink SDK capable capture device
 */
class BlackMagicDeckLinkCaptureDevice:
	public ACaptureDevice,
	public IDeckLinkInputCallback,
	public IDeckLinkProfileCallback,
	public IDeckLinkNotificationCallback,
	public ITimingClock
{
public:

	BlackMagicDeckLinkCaptureDevice(const IDeckLinkComPtr& deckLinkDevice);
	virtual ~BlackMagicDeckLinkCaptureDevice();

	// ACaptureDevice
	void SetCallbackHandler(ICaptureDeviceCallback*) override;
	CString GetName() override;
	bool CanCapture() override;
	void StartCapture() override;
	void StopCapture() override;
	CaptureInputId CurrentCaptureInputId() override;
	CaptureInputs SupportedCaptureInputs() override;
	void SetCaptureInput(const CaptureInputId) override;
	ITimingClock* GetTimingClock() override;
	void SetFrameOffsetMs(int) override;
	double HardwareLatencyMs() const override { return m_hardwareLatencyMs; }
	uint64_t VideoFrameCapturedCount() const override { return m_capturedVideoFrameCount; }
	uint64_t VideoFrameMissedCount() const override { return m_missedVideoFrameCount; }

	// ITimingClock
	timingclocktime_t TimingClockNow() override;
	timingclocktime_t TimingClockTicksPerSecond() const override;
	const TCHAR* TimingClockDescription() override;

	// IDeckLinkInputCallback
	HRESULT VideoInputFormatChanged(BMDVideoInputFormatChangedEvents notificationEvents, IDeckLinkDisplayMode* newDisplayMode, BMDDetectedVideoInputFormatFlags detectedSignalFlags) override;
	HRESULT VideoInputFrameArrived(IDeckLinkVideoInputFrame* videoFrame, IDeckLinkAudioInputPacket* audioPacket) override;

	// IDeckLinkProfileCallback
	HRESULT	ProfileChanging(IDeckLinkProfile* profileToBeActivated, BOOL streamsWillBeForcedToStop) override;
	HRESULT	ProfileActivated(IDeckLinkProfile* activatedProfile) override;

	// IDeckLinkNotificationCallback
	HRESULT Notify(BMDNotifications topic, uint64_t param1, uint64_t param2) override;

	// IUnknown
	HRESULT	QueryInterface(REFIID iid, LPVOID* ppv) override;
	ULONG AddRef() override;
	ULONG Release() override;


private:
	IDeckLinkComPtr m_deckLink;
	CComQIPtr<IDeckLinkConfiguration> m_deckLinkConfiguration;
	CComQIPtr<IDeckLinkProfileAttributes> m_deckLinkAttributes;
	CComQIPtr<IDeckLinkProfileManager> m_deckLinkProfileManager;
	CComQIPtr<IDeckLinkNotification> m_deckLinkNotification;
	CComQIPtr<IDeckLinkStatus> m_deckLinkStatus;
	CComQIPtr<IDeckLinkHDMIInputEDID> m_deckLinkHDMIInputEDID;
	bool m_canCapture = true;
	std::vector<CaptureInput> m_captureInputSet;

	timingclocktime_t m_frameOffsetTicks = 0;
	double m_hardwareLatencyMs = 0;

	// If false this will not send any more frames out.
	std::atomic_bool m_outputCaptureData = false;

	// This is set if the card is capturing, can have only one input supports in here for now.
	// (This implies we support only one callback whereas the hardware supports this per input.)
	BMDVideoConnection m_captureInputId = bmdVideoConnectionUnspecified;
	CComQIPtr<IDeckLinkInput> m_deckLinkInput = nullptr;
	ICaptureDeviceCallback* m_callback = nullptr;

	// We keep the current state of the video here so that we can determine if we have enough
	// data to feed the downstream client. Blackmagic gives you this through many channels and
	// we need to wait for all to be delivered before sending out OnCaptureDeviceVideoStateChange()
	// and OnCaptureDeviceVideoFrame(). There a few helper functions for this as well here.
	// WARNING: R/W from the capture thread, do not read from other thread
	bool m_videoFrameSeen = false;
	BMDPixelFormat m_bmdPixelFormat = BMD_PIXEL_FORMAT_INVALID;
	BMDDisplayMode m_bmdDisplayMode = BMD_DISPLAY_MODE_INVALID;
	timingclocktime_t m_ticksPerFrame = TIMING_CLOCK_TIME_INVALID;
	bool m_videoHasInputSource = false;
	bool m_videoInvertedVertical = false;
	LONGLONG m_videoEotf = BMD_EOTF_INVALID;
	LONGLONG m_videoColorSpace = BMD_COLOR_SPACE_INVALID;
	bool m_videoHasHdrData = false;
	HDRData m_videoHdrData;
	uint64_t m_capturedVideoFrameCount = 0;
	uint64_t m_missedVideoFrameCount = 0;
	timingclocktime_t m_previousTimingClockFrameTime = TIMING_CLOCK_TIME_INVALID;

	void ResetVideoState();

	// Try to create and send a VideoState callback, upon failure will internally call Error() and return false
	bool SendVideoStateCallback();
	void SendCardStateCallback();

	// Current state, update through UpdateState()
	// WARNING: R/W from the capture thread, do not read from other thread
	CaptureDeviceState m_state = CaptureDeviceState::CAPTUREDEVICESTATE_UNKNOWN;
	void UpdateState(CaptureDeviceState state);

	// An error occurred and capture will not proceed (until the input changes)
	void Error(const CString& error);

	// Internal helpers
	void OnNotifyStatusChanged(BMDDeckLinkStatusID statusID);
	void OnLinkStatusBusyChange();

	std::atomic<ULONG> m_refCount;
};
