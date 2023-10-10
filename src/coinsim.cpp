#include "myutils.h"
#include "quarts.h"

/***
Coin toss model.
Win prob P=0.5.

For large N, measured P is approximately normal with:
	mean=P
	std.dev=sqrt(P(1-P)/N).
***/

static unsigned pow2(unsigned k)
	{
	asserta(k <= 31);
	unsigned n = 1;
	for (unsigned i = 0; i < k; ++i)
		n *= 2;
	return n;
	}

static double N1;
static double N2;

static unsigned GetPoints(unsigned M, unsigned T1)
	{
	unsigned r = randu32()%M;
	if (r <= T1)
		{
		++N1;
		return 1;
		}
	++N2;
	return 2;
	}

static float GetMean3(unsigned GameCount, unsigned M, unsigned T1)
	{
	float Sum = 0.0f;
	for (unsigned i = 0; i < GameCount; ++i)
		{
		unsigned Points = GetPoints(M, T1);
		if (randu32()%2 == 0)
			Sum += float(Points);
		else
			Sum -= float(Points);
		}
	return Sum/float(GameCount);
	}

static float GetMean(unsigned GameCount)
	{
	unsigned n = 0;
	for (unsigned i = 0; i < GameCount; ++i)
		if (randu32()%2 == 0)
			++n;
	return float(n)/float(GameCount);
	}

void cmd_coinsim()
	{
	const string &s = opt(coinsim);

	double P1, P2;
	unsigned M, T1;
	vector<string> Fields;
	if (s != "-")
		{
		Split(s, Fields, ',');
		asserta(SIZE(Fields) == 2);
		unsigned W1 = StrToUint(Fields[0]);
		unsigned W2 = StrToUint(Fields[1]);

		unsigned N = W1 + W2;
		P1 = double(W1)/N;
		P2 = double(W2)/N;
		M = 100000000;
		T1 = unsigned(M*P1 + 0.5);
		}

	FILE *f = CreateStdioFile(opt(tabbedout));

	const unsigned Iters = opt(simiters);
	Pf(f, "Games");
	Pf(f, "\tAvg");
	Pf(f, "\tStdDev");
	Pf(f, "\tStdDevA");
	Pf(f, "\tOnes");
	Pf(f, "\tTwos");
	Pf(f, "\tThrees");
	Pf(f, "\tPOne");
	Pf(f, "\tPTwo");
	Pf(f, "\tPThree");
	Pf(f, "\n");

	vector<unsigned> GameCounts;
	for (unsigned k = 8; k <= 20; ++k)
		GameCounts.push_back(pow2(k));
	GameCounts.push_back(10000);
	GameCounts.push_back(40000);
	GameCounts.push_back(100000);
	const unsigned G = SIZE(GameCounts);

	for (unsigned i = 0; i < G; ++i)
		{
		unsigned GameCount = GameCounts[i];
		unsigned OneCount = 0;
		unsigned TwoCount = 0;
		unsigned ThreeCount = 0;
		vector<float> Means;
		for (unsigned Iter = 0; Iter < Iters; ++Iter)
			{
			ProgressStep(Iter, Iters, "Games %u", GameCount);
			if (s == "-")
				{
				float Mean = GetMean(GameCount);
				Means.push_back(Mean);
				if (Mean >= 0.45f && Mean < 0.55f)
					++OneCount;
				if (Mean >= 0.495f && Mean < 0.505f)
					++TwoCount;
				if (Mean >= 0.4995f && Mean < 0.5005f)
					++ThreeCount;
				}
			else
				{
				float Mean = GetMean3(GameCount, M, T1);
				Means.push_back(Mean);
				if (Mean > -0.05f && Mean < 0.05f)
					++OneCount;
				if (Mean >= -0.005f && Mean < 0.005f)
					++TwoCount;
				if (Mean >= -0.0005f && Mean < 0.0005f)
					++ThreeCount;
				}
			}

		QuartsFloat Q;
		GetQuartsFloat(Means, Q);

		double POne = double(OneCount)/Iters;
		double PTwo = double(TwoCount)/Iters;
		double PThree = double(ThreeCount)/Iters;

		double StdDevA = sqrt(0.4/GameCount);

		Pf(f, "%u", GameCount);
		Pf(f, "\t%.4f", Q.Avg);
		Pf(f, "\t%.3g", Q.StdDev);
		Pf(f, "\t%.3g", StdDevA);
		Pf(f, "\t%u", OneCount);
		Pf(f, "\t%u", TwoCount);
		Pf(f, "\t%u", ThreeCount);
		Pf(f, "\t%.4f", POne);
		Pf(f, "\t%.4f", PTwo);
		Pf(f, "\t%.4f", PThree);
		Pf(f, "\n");
		}

	if (s != "-")
		{
		double AN = N1 + N2;
		double AP1 = N1/AN;
		double AP2 = N2/AN;

		Log("N1 = %.3g, N2 = %.3g\n", N1, N2);
		ProgressLog("P1 %.4f, %.4f\n", P1, AP1);
		ProgressLog("P2 %.4f, %.4f\n", P2, AP2);
		}

	CloseStdioFile(f);
	}
