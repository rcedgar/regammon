#include "myutils.h"
#include "bgboard.h"
#include "winrate.h"
#include "bgeval.h"
#include <map>

#define TRACE	0

void GetGnuMoves(const vector<string> &InPosIds,
  const vector<string> &Dices, vector<string> &GnuMoves,
  vector<string> &GnuMovesx);

static FILE *g_fTab;
static unsigned g_GnuWinCount = 0;
static unsigned g_GnuPoints = 0;
static unsigned g_EvPoints = 0;

void cmd_gnubench()
	{
	string Name = opt(gnubench);
	unsigned GameCount = opt(games);
	if (GameCount == 0)
		GameCount = 100;
	g_fTab = CreateStdioFile(opt(tabbedout));

	BgEval *Ev = BgEval::CreateEval(Name);
	const char *EvName = Ev->GetName();

	bool IsPub = (Name == "pube");

	vector<BgBoard *> Boards;
	vector<bool> Blockeds;
	vector<bool> GnuStarts;
	vector<bool> GnuTurns;
	vector<bool> GnuWins;
	vector<bool> GameOvers;
	vector<unsigned> Die1s;
	vector<unsigned> Die2s;
	for (unsigned GameIndex = 0; GameIndex < GameCount; ++GameIndex)
		{
		BgBoard *Board = new BgBoard;
		Board->SetStart();
		bool GnuStart = (GameIndex%2 == 0);

		GnuStarts.push_back(GnuStart);
		GnuTurns.push_back(GnuStart);
		GameOvers.push_back(false);
		Blockeds.push_back(false);
		Die1s.push_back(0);
		Die2s.push_back(0);
		Boards.push_back(Board);
		}

	const unsigned MAX_PLIES = 5000;
	unsigned ActiveCount = GameCount;
	for (unsigned PlyIndex = 0; ; ++PlyIndex)
		{
		if (ActiveCount == 0)
			break;
		vector<string> PosIds;
		ProgressLogPrefix("Ply %u, active %u / %u",
		  PlyIndex, ActiveCount, GameCount);
		asserta(PlyIndex < MAX_PLIES);
		vector<unsigned> Moves;
		vector<string> OldPosIds;
		vector<string> Dices;
		vector<string> GnuMoves;
		vector<string> GnuMovesx;
		for (unsigned GameIndex = 0; GameIndex < GameCount; ++GameIndex)
			{
			if (GameOvers[GameIndex])
				continue;

			BgBoard &Board = *Boards[GameIndex];

			string BeforePosId;
			Board.GetPosId(BeforePosId);

			bool FirstRoll = (PlyIndex == 0);
			Board.RollDice(FirstRoll);

			Die1s[GameIndex] = Board.m_Die1;
			Die2s[GameIndex] = Board.m_Die2;

			bool GnuTurn = GnuTurns[GameIndex];

			vector<unsigned> Moves;
			Board.GetMoves(Moves, !GnuTurn);
			bool Blocked = Moves.empty();
			Blockeds[GameIndex] = Blocked;

			if (Blocked)
				{
				if (g_fTab)
					{
					fprintf(g_fTab, "%u", GameIndex);
					fprintf(g_fTab, "\t%u", PlyIndex);
					fprintf(g_fTab, "\t%s", GnuTurn ? "gnu" : EvName);
					fprintf(g_fTab, "\t%s", BeforePosId.c_str());
					fprintf(g_fTab, "\t%u%u", Board.m_Die1, Board.m_Die2);
					fprintf(g_fTab, "\t(blocked)");
					fprintf(g_fTab, "\n");
					}
				continue;
				}

			if (GnuTurn)
				{
				string Dice;
				Ps(Dice, "%u%u", Board.m_Die1, Board.m_Die2);

				Dices.push_back(Dice);
				OldPosIds.push_back(BeforePosId);
				}
			}

		GetGnuMoves(OldPosIds, Dices, GnuMoves, GnuMovesx);

		unsigned GnuIndex = 0;
		for (unsigned GameIndex = 0; GameIndex < GameCount; ++GameIndex)
			{
			if (GameOvers[GameIndex])
				continue;

			bool GnuTurn = GnuTurns[GameIndex];
			if (Blockeds[GameIndex])
				{
				GnuTurns[GameIndex] = !GnuTurn;
				continue;
				}

			BgBoard &Board = *Boards[GameIndex];

			string BeforePosId;
			Board.ToStr(BeforePosId);

			unsigned Die1 = Die1s[GameIndex];
			unsigned Die2 = Die2s[GameIndex];
			asserta(Die1 >= 1 && Die1 <= 6);
			asserta(Die2 >= 1 && Die2 <= 6);
			Board.m_Die1 = Die1;
			Board.m_Die2 = Die2;
			string AfterPosId;
			string InfoStr;
			if (GnuTurn)
				{
				const string &OldPosId = OldPosIds[GnuIndex];
				const string &Dice = Dices[GnuIndex];
				const string &GnuMove = GnuMoves[GnuIndex];
				const string &GnuMovex = GnuMovesx[GnuIndex];
				++GnuIndex;

				InfoStr = GnuMove;
				bool Ok = Board.ApplyGnuMovex(GnuMovex);
				if (!Ok)
					{
					Log("\n");
					Log("ApplyGnuMovex failed");
					Log(" OldPosId %s", OldPosId.c_str());
					Log(" Dice %s", Dice.c_str());
					Log(" GnuMove %s", GnuMove.c_str());
					Log(" GnuMovex %s", GnuMovex.c_str());
					Log("\n");
					Die("ApplyGnuMovex");
					}
				}
			else
				{
				Ev->DoWhiteMove(Board);
				if (IsPub)
					{
					bool Race = Board.IsRace();
					float Value = Ev->GetWhiteValueAfter(Board);
					Ps(InfoStr, "%c/%.2f", Race ? 'r' : 'c', Value);
					}
				}

			Board.ToStr(AfterPosId);
			bool GameOver = Board.GameOver();
			bool Gammon = false;
			if (GameOver)
				{
				--ActiveCount;
				GameOvers[GameIndex] = true;
				}

			if (opt(log_boards))
				{
				Log("_______________________________\n");
				Log("After %s (%c) move %u-%u\n",
				  GnuTurn ? "gnu" : EvName,
				  GnuTurn ? 'X' : 'O',
				  Die1, Die2);
				Board.LogMe();
				}

			if (g_fTab)
				{
				fprintf(g_fTab, "%u", GameIndex);
				fprintf(g_fTab, "\t%u", PlyIndex);
				fprintf(g_fTab, "\t%s", GnuTurn ? "gnu" : EvName);
				fprintf(g_fTab, "\t%s", BeforePosId.c_str());
				fprintf(g_fTab, "\t%u%u", Die1, Die2);
				fprintf(g_fTab, "\t%s", AfterPosId.c_str());
				if (InfoStr.empty())
					InfoStr = ".";
				fprintf(g_fTab, "\t%s", InfoStr.c_str());
				if (GameOver)
					{
					int Points = Board.GetPoints();
					if (Points < 0)
						Points = -Points;
					if (GnuTurn)
						{
						++g_GnuWinCount;
						g_GnuPoints += unsigned(Points);
						}
					else
						g_EvPoints += unsigned(Points);
					const char *WinnerName = (GnuTurn ? "gnu" : EvName);
					fprintf(g_fTab, "\twin=%s+%d", WinnerName, Points);
					}
				fprintf(g_fTab, "\n");
				}

			GnuTurns[GameIndex] = !GnuTurn;
			}
		if (g_fTab)
			fflush(g_fTab);
		}

	double GnuWinPct = GetPct(g_GnuWinCount, GameCount);
	double ppg = (GameCount == 0 ? 0.0 : double(g_GnuPoints - g_EvPoints)/GameCount);
	ProgressLog("Gnu wins %u (%.1f%%) %+.3fppg\n",
	  g_GnuWinCount, GnuWinPct, ppg);

	CloseStdioFile(g_fTab);
	}
