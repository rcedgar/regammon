#include "myutils.h"
#include <set>

static bool g_InitDone = false;
static set<unsigned> *g_S;

bool DoBench(unsigned GameIndex, unsigned GameCount)
	{
	if (GameIndex == 0 || opt(benchgames) == 0)
		return false;

	if (GameIndex + 1 == GameCount)
		return true;

	if (GameIndex < 100000)
		return GameIndex%10000 == 0;

	return GameIndex%50000 == 0;
	}
