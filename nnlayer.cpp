#include "myutils.h"
#include "nnlayer.h"

void NNLayer::AllocAndZero(unsigned NodeCount, unsigned PrevNodeCount)
	{
	Clear();

	m_NodeCount = NodeCount;

	m_Actives.resize(NodeCount, 0.0f);
	m_Zs.resize(NodeCount, 0.0f);

	m_Biases.resize(NodeCount, 0.0f);
	m_BiasGrads.resize(NodeCount, 0.0f);
	m_SumBiasGrads.resize(NodeCount, 0.0f);

	if (PrevNodeCount > 0)
		{
		m_Weights.resize(NodeCount);
		m_WeightGrads.resize(NodeCount);
		m_SumWeightGrads.resize(NodeCount);
		for (unsigned i = 0; i < NodeCount; ++i)
			{
			m_Weights[i].resize(PrevNodeCount, 0.0f);
			m_WeightGrads[i].resize(PrevNodeCount, 0.0f);
			m_SumWeightGrads[i].resize(PrevNodeCount, 0.0f);
			}
		}
	}

void NNLayer::ZeroGrads()
	{
	zerof(m_BiasGrads);
	const unsigned N = SIZE(m_WeightGrads);
	for (unsigned i = 0; i < N; ++i)
		zerof(m_WeightGrads[i]);
	}

void NNLayer::ZeroSumGrads()
	{
	zerof(m_SumBiasGrads);
	const unsigned N = SIZE(m_SumWeightGrads);
	for (unsigned i = 0; i < N; ++i)
		zerof(m_SumWeightGrads[i]);
	}

void NNLayer::ZeroBackProp()
	{
	ZeroSumGrads();
	}

void NNLayer::UpdateBiasGrad(unsigned NodeIndex, float db)
	{
	m_BiasGrads[NodeIndex] = db;
	m_SumBiasGrads[NodeIndex] += db;
	}

void NNLayer::UpdateWeightGrad(unsigned NodeIndex, unsigned PrevNodeIndex, float dw)
	{
	m_WeightGrads[NodeIndex][PrevNodeIndex] = dw;
	float v = m_SumWeightGrads[NodeIndex][PrevNodeIndex];
	m_SumWeightGrads[NodeIndex][PrevNodeIndex] = v + dw;
	}

float NNLayer::GetActive(unsigned NodeIndex) const
	{
	return m_Actives[NodeIndex];
	}

float NNLayer::GetBias(unsigned NodeIndex) const
	{
	return m_Biases[NodeIndex];
	}

float NNLayer::GetWeight(unsigned NodeIndex, unsigned PrevNodeIndex) const
	{
	return m_Weights[NodeIndex][PrevNodeIndex];
	}

float NNLayer::GetBiasGrad(unsigned NodeIndex) const
	{
	return m_BiasGrads[NodeIndex];
	}

float NNLayer::GetWeightGrad(unsigned NodeIndex, unsigned PrevNodeIndex) const
	{
	return m_WeightGrads[NodeIndex][PrevNodeIndex];
	}

float NNLayer::GetSumBiasGrad(unsigned NodeIndex) const
	{
	return m_SumBiasGrads[NodeIndex];
	}

float NNLayer::GetSumWeightGrad(unsigned NodeIndex, unsigned PrevNodeIndex) const
	{
	return m_SumWeightGrads[NodeIndex][PrevNodeIndex];
	}

void NNLayer::SetBias(unsigned NodeIndex, float Bias)
	{
	m_Biases[NodeIndex] = Bias;
	}

void NNLayer::SetBiasGrad(unsigned NodeIndex, float Grad)
	{
	m_BiasGrads[NodeIndex] = Grad;
	}

void NNLayer::SetWeight(unsigned NodeIndex, unsigned PrevNodeIndex, float Weight)
	{
	m_Weights[NodeIndex][PrevNodeIndex] = Weight;
	}

void NNLayer::SetWeightGrad(unsigned NodeIndex, unsigned PrevNodeIndex, float Grad)
	{
	m_WeightGrads[NodeIndex][PrevNodeIndex] = Grad;
	}

void NNLayer::SetSumBiasGrad(unsigned NodeIndex, float SumBiasGrad)
	{
	m_SumBiasGrads[NodeIndex] = SumBiasGrad;
	}

void NNLayer::SetSumWeightGrad(unsigned NodeIndex, unsigned PrevNodeIndex, float SumWeightGrad)
	{
	m_SumWeightGrads[NodeIndex][PrevNodeIndex] = SumWeightGrad;
	}
