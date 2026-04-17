/*******************************************************************//**
 \file
 \brief Routines which deal with ice tracer data files. These files are in netCDF format
 and should be read, written and created using the routines below.


 This file reads in the definitions of the tracers that are in the ice only - with bmtype icetracer.
 It also reads in the ice data for those tracers that are defined in both the ice and the water column - that
 have inice set to 1.

 The ice only tracers are appended to the end of the tinfo array.
 The values of the tracers in the ice are read into the bm->icetr array. This array is defined for all boxes, for all tracers (to allow for easy indexing) for the
 number of ice layers as defined in global 'icenz' attribute in the initial conditions netcdf file.



 File:           aticeIO.c

 Created:        15-12-2011

 Author:         Beth Fulton
				 CSIRO Marine and Atmospheric Research

 Purpose:        Routines which deal with ice tracer data
				 from general data files. These files are in netCDF format
				 and should be read, written and created
				 using the routines below.

 Arguments:      See each routine below

 Returns:        See each routine below

 Revisions:      15/12/2011 Beth Fulton
				 Ported across from attracerIO.c

				 15/03/2012 Beth Fulton
				 Added time series read in of ice types

 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sjwlib.h>
#include <netcdf.h>
#include <atlantisboxmodel.h>
#include <atUtilLib.h>

double get_Loaded_Ice(MSEBoxModel *bm, FILE *llogfp, int box_id, int iceclass_id, int ice_or_depth);
void get_ice_property(MSEBoxModel *bm, IcePropertyData *propInput);
void open_iceprop(MSEBoxModel *bm, IcePropertyData *propInput);
void close_iceprop(MSEBoxModel *bm, IcePropertyData *propInput);

void CDF_iceBM(MSEBoxModel *bm);
void Enter_IceBM(MSEBoxModel *bm, double **ice_data_array, double *dzData);
void ice_cdf_init(MSEBoxModel *bm);
void Ice_Read_Time_Series(MSEBoxModel *bm, char *name, char *tsname, char *t_units, IceTimeSeries **ts, int *nts, int *typets);

void freeCDFIceFiles(MSEBoxModel *bm);
void Ice_Free_Time_Series(IceTimeSeries *icets, int nnts);
void freeIceFiles(MSEBoxModel *bm);

/* Ice related properties */
int tsIceWarned;
int ntsIce; /**< Number of ice time series (should match one per box at most, but keep this int as check) */
int tsIcetype; /**< Whether to use interpolated or last valid entries from time-series */
int *tsiceid; /**< Array matching box ids to ice time series entry ids */
IceTimeSeries *tsIce; /**< List of ice time series (one per ice class) */



/*******************************************************************//**
 Routine to read the ice tracer information from a netCDF file.
 This reads in the tracers that are active in the ice only. The definitions of the tracers that are active in both
 the ice and the rest of the water column are read in with the rest of the water column tracers.
 This routine assumes that the same file format as in attracerIO.c

 The basic code for these routines have been copied from at tracerIO.c - be careful of copy and paste errors

 *********************************************************************/
