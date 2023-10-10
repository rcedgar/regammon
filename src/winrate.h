#ifndef winrate_h
#define winrate_h

#include "bgeval.h"

class WinRate
	{
public:
	BgEval *m_A;
	BgEval *m_B;
	unsigned m_W1;			// A single wins
	unsigned m_W2;			// A double wins
	unsigned m_W3;			// A triple wins
	unsigned m_L1;			// A single losses
	unsigned m_L2;			// A double losses
	unsigned m_L3;			// A triple losses
	unsigned m_AWFirst;		// A wins playing first
	unsigned m_AWSecond;	// A wins playing second
	unsigned m_ALFirst;		// A losses playing first
	unsigned m_ALSecond;	// B losses playing second
	int m_TotalPointsFirst;
	int m_TotalPointsFirst2;
	unsigned m_GameIndex;
	unsigned m_GameCount;

public:
	WinRate()
		{
		Zero();
		}

	void Zero()
		{
		m_A = 0;
		m_B = 0;
		m_W1 = 0;
		m_W2 = 0;
		m_W3 = 0;
		m_L1 = 0;
		m_L2 = 0;
		m_L3 = 0;
		m_AWFirst = 0;
		m_AWSecond = 0;
		m_ALFirst = 0;
		m_ALSecond = 0;
		m_GameIndex = 0;
		m_GameCount = 0;
		m_TotalPointsFirst = 0;
		m_TotalPointsFirst2 = 0;
		}

	double Run(BgEval &EvA, BgEval &EvB, unsigned GameCount,
	  bool ShowProgress);
	void Update(bool AFirst, bool FirstWin, unsigned Points);
	unsigned GetN() const;
	unsigned GetWinCount() const { return m_W1 + m_W2 + m_W3; }
	unsigned GetLossCount() const { return m_L1 + m_L2 + m_L3; }
	double GetAWinPct() const;
	double GetAWinPct1st() const;
	double GetAWinPct2nd() const;
	double GetPPG() const;
	double GetPPG2() const;
	double GetPPGFirst() const;
	double GetPPG2First() const;
	void TabbedHdr(FILE *f) const;
	void ToTabbed(FILE *f) const;
	};

#endif // winrate_h
