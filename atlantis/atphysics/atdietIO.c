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

/*************//**
Routine to create general age structured diet data summary file
****************/
int createBMDietDataFile(char *destFolder, char *name, MSEBoxModel *bm) {
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
    writeBMDietInfo(fid, bm);
    
    /* Exit from netCDF define mode */
    ncendef(fid);
    ncsync(fid);
    
    return (fid);
}

/*********//**
Routine to write age structured tracer info to general data file
***********/
void writeBMDietInfo(int fid, MSEBoxModel *bm) {
    char ann_name[BMSLEN];
    char ann_longname[BMSLEN];
    char unitstr[BMSLEN];
    int sp = 0;
    nc_type dt;
    int dim[3];
    int vid;
    int cohort, prey;
    
    snprintf(unitstr, sizeof(unitstr), "t km-2 day-1");
    
    /* set netCDF data type */
    dt = NC_DOUBLE;
    
    /* Get dimension ids */
    dim[0] = ncdimid(fid, "t");
    dim[1] = ncdimid(fid, "b");
    dim[2] = ncdimid(fid, "z");
    
    if(bm->ice_on){
        warn("No age specifiic diet data for ice layers, if you want this ask for new code\n");
    }
    
    /* Define time variable */
    vid = ncvardef(fid, "t", NC_DOUBLE, 1, dim);
    ncattput(fid, vid, "units", NC_CHAR, (int) strlen(bm->t_units) + 1, bm->t_units);
    
    /* Loop over the tracers */
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (sp == bm->which_check) {
            for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes * FunctGroupArray[sp].numStocks; cohort++) {
                for (prey=0; prey < bm->K_num_tot_sp; prey++) {
                    snprintf(ann_name, sizeof(ann_name), "%s%d_Diet_%s", FunctGroupArray[sp].groupCode, cohort+1, FunctGroupArray[prey].groupCode);
                    snprintf(ann_longname, sizeof(ann_longname), "Total consumption of %s by %s age %d at this location", FunctGroupArray[prey].name, FunctGroupArray[sp].name, cohort+1);
                    snprintf(unitstr, sizeof(unitstr), "Tonnes");
                
                    /* Define the variable */
                    //if (verbose > 1)
                        printf("doing diet data file creation for %s-%d eatig %s\n", FunctGroupArray[sp].name, cohort, FunctGroupArray[prey].name);
                        
                    vid = ncvardef(fid, ann_name, dt, 3, dim);
                        
                    /* Set the attributes */
                    ncattput(fid, vid, "units", NC_CHAR, (int) strlen(unitstr) + 1, unitstr);
                    ncattput(fid, vid, "long_name", NC_CHAR, (int) strlen(ann_longname) + 1, ann_longname);
                        
                    if (verbose > 1)
                        fprintf(stderr, "writeBMDietInfo: %s info written\n", FunctGroupArray[sp].name);
                }
            }
        }
    }
    
}

/*******************************************************************//**
Routine to write the age structured diet data to a netCDF file.
This routine assumes that the tracer information in the MSEBoxModel is valid and
corresponds with the netCDF file. Minimal checking for consistency
is done here. Does check to see if going to correct output file, depending
on whether fisheries or general data type.
*********************************************************************/
void writeBMDietData(int fid, int dump, MSEBoxModel *bm) {
    char strname[BMSLEN];
    doubleINPUT **val;
    long start[3];
    long count[3];
    int sp, cohort, prey;
    doubleINPUT value;

    // Normalise the data first - not normalising for now
    // NormaliseDietData(bm);
    
    if (verbose > 0)
        fprintf(stderr, "Entering writeBMDietData\n");
    
    /* Set netCDF library error handling */
    ncopts = NC_VERBOSE | NC_FATAL;
    
    /* Allocate temporary storage for variables - one variable at a time */
    val = (doubleINPUT **) alloc2dInput(bm->wcnz + bm->sednz, bm->nbox);
    
    /* Write time value */
    start[0] = dump;
    start[1] = 0;
    start[2] = 0;
    count[0] = 1L;
    count[1] = 1L;
    count[2] = 1L;
    
    value = (double) bm->t;
    ncvarput(fid, ncvarid(fid, "t"), start, count, &value);

    /* Set indices for writing epis */
    start[0] = dump;
    start[1] = 0;
    start[2] = 0;
    count[0] = 1;
    count[1] = bm->nbox;
    count[2] = bm->wcnz + bm->sednz;
    
    /* Loop over the tracers */
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        int vid;
        int b;
        int n;
        int k;
        int kdiff;
        
        if (sp == bm->which_check) {
            for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes * FunctGroupArray[sp].numStocks; cohort++) {
                for (prey=0; prey < bm->K_num_tot_sp; prey++) {
                    snprintf(strname, sizeof(strname), "%s%d_Diet_%s", FunctGroupArray[sp].groupCode, cohort+1, FunctGroupArray[prey].groupCode);

                    /* Get water column data from model storage */
                    for (b = 0; b < bm->nbox; b++) {
                        kdiff = bm->wcnz - bm->boxes[b].nz;
                        for (k = 0; k < bm->wcnz; k++) {
                            if (k < kdiff)
                                val[b][k] = 0.0;
                            else
                                val[b][k] = (double)(bm->totDiet[b][k - kdiff][sp][cohort][prey] * mg_2_tonne * bm->X_CN);  // The original value is in mg (as volume corrected as recorded) so needs to be converted to tonnes
                        }
                    
                        if(bm->boxes[b].type == LAND){
                            val[b][bm->wcnz - 1] = (double)(bm->totDiet[b][0][sp][cohort][prey] * mg_2_tonne * bm->X_CN);
                        }
                    }
                
                    /* Get sediment data from model storage */
                    for (b = 0; b < bm->nbox; b++)
                        for (k = 0; k < bm->sednz; k++)
                            val[b][k + bm->wcnz] = (double)bm->totDiet[b][k+bm->wcnz][sp][cohort][prey] * mg_2_tonne * bm->X_CN;

                
                    /* Get netCDF variable id */
                    vid = ncvarid(fid, strname);
                
                    /* Check double compatibility */
                    if ((n = ncvarsize(fid, vid)) != sizeof(doubleINPUT))
                        quit("writeBMDietData: %s has %d bytes per value in file, code compiled with %d\n", strname, n, sizeof(doubleINPUT));
                            
                    /* Write data */
                    ncvarput(fid, vid, start, count, val[0]);
                }
            }
        }
    }
    
    /* Free temporary storage */
    free2dInput((doubleINPUT **) val);

}

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

