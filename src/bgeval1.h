#ifndef bgeval1_h
#define bgeval1_h

#include "bgnn.h"

class BgEval1 : public BgEval
	{
public:
	BgNN m_NN;

public:
	BgEval1()
		{
		m_Name = "BgEval1";
		m_EB = EB_FLIP;
		CreateBestNN(m_NN);
		}

	virtual ~BgEval1() {}

public:
	virtual float GetWhiteValueAfter_Impl(const BgBoard &Board);
	virtual float GetBlackValueAfter_Impl(const BgBoard &Board);
	virtual float GetWhiteValueBefore_Impl(const BgBoard &Board);
	virtual float GetBlackValueBefore_Impl(const BgBoard &Board);
	};

#endif // bgeval1_h
