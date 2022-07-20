/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <pch.h>


#pragma warning(disable : 26812)  // class enum over class in BM API

#include <set>
#include <math.h>

#include <blackmagic_decklink/BlackMagicDeckLinkTranslate.h>
#include <cie.h>
#include <StringUtils.h>
#include <WallClock.h>

#include "BlackMagicDeckLinkCaptureDevice.h"


static const timingclocktime_t DECKLINK_CLOCK_MAX_TICKS_SECOND = 1000000LL;  // us


//
// Constructor & destructor
//


BlackMagicDeckLinkCaptureDevice::BlackMagicDeckLinkCaptureDevice(const IDeckLinkComPtr& deckLinkDevice) :
	m_deckLink(deckLinkDevice),
	m_deckLinkConfiguration(deckLinkDevice),
	m_deckLinkAttributes(deckLinkDevice),
	m_deckLinkProfileManager(deckLinkDevice),
	m_deckLinkNotification(deckLinkDevice),
	m_deckLinkStatus(deckLinkDevice),
	m_deckLinkHDMIInputEDID(deckLinkDevice)
{
	if (!deckLinkDevice)
		throw std::runtime_error("No DeckLink device given in constructor");

	if (!m_deckLinkNotification)
		throw std::runtime_error("Sorry but this class depends on IDeckLinkNotification which your card does not support");

	// Does the card support format detection?
	BOOL supportsFormatDetection;
	IF_NOT_S_OK(m_deckLinkAttributes->GetFlag(BMDDeckLinkSupportsInputFormatDetection, &supportsFormatDetection))
	{
		m_canCapture = false;
		return;
	}

	// Not simplex or duplex, so no data either way
	LONGLONG duplexMode;
	IF_S_OK(m_deckLinkAttributes->GetInt(BMDDeckLinkDuplex, &duplexMode))
	{
		if((BMDDuplexMode)duplexMode == bmdDuplexInactive)
		{
			m_canCapture = false;
			return;
		}
	}

	// Enable all EDID functionality if possible
	if (m_deckLinkHDMIInputEDID)
	{
		const LONGLONG allKnownRanges = bmdDynamicRangeSDR | bmdDynamicRangeHDRStaticPQ | bmdDynamicRangeHDRStaticHLG;
		IF_NOT_S_OK(m_deckLinkHDMIInputEDID->SetInt(bmdDeckLinkHDMIInputEDIDDynamicRange, allKnownRanges))
			throw std::runtime_error("Failed to set EDID ranges");

		IF_NOT_S_OK(m_deckLinkHDMIInputEDID->WriteToEDID())
			throw std::runtime_error("Failed to write EDID");
	}

	// Get current capture id
	LONGLONG captureInputId;
	if (m_deckLinkConfiguration->GetInt(bmdDeckLinkConfigVideoInputConnection, &captureInputId) != S_OK)
		throw std::runtime_error("Failed to get bmdDeckLinkConfigVideoInputConnection from config");

	m_captureInputId = (BMDVideoConnection)captureInputId;

	if (m_deckLinkProfileManager)
	{
		IF_NOT_S_OK(m_deckLinkProfileManager->SetCallback(this))
			throw std::runtime_error("Failed to set profile manager callback");
	}

	IF_NOT_S_OK(m_deckLinkNotification->Subscribe(bmdStatusChanged, this))
		throw std::runtime_error("Failed to set notification callback");

	// Build capture inputs
	m_captureInputSet.push_back(CaptureInput(static_cast<CaptureInputId>(bmdVideoConnectionSDI), CaptureInputType::SDI_ELECTRICAL, TEXT("SDI")));
	m_captureInputSet.push_back(CaptureInput(static_cast<CaptureInputId>(bmdVideoConnectionHDMI), CaptureInputType::HDMI, TEXT("HDMI")));
	m_captureInputSet.push_back(CaptureInput(static_cast<CaptureInputId>(bmdVideoConnectionOpticalSDI), CaptureInputType::SDI_OPTICAL, TEXT("Optical SDI")));
	m_captureInputSet.push_back(CaptureInput(static_cast<CaptureInputId>(bmdVideoConnectionComponent), CaptureInputType::COMPONENT, TEXT("Component")));
	m_captureInputSet.push_back(CaptureInput(static_cast<CaptureInputId>(bmdVideoConnectionComposite), CaptureInputType::COMPOSITE, TEXT("Composite")));
	m_captureInputSet.push_back(CaptureInput(static_cast<CaptureInputId>(bmdVideoConnectionSVideo), CaptureInputType::S_VIDEO, TEXT("S-Video")));

	ResetVideoState();
	m_state = CaptureDeviceState::CAPTUREDEVICESTATE_READY;
}


BlackMagicDeckLinkCaptureDevice::~BlackMagicDeckLinkCaptureDevice()
{
	m_callback = nullptr;

	if (m_deckLinkProfileManager)
		m_deckLinkProfileManager->SetCallback(nullptr);

	if (m_deckLinkNotification)
		m_deckLinkNotification->Unsubscribe(bmdStatusChanged, this);
}


