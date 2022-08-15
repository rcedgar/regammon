#include "myutils.h"
#include "bgboard.h"
#include "nneval.h"

void NNEval::Load(const string &FileName)
	{
	m_NN.FromTabbedFile(FileName);
	m_NN.InitBackProp();
	}

float NNEval::GetWhiteValueAfter_Impl(const BgBoard &Board)
	{
	asserta(!Board.GameOver());
	float v = m_NN.GetWhiteValueAfter(Board);
	return v;
	}

float NNEval::GetWhiteValueBefore_Impl(const BgBoard &Board)
	{
	asserta(!Board.GameOver());
	float v = m_NN.GetWhiteValueBefore(Board);
	return v;
	}

float NNEval::GetBlackValueBefore_Impl(const BgBoard &Board)
	{
	asserta(!Board.GameOver());
	float v = m_NN.GetBlackValueBefore(Board);
	return v;
	}

float NNEval::GetBlackValueAfter_Impl(const BgBoard &Board)
	{
	asserta(!Board.GameOver());
	float v = m_NN.GetBlackValueAfter(Board);
	return v;
	}

void cmd_nneval()
	{
	const string PosId = opt(nneval);
	string EvalName = "bestnn";
	if (optset_eval_name)
		EvalName = opt(eval_name);
	bool Flip = opt(flip);

	BgBoard Board;
	Board.FromStr(PosId);

	NNEval &Ev = (NNEval &) *BgEval::CreateEval(EvalName);
	BgNN &nn = Ev.m_NN;

	BNA bna = nn.m_BNA;
	asserta(bna == BNA_tdg || bna == BNA_tdgf);

	vector<float> OutputVec_WhiteTurn;
	vector<float> OutputVec_BlackTurn;

	nn.Evaluate(Board, true, OutputVec_WhiteTurn);
	nn.Evaluate(Board, false, OutputVec_BlackTurn);

	Board.LogMe();
	Board.Draw(stdout);
	ProgressLog("\n");
	ProgressLog("    O1      O2      X1      X2\n");
	//           123456  123456  123456  123456  

	ProgressLog("%6.4f  %6.4f  %6.4f  %6.4f  O turn\n",
	  OutputVec_WhiteTurn[0],
	  OutputVec_WhiteTurn[1],
	  OutputVec_WhiteTurn[2],
	  OutputVec_WhiteTurn[3]);
	ProgressLog("%6.4f  %6.4f  %6.4f  %6.4f  X turn\n",
	  OutputVec_BlackTurn[0],
	  OutputVec_BlackTurn[1],
	  OutputVec_BlackTurn[2],
	  OutputVec_BlackTurn[3]);

	ProgressLog("\n");
	ProgressLog("OA  = %.4f\n", Ev.GetWhiteValueAfter(Board));
	ProgressLog("OB = %.4f\n", Ev.GetWhiteValueBefore(Board));
	ProgressLog("XA = %.4f\n", Ev.GetBlackValueAfter(Board));
	ProgressLog("XB = %.4f\n", Ev.GetBlackValueBefore(Board));
	}
