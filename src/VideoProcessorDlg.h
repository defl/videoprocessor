/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include <set>
#include <atomic>

#include <blackmagic_decklink/BlackMagicDeckLinkCaptureDeviceDiscoverer.h>
#include <PixelValueRange.h>
#include <CCie1931Control.h>
#include <IRenderer.h>
#include <VideoFrame.h>
#include <FullscreenWindow.h>

#include "resource.h"


// Custom messages
#define WM_MESSAGE_CAPTURE_DEVICE_FOUND					(WM_APP + 1)
#define WM_MESSAGE_CAPTURE_DEVICE_LOST					(WM_APP + 2)
#define WM_MESSAGE_CAPTURE_DEVICE_STATE_CHANGE	        (WM_APP + 3)
#define WM_MESSAGE_CAPTURE_DEVICE_VIDEO_STATE_CHANGE	(WM_APP + 4)
#define WM_MESSAGE_CAPTURE_DEVICE_CARD_STATE_CHANGE		(WM_APP + 5)
#define WM_MESSAGE_DIRECTSHOW_NOTIFICATION              (WM_APP + 6)
#define WM_MESSAGE_RENDERER_STATE_CHANGE                (WM_APP + 7)


/**
 * Main UI is a simple dialog defined in VideoProcessor.rc
 */