void readBMIceInfo(int fid,char *fileName, MSEBoxModel *bm) {

	int ndims = 0; /* Number of dimensions */
	int nvars = 0; /* Number of variables */
	int ngatts = 0; /* Number of global attributes */
	int recdim = 0; /* ID of unlimited dimension */
	int t_id = 0; /* Time dimension id */
	int b_id = 0; /* Box dimension id */
	int icenz_id = 0; /* Layer dimension id */
	int dims[MAX_NC_DIMS];
	int *idlist = 0;
	long n = 0;
	int i = 0;
	int b = 0;
	int ntr = 0;
	nc_type dt;
	int nd;
	int d[3];
	int na;
	int len;

	int index;
	doubleINPUT value;
	nc_type  t_type;   /* global attribute type */
	int      t_len;    /* global attribute length */

	if (verbose > 1)
		fprintf(stderr, "Entering readBMIceInfo\n");


	/* Set netCDF library error handling */
	ncopts = NC_VERBOSE;


	/* Check numbers of layers from attributes against geometry */
	if(ncattinq(fid, NC_GLOBAL, "icenz", &t_type, &t_len) == -1){
		bm->ice_on = FALSE;
		bm->num_active_habitats = ICE_BASED;
		return;
	}

	bm->num_active_habitats = nlevel_id;
	bm->ice_on = TRUE;


	/* Set netCDF library error handling */
	ncopts = NC_VERBOSE | NC_FATAL;

	/* Inquire about this file */
	ncinquire(fid, &ndims, &nvars, &ngatts, &recdim);
	if ((t_id = ncdimid(fid, "t")) == -1)
		quit("readBMIceInfo: no t dimension");
	if (t_id != recdim)
		quit("readBMIceInfo: t dimension not unlimited\n");
	if ((b_id = ncdimid(fid, "b")) == -1)
		quit("readBMIceInfo: no b dimension\n");
	if ((icenz_id = ncdimid(fid, "icenz")) == -1)
		quit("readBMIceInfo: no icenz dimension\n");


	ncattget(fid, NC_GLOBAL, "icenz", &bm->icenz);

	/* Allocate the space required */
	for (n = 0; n < bm->nbox; n++) {
		bm->boxes[n].ice.gridz = alloc1d(bm->icenz + 1);
		bm->boxes[n].ice.volume = alloc1d(bm->icenz + 1);
		bm->boxes[n].ice.cellz = alloc1d(bm->icenz + 1);
		bm->boxes[n].ice.nz = bm->icenz;
	}

	ncdiminq(fid, icenz_id, NULL, &n);
	if (n != bm->icenz)
		quit("readBMIceInfo: Number of ice layers doesn't match geometry\n");

	/* Find and count tracer variables */
	dims[0] = t_id;
	dims[1] = b_id;
	dims[2] = icenz_id;
	idlist = i_alloc1d(nvars);

	/* Get number of tracer variables */
	n = ncvarfind(fid, 3, dims, "bmtype", "icetracer", idlist);

	if (n < 1)
		quit("readBMIceInfo: No tracers in file\n");


	if (verbose > 1)
		fprintf(stderr, "readBMIceInfo: %ld icetracers in file\n", n);

	/* Expand space for ice tracer info */
	ntr = bm->ntracer;
	ntr += (int)(n);

	if ((bm->tinfo = (TracerInfo *)realloc(bm->tinfo, sizeof(TracerInfo)*(size_t)(ntr + 1))) == NULL)
		quit("readBMIceInfo: Can't allocate memory for info\n");

	/* Loop over the tracer variables to read info */
	for (index = 0; index  < n; index++) {


		i = index + bm->ntracer;

		bm->tinfo[i].isUsed = FALSE;	/* Assume variable is not used until its matched to a group */

		/* Tracer number (must be equal to latest entry, with entries starting at bm->ntracer) */
		bm->tinfo[i].n = i + bm->ntracer;
		bm->tinfo[i].n = i;

		/* Tracer name */
		ncvarinq(fid, idlist[index], bm->tinfo[i].name, &dt, &nd, d, &na);

		/* First check the dimensions are what they should be - check we have ice data. */
		if(d[0] != t_id)
			quit("Tracer %s in file %s should have dimensions t, b, icenz\n", bm->tinfo[i].name, fileName);
		if(d[1] != b_id)
			quit("Tracer %s in file %s should have dimensions t, b, icenz\n", bm->tinfo[i].name, fileName);
		if(d[2] != icenz_id)
			quit("Tracer %s in file %s should have dimensions t, b, icenz\n", bm->tinfo[i].name, fileName);

		memset(bm->tinfo[i].units, 0, sizeof(bm->tinfo[i].units));
		memset(bm->tinfo[i].long_name, 0, sizeof(bm->tinfo[i].long_name));

		if( verbose > 1 )
			fprintf(stderr,"readBMIceInfo: %s info reading\n",bm->tinfo[i].name);

		/* Tracer attributes */
		/* Units */
		if (verbose > 2)
			fprintf(stderr, "readBMIceInfo: reading units\n");
		ncattget(fid, idlist[index], "units", bm->tinfo[i].units);

		if (verbose > 2)
			fprintf(stderr, "readBMIceInfo: reading long_name\n");
		ncattget(fid, idlist[index], "long_name", bm->tinfo[i].long_name);

		/* Dtype */
		if (verbose > 2)
			fprintf(stderr, "readBMIceInfo: reading dtype\n");
		ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "dtype", &dt, &len);

		if (nctypelen(dt) != sizeof(bm->tinfo[i].dtype))
			quit("readBMIceInfo: dtype attribute wrong type (%s)\n", bm->tinfo[i].name);
		ncattget(fid, idlist[index], "dtype", &bm->tinfo[i].dtype);

		/* SumType */
		if (verbose > 2)
			fprintf(stderr, "readBMIceInfo: reading sumtype\n");
		ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "sumtype", &dt, &len);

		if (nctypelen(dt) != sizeof(bm->tinfo[i].sumtype))
			quit("readBMIceInfo: sumtype attribute wrong type (%s)\n", bm->tinfo[i].name);
		ncattget(fid, idlist[index], "sumtype", &bm->tinfo[i].sumtype);

		/* in water column*/
		if (verbose > 2)
			fprintf(stderr, "readBMIceInfo: reading inwc\n");
		ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "inwc", &dt, &len);

		if (nctypelen(dt) != sizeof(bm->tinfo[i].inwc))
			quit("readBMIceInfo: inwc attribute wrong type (%s)\n", bm->tinfo[i].name);
		ncattget(fid, idlist[index], "inwc", &bm->tinfo[i].inwc);

		/* in sediment */
		if (verbose > 2)
			fprintf(stderr, "readBMIceInfo: reading insed\n");
		ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "insed", &dt, &len);

		if (nctypelen(dt) != sizeof(bm->tinfo[i].insed))
			quit("readBMIceInfo: insed attribute wrong type (%s)\n", bm->tinfo[i].name);
		ncattget(fid, idlist[index], "insed", &bm->tinfo[i].insed);

		/* in ice by definition */
		bm->tinfo[i].inice = 1;

		/* Decay rate */
		if (verbose > 2)
			fprintf(stderr, "readBMIceInfo: reading decay\n");
		ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "decay", &dt, &len);

		if (nctypelen(dt) != sizeof(doubleINPUT))
			quit("readBMIceInfo: decay attribute wrong type (%s)\n", bm->tinfo[i].name);
		ncattget(fid, idlist[index], "decay", &value);
		bm->tinfo[i].decay = (double) value;

		/* dissolved flag */
		if (verbose > 2)
			fprintf(stderr, "readBMIceInfo: reading dissol\n");
		ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "dissol", &dt, &len);

		if (nctypelen(dt) != sizeof(bm->tinfo[i].dissol))
			quit("readBMIceInfo: dissol attribute wrong type (%s)\n", bm->tinfo[i].name);
		ncattget(fid, idlist[index], "dissol", &bm->tinfo[i].dissol);

		/* particulate flag */
		if (verbose > 2)
			fprintf(stderr, "readBMIceInfo: reading partic\n");
		ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "partic", &dt, &len);

		if (nctypelen(dt) != sizeof(bm->tinfo[i].partic))
			quit("readBMIceInfo: partic attribute wrong type (%s)\n", bm->tinfo[i].name);
		ncattget(fid, idlist[index], "partic", &bm->tinfo[i].partic);

		/* Get additional attributes for particulate tracers */
		if (bm->tinfo[i].partic != 0) {
			/* Active vs Passive rating */
			if (verbose > 2)
				fprintf(stderr, "readBMIceInfo: reading passive\n");
			ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "passive", &dt, &len);

			if (nctypelen(dt) != sizeof(bm->tinfo[i].passive))
				quit("readBMIceInfo: passive attribute wrong type (%s)\n", bm->tinfo[i].name);
			ncattget(fid, idlist[index], "passive", &bm->tinfo[i].passive);

			/* Settling velocity */
			if (verbose > 2)
				fprintf(stderr, "readBMIceInfo: reading svel\n");
			ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "svel", &dt, &len);

			if (nctypelen(dt) != sizeof(doubleINPUT))
				quit("readBMIceInfo: svel attribute wrong type (%s)\n", bm->tinfo[i].name);
			ncattget(fid, idlist[index], "svel", &value);
			bm->tinfo[i].svel = (double) value;

			/* Extra settling velocity - due to migration of nutrient limitation */
			if (verbose > 2)
				fprintf(stderr, "readBMIceInfo: reading xvel\n");
			ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "xvel", &dt, &len);

			if (nctypelen(dt) != sizeof(doubleINPUT))
				quit("readBMIceInfo: xvel attribute wrong type (%s)\n", bm->tinfo[i].name);
			ncattget(fid, idlist[index], "xvel", &value);
			bm->tinfo[i].xvel = (double) value;

			/* Particle size */
			if (verbose > 2)
				fprintf(stderr, "readBMIceInfo: reading psize\n");
			ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "psize", &dt, &len);

			if (nctypelen(dt) != sizeof(doubleINPUT))
				quit("readBMIceInfo: psize attribute wrong type (%s)\n", bm->tinfo[i].name);
			ncattget(fid, ncvarid(fid, bm->tinfo[i].name), "psize", &value);
			bm->tinfo[i].psize = (double) value;

			/* Particle bulk density */
			if (verbose > 2)
				fprintf(stderr, "readBMIceInfo: reading b_dens\n");
			ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "b_dens", &dt, &len);

			if (nctypelen(dt) != sizeof(doubleINPUT))
				quit("readBMIceInfo: b_dens attribute wrong type (%s)\n", bm->tinfo[i].name);
			ncattget(fid, idlist[index], "b_dens", &value);
			bm->tinfo[i].b_dens = (double) value;

			/* Initial concentration for deposition */
			if (verbose > 2)
				fprintf(stderr, "readBMIceInfo: reading i_conc\n");
			ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "i_conc", &dt, &len);

			if (nctypelen(dt) != sizeof(doubleINPUT))
				quit("readBMIceInfo: i_conc attribute wrong type (%s)\n", bm->tinfo[i].name);
			ncattget(fid, idlist[index], "i_conc", &value);
			bm->tinfo[i].i_conc = (double) value;

			if (verbose > 2)
				fprintf(stderr, "readBMIceInfo: reading inice\n");
			ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "inice", &dt, &len);

			if (nctypelen(dt) != sizeof(bm->tinfo[i].inice))
				quit("readBMIceInfo: inice attribute wrong type (%s)\n", bm->tinfo[i].name);
			ncattget(fid, idlist[index], "inice", &bm->tinfo[i].inice);

		}

		bm->tinfo[i].flagid = 1;

		/*	if( verbose > 1 )
		 fprintf(stderr,"readBMIceInfo: %s info read\n",bm->tinfo[i].name);
		 fprintf(stderr,"with units: %s\n",bm->tinfo[i].units);
		 fprintf(stderr,"and long name: %s\n",bm->tinfo[i].long_name);
		 */
	}

	/* Store number of tracers */
	bm->ntracer = ntr;
	for (b = 0; b < bm->nbox; b++) {
		bm->boxes[b].ice.ntr += (int)(n);
		bm->boxes[b].ice.currentnz = bm->icenz;
		printf("dbm->boxes[%d].ice.currentnz = %d\n",b, bm->boxes[b].ice.currentnz);
	}

	bm->nicetracer = (int)n;
	fprintf(stderr, "Read in %d (%d) extra ice tracers (total tracers %d)\n", (int)(n), ntr, bm->ntracer);

	/* Free tracer id array */
	i_free1d(idlist);
}

