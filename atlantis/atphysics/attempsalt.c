/*********************************************************************

 File:           attempsalt.c

 Created:        Sun Dec 5 15:56:52 EST 2004

 Author:         Beth Fulton
 CSIRO Division of Marine Research

 Purpose:        Routines which deal with 3d temperature and salinity input.

 Arguments:

 Revisions:		22/04/2009 Bec Gorton
 Added code to free the array and value arrays in the get_temp function.


 22-May-2009 Bec Gorton
 Changed the ncopen calls to use sjw_ncopen. This function checks that the
 netcdf file exits before calling ncopen which will crash if the file is not
 found.

 19-07-2009 Bec Gorton
 Changed the temperature and salinity code to allow for more than
 a single temperature and salinity file to be specified in the forcing input file.
 This will require changes to all model run files - documentation will be on the wiki for users.

 28-10-2009 Bec Gorton
 Added the close_tempSaltFinal - this will free up the arrays
 allocated to store the file names and then close any open files.

 30-09-2009 Bec Gorton
 Trunk Merge 1195 - Added a new function freeTempSalt to free up the new arrays bm->tsinput.tempfname
 and bm->tsinput.saltfname. This is called by freePhysics().

 02-08-2011 Bec Gorton
 Split the TempSaltData structure into a structure per property so we can add pH.

 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sjwlib.h>
#include <netcdf.h>
#include <atlantisboxmodel.h>
#include <atUtilLib.h>

/* Prototypes */
void get_property(MSEBoxModel *bm, PhyPropertyData *propInput);
void open_phyprop(MSEBoxModel *bm, PhyPropertyData *propInput);
void close_phyprop(MSEBoxModel *bm, PhyPropertyData *propInput);

void init_forceTracers(MSEBoxModel *bm, FILE *fp);

void init_PhyPropertyData(MSEBoxModel *bm, FILE *fp, PhyPropertyData *propInput, char *variableName, char *shortName, char *longName, double minValue, double maxValue, int is_valid_z);

