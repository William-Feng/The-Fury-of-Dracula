////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// dracula.c: your "Fury of Dracula" Dracula AI
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
// This was created by JAWA on 06/08/2020
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

		// Weight 1: Number of hunters that can reach that location
		int numHunters = huntersNearby(dv, location);
		moveWeight[i] -= 50 * numHunters;

		// Extra weighting if hunter already at location
		for (Player player = PLAYER_LORD_GODALMING; player < PLAYER_DRACULA; player++)
			if (DvGetPlayerLocation(dv, player) == location) moveWeight[i] -= 100;

		// Check death condition
		if (draculaHealth <= numHunters * LIFE_LOSS_HUNTER_ENCOUNTER && location != CASTLE_DRACULA)
			moveWeight[i] -= 100;

		// Weight 2: Type of move
		if (!placeIsReal(move)) moveWeight[i] -= 2;
		if (placeIsSea(location)) moveWeight[i] -= 1;
		
		// Prefers to go to sea if encountered hunter
		for (Player player = PLAYER_LORD_GODALMING; player < PLAYER_DRACULA; player++)
			if (DvGetPlayerLocation(dv, player) == currentLocation && placeIsSea(location)) moveWeight[i] += 4;

		// Weight 3: Prefer moves to CD if low
		if (draculaHealth <= 20 && location == CASTLE_DRACULA) moveWeight[i] += 10;
		// printf("Move: %d, numHunters: %d, Weight: %d\n", move, numHunters, moveWeight[i]);
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

// Decide starting move
static PlaceId draculaStart(DraculaView dv)
{
	// Process options, weighted by the number of hunters that can reach that location
	PlaceId options[4] = {ATHENS, HAMBURG, BORDEAUX, CADIZ};
	int weight[4] = {0};
	for (int i = 0; i < 4; i++) {
		PlaceId option = options[i];
		for (Player player = PLAYER_LORD_GODALMING; player < PLAYER_DRACULA; player++) {
			if (!nearby(dv, player, option))
				weight[i] += huntersNearby(dv, option);
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