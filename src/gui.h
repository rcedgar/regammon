#ifndef gui_h
#define gui_h

#include "stdafx.h"
#include "resource.h"
#include "myutils.h"
#include "bgboard.h"

static const int WIN_OFFSET_X = 20;
static const int WIN_OFFSET_Y = 20;

enum PENDING_ACTION
	{
	PA_Invalid = 0,
	PA_HumanMove,
	PA_ComputerMove,
	PA_HumanPickUp,
	PA_ComputerPickUp,
	PA_NewGame,
	};
const char *PAToStr(PENDING_ACTION PA);

struct GameState
	{
	string PosId;
	int Die1;
	int Die2;
	PENDING_ACTION PA;
	vector<int> HumanPips;
	string HumanMoveStr;
	string ComputerMoveStr;

	bool WaitingForPickUp() const
		{
		return PA == PA_HumanPickUp || PA == PA_ComputerPickUp;
		}

	void LogMe() const
		{
		Log("%s %d-%d %s",
		  PosId.c_str(), Die1, Die2, PAToStr(PA));
		for (int i = 0; i < HumanPips.size(); ++i)
			Log(" [%d]", HumanPips[i]);
		Log("\n");
		}
	};

void StartNewGame();
const GameState &GetCurrentGameState();
void PushGameState(GameState &GS);

void ComputerMove(const string &PosId, bool White,
  int Die1, int Die2, string &PosIdAfter, string &MoveStr);
int GetPosFromXY(int x, int y);
BOOL XYInBar(int x, int y);

extern HWND g_hwnd;
void SetInfoStr(const string &Str, bool Error);
void PaintMainWindow();
void Init(HINSTANCE hInst);
void Undo();
void OnSpaceBar();
void OnMouseMove(int x, int y, bool Dragging);
void OnMouseLeftClick(int x, int y);
void OnMouseRightClick(int x, int y);
void OnCtrlC();
void OnCtrlV();
void Resign();
void NewGame();
void HelpDialog();
void ShowError(const string &s);
bool AtOpeningRoll();
void WriteStatusText(const string &Text);

// Layout calculation adapted from this Javascript code:
//   https://codepen.io/dzsobacsi/pen/LprKQo.
//   Copyright (c) 2018 by Tamas Berki
// Board is sliced to 50 pitches both vertically and horizontally

const int BOARD_WIDTH = 800;
static int w = BOARD_WIDTH;
static int h = int(w*0.7);
static double p = 0.02;

#endif // gui_h
