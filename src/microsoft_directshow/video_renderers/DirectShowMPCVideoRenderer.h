/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include "DirectShowGenericVideoRenderer.h"


/**
 * MPC Video Renderer
 * https://github.com/Aleksoid1978/VideoRenderer/
 */
class DirectShowMPCVideoRenderer:
	public DirectShowVideoRenderer
{
public:

	DirectShowMPCVideoRenderer(
		IRendererCallback& callback,
		HWND videoHwnd,
		HWND eventHwnd,
		UINT eventMsg,
		ITimingClock* timingClock,
		DirectShowStartStopTimeMethod directShowStartStopTimeMethod,
		bool useFrameQueue,
		size_t frameQueueMaxSize,
		VideoConversionOverride videoConversionOverride,
		DXVA_NominalRange forceNominalRange,
		DXVA_VideoTransferFunction forceVideoTransferFunction,
		DXVA_VideoTransferMatrix forceVideoTransferMatrix,
		DXVA_VideoPrimaries forceVideoPrimaries);

	virtual ~DirectShowMPCVideoRenderer() {}

	// IVideoRenderer
	bool OnVideoState(VideoStateComPtr&) override;
	void OnSize() override;
	void OnPaint() override;

protected:

	// DirectShowVideoRenderer
	void WindowSetup() override;
	void RendererBuild() override;
	void MediaTypeGenerate() override;
	void RendererConnect() override;
	void LiveSourceBuildAndConnect() override;

private:

	const DXVA_NominalRange m_forceNominalRange;
	const DXVA_VideoTransferFunction m_forceVideoTransferFunction;
	const DXVA_VideoTransferMatrix m_forceVideoTransferMatrix;
	const DXVA_VideoPrimaries m_forceVideoPrimaries;
};