//
// ACaptureDevice
//


void BlackMagicDeckLinkCaptureDevice::SetCallbackHandler(ICaptureDeviceCallback* callback)
{
	m_callback = callback;

	// Update client if subscribing
	// Note that thisis a read from the state which is set by the capture thread.
	if (m_callback)
	{
		m_callback->OnCaptureDeviceState(m_state);
		SendVideoStateCallback();
	}

	DbgLog((LOG_TRACE, 1, TEXT("BlackMagicDeckLinkCaptureDevice::SetCallbackHandler(): updated callback")));
}


CString BlackMagicDeckLinkCaptureDevice::GetName()
{
	CString name;

	CComBSTR deviceNameBSTR;
	if (m_deckLink->GetDisplayName(&deviceNameBSTR) == S_OK)
	{
		name = CString(deviceNameBSTR);
		::SysFreeString(deviceNameBSTR);
	}
	else
	{
		name = _T("DeckLink");
	}

	return name;
}


bool BlackMagicDeckLinkCaptureDevice::CanCapture()
{
	return m_canCapture;
}


void BlackMagicDeckLinkCaptureDevice::StartCapture()
{
	if (m_outputCaptureData.load(std::memory_order_acquire))
		throw std::runtime_error("StartCapture() callbed but already started");

	if (!CanCapture())
		throw std::runtime_error("Card cannot capture");

	//
	// Set up input
	//

	if (m_captureInputId == bmdVideoConnectionUnspecified)
		throw std::runtime_error("Set input with SetCaptureInput() before calling StartCapture()");

	IF_NOT_S_OK(m_deckLinkConfiguration->SetInt(bmdDeckLinkConfigVideoInputConnection, (int64_t)m_captureInputId))
		throw std::runtime_error("Failed to set bmdDeckLinkConfigVideoInputConnection in config");

	m_deckLinkInput = m_deckLink;
	if (!m_deckLinkInput)
		throw std::runtime_error("Failed to create IDeckLinkInput");

	m_deckLinkInput->SetCallback(this);

	//
	// Enable video input
	//
	CComQIPtr<IDeckLinkDisplayMode> displayMode;
	CComPtr<IDeckLinkDisplayModeIterator> displayModeIterator;

	IF_NOT_S_OK(m_deckLinkInput->GetDisplayModeIterator(&displayModeIterator))
	{
		m_deckLinkInput.Release();
		m_deckLinkInput = nullptr;
		throw std::runtime_error("Failed to get display mode");
	}

	IF_NOT_S_OK(displayModeIterator->Next(&displayMode))
	{
		m_deckLinkInput.Release();
		m_deckLinkInput = nullptr;
		throw std::runtime_error("Failed to get first display mode");
	}

	const static BMDVideoInputFlags videoInputFlags = bmdVideoInputFlagDefault | bmdVideoInputEnableFormatDetection;
	IF_NOT_S_OK(m_deckLinkInput->EnableVideoInput(
		displayMode->GetDisplayMode(),
		bmdFormat8BitYUV,
		videoInputFlags))
	{
		displayMode.Release();
		m_deckLinkInput.Release();
		m_deckLinkInput = nullptr;
		throw std::runtime_error("Failed to EnableVideoInput");
	}

	displayMode.Release();

	//
	// Reset stats
	//

	m_capturedVideoFrameCount = 0;
	m_missedVideoFrameCount = 0;


	//
	// Push current known state, it might be in the
	// right state already so that there never will be
	// a change.
	//

	SendCardStateCallback();


	//
	// Start the capture
	//

	// From here on out data can egress
	m_outputCaptureData.store(true, std::memory_order_release);

	IF_NOT_S_OK(m_deckLinkInput->StartStreams())
	{
		m_deckLinkInput.Release();
		m_deckLinkInput = nullptr;
		throw std::runtime_error("Failed to StartStreams");
	}

	DbgLog((LOG_TRACE, 1, TEXT("BlackMagicDeckLinkCaptureDevice::StopCapture(): completed successfully")));
}


void BlackMagicDeckLinkCaptureDevice::StopCapture()
{
	if (!m_outputCaptureData.load(std::memory_order_acquire))
		throw std::runtime_error("StopCapture() called while not started");

	// Stop egressing data
	m_outputCaptureData.store(false, std::memory_order_release);

	assert(m_deckLinkInput);

	IF_NOT_S_OK(m_deckLinkInput->StopStreams())
	{
		m_deckLinkInput.Release();
		m_deckLinkInput = nullptr;
		throw std::runtime_error("Failed to stop streams");
	}

	IF_NOT_S_OK(m_deckLinkInput->SetCallback(nullptr))
	{
		m_deckLinkInput.Release();
		m_deckLinkInput = nullptr;
		throw std::runtime_error("Failed to set input callback to nullptr");
	}

	IF_NOT_S_OK(m_deckLinkInput->DisableVideoInput())
	{
		m_deckLinkInput.Release();
		m_deckLinkInput = nullptr;
		throw std::runtime_error("Failed to disable video input");
	}

	m_deckLinkInput.Release();
	m_deckLinkInput = nullptr;

	DbgLog((LOG_TRACE, 1, TEXT("BlackMagicDeckLinkCaptureDevice::StopCapture() completed successfully")));
}