/* Routine to initialisation temperature and salinity forcing files */
void tempsalt_init(MSEBoxModel *bm) {
	FILE *fp;
	int tracerIndex;
	int i = 0;
	bm->tempid = -1;
	bm->saltid = -1;
	bm->pHid = -1;


	/* Allocate the memory for the change arrays */
	bm->TempLayerScalar = Util_Alloc_Init_1D_Double(bm->wcnz, 1.0);
	bm->SaltLayerScalar = Util_Alloc_Init_1D_Double(bm->wcnz, 1.0);
	bm->pHLayerScalar = Util_Alloc_Init_1D_Double(bm->wcnz, 1.0);


    /* Open the file containing the list of noise ad pollution files input files */
    if ((fp = Open_Input_File(bm->inputFolder,bm->forceIfname, "r")) == NULL)
        quit("tempsalt_init: Can't open %s%s\n", bm->inputFolder, bm->forceIfname);

    /* Set error routine to quit if parameters not found */
    set_keyprm_errfn(quit);
    readkeyprm_i(fp, "use_pollutantfiles", &bm->use_pollutantfiles);
    readkeyprm_i(fp, "use_phFiles", &bm->use_pHfiles);

    /* Close the parameter file */
    fclose(fp);

    /* Check tracers */
    for (i = 0; i < bm->ntracer; i++) {
		if (strcmp(bm->tinfo[i].name, "Temp") == 0){
			bm->tempid = bm->tinfo[i].n;
			bm->tempinput.tracerID = bm->tinfo[i].n;
            bm->tempinput.wgt_coefft = 1.0;
            bm->tempinput.ResetTol = 0.0;
            bm->tempinput.total_input = 0.0;
		}
		if (strcmp(bm->tinfo[i].name, "salt") == 0){
			bm->saltid = bm->tinfo[i].n;
			bm->saltinput.tracerID = bm->tinfo[i].n;
            bm->saltinput.wgt_coefft = 1.0;
            bm->saltinput.ResetTol = 0.0;
            bm->saltinput.total_input = 0.0;
		}
        
        if (bm->use_pHfiles) {
            if (strcmp(bm->tinfo[i].name, "pH") == 0){
                bm->pHid = bm->tinfo[i].n;
                bm->pHinput.tracerID = bm->tinfo[i].n;
                bm->pHinput.wgt_coefft = 1.0;
                bm->pHinput.ResetTol = 0.0;
                bm->pHinput.total_input = 0.0;
            }
		}
        
        if (bm->track_wind) {
            if (strcmp(bm->tinfo[i].name, "Wind") == 0){
                bm->windid = bm->tinfo[i].n;
                bm->windinput.tracerID = bm->tinfo[i].n;
                bm->windinput.wgt_coefft = 1.0;
                bm->windinput.ResetTol = 0.0;
                bm->windinput.total_input = 0.0;
            }
        }
        
        //printf("use_pollutantfiles %d\n", bm->use_pollutantfiles);
        
        if(bm->use_pollutantfiles){
            
            //printf("Doing comparison for %s\n", bm->tinfo[i].name);
            
            if (strcmp(bm->tinfo[i].name, "noise_pollution") == 0){
                bm->noiseid = bm->tinfo[i].n;
                bm->noiseinput.tracerID = bm->tinfo[i].n;
                bm->noiseinput.wgt_coefft = 1.0;
                bm->noiseinput.ResetTol = 0.0;
                bm->noiseinput.total_input = 0.0;
            }
            if (strcmp(bm->tinfo[i].name, "light_pollution") == 0){
                bm->lightpid = bm->tinfo[i].n;
                bm->lightpinput.tracerID = bm->tinfo[i].n;
                bm->lightpinput.wgt_coefft = 1.0;
                bm->lightpinput.ResetTol = 0.0;
                bm->lightpinput.total_input = 0.0;
            }
        }
	}
    
	if (bm->tempid == -1)
		quit("tempsalt_init: no tempid set\n");
	if (bm->saltid == -1)
		quit("tempsalt_init: no saltid set\n");
    if (bm->use_pHfiles) {
        if (bm->pHid == -1)
            quit("tempsalt_init: no phid set\n");
    }
    if (bm->track_wind) {
        if (bm->windid == -1)
            quit("tempsalt_init: no windid set\n");
    }
    if(bm->use_pollutantfiles){
        if (bm->noiseid == -1)
            quit("tempsalt_init: no noiseid set\n");
        if (bm->lightpid == -1)
            quit("tempsalt_init: no lightpid set\n");
    }

	/* Open the file containing the list of hydro input files */
	if ((fp = Open_Input_File(bm->inputFolder,bm->forceIfname, "r")) == NULL)
		quit("tempsalt_init: Can't open %s%s\n", bm->inputFolder, bm->forceIfname);

	/* Set error routine to quit if parameters not found */
	set_keyprm_errfn(quit);

	/* Read the rest of the list of files */
	readkeyprm_i(fp, "use_tempfiles", &bm->use_tempfiles);
	if (bm->use_tempfiles) {
		init_PhyPropertyData(bm, fp, &bm->tempinput, "temperature", "temp", "Temperature", -40, 40, TRUE);
	}
    
	readkeyprm_i(fp, "use_saltfiles", &bm->use_saltfiles);
	if (bm->use_saltfiles) {
		init_PhyPropertyData(bm, fp, &bm->saltinput, "salinity", "salt", "Salinity", -10, 50, TRUE);
	}

	if (bm->use_pHfiles) {
        if (bm->pHid == -1)
            quit("tempsalt_init: no phid set\n");

		init_PhyPropertyData(bm, fp, &bm->pHinput, "pH",  "pH", "pH", -10, 10, TRUE);
	}
    
    if (bm->track_wind) {
        if (bm->windid == -1)
            quit("tempsalt_init: no windid set\n");

        init_PhyPropertyData(bm, fp, &bm->windinput, "Wind",  "Wind", "Wind", -100, 100, TRUE);
    }

    if (bm->use_pollutantfiles) {
        init_PhyPropertyData(bm, fp, &bm->noiseinput, "noise_pollution", "noise_pollution", "Noise_Pollution", 0, 10000, TRUE);
        init_PhyPropertyData(bm, fp, &bm->lightpinput, "light_pollution", "light_pollution", "Light_Pollution", 0, 10000, TRUE);
    }

    readkeyprm_i(fp, "use_VertMixFiles", &bm->use_VertMixfiles);
    if (bm->use_VertMixfiles) {
        init_PhyPropertyData(bm, fp, &bm->VertMixinput, "vertMixScalar",  "vertMixScalar", "vertMixScalar", 0, 10000000, FALSE);        
    }

	init_forceTracers(bm, fp);
    init_forceMoveEntries(bm, fp);

	/* Close the parameter file */
	fclose(fp);

	warn("tempsalt_init: Starting from beginning of salt and temp inputs\n"
			"This will probably be incorrect for a 'hot' start of the model\n\n");

	bm->tempinput.curFile = 0;
	bm->saltinput.curFile = 0;

	if (bm->use_tempfiles) {
		open_phyprop(bm, &(bm->tempinput));
	}
	if (bm->use_saltfiles) {
		open_phyprop(bm, &(bm->saltinput));
	}
	if (bm->use_pHfiles) {
		bm->pHinput.curFile = 0;
		open_phyprop(bm, &(bm->pHinput));
	}
    if (bm->track_wind) {
        bm->windinput.curFile = 0;
        open_phyprop(bm, &(bm->windinput));
    }
    if (bm->use_VertMixfiles) {
        bm->VertMixinput.curFile = 0;
        open_phyprop(bm, &(bm->VertMixinput));
        bm->VertMixinput.isBoxValue = TRUE;
        bm->VertMixinput.is_valid_z = FALSE;
        bm->VertMixinput.boxValueIndex = VERTMIX_ID;
        bm->VertMixinput.isScalar = TRUE;
    }

    if(bm->use_pollutantfiles){
        bm->noiseinput.curFile = 0;
        bm->lightpinput.curFile = 0;
        open_phyprop(bm, &(bm->noiseinput));
        open_phyprop(bm, &(bm->lightpinput));
    }
    
	/* Read first set of data */
	if (bm->use_tempfiles) {
		get_property(bm, &(bm->tempinput));
	}
	if (bm->use_saltfiles) {
		get_property(bm, &(bm->saltinput));
	}
	if (bm->use_pHfiles) {
		get_property(bm, &(bm->pHinput));
	}
    if (bm->track_wind) {
        get_property(bm, &(bm->windinput));
    }
    if (bm->use_VertMixfiles) {
        get_property(bm, &(bm->VertMixinput));
    }
    
    if(bm->use_pollutantfiles){
        get_property(bm, &(bm->noiseinput));
        get_property(bm, &(bm->lightpinput));
    }

	if (bm->use_forceTracers) {
		for (tracerIndex = 0; tracerIndex < bm->numForceTracers; tracerIndex++) {
			open_phyprop(bm, &(bm->forceTracerInput[tracerIndex]));
			bm->forceTracerInput[tracerIndex].curFile = 0;
			get_property(bm, &(bm->forceTracerInput[tracerIndex]));
		}
	}
    
    if (bm->use_move_entries) {
        for (tracerIndex = 0; tracerIndex < bm->numForceMoveEntries; tracerIndex++) {
            open_move_prop(bm, &(bm->forceMoveEntryInput[tracerIndex]));
            bm->forceMoveEntryInput[tracerIndex].curFile = 0;
            //Init_full_move_array(bm, bm->forceMoveEntryInput, tracerIndex);
            // Initalise after biology read in so first step not over written by prm file
        }
    }
}

/**
 * Initialise the physical property data.
 *
 * This is used to replace the old temperature and salinity functions - a single reusable function means we can use it for swr as well.
 *
 *
 */

void init_PhyPropertyData(MSEBoxModel *bm, FILE *fp, PhyPropertyData *propInput, char *variableName, char *shortName, char *longName, double minValue, double maxValue, int is_valid_z){
	int i;
	char buf[200];

	if (propInput->tracerID == -1)
		quit("init_PhyPropertyData: no %sid set\n", variableName);

	/* property to specify if this is a 2D or 3D property - means we can read in surface data.*/
	propInput->is_valid_z = is_valid_z;
	/* Read the number of files */
	snprintf(buf, sizeof(buf), "n%sfiles", shortName);
	readkeyprm_i(fp, buf, &propInput->nFiles);
	if (propInput->nFiles < 1)
		quit("init_PhyPropertyData: Must be at least 1 %s input file (%d in %s)\n", variableName, propInput->nFiles, bm->forceIfname);

	/* Allocate memory for file names if necessary */
	propInput->fname = c_alloc2d(BMSLEN, propInput->nFiles);
    propInput->use_resets = (int *)i_alloc1d(propInput->nFiles);
	propInput->variableName = c_alloc1d(BMSLEN);
	strcpy(propInput->variableName, variableName);
	snprintf(buf, sizeof(buf), "%s_rewind", shortName);
	readkeyprm_i(fp, buf, &propInput->rewind);

	/* Read the list of files */
	for (i = 0; i < propInput->nFiles; i++) {
		char key[BMSLEN];
		snprintf(key, sizeof(key), "%s%d.name", longName, i);
		readkeyprm_s(fp, key, propInput->fname[i]);
        propInput->use_resets[i] = 0;
	}

	/* Set the min and max values */
	propInput->min_value = minValue;
	propInput->max_value = maxValue;

}

