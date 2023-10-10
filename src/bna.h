#ifndef bna_h
#define bna_h

// Backgammon network architecture
enum BNA
	{
#define B(x)	BNA_##x,
#include "bnas.h"
	};

const char *BNAToStr(BNA bna);
BNA StrToBNA(const string &s);

#endif // bna_h
