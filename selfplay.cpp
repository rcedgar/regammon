#include "stdafx.h"
#include "gui.h"
#include "bgeval.h"

#define TRACE	0

static BgEval *Ev;

void ComputerMove(const string &PosId, bool White,
  int Die1, int Die2, string &PosIdAfter, string &MoveStr)
	{
	if (Ev == 0)
		Ev = BgEval::CreateEval("bestnn");

	BgBoard Board;
	Board.FromStr(PosId);
	Board.m_Die1 = unsigned(Die1);
	Board.m_Die2 = unsigned(Die2);
	unsigned Move = Ev->GetWhiteMove(Board);
	Board.MoveToStr(Move, MoveStr);
	Board.DoMove(Move, White);
	Board.ToStr(PosIdAfter);
	}
