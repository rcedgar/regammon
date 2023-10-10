#include "myutils.h"
#include "cmd.h"

#define IS_TRUE(x)	case CMD_##x: return true;
//#define ____(x)	case CMD_##x: return false;

const char *CmdToStr(CMD Cmd)
	{
	switch (Cmd)
		{
#define A(x)	case CMD_##x: return #x;
#include "cmds.h"
	default:
		asserta(false);
		}
	return 0;
	}

CMD g_Cmd = CMD_none;

static void StoreCmd(CMD x)
	{
	if (g_Cmd != CMD_none)
		Die("Two commands specified %s, %s", CmdToStr(g_Cmd), CmdToStr(x));
	g_Cmd = x;
	}

CMD GetCmd()
	{
	if (g_Cmd != CMD_none)
		return g_Cmd;

	g_Cmd = CMD_none;
#define A(x)	if (optset_##x) StoreCmd(CMD_##x);
#include "cmds.h"

	if (g_Cmd == CMD_none)
		Die("No command specified");

	return g_Cmd;
	}
