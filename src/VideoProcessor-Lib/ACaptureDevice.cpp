/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <pch.h>

#include "ACaptureDevice.h"


CaptureDeviceCardState::~CaptureDeviceCardState()
{
	if (inputDisplayMode)
		inputDisplayMode.reset();
}


HRESULT	CaptureDeviceCardState::QueryInterface(REFIID iid, LPVOID* ppv)
{
	if (!ppv)
		return E_INVALIDARG;

	// Initialise the return result
	*ppv = nullptr;

	// Obtain the IUnknown interface and compare it the provided REFIID
	if (iid == IID_IUnknown)
	{
		*ppv = this;
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}


ULONG CaptureDeviceCardState::AddRef(void)
{
	return ++m_refCount;
}


ULONG CaptureDeviceCardState::Release(void)
{
	ULONG newRefValue = --m_refCount;
	if (newRefValue == 0)
		delete this;

	return newRefValue;
}


const TCHAR* ToString(const CaptureDeviceState eotf)
{
	switch (eotf)
	{
	case CaptureDeviceState::CAPTUREDEVICESTATE_READY:
		return TEXT("Ready");

	case CaptureDeviceState::CAPTUREDEVICESTATE_CAPTURING:
		return TEXT("Capturing");

	case CaptureDeviceState::CAPTUREDEVICESTATE_UNKNOWN:
		return TEXT("Unknown");

	case CaptureDeviceState::CAPTUREDEVICESTATE_STARTING:
		return TEXT("Starting");

	case CaptureDeviceState::CAPTUREDEVICESTATE_STOPPING:
		return TEXT("Stopping");

	case CaptureDeviceState::CAPTUREDEVICESTATE_FAILED:
		return TEXT("Failed");
	}

	throw std::runtime_error("CaptureDeviceState ToString() failed, value not recognized");
}
