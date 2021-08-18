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

	/**
	 * Constructor
	 *
	 * This is just a pointer to some data.
	 * If this data in any way, shape or form might be gone by the time it's used, you can use the
	 * sourceBuffer argument to have the VideoFrame constr/destr do ref management.
	 */
	VideoFrame() {}
	VideoFrame(
		const void* const data, uint64_t counter,
		timingclocktime_t timingTimestamp, IUnknown* sourceBuffer);
	VideoFrame(const VideoFrame&);

	~VideoFrame();

	// Get frame data
	// If you're wondering where the size of GetData() is, it can be found by querying
	// VideoState::BytesPerFrame() which you should get before this gets delivered.
	const void* const GetData() const { return m_data; }

	// Get counter, this is monotoncally increasing from the capture source
	uint64_t GetCounter() const { return m_counter; }

	// Timestamp set by the timing clock.
	timingclocktime_t GetTimingTimestamp() const { return m_timingTimestamp; }

	// Memory functions to hold onto the video buffer for longer
	void SourceBufferAddRef();
	void SourceBufferRelease();

	VideoFrame& operator= (const VideoFrame& videoFrame);

private:
	const void* m_data;
	uint64_t m_counter;
	timingclocktime_t m_timingTimestamp;
	IUnknown* m_sourceBuffer;
};
