/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <stdafx.h>

#include <winnt.h>
extern "C" {
#include <libavutil/log.h>
}

#include <VideoProcessorDlg.h>

#include "VideoProcessorApp.h"


BEGIN_MESSAGE_MAP(CVideoProcessorApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


CVideoProcessorApp videoProcessorApp;


void av_log_callback(void* ptr, int level, const char* fmt, va_list vargs)
{
	vprintf(fmt, vargs);
}


BOOL CVideoProcessorApp::InitInstance()
{
	// Setup ffmpeg logging
	av_log_set_callback(av_log_callback);
#ifdef _DEBUG
	av_log_set_level(AV_LOG_TRACE);
#endif

	CVideoProcessorDlg dlg;
	m_pMainWnd = &dlg;

	try
	{
		if (!CWinAppEx::InitInstance())
			throw std::runtime_error("Failed to initialize VideoProcessorApp");

		// COINIT_MULTITHREADED was used in the Blackmagic SDK examples,
		// using that without further investigation
		if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
			throw std::runtime_error("Failed to initialize com objects");

		// Parse command line
		// https://docs.microsoft.com/en-us/cpp/c-runtime-library/argc-argv-wargv
		int iNumOfArgs;
		LPWSTR* pArgs = CommandLineToArgvW(GetCommandLine(), &iNumOfArgs);
		for (int i = 1; i < iNumOfArgs; i++)
		{
			// /fullscreen
			if (wcscmp(pArgs[i], L"/fullscreen") == 0)
				dlg.StartFullScreen();

			// /renderer "name"
			if (wcscmp(pArgs[i], L"/renderer") == 0 && (i + 1) < iNumOfArgs)
				dlg.DefaultRendererName(pArgs[i + 1]);
		}

		// Set set ourselves to high prio.
		if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS))
			throw std::runtime_error("Failed to set process priority");

		dlg.DoModal();
	}
	catch (std::runtime_error& e)
	{
		dlg.EndDialog(IDABORT);

		size_t size = strlen(e.what()) + 1;
		wchar_t* wtext = new wchar_t[size];
		size_t outSize;
		mbstowcs_s(&outSize, wtext, size, e.what(), size - 1);

		MessageBox(nullptr, wtext, TEXT("Fatal error"), MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);

		delete[] wtext;
	}

	CoUninitialize();

	return FALSE;
}
