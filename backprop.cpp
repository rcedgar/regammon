#include "myutils.h"
#include "sort.h"
#include "nn.h"

// Requires feed-forward already done
void NN::BackPropTD(bool Trace)
	{
	if (Trace)
		{
		Log("\n");
		Log("BackPropTD()\n");
		}

// Last layer
	const unsigned LastLayerIndex = GetLastLayerIndex();

	NNLayer *LastLayer = GetLastLayer();
	const vector<float> &LastActives = LastLayer->m_Actives;

	if (Trace)
		Log("Grad outputs:\n");
	const unsigned LastLayerNodeCount = GetNodeCount(LastLayerIndex);
	asserta(LastLayerNodeCount == 1);
	float a = LastActives[0];
	float Grad = a*(float(1) - a);
	LastLayer->SetBiasGrad(0, Grad);

	if (Trace)
		Log("  Grad output=%.3g\n", Grad);
	if (Trace)
		Log("\n");

	BackProp2(Trace);
	}

// xs is input
// ys is correct output
void NN::BackProp(const vector<float> &xs, const vector<float> &ys, bool Trace)
	{
	if (Trace)
		{
		Log("\n");
		Log("BackProp()\n");
		}

	assert(SIZE(xs) == GetInputNodeCount());
	assert(SIZE(ys) == GetOutputNodeCount());

	FeedForward(xs, Trace);

// Last layer
	const unsigned LastLayerIndex = GetLastLayerIndex();

	NNLayer *LastLayer = GetLastLayer();
	const vector<float> &LastActives = LastLayer->m_Actives;

	if (Trace)
		Log("Grad outputs:\n");
	const unsigned LastLayerNodeCount = GetNodeCount(LastLayerIndex);
	for (unsigned NodeIndex = 0; NodeIndex < LastLayerNodeCount; ++NodeIndex)
		{
		float y = ys[NodeIndex];
		float a = LastActives[NodeIndex];
		float GradC_a = (a - y);
		float Delta = GradC_a*a*(float(1) - a);
		LastLayer->UpdateBiasGrad(NodeIndex, Delta);

		if (Trace)
			Log("  GradBias[%u]=%.3g\n", NodeIndex, Delta);
		}
	if (Trace)
		Log("\n");

	BackProp2(Trace);
	}

void NN::BackProp2(bool Trace)
	{
	const unsigned LastLayerIndex = GetLastLayerIndex();
	for (int iLayerIndex = int(LastLayerIndex-1); iLayerIndex > 0; --iLayerIndex)
		{
		unsigned LayerIndex = unsigned(iLayerIndex);
		unsigned NextLayerIndex = LayerIndex + 1;

		if (Trace)
			Log("\nBack-prop layer %u -> %u\n", NextLayerIndex, LayerIndex);

		NNLayer *Layer = GetLayer(LayerIndex);
		NNLayer *NextLayer = GetLayer(NextLayerIndex);

		const vector<float> &Zs = Layer->m_Zs;

		unsigned NodeCount = GetNodeCount(LayerIndex);
		unsigned NextNodeCount = GetNodeCount(NextLayerIndex);
		for (unsigned NodeIndex = 0; NodeIndex < NodeCount; ++NodeIndex)
			{
			float Sum = float(0);
			for (unsigned NextNodeIndex = 0; NextNodeIndex < NextNodeCount;
			  ++NextNodeIndex)
				{
				float Weight = NextLayer->m_Weights[NextNodeIndex][NodeIndex];
				float Delta = NextLayer->GetBiasGrad(NextNodeIndex);
				Sum += Weight*Delta;
				}
			float z = Zs[NodeIndex];
			float Delta = Sum*SigmoidPrime(z);
			Layer->UpdateBiasGrad(NodeIndex, Delta);
			if (Trace)
				Log("  Delta[%u;%u]=%.3g\n", LayerIndex, NodeIndex, Delta);
			}
		if (Trace)
			Log("\n");
		}

	for (int iLayerIndex = int(LastLayerIndex); iLayerIndex > 0; --iLayerIndex)
		{
		unsigned LayerIndex = unsigned(iLayerIndex);
		unsigned PrevLayerIndex = LayerIndex - 1;

		if (Trace)
			Log("Back-prop weights %u\n", LayerIndex);

		NNLayer *Layer = GetLayer(LayerIndex);
		NNLayer *PrevLayer = GetLayer(PrevLayerIndex);

		const vector<float> &PrevAs = PrevLayer->m_Actives;

		unsigned NodeCount = GetNodeCount(LayerIndex);
		unsigned PrevNodeCount = GetNodeCount(PrevLayerIndex);
		for (unsigned NodeIndex = 0; NodeIndex < NodeCount; ++NodeIndex)
			{
			for (unsigned PrevNodeIndex = 0; PrevNodeIndex < PrevNodeCount;
			  ++PrevNodeIndex)
				{
				float Delta = Layer->GetBiasGrad(NodeIndex);
				float Active = PrevLayer->GetActive(PrevNodeIndex);
				float dw = Delta*Active;
				if (Trace)
					Log("  L %u dw[%u;%u]=%.3g\n",
					  LayerIndex, NodeIndex, PrevNodeIndex, dw);
				Layer->UpdateWeightGrad(NodeIndex, PrevNodeIndex, dw);
				}
			}
		if (Trace)
			Log("\n");
		}

	++m_BackPropCounter;
	}

