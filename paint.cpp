#include "stdafx.h"
#include "gui.h"
#include <assert.h>
#include <time.h>

static HDC myhdc;
static BOOL Ok;

static GameState GS;

static const COLORREF BoardBorderColor = GRAY;
static const COLORREF BoardInnerColor = LIGHTGRAY;

static const COLORREF TriangleColor1 = SLATEBLUE;
static const COLORREF TriangleColor2 = GRAY;
static const COLORREF TriangleBorderColor = GRAY;

static const COLORREF WhiteCheckerBorderColor = GRAY;
static const COLORREF WhiteDiceBorderColor = GRAY;

static const COLORREF BlackCheckerBorderColor = GRAY;
static const COLORREF BlackDiceBorderColor = GRAY;

static const COLORREF WhiteCheckerFillColor = VERYLIGHTYELLOW;
static const COLORREF WhiteDiceFillColor = VERYLIGHTYELLOW;

static const COLORREF BlackCheckerFillColor = DARKGRAY;
static const COLORREF BlackDiceFillColor = DARKGRAY;

static const COLORREF WhiteDiceDotFillColor = DARKGRAY;
static const COLORREF BlackDiceDotFillColor = VERYLIGHTYELLOW;

static const COLORREF WhiteDiceDotBorderColor = DARKGRAY;
static const COLORREF BlackDiceDotBorderColor = VERYLIGHTYELLOW;

static const COLORREF CubeBodyColor = LIGHTGRAY;
static const COLORREF CubeBorderColor = DARKGRAY;

static const COLORREF InfoFillColor = VERYLIGHTBLUE; // LIGHTGRAY;
static const COLORREF InfoBorderColor = GRAY;
static const COLORREF InfoTextColor = DARKGRAY;
static const COLORREF InfoErrColor = BRICK;

static const COLORREF ButtonFillColor = VERYLIGHTBLUE;
static const COLORREF ButtonBorderColor = GRAY;
static const COLORREF ButtonTextColor = GRAY; // DARKGRAY;

static const COLORREF HelpTextColor = LIGHTGRAY;

static const int DIE_SIZE = 40;
static const int DIE_DOT_RADIUS = 4;
static const int INFO_BORDER_WIDTH = 4;
static const int BUTTON_HEIGHT = 40;
static const int BUTTON_WIDTH = 120;
static const int BUTTON_VSPACE = 10;
static const int BUTTON_HSPACE = 10;
static const int BUTTON_BORDER_WIDTH = 4;

static string g_CurrentInfoStr;
static bool g_InfoStrErr;

void SetInfoStr(const string &s, bool Err)
	{
	g_CurrentInfoStr = s;
	g_InfoStrErr = Err;
	}

void SetDC()
	{
	myhdc = GetDC(g_hwnd);
	Ok = OffsetWindowOrgEx(myhdc, -WIN_OFFSET_X, -WIN_OFFSET_Y, NULL);
	assert(Ok);
	}

void Init(HINSTANCE hInst)
	{
	ResetRand(unsigned(time(0)));
	}

void WriteStatusText(const string &Text)
	{
	SetDC();
	int left = 20;
	int top = h + 100;
	int right = h + w;
	int bottom = top + 40;
	RECT rect;
    SetRect(&rect, left, top, right, bottom);

	int OldBkMode = SetBkMode(myhdc, OPAQUE);
    SetTextColor(myhdc, BLACK);
	string s = Text + "               ";
    Ok = DrawText(myhdc, s.c_str(), -1, &rect, DT_NOCLIP);
	assert(Ok);
	SetBkMode(myhdc, OldBkMode);
	ReleaseDC(g_hwnd, myhdc);
	myhdc = NULL;
	}

static void DrawPointNr(int Pos, int x, int y)
	{
	int left = x;
	int top = y;
	int right = x + 10;
	int bottom = y + 10;
	RECT rect;
    SetRect(&rect, left, top, right, bottom);

	int OldBkMode = SetBkMode(myhdc, TRANSPARENT);
    SetTextColor(myhdc, LIGHTGRAY);
	string s;
	Ps(s, "%d", Pos);
	UINT OldAlign = SetTextAlign(myhdc, TA_CENTER);
    Ok = DrawText(myhdc, s.c_str(), -1, &rect, DT_NOCLIP);
	SetTextAlign(myhdc, OldAlign);
	assert(Ok);
	SetBkMode(myhdc, OldBkMode);
	}

