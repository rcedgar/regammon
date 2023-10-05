#include "myutils.h"
#include "bgboard.h"
#include <map>

bool BgBoard::g_Trace = false;
unsigned BgBoard::m_NewCount;
unsigned BgBoard::m_DelCount;

byte BgBoard::m_Start[26]
	{
// 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25
	0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 3, 0, 5, 0, 0, 0, 0, 0, 0
	};

bool BgBoard::IsValid2(const byte *a, const byte *b) const
	{
	unsigned na = 0;
	unsigned nb = 0;
	for (unsigned i = 0; i < 26; ++i)
		{
		na += a[i];
		nb += b[i];
		}
	if (na != 15)
		return false;
	if (nb != 15)
		return false;
	for (unsigned i = 1; i <= 24; ++i)
		{
		if (a[i] != 0 && b[INV(i)] != 0)
			return false;
		}
	return true;
	}

bool BgBoard::IsValid() const
	{
	return IsValid2(m_w, m_b);
	}

void BgBoard::AssertValid2(const byte *a, const byte *b)
	{
	unsigned na = 0;
	unsigned nb = 0;
	for (unsigned i = 0; i < 26; ++i)
		{
		na += a[i];
		nb += b[i];
		}
	asserta(na == 15);
	asserta(nb == 15);
	for (unsigned i = 1; i <= 24; ++i)
		asserta(a[i] == 0 || b[INV(i)] == 0);
	}

void BgBoard::RollDice(bool FirstRoll)
	{
	m_Die1 = Roll1();
	if (FirstRoll)
		{
		m_Die2 = 1 + randu32()%5;
		if (m_Die2 == m_Die1)
			++m_Die2;
		}
	else
		m_Die2 = Roll1();
	asserta(m_Die1 >= 1 && m_Die1 <= 6);
	asserta(m_Die2 >= 1 && m_Die2 <= 6);
	if (FirstRoll)
		asserta(m_Die1 != m_Die2);
	}

void BgBoard::SetStart()
	{
	memcpy(m_w, m_Start, 26);
	memcpy(m_b, m_Start, 26);
	RollDice(true);
	AssertValid();
	}

void BgBoard::Invert()
	{
	byte tmp[26];
	memcpy(tmp, m_w, 26);
	memcpy(m_w, m_b, 26);
	memcpy(m_b, tmp, 26);
	}

#define BDSIZE		13
static const char *g_Bd[BDSIZE] =
	{
//             1         2         3         4   
//   0123456789012345678901234567890123456789012
	"+13-14-15-16-17-18------19-20-21-22-23-24-+", // 0
	"|                  |   |                  |", // 1
	"|                  |   |                  |", // 2
	"|                  |   |                  |", // 3
	"|                  |   |                  |", // 4
	"|                  |   |                  |", // 5
	"|                  |BAR|                  |", // 6
	"|                  |   |                  |", // 7
	"|                  |   |                  |", // 8
	"|                  |   |                  |", // 9
	"|                  |   |                  |", // 10
	"|                  |   |                  |", // 11
	"+12-11-10--9--8--7-------6--5--4--3--2--1-+", // 12
//   0123456789012345678901234567890123456789012
//             1         2         3         4   
	};
static unsigned g_Cols[25] =
	{
	UINT_MAX,
//   1   2   3   4   5   6   7   8   9 10  11 12
	40, 37, 34, 31, 28, 25, 17, 14, 11, 8, 5, 2,
	2, 5, 8, 11, 14, 17, 25, 28, 31, 34, 37, 40
// 13 14 15  16  17  18  19  20  21  22  23  24
	};

static unsigned g_BarCol = 21;
static unsigned g_BarRow = 6;
static unsigned g_BotRow = 11;
static unsigned g_TopRow = 1;

void BgBoard::Draw(FILE *f) const
	{
	if (f == 0)
		return;
	static char **Bd;
	if (Bd == 0)
		{
		Bd = myalloc(char *, BDSIZE);
		for (unsigned i = 0; i < BDSIZE; ++i)
			Bd[i] = myalloc(char, 44);
		}
	for (unsigned i = 0; i < BDSIZE; ++i)
		memcpy(Bd[i], g_Bd[i], 44);

	unsigned wb = m_w[BAR];
	if (wb > 0)
		{
		for (unsigned i = 0; i < wb; ++i)
			{
			unsigned y = g_BarRow + 1 + i;
			if (y > g_BotRow)
				{
				Bd[g_BotRow][g_BarCol] = char('0' + wb);
				break;
				}
			Bd[y][g_BarCol] = WHITE_CHAR;
			}
		}

	unsigned bb = m_b[BAR];
	if (bb > 0)
		{
		for (unsigned i = 0; i < bb; ++i)
			{
			unsigned y = g_BarRow - 1 - i;
			if (y < g_TopRow)
				{
				Bd[g_TopRow][g_BarCol] = char('0' + bb);
				break;
				}
			Bd[y][g_BarCol] = BLACK_CHAR;
			}
		}
	for (unsigned i = 1; i <= 24; ++i)
		{
		unsigned nw = m_w[i];
		unsigned nb = m_b[INV(i)];
		if (nw == 0 && nb == 0)
			continue;
		char c = (nw > 0 ? WHITE_CHAR : BLACK_CHAR);
		unsigned n = nw + nb;
		unsigned Col = g_Cols[i];
		unsigned Row = (i <= 12 ? g_BotRow : g_TopRow);
		unsigned Sign = (i <= 12 ? -1 : +1);
		for (unsigned j = 0; j < n; ++j)
			{
			unsigned y = Row + Sign*j;
			if (j > 4)
				{
				Bd[y-Sign][Col] = char('0' + n);
				break;
				}
			Bd[y][Col] = c;
			}
		}

	unsigned WhitePipCount = GetPipCount(true);
	unsigned BlackPipCount = GetPipCount(false);
	int WhitePips = int(WhitePipCount) - int(BlackPipCount);
	int BlackPips = int(BlackPipCount) - int(WhitePipCount);
	byte MaxDie = max(m_Die1, m_Die2);
	byte MinDie = min(m_Die1, m_Die2);

	fprintf(f, "\n");
	for (unsigned i = 0; i < BDSIZE; ++i)
		{
		fprintf(f, "%s", Bd[i]);
		if (i == 0)
			{
			string s;
			fprintf(f, "  %s", ToStr(s));
			}
		if (i == 1)
			{
			if (MaxDie == 0)
				fprintf(f, "  after %c move", WHITE_CHAR);
			else
				fprintf(f, "  %c to play %u-%u", WHITE_CHAR, MaxDie, MinDie);
			if (m_Value != FLT_MAX)
				fprintf(f, " (%.4f)", m_Value);
			}
		if (i == 3)
			{
			bool Contact = IsContact();
			bool Race = IsRace();
			string Phase;
			GetPhase(Phase);
			fprintf(f, "  %s", Phase.c_str());
			if (Phase == "contact")
				{
				unsigned cw = GetContactCount(true);
				unsigned cb = GetContactCount(false);
				if (cw > 0)
					fprintf(f, " O %u", cw);
				if (cw > 0)
					{
					if (cw > 0)
						fprintf(f, ",");
					fprintf(f, " X %u", cb);
					}
				}
			if (GameOver())
				fprintf(f, "  game over");
			else if (CanTakeOff(true))
				fprintf(f, " (bearoff)");
			}
		if (i == 4)
			{
			float PO = GetHitProb(true);
			float PX = GetHitProb(false);
			if (PO > 0.0f)
				fprintf(f, "  PO(%.2f)", PO);
			if (PX > 0.0f)
				fprintf(f, "  PX(%.2f)", PX);
			}
		if (i == 6)
			fprintf(f, "  O pips %u(%+d)",
			  WhitePipCount, WhitePips);
		if (i == 7)
			fprintf(f, "  X pips %u(%+d)",
			  BlackPipCount, BlackPips);
		if (i == 9 && (m_w[OFF] > 0 || m_b[OFF] > 0))
			{
			fprintf(f, "  Off:");
			if (m_w[OFF] > 0)
				fprintf(f, " O(%u)", m_w[OFF]);
			if (m_b[OFF] > 0)
				fprintf(f, " X(%u)", m_b[OFF]);
			}
		fprintf(f, "\n");
		}
	fprintf(f, "\n");
	}

