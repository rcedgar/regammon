#ifndef bencher_h
#define bencher_h

#include "randev.h"
#include "pubeval.h"
#include "winrate.h"

class Bencher
	{
public:
	static FILE *m_f;

public:
	PubEval m_PE;
	RandEv m_RE;
	WinRate m_WR;
	string m_ProgressStr;

	unsigned m_Window;
	unsigned m_WarmupGameCount;
	unsigned m_BenchStep;
	unsigned m_MinWinPctRand;
	unsigned m_BenchGameCount_Rand;

public:
	unsigned m_LastBenchIndex;
	unsigned m_LastImprovedIndex;
	double m_LastWinPctRand;
	double m_LastWinPctPube;
	double m_LastPPGPube;
	double m_BestWinPctPube;
	double m_BestPPGPube;

public:
	Bencher()
		{
		m_ProgressStr = "Warming up";
		m_MinWinPctRand = 90;
		m_BenchGameCount_Rand = 100;
		m_Window = 5;
		m_WarmupGameCount = 100000;
		m_BenchStep = 50000;

		m_LastBenchIndex = 0;
		m_LastImprovedIndex = 0;
		m_LastWinPctPube = -9.9;
		m_LastPPGPube = -9.9;
		m_BestWinPctPube = -9.9;
		m_BestPPGPube = -9.9;

		if (optset_trainout)
			m_f = CreateStdioFile(opt(trainout));
		}

	~Bencher()
		{
		CloseStdioFile(m_f);
		m_f = 0;
		}

	bool DoBench(unsigned GameIndex, unsigned GameCount) const;
	bool OnGame(BgEval &Ev, unsigned GameIndex, unsigned GameCount);
	const string &GetProgressStr() const { return m_ProgressStr; }
	unsigned GetBenchCountPube(double PPG) const;
	unsigned GetBenchCountPube_Champ(double PPG) const;
	void Flush() const { if (m_f != 0) fflush(m_f); }
	};

#endif // bencher_h
