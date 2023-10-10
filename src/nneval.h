#ifndef nneval_h
#define nneval_h

#include "bgeval.h"
#include "bgnn.h"

class NNEval : public BgEval
	{
public:
	BgNN m_NN;

public:
	NNEval() {}
	virtual ~NNEval() {};

public:
	virtual float GetWhiteValueBefore_Impl(const BgBoard &Board);
	virtual float GetWhiteValueAfter_Impl(const BgBoard &Board);
	virtual float GetBlackValueBefore_Impl(const BgBoard &Board);
	virtual float GetBlackValueAfter_Impl(const BgBoard &Board);

public:
	void Load(const string &FileName);
	};

#endif // nneval_h
