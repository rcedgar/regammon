#include "myutils.h"
#include "bgboard.h"
#include "bgeval.h"
#include "bgeval1.h"
#include "pubeval.h"
#include "randev.h"
#include "nneval.h"

BgEval *BgEval::CreateEval(const string &Name)
	{
	unsigned Hidden = 10;
	if (optset_hidden)
		Hidden = StrToUint(opt(hidden));

	BgEval *BE = 0;
	if (Name == "pube")
		BE = new PubEval;
	else if (Name == "pubec")
		{
		PubEval *PE = new PubEval;
		PE->m_ContactOnly = true;
		BE = PE;
		}
	else if (Name == "rand")
		BE = new RandEv;
	else if (Name == "nn")
		{
		if (!optset_initial_nn)
			Die("-initial_nn required");
		NNEval *NE = new NNEval;
		const string &FileName = opt(initial_nn);
		ProgressLog("Loading %s\n", FileName.c_str());
		NE->m_NN.FromTabbedFile(FileName);
		BE = NE;
		}
	else if (Name == "nn2")
		{
		if (!optset_initial_nn)
			Die("-initial_nn2 required");
		NNEval *NE = new NNEval;
		const string &FileName = opt(initial_nn2);
		ProgressLog("Loading %s\n", FileName.c_str());
		NE->m_NN.FromTabbedFile(FileName);
		BE = NE;
		}
	else if (Name == "bestnn")
		{
		NNEval *NE = new NNEval;
		CreateBestNN(NE->m_NN);
		BE = NE;
		}
	else if (Name == "tdg")
		{
		NNEval *NE = new NNEval;
		NE->m_NN.Init(BNA_tdg, Hidden, true);
		BE = NE;
		}
	else if (Name == "tdgf")
		{
		NNEval *NE = new NNEval;
		NE->m_NN.Init(BNA_tdgf, Hidden, true);
		BE = NE;
		}
	else if (Name == "nn")
		{
		asserta(optset_input_nn);
		NNEval *NE = new NNEval;
		NE->m_NN.FromTabbedFile(opt(input_nn));
		BE = NE;
		}
	else
		Die("CreateEval(%s)", Name.c_str());

	asserta(BE != 0);
	BE->m_Name = Name;
	return BE;
	}

float BgEval::GetWhiteValueAfter_Impl(const BgBoard &Board)
	{
	Die("GetWhiteValueAfter_Impl");
	return 0;
	}

float BgEval::GetBlackValueAfter_Impl(const BgBoard &Board)
	{
	Die("GetBlackValueAfter_Impl");
	return 0;
	}

float BgEval::GetWhiteValueBefore_Impl(const BgBoard &Board)
	{
	Die("GetWhiteValueBefore_Impl");
	return 0;
	}

float BgEval::GetBlackValueBefore_Impl(const BgBoard &Board)
	{
	Die("GetBlackValueBefore_Impl");
	return 0;
	}

float BgEval::GetWhiteValueAfter(const BgBoard &Board)
	{
	if (Board.GameOver())
		return Board.GetWhiteReward();
	float Value = GetWhiteValueAfter_Impl(Board);
	return Value;
	}

float BgEval::GetBlackValueAfter(const BgBoard &Board)
	{
	if (Board.GameOver())
		return Board.GetBlackReward();
	float Value = GetBlackValueAfter_Impl(Board);
	return Value;
	}

float BgEval::GetWhiteValueBefore(const BgBoard &Board)
	{
	if (Board.GameOver())
		return Board.GetWhiteReward();
	float Value = GetWhiteValueBefore_Impl(Board);
	return Value;
	}

float BgEval::GetBlackValueBefore(const BgBoard &Board)
	{
	if (Board.GameOver())
		return Board.GetBlackReward();
	float Value = GetBlackValueBefore_Impl(Board);
	return Value;
	}

void BgEval::DoWhiteMove(BgBoard &Board)
	{
	unsigned Move = GetWhiteMove(Board);
	Board.DoWhiteMove(Move);
	}