CaptureInputId BlackMagicDeckLinkCaptureDevice::CurrentCaptureInputId()
{
	return static_cast<CaptureInputId>(m_captureInputId);
}


CaptureInputs BlackMagicDeckLinkCaptureDevice::SupportedCaptureInputs()
{
	CaptureInputs captureInputs;

	LONGLONG availableInputConnections;
	IF_NOT_S_OK(m_deckLinkAttributes->GetInt(BMDDeckLinkVideoInputConnections, &availableInputConnections))
		throw std::runtime_error("Failed to get BMDDeckLinkVideoInputConnections from attributes");

	for (const auto& captureInput : m_captureInputSet)
	{
		// The used id's are actually BMDVideoConnection.
		if ((captureInput.id & (BMDVideoConnection)availableInputConnections) != 0)
		{
			captureInputs.push_back(captureInput);
		}
	}

	return captureInputs;
}


void BlackMagicDeckLinkCaptureDevice::SetCaptureInput(const CaptureInputId captureInputId)
{
	DbgLog((LOG_TRACE, 1, TEXT("BlackMagicDeckLinkCaptureDevice::SetCaptureInput() to %i (only used in StartCapture())"), m_captureInputId));

	m_captureInputId = static_cast<BMDVideoConnection>(captureInputId);
}


ITimingClock* BlackMagicDeckLinkCaptureDevice::GetTimingClock()
{
	// WARNING: m_state will be updated by some internal capture thread so this might be a race
	//          condition. Probably only an academic problem given that the clients of this will
	//          only be requesting it after they see the capture state
	if (m_state != CaptureDeviceState::CAPTUREDEVICESTATE_CAPTURING)
		return nullptr;

	return this;
}


void BlackMagicDeckLinkCaptureDevice::SetFrameOffsetMs(int frameOffsetMs)
{
	DbgLog((LOG_TRACE, 1, TEXT("BlackMagicDeckLinkCaptureDevice::SetFrameOffsetMs() to %i"), frameOffsetMs));

	static_assert(DECKLINK_CLOCK_MAX_TICKS_SECOND % 1000 == 0, "DECKLINK_CLOCK_MAX_TICKS_SECOND  must be mod 1k for optimization here");
	const timingclocktime_t ticksPerMs = DECKLINK_CLOCK_MAX_TICKS_SECOND / 1000;
	m_frameOffsetTicks = frameOffsetMs * ticksPerMs;
}


//
// ITimingClock
//


timingclocktime_t BlackMagicDeckLinkCaptureDevice::TimingClockNow()
{
	assert(m_deckLinkInput);

	assert(m_outputCaptureData.load(std::memory_order_acquire));
	assert(
		m_state == CaptureDeviceState::CAPTUREDEVICESTATE_CAPTURING ||
		m_state == CaptureDeviceState::CAPTUREDEVICESTATE_READY );  // TODO: We will also get called if we're ready not sure if we want to be more strict on this and not allow it + tighten up state machine

	BMDTimeValue currentTimeTicks;
	IF_NOT_S_OK(m_deckLinkInput->GetHardwareReferenceClock(
		TimingClockTicksPerSecond(),
		&currentTimeTicks,
		nullptr, nullptr))
		throw std::runtime_error("Could not get the hardware clock timestamp");

	return currentTimeTicks;
}


timingclocktime_t BlackMagicDeckLinkCaptureDevice::TimingClockTicksPerSecond() const
{
	// This is hard-coded, we can also take a more course approach by using the exact frame-frequency muliplied by 1000
	// as shown in the DeckLink examples. Given that we most likely want to convert it to a DirectShow timestamp,
	// which is 100ns the choice here is to go for maximum usable resolution.
	return DECKLINK_CLOCK_MAX_TICKS_SECOND;
}


const TCHAR* BlackMagicDeckLinkCaptureDevice::TimingClockDescription()
{
	return TEXT("DeckLink hardware clock");
}


//
// IDeckLinkInputCallback
//


