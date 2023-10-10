#include "myutils.h"
#include "bgeval.h"
#include "nneval.h"
#include "bgboard.h"
#include "sort.h"

void cmd_startrolls()
	{
	BgEval &Ev = (BgEval &) *BgEval::CreateEval(opt(startrolls));
	BgBoard Board;
	Board.SetStart();

	for (unsigned Die1 = 1; Die1 <= 6; ++Die1)
		{
		for (unsigned Die2 = 1; Die2 < Die1; ++Die2)
			{
			Board.m_Die1 = Die1;
			Board.m_Die2 = Die2;

			BgBoard Next;
			unsigned Move = Ev.GetWhiteMove(Board);
			Next.Copy(Board);
			Next.DoMove(Move, true);

			Log("\n========= %u-%u ===========\n", Die1, Die2);
			Next.LogMe();
			float Value = Ev.GetWhiteValueAfter(Next);
			if (Ev.m_Name == "nn")
				{
				BgNN &nn = ((NNEval &) Ev).m_NN;
				vector<float> Values;
				nn.Evaluate(Next, false, Values);
				for (unsigned i = 0; i < SIZE(Values); ++i)
					Log(" %.3f", Values[i]);
				Log("\n");
				}
			Log(" V=%.3f\n", Value);
			}
		}
	}
