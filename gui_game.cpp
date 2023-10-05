#include "stdafx.h"
#include "gui.h"

static vector<GameState> g_States;
bool g_Trace = false;
bool g_SelfPlay = false;
static void LogGS();

const char *PAToStr(PENDING_ACTION PA)
	{
#define c(x)	case PA_##x: return #x;
	switch (PA)
		{
	c(Invalid)
	c(HumanMove)
	c(ComputerMove)
	c(HumanPickUp)
	c(ComputerPickUp)
	c(NewGame)
		}
	return "??";
	}

void ShowError(const string &s)
	{
	SetInfoStr(s, true);
	PaintMainWindow();
	}

bool AtOpeningRoll()
	{
	return g_States.size() == 1;
	}

static PENDING_ACTION GetPA()
	{
	if (g_States.empty())
		return PA_Invalid;
	return g_States.back().PA;
	}

static void SetHumanPips(GameState &GS)
	{
	if (GS.Die1 == GS.Die2)
		{
		GS.HumanPips.push_back(GS.Die1);
		GS.HumanPips.push_back(GS.Die1);
		GS.HumanPips.push_back(GS.Die1);
		GS.HumanPips.push_back(GS.Die1);
		}
	else
		{
		GS.HumanPips.push_back(GS.Die1);
		GS.HumanPips.push_back(GS.Die2);
		}
	}

void StartNewGame()
	{
	g_States.clear();

	GameState GS;
	GS.PosId = START_STR;
	GS.Die1 = 1 + randu32()%6;
	GS.Die2 = 1 + randu32()%5;
	if (GS.Die2 == GS.Die1)
		++GS.Die2;
	if (GS.Die1 > GS.Die2)
		GS.PA = PA_ComputerMove;
	else
		{
		GS.PA = PA_HumanMove;
		SetHumanPips(GS);
		}

	PushGameState(GS);
	if (g_Trace)
		{
		Log("New: ");
		LogGS();
		}
	}

const GameState &GetCurrentGameState()
	{
	assert(!g_States.empty());
	return g_States.back();
	}

static void LogGS()
	{
	if (g_States.empty())
		Log("(no game)");
	else
		GetCurrentGameState().LogMe();
	}

static void GetBigSmall(const GameState &GS, int &Big, int &Small)
	{
	Big = -1;
	Small = -1;
	const int N = ISIZE(GS.HumanPips);
	for (int i = 0; i < N; ++i)
		{
		int n = GS.HumanPips[i];
		if (n < 0)
			continue;
		if (n > Big)
			Big = n;
		if (Small == -1 || n < Small)
			Small = n;
		}
	}

static void SetInfo()
	{
	const GameState &GS = GetCurrentGameState();
	string s;
	switch (GS.PA)
		{
	case PA_HumanMove:
		{
		int Big, Small;
		GetBigSmall(GS, Big, Small);
		assert(Big >= 1 && Big <= 6 && Small >= 1 && Small <= 6);
		if (Big == Small)
			Ps(s, "Your move, click or right-click to move %d", Big);
		else
			Ps(s, "Your move, click to move %d, right-click to move %d", Big, Small);
		break;
		}

	case PA_ComputerMove:
		{
		int MaxDie = max(GS.Die1, GS.Die2);
		int MinDie = min(GS.Die1, GS.Die2);
		Ps(s, "Computer to move %d-%d, press spacebar to continue",
		  MaxDie, MinDie);
		break;
		}

	case PA_HumanPickUp:
		{
		if (!GS.HumanPips.empty())
			Ps(s, "You are blocked, press spacebar to continue");
		else
			Ps(s, "You moved %s, press spacebar to continue or U to undo",
			  GS.HumanMoveStr.c_str());
		break;
		}

	case PA_ComputerPickUp:
		{
		if (GS.ComputerMoveStr == "Blocked")
			Ps(s, "Computer is blocked, press spacebar to continue");
		else
			Ps(s, "Computer moved %s, press spacebar to roll",
			  GS.ComputerMoveStr.c_str());
		break;
		}

	case PA_NewGame:
		break;

	default:
		assert(false);
		}

	SetInfoStr(s, false);
	}

static bool HumanCanMove(const GameState &GS)
	{
	int n = ISIZE(GS.HumanPips);
	bool Any = false;
	BgBoard Board;
	Board.FromStr(GS.PosId);
	for (int i = 0; i < n; ++i)
		{
		int Pips = GS.HumanPips[i];
		assert(Pips >= 1 && Pips <= 6);
		if (Board.CanStepPips(unsigned(Pips), false))
			{
			Any = true;
			break;
			}
		}
	return Any;
	}