HRESULT STDMETHODCALLTYPE BlackMagicDeckLinkCaptureDevice::VideoInputFormatChanged(
	BMDVideoInputFormatChangedEvents notificationEvents,
	IDeckLinkDisplayMode* newMode,
	BMDDetectedVideoInputFormatFlags detectedSignalFlags)
{
	// WARNING: Called from some internal capture card thread!
	// TODO: We can be nicer and "return E_INVALIDARG;" for the throws, investigate how that's handled gracefully

	// Dot not process if we're not capturing anymore
	if (!m_outputCaptureData.load(std::memory_order_acquire))
		return S_OK;

#ifdef _DEBUG

	// We can validate our Displaymode against the given
	{
		DisplayModeSharedPtr dp = Translate(newMode->GetDisplayMode());
		assert(dp->FrameWidth() == newMode->GetWidth());
		assert(dp->FrameHeight() == newMode->GetHeight());

		BMDTimeValue frameDuration;
		BMDTimeScale timeScale;
		assert(newMode->GetFrameRate(&frameDuration, &timeScale) == S_OK);
		const double frameRate = (double)timeScale / frameDuration;
		assert(fabs(frameRate - dp->RefreshRateHz()) < 0.01);
	}

#endif // _DEBUG

	bool changed = false;

	//
	// Determine pixel format
	//
	BMDPixelFormat bmdPixelFormat;
	if (detectedSignalFlags & bmdDetectedVideoInputRGB444)
	{
		if (detectedSignalFlags & bmdDetectedVideoInput8BitDepth)
			bmdPixelFormat = bmdFormat8BitARGB;
		else if (detectedSignalFlags & bmdDetectedVideoInput10BitDepth)
			bmdPixelFormat = bmdFormat10BitRGB;
		else if (detectedSignalFlags & bmdDetectedVideoInput12BitDepth)
			bmdPixelFormat = bmdFormat12BitRGB;
		else
			throw std::runtime_error("Unknown pixel format for RGB444");
	}
	else if (detectedSignalFlags & bmdDetectedVideoInputYCbCr422)
	{
		if (detectedSignalFlags & bmdDetectedVideoInput8BitDepth)
			bmdPixelFormat = bmdFormat8BitYUV;
		else if (detectedSignalFlags & bmdDetectedVideoInput10BitDepth)
			bmdPixelFormat = bmdFormat10BitYUV;
		else if (detectedSignalFlags & bmdDetectedVideoInput12BitDepth)
		{
			Error(TEXT("DeckLink does not support YCbCr422 12bit input"));
			return E_FAIL;
		}
		else
		{
			Error(TEXT("Unknown bit depth for YCbCr422"));
			return E_FAIL;
		}
	}
	else
	{
		Error(TEXT("Failed to determine input (not RGB or YCbCr422)"));
		return E_FAIL;
	}

	//
	// Things changed and we will stop current capture and restart.
	// That means the video state will be invalid and we'll need to wait for it be to be rebuilt.
	//
	if ((m_bmdPixelFormat != bmdPixelFormat) ||
		(notificationEvents & bmdVideoInputDisplayModeChanged) ||
		(notificationEvents & bmdVideoInputColorspaceChanged) ||
		(notificationEvents & bmdVideoInputFieldDominanceChanged))
	{
		DbgLog((LOG_TRACE, 1, TEXT("BlackMagicDeckLinkCaptureDevice::VideoInputFormatChanged(): detected change")));

		//
		// Wipe internal state & store what we know
		//

		ResetVideoState();
		m_bmdPixelFormat = bmdPixelFormat;
		m_bmdDisplayMode = newMode->GetDisplayMode();
		m_ticksPerFrame = (timingclocktime_t)round((1.0 / FPS(m_bmdDisplayMode)) * TimingClockTicksPerSecond());

		// Inform callback handlers that stream will be invalid before re-starting
		if (!SendVideoStateCallback())
			return E_FAIL;

		//
		// Restart stream with new input mode
		//
		IF_NOT_S_OK(m_deckLinkInput->StopStreams())
		{
			m_deckLinkInput.Release();
			m_deckLinkInput = nullptr;

			Error(TEXT("Failed to stop streams"));
			return E_FAIL;
		}

		// Set the video input mode
		IF_NOT_S_OK(m_deckLinkInput->EnableVideoInput(
			newMode->GetDisplayMode(),
			bmdPixelFormat,
			bmdVideoInputFlagDefault | bmdVideoInputEnableFormatDetection))
		{
			// TODO: StopStreams() or how does this work under failure conditions?

			m_deckLinkInput.Release();
			m_deckLinkInput = nullptr;

			Error(TEXT("Failed to set video input"));
			return E_FAIL;
		}

		// Start the capture
		IF_NOT_S_OK(m_deckLinkInput->StartStreams())
		{
			// TODO: StopStreams() or how does this work under failure conditions?

			m_deckLinkInput.Release();
			m_deckLinkInput = nullptr;

			Error(TEXT("Failed to start stream"));
			return E_FAIL;
		}

		DbgLog((LOG_TRACE, 1, TEXT("BlackMagicDeckLinkCaptureDevice::VideoInputFormatChanged(): restart success")));
	}

	return S_OK;
}


