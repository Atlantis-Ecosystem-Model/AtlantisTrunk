/*******************************************************************//**
\file
\brief Routines which deal with annual age specific tracer data files.
These files are in netCDF format and should be read, written and created
using the routines below.

File:           atagetracerIO.c

Created:        Sat Dec 5 08:26:00 EST 2015

Author:         Beth Fulton
CSIRO Oceans & Atmosphere

Purpose:        Routines which deal with annual age specific tracer data files.
These files are in netCDF format and should be read,
written and created using the routines below.

Arguments:      See each routine below

Returns:        See each routine below

Revisions:

*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sjwlib.h>
#include <netcdf.h>
#include <atlantisboxmodel.h>
#include <atHarvestLib.h>
#include <atUtilLib.h>

/*************//**
 Routine to create general age structured biological data summary file
****************/
int createBMAnnAgeBioDataFile(char *destFolder, char *name, MSEBoxModel *bm) {
    int fid;
    char fileName[BMSLEN];
    
    snprintf(fileName, sizeof(fileName), "%s%s", destFolder, name);
    /* Set netCDF error handling */
    ncopts = NC_VERBOSE | NC_FATAL;
    
    /* Create new netCDF file */
    if (bm->flagreusefile == 2)
        fid = nccreate(fileName, NC_CLOBBER);
    else
        fid = nccreate(fileName, NC_NOCLOBBER);
    
    /* Define dimensions */
    ncdimdef(fid, "t", NC_UNLIMITED);
    ncdimdef(fid, "b", bm->nbox);
    ncdimdef(fid, "z", bm->wcnz + bm->sednz);  // Note used in the file, but socan be read by Olive
    
    
    /* Global attributes */
    ncattput(fid, NC_GLOBAL, "title", NC_CHAR, (int) strlen(bm->title) + 1, bm->title);
    ncattput(fid, NC_GLOBAL, "geometry", NC_CHAR, (int) strlen(bm->geomIfname) + 1, bm->geomIfname);
    ncattput(fid, NC_GLOBAL, "parameters", NC_CHAR, (int) strlen(bm->params) + 1, bm->params);
    ncattput(fid, NC_GLOBAL, "wcnz", NC_LONG, 1, &bm->wcnz);
    ncattput(fid, NC_GLOBAL, "sednz", NC_LONG, 1, &bm->sednz);
    
    /* Variables and their attributes
     Note that dtype streaming will mean appropriate data entered in
     each case
     */
    writeBMphysInfo(fid, bm, 1);
    writeBMAnnAgeBioInfo(fid, bm);
    
    /* Exit from netCDF define mode */
    ncendef(fid);
    ncsync(fid);
    
    return (fid);
}

/*********//**
 Routine to write age structured tracer info to general data file
***********/
void writeBMAnnAgeBioInfo(int fid, MSEBoxModel *bm) {
    char ann_name[BMSLEN];
    char ann_longname[BMSLEN];
    char unitstr[BMSLEN];
    int sp = 0;
    nc_type dt;
    int dim[2];
    int vid;
    int this_cohort = 0;
    int cohort, propit, numAges;
    
    /* set netCDF data type */
    dt = NC_DOUBLE;
    
    /* Get dimension ids */
    dim[0] = ncdimid(fid, "t");
    dim[1] = ncdimid(fid, "b");
    
    /* Loop over the tracers */
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].groupAgeType == AGE_STRUCTURED) { // Used to be a test on FunctGroupArray[sp].isVertebrate
            if ( (int)(FunctGroupArray[sp].speciesParams[flag_id])){
                this_cohort = 0;
                numAges = FunctGroupArray[sp].ageClassSize;
                if(numAges < 1)
                    numAges = 1;
                for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes * FunctGroupArray[sp].numStocks * numAges; cohort++) {
                    for (propit = 0; propit < out_catch_id; propit++) {
                        
                        switch (propit) {
                            case out_nums_id:
                                snprintf(ann_name, sizeof(ann_name), "%s%d_Nums", FunctGroupArray[sp].name, this_cohort+1);
                                snprintf(ann_longname, sizeof(ann_longname), "Numbers of %s annual age class %d", FunctGroupArray[sp].name, this_cohort+1);
                                snprintf(unitstr, sizeof(unitstr), "Numbers");
                                break;
                            case out_size_id:
                                snprintf(ann_name, sizeof(ann_name), "%s%d_Weight", FunctGroupArray[sp].name, this_cohort+1);
                                snprintf(ann_longname, sizeof(ann_longname), "Individ weight for %s annual age class %d", FunctGroupArray[sp].name, this_cohort+1);
                                snprintf(unitstr, sizeof(unitstr), "g");
                                break;
                            default:
                                quit("writeBMAnnAgeInfo: How did you get this value as only 2 bio classes (propit: %d)\n", propit);
                                break;
                        }
                        
                        if (verbose > 1)
                            printf("writeBMAnnAgeBioInfo: doing file creation for %s\n", ann_name);
                        
                        /* Define the variable */
                        vid = ncvardef(fid, ann_name, dt, 2, dim);
                        
                        /* Set the attributes */
                        ncattput(fid, vid, "bmtype", NC_CHAR, (int) strlen("epibenthos") + 1, "epibenthos");
                        ncattput(fid, vid, "units", NC_CHAR, (int) strlen(unitstr) + 1, unitstr);
                        ncattput(fid, vid, "long_name", NC_CHAR, (int) strlen(ann_longname) + 1, ann_longname);
                        
                        if (verbose > 1)
                            fprintf(stderr, "writeBMAnnAgeBioInfo: %s info written\n", FunctGroupArray[sp].name);
                    }
                    
                    this_cohort++;
                }
            }
        }
    }
    
}

