////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// testDraculaView.c: test the DraculaView ADT
//
// As supplied, these are very simple tests.  You should write more!
// Don't forget to be rigorous and thorough while writing tests.
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-02	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
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
#include "Places.h"
#include "testUtils.h"

int main(void)
{
	{///////////////////////////////////////////////////////////////////
	
		printf("Test for basic functions, "
			   "just before Dracula's first move\n");

		char *trail =
			"GST.... SAO.... HZU.... MBB....";
		
		Message messages[] = {
			"Hello", "Goodbye", "Stuff", "..."
		};
		
		DraculaView dv = DvNew(trail, messages);

		assert(DvGetRound(dv) == 0);
		assert(DvGetScore(dv) == GAME_START_SCORE);
		assert(DvGetHealth(dv, PLAYER_DRACULA) == GAME_START_BLOOD_POINTS);
		assert(DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) == STRASBOURG);
		assert(DvGetPlayerLocation(dv, PLAYER_DR_SEWARD) == ATLANTIC_OCEAN);
		assert(DvGetPlayerLocation(dv, PLAYER_VAN_HELSING) == ZURICH);
		assert(DvGetPlayerLocation(dv, PLAYER_MINA_HARKER) == BAY_OF_BISCAY);
		assert(DvGetPlayerLocation(dv, PLAYER_DRACULA) == NOWHERE);
		assert(DvGetVampireLocation(dv) == NOWHERE);
		int numTraps = -1;
		PlaceId *traps = DvGetTrapLocations(dv, &numTraps);
		assert(numTraps == 0);
		free(traps);

		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
	
		printf("Test for encountering Dracula\n");

		char *trail =
			"GST.... SAO.... HCD.... MAO.... DGE.V.. "
			"GGEVD.. SAO.... HCD.... MAO....";
		
		Message messages[] = {
			"Hello", "Goodbye", "Stuff", "...", "Mwahahah",
			"Aha!", "", "", ""
		};
		
		DraculaView dv = DvNew(trail, messages);

		assert(DvGetRound(dv) == 1);
		assert(DvGetScore(dv) == GAME_START_SCORE - SCORE_LOSS_DRACULA_TURN);
		assert(DvGetHealth(dv, PLAYER_LORD_GODALMING) == 5);
		assert(DvGetHealth(dv, PLAYER_DRACULA) == 30);
		assert(DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) == GENEVA);
		assert(DvGetPlayerLocation(dv, PLAYER_DRACULA) == GENEVA);
		assert(DvGetVampireLocation(dv) == NOWHERE);

		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
	
		printf("Test for Dracula leaving minions 1\n");

		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DED.V.. "
			"GST.... SST.... HST.... MST.... DMNT... "
			"GST.... SST.... HST.... MST.... DLOT... "
			"GST.... SST.... HST.... MST.... DHIT... "
			"GST.... SST.... HST.... MST....";
		
		Message messages[24] = {};
		DraculaView dv = DvNew(trail, messages);

		assert(DvGetRound(dv) == 4);
		assert(DvGetVampireLocation(dv) == EDINBURGH);
		int numTraps = -1;
		PlaceId *traps = DvGetTrapLocations(dv, &numTraps);
		assert(numTraps == 3);
		sortPlaces(traps, numTraps);
		assert(traps[0] == LONDON);
		assert(traps[1] == LONDON);
		assert(traps[2] == MANCHESTER);
		free(traps);
		
		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
	
		printf("Test for Dracula's valid moves 1\n");
		
		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DCD.V.. "
			"GGE.... SGE.... HGE.... MGE....";
		
		Message messages[9] = {};
		DraculaView dv = DvNew(trail, messages);
		
		int numMoves = -1;
		PlaceId *moves = DvGetValidMoves(dv, &numMoves);
		assert(numMoves == 4);
		sortPlaces(moves, numMoves);
		assert(moves[0] == GALATZ);
		assert(moves[1] == KLAUSENBURG);
		assert(moves[2] == HIDE);
		assert(moves[3] == DOUBLE_BACK_1);
		free(moves);
		
		printf("Test passed!\n");
		DvFree(dv);
	}


    {///////////////////////////////////////////////////////////////////
        // Dracula is at sea
        printf("Test for Dracula's Valid Moves 2\n");
        
        char *trail =
            "GKL.... SKL.... HGA.... MGA.... DCD.V.. "
            "GKL.... SKL.... HGA.... MGA.... DKLT... "
            "GKL.... SKL.... HGA.... MGA.... DCDT... "
            "GKL.... SKL.... HGA.... MGA.... DGAT... "
            "GKL.... SKL.... HGA.... MGA.... DCNT... "
            "GKL.... SKL.... HGA.... MGA.... DBST...";
        
        Message messages[40] = {};
        DraculaView dv = DvNew(trail, messages);

        int numMoves = -1;
        PlaceId *moves = DvGetValidMoves(dv, &numMoves);
        assert(numMoves == 4);
        sortPlaces(moves, numMoves);
        assert(moves[0] == IONIAN_SEA);
        assert(moves[1] == VARNA); 
        assert(moves[2] == DOUBLE_BACK_1); 
		assert(moves[3] == DOUBLE_BACK_2);

        free(moves);
        
        printf("Test passed!\n");
        DvFree(dv);
    }

	{ ////////////////////////////////////////////////////////////////////////
        printf("Test for Dracula's Valid Moves 3\n"); // ensure cannot double back to 
        // a location that is not adjacent. 
        char *trail = 
            "GSW.... SLS.... HMR.... MHA.... DSJ.V.. "
            "GLO.... SAL.... HCO.... MBR.... DBET... "
            "GED.... SBO.... HLI.... MPR.... DKLT... "
            "GLV.... SNA.... HNU.... MBD.... DCDT... "
            "GIR.... SPA.... HPR.... MKLT... DHIT... "
            "GAO.... SST.... HSZ.... MCDTTD.";
            
        Message messages[24] = {};
        DraculaView dv = DvNew(trail, messages);

        
        int numMoves = -1;
        PlaceId *moves = DvGetValidMoves(dv, &numMoves);
        assert(numMoves == 4);
        sortPlaces(moves, numMoves);
		assert(moves[0] == GALATZ);
	    assert(moves[1] == DOUBLE_BACK_1);
        assert(moves[2] == DOUBLE_BACK_2); 
		assert(moves[3] == DOUBLE_BACK_3);  
        free(moves);
        printf("Test passed!\n");
        DvFree(dv);
    }


	
	{///////////////////////////////////////////////////////////////////
		// There is already a DOUBLE_BACK move in his trail
		printf("Test for DvWhereCanIGo 1\n");
		
		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DKL.V.. "
			"GGE.... SGE.... HGE.... MGE.... DD1T... "
			"GGE.... SGE.... HGE.... MGE.... DBCT... "
			"GGE.... SGE.... HGE.... MGE.... DHIT... "
			"GGE.... SGE.... HGE.... MGE....";
		
		Message messages[24] = {};
		DraculaView dv = DvNew(trail, messages);
		
		int numLocs = -1;
		PlaceId *locs = DvWhereCanIGo(dv, &numLocs);
		assert(numLocs == 4);
		sortPlaces(locs, numLocs);
		assert(locs[0] == BELGRADE);
		assert(locs[1] == CONSTANTA);
		assert(locs[2] == GALATZ);
		assert(locs[3] == SOFIA);
		free(locs);
		
		printf("Test passed!\n");
		DvFree(dv);
	}

	// NEW TESTS
	{///////////////////////////////////////////////////////////////////
		printf("Test for DvWhereCanIGo 2\n");
		
		char *trail =
			"GSW.... SLS.... HMR.... MHA.... DSJ.V.. "
			"GSW.... SLS.... HMR.... MHA.... DBET... "
			"GSW.... SLS.... HMR.... MHA.... DSZT... "
			"GSW.... SLS.... HMR.... MHA.... DKLT... "
			"GSW.... SLS.... HMR.... MHA.... DHIT... "
			"GSW.... SLS.... HMR.... MHA....";
		
		Message messages[40] = {};
		DraculaView dv = DvNew(trail, messages);
		int numLocs = -1;
		PlaceId *locs = DvWhereCanIGo(dv, &numLocs);
		assert(numLocs == 7);
		sortPlaces(locs, numLocs);
		assert(locs[0] == BELGRADE);
		assert(locs[1] == BUCHAREST);
		assert(locs[2] == BUDAPEST);
		assert(locs[3] == CASTLE_DRACULA); 
		assert(locs[4] == GALATZ); 
		assert(locs[5] == KLAUSENBURG); 
		assert(locs[6] == SZEGED); 
		free(locs);
		
		printf("Test passed!\n");
		DvFree(dv);
	}
	
	{///////////////////////////////////////////////////////////////////
		// Dracula is at sea
		printf("Test for DvWhereCanIGo 3\n");
		
		char *trail =
			"GKL.... SKL.... HGA.... MGA.... DCD.V.. "
			"GKL.... SKL.... HGA.... MGA.... DKLT... "
			"GKL.... SKL.... HGA.... MGA.... DCDT... "
			"GKL.... SKL.... HGA.... MGA.... DGAT... "
			"GKL.... SKL.... HGA.... MGA.... DCNT... "
			"GKL.... SKL.... HGA.... MGA.... DBST...";
		
		Message messages[40] = {};
		DraculaView dv = DvNew(trail, messages);

		int numLocs = -1;
		PlaceId *locs = DvWhereCanIGo(dv, &numLocs);
		assert(numLocs == 4);
		sortPlaces(locs, numLocs);
		assert(locs[0] == BLACK_SEA);
		assert(locs[1] == CONSTANTA);
		assert(locs[2] == IONIAN_SEA);
		assert(locs[3] == VARNA); 
		free(locs);
		
		printf("Test passed!\n");
		DvFree(dv);
	}

	
	{ ////////////////////////////////////////////////////////////////////////
        printf("Test for DvWhereCanIGo 4\n"); // ensure cannot double back to 
        // a location that is not adjacent. 
        char *trail = 
            "GSW.... SLS.... HMR.... MHA.... DSJ.V.. "
            "GLO.... SAL.... HCO.... MBR.... DBET... "
            "GED.... SBO.... HLI.... MPR.... DKLT... "
            "GLV.... SNA.... HNU.... MBD.... DCDT... "
            "GIR.... SPA.... HPR.... MKLT... DHIT... "
            "GAO.... SST.... HSZ.... MCDTTD.";
            
        Message messages[24] = {};
        DraculaView dv = DvNew(trail, messages);

        
        int numLocs = -1;
        PlaceId *locs = DvWhereCanIGo(dv, &numLocs);
        assert(numLocs == 3);
        sortPlaces(locs, numLocs);
		assert(locs[0] == CASTLE_DRACULA);
	    assert(locs[1] == GALATZ);
        assert(locs[2] == KLAUSENBURG);     
        free(locs);
        printf("Test passed!\n");
        DvFree(dv);
    }

	{
		printf("Test for DvWhereCanIGo 5 \n"); // Make Sure when there is no other
		// valid moves, Dracula TP back to CD. 
        char *trail = 
			"GBB.... SMS.... HAO.... MMS.... DED.V.. "
			"GNA.... SAO.... HEC.... MAO.... DMNT... "
			"GBO.... SIR.... HLE.... MEC.... DD2T... "
			"GSR.... SDU.... HBU.... MPL.... DHIT... "
			"GSN.... SIR.... HAM.... MLO.... DTPT... "
			"GAL.... SAO.... HCO.... MEC.... DCDT... "
			"GMS.... SMS.... HFR.... MLE.... DKLT.V. "
			"GTS.... STS.... HBR.... MCO.... DGAT.M. "
			"GIO.... SIO.... HBD.... MLI.... DD3T.M. "
			"GBS.... SBS.... HKLT... MBR.... DHI..M. "
			"GCN.... SCN.... HCDTTTD MVI....";
            
        Message messages[24] = {};
        DraculaView dv = DvNew(trail, messages);
        
        int numLocs = -1;
        PlaceId *locs = DvWhereCanIGo(dv, &numLocs);
        assert(numLocs == 1);
        sortPlaces(locs, numLocs);
		assert(locs[0] == CASTLE_DRACULA); 
        free(locs);
        printf("Test passed!\n");
        DvFree(dv);
	}

	
	
	{///////////////////////////////////////////////////////////////////
		// There is already a DOUBLE_BACK move in his trail
		// He cannot go by boat
		printf("Test for DvWhereCanIGoByType 1\n");
		
		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DKL.V.. "
			"GGE.... SGE.... HGE.... MGE.... DD1T... "
			"GGE.... SGE.... HGE.... MGE.... DBCT... "
			"GGE.... SGE.... HGE.... MGE.... DHIT... "
			"GGE.... SGE.... HGE.... MGE.... DCNT...";
		
		Message messages[25] = {};
		DraculaView dv = DvNew(trail, messages);
		// VR, GA
		int numLocs = -1;
		PlaceId *locs = DvWhereCanIGoByType(dv, true, false, &numLocs);
		assert(numLocs == 2);
		sortPlaces(locs, numLocs);
		assert(locs[0] == GALATZ);
		assert(locs[1] == VARNA);
		free(locs);
		
		printf("Test passed!\n");
		DvFree(dv);
	}

	// NEW TESTS
	{///////////////////////////////////////////////////////////////////
		// Dracula's only move is D1
		printf("Test for DvWhereCanIGoByType 2\n");
		
		char *trail =
			"GSW.... SLS.... HMR.... MHA.... DSJ.V.. "
			"GSW.... SLS.... HMR.... MHA.... DBET... "
			"GSW.... SLS.... HMR.... MHA.... DSZT... "
			"GSW.... SLS.... HMR.... MHA.... DKLT... "
			"GSW.... SLS.... HMR.... MHA.... DHIT... "
			"GSW.... SLS.... HMR.... MHA....";
		
		Message messages[40] = {};
		DraculaView dv = DvNew(trail, messages);
		int numLocs = -1;
		PlaceId *locs = DvWhereCanIGoByType(dv, false, false, &numLocs);
		assert(numLocs == 1);
		sortPlaces(locs, numLocs);
		assert(locs[0] == KLAUSENBURG);

		free(locs);
		
		printf("Test passed!\n");
		DvFree(dv);
	}


	{///////////////////////////////////////////////////////////////////
		// Dracula's only move is TELEPORT
		printf("Test for DvWhereCanIGoByType 3\n");
		
		char *trail =
			"GSW.... SLS.... HMR.... MHA.... DSJ.V.. "
			"GSW.... SLS.... HMR.... MHA.... DBET... "
			"GSW.... SLS.... HMR.... MHA.... DD1T... "
			"GSW.... SLS.... HMR.... MHA.... DSOT... "
			"GSW.... SLS.... HMR.... MHA.... DBCT... "
			"GSW.... SLS.... HMR.... MHA.... DKLT... "
			"GSW.... SLS.... HMR.... MHA.... DHIT... "
			"GSW.... SLS.... HMR.... MHA....";
		
		Message messages[40] = {};
		DraculaView dv = DvNew(trail, messages);
		int numLocs = -1;
		PlaceId *locs = DvWhereCanIGoByType(dv, false, false, &numLocs);
		assert(numLocs == 1);
		sortPlaces(locs, numLocs);
		assert(locs[0] == CASTLE_DRACULA);

		free(locs);
		
		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
		// Dracula is at sea
		// His only allowed moves are by road
		printf("Test for DvWhereCanIGoByType 4\n");
		
		char *trail =
			"GKL.... SKL.... HGA.... MGA.... DCD.V.. "
			"GKL.... SKL.... HGA.... MGA.... DKLT... "
			"GKL.... SKL.... HGA.... MGA.... DCDT... "
			"GKL.... SKL.... HGA.... MGA.... DGAT... "
			"GKL.... SKL.... HGA.... MGA.... DCNT... "
			"GKL.... SKL.... HGA.... MGA.... DBST...";
		
		Message messages[40] = {};
		DraculaView dv = DvNew(trail, messages);
		
		int numLocs = -1;
		PlaceId *locs = DvWhereCanIGoByType(dv, true, false, &numLocs);
		assert(numLocs == 1);
		sortPlaces(locs, numLocs);
		assert(locs[0] == BLACK_SEA);

		free(locs);
		
		printf("Test passed!\n");
		DvFree(dv);
	}

	{ ////////////////////////////////////////////////////////////////////////
        printf("Test for DvWhereCanIGoByType 5\n"); // ensure cannot double back to 
        // a location that is not adjacent. Vampire should stay in current location.
        char *trail = 
            "GSW.... SLS.... HMR.... MHA.... DSJ.V.. "
            "GLO.... SAL.... HCO.... MBR.... DBET... "
            "GED.... SBO.... HLI.... MPR.... DKLT... "
            "GLV.... SNA.... HNU.... MBD.... DCDT... "
            "GIR.... SPA.... HPR.... MKLT... DHIT... "
            "GAO.... SST.... HSZ.... MCDTTD.";
            
        Message messages[24] = {};
        DraculaView dv = DvNew(trail, messages);

        
        int numLocs = -1;
        PlaceId *locs = DvWhereCanIGoByType(dv, false, true, &numLocs);
        assert(numLocs == 1);
        sortPlaces(locs, numLocs);
		assert(locs[0] == CASTLE_DRACULA);;     
        free(locs);
        printf("Test passed!\n");
        DvFree(dv);
    }

	{
		printf("Test for DvWhereCanIGoByType 6 \n"); // Make Sure when there is no other
		// valid moves, Dracula TP back to CD. 
        char *trail = 
			"GBB.... SMS.... HAO.... MMS.... DED.V.. "
			"GNA.... SAO.... HEC.... MAO.... DMNT... "
			"GBO.... SIR.... HLE.... MEC.... DD2T... "
			"GSR.... SDU.... HBU.... MPL.... DHIT... "
			"GSN.... SIR.... HAM.... MLO.... DTPT... "
			"GAL.... SAO.... HCO.... MEC.... DCDT... "
			"GMS.... SMS.... HFR.... MLE.... DKLT.V. "
			"GTS.... STS.... HBR.... MCO.... DGAT.M. "
			"GIO.... SIO.... HBD.... MLI.... DD3T.M. "
			"GBS.... SBS.... HKLT... MBR.... DHI..M. "
			"GCN.... SCN.... HCDTTTD MVI....";
            
        Message messages[24] = {};
        DraculaView dv = DvNew(trail, messages);
        
        int numLocs = -1;
        PlaceId *locs = DvWhereCanIGoByType(dv,false, false, &numLocs);
        assert(numLocs == 1);
        sortPlaces(locs, numLocs);
		assert(locs[0] == CASTLE_DRACULA); 
        free(locs);
        printf("Test passed!\n");
        DvFree(dv);
	}

	
	
	
	// NEW TESTS
	{///////////////////////////////////////////////////////////////////
		// Max rail move is 2
		printf("Test for DvWhereCanTheyGo 1\n");
		
		char *trail =
			"GSW.... SLS.... HMR.... MHA.... DSJ.V.. "
			"GLO.... SAL.... HCO.... MBR.... DBET... "
			"GED.... SBO.... HLI.... MPR.... DKLT... "
			"GLV.... SNA.... HNU....";

		Message messages[18] = { };
		DraculaView dv = DvNew(trail, messages);
		assert(DvGetPlayerLocation(dv, PLAYER_MINA_HARKER) == PRAGUE);
		assert(DvGetRound(dv) == 3);

		int numLocs = -1;
		PlaceId *locs = DvWhereCanTheyGo(dv, PLAYER_MINA_HARKER, &numLocs);
		assert(numLocs == 8);
		sortPlaces(locs, numLocs);
		assert(locs[0] == BERLIN);
		assert(locs[1] == BUDAPEST);
		assert(locs[2] == HAMBURG);
		assert(locs[3] == LEIPZIG);
		assert(locs[4] == NUREMBURG);
		assert(locs[5] == PRAGUE);
		assert(locs[6] == VENICE);
		assert(locs[7] == VIENNA);
		free(locs); 
		
		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
		// Moving from sea
		printf("Test for DvWhereCanTheyGo 2\n");
		
		char *trail =
			"GSW.... SLS.... HMR.... MHA.... DSJ.V.. "
			"GLO.... SAL.... HCO.... MBR.... DBET... "
			"GED.... SBO.... HLI.... MPR.... DKLT... "
			"GLV.... SNA.... HNU.... MBD.... DCDT... "
			"GIR.... SPA.... HPR.... MKLT... DHIT... "
			"GAO.... SST.... HSZ.... MCDTTD. DGAT... "
			"GMS.... SFL.... HKL.... MSZ.... DCNT.V. "
			"GTS.... SRO.... HBC.... MCNTD..";

		Message messages[39] = { };
		DraculaView dv = DvNew(trail, messages);
		assert(DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) == TYRRHENIAN_SEA);
		assert(DvGetPlayerLocation(dv, PLAYER_DR_SEWARD) == ROME);
		assert(DvGetPlayerLocation(dv, PLAYER_VAN_HELSING) == BUCHAREST);
		assert(DvGetPlayerLocation(dv, PLAYER_MINA_HARKER) == CONSTANTA);
		assert(DvGetRound(dv) == 7);

		int numLocs = -1;
		PlaceId *locs = DvWhereCanTheyGo(dv, PLAYER_LORD_GODALMING, &numLocs);
		assert(numLocs == 7);
		sortPlaces(locs, numLocs);
		assert(locs[0] == CAGLIARI);
		assert(locs[1] == GENOA);
		assert(locs[2] == IONIAN_SEA);
		assert(locs[3] == MEDITERRANEAN_SEA);
		assert(locs[4] == NAPLES);
		assert(locs[5] == ROME);
		assert(locs[6] == TYRRHENIAN_SEA);
		free(locs); 
		
		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
		// Maximum rail move is 3
		printf("Test for DvWhereCanTheyGo 3\n");
		
		char *trail =
			"GSW.... SLS.... HMR.... MHA.... DSJ.V.. "
			"GLO.... SAL.... HCO.... MBR.... DBET... "
			"GED.... SBO.... HLI.... MPR.... DKLT... "
			"GLV.... SNA.... HNU.... MBD.... DCDT... "
			"GIR.... SPA.... HPR.... MKLT... DHIT... "
			"GAO.... SST.... HSZ.... MBC....";

		Message messages[29] = { };
		DraculaView dv = DvNew(trail, messages);
		assert(DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) == ATLANTIC_OCEAN);
		assert(DvGetPlayerLocation(dv, PLAYER_DR_SEWARD) == STRASBOURG);
		assert(DvGetPlayerLocation(dv, PLAYER_VAN_HELSING) == SZEGED);
		assert(DvGetPlayerLocation(dv, PLAYER_MINA_HARKER) == BUCHAREST);
		assert(DvGetRound(dv) == 5);
		
		int numLocs = -1;
		PlaceId *locs = DvWhereCanTheyGo(dv, PLAYER_VAN_HELSING, &numLocs);
		assert(numLocs == 15);
		sortPlaces(locs, numLocs);
		assert(locs[0] == BELGRADE);
		assert(locs[1] == BUCHAREST);
		assert(locs[2] == BUDAPEST);
		assert(locs[3] == CONSTANTA);
		assert(locs[4] == GALATZ);
		assert(locs[5] == KLAUSENBURG);
		assert(locs[6] == PRAGUE);
		assert(locs[7] == SALONICA); 
		assert(locs[8] == SOFIA); 
		assert(locs[9] == ST_JOSEPH_AND_ST_MARY); 
		assert(locs[10] == SZEGED); 
		assert(locs[11] == VARNA); 
		assert(locs[12] == VENICE); 
		assert(locs[13] == VIENNA); 
		assert(locs[14] == ZAGREB); 

		free(locs); 
		
		printf("Test passed!\n");
		DvFree(dv);
	}

	{
		// Maximum rail move is 0
		printf("Test for DvWhereCanTheyGo 4\n");
		
		char *trail =
			"GSW.... SLS.... HMR.... MHA.... DSJ.V.. "
			"GLO.... SAL.... HCO.... MBR.... DBET... "
			"GED.... SBO.... HLI.... MPR.... DKLT... "
			"GLV.... SNA.... HNU.... MBD.... DCDT...";

		Message messages[40] = { };
		DraculaView dv = DvNew(trail, messages);
		assert(DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) == LIVERPOOL);
		assert(DvGetPlayerLocation(dv, PLAYER_DR_SEWARD) == NANTES);
		assert(DvGetPlayerLocation(dv, PLAYER_VAN_HELSING) == NUREMBURG);
		assert(DvGetPlayerLocation(dv, PLAYER_MINA_HARKER) == BUDAPEST);
		assert(DvGetRound(dv) == 4);
		
		int numLocs = -1;
		PlaceId *locs = DvWhereCanTheyGo(dv, PLAYER_LORD_GODALMING, &numLocs);
		assert(numLocs == 4);
		sortPlaces(locs, numLocs);
		assert(locs[0] == IRISH_SEA);
		assert(locs[1] == LIVERPOOL);
		assert(locs[2] == MANCHESTER);
		assert(locs[3] == SWANSEA);
		

		free(locs); 
		
		printf("Test passed!\n");
		DvFree(dv);




	}	
	
	
	
	
	
	{///////////////////////////////////////////////////////////////////
		// Max rail move is 2
		// only possible move is by road
		printf("Test for DvWhereCanTheyGoByType 1\n");
		
		char *trail =
			"GSW.... SLS.... HMR.... MHA.... DSJ.V.. "
			"GLO.... SAL.... HCO.... MBR.... DBET... "
			"GED.... SBO.... HLI.... MPR.... DKLT... "
			"GLV.... SNA.... HNU....";

		Message messages[18] = { };
		DraculaView dv = DvNew(trail, messages);
		assert(DvGetPlayerLocation(dv, PLAYER_MINA_HARKER) == PRAGUE);
		assert(DvGetRound(dv) == 3);

		int numLocs = -1;
		PlaceId *locs = DvWhereCanTheyGoByType(dv, PLAYER_MINA_HARKER, true, false, false, &numLocs);
		assert(numLocs == 4);
		sortPlaces(locs, numLocs);
		assert(locs[0] == BERLIN);
		assert(locs[1] == NUREMBURG);
		assert(locs[2] == PRAGUE);
		assert(locs[3] == VIENNA);
		free(locs); 
		
		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
		// Moving from sea
		// Only possible move is by boat
		printf("Test for DvWhereCanTheyGoByType 2\n");
		
		char *trail =
			"GSW.... SLS.... HMR.... MHA.... DSJ.V.. "
			"GLO.... SAL.... HCO.... MBR.... DBET... "
			"GED.... SBO.... HLI.... MPR.... DKLT... "
			"GLV.... SNA.... HNU.... MBD.... DCDT... "
			"GIR.... SPA.... HPR.... MKLT... DHIT... "
			"GAO.... SST.... HSZ.... MCDTTD. DGAT... "
			"GMS.... SFL.... HKL.... MSZ.... DCNT.V. "
			"GTS.... SRO.... HBC.... MCNTD..";

		Message messages[39] = { };
		DraculaView dv = DvNew(trail, messages);
		assert(DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) == TYRRHENIAN_SEA);
		assert(DvGetPlayerLocation(dv, PLAYER_DR_SEWARD) == ROME);
		assert(DvGetPlayerLocation(dv, PLAYER_VAN_HELSING) == BUCHAREST);
		assert(DvGetPlayerLocation(dv, PLAYER_MINA_HARKER) == CONSTANTA);
		assert(DvGetRound(dv) == 7);

		int numLocs = -1;
		PlaceId *locs = DvWhereCanTheyGoByType(dv, PLAYER_LORD_GODALMING, false, false, true, &numLocs);
		assert(numLocs == 7);
		sortPlaces(locs, numLocs);
		assert(locs[0] == CAGLIARI);
		assert(locs[1] == GENOA);
		assert(locs[2] == IONIAN_SEA);
		assert(locs[3] == MEDITERRANEAN_SEA);
		assert(locs[4] == NAPLES);
		assert(locs[5] == ROME);
		assert(locs[6] == TYRRHENIAN_SEA);
		free(locs); 
		
		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
		// Maximum rail move is 3
		// Only possible move type is by rail.
		printf("Test for DvWhereCanTheyGoByType 3\n");
		
		char *trail =
			"GSW.... SLS.... HMR.... MHA.... DSJ.V.. "
			"GLO.... SAL.... HCO.... MBR.... DBET... "
			"GED.... SBO.... HLI.... MPR.... DKLT... "
			"GLV.... SNA.... HNU.... MBD.... DCDT... "
			"GIR.... SPA.... HPR.... MKLT... DHIT... "
			"GAO.... SST.... HSZ.... MBC....";

		Message messages[29] = { };
		DraculaView dv = DvNew(trail, messages);
		assert(DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) == ATLANTIC_OCEAN);
		assert(DvGetPlayerLocation(dv, PLAYER_DR_SEWARD) == STRASBOURG);
		assert(DvGetPlayerLocation(dv, PLAYER_VAN_HELSING) == SZEGED);
		assert(DvGetPlayerLocation(dv, PLAYER_MINA_HARKER) == BUCHAREST);
		assert(DvGetRound(dv) == 5);
		
		int numLocs = -1;
		PlaceId *locs = DvWhereCanTheyGoByType(dv, PLAYER_VAN_HELSING, false, true, false, &numLocs);
		assert(numLocs == 12);
		sortPlaces(locs, numLocs);
		assert(locs[0] == BELGRADE);
		assert(locs[1] == BUCHAREST);
		assert(locs[2] == BUDAPEST);
		assert(locs[3] == CONSTANTA);
		assert(locs[4] == GALATZ);
		assert(locs[5] == PRAGUE);
		assert(locs[6] == SALONICA); 
		assert(locs[7] == SOFIA); 
		assert(locs[8] == SZEGED); 
		assert(locs[9] == VARNA); 
		assert(locs[10] == VENICE); 
		assert(locs[11] == VIENNA);  

		free(locs); 
		
		printf("Test passed!\n");
		DvFree(dv);
	}
	{///////////////////////////////////////////////////////////////////
        // Dracula is at sea
        printf("Test for DvWhereCanIGo 2\n");
        
        char *trail =
            "GKL.... SKL.... HGA.... MGA.... DCD.V.. "
            "GKL.... SKL.... HGA.... MGA.... DKLT... "
            "GKL.... SKL.... HGA.... MGA.... DD2T... "
            "GKL.... SKL.... HGA.... MGA.... DGAT... "
            "GKL.... SKL.... HGA.... MGA.... DCNT... "
            "GKL.... SKL.... HGA.... MGA.... DBST... "
			"GKL....";
        
        Message messages[40] = {};
        DraculaView dv = DvNew(trail, messages);

        int numLocs = -1;
        PlaceId *locs = DvGetValidMoves(dv, &numLocs);
        assert(numLocs == 2);
        sortPlaces(locs, numLocs);
        assert(locs[0] == IONIAN_SEA);
        assert(locs[1] == VARNA);

        free(locs);
        
        printf("Test passed!\n");
        DvFree(dv);
    }

	{///////////////////////////////////////////////////////////////////
        // Dracula is at sea
        printf("Test for DvWhereCanIGo 2\n");
        
        char *trail =
            "GKL.... SKL.... HGA.... MGA.... DKL.V.. "
            "GKL.... SKL.... HGA.... MGA.... DCDT... "
            "GKL.... SKL.... HGA.... MGA.... DHIT... "
            "GKL.... SKL.... HGA.... MGA.... DGAT... "
            "GKL.... SKL.... HGA.... MGA.... DCNT... "
            "GKL.... SKL.... HGA.... MGA.... DBST... "
			"GKL....";
        
        Message messages[40] = {};
        DraculaView dv = DvNew(trail, messages);

        int numLocs = -1;
        PlaceId *locs = DvGetValidMoves(dv, &numLocs);
        assert(numLocs == 4);
        sortPlaces(locs, numLocs);
        assert(locs[0] == IONIAN_SEA);
        assert(locs[1] == VARNA);
		assert(locs[2] == DOUBLE_BACK_1);
		assert(locs[3] == DOUBLE_BACK_2);

        free(locs);
        
        printf("Test passed!\n");
        DvFree(dv);
    }
	
	{
		// Maximum rail move is 0
		printf("Test for DvWhereCanTheyGoByType 4\n");
		
		char *trail =
			"GSW.... SLS.... HMR.... MHA.... DSJ.V.. "
			"GLO.... SAL.... HCO.... MBR.... DBET... "
			"GED.... SBO.... HLI.... MPR.... DKLT... "
			"GLV.... SNA.... HNU.... MBD.... DCDT...";

		Message messages[40] = { };
		DraculaView dv = DvNew(trail, messages);
		assert(DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) == LIVERPOOL);
		assert(DvGetPlayerLocation(dv, PLAYER_DR_SEWARD) == NANTES);
		assert(DvGetPlayerLocation(dv, PLAYER_VAN_HELSING) == NUREMBURG);
		assert(DvGetPlayerLocation(dv, PLAYER_MINA_HARKER) == BUDAPEST);
		assert(DvGetRound(dv) == 4);
		
		int numLocs = -1;
		PlaceId *locs = DvWhereCanTheyGoByType(dv, PLAYER_LORD_GODALMING,true, true, false, &numLocs);
		assert(numLocs == 3);
		sortPlaces(locs, numLocs);
		assert(locs[0] == LIVERPOOL);
		assert(locs[1] == MANCHESTER);
		assert(locs[2] == SWANSEA);
		

		free(locs); 
		
		printf("Test passed!\n");
		DvFree(dv);
	}	
	
	{
		// If all transport methods are restricted, stay in the same spot.
		printf("Test for DvWhereCanTheyGoByType 5\n");
		
		char *trail =
			"GSW.... SLS.... HMR.... MHA.... DSJ.V.. "
			"GLO.... SAL.... HCO.... MBR.... DBET... "
			"GED.... SBO.... HLI.... MPR.... DKLT... "
			"GLV.... SNA.... HNU.... MBD.... DCDT...";

		Message messages[40] = { };
		DraculaView dv = DvNew(trail, messages);
		assert(DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) == LIVERPOOL);
		assert(DvGetPlayerLocation(dv, PLAYER_DR_SEWARD) == NANTES);
		assert(DvGetPlayerLocation(dv, PLAYER_VAN_HELSING) == NUREMBURG);
		assert(DvGetPlayerLocation(dv, PLAYER_MINA_HARKER) == BUDAPEST);
		assert(DvGetRound(dv) == 4);
		
		int numLocs = -1;
		PlaceId *locs = DvWhereCanTheyGoByType(dv, PLAYER_LORD_GODALMING,false, false,false, &numLocs);
		assert(numLocs == 1);
		sortPlaces(locs, numLocs);
		assert(locs[0] == LIVERPOOL);

		free(locs); 
		
		printf("Test passed!\n");
		DvFree(dv);




	}	
	
	
	
	return EXIT_SUCCESS;
}
