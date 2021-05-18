/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <stdafx.h>

#include "cie.h"


bool CieValidColor(double color)
{
	return (color >= 0) && (color <= 1.0);
}


bool CieEquals(double color1, double color2)
{
	assert(CieValidColor(color1));
	assert(CieValidColor(color2));

	return fabs(color1 - color2) < 0.0001;
}
