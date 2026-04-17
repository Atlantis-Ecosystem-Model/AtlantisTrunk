/*********************************************************************

 File:           atsummaryIO.c

 Created:        Sun Dec 5 11:05:21 EST 2004

 Author:         Beth Fulton
 CSIRO Division of Marine Research

 Purpose:        Routines which deal with summary tracer data
 files. These files are in netCDF format
 and should be written and created using the routines below.

 Arguments:      See each routine below

 Returns:        See each routine below

 Revisions:

 21/10/2008 Bec Gorton
 Changed the order of the volume values written to the output netCDF file. The
 values in the water column are reversed, but the sediment is read in as is

 30-03-2009 Bec Gorton
 Added code to initialise the start and count arrays in writeBMphysData.

 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sjwlib.h>
#include <netcdf.h>
#include <atlantisboxmodel.h>
#include <atUtilLib.h>

/*******************************************************************************************
 Routine to create overall data summary file
 ****************/
int createBMSummaryDataFile(char *destFolder, char *name, MSEBoxModel *bm) {
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
	ncdimdef(fid, "z", bm->wcnz + bm->sednz);

	if (bm->ice_on){
		ncdimdef(fid, "icenz", bm->icenz);
		ncdimdef(fid, "iz", bm->wcnz + bm->sednz + bm->icenz);
	}

	/* Global attributes */
	ncattput(fid, NC_GLOBAL, "title", NC_CHAR, (int) strlen(bm->title) + 1, bm->title);
	ncattput(fid, NC_GLOBAL, "geometry", NC_CHAR, (int) strlen(bm->geomIfname) + 1, bm->geomIfname);
	ncattput(fid, NC_GLOBAL, "parameters", NC_CHAR, (int) strlen(bm->params) + 1, bm->params);
	ncattput(fid, NC_GLOBAL, "wcnz", NC_LONG, 1, &bm->wcnz);
	ncattput(fid, NC_GLOBAL, "sednz", NC_LONG, 1, &bm->sednz);

	/* Variables and their attributes */
	writeBMSummaryInfo(fid, bm);

	/* Exit from netCDF define mode */
	ncendef(fid);
	ncsync(fid);

	return (fid);
}

/***************************************************************************************
 Routine to write summary tracer info to general data file
 ***********/
