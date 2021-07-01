/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


// TODO: Rename because it's not really the timestamp but more time range of the frame
enum RendererTimestamp
{
	// Use the given clock for start plus the theorized frame length for stop,
	// this has the advantage that you don't need a queued frame
	RENDERER_TIMESTAMP_CLOCK_THEO,

	// Use the given clock for start plus the start of the next frame for the stop
	// time.
	RENDERER_TIMESTAMP_CLOCK_CLOCK,

	// Theoretical timestamp based on frame duration
	RENDERER_TIMESTAMP_THEORETICAL,

	// Don't set timestamps (and use clock)
	RENDERER_TIMESTAMP_NONE
};


const TCHAR* ToString(const RendererTimestamp rendererTimestamp);
