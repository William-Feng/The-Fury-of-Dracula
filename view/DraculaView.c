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
	GameView gv;
	Map map;
};





char *movesInTrail (DraculaView dv);
bool inTrailAlready (PlaceId curr, char *Trail, int numTrailMoves); 
int minNum (int a, int b);
PlaceId *DvMakePlaceIdArray (int elements);
int DvFindNumMoves (GameView gv, Player player);
PlaceId draculaLastMove (DraculaView dv);
bool DvIsDoubleBackMove (char *pastPlays, int index);
bool doubleBack (int numMoves, char *trail);
bool DvIsHideMove(char *pastPlays, int index);
bool hideMove (int numMovesTrail, char *trail);
int getNumReachablePlaces (PlaceId *reachable, int numReturnedLocs, bool doubleBackInTrail, 
								bool hideInTrail, char *trail, PlaceId lastMove, 
								int numTrailMoves) ;
PlaceId *makeWhereCanIgoArray (int numReachable, PlaceId *reachable, int numReturnedLocs,
								 bool doubleBackInTrail, bool hideInTrail, 
								 char *trail, PlaceId lastMove, int numTrailMoves);


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

	new->gv = GvNew(pastPlays, messages);
	new->pastPlays = strdup(pastPlays);
	new->map = MapNew();
	return new;
}

