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
#include <math.h>

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

// Unchecked
bool vampireHunterEncounter (GameView gv, int location);
bool hunterRest (GameView gv, int location) ;
int healthDracula (GameView gv, Player player, int numTurns);
int healthHunter (GameView gv, Player player, int numTurns);


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
	int score = GAME_START_SCORE;

	// decreases by 1 each time Dracula finishes his turn
	score -= GvGetRound(gv) * SCORE_LOSS_DRACULA_TURN;

	// decreases by 6 each time a hunter loses all life points and teleported
	// to St Joseph and St Mary
	// PlayerLocation might be wrong???
	if (GvGetPlayer(gv) >= PLAYER_LORD_GODALMING && GvGetPlayer(gv) <= PLAYER_MINA_HARKER) {
		if (GvGetPlayerLocation(gv, GvGetPlayer(gv)) == HOSPITAL_PLACE) {
			score -= SCORE_LOSS_HUNTER_HOSPITAL;
		}
	}

	// decreases by 13 each time a vampire matures (falls off the trail)
	if (GvGetPlayer(gv) == PLAYER_LORD_GODALMING) {
		int lastLocation = 0;
		PlaceId *trail = GvGetLastLocations(gv, PLAYER_DRACULA, TRAIL_SIZE, &lastLocation, false);
		PlaceId vampireLocation = GvGetVampireLocation(gv);

		for (int i = 0; i < lastLocation; i++) {
			if (vampireLocation == trail[i]) {
				score -= SCORE_LOSS_VAMPIRE_MATURES;
			}
		}
	}
	
	// If score reaches zero, Dracula has won
	if (score == 0) {
		// game lost
	}

	// return score 
	assert (score >= 0); 
	return score; 
}

