#ifndef sliv_h
#define sliv_h

#include "bgeval.h"

// Neurogammon features
// (1) pip count
// (2) degree of contact
// (3) number of points occupied in home
// (4) number of points occupied in opponent's home
// (5) number of men in opponent's home
// (6) the presence of a "prime" (perfect block)
// (7) blot exposure (probability that a blot can be hit)
// (8) strength of blockade (probability that trapped man can escape)

enum SLIV_FEAT
	{
	SF_MyPips,
	SF_OppPips,
	SF_HomePoints,
	SF_Blots,
	SF_Pips,
	SF_COUNT,
	};

class Sliv : public BgEval
	{
public:
	unsigned m_FeatCount;
	vector<SLIV_FEAT> m_Feats;
	vector<float> m_Weights;
	bool m_Squash;
	vector<float> m_SumDeltas;
	unsigned m_TrainCount;

public:
	Sliv()
		{
		Init();
		}

public:
	virtual float GetWhiteValueAfter_Impl(const BgBoard &Board);

public:
	void Init();
	const char *ToStr(string &s) const;

	void SetRandomWeights();
	void SetZeroWeights();
	void SetDefaultWeights();

	float GetDefaultWeight(SLIV_FEAT Feat) const;

	void LogMe() const;
	float GetFeature(const BgBoard &Board, SLIV_FEAT f) const;
	float GetFeature(const BgBoard &Board, int i) const { return GetFeature(Board, SLIV_FEAT(i)); }
	float GetFeature(const BgBoard &Board, unsigned i) const { return GetFeature(Board, SLIV_FEAT(i)); }
	float GetAcivation(const vector<float> &FeatVec) const;
	void GetFeatureVec(const BgBoard &Board, vector<float> &Feats) const;
	void GetGradVec(const BgBoard &Board, vector<float> &Grads) const;
	void GetEstimatedGradVec(const BgBoard &Board, vector<float> &Grads);
	float GetEstimatedGrad(const BgBoard &Board, unsigned FeatureIndex);
	float GetValueFromFeatureVec(const vector<float> &FeatureVec) const;
	};

#endif // sliv_h
