/*******************************************************************************
**
** Photivo
**
** Copyright (C) 2011 Bernd Schoeler <brjohn@brother-john.net>
**
** This file is part of Photivo.
**
** Photivo is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License version 3
** as published by the Free Software Foundation.
**
** Photivo is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Photivo.  If not, see <http://www.gnu.org/licenses/>.
**
*******************************************************************************
Photivo'ized singleton version of EcWin7 1.0.2
  EcWin7 - Support library for integrating Windows 7 taskbar features
  into any Qt application
  Copyright (C) 2010 Emanuele Colombo
Find original sources in ReferenceMaterial/ecwin directory or under
http://dukto.googlecode.com/
*******************************************************************************/

#ifndef PTECWIN7_H
#define PTECWIN7_H

//==============================================================================

#include <QtGlobal>
#include <QWidget>

// Windows only data definitions
#ifdef Q_OS_WIN

#include <windows.h>
#include <unknwn.h>
#include <initguid.h>
#define CMIC_MASK_ASYNCOK SEE_MASK_ASYNCOK

//==============================================================================
// Structs types and enums definitions for Windows 7 taskbar

typedef enum THUMBBUTTONMASK
{
    THB_BITMAP = 0x1,
    THB_ICON = 0x2,
    THB_TOOLTIP	= 0x4,
    THB_FLAGS = 0x8
} THUMBBUTTONMASK;

typedef enum THUMBBUTTONFLAGS
{
    THBF_ENABLED = 0,
    THBF_DISABLED = 0x1,
    THBF_DISMISSONCLICK	= 0x2,
    THBF_NOBACKGROUND = 0x4,
    THBF_HIDDEN	= 0x8,
    THBF_NONINTERACTIVE	= 0x10
} THUMBBUTTONFLAGS;

typedef struct THUMBBUTTON
{
    THUMBBUTTONMASK dwMask;
    UINT iId;
    UINT iBitmap;
    HICON hIcon;
    WCHAR szTip[260];
    THUMBBUTTONFLAGS dwFlags;
} THUMBBUTTON;
typedef struct THUMBBUTTON *LPTHUMBBUTTON;

typedef enum TBPFLAG
{
    TBPF_NOPROGRESS = 0,
    TBPF_INDETERMINATE = 0x1,
    TBPF_NORMAL = 0x2,
    TBPF_ERROR = 0x4,
    TBPF_PAUSED = 0x8
} TBPFLAG;

typedef IUnknown *HIMAGELIST;

// Taskbar interface
DECLARE_INTERFACE_(ITaskbarList3,IUnknown)
{
    // IUnknown
    STDMETHOD(QueryInterface) (THIS_ REFIID riid,void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
    // ITaskbarList
    STDMETHOD(HrInit) (THIS) PURE;
    STDMETHOD(AddTab) (THIS_ HWND hwnd) PURE;
    STDMETHOD(DeleteTab) (THIS_ HWND hwnd) PURE;
    STDMETHOD(ActivateTab) (THIS_ HWND hwnd) PURE;
    STDMETHOD(SetActiveAlt) (THIS_ HWND hwnd) PURE;
    STDMETHOD (MarkFullscreenWindow) (THIS_ HWND hwnd, int fFullscreen) PURE;
    // ITaskbarList3
    STDMETHOD (SetProgressValue) (THIS_ HWND hwnd, ULONGLONG ullCompleted, ULONGLONG ullTotal) PURE;
    STDMETHOD (SetProgressState) (THIS_ HWND hwnd, TBPFLAG tbpFlags) PURE;
    STDMETHOD (RegisterTab) (THIS_ HWND hwndTab,HWND hwndMDI) PURE;
    STDMETHOD (UnregisterTab) (THIS_ HWND hwndTab) PURE;
    STDMETHOD (SetTabOrder) (THIS_ HWND hwndTab, HWND hwndInsertBefore) PURE;
    STDMETHOD (SetTabActive) (THIS_ HWND hwndTab, HWND hwndMDI, DWORD dwReserved) PURE;
    STDMETHOD (ThumbBarAddButtons) (THIS_ HWND hwnd, UINT cButtons, LPTHUMBBUTTON pButton) PURE;
    STDMETHOD (ThumbBarUpdateButtons) (THIS_ HWND hwnd, UINT cButtons, LPTHUMBBUTTON pButton) PURE;
    STDMETHOD (ThumbBarSetImageList) (THIS_ HWND hwnd, HIMAGELIST himl) PURE;
    STDMETHOD (SetOverlayIcon) (THIS_ HWND hwnd, HICON hIcon, LPCWSTR pszDescription) PURE;
    STDMETHOD (SetThumbnailTooltip) (THIS_ HWND hwnd, LPCWSTR pszTip) PURE;
    STDMETHOD (SetThumbnailClip) (THIS_ HWND hwnd, RECT *prcClip) PURE;
};
typedef ITaskbarList3 *LPITaskbarList3;

// Windows only data d#endifefinitions - END
#endif

// ********************************************************************
// ptEcWin7 class - Windows 7 taskbar handling for Qt and MinGW

class ptEcWin7 {
public:
  static void CreateInstance(QWidget* window);
  static ptEcWin7* GetInstance();
  static void DestroyInstance();

//-------------------------------------

  void init(HWND wid);
#ifdef Q_OS_WIN
  bool winEvent(MSG * message, long * result);
#endif
  // Overlay icon handling
  void setOverlayIcon(QString iconName, QString description);

  // Progress indicator handling
  enum ToolBarProgressState {
    NoProgress = 0,
    Indeterminate = 1,
    Normal = 2,
    Error = 4,
    Paused = 8
  };
  void setProgressValue(int value, int max);
  void setProgressState(ToolBarProgressState state);


private:
  static ptEcWin7* m_Instance;

  ptEcWin7();
  ~ptEcWin7();

  HWND mWindowId;
#ifdef Q_OS_WIN
  UINT mTaskbarMessageId;
  ITaskbarList3 *mTaskbar;
  HICON mOverlayIcon;
#endif
};

//==============================================================================

#endif // PTECWIN7_H
