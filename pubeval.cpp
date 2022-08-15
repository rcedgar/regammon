#include "myutils.h"
#include "bgboard.h"
#include "pubeval.h"

void PubEval::Setx()
	{
    for (int i = 0; i < 122; ++i)
		m_x[i] = 0.0f;

    for (int j = 1; j <= 24; ++j)
		{
        int jm1 = j - 1;
        int n = m_Pos[25-j];
        if (n != 0)
			{
            if (n == -1)	m_x[5*jm1 + 0] = 1.0f;
            if (n == 1)		m_x[5*jm1 + 1] = 1.0f;
            if (n >= 2)		m_x[5*jm1 + 2] = 1.0f;
            if (n == 3)		m_x[5*jm1 + 3] = 1.0f;
            if (n >= 4)		m_x[5*jm1 + 4] = (n - 3)/2.0f;
			}
		}

// opponent barmen
    m_x[120] = -(float) (m_Pos[0])/2.0f;

// computer's men off
    m_x[121] = (float) (m_Pos[26])/15.0f;
	}

/***
pos[] is an integer array of dimension 28 which should represent a legal
final board state after the move. 

Elements 1-24 correspond to board locations 1-24 from computer's point of view,
i.e. computer's men move in the negative direction from 24 to 1, and opponent's
men move in the positive direction from 1 to 24.

Computer's men are represented by positive integers, and opponent's
men are represented by negative integers. 

Element 25 represents computer's men on the bar (positive integer).
Element  0 represents opponent's men on the bar (negative integer).
Element 26 represents computer's men off the board (positive integer).
Element 27 represents opponent's men off the board (negative integer).
***/
void PubEval::SetPos(const BgBoard &Board)
	{
	const byte *w = Board.m_w;
	const byte *b = Board.m_b;

	for (int i = 0; i < 28; ++i)
		m_Pos[i] = 0;

	for (int i = 1; i <= 24; ++i)
		{
		int ComputerCount = (int) w[i];
		int OppCount = (int) b[INV(i)];
		if (ComputerCount != 0 && OppCount != 0)
			{
			Board.LogMe();
			Log("ComputerCount[%d] = %d\n", i, ComputerCount);
			Log("OppCount[%d] = %d\n", INV(i), OppCount);
			Die("Overlap");
			}
		m_Pos[25-i] = ComputerCount - OppCount;
		}

	m_Pos[25] = (int) w[BAR];
	m_Pos[0] = -(int) b[BAR];
	m_Pos[26] = (int) w[OFF];
	m_Pos[27] = -(int) b[OFF];
	}

float PubEval::GetWhiteValueAfter_Impl(const BgBoard &Board)
	{
	SetPos(Board);
	Setx();

	if (m_Pos[25] == 15)
		return 9999.9f;

	const float *Weights =
	  (Board.IsContact() ? g_ContactWeights : g_RaceWeights);
	if (m_ContactOnly)
		Weights = g_ContactWeights;

	float v = 0.0f;
	for (int i = 0; i < 122; ++i)
		v += m_x[i]*Weights[i];

	return v;
	}

float PubEval::GetWhiteValueTrace(const BgBoard &Board)
	{
	SetPos(Board);
	Setx();

	if (m_Pos[25] == 15)
		return 9999.9f;

	const float *Weights =
	  (Board.IsContact() ? g_ContactWeights : g_RaceWeights);
	if (m_ContactOnly)
		Weights = g_ContactWeights;

	float v = 0.0f;
	for (int i = 0; i < 122; ++i)
		{
		Log("m_x[%d] = %.3g *= w[%d] = %.3g = %.3g\n",
		  i, m_x[i], i, Weights[i], v);
		v += m_x[i]*Weights[i];
		}

	return v;
	}