/**
 *	Read in information about the forcing tracer files.
 *
 *
 */
void init_forceTracers(MSEBoxModel *bm, FILE *fp) {

	int i;
	char key[BMSLEN];
	int tracerIndex;
	char **buf;
	int size;
    int any_use_resets = 0;

	/* By default we are not using netcdf forcing files for eddy values */
	bm->use_eddy_NC = FALSE;

	/* Read the list of files */
	set_keyprm_errfn(warn);
	readkeyprm_i(fp, "use_force_tracers", &bm->use_forceTracers);
	set_keyprm_errfn(quit);

	if (bm->use_forceTracers) {
		printf("use_forceTracers: %d\n", bm->use_forceTracers);

		/* Read the number of files */
		readkeyprm_i(fp, "nforceTracers", &bm->numForceTracers);
		if (bm->numForceTracers < 1) /* Restriction in at present - should only be able to read in a single file */
			quit("init_forceTracers: Must be at least 1 tracer forcing variable (%d in %s)\n", bm->numForceTracers, bm->forceIfname);

		/* Read in the names of the tracers */
		readkeyprm_sarray(fp, "tracerNames", &buf, &size);
        if(size != bm->numForceTracers)
            quit("Your list of tracerNames has a length (%d) that does not match nforceTracers (%d)\n", size, bm->numForceTracers);

        /* Read whether using tracer assimilation weightings */
        readkeyprm_i(fp, "use_weighted_assim", &bm->use_weighted_assim);
        
		bm->forceTracerInput = (PhyPropertyData *) malloc(sizeof(PhyPropertyData) * (long unsigned int)bm->numForceTracers);

		/* Now read in the files for each tracer */
		for (tracerIndex = 0; tracerIndex < bm->numForceTracers; tracerIndex++) {

			bm->forceTracerInput[tracerIndex].variableName = c_alloc1d(BMSLEN);
			bm->forceTracerInput[tracerIndex].isEpi = FALSE;	/* Assume by default this is an wc tracer */
			bm->forceTracerInput[tracerIndex].isBoxValue = FALSE;	/* By default this is not a box variable value */
            bm->forceTracerInput[tracerIndex].isScalar = FALSE;  /* By default assume handling a variable not a scalar*/

			/* property to specify if this is a 2D or 3D property -assume for now all are 3D.*/
			/* if required in the future we can allow users to specify */

			bm->forceTracerInput[tracerIndex].is_valid_z = TRUE;

			strcpy(bm->forceTracerInput[tracerIndex].variableName, buf[tracerIndex]);

			bm->forceTracerInput[tracerIndex].tracerID = -1;
			for (i = 0; i < bm->ntracer; i++) {
				if (strcmp(bm->tinfo[i].name, bm->forceTracerInput[tracerIndex].variableName) == 0){
					bm->forceTracerInput[tracerIndex].tracerID = bm->tinfo[i].n;
					break;
				}
			}

			/* Now try the epi tracers */
			for (i = 0; i < bm->nepi; i++) {
				if (strcmp(bm->einfo[i].name, bm->forceTracerInput[tracerIndex].variableName) == 0){
					bm->forceTracerInput[tracerIndex].tracerID = bm->einfo[i].n;
					bm->forceTracerInput[tracerIndex].isEpi = TRUE;
					bm->forceTracerInput[tracerIndex].is_valid_z = FALSE;
                    bm->forceTracerInput[tracerIndex].isScalar = FALSE;
					break;
				}
			}

			/* Now check to see if this is the eddy value */
			if(strcmp(bm->forceTracerInput[tracerIndex].variableName, "eddy") == 0){
				bm->forceTracerInput[tracerIndex].isBoxValue = TRUE;
				bm->forceTracerInput[tracerIndex].boxValueIndex = EDDY_ID;
                bm->forceTracerInput[tracerIndex].is_valid_z = FALSE;
                bm->forceTracerInput[tracerIndex].isScalar = TRUE;
				bm->use_eddy_NC = TRUE;
			}
			else if(bm->forceTracerInput[tracerIndex].tracerID == -1){
				quit("Unable to find tracer %s from forcing file %s\n", bm->forceTracerInput[tracerIndex].variableName, bm->forceIfname);
			}

			snprintf(key, sizeof(key), "%s_nFiles", bm->forceTracerInput[tracerIndex].variableName);
			readkeyprm_i(fp, key, &bm->forceTracerInput[tracerIndex].nFiles);
            
            bm->forceTracerInput[tracerIndex].fname = c_alloc2d(BMSLEN, bm->forceTracerInput[tracerIndex].nFiles);
            bm->forceTracerInput[tracerIndex].use_resets = (int *)i_alloc1d(bm->forceTracerInput[tracerIndex].nFiles);
            
            printf("Expecting %d files for %s\n", bm->forceTracerInput[tracerIndex].nFiles, bm->forceTracerInput[tracerIndex].variableName);

			for (i = 0; i < bm->forceTracerInput[tracerIndex].nFiles; i++) {

				snprintf(key, sizeof(key), "%s_File%d.name", bm->forceTracerInput[tracerIndex].variableName, i);
                
                printf("Looking for %s\n", key);
                
				readkeyprm_s(fp, key, bm->forceTracerInput[tracerIndex].fname[i]);
                
                printf("file %d for %s is %s\n", i, bm->forceTracerInput[tracerIndex].variableName, bm->forceTracerInput[tracerIndex].fname[i]);
                
                snprintf(key, sizeof(key), "%s_File%d.use_resets", bm->forceTracerInput[tracerIndex].variableName, i);
                readkeyprm_i(fp, key, &bm->forceTracerInput[tracerIndex].use_resets[i]);
                
                if (bm->forceTracerInput[tracerIndex].use_resets[i] > 0) {
                    any_use_resets = 1;
                }

			}

			snprintf(key, sizeof(key), "%s_rewind", bm->forceTracerInput[tracerIndex].variableName);
			readkeyprm_i(fp, key, &bm->forceTracerInput[tracerIndex].rewind);

            if (bm->use_weighted_assim) {
                snprintf(key, sizeof(key), "%s_wgt_coefft", bm->forceTracerInput[tracerIndex].variableName);
                
                readkeyprm_d(fp, key, &bm->forceTracerInput[tracerIndex].wgt_coefft);
            } else { // No use of assimilation weighting so the read-in value gets all the weigting
                bm->forceTracerInput[tracerIndex].wgt_coefft = 1.0;
            }
                        
            if(any_use_resets > 0) {
                snprintf(key, sizeof(key), "%s_ResetTol", bm->forceTracerInput[tracerIndex].variableName);
                readkeyprm_d(fp, key, &bm->forceTracerInput[tracerIndex].ResetTol);
            } else {
                bm->forceTracerInput[tracerIndex].ResetTol = 0.0;
            }
            
            bm->forceTracerInput[tracerIndex].total_input = 0.0;
			bm->forceTracerInput[tracerIndex].curFile = 0;

			free(buf[tracerIndex]);
		}
		free(buf);
	}
}

