


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <math.h>
#include <sjwlib.h>
#include <atlantisboxmodel.h>
#include <atEcologyLib.h>
#include <atUtilLib.h>
#include <atPhysics.h>

void get_property(MSEBoxModel *bm, PhyPropertyData *propInput);
void open_phyprop(MSEBoxModel *bm, PhyPropertyData *propInput);
void free_PhyPropertyData(MSEBoxModel *bm,PhyPropertyData *propInput);

/*********************************************************************/
void swrForcingBM(MSEBoxModel *bm, PhyPropertyData *inputData) {
	int b = 0;
	double tleft = bm->dt;
	double dt;

	if (verbose)
		fprintf(stderr, "Entering swrForcingBM\n");

	if(inputData->atEnd)
		return;

	//fprintf(bm->logFile, "in swrForcingBM\n");

	/* Loop while more time remains in this transport time step */
	while (tleft > 0) {

		/* Get more data if necessary */
		if (inputData->tleft <= 0){
			get_property(bm, inputData);
			if(inputData->atEnd == TRUE)
				return;
		}

		/* Calculate time step allowed */
		dt = min(tleft, inputData->tleft);

		/* Loop over each box to check the values */
		for (b = 0; b < bm->nbox; b++) {
			Box *bp = &bm->boxes[b];

			if(inputData->dataBuffer[b][0] != inputData->missing_value){
				bp->swr = inputData->dataBuffer[b][0];
				if (!_finite(bp->swr)) {
					quit("%s in %d at time %e is not finite %e\n", b, bm->dayt, inputData->variableName, bp->swr);
				}
				/*fprintf(bm->logFile, "Time %e: %s in box %d is set to %e, inputData->tleft= %e, rewind = %d\n",
						bm->dayt, inputData->variableName, b, bp->swr, inputData->tleft, bm->tempinput.rewind);*/
			}

		}

		inputData->tleft -= dt;
		tleft -= dt;
	}

	/* Get more data if necessary, so that time
	 * is updated appropriately
	 */

	if(inputData->rewind == TRUE){
		if (inputData->tleft <= 0)
			get_property(bm, inputData);
	}
}


void init_SolarData(MSEBoxModel *bm, FILE *fp, PhyPropertyData *propInput, char *variableName, char *shortName, char *longName, double minValue, double maxValue){
	char buf[200];

	if (propInput->tracerID == -1)
		quit("init_PhyPropertyData: no %sid set\n", variableName);

	propInput->is_valid_z = FALSE;
	readkeyprm_s(fp, "Solar_radiation", buf);

	/* Will hard code for now - might want more than one solar nc file in the future but they shouldn't be large files as they don't include z */
	propInput->nFiles = 1;

	/* Allocate memory for file names if necessary */
	propInput->fname = c_alloc2d(BMSLEN, propInput->nFiles);
    propInput->use_resets = (int *)i_alloc1d(propInput->nFiles);
    propInput->use_resets[0] = 0;
    
	strcpy(propInput->fname[0], buf);
	propInput->variableName = c_alloc1d(BMSLEN);
	strcpy(propInput->variableName, variableName);
	snprintf(buf, sizeof(buf), "%s_rewind", shortName);
	readkeyprm_i(fp, buf, &propInput->rewind);

    if(propInput->rewind == FALSE) {  // Just inc case looking for swr_rewind and its Solar_radiation_rewind
        snprintf(buf, sizeof(buf), "Solar_radiation_rewind");
        readkeyprm_i(fp, buf, &propInput->rewind);
    }

	/* Set the min and max values */
	propInput->min_value = minValue;
	propInput->max_value = maxValue;

}

void readSolar(MSEBoxModel *bm, char *tunit){
	char buf[STRLEN];
	FILE *fp;

	/* Open the file containing the list of hydro input files */
	if ((fp = Open_Input_File(bm->inputFolder,bm->forceIfname, "r")) == NULL)
		quit("tempsalt_init: Can't open %s%s\n", bm->inputFolder, bm->forceIfname);

	/* Read short wave radiation input */
	if(readkeyprm_s(fp, "Solar_radiation", buf) > 0){

		if(strstr(buf, ".ts") != NULL){
			read_bm_ts_rewind(bm, "Solar_radiation", &bm->swr, tunit, "swr", "W m-2", &bm->swr_id, &bm->swr_rewindid, warn);
			printf("Loaded solar time series from ts file\n");
		} else if(strstr(buf, ".nc") != NULL){
			if(strcmp(buf, ".nc")){
				init_SolarData(bm, fp, &bm->swrinput, "swr", "swr","swr", 0, 1000);
				bm->swrinput.curFile = 0;
				open_phyprop(bm, &(bm->swrinput));
				get_property(bm, &(bm->swrinput));
				printf("Loaded solar time series from nc file\n");
			}
		} else {
			warn("readSolar: No recognized format of solar radiation input file %s in forcing prm file. This assumes that Atlantis will calculate light based on location\n", buf);
		}
	}else{
		warn("readSolar: No solar radiation input file found in forcing prm file. This assumes that Atlantis will calculate light based on location\n");
	}
	fclose(fp);
}

void freeSolar(MSEBoxModel *bm){


	if(bm->swr != NULL){
			tsFree(bm->swr);
			free(bm->swr);
	} else if (bm->swrinput.nFiles > 0){
		free_PhyPropertyData(bm, &bm->swrinput);
	}

}

