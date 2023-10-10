#include "myutils.h"
#include "bgboard.h"
#include "bgnn.h"

void cmd_cvt_gnu_trainset()
	{
	const string &InputFileName = opt(cvt_gnu_trainset);
	const string &OutputFileName = opt(output);

	string Line;
	vector<string> Fields;
	FILE *f = OpenStdioFile(InputFileName);
	asserta(optset_output);
	FILE *fOut = CreateStdioFile(opt(output));
	ProgressFileInit(f, "Reading trainset %s", InputFileName.c_str());
	unsigned n = 0;
	unsigned FailCount = 0;
	while (ReadLineStdioFile(f, Line))
		{
		ProgressFileStep();
		if (Line.empty())
			continue;
		if (Line[0] == '#')
			continue;

		Split(Line, Fields, ' ');
		asserta(SIZE(Fields) == 6);
		const string &PosStr = Fields[0];

		BgBoard B;
		B.FromStr(PosStr);
		B.Invert();
		B.AssertValid();
		string PosId;
		B.ToStr(PosId);

	/***
	1. P(X win | X to move)
	2. P(X g or bg win | X to move)
	3. P(X bg win | X to move)
	4. P(X g or bg loss | X to move)
	5. P(X bg loss | X to move)
	***/
		double XW = StrToFloat(Fields[1]);
		double XW2 = StrToFloat(Fields[2]);
		double XWT = StrToFloat(Fields[3]);
		double XL2 = StrToFloat(Fields[4]);
		double XLT = StrToFloat(Fields[5]);

		double OW = 1.0 - XW;		// P(O win | X to move)
		double OW2 = XL2;			// P(O g or bg win | X to move)
		double OL2 = XW2;			// P(O g or bg loss | X to move)

		bool Fail = false;
#define x(x)	\
		if (x < 0.0) { Fail = true; x = 0.0; }; \
		if (x > 1.0) { Fail = true; x = 1.0; }

		x(OW)
		x(OW2)
		x(OL2)

		if (Fail)
			{
			++FailCount;
			Log("Line=%s", Line.c_str());
			Log(" PosId=%s", PosId.c_str());
			Log("OW=%.5f ", 1.0 - XW);
			Log("OW2=%.5f ", OW2);
			Log("OL2=%.5f ", XW2);
			Log("\n");
			}

		Pf(fOut, "%s", PosId.c_str());
		Pf(fOut, "\t%.5f", OW);
		Pf(fOut, "\t%.5f", OW2);
		Pf(fOut, "\t%.5f", OL2);

		Pf(fOut, "\t%s", Line.c_str());
		Pf(fOut, "\n");
		}
	ProgressFileDone();
	if (FailCount > 0)
		Die("%u fails", FailCount);
	CloseStdioFile(f);
	CloseStdioFile(fOut);
	}

static void ReadGnuTrainsetVecs(const string &FileName,
  vector<string> &PosIds,
  vector<float> &OWs,
  vector<float> &OW2s,
  vector<float> &OL2s, bool Flip)
	{
	PosIds.clear();
	OWs.clear();
	OW2s.clear();
	OL2s.clear();

	FILE *f = OpenStdioFile(FileName);
	string Line;
	vector<string> Fields;
	ProgressFileInit(f, "Reading %s", FileName.c_str());
	while (ReadLineStdioFile(f, Line))
		{
		ProgressFileStep();
		if (Line.empty())
			continue;
		if (Line[0] == '#')
			continue;

//              0  1       2       3       4                    5       6       7       8       9
//	        PosId  OW      OW2     OW2     PosStr               XW      XW2     XWT     WL2     XLT
// 794CABADAAAAAA  1.00000 0.97222 0.00000 ADAAAAHIPHBGAAIAAAAA 0.00000 0.00000 0.00000 0.97222 0.13889

		Split(Line, Fields, '\t');
		asserta(SIZE(Fields) >= 4);
		const string &PosId = Fields[0];
		PosIds.push_back(PosId);

#define X(x, n)	float x = (float) StrToFloat(Fields[n]); x##s.push_back(x);
		X(OW,  1)
		X(OW2, 2)
		X(OL2, 3)
#undef X

		if (Flip)
			{
			BgBoard Board;
			Board.FromStr(PosId);
			Board.Flip();
			string FlippedPosId;
			Board.ToStr(FlippedPosId);

			PosIds.push_back(FlippedPosId);
			OWs.push_back(1.0f - OW);
			OW2s.push_back(OL2);
			OL2s.push_back(OW2);
			}
		}
	ProgressFileDone();
	CloseStdioFile(f);
	}

