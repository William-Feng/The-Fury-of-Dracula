////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// hunter.c: your "Fury of Dracula" hunter AI.
//
// 2014-07-01   v1.0    Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01   v1.1    Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31   v2.0    Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10   v3.0    Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include "Game.h"
#include "hunter.h"
#include "HunterView.h"
#include <stdio.h>

#define HEALTHTHRESHOLD 3

////////////////////////////////////////////////////////////////////////
// Function Prototypes

// Returns a starting location for a player
static PlaceId startingLocation(HunterView hv);
// Checks whether a hunter can reach a location
static bool nearby(HunterView hv, Player hunter, PlaceId dMove, bool rail);
// Checks how many hunters can reach a location
static int huntersNearby(HunterView hv, PlaceId dMove, bool rail);
// Checks if a player is able to reach a location for their next turn
static bool reachableNextTurn(HunterView hv, Player player, PlaceId location);


void decideHunterMove(HunterView hv)
{
	// Extract information
	Round round = HvGetRound(hv);
    Player player = HvGetPlayer(hv);
    PlaceId move = HvGetPlayerLocation(hv, player);
    int health = HvGetHealth(hv, player);
	int roundRevealed = -1;
    PlaceId lastDraculaLocation = HvGetLastKnownDraculaLocation(hv, &roundRevealed);
	int draculaHealth = HvGetHealth(hv, PLAYER_DRACULA);
	Round previousVampireSpawn = round / 13;
	Round nextMaturity = previousVampireSpawn + 6;

	// Starting location
	if (round == 0) {
		move = startingLocation(hv);
		registerBestPlay((char *)placeIdToAbbrev(move), "JAWA - we don't go by the script");
		return;
	}
	
	// Get reachable locations
	int numReturnedLocs = 0;
	PlaceId *reachable = HvWhereCanIGoByType(hv, true, true, true, &numReturnedLocs);

	for (int i = 0; i < numReturnedLocs; i++) {
		PlaceId city = reachable[i];
		// Check winning conditions
		if (city == lastDraculaLocation && round - roundRevealed <= 1 && placeIsReal(lastDraculaLocation) &&
			draculaHealth <= huntersNearby(hv, lastDraculaLocation, true) * LIFE_LOSS_HUNTER_ENCOUNTER) {
			registerBestPlay((char *)placeIdToAbbrev(city), "JAWA - we don't go by the script");
			free(reachable);
			return;
		}
	}
	free(reachable);

	// Vampire about to mature
	PlaceId vampireLocation = HvGetVampireLocation(hv);
	if (placeIsReal(vampireLocation)) {
		// Calculate shortest path length
		Player closestPlayer = -1; int minPathLength = 100000;
		PlaceId shortestPathStep = NOWHERE;
		for (Player hunter = PLAYER_LORD_GODALMING; hunter < PLAYER_DRACULA; hunter++) {
			// Calculate path
			int pathLength = 0;
			PlaceId *path = HvGetShortestPathTo(hv, hunter, vampireLocation, &pathLength);
			if (pathLength < minPathLength) {
				closestPlayer = hunter;
				minPathLength = pathLength;
				shortestPathStep = path[0];
			}
			free(path);
		}
		// Urgent - about to mature
		if (shortestPathStep != NOWHERE && player == closestPlayer &&
			(round + minPathLength) <= nextMaturity && (nextMaturity - round) <= 2) {
			registerBestPlay((char *)placeIdToAbbrev(shortestPathStep), "JAWA - we don't go by the script");
			return;
		}
	}

	// Rest
	bool sacrifice = false;
	if (draculaHealth <= 20 && (round - roundRevealed) == 1 && reachableNextTurn(hv, player, lastDraculaLocation))
		sacrifice = true;
	if (!sacrifice && health <= HEALTHTHRESHOLD) {
		registerBestPlay((char *)placeIdToAbbrev(move), "JAWA - we don't go by the script");
		return;
	}

	// General Vampire
	if (!sacrifice && placeIsReal(vampireLocation)) {
		// Calculate shortest path length
		int closestPlayer = -1; int minPathLength = 100000;
		PlaceId shortestPathStep = NOWHERE;
		for (Player hunter = PLAYER_LORD_GODALMING; hunter < PLAYER_DRACULA; hunter++) {
			// Calculate path
			int pathLength = 0;
			PlaceId *path = HvGetShortestPathTo(hv, hunter, vampireLocation, &pathLength);
			if (pathLength < minPathLength) {
				closestPlayer = hunter;
				minPathLength = pathLength;
				shortestPathStep = path[0];
			}
			free(path);
		}
		if (shortestPathStep != NOWHERE && player == closestPlayer &&
			(round + minPathLength) <= nextMaturity && (nextMaturity - round) <= 4) {
			registerBestPlay((char *)placeIdToAbbrev(shortestPathStep), "JAWA - we don't go by the script");
			return;
		}
	}

	bool skip = false;
	// Dracula BFS
	if (move != lastDraculaLocation && placeIsReal(lastDraculaLocation) && round - roundRevealed <= 4) {
		int pathLength = 0;
		PlaceId *path = HvGetShortestPathTo(hv, player, lastDraculaLocation, &pathLength);
		PlaceId shortestPathStep = path[0];
		free(path);
		if (pathLength != 0) {
			registerBestPlay((char *)placeIdToAbbrev(shortestPathStep), "JAWA - we don't go by the script");
			return;
		} else {
			skip = true;
		}
	}

	// BFS to trap encounters
	Round trapRound = -1;
	PlaceId lastTrapLocation = recentTrapEncounter(hv, &trapRound);
	if (!skip && lastTrapLocation != NOWHERE && placeIsReal(lastTrapLocation) && (round - trapRound) < 5) {
		int pathLength = 0;
		PlaceId *path = HvGetShortestPathTo(hv, player, lastTrapLocation, &pathLength);
		PlaceId shortestPathStep = path[0];
		free(path);
		if (pathLength != 0) {
			registerBestPlay((char *)placeIdToAbbrev(shortestPathStep), "JAWA - we don't go by the script");
			return;
		}
	}
	
	// Collaborative research
	if (!skip && round >= 6 && (!placeIsReal(lastDraculaLocation) || (nextMaturity - round) == 6)) {
		registerBestPlay((char *)placeIdToAbbrev(move), "JAWA - we don't go by the script");
		return;
	}

	// Send nearest player towards CD if Dracula's health is low
	if (draculaHealth <= 20) {
		Player closestPlayer = 0; int minPathLength = 100000;
		PlaceId shortestPathStep = NOWHERE;
		for (Player hunter = PLAYER_LORD_GODALMING; hunter < PLAYER_DRACULA; hunter++) {
			// Calculate path
			int pathLength = 0;
			PlaceId *path = HvGetShortestPathTo(hv, hunter, CASTLE_DRACULA, &pathLength);
			if (pathLength < minPathLength) {
				closestPlayer = hunter;
				minPathLength = pathLength;
				shortestPathStep = path[0];
			}
			free(path);
		}
		if (player == closestPlayer)
			registerBestPlay((char *)placeIdToAbbrev(shortestPathStep), "JAWA - we don't go by the script");
	}

	// Default movement - surround location
	bool boat = (placeIsSea(HvGetPlayerLocation(hv, PLAYER_DRACULA))) ? true : false;
	if (placeIsSea(move)) boat = true;
	PlaceId *generalReachable = HvWhereCanTheyGoByType(hv, player, true, true, boat, &numReturnedLocs);
	int index = (round * (player + 1)) % numReturnedLocs;
	registerBestPlay((char *)placeIdToAbbrev(generalReachable[index]), "JAWA - we don't go by the script");
	free(generalReachable);
	return;
}