/**
 * Free up memory associated with a physical property structure.
 */
void free_PhyPropertyData(MSEBoxModel *bm,PhyPropertyData *propInput){
	c_free2d(propInput->fname);
	c_free1d(propInput->variableName);
    i_free1d(propInput->use_resets);
	close_phyprop(bm, propInput);
}
/**
 * \brief This will close the temeprature and salinity files as well
 * as free up any memory allocated.
 *
 */
void freeTempSalt(MSEBoxModel *bm) {

	int tracerIndex;

	free(bm->TempLayerScalar);
	free(bm->SaltLayerScalar);
	free(bm->pHLayerScalar);

    if (bm->use_VertMixfiles) {
        free_PhyPropertyData(bm, &bm->VertMixinput);
    }
	if (bm->use_pHfiles) {
		free_PhyPropertyData(bm, &bm->pHinput);
	}
	if (bm->use_saltfiles) {
		free_PhyPropertyData(bm, &bm->saltinput);
	}
	if (bm->use_tempfiles) {
		free_PhyPropertyData(bm, &bm->tempinput);
	}
    if (bm->track_wind) {
        free_PhyPropertyData(bm, &bm->windinput);
    }
    if (bm->use_pollutantfiles) {
        free_PhyPropertyData(bm, &bm->noiseinput);
        free_PhyPropertyData(bm, &bm->lightpinput);
    }

	if (bm->use_forceTracers) {
		for (tracerIndex = 0; tracerIndex < bm->numForceTracers; tracerIndex++) {
			free_PhyPropertyData(bm, &bm->forceTracerInput[tracerIndex]);
		}
		free(bm->forceTracerInput);
	}
}

/* Routine to get the property data for this time step.
 * This may involve reading the netCDF input file if the
 * data is not already in the memory buffers
 */
void get_property(MSEBoxModel *bm, PhyPropertyData *propInput) {

	long offset = 0;
	long tstepnum = propInput->nextrec;
	int i, j, k;
	int totnz = bm->wcnz + bm->sednz;
	doubleINPUT *value;
	doubleINPUT ***array = NULL;
	doubleINPUT **array2D = NULL;

	ncopts = NC_VERBOSE | NC_FATAL;

    //printf("Doing %s\n", propInput->variableName);
    
	/* Have we finished this file? */
	if(strcmp(propInput->variableName, "noise_pollution") == 0)
		printf("tstepnum = %ld, propInput->nstep = %ld, propInput->curFile= %d, propInput->nFiles= %d\n", tstepnum, propInput->nstep, propInput->curFile, propInput->nFiles);


    //printf("Dealing with file %s\n", propInput->fname[propInput->curFile]);
    //fprintf(bm->logFile, "Dealing with file %s\n", propInput->fname[propInput->curFile]);
    
	if (tstepnum >= propInput->nstep) {

		/* Are we at the end of the files  - check to see if we should be rewinding*/
		if((propInput->curFile + 1 >= propInput->nFiles) && propInput->rewind == FALSE){
			fprintf(bm->logFile, "Returning prop %s\n",  propInput->variableName);
			propInput->atEnd = TRUE;
			return;
		}

		/* Yes - close it and open the next one */
		close_phyprop(bm, propInput);

		propInput->curFile = (propInput->curFile + 1) % propInput->nFiles;
        
        //fprintf(bm->logFile, "Time: %e set %s curFile to %d which is %s with use_rests: %d\n", bm->dayt, propInput->variableName, propInput->curFile, propInput->fname[propInput->curFile], propInput->use_resets[propInput->curFile]);

		open_phyprop(bm, propInput);
		tstepnum = propInput->nextrec;
	}

	value = alloc1dInput(propInput->nbuf);

	/* Is the requested data already in the memory buffers? */
	if (tstepnum < propInput->bufstart || tstepnum > propInput->bufend) {
		long start[3];
		long count[3];

		/* only allocate the data array that we are going to use */
		if(propInput->is_valid_z == TRUE){
			array = alloc3dInput(bm->wcnz + bm->sednz, bm->nbox, min(propInput->nbuf, propInput->nstep-tstepnum));
		}else{
			array2D = alloc2dInput(bm->nbox, min(propInput->nbuf, propInput->nstep-tstepnum));
		}

		/* Data must be read from file, so we might as well
		 * read propInput->nstep records, starting at the requested
		 * record number (ensuring, of course, that we don't try
		 * to read past the last record in the file).
		 */
		
		start[0] = tstepnum;
		start[1] = 0;
		start[2] = 0;

		count[0] = min(propInput->nbuf, propInput->nstep-tstepnum);
		count[1] = bm->nbox;
		count[2] = totnz;

		ncvarget(propInput->fid, propInput->t_vid, start, count, value);
		for (i = 0; i < propInput->nbuf; i++)
			propInput->tbuf[i] = (double) value[i];

		/*if(propInput->is_valid_z == TRUE){
			ncvarget(propInput->fid, propInput->prop_vid, start, count, array[0][0]);
		}else{
			ncvarget(propInput->fid, propInput->prop_vid, start, count, array2D[0]);
		}*/

        //fprintf(bm->logFile, "Time: %e %s in %s with t indx: %ld, nbox: %ld, totnz: %ld\n", bm->dayt, propInput->variableName, propInput->fname[propInput->curFile], count[0], count[1], count[2]);
        
		if(propInput->is_valid_z == FALSE){
			ncvarget(propInput->fid, propInput->prop_vid, start, count, array2D[0]);
		    for (i = 0; i < count[0]; i++) {
			    for (j = 0; j < count[1]; j++) {

					/* just use the 0 z slot */
					propInput->valuebuf[i][j][0] = (double) array2D[i][j];

					/* Check the value */
					if(propInput->missing_value_set){
						if (propInput->valuebuf[i][j][0] != propInput->missing_value){
							if (propInput->valuebuf[i][j][0] > propInput->max_value || propInput->valuebuf[i][j][0] < propInput->min_value) {
								quit("Hydro property with FALSE is_valid_z with missing_value %s value %f from slot 0 is outside the allowed bounds of %f - %f in box %d, layer %d, t %d. \n", propInput->variableName, propInput->valuebuf[i][j][0], propInput->min_value, propInput->max_value, j, 0, i);
							}
						}
					}
					if (propInput->valuebuf[i][j][0] > propInput->max_value || propInput->valuebuf[i][j][0] < propInput->min_value) {
						quit("Hydro property with FALSE is_valid_z %s value %f from slot 0 is outside the allowed bounds of %f - %f in box %d, layer %d, t %d. \n", propInput->variableName,
								propInput->valuebuf[i][j][0], propInput->min_value, propInput->max_value, j, 0, i);
					}
                }
            }

		} else {

            ncvarget(propInput->fid, propInput->prop_vid, start, count, array[0][0]);
            for (i = 0; i < count[0]; i++) {
			    for (j = 0; j < count[1]; j++) {
					for (k = 0; k < count[2]; k++) {
						propInput->valuebuf[i][j][k] = (double) array[i][j][k];

						/* Check the value */
						if(propInput->missing_value_set){
							if (propInput->valuebuf[i][j][k] != propInput->missing_value){
								if (propInput->valuebuf[i][j][k] > propInput->max_value || propInput->valuebuf[i][j][k] < propInput->min_value) {
									quit("Hydro property with is_valid_z and missing_value set %s value %f from slot k %d is outside the allowed bounds of %f - %f in box %d, layer %d, t %d. \n", propInput->variableName, propInput->valuebuf[i][j][k], k, propInput->min_value, propInput->max_value, j, k, i);
								}
							}
						}
						if (propInput->valuebuf[i][j][k] > propInput->max_value || propInput->valuebuf[i][j][k] < propInput->min_value) {
							quit("Hydro property with is_valid_z %s value %f from slot k %d  is outside the allowed bounds of %f - %f in box %d, layer %d, t %d. \n", propInput->variableName, propInput->valuebuf[i][j][k], k, propInput->min_value, propInput->max_value, j, k, i);
						}
					}
				}
			}
		}

		propInput->bufstart = tstepnum;
		propInput->bufend = tstepnum + count[0] - 1;

		if(propInput->is_valid_z == TRUE){
			free3dInput(array);
		}else{
			free2dInput(array2D);
		}
	}

	/* Data must now be in buffers, so adjust pointers */
	offset = tstepnum - propInput->bufstart;

	propInput->dataBuffer = propInput->valuebuf[offset];

	/* Store hydrodynamic time value */
	propInput->t = propInput->tbuf[offset] + propInput->dt / 2.0;

	/* Reset time left for this hydrodynamic step */
	propInput->tleft = propInput->dt;

	/* Increment next record number */
	propInput->nextrec++;

	/* Reset the checked array */
	for (i = 0; i < bm->nbox; i++) {
		for (j = 0; j < totnz; j++) {
			bm->checkedalready[i][j] = 0;
		}
	}

	free1dInput(value);
}

