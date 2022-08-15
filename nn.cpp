#include "myutils.h"
#include "nn.h"
#include <math.h>

void NN::LogMe() const
	{
	Log("\n");
	Log("%u layers\n", m_LayerCount);
	Log("Input:\n");
	for (unsigned NodeIndex = 0; NodeIndex < GetNodeCount(0); ++NodeIndex)
		Log(" [%u]=%.3g", NodeIndex, GetActive(0, NodeIndex));
	Log("\n");
	for (unsigned LayerIndex = 1; LayerIndex < m_LayerCount; ++LayerIndex)
		{
		Log("=== Layer %u ===\n", LayerIndex);
		unsigned NodeCount = GetNodeCount(LayerIndex);
		unsigned PrevNodeCount = GetNodeCount(LayerIndex-1);
		const NNLayer *Layer = GetLayer(LayerIndex);
		for (unsigned i = 0; i < NodeCount; ++i)
			{
			Log(" Node %u bias %.3g gradbias %.3g sumgradbias %.3g\n",
			  i, Layer->GetBias(i), Layer->GetBiasGrad(i), Layer->GetSumBiasGrad(i));

			for (unsigned j = 0; j < PrevNodeCount; ++j)
				Log("  %u,%u weight %.3g gradweight %.3g sumgradweight %.3g\n",
				  i, j, Layer->GetWeight(i, j), Layer->GetWeightGrad(i, j), Layer->GetSumWeightGrad(i, j));
			}
		}
	Log("Output: ");
	for (unsigned i = 0; i < GetOutputNodeCount(); ++i)
		Log(" [%u]=%.3g", i, GetActive(m_LayerCount-1, i));
	Log("\n");
	}

float rand_gaussian(double Mu, double Sigma)
	{
	static const double ATAN1 = atan(1.0);
	unsigned r1 = randu32();
	unsigned r2 = randu32();
	double u1 = double(r1)/UINT_MAX;
	double u2 = double(r2)/UINT_MAX;
	double r = cos(8.0*ATAN1*u2)*sqrt(-2.0*log(u1)); 
	double g = Sigma*r + Mu;
	return float(g);
	}

float NN::GetBias(unsigned LayerIndex, unsigned NodeIndex) const
	{
	assert(LayerIndex > 0);
	const NNLayer *Layer = GetLayer(LayerIndex);
	assert(NodeIndex < Layer->m_NodeCount);
	float Bias = Layer->m_Biases[NodeIndex];
	return Bias;
	}

void NN::SetBias(unsigned LayerIndex, unsigned NodeIndex, float Bias)
	{
	assert(LayerIndex > 0);
	NNLayer *Layer = GetLayer(LayerIndex);
	assert(NodeIndex < Layer->m_NodeCount);
	Layer->m_Biases[NodeIndex] = Bias;
	}

float NN::GetWeight(unsigned LayerIndex, unsigned NodeIndex,
  unsigned NodeIndex1) const
	{
	assert(LayerIndex > 0);
	const NNLayer *Layer = GetLayer(LayerIndex);
	assert(NodeIndex < Layer->m_NodeCount);
	assert(NodeIndex1 < GetNodeCount(LayerIndex-1));
	float Weight = Layer->m_Weights[NodeIndex][NodeIndex1];
	return Weight;
	}

float NN::GetActive(unsigned LayerIndex, unsigned NodeIndex) const
	{
	const NNLayer *Layer = GetLayer(LayerIndex);
	assert(NodeIndex < Layer->m_NodeCount);
	float Active = Layer->m_Actives[NodeIndex];
	return Active;
	}

void NN::SetWeight(unsigned LayerIndex, unsigned NodeIndex,
  unsigned PrevNodeIndex, float Weight)
	{
	assert(LayerIndex > 0);
	NNLayer *Layer = GetLayer(LayerIndex);
	assert(NodeIndex < Layer->m_NodeCount);
	assert(PrevNodeIndex < GetNodeCount(LayerIndex-1));
	Layer->m_Weights[NodeIndex][PrevNodeIndex] = Weight;
	}

