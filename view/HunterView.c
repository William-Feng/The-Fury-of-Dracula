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
// add your own #includes here

// TODO: ADD YOUR OWN STRUCTS HERE

struct hunterView {
	// TODO: ADD FIELDS HERE
	GameView gv;
	char *pastPlays;

};

// Queue
typedef struct queueNode {
	PlaceId city;
	struct queueNode *next;
} QueueNode;

typedef struct queueRep {
	QueueNode *head;
	QueueNode *tail;
} QueueRep;

typedef QueueRep *Queue;

Queue newQueue() {
	QueueRep *q = malloc(sizeof(*q));
	assert(q != NULL);
	q->head = q->tail = NULL;
	return q;
}

void enQueue(Queue q, PlaceId city) {
	assert(q != NULL);

	QueueNode *newNode = malloc(sizeof(*newNode));
	assert(newNode != NULL);
	newNode->city = city;
	newNode->next = NULL;
	// *newNode = (QueueNode){ .city = city, .next = NULL };

	if (q->head == NULL) q->head = newNode;
	if (q->tail != NULL) q->tail->next = newNode;
	q->tail = newNode;
}

PlaceId deQueue(Queue q) {
	assert(q != NULL);
	assert(q->head != NULL);
	PlaceId city = q->head->city;
	QueueNode *remove = q->head;
	q->head = remove->next;
	if (q->head == NULL) q->tail = NULL;
	// free(remove); // Seg Fault :(
	return city;
}

void showQueue(Queue q) {
	printf("[ ");
	for (QueueNode *curr = q->head; curr; curr=curr->next)
		printf("%d ", curr->city);
	printf("]\n");
}

bool queueIsEmpty(Queue q) {
	return q->head == NULL;
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
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	// BFS queue
	PlaceId src = HvGetPlayerLocation(hv, hunter);
	
	PlaceId *visited = malloc(MAX_REAL_PLACE * sizeof(PlaceId));
	assert(visited != NULL);
	for (int i = 0; i < MAX_REAL_PLACE; i++) {
		// Mark unvisited
		visited[i] = -1;
	}
	bool found = false;
	visited[src] = src;

	// Queue implementation
	Queue q = newQueue();
	enQueue(q, src);
	Round round = HvGetRound(hv);
	while (!found && !queueIsEmpty(q)) {
		// showQueue(q);
		PlaceId city = deQueue(q);
		if (city == dest) {
			found = true;
		} else {
			int numReachable = 0;
			PlaceId *reachableFromCity = GvGetReachable(hv->gv, hunter, round,
														city, &numReachable);
			// For each of the reachable cities
			for (int j = 0; j < numReachable; j++) {
				PlaceId reachableCity = reachableFromCity[j];
				// If reachable city is unvisited
				if (placeIsReal(reachableCity) && visited[reachableCity] == -1) {
					visited[reachableCity] = city;
					enQueue(q, reachableCity);
				}
			}
		}
		round++;
	}
	free(q);

	// No path found
	if (found == false) {
		*pathLength = 0;
		return NULL;
	}

	// Add to path, traversing through the visited array
	PlaceId *path = malloc(MAX_REAL_PLACE * sizeof(PlaceId));
	assert(path != NULL);
	PlaceId city = dest;
	*pathLength = 0;
	path[0] = dest;
	while (city < MAX_REAL_PLACE && city != src) {
		path[*pathLength] = visited[city];
		city = visited[city];
		*pathLength = *pathLength + 1;
	}
	// Flip array
	// for (int i = 0; i < *pathLength/2; i++) {
	// 	PlaceId temp = 
	// }


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
	return GvGetReachable(hv->gv, player, HvGetRound(hv),
						  HvGetPlayerLocation(hv, player), numReturnedLocs);
}

PlaceId *HvWhereCanTheyGoByType(HunterView hv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs)
{
	return GvGetReachableByType(hv->gv, player, HvGetRound(hv),
						  		HvGetPlayerLocation(hv, player), road, rail, boat,
								numReturnedLocs);
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// TODO