HRESULT STDMETHODCALLTYPE BlackMagicDeckLinkCaptureDevice::VideoInputFrameArrived(
	IDeckLinkVideoInputFrame* videoFrame,
	IDeckLinkAudioInputPacket* audioPacket)
{
	// WARNING: Called from some internal capture card thread!

	// Dot not process if we're not capturing anymore
	if (!m_outputCaptureData.load(std::memory_order_acquire))
		return S_OK;

	// TODO: This smells like a poor state machine, how can we be here if m_outputCaptureData is false?
	if (m_bmdDisplayMode == BMD_DISPLAY_MODE_INVALID)
		return S_OK;

	bool videoStateChanged = false;

	if (videoFrame)
	{
		assert(m_bmdDisplayMode);

		timingclocktime_t timingClockFrameTime = 0;

		// Get timestamp
		IF_NOT_S_OK(videoFrame->GetHardwareReferenceTimestamp(TimingClockTicksPerSecond(), &timingClockFrameTime, nullptr))
		{
			Error(TEXT("Failed to get video frame hardware timestamp"));
			return E_FAIL;
		}

		// Figure out how many frames fit in the interval
		if (m_previousTimingClockFrameTime != TIMING_CLOCK_TIME_INVALID)
		{
			assert(m_previousTimingClockFrameTime < timingClockFrameTime);

			const double frameDiffTicks = (double)(timingClockFrameTime - m_previousTimingClockFrameTime);
			const int frames = (int)round(frameDiffTicks / m_ticksPerFrame);
			assert(frames >= 0);

			m_capturedVideoFrameCount += frames;
			m_missedVideoFrameCount += std::max((frames - 1), 0);
		}

		m_previousTimingClockFrameTime = timingClockFrameTime;

		// Every every so often get the hardware latency.
		// TODO: Change to framerate rather than fixed number of frames
		if(m_capturedVideoFrameCount % 20 == 0)
		{
			timingclocktime_t timingClockNow = TimingClockNow();
			m_hardwareLatencyMs = TimingClockDiffMs(timingClockFrameTime, timingClockNow, TimingClockTicksPerSecond());
		}

		// Offset timestamp. Do this after getting the hardware latency else it'll account for this as well
		timingClockFrameTime += m_frameOffsetTicks;

		// Check if vertical inverted
		const bool videoInvertedVertical = (videoFrame->GetFlags() & bmdFrameFlagFlipVertical) != 0;
		if (videoInvertedVertical != m_videoInvertedVertical)
		{
			m_videoInvertedVertical = videoInvertedVertical;
			videoStateChanged = true;
		}

		// TODO: What to do with this?
		//assert(videoFrame->GetFlags() & bmdFrameCapturedAsPsF == 0);

#ifdef _DEBUG
		{
			// Check the incoming video frame against our known state and translations
			DisplayModeSharedPtr dp = Translate(m_bmdDisplayMode);
			assert(videoFrame->GetHeight() == dp->FrameHeight());
			assert(videoFrame->GetWidth() == dp->FrameWidth());
			assert(videoFrame->GetPixelFormat() == m_bmdPixelFormat);

			VideoState vs;
			vs.displayMode = dp;
			vs.videoFrameEncoding = Translate(m_bmdPixelFormat, vs.colorspace);
			assert(vs.BytesPerRow() == videoFrame->GetRowBytes());
		}
#endif // _DEBUG

		double doubleValue = 0.0;
		LONGLONG intValue = 0;

		// Input changed
		const bool hasInput = ((videoFrame->GetFlags() & bmdFrameHasNoInputSource) == 0);
		if (hasInput)
		{
			if (!m_videoHasInputSource)
			{
				m_videoHasInputSource = true;
				videoStateChanged = true;
			}

			m_videoFrameSeen = true;
		}
		else if (m_videoHasInputSource)
		{
			m_videoHasInputSource = false;
			videoStateChanged = true;
		}

		// Metdata
		CComQIPtr<IDeckLinkVideoFrameMetadataExtensions> metadataExtensions(videoFrame);
		if (metadataExtensions)
		{
			// EOTF
			IF_S_OK(metadataExtensions->GetInt(bmdDeckLinkFrameMetadataHDRElectroOpticalTransferFunc, &intValue))
			{
				if (m_videoEotf != intValue)
				{
					m_videoEotf = intValue;
					videoStateChanged = true;
				}
			}

			// Color space
			IF_S_OK(metadataExtensions->GetInt(bmdDeckLinkFrameMetadataColorspace, &intValue))
			{
				if (m_videoColorSpace != intValue)
				{
					m_videoColorSpace = intValue;
					videoStateChanged = true;
				}
			}

			// HDR meta data
			if (videoFrame->GetFlags() & bmdFrameContainsHDRMetadata)
			{
				// Was nothing, now is something
				if (!m_videoHasHdrData)
				{
					m_videoHasHdrData = true;
					videoStateChanged = true;
				}

				// Primaries
				IF_NOT_S_OK(metadataExtensions->GetFloat(bmdDeckLinkFrameMetadataHDRDisplayPrimariesBlueX, &doubleValue))
					doubleValue = 0.0;
				if (!CieEquals(m_videoHdrData.displayPrimaryBlueX, doubleValue))
				{
					m_videoHdrData.displayPrimaryBlueX = doubleValue;
					videoStateChanged = true;
				}

				IF_NOT_S_OK(metadataExtensions->GetFloat(bmdDeckLinkFrameMetadataHDRDisplayPrimariesBlueY, &doubleValue))
					doubleValue = 0.0;
				if (!CieEquals(m_videoHdrData.displayPrimaryBlueY, doubleValue))
				{
					m_videoHdrData.displayPrimaryBlueY = doubleValue;
					videoStateChanged = true;
				}

				IF_NOT_S_OK(metadataExtensions->GetFloat(bmdDeckLinkFrameMetadataHDRDisplayPrimariesRedX, &doubleValue))
					doubleValue = 0.0;
				if (!CieEquals(m_videoHdrData.displayPrimaryRedX, doubleValue))
				{
					m_videoHdrData.displayPrimaryRedX = doubleValue;
					videoStateChanged = true;
				}

				IF_NOT_S_OK(metadataExtensions->GetFloat(bmdDeckLinkFrameMetadataHDRDisplayPrimariesRedY, &doubleValue))
					doubleValue = 0.0;
				if (!CieEquals(m_videoHdrData.displayPrimaryRedY, doubleValue))
				{
					m_videoHdrData.displayPrimaryRedY = doubleValue;
					videoStateChanged = true;
				}

				IF_NOT_S_OK(metadataExtensions->GetFloat(bmdDeckLinkFrameMetadataHDRDisplayPrimariesGreenX, &doubleValue))
					doubleValue = 0.0;
				if (!CieEquals(m_videoHdrData.displayPrimaryGreenX, doubleValue))
				{
					m_videoHdrData.displayPrimaryGreenX = doubleValue;
					videoStateChanged = true;
				}

				IF_NOT_S_OK(metadataExtensions->GetFloat(bmdDeckLinkFrameMetadataHDRDisplayPrimariesGreenY, &doubleValue))
					doubleValue = 0.0;
				if (!CieEquals(m_videoHdrData.displayPrimaryGreenY, doubleValue))
				{
					m_videoHdrData.displayPrimaryGreenY = doubleValue;
					videoStateChanged = true;
				}

				// White point
				IF_NOT_S_OK(metadataExtensions->GetFloat(bmdDeckLinkFrameMetadataHDRWhitePointX, &doubleValue))
					doubleValue = 0.0;
				if (!CieEquals(m_videoHdrData.whitePointX, doubleValue))
				{
					m_videoHdrData.whitePointX = doubleValue;
					videoStateChanged = true;
				}

				IF_NOT_S_OK(metadataExtensions->GetFloat(bmdDeckLinkFrameMetadataHDRWhitePointY, &doubleValue))
					doubleValue = 0.0;
				if (!CieEquals(m_videoHdrData.whitePointY, doubleValue))
				{
					m_videoHdrData.whitePointY = doubleValue;
					videoStateChanged = true;
				}

				// Mastering display luminance
				IF_NOT_S_OK(metadataExtensions->GetFloat(bmdDeckLinkFrameMetadataHDRMaxDisplayMasteringLuminance, &doubleValue))
					doubleValue = 0.0;
				if (fabs(m_videoHdrData.masteringDisplayMaxLuminance-doubleValue) > 0.001)
				{
					m_videoHdrData.masteringDisplayMaxLuminance = doubleValue;
					videoStateChanged = true;
				}

				IF_NOT_S_OK(metadataExtensions->GetFloat(bmdDeckLinkFrameMetadataHDRMinDisplayMasteringLuminance, &doubleValue))
					doubleValue = 0.0;
				if (fabs(m_videoHdrData.masteringDisplayMinLuminance - doubleValue) > 0.001)
				{
					m_videoHdrData.masteringDisplayMinLuminance = doubleValue;
					videoStateChanged = true;
				}

				// MaxCLL MaxFALL
				IF_NOT_S_OK(metadataExtensions->GetFloat(bmdDeckLinkFrameMetadataHDRMaximumContentLightLevel, &doubleValue))
					doubleValue = 0.0;
				if (m_videoHdrData.maxCll != doubleValue)
				{
					m_videoHdrData.maxCll = doubleValue;
					videoStateChanged = true;
				}

				IF_NOT_S_OK(metadataExtensions->GetFloat(bmdDeckLinkFrameMetadataHDRMaximumFrameAverageLightLevel, &doubleValue))
					doubleValue = 0.0;
				if (m_videoHdrData.maxFall != doubleValue)
				{
					m_videoHdrData.maxFall = doubleValue;
					videoStateChanged = true;
				}
			}
			else
			{
				// Now no data, but had data before
				if (m_videoHasHdrData)
				{
					m_videoHasHdrData = false;
					videoStateChanged = true;
				}
			}
		}

		if (videoStateChanged)
		{
			if (!SendVideoStateCallback())
				return E_FAIL;
		}

		void* data;
		if (FAILED(videoFrame->GetBytes(&data)))
			throw std::runtime_error("Failed to get video frame bytes");

		VideoFrame vpVideoFrame(
			data, m_capturedVideoFrameCount,
			timingClockFrameTime, videoFrame);

		m_callback->OnCaptureDeviceVideoFrame(vpVideoFrame);
	}  // videoFrame

	return S_OK;
}


