#ifndef bgnn_h
#define bgnn_h

#include "nn.h"
#include "bna.h"

class BgBoard;

class BgNN : public NN
	{
public:
	BNA m_BNA;
	vector<float> m_InputVec;

public:
	BgNN()
		{
		m_BNA = BNA_undefined;
		}

public:
	void Init(BNA bna, unsigned HiddenNodeCount, bool Random);
	void FromTabbedFile(const string &FileName);
	void ToTabbedFile(const string &FileName, const string &Comments = "") const;
	void ToCPP(const string &FileName) const;
	void AssertIO() const;

	float GetWhiteValueBefore(const BgBoard &Board);
	float GetWhiteValueAfter(const BgBoard &Board);
	float GetBlackValueBefore(const BgBoard &Board);
	float GetBlackValueAfter(const BgBoard &Board);

	void Evaluate(const BgBoard &Board, bool WhiteTurn,
	  vector<float> &OutputVec);
	void Update(const BgBoard &Board, bool WhiteTurn,
	  const vector<float> &TargetOutputVec, float ALpha);
	void GetWinOutputVec(bool WhiteWin, bool Gammon,
	  vector<float> &OutputVec) const;
	void FlipOutputVec(vector<float> &OutputVec) const;

public:
	static unsigned BNAInputs(BNA bna);
	static unsigned BNAOutputs(BNA bna);
	};

void CreateBestNN(BgNN &nn);

#endif // bgnn_h