float BgBoard::GetWhiteReward() const
	{
	if (!GameOver())
		return 0.0f;
	int Points = GetPoints();
	if (Points == 3)
		Points = 2;
	else if (Points == -3)
		Points = -2;
	return float(Points);
	}

float BgBoard::GetBlackReward() const
	{
	if (!GameOver())
		return 0.0f;
	int Points = GetPoints();
	if (Points == 3)
		Points = 2;
	else if (Points == -3)
		Points = -2;
	return float(-Points);
	}

bool BgBoard::GameOver() const
	{
	unsigned nw = m_w[OFF];
	unsigned nb = m_b[OFF];
	asserta(nw < 15 || nb < 15);
	return nw == 15 || nb == 15;
	}

int BgBoard::GetPoints2() const
	{
	int Points = GetPoints();
	if (Points == 3)
		return 2;
	else if (Points == -3)
		return -2;
	return Points;
	}

int BgBoard::GetPoints() const
	{
	unsigned nw = m_w[OFF];
	unsigned nb = m_b[OFF];
	asserta(nw < 15 || nb < 15);
	if (nw == 15)
		{
		asserta(nb < 15);
		if (nb > 0)
			return 1;
		for (unsigned i = OPP_HOME_LO; i <= OPP_HOME_HI; ++i)
			if (m_b[i] > 0)
				return 3;
		return 2;
		}
	else if (nb == 15)
		{
		asserta(nw < 15);
		if (nw > 0)
			return -1;
		for (unsigned i = OPP_HOME_LO; i <= OPP_HOME_HI; ++i)
			if (m_w[i] > 0)
				return -3;
		return -2;
		}
	return 0;
	}

unsigned BgBoard::GetWinPoints() const
	{
	return 1;
	}

//bool BgBoard::IsBlackWin() const
//	{
//	unsigned nw = m_w[OFF];
//	unsigned nb = m_b[OFF];
//	asserta(nw < 15 || nb < 15);
//	return nb == 15;
//	}
//
//unsigned BgBoard::GetWinPoints() const
//	{
//	if (IsBackgammon())
//		return 3;
//	else if (IsGammon())
//		return 2;
//	return 1;
//	}
//
//bool BgBoard::IsWhiteGammonWin() const
//	{
//	unsigned nw = m_w[OFF];
//	unsigned nb = m_b[OFF];
//	asserta(nw < 15 || nb < 15);
//	return nw == 15 && nb == 0;
//	}
//
//bool BgBoard::IsBlackGammonWin() const
//	{
//	unsigned nw = m_w[OFF];
//	unsigned nb = m_b[OFF];
//	asserta(nw < 15 || nb < 15);
//	return nw == 0 && nb == 15;
//	}
//
//bool BgBoard::IsBlackBackgammonWin() const
//	{
//	if (!IsBlackGammonWin())
//		return false;
//	for (unsigned i = OPP_HOME_LO; i <= OPP_HOME_HI; ++i)
//		if (m_w[i] > 0)
//			return true;
//	return false;
//	}
//
//bool BgBoard::IsWhiteBackgammonWin() const
//	{
//	if (!IsWhiteGammonWin())
//		return false;
//	for (unsigned i = OPP_HOME_LO; i <= OPP_HOME_HI; ++i)
//		if (m_b[i] > 0)
//			return true;
//	return false;
//	}

//bool BgBoard::IsGammon() const
//	{
//	return IsWhiteGammonWin() || IsBlackGammonWin();
//	}
//
//bool BgBoard::IsBackgammon() const
//	{
//	return IsWhiteBackgammonWin() || IsBlackBackgammonWin();
//	}

BgBoard *BgBoard::Clone() const
	{
	BgBoard *B = new BgBoard;
	memcpy(B->m_w, m_w, 26);
	memcpy(B->m_b, m_b, 26);
	B->m_Die1 = m_Die1;
	B->m_Die2 = m_Die2;
	return B;
	}

void BgBoard::Flip(const BgBoard &rhs)
	{
	Copy(rhs);
	Invert();
	m_Die1 = rhs.m_Die1;
	m_Die2 = rhs.m_Die2;
	}

void BgBoard::Copy(const BgBoard &rhs)
	{
	memcpy(m_w, rhs.m_w, 26);
	memcpy(m_b, rhs.m_b, 26);
	m_Die1 = rhs.m_Die1;
	m_Die2 = rhs.m_Die2;
	m_Value = rhs.m_Value;
	}

void BgBoard::DoWhiteMove(unsigned Move)
	{
	if (g_Trace)
		Log("DoWhiteMove(White=%c, Move=0x%x)\n", Move);

	if (Move == UINT_MAX)
		{
		if (g_Trace)
			Log("  IsBlocked\n");
		m_Die1 = 0;
		m_Die2 = 0;
		return;
		}

	if (m_Die1 == m_Die2)
		{
		for (unsigned k = 0; k < 4; ++k)
			{
			unsigned From = BM(Move, k);
			if (From == 0xff)
				break;
			if (g_Trace)
				Log("ApplyMove double(k=%u), calls ApplyStep(From=%u, m_Die1=%u\n",
				  k, From, m_Die1);
			ApplyStep(From, m_Die1); 
			}
		}
	else
		{
		unsigned From1 = BM(Move, 0);
		unsigned Die1 = BM(Move, 1);
		unsigned From2 = BM(Move, 2);
		unsigned Die2 = BM(Move, 3);
		asserta(Die1 != 0xff);
		if (g_Trace)
			Log("ApplyMove calls ApplyStep(From1=%u, Die1=%u\n",
			  From1, Die1);
		ApplyStep(From1, Die1); 
		if (g_Trace)
			{
			Log("ApplyMove, after ApplyStep#1 Next:\n");
			LogMe();
			}
		if (Die2 == 0xff)
			{
			if (g_Trace)
				Log("ApplyMove Die2=0xff\n");
			}
		else
			{
			if (g_Trace)
				Log("ApplyMove calls ApplyStep(From1=%u, Die1=%u\n",
				  From1, Die1);
			ApplyStep(From2, Die2);
			if (g_Trace)
				{
				Log("ApplyMove, after ApplyStep#2 Next:\n");
				LogMe();
				}
			}
		}
	AssertValid();
	m_Die1 = 0;
	m_Die2 = 0;
	}

void BgBoard::GetWhiteMoves(vector<unsigned> &Moves) const
	{
	asserta(m_Die1 >= 1 && m_Die1 <= 6);
	asserta(m_Die2 >= 1 && m_Die2 <= 6);
	if (m_Die1 == m_Die2)
		GetMovesDouble(m_Die1, Moves);
	else
		GetMovesNonDouble(m_Die1, m_Die2, Moves);
	}

void BgBoard::GetSteps(unsigned Pips, vector<unsigned> &Froms) const
	{
	GetSteps2(m_w, m_b, Pips, Froms);
	}

bool BgBoard::CanStepPips(unsigned Pips, bool White) const
	{
	vector<unsigned> Froms;
	if (White)
		GetSteps2(m_w, m_b, Pips, Froms);
	else
		GetSteps2(m_b, m_w, Pips, Froms);
	bool CanStep = !Froms.empty();
	return CanStep;
	}