// Returns a starting location for a player
static PlaceId startingLocation(HunterView hv) {
    Player player = HvGetPlayer(hv);
    switch (player) {
        case PLAYER_LORD_GODALMING:
            return KLAUSENBURG;
        case PLAYER_DR_SEWARD:
            return STRASBOURG;
        case PLAYER_VAN_HELSING:
            return SARAGOSSA;
        case PLAYER_MINA_HARKER:
            return ZAGREB;
        default:
			return NOWHERE;
    }
}

// Checks whether a hunter can reach a location
static bool nearby(HunterView hv, Player hunter, PlaceId dMove, bool rail)
{
	int numReturnedLocs = 0;
	PlaceId *hunterConnections = HvWhereCanTheyGoByType(hv, hunter, true, rail, true, &numReturnedLocs);
	for (int i = 0; i < numReturnedLocs; i++) {
		if (hunterConnections[i] == dMove) {
			free(hunterConnections);
			return true;
		}
	}
	free(hunterConnections);
	return false;
}

// Checks how many hunters can reach a location
static int huntersNearby(HunterView hv, PlaceId dMove, bool rail)
{
	int hunters = 0;
	for (Player player = PLAYER_LORD_GODALMING; player < PLAYER_DRACULA; player++)
		if (nearby(hv, player, dMove, rail)) hunters++;
	return hunters;
}

// Checks if a player is able to reach a location for their next turn
static bool reachableNextTurn(HunterView hv, Player player, PlaceId location)
{
	int numReturnedLocs = 0;
	PlaceId *reachable = HvWhereCanTheyGo(hv, player, &numReturnedLocs);
	for (int i = 0; i < numReturnedLocs; i++)
		if (reachable[i] == location) {
			free(reachable);
			return true;
		}
	free(reachable);
	return false;
} 