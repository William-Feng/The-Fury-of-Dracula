////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// hunter.c: your "Fury of Dracula" hunter AI.
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include "Game.h"
#include "hunter.h"
#include "HunterView.h"

void decideHunterMove(HunterView hv)
{
	// Round round = HvGetRound(hv); Player player = HvGetPlayer(hv);
	// int numRailMoves = (round + player) % 4;
	// bool rail = (numRailMoves != 0);
	// int numReturnedLocs = 0;
	// PlaceId *reachable = HvWhereCanIGoByType(hv, true, rail, true, &numReturnedLocs);
	// Or if can travel by rail, prioritise rail by having road/boat false?

	PlaceId move = CASTLE_DRACULA;
	registerBestPlay((char *)placeIdToAbbrev(move), "Have we nothing Toulouse?");
}
