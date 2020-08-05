////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// dracula.c: your "Fury of Dracula" Dracula AI
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include "dracula.h"
#include "DraculaView.h"
#include "Game.h"
#include <stdio.h>

// Checks whether a hunter can reach a location
static bool nearby(DraculaView dv, Player hunter, PlaceId dMove);
// Checks how many hunters can reach a location
static int huntersNearby(DraculaView dv, PlaceId dMove);
// Checks whether a hunter can reach a location in two turns
static bool reachableInTwoTurns(DraculaView dv, PlaceId location, Player player, Round r);
// Decide starting move
static PlaceId draculaStart(DraculaView dv);


void decideDraculaMove(DraculaView dv)
{
	PlaceId draculaMove = TELEPORT;
	// Extract information
	int draculaHealth = DvGetHealth(dv, PLAYER_DRACULA);
	PlaceId currentLocation = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	int numMoves = 0;
	PlaceId *validMoves = DvGetValidMoves(dv, &numMoves);
	Round round = DvGetRound(dv);
	
	// Hasn't gone yet
	if (numMoves == 0 && DvGetPlayerLocation(dv, PLAYER_DRACULA) == NOWHERE) {
		draculaMove = draculaStart(dv);
		registerBestPlay((char *)placeIdToAbbrev(draculaMove), "JAWA - we don't go by the script");
		free(validMoves);
		return;
	// Teleport as only move
	} else if (numMoves == 0) {
		registerBestPlay((char *)placeIdToAbbrev(draculaMove), "JAWA - we don't go by the script");
		free(validMoves);
		return;
	}

	// Higher moveWeight is preferred
	int moveWeight[NUM_REAL_PLACES] = {0};
	// For each valid move
	for (int i = 0; i < numMoves; i++) {
		PlaceId move = validMoves[i];
		PlaceId location = move;
		// HIDE move
		if (move == HIDE) location = currentLocation;
		// DOUBLE_BACK move
		else if (!placeIsReal(move)) location = resolveDoubleBack(dv, move);

		// Weight 1: Number of hunters that can reach that location on their next turn
		int numHunters = huntersNearby(dv, location);
		moveWeight[i] -= 90 * numHunters;

		// Extra weighting if hunter already at location
		for (Player player = PLAYER_LORD_GODALMING; player < PLAYER_DRACULA; player++) {
			PlaceId hunterLocation = DvGetPlayerLocation(dv, player);
			if (hunterLocation == location && !placeIsSea(hunterLocation)) moveWeight[i] -= 450;
		}

		// Reachable within two turns
		int numHuntersNotReachable = 0;
		for (Player player = PLAYER_LORD_GODALMING; player < PLAYER_DRACULA; player++)
			if (!reachableInTwoTurns(dv, location, player, round))
				numHuntersNotReachable++;
		moveWeight[i] += 40 * numHuntersNotReachable;

		// Check death condition
		if (draculaHealth <= numHunters * LIFE_LOSS_HUNTER_ENCOUNTER && location != CASTLE_DRACULA)
			moveWeight[i] -= 100;

		// Weight 2: Type of move
		if (!placeIsReal(move)) moveWeight[i] -= 2;
		if (placeIsSea(location) && draculaHealth > 6) moveWeight[i] -= 1;
		else if (placeIsSea(location)) moveWeight[i] -= 20;

		// Weight 3: Prefer moves towards CD if low
		// BFS to CASTLE_DRACULA
		int pathLengthCurr = 0; int pathLengthNew = 0;
		PlaceId *currPath = DvShortestPathTo(dv, currentLocation, CASTLE_DRACULA, &pathLengthCurr);
		PlaceId *newPath = DvShortestPathTo(dv, location, CASTLE_DRACULA, &pathLengthNew);
		free(currPath);	free(newPath);
		
		// Low health
		if (draculaHealth <= 25) {
			// If move takes you closer to CASTLE_DRACULA
			if (pathLengthNew < pathLengthCurr && draculaHealth > 20) moveWeight[i] += 15;
			else if (pathLengthNew < pathLengthCurr) moveWeight[i] += 25;
		
		// Gain extra health if safe
		} else if (numHuntersNotReachable > 2) {
			moveWeight[i] += 3;
		}
	}

	// Select max weight
	int maxIndex = 0; int maxWeight = moveWeight[0];
	for (int i = 1; i < numMoves; i++) {
		if (moveWeight[i] > maxWeight) {
			maxIndex = i; maxWeight = moveWeight[i];
		}
	}
	draculaMove = validMoves[maxIndex];
	registerBestPlay((char *)placeIdToAbbrev(draculaMove), "JAWA - we don't go by the script");
	free(validMoves);
}


// Checks whether a hunter can reach a location
static bool nearby(DraculaView dv, Player hunter, PlaceId dMove)
{
	int numReturnedLocs = 0;
	PlaceId *hunterConnections = DvWhereCanTheyGo(dv, hunter, &numReturnedLocs);
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
static int huntersNearby(DraculaView dv, PlaceId dMove)
{
	int hunters = 0;
	for (Player player = PLAYER_LORD_GODALMING; player < PLAYER_DRACULA; player++)
		if (nearby(dv, player, dMove)) hunters++;
	return hunters;
}

// Checks whether a hunter can reach a location in two turns
static bool reachableInTwoTurns(DraculaView dv, PlaceId location, Player player, Round r)
{
	int firstOrderLocs = 0;
	PlaceId *firstOrderReachable = DvGetReachable(dv, player, r+1, DvGetPlayerLocation(dv, player),
												  &firstOrderLocs);
	for (int i = 0; i < firstOrderLocs; i++) {
		int secondOrderLocs = 0;
		PlaceId *secondOrderReachable = DvGetReachable(dv, player, r+2, firstOrderReachable[i],
													   &secondOrderLocs);
		for (int j = 0; j < secondOrderLocs; j++) {
			if (firstOrderReachable[i] == location || secondOrderReachable[j] == location) {
				free(firstOrderReachable);
				free(secondOrderReachable);
				return true;
			}
		}
		free(secondOrderReachable);
	}
	free(firstOrderReachable);
	return false;
}

// Decide starting move
static PlaceId draculaStart(DraculaView dv)
{
	// Process options, weighted by the number of hunters that can reach that location
	PlaceId options[4] = {HAMBURG, ATHENS, BORDEAUX, CADIZ};
	int weight[4] = {0};
	for (int i = 0; i < 4; i++) {
		PlaceId option = options[i];
		for (Player player = PLAYER_LORD_GODALMING; player < PLAYER_DRACULA; player++) {
			bool twiceReachable = reachableInTwoTurns(dv, option, player, 0);
			if (!nearby(dv, player, option) && !twiceReachable) return option;
			weight[i] += huntersNearby(dv, option);
			weight[i] += (twiceReachable) ? 2 : 0;
		}
	}
	// If all starting options are reachable - select min of weights
	int minIndex = 0; int minWeight = weight[0];
	for (int i = 1; i < 4; i++) {
		if (weight[i] < minWeight) {
			minIndex = i; minWeight = weight[i];
		}
	}
	return options[minIndex];
}
