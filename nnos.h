#ifndef X
#error "X not defined"
#endif

X(undefined)
X(v)	// P(W | Black to move)
X(t2)	// P(WhiteWin | state_with_turn), P(BlackWin | state_with_turn)

#undef X
