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

#include <math.h>




// TODO: ADD YOUR OWN STRUCTS HERE

struct gameView {
	Round round;
	char *pastPlays;
	Map map;
};


///////////////////////////////////////////////////////////////////////////////
// Function prototypes

int min (int a , int b);
bool vampireHunterEncounter (GameView gv, int location);
bool hunterRest (GameView gv, int location) ;
int healthDracula (GameView gv, Player player, int numTurns);
int healthHunter (GameView gv, Player player, int numTurns);
int findNumMoves (GameView gv, Player player);





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
	new->map = MapNew();

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
	// get number of turns
	Round numTurns = GvGetRound(gv);
	if (GvGetPlayer(gv) > player) numTurns++;

	
	// If player is Dracula
	if (player == PLAYER_DRACULA) {
		return healthDracula (gv, player, numTurns);

	} else { // If player is Hunter
		return healthHunter (gv, player, numTurns);
	}
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
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	if (gv->round == 0) return NOWHERE;
	int lastLocationIndex = (gv->round - 1) * 40 + 32;
	char code[3] = {0};
	code[0] = gv->pastPlays[lastLocationIndex + 1];
	code[1] = gv->pastPlays[lastLocationIndex + 2];
	code[2] = '\0';
	return placeAbbrevToId(code);
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
	int numMoves = findNumMoves (gv, player);
	PlaceId *moveHistory = malloc (numMoves * sizeof(PlaceId));

	int strtElmt = player;
	int incre = 0;

	for (int i = 0; i < numMoves ; i++) { // need cases for Drac and hunter
		char abbrev[3];
		abbrev[0] = gv->pastPlays[(strtElmt * 8) + incre + 1 ];
		abbrev[1] = gv->pastPlays[(strtElmt * 8) + incre + 2 ];
		abbrev[2] = '\0';
		moveHistory [i] = placeAbbrevToId (abbrev);
		incre = incre + 40;
	}

	*numReturnedMoves = numMoves;
	*canFree = false;
	return moveHistory; 
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
	ConnList connList = MapGetConnections (gv->map, from);

	if (player == PLAYER_DRACULA) {
		
		// find the number of valid Connections in the linked list
		int i = 1;
		
		while (connList != NULL ) {
			if (connList->type != RAIL && connList->p != ST_JOSEPH_AND_ST_MARY) {
				i++;
			}
			connList = connList->next;
		}

		//allocate memory for dynamic array
		PlaceId *reachable = malloc(i * sizeof(PlaceId));
		if (reachable == NULL) {
			fprintf(stderr, "Insufficient memory!\n");
			exit(EXIT_FAILURE);
		}
		*numReturnedLocs = i;

		
		int k = 0;
		while (connList!= NULL) {
			if (connList->type != RAIL && connList->p != ST_JOSEPH_AND_ST_MARY) {
				reachable[k] = connList->p;
				k++;
			}
			connList = connList->next;
		}

		return reachable;
	} 
	
	
	
	
	else { // hunter
		int i = 1; 
		while (connList != NULL) {
			if (connList->type == RAIL && (round + player) % 4 == 0) {
				connList = connList->next;
				continue;
			} else {
				connList = connList->next;
			}
			i++;
		}


		PlaceId *reachable = malloc(i * sizeof(PlaceId));
		if (reachable == NULL) {
			fprintf(stderr, "Insufficient memory!\n");
			exit(EXIT_FAILURE);
		}
		*numReturnedLocs = i;

		int k = 0;
		while (connList != NULL) {
			if (connList->type == RAIL && (round + player) % 4 == 0) {
				reachable[k] = connList->p;
				k++;
			}
			connList = connList->next;
		}
		return reachable;
	}
	
}

PlaceId *GvGetReachableByType(GameView gv, Player player, Round round,
                              PlaceId from, bool road, bool rail,
                              bool boat, int *numReturnedLocs)
{
	ConnList connList = MapGetConnections (gv->map, from);

	if (player == PLAYER_DRACULA) {
		// find the number of valid Connections in the linked list
		int i = 1;
		
		while (connList != NULL ) {
			if (connList->type == ROAD && road == true && connList->p != ST_JOSEPH_AND_ST_MARY) {
				i++;
			}
			if (connList->type == BOAT && boat == true && connList->p != ST_JOSEPH_AND_ST_MARY) {
				i++;
			}
			connList = connList->next;
		}

		//allocate memory for dynamic array
		PlaceId *reachable = malloc(i * sizeof(PlaceId));
		if (reachable == NULL) {
			fprintf(stderr, "Insufficient memory!\n");
			exit(EXIT_FAILURE);
		}
		*numReturnedLocs = i;

		
		int k = 0;
		while (connList!= NULL) {
			if (connList->type == ROAD && road == true && connList->p != ST_JOSEPH_AND_ST_MARY) {
				reachable[k] = connList->p;
				k++;
			}
			if (connList->type == BOAT && boat == true && connList->p != ST_JOSEPH_AND_ST_MARY) {
				reachable[k] = connList->p;
				k++;				
			}
			connList = connList->next;
		}

		return reachable;
	} 
	
	
	
	
	else { // hunter
		int i = 1; 
		if (connList->type == RAIL && (round + player) % 4 == 0) {
			rail = false;
		}

		while (connList != NULL) {
			if (connList->type == RAIL && rail == true) {
				i++;
			} else if (connList->type == ROAD && road == true) {
				i++;
			} else if (connList->type == BOAT && boat == true) {
				i++;
			}
			connList = connList->next;
		}


		PlaceId *reachable = malloc(i * sizeof(PlaceId));
		if (reachable == NULL) {
			fprintf(stderr, "Insufficient memory!\n");
			exit(EXIT_FAILURE);
		}
		*numReturnedLocs = i;

		int k = 0;
		while (connList != NULL) {
			if (connList->type == RAIL && rail == true) {
				reachable[k] = connList->p;
				k++;				
			} else if (connList->type == ROAD && road == true) {
				reachable[k] = connList->p;
				k++;
			} else if (connList->type == BOAT && boat == true) {
				reachable[k] = connList->p;
				k++;
			}
			connList = connList->next;
		}
		return reachable;
	}
		
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	return NULL;
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// TODO

// interface functions










int min (int a , int b) {
	return (a > b) ? b : a;
}






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














int findNumMoves (GameView gv, Player player)
{
	// Finding the number of turns they took
	Round round = GvGetRound (gv);
	if (GvGetPlayer(gv) > player) round++;

	return round;
}