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

/////////////////////
// HunterView ADT //
///////////////////

struct hunterView {
	GameView gv;
	char *pastPlays;
	Message *messages;
};

// Calculates how many rounds a player has played
static int roundsPlayed(HunterView gv, Player player);


////////////////
// Queue ADT //
//////////////

typedef struct queueNode {
	PlaceId city;
	struct queueNode *next;
} QueueNode;

typedef struct queueRep {
	QueueNode *head;
	QueueNode *tail;
} QueueRep;

typedef QueueRep *Queue;

/** Creates a new queue. */
Queue newQueue(void);

/** Adds a PlaceId to the queue. */
void enQueue(Queue q, PlaceId city);

/** Removes and returns the first PlaceId from the queue. */
PlaceId deQueue(Queue q);

/** Frees memory used by the queue. */
void dropQueue(Queue q);

/** Displays the queue. */
void showQueue(Queue q);

/** Checks whether the queue is empty or not. */
bool queueIsEmpty(Queue q);


////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

HunterView HvNew(char *pastPlays, Message messages[])
{
	HunterView new = malloc(sizeof(*new));
	if (new == NULL) {
		fprintf(stderr, "Couldn't allocate HunterView!\n");
		exit(EXIT_FAILURE);
	}
	// Create GameView
	new->gv = GvNew(pastPlays, messages);
	// Store pastPlays
	new->pastPlays = strdup(pastPlays);
	// Store messages
    int numPastPlays = (strlen(pastPlays) + 1) / 8;
    new->messages = malloc(numPastPlays * sizeof(Message));
	if (new->messages == NULL) {
        fprintf(stderr, "Failed to allocate memory!\n");
        exit(EXIT_FAILURE);
	}
    for (int i = 0; i < numPastPlays; i++) strcpy(new->messages[i], messages[i]);
	return new;
}

void HvFree(HunterView hv)
{
	GvFree(hv->gv);
	free(hv->pastPlays);
	free(hv->messages);
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
	// Check rounds
	*round = 0;
	Round r = HvGetRound(hv) - 1;
	while (r >= 0) {
		// Extract move
		char move[3] = {0};
		move[0] = hv->pastPlays[r * 40 + 33];
		move[1] = hv->pastPlays[r * 40 + 34];
		move[2] = '\0';
		PlaceId location = placeAbbrevToId(move);
		
		if (placeIsReal(location)) {
			// Location found
			*round = r;
			return location;
		} else if (location == DOUBLE_BACK_2) {
			r -= 2;
		} else if (location == DOUBLE_BACK_3) {
			r -= 3;
		} else if (location == DOUBLE_BACK_4) {
			r -= 4;
		} else if (location == DOUBLE_BACK_5) {
			r -= 5;
		} else {
			// HIDE, DOUBLE_BACK_1, unknown place
			r -= 1;
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
	PlaceId visited[NUM_REAL_PLACES] = {0};
	// Array for storing which round a city would be visited
	PlaceId visitedRound[NUM_REAL_PLACES] = {0};

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
			free(reachableFromCity);
		}
	}
	dropQueue(q);

	// No path found
	if (found == false) {
		*pathLength = 0;
		return NULL;
	}

	// Add to path, traversing through the visited array
	PlaceId *path = malloc(MAX_REAL_PLACE * sizeof(PlaceId));
	if (path == NULL) {
        fprintf(stderr, "Failed to allocate memory!\n");
        exit(EXIT_FAILURE);
	}
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

/////////////////////
// HunterView ADT //
///////////////////

// Calculates how many rounds a player has played
static int roundsPlayed(HunterView gv, Player player)
{
    // Add one to round if player has already gone in current turn
    return (player < HvGetPlayer(gv)) ? HvGetRound(gv) + 1 : HvGetRound(gv);
}


////////////////
// Queue ADT //
//////////////

/** Creates a new queue. */
Queue newQueue(void)
{
	QueueRep *q = malloc(sizeof(*q));
	if (q == NULL) {
        fprintf(stderr, "Failed to create Queue!\n");
        exit(EXIT_FAILURE);
	}
	q->head = q->tail = NULL;
	return q;
}

/** Adds a PlaceId to the queue. */
void enQueue(Queue q, PlaceId city)
{
	assert(q != NULL);

	QueueNode *newNode = malloc(sizeof(*newNode));
	if (newNode == NULL) {
        fprintf(stderr, "Failed to allocate memory!\n");
        exit(EXIT_FAILURE);
	}
	newNode->city = city;
	newNode->next = NULL;

	if (q->head == NULL) q->head = newNode;
	if (q->tail != NULL) q->tail->next = newNode;
	q->tail = newNode;
}

/** Removes and returns the first PlaceId from the queue. */
PlaceId deQueue(Queue q)
{
	assert(q != NULL);
	assert(q->head != NULL);
	PlaceId city = q->head->city;
	QueueNode *remove = q->head;
	q->head = remove->next;
	if (q->head == NULL) q->tail = NULL;
	free(remove);
	return city;
}

/** Frees memory used by the queue. */
void dropQueue(Queue q)
{
    assert(q != NULL);
    for (QueueNode *curr = q->head, *next; curr != NULL; curr = next) {
        next = curr->next;
        free(curr);
    }
    free(q);
}

/** Displays the queue. */
void showQueue(Queue q)
{
	printf("[ ");
	for (QueueNode *curr = q->head; curr; curr=curr->next)
		printf("%d ", curr->city);
	printf("]\n");
}

/** Checks whether the queue is empty or not. */
bool queueIsEmpty(Queue q)
{
	return q->head == NULL;
}