unsigned NN::GetNodeCount(unsigned LayerIndex) const
	{
	assert(LayerIndex < m_LayerCount);
	const NNLayer *Layer = m_Layers[LayerIndex];
	unsigned NodeCount = Layer->m_NodeCount;
	return NodeCount;
	}

void NN::AllocAndZero(const vector<unsigned> &NodeCounts)
	{
	Clear();
	const unsigned LayerCount = SIZE(NodeCounts);
	asserta(LayerCount > 1);
	m_Layers.resize(LayerCount);
	for (unsigned LayerIndex = 0; LayerIndex < LayerCount; ++LayerIndex)
		{
		unsigned NodeCount = NodeCounts[LayerIndex];
		unsigned PrevNodeCount = (LayerIndex == 0 ? 0 : NodeCounts[LayerIndex-1]);
		NNLayer *Layer = new NNLayer;
		Layer->AllocAndZero(NodeCount, PrevNodeCount);
		m_Layers[LayerIndex] = Layer;
		}
	m_LayerCount = LayerCount;
	}

void NN::FromTabbedFile(FILE *f)
	{
	string Line;
	vector<string> Fields;
	bool Ok = ReadLineStdioFile(f, Line);
	asserta(Ok);
	Split(Line, Fields, '\t');
	asserta(SIZE(Fields) == 2 && Fields[0] == "layers");
	unsigned LayerCount = StrToUint(Fields[1]);
	vector<unsigned> Sizes;
	for (unsigned Layer = 0; Layer < LayerCount; ++Layer)
		{
		bool Ok = ReadLineStdioFile(f, Line);
		asserta(Ok);
		Split(Line, Fields, '\t');
		asserta(SIZE(Fields) == 3 && Fields[0] == "size");
		unsigned LayerIndex = StrToUint(Fields[1]);
		asserta(LayerIndex == Layer);
		unsigned Size = StrToUint(Fields[2]);
		asserta(Size > 0);
		Sizes.push_back(Size);
		}
	AllocAndZero(Sizes);

	for (unsigned LayerIndex = 1; LayerIndex < LayerCount; ++LayerIndex)
		{
		NNLayer *Layer = m_Layers[LayerIndex];
		unsigned NodeCount = Sizes[LayerIndex];
		asserta(NodeCount == Layer->m_NodeCount);
		for (unsigned NodeIndex = 0; NodeIndex < NodeCount; ++NodeIndex)
			{
			bool Ok = ReadLineStdioFile(f, Line);
			asserta(Ok);
			Split(Line, Fields, '\t');
			asserta(SIZE(Fields) == 4 && Fields[0] == "bias");
			unsigned LayerIndex2 = StrToUint(Fields[1]);
			asserta(LayerIndex2 == LayerIndex);
			unsigned NodeIndex2 = StrToUint(Fields[2]);
			asserta(NodeIndex2 == NodeIndex);
			float Bias = (float) StrToFloat(Fields[3]);
			Layer->m_Biases[NodeIndex] = Bias;
			}
		}

	for (unsigned LayerIndex = 1; LayerIndex < LayerCount; ++LayerIndex)
		{
		unsigned NodeCount = GetNodeCount(LayerIndex);
		unsigned NodeCount1 = GetNodeCount(LayerIndex - 1);
		NNLayer *Layer = m_Layers[LayerIndex];
		for (unsigned NodeIndex = 0; NodeIndex < NodeCount; ++NodeIndex)
			{
			for (unsigned NodeIndex1 = 0; NodeIndex1 < NodeCount1; ++NodeIndex1)
				{
				bool Ok = ReadLineStdioFile(f, Line);
				asserta(Ok);
				Split(Line, Fields, '\t');
				asserta(SIZE(Fields) == 5 && Fields[0] == "weight");

				unsigned LayerIndexIn = StrToUint(Fields[1]);
				asserta(LayerIndexIn == LayerIndex);

				unsigned NodeIndexIn = StrToUint(Fields[2]);
				asserta(NodeIndexIn == NodeIndex);
				asserta(NodeIndexIn < NodeCount);

				unsigned NodeIndex1In = StrToUint(Fields[3]);
				asserta(NodeIndex1In == NodeIndex1);
				asserta(NodeIndex1In < NodeCount1);

				float Weight = (float) StrToFloat(Fields[4]);
				Layer->m_Weights[NodeIndex][NodeIndex1] = Weight;
				}
			}
		}
	}

