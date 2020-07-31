////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// GameView.c: GameView ADT implementation
//
// 2014-07-01   v1.0    Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01   v1.1    Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31   v2.0    Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10   v3.0    Team Dracula <cs2521@cse.unsw.edu.au>
//
// This was created by JAWA on 31/07/2020.
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

struct gameView {
    char *pastPlays;
    Map map;
};

///////////////////////////////////////////////////////////////////////////////
// Function prototypes

// Returns the higher of two integers
static int max(int a, int b);
// Returns the lower of two integers
static int min(int a, int b);
// Calculates how many rounds a player has played
static int roundsPlayed(GameView gv, Player player);
// Extracts player move in a specific round
static PlaceId getPlayerMove(GameView gv, Player player, Round round);
// Extracts location for a specified move
static PlaceId extractLocation(GameView gv, Player player, PlaceId move, Round round);
// Removes a specified location from an array
static void removeLocation(PlaceId *array, int *arrSize, PlaceId location);
// Returns health of Dracula
static int healthDracula(GameView gv);
// Returns health of a hunter
static int healthHunter(GameView gv, Player player, int numTurns);
// Helper function for healthHunter
static bool hunterRest(GameView gv, int location);
// Appends a city to a PlaceId array if it is unique
static void arrayUniqueAppend(PlaceId *reachable, int *numReturnedLocs, PlaceId city);
// Adds connections to the reachable array which satisfy transport type
static void addReachable(GameView gv, Player player, PlaceId from, int numRailMoves,
                         bool road, bool rail, bool boat, int *numReturnedLocs, PlaceId *reachable);


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
    MapFree(gv->map);
    free(gv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round GvGetRound(GameView gv)
{
	return ((strlen(gv->pastPlays) + 1) / 40);
}

Player GvGetPlayer(GameView gv)
{
    // strlen() % 40 indicates how far across each round
    // Add one before dividing by eight to signify the player for next turn
    // % 5 to make player 5 loop back to player 0
    return (((strlen(gv->pastPlays) % 40) + 1) / 8) % 5;
}

int GvGetScore(GameView gv)
{
	int score = GAME_START_SCORE;
    for (Round round = 0; round <= GvGetRound(gv); round++) {
        // Hunter deaths
        for (Player player = PLAYER_LORD_GODALMING; player < PLAYER_DRACULA; player++)
            if (roundsPlayed(gv, player) > round && healthHunter(gv, player, round + 1) <= 0)
                score -= SCORE_LOSS_HUNTER_HOSPITAL;
        // Dracula turn
        if (roundsPlayed(gv, PLAYER_DRACULA) - 1 < round) continue;
        score -= SCORE_LOSS_DRACULA_TURN;
        // Vampire mature
        if (gv->pastPlays[round * 40 + 32 + 5] == 'V') score -= SCORE_LOSS_VAMPIRE_MATURES;
    }
	return max(score, 0);
}

int GvGetHealth(GameView gv, Player player)
{
    if (player == PLAYER_DRACULA) {
        // Have to check encounters for Dracula
        return healthDracula(gv);
    } else {
        // Check the number of rounds played by a hunter
        int numTurns = roundsPlayed(gv, player);
        return healthHunter(gv, player, numTurns);
    }
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
        // Check for Dracula-spawned vampire
        if (roundsPlayed(gv, PLAYER_DRACULA) > round &&
            gv->pastPlays[round * 40 + 32 + 4] == 'V') {
            PlaceId move = getPlayerMove(gv, PLAYER_DRACULA, round);
            location = extractLocation(gv, PLAYER_DRACULA, move, round);
        }
        // Check if hunters have vanquished the vampire
        for (Player player = PLAYER_LORD_GODALMING; player < PLAYER_DRACULA; player++) {
            if (roundsPlayed(gv, player) - 1 < round) continue;
            for (int encounter = 3; encounter < 7; encounter++)
                if (gv->pastPlays[round * 40 + player * 8 + encounter] == 'V')
                    vanquished = true;
        }
        // Check for matured vampires
        if (roundsPlayed(gv, PLAYER_DRACULA) > round &&
            gv->pastPlays[round * 40 + 32 + 5] == 'V')
            vanquished = true;
    }
    return (vanquished) ? NOWHERE : location;
}

