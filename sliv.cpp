#include "myutils.h"
#include "bgboard.h"
#include "sliv.h"

const char *SFToStr(SLIV_FEAT f)
	{
	switch (f)
		{
#define x(f)	case SF_##f: return #f;
		x(Pips)
		x(MyPips)
		x(OppPips)
		x(HomePoints)
		x(Blots)
#undef x
		}
	asserta(false);
	return "?";
	}

void Sliv::LogMe() const
	{
	for (unsigned i = 0; i < m_FeatCount; ++i)
		{
		if (i > 0)
			Log(", ");
		Log("%s %.3g", SFToStr(SLIV_FEAT(i)), m_Weights[i]);
		}
	Log("\n");
	}

const char *Sliv::ToStr(string &s) const
	{
	s = "";
	for (unsigned i = 0; i < m_FeatCount; ++i)
		{
		if (i > 0)
			Psa(s, ", ");
		Psa(s, "%s %.3g", SFToStr(m_Feats[i]), m_Weights[i]);
		}
	return s.c_str();
	}

void Sliv::Init()
	{
	m_Name = "Sliv";
	m_Squash = true;
	m_TrainCount = 0;

	m_Feats.push_back(SF_Pips);
	m_Feats.push_back(SF_Blots);
	m_Feats.push_back(SF_HomePoints);
	m_FeatCount = SIZE(m_Feats);

	m_Weights.resize(m_FeatCount, MINUS_INF);
	m_SumDeltas.resize(m_FeatCount, 0.0f);

	SetDefaultWeights();

	for (unsigned i = 0; i < m_FeatCount; ++i)
		asserta(m_Weights[i] != MINUS_INF);
	}

float Sliv::GetDefaultWeight(SLIV_FEAT Feat) const
	{
	switch (Feat)
		{
#define f(x, y)	case SF_##x: return float(y);
	f(Pips, 1.49)
	f(HomePoints, 0.323);
	f(Blots, -0.234)
#undef f
		}
	asserta(false);
	return MINUS_INF;
	}

void Sliv::SetDefaultWeights()
	{
	m_Weights.resize(m_FeatCount, MINUS_INF);
	for (unsigned i = 0; i < m_FeatCount; ++i)
		m_Weights[i] = GetDefaultWeight(m_Feats[i]);
	}

void Sliv::SetRandomWeights()
	{
	for (unsigned i = 0; i < m_FeatCount; ++i)
		m_Weights[i] = rand_gaussian(0.0, 0.1);
	}

void Sliv::SetZeroWeights()
	{
	for (unsigned i = 0; i < m_FeatCount; ++i)
		m_Weights[i] = 0.0f;
	}

float Sliv::GetAcivation(const vector<float> &FeatVec) const
	{
	float DotProduct = 0.0f;
	for (unsigned i = 0; i < m_FeatCount; ++i)
		{
		float Weight = m_Weights[i];
		float Feature = FeatVec[i];
		DotProduct += Weight*Feature;
		}
	return DotProduct;
	}

float Sliv::GetValueFromFeatureVec(const vector<float> &FeatVec) const
	{
	float a = GetAcivation(FeatVec);
	if (m_Squash)
		return Sigmoid(a);
	return a;
	}

float Sliv::GetWhiteValueAfter_Impl(const BgBoard &Board)
	{
	vector<float> FeatVec;
	GetFeatureVec(Board, FeatVec);
	float Value = GetValueFromFeatureVec(FeatVec);
	return Value;
	}

float Sliv::GetFeature(const BgBoard &Board, SLIV_FEAT f) const
	{
	switch (f)
		{
	case SF_Pips:
		{
		int MyPipCount = (int) Board.GetPipCount(true);
		int OppPipCount = (int) Board.GetPipCount(false);
		int PipCountDiff = OppPipCount - MyPipCount;
		return PipCountDiff/100.0f;
		}
	case SF_MyPips:
		{
		unsigned MyPipCount = Board.GetPipCount(true);
		return MyPipCount/100.0f;
		}
	case SF_OppPips:
		{
		unsigned OppPipCount = Board.GetPipCount(false);
		return OppPipCount/100.0f;
		}
	case SF_HomePoints:
		return (float) Board.GetHomePointCount(true)/6.0f;

	case SF_Blots:
		return (float) Board.GetBlotCount(true)/3.0f;
		}

	asserta(false);
	return 0.0f;
	}

void Sliv::GetFeatureVec(const BgBoard &Board, vector<float> &Feats) const
	{
	Feats.clear();
	for (unsigned i = 0; i < m_FeatCount; ++i)
		{
		float f = GetFeature(Board, m_Feats[i]);
		Feats.push_back(f);
		}
	}

void Sliv::GetGradVec(const BgBoard &Board, vector<float> &Grads) const
	{
	Grads.clear();
	vector<float> FeatVec;
	GetFeatureVec(Board, FeatVec);
	float a = GetAcivation(FeatVec);
	float s = Sigmoid(a);
	float da = s*(1.0f - s);
	for (unsigned i = 0; i < m_FeatCount; ++i)
		{
		float f = FeatVec[i];
		float g = f*da;
		Grads.push_back(g);
		}
	}

float Sliv::GetEstimatedGrad(const BgBoard &Board, unsigned FeatureIndex)
	{
	asserta(FeatureIndex < m_FeatCount);

	vector<float> FeatVec;
	GetFeatureVec(Board, FeatVec);

	const float dw = 0.01f;

	float SavedWeight = m_Weights[FeatureIndex];

	m_Weights[FeatureIndex] = SavedWeight + dw/2.0f;
	float vhi = GetValueFromFeatureVec(FeatVec);

	m_Weights[FeatureIndex] = SavedWeight - dw/2.0f;
	float vlo = GetValueFromFeatureVec(FeatVec);

	m_Weights[FeatureIndex] = SavedWeight;

	float dv = vhi - vlo;
	float EstimatedGrad = dv/dw;
	return EstimatedGrad;
	}

void Sliv::GetEstimatedGradVec(const BgBoard &Board, vector<float> &Grads)
	{
	Grads.clear();
	for (unsigned i = 0; i < m_FeatCount; ++i)
		{
		float g = GetEstimatedGrad(Board, i);
		Grads.push_back(g);
		}
	}
