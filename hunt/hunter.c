////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// hunter.c: your "Fury of Dracula" hunter AI.
//
// 2014-07-01   v1.0    Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01   v1.1    Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31   v2.0    Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10   v3.0    Team Dracula <cs2521@cse.unsw.edu.au>
//
// This was created by JAWA on 08/08/2020.
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
        if (city == lastDraculaLocation && round - roundRevealed == 1 && placeIsReal(lastDraculaLocation)) {
            registerBestPlay((char *)placeIdToAbbrev(city), "JAWA - we don't go by the script");
            free(reachable);
            return;
        }
    }
    free(reachable);

    // BFS to Dracula location if far away
    if (placeIsReal(lastDraculaLocation)) {
        int pathLengthD = 0;
        PlaceId *pathD = HvGetShortestPathTo(hv, player, lastDraculaLocation, &pathLengthD);
        if (pathLengthD > 3 && round - roundRevealed <= 10) {
            // Move towards Dracula
            registerBestPlay((char *)placeIdToAbbrev(pathD[0]), "JAWA - we don't go by the script");
            free(pathD);
            return;
        } else if (pathLengthD > 1 && round - roundRevealed <= 7) {
            // Move towards Dracula
            registerBestPlay((char *)placeIdToAbbrev(pathD[0]), "JAWA - we don't go by the script");
            free(pathD);
            return;
        }
        free(pathD);
    }

    // BFS to trap location if far away
    if (placeIsReal(lastTrapLocation)) {
        int pathLengthT = 0;
        PlaceId *pathT = HvGetShortestPathTo(hv, player, lastTrapLocation, &pathLengthT);
        if (pathLengthT > 3 && round - roundRevealed <= 10) {
            // Move towards trap location
            registerBestPlay((char *)placeIdToAbbrev(pathT[0]), "JAWA - we don't go by the script");
            free(pathT);
            return;
        } else if (pathLengthT > 1 && round - roundRevealed <= 7) {
            // Move towards trap location
            registerBestPlay((char *)placeIdToAbbrev(pathT[0]), "JAWA - we don't go by the script");
            free(pathT);
            return;
        }
        free(pathT);
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
    if (draculaHealth <= 25 && player == closestPlayer && minPathLength > 3) {
        registerBestPlay((char *)placeIdToAbbrev(shortestPathStep), "JAWA - we don't go by the script");
        return;
    // Noone already there
    } else if (draculaHealth <= 20 && player == closestPlayer && minPathLength != 0 && round % 3 == 0) {
        registerBestPlay((char *)placeIdToAbbrev(shortestPathStep), "JAWA - we don't go by the script");
        return;
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

    // Rest
    if (health < 4) {
        registerBestPlay((char *)placeIdToAbbrev(move), "JAWA - we don't go by the script");
        return;
    }

    // Collaborative research
    if (round >= 6 && round - roundRevealed >= 6) {
        registerBestPlay((char *)placeIdToAbbrev(move), "JAWA - we don't go by the script");
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
        moveWeight[i] = 2 * numHuntersAtLocation(hv, option);
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
        registerBestPlay((char *)placeIdToAbbrev(rand() % numReturnedLocs), "JAWA - we don't go by the script");
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