/*******************************************************************//**
Routine to write the annual age structured tracer data to a netCDF file.
This routine assumes that the tracer information in the MSEBoxModel is valid and
corresponds with the netCDF file. Minimal checking for consistency
is done here. Does check to see if going to correct output file, depending
on whether fisheries or general data type.
*********************************************************************/
void writeBMAnnAgeBioData(int fid, int dump, MSEBoxModel *bm) {
    char strname[BMSLEN];
    doubleINPUT *val;
    long start[2];
    long count[2];
    int sp, cohort, this_cohort, ai, propit, sn, rn, id, k, numAges;
    double tot;
    
    if (verbose > 0)
        fprintf(stderr, "Entering writeBMAnnAgeBioData\n");
    
    /* Set netCDF library error handling */
    ncopts = NC_VERBOSE | NC_FATAL;
    
    /* Allocate temporary storage for one epi variable */
    val = (doubleINPUT *) alloc1dInput(bm->nbox);
    
    /* Set indices for writing epis */
    start[0] = dump;
    start[1] = 0;
    count[0] = 1;
    count[1] = bm->nbox;
    
    /* Loop over the tracers */
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        int vid;
        int b;
        int n;
        if (FunctGroupArray[sp].groupAgeType == AGE_STRUCTURED) { // Used to be a test on FunctGroupArray[sp].isVertebrate
            if ( (int)(FunctGroupArray[sp].speciesParams[flag_id])){
                this_cohort = 0;
                numAges = FunctGroupArray[sp].ageClassSize;
                if(numAges < 1)
                    numAges = 1;
                
                for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++) {
                    for (ai = 0; ai < numAges; ai++) {
                        for (propit = 0; propit < out_catch_id; propit++) {
                            switch (propit) {
                                case out_nums_id:
                                    snprintf(strname, sizeof(strname), "%s%d_Nums", FunctGroupArray[sp].name, this_cohort+1);
                                    break;
                                case out_size_id:
                                    snprintf(strname, sizeof(strname), "%s%d_Weight", FunctGroupArray[sp].name, this_cohort+1);
                                    break;
                                default:
                                    quit("writeBMAnnAgeBioInfo: How did you get this value as only 3 classes (propit: %d)\n", propit);
                                    break;
                            }
                            
                            if (verbose > 1)
                                printf("writeBMAnnAgeBioData: writing %s\n", strname);
                            
                            /* Find match based on sting name and then get data */
                            sn = FunctGroupArray[sp].structNTracers[cohort];
                            rn = FunctGroupArray[sp].resNTracers[cohort];
                            id = FunctGroupArray[sp].NumsTracers[cohort];
                            
                            if (propit == out_nums_id) {  // Numbers case
                                for (b = 0; b < bm->nbox; b++) {
                                    val[b] = 0.0;
                                    for (k = 0; k < bm->wcnz; k++) {  // Sum over all individuals in the box
                                        val[b] += bm->wctr[b][k][id] * FunctGroupArray[sp].boxPopRatio[b][k][cohort][ai];
                                    }
                                }
                            } else  { // Size data - get weighted average of values per layer if numbers > min_dens
                                for (b = 0; b < bm->nbox; b++) {
                                    val[b] = 0.0;
                                    tot = 0.0;
                                    for (k = 0; k < bm->wcnz; k++) {  // Sum over all individuals in the box
                                        if (bm->wctr[b][k][id] > bm->min_dens) {
                                            val[b] += bm->wctr[b][k][id] * (bm->wctr[b][k][sn] + bm->wctr[b][k][rn]) * bm->X_CN * mg_2_g * FunctGroupArray[sp].boxPopRatio[b][k][cohort][ai];
                                            tot += bm->wctr[b][k][id];
                                        }
                                    }
                                    if (tot > 0.0)
                                        val[b] /= tot;
                                }
                            }
                            
                            /* Get netCDF variable id */
                            vid = ncvarid(fid, strname);
                            
                            /* Check double compatibility */
                            if ((n = ncvarsize(fid, vid)) != sizeof(doubleINPUT))
                                quit("writeBMAnnAgeBioData: %s has %d bytes per value in file, code compiled with %d\n", strname, n, sizeof(doubleINPUT));
                            
                            /* Write data */
                            ncvarput(fid, vid, start, count, val);
                            
                        }
                        
                        // Update counter
                        this_cohort++;
                        
                    }
                }
            }
        }
    }
    
    /* Free temporary storage */
    free1dInput((doubleINPUT *) val);
    
}

