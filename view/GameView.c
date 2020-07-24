////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// GameView.c: GameView ADT implementation
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
#include "Map.h"
#include "Places.h"
// add your own #includes here

int draculaHealth(GameView gv);
int hunterHealth(GameView gv, Player player);

// TODO: ADD YOUR OWN STRUCTS HERE

struct gameView {
	// TODO: ADD FIELDS HERE
	Round round;
	// Message messages[];
	char *pastPlays;

	Map map;
};

////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

GameView GvNew(char *pastPlays, Message messages[])
{
	GameView new = malloc(sizeof(*new));
	if (new == NULL) {
		fprintf(stderr, "Couldn't allocate GameView!\n");
		exit(EXIT_FAILURE);
	}
	
	// Current round
	new->round = (strlen(pastPlays) + 1) / 40;
	new->pastPlays = strdup(pastPlays);

	return new;
}

void GvFree(GameView gv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	free(gv->pastPlays);
	free(gv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round GvGetRound(GameView gv)
{
	return gv->round;
}

Player GvGetPlayer(GameView gv)
{
	// strlen() % 40 indicates how far across each round.
	// adding one before dividing by eight signifies
	// the player for the next turn.
	// this % 5 makes player 5 loop back to player 0.
	return (((strlen(gv->pastPlays) % 40) + 1) / 8) % 5;
}

int GvGetScore(GameView gv)
{
	int score = GAME_START_SCORE;
	return score;
}

int GvGetHealth(GameView gv, Player player)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	// return 1;
	// Vampire
	if (player == PLAYER_DRACULA) {
		return draculaHealth(gv);
	// Hunter
	} else {
		return hunterHealth(gv, player);
	}

}

int hunterHealth(GameView gv, Player player) {
	int health = GAME_START_HUNTER_LIFE_POINTS;
	// For each player round
	for (int round = 0; (player < GvGetPlayer(gv)) ? round < gv->round + 1: round < gv->round; round++) {
		// Check encounters
		int roundIndex = 40 * round + 3 + player * 8;
		for (int encounter = roundIndex; encounter < roundIndex + 4; encounter++) {
			if (gv->pastPlays[encounter] == 'T') {
				health -= LIFE_LOSS_TRAP_ENCOUNTER;
			} else if (gv->pastPlays[encounter] == 'D') {
				health -= LIFE_LOSS_DRACULA_ENCOUNTER;
			}
		}
		// Rest (check before?)
		int playerIndex = round * 40 + player * 8;
		if (round > 0 &&
			gv->pastPlays[playerIndex + 1] == gv->pastPlays[playerIndex - 40 + 1] &&
			gv->pastPlays[playerIndex + 2] == gv->pastPlays[playerIndex - 40 + 2]) {
			health = (health + 3 > 9) ? 9 : health + 3;
		}
		// printf("%d %d\n", round, health);
	}
	// Need to add health restore mechanic - dead hunter, skip next round
	return health;
}

int draculaHealth(GameView gv) {
	int health = GAME_START_BLOOD_POINTS;
	return health;
}

PlaceId GvGetPlayerLocation(GameView gv, Player player)
{
	// Calculates index of the last entry for specified player //
	// Player already moved in current round
	int playerTurn = GvGetPlayer(gv);
	int playerIndex;
	// Check current round
	if (player < playerTurn) {
		playerIndex = gv->round * 40 + player * 8;
	// Check round before if it's not the first round
	} else if (gv->round != 0) {
		playerIndex = (gv->round - 1) * 40 + player * 8;
	// Not in first round
	} else {
		return NOWHERE;
	}

	// Dracula teleport

	// Extract location code
	char code[3] = {0};
	code[0] = gv->pastPlays[playerIndex + 1];
	code[1] = gv->pastPlays[playerIndex + 2];
	code[2] = '\0';
	PlaceId location = placeAbbrevToId(code);
	switch (location) {
		case (HIDE):
		case (DOUBLE_BACK_1):
			assert(playerIndex - 40 + 1 >= 0);
			code[0] = gv->pastPlays[playerIndex - 40 + 1];
			code[1] = gv->pastPlays[playerIndex - 40 + 2];
			code[2] = '\0';
			return placeAbbrevToId(code);

		case (DOUBLE_BACK_2):
			assert(playerIndex - 80 + 1 >= 0);
			code[0] = gv->pastPlays[playerIndex - 80 + 1];
			code[1] = gv->pastPlays[playerIndex - 80 + 2];
			code[2] = '\0';
			return placeAbbrevToId(code);

		case (DOUBLE_BACK_3):
			assert(playerIndex - 120 + 1 >= 0);
			code[0] = gv->pastPlays[playerIndex - 120 + 1];
			code[1] = gv->pastPlays[playerIndex - 120 + 2];
			code[2] = '\0';
			return placeAbbrevToId(code);

		case (DOUBLE_BACK_4):
			assert(playerIndex - 160 + 1 >= 0);
			code[0] = gv->pastPlays[playerIndex - 160 + 1];
			code[1] = gv->pastPlays[playerIndex - 160 + 2];
			code[2] = '\0';
			return placeAbbrevToId(code);

		case (DOUBLE_BACK_5):
			assert(playerIndex - 200 + 1 >= 0);
			code[0] = gv->pastPlays[playerIndex - 200 + 1];
			code[1] = gv->pastPlays[playerIndex - 200 + 2];
			code[2] = '\0';
			return placeAbbrevToId(code);

		default:
			if (player != PLAYER_DRACULA && GvGetHealth(gv, player) <= 0)
				return ST_JOSEPH_AND_ST_MARY;
			else return location;
	}
}

PlaceId GvGetVampireLocation(GameView gv)
{
    // Vampire not spawned if the 5th player (Dracula) hasn't played
    if (strlen(gv->pastPlays) <= 4*8) return NOWHERE;
    
    // Divide message length by 40 (rounded down) to get the round
    // Subtract 1 to go to the previous row
    // Multiply by 40 to get the index of the row immediately above
    // Add 36 to get the position of 'V' in that line (arrays zero indexed)
    int index = ((strlen(gv->pastPlays) + 1) / 40 - 1) * 40 + 37 - 1;
    // If finding the vampire index in the first turn
    if (index < 0) index = 36;

	// Vampire has been vanquished by one of the hunters
	int vanquished = ((strlen(gv->pastPlays) + 1) / 40) * 40 + 4 - 1;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (gv->pastPlays[vanquished] == 'V') return NOWHERE;
			vanquished++;
		}
		vanquished += 5;
	}

	for (int i = 0; i < strlen(gv->pastPlays) % 40; i++) {
		// Vampire exists
		if (gv->pastPlays[index] == 'V') {
			char code[3];
			code[0] = gv->pastPlays[index - 3];
			code[1] = gv->pastPlays[index - 2];
			code[2] = '\0';
			return placeAbbrevToId(code);
		}
		// Vampire has matured
		if (gv->pastPlays[index + 1] == 'V') return NOWHERE;
		index -= 40;
	}
	// Otherwise vampire hasn't been spawned
	return NOWHERE;
}

PlaceId *GvGetTrapLocations(GameView gv, int *numTraps)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numTraps = 0;
	return NULL;
}

////////////////////////////////////////////////////////////////////////
// Game History

PlaceId *GvGetMoveHistory(GameView gv, Player player,
                          int *numReturnedMoves, bool *canFree)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedMoves = 0;
	*canFree = false;
	return NULL;
}

PlaceId *GvGetLastMoves(GameView gv, Player player, int numMoves,
                        int *numReturnedMoves, bool *canFree)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedMoves = 0;
	*canFree = false;
	return NULL;
}

PlaceId *GvGetLocationHistory(GameView gv, Player player,
                              int *numReturnedLocs, bool *canFree)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	*canFree = false;
	return NULL;
}

PlaceId *GvGetLastLocations(GameView gv, Player player, int numLocs,
                            int *numReturnedLocs, bool *canFree)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	*canFree = false;
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId *GvGetReachable(GameView gv, Player player, Round round,
                        PlaceId from, int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	return NULL;
}

PlaceId *GvGetReachableByType(GameView gv, Player player, Round round,
                              PlaceId from, bool road, bool rail,
                              bool boat, int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	return NULL;
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// TODO
