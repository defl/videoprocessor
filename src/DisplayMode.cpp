/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <stdafx.h>

#include "DisplayMode.h"


DisplayMode::DisplayMode(
	unsigned int frameWidth,
	unsigned int frameHeight,
	unsigned int refreshRateMilliHz):
	m_frameWidth(frameWidth),
	m_frameHeight(frameHeight),
	m_refreshRateMilliHz(refreshRateMilliHz)
{
	if (frameWidth < 100 || frameWidth > 10000)
		throw std::runtime_error("resolutionX not valid");

	if (frameHeight < 100 || frameHeight > 10000)
		throw std::runtime_error("resolutionY not valid");

	if (refreshRateMilliHz < 10000 || refreshRateMilliHz > 300000)
		throw std::runtime_error("refreshRateMilliHz not valid");
}


timestamp_t DisplayMode::FrameDurationMs() const
{
	double interval = 1000000.0 / m_refreshRateMilliHz;
	return round(interval);
}


timestamp_t DisplayMode::FrameDuration100ns() const
{
	double interval = 10000000000.0 / m_refreshRateMilliHz;
	return round(interval);
}


CString DisplayMode::ToString() const
{
	CString s;

	// Resolutiion
	s.AppendFormat(_T("%ix%i"), m_frameWidth, m_frameHeight);

	// Mode name if well-known
	// https://en.wikipedia.org/wiki/List_of_common_resolutions
	if (m_frameWidth == 1280 && m_frameHeight == 720)
		s += TEXT(" (HD)");
	else if (m_frameWidth == 1920 && m_frameHeight == 1080)
		s += TEXT(" (Full HD)");
	else if (m_frameWidth == 2048 && m_frameHeight == 1556)
		s += TEXT(" (2K FullFrame)");
	else if (m_frameWidth == 2048 && m_frameHeight == 1080)
		s += TEXT(" (2K DCI)");
	else if (m_frameWidth == 3840 && m_frameHeight == 2160)
		s += TEXT(" (4K UHDTV)");
	else if (m_frameWidth == 4096 && m_frameHeight == 2160)
		s += TEXT(" (4K DCI)");

	s += TEXT(" @ ");

	// Refresh
	double x = RefreshRateHz();
	s.AppendFormat(_T("%.3f"), RefreshRateHz());

	return s;
}


bool DisplayMode::operator == (const DisplayMode& other) const
{
	return
		FrameWidth() == other.FrameWidth() &&
		FrameHeight() == other.FrameHeight() &&
		RefreshRateMilliHz() == other.RefreshRateMilliHz();
}


bool DisplayMode::operator != (const DisplayMode& other) const
{
	return !(*this == other);
}