/**//**************************************** Fisheries age structured data *********************************************************************/
/*************//**
 Routine to create general age structured catch an discards data summary file
****************/
int createBMAnnAgeCatDataFile(char *destFolder, char *name, MSEBoxModel *bm) {
    int fid;
    char fileName[BMSLEN];
    
    snprintf(fileName, sizeof(fileName), "%s%s", destFolder, name);
    /* Set netCDF error handling */
    ncopts = NC_VERBOSE | NC_FATAL;
    
    /* Create new netCDF file */
    if (bm->flagreusefile == 2)
        fid = nccreate(fileName, NC_CLOBBER);
    else
        fid = nccreate(fileName, NC_NOCLOBBER);
    
    /* Define dimensions */
    ncdimdef(fid, "t", NC_UNLIMITED);
    ncdimdef(fid, "b", bm->nbox);
    ncdimdef(fid, "z", bm->wcnz + bm->sednz);  // Note used in the file, but socan be read by Olive
    
    /* Global attributes */
    ncattput(fid, NC_GLOBAL, "title", NC_CHAR, (int) strlen(bm->title) + 1, bm->title);
    ncattput(fid, NC_GLOBAL, "geometry", NC_CHAR, (int) strlen(bm->geomIfname) + 1, bm->geomIfname);
    ncattput(fid, NC_GLOBAL, "parameters", NC_CHAR, (int) strlen(bm->params) + 1, bm->params);
    ncattput(fid, NC_GLOBAL, "wcnz", NC_LONG, 1, &bm->wcnz);
    ncattput(fid, NC_GLOBAL, "sednz", NC_LONG, 1, &bm->sednz);
    
    /* Variables and their attributes
     Note that dtype streaming will mean appropriate data entered in
     each case
     */
    writeBMphysInfo(fid, bm, 1);
    writeBMAnnAgeCatInfo(fid, bm);
    
    /* Exit from netCDF define mode */
    ncendef(fid);
    ncsync(fid);
    
    return (fid);
}

