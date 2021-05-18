/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <stdafx.h>

#include <VideoProcessorDlg.h>

#include "VideoProcessorApp.h"


BEGIN_MESSAGE_MAP(CVideoProcessorApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


CVideoProcessorApp videoProcessorApp;


BOOL CVideoProcessorApp::InitInstance()
{
	if (!CWinAppEx::InitInstance())
		throw std::runtime_error("Failed to initialize VideoProcessorApp");

	// COINIT_MULTITHREADED was used in the Blackmagic SDK examples,
	// using that without further investigation
	if(FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
		throw std::runtime_error("Failed to initialize com objects");

	CVideoProcessorDlg dlg;
	m_pMainWnd = &dlg;

	dlg.DoModal();

	CoUninitialize();

	return FALSE;
}
