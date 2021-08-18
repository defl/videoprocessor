/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once

#include <string>

typedef long long CaptureInputId;


#define INVALID_CAPTURE_INPUT_ID (CaptureInputId)-1


enum class CaptureInputType
{
	HDMI,
	SDI_OPTICAL,
	SDI_ELECTRICAL,
	COMPONENT,
	COMPOSITE,
	S_VIDEO
};


/**
 * Simple data object which represent an input to a capture device
 */
class CaptureInput
{
public:
	CaptureInput(CaptureInputId c_id, CaptureInputType c_type, CString&& c_name);

	// This is a unique ID with which the input can be identified.
	CaptureInputId id;

	// The type of capture input
	CaptureInputType type;

	// Name of the input
	CString name;
};