/**** Free any memory allocated during a readBMIceInfo call.
 ****/
void freeBMIceInfo(MSEBoxModel *bm) {

	int n;

	for (n = 0; n < bm->nbox; n++) {
		free1d(bm->boxes[n].ice.gridz);
		free1d(bm->boxes[n].ice.volume);
		free1d(bm->boxes[n].ice.cellz);
	}

	freeIceFiles(bm);

}

/*********//**
 Routine to write ice tracer info to general data file
 ***********/
void writeBMIceInfo(int fid, MSEBoxModel *bm, int dtype) {

	int i = 0;
	nc_type dt;
	int dim[3];
	int vid;
	doubleINPUT value;

	/* set netCDF data type */
	dt = NC_DOUBLE;

	/* Get dimension ids */
	dim[0] = ncdimid(fid, "t");
	dim[1] = ncdimid(fid, "b");
	dim[2] = ncdimid(fid, "icenz");

	/* Loop over the tracers */
	for (i = 0; i < bm->ntracer; i++) {
		/* Check to see which output stream being dealt with */
		if (dtype == bm->tinfo[i].dtype && bm->tinfo[i].flagid && bm->tinfo[i].inice) {


			if(bm->tinfo[i].inice == TRUE && (bm->tinfo[i].inwc == TRUE || bm->tinfo[i].insed == TRUE) )
				continue;

			/* Define the variable */
			if (verbose > 1)
				printf("bm->tinfo[%d].name = %s\n", i, bm->tinfo[i].name);

			vid = ncvardef(fid, bm->tinfo[i].name, dt, 3, dim);

			/* Set the attributes */
			ncattput(fid, vid, "bmtype", NC_CHAR, (int) strlen("icetracer") + 1, "icetracer");
			ncattput(fid, vid, "units", NC_CHAR, (int) strlen(bm->tinfo[i].units) + 1, bm->tinfo[i].units);
			ncattput(fid, vid, "long_name", NC_CHAR, (int) strlen(bm->tinfo[i].long_name) + 1, bm->tinfo[i].long_name);
			ncattput(fid, vid, "dtype", NC_LONG, 1, &bm->tinfo[i].dtype);
			ncattput(fid, vid, "sumtype", NC_LONG, 1, &bm->tinfo[i].sumtype);
			ncattput(fid, vid, "inwc", NC_LONG, 1, &bm->tinfo[i].inwc);
			ncattput(fid, vid, "insed", NC_LONG, 1, &bm->tinfo[i].insed);
			value = bm->tinfo[i].decay;
			ncattput(fid, vid, "decay", NC_DOUBLE, 1, &value);
			ncattput(fid, vid, "dissol", NC_LONG, 1, &bm->tinfo[i].dissol);
			ncattput(fid, vid, "partic", NC_LONG, 1, &bm->tinfo[i].partic);
			if (bm->tinfo[i].partic) {
				value = bm->tinfo[i].svel;
				ncattput(fid, vid, "svel", NC_DOUBLE, 1, &value);
				value = bm->tinfo[i].xvel;
				ncattput(fid, vid, "xvel", NC_DOUBLE, 1, &value);
				value = bm->tinfo[i].psize;
				ncattput(fid, vid, "psize", NC_DOUBLE, 1, &value);
				value = bm->tinfo[i].b_dens;
				ncattput(fid, vid, "b_dens", NC_DOUBLE, 1, &value);
				value = bm->tinfo[i].i_conc;
				ncattput(fid, vid, "i_conc", NC_DOUBLE, 1, &value);
			}
			value = bm->tinfo[i].inice;
			ncattput(fid, vid, "inice", NC_LONG, 1, &value);

			if (verbose > 1)
				fprintf(stderr, "writeBMIceInfo: %s info written\n", bm->tinfo[i].name);

		}
	}
}

