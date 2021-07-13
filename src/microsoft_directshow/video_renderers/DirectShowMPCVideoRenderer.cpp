/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include "stdafx.h"

#include <FilterInterfaces.h>

#include "DirectShowMPCVideoRenderer.h"
#include <guid.h>


DirectShowMPCVideoRenderer::DirectShowMPCVideoRenderer(
	IRendererCallback& callback,
	HWND videoHwnd,
	HWND eventHwnd,
	UINT eventMsg,
	ITimingClock* timingClock,
	DirectShowStartStopTimeMethod directShowStartStopTimeMethod,
	bool useFrameQueue,
	size_t frameQueueMaxSize):
	DirectShowGenericVideoRenderer(
		CLSID_MPCVR,
		callback,
		videoHwnd,
		eventHwnd,
		eventMsg,
		timingClock,
		directShowStartStopTimeMethod,
		useFrameQueue,
		frameQueueMaxSize)
{
	callback.OnRendererDetailString(TEXT("MPC Video Renderer DirectShow"));
}


void DirectShowMPCVideoRenderer::OnSize()
{
	DirectShowGenericVideoRenderer::OnSize();

	if (m_pRenderer)
	{
		if (CComQIPtr<IBasicVideo> pBV = m_pRenderer)
		{
			pBV->SetDefaultSourcePosition();
			pBV->SetDestinationPosition(0, 0, m_renderBoxWidth, m_renderBoxHeight);
		}

		if (CComQIPtr<IVideoWindow> pVW = m_pRenderer) {
			pVW->SetWindowPosition(0, 0, m_renderBoxWidth, m_renderBoxHeight);
		}
	}
}


void DirectShowMPCVideoRenderer::OnPaint()
{
	DirectShowGenericVideoRenderer::OnPaint();

	if (CComQIPtr<IExFilterConfig> pIExFilterConfig = m_pRenderer) {
		if (!SUCCEEDED(pIExFilterConfig->SetBool("cmd_redraw", true)))
			throw std::runtime_error("Failed to redraw MPC");
	}
}


void DirectShowMPCVideoRenderer::WindowSetup()
{
	if (FAILED(m_videoWindow->put_Owner((OAHWND)m_videoHwnd)))
		throw std::runtime_error("Failed to set owner of video window");

	if(CComQIPtr<IBasicVideo> pBV = m_pRenderer)
	{
		pBV->SetDefaultSourcePosition();
		pBV->SetDestinationPosition(0, 0, m_renderBoxWidth, m_renderBoxHeight);
	}

	if (CComQIPtr<IVideoWindow> pVW = m_pRenderer) {
		pVW->SetWindowPosition(0, 0, m_renderBoxWidth, m_renderBoxHeight);
	}
}
