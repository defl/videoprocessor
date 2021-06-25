/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <stdafx.h>

#include "IRenderer.h"


const TCHAR* ToString(const RendererTimestamp rendererTimestamp)
{
	switch (rendererTimestamp)
	{
	case RendererTimestamp::RENDERER_TIMESTAMP_CLOCK:
		return TEXT("Clock");

	case RendererTimestamp::RENDERER_TIMESTAMP_THEORETICAL:
		return TEXT("Theo");

	case RendererTimestamp::RENDERER_TIMESTAMP_NONE:
		return TEXT("None");
	}

	throw std::runtime_error("UNSPECIFIED RendererTimestamp");
}


const TCHAR* ToString(const RendererState rendererState)
{
	switch (rendererState)
	{
	case RendererState::RENDERSTATE_READY:
		return TEXT("Ready");

	case RendererState::RENDERSTATE_RENDERING:
		return TEXT("Rendering");

	case RendererState::RENDERSTATE_STOPPED:
		return TEXT("Stopped");

	case RendererState::RENDERSTATE_UNKNOWN:
		return TEXT("Unknown");

	case RendererState::RENDERSTATE_STOPPING:
		return TEXT("Stopping");

	case RendererState::RENDERSTATE_STARTING:
		return TEXT("Stopping");

	case RendererState::RENDERSTATE_FAILED:
		return TEXT("Failed");
	}

	throw std::runtime_error("UNSPECIFIED RendererState");
}