/*******************************************************************//**
 Routine to read the ice tracer data from a netCDF file. This routine
 assumes that the tracer information in the MSEBoxModel is valid and
 corresponds with the netCDF file. Minimal checking for consistency
 is done here.
 *********************************************************************/
void readBMIceData(int fid, int dump, MSEBoxModel *bm) {

	doubleINPUT **val;
	long start[3];
	long icestart[3];
	long count[3];
	long n = 0;
	int b, status, k, i, rh_id;

	/* Set netCDF library error handling */
	ncopts = NC_VERBOSE | NC_FATAL;

	/* Check that requested dump exists in file */
	ncdiminq(fid, ncdimid(fid, "t"), NULL, &n);
	if (dump >= n)
		quit("readBMIceData: dump %d not in file (%ld records)\n", dump, n);

	/* Allocate temporary storage for one tracer */
	val = (doubleINPUT **) alloc2dInput(bm->icenz, bm->nbox);

	/* Set indices for reading tracers */
	start[0] = dump;
	start[1] = 0;
	start[2] = 0;

	icestart[0] = dump;
	icestart[1] = 0;
	icestart[2] = 0;

	count[0] = 1;
	count[1] = bm->nbox;
	count[2] = bm->icenz;


	/* Read physical variables */
	status = nc_inq_varid (fid, "ice_dz", &rh_id);
	if (status != NC_NOERR) {
		quit("ice_dz not defined in initial conditions file\n");
	}

	ncvarread(fid, "ice_dz", sizeof(doubleINPUT), &start[1], &count[1], (void *) val[0]);
	checkNetCDFData2D("readBMIceData", "ice_dz", val,  bm->nbox, bm->icenz);

	for (b = 0; b < bm->nbox; b++) {
		for (i = 0; i < bm->icenz; i++) {
			bm->boxes[b].ice.dz[i] = val[b][i];
			printf("bm->boxes[%d].ice.dz[%d]= %e\n", b, i, bm->boxes[b].ice.dz[i]);
		}
	}

	/* Loop over each tracer */
	for (i = 0; i < bm->ntracer; i++) {

		/* Check if really an ice tracer */
		if(!bm->tinfo[i].inice){
			for (b = 0; b < bm->nbox; b++) {
				for (k = 0; k < bm->icenz; k++) {
					bm->icetr[b][k][i] = 0.0;  // Not actually in the ice so zero away
				}
			}
			/* Skip to the next tracer */
			continue;
		}

		printf(" bm->tinfo[i].name, = %s\n", bm->tinfo[i].name);

		if(bm->tinfo[i].insed || bm->tinfo[i].inwc){
			printf("in gere\n");
			/* Read data */
			ncvarread(fid, bm->tinfo[i].name, sizeof(doubleINPUT), icestart, count, val[0]);

		}else{
			printf("in gere 2\n");

			/* Read data */
			ncvarread(fid, bm->tinfo[i].name, sizeof(doubleINPUT), start, count, val[0]);

		}


		/* Move ice tracer data to model storage */
		for (b = 0; b < bm->nbox; b++) {
			for (k = 0; k < bm->icenz; k++) {
				bm->icetr[b][k][i] = (double) val[b][k];


				if (!(_finite(bm->icetr[b][k][i]))) {
					quit("readBMIceData - box: %d, layer: %d %s (%d) localpool set to: %e.\n", b, k, bm->tinfo[i].name, i, bm->icetr[b][k][i]);
				}
			}
		}
	}

	/* Free temporary storage */
	free2dInput((doubleINPUT **) val);
	//free2d((double **)val);

}

/*******************************************************************//**
 Routine to write the ice tracer data to a netCDF file. This routine
 assumes that the tracer information in the MSEBoxModel is valid and
 corresponds with the netCDF file. Minimal checking for consistency
 is done here. Does check to see if going to correct output file, depending
 on whether fisheries or general data type.
 *********************************************************************/
void writeBMIceData(int fid, int dump, MSEBoxModel *bm, int dtype) {

	int i = 0;
	doubleINPUT **val;
	long start[3];
	long count[3];
	long icestart[3];
	int n = 0;

	if (verbose > 0)
		fprintf(stderr, "Entering writeBMIceData\n");

	/* Set netCDF library error handling */
	ncopts = NC_VERBOSE | NC_FATAL;

	/* Allocate temporary storage for one tracer */
	val = (doubleINPUT **) alloc2dInput(bm->icenz, bm->nbox);

	/* Set indices for writing tracers */
	start[0] = dump;
	start[1] = 0;
	start[2] = 0;
	count[0] = 1;
	count[1] = bm->nbox;
	count[2] = bm->icenz;

	icestart[0] = dump;
	icestart[1] = 0;
	icestart[2] = 0;

	/* Loop over each tracer */
	for (i = 0; i < bm->ntracer; i++) {
		int vid;
		int b;
		int k;
		int kdiff;

        //fprintf(bm->logFile, "Time: %e tracer %s (%d) has inice: %d\n", bm->dayt, bm->tinfo[i].name, i, bm->tinfo[i].inice);
        
        
		/* Check if really an ice tracer */
		if(!bm->tinfo[i].inice){
			continue;
		}

		/* Check to see which output stream being dealt with */
		if (dtype == bm->tinfo[i].dtype && bm->tinfo[i].flagid) {

			/* Get netCDF variable id */
			vid = ncvarid(fid, bm->tinfo[i].name);

			/* Check double compatibility */
			if ((n = ncvarsize(fid, vid)) != sizeof(doubleINPUT))
				quit("writeBMIceData: %s has %d bytes per value in file, code compiled with %d\n", bm->tinfo[i].name, n, sizeof(doubleINPUT));

			/* Get ice data from model storage */
			for (b = 0; b < bm->nbox; b++) {
				kdiff = bm->icenz - bm->boxes[b].ice.currentnz;
				for (k = 0; k < bm->icenz; k++) {
					if (k < kdiff)
						val[b][k] = 0.0;
					else
						// This flip is because viewers assume 0 at surface for water column and trying to be consistent with that
						val[b][k] = (double)(bm->icetr[b][k - kdiff][i]);
				}
			}


			/* Write data */
			if(bm->tinfo[i].insed || bm->tinfo[i].inwc){
				ncvarput(fid, vid, icestart, count, val[0]);

			}else{
				ncvarput(fid, vid, start, count, val[0]);
			}
		}
	}

	/* Free temporary storage */
	free2dInput((doubleINPUT **) val);

}

/******************************************************** Read in ice time series **************************************************/

