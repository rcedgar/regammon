#include "myutils.h"
#include "bgboard.h"
#include "nneval.h"
#include "bencher.h"
#include "selfgame.h"

void cmd_traintd()
	{
	const string EvalName = opt(traintd);
	const unsigned GameCount = opt(games);
	const unsigned SaveStep = opt(save_step);
	bool Flip = opt(flip);
	bool FlipGames = opt(flip_games);
	float Lambda = 0.0f;
	if (opt(td1))
		Lambda = 1.0f;

	NNEval &Ev = (NNEval &) *BgEval::CreateEval(EvalName);
	if (optset_initial_nn)
		Ev.m_NN.FromTabbedFile(opt(initial_nn));

	Bencher B;
	for (unsigned GameIndex = 0; GameIndex < GameCount; ++GameIndex)
		{
		float Alpha = Interp(GameIndex, GameCount, alpha);

		const string &s = B.GetProgressStr();
		ProgressStep(GameIndex, GameCount, s.c_str());

		vector<string> PosIds;
		int Points = SelfGame(Ev, PosIds, Flip);
		TD_Game(Ev.m_NN, Flip, PosIds, Points, Lambda, Alpha);
		if (FlipGames)
			{
			vector<string> FlippedPosIds;
			TD_Game(Ev.m_NN, Flip, FlippedPosIds, -Points, Lambda, Alpha);
			}

		bool NewChamp = B.OnGame(Ev, GameIndex, GameCount);
		if (NewChamp)
			{
			string Comments;
			Ps(Comments, "game %u (best)", GameIndex);
			Ev.m_NN.ToTabbedFile(opt(best_nn), Comments);
			}
		if (optset_save_nn && GameIndex > 0 && GameIndex%SaveStep == 0)
			{
			string Comments;
			Ps(Comments, "save after game %u", GameIndex);
			Ev.m_NN.ToTabbedFile(opt(save_nn), Comments);
			}
		}

	string Comments;
	Ps(Comments, "final after games %u (best)", GameCount);
	Ev.m_NN.ToTabbedFile(opt(final_nn), Comments);
	}
