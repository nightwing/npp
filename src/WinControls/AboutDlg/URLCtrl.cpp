#include "URLCtrl.h"

static BYTE XORMask[128] =
{
  0xff, 0xff, 0xff, 0xff,
  0xf9, 0xff, 0xff, 0xff, 
  0xf0, 0xff, 0xff, 0xff, 
  0xf0, 0xff, 0xff, 0xff, 
  0xf0, 0xff, 0xff, 0xff,
  0xf0, 0xff, 0xff, 0xff, 
  0xf0, 0x24, 0xff, 0xff, 
  0xf0, 0x00, 0x7f, 0xff, 
  0xc0, 0x00, 0x7f, 0xff,
  0x80, 0x00, 0x7f, 0xff, 
  0x80, 0x00, 0x7f, 0xff, 
  0x80, 0x00, 0x7f, 0xff, 
  0x80, 0x00, 0x7f, 0xff,
  0x80, 0x00, 0x7f, 0xff, 
  0xc0, 0x00, 0x7f, 0xff, 
  0xe0, 0x00, 0x7f, 0xff, 
  0xf0, 0x00, 0xff, 0xff,
  0xf0, 0x00, 0xff, 0xff, 
  0xf0, 0x00, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 
};

/* AND mask for hand cursor */
/* Generated by HexEdit */
static BYTE ANDMask[128] =
{
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 
  0x06, 0x00, 0x00, 0x00, 
  0x06, 0x00, 0x00, 0x00, 
  0x06, 0x00, 0x00, 0x00,
  0x06, 0x00, 0x00, 0x00, 
  0x06, 0x00, 0x00, 0x00, 
  0x06, 0xdb, 0x00, 0x00,
  0x06, 0xdb, 0x00, 0x00, 
  0x36, 0xdb, 0x00, 0x00, 
  0x36, 0xdb, 0x00, 0x00, 
  0x37, 0xff, 0x00, 0x00, 
  0x3f, 0xff, 0x00, 0x00,
  0x3f, 0xff, 0x00, 0x00, 
  0x1f, 0xff, 0x00, 0x00, 
  0x0f, 0xff, 0x00, 0x00, 
  0x07, 0xfe, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 
};

static COLORREF getParentDlgBkColor(HWND hWnd)
{
	COLORREF crRet = CLR_INVALID;
	if (hWnd && IsWindow(hWnd))
	{
		HWND hWndParent = GetParent(hWnd);
		if (hWndParent)
		{
			RECT rc;
			if (GetClientRect(hWndParent, &rc))
			{
				HDC hDC = GetDC(hWndParent);
				if (hDC)
				{
					HDC hdcMem = CreateCompatibleDC(hDC);
					if (hdcMem)
					{
						HBITMAP hBmp = CreateCompatibleBitmap(hDC,
						rc.right, rc.bottom);
						if (hBmp)
						{
							HGDIOBJ hOld = SelectObject(hdcMem, hBmp);
							if (hOld)
							{
								if (SendMessage(hWndParent,	WM_ERASEBKGND, (WPARAM)hdcMem, 0))
								{
									crRet = GetPixel(hdcMem, 0, 0);
								}
								SelectObject(hdcMem, hOld);
							}
							DeleteObject(hBmp);
						}
						DeleteDC(hdcMem);
					}
					ReleaseDC(hWndParent, hDC);
				}
			}
		}
	}
	return crRet;
}

