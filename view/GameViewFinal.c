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

// TODO: ADD YOUR OWN STRUCTS HERE

struct gameView {
    char *pastPlays;
    Map map;
};

///////////////////////////////////////////////////////////////////////////////
// Function prototypes

// Returns the higher of two integers
int max(int a, int b);
// Returns the lower of two integers
int min(int a, int b);
// Calculates how many rounds a player has played
int roundsPlayed(GameView gv, Player player);
// Extract location for a specified move
PlaceId extractLocation(GameView gv, Player player, PlaceId move, Round round);
// Appends a city to a PlaceId array if it is unique
void arrayUniqueAppend(PlaceId *reachable, int *numReturnedLocs, PlaceId city);
// Adds connections to the reachable array which satisfy transport type
void addReachable(GameView gv, Player player, PlaceId from, int numRailMoves,
                  bool road, bool rail, bool boat,int *numReturnedLocs, PlaceId *reachable);

////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

GameView GvNew(char *pastPlays, Message messages[])
{
	GameView new = malloc(sizeof(*new));
	if (new == NULL) {
		fprintf(stderr, "Couldn't allocate GameView!\n");
		exit(EXIT_FAILURE);
	}
    new->pastPlays = strdup(pastPlays);
    new->map = MapNew();
	return new;
}

void GvFree(GameView gv)
{
    free(gv->pastPlays);
	free(gv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round GvGetRound(GameView gv)
{
	return (strlen(gv->pastPlays) + 1) / 40;
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
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	int score = GAME_START_SCORE;
	// For each round
	for (Round round = 0; round < GvGetRound(gv); round++) {
		/* NOTE EDGE CASES NOT IMPLEMENTED YET */
		// Check player death
		for (Player player = PLAYER_LORD_GODALMING; player <= PLAYER_MINA_HARKER; player++) {
			// Player has gone in current round
			if ((round * 40 + player * 8 + 7) <= strlen(gv->pastPlays) &&
				healthHunter(gv, player, round) <= 0)
				score -= SCORE_LOSS_HUNTER_HOSPITAL;
		}
		// Check dracula turn
		if ((round * 40 + 4 * 8) < strlen(gv->pastPlays)) score -= SCORE_LOSS_DRACULA_TURN;
		// Check vampire mature
		
	}

	return score;
}

int GvGetHealth(GameView gv, Player player)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return 0;
}

PlaceId GvGetPlayerLocation(GameView gv, Player player)
{
    // Retrieve the last location using GvGetLastLocations
    int numLocs = 0; bool canFree = true;
    PlaceId *locations = GvGetLastLocations(gv, player, 1, &numLocs, &canFree);
    PlaceId location = locations[0];
    free(locations);

    // No locations returned
    if (numLocs == 0) {
		return NOWHERE;
	// Player is a hunter
	} else if (player != PLAYER_DRACULA) {
		return (GvGetHealth(gv, player) <= 0) ? ST_JOSEPH_AND_ST_MARY : location;
	// Player is Dracula
	} else {
        return location;
    }
}

PlaceId GvGetVampireLocation(GameView gv)
{
    PlaceId location = NOWHERE;
    bool vanquished = false;
    int firstRound = max(GvGetRound(gv) - 13, 0);
    for (Round round = firstRound; round <= GvGetRound(gv); round++) {
        // Dracula Spawned Vampire
        if (roundsPlayed(gv, PLAYER_DRACULA) > round &&
            gv->pastPlays[round * 40 + 32 + 4] == 'V') {
            char move[3];
            move[0] = gv->pastPlays[round * 40 + 32 + 1];
            move[1] = gv->pastPlays[round * 40 + 32 + 2];
            move[2] = '\0';
            location = extractLocation(gv, PLAYER_DRACULA, placeAbbrevToId(move), round);
        }
        // Vanquished
        for (Player player = PLAYER_LORD_GODALMING; player < PLAYER_DRACULA; player++) {
            if (roundsPlayed(gv, player) - 1 < round) continue;
            for (int encounter = 3; encounter < 7; encounter++)
                if (gv->pastPlays[round * 40 + player * 8 + encounter] == 'V')
                    vanquished = true;
        }
        // Matured
        if (roundsPlayed(gv, PLAYER_DRACULA) > round &&
            gv->pastPlays[round * 40 + 32 + 5] == 'V')
            vanquished = true;
    }
    return (vanquished) ? NOWHERE : location;
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
    // Run GvGetLastMoves for all the rounds the player has played in
    int numMoves = roundsPlayed(gv, player);
    return GvGetLastMoves(gv, player, numMoves, numReturnedMoves, canFree);
}

PlaceId *GvGetLastMoves(GameView gv, Player player, int numMoves,
                        int *numReturnedMoves, bool *canFree)
{
	// Allocate memory for the array
    PlaceId *moves = malloc(numMoves * sizeof(PlaceId));
    if (moves == NULL) {
        fprintf(stderr, "Failed to allocate memory!\n");
        exit(EXIT_FAILURE);
    }

    // Determine the bounds for the for loop
    int finalMove = roundsPlayed(gv, player);
    int firstMove = max(finalMove - numMoves, 0);
    int moveCounter = 0;
    for (int round = firstMove; round < finalMove; round++) {
        // Extract location for player within round
        char abbrev[3] = {0};
        abbrev[0] = gv->pastPlays[round * 40 + player * 8 + 1];
        abbrev[1] = gv->pastPlays[round * 40 + player * 8 + 2];
        abbrev[2] = '\0';
        // Append to moves array
        moves[moveCounter] = placeAbbrevToId(abbrev);
		moveCounter++;
    }
    
	*numReturnedMoves = moveCounter;
	*canFree = true;
	return moves;
}

PlaceId *GvGetLocationHistory(GameView gv, Player player,
                              int *numReturnedLocs, bool *canFree)
{
	// Run GvGetLastLocations for all the rounds the player has played in
    int numLocs = roundsPlayed(gv, player);
    return GvGetLastLocations(gv, player, numLocs, numReturnedLocs, canFree);
}

PlaceId *GvGetLastLocations(GameView gv, Player player, int numLocs,
                            int *numReturnedLocs, bool *canFree)
{
	// Retrieve the last moves using GvGetLastMoves
    PlaceId *moves = GvGetLastMoves(gv, player, numLocs, numReturnedLocs, canFree);
    
    // Find location of special moves
    int roundOffset = roundsPlayed(gv, player) - *numReturnedLocs;
    for (int index = 0; index < *numReturnedLocs; index++) {
        moves[index] = extractLocation(gv, player, moves[index], index + roundOffset);
    }
    return moves;
}

////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId *GvGetReachable(GameView gv, Player player, Round round,
                        PlaceId from, int *numReturnedLocs)
{
	return (player == PLAYER_DRACULA) ?
	GvGetReachableByType(gv, player, round, from, true, false, true, numReturnedLocs):
	GvGetReachableByType(gv, player, round, from, true, true, true, numReturnedLocs);
}

