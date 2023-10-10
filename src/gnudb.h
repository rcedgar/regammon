#ifndef gnudb_h
#define gnudb_h

#include "bna.h"

void ReadGnuTrainset(const string &FileName, BNA bna,
  vector<string> &PosIds,
  vector<vector<float> > &InputVecs,
  vector<vector<float> > &OutputVecs, bool Flip);

void ReadGnuTrainsetPtrs(const string &FileName, BNA bna,
  vector<string> &PosIds,
  vector<vector<float> *> &InputVecPtrs,
  vector<vector<float> *> &OutputVecPtrs, bool Flip);

#endif // gnudb_h
