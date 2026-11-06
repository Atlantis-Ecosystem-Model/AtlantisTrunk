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
Count the prey groups that appear on the prey axis of the diet file (the groups
that are switched on, as in the text diet output). If map is not NULL it must
have room for K_num_tot_sp entries and is filled with the functional group
index of each prey, in prey axis order.
Also used by atlantismain.c to check the prey dimension of a re-used file.
****************/
int countBMDietPrey(MSEBoxModel *bm, int *map) {
    int prey, n = 0;

    for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
        if ((int) (FunctGroupArray[prey].speciesParams[flag_id])) {
            if (map != NULL)
                map[n] = prey;
            n++;
        }
    }
    return (n);
}

/***************
Routine to create general age structured diet data summary file
****************/
int createBMDietDataFile(char *destFolder, char *name, MSEBoxModel *bm) {
    int fid, i, nprey;
    int *map;
    long nlen = 1;
    long start[2], count[2];
    char fileName[BMSLEN];
    char *names;

    sprintf(fileName, "%s%s", destFolder, name);
    /* Set netCDF error handling */
    ncopts = NC_VERBOSE | NC_FATAL;

    /* Which prey go on the prey axis */
    map = (int *) malloc((size_t) bm->K_num_tot_sp * sizeof(int));
    if (map == NULL)
        quit("createBMDietDataFile: could not allocate prey map\n");
    nprey = countBMDietPrey(bm, map);
    if (nprey < 1)
        quit("createBMDietDataFile: no prey groups are switched on, nothing to write\n");

    /* Create new netCDF file */
    if (bm->flagreusefile == 2)
        fid = nccreate(fileName, NC_CLOBBER);
    else
        fid = nccreate(fileName, NC_NOCLOBBER);

    /* Length needed to hold the longest prey group code (plus terminator) */
    for (i = 0; i < nprey; i++) {
        if ((long) strlen(FunctGroupArray[map[i]].groupCode) + 1 > nlen)
            nlen = (long) strlen(FunctGroupArray[map[i]].groupCode) + 1;
    }

    /* Define dimensions */
    ncdimdef(fid, "t", NC_UNLIMITED);
    ncdimdef(fid, "b", bm->nbox);
    ncdimdef(fid, "z", bm->wcnz + bm->sednz);
    ncdimdef(fid, "prey", nprey);   /* only prey groups that are switched on, see prey_name / prey_group */
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

    /* Write the prey group codes and group indices once (static, so written at creation) */
    names = (char *) calloc((size_t) nprey * (size_t) nlen, sizeof(char));
    if (names == NULL)
        quit("createBMDietDataFile: could not allocate prey names\n");
    for (i = 0; i < nprey; i++)
        strcpy(names + (size_t) i * (size_t) nlen, FunctGroupArray[map[i]].groupCode);
    start[0] = 0;
    start[1] = 0;
    count[0] = nprey;
    count[1] = nlen;
    ncvarput(fid, ncvarid(fid, "prey_name"), start, count, names);
    ncvarput(fid, ncvarid(fid, "prey_group"), start, count, map);  /* 1-D variable: only start[0], count[0] are used */
    free(names);
    free(map);

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
    float fillval = (float) NC_FILL_FLOAT;

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

    /* Prey group codes and functional group indices, in the same order as the prey dimension */
    sdim[0] = ncdimid(fid, "prey");
    sdim[1] = ncdimid(fid, "name_len");
    vid = ncvardef(fid, "prey_name", NC_CHAR, 2, sdim);
    sprintf(ann_longname, "Functional group code of each prey index");
    ncattput(fid, vid, "long_name", NC_CHAR, (int) strlen(ann_longname) + 1, ann_longname);

    vid = ncvardef(fid, "prey_group", NC_INT, 1, sdim);
    sprintf(ann_longname, "Functional group index (0-based) of each prey index");
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

            vid = ncvardef(fid, ann_name, NC_FLOAT, 4, dim);

            /* Set the attributes */
            ncattput(fid, vid, "units", NC_CHAR, (int) strlen("Tonnes") + 1, "Tonnes");
            ncattput(fid, vid, "long_name", NC_CHAR, (int) strlen(ann_longname) + 1, ann_longname);
            ncattput(fid, vid, "_FillValue", NC_FLOAT, 1, &fillval);
            ncattput(fid, vid, "predator", NC_CHAR, (int) strlen(FunctGroupArray[sp].groupCode) + 1, FunctGroupArray[sp].groupCode);
            ncattput(fid, vid, "cohort", NC_LONG, 1, &cohortnum);
        }
    }
}

