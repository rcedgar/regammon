#include "myutils.h"
#include "bgnn.h"
#include "bgboard.h"

const char *BNAToStr(BNA bna)
	{
	switch (bna)
		{
#define B(x)	case BNA_##x: return #x;
#include "bnas.h"
		}
	asserta(false);
	return "";
	}

BNA StrToBNA(const string &s)
	{
	if (0) ;
#define B(x)	else if (s == #x) return BNA_##x;
#include "bnas.h"
	Die("Invalid BNA '%s'", s.c_str());
	return BNA_undefined;
	}

unsigned BgNN::BNAInputs(BNA bna)
	{
// Number of inputs for position
	const unsigned POSITION = 196;

// Number of feature input values
	const unsigned FEATURES = 14;

// Number of inputs to represent who is on roll
	const unsigned TURN = 2;

	switch (bna)
		{
	case BNA_tdg:
		return POSITION + TURN;

	case BNA_tdgf:
		return POSITION + TURN + FEATURES;

	case BNA_po:
		return POSITION;

	case BNA_pf:
		return POSITION + FEATURES;
		}

	asserta(false);
	return UINT_MAX;
	}

unsigned BgNN::BNAOutputs(BNA bna)
	{
	switch (bna)
		{
	case BNA_tdg:
	case BNA_tdgf:
	case BNA_po:
	case BNA_pf:
		return 4;
		}
	asserta(false);
	return UINT_MAX;
	}

void BgNN::FromTabbedFile(const string &FileName)
	{
	FILE *f = OpenStdioFile(FileName);
	string Line;

	for (;;)
		{
		bool Ok = ReadLineStdioFile(f, Line);
		if (!Ok)
			Die("Empty file %s", FileName.c_str());
		if (!(Line.empty() || Line[0] == '#'))
			break;
		}
	vector<string> Fields;
	Split(Line, Fields, '\t');
	if (SIZE(Fields) != 2 || Fields[0] != "bna")
		Die("Not bg network %s", FileName.c_str());
	m_BNA = StrToBNA(Fields[1]);
	NN::FromTabbedFile(f);
	AssertIO();
	CloseStdioFile(f);
	}

void BgNN::ToTabbedFile(const string &FileName, const string &Comments) const
	{
	if (FileName == "")
		return;

	AssertIO();
	FILE *f = CreateStdioFile(FileName);

	fprintf(f, "#");
	const unsigned ArgCount = SIZE(g_Argv);
	for (unsigned i = 0; i < ArgCount; ++i)
		fprintf(f, " %s", g_Argv[i].c_str());
	fprintf(f, "\n");
	if (Comments != "")
		fprintf(f, "# %s\n", Comments.c_str());
	fprintf(f, "# svn %s\n", SVN_VERSION);

	fprintf(f, "bna\t%s\n", BNAToStr(m_BNA));
	NN::ToTabbedFile(f);
	CloseStdioFile(f);
	}

void BgNN::AssertIO() const
	{
	unsigned IN = GetInputNodeCount();
	unsigned ON = GetOutputNodeCount();
	unsigned BI = BNAInputs(m_BNA);
	unsigned BO = BNAOutputs(m_BNA);
	asserta(IN == BI && ON == BO);
	}

void BgNN::Init(BNA bna, unsigned HiddenNodeCount, bool Random)
	{
	m_BNA = bna;
	ProgressLog("Initializing NN(%s,%u), %s weights\n",
		BNAToStr(bna), HiddenNodeCount, Random ? "random" : "zero");
	unsigned InputCount = BNAInputs(bna);
	unsigned OutputCount = BNAOutputs(bna);

	vector<unsigned> Sizes;
	Sizes.push_back(InputCount);
	Sizes.push_back(HiddenNodeCount);
	Sizes.push_back(OutputCount);
	AllocAndZero(Sizes);
	if (Random)
		{
		float Sigma = 0.5f;
		if (optset_rand_sigma)
			Sigma = (float) opt(rand_sigma);
		NN::InitRandom(Sigma);
		}
	AssertIO();
	}

void BgNN::Evaluate(const BgBoard &Board, bool WhiteTurn,
  vector<float> &OutputVec)
	{
	Board.GetNNInput(m_BNA, WhiteTurn, m_InputVec);
	FeedForward(m_InputVec);
	OutputVec = GetOutput();
	}

float BgNN::GetWhiteValueAfter(const BgBoard &Board)
	{
	vector<float> OutputVec;
	Evaluate(Board, false, OutputVec);

	switch (m_BNA)
		{
	case BNA_tdg:
	case BNA_tdgf:
	case BNA_po:
	case BNA_pf:
		{
		asserta(SIZE(OutputVec) == 4);
		float OW1 = OutputVec[0];
		float OW2 = OutputVec[1];
		float XW1 = OutputVec[2];
		float XW2 = OutputVec[3];
		float V = OW1 + 2*OW2 - XW1 - 2*XW2;
		return V;
		}
		}
	asserta(false);
	return 0;
	}

float BgNN::GetWhiteValueBefore(const BgBoard &Board)
	{
	vector<float> OutputVec;
	Evaluate(Board, true, OutputVec);

	switch (m_BNA)
		{
	case BNA_tdg:
	case BNA_tdgf:
		{
		asserta(SIZE(OutputVec) == 4);
		float OW1 = OutputVec[0];
		float OW2 = OutputVec[1];
		float XW1 = OutputVec[2];
		float XW2 = OutputVec[3];
		float V = OW1 + 2*OW2 - XW1 - 2*XW2;
		return V;
		}
		}
	asserta(false);
	return 0;
	}

