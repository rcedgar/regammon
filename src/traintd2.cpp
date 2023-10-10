#include "myutils.h"
#include "bgboard.h"
#include "nneval.h"
#include "bencher.h"

#if 0
static unsigned g_GameIndex;
static unsigned g_PlyIndex;
static FILE *g_fTab;

static void RunGame(NNEval &Ev, vector<string> &PosIds, bool &Gammon)
	{
	PosIds.clear();

	BgBoard Board;
	Board.SetStart();

	g_PlyIndex = 0;
	string PosId;
	bool FirstRoll = true;
	for (;;)
		{
		Board.RollDice(FirstRoll);
		FirstRoll = false;

		Ev.DoMove(Board, true);

		if (Board.GameOver())
			{
			asserta(Board.IsWhiteWin());
			Gammon = Board.IsWhiteGammonWin();
			return;
			}

		Board.ToStr(PosId);
		PosIds.push_back(PosId);

		Board.Flip();
		++g_PlyIndex;
		}
	}

//static void TDTrain(BgNN &nn, const vector<string> &PosIds,
//  bool Gammon, float Alpha)
//	{
//	const int PlyCount = (int) SIZE(PosIds);
//	const unsigned OutputSize = nn.GetOutputSize();
//
//	vector<float> NextValues;
//	nn.GetWinOutputVec(true, Gammon, NextValues);
//	asserta(SIZE(NextValues) == OutputSize);
//
//	BgBoard Board;
//	for (int PlyIndex = PlyCount - 1; PlyIndex >= 0; --PlyIndex)
//		{
//		nn.FlipOutputVec(NextValues);
//		vector<float> ValuesBefore;
//		vector<float> ValuesAfter;
//		const string &PosId = PosIds[PlyIndex];
//		Board.FromStr(PosId);
//		if (g_fTab)
//			{
//			nn.Evaluate(Board, false, ValuesBefore);
//			asserta(SIZE(ValuesBefore) == OutputSize);
//			}
//		nn.Update(Board, false, NextValues, Alpha);
//		asserta(SIZE(NextValues) == OutputSize);
//		nn.Evaluate(Board, false, ValuesAfter);
//		asserta(SIZE(ValuesAfter) == OutputSize);
//		if (g_fTab)
//			{
//			fprintf(g_fTab, "%u", g_GameIndex);
//			fprintf(g_fTab, "\t%d", PlyIndex);
//			if (Gammon)
//				fprintf(g_fTab, "\tD");
//			else
//				fprintf(g_fTab, "\tS");
//			fprintf(g_fTab, "\t%s", PosId.c_str());
//			for (unsigned i = 0; i < OutputSize; ++i)
//				{
//				fprintf(g_fTab, "\t%6.4f", ValuesBefore[i]);
//				fprintf(g_fTab, ",%6.4f", NextValues[i]);
//				fprintf(g_fTab, ",%6.4f", ValuesAfter[i]);
//				}
//			fprintf(g_fTab, "\n");
//			}
//		NextValues = ValuesAfter;
//		}
//	if (g_fTab)
//		fprintf(g_fTab, "\n");
//	}

// With flipping
void cmd_traintd2()
	{
	const string EvalName = opt(traintd2);
	const unsigned GameCount = opt(games);
	const unsigned StartGame = opt(start_game);
	const unsigned SaveStep = opt(save_step);

	if (optset_tabbedout)
		g_fTab = CreateStdioFile(opt(tabbedout));

	NNEval &Ev = (NNEval &) *BgEval::CreateEval(EvalName);
	if (optset_initial_nn)
		Ev.m_NN.FromTabbedFile(opt(initial_nn));

	Bencher B;
	if (StartGame > 0)
		ProgressStep(0, GameCount, "Starting");
	for (g_GameIndex = StartGame; g_GameIndex < GameCount; ++g_GameIndex)
		{
		float Alpha = Interp(g_GameIndex, GameCount, alpha);

		const string &s = B.GetProgressStr();
		ProgressStep(g_GameIndex, GameCount, s.c_str());

		vector<string> PosIds;
		bool Gammon;
		RunGame(Ev, PosIds, Gammon);
		TDTrain(Ev.m_NN, PosIds, Gammon, Alpha);
		bool NewChamp = B.OnGame(Ev, g_GameIndex, GameCount);
		if (NewChamp)
			{
			string Comments;
			Ps(Comments, "game %u (best)", g_GameIndex);
			Ev.m_NN.ToTabbedFile(opt(best_nn), Comments);
			}
		if (optset_save_nn && g_GameIndex > 0 && g_GameIndex%SaveStep == 0)
			{
			string Comments;
			Ps(Comments, "save after game %u", g_GameIndex);
			Ev.m_NN.ToTabbedFile(opt(save_nn), Comments);
			}
		}

	string Comments;
	Ps(Comments, "final after games %u (best)", GameCount);
	Ev.m_NN.ToTabbedFile(opt(final_nn), Comments);

	B.OnAllDone();
	CloseStdioFile(g_fTab);
	}
#endif // 0
