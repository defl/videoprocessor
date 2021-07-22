/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once

#include <afxstr.h>
#include <memory>
#include <WallClock.h>


/**
 * The display mode represents the metadata surrounding a stream of frames, describing
 * both the individual frames as well as their interplay.
 *
 * If you want to know more about this, the following URLs provide a great resource:
 *  - https://lurkertech.com/lg/fields/
 *  - https://docs.microsoft.com/en-us/windows/win32/medfound/video-interlacing
 *
 * (It does not store how the data is encoded, see the VideoFrameEncodning enum for that.)
 */
class DisplayMode
{
public:

	// Constructor, width and height in pixels/frame
	// - width & height in pixels
	// - timeScale in ticks per second
	// - frameDuration in ticks per frame
	DisplayMode(
		unsigned int frameWidth,
		unsigned int frameHeight,
		bool interlaced,  // Two field per frame expected
		unsigned int timeScale,
		unsigned int frameDuration);

	// Amount of X-axis pixels per frame
	unsigned int FrameWidth() const { return m_frameWidth; }

	// Amount of Y-axis pixels per frame
	unsigned int FrameHeight() const { return m_frameHeight; }

	// Ticks per second
	unsigned int TimeScale() const { return m_timeScale; }

	// Ticks per frame (expressed in TimeScale())
	unsigned int FrameDuration() const { return m_frameDuration; }

	// Frame contains interlaced data
	bool IsInterlaced() const { return m_interlaced; }


	// Refresh rate in Hz as double
	double RefreshRateHz() const;

	// Nanosecond per frame
	uint64_t NanosecondsPerFrame() const {}

	// Return the mode as a human-understandable string
	CString ToString() const;

	bool operator == (const DisplayMode& other) const;
	bool operator != (const DisplayMode& other) const;

private:

	const unsigned int m_frameWidth;
	const unsigned int m_frameHeight;
	const bool m_interlaced;
	const unsigned int m_timeScale;
	const unsigned int m_frameDuration;
};


typedef std::shared_ptr<DisplayMode> DisplayModeSharedPtr;
