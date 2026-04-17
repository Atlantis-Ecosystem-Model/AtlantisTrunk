/**
 \file
 \brief C file for updating mvoement distributions
 \ingroup atecology
 File:        atForcedMovement.c
 Created:    1/8/2021
 Author:        Beth Fulton,   CSIRO
 Purpose:    C file for updating the distrubutions for running the movement routines - expanded to be dynamic.

 Revisions:  See SVN repository
 
 ************************************************************************************************/

/*  Open library routines */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <netcdf.h>
#include "atecology.h"
#include <atHarvestLib.h>

void Ecology_Update_Move_Entry(MSEBoxModel *bm, FILE *llogfp);
void get_move_property(MSEBoxModel *bm, PhyPropertyData *propInput);

void open_move_prop(MSEBoxModel *bm, PhyPropertyData *propInput);
void close_move_prop(MSEBoxModel *bm, PhyPropertyData *propInput);

/* Routine to open a hydrodynamic input file and check
 * that it has appropriate structure.
 */

/**
 *    Read in information about any forcing of movement entries
 *
 *
 */
void init_forceMoveEntries(MSEBoxModel *bm, FILE *fp) {
    int i, stage;
    int stageID;
    char key[BMSLEN];
    char checkname[BMSLEN];
    int tracerIndex;
    char **buf;
    int size, speciesIndex = 0;
    int nFiles = 1; //Too hard to do current distribution and next distribution if split over multiple files

    /* Read the list of files */
    set_keyprm_errfn(warn);
    readkeyprm_i(fp, "use_move_entries", &bm->use_move_entries);
    set_keyprm_errfn(quit);

    if (bm->use_move_entries) {
        //printf("use_move_entries: %d\n", bm->use_move_entries);

        /* Read the number of files */
        readkeyprm_i(fp, "nforceMoveGroups", &bm->numForceMoveEntries);
        if (bm->numForceMoveEntries < 1)
            quit("init_forceMoveEntries: You have set use_move_entries to 1 so there must be a forcing distribution file in %s and at least one group defined as moving (nforceMoveGroups must be 1 or higher)\n", bm->forceIfname);
        
        /* Read in the names of the tracers */
        readkeyprm_sarray(fp, "MoveGroupCodes", &buf, &size);
        bm->forceMoveEntryInput = (PhyPropertyData *) malloc(sizeof(PhyPropertyData) * (long unsigned int)bm->numForceMoveEntries);
        
        /* Now read in the files for each tracer */
        for (tracerIndex = 0; tracerIndex < bm->numForceMoveEntries; tracerIndex++) {
            
            bm->forceMoveEntryInput[tracerIndex].fname = c_alloc2d(BMSLEN, nFiles);
            bm->forceMoveEntryInput[tracerIndex].variableName = c_alloc1d(BMSLEN);
            bm->forceMoveEntryInput[tracerIndex].isEpi = FALSE;    /* NA */
            bm->forceMoveEntryInput[tracerIndex].isBoxValue = FALSE;    /* To be filled */
            bm->forceMoveEntryInput[tracerIndex].isScalar = TRUE;  /* So we can test per species */

            /* property to specify if this is a 2D or 3D property - assume for now all distributions are 2D. */
            /* if required in the future we can allow users to specify */
            bm->forceMoveEntryInput[tracerIndex].is_valid_z = FALSE;
            strcpy(bm->forceMoveEntryInput[tracerIndex].variableName, buf[tracerIndex]);
            
            bm->forceMoveEntryInput[tracerIndex].tracerID = -1;
            stageID = 0;
            for (i = 0; i < bm->K_num_tot_sp; i++) {
                for (stage = 0; stage < FunctGroupArray[i].numStages; stage++) {
                    stageID = stage;
                    snprintf(checkname, sizeof(checkname), "%s_stage_%d", FunctGroupArray[i].groupCode, stageID);
                    if(strcmp(checkname, bm->forceMoveEntryInput[tracerIndex].variableName) == 0){
                        bm->forceMoveEntryInput[tracerIndex].tracerID = i;
                        bm->forceMoveEntryInput[tracerIndex].stageID = stage;
                        break;
                    }
                }
            }

            snprintf(key, sizeof(key), "%s_File.name", bm->forceMoveEntryInput[tracerIndex].variableName);
            readkeyprm_s(fp, key, bm->forceMoveEntryInput[tracerIndex].fname[0]);
            
            snprintf(key, sizeof(key), "%s_tstart", bm->forceMoveEntryInput[tracerIndex].variableName);
            readkeyprm_d(fp, key, &bm->forceMoveEntryInput[tracerIndex].movetstart);
            
            bm->forceMoveEntryInput[tracerIndex].rewind = 0;
            bm->forceMoveEntryInput[tracerIndex].wgt_coefft = 1.0;
            bm->forceMoveEntryInput[tracerIndex].curFile = 0;
            bm->forceMoveEntryInput[tracerIndex].ResetTol = 0.0;
            bm->forceMoveEntryInput[tracerIndex].total_input = 0.0;
            
            bm->forceMoveEntryInput[tracerIndex].use_resets = (int *)i_alloc1d(nFiles);
            bm->forceMoveEntryInput[tracerIndex].use_resets[0] = 0;
            
            
            /* Set the min and max values */
            bm->forceMoveEntryInput[tracerIndex].min_value = 0;
            bm->forceMoveEntryInput[tracerIndex].max_value = 1.0;
            
            /* Setting move index */
            speciesIndex = bm->forceMoveEntryInput[tracerIndex].tracerID;
            FunctGroupArray[speciesIndex].moveEntryIndex = 0;
            FunctGroupArray[speciesIndex].next_moveEntryIndex = 1;
            
            bm->forceMoveEntryInput[tracerIndex].moveInitDone = 0;

            free(buf[tracerIndex]);
        }
        free(buf);
    }

}

