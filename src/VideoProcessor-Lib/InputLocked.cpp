/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <pch.h>

#include "InputLocked.h"


const TCHAR* ToString(const InputLocked inputLocked)
{
	switch (inputLocked)
	{
	case InputLocked::UNKNOWN:
		return TEXT("UNKNOWN");

	case InputLocked::YES:
		return TEXT("Yes");

	case InputLocked::NO:
		return TEXT("No");
	}

	throw std::runtime_error("InputLocked ToString() failed, value not recognized");
}
