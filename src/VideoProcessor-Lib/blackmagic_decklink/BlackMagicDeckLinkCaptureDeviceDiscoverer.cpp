/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <pch.h>

#include <algorithm>

#include "BlackMagicDeckLinkCaptureDeviceDiscoverer.h"
#include "BlackMagicDeckLinkCaptureDevice.h"


BlackMagicDeckLinkCaptureDeviceDiscoverer::BlackMagicDeckLinkCaptureDeviceDiscoverer(ICaptureDeviceDiscovererCallback& callback):
	ACaptureDeviceDiscoverer(callback)
{
}


BlackMagicDeckLinkCaptureDeviceDiscoverer::~BlackMagicDeckLinkCaptureDeviceDiscoverer()
{
	for (auto& kv : m_captureDeviceMap)
		kv.second.Release();
}


void BlackMagicDeckLinkCaptureDeviceDiscoverer::Start()
{
	if (m_deckLinkDiscovery)
		throw std::runtime_error("Discoverer already started");

	assert(m_captureDeviceMap.empty());

	if (CoCreateInstance(CLSID_CDeckLinkDiscovery, nullptr, CLSCTX_ALL, IID_IDeckLinkDiscovery, (void**)&m_deckLinkDiscovery) != S_OK)
		throw std::runtime_error("Unable to create IDeckLinkDiscovery interface object");

	if (!m_deckLinkDiscovery)
		throw std::runtime_error("DeckLinkDiscovery object was null");

	const HRESULT result = m_deckLinkDiscovery->InstallDeviceNotifications(this);
	if (result != S_OK)
		throw std::runtime_error("DeckLinkDiscovery InstallDeviceNotifications() failed");
}


void BlackMagicDeckLinkCaptureDeviceDiscoverer::Stop()
{
	if (!m_deckLinkDiscovery)
		throw std::runtime_error("Discoverer not running");

	// Stop notifications and end discovery
	m_deckLinkDiscovery->UninstallDeviceNotifications();
	m_deckLinkDiscovery.Release();

	// Send CaptureDeviceLost for all known
	for (auto& kv : m_captureDeviceMap)
	{
		m_callback.OnCaptureDeviceLost(kv.second);
		kv.second.Release();
	}

	m_captureDeviceMap.clear();
}


HRESULT	STDMETHODCALLTYPE BlackMagicDeckLinkCaptureDeviceDiscoverer::DeckLinkDeviceArrived(IDeckLink* deckLinkDevice)
{
	assert(m_deckLinkDiscovery);
	assert(m_captureDeviceMap.find(deckLinkDevice) == m_captureDeviceMap.end());

	// Build device
	ACaptureDeviceComPtr captureDevice;
	captureDevice.Attach(new BlackMagicDeckLinkCaptureDevice(deckLinkDevice));

	m_captureDeviceMap[deckLinkDevice] = captureDevice;

	m_callback.OnCaptureDeviceFound(captureDevice);

	return S_OK;
}


HRESULT	STDMETHODCALLTYPE BlackMagicDeckLinkCaptureDeviceDiscoverer::DeckLinkDeviceRemoved(IDeckLink* deckLinkDevice)
{
	assert(m_deckLinkDiscovery);
	assert(m_captureDeviceMap.find(deckLinkDevice) != m_captureDeviceMap.end());

	m_callback.OnCaptureDeviceLost(m_captureDeviceMap[deckLinkDevice]);

	m_captureDeviceMap.erase(deckLinkDevice);

	return S_OK;
}


HRESULT	BlackMagicDeckLinkCaptureDeviceDiscoverer::QueryInterface(REFIID iid, LPVOID* ppv)
{
	if (!ppv)
		return E_INVALIDARG;

	// Initialise the return result
	*ppv = nullptr;

	if (iid == IID_IUnknown || iid == IID_IDeckLinkInputCallback)
	{
		*ppv = this;
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}


ULONG BlackMagicDeckLinkCaptureDeviceDiscoverer::AddRef(void)
{
	return ++m_refCount;
}


ULONG BlackMagicDeckLinkCaptureDeviceDiscoverer::Release(void)
{
	ULONG newRefValue = --m_refCount;
	if (newRefValue == 0)
		delete this;

	return newRefValue;
}
