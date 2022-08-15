#ifndef pubeval_h
#define pubeval_h

#include "bgeval.h"

class PubEval : public BgEval
	{
public:
	bool m_ContactOnly;
	int m_Pos[28];
	float m_x[122];

public:
	PubEval()
		{
		m_ContactOnly = false;
		}

public:
	static const float g_RaceWeights[122];
	static const float g_ContactWeights[122];

public:
	virtual float GetWhiteValueAfter_Impl(const BgBoard &Board);
	float GetWhiteValueTrace(const BgBoard &Board);

public:
	void SetPos(const BgBoard &Board);
	void Setx();
	};

#endif // pubeval_h
