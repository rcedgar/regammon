#include "myutils.h"
#include "bgeval.h"
#include "nneval.h"
#include "pubeval.h"
#include "randev.h"
#include "bgboard.h"
#include "winrate.h"

void TDW(FILE *fTab, NNEval &Ev, 
  const vector<vector<string> > &Games,
  const vector<bool> &EvWins,
  const vector<bool> &EvWhites,
  const vector<int> &PointsVec,
  float Alpha);

bool DoBench(unsigned GameIndex, unsigned GameCount);

static FILE *g_fTab;
static FILE *g_fTrain;
static unsigned g_GameIndex;

static bool RunGame(NNEval &Ev, PubEval &PE,
  bool EvFirst, bool EvWhite,
  vector<string> &PosIds, int &Points)
	{
	PosIds.clear();

	BgBoard Board;
	Board.SetStart();

	string PosId;
	bool WhiteTurn = (EvFirst == EvWhite);
	bool EvTurn = EvFirst;
	bool PEWhite = !EvWhite;
	bool FirstRoll = true;
	for (;;)
		{
		Board.RollDice(FirstRoll);
		FirstRoll = false;

		if (EvTurn)
			{
			Ev.DoMove(Board, EvWhite);
			Board.ToStr(PosId);
			PosIds.push_back(PosId);
			}
		else
			{
			if (PEWhite)
				PE.DoWhiteMove(Board);
			else
				{
				Board.Flip();
				PE.DoWhiteMove(Board);
				Board.Flip();
				}
			}

		if (Board.GameOver())
			{
			bool WhiteWin = WhiteTurn;
			bool EvWin = (WhiteWin == EvWhite);
			Points = Board.GetPoints();
			return EvWin;
			}

		EvTurn = !EvTurn;
		WhiteTurn = !WhiteTurn;
		}
	}

void cmd_beatpube()
	{
	const string EvalName = opt(beatpube);
	const unsigned GameCount = opt(games);

	if (optset_tabbedout)
		g_fTab = CreateStdioFile(opt(tabbedout));

	if (optset_trainout)
		g_fTrain = CreateStdioFile(opt(trainout));

	NNEval &Ev = (NNEval &) *BgEval::CreateEval(EvalName);
	BgNN &nn = Ev.m_NN;

	if (optset_initial_nn)
		{
		const string &FileName = opt(initial_nn);
		ProgressLog("Loading %s\n", FileName.c_str());
		nn.FromTabbedFile(FileName);
		}

	BgEval &Rande = *BgEval::CreateEval("rand");
	PubEval &PE = *new PubEval;

	unsigned BenchGameCount = 1000;

	double BestWinPctPube = 0.0;
	unsigned LastImprovedIndex = 0;

	double WinPctRand = 0.0;
	double WinPctPube = 0.0;
	double BestPPG = -9.9;
	WinRate WR;

	vector<vector<string> > Games;
	vector<bool> EvFirsts;
	vector<bool> EvWhites;
	vector<bool> EvWins;
	vector<int> PointsVec;
	unsigned AccumEvWinCount = 0;
	unsigned AccumEvLossCount = 0;
	unsigned TotalEvWinCount = 0;
	unsigned TotalEvLossCount = 0;
	for (g_GameIndex = 0; g_GameIndex < GameCount; ++g_GameIndex)
		{
		float Alpha = Interp(g_GameIndex, GameCount, alpha);
		bool EvFirst = ((g_GameIndex)%2 == 0);
		bool EvWhite = ((g_GameIndex/2)%2 == 0);

		ProgressStep(g_GameIndex, GameCount,
		  "games %u (%u) alpha %.3g W%u rand %.1f%%, pube %.1f%% (%.1f%%, %+.3f)",
			g_GameIndex, LastImprovedIndex, Alpha,
			TotalEvWinCount,
			WinPctRand, WinPctPube, BestWinPctPube, BestPPG);

		vector<string> PosIds;
		int Points;
		bool EvWin = RunGame(Ev, PE, EvFirst, EvWhite, PosIds, Points);
		Games.push_back(PosIds);
		EvFirsts.push_back(EvFirst);
		EvWins.push_back(EvWin);
		EvWhites.push_back(EvWhite);
		PointsVec.push_back(Points);

		if (EvWin)
			{
			++AccumEvWinCount;
			++TotalEvWinCount;
			}
		else
			{
			++AccumEvLossCount;
			++TotalEvLossCount;
			}

		if (AccumEvWinCount >= 1 && AccumEvLossCount >= 1)
			{
			TDW(g_fTab, Ev, Games, EvWins, EvWhites, PointsVec, Alpha);
			Games.clear();
			EvWins.clear();
			EvWhites.clear();
			PointsVec.clear();
			AccumEvWinCount = 0;
			AccumEvLossCount = 0;
			}

		bool Bench = DoBench(g_GameIndex, GameCount);
		if (Bench)
			{
			WR.Run(Ev, PE, BenchGameCount, false);
			WinPctPube = WR.GetAWinPct();
			if (BenchGameCount == 1000 && WinPctPube > 45.0)
				{
				BenchGameCount = 40000;
				WR.Run(Ev, PE, BenchGameCount, false);
				WinPctPube = WR.GetAWinPct();
				}
			double PPG = WR.GetPPG();
			if (WinPctRand < 90.0 || WinPctPube < 20.0)
				{
				WR.Run(Ev, Rande, BenchGameCount, false);
				WinPctRand = WR.GetAWinPct();
				}

			bool NewChamp = false;
			if (WinPctPube > BestWinPctPube)
				BestWinPctPube = WinPctPube;

			if (PPG > BestPPG)
				BestPPG = PPG;

			if (NewChamp)
				{
				LastImprovedIndex = g_GameIndex;
				Ev.m_NN.ToTabbedFile(opt(best_nn));
				}

			if (g_fTrain)
				{
				static bool HdrDone = false;
				if (!HdrDone)
					{
					fprintf(g_fTrain, "Game");
					fprintf(g_fTrain, "\tGames");
					fprintf(g_fTrain, "\tLastI");
					fprintf(g_fTrain, "\tBenchN");
					fprintf(g_fTrain, "\tRand");
					fprintf(g_fTrain, "\tPube");
					fprintf(g_fTrain, "\tBest");
					fprintf(g_fTrain, "\tppg");
					fprintf(g_fTrain, "\tppg+");
					fprintf(g_fTrain, "\n");
					HdrDone = true;
					}
				fprintf(g_fTrain, "%u", g_GameIndex);
				fprintf(g_fTrain, "\t%u", GameCount);
				fprintf(g_fTrain, "\t%u", LastImprovedIndex);
				fprintf(g_fTrain, "\t%u", BenchGameCount);
				fprintf(g_fTrain, "\t%.1f", WinPctRand);
				fprintf(g_fTrain, "\t%.1f", WinPctPube);
				fprintf(g_fTrain, "\t%.1f", BestWinPctPube);
				fprintf(g_fTrain, "\t%.3f", PPG);
				fprintf(g_fTrain, "\t%.3f", BestPPG);
				fprintf(g_fTrain, "\n");
				fflush(g_fTrain);
				}
			}
		}

	Ev.m_NN.ToTabbedFile(opt(final_nn));

	CloseStdioFile(g_fTab);
	CloseStdioFile(g_fTrain);
	}
