#ifndef B
#error "B not defined"
#endif

/***
Tesauro 1995: "Y[t] is a four component vector corresponding
to the four  possible outcomes of either White or Black winning
either a normal win [single] or a gammon [double]".
Net output vector for BNA_tdg/f and BNA_po/f:
	[0]		P(white win single)
	[1]		P(white win double)
	[3]		P(black win single)
	[4]		P(black win double)
***/

B(undefined)
B(tdg)			// TD-Gammon without features
B(tdgf)			// TD-Gammon with features
B(po)			// Position only (no turn)
B(pf)			// Position and features (no turn)

#undef B