static void GetTriangleRect(int Pos, int &left, int &top,
  int &right, int &bottom)
	{
	int width = 3;
	int height = 20;
	int i, startx, starty;
	if (Pos >= 13 && Pos <= 18) // top-left
		{
		i = Pos - 13;
		startx = 5 + i*3;
		starty = 2;
		}
	else if (Pos >= 7 && Pos <= 12) // bottom-left
		{
		i = 12 - Pos;
		startx = 5 + i*3;
		starty = 28;
		}
	else if (Pos >= 19 && Pos <= 24) // top-right
		{
		i = Pos - 19;
		startx = 27 + i*3;
		starty = 2;
		}
	else if (Pos >= 1 && Pos <= 6) // bottom-right
		{
		i = 6 - Pos;
		startx = 27 + i*3;
		starty = 28;
		}
	else
		assert(false);

	left = int(w*p*startx);
	right = int(w*p*(startx + width));
	top = int(h*p*starty);
	bottom = int(h*p*(starty + height));
	}

//Ok = DrawRect(w*p*5, h*p*2, w*p*18, h*p*46); //left board
//Ok = DrawRect(w*p*27, h*p*2, w*p*18, h*p*46); //right board
static void GetBarRect(int &left, int &top, int &right, int &bottom)
	{
	left = int(w*p*23);
	top = int(h*p*2);
	right = int(w*p*27);
	bottom = int(h*p*46);
	}

BOOL XYInBar(int x, int y)
	{
	x -= WIN_OFFSET_X;
	y -= WIN_OFFSET_Y;
	int left, top, right, bottom;
	GetBarRect(left, top, right, bottom);
	return x >= left && x <= right && y >= top && y <= bottom;
	}

int GetPosFromXY(int x, int y)
	{
	x -= WIN_OFFSET_X;
	y -= WIN_OFFSET_Y;

	int left, top, right, bottom;
	GetBarRect(left, top, right, bottom);
	if (x >= left && x <= right && y >= top && y <= bottom)
		return 0;

	for (int Pos = 1; Pos <= 24; ++Pos)
		{
		GetTriangleRect(Pos, left, top, right, bottom);
		if (x >= left && x <= right && y >= top && y <= bottom)
			return Pos;
		}

	return -1;
	}