void writeBMSummaryInfo(int fid, MSEBoxModel *bm) {
	int i = 0;
	nc_type dt;
	int dim[3];
	//int icedim[3];

	/* set netCDF data type */
	dt = NC_DOUBLE;

	/* Get dimension ids */
	dim[0] = ncdimid(fid, "t");
	dim[1] = ncdimid(fid, "b");
	dim[2] = ncdimid(fid, "z");

	/* Define time variable */
	i = ncvardef(fid, "t", NC_DOUBLE, 1, dim);
	ncattput(fid, i, "units", NC_CHAR, (int) strlen(bm->t_units) + 1, bm->t_units);

	/* Define the volume variable */
	i = ncvardef(fid, "volume", dt, 3, dim);
	ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("phys") + 1, "phys");
	ncattput(fid, i, "units", NC_CHAR, (int) strlen("m3") + 1, "m3");
	ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Volume") + 1, "Volume");

	/* Define the thickness variables */
	/* Note that nominal_dz is not time dependent */
	i = ncvardef(fid, "nominal_dz", dt, 2, &dim[1]);
	ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("phys") + 1, "phys");
	ncattput(fid, i, "units", NC_CHAR, (int) strlen("m") + 1, "m");
	ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Thickness") + 1, "Thickness");
	i = ncvardef(fid, "dz", dt, 3, dim);
	ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("phys") + 1, "phys");
	ncattput(fid, i, "units", NC_CHAR, (int) strlen("m") + 1, "m");
	ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Thickness") + 1, "Thickness");

	/* Define the sediment top index variable */
	i = ncvardef(fid, "topk", NC_SHORT, 2, dim);
	ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("phys") + 1, "phys");
	ncattput(fid, i, "units", NC_CHAR, (int) strlen("") + 1, "");
	ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Sediment top index") + 1, "Sediment top index");

	/* Loop over the tracers */
	for (i = 0; i < bm->ntracer; i++) {
		int vid;

		/* Check to see which output stream being dealt with */
		if (bm->tinfo[i].sumtype) {

			/* Ice tracers are written out separately */
			if(bm->ice_on){
				/* If this tracer is ice only then ignore*/
				if(bm->tinfo[i].inice == TRUE && bm->tinfo[i].inwc == FALSE && bm->tinfo[i].insed == FALSE )
					continue;
			}

			/* Define the variable */
			vid = ncvardef(fid, bm->tinfo[i].name, dt, 2, dim);

			/* Set the attributes - even tracers entered as epibenthos here so
			 olive knows how to present it*/
			ncattput(fid, vid, "bmtype", NC_CHAR, (int) strlen("epibenthos") + 1, "epibenthos");

			if (strcmp(bm->tinfo[i].name, "Chl_a") == 0) {
				ncattput(fid, vid, "units", NC_CHAR, (int) strlen("mg m-2") + 1, "mg m-2");
			} else {
				ncattput(fid, vid, "units", NC_CHAR, (int) strlen("t km-2") + 1, "t km-2");
			}

			ncattput(fid, vid, "long_name", NC_CHAR, (int) strlen(bm->tinfo[i].long_name) + 1, bm->tinfo[i].long_name);
			ncattput(fid, vid, "dtype", NC_LONG, 1, &bm->tinfo[i].dtype);
			ncattput(fid, vid, "sumtype", NC_LONG, 1, &bm->tinfo[i].sumtype);

			if (verbose > 1)
				fprintf(stderr, "writeBMSummaryInfo: %s info written\n", bm->tinfo[i].name);

		}
	}

	/* Loop over the epi variables */
	for (i = 0; i < bm->nepi; i++) {
		int vid;

		if (bm->einfo[i].sumtype) {

			/* Define the variable */
			vid = ncvardef(fid, bm->einfo[i].name, dt, 2, dim);

//			if(bm->ice_on == TRUE){
//				if(bm->tinfo[i].inice == TRUE){
//					vid = ncvardef(fid, bm->tinfo[i].name, dt, 3, icedim);
//				}else{
//					vid = ncvardef(fid, bm->tinfo[i].name, dt, 3, dim);
//				}
//			}else{
//				vid = ncvardef(fid, bm->tinfo[i].name, dt, 3, dim);
//			}


			/* Set the attributes */
			ncattput(fid, vid, "bmtype", NC_CHAR, (int) strlen("epibenthos") + 1, "epibenthos");
			ncattput(fid, vid, "units", NC_CHAR, (int) strlen("t km-2") + 1, "t km-2");
			ncattput(fid, vid, "long_name", NC_CHAR, (int) strlen(bm->einfo[i].long_name) + 1, bm->einfo[i].long_name);
			ncattput(fid, vid, "dtype", NC_LONG, 1, &bm->einfo[i].dtype);
			ncattput(fid, vid, "sumtype", NC_LONG, 1, &bm->einfo[i].sumtype);

			if (verbose > 1)
				fprintf(stderr, "writeBMSummaryInfo: %s info written\n", bm->einfo[i].name);
		}
	}
}

/*********************************************************************
 Routine to write the summary tracer data to a netCDF file. This routine
 assumes that the tracer information in the MSEBoxModel is valid and
 corresponds with the netCDF file. Minimal checking for consistency
 is done here. Does check to see if going to correct output file.
 *********************************************************************/
