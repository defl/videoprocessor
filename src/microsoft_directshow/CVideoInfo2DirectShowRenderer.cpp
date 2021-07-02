/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include "stdafx.h"

#include <dvdmedia.h>

#include <guid.h>
#include <microsoft_directshow/live_source_filter/CLiveSource.h>
#include <microsoft_directshow/DIrectShowTranslations.h>
#include <CNoopVideoFrameFormatter.h>
#include <ffmpeg/CFFMpegDecoderVideoFrameFormatter.h>

#include "CVideoInfo2DirectShowRenderer.h"


CVideoInfo2DirectShowRenderer::CVideoInfo2DirectShowRenderer(
	GUID rendererCLSID,
	IRendererCallback& callback,
	HWND videoHwnd,
	HWND eventHwnd,
	UINT eventMsg,
	ITimingClock* timingClock,
	VideoStateComPtr& videoState,
	RendererTimestamp timestamp,
	bool useFrameQueue,
	size_t frameQueueMaxSize,
	DXVA_NominalRange forceNominalRange,
	DXVA_VideoTransferFunction forceVideoTransferFunction,
	DXVA_VideoTransferMatrix forceVideoTransferMatrix,
	DXVA_VideoPrimaries forceVideoPrimaries):
	ADirectShowRenderer(
		rendererCLSID,
		callback,
		videoHwnd,
		eventHwnd,
		eventMsg,
		timingClock,
		videoState,
		timestamp,
		useFrameQueue,
		frameQueueMaxSize,
		true /* useHDRDdata */),
	m_forceNominalRange(forceNominalRange),
	m_forceVideoTransferFunction(forceVideoTransferFunction),
	m_forceVideoTransferMatrix(forceVideoTransferMatrix),
	m_forceVideoPrimaries(forceVideoPrimaries)
{
}


void CVideoInfo2DirectShowRenderer::MediaTypeGenerate()
{
	GUID mediaSubType;
	int bitCount;
	LONG heightMultiplier = 1;

	switch (m_videoState->pixelFormat)
	{

	// r210 to RGB48
	case PixelFormat::R210:

		mediaSubType = MEDIASUBTYPE_RGB48LE;
		bitCount = 48;
		heightMultiplier = -1;

		m_videoFramFormatter = new CFFMpegDecoderVideoFrameFormatter(
			AV_CODEC_ID_R210,
			AV_PIX_FMT_RGB48LE);
		break;

	// RGB 12-bit to RGB48
	case PixelFormat::R12B:

		mediaSubType = MEDIASUBTYPE_RGB48LE;
		bitCount = 48;
		heightMultiplier = -1;

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

	// Build pmt
	assert(!m_pmt.pbFormat);
	ZeroMemory(&m_pmt, sizeof(AM_MEDIA_TYPE));

	m_pmt.formattype = FORMAT_VIDEOINFO2;
	m_pmt.cbFormat = sizeof(VIDEOINFOHEADER2);
	m_pmt.majortype = MEDIATYPE_Video;
	m_pmt.subtype = mediaSubType;
	m_pmt.bFixedSizeSamples = TRUE;
	m_pmt.bTemporalCompression = FALSE;

	assert(!m_pmt.pbFormat);
	m_pmt.pbFormat = (BYTE*)CoTaskMemAlloc(sizeof(VIDEOINFOHEADER2));
	if (!m_pmt.pbFormat)
		throw std::runtime_error("Out of mem");

	VIDEOINFOHEADER2* pvi2 = (VIDEOINFOHEADER2*)m_pmt.pbFormat;
	ZeroMemory(pvi2, sizeof(VIDEOINFOHEADER2));

	// Populate bitmap info header
	// https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader

	pvi2->bmiHeader.biSizeImage = m_videoFramFormatter->GetOutFrameSize();
	pvi2->bmiHeader.biBitCount = bitCount;
	pvi2->bmiHeader.biCompression = m_pmt.subtype.Data1;
	pvi2->bmiHeader.biWidth = m_videoState->displayMode->FrameWidth();
	pvi2->bmiHeader.biHeight = ((long)m_videoState->displayMode->FrameHeight()) * heightMultiplier;
	pvi2->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pvi2->bmiHeader.biPlanes = 1;
	pvi2->bmiHeader.biClrImportant = 0;
	pvi2->bmiHeader.biClrUsed = 0;

	pvi2->AvgTimePerFrame = (REFERENCE_TIME)(UNITS / m_videoState->displayMode->RefreshRateHz());

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
		DXVA_NominalRange::DXVA_NominalRange_Unknown;  // = Let renderer guess

	pvi2->dwControlFlags += AMCONTROL_USED;
	pvi2->dwControlFlags += AMCONTROL_COLORINFO_PRESENT;

	m_pmt.lSampleSize = DIBSIZE(pvi2->bmiHeader);
}


void CVideoInfo2DirectShowRenderer::Connect()
{
	IEnumPins* pEnum = nullptr;
	IPin* pLiveSourceOutputPin = nullptr;
	IPin* pRendererInputPin = nullptr;

	if (FAILED(m_liveSource->EnumPins(&pEnum)))
		throw std::runtime_error("Failed to get livesource pin enumerator");

	if (pEnum->Next(1, &pLiveSourceOutputPin, nullptr) != S_OK)
	{
		pEnum->Release();

		throw std::runtime_error("Failed to run next on livesource pin");
	}

	pEnum->Release();
	pEnum = nullptr;

	if (FAILED(m_pRenderer->EnumPins(&pEnum)))
	{
		pLiveSourceOutputPin->Release();
		pRendererInputPin->Release();

		throw std::runtime_error("Failed to get livesource pin enumerator");
	}

	if (pEnum->Next(1, &pRendererInputPin, nullptr) != S_OK)
	{
		pLiveSourceOutputPin->Release();
		pRendererInputPin->Release();
		pEnum->Release();

		throw std::runtime_error("Failed to get livesource pin enumerator");
	}

	pEnum->Release();
	pEnum = nullptr;

	// Given that VideoInfo2 is not understood by any usable filter, we connect directly here
	// and if that fails it's bad luck
	if (FAILED(m_pGraph->ConnectDirect(pLiveSourceOutputPin, pRendererInputPin, &m_pmt)))
	{
		pLiveSourceOutputPin->Release();
		pRendererInputPin->Release();

		throw std::runtime_error("Failed to connect pins");
	}

	pLiveSourceOutputPin->Release();
	pRendererInputPin->Release();
}
