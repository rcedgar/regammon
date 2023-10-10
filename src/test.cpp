#include "myutils.h"
#include "bgboard.h"
#include "pubeval.h"

void cmd_test()
	{
	const string PosId = "4OsDEGiMz6gHAA";
	BgBoard Board;
	Board.FromStr(PosId);
	Board.m_Die1 = 3;
	Board.m_Die2 = 2;

	PubEval PE;
	unsigned Move = PE.GetBlackMove(Board);
	Board.DoBlackMove(Move);
	}
