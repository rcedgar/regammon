#include "myutils.h"

float CalcSigmoid(float x)
	{
	float y = float(1.0f)/(float(1.0f) + expf(-x));
	return y;
	}

float CalcSigmoidPrime(float x)
	{
	float S_x = CalcSigmoid(x);
	float y = S_x*(float(1) - S_x);
	return y;
	}

// Sigmoid(-10) = 5e-5, hopefully close enough to zero.
// Domain is [g_Lo, g_Hi)
// 16384 = 2^14 ~ 1/5e5
//static const unsigned g_TableSize = 1638400;
//static const float g_Lo = float(-30);
//static const float g_Hi = float(30);

static const unsigned g_TableSize = 16384;
static const float g_Lo = float(-10);
static const float g_Hi = float(10);

static const float g_DomainSize = g_Hi - g_Lo;
static float g_Table[g_TableSize];
static float g_TablePrime[g_TableSize];

static bool InitSigmoid()
	{
	for (unsigned i = 0; i < g_TableSize; ++i)
		{
		float x = g_Lo + i*(g_Hi - g_Lo)/g_TableSize;
		g_Table[i] = CalcSigmoid(x);
		g_TablePrime[i] = CalcSigmoidPrime(x);
		}
	return true;
	}

static bool InitDone = InitSigmoid();

static float GetSigmoid_Table(float x)
	{
	if (x < g_Lo + 0.001)
		return g_Table[0];
	if (x > g_Hi - 0.001)
		return g_Table[g_TableSize-1];
	unsigned i = unsigned(((x - g_Lo)/g_DomainSize)*g_TableSize + 0.5);
	assert(i < g_TableSize);
	float y = g_Table[i];
	return y;
	}

static float GetSigmoidPrime_Table(float x)
	{
	if (x < g_Lo + 0.001)
		return g_TablePrime[0];
	if (x > g_Hi - 0.001)
		return g_TablePrime[g_TableSize-1];
	unsigned i = unsigned(((x - g_Lo)/g_DomainSize)*g_TableSize + 0.5);
	assert(i < g_TableSize);
	float y = g_TablePrime[i];
	return y;
	}

float Sigmoid(float x)
	{
	return GetSigmoid_Table(x);
	}

float SigmoidPrime(float x)
	{
	return GetSigmoidPrime_Table(x);
	}

#if 0
static void Test1(double a)
	{
	float x = float(a);
	ProgressLog("x %.3g ", x);
	ProgressLog(", Calc %.3g ", CalcSigmoid(x));
	ProgressLog(", Lookup %.3g ", Sigmoid(x));
	ProgressLog(", Calc' %.3g ", CalcSigmoidPrime(x));
	ProgressLog(", Lookup' %.3g ", SigmoidPrime(x));
	ProgressLog("\n");
	}

void cmd_test_sigmoid()
	{
	Test1(-1);
	Test1(0);
	Test1(1);
	Test1(0.1);
	Test1(0.2);
	Test1(0.3);
	Test1(0.4);
	Test1(0.5);
	Test1(0.6);
	Test1(0.7);
	Test1(0.8);
	Test1(0.9);
	Test1(-0.1);
	Test1(-0.2);
	Test1(-0.3);
	Test1(-0.4);
	Test1(-0.5);
	Test1(-0.6);
	Test1(-0.7);
	Test1(-0.8);
	Test1(-0.9);
	}
#endif // 0