void NN::InitBackProp()
	{
	unsigned LayerCount = GetLayerCount();
	for (unsigned LayerIndex = 0; LayerIndex < LayerCount; ++LayerIndex)
		{
		NNLayer *Layer = GetLayer(LayerIndex);
		Layer->ZeroBackProp();
		}
	m_BackPropCounter = 0;
	}

void NN::BackPropUpdate(float Eta, bool Trace)
	{
	if (m_BackPropCounter == 0)
		return;

	const float Rate = Eta/m_BackPropCounter;
	if (Trace)
		Log("\nBackPropUpdate(Eta=%.3g) m=%u Rate=%.3g\n",
		  Eta, m_BackPropCounter, Rate);

	for (unsigned LayerIndex = 1; LayerIndex < m_LayerCount; ++LayerIndex)
		{
		if (Trace)
			Log("\nUpdate layer %u\n", LayerIndex);

		unsigned PrevLayerIndex = LayerIndex - 1;
		NNLayer *Layer = GetLayer(LayerIndex);
		const NNLayer *PrevLayer = GetLayer(PrevLayerIndex);

		vector<float> &Biases = Layer->m_Biases;
		vector<vector<float> > &Weights = Layer->m_Weights;

		const unsigned NodeCount = GetNodeCount(LayerIndex);
		const unsigned PrevNodeCount = GetNodeCount(PrevLayerIndex);

		for (unsigned NodeIndex = 0; NodeIndex < NodeCount; ++NodeIndex)
			{
			float SumGrad = Layer->GetSumBiasGrad(NodeIndex);
			float db = SumGrad*Rate;
			float NewBias = Biases[NodeIndex] - db;
			if (Trace)
				Log(" Node %u bias=%.3g sumgrad=%.3g db=%.3g newbias %.3g\n",
				  NodeIndex, Biases[NodeIndex], SumGrad, db, NewBias);
			Biases[NodeIndex] = NewBias;

			for (unsigned PrevNodeIndex = 0; PrevNodeIndex < PrevNodeCount; ++PrevNodeIndex)
				{
				float Weight = Layer->GetWeight(NodeIndex, PrevNodeIndex);
				float SumGrad = Layer->GetSumWeightGrad(NodeIndex, PrevNodeIndex);
				float dw = SumGrad*Rate;
				float NewWeight = Weight - dw;
				if (Trace)
					Log(" Node %u PrevNode %u sumgrad=%.3g oldw=%.3g dw=%.3g neww=%.3g\n",
					  NodeIndex, PrevNodeIndex, SumGrad, Weight, dw, NewWeight);
				Weights[NodeIndex][PrevNodeIndex] = NewWeight;
				}
			}
		}
	}

void NN::BackPropVec(const vector<vector<float> *> &Xs, const vector<vector<float> *> &Ys,
  float Eta, bool Trace)
	{
	InitBackProp();
	const unsigned m = SIZE(Xs);
	assert(SIZE(Ys) == m);
	for (unsigned i = 0; i < m; ++i)
		{
//		ProgressStep(i, m, "Back prop vec");
		const vector<float> *xs = Xs[i];
		const vector<float> *ys = Ys[i];
		BackProp(*xs, *ys, Trace);
		}
	BackPropUpdate(Eta, Trace);
	}

void NN::BackPropMiniBatchEpoch(const vector<vector<float> *> &Xs,
  const vector<vector<float> *> &Ys, unsigned BatchSize, float Eta, bool Trace)
	{
	const unsigned N = SIZE(Xs);
	asserta(N > 0);
	asserta(BatchSize > 0);

	vector<unsigned > Indexes;
	Range(Indexes, N);

	void Shuffle(vector<unsigned> &v);
	Shuffle(Indexes);

	unsigned Lo = 0;
	unsigned Counter = 0;
	for (;;)
		{
		if (Lo >= N)
			return;
		unsigned Hi = Lo + BatchSize - 1;
		if (Hi >= N)
			Hi = N - 1;

		vector<vector<float> *> BatchXs;
		vector<vector<float> *> BatchYs;
		for (unsigned i = Lo; i <= Hi; ++i)
			{
			++Counter;
			unsigned Index = Indexes[i];
			BatchXs.push_back(Xs[Index]);
			BatchYs.push_back(Ys[Index]);
			}
		BackPropVec(BatchXs, BatchYs, Eta, Trace);
		Lo = Hi + 1;
		}
	asserta(Counter == N);
	++m_EpochIndex;
	}
