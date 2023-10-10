#ifndef nnlayer_h
#define nnlayer_h

class NNLayer
	{
public:
// Parameters
	unsigned m_NodeCount;
	vector<vector<float> > m_Weights;
	vector<float> m_Biases;

// State
	vector<float> m_Actives;
	vector<float> m_Zs;

// Back-prop
	vector<vector<float> > m_WeightGrads;
	vector<float> m_BiasGrads;

	vector<vector<float> > m_SumWeightGrads;
	vector<float> m_SumBiasGrads;

public:
	NNLayer()
		{
		m_NodeCount = 0;
		}

	void Clear()
		{
		m_NodeCount = 0;
		m_Weights.clear();
		m_Biases.clear();
		m_Actives.clear();
		m_Zs.clear();

		m_SumWeightGrads.clear();
		m_SumBiasGrads.clear();
		}

	~NNLayer()
		{
		Clear();
		}

	void AllocAndZero(unsigned NodeCount, unsigned PrevNodeCount);
	void ZeroGrads();
	void ZeroSumGrads();
	void ZeroBackProp();
	void UpdateBiasGrad(unsigned NodeIndex, float db);
	void UpdateWeightGrad(unsigned NodeIndex, unsigned PrevNodeIndex, float dw);

	float GetActive(unsigned NodeIndex) const;

	float GetBias(unsigned NodeIndex) const;
	float GetWeight(unsigned NodeIndex, unsigned PrevNodeIndex) const;

	float GetSumBiasGrad(unsigned NodeIndex) const;
	float GetBiasGrad(unsigned NodeIndex) const;

	float GetWeightGrad(unsigned NodeIndex, unsigned PrevNodeIndex) const;
	float GetSumWeightGrad(unsigned NodeIndex, unsigned PrevNodeIndex) const;

	void SetBias(unsigned NodeIndex, float Bias);
	void SetBiasGrad(unsigned NodeIndex, float Grad);

	void SetWeight(unsigned NodeIndex, unsigned PrevNodeIndex, float Weight);
	void SetWeightGrad(unsigned NodeIndex, unsigned PrevNodeIndex, float Grad);

	void SetSumBiasGrad(unsigned NodeIndex, float SumBiasGrad);
	void SetSumWeightGrad(unsigned NodeIndex, unsigned PrevNodeIndex, float SumWeightGrad);
	};

#endif // nnlayer_h