void Get_Ice(MSEBoxModel *bm, FILE *llogfp)
{
	int i, b;
	double new_val = 0.0;

	if (verbose)
		fprintf(stderr, "Entering getIce\n");

	switch(bm->kind_ice_model) {
	case ts_file:
		for(i=0; i<bm->K_num_ice_classes; i++){
			for(b=0; b<bm->nbox; b++){
				if(bm->boxes[b].type != BOUNDARY) {
					new_val = get_Loaded_Ice(bm, llogfp, b, i, 1);
					bm->ts_ice_data_array[b][i] = new_val;
					new_val = get_Loaded_Ice(bm, llogfp, b, i, 0);
					bm->ts_ice_dz_data[b] = new_val;
				}
			}
		}
		Enter_IceBM(bm, bm->ts_ice_data_array, bm->ts_ice_dz_data); // Use this not a direct read so that get the vertical distribution
		break;
	case cdf_file:
		CDF_iceBM(bm);
		for(i=0; i<bm->K_num_ice_classes; i++){
			for(b=0; b<bm->nbox; b++){
				bm->ts_ice_data_array[b][i] = bm->iceinput.dataBuffer[b][i];
				bm->ts_ice_dz_data[b] = bm->iceinput.dzBuffer[b];
			}
		}
		Enter_IceBM(bm, bm->ts_ice_data_array, bm->ts_ice_dz_data);
		break;
	default:
		quit("So far only ts and netcdf files are supported for reading in ice states\n");
	}

}

/* Routine to call back ice timeseries value (loaded from a ts file)
 * ice_or_depth = 1 returns ice class value, 0 returns depth of ice
 */
double get_Loaded_Ice(MSEBoxModel *bm, FILE *llogfp, int box_id, int iceclass_id, int ice_or_depth) {
	double tsEvaled = 0.0;
	IceTimeSeries *this_tsIce = &tsIce[iceclass_id];
	int ts_id = tsiceid[box_id];

	if (tsIcetype == interpolate_id) {
		if(ice_or_depth)
			tsEvaled = tsEval(&this_tsIce->ts, ts_id, bm->t);
		else
			tsEvaled = tsEval(&this_tsIce->depthts, ts_id, bm->t);
	} else {
		if(ice_or_depth)
			tsEvaled = tsEvalEx(&this_tsIce->ts, ts_id, bm->t);
		else
			tsEvaled = tsEvalEx(&this_tsIce->depthts, ts_id, bm->t);
	}

	return tsEvaled;
}

/*************************************************************************************
 * \brief Load the ice time series data - for ts file type.
 *
 */
int Load_Ice_Timeseries(MSEBoxModel *bm) {
	int b, i;
	char key[BMSLEN];

	/* Load catch time series */
	if (bm->ice_on) {
		switch(bm->kind_ice_model) {
		case ts_file:
			tsIceWarned = 0;
			Ice_Read_Time_Series(bm, bm->forceIfname, "Ice", bm->t_units, &tsIce, &ntsIce, &tsIcetype);

			if (ntsIce) {
				IceTimeSeries *this_ts = &tsIce[0];

				/* Assign box ids */
				for (b = 0; b < bm->nbox; b++) {
					tsiceid[b] = -1;
					for (i = 0; i < this_ts->ts.nv; i++) {
						if (strcmp(key, this_ts->ts.varname[b]) == 0) {
							tsiceid[b] = i;
							break;
						}
					}
				}

			} else
				quit("Expecting ice time series but didn't find any\n");
			break;
		case cdf_file:
			ice_cdf_init(bm);
			break;
		default:
			quit("So far only ts and netcdf files are supported for reading in ice states\n");

		}

		// Create data stores
		bm->ts_ice_data_array = Util_Alloc_Init_2D_Double(bm->K_num_ice_classes, bm->nbox, 0.0);
		bm->ts_ice_dz_data = Util_Alloc_Init_1D_Double(bm->nbox, 0.0);
	} else {
		tsIce = NULL;
	}

	get_ice_property(bm, &(bm->iceinput));

	return 0;
}

/*	Input variables and parameters:
*	@input name:		Name of ascii file containing list of fisheries time series
*	@input tsname:		Name of times series (catch or effort) being read
*	@input t_units:	Time units to be used for time series
*	@input icets:		Returned pointer to fisheries time series list
*  @input nts:			Returned number of fisheries time series
*/
void Ice_Read_Time_Series(MSEBoxModel *bm, char *name, char *tsname, char *t_units, IceTimeSeries **ts, int *nts, int *typets) {
	FILE *fp;
	char keyn[BMSLEN];
	int nnts = 0;
	int typts = 0;
	IceTimeSeries *fts = NULL;
	int i = 0;

	/* Open the file */
	if ((fp = fopen(name, "r")) == NULL)
		quit("read%sTimeSeries: Can't open %s\n", tsname, name);

	/* Print a heading on stderr if verbose enough */
	if (verbose > 1)
		fprintf(stderr, "Reading %s Time Series\n", tsname);

	/* Get the number of time series */
	snprintf(keyn, sizeof(keyn), "n%sts", tsname);
	readkeyprm_i(fp, keyn, &nnts);

	/* Get the type of time series */
	snprintf(keyn, sizeof(keyn), "type%sts", tsname);
	readkeyprm_i(fp, keyn, &typts);

	/* Allocate memory for list of time series */
	if ((fts = (IceTimeSeries *) malloc((size_t)nnts * sizeof(IceTimeSeries))) == NULL)
		quit("read%sTimeSeries: Can't allocate memory for %s time series list\n", tsname);

	/* Read each fisheries time series input */
	for (i = 0; i < nnts; i++) {
		char key[MAXLINELEN];
		char buf[MAXLINELEN];

		/* Ice Data */
		snprintf(key, sizeof(key), "%sts%d.data", tsname, i);

		fprintf(stderr, "Loading %s for class %d\n", key, fts[i].iceclass);

		readkeyprm_s(fp, key, buf);
		tsRead(bm->inputFolder, buf, &fts[i].ts);
		if (verbose > 1)
			tsPrintInfo(&fts[i].ts, stderr);

		fprintf(stderr, "Loaded %s\n", fts[i].ts.df->name);

		/* Check data time units */
		if (strcmp(fts[i].ts.t_units, t_units) != 0) {
			if (verbose > 1) {
				fprintf(stderr, "Converting %s time series time units\n", tsname);
				fprintf(stderr, "    Was %s\n", fts[i].ts.t_units);
				fprintf(stderr, "    Now %s\n", t_units);
			}
			tsNewTimeUnits(&fts[i].ts, t_units);
		}

		/* Ice Depth Data */
		snprintf(key, sizeof(key), "%sts%d.depth_data", tsname, i);

		readkeyprm_s(fp, key, buf);
		tsRead(bm->inputFolder, buf, &fts[i].depthts);
		if (verbose > 1)
			tsPrintInfo(&fts[i].depthts, stderr);

		fprintf(stderr, "Loaded %s\n", fts[i].depthts.df->name);

		/* Check data time units for ice dz */
		if (strcmp(fts[i].depthts.t_units, t_units) != 0) {
			if (verbose > 1) {
				fprintf(stderr, "Converting %s time series time units\n", tsname);
				fprintf(stderr, "    Was %s\n", fts[i].depthts.t_units);
				fprintf(stderr, "    Now %s\n", t_units);
			}
			tsNewTimeUnits(&fts[i].depthts, t_units);
		}

	}

	/* Close the file */
	fclose(fp);

	/* Store pointer to list of point source/sinks */
	*ts = fts;
	*nts = nnts;
	*typets = typts;

	return;
}

