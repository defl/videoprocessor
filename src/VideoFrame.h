/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once

#include <ITimingClock.h>


/**
 * Structure which represents a single video frame
 */
class VideoFrame
{
public:

	VideoFrame(const void* const data, timingclocktime_t timingTimestamp);
	VideoFrame(const VideoFrame&) = delete;

	// Get frame data
	// If you're wondering where the size of GetData() is, it can be found by querying
	// VideoState::BytesPerFrame() which you should get before this gets delivered.
	const void* const GetData() const { return m_data; }

	// Timestamp set by the timing clock.
	timingclocktime_t GetTimingTimestamp() const { return m_timingTimestamp; }

private:
	const void* const m_data;
	const timingclocktime_t m_timingTimestamp;
};
