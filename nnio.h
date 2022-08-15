#ifndef nnio_h
#define nnio_h

enum NNI
	{
#define X(x)	NNI_##x,
#include "nnis.h"
	};

enum NNO
	{
#define X(x)	NNO_##x,
#include "nnos.h"
	};

#endif // nnio_h
