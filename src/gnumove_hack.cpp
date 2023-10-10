#if 0
#include "myutils.h"
#include "bgboard.h"

#if defined(__GNUC__)

#include <stdlib.h>
#include <unistd.h>
#include "sort.h"

static void GetRunName(string &RunName)
	{
	static int pid = INT_MAX;
	if (pid == INT_MAX)
		{
		pid = getpid();
		ProgressLog("pid %d\n", pid);
		}
	static unsigned Counter = 0;
	++Counter;
	Ps(RunName, "pid%d.ply%u", pid, Counter);
	}

void GetGnuMoves(const vector<string> &InPosIds,
  const vector<string> &Dices, vector<string> &GnuMoves,
  vector<string> &GnuMovesx)
	{
	GnuMoves.clear();
	GnuMovesx.clear();
	const unsigned N = SIZE(InPosIds);
	if (N == 0)
		return;

	string RunName;
	GetRunName(RunName);

	string InFileName;
	string OutFileName;
	Ps(InFileName, "/p/res/gwrap/input/%s", RunName.c_str());
	Ps(OutFileName, "/p/res/gwrap/output/%s", RunName.c_str());

	FILE *f = CreateStdioFile(InFileName);
	for (unsigned i = 0; i < N; ++i)
		{
		const string &PosId = InPosIds[i];
		const string &Dice = Dices[i];
		fprintf(f, "%s	%s\n", PosId.c_str(), Dice.c_str());
		}
	CloseStdioFile(f);
	f = 0;

	string CmdLine;
	Ps(CmdLine, "/p/res/gwrap/scripts/run1_hack %s", RunName.c_str());
	int rc = system(CmdLine.c_str());
	if (rc != 0)
		{
		Log("CmdLine='%s'\n", CmdLine.c_str());
		Die("system() = %d, %s", rc, strerror(rc));
		}

	f = OpenStdioFile(OutFileName);
	string Line;
	vector<string> Fields;
	for (unsigned i = 0; i < N; ++i)
		{
		bool Ok = ReadLineStdioFile(f, Line);
		if (!Ok)
			Die("Premature eof in %s\n", OutFileName.c_str());

		Split(Line, Fields, '\t');
		asserta(SIZE(Fields) == 4);

		const string &OldPosIdFlipped = Fields[0];
		const string &Dice = Fields[1];
		const string &GnuMove = Fields[2];
		const string &GnuMovex = Fields[3];
		asserta(Dice == Dices[i]);

		BgBoard BF;
		BF.FromStr(OldPosIdFlipped);
		BF.Flip();
		string OldPosId;
		BF.ToStr(OldPosId);
		asserta(OldPosId == InPosIds[i]);

		GnuMoves.push_back(GnuMove);
		GnuMovesx.push_back(GnuMovex);
		}
	CloseStdioFile(f);
	}

#else // __GNUC__

void GetGnuMoves(const vector<string> &InPosIds,
  const vector<string> &Dices, vector<string> &GnuMoves,
  vector<string> &GnuMovesx)
	{
	Die("GetGnuMoves() not supported");
	}

#endif

void cmd_gnumove()
	{
	Die("-gnumove not supported");
	}
#endif // 0
