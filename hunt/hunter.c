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

    // Prefer more recent trap rounds
    if (trapRound > roundRevealed) {
        lastDraculaLocation = lastTrapLocation;
        roundRevealed = trapRound;
    }

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
    if (placeIsReal(lastDraculaLocation) && round - roundRevealed <= 7) {
        int pathLengthD = 0;
        PlaceId *pathD = HvGetShortestPathTo(hv, player, lastDraculaLocation, &pathLengthD);
        if (pathLengthD > 1) {
            // Move towards Dracula
            registerBestPlay((char *)placeIdToAbbrev(pathD[0]), "dracula");
            free(pathD);
            return;
        }
        free(pathD);
    }
    

    // BFS to trap location if far away
    if (placeIsReal(lastTrapLocation) && round - roundRevealed <= 7) {
        int pathLengthT = 0;
        PlaceId *pathT = HvGetShortestPathTo(hv, player, lastTrapLocation, &pathLengthT);
        if (pathLengthT > 1) {
            // Move towards trap location
            registerBestPlay((char *)placeIdToAbbrev(pathT[0]), "trap");
            free(pathT);
            return;
        }
        free(pathT);
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
    if (round >= 6 && ((!placeIsReal(lastDraculaLocation) && round - roundRevealed >= 6) || (nextMaturity - round) == 6)) {
        registerBestPlay((char *)placeIdToAbbrev(move), "zz");
        return;
    }

    // Default movement
    PlaceId *generalReachable = HvWhereCanTheyGo(hv, player, &numReturnedLocs);
    int index = (round * (player + 1)) % numReturnedLocs;
    if (generalReachable[index] == move) index = (index + 1) % numReturnedLocs;
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
