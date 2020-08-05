////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// DraculaView.c: the DraculaView ADT implementation
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "DraculaView.h"
#include "Game.h"
#include "GameView.h"
#include "Map.h"
// add your own #includes here
#include "Queue.h"
#include "utils.h"

struct draculaView {
	GameView gv;
	
	// for convenience
	PlaceId trailMoves[TRAIL_SIZE - 1];     // Dracula's last 5 moves in
	                                        // reverse order
	PlaceId trailLocations[TRAIL_SIZE - 1]; // Dracula's last 5 locations
	                                        // in reverse order
	int trailLength;
};

PlaceId DvWhereAmI(DraculaView dv);

static void fillTrail(DraculaView dv);
static bool canMoveTo(DraculaView dv, PlaceId location);

////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

DraculaView DvNew(char *pastPlays, Message messages[])
{
	DraculaView dv = malloc(sizeof(*dv));
	if (dv == NULL) {
		fprintf(stderr, "Couldn't allocate DraculaView\n");
		exit(EXIT_FAILURE);
	}

	dv->gv = GvNew(pastPlays, messages);
	fillTrail(dv);
	return dv;
}

/**
 * For convenience, fills the trailMoves and trailLocations arrays in
 * the DraculaView struct with Dracula's last 5 moves/locations, and
 * sets trailLength to the number of moves stored (in case Dracula
 * hasn't made 5 moves yet)
 */
static void fillTrail(DraculaView dv) {
	int numMoves = TRAIL_SIZE - 1;
	int numLocations = TRAIL_SIZE - 1;
	
	bool canFreeMoves = false;
	bool canFreeLocations = false;
	
	PlaceId *moves = GvGetLastMoves(dv->gv, PLAYER_DRACULA, numMoves,
	                                &numMoves, &canFreeMoves);
	
	PlaceId *locations = GvGetLastLocations(dv->gv, PLAYER_DRACULA, numLocations,
	                                        &numLocations, &canFreeLocations);
	
	placesCopy(dv->trailMoves, moves, numMoves);
	placesCopy(dv->trailLocations, locations, numLocations);
	
	placesReverse(dv->trailMoves, numMoves);
	placesReverse(dv->trailLocations, numLocations);
	
	dv->trailLength = numMoves;
	if (canFreeMoves) free(moves);
	if (canFreeLocations) free(locations);
}

