// gui.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "gui.h"
#include "myutils.h"
#include "resource.h"
#include <windowsx.h>

static bool g_TimerStarted = false;
HWND g_hwnd;
HDC g_hdc;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GUI, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

//	SetLogFileName("e:/tmp/gui.log");

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GUI));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GUI));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL; // MAKEINTRESOURCEW(IDC_GUI);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, w+100, h+200, nullptr, nullptr, hInstance, nullptr);
   ::SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE)&~WS_SIZEBOX);
   g_hwnd = hWnd;

   if (!hWnd)
   {
      return FALSE;
   }

   Init(hInstance);
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
		g_hwnd = hWnd;
		PAINTSTRUCT ps;
		g_hdc = BeginPaint(hWnd, &ps);
		PaintMainWindow();
		EndPaint(hWnd, &ps);
        }
        break;

	case WM_DESTROY:
        PostQuitMessage(0);
        break;

	case WM_CHAR:
		{
#define Ctrl(c)	((c) - 'A')
		if (wParam == VK_SPACE)
			OnSpaceBar();
		else if (wParam == VK_ESCAPE)
			{
			extern bool g_SelfPlay;
			if (g_SelfPlay)
				{
				g_SelfPlay = false;
				StartNewGame();
				}
			}
		else if (wParam == 'H' || wParam == 'h')
			HelpDialog();
		else if (wParam == 'U' || wParam == 'u')
			Undo();
		else if (wParam == 'R' || wParam == 'r')
			Resign();
		else if (wParam == 'N' || wParam == 'n')
			NewGame();
		else if (wParam == Ctrl('C'))
			OnCtrlC();
		else if (wParam == Ctrl('V'))
			OnCtrlV();
		else if (wParam == 'S')
			{
			void SelfPlay();
			SelfPlay();
			}
#undef Ctrl
		break;
		}

	case WM_MOUSEMOVE:
		{
		int x = GET_X_LPARAM(lParam) - WIN_OFFSET_X;
		int y = GET_Y_LPARAM(lParam) - WIN_OFFSET_Y;
		bool Dragging = ((wParam & MK_LBUTTON) != 0); 
		OnMouseMove(x, y, Dragging);
		break;
		}

	case WM_LBUTTONUP:
		{
		int x = GET_X_LPARAM(lParam) - WIN_OFFSET_X;
		int y = GET_Y_LPARAM(lParam) - WIN_OFFSET_Y;
		OnMouseLeftClick(x, y);
		break;
		}

	case WM_RBUTTONUP:
		{
		int x = GET_X_LPARAM(lParam) - WIN_OFFSET_X;
		int y = GET_Y_LPARAM(lParam) - WIN_OFFSET_Y;
		OnMouseRightClick(x, y);
		break;
		}

	default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

static const char *HelpText =
  "This is some help text\n"
  "Multiple lines";

// Message handler for help dialog.
INT_PTR CALLBACK HelpCallback(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
		SetWindowText(hDlg, "How to play");
		SetDlgItemText(hDlg, IDC_HELPTEXT, HelpText);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void HelpDialog()
	{
	DialogBox(hInst, MAKEINTRESOURCE(IDD_HELP), g_hwnd, HelpCallback);
	}
