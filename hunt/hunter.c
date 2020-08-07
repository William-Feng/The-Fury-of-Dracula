////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// hunter.c: your "Fury of Dracula" hunter AI.
//
// 2014-07-01   v1.0    Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01   v1.1    Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31   v2.0    Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10   v3.0    Team Dracula <cs2521@cse.unsw.edu.au>
//
// This was created by JAWA on 07/08/2020.
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


void decideHunterMove(HunterView hv)
{
    // Extract player/game state information
    Round round = HvGetRound(hv); Player player = HvGetPlayer(hv);
    PlaceId move = HvGetPlayerLocation(hv, player);
    int health = HvGetHealth(hv, player);
    // Extract Dracula information
    int roundRevealed = -1;
    PlaceId lastDraculaLocation = HvGetLastKnownDraculaLocation(hv, &roundRevealed);
    int draculaHealth = HvGetHealth(hv, PLAYER_DRACULA);
    // Extract vampire location
    Round nextMaturity = (round / 13) + 6;
    PlaceId vampireLocation = HvGetVampireLocation(hv);
    // Extract trap location
    Round trapRound = -1;
    PlaceId lastTrapLocation = recentTrapEncounter(hv, &trapRound);

    // Register starting location
    if (round == 0) {
        PlaceId loc = startingLocation(hv);
        registerBestPlay((char *)placeIdToAbbrev(loc), "start");
        return;
    }

    // Get reachable locations from current location
    int numReturnedLocs = 0;
    PlaceId *reachable = HvWhereCanIGo(hv, &numReturnedLocs);

    // Check whether Dracula can be encountered on the next turn
    for (int i = 0; i < numReturnedLocs; i++) {
        PlaceId city = reachable[i];
        if (city == lastDraculaLocation && round - roundRevealed == 1 && placeIsReal(lastDraculaLocation)) {
            registerBestPlay((char *)placeIdToAbbrev(city), "kill");
            free(reachable);
            return;
        }
    }
    free(reachable);

    // BFS to Dracula location if far away
    int pathLengthD = 0;
    PlaceId *pathD = HvGetShortestPathTo(hv, player, lastDraculaLocation, &pathLengthD);
    if (placeIsReal(lastDraculaLocation) && pathLengthD > 1 && round - roundRevealed <= 7) {
        // Move towards Dracula
        registerBestPlay((char *)placeIdToAbbrev(pathD[0]), "dracula");
        free(pathD);
        return;
    }
    free(pathD);

    // BFS to trap location if far away
    int pathLengthT = 0;
    PlaceId *pathT = HvGetShortestPathTo(hv, player, lastTrapLocation, &pathLengthT);
    if (placeIsReal(lastTrapLocation) && pathLengthT > 1 && round - roundRevealed <= 7) {
        // Move towards trap location
        registerBestPlay((char *)placeIdToAbbrev(pathT[0]), "trap");
        free(pathT);
        return;
    }
    free(pathT);

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
            registerBestPlay((char *)placeIdToAbbrev(shortestPathStep), "vampire");
            return;
        }
    }

    // Rest
    if (health <= 3) {
        registerBestPlay((char *)placeIdToAbbrev(move), "rest");
        return;
    }

    // Guard CD
    if (draculaHealth <= 20 && round % 3 == 0) {
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
        // Noone already there
        if (placeIsReal(shortestPathStep) && player == closestPlayer && minPathLength != 0) {
            registerBestPlay((char *)placeIdToAbbrev(shortestPathStep), "cd");
            return;
        }
    }

    // Collaborative research - others may not also do in the same round
    if (round >= 6 && (!placeIsReal(lastDraculaLocation) || (nextMaturity - round) == 6)) {
        registerBestPlay((char *)placeIdToAbbrev(move), "zz");
        return;
    }

    // Default movement
    PlaceId *generalReachable = HvWhereCanIGo(hv, &numReturnedLocs);
    // Find minimum number 
    int moveWeight[NUM_REAL_PLACES] = {0}; int minimumWeight = 100000;
    for (int i = 0; i < numReturnedLocs; i++) {
        PlaceId option = generalReachable[i];
        moveWeight[i] = 2 * numHuntersAtLocation(hv, option);
        moveWeight[i] += numHuntersReachable(hv, option, player);
        if (moveWeight[i] < minimumWeight) minimumWeight = moveWeight[i];
    }

    // Select minimum weights
    int arrSize = 0;
    int minimumIndices[NUM_REAL_PLACES] = {0};
    for (int i = 0; i < numReturnedLocs; i++) {
        if (moveWeight[i] >= minimumWeight || moveWeight[i] <= minimumWeight + 2) {
            // Append to new array of minimum weights
            minimumIndices[arrSize] = i;
            arrSize++;
        }
    }
    int indexOfMin = rand() % arrSize;
    // Prevent idle
    if (arrSize == 0) {
        registerBestPlay((char *)placeIdToAbbrev(rand() % numReturnedLocs), "no min");
        free(generalReachable);
        return;
    }
    int index = minimumIndices[indexOfMin];
    if (generalReachable[index] == move) indexOfMin = (indexOfMin + 1) % arrSize;
    index = minimumIndices[indexOfMin];
    if (generalReachable[index] == move) index = rand() % numReturnedLocs;
    registerBestPlay((char *)placeIdToAbbrev(generalReachable[index]), "min");
    free(generalReachable);
    return;
}


// Registers a starting location for a player
static PlaceId startingLocation(HunterView hv)
{
    Player player = HvGetPlayer(hv);
    switch (player) {
        case PLAYER_LORD_GODALMING:
            return ENGLISH_CHANNEL;
        case PLAYER_DR_SEWARD:
            return BARCELONA;
        case PLAYER_VAN_HELSING:
            return HAMBURG;
        case PLAYER_MINA_HARKER:
            return GALATZ;
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