/**
 *	Free Ice TS
 */
void Ice_Free_Time_Series(IceTimeSeries *icets, int nnts) {
	int i = 0;

	for (i = 0; i < nnts; i++) {
		icets[i].iceclass = -1;
		tsFree(&icets[i].ts);
		tsFree(&icets[i].depthts);
	}
	free(icets);
	return;
}

/*****************************************************************************************
 * Load ice time series file from netcdf file type
 */

/* Routine to initialisation temperature and salinity forcing files */
void ice_cdf_init(MSEBoxModel *bm) {
	FILE *fp;
	int i = 0;

	/* Open the file containing the list of hydro input files */
	if ((fp = Open_Input_File(bm->inputFolder, bm->forceIfname, "r")) == NULL)
		quit("ice_cdf_init: Can't open %s\n", bm->forceIfname);

	/* Set error routine to quit if parameters not found */
	set_keyprm_errfn(quit);

	/* Read the number of files */
	readkeyprm_i(fp, "nInceFiles", &bm->iceinput.nFiles);
	ntsIce = bm->iceinput.nFiles;
	if (ntsIce < 1)
		quit("ice_cdf_init: Ice on and necdf chosen as method of input so there must be at least 1 ice input file (%d in %s)\n", ntsIce, bm->forceIfname);

	/* Allocate memory for file names if necessary */
	bm->iceinput.fname = c_alloc2d(BMSLEN, bm->iceinput.nFiles);
	bm->iceinput.variableName = c_alloc2d(BMSLEN, bm->K_num_ice_classes);
	bm->iceinput.prop_vid = i_alloc1d(bm->K_num_ice_classes);
	bm->iceinput.depthName = c_alloc1d(BMSLEN);
   // bm->iceinput.use_resets  = i_alloc1d(bm->iceinput.nFiles);
    
	snprintf(bm->iceinput.depthName, BMSLEN, "total_depth");


	for(i=0; i<bm->K_num_ice_classes; i++){
		snprintf(bm->iceinput.variableName[i], BMSLEN, "Ice_Class%d", i + 1);
	}

	/* Read the list of files */
	for (i = 0; i < ntsIce; i++) {
		char key[BMSLEN];
		snprintf(key, sizeof(key), "Ice%d.name", i);
		//printf("key = %s\n", key);
		readkeyprm_s(fp, key, bm->iceinput.fname[i]);
        
        //bm->iceinput.use_resets[i] = 0;
	}

	/* Close the parameter file */
	fclose(fp);

	warn("ice_cdf_init: Starting from beginning of ice inputs\n"
		"This will probably be incorrect for a 'hot' start of the model\n\n");

	bm->iceinput.curFile = 0;

	open_iceprop(bm, &(bm->iceinput));

	/* Read first set of data */
	printf("init - filename = %s, fid = %d\n", bm->iceinput.fname[bm->iceinput.curFile], bm->iceinput.fid);
	get_ice_property(bm, &(bm->iceinput));
}

/**
 * \brief This will close the ice netcdf files and free up any memory allocated.
 *
 */
void freeCDFIceFiles(MSEBoxModel *bm) {

	c_free2d(bm->iceinput.fname);
	c_free2d(bm->iceinput.variableName);
	i_free1d(bm->iceinput.prop_vid);
	c_free1d(bm->iceinput.depthName);

	close_iceprop(bm, &(bm->iceinput));

}

/**
 * \brief Get ice values
 */
void CDF_iceBM(MSEBoxModel *bm) {
	double tleft = bm->dt;
	double dt;

	if (verbose)
		fprintf(stderr, "Entering iceBM\n");

	/* Loop while more time remains in this transport time step */
	while (tleft > 0) {
		/* Get more ice data if necessary */
		if (bm->iceinput.tleft <= 0)
			get_ice_property(bm, &(bm->iceinput));

		/* Calculate time step allowed */
		dt = min(tleft, bm->iceinput.tleft);

		bm->iceinput.tleft -= dt;
		tleft -= dt;
	}

	/* Get more ice data if necessary, so that ice time
	 * is updated appropriately
	 */
	if (bm->iceinput.tleft <= 0)
		get_ice_property(bm, &(bm->iceinput));
}

/* Routine to get the property data for this time step.
 * This may involve reading the netCDF input file if the
 * data is not already in the memory buffers
 */
