/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <stdafx.h>

#include "Encoding.h"


const TCHAR* ToString(const Encoding encoding)
{
	switch (encoding)
	{
	case Encoding::UNKNOWN:
		return TEXT("UNKNOWN");

	case Encoding::YCbCr422:
		return TEXT("YCbCr 4:2:2");

	case Encoding::RGB444:
		return TEXT("RGB 4:4:4");
	}

	throw std::runtime_error("UNSPECIFIED Encoding");
}