void NN::FromTabbedFile(const string &FileName)
	{
	FILE *f = OpenStdioFile(FileName);
	FromTabbedFile(f);
	CloseStdioFile(f);
	}

void NN::ToTabbedFile(const string &FileName) const
	{
	if (FileName.empty())
		return;
	FILE *f = CreateStdioFile(FileName);
	ToTabbedFile(f);
	CloseStdioFile(f);
	}

void NN::ToTabbedFile(FILE *f) const
	{
	if (f == 0)
		return;
	unsigned LayerCount = GetLayerCount();
	fprintf(f, "layers	%u\n", LayerCount);
	for (unsigned LayerIndex = 0; LayerIndex < LayerCount; ++LayerIndex)
		fprintf(f, "size	%u	%u\n", LayerIndex, GetNodeCount(LayerIndex));

	for (unsigned LayerIndex = 1; LayerIndex < LayerCount; ++LayerIndex)
		{
		const NNLayer *Layer = m_Layers[LayerIndex];
		unsigned NodeCount = GetNodeCount(LayerIndex);
		for (unsigned NodeIndex = 0; NodeIndex < NodeCount; ++NodeIndex)
			{
			float Bias = Layer->m_Biases[NodeIndex];
			fprintf(f, "bias	%u	%u	%.3g\n", LayerIndex, NodeIndex, Bias);
			}
		}

	for (unsigned LayerIndex = 1; LayerIndex < LayerCount; ++LayerIndex)
		{
		const NNLayer *Layer = m_Layers[LayerIndex];
		unsigned NodeCount = GetNodeCount(LayerIndex);
		unsigned NodeCount1 = GetNodeCount(LayerIndex-1);
		for (unsigned NodeIndex = 0; NodeIndex < NodeCount; ++NodeIndex)
			{
			for (unsigned NodeIndex1 = 0; NodeIndex1 < NodeCount1; ++NodeIndex1)
				{
				float Weight = Layer->m_Weights[NodeIndex][NodeIndex1];
				fprintf(f, "weight	%u	%u	%u	%.3g\n",
				  LayerIndex, NodeIndex, NodeIndex1, Weight);
				}
			}
		}
	}

void NN::UpdateNode(unsigned LayerIndex, unsigned NodeIndex, bool Trace)
	{
	unsigned NodeCount = GetNodeCount(LayerIndex);
	assert(LayerIndex > 0);
	asserta(NodeIndex < NodeCount);
	if (Trace)
		Log("  Node %u[%u]", LayerIndex, NodeIndex);

	unsigned NodeCount1 = GetNodeCount(LayerIndex - 1);

	const NNLayer *Layer1 = m_Layers[LayerIndex-1];
	NNLayer *Layer = m_Layers[LayerIndex];

	float Bias = Layer->m_Biases[NodeIndex];
	const vector<float> &Actives = Layer1->m_Actives;
	const vector<vector<float> > &WeightMx = Layer->m_Weights;

	float z = float(0);
	for (unsigned NodeIndex1 = 0; NodeIndex1 < NodeCount1; ++NodeIndex1)
		{
		float Active = Actives[NodeIndex1];
		float Weight = WeightMx[NodeIndex][NodeIndex1];
		float ThisZ = Active*Weight;
		z += ThisZ;
		if (Trace)
			Log(" %u a(%.3g)*w(%.3g)=%.3g", NodeIndex1, Active, Weight, ThisZ);
		}
	z += Bias;
	float a = Sigmoid(z);
	if (Trace)
		Log("; b=%.3g z=%.3g, a=%.3g\n", Bias, z, a);

	Layer->m_Zs[NodeIndex] = z;
	Layer->m_Actives[NodeIndex] = a;
	}

