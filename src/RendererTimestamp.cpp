/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <stdafx.h>

#include "RendererTimestamp.h"


const TCHAR* ToString(const RendererTimestamp rendererTimestamp)
{
	switch (rendererTimestamp)
	{
	case RendererTimestamp::RENDERER_TIMESTAMP_CLOCK_THEO:
		return TEXT("Clock+Theo");

	case RendererTimestamp::RENDERER_TIMESTAMP_CLOCK_CLOCK:
		return TEXT("Clock+Clock");

	case RendererTimestamp::RENDERER_TIMESTAMP_THEORETICAL:
		return TEXT("Theo");

	case RendererTimestamp::RENDERER_TIMESTAMP_NONE:
		return TEXT("None");
	}

	throw std::runtime_error("UNSPECIFIED RendererTimestamp");
}
