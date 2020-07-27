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
#include <string.h>

#include "DraculaView.h"
#include "Game.h"
#include "GameView.h"
#include "Map.h"
// add your own #includes here

// TODO: ADD YOUR OWN STRUCTS HERE

struct draculaView {
	char *pastPlays;
	Map map;
};




///////////////////////////////////////////////////////////////////////////////
// Function prototypes

int DvFindNumMoves (DraculaView dv, Player player);
// Calculates how many rounds a player has played
int DvRoundsPlayed (DraculaView dv, Player player);
// Extract location for a specified move
PlaceId DvExtractLocation (DraculaView dv, Player player, PlaceId move, Round round);
// Unchecked
bool DvVampireHunterEncounter (DraculaView dv, int location);
bool DvHunterRest (DraculaView dv, int location) ;
int DvHealthDracula (DraculaView dv, Player player, int numTurns);

char *DvPlayToPlcAbbrev (char *play, int location);

int DvIsPlaceSeaOrCastle (char *abbrev, int health);
int doDvIsPlaceSeaOrCastle (PlaceId pid, PlaceType pType, int health);
bool DvIsDoubleBackMove (char *pastPlays, int index);
bool DvIsHideMove (char *pastPlays, int location);
int DvHealthHunter (DraculaView dv, Player player, int numTurns);
PlaceId *DvMakePlaceIdArray (int elements);
int maxNum (int a, int b);
int minNum (int a, int b);
PlaceId *DvGetLastMoves (DraculaView dv, Player player, int numMoves,
                        int *numReturnedMoves, bool *canFree);
PlaceId *DvGetLastLocations (DraculaView dv, Player player, int numLocs,
                            int *numReturnedLocs, bool *canFree);
Player DvGetPlayer (DraculaView dv);
char *movesInTrail (DraculaView dv);


///NewONes
bool inTrailAlready (PlaceId curr, char *Trail, int numTrailMoves); 

	



PlaceId draculaLastMove (DraculaView dv) {

	int numReturnedMoves;
	bool canFree;

	PlaceId *lastMove = DvGetLastMoves (dv, PLAYER_DRACULA, 1 ,
                        &numReturnedMoves, &canFree);
	
	if (lastMove[0] == HIDE) {
		lastMove = DvGetLastMoves (dv, PLAYER_DRACULA, 2, &numReturnedMoves, &canFree);
	} else if (lastMove[0] == DOUBLE_BACK_1) {
		lastMove = DvGetLastMoves (dv, PLAYER_DRACULA, 2, &numReturnedMoves, &canFree);
	} else if (lastMove[0] == DOUBLE_BACK_2) {
		lastMove = DvGetLastMoves (dv, PLAYER_DRACULA, 3, &numReturnedMoves, &canFree);
	} else if ( lastMove[0] == DOUBLE_BACK_3) {
		lastMove = DvGetLastMoves (dv, PLAYER_DRACULA, 4, &numReturnedMoves, &canFree);
	} else if (lastMove[0] == DOUBLE_BACK_4) {
		lastMove = DvGetLastMoves (dv, PLAYER_DRACULA, 5, &numReturnedMoves, &canFree);
	} else if (lastMove[0] == DOUBLE_BACK_5) {
		lastMove = DvGetLastMoves (dv, PLAYER_DRACULA, 6, &numReturnedMoves, &canFree);
	}

	return lastMove[0];
}





bool doubleBack (int numMoves, char *trail) {
	for (int i = 0; i < numMoves; i++) {
		if (DvIsDoubleBackMove(trail, i * 3)) return true;
	}
	return false;
}



bool hideMove (int numMovesTrail, char *trail) {
	for (int i = 0; i < numMovesTrail; i++) {
		if (DvIsHideMove (trail, i * 3)) return true;
	}
	return false;
}