void NN::FeedForward(const vector<float> &Input, bool Trace)
	{
	if (Trace)
		Log("\n");
	const unsigned LayerCount = GetLayerCount();
	asserta(LayerCount > 1);
	const unsigned InputSize = SIZE(Input);
	unsigned NodeCount0 = GetNodeCount(0);
	asserta(InputSize == NodeCount0);
	NNLayer *Layer0 = m_Layers[0];
	if (Trace)
		Log("FF(0)");
	for (unsigned NodeIndex = 0; NodeIndex < NodeCount0; ++NodeIndex)
		{
		float x = Input[NodeIndex];
		Layer0->m_Actives[NodeIndex] = x;
		if (Trace)
			Log(" x[%u]=%.3g", NodeIndex, x);
		}
	if (Trace)
		Log("\n");
	FeedForward2(Trace);
	}

void NN::FeedForward2(bool Trace)
	{
	const unsigned LayerCount = GetLayerCount();
	for (unsigned LayerIndex = 1; LayerIndex < LayerCount; ++LayerIndex)
		{
		if (Trace)
			Log("FF(%u)\n", LayerIndex);
		NNLayer *Layer = m_Layers[LayerIndex];
		unsigned NodeCount = GetNodeCount(LayerIndex);
		for (unsigned NodeIndex = 0; NodeIndex < NodeCount; ++NodeIndex)
			UpdateNode(LayerIndex, NodeIndex, Trace);
		}
	}

unsigned NN::GetLastLayerIndex() const
	{
	unsigned LayerCount = GetLayerCount();
	assert(LayerCount > 1);
	return LayerCount - 1;
	}

unsigned NN::GetOutputSize() const
	{
	unsigned LastLayerIndex = GetLastLayerIndex();
	unsigned Size = GetNodeCount(LastLayerIndex);
	return Size;
	}

NNLayer *NN::GetLayer(unsigned LayerIndex)
	{
	assert(LayerIndex < m_LayerCount);
	return m_Layers[LayerIndex];
	}

const NNLayer *NN::GetLayer(unsigned LayerIndex) const
	{
	assert(LayerIndex < m_LayerCount);
	return m_Layers[LayerIndex];
	}

const NNLayer *NN::GetLastLayer() const
	{
	assert(m_LayerCount > 1);
	return m_Layers[m_LayerCount - 1];
	}

NNLayer *NN::GetLastLayer()
	{
	assert(m_LayerCount > 1);
	return m_Layers[m_LayerCount - 1];
	}

const vector<float > &NN::GetOutput() const
	{
	unsigned LastLayerIndex = GetLastLayerIndex();
	const NNLayer *LastLayer = GetLayer(LastLayerIndex);
	const vector<float> &Output = LastLayer->m_Actives;
	return Output;
	}

float NN::GetOutput0() const
	{
	const vector<float> &Output = GetOutput();
	float Output0 = Output[0];
	return Output0;
	}

float NN::GetOutput_i(unsigned i) const
	{
	const vector<float > &Output = GetOutput();
	float Output_i = Output[i];
	return Output_i;
	}

void NN::InitRandom(float Sigma)
	{
	for (unsigned LayerIndex = 1; LayerIndex < m_LayerCount; ++LayerIndex)
		{
		NNLayer *Layer = GetLayer(LayerIndex);
		const unsigned NodeCount = GetNodeCount(LayerIndex);
		const unsigned NodeCount1 = GetNodeCount(LayerIndex-1);
		for (unsigned NodeIndex = 0; NodeIndex < NodeCount; ++NodeIndex)
			{
			float r = rand_gaussian(0.0, Sigma);
			Layer->m_Biases[NodeIndex] = r;

			vector<vector<float> > &Weights = Layer->m_Weights;
			for (unsigned NodeIndex1 = 0; NodeIndex1 < NodeCount1; ++NodeIndex1)
				{
				float r = rand_gaussian(0.0, Sigma);
				Weights[NodeIndex][NodeIndex1] = r;
				}
			}
		}
	}

