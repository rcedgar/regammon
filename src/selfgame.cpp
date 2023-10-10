#include "myutils.h"
#include "bgeval.h"

static int SelfGame_Flip(BgEval &Ev, vector<string> &PosIds)
	{
	PosIds.clear();

	BgBoard Board;
	Board.SetStart();

	string PosId;
	bool FirstRoll = true;
	for (;;)
		{
		Board.RollDice(FirstRoll);
		FirstRoll = false;

		Ev.DoMove(Board, true);

		if (Board.GameOver())
			{
			int Points = Board.GetPoints();
			asserta(Points > 0);
			return Points;
			}

		Board.ToStr(PosId);
		PosIds.push_back(PosId);

		Board.Flip();
		}
	}

int SelfGame(BgEval &Ev, vector<string> &PosIds, bool Flip)
	{
	if (Flip)
		return SelfGame_Flip(Ev, PosIds);

	PosIds.clear();

	BgBoard Board;
	Board.SetStart();

	string PosId;
	bool WhiteTurn = true;
	bool FirstRoll = true;
	for (;;)
		{
		Board.RollDice(FirstRoll);
		FirstRoll = false;

		Ev.DoMove(Board, WhiteTurn);

		if (Board.GameOver())
			{
			int Points = Board.GetPoints();
			if (WhiteTurn)
				asserta(Points > 0);
			else
				asserta(Points < 0);
			return Points;
			}

		Board.ToStr(PosId);
		PosIds.push_back(PosId);

		WhiteTurn = !WhiteTurn;
		}
	}
