#ifndef bgboard_h
#define bgboard_h

#include "bna.h"

#define START_STR	"4HPwATDgc/ABMA"

// If double, 4*from, else from,diceval,from,diceval
// 0 for unable to move
#define BM(u, k)	(((byte *) &(u))[(k)])

#define BAR			0
#define OPP_HOME_LO	1
#define OPP_HOME_HI	6
#define OUTER_LO	13
#define OUTER_HI	18
#define HOME_LO		19
#define HOME_HI		24
#define OFF			25
#define WHITE_CHAR		'O'
#define BLACK_CHAR		'X'
#define INV(n)		(25-(n))

#define SETA		const byte *a = (White ? m_w : m_b)
#define SETB		const byte *b = (White ? m_b : m_w)

typedef unsigned int TanBoard[2][25];

class BgBoard
	{
public:
	static bool g_Trace;
	static byte m_Start[26];
	static unsigned NF;
	static unsigned m_NewCount;
	static unsigned m_DelCount;

public:
	byte m_w[26];
	byte m_b[26];
	unsigned m_Die1;
	unsigned m_Die2;
	float m_Value;

public:
	void GetWhiteMoves(vector<unsigned> &Moves) const;
	void GetBlackMoves(vector<unsigned> &Moves) const;
	void GetMoves(vector<unsigned> &Moves, bool White) const;

	void DoWhiteMove(unsigned Move);
	void DoBlackMove(unsigned Move);
	void DoMove(unsigned Move, bool White);

	int GetPoints() const;
	int GetPoints2() const;

	BgBoard *Clone() const;
	void CopyAndFlip(const BgBoard &rhs) { Copy(rhs); Flip(); }
	void Flip(const BgBoard &rhs);
	void Copy(const BgBoard &rhs);
	void LogMe() const;
	const char *ToStr(string &s) const;
	bool GameOver() const;
	float GetWhiteReward() const;
	float GetBlackReward() const;
	const char *GetPosId(string &s) const { return ToStr(s); }

	void GetNNInput(BNA bna, bool WhiteTurn,
	  vector<float> &Input) const;
	void GetNNInput_simple(vector<float> &x) const;
	void GetNNInput_simplef(vector<float> &x) const;
	void GetNNInput_simple2(vector<float> &x) const;
	void GetNNInput_tesauro(vector<float> &x, bool WhiteTurn) const;
	void GetNNInput_tesaurof(vector<float> &x, bool WhiteTurn) const;
	void AppendNNInputFeatures(vector<float> &x) const;
	unsigned GetWinPoints() const;

	void FromNNInput(const vector<float> &x);

public:
	BgBoard()
		{
		++m_NewCount;
		Clear();
		}
	~BgBoard()
		{
		++m_DelCount;
		}

	void Clear()
		{
		memset(m_w, 0, 26);
		memset(m_b, 0, 26);
		m_Die1 = 0;
		m_Die2 = 0;
		m_Value = FLT_MAX;
		}
	void FromStr(const string &s);
	void FromSketch(const vector<string> &Lines);
	void FromSketchFile(const string &FileName);
	void ToSketchFile(FILE *f) const;
	void ToSketch(vector<string> &Lines) const;

	bool FromBotStr(const string &s);
	const char *ToBotStr(string &s) const;

	void AppendFile(FILE *f) const;

	unsigned Roll1() const { return 1 + randu32()%6; }
	void RollDice(bool FirstRoll);
	void SetStart();
	unsigned GetHash() const;

	bool IsValid() const;
	bool IsValid2(const byte *a, const byte *b) const;
	void AssertValid() const
		{
		AssertValid2(m_w, m_b);
		}
	void Draw(FILE *f = g_fLog) const;
	void Invert();
	void Flip() { Invert(); }

	BgBoard *CloneAndApplyStep(unsigned From, unsigned Pips) const;
	void GetSteps(unsigned Pips, vector<unsigned> &Froms) const;
	void ApplyStep(unsigned From, unsigned Pips);
	bool CanStepPips(unsigned Pips, bool White) const;

	void GetMovesDouble(unsigned Pips, vector<unsigned> &Moves) const;
	void GetMovesNonDouble(unsigned Pips1, unsigned Pips2,
	  vector<unsigned> &Moves) const;

	void GetUniqueMoves(const vector<unsigned> &Moves,
	  const vector<BgBoard *> &Boards, vector<unsigned> &UniqueMoves) const;

	void GetPubEvalPosVec(vector<int> &Pos) const;
	void GetPubEvalXVec(vector<float> &X) const;

	unsigned GetPipCount(bool White) const;
	int GetPipCountDiff() const;
	bool IsBlocked(bool White) const;
	bool CanTakeOff(bool White) const;
	const char *MoveToStr(unsigned Move, string &s) const;
	bool IsContact() const;
	bool IsRace() const { return !IsContact(); }
	bool IsContact_gnu() const;
	bool IsCrashed_gnu() const;
	bool IsRace_gnu() const;
	const string &GetPhase(string &s) const;
	bool CanHit(bool White, unsigned Die1, unsigned Die2) const;

	float GetHitProb(bool White) const;
	unsigned GetHomePointCount(bool White) const;
	unsigned GetHomeManCount(bool White) const;
	unsigned GetManOffCount(bool White) const;
	unsigned GetBuilderCount(bool White) const;
	unsigned GetBlotCount(bool White) const;
	bool HasPrime(bool White) const;
	float GetBlockadeQual(bool White) const;
	unsigned GetContactCount(bool White) const;

	void GetTanBoard(TanBoard &TB) const;
	void GetTanBoard_Flipped(TanBoard &TB) const;
	int GetGnuClass() const;
	bool ApplyGnuMovex(const string &Movex);
	bool IsValidAfterState(const BgBoard &Next, bool White) const;

public:
#define X(x)	float GetFeature_##x() const;
#include "feats.h"

public:
	static void GetSteps2(const byte *a, const byte *b, unsigned Pips,
	  vector<unsigned> &Froms);
	static void AssertValid2(const byte *a, const byte *b);
	static bool ApplyStep2(byte *a, byte *b, unsigned From, unsigned Pips,
	  bool DieOnFail = true);
	static bool IsLegalTakeOff(const byte *a, unsigned From, unsigned Pips);
	static bool Eq2(const byte *a1, const byte *b1, const byte *a2, const byte *b2)
		{
		return (memcmp(a1, a2, 26) == 0 && memcmp(b1, b2, 26) == 0);
		}
	static bool Eq(const BgBoard &B1, const BgBoard &B2)
		{
		return Eq2(B1.m_w, B1.m_b, B2.m_w, B2.m_b);
		}
	};

#define BND(x)	unsigned x = BgBoard::m_NewCount - BgBoard::m_DelCount

#endif //  bgboard_h