float NN::GetAvgLossVecs(const vector<vector<float> *> &Xs,
  const vector<vector<float> *> &Ys)
	{
	float Sum = float(0);
	const unsigned N = SIZE(Xs);
	asserta(SIZE(Ys) == N);
	for (unsigned i = 0; i < N; ++i)
		{
		const vector<float> &x = *Xs[i];
		const vector<float> &y = *Ys[i];
		float Loss = GetLoss(x, y);
		Sum += Loss;
		}
	float Avg = float(Sum/N);
	return Avg;
	}

float NN::GetLoss(const vector<float> &xs, const vector<float> &ys)
	{
	FeedForward(xs);
	const vector<float> &as = GetOutput();

	const unsigned N = SIZE(as);
	asserta(SIZE(ys) == N);
	float Sum = float(0);
	for (unsigned i = 0; i < N; ++i)
		{
		float d = as[i] - ys[i];
		Sum += d*d;
		}
	float Loss = Sum/float(2);
	return Loss;
	}

void NN::DumpTrainSet(const vector<vector<float> *> &Xs,
  const vector<vector<float> *> &Ys, bool Trace)
	{
	const unsigned N = SIZE(Xs);
	asserta(SIZE(Ys) == N);
	for (unsigned i = 0; i < N; ++i)
		{
		const vector<float> &xs = *Xs[i];
		const vector<float> &ys = *Ys[i];
		FeedForward(xs, Trace);
		const vector<float> &as = GetOutput();

		Log("%5u", i);
		Log(" x<");
		for (unsigned j = 0; j < SIZE(xs); ++j)
			{
			if (j > 0)
				Log(", ");
			Log("%.3g", xs[j]);
			}
		Log(">, y<");
		for (unsigned j = 0; j < SIZE(ys); ++j)
			{
			if (j > 0)
				Log(", ");
			Log("%.3g", ys[j]);
			}
		Log(">, a<");
		for (unsigned j = 0; j < SIZE(as); ++j)
			{
			if (j > 0)
				Log(", ");
			Log("%.3g", as[j]);
			}
		Log(">\n");
		}
	}

void NN::Copy(const NN &rhs)
	{
	vector<unsigned> Sizes;
	unsigned LayerCount = rhs.GetLayerCount();
	for (unsigned LayerIndex = 0; LayerIndex < LayerCount; ++LayerIndex)
		{
		unsigned Size = rhs.GetNodeCount(LayerIndex);
		Sizes.push_back(Size);
		}
	AllocAndZero(Sizes);

	for (unsigned LayerIndex = 1; LayerIndex < LayerCount; ++LayerIndex)
		{
		unsigned NodeCount = Sizes[LayerIndex];
		for (unsigned NodeIndex = 0; NodeIndex < NodeCount; ++NodeIndex)
			{
			float Bias = rhs.GetBias(LayerIndex, NodeIndex);
			SetBias(LayerIndex, NodeIndex, Bias);
			}
		}

	for (unsigned LayerIndex = 1; LayerIndex < LayerCount; ++LayerIndex)
		{
		unsigned NodeCount = Sizes[LayerIndex];
		unsigned NodeCount1 = Sizes[LayerIndex-1];
		for (unsigned NodeIndex = 0; NodeIndex < NodeCount; ++NodeIndex)
			{
			for (unsigned NodeIndex1 = 0; NodeIndex1 < NodeCount1; ++NodeIndex1)
				{
				float Weight = rhs.GetWeight(LayerIndex, NodeIndex, NodeIndex1);
				SetWeight(LayerIndex, NodeIndex, NodeIndex1, Weight);
				}
			}
		}
	}

void NN::ZeroAllBiases()
	{
	unsigned LayerCount = GetLayerCount();
	for (unsigned LayerIndex = 1; LayerIndex < LayerCount; ++LayerIndex)
		{
		unsigned NodeCount = GetNodeCount(LayerIndex);
		for (unsigned NodeIndex = 0; NodeIndex < NodeCount; ++NodeIndex)
			SetBias(LayerIndex, NodeIndex, 0.0f);
		}
	}