int GvGetHealth(GameView gv, Player player)
{
	// Retrieve number of turns
	Round numTurns = GvGetRound(gv);
	if (GvGetPlayer(gv) > player) numTurns++;

	// Player is a hunter
	if (player != PLAYER_DRACULA) {
		return healthHunter(gv, player, numTurns);
	// Player is Dracula
	} else {
		return healthDracula(gv, player, numTurns);
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
	// Dynamically allocated array of trap locations
	PlaceId *trapLocations = malloc(sizeof(PlaceId *));

	char *token;

	// Extract plays from pastPlays string
	token = strtok(gv->pastPlays, " ");

	// Keep count of traps counted
	int traps = 0;

	while (token != NULL) {
		// store each move made
		char move[7];
		char location[3];
		location[2] = '\0';
		strcpy(move, token);
		if (move[0] == 'D' && move[3] == 'T') {
			// trap found
			location[0] = move[1];
			location[1] = move[2];
			PlaceId loc = placeAbbrevToId(location);
			
			trapLocations[traps] = loc;
			traps++;

			strncpy(move, "", strlen(move));
		} else if (move[0] != 'D' && move[3] == 'T') {
			traps--;
			location[0] = move[1];
			location[1] = move[2];
			PlaceId loc = placeAbbrevToId(location);
			for (int k = 0; k < traps; k++) {
				if (trapLocations[k] == loc) {
					trapLocations[k] = trapLocations[traps];
					break;
				}
			}
			strncpy(move, "", strlen(move));
		}
		token = strtok(NULL, " ");
	}

	*numTraps = traps;
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

// Helper Functions


int max(int a, int b) {
    return (a > b) ? a : b;
}

int min(int a, int b) {
    return (a > b) ? b : a;
}

int roundsPlayed(GameView gv, Player player) {
    // Add one to round if player has already gone in current turn
    return (player < GvGetPlayer(gv)) ? GvGetRound(gv) + 1 : GvGetRound(gv);
}

PlaceId extractLocation(GameView gv, Player player, PlaceId move, Round round) {
    // Player is a Hunter
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


// Unchecked yet

bool vampireHunterEncounter (GameView gv, int location) {
	Player currPlayer = GvGetPlayer(gv);
	for(int i = 0; i < currPlayer; i++) {	
		if (gv->pastPlays[location + (8 * i) + 4] == 'D') { // vampire encountered
			return true;
		}
	}
	return false;
}







bool hunterRest (GameView gv, int location) {
	
	if (gv->pastPlays[location] != gv->pastPlays[location - 40]) {
		return false; 
	}
	if (gv->pastPlays[location + 1] != gv->pastPlays[location + 1 - 40]) {
		return false; 
	}
	return true; 
}








int healthDracula (GameView gv, Player player, int numTurns) {
	
	int strtElmt = player;
	int incre = 0;

	int health = GAME_START_BLOOD_POINTS;

	
	for (int i = 0; i < numTurns; i++) {
		char abbrev[3];
		abbrev[0] = gv->pastPlays[(strtElmt * 8) + incre + 1];
		abbrev[1] = gv->pastPlays[(strtElmt * 8) + incre + 2];
		abbrev[2] = '\0';
		PlaceId pid = placeAbbrevToId (abbrev);
		PlaceType pT = placeIdToType (pid);
		if (pT == SEA) { // is the place a sea?
			health = health - LIFE_LOSS_SEA;	
		} else if (gv->pastPlays[(strtElmt * 8) + incre + 1] == 'S' && gv->pastPlays[(strtElmt * 8) + incre + 2] == '?') { // in sea at end of turn
			health = health - LIFE_LOSS_SEA;
		} else if (pid == CASTLE_DRACULA) { // in castle dracula
			health = health + LIFE_GAIN_CASTLE_DRACULA;
		} else if (gv->pastPlays[(strtElmt * 8) + incre + 1] == 'D') { // test for doubel back
			int p = gv->pastPlays[(strtElmt * 8) + incre + 2] - 48;
			if (gv->pastPlays[((strtElmt * 8) + incre + 1) - (p * 40)] == 'S' && gv->pastPlays[(((strtElmt * 8) + incre + 1) - (p * 40)) + 1] == '?') { // is the place a sea?
				health = health - LIFE_LOSS_SEA;
			} else { /// is the place a sea? 
				char abbreviation[3];
				abbreviation[0] = gv->pastPlays[((strtElmt * 8) + incre + 1) - (p * 40)];
				abbreviation[1] = gv->pastPlays[(((strtElmt * 8) + incre + 1) - (p * 40)) + 1];
				abbreviation[2] = '\0';
				PlaceId plcid = placeAbbrevToId (abbreviation);
				PlaceType plcT = placeIdToType (plcid);
				if (plcT == SEA) {
					health = health - LIFE_LOSS_SEA;	
				}
				if (plcid == CASTLE_DRACULA) {
					health = health + LIFE_GAIN_CASTLE_DRACULA;
				}
			}
		} else if (gv->pastPlays[(strtElmt * 8) + incre + 1] == 'H') {
			if (gv->pastPlays[(strtElmt * 8) + incre + 2] == 'I') {
				if (gv->pastPlays[((strtElmt * 8) + incre + 1) - 40] == 'S') {
					health = health - LIFE_LOSS_SEA;
				}
			}
		}

		if (vampireHunterEncounter(gv, (((strtElmt + 1 ) * 8) + incre))) { // encounters a hunter
			health = health - LIFE_LOSS_HUNTER_ENCOUNTER;
		} 
		incre = incre + 40;
	}
	if (health < 0) return 0;
	return health;
}





int healthHunter (GameView gv, Player player, int numTurns) {
	int strtElmt = player;
	int incre = 0;

	int health = GAME_START_HUNTER_LIFE_POINTS;

	for (int j = 0; j < numTurns; j++) {
		for (int i = 0; i < 4; i++) {
			if (gv->pastPlays[(strtElmt * 8) + 3 + incre + i] == 'T') { // trap
				health = health - LIFE_LOSS_TRAP_ENCOUNTER;
			}
			if (gv->pastPlays[(strtElmt * 8) + 3 + incre + i] == 'D') { // encounter dracula
				health = health - LIFE_LOSS_DRACULA_ENCOUNTER;
			} 
			if (hunterRest(gv, (strtElmt * 8) + 1 + incre)) { // gains 3 life points each time they rest // need to code for other bit
				health = min ( (health + LIFE_GAIN_REST), GAME_START_HUNTER_LIFE_POINTS);
			}
		}
		incre = incre + 40;
	}
	if (health < 0) return 0;
	return health;
}





