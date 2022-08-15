#include "myutils.h"
#include "bgboard.h"
#include "nneval.h"
#include "bgeval1.h"

#if 0

#define TRACE	1

float BgEval1::GetWhiteValueAfter_Impl(const BgBoard &Board)
	{
	double Sum = 0.0;
	unsigned Count = 0;
	BgBoard FlippedBoard;
	FlippedBoard.CopyAndFlip(Board);
	for (unsigned Die1 = 1; Die1 <= 6; ++Die1)
		{
		for (unsigned Die2 = 1; Die2 <= Die1; ++Die2)
			{
#if TRACE
			string BestPosId;
			Log("%u-%u\n", Die1, Die2);
#endif
			float BestValue = PLUS_INF;
			vector<unsigned> Moves;
			FlippedBoard.m_Die1 = Die1;
			FlippedBoard.m_Die2 = Die2;
			DoWhiteMove(FlippedBoard);

			unsigned BestMove = UINT_MAX;
			const unsigned N = SIZE(BlackMoves);
			if (N == 0)
				{
				BestValue = m_NN.GetWhiteValueBefore(FlippedBoard);
#if TRACE
				FlippedBoard.ToStr(BestPosId);
#endif
				}
			else
				{
				for (unsigned i = 0; i < N; ++i)
					{
					unsigned BlackMove = BlackMoves[i];
					BgBoard Next;
					Next.Copy(FlippedBoard);
					Next.DoBlackMove(BlackMove);
#if TRACE
					string NextStr;
					Next.ToStr(NextStr);
#endif
					float NextValue = m_NN.GetWhiteValueBefore(Next);
					if (NextValue < BestValue)
						{
						BestValue = NextValue;
#if TRACE
						BestPosId = NextStr;
#endif
						}
#if TRACE
					Log("  %+.4f  %s\n", NextValue, NextStr.c_str());
#endif
					}
				}
			unsigned n = 2;
			if (Die1 == Die2)
				n = 1;
			Sum += n*BestValue;
			Count += n;
#if TRACE
			Log("  %+.4f  %s << MIN\n\n", BestValue, BestPosId.c_str());
#endif
			}
		}
	asserta(Count == 36);
	double Avg = Sum/Count;
#if TRACE
	float Value0 = m_NN.GetWhiteValueAfter(Board);
	Log("  %+.4f  (Value0 %+.4f)\n", Avg, Value0);
#endif
	return float(Avg);
	}

float BgEval1::GetBlackValueAfter_Impl(const BgBoard &Board)
	{
	Die("BgEval1::GetBlackValueAfter_Impl");
	return 0.0f;
	}

float BgEval1::GetWhiteValueBefore_Impl(const BgBoard &Board)
	{
	Die("BgEval1::GetWhiteValueBefore_Impl");
	return 0.0f;
	}

float BgEval1::GetBlackValueBefore_Impl(const BgBoard &Board)
	{
	Die("BgEval1::GetBlackValueBefore_Impl");
	return 0.0f;
	}

static void LookaheadDice()
	{
	Die("-dice not supported");
	}

void cmd_lookahead()
	{
	const string &PosId = opt(lookahead);

	if (optset_dice)
		{
		LookaheadDice();
		return;
		}

	BgBoard Board;
	Board.FromStr(PosId);

	BgEval1 BE;

	float vb = BE.m_NN.GetWhiteValueBefore(Board);
	float va = BE.m_NN.GetWhiteValueAfter(Board);

	Board.LogMe();
	ProgressLog("%s %.4f %.4f\n", PosId.c_str(), vb, va);
	return;

	BE.GetWhiteValueAfter(Board);
	}
#else

float BgEval1::GetWhiteValueBefore_Impl(const BgBoard &Board) { Die("GetWhiteValueBefore_Impl"); return 0.0; }
float BgEval1::GetBlackValueBefore_Impl(const BgBoard &Board) { Die("GetWhiteValueBefore_Impl"); return 0.0; }
float BgEval1::GetWhiteValueAfter_Impl(const BgBoard &Board) { Die("GetWhiteValueAfter_Impl"); return 0.0; }
float BgEval1::GetBlackValueAfter_Impl(const BgBoard &Board) { Die("GetWhiteValueAfter_Impl"); return 0.0; }
void cmd_lookahead() { Die("cmd_lookahead"); }

#endif
