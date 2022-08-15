#include "myutils.h"
#include "bgboard.h"
#include "bgnn.h"
#include "gnudb.h"
#include "winrate.h"
#include "pubeval.h"
#include "nneval.h"

static bool g_Dump = false;
static bool g_ZeroBiases = false;
static FILE *g_fTrain;

static float GetAvgE(NN &nn, const vector<vector<float> *> &Xs,
  const vector<vector<float> *> &Ys, vector<float> &AvgEs)
	{
	AvgEs.clear();
	const unsigned N = SIZE(Xs);
	asserta(SIZE(Ys) == N);
	double Sum = 0.0;
	const unsigned OutputSize = nn.GetOutputSize();
	vector<float> Sums(OutputSize);

	for (unsigned i = 0; i < N; ++i)
		{
		const vector<float> &InputVec = *Xs[i];
		const vector<float> &TargetVec = *Ys[i];
		asserta(SIZE(TargetVec) == OutputSize);

		nn.FeedForward(InputVec);
		const vector<float> &OutputVec = nn.GetOutput();
		for (unsigned j = 0; j < OutputSize; ++j)
			{
			float ytrue = TargetVec[j];
			float ypred = OutputVec[j];
			float d = ypred - ytrue;
			Sums[j] += fabs(d);
			}
		}

	float Total = 0.0f;
	for (unsigned j = 0; j < OutputSize; ++j)
		{
		float Avg = Sums[j]/N;
		Total += Avg;
		AvgEs.push_back(Avg);
		}
	float AvgE = Total/OutputSize;
	return AvgE;
	}

void cmd_bgsuptrain()
	{
	const string &TrainsetFileName = opt(bgsuptrain);
	g_fTrain = CreateStdioFile(opt(trainout));
	g_ZeroBiases = opt(zero_biases);
	bool Flip = opt(flip);

	BNA bna = BNA_tdg;
	if (optset_bna)
		bna = StrToBNA(opt(bna));
	ProgressLog("BNA %s\n", BNAToStr(bna));

	unsigned Epochs = opt(epochs);
	unsigned BatchSize = opt(batch);

	unsigned Hidden = 50;
	if (optset_hidden)
		Hidden = StrToUint(opt(hidden));

	NNEval Ev;
	BgNN &nn = Ev.m_NN;
	nn.Init(bna, Hidden, true);
	if (g_ZeroBiases)
		{
		ProgressLog("Zero biases\n");
		nn.ZeroAllBiases();
		}
	const unsigned OutputSize = nn.GetOutputSize();

	vector<vector<float> *> Xs;
	vector<vector<float> *> Ys;
	vector<string> PosIds;
	ReadGnuTrainsetPtrs(TrainsetFileName, bna, PosIds, Xs, Ys, Flip);

	const unsigned BoardCount = SIZE(Xs);
	ProgressLog("%u boards\n", BoardCount);

	vector<float> AvgEs;
	float AvgE = GetAvgE(nn, Xs, Ys, AvgEs);
	asserta(SIZE(AvgEs) == OutputSize);

	ProgressLog("Initial err %.3g ", AvgE);
	for (unsigned i = 0; i < OutputSize; ++i)
		ProgressLog(" %.3g", AvgEs[i]);
	ProgressLog("\n");

	const unsigned BenchStep = 10;
	const unsigned BenchGameCount = 10000;
	WinRate WR;
	PubEval Pube;
	double BestErr = FLT_MAX;
	double BestPPG = 0;
	for (unsigned Epoch = 0; Epoch < Epochs; ++Epoch)
		{
		if (Epoch > 0 && Epoch%BenchStep == 0)
			{
			Progress("Bench...");
			WR.Run(Ev, Pube, BenchGameCount, false);
			double PPG = WR.GetPPG();
			Progress("  %+.3f\n", PPG);
			if (BestPPG == 0 || PPG > BestPPG)
				BestPPG = PPG;
			}

		float Alpha = Interp(Epoch, Epochs, alpha);
		nn.BackPropMiniBatchEpoch(Xs, Ys, BatchSize, Alpha, false);
		if (g_ZeroBiases)
			nn.ZeroAllBiases();
		float AvgE = GetAvgE(nn, Xs, Ys, AvgEs);
		if (AvgE < BestErr)
			{
			BestErr = AvgE;
			if (optset_best_nn)
				nn.ToTabbedFile(opt(best_nn));
			}
		ProgressLogPrefix("Epoch %u/%u, alpha %.3g err %.3g, best %.3g ppg %+.3f",
		  Epoch+1, Epochs, Alpha, AvgE, BestErr, BestPPG);
		Pf(g_fTrain, "%u	%.3g	%.3g	%.3g	%.3g",
			Epoch+1, Alpha, AvgE, BestErr, BestPPG);
		for (unsigned j = 0; j < OutputSize; ++j)
			Pf(g_fTrain, "	%.3g", AvgEs[j]);
		Pf(g_fTrain, "\n");
		fflush(g_fTrain);
		}
	nn.ToTabbedFile(opt(final_nn));
	CloseStdioFile(g_fTrain);
	}
