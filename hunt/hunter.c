////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// hunter.c: your "Fury of Dracula" hunter AI.
//
// 2014-07-01   v1.0    Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01   v1.1    Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31   v2.0    Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10   v3.0    Team Dracula <cs2521@cse.unsw.edu.au>
//
// This was created by JAWA on 10/08/2020.
//
////////////////////////////////////////////////////////////////////////

#include "Game.h"
#include "hunter.h"
#include "HunterView.h"
#include <stdio.h>

////////////////////////////////////////////////////////////////////////
// Function Prototypes

// Registers a starting location for a player
static PlaceId startingLocation(HunterView hv);
// Returns number of hunters at location
static int numHuntersAtLocation(HunterView hv, PlaceId location);
// Return number of hunters who can reach a location on their next turn
static int numHuntersReachable(HunterView hv, PlaceId location, Player hunter);
// Near trap
static bool nearTrap(HunterView hv, PlaceId currentLocation, PlaceId lastTrapLocation);

static bool possibleDraculaLocation(HunterView hv, PlaceId location);

void decideHunterMove(HunterView hv)
{
    // Extract player/game state information
    Round round = HvGetRound(hv); Player player = HvGetPlayer(hv);
    PlaceId move = HvGetPlayerLocation(hv, player);
    int health = HvGetHealth(hv, player);
    // Extract Dracula information
    Round roundRevealed = -1;
    PlaceId lastDraculaLocation = HvGetLastKnownDraculaLocation(hv, &roundRevealed);
    int draculaHealth = HvGetHealth(hv, PLAYER_DRACULA);
    // Extract vampire location
    PlaceId vampireLocation = HvGetVampireLocation(hv);
    // Extract trap location
    Round trapRound = -1;
    PlaceId lastTrapLocation = recentTrapEncounter(hv, &trapRound);

    // Prefer more recent trap rounds
    if (trapRound > roundRevealed) {
        lastDraculaLocation = lastTrapLocation;
        roundRevealed = trapRound;
    }

    // Register starting location
    if (round == 0) {
        PlaceId loc = startingLocation(hv);
        registerBestPlay((char *)placeIdToAbbrev(loc), "JAWA - we don't go by the script");
        return;
    }

    // Get reachable locations from current location
    int numReturnedLocs = 0;
    PlaceId *reachable = HvWhereCanIGo(hv, &numReturnedLocs);

    // Check whether Dracula can be encountered on the next turn
    for (int i = 0; i < numReturnedLocs; i++) {
        PlaceId city = reachable[i];
        if (city == move) continue;
        if (city == lastDraculaLocation && round - roundRevealed <= 2 && placeIsReal(lastDraculaLocation)) {
            registerBestPlay((char *)placeIdToAbbrev(city), "JAWA - we don't go by the script");
            free(reachable);
            return;
        }
    }

    // BFS to Dracula location if far away
    if (placeIsReal(lastDraculaLocation)) {
        int pathLengthD = 0;
        PlaceId *pathD = HvGetShortestPathTo(hv, player, lastDraculaLocation, &pathLengthD);
        PlaceId shortestPathStep = pathD[0];
        free(pathD);
        if (pathLengthD > 2 && round - roundRevealed <= 9) {
            // Move towards Dracula
            registerBestPlay((char *)placeIdToAbbrev(shortestPathStep), "JAWA - we don't go by the script");
            return;
        } else if (pathLengthD > 1 && round - roundRevealed <= 8) {
            // Move towards Dracula
            registerBestPlay((char *)placeIdToAbbrev(shortestPathStep), "JAWA - we don't go by the script");
            return;
        }
    }

    // BFS to trap location if far away
    if (placeIsReal(lastTrapLocation)) {
        int pathLengthT = 0;
        PlaceId *pathT = HvGetShortestPathTo(hv, player, lastTrapLocation, &pathLengthT);
        PlaceId shortestPathStep = pathT[0];
        free(pathT);
        if (pathLengthT > 2 && round - roundRevealed <= 9) {
            // Move towards trap location
            registerBestPlay((char *)placeIdToAbbrev(shortestPathStep), "JAWA - we don't go by the script");
            return;
        } else if (pathLengthT > 1 && round - roundRevealed <= 8) {
            // Move towards trap location
            registerBestPlay((char *)placeIdToAbbrev(shortestPathStep), "JAWA - we don't go by the script");
            return;
        }
    }

    // Vampire
    if (placeIsReal(vampireLocation)) {
        // Calculate shortest path to location
        Player closestPlayer = -1; int minPathLength = 100000;
        PlaceId shortestPathStep = NOWHERE;
        for (Player hunter = PLAYER_LORD_GODALMING; hunter < PLAYER_DRACULA; hunter++) {
            // Calculate path
            int pathLength = 0;
            PlaceId *path = HvGetShortestPathTo(hv, hunter, vampireLocation, &pathLength);
            if (pathLength < minPathLength) {
                closestPlayer = hunter;
                minPathLength = pathLength;
                shortestPathStep = path[0];
            }
            free(path);
        }
        // Noone already there
        if (placeIsReal(shortestPathStep) && player == closestPlayer && minPathLength != 0) {
            registerBestPlay((char *)placeIdToAbbrev(shortestPathStep), "JAWA - we don't go by the script");
            return;
        }
    }

    // Collaborative research
    if (round >= 6 && round - roundRevealed >= 15) {
        registerBestPlay((char *)placeIdToAbbrev(move), "JAWA - we don't go by the script");
        return;
    }

    // Rest
    if (health < 3) {
        registerBestPlay((char *)placeIdToAbbrev(move), "JAWA - we don't go by the script");
        return;
    }

    // Guard CASTLE_DRACULA
    Player closestPlayer = -1; int minPathLength = 100000;
    PlaceId shortestPathStep = NOWHERE;
    for (Player hunter = PLAYER_LORD_GODALMING; hunter < PLAYER_DRACULA; hunter++) {
        // Calculate path
        int pathLength = 0;
        PlaceId *path = HvGetShortestPathTo(hv, hunter, CASTLE_DRACULA, &pathLength);
        if (pathLength < minPathLength) {
            closestPlayer = hunter;
            minPathLength = pathLength;
            shortestPathStep = path[0];
        }
        free(path);
    }
    // Move towards
    if (draculaHealth <= 20 && player == closestPlayer && minPathLength > 3) {
        registerBestPlay((char *)placeIdToAbbrev(shortestPathStep), "JAWA - we don't go by the script");
        return;
    // Guard 
    } else if (draculaHealth <= 15 && player == closestPlayer && minPathLength != 0 && round % 5 == 0) {
        registerBestPlay((char *)placeIdToAbbrev(shortestPathStep), "JAWA - we don't go by the script");
        return;
    } else if (draculaHealth <= 8 && player == closestPlayer && minPathLength != 0 && round % 3 == 0) {
        registerBestPlay((char *)placeIdToAbbrev(shortestPathStep), "JAWA - we don't go by the script");
        return;
    }

    // Default movement
    PlaceId *generalReachable = HvWhereCanIGo(hv, &numReturnedLocs);
    // If Dracula at sea
    if (placeIsSea(HvGetPlayerLocation(hv, PLAYER_DRACULA))) {
        // Try to move to sea
        PlaceId *generalReachable = HvWhereCanIGoByType(hv, false, false, true, &numReturnedLocs);
        if (numReturnedLocs == 0) {
            // Revert to default movement
            free(generalReachable);
            generalReachable = HvWhereCanIGo(hv, &numReturnedLocs);
        }
    }

    // Find minimum number 
    int moveWeight[NUM_REAL_PLACES] = {0}; int minimumWeight = 100000;
    for (int i = 0; i < numReturnedLocs; i++) {
        PlaceId option = generalReachable[i];
        moveWeight[i] = 5 * numHuntersAtLocation(hv, option);
        moveWeight[i] += numHuntersReachable(hv, option, player);
        moveWeight[i] += 2 * visited(hv, option);
        moveWeight[i] += inTrail(hv, option);
        moveWeight[i] -= possibleDraculaLocation(hv, option);
        if (round - trapRound < 2) moveWeight[i] -= 2 * nearTrap(hv, move, lastTrapLocation);
        moveWeight[i] -= 1 * nearTrap(hv, move, lastTrapLocation);
        if (moveWeight[i] < minimumWeight) minimumWeight = moveWeight[i];
    }

    // Select minimum weights
    int arrSize = 0;
    int minimumIndices[NUM_REAL_PLACES] = {0};
    for (int i = 0; i < numReturnedLocs; i++) {
        if (moveWeight[i] >= minimumWeight && moveWeight[i] <= minimumWeight + 4) {
            // Append to new array of minimum weights
            minimumIndices[arrSize] = i;
            arrSize++;
        }
    }
    int indexOfMin = rand() % arrSize;
    // Prevent idle
    if (arrSize == 0) {
        int index = rand() % numReturnedLocs;
        if (generalReachable[index] == move) index = (index + 1) % numReturnedLocs;
        registerBestPlay((char *)placeIdToAbbrev(generalReachable[index]), "JAWA - we don't go by the script");
        free(generalReachable);
        return;
    }
    int index = minimumIndices[indexOfMin];
    if (generalReachable[index] == move) indexOfMin = (indexOfMin + 1) % arrSize;
    index = minimumIndices[indexOfMin];
    if (generalReachable[index] == move) index = rand() % numReturnedLocs;
    registerBestPlay((char *)placeIdToAbbrev(generalReachable[index]), "JAWA - we don't go by the script");
    free(generalReachable);
    return;
}

