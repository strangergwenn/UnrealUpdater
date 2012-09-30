/**
 *  This work is distributed under the General Public License,
 *	see LICENSE for details
 *
 *  @author Gwennaël ARBONA
 **/


#include "windows-specific.h"


/*----------------------------------------------
	 Private data
----------------------------------------------*/

typedef struct _DWM_BLURBEHIND
{
	DWORD dwFlags;
	BOOL fEnable;
	HRGN hRgnBlur;
	BOOL fTransitionOnMaximized;
} DWM_BLURBEHIND, *PDWM_BLURBEHIND;

typedef struct _MARGINS
{
	int cxLeftWidth;
	int cxRightWidth;
	int cyTopHeight;
	int cyBottomHeight;
} MARGINS, *PMARGINS;

static HMODULE gShell;


/*----------------------------------------------
	 Private functions
----------------------------------------------*/

extern "C"
{
	typedef HRESULT (WINAPI *t_DwmEnableBlurBehindWindow)(HWND win, const DWM_BLURBEHIND* pBlurBehind);
	typedef HRESULT (WINAPI *t_DwmExtendFrameIntoClientArea)(HWND win, const MARGINS *pMarInset);
}

void DwmExtendFrameIntoClientArea(HWND win, const MARGINS *pMarInset)
{
	t_DwmExtendFrameIntoClientArea set_window_frame_into_client_area =
			reinterpret_cast<t_DwmExtendFrameIntoClientArea>(GetProcAddress(gShell, "DwmExtendFrameIntoClientArea"));
	set_window_frame_into_client_area(win, pMarInset);
}

void DwmEnableBlurBehindWindow(HWND win, const DWM_BLURBEHIND* pBlurBehind)
{
	t_DwmEnableBlurBehindWindow set_window_blur =
			reinterpret_cast<t_DwmEnableBlurBehindWindow>(GetProcAddress(gShell, "DwmEnableBlurBehindWindow"));
	set_window_blur(win, pBlurBehind);
}


/*----------------------------------------------
	 Public functions
----------------------------------------------*/

/*--- Call the necessary Aero setups in Windows API ---*/
void SetupAeroEffects(QWidget* widget)
{
	// Data
	HWND win = widget->winId();
	MARGINS margins = {-1};
	DWM_BLURBEHIND bb = {0};

	// Setup
	bb.dwFlags = 1;
	bb.fEnable = true;
	bb.hRgnBlur = NULL;
	widget->setAttribute(Qt::WA_TranslucentBackground, true);
	widget->setAttribute(Qt::WA_NoSystemBackground, true);

	// Windows API calls
	gShell = LoadLibrary(L"dwmapi.dll");
	if (gShell)
	{
		DwmEnableBlurBehindWindow(win, &bb);
		DwmExtendFrameIntoClientArea(widget->winId(), &margins);
	}
	FreeLibrary(gShell);
}
