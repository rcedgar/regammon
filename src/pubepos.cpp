#include "myutils.h"
#include "bgboard.h"
#include "pubeval.h"

static bool MoveMan(BgBoard &Board)
	{
	vector<unsigned> Points;
	for (unsigned i = 1; i <= 24; ++i)
		if (Board.m_b[i] > 2)
			Points.push_back(i);
	if (SIZE(Points) < 2)
		return false;

	Shuffle(Points);
	unsigned i1 = Points[0];
	unsigned i2 = Points[1];
	--Board.m_b[i1];
	++Board.m_b[i2];
	Board.AssertValid();
	return true;
	}

static bool MovePoint(BgBoard &Board)
	{
	vector<unsigned> Points;
	vector<unsigned> Voids;
	for (unsigned i = 1; i <= 24; ++i)
		{
		if (Board.m_b[i] >= 2)
			Points.push_back(i);
		if (Board.m_b[i] == 0 && Board.m_w[INV(i)] == 0)
			Voids.push_back(i);
		}

	if (SIZE(Points) == 0 || SIZE(Voids) == 0)
		return false;

	Shuffle(Points);
	Shuffle(Voids);
	unsigned Point = Points[0];
	unsigned Void = Voids[0];
	unsigned n = Board.m_b[Point];
	asserta(n >= 2);
	asserta(Board.m_b[Void] == 0 && Board.m_w[INV(Void)] == 0);
	Board.m_b[Point] = 0;
	Board.m_b[Void] = n;
	return true;
	}

void cmd_pubepos()
	{
	const string &PosId = opt(pubepos);
	BgBoard Board;
	Board.FromStr(PosId);

	PubEval PE;
	float Value = PE.GetWhiteValueAfter(Board);

	Board.LogMe();
	Log("Value %.1f\n", Value);

	string s;
	bool Ok;
	for (unsigned Iter = 0; Iter < 100; ++Iter)
		{
		Ok = MoveMan(Board);
		if (Ok)
			{
			Board.ToStr(s);
			Value = PE.GetWhiteValueAfter(Board);
			Log("%s %.3f\n", s.c_str(), Value);
			}

		Ok = MovePoint(Board);
		if (Ok)
			{
			Board.ToStr(s);
			Value = PE.GetWhiteValueAfter(Board);
			Log("%s %.3f\n", s.c_str(), Value);
			}
		}

	Board.LogMe();
	}
