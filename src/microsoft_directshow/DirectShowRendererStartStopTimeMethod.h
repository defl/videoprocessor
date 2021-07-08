/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


/**
 * This determines how frames are timestamped with a start and a stop time.
 * It is specific to DirectShow.
 */
enum DirectShowStartStopTimeMethod
{
	//
	// Both a start and a stop time
	//

	// Smart. Uses the next clock if available else will take theo.
	DS_SSTM_CLOCK_SMART,

	// Use the given clock for start plus the theorized frame length for stop
	DS_SSTM_CLOCK_THEO,

	// Use the given clock for start plus the start of the next frame for the stop time.
	DS_SSTM_CLOCK_CLOCK,

	// Theoretical timestamp based on frame duration
	DS_SSTM_THEO_THEO,

	//
	// Only a start time, no stop time set
	//

	DS_SSTM_CLOCK_NONE,


	DS_SSTM_THEO_NONE,

	//
	// No timestamps
	// This will mean to the DirectShow renderer that every timestamp is late.
	//

	DS_SSTM_NONE
};


const TCHAR* ToString(const DirectShowStartStopTimeMethod rendererTimestamp);
