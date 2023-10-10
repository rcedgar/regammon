#ifndef nn_h
#define nn_h

#include "nnlayer.h"

class NN
	{
public:
	unsigned m_LayerCount;
	vector<NNLayer *> m_Layers;
	unsigned m_BackPropCounter;
	unsigned m_EpochIndex;
	
public:
	NN()
		{
		m_LayerCount = 0;
		m_BackPropCounter = 0;
		m_EpochIndex = 0;
		}

public:
	void Clear()
		{
		m_Layers.clear();
		m_BackPropCounter = 0;
		m_EpochIndex = 0;
		};

	void Copy(const NN &rhs);

	void InitRandom(float Sigma);
	unsigned GetLayerCount() const { return m_LayerCount; }
	unsigned GetNodeCount(unsigned LayerIndex) const;
	unsigned GetInputNodeCount() const { return GetNodeCount(0); }
	unsigned GetOutputNodeCount() const { return GetNodeCount(m_LayerCount-1); }
	void AllocAndZero(const vector<unsigned> &NodeCounts);
	void FromTabbedFile(const string &FileName);
	void FromTabbedFile(FILE *f);
	void ToTabbedFile(const string &FileName) const;
	void ToTabbedFile(FILE *f) const;
	unsigned GetLastLayerIndex() const;
	unsigned GetOutputSize() const;
	const vector<float> &GetOutput() const;
	float GetOutput0() const;
	float GetOutput_i(unsigned i) const;

	const NNLayer *GetLayer(unsigned LayerIndex) const;
	NNLayer *GetLayer(unsigned LayerIndex);

	NNLayer *GetLastLayer();
	const NNLayer *GetLastLayer() const;

	float GetLoss(const vector<float> &xs, const vector<float> &ys);
	float GetAvgLossVecs(const vector<vector<float> *> &Xs,
	  const vector<vector<float> *> &Ys);
	float GetBias(unsigned LayerIndex, unsigned NodeIndex) const;
	void SetBias(unsigned LayerIndex, unsigned NodeIndex, float Bias);
	float GetActive(unsigned LayerIndex, unsigned NodeIndex) const;
	float GetWeight(unsigned LayerIndex, unsigned NodeIndex,
	  unsigned NodeIndex1) const;
	void SetWeight(unsigned LayerIndex, unsigned NodeIndex,
	  unsigned NodeIndex1, float Weight);
	void ZeroAllBiases();

// Feedforward
	void FeedForward2(bool Trace = false);
	void FeedForward(const vector<float> &Input, bool Trace = false);
	void UpdateNode(unsigned LayerIndex, unsigned NodeIndex, bool Trace = false);

// Backprop
	void InitBackProp();
	void BackPropTD(bool Trace = false);
	void BackProp(const vector<float> &xs, const vector<float> &ys, bool Trace = false);
	void BackProp2(bool Trace = false);
	void BackPropUpdate(float Eta, bool Trace = false);
	void BackPropVec(const vector<vector<float>  *> &Xs, const vector<vector<float>  *> &Ys,
	  float Eta, bool Trace = false);
	void BackPropMiniBatchEpoch(const vector<vector<float>  *> &Xs, const vector<vector<float>  *> &Ys,
	  unsigned BatchSize, float Eta, bool Trace = false);

// Debugging
	void LogMe() const;
	void DumpTrainSet(const vector<vector<float> *> &Xs,
	  const vector<vector<float> *> &Ys, bool Trace = false);
	};

#endif // nn_h
