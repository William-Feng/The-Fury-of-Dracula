////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// GameView.c: GameView ADT implementation
//
// 2014-07-01   v1.0    Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01   v1.1    Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31   v2.0    Team Dracula <cs2521@cse.unsw.edu.au>
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
static int roundsPlayed(GameView gv, Player player);
// Extract location for a specified move
PlaceId extractLocation(GameView gv, Player player, PlaceId move, Round round);
// Appends a city to a PlaceId array if it is unique
void arrayUniqueAppend(PlaceId *reachable, int *numReturnedLocs, PlaceId city);
// Adds connections to the reachable array which satisfy transport type
void addReachable(GameView gv, Player player, PlaceId from, int numRailMoves,
                  bool road, bool rail, bool boat,int *numReturnedLocs, PlaceId *reachable);

// Unchecked
bool vampireHunterEncounter (GameView gv, int location);
bool hunterRest (GameView gv, int location) ;
int healthDracula (GameView gv, Player player, int numTurns);
char *playToPlcAbbrev (char *play, int location);
int isPlaceSeaOrCastle (char *abbrev, int health);
int doIsPlaceSeaOrCastle(PlaceId pid, PlaceType pType, int health);
bool isDoubleBackMove (char *pastPlays, int index);
bool isHideMove(char *pastPlays, int location);
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
    // TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    free (gv->pastPlays);
    MapFree (gv->map);
    free (gv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round GvGetRound(GameView gv)
{
	return ((strlen(gv->pastPlays) + 1) / 40);
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
	if (GvGetRound(gv) == 0) {
		return GAME_START_SCORE;
	}

	int score = GAME_START_SCORE;

	// decreases by 1 each time Dracula finishes his turn
	score = score - GvGetRound(gv) * SCORE_LOSS_DRACULA_TURN;

        // decreases by 6 each time a hunter loses all life points and teleported
	// to St Joseph and St Mary
	for (int i = 0; i < NUM_PLAYERS - 1; i++) {
		if (GvGetHealth(gv, i) <= 0) {
			score -= SCORE_LOSS_HUNTER_HOSPITAL;
		}
	}

	// decreases by 13 each time a vampire matures (falls off the trail)
	int index = ((strlen(gv->pastPlays) + 1) / 40 - 1) * 40 
    + 38 - 1;
        // If finding the vampire index in the first turn
	if (index < 0) index = 37;
	
	for (int i = 0; i < strlen(gv->pastPlays) % 40; i++) {
		if (gv->pastPlays[index] == 'V') {
			score -= SCORE_LOSS_VAMPIRE_MATURES;
		}
		index -= 40;
	}

	// If score reaches zero, Dracula has won
	if (score == 0) {
		// game lost
	}

	return score; 
}

int GvGetHealth(GameView gv, Player player)
{
    // get number of turns
    Round numTurns = roundsPlayed(gv, player);

    if (player == PLAYER_DRACULA) { //player is Dracula
        return healthDracula (gv, player, numTurns);
    } else { // If player is Hunter
        return healthHunter (gv, player, numTurns);
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
	int traps = 0;
	int index = 3;
	PlaceId *trapLocations = malloc(sizeof(PlaceId*));
	for (int i = 0; i < strlen(gv->pastPlays) / 8; i++) {
		if (gv->pastPlays[index] == 'T') {
			if (gv->pastPlays[index - 3] == 'D') {
				char location[3];
				location[2] = '\0';
				location[0] = gv->pastPlays[index - 2];
				location[1] = gv->pastPlays[index - 1];
				PlaceId loc = placeAbbrevToId(location);
				trapLocations[traps] = loc;
				traps++;
				strncpy(location, "", strlen(location));
			} else if (gv->pastPlays[index - 3] != 'D') {
				traps--;
				char location[3];
				location[2] = '\0';
				location[0] = gv->pastPlays[index - 2];
				location[1] = gv->pastPlays[index - 1];
				PlaceId loc = placeAbbrevToId(location);
				for (int j = 0; j < traps; j++) {
					if (trapLocations[j] == loc && j != traps - 1) {
						trapLocations[j] = trapLocations[traps];
						break;
					}
				}
			} 

		}
		index += 8;
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

// Returns the higher of two integers
int max(int a, int b) {
    return (a > b) ? a : b;
}

// Returns the lower of two integers
int min(int a, int b) {
    return (a > b) ? b : a;
}

// Calculates how many rounds a player has played
static int roundsPlayed(GameView gv, Player player) {
    // Add one to round if player has already gone in current turn
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
		// Loop until location is not a move (placeIsReal ignores unknown)
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




// Unchecked:

//Convert a pastPlay into an abbreviation for a place
char *playToPlcAbbrev (char *play, int index) 
{
    char *abbrev = malloc (3 * sizeof(char));
    abbrev[0] = play[index];
    abbrev[1] = play[index + 1];
    abbrev[2] = '\0';
    return abbrev;
}


// Finding the health of a Dracula
int healthDracula (GameView gv, Player player, int numTurns) 
{
    int strtElmt = player;
    int increment = 0;
    int health = GAME_START_BLOOD_POINTS;

    for (int i = 0; i < numTurns; i++) {
        // For Normal Moves,
        char *abbrev = playToPlcAbbrev(gv->pastPlays, (strtElmt * 8) + increment + 1);
        health = isPlaceSeaOrCastle (abbrev, health);

        
        //For Double Back Moves
        if (isDoubleBackMove (gv->pastPlays, (strtElmt * 8) + increment + 1)) { // test for double back
            int p = gv->pastPlays[(strtElmt * 8) + increment + 2] - 48;
            char *abbrvtn = playToPlcAbbrev (gv->pastPlays,((strtElmt * 8) + increment + 1) - (p * 40));
            health = isPlaceSeaOrCastle (abbrvtn, health);
        } 
        
        // For HideMoves
        else if (isHideMove(gv->pastPlays, (strtElmt * 8) + increment + 1)) {
            char *plcAbbrev = playToPlcAbbrev (gv->pastPlays,((strtElmt * 8) + increment + 1) - 40 );
            health = isPlaceSeaOrCastle (plcAbbrev, health);
        }

        if (vampireHunterEncounter(gv, (((strtElmt + 1 ) * 8) + increment))) { // encounters a hunter
            health = health - LIFE_LOSS_HUNTER_ENCOUNTER;
        } 
        increment = increment + 40;
    }

    if (health < 0) return 0;
    return health;
}


// The wrapper function
// Determine if an abbreviation corresponds to a Sea or castle or neither. 
// It also calculates the new health for Dracula due to his location.
int isPlaceSeaOrCastle (char *abbrev, int health) 
{ 
    PlaceId pid = placeAbbrevToId (abbrev);
    PlaceType pType = placeIdToType (pid);
    health = doIsPlaceSeaOrCastle(pid, pType, health);
    free(abbrev);
    return health;
}


// Determine if an abbreviation corresponds to a Sea or castle or neither. 
// It also calculates the new health for Dracula due to his location.
int doIsPlaceSeaOrCastle(PlaceId pid, PlaceType pType, int health) 
{
    if (pType == SEA) { // place is sea
        health = health - LIFE_LOSS_SEA;    
    } else if (pid == CASTLE_DRACULA) { // in castle dracula
        health = health + LIFE_GAIN_CASTLE_DRACULA;
    }  
    return health;
}


//Determines if Dracula's move in the pastPlays string was a double back move.
bool isDoubleBackMove (char *pastPlays, int index) 
{
    if (pastPlays[index] != 'D') return false;
    if (pastPlays[index + 1] < '1' || pastPlays[index + 1] > '5' ) { //ensures 
    // that the char following 'D' is a number between 1 and 5
        return false;
    }
    return true;
    
}


// Determines if Dracula's move in the pastPlays string was a hide move.
bool isHideMove(char *pastPlays, int location) 
{
    if (pastPlays[location] != 'H') return false;
    if (pastPlays[location] != 'I') return false;
        return true;
}


// Determines whether Vampire and Hunter encounters each other from the past plays
// string.
bool vampireHunterEncounter (GameView gv, int location) 
{
    Player currPlayer = GvGetPlayer(gv);
    for(int i = 0; i < currPlayer; i++) {   
        if (gv->pastPlays[location + (8 * i) + 4] == 'D') { // vampire encountered
            return true;
        }
    }
    return false;
}


// Finds the Health of a Hunter given the pastPlays string
int healthHunter (GameView gv, Player player, int numTurns) 
{
    int strtElmt = player;
    int incre = 0;
    int health = GAME_START_HUNTER_LIFE_POINTS;

    for (int j = 0; j < numTurns; j++) {
        for (int i = 0; i < 4; i++) { // could separate this bit into a function
            if (gv->pastPlays[(strtElmt * 8) + 3 + incre + i] == 'T') { // trap
                health = health - LIFE_LOSS_TRAP_ENCOUNTER;
            } else if (gv->pastPlays[(strtElmt * 8) + 3 + incre + i] == 'D') { 
                // encounter dracula
                health = health - LIFE_LOSS_DRACULA_ENCOUNTER;
            } 
        }
        if (hunterRest(gv, (strtElmt * 8) + 1 + incre)) { 
            // Hunters have a maximum health
            health = min (health + LIFE_GAIN_REST, GAME_START_HUNTER_LIFE_POINTS);
        }
        incre = incre + 40;
    }

    if (health < 0) return 0;
    return health;
}


// Determines if hunter stays in the same location between sucessive turns 
// Hunter should not attempt to move to another location by rail.
bool hunterRest (GameView gv, int location) // need to also make sure that the hunter doesnt TRY to go anywhere by rail even thhough doesnt move . (idk how to do this....)
{   
    if (location - 40 < 0) return false;
    if (gv->pastPlays[location] != gv->pastPlays[location - 40]) return false; 
    if (gv->pastPlays[location + 1] != gv->pastPlays[location + 1 - 40]) {
        return false; 
    }
    return true; 
}
