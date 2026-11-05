/*******************************************************************//**
\file
\brief Routines which deal with annual age specific diet data files. These files are in netCDF format
and should be read, written and created using the routines below.
                                                                      
File:           atagedietIO.c
                                                                      
Created:        Sat Dec 6 08:56:00 EST 2015
                                                                      
Author:         Beth Fulton
CSIRO Oceans & Atmosphere
                                                                      
Purpose:        Routines which deal with annual age specific diet data files.
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
#include <atUtilLib.h>

/********************************************************************
 Prototypes
 **/

//static void NormaliseDietData(MSEBoxModel *bm);

/***************
Build the netCDF variable name for one predator age class: <groupCode><cohort+1>
(e.g. FVS1), following the convention of the other age structured .nc files.
Used by both the define and the write routines so the names always match.
****************/
static void DietVarName(char *name, int sp, int cohort) {
    sprintf(name, "%s%d", FunctGroupArray[sp].groupCode, cohort + 1);
}
 
/***************
Same predator test as Write_DetailedDietCheck in atbiolmessage.c
****************/
static int IsDietPredator(int sp) {
    return ((int) (FunctGroupArray[sp].speciesParams[flag_id]) && FunctGroupArray[sp].isPredator == TRUE);
}

/***************
Routine to create general age structured diet data summary file
****************/
int createBMDietDataFile(char *destFolder, char *name, MSEBoxModel *bm) {
    int fid, prey;
    long nlen = 1;
    long start[2], count[2];
    char fileName[BMSLEN];
    char *names;
 
    sprintf(fileName, "%s%s", destFolder, name);
    /* Set netCDF error handling */
    ncopts = NC_VERBOSE | NC_FATAL;
 
    /* Create new netCDF file */
    if (bm->flagreusefile == 2)
        fid = nccreate(fileName, NC_CLOBBER);
    else
        fid = nccreate(fileName, NC_NOCLOBBER);
 
    /* Length needed to hold the longest group code (plus terminator) */
    for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
        if ((long) strlen(FunctGroupArray[prey].groupCode) + 1 > nlen)
            nlen = (long) strlen(FunctGroupArray[prey].groupCode) + 1;
    }
 
    /* Define dimensions */
    ncdimdef(fid, "t", NC_UNLIMITED);
    ncdimdef(fid, "b", bm->nbox);
    ncdimdef(fid, "z", bm->wcnz + bm->sednz);
    ncdimdef(fid, "prey", bm->K_num_tot_sp);
    ncdimdef(fid, "name_len", nlen);
 
    /* Global attributes */
    ncattput(fid, NC_GLOBAL, "title", NC_CHAR, (int) strlen(bm->title) + 1, bm->title);
    ncattput(fid, NC_GLOBAL, "geometry", NC_CHAR, (int) strlen(bm->geomIfname) + 1, bm->geomIfname);
    ncattput(fid, NC_GLOBAL, "parameters", NC_CHAR, (int) strlen(bm->params) + 1, bm->params);
    ncattput(fid, NC_GLOBAL, "wcnz", NC_LONG, 1, &bm->wcnz);
    ncattput(fid, NC_GLOBAL, "sednz", NC_LONG, 1, &bm->sednz);
 
    /* Variables and their attributes */
    writeBMDietInfo(fid, bm);
 
    /* Exit from netCDF define mode */
    ncendef(fid);
 
    /* Write the prey group codes once (static, so written at creation) */
    names = (char *) calloc((size_t) bm->K_num_tot_sp * (size_t) nlen, sizeof(char));
    for (prey = 0; prey < bm->K_num_tot_sp; prey++)
        strcpy(names + (size_t) prey * (size_t) nlen, FunctGroupArray[prey].groupCode);
    start[0] = 0;
    start[1] = 0;
    count[0] = bm->K_num_tot_sp;
    count[1] = nlen;
    ncvarput(fid, ncvarid(fid, "prey_name"), start, count, names);
    free(names);
 
    ncsync(fid);
 
    return (fid);
}

