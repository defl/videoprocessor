/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include "ALiveSourceVideoOutputPin.h"

#include "CLiveSource.h"


/**
 * This is an unbuffered video output pin, any presented frame will go directly to the
 * renderer. Problems guaranteed if you use timestamps as that will block the
 * renderer for the given duration
 */
class CUnbufferedLiveSourceVideoOutputPin:
	public ALiveSourceVideoOutputPin
{
public:

	CUnbufferedLiveSourceVideoOutputPin(
		CLiveSource* filter,
		CCritSec* pLock,
		HRESULT* phr);
	virtual ~CUnbufferedLiveSourceVideoOutputPin();

	// ALiveSourceVideoOutputPin
	HRESULT OnVideoFrame(VideoFrame&) override;
	void SetFrameQueueMaxSize(size_t) override;
	size_t GetFrameQueueSize() override { return 0; }
};
