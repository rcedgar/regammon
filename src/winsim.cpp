#include "myutils.h"
#include "quarts.h"

static unsigned pow2(unsigned k)
	{
	asserta(k <= 31);
	unsigned n = 1;
	for (unsigned i = 0; i < k; ++i)
		n *= 2;
	return n;
	}

static unsigned SumW1 = 0;
static unsigned SumW2 = 0;
static unsigned SumL1 = 0;
static unsigned SumL2 = 0;
static unsigned Total = 0;

static int GetPoints(unsigned M, unsigned TW1, unsigned TW2, unsigned TL1)
	{
	++Total;
	unsigned r = randu32()%M;
	if (r <= TW1)
		{
		++SumW1;
		return +1;
		}
	else if (r <= TW2)
		{
		++SumW2;
		return +2;
		}
	else if (r <= TL1)
		{
		++SumL1;
		return -1;
		}
	asserta(r < M);
	++SumL2;
	return -2;
	}

void cmd_winsim()
	{
	const string &s = opt(winsim);
	FILE *f = CreateStdioFile(opt(tabbedout));
	vector<string> Fields;
	Split(s, Fields, ',');
	unsigned W1 = 0;
	unsigned W2 = 0;
	unsigned L1 = 0;
	unsigned L2 = 0;
	if (SIZE(Fields) == 4)
		{
		W1 = StrToUint(Fields[0]);
		W2 = StrToUint(Fields[1]);
		L1 = StrToUint(Fields[3]);
		L2 = StrToUint(Fields[4]);
		}
	else if (SIZE(Fields) == 2)
		{
		W1 = StrToUint(Fields[0]);
		W2 = StrToUint(Fields[1]);
		L1 = W1;
		L2 = W2;
		}
	else
		asserta(false);

	unsigned N = W1 + W2 + L1 + L2;
	double PW1 = double(W1)/N;
	double PW2 = double(W2)/N;
	double PL1 = double(L1)/N;
	double PL2 = double(L2)/N;

	Log("PW1 %.4f, PW2 %.4f, PL1 %.4f, PL2 %.4f\n",
	  PW1, PW2, PL1, PL2);
	asserta(feq(PW1 + PW2 + PL1 + PL2, 1.0));

	const unsigned M = 100000000;
	const unsigned TW1 = unsigned(M*PW1 + 0.5);
	const unsigned TW2 = TW1 + unsigned(M*PW2 + 0.5);
	const unsigned TL1 = TW2 + unsigned(M*PL1 + 0.5);
	const unsigned TL2 = TL1 + unsigned(M*PL2 + 0.5);
	asserta(TL2 >= M - 1 && TL2 <= M + 1);

	const unsigned Iters = opt(simiters);

	Pf(f, "Games");
	Pf(f, "\tPts");
	Pf(f, "\tWins");
	Pf(f, "\tSDPts");
	Pf(f, "\tSDWins");
	Pf(f, "\tOnes");
	Pf(f, "\tTwos");
	Pf(f, "\tThrees");
	Pf(f, "\tPOne");
	Pf(f, "\tPTwo");
	Pf(f, "\tPThree");
	Pf(f, "\n");
	float Pts = 0.0f;
	float Wins = 0.0f;
	float PtsSD = 0.0f;
	float WinsSD = 0.0f;

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
		vector<float> MeanPointsVec;
		vector<float> MeanWinsVec;
		unsigned OneCount = 0;
		unsigned TwoCount = 0;
		unsigned ThreeCount = 0;
		for (unsigned Iter = 0; Iter < Iters; ++Iter)
			{
			ProgressStep(Iter, Iters, "Games %u", GameCount);
			float SumPoints = 0.0;
			float SumWins = 0.0;
			for (unsigned GameIndex = 0; GameIndex < GameCount; ++GameIndex)
				{
				int Points = GetPoints(M, TW1, TW2, TL1);
				SumPoints += float(Points);
				if (Points > 0)
					SumWins += 1.0f;
				}
			float MeanPoints = SumPoints/GameCount;
			float MeanWins = SumWins/GameCount;

			MeanPointsVec.push_back(MeanPoints);
			MeanWinsVec.push_back(MeanWins);
			if (MeanPoints > -0.05f && MeanPoints < 0.05f)
				++OneCount;
			if (MeanPoints > -0.005f && MeanPoints < 0.005f)
				++TwoCount;
			if (MeanPoints > -0.0005f && MeanPoints < 0.0005f)
				++ThreeCount;
			}
		QuartsFloat QPoints;
		QuartsFloat QWins;
		GetQuartsFloat(MeanPointsVec, QPoints);
		GetQuartsFloat(MeanWinsVec, QWins);

		double POne = double(OneCount)/Iters;
		double PTwo = double(TwoCount)/Iters;
		double PThree = double(ThreeCount)/Iters;

		Pts = QPoints.Avg;
		Wins = QWins.Avg;
		PtsSD = QPoints.StdDev;
		WinsSD = QWins.StdDev;

		Pf(f, "%u", GameCount);
		Pf(f, "\t%.4f", Pts);
		Pf(f, "\t%.4f", Wins);
		Pf(f, "\t%.3g", PtsSD);
		Pf(f, "\t%.3g", WinsSD);
		Pf(f, "\t%u", OneCount);
		Pf(f, "\t%u", TwoCount);
		Pf(f, "\t%u", ThreeCount);
		Pf(f, "\t%.4f", POne);
		Pf(f, "\t%.4f", PTwo);
		Pf(f, "\t%.4f", PThree);
		Pf(f, "\n");

		Progress("Pts %.4f(%.3g)", Pts, PtsSD);
		Progress(", %.4f(%.3g)\n", Wins, WinsSD);
		}

	double MPW1 = double(SumW1)/Total;
	double MPW2 =   double(SumW2)/Total;
	double MPL1 = double(SumL1)/Total;
	double MPL2 = double(SumL2)/Total;
	double Sum = PW1 + PW2 + PL1 + PL2;
	double MSum = MPW1 + MPW2 + MPL1 + MPL2;
	ProgressLog("%.4f %.4f %.4f %.4f (%.4f)\n", PW1, PW2, PL1, PL2, Sum);
	ProgressLog("%.4f %.4f %.4f %.4f (%.4f)\n", MPW1, MPW2, MPL1, MPL2, MSum);
	CloseStdioFile(f);
	}