/* Routine to open a hydrodynamic input file and check
 * that it has appropriate structure.
 */

/* Maximum amount of memory to allocate for exchange values */
#define MAXBUFMEM (2L*1024L*1024L)

void open_phyprop(MSEBoxModel *bm, PhyPropertyData *propInput) {
	int ndims = 0;
	int nvars = 0;
	int natts = 0;
	int recdim = 0;
	int len = 0;
	long n = 0;
	nc_type daty;
	int dims[MAX_NC_DIMS];
	char hdu[BMSLEN];
	char bmu[BMSLEN];
	long bytes = 0;
	doubleINPUT value;
	int status;

	/* Set netCDF library error handling */
	ncopts = NC_VERBOSE;

	/* Open the file */

	if ((propInput->fid = Util_ncopen(bm->inputFolder, propInput->fname[propInput->curFile], NC_NOWRITE)) < 0)
		quit("open_phyprop: Can't open netcdf input data file %s\n", propInput->fname[propInput->curFile]);

	/* Inquire about this file */
	ncopts = NC_VERBOSE; // | NC_FATAL;
	ncinquire(propInput->fid, &ndims, &nvars, &natts, &recdim);
	if(propInput->is_valid_z == TRUE){
		if (ndims < 3)
			quit("open_phyprop: not enough dimensions in %s\n", propInput->fname[propInput->curFile]);
	}else{
		if (ndims < 2)
			quit("open_phyprop: not enough dimensions in %s\n", propInput->fname[propInput->curFile]);
	}
	if (nvars < 2)
		quit("open_phyprop: not enough variables in %s\n", propInput->fname[propInput->curFile]);

	/* Check dimensions are as expected */
	if ((propInput->t_did = ncdimid(propInput->fid, "t")) == -1)
		quit("open_phyprop: no t dimension in %s\n", propInput->fname[propInput->curFile]);
	if (propInput->t_did != recdim)
		quit("open_phyprop: t dimension not unlimited in %s\n", propInput->fname[propInput->curFile]);
	if ((propInput->b_did = ncdimid(propInput->fid, "b")) == -1)
		quit("open_phyprop: no b dimension in %s\n", propInput->fname[propInput->curFile]);
	/* if this value is valid in z dimemension then include as well */
	if(propInput->is_valid_z == TRUE){
		if ((propInput->z_did = ncdimid(propInput->fid, "z")) == -1)
			quit("open_phyprop: no z dimension in %s\n", propInput->fname[propInput->curFile]);
	}

	/* Get dimension sizes and check against geometry */
	ncdiminq(propInput->fid, propInput->b_did, NULL, &n);
	if (n != bm->nbox)
		quit("open_phyprop: Number of boxes (%ld) in %s doesn't match geometry (%d)\n", n, propInput->fname[propInput->curFile], bm->nbox);

	if(propInput->is_valid_z == TRUE){
		ncdiminq(propInput->fid, propInput->z_did, NULL, &n);
		if (n != bm->wcnz + bm->sednz)
			quit("open_phyprop: Number of layers (%ld) in %s doesn't match geometry (%d)\n", n, propInput->fname[propInput->curFile], (bm->wcnz + bm->sednz));
	}

	/* Check that time units and steps match this model */
	propInput->t_vid = ncvarid(propInput->fid, "t");
	if (propInput->t_vid < 0)
		quit("open_phyprop: no t variable in %s\n", propInput->fname[propInput->curFile]);

	memset(propInput->t_units, 0, BMSLEN);
	ncattget(propInput->fid, propInput->t_vid, "units", propInput->t_units);
	sscanf(propInput->t_units, "%s", hdu);
	sscanf(bm->t_units, "%s", bmu);
	if (strcmp(hdu, bmu) != 0)
		quit("open_phyprop: Time units (%s) don't match model time units (%s)\n", hdu, bmu);

	ncattinq(propInput->fid, propInput->t_vid, "dt", &daty, &len);
	if (nctypelen(daty) != sizeof(doubleINPUT))
		quit("open_phyprop: temperature dt attribute wrong type\n");

	ncattget(propInput->fid, propInput->t_vid, "dt", &value);
	propInput->dt = (double) value;
	if (propInput->dt <= 0.0)
		quit("open_phyprop: %s dt attribute must have positive value\n", propInput->variableName);

	/* Find out how many time steps are in the file */
	ncdiminq(propInput->fid, propInput->t_did, NULL, &propInput->nstep);

	/* Get other variable ids  - need to allow the user to specify as many tracers as they want in a single file.*/
	propInput->prop_vid = ncvarid(propInput->fid, propInput->variableName);
    if (propInput->prop_vid < 0) {
        quit("open_phyprop: no %s variable in %s\n", propInput->variableName, propInput->fname[propInput->curFile]);
    }
    
	/* Check variable types and dimensions */
	ncvarinq(propInput->fid, propInput->prop_vid, NULL, &daty, &ndims, dims, &natts);
	if (nctypelen(daty) != sizeof(doubleINPUT))
		quit("open_phyprop: Type of %s variable doesn't match model\n",  propInput->variableName);
	if(propInput->is_valid_z == TRUE){
		if (ndims != 3 || dims[0] != propInput->t_did || dims[1] != propInput->b_did || dims[2] != propInput->z_did)
			quit("open_phyprop: %s variable has incorrect dimensions\n", propInput->variableName);
	}else{
		if (ndims != 2 || dims[0] != propInput->t_did || dims[1] != propInput->b_did)
			quit("open_phyprop: %s variable has incorrect dimensions\n", propInput->variableName);
	}
	/* Calculate a reasonable number of time steps
	 * to store in buffers in memory
	 */
	bytes = (long int)sizeof(double) * (long int)bm->nbox * (long int)bm->wcnz;
	propInput->nbuf = min(MAXBUFMEM/bytes, propInput->nstep);
	if (propInput->nbuf < 1)
		propInput->nbuf = 1;

	/* Allocate memory for data buffers */
	propInput->tbuf = alloc1d(propInput->nbuf);
	if(propInput->is_valid_z == TRUE){
		propInput->valuebuf = (double ***) alloc3d(bm->wcnz + bm->sednz, bm->nbox, propInput->nbuf);
	}else{
		/* allocate a single value in the z dimemsion for easy code resue */
		propInput->valuebuf = (double ***) alloc3d(1, bm->nbox, propInput->nbuf);
	}

	/* Signify that buffers are empty */
	propInput->bufstart = -1;
	propInput->bufend = -1;
	propInput->nextrec = 0;
	propInput->atEnd = FALSE;


	/* Get the missing information - only needed for tracers other than temperature, salinity pH and swr */
	if(strcmp(propInput->variableName, "temperature") != 0 && strcmp(propInput->variableName, "salinity") != 0 && strcmp(propInput->variableName, "pH") != 0 && strcmp(propInput->variableName, "swr") != 0){
		status = ncattinq(propInput->fid, propInput->prop_vid, "missing_value", &daty, &len);
		if (nctypelen(daty) != sizeof(doubleINPUT))
			quit("open_phyprop: %s missing_value attribute wrong type\n", propInput->variableName);
		if (status >= 0){
			/* go ahead and grab the value */
			status = ncattget(propInput->fid, propInput->prop_vid, "missing_value",&propInput->missing_value);
			propInput->missing_value_set = 1;
		} else{
			propInput->missing_value_set = 0;
			propInput->missing_value = -1; /* Set so we are not referencing invalid memory */
		}

		status = ncattinq(propInput->fid, propInput->prop_vid, "valid_max", &daty, &len);
		if (nctypelen(daty) != sizeof(doubleINPUT))
			quit("open_phyprop: %s valid_max attribute wrong type\n", propInput->variableName);

		if (status >= 0){
			/* go ahead and grab the value */
			status = ncattget(propInput->fid, propInput->prop_vid, "valid_max", &propInput->max_value);
		}

		status = ncattinq(propInput->fid, propInput->prop_vid, "valid_min", &daty, &len);
		if (nctypelen(daty) != sizeof(doubleINPUT))
			quit("open_phyprop: %s valid_min attribute wrong type\n", propInput->variableName);

		if (status >= 0){
			/* go ahead and grab the value */
			status = ncattget(propInput->fid, propInput->prop_vid, "valid_min", &propInput->min_value);
		}

		/* Grab the units of the tracer and check that its the same as the units of the tracer in the model */

		memset(propInput->tracer_units, 0, BMSLEN);
		ncattget(propInput->fid, propInput->prop_vid, "units", propInput->tracer_units);


		printf("propInput->tracer_units = %s\n",propInput->tracer_units);
        if (propInput->isScalar) {
            if(strcmp(propInput->tracer_units, "") != 0){
                quit("open_phyprop: Incompatible units in tracer forcing file %s for scalar %s. Units for scalar are '' and units in forcing file are '%s'.",
                     propInput->fname[propInput->curFile], propInput->variableName, propInput->tracer_units);
            }
        } else if(propInput->isEpi == TRUE) {
            if(strcmp(propInput->tracer_units, bm->einfo[propInput->tracerID].units) != 0){
                quit("open_phyprop: Incompatible units in tracer forcing file %s for tracer %s. Units for tracer are '%s' and units in forcing file are '%s'.",
                     propInput->fname[propInput->curFile], propInput->variableName, bm->tinfo[propInput->tracerID].units, propInput->tracer_units);
            }
        } else {
            if(strcmp(propInput->tracer_units, bm->tinfo[propInput->tracerID].units) != 0){
                quit("open_phyprop: Incompatible units in tracer forcing file %s for tracer %s. Units for tracer are '%s' and units in forcing file are '%s'.",
                     propInput->fname[propInput->curFile], propInput->variableName, bm->tinfo[propInput->tracerID].units, propInput->tracer_units);
            }
        }
	}


	/* Reset netCDF error handling */
	ncopts = NC_VERBOSE | NC_FATAL;
}