void DvFree (DraculaView dv)
{
	GvFree(dv->gv);
	free (dv->pastPlays);
	free (dv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round DvGetRound(DraculaView dv)
{
	return GvGetRound(dv->gv);
}

int DvGetScore(DraculaView dv)
{
	return GvGetScore(dv->gv); 
}

int DvGetHealth(DraculaView dv, Player player)
{
	return GvGetHealth(dv->gv,player);
}

PlaceId DvGetPlayerLocation(DraculaView dv, Player player)
{
	return GvGetPlayerLocation(dv->gv, player);
}

PlaceId DvGetVampireLocation(DraculaView dv)
{
	return GvGetVampireLocation(dv->gv);
}

PlaceId *DvGetTrapLocations(DraculaView dv, int *numTraps)
{
	return GvGetTrapLocations(dv->gv, numTraps);	
}

////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId *DvGetValidMoves(DraculaView dv, int *numReturnedMoves)
{
	if (DvFindNumMoves(dv->gv, PLAYER_DRACULA) == 0) { // There are no previous moves.
		*numReturnedMoves = 0;
		return NULL;
	}
	
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedMoves = 0;
	return NULL;
}



// i did not consider TP
PlaceId *DvWhereCanIGo(DraculaView dv, int *numReturnedLocs)
{
	return DvWhereCanIGoByType(dv, true, true, numReturnedLocs);
}





PlaceId *DvWhereCanIGoByType(DraculaView dv, bool road, bool boat,
                             int *numReturnedLocs)
{
	if (DvFindNumMoves(dv->gv, PLAYER_DRACULA) == 0) { // There are no previous moves.
		*numReturnedLocs = 0;
		return NULL;
	}
	
	// Finding the last known location of dracula
	PlaceId lastMove = draculaLastMove (dv);

	int numberLocsGv;
	PlaceId *reachable = GvGetReachableByType(dv->gv,PLAYER_DRACULA, 
												GvGetRound(dv->gv), lastMove, road,
												false, boat, &numberLocsGv);

	
	//Determing the number of moves in Dracula's trail.
	char *trail = movesInTrail(dv);
	int numTrailMoves = minNum(TRAIL_SIZE, DvFindNumMoves(dv->gv, PLAYER_DRACULA));	
	
	// Determining if there is a double back move in the trail
	bool doubleBackInTrail = doubleBack (numTrailMoves, trail);
	// Determining if there is a hide move in the trail
	bool hideInTrail = hideMove (numTrailMoves, trail);

	int numReachable = getNumReachablePlaces (reachable, numberLocsGv, doubleBackInTrail, 
												hideInTrail, trail, lastMove, 
												numTrailMoves);
	*numReturnedLocs = numReachable;
	if (numReturnedLocs == 0) return NULL;

	// creating and filling an array of reachable places from Dracula's current location.
	return makeWhereCanIgoArray (numReachable, reachable, numberLocsGv, doubleBackInTrail,
								 hideInTrail, trail, lastMove, numTrailMoves);		
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









// Find the moves in draculaView Trail.
char *movesInTrail (DraculaView dv) {
	
	int turns = DvFindNumMoves(dv->gv, PLAYER_DRACULA);
	int elements = minNum (TRAIL_SIZE, turns);
	char *Trail = malloc ((elements * 3 * sizeof(char)) + 1);
	
	
	int stringLen = strlen(dv->pastPlays);
	int currentPlayer = GvGetPlayer(dv->gv);
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


char *GvPlayToPlcAbbrev (char *play, int index) 
{
    char *abbrev = malloc (3 * sizeof(char));
    abbrev[0] = play[index];
    abbrev[1] = play[index + 1];
    abbrev[2] = '\0';
    return abbrev;
}



bool inTrailAlready (PlaceId curr, char *Trail, int numTrailMoves) {			
	for (int i = 0; i < numTrailMoves ; i++){
		char *placeInTrail = GvPlayToPlcAbbrev(Trail, i * 3);
		PlaceId trailPlace = placeAbbrevToId(placeInTrail);
		if (curr == trailPlace) return true;	
	}
	return false;
}





int minNum (int a, int b) {
    return (a > b) ? b : a;
}





	
PlaceId *DvMakePlaceIdArray (int elements)
{
    PlaceId *array = malloc(elements * sizeof(PlaceId));
    if (array == NULL) { // memory not allocated
        fprintf(stderr, "Insufficient memory!\n");
        exit(EXIT_FAILURE);
    }
    return array;
}


// Finding the number of turns the player took
int DvFindNumMoves (GameView gv, Player player)
{
    Round round = GvGetRound (gv);
    if (GvGetPlayer(gv) > player) round++; // because rounds are zero indexed
    return round;
}



PlaceId draculaLastMove (DraculaView dv) {

	int numReturnedMoves;
	bool canFree;

	PlaceId *lastMove = GvGetLastMoves (dv->gv, PLAYER_DRACULA, 1 ,
                        &numReturnedMoves, &canFree);
	
	if (lastMove[0] == HIDE) {
		lastMove = GvGetLastMoves (dv->gv, PLAYER_DRACULA, 2, &numReturnedMoves, &canFree);
	} else if (lastMove[0] == DOUBLE_BACK_1) {
		lastMove = GvGetLastMoves (dv->gv, PLAYER_DRACULA, 2, &numReturnedMoves, &canFree);
	} else if (lastMove[0] == DOUBLE_BACK_2) {
		lastMove = GvGetLastMoves (dv->gv, PLAYER_DRACULA, 3, &numReturnedMoves, &canFree);
	} else if ( lastMove[0] == DOUBLE_BACK_3) {
		lastMove = GvGetLastMoves (dv->gv, PLAYER_DRACULA, 4, &numReturnedMoves, &canFree);
	} else if (lastMove[0] == DOUBLE_BACK_4) {
		lastMove = GvGetLastMoves (dv->gv, PLAYER_DRACULA, 5, &numReturnedMoves, &canFree);
	} else if (lastMove[0] == DOUBLE_BACK_5) {
		lastMove = GvGetLastMoves (dv->gv, PLAYER_DRACULA, 6, &numReturnedMoves, &canFree);
	}

	return lastMove[0];
}



bool doubleBack (int numMoves, char *trail) {
	for (int i = 0; i < numMoves; i++) {
		if (DvIsDoubleBackMove(trail, i * 3)) return true;
	}
	return false;
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







bool hideMove (int numMovesTrail, char *trail) {
	for (int i = 0; i < numMovesTrail; i++) {
		if (DvIsHideMove (trail, i * 3)) return true;
	}
	return false;
}




// Determines if Dracula's move in the pastPlays string was a hide move.
bool DvIsHideMove(char *pastPlays, int index) 
{
    if (pastPlays[index] != 'H') return false;
    if (pastPlays[index + 1] != 'I') return false;
        return true;
}





int getNumReachablePlaces (PlaceId *reachable, int numReturnedLocs, bool doubleBackInTrail, 
								bool hideInTrail, char *trail, PlaceId lastMove, 
								int numTrailMoves) 
{
	int numReachable = 0;
	for (int i= 0; i < numReturnedLocs; i++) {
		PlaceId curr = reachable[i];
		if (inTrailAlready (curr, trail, numTrailMoves) && doubleBackInTrail == true) {
			continue;
		} else if (inTrailAlready (curr, trail, numTrailMoves) && hideInTrail == true
					&& curr == lastMove) {
			continue;
		} 
		numReachable++;
	}
	return numReachable;
}




PlaceId *makeWhereCanIgoArray ( int numReachable, PlaceId *reachable, int numReturnedLocs,
								 bool doubleBackInTrail, bool hideInTrail, 
								 char *trail, PlaceId lastMove, int numTrailMoves) 
{
	int j = 0;
	PlaceId *whereCanIgo = DvMakePlaceIdArray (numReachable);
	for (int i = 0; i < numReturnedLocs; i++) {
		PlaceId curr = reachable[i];
		if (inTrailAlready (curr, trail, numTrailMoves) && doubleBackInTrail == true) {
			continue;
		} else if (inTrailAlready (curr, trail, numTrailMoves) && hideInTrail == true
					&& curr == lastMove) {
			continue;
		} 
		whereCanIgo[j] = reachable[i];
		j++;
	}
	return whereCanIgo;

}