void BgBoard::GetSteps2(const byte *a, const byte *b,
  unsigned Pips, vector<unsigned> &Froms)
	{
	Froms.clear();
	asserta(Pips >= 1 && Pips <= 6);
	if (a[BAR] > 0)
		{
		unsigned To = INV(Pips);
		if (b[To] < 2)
			Froms.push_back(BAR);
		return;
		}
	for (unsigned From = 1; From <= 24; ++From)
		{
		if (a[From] == 0)
			continue;
		unsigned To = From + Pips;
		if (To >= OFF)
			{
			if (IsLegalTakeOff(a, From, Pips))
				Froms.push_back(From);
			}
		else if (a[From] > 0 && b[INV(To)] <= 1)
			Froms.push_back(From);
		}
	}

BgBoard *BgBoard::CloneAndApplyStep(unsigned From, unsigned Pips) const
	{
	BgBoard *Next = Clone();
	Next->ApplyStep(From, Pips);
	return Next;
	}

void BgBoard::ApplyStep(unsigned From, unsigned Pips)
	{
	ApplyStep2(m_w, m_b, From, Pips);
	}

void BgBoard::GetMovesDouble(unsigned Pips, vector<unsigned> &UniqueMoves) const
	{
	if (g_Trace)
		Log("GetMovesDouble(White=%c, Pips=%u)\n", Pips);
	UniqueMoves.clear();

	vector<unsigned> Moves;
	vector<unsigned> Lengths;
	vector<BgBoard *> Boards;
	unsigned MaxLength = 0;

	vector<BgBoard *> TmpBoards;

	vector<unsigned> Froms1;
	GetSteps(Pips, Froms1);
	unsigned Size1 = SIZE(Froms1);
	if (g_Trace)
		Log("GetMovesDouble calls GetSteps, SIZE(Froms1)=%u\n", Size1);
	if (Size1 == 0)
		return;

	for (unsigned i = 0; i < Size1; ++i)
		{
		unsigned From1 = Froms1[i];
		uint32 Move = 0xffffffff;
		BM(Move, 0) = From1;

		BgBoard *Next1 = 
		  this->CloneAndApplyStep(From1, Pips);
		TmpBoards.push_back(Next1);
		if (g_Trace)
			{
			Log("GetMovesDouble calls CloneAndApplyStep(White=%c, From1=%u, Pips=%u), Next1:\n",
			  From1, Pips);
			Next1->LogMe();
			}

		vector<unsigned> Froms2;
		Next1->GetSteps(Pips, Froms2);
		unsigned Size2 = SIZE(Froms2);
		if (g_Trace)
			Log("GetMovesDouble calls Next1->GetSteps, SIZE(Froms2)=%u\n", Size1);
		if (Size2 == 0)
			{
			Moves.push_back(Move);
			if (g_Trace)
				{
				string Tmp;
				Log("GetMovesDouble add move %s\n", MoveToStr(Move, Tmp));
				}
			Boards.push_back(Next1);
			Lengths.push_back(1);
			MaxLength = max(MaxLength, 1u);
			}
		for (unsigned i = 0; i < Size2; ++i)
			{
			unsigned From2 = Froms2[i];
			BM(Move, 1) = From2;

			BgBoard *Next2 = 
			  Next1->CloneAndApplyStep(From2, Pips);
			TmpBoards.push_back(Next2);
			if (g_Trace)
				{
				Log("GetMovesDouble calls Next1->CloneAndApplyStep(White=%c, From2=%u, Pips=%u), Next2:\n",
				  From2, Pips);
				Next2->LogMe();
				}

			vector<unsigned> Froms3;
			Next2->GetSteps(Pips, Froms3);

			unsigned Size3 = SIZE(Froms3);
			if (g_Trace)
				Log("GetMovesDouble calls Next2->GetSteps, SIZE(Froms3)=%u\n", Size3);
			if (Size3 == 0)
				{
				Moves.push_back(Move);
				if (g_Trace)
					{
					string Tmp;
					Log("GetMovesDouble add move %s\n", MoveToStr(Move, Tmp));
					}
				Boards.push_back(Next2);
				Lengths.push_back(2);
				MaxLength = max(MaxLength, 2u);
				}
			for (unsigned i = 0; i < Size3; ++i)
				{
				unsigned From3 = Froms3[i];
				BM(Move, 2) = From3;

				BgBoard *Next3 = 
				  Next2->CloneAndApplyStep(From3, Pips);
				TmpBoards.push_back(Next3);
				if (g_Trace)
					{
					Log("GetMovesDouble calls Next2->CloneAndApplyStep(White=%c, From3=%u, Pips=%u), Next3:\n",
					  From3, Pips);
					Next3->LogMe();
					}

				vector<unsigned> Froms4;
				Next3->GetSteps(Pips, Froms4);
				unsigned Size4 = SIZE(Froms4);
				if (g_Trace)
					Log("GetMovesDouble calls Next3->GetSteps, SIZE(Froms4)=%u\n", Size4);
				if (Size4 == 0)
					{
					Moves.push_back(Move);
					if (g_Trace)
						{
						string Tmp;
						Log("GetMovesDouble add move %s\n", MoveToStr(Move, Tmp));
						}
					Boards.push_back(Next3);
					Lengths.push_back(3);
					MaxLength = max(MaxLength, 3u);
					}
				for (unsigned i = 0; i < Size4; ++i)
					{
					unsigned From4 = Froms4[i];
					BgBoard *Next4 = 
					  Next3->CloneAndApplyStep(From4, Pips);
					TmpBoards.push_back(Next4);
					if (g_Trace)
						{
						Log("GetMovesDouble calls Next4->CloneAndApplyStep(White=%c, From4=%u, Pips=%u), Next4:\n",
						  From4, Pips);
						Next4->LogMe();
						}
					
					BM(Move, 3) = From4;
					Moves.push_back(Move);
					if (g_Trace)
						{
						string Tmp;
						Log("GetMovesDouble add move %s\n", MoveToStr(Move, Tmp));
						}
					Boards.push_back(Next4);
					Lengths.push_back(4);
					MaxLength = 4;
					}
				}
			}
		}

	asserta(SIZE(Lengths) == SIZE(Moves));
	asserta(SIZE(Boards) == SIZE(Moves));
	GetUniqueMoves(Moves, Boards, UniqueMoves);
	for (unsigned i = 0; i < SIZE(TmpBoards); ++i)
		delete TmpBoards[i];
	}

void BgBoard::GetMovesNonDouble(unsigned Pips1, unsigned Pips2,
  vector<unsigned> &UniqueMoves) const
	{
	UniqueMoves.clear();
	asserta(Pips1 != Pips2);
	vector<unsigned> Moves;
	vector<BgBoard *> Boards;
	vector<unsigned> Lengths;
	unsigned MaxLength = 0;

	vector<BgBoard *> TmpBoards;

	for (unsigned Swap = 0; Swap < 2; ++Swap)
		{
		unsigned Pips1 = (Swap == 0 ? m_Die1 : m_Die2);
		unsigned Pips2 = (Swap == 0 ? m_Die2 : m_Die1);

		vector<unsigned> Froms1;
		GetSteps(Pips1, Froms1);
		unsigned Size1 = SIZE(Froms1);
		for (unsigned i = 0; i < Size1; ++i)
			{
			unsigned From1 = Froms1[i];
			uint32 Move = 0xffffffff;
			BM(Move, 0) = From1;
			BM(Move, 1) = Pips1;

			BgBoard *Next1 =
			  this->CloneAndApplyStep(From1, Pips1);
			TmpBoards.push_back(Next1);

			vector<unsigned> Froms2;
			Next1->GetSteps(Pips2, Froms2);
			unsigned Size2 = SIZE(Froms2);
			if (Size2 == 0)
				{
				Moves.push_back(Move);
				Boards.push_back(Next1);
				Lengths.push_back(1);
				MaxLength = max(MaxLength, 1u);
				}
			for (unsigned i = 0; i < Size2; ++i)
				{
				unsigned From2 = Froms2[i];
				BM(Move, 2) = From2;
				BM(Move, 3) = Pips2;
				BgBoard *Next2 =
				  Next1->CloneAndApplyStep(From2, Pips2);
				TmpBoards.push_back(Next2);

				Moves.push_back(Move);
				Boards.push_back(Next2);
				Lengths.push_back(2);
				MaxLength = 2;
				}
			}
		}
	asserta(SIZE(Lengths) == SIZE(Moves));
	asserta(SIZE(Boards) == SIZE(Moves));
	GetUniqueMoves(Moves, Boards, UniqueMoves);
	for (unsigned i = 0; i < SIZE(TmpBoards); ++i)
		delete TmpBoards[i];
	}