/* Routine to close an already open hydrodynamic input file */
void close_phyprop(MSEBoxModel *bm, PhyPropertyData *propInput) {

	/* Close salinity file */
	if (propInput->fid >= 0)
		ncclose(propInput->fid);
	propInput->fid = -1;

	/* Free buffers */
	free1d(propInput->tbuf);
	free3d((double ***) propInput->valuebuf);

	propInput->tbuf = NULL;
	propInput->valuebuf = NULL;
}

/*********************************************************************/
void tracerForcingBM(MSEBoxModel *bm, double ***newwc, double ***newsedtr, PhyPropertyData *inputData) {
	int b = 0;
	int k = 0;
	double tleft = bm->dt;
	double dt, cur_value, new_value;
	int totnz = bm->wcnz + bm->sednz;
	int i, j;

	if (verbose)
		fprintf(stderr, "Entering tracerForcingBM and doing %s ID %d\n", inputData->variableName, inputData->tracerID);

	if(inputData->atEnd)
		return;
    
	/* Loop while more time remains in this transport time step */
	while (tleft > 0) {

		/* Get more data if necessary */
		if (inputData->tleft <= 0){
            
            //printf("get property\n");
			
            get_property(bm, inputData);
			if(inputData->atEnd == TRUE)
				return;
		}


		/* Calculate time step allowed */
		dt = min(tleft, inputData->tleft);

       // printf("dt now: %e\n", dt);
        
		/* Loop over each box to check the values */
		for (b = 0; b < bm->nbox; b++) {
			Box *bp = &bm->boxes[b];
			if(inputData->is_valid_z == TRUE){
				/* Loop through water column */
				for (k = 0; k < bp->nz; k++) {
					if(inputData->missing_value_set == FALSE || inputData->dataBuffer[b][k] != inputData->missing_value){
                       //newwc[b][k][inputData->tracerID] = inputData->dataBuffer[b][k];
                        cur_value = newwc[b][k][inputData->tracerID];
                        
                        if(inputData->wgt_coefft < 0) {
                            new_value = cur_value + inputData->dataBuffer[b][k];
                        } else {
                            new_value = ((1.0 - inputData->wgt_coefft) * cur_value + inputData->wgt_coefft *
                                         inputData->dataBuffer[b][k]);
                        }
                        newwc[b][k][inputData->tracerID] = new_value;
                        inputData->total_input += ((new_value - cur_value) * bp->dz[k]);
                                                
						if (!_finite(newwc[b][k][inputData->tracerID])) {
							quit("tracerForcingBM - %s in %d:%d at time %e is not finite %e\n", inputData->variableName, b, k, bm->dayt, newwc[b][k][inputData->tracerID]);
						}
					}
				}
				/* Loop through sediment columns */
				for (k = 0; k < bp->sm.nz; k++) {

					if(inputData->missing_value_set == FALSE || inputData->dataBuffer[b][k + bm->wcnz] != inputData->missing_value){
                        //newsedtr[b][k][inputData->tracerID] = inputData->dataBuffer[b][k + bm->wcnz];
                        cur_value = newsedtr[b][k][inputData->tracerID];
                        if(inputData->wgt_coefft < 0) {
                            new_value = cur_value + inputData->dataBuffer[b][k + bm->wcnz];
                        } else {
                            new_value = ((1.0 - inputData->wgt_coefft) * cur_value + inputData->wgt_coefft * inputData->dataBuffer[b][k + bm->wcnz]);
                        }
                        newsedtr[b][k][inputData->tracerID] = new_value;
                        
                        inputData->total_input += ((new_value - cur_value) * bp->dz[k]);

						if (!_finite(newsedtr[b][k][inputData->tracerID])) {
							quit("tracerForcingBM - %s in %d:%d at time %e sed value is not finite %e\n", b, k, bm->dayt, inputData->variableName, newsedtr[b][k][inputData->tracerID]);
						}
						//fprintf(bm->logFile, "Time %e: %s in box %d:sediment layer %d is set to %e, inputData->tleft= %e, rewind = %d\n",
						//		bm->dayt, inputData->variableName, b, k, newsedtr[b][k][inputData->tracerID], inputData->tleft, inputData->rewind);
					}
				}
			} else {

				/* We are just looking at 1D data */

				/* Check the value read in is valid - otherwise quit now - non NAN values allowed ! */
				if (!_finite(inputData->dataBuffer[b][0])) {
					quit("tracerForcingBM - %s in %d:%d at time %e has non finite %e input value\n", b, k, bm->dayt, inputData->variableName, inputData->dataBuffer[b][0]);
				}

				if(inputData->isBoxValue == TRUE){
					switch(inputData->boxValueIndex){
						case EDDY_ID:
							bm->boxes[b].eddy = inputData->dataBuffer[b][0];
							break;
                        case VERTMIX_ID:
                            bm->boxes[b].vmix_scale = inputData->dataBuffer[b][0];
                            break;
					}

				}
				else{
					if(inputData->missing_value_set == FALSE || inputData->dataBuffer[b][0] != inputData->missing_value){
						if(inputData->isEpi == TRUE){
							/* Not 100% sure about using this array here */
							bm->epi[b][inputData->tracerID] = inputData->dataBuffer[b][0];

						}else{
							newwc[b][0][inputData->tracerID] = inputData->dataBuffer[b][0];
						}
						/* fprintf(bm->logFile, "Time %e: %s in box %d:%d is set to %e, inputData->tleft= %e, rewind = %d\n",
								bm->dayt, inputData->variableName, b, 0, newwc[b][0][inputData->tracerID], inputData->tleft, inputData->rewind);*/
					}
				}
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
	/* Reset the checked array */
	for (i = 0; i < bm->nbox; i++) {
		for (j = 0; j < totnz; j++) {
			bm->checkedalready[i][j] = 0;
		}
	}
    
    if(verbose) {
        printf("Leaving tracerForcingBM\n");
    }
    
    return;

}

/*********************************************************************/
void resetForcingBM(MSEBoxModel *bm, double ***newwc, double ***newsedtr, PhyPropertyData *inputData) {
    int b = 0;
    int k = 0;
    double tleft = bm->dt;
    double dt, cur_value, new_value, tol_valueA, tol_valueB;
    int totnz = bm->wcnz + bm->sednz;
    int i, j;

    if (verbose)
        fprintf(stderr, "Entering resetForcingBM and doing %s ID %d\n", inputData->variableName, inputData->tracerID);

    if(inputData->atEnd)
        return;

    /* Loop while more time remains in this transport time step */
    while (tleft > 0) {

        /* Get more data if necessary */
        if (inputData->tleft <= 0){
            
            //printf("get property\n");
            
            get_property(bm, inputData);
            if(inputData->atEnd == TRUE)
                return;
        }


        /* Calculate time step allowed */
        dt = min(tleft, inputData->tleft);

       // printf("dt now: %e\n", dt);
        
        /* Loop over each box to check the values */
        for (b = 0; b < bm->nbox; b++) {
            Box *bp = &bm->boxes[b];
            if(inputData->is_valid_z == TRUE){
                /* Loop through water column */
                for (k = 0; k < bp->nz; k++) {
                    if (inputData->missing_value_set == FALSE || inputData->dataBuffer[b][k] != inputData->missing_value){
                       //newwc[b][k][inputData->tracerID] = inputData->dataBuffer[b][k];
                        cur_value = newwc[b][k][inputData->tracerID];

                        if(inputData->wgt_coefft < 0) {
                            new_value = cur_value + inputData->dataBuffer[b][k];
                        } else {
                            new_value = ((1.0 - inputData->wgt_coefft) * cur_value + inputData->wgt_coefft * inputData->dataBuffer[b][k]);
                        }
                        tol_valueA = (1.0 - inputData->ResetTol) * new_value;
                        tol_valueB = (1.0 + inputData->ResetTol) * new_value;
                        
                        if((cur_value < tol_valueA) || (cur_value > tol_valueB)) {
                            newwc[b][k][inputData->tracerID] = new_value;
                            inputData->total_input += (new_value * bp->dz[k]);
                        }

                        if (!_finite(newwc[b][k][inputData->tracerID])) {
                            quit("resetForcingBM - %s in %d:%d at time %e is not finite %e\n", inputData->variableName, b, k, bm->dayt, newwc[b][k][inputData->tracerID]);
                        }
                        //fprintf(bm->logFile, "Time %e: %s in box %d:%d is set to %e, inputData->tleft= %e, rewind = %d, new_value: %e, cur_value: %e, tol_valueA: %e, tol_valueB: %e\n", bm->dayt, inputData->variableName, b, k, newwc[b][k][inputData->tracerID], inputData->tleft, inputData->rewind, new_value, cur_value, tol_valueA, tol_valueB);
                    }
                }
                /* Loop through sediment columns */
                for (k = 0; k < bp->sm.nz; k++) {

                    if(inputData->missing_value_set == FALSE || inputData->dataBuffer[b][k + bm->wcnz] != inputData->missing_value){
                        //newsedtr[b][k][inputData->tracerID] = inputData->dataBuffer[b][k + bm->wcnz];
                        cur_value = newsedtr[b][k][inputData->tracerID];
                        
                        if(inputData->wgt_coefft < 0) {
                            new_value = cur_value + inputData->dataBuffer[b][k + bm->wcnz];
                        } else {
                            new_value = ((1.0 - inputData->wgt_coefft) * cur_value + inputData->wgt_coefft * inputData->dataBuffer[b][k + bm->wcnz]);
                        }
                        tol_valueA = (1.0 - inputData->ResetTol) * new_value;
                        tol_valueB = (1.0 + inputData->ResetTol) * new_value;
                        
                        if((cur_value < tol_valueA) || (cur_value > tol_valueB)) {
                            newsedtr[b][k][inputData->tracerID] = new_value;
                            inputData->total_input += (new_value * bp->sm.dz[k]);
                            
                           // fprintf(bm->logFile, "Time: %e box%d-%d in sed %s added = %e with total_input %e\n", bm->dayt, b, k, inputData->variableName, new_value, inputData->total_input);
                        }

                        if (!_finite(newsedtr[b][k][inputData->tracerID])) {
                            quit("resetForcingBM - %s in %d:%d sed value at time %e is not finite %e\n", b, k, bm->dayt, inputData->variableName, newsedtr[b][k][inputData->tracerID]);
                        }
                        //fprintf(bm->logFile, "Time %e: %s in box %d:sediment layer %d is set to %e, inputData->tleft= %e, rewind = %d, new_value: %e, cur_value: %e, tol_valueA: %e, tol_valueB: %e\n", bm->dayt, inputData->variableName, b, k, newsedtr[b][k][inputData->tracerID], inputData->tleft, inputData->rewind, new_value, cur_value, tol_valueA, tol_valueB);
                    }
                }
            } else {

                /* We are just looking at 1D data */

                /* Check the value read in is valid - otherwise quit now - non NAN values allowed ! */
                if (!_finite(inputData->dataBuffer[b][0])) {
                    quit("resetForcingBM - %s in %d:%d at time %e is non finite %e input\n", b, k, bm->dayt, inputData->variableName, inputData->dataBuffer[b][0]);
                }

                if(inputData->isBoxValue == TRUE){
                    switch(inputData->boxValueIndex){
                        case EDDY_ID:
                            bm->boxes[b].eddy = inputData->dataBuffer[b][0];
                            break;
                        case VERTMIX_ID:
                            bm->boxes[b].vmix_scale = inputData->dataBuffer[b][0];
                            break;
                    }

                }
                else{
                    if(inputData->missing_value_set == FALSE || inputData->dataBuffer[b][0] != inputData->missing_value){
                        if(inputData->isEpi == TRUE){
                            /* Not 100% sure about using this array here */
                            bm->epi[b][inputData->tracerID] = inputData->dataBuffer[b][0];

                        }else{
                            newwc[b][0][inputData->tracerID] = inputData->dataBuffer[b][0];
                        }
                        /* fprintf(bm->logFile, "Time %e: %s in box %d:%d is set to %e, inputData->tleft= %e, rewind = %d\n",
                                bm->dayt, inputData->variableName, b, 0, newwc[b][0][inputData->tracerID], inputData->tleft, inputData->rewind);*/
                    }
                }
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
    /* Reset the checked array */
    for (i = 0; i < bm->nbox; i++) {
        for (j = 0; j < totnz; j++) {
            bm->checkedalready[i][j] = 0;
        }
    }
    
}