void open_move_prop(MSEBoxModel *bm, PhyPropertyData *propInput) {
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
    //long bytes = 0;
    doubleINPUT value;
    int status;

    /* Set netCDF library error handling */
    ncopts = NC_VERBOSE;

    /* Open the file */
    printf("open_move_prop - opening %s", propInput->fname[propInput->curFile]);
    if ((propInput->fid = Util_ncopen(bm->inputFolder, propInput->fname[propInput->curFile], NC_NOWRITE)) < 0)
        quit("open_move_prop: Can't open netcdf input data file %s\n", propInput->fname[propInput->curFile]);

    /* Inquire about this file */
    ncopts = NC_VERBOSE; // | NC_FATAL;
    ncinquire(propInput->fid, &ndims, &nvars, &natts, &recdim);
    if (ndims < 2) {
        quit("open_move_prop: not enough dimensions in %s - should have time and box dimensions (2D)\n", propInput->fname[propInput->curFile]);
    }
    if (nvars < 2)
        quit("open_move_prop: not enough variables in %s - needs a time entry and a distribution entry\n", propInput->fname[propInput->curFile]);

    /* Check dimensions are as expected */
    if ((propInput->t_did = ncdimid(propInput->fid, "t")) == -1)
        quit("open_move_prop: no t dimension in %s\n", propInput->fname[propInput->curFile]);
    if (propInput->t_did != recdim)
        quit("open_move_prop: t dimension not unlimited in %s\n", propInput->fname[propInput->curFile]);
    if ((propInput->b_did = ncdimid(propInput->fid, "b")) == -1)
        quit("open_move_prop: no b dimension in %s (i.e. no box dimension)\n", propInput->fname[propInput->curFile]);

    /* Get dimension sizes and check against geometry */
    ncdiminq(propInput->fid, propInput->b_did, NULL, &n);
    if (n != bm->nbox)
        quit("open_move_prop: Number of boxes (%ld) in %s doesn't match geometry (%d)\n", n, propInput->fname[propInput->curFile], bm->nbox);

    /* Check that time units and steps match this model */
    propInput->t_vid = ncvarid(propInput->fid, "t");
    if (propInput->t_vid < 0)
        quit("open_move_prop: no t variable in %s\n", propInput->fname[propInput->curFile]);

    memset(propInput->t_units, 0, BMSLEN);
    ncattget(propInput->fid, propInput->t_vid, "units", propInput->t_units);
    sscanf(propInput->t_units, "%s", hdu);
    sscanf(bm->t_units, "%s", bmu);
    if (strcmp(hdu, bmu) != 0)
        quit("open_move_prop: Time units (%s) don't match model time units (%s)\n", hdu, bmu);

    ncattinq(propInput->fid, propInput->t_vid, "dt", &daty, &len);
    if (nctypelen(daty) != sizeof(doubleINPUT))
        quit("open_move_prop: dt attribute wrong type\n");

    ncattget(propInput->fid, propInput->t_vid, "dt", &value);
    propInput->dt = (double) value;
    if (propInput->dt <= 0.0)
        quit("open_move_prop: %s dt attribute must have positive value\n", propInput->variableName);

    /* Find out how many time steps are in the file */
    ncdiminq(propInput->fid, propInput->t_did, NULL, &propInput->nstep);

    /* Get other variable ids  - need to allow the user to specify as many tracers as they want in a single file - though really only goinf to read one */
    propInput->prop_vid = ncvarid(propInput->fid, propInput->variableName);
    if (propInput->prop_vid < 0)
        quit("open_move_prop: no %s variable in %s\n", propInput->variableName, propInput->fname[propInput->curFile]);

    /* Check variable types and dimensions */
    ncvarinq(propInput->fid, propInput->prop_vid, NULL, &daty, &ndims, dims, &natts);
    if (nctypelen(daty) != sizeof(doubleINPUT))
        quit("open_move_prop: Type of %s variable doesn't match model assumptions (i.e. need double)\n",  propInput->variableName);
    if (ndims != 2 || dims[0] != propInput->t_did || dims[1] != propInput->b_did) {
        quit("open_move_prop: %s variable has incorrect dimensions\n", propInput->variableName);
    }
    /* Store one time step in memory */
    propInput->nbuf = 1;

    /* Allocate memory for data buffers */
    propInput->tbuf = alloc1d(propInput->nbuf);
    /* Allocate a single value in the z dimemsion for easy code resue */
    propInput->valuebuf = (double ***) alloc3d(1, bm->nbox, propInput->nbuf);

    /* Signify that buffers are empty */
    propInput->bufstart = -1;
    propInput->bufend = -1;
    propInput->nextrec = 0;
    propInput->atEnd = FALSE;


    /* Get the missing value */
    status = ncattinq(propInput->fid, propInput->prop_vid, "missing_value", &daty, &len);
    if (nctypelen(daty) != sizeof(doubleINPUT)){
        quit("open_move_prop: %s missing_value attribute wrong type\n", propInput->variableName);
    }
    if (status >= 0){
        /* go ahead and grab the value */
        status = ncattget(propInput->fid, propInput->prop_vid, "missing_value",&propInput->missing_value);
        propInput->missing_value_set = 1;
    } else {
        propInput->missing_value_set = 0;
        propInput->missing_value = -1; /* Set so we are not referencing invalid memory */
    }

    status = ncattinq(propInput->fid, propInput->prop_vid, "valid_max", &daty, &len);
    if (nctypelen(daty) != sizeof(doubleINPUT)){
        quit("open_move_prop: %s valid_max attribute wrong type\n", propInput->variableName);
    }
    if (status >= 0){
        /* go ahead and grab the value */
        status = ncattget(propInput->fid, propInput->prop_vid, "valid_max", &propInput->max_value);
    }


    status = ncattinq(propInput->fid, propInput->prop_vid, "valid_min", &daty, &len);
    if (nctypelen(daty) != sizeof(doubleINPUT)){
        quit("open_move_prop: %s valid_min attribute wrong type\n", propInput->variableName);
    }
    if (status >= 0){
        /* go ahead and grab the value */
        status = ncattget(propInput->fid, propInput->prop_vid, "valid_min", &propInput->min_value);
    }

    /* Grab the units of the tracer and check that its the same as the units of the tracer in the model */
    memset(propInput->tracer_units, 0, BMSLEN);
    ncattget(propInput->fid, propInput->prop_vid, "units", propInput->tracer_units);

    //printf("propInput->tracer_units = %s\n",propInput->tracer_units);

    /* Reset netCDF error handling */
    ncopts = NC_VERBOSE | NC_FATAL;
}

