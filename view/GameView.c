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

	// Extract location code //
	char code[3] = {0};
	code[0] = gv->pastPlays[playerIndex + 1];
	code[1] = gv->pastPlays[playerIndex + 2];
	code[2] = '\0';
	PlaceId location = placeAbbrevToId(code);

	if (player != PLAYER_DRACULA) {
		return (GvGetHealth(gv, player) <= 0) ? ST_JOSEPH_AND_ST_MARY : location;
	} 

	bool found = false;
	int newPlayerIndex = playerIndex;
	while (!found && newPlayerIndex >= 0) {
		// Loop until location is not a move
		if (location != HIDE && location != DOUBLE_BACK_1 && location != DOUBLE_BACK_2 &&
			location != DOUBLE_BACK_3 && location != DOUBLE_BACK_4 &&
			location != DOUBLE_BACK_5) {
			found = true;
			break;
		}

		// Determine how far to go back
		if (playerIndex - 40 + 1 >= 0 &&
			(location == HIDE || location == DOUBLE_BACK_1)) {
			newPlayerIndex -= 40;
		} else if (playerIndex - 80 + 1 >= 0 && location == DOUBLE_BACK_2) {
			newPlayerIndex -= 80;
		} else if (playerIndex - 120 + 1 >= 0 && location == DOUBLE_BACK_3) {
			newPlayerIndex -= 120;
		} else if (playerIndex - 160 + 1 >= 0 && location == DOUBLE_BACK_4) {
			newPlayerIndex -= 160;
		} else if (playerIndex - 200 + 1 >= 0 && location == DOUBLE_BACK_5) {
			newPlayerIndex -= 200;
		}
		
		// Update location
		code[0] = gv->pastPlays[newPlayerIndex + 1];
		code[1] = gv->pastPlays[newPlayerIndex + 2];
		code[2] = '\0';
		location = placeAbbrevToId(code);
	}

	if (!found) return UNKNOWN_PLACE;
	else if (location == TELEPORT) return CASTLE_DRACULA;
	else return location;
}

PlaceId GvGetVampireLocation(GameView gv)
{
    // Vampire not spawned if the 5th player (Dracula) hasn't played
    if (strlen(gv->pastPlays) <= 4*8) return NOWHERE;
    
    // Divide message length by 40 (rounded down) to get the round
    // Subtract 1 to go to the previous row
    // Multiply by 40 to get the index of the row immediately above
    // Add 36 to get the position of 'V' in that line (arrays zero indexed)
    int vampireIndex = (strlen(gv->pastPlays) / 40 - 1) * 40 + 36;
    // If finding the vampire index in the first turn
    if (vampireIndex < 0) vampireIndex = 36;

    char code[3] = {0};
    code[2] = '\0';
    // Vampire does not exist
    if (gv->pastPlays[vampireIndex] != 'V') {
        return NOWHERE;
    } else { // Vampire does exist
        code[0] = gv->pastPlays[vampireIndex - 3];
        code[1] = gv->pastPlays[vampireIndex - 2];
        return placeAbbrevToId(code);
    }
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
	*canFree = true;
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