int getNumReachablePlaces (ConnList connList, bool doubleBackInTrail, 
								bool hideInTrail, char *trail, PlaceId lastMove, 
								int numTrailMoves) 
{
	int numReachable = 0;
	while (connList != NULL) {
		PlaceId curr = connList->p;
		if (connList->type == RAIL) {
			connList = connList->next;
			continue;
		} else if (inTrailAlready (curr, trail, numTrailMoves) && doubleBackInTrail == true) {
			connList = connList->next;
			continue;
		} else if (inTrailAlready (curr, trail, numTrailMoves) && hideInTrail == true
					&& curr == lastMove) {
			connList = connList->next;
			continue;
		} 
		numReachable++;
		connList = connList->next;
	}
	return numReachable;
}




PlaceId *makeWhereCanIgoArray ( int numReachable, ConnList connList,
								 bool doubleBackInTrail, bool hideInTrail, 
								 char *trail, PlaceId lastMove, int numTrailMoves) 
{
	PlaceId *whereCanIgo = DvMakePlaceIdArray (numReachable);
	int i = 0;
	while (connList != NULL) {
		PlaceId curr = connList->p;
		if (connList->type == RAIL) {
			connList = connList->next;
			continue;
		} else if (inTrailAlready (curr, trail, numTrailMoves) && doubleBackInTrail == true) {
			connList = connList->next;
			continue;
		} else if (inTrailAlready (curr, trail, numTrailMoves) && hideInTrail == true
					&& curr == lastMove) {
			connList = connList->next;
			continue;
		} 
		whereCanIgo[i] = connList->p;
		i++;
		connList = connList->next;
	}
	return whereCanIgo;

}







int getNumReachablePlacesByType (ConnList connList, bool doubleBackInTrail, 
								bool hideInTrail, char *trail, PlaceId lastMove, 
								int numTrailMoves, bool boat, bool road) 
{ 
	int numReachable = 0;
	while (connList != NULL) {
		PlaceId curr = connList->p;
		if (connList->type == RAIL) {
			connList = connList->next;
			continue;
		} else if (connList->type == ROAD && road == false) {
			connList = connList->next;
			continue;
		} else if (connList->type == BOAT && boat == false) {
			connList = connList->next;
			continue;
		} else if (inTrailAlready (curr, trail, numTrailMoves) && doubleBackInTrail == true) {
			connList = connList->next;
			continue;
		} else if (inTrailAlready (curr, trail, numTrailMoves) && hideInTrail == true
					&& curr == lastMove) {
			connList = connList->next;
			continue;
		} 
		numReachable++;
		connList = connList->next;
	}
	return numReachable;
}






PlaceId *makeWhereCanIgoArrayByType ( int numReachable, ConnList connList,
								 	bool doubleBackInTrail, bool hideInTrail, 
								 	char *trail, PlaceId lastMove, 
									int numTrailMoves, bool road, bool boat) 
{
	PlaceId *whereCanIgo = malloc (numReachable * sizeof(PlaceId));
	int i = 0;
	while (connList != NULL) {
		PlaceId curr = connList->p;
		if (connList->type == RAIL) {
			connList = connList->next;
			continue;
		} else if (connList->type == ROAD && road == false) {
			connList = connList->next;
			continue;
		} else if (connList->type == BOAT && boat == false) {
			connList = connList->next;
			continue;
		} else if (inTrailAlready (curr, trail, numTrailMoves) && doubleBackInTrail == true) {
			connList = connList->next;
			continue;
		} else if (inTrailAlready (curr, trail, numTrailMoves) && hideInTrail == true
					&& curr == lastMove) {
			connList = connList->next;
			continue;
		} 
		whereCanIgo[i] = connList->p;
		i++;
		connList = connList->next;
	}


	return whereCanIgo;
}









////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

DraculaView DvNew(char *pastPlays, Message messages[])
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	DraculaView new = malloc(sizeof(*new));
	if (new == NULL) {
		fprintf(stderr, "Couldn't allocate DraculaView\n");
		exit(EXIT_FAILURE);
	}
	new->pastPlays = strdup(pastPlays);
	new->map = MapNew();
	return new;
}

