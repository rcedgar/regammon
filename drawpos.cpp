#include "myutils.h"
#include "bgboard.h"

void cmd_drawpos()
	{
	const string &PosId = opt(drawpos);
	BgBoard Board;
	Board.FromStr(PosId);
	Board.Draw(stdout);
	Board.Draw(g_fLog);

	if (opt(flip))
		{
		printf("\n\n");
		Board.Invert();
		Board.Draw(stdout);
		Board.Draw(g_fLog);
		}
	}

void cmd_sketch2pos()
	{
	const string FileName = opt(sketch2pos);
	BgBoard Board;
	Board.FromSketchFile(FileName);
	Board.Draw(stdout);
	Board.Draw(g_fLog);

	if (opt(flip))
		{
		printf("\n\n");
		Board.Invert();
		Board.Draw(stdout);
		Board.Draw(g_fLog);
		}
	}