/***********
Routine to define the age structured diet variables in the diet data file
***********/

void writeBMDietInfo(int fid, MSEBoxModel *bm) {
    char ann_name[BMSLEN];
    char ann_longname[BMSLEN];
    int sp, cohort;
    int dim[4];
    int sdim[2];
    int vid;
    long cohortnum;
    double fillval = NC_FILL_DOUBLE;
 
    /* Get dimension ids */
    dim[0] = ncdimid(fid, "t");
    dim[1] = ncdimid(fid, "b");
    dim[2] = ncdimid(fid, "z");
    dim[3] = ncdimid(fid, "prey");
 
    if (bm->ice_on) {
        warn("No age specific diet data for ice layers, if you want this ask for new code\n");
    }
 
    /* Define time variable */
    vid = ncvardef(fid, "t", NC_DOUBLE, 1, dim);
    ncattput(fid, vid, "units", NC_CHAR, (int) strlen(bm->t_units) + 1, bm->t_units);
 
    /* Prey group codes, in the same order as the prey dimension */
    sdim[0] = ncdimid(fid, "prey");
    sdim[1] = ncdimid(fid, "name_len");
    vid = ncvardef(fid, "prey_name", NC_CHAR, 2, sdim);
    sprintf(ann_longname, "Functional group code of each prey index");
    ncattput(fid, vid, "long_name", NC_CHAR, (int) strlen(ann_longname) + 1, ann_longname);
 
    /* One variable per predator age class */
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (!IsDietPredator(sp))
            continue;
 
        for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++) {
            DietVarName(ann_name, sp, cohort);
            sprintf(ann_longname, "Total consumption by %s age class %d, by prey group", FunctGroupArray[sp].name, cohort + 1);
            cohortnum = cohort + 1;
 
            if (verbose > 1)
                fprintf(stderr, "writeBMDietInfo: defining %s\n", ann_name);
 
            vid = ncvardef(fid, ann_name, NC_DOUBLE, 4, dim);
 
            /* Set the attributes */
            ncattput(fid, vid, "units", NC_CHAR, (int) strlen("Tonnes") + 1, "Tonnes");
            ncattput(fid, vid, "long_name", NC_CHAR, (int) strlen(ann_longname) + 1, ann_longname);
            ncattput(fid, vid, "_FillValue", NC_DOUBLE, 1, &fillval);
            ncattput(fid, vid, "predator", NC_CHAR, (int) strlen(FunctGroupArray[sp].groupCode) + 1, FunctGroupArray[sp].groupCode);
            ncattput(fid, vid, "cohort", NC_LONG, 1, &cohortnum);
        }
    }
}
 
/*******************************************************************//**
Routine to write the age structured diet data to a netCDF file.
One variable per predator age class, each t x b x z x prey. Cells that do not
exist (layers below the seabed of shallow boxes, land boxes, prey groups that
are not switched on) are left as the fill value, so they can be told apart
from a real zero (nothing eaten). Values are total consumption in tonnes.
*********************************************************************/
#define DIETIDX(b, k, p) ((((size_t)(b) * (size_t)nz) + (size_t)(k)) * (size_t)nprey + (size_t)(p))
 