void URLCtrl::create(HWND itemHandle, TCHAR * link, COLORREF linkColor)
{
	// turn on notify style
    ::SetWindowLongPtr(itemHandle, GWL_STYLE, ::GetWindowLongPtr(itemHandle, GWL_STYLE) | SS_NOTIFY);

	// set the URL text (not the display text)
	if (link)
		_URL = link;

	// set the hyperlink colour
    _linkColor = linkColor;

	// set the visited colour
	_visitedColor = RGB(128,0,128);

	// subclass the static control
    _oldproc = (WNDPROC)::SetWindowLongPtr(itemHandle, GWL_WNDPROC, (LONG)URLCtrlProc);

	// associate the URL structure with the static control
    ::SetWindowLongPtr(itemHandle, GWL_USERDATA, (LONG)this);

}
void URLCtrl::create(HWND itemHandle, int cmd, HWND msgDest)
{
	// turn on notify style
    ::SetWindowLongPtr(itemHandle, GWL_STYLE, ::GetWindowLongPtr(itemHandle, GWL_STYLE) | SS_NOTIFY);

	_cmdID = cmd;
	_msgDest = msgDest;

	// set the hyperlink colour
    _linkColor = RGB(0,0,255);

	// subclass the static control
    _oldproc = (WNDPROC)::SetWindowLongPtr(itemHandle, GWL_WNDPROC, (LONG)URLCtrlProc);

	// associate the URL structure with the static control
    ::SetWindowLongPtr(itemHandle, GWL_USERDATA, (LONG)this);
}
LRESULT URLCtrl::runProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) 
{
    switch(Message)
    {
	    // Free up the structure we allocated
	    case WM_NCDESTROY:
		    //HeapFree(GetProcessHeap(), 0, url);
		    break;
    	
	    // Paint the static control using our custom
	    // colours, and with an underline text style
	    case WM_PAINT:
        {
			DWORD dwStyle = ::GetWindowLongPtr(hwnd, GWL_STYLE);
		    DWORD dwDTStyle = DT_SINGLELINE;
    		
		    //Test if centered horizontally or vertically
		    if(dwStyle & SS_CENTER)	     dwDTStyle |= DT_CENTER;
		    if(dwStyle & SS_RIGHT)		 dwDTStyle |= DT_RIGHT;
		    if(dwStyle & SS_CENTERIMAGE) dwDTStyle |= DT_VCENTER;

	        RECT		rect;
            ::GetClientRect(hwnd, &rect);

            PAINTSTRUCT ps;
            HDC hdc = ::BeginPaint(hwnd, &ps);
    		
            ::SetTextColor(hdc, _linkColor);

            ::SetBkColor(hdc, getParentDlgBkColor(hwnd)); ///*::GetSysColor(COLOR_3DFACE)*/);
    		
		    // Create an underline font 
		    if(_hfUnderlined == 0)
		    {
			    // Get the default GUI font
			    LOGFONT lf;
                HFONT hf = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);

			    // Add UNDERLINE attribute
			    GetObject(hf, sizeof lf, &lf);
                lf.lfUnderline = TRUE;
    			
			    // Create a new font
                _hfUnderlined = ::CreateFontIndirect(&lf);
		    }
    		
		    HANDLE hOld = SelectObject(hdc, _hfUnderlined);

		    // Draw the text!
            TCHAR szWinText[MAX_PATH];
            ::GetWindowText(hwnd, szWinText, MAX_PATH);
            ::DrawText(hdc, szWinText, -1, &rect, dwDTStyle);
    		
            ::SelectObject(hdc, hOld);

            ::EndPaint(hwnd, &ps);

		    return 0;
        }

	    case WM_SETTEXT:
        {
            LRESULT ret = ::CallWindowProc(_oldproc, hwnd, Message, wParam, lParam);
            ::InvalidateRect(hwnd, 0, 0);
            return ret;
        }
	    // Provide a hand cursor when the mouse moves over us
	    //case WM_SETCURSOR:
        case WM_MOUSEMOVE:
        {
            if (_hCursor == 0)
                _hCursor = ::CreateCursor(::GetModuleHandle(0), 5, 2, 32, 32, XORMask, ANDMask);
    	
            SetCursor(_hCursor);
            return TRUE;
        }
		    
	    case WM_LBUTTONDOWN:
		    _clicking = true;
		    break;

	    case WM_LBUTTONUP:
		    if(_clicking)
		    {
			    _clicking = false;
				if (_cmdID)
				{
					::SendMessage(_msgDest?_msgDest:_hParent, WM_COMMAND, _cmdID, 0);
				}
				else
				{
			    _linkColor = _visitedColor;
    			
                ::InvalidateRect(hwnd, 0, 0);
                ::UpdateWindow(hwnd);

			    // Open a browser
			    if(_URL != TEXT(""))
			    {
                    ::ShellExecute(NULL, TEXT("open"), _URL.c_str(), NULL, NULL, SW_SHOWNORMAL);
			    }
			    else
			    {
                    TCHAR szWinText[MAX_PATH];
                    ::GetWindowText(hwnd, szWinText, MAX_PATH);
                    ::ShellExecute(NULL, TEXT("open"), szWinText, NULL, NULL, SW_SHOWNORMAL);
			    }
				}
		    }

		    break;
    		
	    // A standard static control returns HTTRANSPARENT here, which
	    // prevents us from receiving any mouse messages. So, return
	    // HTCLIENT instead.
	    case WM_NCHITTEST:
		    return HTCLIENT;
    }
    return ::CallWindowProc(_oldproc, hwnd, Message, wParam, lParam);
}