void writeBMSummaryData(int fid, int dump, MSEBoxModel *bm) {
	int i = 0;//, j = 0;
	doubleINPUT *val;
	doubleINPUT **valdz;
	doubleINPUT **valvol;
	doubleINPUT value;

	short *topk;
	long start[3];
	long count[3];

	int sizen, b, k, kdiff;

	if (verbose > 0)
		fprintf(stderr, "Entering writeBMSummaryData\n");

	/* Set netCDF library error handling */
	ncopts = NC_VERBOSE | NC_FATAL;

	/* Allocate temporary storage for one tracer */

	val = (doubleINPUT *) alloc1dInput(bm->nbox);
	valdz = (doubleINPUT **) alloc2dInput(bm->wcnz + bm->sednz, bm->nbox);
	valvol = (doubleINPUT **) alloc2dInput(bm->wcnz + bm->sednz, bm->nbox);

	topk = s_alloc1d(bm->nbox);

	/* Write time value */
	start[0] = dump;
	start[1] = 0;
	start[2] = 0;
	count[0] = 1L;
	count[1] = 1L;
	count[2] = 1L;

	value = (double) bm->t;
	ncvarput(fid, ncvarid(fid, "t"), start, count, &value);

	/* Set indices for writing tracers */
	start[0] = dump;
	start[1] = 0;
	start[2] = 0;
	count[0] = 1;
	count[1] = bm->nbox;
	count[2] = bm->wcnz + bm->sednz;

	/* Write physical variables */
    ncvarput(fid, ncvarid(fid, "nominal_dz"), &start[1], &count[1], bm->nom_dz[0]);
	ncvarput(fid, ncvarid(fid, "dz"), start, count, bm->dz[0]);
	ncvarput(fid, ncvarid(fid, "volume"), start, count, bm->vol[0]);

	/* Slide the dz and volume in the same way the tracers are. Old
	 code preserved below should reversal be necessary.
	 */
	for (b = 0; b < bm->nbox; b++) {
		kdiff = bm->wcnz - bm->boxes[b].nz;
		for (k = 0; k < bm->wcnz; k++) {
			if (k < kdiff) {
				valdz[b][k] = 0.0;
				valvol[b][k] = 0.0;
			} else {
				valdz[b][k] = bm->boxes[b].dz[k - kdiff];
				valvol[b][k] = bm->boxes[b].volume[k - kdiff];
			}
		}
	}

	/* Get sediment data from model storage */
	for (b = 0; b < bm->nbox; b++) {
		for (k = 0; k < bm->sednz; k++) {
			valdz[b][k + bm->wcnz] = bm->boxes[b].sm.dz[k];
			valvol[b][k + bm->wcnz] = bm->boxes[b].sm.volume[k];
		}
	}

	/* Write data */
	//ncvarput(fid,ncvarid(fid,"dz"),start,count,bm->dz[0]);
	ncvarput(fid, ncvarid(fid, "dz"), start, count, valdz[0]);
	//ncvarput(fid,ncvarid(fid,"volume"),start,count,bm->vol[0]);
	ncvarput(fid, ncvarid(fid, "volume"), start, count, valvol[0]);

	//ncvarput(fid,ncvarid(fid,"dz"),start,count,bm->dz[0]);
	//ncvarput(fid,ncvarid(fid,"volume"),start,count,bm->vol[0]);

	for (b = 0; b < bm->nbox; b++)
		topk[b] = (short)bm->boxes[b].sm.topk;
	ncvarput(fid, ncvarid(fid, "topk"), start, count, topk);

	/* Loop over each tracer */
	for (i = 0; i < bm->ntracer; i++) {
		int vid;
		int k;


		/* Ice tracers are written out separately */
		if(bm->ice_on){
			/* If this tracer is ice only then ignore*/
			if(bm->tinfo[i].inice == TRUE && bm->tinfo[i].inwc == FALSE && bm->tinfo[i].insed == FALSE )
				continue;
		}

		if (bm->tinfo[i].sumtype) {
			/* Get netCDF variable id */
			vid = ncvarid(fid, bm->tinfo[i].name);

			/* Check double compatibility */
			if ((sizen = ncvarsize(fid, vid)) != sizeof(doubleINPUT))
				quit("writeBMSummaryData: %s has %d bytes per value in file, code compiled with %d\n", bm->tinfo[i].name, sizen, sizeof(doubleINPUT));

			/* Check to see if chlorophyll */
			if (strcmp(bm->tinfo[i].name, "Chl_a") == 0) {
				/* Get water column data from model storage */
				for (b = 0; b < bm->nbox; b++) {
					val[b] = 0.0;
					for (k = 0; k < bm->wcnz; k++) {
						val[b] += bm->wctr[b][k][i] * bm->boxes[b].dz[k];
					}
				}

				/* No sediment data required from model storage and
				 no unit conversion necessary as assuming mg/m2 */

			} else {

				for (b = 0; b < bm->nbox; b++) {
					val[b] = 0.0;
				}
				/* Check to see if water column variable or vertebrate
				 summary variable (which are set with inwc and insed == 0) */
				if (bm->tinfo[i].inwc || (!bm->tinfo[i].inwc && !bm->tinfo[i].insed)) {
					/* Get water column data from model storage */
					for (b = 0; b < bm->nbox; b++) {
						val[b] = 0.0;
						for (k = 0; k < bm->wcnz; k++) {
							val[b] += bm->wctr[b][k][i] * bm->boxes[b].dz[k];
						}
					}
				}

				if (bm->tinfo[i].insed) {
					/* Get sediment data from model storage */
					for (b = 0; b < bm->nbox; b++) {
						for (k = 0; k < bm->sednz; k++) {
							val[b] += bm->sedtr[b][k][i] * bm->boxes[b].sm.dz[k];
						}
					}
				}

				/* Do units conversion - assuming units are t/km2 (= g/m2) */
				for (b = 0; b < bm->nbox; b++)
					val[b] *= (bm->X_CN * mg_2_g);
			}

			/* Write data */
			ncvarput(fid, vid, start, count, val);
		}
	}

	/* Free temporary storage */
	free1dInput((doubleINPUT *) val);
	free2dInput((doubleINPUT **) valdz);
	free2dInput((doubleINPUT **) valvol);

	s_free1d(topk);
}