void BgBoard::GetUniqueMoves(const vector<unsigned> &Moves,
  const vector<BgBoard *> &Boards, vector<unsigned> &UniqueMoves) const
	{
	map<unsigned, vector<BgBoard *> > m;
	const unsigned N = SIZE(Moves);
	asserta(SIZE(Boards) == N);
	for (unsigned i = 0; i < N; ++i)
		{
		BgBoard *B = Boards[i];
		unsigned h = B->GetHash();
		map<unsigned, vector<BgBoard *> >::const_iterator p = m.find(h);
		if (p == m.end())
			{
			vector<BgBoard *> Empty;
			m[h] = Empty;
			m[h].push_back(B);
			UniqueMoves.push_back(Moves[i]);
			}
		else
			{
			const vector<BgBoard *> &v = p->second;
			const unsigned n = SIZE(v);
			bool Found = false;
			for (unsigned j = 0; j < n; ++j)
				{
				BgBoard *B2 = v[j];
				if (BgBoard::Eq(*B2, *B))
					{
					Found = true;
					break;
					}
				}
			if (!Found)
				{
				m[h].push_back(B);
				UniqueMoves.push_back(Moves[i]);
				}
			}
		}
	}

bool BgBoard::ApplyGnuMovex(const string &Movex)
	{
	vector<string> Fields;
	vector<string> Fields2;
	Split(Movex, Fields, ',');
	const unsigned N = SIZE(Fields);
	for (unsigned i = 0; i < N; ++i)
		{
		const string &Field = Fields[i];
		Split(Field, Fields2, '/');
		if (SIZE(Fields2) != 2)
			{
			LogMe();
			Log("Movex = %s", Movex.c_str());
			Warning("SIZE(Fields2) = %u != 2", SIZE(Fields2));
			return false;
			}
		const string sFrom = Fields2[0];
		const string sTo = Fields2[1];
		unsigned From, To;
		if (sFrom == "bar")
			From = BAR;
		else
			{
			From = StrToUint(sFrom);
			From = INV(From);
			asserta(From >= 1 && From <= 24);
			}
		if (sTo == "off")
			To = OFF;
		else
			{
			To = StrToUint(sTo);
			To = INV(To);
			asserta(To >= 1 && To <= 24);
			unsigned wTo = INV(To);
			unsigned nw = m_w[wTo];
			if (nw >= 2)
				{
				LogMe();
				Log("Movex = %s", Movex.c_str());
				Log(" From %u To %u wTo %u\n", From, To, wTo);
				Warning("nw >= 2");
				return false;
				}
			if (nw == 1)
				{
				m_w[wTo] = 0;
				++(m_w[BAR]);
				}
			}
		if (m_b[From] == 0)
			{
			LogMe();
			Log("Movex = %s", Movex.c_str());
			Log(" From %u To %u m_b[From]=0\n", From, To);
			Warning("m_b[From]==0");
			return false;
			}
		--(m_b[From]);
		++(m_b[To]);
		}
	if (!IsValid())
		{
		Warning("ApplyMovex not valid");
		return false;
		}
	return true;
	}

bool BgBoard::ApplyStep2(byte *a, byte *b, unsigned From, unsigned Pips,
  bool DieOnFail)
	{
	asserta(Pips >= 1 && Pips <= 6);
	asserta(From < OFF);
	if (a[BAR] > 0 && From != 0)
		{
		if (DieOnFail)
			Die("ApplyStep2(From=%u, Pips=%u) a[BAR]>0",
			  From, Pips);
		return false;
		}

	if (a[From] == 0)
		{
		if (DieOnFail)
			Die("ApplyStep2(From=%u, Pips=%u) a[From]=%u",
			  From, Pips, a[From]);
		return false;
		}
	asserta(a[From] > 0);
	--a[From];
	unsigned To = From + Pips;
	if (To < OFF)
		{
		unsigned bTo = INV(To);
		if (b[bTo] > 0)
			{
			if (a[To] != 0 || b[bTo] != 1)
				{
				if (DieOnFail)
					Die("ApplyStepTo, case #2");
				return false;
				}
			--b[bTo];
			++b[BAR];
			}
		++a[To];
		}
	else
		{
		if (!IsLegalTakeOff(a, From, Pips))
			{
			if (DieOnFail)
				Die("ApplyStep, illegal bearoff");
			return false;
			}
		++a[OFF];
		}
	AssertValid2(a, b);
	return true;
	}

bool BgBoard::CanTakeOff(bool White) const
	{
	SETA;
	unsigned h = 0;
	for (unsigned k = 1; k < HOME_LO; ++k)
		{
		unsigned n = a[k];
		if (n > 0)
			return false;
		}
	return true;
	}

bool BgBoard::IsLegalTakeOff(const byte *a, unsigned From, unsigned Pips)
	{
	asserta(Pips >= 1 && Pips <= 6);

	if (From == 0)
		return false;

	unsigned To = From + Pips;
	if (To < HOME_LO)
		return false;

	unsigned h = 0;
	for (unsigned k = 1; k < HOME_LO; ++k)
		{
		unsigned n = a[k];
		if (n > 0)
			return false;
		}

	if (To == OFF)
		return true;

	asserta(To > OFF);
	for (unsigned k = From-1; k >= HOME_LO; --k)
		if (a[k] > 0)
			return false;

	return true;
	}

// JS hash found on web
unsigned BgBoard::GetHash() const
	{
	unsigned hash = 0x4e67c6a7;
	for (unsigned i = 0; i < 26; ++i)
		hash ^= ((hash << 5) + m_w[i] + (hash >> 2));
	for (unsigned i = 0; i < 26; ++i)
		hash ^= ((hash << 5) + m_b[i] + (hash >> 2));
	return hash;
	}

unsigned BgBoard::GetPipCount(bool White) const
	{
	SETA;
	unsigned n = 0;
	for (unsigned i = 0; i < 25; ++i)
		n += (25-i)*a[i];
	return n;
	}

int BgBoard::GetPipCountDiff() const
	{
	int nw = (int) GetPipCount(true);
	int nb = (int) GetPipCount(false);
	return nw - nb;
	}

bool BgBoard::IsBlocked(bool White) const
	{
	SETA;
	SETB;
	if (a[BAR] > 0)
		{
		unsigned To1 = INV(m_Die1);
		unsigned To2 = INV(m_Die2);
		return (b[To1] > 1 && b[To2] > 1);
		}

	for (int d = 0; d < 2; ++d)
		{
		if (d == 1 && m_Die1 == m_Die2)
			return true;

		unsigned Pips = (d == 0 ? m_Die1 : m_Die2);
		for (unsigned From = 1; From <= 24; ++From)
			{
			if (a[From] == 0)
				continue;
			unsigned To = From + Pips;
			if (To >= OFF)
				{
				if (IsLegalTakeOff(a, From, Pips))
					return false;
				}
			else if (a[From] > 0 && b[INV(To)] <= 1)
				return false;
			}
		}
	return true;
	}

void BgBoard::AppendFile(FILE *f) const
	{
	if (f == 0)
		return;
	string s;
	ToStr(s);
	fprintf(f, "%s\n", s.c_str());
	}

void BgBoard::LogMe() const
	{
	Draw();
	}