float BgNN::GetBlackValueAfter(const BgBoard &Board)
	{
	vector<float> OutputVec;
	Evaluate(Board, true, OutputVec);

	switch (m_BNA)
		{
	case BNA_tdg:
	case BNA_tdgf:
		{
		asserta(SIZE(OutputVec) == 4);
		float OW1 = OutputVec[0];
		float OW2 = OutputVec[1];
		float XW1 = OutputVec[2];
		float XW2 = OutputVec[3];
		float V = XW1 + 2*XW2 - OW1 - 2*OW2;
		return V;
		}
		}
	asserta(false);
	return 0;
	}

float BgNN::GetBlackValueBefore(const BgBoard &Board)
	{
	vector<float> OutputVec;
	Evaluate(Board, false, OutputVec);

	switch (m_BNA)
		{
	case BNA_tdg:
	case BNA_tdgf:
		{
		asserta(SIZE(OutputVec) == 4);
		float OW1 = OutputVec[0];
		float OW2 = OutputVec[1];
		float XW1 = OutputVec[2];
		float XW2 = OutputVec[3];
		float V = XW1 + 2*XW2 - OW1 - 2*OW2;
		return V;
		}
		}
	asserta(false);
	return 0;
	}

void BgNN::GetWinOutputVec(bool WhiteWin, bool Gammon,
  vector<float> &OutputVec) const
	{
	OutputVec.clear();

	switch (m_BNA)
		{
	case BNA_tdg:
	case BNA_tdgf:
	case BNA_po:
	case BNA_pf:
		{
		if (WhiteWin && !Gammon)
			{
			OutputVec.push_back(1.0f);
			OutputVec.push_back(0.0f);
			OutputVec.push_back(0.0f);
			OutputVec.push_back(0.0f);
			}
		else if (WhiteWin && Gammon)
			{
			OutputVec.push_back(0.0f);
			OutputVec.push_back(1.0f);
			OutputVec.push_back(0.0f);
			OutputVec.push_back(0.0f);
			}
		else if (!WhiteWin && !Gammon)
			{
			OutputVec.push_back(0.0f);
			OutputVec.push_back(0.0f);
			OutputVec.push_back(1.0f);
			OutputVec.push_back(0.0f);
			}
		else if (!WhiteWin && Gammon)
			{
			OutputVec.push_back(0.0f);
			OutputVec.push_back(0.0f);
			OutputVec.push_back(0.0f);
			OutputVec.push_back(1.0f);
			}
		else
			asserta(false);
		return;
		}
		}
	asserta(false);
	}

void BgNN::FlipOutputVec(vector<float> &OutputVec) const
	{
	switch (m_BNA)
		{
	case BNA_tdg:
	case BNA_tdgf:
	case BNA_po:
	case BNA_pf:
		{
		asserta(SIZE(OutputVec) == 4);
		float v0 = OutputVec[0];
		float v1 = OutputVec[1];
		float v2 = OutputVec[2];
		float v3 = OutputVec[3];

		OutputVec[0] = v2;
		OutputVec[1] = v3;
		OutputVec[2] = v0;
		OutputVec[3] = v1;
		return;
		}
		}
	asserta(false);
	}

void BgNN::Update(const BgBoard &Board, bool WhiteTurn,
  const vector<float> &TargetOutputVec, float Alpha)
	{
	Board.GetNNInput(m_BNA, WhiteTurn, m_InputVec);
	InitBackProp();
	BackProp(m_InputVec, TargetOutputVec);
	BackPropUpdate(Alpha);
	}

void BgNN::ToCPP(const string &FileName) const
	{
	if (FileName == "")
		return;

	AssertIO();
	FILE *f = CreateStdioFile(FileName);
	unsigned LayerCount = GetLayerCount();
	asserta(LayerCount == 3);
	unsigned HiddenNodeCount = GetNodeCount(1);
	fprintf(f, "	nn.Init(BNA_%s, %u, false);\n",
	  BNAToStr(m_BNA), HiddenNodeCount);

	for (unsigned LayerIndex = 1; LayerIndex < LayerCount; ++LayerIndex)
		{
		const NNLayer *Layer = m_Layers[LayerIndex];
		unsigned NodeCount = GetNodeCount(LayerIndex);
		for (unsigned NodeIndex = 0; NodeIndex < NodeCount; ++NodeIndex)
			{
			float Bias = Layer->m_Biases[NodeIndex];
			fprintf(f, "	nn.SetBias(%u, %u, float(%.3g));\n",
			  LayerIndex, NodeIndex, Bias);
			}
		}

	for (unsigned LayerIndex = 1; LayerIndex < LayerCount; ++LayerIndex)
		{
		const NNLayer *Layer = m_Layers[LayerIndex];
		unsigned NodeCount = GetNodeCount(LayerIndex);
		unsigned NodeCount1 = GetNodeCount(LayerIndex-1);
		for (unsigned NodeIndex = 0; NodeIndex < NodeCount; ++NodeIndex)
			{
			for (unsigned NodeIndex1 = 0; NodeIndex1 < NodeCount1; ++NodeIndex1)
				{
				float Weight = Layer->m_Weights[NodeIndex][NodeIndex1];
				fprintf(f, "	nn.SetWeight(%u, %u, %u, float(%.3g));\n",
				  LayerIndex, NodeIndex, NodeIndex1, Weight);
				}
			}
		}
	CloseStdioFile(f);
	}