static void GetOutputVec(BNA bna, float OW, float OW2, float OL2,
  vector<float> &OutputVec)
	{
	OutputVec.clear();
	switch (bna)
		{
	case BNA_tdg:
	case BNA_tdgf:
	case BNA_po:
	case BNA_pf:
		float OW1 = OW - OW2;
		float XW = 1 - OW;
		float XW2 = OL2;
		float XW1 = XW - XW2;

		OutputVec.push_back(OW1);
		OutputVec.push_back(OW2);
		OutputVec.push_back(XW1);
		OutputVec.push_back(XW2);
		return;
		}
	asserta(false);
	}

void ReadGnuTrainset(const string &FileName, BNA bna,
  vector<string> &PosIds,
  vector<vector<float> > &InputVecs,
  vector<vector<float> > &OutputVecs, bool Flip)
	{
	PosIds.clear();
	InputVecs.clear();
	OutputVecs.clear();

	vector<float> OWs;
	vector<float> OW2s;
	vector<float> OL2s;
	ReadGnuTrainsetVecs(FileName, PosIds, OWs, OW2s, OL2s, Flip);

	const unsigned N = SIZE(PosIds);
	asserta(SIZE(OWs) == N);
	asserta(SIZE(OW2s) == N);
	asserta(SIZE(OL2s) == N);

	BgBoard Board;
	vector<float> InputVec;
	vector<float> OutputVec;
	for (unsigned i = 0; i < N; ++i)
		{
		ProgressStep(i, N, "Making vectors");

		const string &PosId = PosIds[i];
		float OW = OWs[i];
		float OW2 = OW2s[i];
		float OL2 = OL2s[i];

		Board.FromStr(PosId);
		Board.GetNNInput(bna, false, InputVec);

		GetOutputVec(bna, OW, OW2, OL2, OutputVec);

		InputVecs.push_back(InputVec);
		OutputVecs.push_back(OutputVec);
		}
	}

void ReadGnuTrainsetPtrs(const string &FileName, BNA bna,
  vector<string> &PosIds,
  vector<vector<float> *> &InputVecPtrs,
  vector<vector<float> *> &OutputVecPtrs, bool Flip)
	{
	PosIds.clear();
	InputVecPtrs.clear();
	OutputVecPtrs.clear();

	vector<vector<float> > InputVecs;
	vector<vector<float> > OutputVecs;
	ReadGnuTrainset(FileName, bna, PosIds, InputVecs, OutputVecs, Flip);

	const unsigned N = SIZE(InputVecs);
	asserta(SIZE(OutputVecs) == N);
	
	for (unsigned i = 0; i < N; ++i)
		{
		ProgressStep(i, N, "Vector pointers");

		vector<float> *InputVecPtr = new vector<float>;
		vector<float> *OutputVecPtr = new vector<float>;

		*InputVecPtr = InputVecs[i];
		*OutputVecPtr = OutputVecs[i];

		InputVecPtrs.push_back(InputVecPtr);
		OutputVecPtrs.push_back(OutputVecPtr);
		}
	}

void cmd_gnudb_flip()
	{
	const string &DBFileName = opt(gnudb_flip);
	asserta(optset_input_nn);
	const string &NNFileName = opt(input_nn);

	vector<string> PosIds;
	vector<float> OWs;
	vector<float> OW2s;
	vector<float> OL2s;
	ReadGnuTrainsetVecs(DBFileName, PosIds, OWs, OW2s, OL2s, false);

	BgNN NN;
	NN.FromTabbedFile(NNFileName);

	const unsigned N = SIZE(PosIds);
	asserta(SIZE(OWs) == N);
	asserta(SIZE(OW2s) == N);
	asserta(SIZE(OL2s) == N);

	BgBoard Board;
	vector<float> OutputVec;
	vector<float> FlippedOutputVec;
	for (unsigned i = 0; i < N; ++i)
		{
		ProgressStep(i, N, "Flipping");

		const string &PosId = PosIds[i];
		float OW = OWs[i];
		float OW2 = OW2s[i];
		float OL2 = OL2s[i];


		BgBoard Board;
		BgBoard FlippedBoard;
		Board.FromStr(PosId);
		FlippedBoard.CopyAndFlip(Board);

		string FlippedPosId;
		FlippedBoard.ToStr(FlippedPosId);

		NN.Evaluate(Board, false, OutputVec);
		NN.Evaluate(FlippedBoard, false, FlippedOutputVec);

		Log("%s", PosId.c_str());
		Log("  %s", FlippedPosId.c_str());
		Log("  %.4f(%.4f)", OW, OutputVec[0]);
		Log("  %.4f(%.4f)", OW2, OutputVec[1]);
		Log("  %.4f(%.4f)", OL2, OutputVec[2]);
		Log(" | %.4f(%.4f)", 1.0f - OW, FlippedOutputVec[0]);
		Log("  %.4f(%.4f)", OL2, FlippedOutputVec[1]);
		Log("  %.4f(%.4f)", OW2, FlippedOutputVec[2]);
		Log("\n");
		}
	}
