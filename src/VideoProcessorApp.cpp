/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <stdafx.h>

#include <winnt.h>

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

	// If the first command line arg is "/fullsreen" then we notify the app of this
	// https://docs.microsoft.com/en-us/cpp/c-runtime-library/argc-argv-wargv
	bool startFullScreen = false;
	int iNumOfArgs;
	LPWSTR* pArgs = CommandLineToArgvW(GetCommandLine(), &iNumOfArgs);
	if (iNumOfArgs >= 2 && wcscmp(pArgs[1], L"/fullscreen") == 0)
		startFullScreen = true;

	CVideoProcessorDlg dlg(startFullScreen);
	m_pMainWnd = &dlg;

	try
	{
		dlg.DoModal();
	}
	catch (std::runtime_error& e)
	{
		dlg.EndDialog(IDABORT);

		size_t size = strlen(e.what()) + 1;
		wchar_t* wtext = new wchar_t[size];
		size_t outSize;
		mbstowcs_s(&outSize, wtext, size, e.what(), size - 1);

		MessageBox(NULL, wtext, TEXT("Fatal error"), MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);

		delete[] wtext;
	}

	CoUninitialize();

	return FALSE;
}