const char *BgBoard::MoveToStr(unsigned Move, string &s) const
	{
	s.clear();
	if (Move == UINT_MAX)
		{
		s = "Blocked";
		return s.c_str();
		}

	unsigned MinDie = min(m_Die1, m_Die2);
	unsigned MaxDie = max(m_Die1, m_Die2);

	if (m_Die1 == m_Die2)
		{
		unsigned Pips = m_Die1;
		for (unsigned k = 0; k < 4; ++k)
			{
			unsigned m = BM(Move, k);
			if (k == 0)
				asserta(m != 0xff);
			else
				{
				if (m == 0xff)
					{
					if (k > 0)
						Psa(s, ", ");
					Psa(s, "(blocked)");
					break;
					}
				}
			unsigned From = m;
			asserta(From < OFF);
			if (k > 0)
				s += ", ";
			Psa(s, "[%u]", Pips);
			if (From == 0)
				Psa(s, "bar");
			else
				Psa(s, "%u", From);
			unsigned To = m + Pips;
			if (To >= OFF)
				Psa(s, "/off");
			else
				Psa(s, "/%u", To);
			}
		}
	else
		{
		unsigned From1 = BM(Move, 0);
		unsigned Pips1 = BM(Move, 1);
		unsigned From2 = BM(Move, 2);
		unsigned Pips2 = BM(Move, 3);
		asserta(Pips1 != Pips2);
		asserta(From1 != 0xff);
		asserta(From1 < OFF);
		unsigned To1 = From1 + Pips1;
		Psa(s, "[%u]", Pips1);
		if (From1 == 0)
			Psa(s, "bar");
		else
			Psa(s, "%u", From1);
		if (To1 >= OFF)
			Psa(s, "/off");
		else
			Psa(s, "/%u", To1);
		if (From2 == 0xff)
			Psa(s, ", (blocked)");
		else
			{
			asserta(From2 < OFF);
			unsigned To2 = From2 + Pips2;
			Psa(s, ", [%u]", Pips2);
			if (From2 == 0)
				Psa(s, "bar");
			else
				Psa(s, "%u", From2);
			unsigned To1 = From2 + Pips2;
			if (To2 >= OFF)
				Psa(s, "/off");
			else
				Psa(s, "/%u", To2);
			}
		}
	return s.c_str();
	}

bool BgBoard::IsRace_gnu() const
	{
	int GnuClass = GetGnuClass();
	return GnuClass == 2;
	}

const string &BgBoard::GetPhase(string &s) const
	{
	s.clear();
	int GnuClass = GetGnuClass();
	switch (GnuClass)
		{
	case 1: s = "contact";	break;
	case 2:	s = "race";		break;
	case 3: s = "crashed";	break;
		}
	return s;
	}

bool BgBoard::IsContact_gnu() const
	{
	int GnuClass = GetGnuClass();
	return GnuClass == 1;
	}

/***
http://lists.gnu.org/archive/html/bug-gnubg/2012-02/msg00022.html

CRASHED attempts to capture the positions where one side has only a
small number of "active pieces". The number of active pieces has been
arbitrarily set at 6, and the definition requires that you have at
most 6 checkers not on points 1 or 2, accounting for the possibility
of one checker from 2 sent back after the rest piled on point 1.

The most important part in this celebration of arbitrary decisions was
to use a definition which is non cyclic - positions resulting from a
crashed positions should be crashed. When this is violated,
performance deteriorates since each net is trained only on it's own
kind of positions.

A position is crashed if most of a players chequers have ended up on his 
own 1 and 2 points, giving him little flexibility. He has no control of his 
inner  board or the outfield, which allows his opponent to spread his chequers 
around  without fear of being hit and contained. This calls for different tactics
from the standard contact positions. 

Perhaps a couple of diagrams will help (view with fixed-width font). Player X 
has the same structure each time; the difference is in the opponent's position. 
In the crashed position X can safely slot the front of the prime to roll it 
forward, but this would be too dangerous when O still has good structure.

    GNU Backgammon  Position ID: /z4AADBsuxsEAA
                    Match ID   : cAngAAAAAAAE
    +24-23-22-21-20-19------18-17-16-15-14-13-+  O: White
    | O  O             |   |                X |  0 points
    | O  O             |   |                  |  
    | O  O             |   |                  |  
    | O  O             |   |                  |  
    | 8  O             |   |                  |  
    |                  |BAR|                  |v 7 point match (Cube: 1)
    |                  |   |                  |  
    |                  |   |                  |  
    |                X |   | X                |  
    | O     X  X  X  X |   | X  X             |  On roll
    | O     X  X  X  X |   | X  X             |  0 points
    +-1--2--3--4--5--6-------7--8--9-10-11-12-+  X: Blue

    GNU Backgammon  Position ID: sN0tADBsuxsEAA
                    Match ID   : cAngAAAAAAAE
    +24-23-22-21-20-19------18-17-16-15-14-13-+  O: White
    |             O  O |   | O  O  O  O     X |  0 points
    |             O  O |   | O  O  O          |  
    |                  |   | O  O             |  
    |                  |   |                  |  
    |                  |   |                  |  
    |                  |BAR|                  |v 7 point match (Cube: 1)
    |                  |   |                  |  
    |                  |   |                  |  
    |                X |   | X                |  
    | O     X  X  X  X |   | X  X             |  On roll
    | O     X  X  X  X |   | X  X             |  0 points
    +-1--2--3--4--5--6-------7--8--9-10-11-12-+  X: Blue
***/

bool BgBoard::IsCrashed_gnu() const
	{
	int GnuClass = GetGnuClass();
	return GnuClass == 3;
	}

bool BgBoard::CanHit(bool White, unsigned Die1, unsigned Die2) const
	{
	SETA;
	SETB;
	if (a[BAR] > 0)
		{
		if (b[Die1] == 1 || b[Die2] == 1)
			return true;
		if (b[Die1] == 0)
			{
			for (unsigned i = 1; i <= 24; ++i)
				{
				if (a[i] > 0)
					{
					unsigned To2 = i + Die2;
					if (To2 <= 24 && b[INV(To2)] == 1)
						return true;
					}
				}
			}
		if (b[Die2] == 0)
			{
			for (unsigned i = 1; i <= 24; ++i)
				{
				if (a[i] > 0)
					{
					unsigned To1 = i + Die1;
					if (To1 <= 24 && b[INV(To1)] == 1)
						return true;
					}
				}
			}
		return false;
		}

	for (unsigned i = 1; i <= 24; ++i)
		{
		if (a[i] > 0)
			{
			unsigned To1 = i + Die1;
			unsigned To2 = i + Die2;
			if (To1 <= 24 && b[INV(To1)] == 1)
				return true;
			if (To2 <= 24 && b[INV(To2)] == 1)
				return true;

			unsigned Toc = i + Die1 + Die2;
			if (Toc < 24 && (b[INV(To1)] == 0 || b[INV(To2)] == 0)
			  && b[INV(Toc)] == 1)
				return true;
			}
		}
	return false;
	}

float BgBoard::GetHitProb(bool White) const
	{
	unsigned Hits = 0;
	for (unsigned Die1 = 1; Die1 <= 6; ++Die1)
		{
		for (unsigned Die2 = Die1; Die2 <= 6; ++Die2)
			{
			if (CanHit(!White, Die1, Die2))
				{
				if (Die1 == Die2)
					++Hits;
				else
					Hits += 2;
				}
			}
		}
	float Prob = Hits/36.0f;
	return Prob;
	}

unsigned BgBoard::GetHomePointCount(bool White) const
	{
	SETA;
	unsigned n = 0;
	for (unsigned i = HOME_LO; i <= HOME_HI; ++i)
		if (a[i] >= 2)
			++n;
	return n;
	}

unsigned BgBoard::GetHomeManCount(bool White) const
	{
	SETA;
	unsigned n = 0;
	for (unsigned i = HOME_LO; i <= HOME_HI; ++i)
		n += a[i];
	return n;
	}

