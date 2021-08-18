/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */


#include <pch.h>

#include <propvarutil.h>
#include <guiddef.h>

#include <guid.h>
#include "DirectShowRenderers.h"
#include <microsoft_directshow/video_renderers/DirectShowMPCVideoRenderer.h>


void DirectShowVideoRendererIds(std::vector<RendererId>& rendererIds)
{
	// https://docs.microsoft.com/en-us/windows/win32/directshow/using-the-filter-mapper

	IFilterMapper2* pMapper = nullptr;
	IEnumMoniker* pEnum = nullptr;
	HRESULT hr;

	hr = CoCreateInstance(CLSID_FilterMapper2,
		nullptr, CLSCTX_INPROC, IID_IFilterMapper2,
		(void**)&pMapper);

	if (FAILED(hr))
		throw std::runtime_error("Failed to instantiate the filter mapper");

	GUID arrayInTypes[2];
	arrayInTypes[0] = MEDIATYPE_Video;
	arrayInTypes[1] = GUID_NULL;

	hr = pMapper->EnumMatchingFilters(
		&pEnum,
		0,                  // Reserved.
		TRUE,               // Use exact match?
		MERIT_DO_NOT_USE,   // Minimum merit.
		TRUE,               // At least one input pin?
		1,                  // Number of major type/subtype pairs for input.
		arrayInTypes,       // Array of major type/subtype pairs for input.
		nullptr,               // Input medium.
		nullptr,               // Input pin category.
		FALSE,              // Must be a renderer?
		FALSE,              // At least one output pin?
		0,                  // Number of major type/subtype pairs for output.
		nullptr,               // Array of major type/subtype pairs for output.
		nullptr,               // Output medium.
		nullptr);              // Output pin category.

	// Enumerate the monikers.
	IMoniker* pMoniker;
	ULONG cFetched;
	while (pEnum->Next(1, &pMoniker, &cFetched) == S_OK)
	{
		IPropertyBag* pPropBag = nullptr;
		hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);

		if (SUCCEEDED(hr))
		{
			VARIANT nameVariant;
			VARIANT clsidVariant;
			VariantInit(&nameVariant);
			VariantInit(&clsidVariant);

			HRESULT nameHr = pPropBag->Read(L"FriendlyName", &nameVariant, 0);
			HRESULT clsidHr = pPropBag->Read(L"CLSID", &clsidVariant, 0);
			if (SUCCEEDED(nameHr) && SUCCEEDED(clsidHr))
			{
				CString name = nameVariant.bstrVal;

				// Filter by name.
				// Unforuntately renderes don't actually tell us if they can render so we need a hand-maintained list here.
				// VR = Abbreviation of Video Renderer
				if (((name.Find(TEXT("Video")) >= 0) && (name.Find(TEXT("Render")) >= 0)) ||
					(name.Find(TEXT("VR")) >= 0)
					)
				{
					CString rendererEntityName;
					rendererEntityName.Format(_T("DirectShow - %s"), name);

					RendererId rendererId;
					rendererId.name = rendererEntityName;

					hr = VariantToGUID(clsidVariant, &(rendererId.guid));
					if (FAILED(hr))
						throw std::runtime_error("Failed to convert veriant to GUID");

					rendererIds.push_back(rendererId);
				}
			}

			VariantClear(&nameVariant);
			VariantClear(&clsidVariant);

			pPropBag->Release();
		}
		pMoniker->Release();
	}

	pMapper->Release();
	pEnum->Release();
}