/*********************************************************************
 Routine to write the summary epi data to a netCDF file. This routine
 assumes that the epi information in the MSEBoxModel is valid and
 corresponds with the netCDF file. Minimal checking for consistency
 is done here.
 *********************************************************************/
void writeBMSummaryEpiData(int fid, int dump, MSEBoxModel *bm) {
	int i = 0;
	doubleINPUT *val;
	long start[2];
	long count[2];

	if (verbose > 0)
		fprintf(stderr, "Entering writeBMSummaryEpiData\n");

	/* Set netCDF library error handling */
	ncopts = NC_VERBOSE | NC_FATAL;

	/* Allocate temporary storage for one epi variable */
	val = (doubleINPUT *) alloc1dInput(bm->nbox);

	/* Set indices for writing epis */
	start[0] = dump;
	start[1] = 0;
	count[0] = 1;
	count[1] = bm->nbox;

	/* Loop over each epi */
	for (i = 0; i < bm->nepi; i++) {
		int vid;
		int b;
		int n;

		if (bm->einfo[i].sumtype) {
			/* Get netCDF variable id */
			vid = ncvarid(fid, bm->einfo[i].name);

			/* Check double compatibility */
			if ((n = ncvarsize(fid, vid)) != sizeof(doubleINPUT))
				quit("writeBMSummaryEpiData: %s has %d bytes per value in file, code compiled with %d\n", bm->einfo[i].name, n, sizeof(doubleINPUT));

			/* Get epi data from model storage */
			for (b = 0; b < bm->nbox; b++) {
				val[b] = bm->epi[b][i];

				/* Do units conversion - assuming units are t/km2 (= g/m2) */
				val[b] *= (bm->X_CN * mg_2_g);
			}

			/* Write data */
			ncvarput(fid, vid, start, count, val);
		}
	}

	/* Free temporary storage */
	free1dInput((doubleINPUT *) val);

}