unsigned BgBoard::GetManOffCount(bool White) const
	{
	SETA;
	unsigned n = a[OFF];
	return n;
	}

unsigned BgBoard::GetBuilderCount(bool White) const
	{
	SETA;
	unsigned n = 0;
	for (unsigned i = OUTER_LO; i <= OUTER_HI; ++i)
		{
		byte k = a[i];
		if (k%2 == 1)
			++n;
		}
	return n;
	}

unsigned BgBoard::GetBlotCount(bool White) const
	{
	SETA;
	unsigned n = 0;
	for (unsigned i = 1; i <= 24; ++i)
		if (a[i] == 1)
			++n;
	return n;
	}

bool BgBoard::HasPrime(bool White) const
	{
	SETA;
	unsigned Length = 0;
	for (unsigned i = OUTER_LO; i <= HOME_HI; ++i)
		{
		if (a[i] >= 2)
			{
			++Length;
			if (Length == 6)
				return true;
			}
		else
			Length = 0;
		}
	return false;
	}

float BgBoard::GetBlockadeQual(bool White) const
	{
	SETA;
	unsigned Maxn = 0;
	for (unsigned Start = 16; Start <= HOME_HI-6; ++Start)
		{
		unsigned n = 0;
		for (unsigned i = Start; i < Start+6; ++i)
			if (a[i] >= 2)
				++n;
		}
	float v = Maxn/6.0f;
	float Qual = v*v;
	return Qual;
	}

/***
From Tesauro's pubeval source code comments:

pos[] is an integer array of dimension 28 which should represent a legal
final board state after the move. 

Elements 1-24 correspond to board locations 1-24 from computer's point of view,
i.e. computer's men move in the negative direction from 24 to 1, and opponent's
men move in the positive direction from 1 to 24.

Computer's men are represented by positive integers, and opponent's
men are represented by negative integers. 

Element 25 represents computer's men on the bar (positive integer).
Element  0 represents opponent's men on the bar (negative integer).
Element 26 represents computer's men off the board (positive integer).
Element 27 represents opponent's men off the board (negative integer).
***/
void BgBoard::GetPubEvalPosVec(vector<int> &Pos) const
	{
	Pos.clear();
	Pos.resize(28, 0);
	for (int i = 1; i <= 24; ++i)
		{
		int ComputerCount = (int) m_w[i];
		int OppCount = (int) m_b[INV(i)];
		if (ComputerCount != 0 && OppCount != 0)
			{
			LogMe();
			Log("ComputerCount[%d] = %d\n", i, ComputerCount);
			Log("OppCount[%d] = %d\n", INV(i), OppCount);
			Die("Overlap");
			}
		if (ComputerCount > 0)
			Pos[25-i] = ComputerCount;
		else if (OppCount > 0)
			Pos[i] = -OppCount;
		}

	Pos[25] = (int) m_w[BAR];
	Pos[0] = -(int) m_b[BAR];
	Pos[26] = (int) m_w[OFF];
	Pos[27] = -(int) m_b[OFF];
	}

void BgBoard::GetPubEvalXVec(vector<float> &X) const
	{
	vector<int> Pos;
	GetPubEvalPosVec(Pos);

	if (SIZE(X) != 122)
		{
		X.clear();
		X.resize(122, 0.0f);
		}

    for (int j = 1; j <= 24; ++j)
		{
        int jm1 = j - 1;
        int n = Pos[25-j];
        if (n == 0)
			{
            X[5*jm1 + 0] = 0.0f;
            X[5*jm1 + 1] = 0.0f;
            X[5*jm1 + 2] = 0.0f;
            X[5*jm1 + 3] = 0.0f;
            X[5*jm1 + 4] = 0.0f;
			}
		else
			{
            if (n == -1)	X[5*jm1 + 0] = 1.0f;
            if (n == 1)		X[5*jm1 + 1] = 1.0f;
            if (n >= 2)		X[5*jm1 + 2] = 1.0f;
            if (n == 3)		X[5*jm1 + 3] = 1.0f;
            if (n >= 4)		X[5*jm1 + 4] = (n - 3)/2.0f;
			}
		}

// opponent barmen
    X[120] = -(float) (Pos[0])/2.0f;

// computer's men off
    X[121] = (float) (Pos[26])/15.0f;
	}

/***
From Tesauro 1992 "Practical Issues in Temporal Difference Learning":

"[A] truncated unary encoding with four units was used. The first three units
encoded separately the cases of one man, two men, and three men, while the
fourth unit encoded the number of men beyond 3. (In the development of
Neurogammon, it was found that truncating at 5 or 6 units rather than 4 units
gives better performance but of course takes longer to simulate.) This coding
scheme thus used 96 units for each side to encode the information at locations
1-24, and an additional 6 units to encode the number of men on the bar, off
the board, and the player to move, for a total of 198 input units."

https://users.auth.gr/kehagiat/Research/GameTheory/12CombBiblio/BackGammon.html

There were a total of 198 input units to the network. For each point on
the backgammon board, four units indicated the number of white pieces
on the point. If there were no white pieces, then all four units took
on the value zero. If there was one piece, then the first unit took
on the value 1. If there were two pieces, then both the first and the second
unit were 1. If there were three or more pieces on the point, then all of
the first three units were 1. If there were more than three pieces, the
fourth unit also came on, to a degree indicating the number of additional
pieces beyond three. Letting n denote the total number of pieces on the point,
if n>3, then the fourth unit took on the value (n-3)/2. With four units for
white and four for black at each of the 24 points, that made a total of 192 
units. Two additional units encoded the number of white and black pieces on the
bar (each took the value n/2, where n is the number of pieces on the bar), and
two more encoded the number of black and white pieces already successfully
removed from the board (these took the value n/15, where n is the number of 
pieces already borne off). Finally, two units indicated in a binary fashion 
whether it was white's or black's turn to move. Tesauro tried to represent
the position in a straightforward way, making little attempt to minimize
the number of units. He provided one unit for each conceptually distinct
possibility that seemed likely to be relevant, and he scaled them to roughly
the same range, in this case between 0 and 1.
***/
void BgBoard::GetNNInput_simple(vector<float> &x) const
	{
// Not using white/black turn outputs
	x.resize(196);
	zerof(x);

	for (unsigned i = 0; i < 24; ++i)
		{
		unsigned nw = m_w[i+1];
		if (nw >= 1)
			x[i*4 + 0] = float(1);
		if (nw >= 2)
			x[i*4 + 1] = float(1);
		if (nw >= 3)
			x[i*4 + 2] = float(1);
		if (nw >= 4)
			x[i*4 + 3] = float((nw-3)/2.0);

		unsigned nb = m_b[i+1];
		if (nb >= 1)
			x[96 + i*4 + 0] = float(1);
		if (nb >= 2)
			x[96 + i*4 + 1] = float(1);
		if (nb >= 3)
			x[96 + i*4 + 2] = float(1);
		if (nb >= 4)
			x[96 + i*4 + 3] = float((nb-3)/2.0);
		}

	unsigned nw_bar = m_w[BAR];
	unsigned nb_bar = m_b[BAR];
	x[192] = float(nw_bar/2.0);
	x[193] = float(nb_bar/2.0);

	unsigned nw_off = m_w[OFF];
	unsigned nb_off = m_b[OFF];
	x[194] = float(nw_off/15.0);
	x[195] = float(nb_off/15.0);
	}

void BgBoard::GetNNInput_simple2(vector<float> &x) const
	{
	GetNNInput_simple(x);
	}

void BgBoard::GetNNInput_tesauro(vector<float> &x, bool WhiteTurn) const
	{
	GetNNInput_simple(x);
	if (WhiteTurn)
		{
		x.push_back(1.0f);
		x.push_back(0.0f);
		}
	else
		{
		x.push_back(0.0f);
		x.push_back(1.0f);
		}
	}