class CVideoProcessorDlg:
	public CDialog,
	public ICaptureDeviceDiscovererCallback,
	public ICaptureDeviceCallback,
	public IRendererCallback
{
public:
	CVideoProcessorDlg(bool startFullscreen);
	virtual ~CVideoProcessorDlg();

	// Dialog Data
	enum { IDD = IDD_VIDEOPROCESSOR_DIALOG };

	// UI-related handlers
	afx_msg void OnCaptureDeviceSelected();
	afx_msg void OnCaptureInputSelected();
	afx_msg void OnRendererSelected();
	afx_msg void OnRendererTimestampSelected();
	afx_msg void OnRendererNominalRangeSelected();
	afx_msg void OnRendererTransferFunctionSelected();
	afx_msg void OnRendererTransferMatrixSelected();
	afx_msg void OnRendererPrimariesSelected();
	afx_msg void OnBnClickedFullScreenButton();
	afx_msg void OnBnClickedTimingClockFrameOffsetAutoCheck();
	afx_msg void OnBnClickedRendererRestart();
	afx_msg void OnBnClickedRendererReset();
	afx_msg void OnBnClickedRendererResetAutoCheck();
	afx_msg void OnBnClickedRendererVideoFrameUseQueueCheck();

	// Custom message handlers
	afx_msg LRESULT OnMessageCaptureDeviceFound(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMessageCaptureDeviceLost(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMessageCaptureDeviceStateChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMessageCaptureDeviceCardStateChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMessageCaptureDeviceVideoStateChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMessageDirectShowNotification(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMessageRendererStateChange(WPARAM wParam, LPARAM lParam);

	// Command handlers
	void OnCommandFullScreenToggle();
	void OnCommandFullScreenExit();

	// ICaptureDeviceDiscovererCallback
	void OnCaptureDeviceFound(ACaptureDeviceComPtr& captureDevice) override;
	void OnCaptureDeviceLost(ACaptureDeviceComPtr& captureDevice) override;

	// ICaptureDeviceCallback
	void OnCaptureDeviceState(CaptureDeviceState state) override;
	void OnCaptureDeviceCardStateChange(CaptureDeviceCardStateComPtr cardState) override;
	void OnCaptureDeviceVideoStateChange(VideoStateComPtr videoState) override;
	void OnCaptureDeviceVideoFrame(VideoFrame& videoFrame) override;

	// IRendererCallback
	void OnRendererState(RendererState rendererState) override;

protected:

	//
	// UI elements
	//

	// Capture device group
	CComboBox m_captureDeviceCombo;
	CComboBox m_captureInputCombo;
	CStatic m_captureDeviceStateText;
	CListBox m_captureDeviceOtherList;

	// Input group
	CStatic m_inputLockedText;
	CStatic m_inputDisplayModeText;
	CStatic m_inputEncodingText;
	CStatic m_inputBitDepthText;
	CStatic m_inputVideoFrameCountText;
	CStatic m_inputVideoFrameMissedText;

	// Captured video group
	CStatic m_videoValidText;
	CStatic m_videoDisplayModeText;
	CStatic m_videoPixelFormatText;
	CStatic m_videoEotfText;
	CStatic m_videoColorSpaceText;

	// Timing clock group
	CStatic m_timingClockDescriptionText;
	CEdit m_timingClockFrameOffsetEdit;
	CButton m_timingClockFrameOffsetAutoCheck;

	// ColorSpace group
	CCie1931Control m_colorspaceCie1931xy;

	// HDR group
	CStatic	m_hdrDml;
	CStatic	m_hdrMaxCll;
	CStatic	m_hdrMaxFall;

	// Latency group
	CStatic m_latencyCaptureText;
	CStatic m_latencyToVPRendererText;
	CStatic m_latencyToDSRendererText;

	// Renderer group
	CComboBox m_rendererCombo;
	CComboBox m_rendererTimestampCombo;
	CComboBox m_rendererNominalRangeCombo;
	CComboBox m_rendererTransferFunctionCombo;
	CComboBox m_rendererTransferMatrixCombo;
	CComboBox m_rendererPrimariesCombo;
	CButton m_rendererFullscreenButton;
	CButton m_rendererRestartButton;
	CButton m_rendererResetButton;
	CButton m_rendererResetAutoCheck;
	CStatic m_rendererStateText;
	CButton m_rendererVideoFrameUseQeueueCheck;
	CStatic m_rendererVideoFrameQueueSizeText;
	CEdit m_rendererVideoFrameQueueSizeMaxEdit;
	CStatic m_rendererDroppedFrameCountText;
	CStatic	m_rendererBox;  // This is the small renderer window

	CSize m_minDialogSize;
	HICON m_hIcon;
	HACCEL m_accelerator;

	bool m_rendererfullScreen = false;
	FullscreenWindow* m_fullScreenRenderWindow = nullptr;

	//
	// Program data
	//

	CComPtr<BlackMagicDeckLinkCaptureDeviceDiscoverer> m_blackMagicDeviceDiscoverer;

	std::set<ACaptureDeviceComPtr> m_captureDevices;
	CComPtr<ACaptureDevice>	m_captureDevice;
	CaptureInputId m_currentCaptureInputId = INVALID_CAPTURE_INPUT_ID;
	CaptureDeviceState m_captureDeviceState = CaptureDeviceState::CAPTUREDEVICESTATE_UNKNOWN;
	VideoStateComPtr m_captureDeviceVideoState = nullptr;

	IRenderer* m_renderer = nullptr;
	RendererState m_rendererState = RendererState::RENDERSTATE_UNKNOWN;

	std::atomic_bool m_deliverCaptureDataToRenderer = false;

	uint32_t m_timerSeconds = 0;

	// We often have to wait for devices to come back etc. Hence many functions can't complete
	// immediately. We solve this by setting a desired capture device and input and calling UpdateState()
	// at various points which will work towards our desired state
	CComPtr<ACaptureDevice>	m_desiredCaptureDevice = nullptr;
	CaptureInputId m_desiredCaptureInputId = INVALID_CAPTURE_INPUT_ID;
	//PixelValueRange m_desiredRendererPixelValueRange = PixelValueRange::PIXELVALUERANGE_UNKNOWN;  // = let render decide
	bool m_wantToRestartCapture = false;
	bool m_wantToRestartRenderer = false;
	bool m_wantToTerminate = false;
	void UpdateState();

	// Helpers
	void RefreshCaptureDeviceList();
	void RefreshInputConnectionCombo();
	void CaptureStart();
	void CaptureStop();
	void CaptureRemove();
	void CaptureGUIClear();
	void RenderStart();
	void RenderStop();
	void RenderRemove();
	void RenderGUIClear();
	void FullScreenWindowConstruct();
	void FullScreenWindowDestroy();
	HWND GetRenderWindow();
	size_t GetRendererVideoFrameQueueSizeMax();
	bool GetRendererVideoFrameUseQueue();
	int GetTimingClockFrameOffsetMs();
	void SetTimingClockFrameOffsetMs(int timingClockFrameOffsetMs);
	void UpdateTimingClockFrameOffset();
	void RebuildRendererCombo();
	void ClearRendererCombo();

	// CDialog
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;
	BOOL PreTranslateMessage(MSG* pMsg) override;
	void OnOK() override;
	void OnPaint();
	void OnSize(UINT nType, int cx, int cy);
	void OnSetFocus(CWnd* pOldWnd);
	void OnClose();
	void OnTimer(UINT_PTR nIDEvent);
	HCURSOR	OnQueryDragIcon();
	void OnGetMinMaxInfo(MINMAXINFO* minMaxInfo);

	DECLARE_MESSAGE_MAP()
};