static void DrawButton(int k, const string &Text)
	{
	int left = w + BUTTON_HSPACE;
	int top = k*(BUTTON_HEIGHT + BUTTON_VSPACE);
	int right = left + BUTTON_WIDTH;
	int bottom = top + BUTTON_HEIGHT;

	HGDIOBJ hBrush = CreateSolidBrush(ButtonFillColor);
	HGDIOBJ hPen = CreatePen(PS_SOLID, BUTTON_BORDER_WIDTH, ButtonBorderColor);

	HGDIOBJ hOldBrush = SelectObject(myhdc, hBrush);
	HGDIOBJ hOldPen = SelectObject(myhdc, hPen);

	int width = 7;
	int height = 7;
	Ok = RoundRect(myhdc, left, top, right, bottom, width, height);
	assert(Ok);

	RECT rect;
    SetRect(&rect, left, top, right, bottom);

	int margin = int((bottom - top)*0.2);
	int FontHeight = bottom - top - 2*margin;
	int FontWidth = int(FontHeight/2.5);
	int FontAngle = 0; // rotation angle in tenths of a degree
	int FontOrientation = 0;
    HFONT hFont = CreateFont(FontHeight, FontWidth, FontAngle, FontOrientation,
	  FW_HEAVY, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
      CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
    HGDIOBJ hOldFont = SelectObject(myhdc, hFont);

	UINT OldAlign = SetTextAlign(myhdc, TA_CENTER);

	int OldBkMode = SetBkMode(myhdc, TRANSPARENT);
    SetTextColor(myhdc, ButtonTextColor);
	Ok = TextOut(myhdc, (left + right)/2, top + margin,
	  Text.c_str(), int(Text.size()));
	assert(Ok);

	SetBkMode(myhdc, OldBkMode);
	SetTextAlign(myhdc, OldAlign);

	SelectObject(myhdc, hOldBrush);
	SelectObject(myhdc, hOldPen);
	SelectObject(myhdc, hOldFont);

	DeleteObject(hPen);
	DeleteObject(hBrush);
	DeleteObject(hFont);
	}

void DrawInfoStr(const string &Text, bool Error)
	{
	int left = 0; // int(w*p*0);
	int top = h + 15;
	int right = w; // int(w - w*p*3);
	int bottom = top + 30;

	HGDIOBJ hBrush = CreateSolidBrush(InfoFillColor);
	HGDIOBJ hPen = CreatePen(PS_SOLID, INFO_BORDER_WIDTH, InfoBorderColor);

	HGDIOBJ hOldBrush = SelectObject(myhdc, hBrush);
	HGDIOBJ hOldPen = SelectObject(myhdc, hPen);

	int width = 7;
	int height = 7;
	Ok = RoundRect(myhdc, left, top, right, bottom, width, height);
	assert(Ok);

	RECT rect;
    SetRect(&rect, left, top, right, bottom);

	int margin = int((bottom - top)*0.2);
	int FontHeight = bottom - top - 2*margin;
	int FontWidth = int(FontHeight/2.5);
	int FontAngle = 0; // rotation angle in tenths of a degree
	int FontOrientation = 0;
    HFONT hFont = CreateFont(FontHeight, FontWidth, FontAngle, FontOrientation,
	  FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
      CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
    HGDIOBJ hOldFont = SelectObject(myhdc, hFont);

	UINT OldAlign = SetTextAlign(myhdc, TA_LEFT);

	int OldBkMode = SetBkMode(myhdc, TRANSPARENT);
    SetTextColor(myhdc, Error ? InfoErrColor : InfoTextColor);
	Ok = TextOut(myhdc, left + 2*margin, top + margin,
	  Text.c_str(), int(Text.size()));
	assert(Ok);

    SetTextColor(myhdc, HelpTextColor);
	string HelpText = "U=Undo, N=New game, R=Resign";
	Ok = TextOut(myhdc, left + 2*margin, bottom + margin,
	  HelpText.c_str(), int(HelpText.size()));
	assert(Ok);

	SetBkMode(myhdc, OldBkMode);
	SetTextAlign(myhdc, OldAlign);

	SelectObject(myhdc, hOldBrush);
	SelectObject(myhdc, hOldPen);
	SelectObject(myhdc, hOldFont);

	DeleteObject(hPen);
	DeleteObject(hBrush);
	DeleteObject(hFont);
	}

static BOOL DrawRect(double left, double top, double width, double height)
	{
	int right = int(left + width);
	int bottom = int(top + height);
	BOOL Ok = Rectangle(myhdc, int(left), int(top), right, bottom);
	return Ok;
	}

static void DrawTriangle(int Pos, int left, int top, int bottom, int right)
	{
	HGDIOBJ hBrush = CreateSolidBrush((Pos%2==0) ? TriangleColor1 : TriangleColor2);
	HGDIOBJ hPen = CreatePen(PS_SOLID, 1, TriangleBorderColor);

	HGDIOBJ hOldBrush = SelectObject(myhdc, hBrush);
	HGDIOBJ hOldPen = SelectObject(myhdc, hPen);

	POINT Points[3];
	if (Pos <= 12)
		{
		Points[0].x = left;
		Points[0].y = bottom;

		Points[1].x = (left + right)/2;
		Points[1].y = top;

		Points[2].x = right;
		Points[2].y = bottom;
		}
	else
		{
		Points[0].x = left;
		Points[0].y = top;

		Points[1].x = (left + right)/2;
		Points[1].y = bottom;

		Points[2].x = right;
		Points[2].y = top;
		}

	Ok = Polygon(myhdc, Points, 3);
//	assert(Ok);

	SelectObject(myhdc, hOldBrush);
	SelectObject(myhdc, hOldPen);
	DeleteObject(hBrush);
	DeleteObject(hPen);

	int x = Points[1].x;
	int y = Points[0].y;
	if (Pos >= 13)
		DrawPointNr(Pos, x, y - 18);
	else
		DrawPointNr(Pos, x, y + 5);
	}

static void DrawDieDot(int left, int top, int right, int bottom, int k)
	{
	double hs = (right - left)/4.0;
	double vs = (bottom - top)/4.0;
	int radius = DIE_DOT_RADIUS;
	int x = int(left + (k%3 + 1)*hs);
	int y = int(top + (k/3 + 1)*vs);

	Ok = Ellipse(myhdc, x - radius, y - radius, x + radius, y + radius);
	assert(Ok);
	}

static void DrawDie(int left, int top, int right, int bottom, int n,
  bool White)
	{
	COLORREF FillColor = (White ? WhiteDiceFillColor : BlackDiceFillColor);
	COLORREF BorderColor = (White ? WhiteDiceBorderColor : BlackDiceBorderColor);

	if (GS.WaitingForPickUp())
		{
		FillColor = GRAY;
		BorderColor = GRAY;
		}

	HGDIOBJ hBrush = CreateSolidBrush(FillColor);
	HGDIOBJ hPen = CreatePen(PS_SOLID, 1, BorderColor);

	HGDIOBJ hOldBrush = SelectObject(myhdc, hBrush);
	HGDIOBJ hOldPen = SelectObject(myhdc, hPen);

	int width = 7;
	int height = 7;
	Ok = RoundRect(myhdc, left, top, right, bottom, width, height);
	assert(Ok);

	FillColor = (White ? WhiteDiceDotFillColor : BlackDiceDotFillColor);
	BorderColor = (White ? WhiteDiceDotBorderColor : BlackDiceDotBorderColor);

	hBrush = CreateSolidBrush(FillColor);
	hPen = CreatePen(PS_SOLID, 1, BorderColor);

	SelectObject(myhdc, hBrush);
	SelectObject(myhdc, hPen);

	switch (n)
		{
	case 1:
		DrawDieDot(left, top, right, bottom, 4);
		break;
	case 2:
		DrawDieDot(left, top, right, bottom, 0);
		DrawDieDot(left, top, right, bottom, 8);
		break;
	case 3:
		DrawDieDot(left, top, right, bottom, 0);
		DrawDieDot(left, top, right, bottom, 4);
		DrawDieDot(left, top, right, bottom, 8);
		break;
	case 4:
		DrawDieDot(left, top, right, bottom, 0);
		DrawDieDot(left, top, right, bottom, 2);
		DrawDieDot(left, top, right, bottom, 6);
		DrawDieDot(left, top, right, bottom, 8);
		break;
	case 5:
		DrawDieDot(left, top, right, bottom, 0);
		DrawDieDot(left, top, right, bottom, 2);
		DrawDieDot(left, top, right, bottom, 4);
		DrawDieDot(left, top, right, bottom, 6);
		DrawDieDot(left, top, right, bottom, 8);
		break;
	case 6:
		DrawDieDot(left, top, right, bottom, 0);
		DrawDieDot(left, top, right, bottom, 3);
		DrawDieDot(left, top, right, bottom, 6);
		DrawDieDot(left, top, right, bottom, 2);
		DrawDieDot(left, top, right, bottom, 5);
		DrawDieDot(left, top, right, bottom, 8);
		break;
	default:
		assert(false);
		}

	SelectObject(myhdc, hOldBrush);
	SelectObject(myhdc, hOldPen);

	DeleteObject(hBrush);
	DeleteObject(hPen);
	}

static void DrawCube()
	{
	HGDIOBJ hBrush = CreateSolidBrush(CubeBodyColor);
	HGDIOBJ hPen = CreatePen(PS_SOLID, 1, CubeBorderColor);

	HGDIOBJ hOldBrush = SelectObject(myhdc, hBrush);
	HGDIOBJ hOldPen = SelectObject(myhdc, hPen);

// Center of cube
	int x = int(w*p*2.5);
	int y = int(h*p*25);

// Side length
	int size = int(w*p*3*0.95);

	int left = x - size/2;
	int right = x + size/2;
	int top = y - size/2;
	int bottom = y + size/2;
	int width = 10;
	int height = 10;

	Ok = RoundRect(myhdc, left, top, right, bottom, width, height);
	assert(Ok);

	int margin = int((bottom - top)*0.3);
	int FontHeight = bottom - top - margin;
	int FontWidth = FontHeight/2;
	int FontAngle = 900; // rotation angle in tenths of a degree
	int FontOrientation = 0;
    HFONT hFont = CreateFont(FontHeight, FontWidth, FontAngle, FontOrientation,
	  FW_HEAVY, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
      CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
    HGDIOBJ hOldFont = SelectObject(myhdc, hFont);
        
    //Sets the coordinates for the rectangle in which the text is to be formatted.
	RECT rect;
    SetRect(&rect, left + margin/2, bottom - margin/2, right, bottom);
	int OldBkMode = SetBkMode(myhdc, TRANSPARENT);
    SetTextColor(myhdc, BLACK);
    DrawText(myhdc, TEXT("64"), -1, &rect, DT_NOCLIP);
	SetBkMode(myhdc, OldBkMode);
        
	SelectObject(myhdc, hOldBrush);
	SelectObject(myhdc, hOldPen);
	SelectObject(myhdc, hOldFont);

	DeleteObject(hBrush);
	DeleteObject(hPen);
	DeleteObject(hFont);
	}

static void GetCheckerCenter(int PointIndex, int k, int &x, int &y)
	{
	assert(PointIndex >= 1 && PointIndex <= 24);
	assert(k >= 0 && k < 5);

	if (PointIndex >= 19)
		{
		x = int(w*p * (28.5 + (PointIndex-19) * 3));
		y = int(2*h*p + 1.5*w*p + k*3*w*p);
		}
	else if (PointIndex >= 13)
		{
		x = int(w*p * (6.5 + (PointIndex-13) * 3));
		y = int(2*h*p + 1.5*w*p + k*3*w*p);
		}
	else if (PointIndex >= 7)
		{
		x = int(w*p * (6.5 + (12-PointIndex) * 3));
		y = int(48*h*p - 1.5*w*p - k*3*w*p);
		}
	else
		{
		x = int(w*p * (28.5 + (6-PointIndex) * 3));
		y = int(48*h*p - 1.5*w*p - k*3*w*p);
		}
	}

static void GetCheckerRectPoint(int PointIndex, int k,
  int &left, int &top, int &right, int &bottom)
	{
	int x, y;
	GetCheckerCenter(PointIndex, k, x, y);

	double radius = w*p*1.5;
	left = int(x - radius);
	top = int(y - radius);
	right = int(x + radius);
	bottom = int(y + radius);
	}

static void DrawCheckNrGt5(int PointIndex, int n, bool White)
	{
	int left, top, right, bottom;
	GetCheckerRectPoint(PointIndex, 4, left, top, right, bottom);

	char Str[16];
	sprintf(Str, "%d", n);

	int margin = 10;
	int FontHeight = bottom - top - margin;
	int FontWidth = FontHeight/2;
	int FontAngle = 0; // rotation angle in tenths of a degree
	int FontOrientation = 0;
    HFONT hFont = CreateFont(FontHeight, FontWidth, FontAngle, FontOrientation,
	  FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
      CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
    SelectObject(myhdc, hFont);
        
    //Sets the coordinates for the rectangle in which the text is to be formatted.
	RECT rect;
    SetRect(&rect, left + margin, top, right, bottom);
	int OldBkMode = SetBkMode(myhdc, TRANSPARENT);
    SetTextColor(myhdc, White ? BLACK : WHITE);
    DrawText(myhdc, Str, -1, &rect, DT_NOCLIP);
	SetBkMode(myhdc, OldBkMode);
	}

static void GetCheckerRectBar(int k, int &left, int &top, int &right,
  int &bottom, bool White)
	{
	int x = int(w*p*25);
	int y = White ? int(h*p*32 + k*w*p*3) : int(h*p*18 - k*w*p*3);
	double radius = w*p*1.5;
	left = int(x - radius);
	top = int(y - radius);
	right = int(x + radius);
	bottom = int(y + radius);
	}

static void DrawCheckerRectColors(int left, int top, int right, int bottom,
  COLORREF FillColor, COLORREF BorderColor, int PenWidth)
	{
	HGDIOBJ hBrush = CreateSolidBrush(FillColor);
	HGDIOBJ hPen = CreatePen(PS_SOLID, PenWidth, BorderColor);

	HGDIOBJ hOldBrush = SelectObject(myhdc, hBrush);
	HGDIOBJ hOldPen = SelectObject(myhdc, hPen);

	Ok = Ellipse(myhdc, left, top, right, bottom);
	assert(Ok);

	SelectObject(myhdc, hOldBrush);
	SelectObject(myhdc, hOldPen);

	DeleteObject(hBrush);
	DeleteObject(hPen);
	}

static void DrawCheckerRect(int left, int top, int right, int bottom, bool White)
	{
	COLORREF FillColor = (White ? WhiteCheckerFillColor : BlackCheckerFillColor);
	COLORREF BorderColor = (White ? WhiteCheckerBorderColor : BlackCheckerBorderColor);
	DrawCheckerRectColors(left, top, right, bottom, FillColor, BorderColor, 1);
	}

// Draw k'th checker on PointIndex'th point
// PointIndex = 1, 2 .. 24
static void DrawCheckerPoint(int PointIndex, int k, bool White)
	{
	int left, top, right, bottom;
	GetCheckerRectPoint(PointIndex, k, left, top, right, bottom);
	DrawCheckerRect(left, top, right, bottom, White);
	}

static void DrawCheckerBar(int k, bool White)
	{
	int left, top, right, bottom;
	GetCheckerRectBar(k, left, top, right, bottom, White);
	DrawCheckerRect(left, top, right, bottom, White);
	}

static void DrawDice()
	{
	int n1 = GS.Die1;
	int n2 = GS.Die2;

	int top = h/2 - DIE_SIZE/2;
	int bottom = top + DIE_SIZE;

	int leftw = int(w - w*p*7 - 2*DIE_SIZE - 10);
	int rightw = leftw + DIE_SIZE;

	int leftb = int(w*p*7 + DIE_SIZE);
	int rightb = leftb + DIE_SIZE;

	bool ComputerMove = (GS.PA == PA_ComputerMove ||
	  GS.PA == PA_ComputerPickUp);
	if (AtOpeningRoll())
		{
		int minn = min(n1, n2);
		int maxn = max(n1, n2);
		int nw = (ComputerMove ? maxn : minn);
		int nb = (ComputerMove ? minn : maxn);

		DrawDie(leftw, top, rightw, bottom, nw, true);
		DrawDie(leftb, top, rightb, bottom, nb, false);
		return;
		}

	if (n1 < n2)
		swap(n1, n2);

	int left = (ComputerMove ? leftw : leftb);
	int right = left + DIE_SIZE;
	DrawDie(left, top, right, bottom, n1, ComputerMove);

	left += DIE_SIZE + 10;
	right += DIE_SIZE + 10;

	DrawDie(left, top, right, bottom, n2, ComputerMove);
	}

static void DrawCheckerOff(int k, bool White)
	{
	COLORREF FillColor = (White ? WhiteCheckerFillColor : BlackCheckerFillColor);
	COLORREF BorderColor = (White ? WhiteCheckerBorderColor : BlackCheckerBorderColor);
	HGDIOBJ hBrush = CreateSolidBrush(FillColor);
	HGDIOBJ hPen = CreatePen(PS_SOLID, 1, BorderColor);

	HGDIOBJ hOldBrush = SelectObject(myhdc, hBrush);
	HGDIOBJ hOldPen = SelectObject(myhdc, hPen);

	int width = int(w*p*3);
	int height = int(h*p*4.0/3.0);

	int left, top;
	if (White)
		{
		left = int(w*p*46);
		top = int(h*p*(2 + k*4.0/3.0));
		}
	else
		{
		left = int(w*p*46);
		top = int(h*p*(48 - (k+1)*4.0/3.0));
		}

	int right = left + width;
	int bottom = top + height;
	BOOL Ok = Rectangle(myhdc, left, top, right, bottom);
	assert(Ok);

	SelectObject(myhdc, hOldBrush);
	SelectObject(myhdc, hOldPen);

	DeleteObject(hBrush);
	DeleteObject(hPen);
	}

void PaintMainWindow()
	{
	if (g_hwnd == NULL)
		return;

	static bool FirstTime = true;
	if (FirstTime)
		{
		FirstTime = false;
		StartNewGame();
		}

	GS = GetCurrentGameState();

	SetDC();
	OffsetWindowOrgEx(myhdc, -WIN_OFFSET_X, -WIN_OFFSET_Y, NULL);

	HGDIOBJ hBrushInner = CreateSolidBrush(BoardInnerColor);
	HGDIOBJ hBrushBorder = CreateSolidBrush(BoardBorderColor);
    HGDIOBJ hPen = CreatePen(PS_SOLID, 1, BoardBorderColor);
	HGDIOBJ hPenOld = SelectObject(myhdc, hPen); 

    HGDIOBJ hBrushOld = SelectObject(myhdc, hBrushBorder); 
    Ok = DrawRect(0, 0, w, h);

    SelectObject(myhdc, hBrushInner); 
	Ok = DrawRect(w*p, h*p*2, w*p*3, h*p*20); //top left holder
    Ok = DrawRect(w*p, h*p*28, w*p*3, h*p*20); //bottom left holder
    Ok = DrawRect(w*p*46, h*p*2, w*p*3, h*p*20); //top right holder
    Ok = DrawRect(w*p*46, h*p*28, w*p*3, h*p*20); //bottom right holder
    Ok = DrawRect(w*p*5, h*p*2, w*p*18, h*p*46); //left board
    Ok = DrawRect(w*p*27, h*p*2, w*p*18, h*p*46); //right board

	for (int Pos = 1; Pos <= 24; ++Pos)
		{
		int left, top, bottom, right;
		GetTriangleRect(Pos, left, top, right, bottom);
		DrawTriangle(Pos, left, top, bottom, right);
		}

	DrawCube();

	BgBoard Board;
	Board.FromStr(GS.PosId);
	const byte *w = Board.m_w;
	const byte *b = Board.m_b;

	for (int k = 0; k < w[BAR]; ++k)
		DrawCheckerBar(k, true);
	for (int k = 0; k < b[BAR]; ++k)
		DrawCheckerBar(k, false);

	for (int Pos = 1; Pos <= 24; ++Pos)
		{
		byte nw = w[Pos];
		byte nb = b[INV(Pos)];
		int n = nw + nb;
		assert(nw == 0 || nb == 0);
		for (int k = 0; k < n; ++k)
			{
			if (k >= 5)
				{
				DrawCheckNrGt5(Pos, n, nw > 0);
				break;
				}
			DrawCheckerPoint(Pos, k, nw > 0);
			}
		}

	for (int k = 0; k < w[OFF]; ++k)
		DrawCheckerOff(k, true);
	for (int k = 0; k < b[OFF]; ++k)
		DrawCheckerOff(k, false);

	DrawDice();
	DrawInfoStr(g_CurrentInfoStr, g_InfoStrErr);

	SelectObject(myhdc, hPenOld); 
	SelectObject(myhdc, hBrushOld); 

	DeleteObject(hBrushInner); 
    DeleteObject(hBrushBorder); 
    DeleteObject(hPen); 
	ReleaseDC(g_hwnd, myhdc);
	myhdc = NULL;

//	RECT r;
//	GetWindowRect(g_hwnd, &r);
//	int width = r.right - r.left;
//	int height = r.bottom - r.top;
//
//	Ok = BitBlt(g_wnd_hdc, 0, 0, width, height, myhdc, 0, 0, SRCCOPY);
//	DWORD Err = GetLastError();
//	assert(Ok);
////	ReleaseDC(g_hwnd, g_wnd_hdc);
	}