void BgBoard::GetNNInput_simplef(vector<float> &x) const
	{
	GetNNInput_simple(x);
	AppendNNInputFeatures(x);
	}

void BgBoard::GetNNInput_tesaurof(vector<float> &x, bool WhiteTurn) const
	{
	GetNNInput_tesauro(x, WhiteTurn);
	AppendNNInputFeatures(x);
	}

/***
Nerogammon feautres according to Tesauro & Sejnowski 1989:
(1) Pip count
(2) Degree of contact
(3) Number of points in inner board [aka home board]
(4) Number of points in enemy inner board [aka home board]
(5) Number of men in enemy inner board
(6) "Prime" (blockade)
(7) Blot exposure (probability of being hit)
(8) Blockade strength (probability of escape).
***/
void BgBoard::AppendNNInputFeatures(vector<float> &InputVec) const
	{
#define X(x)	InputVec.push_back(GetFeature_##x());
#include "feats.h"
	}

void BgBoard::FromNNInput(const vector<float> &x)
	{
	asserta(SIZE(x) >= 196);

	const float eps = float(0.01);
	for (unsigned i = 0; i < 24; ++i)
		{
		unsigned nw = m_w[i+1];
		float v0 = x[i*4 + 0];
		float v1 = x[i*4 + 1];
		float v2 = x[i*4 + 2];
		float v3 = x[i*4 + 3];
		if (v3 > 0)
			{
			asserta(v0 == float(1));
			asserta(v1 == float(1));
			asserta(v2 == float(1));
			byte k = byte(v3*2.0 + eps) + 3;
			m_w[i+1] = k;
			}
		else if (v2 > 0)
			{
			asserta(v0 == float(1));
			asserta(v1 == float(1));
			asserta(v2 == float(1));
			asserta(v3 == float(0));
			m_w[i+1] = 3;
			}
		else if (v1 > 0)
			{
			asserta(v0 == float(1));
			asserta(v1 == float(1));
			asserta(v2 == float(0));
			asserta(v3 == float(0));
			m_w[i+1] = 2;
			}
		else if (v0 > 0)
			{
			asserta(v0 == float(1));
			asserta(v1 == float(0));
			asserta(v2 == float(0));
			asserta(v3 == float(0));
			m_w[i+1] = 1;
			}
		else
			{
			asserta(v0 == float(0));
			asserta(v1 == float(0));
			asserta(v2 == float(0));
			asserta(v3 == float(0));
			}
		}

	for (unsigned i = 0; i < 24; ++i)
		{
		unsigned nw = m_b[i+1];
		float v0 = x[96 + i*4 + 0];
		float v1 = x[96 + i*4 + 1];
		float v2 = x[96 + i*4 + 2];
		float v3 = x[96 + i*4 + 3];
		if (v3 > 0)
			{
			asserta(v0 == float(1));
			asserta(v1 == float(1));
			asserta(v2 == float(1));
			byte k = byte(v3*2.0 + eps) + 3;
			m_b[i+1] = k;
			}
		else if (v2 > 0)
			{
			asserta(v0 == float(1));
			asserta(v1 == float(1));
			asserta(v2 == float(1));
			asserta(v3 == float(0));
			m_b[i+1] = 3;
			}
		else if (v1 > 0)
			{
			asserta(v0 == float(1));
			asserta(v1 == float(1));
			asserta(v2 == float(0));
			asserta(v3 == float(0));
			m_b[i+1] = 2;
			}
		else if (v0 > 0)
			{
			asserta(v0 == float(1));
			asserta(v1 == float(0));
			asserta(v2 == float(0));
			asserta(v3 == float(0));
			m_b[i+1] = 1;
			}
		else
			{
			asserta(v0 == float(0));
			asserta(v1 == float(0));
			asserta(v2 == float(0));
			asserta(v3 == float(0));
			}
		}

#define g(n, f)	byte x##n = byte(x[n]*f + eps);
	g(192, 2)
	g(193, 2)
	g(194, 15)
	g(195, 15)
#undef g

	m_w[BAR] = x192;
	m_b[BAR] = x193;

	m_w[OFF] = x194;
	m_b[OFF] = x195;
	AssertValid();
	}

void BgBoard::GetNNInput(BNA bna, bool WhiteTurn,
  vector<float> &Input) const
	{
	switch (bna)
		{
	case BNA_tdg:
		GetNNInput_tesauro(Input, WhiteTurn);
		return;

	case BNA_tdgf:
		GetNNInput_tesaurof(Input, WhiteTurn);
		return;

	case BNA_po:
		GetNNInput_simple(Input);
		return;

	case BNA_pf:
		GetNNInput_simple(Input);
		AppendNNInputFeatures(Input);
		return;
		}

	asserta(false);
	}

void BgBoard::FromSketch(const vector<string> &Lines)
	{
	Clear();
	unsigned wb = 0;
	for (unsigned y = g_BarRow + 1; y <= g_BotRow; ++y)
		{
		char c = Lines[y][g_BarCol];
		if (c == ' ')
			break;
		else if (c == WHITE_CHAR)
			++wb;
		else if (isdigit(c))
			{
			wb = (c - '0');
			break;
			}
		else
			asserta(false);
		}
	m_w[BAR] = wb;

	unsigned bb = 0;
	for (unsigned y = g_BarRow - 1; y >= g_TopRow; --y)
		{
		char c = Lines[y][g_BarCol];
		if (c == ' ')
			break;
		else if (c == BLACK_CHAR)
			++bb;
		else if (isdigit(c))
			{
			bb = (c - '0');
			break;
			}
		}
	m_b[BAR] = bb;

	for (unsigned i = 1; i <= 24; ++i)
		{
		unsigned nw = m_w[i];
		unsigned nb = m_b[INV(i)];
		asserta(nw == 0 && nb == 0);
		char c = (nw > 0 ? WHITE_CHAR : BLACK_CHAR);
		unsigned Col = g_Cols[i];
		unsigned Row = (i <= 12 ? g_BotRow : g_TopRow);
		unsigned Sign = (i <= 12 ? -1 : +1);
		for (unsigned j = 0; j < 5; ++j)
			{
			unsigned y = Row + Sign*j;
			char c = Lines[y][Col];
			if (c == WHITE_CHAR)
				{
				asserta(nb == 0);
				++nw;
				}
			else if (c == BLACK_CHAR)
				{
				asserta(nw == 0);
				++nb;
				}
			else if (isdigit(c))
				{
				unsigned k = (c - '0');
				if (nw > 0)
					nw += k;
				else if (nb > 0)
					nb += k;
				else
					asserta(false);
				}
			}
		m_w[i] = nw;
		m_b[INV(i)] = nb;
		}

	unsigned nw = 0;
	unsigned nb = 0;
	for (unsigned i = 0; i <= 24; ++i)
		{
		nw += m_w[i];
		nb += m_b[i];
		}
	asserta(nw <= 15 && nb <= 15);
	m_w[OFF] = 15 - nw;
	m_b[OFF] = 15 - nb;
	AssertValid();
	}

void BgBoard::ToSketchFile(FILE *f) const
	{
	if (f == 0)
		return;
	vector<string> Lines;
	ToSketch(Lines);
	for (unsigned i = 0; i < SIZE(Lines); ++i)
		fprintf(f, "%s\n", Lines[i].c_str());
	}

