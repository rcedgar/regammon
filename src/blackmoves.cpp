#include "myutils.h"
#include "bgeval.h"
#include "bgboard.h"
#include "sort.h"

void cmd_blackmoves()
	{
	const string &SketchFileName = opt(blackmoves);

	BgBoard InitialBoard;
	InitialBoard.FromSketchFile(SketchFileName);
	InitialBoard.LogMe();

	string Dice = "3-1";
	if (optset_dice)
		Dice = opt(dice);
	vector<string> Fields;
	Split(Dice, Fields, '-');
	asserta(SIZE(Fields) == 2);

	InitialBoard.m_Die1 = StrToUint(Fields[0]);
	InitialBoard.m_Die2 = StrToUint(Fields[1]);

	vector<unsigned> Moves;
	InitialBoard.GetBlackMoves(Moves);
	const size_t N = Moves.size();
	for (size_t i = 0; i < N; ++i)
		{
		unsigned Move = Moves[i];

		BgBoard NextBoard;
		NextBoard.Copy(InitialBoard);
		InitialBoard.DoBlackMove(Move);
		NextBoard.LogMe();
		}
	}