/*********//**
 Routine to write age structured tracer info to general data file
***********/
void writeBMAnnAgeCatInfo(int fid, MSEBoxModel *bm) {
    char ann_name[BMSLEN];
    char ann_longname[BMSLEN];
    char unitstr[BMSLEN];
    int sp = 0;
    nc_type dt;
    int dim[2];
    int vid;
    int this_cohort = 0;
    int cohort, propit, numAges, nf;
    
    //if (verbose)
        printf("Entering writeBMAnnAgeCatInfo\n");
    
    /* set netCDF data type */
    dt = NC_DOUBLE;
    
    /* Get dimension ids */
    dim[0] = ncdimid(fid, "t");
    dim[1] = ncdimid(fid, "b");
    
    /* Loop over the tracers */
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].groupAgeType == AGE_STRUCTURED) { // Used to be a test on FunctGroupArray[sp].isVertebrate
            if (( (int)(FunctGroupArray[sp].speciesParams[flag_id])) && (FunctGroupArray[sp].isFished || FunctGroupArray[sp].isImpacted)){
                this_cohort = 0;
                numAges = FunctGroupArray[sp].ageClassSize;
                if(numAges < 1)
                    numAges = 1;
                
                for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes * FunctGroupArray[sp].numStocks * numAges; cohort++) {
                    for (propit = out_catch_id; propit < (out_discards_id + 1); propit++) {
                        for (nf = 0; nf < bm->K_num_fisheries; nf++) {
                            if (FisherySpeciesCatchFlags[sp][nf] == TRUE) {
                                switch (propit) {
                                    case out_catch_id:
                                        snprintf(ann_name, sizeof(ann_name), "%s%d_Catch_%s", FunctGroupArray[sp].name, this_cohort+1,FisheryArray[nf].fisheryCode);
                                        snprintf(ann_longname, sizeof(ann_longname), "Catch of %s annual age class %d by fishery %s", FunctGroupArray[sp].name, this_cohort+1, FisheryArray[nf].name);
                                        snprintf(unitstr, sizeof(unitstr), "Numbers");
                                        break;
                                    case out_discards_id:
                                        snprintf(ann_name, sizeof(ann_name), "%s%d_Discard_%s", FunctGroupArray[sp].name, this_cohort+1,FisheryArray[nf].fisheryCode);
                                        snprintf(ann_longname, sizeof(ann_longname), "Discards of %s annual age class %d by fshery %s", FunctGroupArray[sp].name, this_cohort+1, FisheryArray[nf].name);
                                        snprintf(unitstr, sizeof(unitstr), "Numbers");
                                        break;
                                    default:
                                        quit("writeBMAnnAgeInfo: How did you get this value as only 3 classes (propit: %d)\n", propit);
                                        break;
                                }
                                
                                /* Define the variable */
                                if (verbose > 1)
                                    printf("writeBMAnnAgeCatInfo: doing file creation for %s\n", ann_name);
                            
                                vid = ncvardef(fid, ann_name, dt, 2, dim);
                            
                                /* Set the attributes */
                                ncattput(fid, vid, "bmtype", NC_CHAR, (int) strlen("epibenthos") + 1, "epibenthos");
                                ncattput(fid, vid, "units", NC_CHAR, (int) strlen(unitstr) + 1, unitstr);
                                ncattput(fid, vid, "long_name", NC_CHAR, (int) strlen(ann_longname) + 1, ann_longname);
                            
                                if (verbose > 1)
                                    fprintf(stderr, "writeBMAnnAgeCatInfo: %s info written\n", FunctGroupArray[sp].name);
                            }
                        }
                    }
                    
                    this_cohort++;
                }
            }
        }
    }
    
}

