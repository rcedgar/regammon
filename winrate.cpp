#include "myutils.h"
#include "bgeval.h"
#include "bgeval1.h"
#include "bgboard.h"
#include "winrate.h"
#include "omplock.h"

static const unsigned MAX_PLIES = 100000;

static bool RunGame(BgEval &E1, BgEval &E2, unsigned &Points)
	{
	Points = UINT_MAX;
	BgBoard Board;
	Board.SetStart();
	bool FirstRoll = true;
	for (unsigned Ply = 0; Ply < MAX_PLIES; ++Ply)
		{
		Board.RollDice(FirstRoll);
		FirstRoll = false;
		bool E1Turn = (Ply%2 == 0);
		if (E1Turn)
			E1.DoWhiteMove(Board);
		else
			E2.DoWhiteMove(Board);

		if (Board.GameOver())
			{
			int iPoints = Board.GetPoints();
			asserta(iPoints >= 1 && iPoints <= 3);
			Points = unsigned(iPoints);
			return E1Turn;
			}

		Board.Invert();
		asserta(!Board.GameOver());
		}

	Die("MaxPlies %s vs %s", E1.GetName(), E2.GetName());
	return false;
	}

void WinRate::Update(bool AFirst, bool FirstWin, unsigned Points)
	{
	bool AWin = (FirstWin == AFirst);
	if (AWin)
		{
		if (Points == 3)
			++m_W3;
		else if (Points == 2)
			++m_W2;
		else if (Points == 1)
			++m_W1;
		else
			asserta(false);
		}
	else
		{
		if (Points == 3)
			++m_L3;
		else if (Points == 2)
			++m_L2;
		else if (Points == 1)
			++m_L1;
		else
			asserta(false);
		}

	if (AFirst)
		{
		if (AWin)
			++m_AWFirst;
		else
			++m_ALFirst;
		}
	else
		{
		if (AWin)
			++m_AWSecond;
		else
			++m_ALSecond;
		}
	}

double WinRate::Run(BgEval &EvA, BgEval &EvB, unsigned GameCount,
  bool ShowProgress)
	{
	Zero();

	m_A = &EvA;
	m_B = &EvB;
	m_GameCount = GameCount;

	const char *NameA = EvA.GetName();
	const char *NameB = EvB.GetName();

	for (m_GameIndex = 0; m_GameIndex < GameCount; ++m_GameIndex)
		{
		if (ShowProgress)
			ProgressStep(m_GameIndex, m_GameCount, "%s vs %s, %u games, wins %.1f%% (%.4f ppg)",
			  NameA, NameB, m_GameIndex + 1, GetAWinPct(), GetPPG2());

		bool FirstWin;
		unsigned Points = UINT_MAX;
		bool AFirst = (m_GameIndex%2 == 0);
		if (AFirst)
			FirstWin = RunGame(EvA, EvB, Points);
		else
			FirstWin = RunGame(EvB, EvA, Points);
		Update(AFirst, FirstWin, Points);
		if (FirstWin)
			m_TotalPointsFirst += Points;
		else
			m_TotalPointsFirst -= Points;
		if (Points == 3)
			Points = 2;
		if (FirstWin)
			m_TotalPointsFirst2 += Points;
		else
			m_TotalPointsFirst2 -= Points;
		}
	double Pct = GetAWinPct();
	return Pct;
	}

unsigned WinRate::GetN() const
	{
	unsigned N = GetWinCount() + GetLossCount();
	asserta(N == m_AWFirst + m_AWSecond + m_ALFirst + m_ALSecond);
	return N;
	}

double WinRate::GetAWinPct() const
	{
	unsigned N = GetN();
	unsigned W = GetWinCount();
	unsigned L = GetLossCount();
	asserta(W + L == N);
	double Pct = GetPct(W, N);
	return Pct;
	}

double WinRate::GetAWinPct1st() const
	{
	unsigned W = m_AWFirst;
	unsigned L = m_ALFirst;
	unsigned N = W + L;
	double Pct = GetPct(W, N);
	return Pct;
	}

double WinRate::GetAWinPct2nd() const
	{
	unsigned W = m_AWSecond;
	unsigned L = m_ALSecond;
	unsigned N = W + L;
	double Pct = GetPct(W, N);
	return Pct;
	}

double WinRate::GetPPG() const
	{
	unsigned N = GetN();
	if (N == 0)
		return 0.0;

	double PointsA = double(m_W1 + 2*m_W2 + 3*m_W3);
	double PointsB = double(m_L1 + 2*m_L2 + 3*m_L3);
	double PPG = (PointsA - PointsB)/N;
	return PPG;
	}

