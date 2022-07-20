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
#include <FullscreenVideoWindow.h>
#include <VideoConversionOverride.h>
#include <WindowedVideoWindow.h>
#include <microsoft_directshow/DirectShowRendererStartStopTimeMethod.h>
#include <microsoft_directshow/DirectShowDefines.h>

#include "resource.h"


// Custom messages
#define WM_MESSAGE_CAPTURE_DEVICE_FOUND					(WM_APP + 1)
#define WM_MESSAGE_CAPTURE_DEVICE_LOST					(WM_APP + 2)
#define WM_MESSAGE_CAPTURE_DEVICE_STATE_CHANGE	        (WM_APP + 3)
#define WM_MESSAGE_CAPTURE_DEVICE_VIDEO_STATE_CHANGE	(WM_APP + 4)
#define WM_MESSAGE_CAPTURE_DEVICE_CARD_STATE_CHANGE		(WM_APP + 5)
#define WM_MESSAGE_CAPTURE_DEVICE_ERROR					(WM_APP + 6)
#define WM_MESSAGE_DIRECTSHOW_NOTIFICATION              (WM_APP + 7)
#define WM_MESSAGE_RENDERER_STATE_CHANGE                (WM_APP + 8)
#define WM_MESSAGE_RENDERER_DETAIL_STRING               (WM_APP + 9)


enum class HdrColorspaceOptions
{
	HDR_COLORSPACE_FOLLOW_INPUT,
	HDR_COLORSPACE_FOLLOW_INPUT_LLDV,
	HDR_COLORSPACE_FOLLOW_CONTAINER,
	HDR_COLORSPACE_BT2020,
	HDR_COLORSPACE_P3,
	HDR_COLORSPACE_REC709
};


