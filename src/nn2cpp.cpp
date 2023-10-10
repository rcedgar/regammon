#include "myutils.h"
#include "bgnn.h"

void cmd_nn2cpp()
	{
	BgNN nn;
	nn.FromTabbedFile(opt(nn2cpp));
	nn.ToCPP(opt(output));
	}