void PushGameState(GameState &GS)
	{
	if (GS.PA == PA_HumanMove && !HumanCanMove(GS))
		GS.PA = PA_HumanPickUp;

	BgBoard Board;
	Board.FromStr(GS.PosId);
	if (Board.GameOver())
		{
		if (g_SelfPlay)
			{
			SHORT KS = GetKeyState(VK_ESCAPE);
			if (KS != 0)
				g_SelfPlay = false;
			}
		else
			{
			int Points = Board.GetPoints();
			string s;
			Ps(s, "%+d points", Points);
			MessageBox(g_hwnd, s.c_str(), "", MB_OK);
			}
		StartNewGame();
		return;
		}

	g_States.push_back(GS);
	SetInfo();
	PaintMainWindow();
	}

void OnSpaceBar()
	{
	if (g_Trace)
		{
		Log("SpaceBar ");
		LogGS();
		}
	if (g_States.empty())
		{
		if (g_Trace)
			Log("StartNewGame\n");
		StartNewGame();
		}

	PENDING_ACTION PA = GetPA();
	switch (PA)
		{
	case PA_HumanMove:
		return;

	case PA_ComputerMove:
		{
		GameState GS = GetCurrentGameState();
		string PosIdAfter;
		ComputerMove(GS.PosId, true, GS.Die1, GS.Die2, PosIdAfter,
		  GS.ComputerMoveStr);
		GS.PosId = PosIdAfter;
		GS.PA = PA_ComputerPickUp;
		PushGameState(GS);
		if (g_Trace)
			{
			Log("After ComputerMove ");
			LogGS();
			}
		return;
		}

	case PA_HumanPickUp:
		{
		GameState GS = GetCurrentGameState();
		if (!GS.HumanPips.empty())
			assert(!HumanCanMove(GS));
		GS.HumanPips.clear();
		GS.Die1 = 1 + randu32()%6;
		GS.Die2 = 1 + randu32()%6;
		GS.PA = PA_ComputerMove;
		GS.HumanMoveStr.clear();
		PushGameState(GS);
		if (g_Trace)
			{
			Log("After HumanPickUp ");
			LogGS();
			}
		return;
		}

	case PA_ComputerPickUp:
		{
		GameState GS = GetCurrentGameState();
		GS.Die1 = 1 + randu32()%6;
		GS.Die2 = 1 + randu32()%6;
		GS.PA = PA_HumanMove;
		SetHumanPips(GS);
		PushGameState(GS);
		if (g_Trace)
			{
			Log("After ComputerPickUp ");
			LogGS();
			}
		return;
		}

	case PA_NewGame:
		StartNewGame();
		return;
		}

	assert(false);
	}

void PlayDie(int Pos, bool DoBig)
	{
	if (Pos < 0 || Pos > 24)
		return;

	const GameState &GS = GetCurrentGameState();
	if (GS.HumanPips.empty())
		{
		SetInfoStr("Turn already finished", true);
		PaintMainWindow();
		return;
		}

	int Big, Small;
	GetBigSmall(GS, Big, Small);
	int Pips = (DoBig ? Big : Small);
	if (Pips == -1)
		return;

	BgBoard Board;
	Board.FromStr(GS.PosId);
	byte w[26];
	byte b[26];
	memcpy(w, Board.m_w, 26);
	memcpy(b, Board.m_b, 26);

	int InvPos = (Pos == 0 ? Pos : INV(Pos));

	bool Ok = BgBoard::ApplyStep2(b, w, InvPos, Pips, false);
	if (!Ok)
		{
		SetInfoStr("Invalid move", true);
		PaintMainWindow();
		return;
		}

	memcpy(Board.m_w, w, 26);
	memcpy(Board.m_b, b, 26);

	string NewPosId;
	Board.ToStr(NewPosId);

	GameState NewGS = GS;
	NewGS.PosId = NewPosId;

	vector<int> NewHumanPips;
	bool Found = false;
	int N = ISIZE(GS.HumanPips);
	for (int i = 0; i < N; ++i)
		{
		int n = GS.HumanPips[i];
		if (n == Pips && !Found)
			Found = true;
		else
			NewHumanPips.push_back(n);
		}
	NewGS.HumanPips = NewHumanPips;
	if (NewHumanPips.empty())
		NewGS.PA = PA_HumanPickUp;
	string MoveStr;
	int To = Pos - Pips;
	if (To <= 0)
		Ps(MoveStr, "[%d]%d off", Pips, Pos);
	else
		Ps(MoveStr, "[%d]%d/%d", Pips, Pos, To);
	if (!NewGS.HumanMoveStr.empty())
		NewGS.HumanMoveStr += ", ";
	NewGS.HumanMoveStr += MoveStr;
	PushGameState(NewGS);
	if (g_Trace)
		{
		Log("After PlayPips ");
		LogGS();
		}
	}