PlaceId *GvGetReachableByType(GameView gv, Player player, Round round,
                              PlaceId from, bool road, bool rail,
                              bool boat, int *numReturnedLocs)
{
    PlaceId *reachable = malloc(NUM_REAL_PLACES * sizeof(PlaceId));
    assert(reachable != NULL);
    *numReturnedLocs = 0;
    int numRailMoves = (rail) ? (round + player) % 4 : 0;
    addReachable(gv, player, from, numRailMoves, road, rail, boat,
                 numReturnedLocs, reachable);
    return reachable;
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// TODO

// Helper Functions

// Returns the higher of two integers
int max(int a, int b) {
    return (a > b) ? a : b;
}

// Returns the lower of two integers
int min(int a, int b) {
    return (a > b) ? b : a;
}

// Calculates how many rounds a player has played
int roundsPlayed(GameView gv, Player player) {
    // Add one if player has already gone in current turn
    return (player < GvGetPlayer(gv)) ? GvGetRound(gv) + 1 : GvGetRound(gv);
}

// Extract location for a specified move
PlaceId extractLocation(GameView gv, Player player, PlaceId move, Round round) {
    // Hunters
    if (player != PLAYER_DRACULA) {
		// Health up to turn
        return (GvGetHealth(gv, player) <= 0) ? ST_JOSEPH_AND_ST_MARY : move;
	} 

	bool found = false;
	int playerIndex = round * 40 + player * 8;
	while (!found && playerIndex >= 0) {
		// Loop until location is not a move
		if (move != HIDE && move != DOUBLE_BACK_1 && move != DOUBLE_BACK_2 &&
			move != DOUBLE_BACK_3 && move != DOUBLE_BACK_4 &&
			move != DOUBLE_BACK_5) {
			found = true;
			break;
		}

		// Determine how far to go back
		if (playerIndex - 40 + 1 >= 0 &&
			(move == HIDE || move == DOUBLE_BACK_1)) {
			playerIndex -= 40;
		} else if (playerIndex - 80 + 1 >= 0 && move == DOUBLE_BACK_2) {
			playerIndex -= 80;
		} else if (playerIndex - 120 + 1 >= 0 && move == DOUBLE_BACK_3) {
			playerIndex -= 120;
		} else if (playerIndex - 160 + 1 >= 0 && move == DOUBLE_BACK_4) {
			playerIndex -= 160;
		} else if (playerIndex - 200 + 1 >= 0 && move == DOUBLE_BACK_5) {
			playerIndex -= 200;
		}
		
		// Update location
		char code[3];
        code[0] = gv->pastPlays[playerIndex + 1];
		code[1] = gv->pastPlays[playerIndex + 2];
		code[2] = '\0';
		move = placeAbbrevToId(code);
	}

	if (!found) return UNKNOWN_PLACE;
	else if (move == TELEPORT) return CASTLE_DRACULA;
	else return move;
}

// Appends a city to a PlaceId array if it is unique
void arrayUniqueAppend(PlaceId *reachable, int *numReturnedLocs, PlaceId city) {
    // Check Unique
    for (int i = 0; i < *numReturnedLocs; i++)
        if (reachable[i] == city) return;
    // Append if unique
    reachable[*numReturnedLocs] = city;
    (*numReturnedLocs)++;
}


// Adds connections to the reachable array which satisfy transport type
void addReachable(GameView gv, Player player, PlaceId from, int numRailMoves,
                   bool road, bool rail, bool boat, int *numReturnedLocs, PlaceId *reachable)
{
    // Add current location
    arrayUniqueAppend(reachable, numReturnedLocs, from);
    // Extract Connections
    ConnList connections = MapGetConnections(gv->map, from);
    for (ConnList c = connections; c != NULL; c = c->next) {
        if (player == PLAYER_DRACULA && c->p == ST_JOSEPH_AND_ST_MARY) continue;
        // Add unique wanted roads/boats
        if ((road == true && c->type == ROAD) || (boat == true && c->type == BOAT))
            arrayUniqueAppend(reachable, numReturnedLocs, c->p);
        // Unique Rail
        if (rail == true && numRailMoves > 0 && c->type == RAIL) {
            arrayUniqueAppend(reachable, numReturnedLocs, c->p);
            // Further extraction for rail (recursion)
            if (numRailMoves > 1)
                addReachable(gv, player, c->p, numRailMoves - 1, false, true, false,
                                   numReturnedLocs, reachable);
        }
    }
}