//
// IDeckLinkProfileCallback
//


HRESULT	STDMETHODCALLTYPE BlackMagicDeckLinkCaptureDevice::ProfileChanging(
	IDeckLinkProfile* profileToBeActivated, BOOL streamsWillBeForcedToStop)
{
	// WARNING: Called from some internal capture card thread!

	if (streamsWillBeForcedToStop)
	{
		// TODO: Handle properly by stopping and informing the client through the state callback
		throw std::runtime_error("Profile changed and forced to stop, not yet implemented");
	}

	return S_OK;
}


HRESULT	STDMETHODCALLTYPE BlackMagicDeckLinkCaptureDevice::ProfileActivated(IDeckLinkProfile* activatedProfile)
{
	// WARNING: Called from some internal capture card thread!
	// TODO: Do we need to do anything here?

	return S_OK;
}


//
// IDeckLinkNotificationCallback
//


HRESULT BlackMagicDeckLinkCaptureDevice::Notify(BMDNotifications topic, uint64_t param1, uint64_t param2)
{
	// WARNING: Called from some internal capture card thread!

	switch (topic)
	{

	case bmdPreferencesChanged:
		throw std::runtime_error("Preference change not implemented");
		break;

	case bmdStatusChanged:
		OnNotifyStatusChanged(static_cast<BMDDeckLinkStatusID>(param1));
		break;

	default:
		throw std::runtime_error("Unknown BMDNotifications");
	}

	return S_OK;
}


