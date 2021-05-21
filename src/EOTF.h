/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include <atlstr.h>


// Electical-Optical Transfer Function
enum EOTF
{
	UNKNOWN,

	// Traditional gamma, SDR luminance range (max typically 100 cd/m2)
	SDR,

	// Traditional gamma, HDR luminance range (max depends on device)
	HDR,

	// SMPTE ST 2084
	PQ,

	// Hybrid Log-Gamma, ITU-R BT.2100-0
	HLG
};


const TCHAR* ToString(const EOTF eotf);
