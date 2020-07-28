////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// HunterView.c: the HunterView ADT implementation
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10   v3.0    Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Game.h"
#include "GameView.h"
#include "HunterView.h"
#include "Map.h"
#include "Places.h"
#include "Queue.h"

// add your own #includes here

// TODO: ADD YOUR OWN STRUCTS HERE

struct hunterView {
	GameView gv;
	char *pastPlays;
};

// // Queue
// typedef struct queueNode {
// 	PlaceId city;
// 	struct queueNode *next;
// } QueueNode;

// typedef struct queueRep {
// 	QueueNode *head;
// 	QueueNode *tail;
// } QueueRep;

// typedef QueueRep *Queue;

// Queue newQueue();

// void enQueue(Queue q, PlaceId city);

// PlaceId deQueue(Queue q);

// void dropQueue(Queue q);

// void showQueue(Queue q);

// bool queueIsEmpty(Queue q);


static int roundsPlayed(HunterView gv, Player player) { //same as FindNumMoves
    // Add one to round if player has already gone in current turn
    return (player < HvGetPlayer(gv)) ? HvGetRound(gv) + 1 : HvGetRound(gv);
}


////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

HunterView HvNew(char *pastPlays, Message messages[])
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	HunterView new = malloc(sizeof(*new));
	if (new == NULL) {
		fprintf(stderr, "Couldn't allocate HunterView!\n");
		exit(EXIT_FAILURE);
	}
	new->gv = GvNew(pastPlays, messages);
	new->pastPlays = strdup(pastPlays);
	
	return new;
}

void HvFree(HunterView hv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	GvFree(hv->gv);
	free(hv->pastPlays);
	free(hv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round HvGetRound(HunterView hv)
{
	return GvGetRound(hv->gv);
}

Player HvGetPlayer(HunterView hv)
{
	return GvGetPlayer(hv->gv);
}

int HvGetScore(HunterView hv)
{
	return GvGetScore(hv->gv);
}

int HvGetHealth(HunterView hv, Player player)
{
	return GvGetHealth(hv->gv, player);
}

PlaceId HvGetPlayerLocation(HunterView hv, Player player)
{
	return GvGetPlayerLocation(hv->gv, player);
}

PlaceId HvGetVampireLocation(HunterView hv)
{
	return GvGetVampireLocation(hv->gv);
}

////////////////////////////////////////////////////////////////////////
// Utility Functions

PlaceId HvGetLastKnownDraculaLocation(HunterView hv, Round *round)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*round = 0;
	// return GvGetPlayerLocation(hv->gv, PLAYER_DRACULA);

	// Rounds
	Round i = HvGetRound(hv) - 1;
	while (i >= 0) {
		// Extract Move
		char abbrev[3] = {0};
		abbrev[0] = hv->pastPlays[i * 40 + 32 + 1];
		abbrev[1] = hv->pastPlays[i * 40 + 32 + 2];
		abbrev[2] = '\0';
		PlaceId move = placeAbbrevToId(abbrev);

		if (placeIsReal(move)) {
			*round = i;
			return move;
		} else if (move == DOUBLE_BACK_2) {
			i -= 2;
		} else if (move == DOUBLE_BACK_3) {
			i -= 3;
		} else if (move == DOUBLE_BACK_4) {
			i -= 4;
		} else if (move == DOUBLE_BACK_5) {
			i -= 5;
		// HIDE, DOUBLE_BACK_1, unknown place
		} else {
			i -= 1;
		}
	}

	// Not found yet
	return NOWHERE;
}

PlaceId *HvGetShortestPathTo(HunterView hv, Player hunter, PlaceId dest,
                             int *pathLength)
{
	// Breadth First Search Implementation
	PlaceId src = HvGetPlayerLocation(hv, hunter);
	
	// Visited array for storing predecessor city
	PlaceId *visited = malloc(NUM_REAL_PLACES * sizeof(PlaceId));
	assert(visited != NULL);
	// Array for storing which round a city would be visited
	PlaceId *visitedRound = malloc(NUM_REAL_PLACES * sizeof(PlaceId));
	assert(visitedRound != NULL);

	// Initialisation
	for (int i = 0; i < MAX_REAL_PLACE; i++) {
		// Mark unvisited
		visited[i] = -1;
		visitedRound[i] = -1;
	}
	bool found = false;
	visited[src] = src;
	visitedRound[src] = roundsPlayed(hv, hunter);

	// Queue Implementation
	Queue q = newQueue();
	enQueue(q, src);
	while (!found && !queueIsEmpty(q)) {
		PlaceId city = deQueue(q);
		if (city == dest) {
			found = true;
		} else {
			int numReachable = 0;
			PlaceId *reachableFromCity = GvGetReachable(hv->gv, hunter, visitedRound[city],
														city, &numReachable);
			// For each of the reachable cities
			for (int j = 0; j < numReachable; j++) {
				PlaceId reachableCity = reachableFromCity[j];
				// If reachable city is unvisited
				if (placeIsReal(reachableCity) && visited[reachableCity] == -1) {
					// Update predecessor city
					visited[reachableCity] = city;
					// Update which round the city would be visited
					visitedRound[reachableCity] = visitedRound[city] + 1;
					enQueue(q, reachableCity);
				}
			}
		}
	}
	dropQueue(q);
	free(visitedRound);

	// No path found
	if (found == false) {
		*pathLength = 0;
		free(visited);
		return NULL;
	}

	// Add to path, traversing through the visited array
	PlaceId *path = malloc(MAX_REAL_PLACE * sizeof(PlaceId));
	assert(path != NULL);
	PlaceId city = dest;
	path[0] = dest;
	*pathLength = 1;
	while (city < MAX_REAL_PLACE && city != src) {
		path[*pathLength] = visited[city];
		city = visited[city];
		(*pathLength)++;
	}
	(*pathLength)--;
	
	// Flip array to be from src to dest
	for (int i = 0; i < *pathLength/2; i++) {
		PlaceId temp = path[i];
		path[i] = path[*pathLength - 1 - i];
		path[*pathLength - 1 - i] = temp;
	}

	// Add to ADT?
	free(visited);
	return path;
}

////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId *HvWhereCanIGo(HunterView hv, int *numReturnedLocs)
{
	Player player = HvGetPlayer(hv);
	return GvGetReachable(hv->gv, player, HvGetRound(hv),
						  HvGetPlayerLocation(hv, player), numReturnedLocs);
}

PlaceId *HvWhereCanIGoByType(HunterView hv, bool road, bool rail,
                             bool boat, int *numReturnedLocs)
{
	Player player = HvGetPlayer(hv);
	return GvGetReachableByType(hv->gv, player, HvGetRound(hv),
						  		HvGetPlayerLocation(hv, player), road, rail, boat,
								numReturnedLocs);
}

PlaceId *HvWhereCanTheyGo(HunterView hv, Player player,
                          int *numReturnedLocs)
{
	// player has already played in current round
	return GvGetReachable(hv->gv, player, roundsPlayed(hv, player),
						  HvGetPlayerLocation(hv, player), numReturnedLocs);
}

PlaceId *HvWhereCanTheyGoByType(HunterView hv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs)
{
	return GvGetReachableByType(hv->gv, player, roundsPlayed(hv, player),
						  		HvGetPlayerLocation(hv, player), road, rail, boat,
								numReturnedLocs);
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// TODO
