#ifndef MY_VERSION
#define MY_VERSION	"0.1"
#endif

#define A(x)		STR_OPT(x)
#include "cmds.h"

STR_OPT(log)
STR_OPT(output)
STR_OPT(data)
STR_OPT(hidden)
STR_OPT(board)
STR_OPT(input_nn)
STR_OPT(output_nn)
STR_OPT(initial_nn)
STR_OPT(initial_nn2)
STR_OPT(final_nn)
STR_OPT(best_nn)
STR_OPT(tabbedout)
STR_OPT(roll)
STR_OPT(nn_contact)
STR_OPT(nn_race)
STR_OPT(nn_crashed)
STR_OPT(sliv_init)
STR_OPT(sketch)
STR_OPT(dice)
STR_OPT(traceboards)
STR_OPT(tracevalues)
STR_OPT(phase)
STR_OPT(names)
STR_OPT(bna)
STR_OPT(eval_name)
STR_OPT(trainout)
STR_OPT(trainset)
STR_OPT(race_nn)
STR_OPT(contact_nn)
STR_OPT(crashed_nn)
STR_OPT(save_nn)

UNS_OPT(threads,			1,			0,			UINT_MAX)	// -
UNS_OPT(randseed,			1,			0,			UINT_MAX)	// -
UNS_OPT(epochs,				1,			0,			UINT_MAX)	// -
UNS_OPT(batch,				1,			0,			UINT_MAX)	// -
UNS_OPT(games,				1,			0,			UINT_MAX)	// -
UNS_OPT(game_index,			1,			0,			UINT_MAX)	// -
UNS_OPT(nodes,				1,			0,			UINT_MAX)	// -
UNS_OPT(traceint,			UINT_MAX,	0,			UINT_MAX)	// -
UNS_OPT(trace,				0,			0,			UINT_MAX)	// -
UNS_OPT(h,					0,			0,			UINT_MAX)	// -
UNS_OPT(benchgames,			40000,		0,			UINT_MAX)	// -
UNS_OPT(simiters,			10000,		0,			UINT_MAX)	// -
UNS_OPT(save_step,			50000,		1,			UINT_MAX)	// -

FLT_OPT(alpha,				0.1,		0.0,		1.0)		// -
FLT_OPT(alpha_start,		0.1,		0.0,		1.0)		// -
FLT_OPT(alpha_end,			0.1,		0.0,		1.0)		// -
FLT_OPT(epsilon,			0.0,		0.0,		1.0)		// -
FLT_OPT(rand_sigma,			0.0,		0.0,		1.0)		// -
FLT_OPT(pwin1,				0.5,		0.0,		1.0)		// -
FLT_OPT(pdouble,			0.3,		0.0,		3.0)		// -

FLAG_OPT(ttplus)
FLAG_OPT(tt3)
FLAG_OPT(log_strs)
FLAG_OPT(log_boards)
FLAG_OPT(init_zero)
FLAG_OPT(tt82)
FLAG_OPT(pescores)
FLAG_OPT(dumptrainset)
FLAG_OPT(bgnn2)
FLAG_OPT(flip)
FLAG_OPT(diag)
FLAG_OPT(train1)
FLAG_OPT(mypube_c)
FLAG_OPT(zero_biases)
FLAG_OPT(white_turn)
FLAG_OPT(flip_games)
FLAG_OPT(td1)

FLAG_OPT(quiet)
FLAG_OPT(compilerinfo)

#undef FLAG_OPT
#undef UNS_OPT
#undef FLT_OPT
#undef STR_OPT
