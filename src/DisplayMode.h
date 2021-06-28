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
#include <WallClock.h>  // TODO: Extract timestamp_t to somewhere we don't need the clock


/**
 * The display mode represents the metadata surrounding a stream of frames, describing
 * both the individual frames as well as their interplay.
 *
 * If you want to know more about this, the following URLs provide a great resource:
 *  - https://lurkertech.com/lg/fields/
 *  - https://docs.microsoft.com/en-us/windows/win32/medfound/video-interlacing
 *
 * TODO: Interleaved formats are not supported, only progressive.
 *
 * (It dees not store how the data is compressed, see the PixelFormat enum for that.)
 */
class DisplayMode
{
public:

	// Constructor, width and height in pixels/frame
	DisplayMode(
		unsigned int frameWidth,
		unsigned int frameHeight,
		unsigned int refreshRateMilliHz);

	// Amount of X-axis pixels per frame
	unsigned int FrameWidth() const { return m_frameWidth; }

	// Amount of Y-axis pixels per frame
	unsigned int FrameHeight() const { return m_frameHeight; }

	// Refresh rate as an approximated double
	double RefreshRateHz() const { return m_refreshRateMilliHz / 1000.0; }

	// Refresh rate in Milli Hz
	int RefreshRateMilliHz() const { return m_refreshRateMilliHz; }

	// timestamp ticks (100ns units) per frame
	// TODO: Not very happy with the naming, nor the implicit 100ns. Replace me with something much better.
	timestamp_t FrameDuration() const;

	// Return the mode as a human-understandable string
	CString ToString() const;

	bool operator == (const DisplayMode& other) const;
	bool operator != (const DisplayMode& other) const;

private:

	unsigned int m_frameWidth = 0;
	unsigned int m_frameHeight = 0;
	uint32_t m_refreshRateMilliHz = 0;
};


typedef std::shared_ptr<DisplayMode> DisplayModeSharedPtr;


//
// Well known modes
//

// 720p
#define DISPLAYMODE_720p_50    DisplayMode(1280, 720, 50000)
#define DISPLAYMODE_720p_59_94 DisplayMode(1280, 720, 59940)
#define DISPLAYMODE_720p_60    DisplayMode(1280, 720, 60000)

// 1080p
#define DISPLAYMODE_1080p_23_976 DisplayMode(1920, 1080, 23976)
#define DISPLAYMODE_1080p_24     DisplayMode(1920, 1080, 24000)
#define DISPLAYMODE_1080p_25     DisplayMode(1920, 1080, 25000)
#define DISPLAYMODE_1080p_29_97  DisplayMode(1920, 1080, 29970)
#define DISPLAYMODE_1080p_30     DisplayMode(1920, 1080, 30000)
#define DISPLAYMODE_1080p_47_95  DisplayMode(1920, 1080, 47950)
#define DISPLAYMODE_1080p_48     DisplayMode(1920, 1080, 48000)
#define DISPLAYMODE_1080p_50     DisplayMode(1920, 1080, 50000)
#define DISPLAYMODE_1080p_59_94  DisplayMode(1920, 1080, 59940)
#define DISPLAYMODE_1080p_60     DisplayMode(1920, 1080, 60000)

// 2K 35mm full frame size
#define DISPLAYMODE_2KFULLFRAME_23_976 DisplayMode(2048, 1556, 23976)
#define DISPLAYMODE_2KFULLFRAME_24     DisplayMode(2048, 1556, 24000)
#define DISPLAYMODE_2KFULLFRAME_25     DisplayMode(2048, 1556, 25000)

// 2K DCI native (https://en.wikipedia.org/wiki/2K_resolution)
#define DISPLAYMODE_2KDCI_23_976 DisplayMode(2048, 1080, 23976)
#define DISPLAYMODE_2KDCI_24     DisplayMode(2048, 1080, 24000)
#define DISPLAYMODE_2KDCI_25     DisplayMode(2048, 1080, 25000)
#define DISPLAYMODE_2KDCI_29_97  DisplayMode(2048, 1080, 29970)
#define DISPLAYMODE_2KDCI_30     DisplayMode(2048, 1080, 30000)
#define DISPLAYMODE_2KDCI_47_95  DisplayMode(2048, 1080, 47950)
#define DISPLAYMODE_2KDCI_48     DisplayMode(2048, 1080, 48000)
#define DISPLAYMODE_2KDCI_50     DisplayMode(2048, 1080, 50000)
#define DISPLAYMODE_2KDCI_59_94  DisplayMode(2048, 1080, 59940)
#define DISPLAYMODE_2KDCI_60     DisplayMode(2048, 1080, 60000)

// 4K UDHTV, 2160p (https://en.wikipedia.org/wiki/4K_resolution)
#define DISPLAYMODE_4K_23_976 DisplayMode(3840, 2160, 23976)
#define DISPLAYMODE_4K_24     DisplayMode(3840, 2160, 24000)
#define DISPLAYMODE_4K_25     DisplayMode(3840, 2160, 25000)
#define DISPLAYMODE_4K_29_97  DisplayMode(3840, 2160, 29970)
#define DISPLAYMODE_4K_30     DisplayMode(3840, 2160, 30000)
#define DISPLAYMODE_4K_47_95  DisplayMode(3840, 2160, 47950)
#define DISPLAYMODE_4K_48     DisplayMode(3840, 2160, 48000)
#define DISPLAYMODE_4K_50     DisplayMode(3840, 2160, 50000)
#define DISPLAYMODE_4K_59_94  DisplayMode(3840, 2160, 59940)
#define DISPLAYMODE_4K_60     DisplayMode(3840, 2160, 60000)

// 4K DCI native (aka full frame) (https://en.wikipedia.org/wiki/4K_resolution)
#define DISPLAYMODE_4KDCI_23_976 DisplayMode(4096, 2160, 23976)
#define DISPLAYMODE_4KDCI_24     DisplayMode(4096, 2160, 24000)
#define DISPLAYMODE_4KDCI_25     DisplayMode(4096, 2160, 25000)
#define DISPLAYMODE_4KDCI_29_97  DisplayMode(4096, 2160, 29970)
#define DISPLAYMODE_4KDCI_30     DisplayMode(4096, 2160, 30000)
#define DISPLAYMODE_4KDCI_47_95  DisplayMode(4096, 2160, 47950)
#define DISPLAYMODE_4KDCI_48     DisplayMode(4096, 2160, 48000)
#define DISPLAYMODE_4KDCI_50     DisplayMode(4096, 2160, 50000)
#define DISPLAYMODE_4KDCI_59_94  DisplayMode(4096, 2160, 59940)
#define DISPLAYMODE_4KDCI_60     DisplayMode(4096, 2160, 60000)