void writeBMDietData(int fid, int dump, MSEBoxModel *bm) {
    char strname[BMSLEN];
    doubleINPUT *buf;
    doubleINPUT fillval = (doubleINPUT) NC_FILL_DOUBLE;
    doubleINPUT value;
    int *preyflag;
    long start[4];
    long count[4];
    int nz = bm->wcnz + bm->sednz;
    int nprey = bm->K_num_tot_sp;
    size_t i, n = (size_t) bm->nbox * (size_t) nz * (size_t) nprey;
    int sp, cohort, prey, b, k, kdiff, vid, nbytes;
 
    if (verbose > 0)
        fprintf(stderr, "Entering writeBMDietData\n");
 
    /* Set netCDF library error handling */
    ncopts = NC_VERBOSE | NC_FATAL;
 
    /* Write time value */
    start[0] = dump;
    start[1] = 0;
    start[2] = 0;
    start[3] = 0;
    count[0] = 1L;
    count[1] = 1L;
    count[2] = 1L;
    count[3] = 1L;
    value = (doubleINPUT) bm->t;
    ncvarput(fid, ncvarid(fid, "t"), start, count, &value);
 
    /* Indices for writing one predator age class: one time step, all boxes, layers and prey */
    count[0] = 1;
    count[1] = bm->nbox;
    count[2] = nz;
    count[3] = nprey;
 
    /* Temporary storage, reused for every predator age class */
    buf = (doubleINPUT *) malloc(n * sizeof(doubleINPUT));
    preyflag = (int *) malloc((size_t) nprey * sizeof(int));
    if (buf == NULL || preyflag == NULL)
        quit("writeBMDietData: could not allocate temporary storage\n");
 
    /* Only prey groups that are switched on get values (as in the diet text output) */
    for (prey = 0; prey < nprey; prey++)
        preyflag[prey] = (int) (FunctGroupArray[prey].speciesParams[flag_id]);
 
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (!IsDietPredator(sp))
            continue;
 
        for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++) {
            /* Start with everything missing, then fill in the cells that exist */
            for (i = 0; i < n; i++)
                buf[i] = fillval;
 
            for (b = 0; b < bm->nbox; b++) {
                if (bm->boxes[b].type == LAND)
                    continue;
 
                /* Water column: layers are offset in shallow boxes (same as the tracer output) */
                kdiff = bm->wcnz - bm->boxes[b].nz;
                for (k = kdiff; k < bm->wcnz; k++) {
                    for (prey = 0; prey < nprey; prey++) {
                        if (preyflag[prey])
                            buf[DIETIDX(b, k, prey)] = (doubleINPUT) (bm->totDiet[b][k - kdiff][sp][cohort][prey] * mg_2_tonne * bm->X_CN); // recorded in mg, converted to tonnes
                    }
                }
 
                /* Sediment layers sit after the water column layers in totDiet */
                for (k = 0; k < bm->sednz; k++) {
                    for (prey = 0; prey < nprey; prey++) {
                        if (preyflag[prey])
                            buf[DIETIDX(b, bm->wcnz + k, prey)] = (doubleINPUT) (bm->totDiet[b][bm->wcnz + k][sp][cohort][prey] * mg_2_tonne * bm->X_CN);
                    }
                }
            }
 
            DietVarName(strname, sp, cohort);
            vid = ncvarid(fid, strname);
 
            /* Check double compatibility */
            if ((nbytes = ncvarsize(fid, vid)) != (int) sizeof(doubleINPUT))
                quit("writeBMDietData: %s has %d bytes per value in file, code compiled with %d\n", strname, nbytes, (int) sizeof(doubleINPUT));
 
            /* Write data */
            ncvarput(fid, vid, start, count, buf);
        }
    }
 
    /* Free temporary storage */
    free(preyflag);
    free(buf);
}
#undef DIETIDX

/*******************************************************************//**
Routine to normalise the diet data per box.
 *********************************************************************
void NormaliseDietData(MSEBoxModel *bm) {
    double tot;
    int b, k, sp, cohort, prey;
    
    for (b = 0; b < bm->nbox; b++) {
        for (k = 0; k < bm->wcnz+bm->sednz; k++) {
            for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes * FunctGroupArray[sp].numStocks; cohort++) {
                    tot = 0.0;
                    for (prey=0; prey < bm->K_num_tot_sp; prey++) {
                        tot += bm->totDiet[b][k][sp][cohort][prey];
                    }
                    if (!tot)
                        tot = small_num;
                    
                    // Normalise
                    for (prey=0; prey < bm->K_num_tot_sp; prey++) {
                        bm->totDiet[b][k][sp][cohort][prey] /= tot;
                    }
                }
            }
        }
    }
}
*/

