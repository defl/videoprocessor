/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include "ADirectShowRenderer.h"


/**
 * DirectShow renderer which supports FORMAT_VideoInfo2 connections
 */
class CVideoInfo2DirectShowRenderer :
	public ADirectShowRenderer
{
public:

	CVideoInfo2DirectShowRenderer(
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
		DXVA_VideoPrimaries forceVideoPrimaries);

	virtual ~CVideoInfo2DirectShowRenderer() {}

private:
	DXVA_NominalRange m_forceNominalRange;
	DXVA_VideoTransferFunction m_forceVideoTransferFunction;
	DXVA_VideoTransferMatrix m_forceVideoTransferMatrix;
	DXVA_VideoPrimaries m_forceVideoPrimaries;

	void MediaTypeGenerate() override;
	void Connect() override;
};