enum class HdrLuminanceOptions
{
	HDR_LUMINANCE_FOLLOW_INPUT,
	HDR_LUMINANCE_FOLLOW_INPUT_LLDV,
	HDR_LUMINANCE_USER,
};


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
	CVideoProcessorDlg();
	virtual ~CVideoProcessorDlg();

	// Dialog Data
	enum { IDD = IDD_VIDEOPROCESSOR_DIALOG };

	// Option handlers
	void StartFullScreen();
	void DefaultRendererName(const CString&);
	void StartFrameOffsetAuto();
	void StartFrameOffset(const CString&);
	void DefaultVideoConversionOverride(VideoConversionOverride);
	void DefaultContainerColorSpace(ColorSpace);
	void DefaultHDRColorSpace(HdrColorspaceOptions);
	void DefaultHDRLuminance(HdrLuminanceOptions);
	void DefaultRendererStartStopTimeMethod(DirectShowStartStopTimeMethod);
	void DefaultRendererNominalRange(DXVA_NominalRange);
	void DefaultRendererTransferFunction(DXVA_VideoTransferFunction);
	void DefaultRendererTransferMatrix(DXVA_VideoTransferMatrix);
	void DefaultRendererPrimaries(DXVA_VideoPrimaries);


	// UI-related handlers
	afx_msg void OnCaptureDeviceSelected();
	afx_msg void OnCaptureInputSelected();
	afx_msg void OnBnClickedCaptureRestart();
	afx_msg void OnBnClickedTimingClockFrameOffsetAutoCheck();
	afx_msg void OnColorSpaceContainerSelected();
	afx_msg void OnHdrColorSpaceSelected();
	afx_msg void OnHdrLuminanceSelected();
	afx_msg void OnRendererSelected();
	afx_msg void OnBnClickedRendererRestart();
	afx_msg void OnRendererVideoConversionSelected();
	afx_msg void OnBnClickedRendererVideoFrameUseQueueCheck();
	afx_msg void OnBnClickedRendererReset();
	afx_msg void OnBnClickedRendererResetAutoCheck();
	afx_msg void OnRendererDirectShowStartStopTimeMethodSelected();
	afx_msg void OnRendererDirectShowNominalRangeSelected();
	afx_msg void OnRendererDirectShowTransferFunctionSelected();
	afx_msg void OnRendererDirectShowTransferMatrixSelected();
	afx_msg void OnRendererDirectShowPrimariesSelected();
	afx_msg void OnBnClickedRendererFullScreenCheck();

	// Custom message handlers
	afx_msg LRESULT OnMessageCaptureDeviceFound(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMessageCaptureDeviceLost(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMessageCaptureDeviceStateChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMessageCaptureDeviceCardStateChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMessageCaptureDeviceVideoStateChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMessageCaptureDeviceError(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMessageDirectShowNotification(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMessageRendererStateChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMessageRendererDetailString(WPARAM wParam, LPARAM lParam);

	// Command handlers
	void OnCommandFullScreenToggle();
	void OnCommandFullScreenExit();
	void OnCommandRendererReset();

	// ICaptureDeviceDiscovererCallback
	void OnCaptureDeviceFound(ACaptureDeviceComPtr& captureDevice) override;
	void OnCaptureDeviceLost(ACaptureDeviceComPtr& captureDevice) override;

	// ICaptureDeviceCallback
	void OnCaptureDeviceState(CaptureDeviceState state) override;
	void OnCaptureDeviceCardStateChange(CaptureDeviceCardStateComPtr cardState) override;
	void OnCaptureDeviceVideoStateChange(VideoStateComPtr videoState) override;
	void OnCaptureDeviceVideoFrame(VideoFrame& videoFrame) override;
	void OnCaptureDeviceError(const CString& error) override;

	// IRendererCallback
	void OnRendererState(RendererState rendererState) override;
	void OnRendererDetailString(const CString& details) override;

protected:

	//
	// UI elements
	//

	// Capture device group
	CComboBox m_captureDeviceCombo;
	CComboBox m_captureInputCombo;
	CStatic m_captureDeviceStateText;
	CButton m_captureDeviceRestartButton;
	CListBox m_captureDeviceOtherList;

	// Input group
	CStatic m_inputLockedText;
	CStatic m_inputDisplayModeText;
	CStatic m_inputEncodingText;
	CStatic m_inputBitDepthText;
	CStatic m_inputVideoFrameCountText;
	CStatic m_inputVideoFrameMissedText;
	CStatic m_inputLatencyMsText;

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

	// Colorspace group
	CComboBox m_colorspaceContainerCombo;

	// HDR colorSpace group
	CEdit m_hdrColorspaceREdit;
	CEdit m_hdrColorspaceGEdit;
	CEdit m_hdrColorspaceBEdit;
	CEdit m_hdrColorspaceWPEdit;
	CComboBox m_hdrColorspaceCombo;

	// HDR Lumiance group
	CEdit m_hdrLuminanceMaxCll;
	CEdit m_hdrLuminanceMaxFall;
	CEdit m_hdrLuminanceMasterMin;
	CEdit m_hdrLuminanceMasterMax;
	CComboBox m_hdrLuminanceCombo;

	// CIE1931 graph
	CCie1931Control m_colorspaceCie1931xy;

	// Renderer group
	CComboBox m_rendererCombo;
	CStatic m_rendererDetailStringStatic;
	CButton m_rendererRestartButton;
	CStatic m_rendererStateText;
	WindowedVideoWindow	m_windowedVideoWindow;

	// Renderer Queue group
	CButton m_rendererVideoFrameUseQeueueCheck;
	CStatic m_rendererVideoFrameQueueSizeText;
	CEdit m_rendererVideoFrameQueueSizeMaxEdit;
	CStatic m_rendererDroppedFrameCountText;
	CButton m_rendererResetButton;
	CButton m_rendererResetAutoCheck;

	// Renderer Video conversion group
	CComboBox m_rendererVideoConversionCombo;

	// Renderer DirectShow override group
	CComboBox m_rendererDirectShowStartStopTimeMethodCombo;
	CComboBox m_rendererNominalRangeCombo;
	CComboBox m_rendererTransferFunctionCombo;
	CComboBox m_rendererTransferMatrixCombo;
	CComboBox m_rendererPrimariesCombo;

	// Renderer latency (ms) group
	CStatic m_rendererLatencyToVPText;
	CStatic m_rendererLatencyToDSText;

	// Renderer output group
	CButton m_rendererFullscreenCheck;

	CSize m_minDialogSize;
	HICON m_hIcon;
	HACCEL m_accelerator;

	FullscreenVideoWindow* m_fullScreenVideoWindow = nullptr;

	//
	// Program data
	//

	CComPtr<BlackMagicDeckLinkCaptureDeviceDiscoverer> m_blackMagicDeviceDiscoverer;

	std::set<ACaptureDeviceComPtr> m_captureDevices;
	CComPtr<ACaptureDevice>	m_captureDevice;
	CaptureInputId m_currentCaptureInputId = INVALID_CAPTURE_INPUT_ID;
	CaptureDeviceState m_captureDeviceState = CaptureDeviceState::CAPTUREDEVICESTATE_UNKNOWN;
	VideoStateComPtr m_captureDeviceVideoState = nullptr;  // This is what we get from the capture card

	VideoStateComPtr m_builtVideoState = nullptr;  // This is what we make of it

	// Startup options
	bool m_rendererFullScreenStart = false;
	CString m_defaultRendererName;
	bool m_frameOffsetAutoStart = false;
	CString m_defaultFrameOffset = TEXT("90");
	VideoConversionOverride m_defaultVideoConversionOverride = VideoConversionOverride::VIDEOCONVERSION_NONE;
	ColorSpace m_defaultContainerColorSpace = ColorSpace::UNKNOWN;
	HdrColorspaceOptions m_defaultHDRColorSpaceOption = HdrColorspaceOptions::HDR_COLORSPACE_FOLLOW_INPUT;
	HdrLuminanceOptions m_defaultHDRLuminanceOption = HdrLuminanceOptions::HDR_LUMINANCE_FOLLOW_INPUT;
	DirectShowStartStopTimeMethod m_defaultDSSSTimeMethod = DirectShowStartStopTimeMethod::DS_SSTM_CLOCK_SMART;
	DXVA_NominalRange m_defaultNominalRange = DXVA_NominalRange::DXVA_NominalRange_Unknown;  // Auto
	DXVA_VideoTransferFunction m_defaultTransferFunction = DXVA_VideoTransferFunction::DXVA_VideoTransFunc_Unknown;  // Auto
	DXVA_VideoTransferMatrix m_defaultTransferMatrix = DXVA_VideoTransferMatrix::DXVA_VideoTransferMatrix_Unknown;  // Auto
	DXVA_VideoPrimaries m_defaultPrimaries = DXVA_VideoPrimaries::DXVA_VideoPrimaries_Unknown;  // Auto


	IVideoRenderer* m_videoRenderer = nullptr;
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
	void FullScreenVideoWindowConstruct();
	void FullScreenVideoWindowDestroy();
	HWND GetRenderWindow();
	size_t GetRendererVideoFrameQueueSizeMax();
	bool GetRendererVideoFrameUseQueue();
	double GetWindowTextAsDouble(CEdit&);
	int GetTimingClockFrameOffsetMs();
	void SetTimingClockFrameOffsetMs(int timingClockFrameOffsetMs);
	void UpdateTimingClockFrameOffset();
	void RebuildRendererCombo();
	void ClearRendererCombo();

	bool BuildPushVideoState();
	void BuildPushRestartVideoState();

#define FatalError(error) (_FatalError(__LINE__, __FUNCTION__, error))
	void _FatalError(int line, const std::string& functionName, const CString& error);

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
