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

// Returns a starting location for a player
static PlaceId startingLocation(HunterView hv);
// Returns number of hunters at location
static int numHuntersAtLocation(HunterView hv, PlaceId location);


void decideHunterMove(HunterView hv)
{
    // Extract information
    Round round = HvGetRound(hv);
    Player player = HvGetPlayer(hv);
    PlaceId move = HvGetPlayerLocation(hv, player);
    int health = HvGetHealth(hv, player);
    int roundRevealed = -1;
    PlaceId lastDraculaLocation = HvGetLastKnownDraculaLocation(hv, &roundRevealed);
    int draculaHealth = HvGetHealth(hv, PLAYER_DRACULA);
    Round previousVampireSpawn = round / 13;
    Round nextMaturity = previousVampireSpawn + 6;

    // Trap information - direction may be incorrect
    Round trapRound = -1;
    PlaceId lastTrapLocation = recentTrapEncounter(hv, &trapRound);
    if (trapRound > roundRevealed) {
        lastDraculaLocation = lastTrapLocation;
        roundRevealed = trapRound;
    }

    // Starting location
    if (round == 0) {
        move = startingLocation(hv);
        registerBestPlay((char *)placeIdToAbbrev(move), "JAWA - we don't go by the script");
        return;
    }
    
    // Get reachable locations
    int numReturnedLocs = 0;
    PlaceId *reachable = HvWhereCanIGo(hv, &numReturnedLocs);

    for (int i = 0; i < numReturnedLocs; i++) {
        PlaceId city = reachable[i];
        // Check if Dracula can be definitely encountered
        if (city == lastDraculaLocation && round - roundRevealed <= 1 && placeIsReal(lastDraculaLocation)) {
            registerBestPlay((char *)placeIdToAbbrev(city), "JAWA - we don't go by the script");
            free(reachable);
            return;
        }
    }
    free(reachable);

    // Vampire about to mature
    PlaceId vampireLocation = HvGetVampireLocation(hv);
    if (placeIsReal(vampireLocation)) {
        // Calculate shortest path length
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
        // Urgent - about to mature
        if (shortestPathStep != NOWHERE && player == closestPlayer &&
            (round + minPathLength) <= nextMaturity && (nextMaturity - round) <= 2) {
            registerBestPlay((char *)placeIdToAbbrev(shortestPathStep), "JAWA - we don't go by the script");
            return;
        }
    }

    // Rest
    if (health <= 3) {
        registerBestPlay((char *)placeIdToAbbrev(move), "JAWA - we don't go by the script");
        return;
    }

    // General Vampire
    if (placeIsReal(vampireLocation)) {
        // Calculate shortest path length
        int closestPlayer = -1; int minPathLength = 100000;
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
        if (shortestPathStep != NOWHERE && player == closestPlayer &&
            (round + minPathLength) <= nextMaturity && (nextMaturity - round) <= 4) {
            registerBestPlay((char *)placeIdToAbbrev(shortestPathStep), "JAWA - we don't go by the script");
            return;
        }
    }

    // Send nearest player towards CD if Dracula's health is low and noone is there
    if (draculaHealth <= 20 && round % 3 == 0) {
        Player closestPlayer = 0; int minPathLength = 100000;
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
        if (player == closestPlayer && minPathLength != 0 && numHuntersAtLocation(hv, CASTLE_DRACULA) == 0) {
            registerBestPlay((char *)placeIdToAbbrev(shortestPathStep), "JAWA - we don't go by the script");
            return;
        }
    }

    // Try and overtake, not just path to destination.
    // Dracula BFS
    if (move != lastDraculaLocation && placeIsReal(lastDraculaLocation) && round - roundRevealed <= 6) {
        int pathLength = 0;
        PlaceId *path = HvGetShortestPathTo(hv, player, lastDraculaLocation, &pathLength);
        PlaceId shortestPathStep = path[0];
        free(path);
        if (pathLength != 0) {
            registerBestPlay((char *)placeIdToAbbrev(shortestPathStep), (char * )placeIdToAbbrev(lastDraculaLocation));
            return;
        }
    }

    // BFS to trap encounters
    if (lastTrapLocation != NOWHERE && placeIsReal(lastTrapLocation) && round - trapRound <= 6) {
        int pathLength = 0;
        PlaceId *path = HvGetShortestPathTo(hv, player, lastTrapLocation, &pathLength);
        PlaceId shortestPathStep = path[0];
        free(path);
        if (pathLength != 0) {
            registerBestPlay((char *)placeIdToAbbrev(shortestPathStep), (char * )placeIdToAbbrev(lastTrapLocation));
            return;
        }
    }
    
    // Collaborative research
    if (round >= 6 && (!placeIsReal(lastDraculaLocation) || (nextMaturity - round) == 6)) {
        registerBestPlay((char *)placeIdToAbbrev(move), "JAWA - we don't go by the script");
        return;
    }

    // Default movement - surround location
    PlaceId *generalReachable = HvWhereCanTheyGo(hv, player, &numReturnedLocs);
    int index = (round * (player + 1)) % numReturnedLocs;
    if (generalReachable[index] == move) index = (index + 1) % numReturnedLocs;
    registerBestPlay((char *)placeIdToAbbrev(generalReachable[index]), "JAWA - we don't go by the script");
    free(generalReachable);
    return;
}


// Returns a starting location for a player
static PlaceId startingLocation(HunterView hv) {
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
static int numHuntersAtLocation(HunterView hv, PlaceId location) {
    int numHunters = 0;
    for (Player player = PLAYER_LORD_GODALMING; player < PLAYER_DRACULA; player++)
        if (HvGetPlayerLocation(hv, player) == location) numHunters++;
    return numHunters;
}