PlaceId *GvGetTrapLocations(GameView gv, int *numTraps)
{ 
    PlaceId *trapLocations = malloc(GvGetRound(gv) * sizeof(PlaceId));
    if (trapLocations == NULL) {
        fprintf(stderr, "Failed to allocate memory!\n");
        exit(EXIT_FAILURE);
	}
    *numTraps = 0;

    for (Round round = 0; round <= GvGetRound(gv); round++) {
        // Add Dracula-placed traps
        if (roundsPlayed(gv, PLAYER_DRACULA) - 1 >= round &&
            gv->pastPlays[round * 40 + 35] == 'T') {
            PlaceId move = getPlayerMove(gv, PLAYER_DRACULA, round);
            trapLocations[*numTraps] = extractLocation(gv, PLAYER_DRACULA, move, round);
            (*numTraps)++;
        }
        // Remove encountered traps
        for (Player player = PLAYER_LORD_GODALMING; player < PLAYER_DRACULA; player++) {
            if (roundsPlayed(gv, player) - 1 < round) continue;
            // Retrieve health at start of turn
            int playerHealth = healthHunter(gv, player, round);
            // Check encounters
            for (int encounter = 3; encounter < 7; encounter++) {
                // Ignore future encounters if player died first
                if (playerHealth <= 0) break;
                if (gv->pastPlays[round * 40 + player * 8 + encounter] == 'T') {
                    PlaceId move = getPlayerMove(gv, player, round);
                    removeLocation(trapLocations, numTraps, move);
                    playerHealth -= LIFE_LOSS_TRAP_ENCOUNTER;
                }
            }  
        }
        // Remove traps that have left the trail
        if (roundsPlayed(gv, PLAYER_DRACULA) - 1 >= round &&
            gv->pastPlays[round * 40 + 37] == 'M') {
            PlaceId move = getPlayerMove(gv, PLAYER_DRACULA, round - TRAIL_SIZE);
            PlaceId location = extractLocation(gv, PLAYER_DRACULA, move, round - TRAIL_SIZE);
            removeLocation(trapLocations, numTraps, location);
        }
    }
	return trapLocations; 	
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

	// Determine the bounds for the loop
    int finalMove = roundsPlayed(gv, player);
    int firstMove = max(finalMove - numMoves, 0);
    int moveCounter = 0;
    for (Round round = firstMove; round < finalMove; round++) {
        // Append move to moves array
        moves[moveCounter] = getPlayerMove(gv, player, round);
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
    
    // Find location for special moves
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
	bool rail = (player == PLAYER_DRACULA) ? false : true;
    return GvGetReachableByType(gv, player, round, from, true, rail, true, numReturnedLocs);
}

PlaceId *GvGetReachableByType(GameView gv, Player player, Round round,
                              PlaceId from, bool road, bool rail,
                              bool boat, int *numReturnedLocs)
{
    PlaceId *reachable = malloc(NUM_REAL_PLACES * sizeof(PlaceId));
    if (reachable == NULL) {
        fprintf(stderr, "Failed to allocate memory!\n");
        exit(EXIT_FAILURE);
	}
    *numReturnedLocs = 0;
    if (player == PLAYER_DRACULA) rail = false;
    int numRailMoves = (rail) ? (round + player) % 4 : 0;
    addReachable(gv, player, from, numRailMoves, road, rail, boat,
                 numReturnedLocs, reachable);
    return reachable;
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// Returns the higher of two integers
static int max(int a, int b)
{
    return (a > b) ? a : b;
}

// Returns the lower of two integers
static int min(int a, int b)
{
    return (a > b) ? b : a;
}

// Calculates how many rounds a player has played
static int roundsPlayed(GameView gv, Player player)
{
    // Add one to round if player has already gone in current turn
    return (player < GvGetPlayer(gv)) ? GvGetRound(gv) + 1 : GvGetRound(gv);
}

// Extracts move of a player in a specific round
static PlaceId getPlayerMove(GameView gv, Player player, Round round)
{
    char move[3];
    move[0] = gv->pastPlays[round * 40 + player * 8 + 1];
    move[1] = gv->pastPlays[round * 40 + player * 8 + 2];
    move[2] = '\0';
    return placeAbbrevToId(move);
}

// Extracts location for a specified move
static PlaceId extractLocation(GameView gv, Player player, PlaceId move, Round round)
{
    // Hunters
    if (player != PLAYER_DRACULA) {
		// Health up to turn
        return (GvGetHealth(gv, player) <= 0) ? ST_JOSEPH_AND_ST_MARY : move;
	} 

	bool found = false;
	int playerIndex = round * 40 + player * 8;
	while (!found && playerIndex >= 0) {
		// Loop until move is a location (placeIsReal ignores unknown)
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

// Removes a specified location from an array
static void removeLocation(PlaceId *array, int *arrSize, PlaceId location)
{
    // Find index of element to be removed
    int i;
    for (i = 0; i < *arrSize; i++)
        if (array[i] == location) break;
    (*arrSize)--;
    // Swap all elements from this index onwards with its successor
    for (int j = i; j < *arrSize; j++)
        array[j] = array[j + 1];
}

// Finding the health of a Dracula
static int healthDracula(GameView gv) 
{
    int health = GAME_START_BLOOD_POINTS;
    for (Round round = 0; round <= GvGetRound(gv); round++) {
        // Check Dracula location
        if (roundsPlayed(gv, PLAYER_DRACULA) > round) {
            PlaceId move = getPlayerMove(gv, PLAYER_DRACULA, round);
            PlaceId loc = extractLocation(gv, PLAYER_DRACULA, move, round);
            if (loc == CASTLE_DRACULA) health += LIFE_GAIN_CASTLE_DRACULA;
            if (placeIsSea(loc)) health -= LIFE_LOSS_SEA;
        }    
        // Check hunter encounters
        for (Player player = PLAYER_LORD_GODALMING; player < PLAYER_DRACULA; player++) {
            if (roundsPlayed(gv, player) - 1 < round) continue;
            // Check encounters
            for (int encounter = 3; encounter < 7; encounter++) {
                if (gv->pastPlays[round * 40 + player * 8 + encounter] == 'D')
                    health -= LIFE_LOSS_HUNTER_ENCOUNTER;
            }
        }
    }
    return max(health, 0);
}

// Finds the Health of a Hunter given the pastPlays string
static int healthHunter(GameView gv, Player player, int numTurns) 
{
    int strtElmt = player;
    int incre = 0;
    int health = GAME_START_HUNTER_LIFE_POINTS;

    for (int j = 0; j < numTurns; j++) {
        // Revive upon death
        if (health <= 0) health = GAME_START_HUNTER_LIFE_POINTS;
        // Rest
        if (hunterRest(gv, (strtElmt * 8) + 1 + incre)) { 
            // Hunters have a maximum health
            health = min(health + LIFE_GAIN_REST, GAME_START_HUNTER_LIFE_POINTS);
        }
        // Encounters
        for (int i = 0; i < 4; i++) {
            if (gv->pastPlays[(strtElmt * 8) + 3 + incre + i] == 'T') {
                // Trap
                health -= LIFE_LOSS_TRAP_ENCOUNTER;
            } else if (gv->pastPlays[(strtElmt * 8) + 3 + incre + i] == 'D') { 
                // Dracula
                health -= LIFE_LOSS_DRACULA_ENCOUNTER;
            } 
        }
        incre += 40;
    }
    return max(health, 0);
}

// Determines if hunter stays in the same location between sucessive turns
static bool hunterRest(GameView gv, int location)
{   
    if (location - 40 < 0) return false;
    if (gv->pastPlays[location] != gv->pastPlays[location - 40]) return false; 
    if (gv->pastPlays[location + 1] != gv->pastPlays[location + 1 - 40]) return false;
    return true; 
}

// Appends a city to a PlaceId array if it is unique
static void arrayUniqueAppend(PlaceId *reachable, int *numReturnedLocs, PlaceId city)
{
    // Check Unique
    for (int i = 0; i < *numReturnedLocs; i++)
        if (reachable[i] == city) return;
    // Append if unique
    reachable[*numReturnedLocs] = city;
    (*numReturnedLocs)++;
}

// Adds connections to the reachable array which satisfy transport type
static void addReachable(GameView gv, Player player, PlaceId from, int numRailMoves,
                bool road, bool rail, bool boat, int *numReturnedLocs, PlaceId *reachable)
{
    // Add current location
    if (player == PLAYER_DRACULA && from == ST_JOSEPH_AND_ST_MARY) return;
    arrayUniqueAppend(reachable, numReturnedLocs, from);
    // Extract connections
    ConnList connections = MapGetConnections(gv->map, from);
    for (ConnList c = connections; c != NULL; c = c->next) {
        if (player == PLAYER_DRACULA && c->p == ST_JOSEPH_AND_ST_MARY) continue;
        // Add unique wanted road/boat connections
        if ((road == true && c->type == ROAD) || (boat == true && c->type == BOAT))
            arrayUniqueAppend(reachable, numReturnedLocs, c->p);
        // Add rail connections
        if (rail == true && numRailMoves > 0 && c->type == RAIL) {
            arrayUniqueAppend(reachable, numReturnedLocs, c->p);
            // Further recursive extraction for rail
            if (numRailMoves > 1)
                addReachable(gv, player, c->p, numRailMoves - 1, false, true, false,
                             numReturnedLocs, reachable);
        }
    }
}
