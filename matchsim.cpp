#include "myutils.h"

static void GetPoints(double PWin1, double PDouble,
  unsigned &Pts, bool &Win1)
	{
	const unsigned M = 100049;
	Win1 = (randu32()%M < (PWin1*M));
	Pts = (randu32()%M < (PDouble*M)) ? 1 : 2;
	}

static bool SimMatch(unsigned MatchPoints, double P1, double W1)
	{
	unsigned Pts1 = 0;
	unsigned Pts2 = 0;
	for (;;)
		{
		unsigned Pts;
		bool Win1;
		GetPoints(P1, W1, Pts, Win1);
		if (Win1)
			Pts1 += unsigned(Pts);
		else
			Pts2 += Pts;
		if (Pts1 >= MatchPoints)
			return true;
		else if (Pts2 >= MatchPoints)
			return false;
		}
	}

void cmd_matchsim()
	{
	const unsigned MatchPoints = StrToUint(opt(matchsim));
	const double PWin1 = opt(pwin1);
	const double PDouble = opt(pdouble);
	const unsigned Iters = opt(simiters);

	unsigned M1Count = 0;
	double PM1 = 0.0;
	for (unsigned Iter = 0; Iter < Iters; ++Iter)
		{
		ProgressStep(Iter, Iters, "Running", PM1);
		bool M1 = SimMatch(MatchPoints, PWin1, PDouble);
		if (M1)
			++M1Count;
		PM1 = double(M1Count)/(Iter+1);
		}

	double PSingle = 1.0 - PDouble;
	double AbsPPG = PSingle + 2*PDouble;
	double PWin2 = 1.0 - PWin1;
	double PPG = PWin1*AbsPPG - PWin2*AbsPPG;
	ProgressLog("PM1=%.4f ppg=%+.4f\n", PM1, PPG);
	}
