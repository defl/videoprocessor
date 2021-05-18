/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <stdafx.h>


#pragma warning(disable : 26812)  // class enum over class in BM API

#include <set>

#include <blackmagic_decklink/BlackMagicDeckLinkTranslate.h>
#include <cie.h>

#include "BlackMagicDeckLinkCaptureDevice.h"


//
// Constructor & destructor
//


BlackMagicDeckLinkCaptureDevice::BlackMagicDeckLinkCaptureDevice(const IDeckLinkComPtr& deckLinkDevice) :
	m_deckLink(deckLinkDevice),
	m_deckLinkConfiguration(deckLinkDevice),
	m_deckLinkAttributes(deckLinkDevice),
	m_deckLinkProfileManager(deckLinkDevice),
	m_deckLinkNotification(deckLinkDevice),
	m_deckLinkStatus(deckLinkDevice)
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
	CComQIPtr<IDeckLinkHDMIInputEDID> deckLinkHDMIInputEDID = deckLinkDevice;
	if (deckLinkHDMIInputEDID)
	{
		const LONGLONG allKnownRanges = bmdDynamicRangeSDR | bmdDynamicRangeHDRStaticPQ | bmdDynamicRangeHDRStaticHLG;
		IF_NOT_S_OK(deckLinkHDMIInputEDID->SetInt(bmdDeckLinkHDMIInputEDIDDynamicRange, allKnownRanges))
			throw std::runtime_error("Failed to set EDID ranges");

		IF_NOT_S_OK(deckLinkHDMIInputEDID->WriteToEDID())
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
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		m_callback = callback;

		// Update client if subscribing
		if (m_callback)
			m_callback->OnCaptureDeviceState(m_state);
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
	if (!CanCapture())
		throw std::runtime_error("Card cannot capture");

	{
		std::lock_guard<std::mutex> lock(m_mutex);

		if (m_deckLinkInput)
			throw std::runtime_error("Card already capturing");

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
		// Start the capture
		//

		IF_NOT_S_OK(m_deckLinkInput->StartStreams())
		{
			m_deckLinkInput.Release();
			m_deckLinkInput = nullptr;
			throw std::runtime_error("Failed to StartStreams");
		}
	}

	DbgLog((LOG_TRACE, 1, TEXT("BlackMagicDeckLinkCaptureDevice::StopCapture(): completed successfully")));
}


void BlackMagicDeckLinkCaptureDevice::StopCapture()
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		if (!m_deckLinkInput)
			return;

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
	}

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
	m_captureInputId = static_cast<BMDVideoConnection>(captureInputId);

	DbgLog((LOG_TRACE, 1, TEXT("BlackMagicDeckLinkCaptureDevice::SetCaptureInput() to %i (only used in StartCapture())"), m_captureInputId));
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
		assert(fabs(frameRate - dp->RefreshRateHz()) < 0.1);
	}

