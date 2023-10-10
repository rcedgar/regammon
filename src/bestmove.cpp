#include "myutils.h"
#include "bgeval.h"
#include "bgboard.h"
#include "sort.h"

void cmd_bestmove()
	{
	BgEval &Ev = *BgEval::CreateEval(opt(bestmove));

	BgBoard InitialBoard;
	if (optset_sketch)
		InitialBoard.FromSketchFile(opt(sketch));
	else
		InitialBoard.SetStart();

	string Dice = "3-1";
	if (optset_dice)
		Dice = opt(dice);
	vector<string> Fields;
	Split(Dice, Fields, '-');
	asserta(SIZE(Fields) == 2);

	InitialBoard.m_Die1 = StrToUint(Fields[0]);
	InitialBoard.m_Die2 = StrToUint(Fields[1]);

	vector<unsigned> Moves;
	InitialBoard.GetWhiteMoves(Moves);
	const size_t N = Moves.size();
	float BestValue = MINUS_INF;
	unsigned BestMove = UINT_MAX;
	vector<string> MoveStrs;
	vector<string> PosIds;
	vector<float> Values;
	string MoveStr;
	string PosId;
	string BestPosId;
	for (size_t i = 0; i < N; ++i)
		{
		unsigned Move = Moves[i];
		InitialBoard.MoveToStr(Move, MoveStr);
		BgBoard NextBoard;
		NextBoard.Copy(InitialBoard);
		NextBoard.DoWhiteMove(Move);
		float Value = Ev.GetWhiteValueAfter(NextBoard);
		if (Value > BestValue)
			{
			BestValue = Value;
			BestMove = Move;
			NextBoard.ToStr(BestPosId);
			}

		NextBoard.ToStr(PosId);
		Values.push_back(Value);
		PosIds.push_back(PosId);
		MoveStrs.push_back(MoveStr);
		}

	Log("\n");
	InitialBoard.LogMe();
	vector<unsigned> Order(N);;
	QuickSortOrder(Values.data(), unsigned(N), Order.data());
	for (size_t k = 0; k < N; ++k)
		{
		size_t i = Order[k];
		float Value = Values[i];
		const char *PosId = PosIds[i].c_str();
		const char *MoveStr = MoveStrs[i].c_str();
		Log("  %s  %6.4f  %s\n", PosId, Value, MoveStr);
		}

	if (BestValue > 0)
		{
		BgBoard BestBoard;
		BestBoard.FromStr(BestPosId);
		BestBoard.m_Value = BestValue;
		BestBoard.LogMe();
		}
	}
