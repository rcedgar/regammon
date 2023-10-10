#include "myutils.h"
#include "bgboard.h"

unsigned BgBoard::NF = 0
#define X(c)	+ 1
#include "feats.h"
;

float BgBoard::GetFeature_PipsWhite() const
	{
	return (float) GetPipCount(true)/100.0f;
	}

float BgBoard::GetFeature_PipsBlack() const
	{
	return (float) GetPipCount(false)/100.0f;
	}

float BgBoard::GetFeature_HomePointsWhite() const
	{
	float n = (float) GetHomePointCount(true);
	return n/6.0f;
	}

float BgBoard::GetFeature_HomePointsBlack() const
	{
	float n = (float) GetHomePointCount(false);
	return n/6.0f;
	}

float BgBoard::GetFeature_HomeMenWhite() const
	{
	float n = (float) GetHomeManCount(true);
	return n/12.0f;
	}

float BgBoard::GetFeature_HomeMenBlack() const
	{
	float n = (float) GetHomeManCount(false);
	return n/12.0f;
	}

float BgBoard::GetFeature_HitProbWhite() const
	{
	float HitProb = (float) GetHitProb(true);
	return HitProb;
	}

float BgBoard::GetFeature_HitProbBlack() const
	{
	float HitProb = (float) GetHitProb(false);
	return HitProb;
	}

float BgBoard::GetFeature_BlockQualWhite() const
	{
	float BlockQual = (float) GetBlockadeQual(true);
	return BlockQual;
	}

float BgBoard::GetFeature_BlockQualBlack() const
	{
	float BlockQual = (float) GetBlockadeQual(false);
	return BlockQual;
	}

float BgBoard::GetFeature_ContactWhite() const
	{
	float Contact = (float) GetContactCount(true)/6.0f;
	return Contact;
	}

float BgBoard::GetFeature_ContactBlack() const
	{
	float Contact = (float) GetContactCount(false)/6.0f;
	return Contact;
	}

float BgBoard::GetFeature_BuildersWhite() const
	{
	float n = (float) GetBuilderCount(true);
	return n/6.0f;
	}

float BgBoard::GetFeature_BuildersBlack() const
	{
	float n = (float) GetBuilderCount(false);
	return n/6.0f;
	}