/* Routine to close an already open hydrodynamic input file */
void close_move_prop(MSEBoxModel *bm, PhyPropertyData *propInput) {

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

/**
 * \brief Read in a new chunk of the movement entry forcing file - get the movement data for this time period and next.
 * This may involve reading the netCDF input file if the
 * data is not already in the memory buffers
 * Logic for the buffer read is as follows
 *
 */
void get_move_property(MSEBoxModel *bm, PhyPropertyData *propInput) {

    long offset = 0;
    long tstepnum = propInput->nextrec;
    int i, j;
    doubleINPUT *value;
    doubleINPUT **array2D = NULL;

    ncopts = NC_VERBOSE | NC_FATAL;

    if(verbose)
        printf("Doing %s\n", propInput->variableName);
    
    /* Have we finished this file? */
    if (tstepnum >= propInput->nstep) {
        /* Are we at the end of the files  - assumes never rewind */
        if(propInput->curFile + 1 >= propInput->nFiles){
            propInput->atEnd = TRUE;
            return;
        }
    }

    value = alloc1dInput(propInput->nbuf);

    /* Is the requested data already in the memory buffers? */
    if (tstepnum < propInput->bufstart || tstepnum > propInput->bufend) {
        long start[3];
        long count[3];

        /* only allocate the data array that we are going to use */
        array2D = alloc2dInput(bm->nbox, min(propInput->nbuf, propInput->nstep-tstepnum));

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
        count[2] = 1; // As assuming a 2D array only
        
       ncvarget(propInput->fid, propInput->t_vid, start, count, value);
        for (i = 0; i < propInput->nbuf; i++)
            propInput->tbuf[i] = (double) value[i];

        ncvarget(propInput->fid, propInput->prop_vid, start, count, array2D[0]);
        for (i = 0; i < count[0]; i++) {
            for (j = 0; j < count[1]; j++) {

                /* just use the 0 z slot */
                propInput->valuebuf[i][j][0] = (double) array2D[i][j];

                /* Check the value */
                if (propInput->valuebuf[i][j][0] > propInput->max_value || propInput->valuebuf[i][j][0] < propInput->min_value) {
                    quit("Distribution being read in %s has value %f, which is outside the allowed bounds of %f - %f in box %d at t %d. \n", propInput->variableName,propInput->valuebuf[i][j][0], propInput->min_value, propInput->max_value, j, i);
                }
            }

        }
        
        propInput->bufstart = tstepnum;
        propInput->bufend = tstepnum + count[0] - 1;

       free2dInput(array2D);
    }

    /* Data must now be in buffers, so adjust pointers */
    offset = tstepnum - propInput->bufstart;

    propInput->dataBuffer = propInput->valuebuf[offset];

    /* Store hydrodynamic time value */
    propInput->t = propInput->tbuf[offset] + propInput->dt / 2.0;

    /* Reset time left for this step */
    propInput->tleft = propInput->dt;
    
    /* Increment next record number */
    propInput->nextrec++;

    free1dInput(value);

}

/**
 * \brief Set the new movement entries - from forcing file
 *
 */
void Init_full_move_array(MSEBoxModel *bm, PhyPropertyData *propInput, int tracerIndex){
    int speciesIndex, stageIndex, qrt, next_qrt, b;
    double new_value;
    
    if(verbose > 0)
        printf("Doing Init_full_move_array for %s (tracerIndex: %d)\n", bm->forceMoveEntryInput[tracerIndex].variableName, tracerIndex);
    
    speciesIndex = bm->forceMoveEntryInput[tracerIndex].tracerID;
    stageIndex = bm->forceMoveEntryInput[tracerIndex].stageID;
    qrt = FunctGroupArray[speciesIndex].moveEntryIndex;
    next_qrt = FunctGroupArray[speciesIndex].next_moveEntryIndex;
    
    //fprintf(bm->logFile, "Doing tracerIndex: %d speciesIndex: %d stageIndex: %d qrt: %d next_qrt: %d\n", tracerIndex, speciesIndex, stageIndex, qrt, next_qrt);

    /* Apply the new value per box - need current entry and next one as need qrt and qrt + 1 */
    //FunctGroupArray[sp].distrib[ij][qrt][stage]

    if(bm->forceMoveEntryInput[tracerIndex].atEnd)
        return;

    /* Get first set of values */
    get_move_property(bm, &(bm->forceMoveEntryInput[tracerIndex]));  // based on get_property()
    if(bm->forceMoveEntryInput[tracerIndex].atEnd == TRUE) {
        quit("Something has gone wrong as the file for %s-%d distributions is empty\n", FunctGroupArray[speciesIndex].groupCode, stageIndex);
    }
    
    /* Loop over each box to store this first set of values */
    for (b = 0; b < bm->nbox; b++) {
        new_value = bm->forceMoveEntryInput[tracerIndex].dataBuffer[b][0]; // Take the entry for next time period from the file

        if (!_finite(new_value)) {
            quit("%s in box %d is not finite %e for the first period\n", bm->forceMoveEntryInput[tracerIndex].variableName, b);
        }

        fprintf(bm->logFile, "Time: %e Doing b %d speciesIndex: %d qrt: %d stageIndex: %d new_value: %e\n", bm->dayt, b, speciesIndex, qrt, stageIndex, new_value);
        
        // Assign the values
        FunctGroupArray[speciesIndex].distrib[b][qrt][stageIndex] = new_value;
    }
    
    //* Now repeat for second value
    get_move_property(bm, &(bm->forceMoveEntryInput[tracerIndex]));  // based on get_property()
    if(bm->forceMoveEntryInput[tracerIndex].atEnd == TRUE) {
        warn("Really only supposed to hae a single distribution value in the file for %s-%d\n", FunctGroupArray[speciesIndex].groupCode, stageIndex);
        
        for (b = 0; b < bm->nbox; b++) {
            FunctGroupArray[speciesIndex].distrib[b][next_qrt][stageIndex] = FunctGroupArray[speciesIndex].distrib[b][qrt][stageIndex];
        }
    } else {
        /* As thee are valid values Loop over each box to store this second set of values */
        for (b = 0; b < bm->nbox; b++) {
            new_value = bm->forceMoveEntryInput[tracerIndex].dataBuffer[b][0]; // Take the entry for next time period from the file

            if (!_finite(new_value)) {
                quit("%s in box %d is not finite for the second period\n", bm->forceMoveEntryInput[tracerIndex].variableName, b);
            }

            fprintf(bm->logFile, "Time: %e Doing b %d speciesIndex: %d next_qrt: %d stageIndex: %d new_value: %e\n", bm->dayt, b, speciesIndex, next_qrt, stageIndex, new_value);

            // Assign the values
            FunctGroupArray[speciesIndex].distrib[b][next_qrt][stageIndex] = new_value;
        }
    }
}


/**
 * \brief Set the new movement entries - from forcing file
 *
 */
void Ecology_Update_Move_Entry(MSEBoxModel *bm, FILE *llogfp) {
    int speciesIndex, stageIndex, tracerIndex;
    int qrt, next_qrt, last_qrt;
    double new_value;
    //double distrib_last_qrt, distrib_qrt, distrib_next_qrt;
    int b = 0;
    
    if (verbose)
        printf("Doing Ecology_Update_Move_Entry\n");
    
    if (bm->use_move_entries) {
        /* Initialise the fored movement arrays - once get to tstart */
        for (tracerIndex = 0; tracerIndex < bm->numForceMoveEntries; tracerIndex++) {
            if (!bm->forceMoveEntryInput[tracerIndex].moveInitDone) {
                if (bm->dayt != bm->forceMoveEntryInput[tracerIndex].movetstart) {
                    // Nothing to do as too early
                } else {
                    Init_full_move_array(bm, &(bm->forceMoveEntryInput[tracerIndex]), tracerIndex);
                    bm->forceMoveEntryInput[tracerIndex].moveInitDone = 1;
                    speciesIndex = bm->forceMoveEntryInput[tracerIndex].tracerID;
                    for (stageIndex = 0; stageIndex < FunctGroupArray[speciesIndex].numStages; stageIndex++) {
                        FunctGroupArray[speciesIndex].NeedMoveUpdate[stageIndex] = 0;
                    }
                }
            }
        }
        
        /* Move read-in value to the movement arrays */
        for (tracerIndex = 0; tracerIndex < bm->numForceMoveEntries; tracerIndex++) {
            if(bm->dayt > bm->forceMoveEntryInput[tracerIndex].movetstart) {
                speciesIndex = bm->forceMoveEntryInput[tracerIndex].tracerID;
                stageIndex = bm->forceMoveEntryInput[tracerIndex].stageID;
            
                // Check any species need entries updated - if not skip ahead
                if (!FunctGroupArray[speciesIndex].NeedMoveUpdate[stageIndex])
                    continue;
            
                if(bm->forceMoveEntryInput[tracerIndex].atEnd)
                    return;

                /* Loop while more time remains in this transport time step */
                qrt = FunctGroupArray[speciesIndex].moveEntryIndex;
                last_qrt = qrt - 1;
                if(last_qrt < 0)
                    last_qrt = 0;
                next_qrt = FunctGroupArray[speciesIndex].next_moveEntryIndex;
           
                /* Get more data if necessary */
                get_move_property(bm, &(bm->forceMoveEntryInput[tracerIndex]));  // based on get_property()
                if(bm->forceMoveEntryInput[tracerIndex].atEnd == TRUE)
                    return;

                /* Loop over each box to check the values */
                for (b = 0; b < bm->nbox; b++) {
                    
                    /*
                    distrib_last_qrt = FunctGroupArray[speciesIndex].distrib[b][last_qrt][stageIndex];
                    distrib_qrt = FunctGroupArray[speciesIndex].distrib[b][qrt][stageIndex];
                    distrib_next_qrt = FunctGroupArray[speciesIndex].distrib[b][next_qrt][stageIndex];
                    
                    fprintf(llogfp, "Ecology_Update_Move_Entry Time: %e, %s-%d box: %d last_qrt: %d, qrt: %d next_qrt: %d distrib_last_qrt: %e distrib_qrt: %e orig distrib_next_qrt: %e ", bm->dayt, FunctGroupArray[speciesIndex].groupCode, stageIndex, b, last_qrt, qrt, next_qrt, distrib_last_qrt, distrib_qrt, distrib_next_qrt);
                    */
                    
                    new_value = bm->forceMoveEntryInput[tracerIndex].dataBuffer[b][0]; // Take the entry for next time period from the file

                    if (!_finite(new_value)) {
                        quit("%s in box %d at time %e is not finite %e for the next period\n", bm->forceMoveEntryInput[tracerIndex].variableName, b, bm->dayt, new_value);
                    }

                    // Assign the values
                    FunctGroupArray[speciesIndex].distrib[b][next_qrt][stageIndex] = new_value;
                
                    //fprintf(llogfp, "new_value_next_qrt: %e\n", new_value);
                }
                FunctGroupArray[speciesIndex].NeedMoveUpdate[stageIndex] = 0;
            }
        }
    }
}
