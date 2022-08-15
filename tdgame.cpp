#include "myutils.h"
#include "bgboard.h"
#include "nneval.h"
#include "bencher.h"
#include "selfgame.h"

void FlipPosIds(const vector<string> &PosIds,
  vector<string> &FlippedPosIds)
	{
	FlippedPosIds.clear();
	const unsigned N = SIZE(PosIds);
	for (unsigned i = 0; i < N; ++i)
		{
		BgBoard Board;
		Board.FromStr(PosIds[i]);
		Board.Flip();
		string FlippedPosId;
		Board.ToStr(FlippedPosId);
		FlippedPosIds.push_back(FlippedPosId);
		}
	}

void TD_Game(BgNN &nn,  bool Flip, const vector<string> &PosIds,
  int Points, float Lambda, float Alpha)
	{
	asserta(Lambda >= 0.0f && Lambda <= 1.0f);
	asserta(Lambda == 0.0f || Lambda == 1.0f);

	const int PlyCount = (int) SIZE(PosIds);
	const unsigned OutputSize = nn.GetOutputSize();

	bool WhiteWin = (Points > 0);
	if (Flip)
		asserta(WhiteWin);

	if (Points > 0)
		Points = -Points;
	bool Gammon = (Points > 1);

	vector<float> NextValues;
	nn.GetWinOutputVec(WhiteWin, Gammon, NextValues);
	asserta(SIZE(NextValues) == OutputSize);

	bool WhiteTurn = WhiteWin;
	BgBoard Board;
	for (int PlyIndex = PlyCount - 1; PlyIndex >= 0; --PlyIndex)
		{
		if (Flip)
			nn.FlipOutputVec(NextValues);

		vector<float> ValuesBefore;
		vector<float> ValuesAfter;
		const string &PosId = PosIds[PlyIndex];
		Board.FromStr(PosId);
		nn.Update(Board, WhiteTurn, NextValues, Alpha);
		asserta(SIZE(NextValues) == OutputSize);

		if (Lambda == 0.0f)
			{
			nn.Evaluate(Board, WhiteTurn, ValuesAfter);
			asserta(SIZE(ValuesAfter) == OutputSize);
			NextValues = ValuesAfter;
			}
		else if (Lambda == 1.0f)
			;
		else
			asserta(false);

		WhiteTurn = !WhiteTurn;
		}
	}
