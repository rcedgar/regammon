#include "myutils.h"
#include "bgboard.h"
#include "pubeval.h"

static void ParseGameOverInfo(const string &Info, string &Player,
  unsigned &Points)
	{
	asserta(StartsWith(Info, "win="));
	string s = Info.substr(4, string::npos);
	vector<string> Fields;
	Split(s, Fields, '+');
	asserta(SIZE(Fields) == 2);
	Player = Fields[0];
	Points = StrToUint(Fields[1]);
	}

void cmd_checkbench()
	{
	const string &FileName = opt(checkbench);

	PubEval PE;

	FILE *f = OpenStdioFile(FileName);

	string Line;
	vector<string> Fields;

	unsigned PrevGameIndex = UINT_MAX;
	unsigned PrevPlyIndex = UINT_MAX;
	string OppName;
	string PrevPosIdBefore;
	string PrevPosIdAfter;
	string PrevPlayerName;
	string PrevInfo;
	unsigned LineNr = 0;
	ProgressFileInit(f, "Checking");
	while (ReadLineStdioFile(f, Line))
		{
		ProgressFileStep();
		++LineNr;

//       0  1         2                 3   4      5               6
// 1000016 37      pube    c7cNCAC3bgAAHg  34      c7cNCADvOgAAHg  c/-1.52
// 1000016 38      gnu     c7cNCADvOgAAHg  12      c3cHCADvOgAAHg  7/6 7/5
// 1000016 39      pube    c3cHCADvOgAAHg  44      (blocked)
// 1000016 40      gnu     c3cHCADvOgAAHg  35      c3cPAADvOgAAHg  14/6
		Split(Line, Fields, '\t');
		const unsigned n = SIZE(Fields);
		asserta(n >= 6);

		unsigned GameIndex = StrToUint(Fields[0]);
		unsigned PlyIndex = StrToUint(Fields[1]);
		const string &PlayerName = Fields[2];
		const string &PosIdBefore = Fields[3];
		const string &Dice = Fields[4];
		const string &PosIdAfter = Fields[5];
		string MoveStr;
		string Info;
		if (n > 6)
			MoveStr = Fields[6];
		if (n > 7)
			Info = Fields[7];

		asserta(SIZE(Dice) == 2);

		unsigned Die1 = unsigned(Dice[0] - '0');
		unsigned Die2 = unsigned(Dice[1] - '0');
		asserta(Die1 >= 1 && Die1 <= 6 && Die2 >= 1 && Die2 <= 6);

		if (PlayerName != "gnu")
			{
			if (OppName == "")
				OppName = PlayerName;
			else
				asserta(OppName == PlayerName);
			}
		asserta(PlayerName == "gnu" || PlayerName == OppName);
		if (PlyIndex != 0)
			asserta(PlayerName != PrevPlayerName);
		bool White = (PlayerName != "gnu");

		if (PosIdAfter != "(blocked)")
			{
			BgBoard Board;
			Board.FromStr(PosIdAfter);
			if (Board.GameOver())
				{
				string Player;
				unsigned Points;
				ParseGameOverInfo(Info, Player, Points);
				asserta(Player == PlayerName);
				int iPoints = Board.GetPoints();
				if (Player == "gnu")
					asserta(iPoints == -int(Points));
				else
					asserta(iPoints == int(Points));
				}
			}

		if (PrevGameIndex != GameIndex)
			{
			asserta(PlyIndex == 0);
			asserta(PosIdBefore == START_STR);
			asserta(!(Dice[0] == Dice[1]));

			if (PrevGameIndex != UINT_MAX)
				{
				BgBoard PrevBoard;
				PrevBoard.FromStr(PrevPosIdAfter);
				asserta(PrevBoard.GameOver());
				}
			}
		else
			{
			BgBoard BoardBefore;
			BoardBefore.FromStr(PosIdBefore);
			BoardBefore.m_Die1 = Die1;
			BoardBefore.m_Die2 = Die2;
			if (PosIdAfter == "(blocked)")
				{
				vector<unsigned> Moves;
				BoardBefore.GetMoves(Moves, White);
				asserta(SIZE(Moves) == 0);
				}
			else
				{
				if (PrevPosIdAfter == "(blocked)")
					asserta(PosIdBefore == PrevPosIdBefore);
				else
					asserta(PosIdBefore == PrevPosIdAfter);

				BgBoard BoardAfter;
				BoardAfter.FromStr(PosIdAfter);
				if (!BoardBefore.IsValidAfterState(BoardAfter, White))
					{
					Log("\n");
					Log("%u: %s\n", LineNr, Line.c_str());
					Log("------ Board before -----\n");
					BoardBefore.LogMe();

					Log("\n");
					Log("------ Board after -----\n");
					BoardAfter.LogMe();

					Die("AfterState White=%c", tof(White));
					}
				}
			}

		if (PlayerName == "pube" && PosIdAfter != "(blocked)")
			{
			BgBoard Board;
			Board.FromStr(PosIdBefore);
			Board.m_Die1 = Die1;
			Board.m_Die2 = Die2;
			PE.DoWhiteMove(Board);
			string PosId;
			Board.ToStr(PosId);
			asserta(PosId == PosIdAfter);
			}

		PrevGameIndex = GameIndex;
		PrevPlyIndex = PlyIndex;
		PrevPosIdBefore = PosIdBefore;
		PrevPosIdAfter = PosIdAfter;
		PrevPlayerName = PlayerName;
		PrevInfo = Info;
		}
	ProgressFileDone();

	CloseStdioFile(f);
	}
