#include "myutils.h"
#include "nneval.h"
#include "bgboard.h"

static unsigned g_GameIndex;

static void TDW1(FILE *fTab, NNEval &Ev,
  const vector<string> &PosIds, 
  bool EvWhite, bool EvWin, bool Gammon,
  float Alpha)
	{
	BgNN &nn = Ev.m_NN;
	const unsigned NI = nn.GetOutputSize();
	const int PosCount = (int) SIZE(PosIds);
	vector<float> TargetOutputVec;
	nn.GetWinOutputVec(EvWin, Gammon, TargetOutputVec);
	asserta(SIZE(TargetOutputVec) == NI);
	const char WLChar = (EvWin ? 'W' : 'L');

	BgBoard Board;
	for (int Index = PosCount - 1; Index >= 0; --Index)
		{
		const string &PosId = PosIds[Index];
		Board.FromStr(PosId);

		nn.Update(Board, !EvWhite, TargetOutputVec, Alpha);

		vector<float> UpdatedOutputVec;
		nn.Evaluate(Board, !EvWhite, UpdatedOutputVec);
		asserta(SIZE(UpdatedOutputVec) == NI);

		if (fTab)
			{
			fprintf(fTab, "%u", g_GameIndex);
			fprintf(fTab, "\t%d", Index);
			fprintf(fTab, "\t%c", WLChar);
			fprintf(fTab, "\t%s", PosId.c_str());
			for (unsigned i = 0; i < NI; ++i)
				fprintf(fTab, "\t%.4f(%.4f)",
				  TargetOutputVec[i], UpdatedOutputVec[i]);
			fprintf(fTab, "\n");
			}

		TargetOutputVec = UpdatedOutputVec;
		}
	if (fTab)
		fprintf(fTab, "\n");
	++g_GameIndex;
	}

void TDW(FILE *fTab, NNEval &Ev,
  const vector<vector<string> > &Games,
  const vector<bool> &EvWins,
  const vector<bool> &EvWhites,
  const vector<int> &PointsVec,
  float Alpha)
	{
	const unsigned N = SIZE(Games);
	asserta(SIZE(EvWins) == N);
	asserta(SIZE(EvWhites) == N);
	asserta(SIZE(PointsVec) == N);

	vector<vector<string> > EvWGames;
	vector<vector<string> > EvLGames;
	vector<bool> EvWWhites;
	vector<bool> EvLWhites;
	vector<bool> EvWGammons;
	vector<bool> EvLGammons;
	for (unsigned i = 0; i < N; ++i)
		{
		const vector<string> &Game = Games[i];
		bool EvWin = EvWins[i];
		bool EvWhite = EvWhites[i];
		int Points = PointsVec[i];
		if (Points < 0)
			Points = -Points;
		bool Gammon = (Points > 1);
		if (EvWin)
			{
			EvWGames.push_back(Game);
			EvWWhites.push_back(EvWhite);
			EvWGammons.push_back(Gammon);
			}
		else
			{
			EvLGames.push_back(Game);
			EvLWhites.push_back(EvWhite);
			EvLGammons.push_back(Gammon);
			}
		}

	const unsigned NW = SIZE(EvWGames);
	const unsigned NL = SIZE(EvLGames);
	asserta(NW + NL == N);
	asserta(NW > 0 && NL > 0);
	asserta(SIZE(EvWWhites) == NW);
	asserta(SIZE(EvLWhites) == NL);
	asserta(SIZE(EvWGammons) == NW);
	asserta(SIZE(EvLGammons) == NL);

	float AlphaW = (Alpha*NL)/N;
	float AlphaL = (Alpha*NW)/N;

	for (unsigned i = 0; i < NW; ++i)
		TDW1(fTab, Ev, EvWGames[i], EvWWhites[i], true,
		  EvWGammons[i], AlphaW);

	for (unsigned i = 0; i < NL; ++i)
		TDW1(fTab, Ev, EvLGames[i], EvLWhites[i], false,
		  EvLGammons[i], AlphaL);
	}