void get_ice_property(MSEBoxModel *bm, IcePropertyData *propInput){

	long offset = 0;
	long tstepnum = propInput->nextrec;
	int i, j, k;
	doubleINPUT *value;
	doubleINPUT **dzarray;
	doubleINPUT **array;

	ncopts = NC_VERBOSE | NC_FATAL;
	/* Have we finished this temperature file? */
	if (tstepnum >= propInput->nstep) {
		/* Yes - close it and open the next one */
		close_iceprop(bm, propInput);

		propInput->curFile = (propInput->curFile + 1) % propInput->nFiles;
		open_iceprop(bm, propInput);
		tstepnum = propInput->nextrec;
	}

	array = alloc2dInput(bm->nbox, min(propInput->nbuf, propInput->nstep-tstepnum));
	dzarray = alloc2dInput(bm->nbox, min(propInput->nbuf, propInput->nstep-tstepnum));
	value = alloc1dInput(propInput->nbuf);

	/* Is the requested data already in the memory buffers? */
	if (tstepnum < propInput->bufstart || tstepnum > propInput->bufend) {

		/* Data must be read from file, so we might as well
		 * read propInput->nstep records, starting at the requested
		 * record number (ensuring, of course, that we don't try
		 * to read past the last record in the file).
		 */
		long start[2];
		long count[2];

		start[0] = tstepnum;
		start[1] = 0;
		count[0] = min(propInput->nbuf, propInput->nstep-tstepnum);
		count[1] = bm->nbox;

		ncvarget(propInput->fid, propInput->t_vid, start, count, value);
		for (i = 0; i < propInput->nbuf; i++)
			propInput->tbuf[i] = (double)value[i];

		for(k=0; k<bm->K_num_ice_classes; k++){
			ncvarget(propInput->fid, propInput->prop_vid[k], start, count, array[0]);
			for (i = 0; i < count[0]; i++)
				for (j = 0; j < count[1]; j++)
					propInput->valuebuf[i][j][k] = (double)array[i][j];
		}

		ncvarget(propInput->fid, propInput->dz_vid, start, count, dzarray[0]);
		for (i = 0; i < count[0]; i++)
			for (j = 0; j < count[1]; j++)
				propInput->dzvaluebuf[i][j] = (double)dzarray[i][j];


		propInput->bufstart = tstepnum;
		propInput->bufend = tstepnum + count[0] - 1;
	}

	/* Data must now be in buffers, so adjust pointers */
	offset = tstepnum - propInput->bufstart;
	propInput->dataBuffer = propInput->valuebuf[offset];
	propInput->dzBuffer = propInput->dzvaluebuf[offset];

	/* Store hydrodynamic time value */
	propInput->t = propInput->tbuf[offset] + propInput->dt / 2.0;

	/* Reset time left for this hydrodynamic step */
	propInput->tleft = propInput->dt;

	/* Increment next record number */
	propInput->nextrec++;

	free2dInput(array);
	free2dInput(dzarray);
	free1dInput(value);
}

/* Routine to open a hydrodynamic input file and check
 * that it has appropriate structure.
 */

/* Maximum amount of memory to allocate for exchange values */
#define MAXBUFMEM (2L*1024L*1024L)

void open_iceprop(MSEBoxModel *bm, IcePropertyData *propInput) {
	int i;
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

	/* Set netCDF library error handling */
	ncopts = NC_VERBOSE;

	/* Open the ice file */
	if ((propInput->fid =  Util_ncopen("", propInput->fname[propInput->curFile], NC_NOWRITE)) < 0)
		quit("open_iceprop: Can't open ice input data file %s\n", propInput->fname[propInput->curFile]);

	printf("filename = %s, fid = %d\n", propInput->fname[propInput->curFile], propInput->fid);

	/* Inquire about this file */
	ncopts = NC_VERBOSE;// | NC_FATAL;
	ncinquire(propInput->fid, &ndims, &nvars, &natts, &recdim);
	if (ndims < 2)
		quit("open_iceprop: not enough dimensions in %s\n", propInput->fname[propInput->curFile]);
	if (nvars < bm->K_num_ice_classes+1)
		quit("open_iceprop: not enough variables in %s\n", propInput->fname[propInput->curFile]);

	/* Check dimensions are as expected */
	if ((propInput->t_did = ncdimid(propInput->fid, "t")) == -1)
		quit("open_iceprop: no t dimension in %s\n", propInput->fname[propInput->curFile]);
	if (propInput->t_did != recdim)
		quit("open_iceprop: t dimension not unlimited in %s\n", propInput->fname[propInput->curFile]);
	if ((propInput->b_did = ncdimid(propInput->fid, "b")) == -1)
		quit("open_iceprop: no b dimension in %s\n", propInput->fname[propInput->curFile]);

	/* Get dimension sizes and check against geometry */
	ncdiminq(propInput->fid, propInput->b_did, NULL, &n);
	if (n != bm->nbox)
		quit("open_iceprop: Number of boxes (%ld) in %s doesn't match geometry (%d)\n", n, propInput->fname[propInput->curFile], bm->nbox);

	/* Check that time units and steps match this model */
	propInput->t_vid = ncvarid(propInput->fid, "t");
	if (propInput->t_vid < 0)
		quit("open_iceprop: no t variable in %s\n", propInput->fname[propInput->curFile]);

	memset(propInput->t_units, 0, BMSLEN);
	ncattget(propInput->fid, propInput->t_vid, "units", propInput->t_units);
	sscanf(propInput->t_units, "%s", hdu);
	sscanf(bm->t_units, "%s", bmu);
	if (strcmp(hdu, bmu) != 0)
		quit("open_iceprop: Time units (%s) don't match model time units (%s)\n", hdu, bmu);

	ncattinq(propInput->fid, propInput->t_vid, "dt", &daty, &len);
	if (nctypelen(daty) != sizeof(doubleINPUT))
		quit("open_iceprop: dt attribute wrong type\n");

	ncattget(propInput->fid, propInput->t_vid, "dt", &value);
	propInput->dt = (double) value;
	if (propInput->dt <= 0.0)
		quit("open_iceprop: dt attribute must have positive value\n");

	/* Find out how many time steps are in the file */
	ncdiminq(propInput->fid, propInput->t_did, NULL, &propInput->nstep);

	/* Get other variable ids */
	propInput->dz_vid = ncvarid(propInput->fid, propInput->depthName);
	if (propInput->dz_vid < 0)
		quit("open_iceprop: no %s variable in %s\n", propInput->depthName, propInput->fname[propInput->curFile]);

	for(i=0; i<bm->K_num_ice_classes; i++){
		propInput->prop_vid[i] = ncvarid(propInput->fid, propInput->variableName[i]);
		if (propInput->prop_vid[i] < 0)
			quit("open_iceprop: no %s variable in %s\n", propInput->variableName[i], propInput->fname[propInput->curFile]);

		/* Check variable types and dimensions */
		ncvarinq(propInput->fid, propInput->prop_vid[i], NULL, &daty, &ndims, dims, &natts);
		if (nctypelen(daty) != sizeof(doubleINPUT))
			quit("open_iceprop: Type of ice variable doesn't match model\n");
		if (ndims != 2 || dims[0] != propInput->t_did || dims[1] != propInput->b_did)
			quit("open_iceprop: %s variable has incorrect dimensions\n", propInput->variableName[i]);
	}

	/* Calculate a reasonable number of time steps
	 * to store in buffers in memory
	 */
	bytes = (long int)sizeof(double) * bm->nbox * bm->icenz;
	propInput->nbuf = min(MAXBUFMEM/bytes, propInput->nstep);
	if (propInput->nbuf < 1)
		propInput->nbuf = 1;

	/* Allocate memory for data buffers */
	propInput->tbuf = alloc1d(propInput->nbuf);
	propInput->valuebuf = (double ***) alloc3d(bm->K_num_ice_classes, bm->nbox, propInput->nbuf);
	propInput->dzvaluebuf = (double **) alloc2d(bm->nbox, propInput->nbuf);

	/* Signify that buffers are empty */
	propInput->bufstart = -1;
	propInput->bufend = -1;
	propInput->nextrec = 0;

	/* Reset netCDF error handling */
	ncopts = NC_VERBOSE | NC_FATAL;
}


