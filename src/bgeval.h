#ifndef bgeval_h
#define bgeval_h

#include "bgboard.h"

enum EVAL_BLACK
	{
	EB_FLIP,	// Flip board and maximize white value of after-state
	EB_MIN,		// Minimize white value of before-state
	EB_MAX,		// Maximize black value of after-state
	};

class BgEval
	{
public:
	string m_Name;
	EVAL_BLACK m_EB;

public:
	BgEval()
		{
		m_Name = "BgEval";
		m_EB = EB_FLIP;
		}

	virtual ~BgEval() {}

public:
	virtual float GetWhiteValueAfter_Impl(const BgBoard &Board);
	virtual float GetBlackValueAfter_Impl(const BgBoard &Board);
	virtual float GetWhiteValueBefore_Impl(const BgBoard &Board);
	virtual float GetBlackValueBefore_Impl(const BgBoard &Board);

public:
	const char *GetName() const { return m_Name.c_str(); }
	float GetWhiteValueBefore(const BgBoard &Board);
	float GetBlackValueBefore(const BgBoard &Board);
	float GetWhiteValueAfter(const BgBoard &Board);
	float GetBlackValueAfter(const BgBoard &Board);

	void DoWhiteMove(BgBoard &Board);
	void DoBlackMove(BgBoard &Board);
	void DoMove(BgBoard &Board, bool White);

	unsigned GetWhiteMove(const BgBoard &Board);
	unsigned GetBlackMove(const BgBoard &Board);

	unsigned GetBlackMove_Flip(const BgBoard &Board);
	unsigned GetBlackMove_Min(const BgBoard &Board);
	unsigned GetBlackMove_Max(const BgBoard &Board);

public:
	static BgEval *CreateEval(const string &Name);
	};

#endif // bgeval_h
