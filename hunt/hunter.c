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

////////////////////////////////////////////////////////////////////////
// Function Prototypes
PlaceId startingLocation(HunterView hv);
bool hunterToDraculaReachable(HunterView hv, Player hunter, PlaceId dLocation);


void decideHunterMove(HunterView hv)
{
	Round round = HvGetRound(hv);
	Player player = HvGetPlayer(hv);
	int health = HvGetHealth(hv, player);
	int numRailMoves = (round + player) % 4;
	bool rail = (numRailMoves != 0);
	int numReturnedLocs = 0;
	PlaceId *reachable = HvWhereCanIGoByType(hv, true, rail, true, &numReturnedLocs);
	// Or if can travel by rail, prioritise rail by having road/boat false?

	PlaceId move = HvGetPlayerLocation(hv, player);
	// if (round == 0) {
	// 	move = startingLocation(hv);
	// 	registerBestPlay((char *)placeIdToAbbrev(move), "Have we nothing Toulouse?");
	// 	return;
	// }
	
	int healthThreshold = 3;

	int roundRevealed = -1;
	PlaceId lastDraculaLocation = HvGetLastKnownDraculaLocation(hv, &roundRevealed);
	// Can BFS there
	if (health > healthThreshold && placeIsReal(lastDraculaLocation) && round - roundRevealed < 4) {
		int pathLength = 0;
		move = HvGetShortestPathTo(hv, player, lastDraculaLocation, &pathLength)[0];
	}
	else if (health <= healthThreshold && placeIsReal(lastDraculaLocation) && round - roundRevealed == 1) {
		int playersReachable = 0;
		for (player = PLAYER_LORD_GODALMING; player < PLAYER_MINA_HARKER; player++) {
			if (hunterToDraculaReachable(hv, player, lastDraculaLocation)) playersReachable++;
		}
		if (HvGetHealth(hv, PLAYER_DRACULA) <= (playersReachable * LIFE_LOSS_HUNTER_ENCOUNTER)) {
			int pathLength = 0;
			move = HvGetShortestPathTo(hv, player, lastDraculaLocation, &pathLength)[0];
		}
	}
	// Otherwise, the hunter stays in the same location
	// (resting for health or collaborative research)

	registerBestPlay((char *)placeIdToAbbrev(move), "Have we nothing Toulouse?");
}

PlaceId startingLocation(HunterView hv) {
	Player player = HvGetPlayer(hv);
	PlaceId location = HvGetPlayerLocation(hv, player);
	switch (player) {
		case PLAYER_LORD_GODALMING:
			location = KLAUSENBURG; break;
		case PLAYER_DR_SEWARD:
			location = STRASBOURG; break;
		case PLAYER_VAN_HELSING:
			location = SARAGOSSA; break;
		case PLAYER_MINA_HARKER:
			location = ZAGREB; break;
		default: break;
	}
	return location;
}

bool hunterToDraculaReachable(HunterView hv, Player hunter, PlaceId dLocation) {
	// Extract locations
	int numLocs = 0;
	PlaceId *reachable = HvWhereCanTheyGo(hv, hunter, &numLocs);
	for (int i = 0; i < numLocs; i++)
		if (reachable[i] == dLocation) return true;
	return false;
}