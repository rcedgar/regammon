#include "myutils.h"
#include "bgboard.h"

const unsigned MAX_LOG_ERRS = 10;

static void Cvt1(FILE *fOut, const string &Name)
	{
	const string PathName = "e:/db/gnubg/" + Name + "-train-data";

	FILE *f = OpenStdioFile(PathName);
	string Line;
	vector<string> Fields;
	ProgressFileInit(f, "Converting %s", Name.c_str());
	BgBoard Board;
	BgBoard Board2;
	unsigned ContactErrCount = 0;
	unsigned RaceErrCount = 0;
	unsigned CrashedErrCount = 0;
	while (ReadLineStdioFile(f, Line))
		{
		ProgressFileStep();
		if (Line.empty())
			continue;
		if (Line[0] == '#')
			continue;
		Split(Line, Fields, ' ');
		asserta(SIZE(Fields) == 6);

		const string &OldId = Fields[0];
		Board.FromStr(OldId);
		Board.m_Value = (float) StrToFloat(Fields[1]);

//		int iClass = Board.GetGnuClass();

		string NewId;
		Board.ToStr(NewId);

		Board2.FromStr(NewId);
		asserta(BgBoard::Eq(Board, Board2));

		fprintf(fOut, "%s", Name.c_str());
		fprintf(fOut, "\t%s", NewId.c_str());
		for (unsigned i = 0; i < 6; ++i)
			fprintf(fOut, "\t%s", Fields[i].c_str());
		fprintf(fOut, "\n");

		if (Name == "contact" && !Board.IsContact_gnu())
			{
			++ContactErrCount;
			if (ContactErrCount <= MAX_LOG_ERRS)
				{
				Log("\n");
				Log("=== not contact ===\n");
				fprintf(g_fLog, "%s\n", Line.c_str());
				Board.LogMe();
				}
			if (ContactErrCount > MAX_LOG_ERRS)
				Die("max");
			}
		else if (Name == "race" && Board.IsContact_gnu())
			{
			++RaceErrCount;
			if (RaceErrCount <= MAX_LOG_ERRS)
				{
				Log("\n");
				Log("=== not race ===\n");
				fprintf(g_fLog, "%s\n", Line.c_str());
				Board.LogMe();
				}
			}
		else if (Name == "crashed" && !Board.IsCrashed_gnu())
			{
			++CrashedErrCount;
			if (CrashedErrCount <= MAX_LOG_ERRS)
				{
				Log("\n");
				Log("=== not crashed ===\n");
				fprintf(g_fLog, "%s\n", Line.c_str());
				Board.LogMe();
				}
			}
		}
	ProgressFileDone();
	CloseStdioFile(f);
	if (ContactErrCount + RaceErrCount + CrashedErrCount > 0)
		ProgressLog("Errs contact %u, race %u, crashed %u\n",
		  ContactErrCount, RaceErrCount, CrashedErrCount);
	}

void cmd_cvtgnudbs()
	{
	FILE *fOut = CreateStdioFile(opt(cvtgnudbs));
	Cvt1(fOut, "contact");
	Cvt1(fOut, "race");
	Cvt1(fOut, "crashed");
	CloseStdioFile(fOut);
	}
