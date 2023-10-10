#include "myutils.h"
#include "quarts.h"

static unsigned Run1(unsigned TossCount)
	{
	unsigned T = 0;
	for (unsigned i = 0; i < TossCount; ++i)
		if (randu32()%2 == 0)
			++T;
	return T;
	}

static void RunN(unsigned TossCount, unsigned N,
  float &Mean, float &StdDev)
	{
	vector<float> Ts;
	for (unsigned i = 0; i < N; ++i)
		{
		float T = (float) Run1(TossCount);
		Ts.push_back(T);
		}
	QuartsFloat Q;
	GetQuartsFloat(Ts, Q);
	Mean = Q.Avg;
	StdDev = Q.StdDev;
	}

void cmd_htsim()
	{
	vector<string> Fields;
	Split(opt(htsim), Fields, ',');
	asserta(SIZE(Fields) == 2);
	unsigned TossCount = StrToUint(Fields[0]);
	unsigned N = StrToUint(Fields[1]);

	float Mean;
	float StdDev;
	RunN(TossCount, N, Mean, StdDev);
	ProgressLog("%u tosses, %u runs, mean %.3f%% stddev %.3g (%.2f%%)\n",
	  TossCount, N, GetPct(Mean, TossCount), StdDev, GetPct(StdDev, Mean));
	}