/*******************************************************************//**
Routine to write the annual age structured tracer data to a netCDF file.
This routine assumes that the tracer information in the MSEBoxModel is valid and
corresponds with the netCDF file. Minimal checking for consistency
is done here. Does check to see if going to correct output file, depending
on whether fisheries or general data type.
*********************************************************************/
void writeBMAnnAgeCatData(int fid, int dump, MSEBoxModel *bm) {
    char strname[BMSLEN];
    doubleINPUT *val;
    long start[2];
    long count[2];
    int sp, cohort, this_cohort, ai, propit, sn, rn, did, id, k, nf, tid, numAges;
    double totlayer, thistotden, rel_access, tot;
    double *avgpopratio;
    double *avgsze;
    
    /* double dummy*/
    if (verbose > 0)
        fprintf(stderr, "Entering writeBMAnnAgeCatData\n");
    
    /* Set netCDF library error handling */
    ncopts = NC_VERBOSE | NC_FATAL;
    
    /* Allocate temporary storage for one epi variable */
    val = (doubleINPUT *) alloc1dInput(bm->nbox);
    avgpopratio = (double *) alloc1dInput(bm->nbox);
    avgsze = (double *) alloc1dInput(bm->nbox);
    
    /* Set indices for writing epis */
    start[0] = dump;
    start[1] = 0;
    count[0] = 1;
    count[1] = bm->nbox;
    
    /* Loop over the tracers */
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        int vid;
        int b;
        int n;
        if (FunctGroupArray[sp].groupAgeType == AGE_STRUCTURED) { // Used to be a test on FunctGroupArray[sp].isVertebrate
            if ( (int)(FunctGroupArray[sp].speciesParams[flag_id])){
                this_cohort = 0;
                numAges = FunctGroupArray[sp].ageClassSize;
                if(numAges < 1)
                    numAges = 1;
                
                for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++) {
                    did = FunctGroupArray[sp].NumsTracers[cohort];
                    sn = FunctGroupArray[sp].structNTracers[cohort];
                    rn = FunctGroupArray[sp].resNTracers[cohort];
                    
                    for (ai = 0; ai < numAges; ai++) {
                        // Get average relative age class content for the water column (as catch data stored 2D but age tacking 3D
                        for (b = 0; b < bm->nbox; b++) {
                            avgpopratio[b] = 0.0;
                            avgsze[b] = 0.0;
                            thistotden = 0.0;
                            totlayer = 0.0;
                            for (k = 0; k < bm->wcnz; k++) {
                                if (bm->wctr[b][k][did] > bm->min_dens) { // Sum over water column but only if > min_dens present
                                    avgpopratio[b] += FunctGroupArray[sp].boxPopRatio[b][k][cohort][ai];
                                    totlayer += 1.0;
                                    avgsze[b] += (bm->wctr[b][k][sn] + bm->wctr[b][k][rn]) * bm->wctr[b][k][did];
                                    thistotden += bm->wctr[b][k][did];
                                }
                            }
                            // Sanity checks
                            if(!totlayer)
                                totlayer = small_num;
                            if(!thistotden)
                                thistotden = small_num;
                            
                            // Final averages
                            avgpopratio[b] /= totlayer;
                            avgsze[b] /= thistotden;
                        }
                        
                        for (propit = out_catch_id; propit < (out_discards_id + 1); propit++) {
                            for (b = 0; b < bm->nbox; b++) {
                                tot = 0.0;
                                for (nf = 0; nf < bm->K_num_fisheries; nf++) {
                                    if (FisherySpeciesCatchFlags[sp][nf] == TRUE) {
                                        rel_access = Harvest_How_Much_Fishery_Access(bm,sp,cohort,nf,avgsze[b]);
                                    
                                        if (propit == out_catch_id) {
                                            tid = FunctGroupArray[sp].CaughtByFisheryTracers[nf];
                                        } else {
                                            tid = FunctGroupArray[sp].DiscardedByFisheryTracers[nf];
                                        }
                                        tot += rel_access * bm->fishstat[b][tid];
                                    }
                                }
                                if (!tot)
                                    tot = small_num;
                                
                                for (nf = 0; nf < bm->K_num_fisheries; nf++) {
                                    if (FisherySpeciesCatchFlags[sp][nf] == TRUE) {
                                        rel_access = Harvest_How_Much_Fishery_Access(bm,sp,cohort,nf,avgsze[b]);
                                        switch (propit) {
                                            case out_catch_id:
                                                snprintf(strname, sizeof(strname), "%s%d_Catch_%s", FunctGroupArray[sp].name, this_cohort+1,FisheryArray[nf].fisheryCode);
                                                tid = FunctGroupArray[sp].CaughtByFisheryTracers[nf];
                                                id = FunctGroupArray[sp].CatchTracers[cohort];  // This points to numbers caught at that age
                                                break;
                                            case out_discards_id:
                                                snprintf(strname, sizeof(strname), "%s%d_Discard_%s", FunctGroupArray[sp].name, this_cohort+1,FisheryArray[nf].fisheryCode);
                                                tid = FunctGroupArray[sp].DiscardedByFisheryTracers[nf];
                                                id = FunctGroupArray[sp].DiscardTracers[cohort];  // This points to numbers discarded at that age
                                                break;
                                            default:
                                                quit("writeBMAnnAgeCatInfo: How did you get this value as only 3 classes (propit: %d)\n", propit);
                                                break;
                                        }
                                    
                                        if (verbose > 1)
                                            printf("writeBMAnnAgeCatData: writing %s\n", strname);
                                    
                                        val[b] = (doubleINPUT)(avgpopratio[b] * bm->fishstat[b][id] * (rel_access * bm->fishstat[b][tid] / tot));
                                        
                                        /** dummy = val[b] * (avgsze[b] * bm->X_CN * mg_2_tonne);

                                        if(sp == 2)
                                            fprintf(bm->logFile,"Time: %e printing out fishstats %s-%d (%d) fishery: %d box%d val: %e avgpopratio: %e rel_access: %e fishstatDEN: %e fishstatTIDratio: %e (equivalent to %e t as avgsze: %e X_CN: %e mg_2_tonne: %e)\n", bm->dayt, FunctGroupArray[sp].groupCode, cohort, ai, nf, b, val[b], avgpopratio[b], rel_access, bm->fishstat[b][id], bm->fishstat[b][tid] / tot, dummy, avgsze[b], bm->X_CN, mg_2_tonne);
                                       **/
                                    
                                        /* Get netCDF variable id */
                                        vid = ncvarid(fid, strname);
                                    
                                        /* Check double compatibility */
                                        if ((n = ncvarsize(fid, vid)) != sizeof(doubleINPUT))
                                            quit("writeBMAnnAgeCatData: %s has %d bytes per value in file, code compiled with %d\n", strname, n, sizeof(doubleINPUT));
                                    
                                        /* Write data */
                                        ncvarput(fid, vid, start, count, val);
                                    }
                                }
                            }
                        }
                        
                        
                        // Update counter
                        this_cohort++;
                    }
                }
            }
        }
    }
    
    /* Free temporary storage */
    free1dInput((doubleINPUT *) val);
    free1d(avgpopratio);
    free1d(avgsze);
    
}

