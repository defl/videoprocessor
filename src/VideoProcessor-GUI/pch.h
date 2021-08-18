/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

 #pragma once


// Windows define magic
#define NOMINMAX
#define VC_EXTRALEAN                         // Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN                  // Exclude rarely-used stuff from Windows headers

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS   // some CString constructors will be explicit
#define _AFX_ALL_WARNINGS                    // turns off MFC's hiding of some common and often safely ignored warning messages


#include "targetver.h"


#ifdef _DEBUG
	// Visual Leak Detector
	// https://stackoverflow.com/questions/58439722/how-to-install-visual-leak-detector-vld-on-visual-studio-2019
	#include <vld.h>
#endif


// Common includes
#include <set>
#include <mutex>
#include <stdexcept>
#include <assert.h>
#include <afxwin.h>
#include <afxext.h>
#include <afxwinappex.h>
#include <streams.h>


// Helper macros for HRESULT functions
#define IF_NOT_S_OK(exp) if((exp) != S_OK)
#define IF_S_OK(exp) if((exp) == S_OK)
