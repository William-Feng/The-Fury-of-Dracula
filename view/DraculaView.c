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

struct draculaView {
	char *pastPlays;
	GameView gv;
};

///////////////////////////////////////////////////////////////////////////////
// Function prototypes

// Checks if a PlaceId is already in the array
static bool inArray(PlaceId *array, int arrSize, PlaceId find);
// Extract location for a specified move
static PlaceId extractLocation(DraculaView dv, PlaceId move, Round round);
// Checks if there are DOUBLE_BACK moves in trail
static bool doublebackInTrail(PlaceId *trail, int trailSize);


////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

DraculaView DvNew(char *pastPlays, Message messages[])
{
	DraculaView new = malloc(sizeof(*new));
	if (new == NULL) {
		fprintf(stderr, "Couldn't allocate DraculaView\n");
		exit(EXIT_FAILURE);
	}
	new->gv = GvNew(pastPlays, messages);
	new->pastPlays = strdup(pastPlays);
	return new;
}

void DvFree (DraculaView dv)
{
	GvFree(dv->gv);
	free(dv->pastPlays);
	free(dv);
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
    // Get reachable locations from current location
    PlaceId from = DvGetPlayerLocation(dv, PLAYER_DRACULA);
    int numReachable = 0;
    PlaceId *reachable = GvGetReachable(dv->gv, PLAYER_DRACULA, DvGetRound(dv),
                                        from, &numReachable);
    
    // Create valid moves array
    PlaceId *validMoves = malloc((numReachable + 6) * sizeof(PlaceId));
    assert(validMoves);
    *numReturnedMoves = 0;

    // Find trail
    int numMoves = 0; bool canFreeM = false;
    PlaceId *trailMoves = GvGetLastMoves(dv->gv, PLAYER_DRACULA, 5, &numMoves, &canFreeM);
    int numLocs = 0; bool canFreeL = false;
    PlaceId *trailLocs = GvGetLastLocations(dv->gv, PLAYER_DRACULA, 5, &numLocs, &canFreeL);

    // Filter reachable locations
    for (int i = 0; i < numReachable; i++) {
        PlaceId city = reachable[i];
        // If city not in trailLocations
        if (!inArray(trailLocs, numLocs, city)) {
            // Add to validMoves
            validMoves[*numReturnedMoves] = city;
            (*numReturnedMoves)++;
        // City already in trailLocations
        } else {
            // Check if special move already used //
            // Check HIDE
			if (!inArray(trailMoves, numMoves, HIDE)) {
				// Add to validMoves
				validMoves[*numReturnedMoves] = HIDE;
            	(*numReturnedMoves)++;
			}
			// Check DOUBLE_BACK
			if (doublebackInTrail(trailMoves, numMoves)) continue;
			for (PlaceId move = DOUBLE_BACK_1; move <= DOUBLE_BACK_5; move++) {
                // Invalid move, or already in array
                if (DvGetRound(dv) - (move - DOUBLE_BACK_1) <= 0 ||
					inArray(trailMoves, numMoves, move))
					continue;
                // Add to validMoves
				validMoves[*numReturnedMoves] = move;
				(*numReturnedMoves)++;
            }
        }
    }
	
	// Teleport case
    if (*numReturnedMoves == 0) {
        validMoves[*numReturnedMoves] = TELEPORT;
        (*numReturnedMoves)++;
    }

	free(reachable);
	free(trailMoves);
    free(trailLocs);
	return validMoves;
}

PlaceId *DvWhereCanIGo(DraculaView dv, int *numReturnedLocs)
{
	return DvWhereCanIGoByType(dv, true, true, numReturnedLocs);
}

PlaceId *DvWhereCanIGoByType(DraculaView dv, bool road, bool boat,
                             int *numReturnedLocs)
{
	// Get valid moves
    int numValidMoves = 0;
    PlaceId *validMoves = DvGetValidMoves(dv, &numValidMoves);

    // Create valid locations array
    PlaceId *validLocations = malloc(numValidMoves * sizeof(PlaceId));
    assert(validLocations);
    *numReturnedLocs = 0;

    // Get specified connections
    int numConnections = 0;
    PlaceId from = DvGetPlayerLocation(dv, PLAYER_DRACULA);
    PlaceId *connections = GvGetReachableByType(dv->gv, PLAYER_DRACULA, DvGetRound(dv),
                                                from, road, false, boat, &numConnections);
    
    // For each connection
    for (int i = 0; i < numConnections; i++) {
        // For each move
        for (int j = 0; j < numValidMoves; j++) {
            // Location for move
            PlaceId location = extractLocation(dv, validMoves[j], DvGetRound(dv));
            // If the connection is a valid location (move) and unique
            if (connections[i] == location &&
                !inArray(validLocations, *numReturnedLocs, location)) {
                // Add to validLocations
                validLocations[*numReturnedLocs] = location;
                (*numReturnedLocs)++;
            }

        }
    }

	free(validMoves);
	free(connections);
	return validLocations;
}

PlaceId *DvWhereCanTheyGo(DraculaView dv, Player player,
                          int *numReturnedLocs)
{
	return DvWhereCanTheyGoByType(dv, player, true, true, true, numReturnedLocs);
}

PlaceId *DvWhereCanTheyGoByType(DraculaView dv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs)
{
	if (player == PLAYER_DRACULA)
		return DvWhereCanIGoByType(dv, road, boat, numReturnedLocs);
	else
		return GvGetReachableByType(dv->gv, player, DvGetRound(dv),
						 			DvGetPlayerLocation(dv, player),
									road, rail, boat, numReturnedLocs);
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// Checks if a PlaceId is already in the array
static bool inArray(PlaceId *array, int arrSize, PlaceId find)
{
    for (int i = 0; i < arrSize; i++)
        if (array[i] == find) return true;
    return false;
}


// Extract location for a specified move
static PlaceId extractLocation(DraculaView dv, PlaceId move, Round round)
{
	bool found = false;
	int playerIndex = round * 40 + 32;
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
        code[0] = dv->pastPlays[playerIndex + 1];
		code[1] = dv->pastPlays[playerIndex + 2];
		code[2] = '\0';
		move = placeAbbrevToId(code);
	}

	if (!found) return UNKNOWN_PLACE;
	else if (move == TELEPORT) return CASTLE_DRACULA;
	else return move;
}

// Checks if there are DOUBLE_BACK moves in trail
static bool doublebackInTrail(PlaceId *trail, int trailSize)
{
	for (int i = 0; i < trailSize; i++) {
		PlaceId p = trail[i];
		if (p == DOUBLE_BACK_1 || p == DOUBLE_BACK_2 || p == DOUBLE_BACK_3 ||
			p == DOUBLE_BACK_4 || p == DOUBLE_BACK_5) 
			return true;
	}
	return false;
}