void BgEval::DoBlackMove(BgBoard &Board)
	{
	unsigned Move = GetBlackMove(Board);
	Board.DoBlackMove(Move);
	}

void BgEval::DoMove(BgBoard &Board, bool White)
	{
	if (White)
		DoWhiteMove(Board);
	else
		DoBlackMove(Board);
	}

unsigned BgEval::GetWhiteMove(const BgBoard &Board)
	{
	vector<unsigned> Moves;
	Board.GetWhiteMoves(Moves);
	unsigned BestMove = UINT_MAX;
	float BestValue = MINUS_INF;
	const unsigned N = SIZE(Moves);
	for (unsigned i = 0; i < N; ++i)
		{
		unsigned Move = Moves[i];
		BgBoard Next;
		Next.Copy(Board);
		Next.DoWhiteMove(Move);
		float Value = GetWhiteValueAfter(Next);
		if (Value > BestValue)
			{
			BestMove = Move;
			BestValue = Value;
			}
		}
	return BestMove;
	}

unsigned BgEval::GetBlackMove_Flip(const BgBoard &Board)
	{
	BgBoard FlippedBoard;
	FlippedBoard.CopyAndFlip(Board);
	unsigned Move = GetWhiteMove(FlippedBoard);
	return Move;
	}

unsigned BgEval::GetBlackMove_Max(const BgBoard &Board)
	{
	vector<unsigned> Moves;
	Board.GetBlackMoves(Moves);
	unsigned BestMove = UINT_MAX;
	float BestValue = MINUS_INF;
	const unsigned N = SIZE(Moves);
	for (unsigned i = 0; i < N; ++i)
		{
		unsigned Move = Moves[i];
		BgBoard Next;
		Next.Copy(Board);
		Next.DoBlackMove(Move);
		float Value = GetBlackValueAfter(Next);
		if (Value > BestValue)
			{
			BestMove = Move;
			BestValue = Value;
			}
		}
	return BestMove;
	}

unsigned BgEval::GetBlackMove_Min(const BgBoard &Board)
	{
	vector<unsigned> Moves;
	Board.GetBlackMoves(Moves);
	unsigned BestMove = UINT_MAX;
	float BestValue = PLUS_INF;
	const unsigned N = SIZE(Moves);
	for (unsigned i = 0; i < N; ++i)
		{
		unsigned Move = Moves[i];
		BgBoard Next;
		Next.Copy(Board);
		Next.DoBlackMove(Move);
		float Value = GetWhiteValueBefore(Next);
		if (Value < BestValue)
			{
			BestMove = Move;
			BestValue = Value;
			}
		}
	return BestMove;
	}

unsigned BgEval::GetBlackMove(const BgBoard &Board)
	{
	switch (m_EB)
		{
	case EB_FLIP:
		return GetBlackMove_Flip(Board);

	case EB_MIN:
		return GetBlackMove_Min(Board);

	case EB_MAX:
		return GetBlackMove_Max(Board);
		}

	asserta(false);
	return UINT_MAX;
	}

void cmd_eval()
	{
	const string PosId = opt(eval);
	string EvalName = "bestnn";
	if (optset_eval_name)
		EvalName = opt(eval_name);
	bool Flip = opt(flip);

	BgBoard Board;
	Board.FromStr(PosId);

	BgEval &Ev = *BgEval::CreateEval(EvalName);
	float Value = Ev.GetWhiteValueAfter(Board);

	Board.LogMe();
	Board.Draw(stdout);
	ProgressLog("Value = %.5f\n", Value);

	if (!Flip)
		return;

	BgBoard FlippedBoard;
	FlippedBoard.CopyAndFlip(Board);

	FlippedBoard.LogMe();
	FlippedBoard.Draw(stdout);
	Value = Ev.GetWhiteValueAfter(FlippedBoard);
	ProgressLog("Value = %.5f\n", Value);
	}
