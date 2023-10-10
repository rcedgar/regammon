#include "myutils.h"
#include "bencher.h"

FILE *Bencher::m_f = 0;

bool Bencher::DoBench(unsigned GameIndex, unsigned GameCount) const
	{
	if (GameIndex == 0 || opt(benchgames) == 0)
		return false;
	if (GameIndex + 1 == GameCount)
		return true;
	if (GameIndex < m_WarmupGameCount)
		return GameIndex%10000 == 0;
	return GameIndex%m_BenchStep == 0;
	}

bool Bencher::OnGame(BgEval &Ev, unsigned GameIndex, unsigned GameCount)
	{
	if (!DoBench(GameIndex, GameCount))
		return false;

	m_LastBenchIndex = GameIndex;
	if (m_LastWinPctRand < m_MinWinPctRand)
		{
		m_WR.Run(Ev, m_RE, m_BenchGameCount_Rand, false);
		m_LastWinPctRand = m_WR.GetAWinPct();
		Ps(m_ProgressStr, "rand %.1f%%", m_LastWinPctRand);
		Pf(m_f, "@RAND");
		Pf(m_f, "\tgame=%u", GameIndex);
		Pf(m_f, "\trand=%.1f", m_LastWinPctRand);
		Pf(m_f, "\n");
		Flush();
		return false;
		}

	bool NewChamp = false;
	unsigned BenchGameCount = GetBenchCountPube(m_BestPPGPube);
	m_WR.Run(Ev, m_PE, BenchGameCount, false);
	double PPG = m_WR.GetPPG2();
	m_LastPPGPube = PPG;
	m_LastWinPctPube = m_WR.GetAWinPct();
	string TimeStr;
	Pf(m_f, "@PUBE=%s", GetElapsedTimeStr(TimeStr));
	Pf(m_f, "\tgame=%u", GameIndex);
	Pf(m_f, "\tbenchgames=%u", BenchGameCount);
	Pf(m_f, "\tchamp=%u", m_LastImprovedIndex);
	Pf(m_f, "\twin=%.1f", m_LastWinPctPube);
	Pf(m_f, "\tWIN=%.1f", m_BestWinPctPube);
	Pf(m_f, "\tppg=%+.2f", m_LastPPGPube);
	Pf(m_f, "\tPPG=%+.2f", m_BestPPGPube);
	Pf(m_f, "\n");
	Flush();
	if (m_LastPPGPube > m_BestPPGPube)
		{
		unsigned BenchGameCountChamp = GetBenchCountPube_Champ(m_BestPPGPube);
		if (BenchGameCountChamp > 0)
			{
			m_WR.Run(Ev, m_PE, BenchGameCountChamp, false);
			PPG = m_WR.GetPPG2();
			string TimeStr;
			Pf(m_f, "@CHAMP=%s", GetElapsedTimeStr(TimeStr));
			Pf(m_f, "\tchampgames=%u", BenchGameCountChamp);
			Pf(m_f, "\tgame=%u", GameIndex);
			Pf(m_f, "\twin=%.1f", m_WR.GetAWinPct());
			Pf(m_f, "\tWIN=%.1f", m_BestWinPctPube);
			Pf(m_f, "\tppg=%+.2f", PPG);
			Pf(m_f, "\tPPG=%+.2f", m_BestPPGPube);
			if (PPG > m_BestPPGPube)
				Pf(m_f, "\tnewchamp=yes");
			else
				Pf(m_f, "\tnewchamp=no");
			Pf(m_f, "\n");
			Flush();
			}

		if (PPG > m_BestPPGPube)
			{
			NewChamp = true;
			m_LastImprovedIndex = GameIndex;
			m_BestPPGPube = PPG;
			m_BestWinPctPube = m_WR.GetAWinPct();
			}
		}

	if (m_BestPPGPube > 0.0)
		Ps(m_ProgressStr, "ppg %+.2f (%+.2f)", PPG, m_BestPPGPube);
	else
		Ps(m_ProgressStr, "ppg %+.2f", PPG);

	return NewChamp;
	}

unsigned Bencher::GetBenchCountPube(double PPG) const
	{
	if (PPG <= 0.0)
		return 1000;
	else if (PPG <= 2.0)
		return 10000;
	else if (PPG <= 3.0)
		return 20000;
	return 40000;
	}

unsigned Bencher::GetBenchCountPube_Champ(double PPG) const
	{
	if (PPG <= 3.0)
		return 0;
	return 100000;
	}