double WinRate::GetPPG2() const
	{
	unsigned N = GetN();
	if (N == 0)
		return 0.0;

	double PointsA = double(m_W1 + 2*m_W2 + 2*m_W3);
	double PointsB = double(m_L1 + 2*m_L2 + 2*m_L3);
	double PPG = (PointsA - PointsB)/N;
	return PPG;
	}

double WinRate::GetPPGFirst() const
	{
	unsigned N = GetN();
	if (N == 0)
		return 0.0;

	double PPG = double(m_TotalPointsFirst)/N;
	return PPG;
	}

double WinRate::GetPPG2First() const
	{
	unsigned N = GetN();
	if (N == 0)
		return 0.0;

	double PPG = double(m_TotalPointsFirst2)/N;
	return PPG;
	}

void WinRate::TabbedHdr(FILE *f) const
	{
	if (f == 0)
		return;

	fprintf(f, "A");
	fprintf(f, "\tB");
	fprintf(f, "\tGames");
	fprintf(f, "\tW1");
	fprintf(f, "\tW2");
	fprintf(f, "\tW3");
	fprintf(f, "\tL1");
	fprintf(f, "\tL2");
	fprintf(f, "\tL3");
	fprintf(f, "\tW1st");
	fprintf(f, "\tW2nd");
	fprintf(f, "\tPts1st");
	fprintf(f, "\tPPG1st");
	fprintf(f, "\tPPG2nd");
	fprintf(f, "\tWPct");
	fprintf(f, "\tWPct1st");
	fprintf(f, "\tWPct2nd");
	fprintf(f, "\tPPG");
	fprintf(f, "\tPPG2");
	fprintf(f, "\n");
	}

void WinRate::ToTabbed(FILE *f) const
	{
	if (f == 0)
		return;

	fprintf(f, "%s", m_A->GetName());
	fprintf(f, "\t%s", m_B->GetName());
	fprintf(f, "\t%u", GetN());
	fprintf(f, "\t%u", m_W1);
	fprintf(f, "\t%u", m_W2);
	fprintf(f, "\t%u", m_W3);
	fprintf(f, "\t%u", m_L1);
	fprintf(f, "\t%u", m_L2);
	fprintf(f, "\t%u", m_L3);
	fprintf(f, "\t%u", m_AWFirst);
	fprintf(f, "\t%u", m_AWSecond);
	fprintf(f, "\t%+d", m_TotalPointsFirst);
	fprintf(f, "\t%+.4f", GetPPGFirst());
	fprintf(f, "\t%+.4f", GetPPG2First());
	fprintf(f, "\t%.1f", GetAWinPct());
	fprintf(f, "\t%.1f", GetAWinPct1st());
	fprintf(f, "\t%.1f", GetAWinPct2nd());
	fprintf(f, "\t%+.4f", GetPPG());
	fprintf(f, "\t%+.4f", GetPPG2());
	fprintf(f, "\n");
	}

void cmd_winrate()
	{
	string NamesStr = opt(winrate);
	const unsigned GameCount = opt(games);
	FILE *fTab = CreateStdioFile(opt(tabbedout));
	bool ShowProgress = (GameCount >= 2000);
	ShowProgress = true;

	vector<string> Names;
	Split(NamesStr, Names, ',');
	const unsigned N = SIZE(Names);
	vector<BgEval *> BEs;
	for (unsigned i = 0; i < N; ++i)
		{
		const string &Name = Names[i];
		BgEval *Ev = BgEval::CreateEval(Name);
		BEs.push_back(Ev);
		}

	WinRate WR;
	WR.TabbedHdr(fTab);
	for (unsigned i = 0; i < N; ++i)
		{
		BgEval &EvA = *BEs[i];
		for (unsigned j = i; j < N; ++j)
			{
			if (j == i && !opt(diag))
				continue;
			BgEval &EvB = *BEs[j];
			WR.Run(EvA, EvB, GameCount, ShowProgress);

			double AWinPct = WR.GetAWinPct();
			double PPG2 = WR.GetPPG2();
			WR.ToTabbed(fTab);
			ProgressLog("%s vs %s, %.1f%% wins, %+.4f ppg\n",
			  EvA.GetName(), EvB.GetName(), AWinPct, PPG2);
			}
		}
	CloseStdioFile(fTab);
	}