void DvFree (DraculaView dv)
{
	free (dv->pastPlays);
	MapFree(dv->map);
	free (dv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round DvGetRound(DraculaView dv)
{
	return (strlen(dv->pastPlays) + 1) / 40;
}

int DvGetScore(DraculaView dv)
{
	if (DvGetRound(dv) == 0) {
		return GAME_START_SCORE;
	}

	int score = GAME_START_SCORE;

	// decreases by 1 each time Dracula finishes his turn
	score = score - DvGetRound(dv) * SCORE_LOSS_DRACULA_TURN;

        // decreases by 6 each time a hunter loses all life points and teleported
	// to St Joseph and St Mary
	for (int i = 0; i < NUM_PLAYERS - 1; i++) {
		if (DvGetHealth(dv, i) <= 0) {
			score -= SCORE_LOSS_HUNTER_HOSPITAL;
		}
	}

	// decreases by 13 each time a vampire matures (falls off the trail)
	int index = ((strlen(dv->pastPlays) + 1) / 40 - 1) * 40 + 38 - 1;
        // If finding the vampire index in the first turn
	if (index < 0) index = 37;
	
	for (int i = 0; i < strlen(dv->pastPlays) % 40; i++) {
		if (dv->pastPlays[index] == 'V') {
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

int DvGetHealth(DraculaView dv, Player player)
{
	// get number of turns
    Round numTurns = DvFindNumMoves(dv, player);

    if (player == PLAYER_DRACULA) { //player is Dracula
        return DvHealthDracula (dv, player, numTurns);
    } else { // If player is Hunter
        return DvHealthHunter (dv, player, numTurns);
    }
}

PlaceId DvGetPlayerLocation(DraculaView dv, Player player)
{
	// Retrieve the last location using DvGetLastLocations
	int numLocs = 0; bool canFree = true;
    PlaceId *locations = DvGetLastLocations(dv, player, 1, &numLocs, &canFree);
    PlaceId location = locations[0];
    free(locations);

    // No locations returned
    if (numLocs == 0) {
		return NOWHERE;
	// Player is a hunter
	} else if (player != PLAYER_DRACULA) {
		return (DvGetHealth(dv, player) <= 0) ? ST_JOSEPH_AND_ST_MARY : location;
	// Player is Dracula
	} else {
        return location;
    }
}

PlaceId DvGetVampireLocation(DraculaView dv)
{
    // Vampire not spawned if the 5th player (Dracula) hasn't played
    if (strlen(dv->pastPlays) <= 4*8) return NOWHERE;
    // Divide message length by 40 (rounded down) to get the round
    // Subtract 1 to go to the previous row
    // Multiply by 40 to get the index of the row immediately above
    // Add 36 to get the position of 'V' in that line (arrays zero indexed)
    int index = ((strlen(dv->pastPlays) + 1) / 40 - 1) * 40 + 37 - 1;
    // If finding the vampire index in the first turn
    if (index < 0) index = 36;
    // Vampire has been vanquished by one of the hunters
    int vanquished = ((strlen(dv->pastPlays) + 1) / 40) * 40 + 4 - 1;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (dv->pastPlays[vanquished] == 'V') return NOWHERE;
            vanquished++;
        }
        vanquished += 5;
    }
    for (int i = 0; i < strlen(dv->pastPlays) % 40; i++) {
        // Vampire exists
        if (dv->pastPlays[index] == 'V') {
            char code[3];
            code[0] = dv->pastPlays[index - 3];
            code[1] = dv->pastPlays[index - 2];
            code[2] = '\0';
            return placeAbbrevToId(code);
        }
        // Vampire has matured
        if (dv->pastPlays[index + 1] == 'V') return NOWHERE;
        index -= 40;
    }
    
    // Otherwise vampire hasn't been spawned
    return NOWHERE; 
}

PlaceId *DvGetTrapLocations(DraculaView dv, int *numTraps)
{
	int traps = 0;
	int index = 3;
	PlaceId *trapLocations = malloc(sizeof(PlaceId*));
	for (int i = 0; i < strlen(dv->pastPlays) / 8; i++) {
		if (dv->pastPlays[index] == 'T') {
			if (dv->pastPlays[index - 3] == 'D') {
				char location[3];
				location[2] = '\0';
				location[0] = dv->pastPlays[index - 2];
				location[1] = dv->pastPlays[index - 1];
				PlaceId loc = placeAbbrevToId(location);
				trapLocations[traps] = loc;
				traps++;
				strncpy(location, "", strlen(location));
			} else if (dv->pastPlays[index - 3] != 'D') {
				traps--;
				char location[3];
				location[2] = '\0';
				location[0] = dv->pastPlays[index - 2];
				location[1] = dv->pastPlays[index - 1];
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
// Making a Move

PlaceId *DvGetValidMoves(DraculaView dv, int *numReturnedMoves)
{
	if (DvFindNumMoves(dv, PLAYER_DRACULA) == 0) { // There are no previous moves.
		*numReturnedLocs = 0;
		return NULL;
	}
	
	
	
	
	
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedMoves = 0;
	return NULL;
}



// i did not consider TP
PlaceId *DvWhereCanIGo(DraculaView dv, int *numReturnedLocs)
{
	if (DvFindNumMoves(dv, PLAYER_DRACULA) == 0) { // There are no previous moves.
		*numReturnedLocs = 0;
		return NULL;
	}

	// Finding the last known location of dracula
	PlaceId lastMove = draculaLastMove (dv);
	// Finding the locations that are connected to dracula's location
	ConnList connList = MapGetConnections (dv->map, lastMove);
	
	//Making dracula's trail and determining the number of moves in his trail.
	char *trail = movesInTrail(dv);
	int numTrailMoves = minNum(TRAIL_SIZE, DvRoundsPlayed(dv, PLAYER_DRACULA));
	
	// Determining if there is a double back move in the trail
	bool doubleBackInTrail = doubleBack (numTrailMoves, trail);
	// Determining if there is a hide move in the trail
	bool hideInTrail = hideMove (numTrailMoves, trail);

	// Finding the number of reachable places from Dracula's current location.
	int numReachable = getNumReachablePlaces (connList, doubleBackInTrail, 
												hideInTrail, trail, lastMove, 
												numTrailMoves);
	*numReturnedLocs = numReachable;
	if (numReturnedLocs == 0) return NULL;

	// creating and filling an array of reachable places from Dracula's current location.
	return makeWhereCanIgoArray (numReachable, connList, doubleBackInTrail,
								 hideInTrail, trail, lastMove, numTrailMoves);
}





PlaceId *DvWhereCanIGoByType(DraculaView dv, bool road, bool boat,
                             int *numReturnedLocs)
{
	if (DvFindNumMoves(dv, PLAYER_DRACULA) == 0) { // There are no previous moves.
		*numReturnedLocs = 0;
		return NULL;
	}

	// Finding the last known location of dracula
	PlaceId lastMove = draculaLastMove (dv);
	// Finding the locations that are connected to dracula's location
	ConnList connList = MapGetConnections (dv->map, lastMove);
	
	//Making dracula's trail and determining the number of moves in his trail.
	char *trail = movesInTrail(dv);
	int numTrailMoves = minNum(TRAIL_SIZE, DvRoundsPlayed(dv, PLAYER_DRACULA));
	
	// Determining if there is a double back move in the trail
	bool doubleBackInTrail = doubleBack (numTrailMoves, trail);
	// Determining if there is a hide move in the trail
	bool hideInTrail = hideMove (numTrailMoves, trail);
	
	// Finding the number of reachable places from Dracula's current location.
	// This is is dependent on the type of transport required to reach this location
	// from Dracula's current location. 
	int numReachable = getNumReachablePlacesByType (connList, doubleBackInTrail, 
													hideInTrail, trail, lastMove, 
													numTrailMoves, boat, road); 

	*numReturnedLocs = numReachable;
	if (numReturnedLocs == 0) return NULL;

	// creating and filling an array of reachable places from Dracula's current location.
	// This is is dependent on the type of transport required to reach this location. 
	// from Dracula's current location.
	return makeWhereCanIgoArrayByType ( numReachable, connList, doubleBackInTrail, 
										hideInTrail, trail, lastMove, numTrailMoves,
										road,boat); 
}

PlaceId *DvWhereCanTheyGo(DraculaView dv, Player player,
                          int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	return NULL;
}

PlaceId *DvWhereCanTheyGoByType(DraculaView dv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	return NULL;
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// TODO




// Finding the number of turns the player took
int DvFindNumMoves (DraculaView dv, Player player)
{
    Round round = DvGetRound (dv);
    if (DvGetPlayer(dv) > player) round++; // because rounds are zero indexed
    return round;
}






// Finding the health of a Dracula
int DvHealthDracula (DraculaView dv, Player player, int numTurns) 
{
    int strtElmt = player;
    int increment = 0;
    int health = GAME_START_BLOOD_POINTS;

    for (int i = 0; i < numTurns; i++) {
        // For Normal Moves,
        char *abbrev = DvPlayToPlcAbbrev(dv->pastPlays, (strtElmt * 8) + increment + 1);
        health = DvIsPlaceSeaOrCastle (abbrev, health);

        
        //For Double Back Moves
        if (DvIsDoubleBackMove (dv->pastPlays, (strtElmt * 8) + increment + 1)) { // test for double back
            int p = dv->pastPlays[(strtElmt * 8) + increment + 2] - 48;
            char *abbrvtn = DvPlayToPlcAbbrev (dv->pastPlays,((strtElmt * 8) + increment + 1) - (p * 40));
            health = DvIsPlaceSeaOrCastle (abbrvtn, health);
        } 
        
        // For HideMoves
        else if (DvIsHideMove(dv->pastPlays, (strtElmt * 8) + increment + 1)) {
            char *plcAbbrev = DvPlayToPlcAbbrev (dv->pastPlays,((strtElmt * 8) + increment + 1) - 40 );
            health = DvIsPlaceSeaOrCastle (plcAbbrev, health);
        }

        if (DvVampireHunterEncounter(dv, (((strtElmt + 1 ) * 8) + increment))) { // encounters a hunter
            health = health - LIFE_LOSS_HUNTER_ENCOUNTER;
        } 
        increment = increment + 40;
    }

    if (health < 0) return 0;
    return health;
}



int DvRoundsPlayed(DraculaView dv, Player player) {
    // Add one to round if player has already gone in current turn
    return (player < DvGetPlayer(dv)) ? DvGetRound(dv) + 1 : DvGetRound(dv);
}

//Convert a pastPlay into an abbreviation for a place
char *DvPlayToPlcAbbrev (char *play, int index) 
{
    char *abbrev = malloc (3 * sizeof(char));
    abbrev[0] = play[index];
    abbrev[1] = play[index + 1];
    abbrev[2] = '\0';
    return abbrev;
}


PlaceId DvExtractLocation(DraculaView dv, Player player, PlaceId move, Round round) {
    // Hunters
    if (player != PLAYER_DRACULA) {
		// Health up to turn
        return (DvGetHealth(dv, player) <= 0) ? ST_JOSEPH_AND_ST_MARY : move;
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
        code[0] = dv->pastPlays[playerIndex + 1];
		code[1] = dv->pastPlays[playerIndex + 2];
		code[2] = '\0';
		move = placeAbbrevToId(code);
	}

	if (!found) return UNKNOWN_PLACE;
	else if (move == TELEPORT) return CASTLE_DRACULA;
	else return move;
}

// The wrapper function
// Determine if an abbreviation corresponds to a Sea or castle or neither. 
// It also calculates the new health for Dracula due to his location.
int DvIsPlaceSeaOrCastle (char *abbrev, int health) 
{ 
    PlaceId pid = placeAbbrevToId (abbrev);
    PlaceType pType = placeIdToType (pid);
    health = doDvIsPlaceSeaOrCastle(pid, pType, health);
    free(abbrev);
    return health;
}


// Unchecked yet

// Determine if an abbreviation corresponds to a Sea or castle or neither. 
// It also calculates the new health for Dracula due to his location.
int doDvIsPlaceSeaOrCastle(PlaceId pid, PlaceType pType, int health) 
{
    if (pType == SEA) { // place is sea
        health = health - LIFE_LOSS_SEA;    
    } else if (pid == CASTLE_DRACULA) { // in castle dracula
        health = health + LIFE_GAIN_CASTLE_DRACULA;
    }  
    return health;
}





//Determines if Dracula's move in the pastPlays string was a double back move.
bool DvIsDoubleBackMove (char *pastPlays, int index) 
{
    if (pastPlays[index] != 'D') return false;
    if (pastPlays[index + 1] < '1' || pastPlays[index + 1] > '5' ) { //ensures 
    // that the char following 'D' is a number between 1 and 5
        return false;
    }
    return true;
    
}




// Determines if Dracula's move in the pastPlays string was a hide move.
bool DvIsHideMove(char *pastPlays, int location) 
{
    if (pastPlays[location] != 'H') return false;
    if (pastPlays[location + 1] != 'I') return false;
        return true;
}






// Determines whether Vampire and Hunter encounters each other from the past plays
// string.
bool DvVampireHunterEncounter (DraculaView dv, int location) 
{
    Player currPlayer = DvGetPlayer(dv);
    for(int i = 0; i < currPlayer; i++) {   
        if (dv->pastPlays[location + (8 * i) + 4] == 'D') { // vampire encountered
            return true;
        }
    }
    return false;
}





// Finds the Health of a Hunter given the pastPlays string
int DvHealthHunter (DraculaView dv, Player player, int numTurns) 
{
    int strtElmt = player;
    int incre = 0;
    int health = GAME_START_HUNTER_LIFE_POINTS;

    for (int j = 0; j < numTurns; j++) {
        for (int i = 0; i < 4; i++) { // could separate this bit into a function
            if (dv->pastPlays[(strtElmt * 8) + 3 + incre + i] == 'T') { // trap
                health = health - LIFE_LOSS_TRAP_ENCOUNTER;
            } else if (dv->pastPlays[(strtElmt * 8) + 3 + incre + i] == 'D') { 
                // encounter dracula
                health = health - LIFE_LOSS_DRACULA_ENCOUNTER;
            } 
        }
        if (DvHunterRest(dv, (strtElmt * 8) + 1 + incre)) { 
            // Hunters have a maximum health
            health = minNum(health + LIFE_GAIN_REST, GAME_START_HUNTER_LIFE_POINTS);
        }
        incre = incre + 40;

		if (health <= 0 && j < numTurns - 1) health = GAME_START_HUNTER_LIFE_POINTS;
        else if (health <= 0 && j == numTurns - 1) health = 0; 
    }

    return health;
}








// Determines if hunter stays in the same location between sucessive turns 
// Hunter should not attempt to move to another location by rail.
bool DvHunterRest (DraculaView dv, int location) // need to also make sure that the hunter doesnt TRY to go anywhere by rail even thhough doesnt move . (idk how to do this....)
{   
    if (location - 40 < 0) return false;
    if (dv->pastPlays[location] != dv->pastPlays[location - 40]) return false; 
    if (dv->pastPlays[location + 1] != dv->pastPlays[location + 1 - 40]) {
        return false; 
    }
    return true; 
}










// Dynamically allocates memory for a PlaceId array given the number of elements
PlaceId *DvMakePlaceIdArray (int elements)
{
    PlaceId *array = malloc(elements * sizeof(PlaceId));
    if (array == NULL) { // memory not allocated
        fprintf(stderr, "Insufficient memory!\n");
        exit(EXIT_FAILURE);
    }
    return array;
}





int maxNum(int a, int b) {
    return (a > b) ? a : b;
}


int minNum(int a, int b) {
    return (a > b) ? b : a;
}


PlaceId *DvGetLastMoves(DraculaView dv, Player player, int numMoves,
                        int *numReturnedMoves, bool *canFree)
{
	// Allocate memory for the array
	PlaceId *moves = malloc(numMoves * sizeof(PlaceId));
	if (moves == NULL) { /////////////////////////////////////////////////////////////// What if numMOves = 0;
        	fprintf(stderr, "Failed to allocate memory!\n");
        	exit(EXIT_FAILURE);
	}

	// Determine the bounds for the loop
	    int finalMove = DvRoundsPlayed(dv, player);
	    int firstMove = maxNum(finalMove - numMoves, 0);
	    int moveCounter = 0;
	    for (int round = firstMove; round < finalMove; round++) {
		// Extract location for player within round
		char abbrev[3] = {0};
		abbrev[0] = dv->pastPlays[round * 40 + player * 8 + 1];
		abbrev[1] = dv->pastPlays[round * 40 + player * 8 + 2];
		abbrev[2] = '\0';
		// Append to moves array
		moves[moveCounter] = placeAbbrevToId(abbrev);
		moveCounter++;
	    }
    
	*numReturnedMoves = moveCounter;
	*canFree = true;
	return moves;
}







PlaceId *DvGetLastLocations(DraculaView dv, Player player, int numLocs,
                            int *numReturnedLocs, bool *canFree)
{
	// Retrieve the last moves using GvGetLastMoves
	PlaceId *moves = DvGetLastMoves(dv, player, numLocs, numReturnedLocs, canFree);
    
    // Find location for special moves
    int roundOffset = DvRoundsPlayed(dv, player) - *numReturnedLocs;
    for (int index = 0; index < *numReturnedLocs; index++) {
        moves[index] = DvExtractLocation(dv, player, moves[index], index + roundOffset);
    }
    return moves;
}


Player DvGetPlayer(DraculaView dv)
{
    // strlen() % 40 indicates how far across each round.
    // adding one before dividing by eight signifies
    // the player for the next turn.
    // this % 5 makes player 5 loop back to player 0.
    return (((strlen(dv->pastPlays) % 40) + 1) / 8) % 5;
}









// Find the moves in draculaView Trail.
char *movesInTrail (DraculaView dv) {
	
	int turns = DvRoundsPlayed (dv, PLAYER_DRACULA);
	int elements = minNum (TRAIL_SIZE, turns);
	char *Trail = malloc ((elements * 3 * sizeof(char)) + 1);
	
	
	int stringLen = strlen(dv->pastPlays);
	int currentPlayer = DvGetPlayer(dv);
	int startingIndex = stringLen - 1 - 5 - (currentPlayer * 8);

	for (int i = 0; i < (elements * 3) + 1; i+=3) {
		Trail[i] = dv->pastPlays[startingIndex];
		Trail[i + 1] = dv->pastPlays[startingIndex + 1];
		Trail[i + 2] = ' ';
		startingIndex -= 40;
	}
	return Trail;

}


// Free moves in trail function
// void freeMovesInTrail (char **trail) { }
// can our program leak memory???






bool inTrailAlready (PlaceId curr, char *Trail, int numTrailMoves) {			
	for (int i = 0; i < numTrailMoves ; i++){
		char *placeInTrail = DvPlayToPlcAbbrev(Trail, i * 3);
		PlaceId trailPlace = placeAbbrevToId(placeInTrail);
		if (curr == trailPlace) return true;	
	}
	return false;
}






