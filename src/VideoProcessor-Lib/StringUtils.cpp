/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <pch.h>

#include "StringUtils.h"


wchar_t* ToString(const char* c)
{
	const size_t size = strlen(c) + 1;
	wchar_t* wText = new wchar_t[size];

	size_t outSize;
	mbstowcs_s(&outSize, wText, size, c, size - 1);

	return wText;
}