static bool possibleDraculaLocation(HunterView hv, PlaceId location) {
    int numReturnedLocs = 0;
    PlaceId *possible = HvWhereCanTheyGo(hv, PLAYER_DRACULA, &numReturnedLocs);
    for (int i = 0; i < numReturnedLocs; i++) {
        if (possible[i] == location) {
            free(possible);
            return true;
        }
    }
    free(possible);
    return false;
}



// Registers a starting location for a player
static PlaceId startingLocation(HunterView hv)
{
    Player player = HvGetPlayer(hv);
    switch (player) {
        case PLAYER_LORD_GODALMING:
            return EDINBURGH;
        case PLAYER_DR_SEWARD:
            return LISBON;
        case PLAYER_VAN_HELSING:
            return VENICE;
        case PLAYER_MINA_HARKER:
            return SOFIA;
        default:
            return NOWHERE;
    }
}

// Returns number of hunters at location
static int numHuntersAtLocation(HunterView hv, PlaceId location)
{
    int numHunters = 0;
    for (Player player = PLAYER_LORD_GODALMING; player < PLAYER_DRACULA; player++)
        if (HvGetPlayerLocation(hv, player) == location) numHunters++;
    return numHunters;
}

// Return number of hunters who can reach a location on their next turn
static int numHuntersReachable(HunterView hv, PlaceId location, Player hunter)
{
    int numHunters = 0;
    for (Player player = PLAYER_LORD_GODALMING; player < PLAYER_DRACULA; player++) {
        // Where can the other hunters go
        if (player == hunter) continue;
        int numReturnedLocs = 0;
        PlaceId *playerReachable = HvWhereCanTheyGoByType(hv, player, true, false, true, &numReturnedLocs);
        for (int i = 0; i < numReturnedLocs; i++) {
            if (playerReachable[i] == location) {
                numHunters++;
                break;
            }
        }
        free(playerReachable);
    }
    return numHunters;
}

// Near trap
static bool nearTrap(HunterView hv, PlaceId currentLocation, PlaceId lastTrapLocation) {
    if (lastTrapLocation == NOWHERE) return false;
    int numReturnedLocs = 0;
    PlaceId *reachable = HvWhereCanIGo(hv, &numReturnedLocs);
    for (int i = 0; i < numReturnedLocs; i++) {
        if (reachable[i] == lastTrapLocation) {
            free(reachable);
            return true;
        }
    }
    free(reachable);
    return false;
}