void BgBoard::ToSketch(vector<string> &Lines) const
	{
	Lines.clear();
	Lines.resize(BDSIZE);
	for (unsigned i = 0; i < BDSIZE; ++i)
		Lines[i] = string(g_Bd[i]);

	unsigned wb = m_w[BAR];
	if (wb > 0)
		{
		for (unsigned i = 0; i < wb; ++i)
			{
			unsigned y = g_BarRow + 1 + i;
			if (y > g_BotRow)
				{
				Lines[g_BotRow][g_BarCol] = char('0' + wb);
				break;
				}
			Lines[y][g_BarCol] = WHITE_CHAR;
			}
		}

	unsigned bb = m_b[BAR];
	if (bb > 0)
		{
		for (unsigned i = 0; i < bb; ++i)
			{
			unsigned y = g_BarRow - 1 - i;
			if (y < g_TopRow)
				{
				Lines[g_BotRow][g_BarCol] = char('0' + bb);
				break;
				}
			Lines[y][g_BarCol] = BLACK_CHAR;
			}
		}
	for (unsigned i = 1; i <= 24; ++i)
		{
		unsigned nw = m_w[i];
		unsigned nb = m_b[INV(i)];
		if (nw == 0 && nb == 0)
			continue;
		char c = (nw > 0 ? WHITE_CHAR : BLACK_CHAR);
		unsigned n = nw + nb;
		unsigned Col = g_Cols[i];
		unsigned Row = (i <= 12 ? g_BotRow : g_TopRow);
		unsigned Sign = (i <= 12 ? -1 : +1);
		for (unsigned j = 0; j < n; ++j)
			{
			unsigned y = Row + Sign*j;
			if (j > 4)
				{
				Lines[y-Sign][Col] = char('0' + n);
				break;
				}
			Lines[y][Col] = c;
			}
		}
	}

void BgBoard::FromSketchFile(const string &FileName)
	{
	FILE *f = OpenStdioFile(FileName);
	vector<string> Lines;
	string Line;
	for (unsigned i = 0; i < BDSIZE; ++i)
		{
		bool Ok = ReadLineStdioFile(f, Line);
		asserta(Ok);
		Lines.push_back(Line);
		}
	CloseStdioFile(f);
	FromSketch(Lines);
	}

void BgBoard::GetBlackMoves(vector<unsigned> &Moves) const
	{
	BgBoard B;
	B.CopyAndFlip(*this);
	B.GetWhiteMoves(Moves);
	}

void BgBoard::DoBlackMove(unsigned Move)
	{
	BgBoard NextBoard;
	NextBoard.CopyAndFlip(*this);
	NextBoard.DoWhiteMove(Move);
	CopyAndFlip(NextBoard);
	}

bool BgBoard::IsContact() const
	{
	return GetContactCount(true) > 0 ||
	  GetContactCount(false) > 0;
	}

unsigned BgBoard::GetContactCount(bool White) const
	{
	SETA;
	SETB;
	unsigned n = 0;
	for (int i = 24; i > 0; --i)
		if (b[INV(i)] > 0)
			{
			for (; i >= 0; --i)
				n += a[i];
			break;
			}
	return n;
	}

void BgBoard::GetMoves(vector<unsigned> &Moves, bool White) const
	{
	if (White)
		GetWhiteMoves(Moves);
	else
		GetBlackMoves(Moves);
	}

void BgBoard::DoMove(unsigned Move, bool White)
	{
	if (White)
		DoWhiteMove(Move);
	else
		DoBlackMove(Move);
	}

/***
W01B00W01E00E00E00E00B00E00B00E00E00E00W01B00E00E00E00W01E00W01E00E00E00E00B00W01B0031

xxx  k         Description
W00  0,1,2	   W bar
B00  3,4,5     B bar
cnn  6,7,8     Point #01, c=W/B, nn=number of men
cnn  9,10,11   Point #02, c=W/B, nn=number of men
...
cnn  75,76,77  Point #24, c=W/B, nn=number of men
W00  78,79,80  W off
B00  81,82,83  B off
3    84        Die1
1    85        Die2
***/

#define INT2(s)	\
	if (!isdigit(c1) || !isdigit(c2)) \
		{ \
		Log("INT2(%s) c1=%c c2=%c\n", s, c1, c2); \
		return false; \
		} \
	n = (c1 - '0')*10 + (c2 - '0');

bool BgBoard::FromBotStr(const string &s)
	{
	Clear();

	if (SIZE(s) != 86)
		{
		Log("SIZE %u\n", SIZE(s));
		return false;
		}
	if (s[0] != 'W' || s[3] != 'B')
		{
		Log("fail0-3\n");
		return false;
		}
	if (s[78] != 'W' || s[81] != 'B')
		{
		Log("fail78 %c 81 %c\n", s[78], s[81]);
		return false;
		}
	if (!isdigit(s[84]) || !isdigit(s[85]))
		{
		Log("fail84-5\n");
		return false;
		}

	m_Die1 = s[84] - '0';
	m_Die2 = s[85] - '0';
	if (m_Die1 < 1 || m_Die1 > 6 || m_Die2 < 1 || m_Die2 > 6)
		{
		Log("Die %c %c %u %u\n", s[84], s[85], m_Die1, m_Die2);
		return false;
		}
	unsigned SumW = 0;
	unsigned SumB = 0;
	char c1;
	char c2;
	unsigned n;

	c1 = s[1];
	c2 = s[2];
	INT2("1")
	m_w[0] = n;
	SumW += n;

	c1 = s[4];
	c2 = s[5];
	INT2("2")
	m_b[0] = n;
	SumB += n;

	c1 = s[79];
	c2 = s[80];
	INT2("3")
	m_w[25] = n;
	SumW += n;

	c1 = s[82];
	c2 = s[83];
	INT2("4")
	m_b[25] = n;
	SumB += n;

	for (unsigned i = 1; i <= 24; ++i)
		{
		unsigned k = 3*(i + 1);
		char c = s[k + 0];
		c1 = s[k + 1];
		c2 = s[k + 2];
		if (c != 'W' && c != 'B' && c != 'E')
			{
			Log("failWB c=%c\n", c);
			return false;
			}
		INT2("5")
		if (c == 'E')
			{
			if (c1 != '0' || c2 != '0')
				{
				Log("E%c%c\n", c1, c2);
				return false;
				}
			}
		else if (c == 'W')
			{
			m_w[i] = n;
			SumW += n;
			}
		else if (c == 'B')
			{
			m_b[INV(i)] = n;
			SumB += n;
			}
		else
			{
			Log("Not BWE %c\n", c);
			return false;
			}
		}

	if (SumW != 15 || SumB != 15)
		{
		Log("Sums %u %u\n", SumW, SumB);
		return false;
		}

	if (!IsValid())
		{
		Log("!valid\n");
		return false;
		}

	return true;
	}

static void IntToStr2(char c, unsigned n, string &s, unsigned k)
	{
	if (n > 99)
		{
		s[k] = '!';
		s[k+1] = '!';
		s[k+2] = '!';
		return;
		}
	s[k+0] = c;
	s[k+1] = '0' + n/10;
	s[k+2] = '0' + n%10;
	}

const char *BgBoard::ToBotStr(string &s) const
	{
	s.clear();
	s.resize(84, '#');

#define IW(n, k)	IntToStr2('W', n, s, k)
#define IB(n, k)	IntToStr2('B', n, s, k)
#define IE(k)		IntToStr2('E', 0, s, k)

	IW(m_w[BAR], 0);
 	IB(m_b[BAR], 3);
	IW(m_w[OFF], 78);
	IB(m_b[OFF], 81);

	for (unsigned i = 1; i <= 24; ++i)
		{
		unsigned nw = m_w[i];
		unsigned nb = m_b[INV(i)];
		unsigned k = 3*(i+1);
		if (nw > 0)
			IW(nw, k);
		else if (nb > 0)
			IB(nb, k);
		else
			IE(k);
		}

#undef IW
#undef IB
#undef IE

	return s.c_str();
	}

bool BgBoard::IsValidAfterState(const BgBoard &Next, bool White) const
	{
	vector<unsigned> Moves;
	GetMoves(Moves, White);

	const unsigned N = SIZE(Moves);
	for (unsigned i = 0; i < N; ++i)
		{
		unsigned Move = Moves[i];

		BgBoard Next2;
		Next2.Copy(*this);
		Next2.DoMove(Move, White);

		if (Eq(Next, Next2))
			return true;
		}

	return false;
	}
