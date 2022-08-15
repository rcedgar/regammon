#ifndef selfgame_h
#define selfgame_h

#include "bgeval.h"

int SelfGame(BgEval &Ev, vector<string> &PosIds, bool Flip);
void TD_Game(BgNN &nn,  bool Flip, const vector<string> &PosIds,
  int Points, float Lambda, float Alpha);
void FlipPosIds(const vector<string> &PosIds,
  vector<string> &FlippedPosIds);

#endif // selfgame_h