#endif // _DEBUG

	bool changed = false;

	//
	// Determine pixel format
	//
	BMDPixelFormat pixelFormat;
	if (detectedSignalFlags & bmdDetectedVideoInputRGB444)
	{
		if (detectedSignalFlags & bmdDetectedVideoInput8BitDepth)
			pixelFormat = bmdFormat8BitARGB;
		else if (detectedSignalFlags & bmdDetectedVideoInput10BitDepth)
			pixelFormat = bmdFormat10BitRGB;
		else if (detectedSignalFlags & bmdDetectedVideoInput12BitDepth)
			pixelFormat = bmdFormat12BitRGB;
		else
			throw std::runtime_error("Unknown pixel format for RGB444");
	}
	else if (detectedSignalFlags & bmdDetectedVideoInputYCbCr422)
	{
		if (detectedSignalFlags & bmdDetectedVideoInput8BitDepth)
			pixelFormat = bmdFormat8BitYUV;
		else if (detectedSignalFlags & bmdDetectedVideoInput10BitDepth)
			pixelFormat = bmdFormat10BitYUV;
		else
			throw std::runtime_error("Unknown pixel format for YCbCr422");
	}
	else
		throw std::runtime_error("Unknown video input");

	//
	// Things changed and we will stop current capture and restart.
	// That means the video state will be invalid and we'll need to wait for it be to be rebuilt.
	//
	if ((m_pixelFormat != pixelFormat) ||
		(notificationEvents & bmdVideoInputDisplayModeChanged) ||
		(notificationEvents & bmdVideoInputColorspaceChanged) ||
		(notificationEvents & bmdVideoInputFieldDominanceChanged))
	{
		DbgLog((LOG_TRACE, 1, TEXT("BlackMagicDeckLinkCaptureDevice::VideoInputFormatChanged(): detected change")));

		//
		// Wipe internal state & store what we know
		//

		ResetVideoState();
		m_pixelFormat = pixelFormat;
		m_videoDisplayMode = newMode->GetDisplayMode();

		{
			std::lock_guard<std::mutex> lock(m_mutex);

			// Inform callback handlers that stream will be invalid before re-starting
			// Must be locked!
			SendVideoStateCallback();

			//
			// Restart stream with new input mode
			//
			IF_NOT_S_OK(m_deckLinkInput->StopStreams())
			{
				m_deckLinkInput.Release();
				m_deckLinkInput = nullptr;
				throw std::runtime_error("Failed to stop streams");
			}

			// Set the video input mode
			IF_NOT_S_OK(m_deckLinkInput->EnableVideoInput(
				newMode->GetDisplayMode(),
				pixelFormat,
				bmdVideoInputFlagDefault | bmdVideoInputEnableFormatDetection))
			{
				// TODO: StopStreams() or how does this work under failure conditions?

				m_deckLinkInput.Release();
				m_deckLinkInput = nullptr;
				throw std::runtime_error("Failed to set video input");
			}

			// Start the capture
			IF_NOT_S_OK(m_deckLinkInput->StartStreams())
			{
				// TODO: StopStreams() or how does this work under failure conditions?

				m_deckLinkInput.Release();
				m_deckLinkInput = nullptr;
				throw std::runtime_error("Failed to start stream");
			}
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
	// TODO: Process audioPacket

	if (m_videoDisplayMode == BMD_DISPLAY_MODE_INVALID)
		return -1;

	bool videoStateChanged = false;

	if (videoFrame)
	{
		assert(m_videoDisplayMode);

		// TODO: The following 2 are set, no idea what to do with them on output as everything seems to work
		//assert(videoFrame->GetFlags() & bmdFrameFlagFlipVertical == 0);
		//assert(videoFrame->GetFlags() & bmdFrameCapturedAsPsF == 0);

#ifdef _DEBUG
		{
			// Check the incoming video frame against our known state and translations
			DisplayModeSharedPtr dp = Translate(m_videoDisplayMode);
			assert(videoFrame->GetHeight() == dp->FrameHeight());
			assert(videoFrame->GetWidth() == dp->FrameWidth());
			assert(videoFrame->GetPixelFormat() == m_pixelFormat);

			VideoState vs;
			vs.displayMode = dp;
			vs.pixelFormat = Translate(m_pixelFormat);
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

		void* data;
		if (FAILED(videoFrame->GetBytes(&data)))
			throw std::runtime_error("Failed to get video frame bytes");

		VideoFrame vpVideoFrame(data);

		// Send to client, must be locked at this point
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			if (videoStateChanged)
				SendVideoStateCallback();

			m_callback->OnCaptureDeviceVideoFrame(vpVideoFrame);
		}
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
	if (ppv == NULL)
		return E_INVALIDARG;

	// Initialise the return result
	*ppv = NULL;

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
	m_pixelFormat = BMD_PIXEL_FORMAT_INVALID;
	m_videoDisplayMode = BMD_DISPLAY_MODE_INVALID;
	m_videoHasInputSource = false;
	m_videoEotf = BMD_EOTF_INVALID;
	m_videoColorSpace = BMD_COLOR_SPACE_INVALID;
	m_videoHasHdrData = false;

	ZeroMemory(&m_videoHdrData, sizeof(m_videoHdrData));
}


void BlackMagicDeckLinkCaptureDevice::SendVideoStateCallback()
{
	// WARNING: Needs to be locked

	assert(m_callback);

	const bool hasValidVideoState =
		(m_videoFrameSeen) &&
		(m_pixelFormat != BMD_PIXEL_FORMAT_INVALID) &&
		(m_videoDisplayMode != BMD_DISPLAY_MODE_INVALID) &&
		(m_videoHasInputSource) &&
		(m_videoEotf != BMD_EOTF_INVALID) &&
		(m_videoColorSpace != BMD_COLOR_SPACE_INVALID);

	// TODO: Move these checks into the HDRData struct and make that a class
	const bool hasValidHdrData =
		m_videoHasHdrData &&
		m_videoHdrData.displayPrimaryRedX > 0 &&
		m_videoHdrData.displayPrimaryRedY > 0 &&
		m_videoHdrData.displayPrimaryGreenX > 0 &&
		m_videoHdrData.displayPrimaryGreenY > 0 &&
		m_videoHdrData.displayPrimaryBlueX > 0 &&
		m_videoHdrData.displayPrimaryBlueY > 0 &&
		m_videoHdrData.whitePointX > 0 &&
		m_videoHdrData.whitePointY > 0 &&
		m_videoHdrData.masteringDisplayMaxLuminance > 0 &&
		m_videoHdrData.masteringDisplayMinLuminance > 0 &&
		m_videoHdrData.maxCll > 0 &&
		m_videoHdrData.maxFall > 0;

	//
	// Build and send reply
	//

	VideoStateComPtr videoState = new VideoState();
	if (!videoState)
		throw std::runtime_error("Failed to alloc VideoStateComPtr");

	//ZeroMemory(videoState.p, sizeof(videoState.p));

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
		assert(m_pixelFormat != BMD_PIXEL_FORMAT_INVALID);
		assert(m_videoDisplayMode != BMD_DISPLAY_MODE_INVALID);
		assert(m_videoEotf != BMD_EOTF_INVALID);
		assert(m_videoColorSpace != BMD_EOTF_INVALID);

		videoState->valid = true;
		videoState->pixelFormat = Translate(m_pixelFormat);
		videoState->displayMode = Translate(m_videoDisplayMode);
		videoState->eotf = TranslateEOTF(m_videoEotf);
		videoState->colorspace = Translate(
			(BMDColorspace)m_videoColorSpace,
			videoState->displayMode->FrameHeight());

		// Build a fresh copy of the HDR data if valid
		if (hasValidHdrData)
		{
			videoState->hdrData = std::make_shared<HDRData>();
			*(videoState->hdrData) = m_videoHdrData;
		}
	}

	m_callback->OnCaptureDeviceVideoStateChange(videoState);
}


void BlackMagicDeckLinkCaptureDevice::SendCardStateCallback()
{
	assert(m_deckLinkStatus);

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
		cardState->inputEncoding = TranslateEncoding((BMDDetectedVideoInputFormatFlags)intValue);
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
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		if (m_callback)
		{
			m_callback->OnCaptureDeviceCardStateChange(cardState);
		}
	}
}


void BlackMagicDeckLinkCaptureDevice::UpdateState(CaptureDeviceState state)
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		assert(state != m_state);  // Double state is not allowed
		m_state = state;

		if (m_callback)
		{
			m_callback->OnCaptureDeviceState(m_state);
		}
	}
}


//
// Internal helpers
//


void BlackMagicDeckLinkCaptureDevice::OnNotifyStatusChanged(BMDDeckLinkStatusID statusID)
{
	if (!m_deckLinkInput)
		return;

	switch (statusID)
	{
	// Device state changed
	case bmdDeckLinkStatusBusy:
		OnLinkStatusBusyChange();

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
		UpdateState(CaptureDeviceState::CAPTUREDEVICESTATE_READY);
	}
}
