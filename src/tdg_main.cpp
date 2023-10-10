#include "myutils.h"
#include "cmd.h"

static int MyMain(int argc, char **argv)
	{
#ifdef _MSC_VER
	_putenv("TZ=");
#endif
	setbuf(stdout, 0);
	setbuf(stderr, 0);

	MyCmdLine(argc, argv);

	if (!opt(quiet))
		{
		PrintProgramInfo(stdout);
		PrintCopyright(stdout);
		}

	SetLogFileName(opt(log));
	LogProgramInfoAndCmdLine();

	if (!optset_randseed)
		{
		optset_randseed = true;
		opt_randseed = 1;
		}

	CMD Cmd = GetCmd();
	switch (Cmd)
		{
#define A(x)	case CMD_##x: { void cmd_##x(); cmd_##x(); break; }
#include "cmds.h"
	default:
		asserta(false);
		}

	LogElapsedTimeAndRAM();
	return 0;
	}

int main(int argc, char **argv)
	{
	try
		{
		MyMain(argc, argv);
		}
	catch (...)
		{
		fprintf(stderr, "\n\nEXCEPTION CAUGHT\n\n");
		Log("\n\nEXCEPTION CAUGHT\n\n");
		LogElapsedTimeAndRAM();
		}
	}
