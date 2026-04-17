/**
 *	\brief The process functions for land related processes
 *	\ingroup atecology
 *
 *
 *	At present the only land functionality is movement into/out of land boxes of vertebrates.
 *	Only prescribed movement is supported for groups that are active in land boxes.
 *
 *	Vertebrates in land boxes will reproduce and age as per normal.
 *
 *
 *
 *
 *	15-11-2014 Bec Gorton
 *	Created the file
 */

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sjwlib.h>
#include <atecology.h>

/**
 \file
 \brief The land related ecological processes.

 **/

/*
 * brief\ Day to day land primary production
 *
 */
void Land_PrimaryProduction(MSEBoxModel *bm) {

	// Nothing to do here yet as not doing explicit vegetation as yet
	return;
}

/*
 * brief\ Land related habitat state
 *
 */
void Land_HabitatState(MSEBoxModel *bm) {
	int b;

	/* Set habitat state - at present nothing to do here so long
	 * as land available in the cell then that is sufficient
	 * TODO: Actually flesh that out once have some idea of terrestrial features desired
	 */

	for (b = 0; b < bm->nbox; b++) {
		if (bm->boxes[b].has_land) {
			bm->boxes[b].land_habitat_qual = 1.0;
		}
	}

	return;
}

/**
 * Land process function.
 *
 *
 */
void Ecology_Land_Biology_Process(MSEBoxModel *bm, Box *pBox) {

	int k;
	int maxdeep = bm->wcnz;
	int totaldeep = pBox->nz;

	/* Set current box */
	bm->current_box = pBox->n;
    
    /* Initialise diet data content - if required
    if (bm->flagdietcheck) {
        ReInitDietData(bm, pBox->n);
    }

     */
	/* Fill in relevant routines - match ice and water ecology style code approach */

	//Copy_WC_Tracers(bm, boxLayerInfo->localWCTracers, boxLayerInfo->localWCFlux, bm->logFile);

	//Vertebrate_Reproduction(bm, 0, maxdeep, totaldeep, 1, boxLayerInfo->localWCTracers, bm->logFile);

	/* Transfer all the temporary values back to their final locations *
	for (k = 0; k < numwcvar; k++) {
		newwctr[bm->current_box][0][k] = boxLayerInfo->localWCTracers[k];
	}
*/

}