//
// IUnknown
//


HRESULT	BlackMagicDeckLinkCaptureDevice::QueryInterface(REFIID iid, LPVOID* ppv)
{
	if (!ppv)
		return E_INVALIDARG;

	// Initialise the return result
	*ppv = nullptr;

	// Obtain the IUnknown interface and compare it the provided REFIID
	if (iid == IID_IUnknown)
	{
		*ppv = this;
		AddRef();
		return S_OK;
	}
	else if (iid == IID_IDeckLinkInputCallback)
	{
		*ppv = static_cast<IDeckLinkInputCallback*>(this);
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}


ULONG BlackMagicDeckLinkCaptureDevice::AddRef(void)
{
	return ++m_refCount;
}


ULONG BlackMagicDeckLinkCaptureDevice::Release(void)
{
	ULONG newRefValue = --m_refCount;
	if (newRefValue == 0)
		delete this;

	return newRefValue;
}


void BlackMagicDeckLinkCaptureDevice::ResetVideoState()
{
	m_videoFrameSeen = false;
	m_bmdPixelFormat = BMD_PIXEL_FORMAT_INVALID;
	m_bmdDisplayMode = BMD_DISPLAY_MODE_INVALID;
	m_videoHasInputSource = false;
	m_videoEotf = BMD_EOTF_INVALID;
	m_videoColorSpace = BMD_COLOR_SPACE_INVALID;
	m_videoHasHdrData = false;

	ZeroMemory(&m_videoHdrData, sizeof(m_videoHdrData));
}


bool BlackMagicDeckLinkCaptureDevice::SendVideoStateCallback()
{
	// WARNING: Called from some internal capture card thread!

	assert(m_callback);

	const bool hasValidVideoState =
		(m_videoFrameSeen) &&
		(m_bmdPixelFormat != BMD_PIXEL_FORMAT_INVALID) &&
		(m_bmdDisplayMode != BMD_DISPLAY_MODE_INVALID) &&
		(m_videoHasInputSource) &&
		(m_videoEotf != BMD_EOTF_INVALID) &&
		(m_videoColorSpace != BMD_COLOR_SPACE_INVALID);

	const bool hasValidHdrData =
		m_videoHasHdrData &&
		m_videoHdrData.IsValid();

	//
	// Build and send reply
	//

	try
	{

		VideoStateComPtr videoState = new VideoState();
		if (!videoState)
			throw std::runtime_error("Failed to alloc VideoStateComPtr");

		// Not valid, don't send
		if (!hasValidVideoState)
		{
			videoState->valid = false;
		}
		// Valid state, send
		else
		{
			assert(m_videoFrameSeen);
			assert(m_videoHasInputSource);
			assert(m_bmdPixelFormat != BMD_PIXEL_FORMAT_INVALID);
			assert(m_bmdDisplayMode != BMD_DISPLAY_MODE_INVALID);
			assert(m_videoEotf != BMD_EOTF_INVALID);
			assert(m_videoColorSpace != BMD_COLOR_SPACE_INVALID);

			videoState->valid = true;
			videoState->displayMode = Translate(m_bmdDisplayMode);
			videoState->eotf = TranslateEOTF(m_videoEotf);
			videoState->colorspace = Translate(
				(BMDColorspace)m_videoColorSpace,
				videoState->displayMode->FrameHeight());
			videoState->invertedVertical = m_videoInvertedVertical;
			videoState->videoFrameEncoding = Translate(m_bmdPixelFormat, videoState->colorspace);

			// Build a fresh copy of the HDR data if valid
			if (hasValidHdrData)
			{
				videoState->hdrData = std::make_shared<HDRData>();
				*(videoState->hdrData) = m_videoHdrData;
			}
		}

		m_callback->OnCaptureDeviceVideoStateChange(videoState);
	}
	catch (const std::runtime_error& e)
	{
		wchar_t* ew = ToString(e.what());
		Error(ew);
		delete[] ew;

		return false;
	}

	return true;
}


void BlackMagicDeckLinkCaptureDevice::SendCardStateCallback()
{
	// WARNING: Called from some internal capture card thread!

	assert(m_deckLinkStatus);

	if (!m_callback)
		return;

	CaptureDeviceCardStateComPtr cardState = new CaptureDeviceCardState();
	if (!cardState)
		throw std::runtime_error("Failed to alloc CaptureDeviceCardStateComPtr");

	LONGLONG intValue;
	BOOL boolValue;

	//
	// Input data
	//

	IF_S_OK(m_deckLinkStatus->GetFlag(bmdDeckLinkStatusVideoInputSignalLocked, &boolValue))
		cardState->inputLocked = boolValue ? InputLocked::YES : InputLocked::NO;

	IF_S_OK(m_deckLinkStatus->GetInt(bmdDeckLinkStatusDetectedVideoInputMode, &intValue))
		cardState->inputDisplayMode = Translate((BMDDisplayMode)intValue);

	IF_S_OK(m_deckLinkStatus->GetInt(bmdDeckLinkStatusDetectedVideoInputFormatFlags, &intValue))
	{
		cardState->inputEncoding = TranslateColorFormat((BMDDetectedVideoInputFormatFlags)intValue);
		cardState->inputBitDepth = TranslateBithDepth((BMDDetectedVideoInputFormatFlags)intValue);
	}

	//
	// Other
	//
	CString s;

	if (m_deckLinkStatus->GetInt(bmdDeckLinkStatusPCIExpressLinkWidth, &intValue) == S_OK)
	{
		s.Format(_T("PCIe link width: %lld"), intValue);
		cardState->other.push_back(s);
	}

	if (m_deckLinkStatus->GetInt(bmdDeckLinkStatusPCIExpressLinkSpeed, &intValue) == S_OK)
	{
		s.Format(_T("PCIe link speed: %lld"), intValue);
		cardState->other.push_back(s);
	}

	//
	// Send
	//
	m_callback->OnCaptureDeviceCardStateChange(cardState);
}


void BlackMagicDeckLinkCaptureDevice::UpdateState(CaptureDeviceState state)
{
	// WARNING: Called from some internal capture card thread!

	assert(state != m_state);  // Double state is not allowed
	m_state = state;

	if (m_callback)
		m_callback->OnCaptureDeviceState(state);
}


void BlackMagicDeckLinkCaptureDevice::Error(const CString& error)
{
	// WARNING: Can be called from any thread.

	if (m_callback)
		m_callback->OnCaptureDeviceError(error);

	// TODO: Stop capture and return error state?
}


//
// Internal helpers
//


void BlackMagicDeckLinkCaptureDevice::OnNotifyStatusChanged(BMDDeckLinkStatusID statusID)
{
	// WARNING: Called from some internal capture card thread!

	if (!m_deckLinkInput)
		return;

	switch (statusID)
	{
	// Device state changed
	case bmdDeckLinkStatusBusy:
		OnLinkStatusBusyChange();
		break;

	// Card state changed.
	case bmdDeckLinkStatusVideoInputSignalLocked:
	case bmdDeckLinkStatusDetectedVideoInputFieldDominance:
	case bmdDeckLinkStatusDetectedVideoInputMode:
	case bmdDeckLinkStatusDetectedVideoInputFormatFlags:
		SendCardStateCallback();
		break;

	// Video state changed
	case bmdDeckLinkStatusCurrentVideoInputPixelFormat:
	case bmdDeckLinkStatusDetectedVideoInputColorspace:
	case bmdDeckLinkStatusCurrentVideoInputMode:
		// not used as we get these from from the frame and format callbacks
		break;

	// All others ignored
	default:
		break;
	}
}


void BlackMagicDeckLinkCaptureDevice::OnLinkStatusBusyChange()
{
	// WARNING: Called from some internal capture card thread!

	LONGLONG intValue;
	IF_NOT_S_OK(m_deckLinkStatus->GetInt(bmdDeckLinkStatusBusy, &intValue))
		throw std::runtime_error("Failed to call bmdDeckLinkStatusBusy");

	const bool captureBusy = (intValue & bmdDeviceCaptureBusy);

	if (captureBusy)
	{
		assert(m_state == CaptureDeviceState::CAPTUREDEVICESTATE_READY);
		UpdateState(CaptureDeviceState::CAPTUREDEVICESTATE_CAPTURING);
	}
	else
	{
		assert(m_state == CaptureDeviceState::CAPTUREDEVICESTATE_CAPTURING);
		ResetVideoState();
		UpdateState(CaptureDeviceState::CAPTUREDEVICESTATE_READY);
	}
}
