#ifndef randev_h
#define randev_h

#include "bgeval.h"

class RandEv : public BgEval
	{
public:
	virtual float GetWhiteValueAfter_Impl(const BgBoard &Board)
		{
		return float(randu32()%100);
		};
	};

#endif // randev_h
