#include "myutils.h"
#include "nneval.h"
#include "bgeval.h"

static const unsigned MAX_PLIES = 10000;

static void RunGame(FILE *f, NNEval &Ev)
	{
	if (f == 0)
		return;

	BgBoard Board;
	Board.SetStart();
	bool FirstRoll = true;
	vector<float> OutputVec;
	for (unsigned Ply = 0; Ply < MAX_PLIES; ++Ply)
		{
		Board.RollDice(FirstRoll);
		unsigned Die1 = Board.m_Die1;
		unsigned Die2 = Board.m_Die2;
		FirstRoll = false;
		bool WhiteTurn = (Ply%2 == 0);
		if (WhiteTurn)
			Ev.DoWhiteMove(Board);
		else
			Ev.DoBlackMove(Board);

		string PosId;
		Board.GetPosId(PosId);
		Ev.m_NN.Evaluate(Board, WhiteTurn, OutputVec);

		float WhiteValue, BlackValue;
		if (WhiteTurn)
			{
			WhiteValue = Ev.m_NN.GetWhiteValueAfter(Board);
			BlackValue = Ev.m_NN.GetBlackValueBefore(Board);
			}
		else
			{
			WhiteValue = Ev.m_NN.GetWhiteValueBefore(Board);
			BlackValue = Ev.m_NN.GetBlackValueAfter(Board);
			}

		fprintf(f, "%s", PosId.c_str());
		fprintf(f, "\t%u-%u", Die1, Die2);
		for (unsigned i = 0; i < SIZE(OutputVec); ++i)
			fprintf(f, "\t%.4f", OutputVec[i]);
		fprintf(f, "\t%.4f", WhiteValue);
		fprintf(f, "\t%.4f", BlackValue);
		fprintf(f, "\n");

		if (Board.GameOver())
			return;
		}

	Die("MaxPlies");
	}

void cmd_nneval_game()
	{
	const string &NotUsed = opt(nneval_game);

	string EvalName = "bestnn";
	if (optset_eval_name)
		EvalName = opt(eval_name);
	NNEval &Ev = (NNEval &) *BgEval::CreateEval(EvalName);

	FILE *fTab = CreateStdioFile(opt(tabbedout));
	RunGame(fTab, Ev);
	CloseStdioFile(fTab);
	}