/*******************************************************************//**
Routine to write the age structured diet data to a netCDF file.
One variable per predator age class, each t x b x z x prey (floats). Cells that
do not exist (layers below the seabed of shallow boxes, land boxes) are left as
the fill value, so they can be told apart from a real zero (nothing eaten).
Values are total consumption in tonnes. The prey axis holds only the prey
groups that are switched on, in the order given by countBMDietPrey.
*********************************************************************/
#define DIETIDX(b, k, p) ((((size_t)(b) * (size_t)nz) + (size_t)(k)) * (size_t)nprey + (size_t)(p))

void writeBMDietData(int fid, int dump, MSEBoxModel *bm) {
    char strname[BMSLEN];
    float *buf;
    float fillval = (float) NC_FILL_FLOAT;
    double value;
    int *map;
    long start[4];
    long count[4];
    int nz = bm->wcnz + bm->sednz;
    int nprey;
    size_t i, n;
    int sp, cohort, prey, pi, b, k, kdiff, vid, nbytes;

    if (verbose > 0)
        fprintf(stderr, "Entering writeBMDietData\n");

    /* Set netCDF library error handling */
    ncopts = NC_VERBOSE | NC_FATAL;

    /* Which prey are on the prey axis */
    map = (int *) malloc((size_t) bm->K_num_tot_sp * sizeof(int));
    if (map == NULL)
        quit("writeBMDietData: could not allocate prey map\n");
    nprey = countBMDietPrey(bm, map);

    /* Write time value */
    start[0] = dump;
    start[1] = 0;
    start[2] = 0;
    start[3] = 0;
    count[0] = 1L;
    count[1] = 1L;
    count[2] = 1L;
    count[3] = 1L;
    value = (double) bm->t;
    ncvarput(fid, ncvarid(fid, "t"), start, count, &value);

    /* Indices for writing one predator age class: one time step, all boxes, layers and prey */
    count[0] = 1;
    count[1] = bm->nbox;
    count[2] = nz;
    count[3] = nprey;

    /* Temporary storage, reused for every predator age class */
    n = (size_t) bm->nbox * (size_t) nz * (size_t) nprey;
    buf = (float *) malloc(n * sizeof(float));
    if (buf == NULL)
        quit("writeBMDietData: could not allocate temporary storage\n");

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
                    for (pi = 0; pi < nprey; pi++) {
                        prey = map[pi];
                        buf[DIETIDX(b, k, pi)] = (float) (bm->totDiet[b][k - kdiff][sp][cohort][prey] * mg_2_tonne * bm->X_CN); // recorded in mg, converted to tonnes
                    }
                }

                /* Sediment layers sit after the water column layers in totDiet */
                for (k = 0; k < bm->sednz; k++) {
                    for (pi = 0; pi < nprey; pi++) {
                        prey = map[pi];
                        buf[DIETIDX(b, bm->wcnz + k, pi)] = (float) (bm->totDiet[b][bm->wcnz + k][sp][cohort][prey] * mg_2_tonne * bm->X_CN);
                    }
                }
            }

            DietVarName(strname, sp, cohort);
            vid = ncvarid(fid, strname);

            /* Check the variable is stored as float */
            if ((nbytes = ncvarsize(fid, vid)) != (int) sizeof(float))
                quit("writeBMDietData: %s has %d bytes per value in file, code writes %d\n", strname, nbytes, (int) sizeof(float));

            /* Write data */
            ncvarput(fid, vid, start, count, buf);
        }
    }

    /* Free temporary storage */
    free(map);
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