void OnMouseMove(int x, int y, bool Dragging)
	{
	//string s;
	//Ps(s, "Mouse(%d, %d, %c)", x, y, tof(Dragging));
	//int Pos = GetPosFromXY(x, y);
	//Psa(s, " pt=%d", Pos);
	//Psa(s, " bar=%c", tof(XYInBar(x, y)));
	//WriteStatusText(s);
	}

void OnMouseLeftClick(int x, int y)
	{
	//string s;
	//Ps(s, "OnMouseLeftClick(%d, %d)", x, y);
	//WriteStatusText(s);
	int Pos = GetPosFromXY(x, y);
	if (g_Trace)
		{
		if (Pos < 0)
			Log("LeftClick, Pos < 0\n");
		else
			{
			Log("LeftClick, Pos=%d ", Pos);
			LogGS();
			Log("\n");
			}
		}
	PlayDie(Pos, true);
	}

void OnMouseRightClick(int x, int y)
	{
	//string s;
	//Ps(s, "OnMouseRightClick(%d, %d)", x, y);
	//WriteStatusText(s);
	int Pos = GetPosFromXY(x, y);
	if (g_Trace)
		{
		if (Pos < 0)
			Log("RightClick, Pos < 0\n");
		else
			{
			Log("RightClick, Pos=%d ", Pos);
			LogGS();
			Log("\n");
			}
		}
	PlayDie(Pos, false);
	}

void Undo()
	{
	const GameState &GS = GetCurrentGameState();
	if (g_Trace)
		{
		Log("Undo");
		LogGS();
		Log("\n");
		}
	if (GS.HumanMoveStr.empty())
		{
		ShowError("You haven't moved yet, can't undo");
		return;
		}

	if (!g_States.empty())
		{
		g_States.resize(g_States.size() - 1);
		SetInfo();
		if (g_Trace)
			{
			Log("After undo");
			LogGS();
			Log("\n");
			}
		PaintMainWindow();
		}
	}

void OnCtrlC()
	{
	}

void OnCtrlV()
	{
	}

void Resign()
	{
	int Id = MessageBox(g_hwnd, "Are you sure?", "Resign", MB_YESNO);
	if (Id == IDYES)
		StartNewGame();
	}

void NewGame()
	{
	int Id = MessageBox(g_hwnd, "Start new game?", "New", MB_YESNO);
	if (Id == IDYES)
		StartNewGame();
	}

void SelfPlay()
	{
	g_SelfPlay = true;
	for (;;)
		{
		if (!g_SelfPlay)
			return;

		if (g_States.empty())
			{
			SHORT KS = GetKeyState(VK_ESCAPE);
			if (KS != 0)
				{
				g_SelfPlay = false;
				return;
				}
			StartNewGame();
			}
		if (!g_SelfPlay)
			return;

		PENDING_ACTION PA = GetPA();
		switch (PA)
			{
		case PA_HumanMove:
			{
			GameState GS = GetCurrentGameState();
			string PosIdAfter;
			ComputerMove(GS.PosId, false, GS.Die1, GS.Die2, PosIdAfter,
			  GS.ComputerMoveStr);
			GS.PosId = PosIdAfter;
			GS.PA = PA_HumanPickUp;
			PushGameState(GS);
			break;
			}

		case PA_ComputerMove:
			{
			GameState GS = GetCurrentGameState();
			string PosIdAfter;
			ComputerMove(GS.PosId, true, GS.Die1, GS.Die2, PosIdAfter,
			  GS.ComputerMoveStr);
			GS.PosId = PosIdAfter;
			GS.PA = PA_ComputerPickUp;
			PushGameState(GS);
			break;
			}

		case PA_HumanPickUp:
			{
			GameState GS = GetCurrentGameState();
			GS.HumanPips.clear();
			GS.Die1 = 1 + randu32()%6;
			GS.Die2 = 1 + randu32()%6;
			GS.PA = PA_ComputerMove;
			GS.HumanMoveStr.clear();
			PushGameState(GS);
			break;
			}

		case PA_ComputerPickUp:
			{
			GameState GS = GetCurrentGameState();
			GS.Die1 = 1 + randu32()%6;
			GS.Die2 = 1 + randu32()%6;
			GS.PA = PA_HumanMove;
			SetHumanPips(GS);
			PushGameState(GS);
			break;
			}

		case PA_NewGame:
			{
			StartNewGame();
			SHORT KS = GetKeyState(VK_ESCAPE);
			if (KS != 0)
				{
				g_SelfPlay = false;
				return;
				}
			break;
			}

		default:
			assert(false);
			}
		}
	}
