/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include "ACaptureDevice.h"


/**
 * Callback handler for callbacks from the Device Discover
 */
class ICaptureDeviceDiscovererCallback
{
public:

	// Found a new capture device
	// WARNING: Most likely to be called from some internal capture card thread!
	virtual void OnCaptureDeviceFound(ACaptureDeviceComPtr& captureDevice) = 0;

	// Lost capture device previously announced through OnCaptureDeviceFound()
	// WARNING: Most likely to be called from some internal capture card thread!
	virtual void OnCaptureDeviceLost(ACaptureDeviceComPtr& captureDevice) = 0;
};


/**
 * This class provides an abstract interface for discovering devices.
 *
 * Does not auto-Start() you need to call Start()
 */
class ACaptureDeviceDiscoverer:
	public IUnknown
{
public:

	ACaptureDeviceDiscoverer(ICaptureDeviceDiscovererCallback& callback);
	virtual ~ACaptureDeviceDiscoverer();

	// Start automatic discovery
	virtual void Start() = 0;

	// Stop automatic discovery.
	// Upon stopping will send out CaptureDeviceLost() to callback
	virtual void Stop() = 0;

protected:

	ICaptureDeviceDiscovererCallback& m_callback;
};