void DvFree(DraculaView dv)
{
	GvFree(dv->gv);
	free(dv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round DvGetRound(DraculaView dv)
{
	return GvGetRound(dv->gv);
}

int DvGetScore(DraculaView dv)
{
	return GvGetScore(dv->gv);
}

int DvGetHealth(DraculaView dv, Player player)
{
	return GvGetHealth(dv->gv, player);
}

PlaceId DvGetPlayerLocation(DraculaView dv, Player player)
{
	return GvGetPlayerLocation(dv->gv, player);
}

PlaceId DvGetVampireLocation(DraculaView dv)
{
	return GvGetVampireLocation(dv->gv);
}

PlaceId *DvGetTrapLocations(DraculaView dv, int *numTraps)
{
	return GvGetTrapLocations(dv->gv, numTraps);
}

////////////////////////////////////////////////////////////////////////
// Making a Move

static void    addLocationMoves(DraculaView dv, PlaceId *moves,
                                int *numReturnedMoves);
static void    addDoubleBackMoves(DraculaView dv, PlaceId *moves,
                                  int *numReturnedMoves);
static void    addHideMoves(DraculaView dv, PlaceId *moves,
                            int *numReturnedMoves);
static bool    moveIsLegal(DraculaView dv, PlaceId move);
static bool    trailContains(DraculaView dv, PlaceId move);
PlaceId 	   resolveDoubleBack(DraculaView dv, PlaceId db);
static bool    canReach(DraculaView dv, PlaceId location);
static bool    trailContainsDoubleBack(DraculaView dv);
static bool    isDoubleBack(PlaceId move);

PlaceId *DvGetValidMoves(DraculaView dv, int *numReturnedMoves)
{
	if (DvWhereAmI(dv) == NOWHERE) {
		*numReturnedMoves = 0;
		return NULL;
	}

	// There can't be more than NUM_REAL_PLACES
	// valid moves
	PlaceId *moves = malloc(NUM_REAL_PLACES * sizeof(PlaceId));
	assert(moves != NULL);
	
	*numReturnedMoves = 0;
	addLocationMoves(dv, moves, numReturnedMoves);
	addDoubleBackMoves(dv, moves, numReturnedMoves);
	addHideMoves(dv, moves, numReturnedMoves);
	return moves;
}

static void addLocationMoves(DraculaView dv, PlaceId *moves,
                             int *numReturnedMoves) {
	// Get the locations that Dracula can reach (we pass 1 as the round
	// number, since Dracula's movement doesn't depend on round number)
	int numLocs = 0;
	PlaceId *locs = GvGetReachable(dv->gv, PLAYER_DRACULA, 1,
	                               DvWhereAmI(dv), &numLocs);
	
	// For each location, check if it's a legal move, and add it to the
	// moves array if so
	for (int i = 0; i < numLocs; i++) {
		if (moveIsLegal(dv, locs[i])) {
			moves[(*numReturnedMoves)++] = locs[i];
		}
	}
	
	free(locs);
}

static void addDoubleBackMoves(DraculaView dv, PlaceId *moves,
                               int *numReturnedMoves) {
    
    // For each DOUBLE_BACK move, check it it's a legal move and add it
    // to the moves array if so
	for (PlaceId move = DOUBLE_BACK_1; move <= DOUBLE_BACK_5; move++) {
		if (moveIsLegal(dv, move)) {
			moves[(*numReturnedMoves)++] = move;
		}
	}
}

static void addHideMoves(DraculaView dv, PlaceId *moves,
                         int *numReturnedMoves) {
	if (moveIsLegal(dv, HIDE)) {
		moves[(*numReturnedMoves)++] = HIDE;
	}
}

static bool moveIsLegal(DraculaView dv, PlaceId move) {
	
	if (placeIsReal(move)) {
		return !trailContains(dv, move) && canReach(dv, move);
	
	} else if (isDoubleBack(move)) {
		PlaceId location = resolveDoubleBack(dv, move);
		return !trailContainsDoubleBack(dv) && canReach(dv, location);
		
	} else if (move == HIDE) {
		return !trailContains(dv, move) && !placeIsSea(DvWhereAmI(dv));
	
	} else {
		return false;
	}
}

static bool trailContains(DraculaView dv, PlaceId move) {
	return placesContains(dv->trailMoves, dv->trailLength, move);
}

static bool canReach(DraculaView dv, PlaceId location) {
	int numLocs = 0;
	PlaceId *places = GvGetReachable(dv->gv, PLAYER_DRACULA, 1,
	                                 DvWhereAmI(dv), &numLocs);
	bool result = placesContains(places, numLocs, location);
	free(places);
	return result;
}

/**
 * Resolves a DOUBLE_BACK move to a place
 */
PlaceId resolveDoubleBack(DraculaView dv, PlaceId db) {
	// Get the position in the trail that the DOUBLE_BACK move refers to
	// DOUBLE_BACK_1 => 0, DOUBLE_BACK_2 => 1, etc.
	int pos = db - DOUBLE_BACK_1;
	
	// If the position in the trail is out of range, then return NOWHERE
	// otherwise return the location
	return (pos >= dv->trailLength ? NOWHERE : dv->trailLocations[pos]);
}

static bool trailContainsDoubleBack(DraculaView dv) {
	for (int i = 0; i < dv->trailLength; i++) {
		if (isDoubleBack(dv->trailMoves[i])) {
			return true;
		}
	}
	return false;
}

static bool isDoubleBack(PlaceId move) {
	return move >= DOUBLE_BACK_1 && move <= DOUBLE_BACK_5;
}

////////////////////////////////////////////////////////////////////////

PlaceId *DvWhereCanIGo(DraculaView dv, int *numReturnedLocs)
{
	return DvWhereCanIGoByType(dv, true, true, numReturnedLocs);
}

PlaceId *DvWhereCanIGoByType(DraculaView dv, bool road, bool boat,
                             int *numReturnedLocs)
{
	if (DvWhereAmI(dv) == NOWHERE) {
		*numReturnedLocs = 0;
		return NULL;
	}

	// Get an array of locations connected to Dracula's location
	// by the given transport methods
	int numReachable = 0;
	PlaceId *reachable = GvGetReachableByType(dv->gv, PLAYER_DRACULA, 1,
	                                          DvWhereAmI(dv), road, false,
	                                          boat, &numReachable);
	
	// Remove all of those locations that Dracula can't move to
	int i = 0;
	while (i < numReachable) {
		if (!canMoveTo(dv, reachable[i])) {
			// 'Remove' by replacing with the last location
			reachable[i] = reachable[numReachable - 1];
			numReachable--;
		} else {
			i++;
		}
	}
	
	*numReturnedLocs = numReachable;
	return reachable;
}

/**
 * Check if Dracula can move to the given location
 * Assumes the location is adjacent to Dracula's current location
 * and it's not a forbidden location
 */
static bool canMoveTo(DraculaView dv, PlaceId location) {
	// If the location is not in the trail, Dracula can move there
	if (!trailContains(dv, location)) {
		return true;
	
	// If the location is in the trail, but there is no DOUBLE_BACK
	// in Dracula's trail, he can move there
	} else if (!trailContainsDoubleBack(dv)) {
		return true;
	
	// Otherwise, if the location is not Dracula's current location,
	// he can't move there
	} else if (location != DvWhereAmI(dv)) {
		return false;
	
	// Otherwise, if Dracula is able to HIDE, he can move there
	} else {
		return !trailContains(dv, HIDE) && !placeIsSea(location);
	}
}

////////////////////////////////////////////////////////////////////////

PlaceId *DvWhereCanTheyGo(DraculaView dv, Player player,
                          int *numReturnedLocs)
{
	return DvWhereCanTheyGoByType(dv, player, true, true, true,
	                              numReturnedLocs);
}

PlaceId *DvWhereCanTheyGoByType(DraculaView dv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs)
{
	if (DvGetPlayerLocation(dv, player) == NOWHERE) {
		*numReturnedLocs = 0;
		return NULL;
	}
	
	if (player == PLAYER_DRACULA) {
		return DvWhereCanIGoByType(dv, road, boat, numReturnedLocs);
	} else {
		// The next move for all hunters is next round
		Round round = GvGetRound(dv->gv) + 1;
		return GvGetReachableByType(dv->gv, player, round,
		                            GvGetPlayerLocation(dv->gv, player),
		                            road, rail, boat, numReturnedLocs);
	}
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// TODO

PlaceId DvWhereAmI(DraculaView dv)
{
	return DvGetPlayerLocation(dv, PLAYER_DRACULA);
}

PlaceId *DvShortestPathTo(DraculaView dv, PlaceId src, PlaceId dest, int *pathLength)
{
	// Predecessor array
	PlaceId pred[NUM_REAL_PLACES] = {0};
	for (int i = 0; i < NUM_REAL_PLACES; i++)
		pred[i] = -1;
	pred[src] = src;
	
	Queue q1 = QueueNew(); // current round locations
	Queue q2 = QueueNew(); // next round locations

	QueueEnqueue(q1, src);
	while (!(QueueIsEmpty(q1) && QueueIsEmpty(q2))) {
		PlaceId curr = QueueDequeue(q1);
		int numReachable = 0;
		PlaceId *reachable = GvGetReachable(dv->gv, PLAYER_DRACULA, -1, curr,
		                                    &numReachable);
		
		for (int i = 0; i < numReachable; i++) {
			if (pred[reachable[i]] == -1) {
				pred[reachable[i]] = curr;
				QueueEnqueue(q2, reachable[i]);
			}
		}
		free(reachable);
		
		// When we've exhausted the current round's locations, advance
		// to the next round and swap the queues (so the next round's
		// locations becomes the current round's locations)
		if (QueueIsEmpty(q1)) {
			Queue tmp = q1; q1 = q2; q2 = tmp; // swap queues
		}
	}

	QueueDrop(q1);
	QueueDrop(q2);

	// Process shortest path
	// One pass to get the path length
	int dist = 0;
	PlaceId curr = dest;
	while (curr != src) {
		dist++;
		curr = pred[curr];
	}
	
	PlaceId *path = malloc(dist * sizeof(PlaceId));
	// Another pass to copy the path in
	int i = dist - 1;
	curr = dest;
	while (curr != src) {
		path[i] = curr;
		curr = pred[curr];
		i--;
	}
	*pathLength = dist;
	return path;
}

PlaceId *DvGetReachable(DraculaView dv, Player player, Round round, PlaceId from, int *numReturnedLocs)
{
	return GvGetReachable(dv->gv, player, round, from, numReturnedLocs);
}