/* Routine to close an already open ice input file */
void close_iceprop(MSEBoxModel *bm, IcePropertyData *propInput){

	/* Close ice file */
	if (propInput->fid >= 0)
		ncclose(propInput->fid);
	propInput->fid = -1;

	/* Free buffers */
	free1d(propInput->tbuf);
	free3d((double ***) propInput->valuebuf);
	free2d((double **)propInput->dzvaluebuf);

	propInput->tbuf = NULL;
	propInput->valuebuf = NULL;
}

/*****************************************************************************************
 * Enter ice values into the IceModel per box
 */

void Enter_IceBM(MSEBoxModel *bm, double **ice_data_array, double *dzData) {
	int b = 0;
	int i = 0;
	int z = 0;
	int k = 0;
	int slush_allowed = 0;
	double ice_depth = 0.0;
	double depth_prop = 1.0;
	double tot_depth = 0.0;
	double last_depth = 0.0;
	double icedepth = 0.0;
	double max_possible_ice = (double)(bm->maxicedz * bm->icenz);

	/* Loop over each box to apply ice values */
	for(b=0; b<bm->nbox; b++) {
		IceModel *ice = &bm->boxes[b].ice;

		/* Loop through ice levels and distribute them accordingly */
		depth_prop = 1.0;
		ice_depth = dzData[b];
		tot_depth = 0;
		/* Check if freezing or melting - assume no change at the start */
		ice->is_freezing = 0;
		ice->ice_growth_rate = (ice_depth - ice->last_depth) / bm->dt;
		if(ice_depth > ice->last_depth){
			ice->is_freezing = 1;	// Freezing
		} else if(ice_depth < ice->last_depth){
			ice->is_freezing = -1;  // Melting
		}
		ice->last_depth = ice_depth;

		/* Sort out ice layers */
		if(ice_depth > 0.0){
			z = 0;
			slush_allowed = 1;

			while(ice_depth > tot_depth && z < bm->icenz){
				last_depth = tot_depth;

				// Sort out the ice entry
				for(i=0; i<bm->K_num_ice_classes; i++){
					ice->ice_classes[z][i] = ice_data_array[b][i] * depth_prop;
				}

				// Now update
				tot_depth += ice->dz[z];

				z++;
			}
			// Make sure don't exceed max possible depth
			if(tot_depth > (max_possible_ice)){
				tot_depth = ice_depth - 1.0;
				z = bm->icenz - 1;
				slush_allowed = 0;
			}
			if(z >= bm->icenz) {
				// Nothing else to do as all ice full
			} else {
				// Final layer - all slush
				// TODO: Handle this more effectively
				if( slush_allowed ) {
					for(i=0; i<bm->K_num_ice_classes; i++){
						if ( i != bm->slush_id ){
							ice->ice_classes[z-1][i] = 0.0;
						} else {
							ice->ice_classes[z-1][i] = ice_data_array[b][i] * depth_prop;
						}
					}
				} else {
				// Final layer is same as other layers all way through
					for(i=0; i<bm->K_num_ice_classes; i++){
						ice->ice_classes[z-1][i] = ice_data_array[b][i] * depth_prop;
					}
				}

			}
			ice->currentnz = z-1;
			ice->currentnz = z;

		} else {
			// No ice so set all classes to zero
			for(z=0; z<bm->icenz; z++){
				for(i=0; i<bm->K_num_ice_classes; i++){
					ice->ice_classes[z][i] = 0.0;
				}
			}
			ice->currentnz = 0;
		}

		/* Figure out the mapping to water column layers for movement purposes */
		last_depth = 0.0;
		tot_depth = 0.0;
		bm->boxes[b].has_ice = 0;

		// Init the property
		for(z=0; z<bm->boxes[b].nz; z++){
			for(k=0; k<bm->K_num_ice_classes; k++){
				bm->boxes[b].ice_prop[z][k] = 0.0;
			}
		}

		// Assign the actual value
		for(z=bm->boxes[b].nz - 1; z>= 0; z--){
			tot_depth += bm->boxes[b].dz[z];
			icedepth = 0.0;
			for(i=0; i<bm->icenz; i++){
				icedepth += ice->dz[i];

				ice->wc_layer_match[i] = bm->boxes[b].nz - 1;  // As a default say its at the surface
				if((last_depth < icedepth) && (icedepth <= tot_depth)){
					bm->boxes[b].has_ice = 1;
					for(k=0; k<bm->K_num_ice_classes; k++){
						bm->boxes[b].ice_prop[z][k] += ice->ice_classes[i][k];
						ice->wc_layer_match[i] = z;
					}
				} else if(icedepth > tot_depth) {
					break;
				}
			}
			last_depth = tot_depth;
		}

		// Find total ice cover
		bm->boxes[b].tot_ice_prop = 0.0;
		z = 0;
		for(k=0; k<bm->K_num_ice_classes; k++){
			bm->boxes[b].tot_ice_prop += ice->ice_classes[z][k];
		}

	}
}

 /*****************************************************************************************
  * Free ice time series
  */

void freeIceFiles(MSEBoxModel *bm){
	int b;

	switch(bm->kind_ice_model) {
	case ts_file:
		Ice_Free_Time_Series(tsIce, ntsIce);
		break;
	case cdf_file:
		freeCDFIceFiles(bm);
		break;
	default:
		quit("So far only ts and netcdf files are supported for reading in ice states\n");
	}

	if(bm->ts_ice_data_array) free2d(bm->ts_ice_data_array);
	if(bm->ts_ice_dz_data) free1d(bm->ts_ice_dz_data);

	for(b=0; b<bm->nbox; b++){
		if(bm->boxes[b].ice_prop)
			free2d(bm->boxes[b].ice_prop);
		if(bm->boxes[b].ice.wc_layer_match)
			i_free1d(bm->boxes[b].ice.wc_layer_match);
		if(bm->boxes[b].ice.ice_classes)
			free2d(bm->boxes[b].ice.ice_classes);

	}

	return;
}
