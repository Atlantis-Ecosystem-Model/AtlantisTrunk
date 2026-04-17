/*******************************************************************//**
 \file
 \brief Routines which deal with tracer data files. These files are in netCDF format
 and should be read, written and created using the routines below.

 File:           attracerIO.c

 Created:        Thu Nov 17 15:41:37 EST 1994

 Author:         Stephen Walker
 CSIRO Division of Oceanography

 Purpose:        Routines which deal with tracer data
 files. These files are in netCDF format
 and should be read, written and created
 using the routines below.

 Arguments:      See each routine below

 Returns:        See each routine below

 Revisions:      8/8/2004 EA Fulton
 Ported across the code from the southeast (sephys) model

 17/11/2004 EA Fulton
 Converted original routine definitions from
 void
 routine_name(blah,blahblah)
 int blah;
 double blahblah;
 to

 void routine_name(int blah, double blahblah)

 28/3/2005 Added bm->tinfo[n].flagid to indicate whether a
 group is active and so can be advected/diffused (so that
 only active groups are advected and avoid accumulation of
 1e-8 rounding errors in backwaters).

 8/3/2005 EA Fulton
 Added flagid check added so only output active groups.

 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sjwlib.h>
#include <netcdf.h>
#include <atlantisboxmodel.h>
#include <atUtilLib.h>

/*************//**
 Routine to create general data summary file
 ****************/
int createBMDataFile(char *destFolder, char *name, MSEBoxModel *bm, int dtype) {
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
	}

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
	writeBMphysInfo(fid, bm, dtype);
	writeBMTracerInfo(fid, bm, dtype);
	writeBMEpiInfo(fid, bm, dtype);
	writeBMDiagInfo(fid, bm, dtype);
	if (bm->flag_fisheries_on) {
		writeBMFisheriesInfo(fid, bm, dtype);
	}
	if (bm->ice_on){
		writeBMIceInfo(fid, bm, dtype);
	}
	if (bm->terrestrial_on){
		writeBMLandInfo(fid, bm, dtype);
	}
    
	/* Exit from netCDF define mode */
	ncendef(fid);
	ncsync(fid);

	return (fid);
}

/*******************************************************************//**
 Routine to read the tracer information from a netCDF file.
 This routine assumes that the file has the following
 format (roughly written in CDL).

 dimensions:

 t = unlimited;    // Time dimension
 b = N;            // Number of boxes
 z = N;	    // Number of layers (water column + sediment)

 // z values correspont to vertical positions as follows:
 // z = 0             Bottom water layer
 // z = wcnz-1        Surface water column layer
 // z = wcnz          Uppermost sediment layer
 // z = wcnz+sednz-1  Deepest sediment layer

 variables:

 // A typical dissolved tracer entry
 double salt(t,b,z);            // Salinity values
 salt:bmtype = "tracer";      // This is a tracer variable
 salt:units = "PSU";          // Units for salt
 salt:long_name = "Salinity"; // Long name of salt
 salt:dtype = 0;              // Flag => general data type (1 if fisheries data)
 salt:inwc = 1;               // Flag => exists in water column
 salt:insed = 1;              // Flag => exists in sediment
 salt:decay = 0.0;            // Decay constant
 salt:dissol = 1;             // Flag => dissolved
 salt:partic = 0;             // Flag => particulate

 // A typical particulate tracer entry
 double sand(t,b,z);            // Sand concentration values
 sand:bmtype = "tracer";      // This is a tracer variable
 sand:units = "kg m-3";       // Units for sand
 sand:long_name = "Sand";     // Long name of sand
 salt:dtype = 0;              // Flag => general data type (1 if fisheries data)
 sand:inwc = 1;               // Flag => exists in water column
 sand:insed = 1;              // Flag => exists in sediment
 sand:decay = 0.0;            // Decay constant
 sand:dissol = 0;             // Flag => dissolved
 sand:partic = 1;             // Flag => particulate
 sand:svel = -0.02;           // Settling velocity
 sand:psize = 0.0002;         // Particle size
 sand:b_dens = 2000.0;        // Bulk grain density
 sand:i_conc = 500.0;         // Initial deposit concentration
 sand:_FillValue = 35.0;      // Default value

 // The netCDF file has global attributes as follows
 :title = "Description of this data set";
 :geometry = "Name of geometry file";
 :parameters = "command line used to create this run";
 :wcnz = N;     // Number of water column layers
 :sednz = N;    // Number of sediment layers

 The routine finds out how many tracer variables there
 are, allocates memory and reads the tracer information.

 *********************************************************************/
void readBMTracerInfo(int fid, char *fileName, MSEBoxModel *bm) {
	int ndims = 0; /* Number of dimensions */
	int nvars = 0; /* Number of variables */
	int ngatts = 0; /* Number of global attributes */
	int recdim = 0; /* ID of unlimited dimension */
	int t_id = 0; /* Time dimension id */
	int b_id = 0; /* Box dimension id */
	int z_id = 0; /* Layer dimension id */
	//int dims[MAX_NC_DIMS];
	int *idlist = 0;
	long n = 0;
	int i = 0;
	int b = 0;
	int nice = 0;
	doubleINPUT value;
	nc_type  t_type;   /* global attribute type */
	int      t_len;    /* global attribute length */

	if (verbose > 1)
        printf( "Entering readBMTracerInfo\n");

	/* Set netCDF library error handling */
	ncopts = NC_VERBOSE | NC_FATAL;

	/* Inquire about this file */
	ncinquire(fid, &ndims, &nvars, &ngatts, &recdim);
	if (ndims < 3)
		quit("readBMTracerInfo: not enough dimensions\n");
	if (nvars < 2)
		quit("readBMTracerInfo: not enough variables\n");
	if (ngatts < 2)
		quit("readBMTracerInfo: not enough attributes\n");

	/* Check dimensions are as expected */
	if ((t_id = ncdimid(fid, "t")) == -1)
		quit("readBMTracerInfo: no t dimension");
	if (t_id != recdim)
		quit("readBMTracerInfo: t dimension not unlimited\n");
	if ((b_id = ncdimid(fid, "b")) == -1)
		quit("readBMTracerInfo: no b dimension\n");
	if ((z_id = ncdimid(fid, "z")) == -1)
		quit("readBMTracerInfo: no z dimension\n");

	/* Get dimension sizes and check against geometry */
	ncdiminq(fid, b_id, NULL, &n);
	if (n != bm->nbox)
		quit("readBMTracerInfo: Number of boxes (%ld) doesn't match geometry (%d)\n", n, bm->nbox);

	ncdiminq(fid, z_id, NULL, &n);
	if (n != bm->wcnz + bm->sednz)
		quit("readBMTracerInfo: Total layers (%ld) doesn't match geometry (%d+%d)\n", n, bm->wcnz, bm->sednz);

	/* Check numbers of layers from attributes against geometry */
	ncattget(fid, NC_GLOBAL, "wcnz", &n);
	if (n != bm->wcnz)
		quit("readBMTracerInfo: Number of layers in water doesn't match geometry\n");

	ncattget(fid, NC_GLOBAL, "sednz", &n);
	if (n != bm->sednz)
		quit("readBMTracerInfo: Number of layers in sediment doesn't match geometry\n");

	ncopts = 0;	// turn off verbose as well otherwise we get a warning about attribute not found if icenz not found - quite misleading as its fine if its not found it
				// just means ice is not active in this model.

	/* Check numbers of layers from attributes against geometry */
    ncattget(fid, NC_GLOBAL, "icenz", &n);
    if(n!= 0){
	//if(ncattinq(fid, NC_GLOBAL, "icenz", &t_type, &t_len) != -1){  //icenz is not a variable its a global dimension
		bm->ice_on = TRUE;
		bm->num_active_habitats = ICE_BASED + 1;
	} else {
		bm->ice_on = FALSE;
        bm->num_active_habitats = EPIFAUNA + 1;  // TODO: Should this be LAND_BASED + 1 if terrestrial on? Likely not as treat land animals as epifauna
        bm->icenz = 0;
	}


	ncopts = NC_VERBOSE | NC_FATAL;

	/* Clear water id in model */
	bm->waterid = -1;

	/* Find and count tracer variables */
//	dims[0] = t_id;
//	dims[1] = b_id;
//	dims[2] = z_id;
	idlist = i_alloc1d(nvars);

	/* Get number of tracer variables */
	//n = ncvarfind(fid, 3, dims, "bLmtype", "tracer", idlist);
	n = ncvarfind(fid, 3, NULL, "bmtype", "tracer", idlist);

	if (n < 1)
		quit("readBMTracerInfo: No tracers in file\n");

	if (verbose > 1)
        printf( "readBMTracerInfo: %ld tracers in file\n", n);

	/* Allocate space for tracer info */
	if ((bm->tinfo = (TracerInfo *) malloc((size_t)n * sizeof(TracerInfo))) == NULL)
		quit("readBMTracerInfo: Can't allocate memory for info\n");

	/* Clear tracer info */
	memset(bm->tinfo, 0, (size_t)n * sizeof(TracerInfo));

	/* Loop over the tracer variables to read info */
	for (i = 0; i < n; i++) {
		nc_type dt;
		int nd;
		int d[3];
		int na;
		int len;

        /* Tracer number (must be equal to i) */
		bm->tinfo[i].n = i;
		bm->tinfo[i].isUsed = FALSE;	/* Assume variable is not used until its matched to a group */
		/* Tracer name */
		ncvarinq(fid, idlist[i], bm->tinfo[i].name, &dt, &nd, d, &na);

		//if( verbose > 1 )
		//	fprintf(stderr,"readBMTracerInfo: %s info reading\n",bm->tinfo[i].name);

		/* Find water id */
		if (strcmp(bm->tinfo[i].name, WATER) == 0)
			bm->waterid = i;
		/* Tracer attributes */
        
		/* Units */
		if (verbose > 2)
			fprintf(stderr, "readBMTracerInfo: reading units\n");
		ncattget(fid, idlist[i], "units", bm->tinfo[i].units);

		if (verbose > 2)
			fprintf(stderr, "readBMTracerInfo: reading long_name for %s\n", bm->tinfo[i].name);
		ncattget(fid, idlist[i], "long_name", bm->tinfo[i].long_name);

		/* Dtype */
		if (verbose > 2)
			fprintf(stderr, "readBMTracerInfo: reading dtype\n");
		ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "dtype", &dt, &len);

		if (nctypelen(dt) != sizeof(bm->tinfo[i].dtype))
			quit("readBMTracerInfo: dtype attribute wrong type (%s)\n", bm->tinfo[i].name);
		ncattget(fid, idlist[i], "dtype", &bm->tinfo[i].dtype);

		/* SumType */
		if (verbose > 2)
			fprintf(stderr, "readBMTracerInfo: reading sumtype\n");
		ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "sumtype", &dt, &len);

		if (nctypelen(dt) != sizeof(bm->tinfo[i].sumtype))
			quit("readBMTracerInfo: sumtype attribute wrong type (%s)\n", bm->tinfo[i].name);
		ncattget(fid, idlist[i], "sumtype", &bm->tinfo[i].sumtype);

		/* in water column*/
		if (verbose > 2)
			fprintf(stderr, "readBMTracerInfo: reading inwc\n");
		ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "inwc", &dt, &len);

		if (nctypelen(dt) != sizeof(bm->tinfo[i].inwc))
			quit("readBMTracerInfo: inwc attribute wrong type (%s)\n", bm->tinfo[i].name);
		ncattget(fid, idlist[i], "inwc", &bm->tinfo[i].inwc);

        /* in sediment */
		if (verbose > 2)
			fprintf(stderr, "readBMTracerInfo: reading insed\n");
		ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "insed", &dt, &len);

        if (nctypelen(dt) != sizeof(bm->tinfo[i].insed))
			quit("readBMTracerInfo: insed attribute wrong type (%s)\n", bm->tinfo[i].name);
		ncattget(fid, idlist[i], "insed", &bm->tinfo[i].insed);

        /* set nopts to 0 so we don't crash if attribute not found */
		ncopts = 0;
		/* if ice on and a species that can live in ice, water column and predator */
		if(bm->ice_on){
			if (verbose > 2)
				fprintf(stderr, "readBMTracerInfo: reading inice\n");
			ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "inice", &dt, &len);

			if (nctypelen(dt) != sizeof(bm->tinfo[i].inice))
				quit("readBMTracerInfo: inice attribute wrong type (%s)\n", bm->tinfo[i].name);
			ncattget(fid, idlist[i], "inice", &bm->tinfo[i].inice);

			nice++;
		} else {
			bm->tinfo[i].inice = 0;
		}
		/* Set the ops back to normal values so we do crash again if attributes not found */
		ncopts = NC_VERBOSE | NC_FATAL;

		/* First check the dimensions are what they should be - check we have ice data. */
		if(d[0] != t_id)
			quit("Tracer %s in file %s should have dimensions t, b, z\n", bm->tinfo[i].name, fileName);
		if(d[1] != b_id)
			quit("Tracer %s in file %s should have dimensions t, b, z\n", bm->tinfo[i].name, fileName);

        /* Decay rate */
		if (verbose > 2)
			fprintf(stderr, "readBMTracerInfo: reading decay\n");
		ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "decay", &dt, &len);

		if (nctypelen(dt) != sizeof(doubleINPUT))
			quit("readBMTracerInfo: decay attribute wrong type (%s)\n", bm->tinfo[i].name);
		ncattget(fid, idlist[i], "decay", &value);
		bm->tinfo[i].decay = (double) value;

        /* dissolved flag */
		if (verbose > 2)
			fprintf(stderr, "readBMTracerInfo: reading dissol\n");
		ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "dissol", &dt, &len);

		if (nctypelen(dt) != sizeof(bm->tinfo[i].dissol))
			quit("readBMTracerInfo: dissol attribute wrong type (%s)\n", bm->tinfo[i].name);
		ncattget(fid, idlist[i], "dissol", &bm->tinfo[i].dissol);

		/* particulate flag */
		if (verbose > 2)
			fprintf(stderr, "readBMTracerInfo: reading partic\n");
		ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "partic", &dt, &len);

		if (nctypelen(dt) != sizeof(bm->tinfo[i].partic))
			quit("readBMTracerInfo: partic attribute wrong type (%s)\n", bm->tinfo[i].name);
		ncattget(fid, idlist[i], "partic", &bm->tinfo[i].partic);

        /* Get additional attributes for particulate tracers */
		if (bm->tinfo[i].partic != 0) {
			/* Active vs Passive rating */
			if (verbose > 2)
				fprintf(stderr, "readBMTracerInfo: reading passive\n");
			ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "passive", &dt, &len);

			if (nctypelen(dt) != sizeof(bm->tinfo[i].passive))
				quit("readBMTracerInfo: passive attribute wrong type (%s)\n", bm->tinfo[i].name);
			ncattget(fid, idlist[i], "passive", &bm->tinfo[i].passive);

			/* Settling velocity */
			if (verbose > 2)
				fprintf(stderr, "readBMTracerInfo: reading svel\n");
			ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "svel", &dt, &len);

			if (nctypelen(dt) != sizeof(doubleINPUT))
				quit("readBMTracerInfo: svel attribute wrong type (%s)\n", bm->tinfo[i].name);
			ncattget(fid, idlist[i], "svel", &value);
			bm->tinfo[i].svel = (double) value;

			/* Extra settling velocity - due to migration of nutrient limitation */
			if (verbose > 2)
				fprintf(stderr, "readBMTracerInfo: reading xvel\n");
			ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "xvel", &dt, &len);

			if (nctypelen(dt) != sizeof(doubleINPUT))
				quit("readBMTracerInfo: xvel attribute wrong type (%s)\n", bm->tinfo[i].name);
			ncattget(fid, idlist[i], "xvel", &value);
			bm->tinfo[i].xvel = (double) value;

			/* Particle size */
			if (verbose > 2)
				fprintf(stderr, "readBMTracerInfo: reading psize\n");
			ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "psize", &dt, &len);

			if (nctypelen(dt) != sizeof(doubleINPUT))
				quit("readBMTracerInfo: psize attribute wrong type (%s)\n", bm->tinfo[i].name);
			ncattget(fid, idlist[i], "psize", &value);
			bm->tinfo[i].psize = (double) value;

			/* Particle bulk density */
			if (verbose > 2)
				fprintf(stderr, "readBMTracerInfo: reading b_dens\n");
			ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "b_dens", &dt, &len);

			if (nctypelen(dt) != sizeof(doubleINPUT))
				quit("readBMTracerInfo: b_dens attribute wrong type (%s)\n", bm->tinfo[i].name);
			ncattget(fid, idlist[i], "b_dens", &value);
			bm->tinfo[i].b_dens = (double) value;

			/* Initial concentration for deposition */
			if (verbose > 2)
				fprintf(stderr, "readBMTracerInfo: reading i_conc\n");
			ncattinq(fid, ncvarid(fid, bm->tinfo[i].name), "i_conc", &dt, &len);

			if (nctypelen(dt) != sizeof(doubleINPUT))
				quit("readBMTracerInfo: i_conc attribute wrong type (%s)\n", bm->tinfo[i].name);
			ncattget(fid, idlist[i], "i_conc", &value);
			bm->tinfo[i].i_conc = (double) value;

		}

        /* Set some flags for rugosity and aragonite saturation so not advected */
        bm->tinfo[i].can_be_moved = 1;
        
        if(!strcmp(bm->tinfo[i].name,"Rugosity") || !strcmp(bm->tinfo[i].name,"AragoniteSaturation")) {
            bm->tinfo[i].passive = 0;
            bm->tinfo[i].can_be_moved = 0;
        }
        
        
		bm->tinfo[i].flagid = 1;

		/*	if( verbose > 1 )
		 fprintf(stderr,"readBMTracerInfo: %s info read\n",bm->tinfo[i].name);
		 fprintf(stderr,"with units: %s\n",bm->tinfo[i].units);
		 fprintf(stderr,"and long name: %s\n",bm->tinfo[i].long_name);
		 */
	}

	/* Store number of tracers */
	bm->ntracer = (int)n;
	for (b = 0; b < bm->nbox; b++) {
		bm->boxes[b].ntr = (int)n;
		bm->boxes[b].sm.ntr = (int)n;
		bm->boxes[b].ice.ntr = nice;
	}

	fprintf(stderr, "Read in %d tracers\n", bm->ntracer);

	/* Free tracer id array */
	i_free1d(idlist);
}

/**** Free any memory allocated during a readBMTracerInfo call.
 ****/
void freeBMTracerInfo(MSEBoxModel *bm) {
	free(bm->tinfo);
}

/*********//**
 Routine to write tracer info to general data file
 ***********/
void writeBMTracerInfo(int fid, MSEBoxModel *bm, int dtype) {
	int i = 0;
	nc_type dt;
	int dim[3];
	int icedim[3];
	int vid;
	doubleINPUT value;

	/* set netCDF data type */
	dt = NC_DOUBLE;

	/* Get dimension ids */
	dim[0] = ncdimid(fid, "t");
	dim[1] = ncdimid(fid, "b");
	dim[2] = ncdimid(fid, "z");

	if(bm->ice_on){
		icedim[0] = ncdimid(fid, "t");
		icedim[1] = ncdimid(fid, "b");
		icedim[2] = ncdimid(fid, "icenz");
	}

	/* Loop over the tracers */
	for (i = 0; i < bm->ntracer; i++) {
        
		/* Check to see which output stream being dealt with */
		if (dtype == bm->tinfo[i].dtype && bm->tinfo[i].flagid && bm->tinfo[i].isUsed) {

			/* Ice tracers are written out separately */
			if(bm->ice_on){
				/* If this tracer is ice only then ignore*/
				if(bm->tinfo[i].inice == TRUE && bm->tinfo[i].inwc == FALSE && bm->tinfo[i].insed == FALSE )
					continue;
			}

			/* Define the variable */
			if (verbose > 1)
				printf("bm->tinfo[%d].name = %s\n", i, bm->tinfo[i].name);

			if(bm->ice_on == TRUE){
				if(bm->tinfo[i].inice == TRUE){
					vid = ncvardef(fid, bm->tinfo[i].name, dt, 3, icedim);
				}else{
					vid = ncvardef(fid, bm->tinfo[i].name, dt, 3, dim);
				}
			} else{
				vid = ncvardef(fid, bm->tinfo[i].name, dt, 3, dim);  // This line causing the memory issues
			}
            
			/* Set the attributes */
			ncattput(fid, vid, "bmtype", NC_CHAR, (int) strlen("tracer") + 1, "tracer");
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

			if (verbose > 1)
				fprintf(stderr, "writeBMTracerInfo: %s info written\n", bm->tinfo[i].name);

		}
	}
}

/*******************************************************************//**
 Routine to read the tracer data from a netCDF file. This routine
 assumes that the tracer information in the MSEBoxModel is valid and
 corresponds with the netCDF file. Minimal checking for consistency
 is done here.

 TODO: Figure out how to just read in numbers at age and then allocate to
 genotype - ask Bec
 *********************************************************************/
void readBMTracerData(int fid, int dump, MSEBoxModel *bm) {
	int i, sp, found_partial_match = 0, found_partial_match_n = 0, this_sp;
    int found_sn, found_rn;
	doubleINPUT **val;
	long int start[3];
	long int count[3];
	long n = 0;
    char *pdest;
    char *pdestsn;
    char *pdestrn;
	char *pdestsp;
	char denstr[] = "_Nums";
    char snstr[] = "_StructN";
    char rnstr[] = "_ResN";
    char bstr[] = "_N";


	/* Set netCDF library error handling */
	ncopts = NC_VERBOSE | NC_FATAL;

	/* Check that requested dump exists in file */
	ncdiminq(fid, ncdimid(fid, "t"), NULL, &n);
	if (dump >= n)
		quit("readBMTracerData: dump %d not in file (%ld records)\n", dump, n);

	/* Allocate temporary storage for one tracer */
	val = (doubleINPUT **) alloc2dInput(bm->wcnz + bm->sednz, bm->nbox);

	/* Set indices for reading tracers */
	start[0] = dump;
	start[1] = 0;
	start[2] = 0;
	count[0] = 1;
	count[1] = bm->nbox;
	count[2] = bm->wcnz + bm->sednz;

	bm->supplied_stress = 0;

	/* Loop over each tracer */
	for (i = 0; i < bm->ntracer; i++) {
		int b;
		int k;


		if(bm->ice_on == TRUE){
			/* If this is ice only don't read anything. */
			if(bm->tinfo[i].inice == TRUE){
				continue;
			}
		}

		/* Read data */
		ncvarread(fid, bm->tinfo[i].name, sizeof(doubleINPUT), start, count, val[0]);
		checkNetCDFData2D("readBMTracerData", bm->tinfo[i].name, val,  bm->nbox, bm->wcnz);

		/* Check that we actually have some data */
		for (b = 0; b < bm->nbox; b++) {
			for (k = 0; k < bm->wcnz; k++) {
				if(NC_FILL_DOUBLE == val[b][k]){
					fprintf(stderr, "\n\nERROR: readBMTracerData - Undefined data for tracer %s in box %d, layer %d in your initial conditions file.\n\n",
							bm->tinfo[i].name, b, k);
					fprintf(stderr, "Please make sure all variables in your initial conditions file either have a fill value or have defined data values\n");
					quit("");
				}
			}
		}

		/* Check if need to scale initial vertebrate numbers or invertebrate biomasses */
		if (bm->flagscaleinit) {
			found_partial_match = 0;
			this_sp = bm->K_num_tot_sp;
			/* Identify scalar to use - if numbers */
			pdest = strstr(bm->tinfo[i].name, denstr);
            pdestsn = strstr(bm->tinfo[i].name, snstr);
            pdestrn = strstr(bm->tinfo[i].name, rnstr);
            
			if (pdest != NULL) {
				found_partial_match = 1;
				/* Had a match now figure out what species it was */
				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isVertebrate == TRUE) {
						pdestsp = strstr(bm->tinfo[i].name, FunctGroupArray[sp].name);
						if (pdestsp != NULL) {
							this_sp = sp;
							break;
						}
					}
				}
			}
            /* Identify scale to use - if biomass */
            if (this_sp == bm->K_num_tot_sp) {
                pdest = strstr(bm->tinfo[i].name, bstr);
                if (pdest != NULL) {
                    found_partial_match_n = 1;
                    /* Had a match now figure out what species it was */
                    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                        if ((FunctGroupArray[sp].isDetritus == FALSE) && (FunctGroupArray[sp].isEpiFauna == FALSE)) {
                            pdestsp = strstr(bm->tinfo[i].name, FunctGroupArray[sp].name);
                            if (pdestsp != NULL) {
                                this_sp = sp;
                                break;
                            }
                        }
                    }
                }
            }

            /* Check if found StructN or ResN for age structured - not scaling those so skip them in the warning message */
            found_sn = 0;
            if (pdestsn != NULL) {
                found_sn = 1;
            }
            found_rn = 0;
            if (pdestrn != NULL) {
                found_rn = 1;
            }
            
			/* If a species was found scale */
			if (this_sp < bm->K_num_tot_sp) {
				if(bm->init_scalar[this_sp] != 1.0){
					warn("%s scaled by %e\n", bm->tinfo[i].name, bm->init_scalar[this_sp]);
				}
				for (b = 0; b < bm->nbox; b++) {
					for (k = 0; k < bm->wcnz; k++) {
						val[b][k] *= bm->init_scalar[this_sp];
                        
                        //fprintf(bm->logFile, "readBMTracerData - box: %d, layer: %d %s (%s) scaled by %e\n", b, k, bm->tinfo[i].name, FunctGroupArray[this_sp].groupCode, bm->init_scalar[this_sp]);
					}
                    for (k = 0; k < bm->sednz; k++) {
                        val[b][k + bm->wcnz] *= bm->init_scalar[this_sp];
                    }
				}
			} else if ((found_partial_match || found_partial_match_n) && (!found_sn && !found_rn)){
				warn("run.prm file says scale initial numbers or biomasses, but no string matches found, no scaling performed for %s\n", bm->tinfo[i].name);
			}
		}

		/* Move water column data to model storage */
		for (b = 0; b < bm->nbox; b++) {
			for (k = 0; k < bm->wcnz; k++) {
				bm->wctr[b][k][i] = (double) val[b][k];
				if (!(_finite(bm->wctr[b][k][i]))) {
					quit("readBMTracerData - box: %d, layer: %d %s (%d) localpool set to: %e\n", b, k, bm->tinfo[i].name, i, bm->wctr[b][k][i]);
				}
                
                //fprintf(bm->logFile, "readBMTracerData - box: %d, layer: %d %s (%d) localpool set to: %e (val %f)\n", b, k, bm->tinfo[i].name, i, bm->wctr[b][k][i], val[b][k]);
			}
		}

		/* Move sediment data to model storage */
		for (b = 0; b < bm->nbox; b++) {
			for (k = 0; k < bm->sednz; k++)
				bm->sedtr[b][k][i] = (double) val[b][k + bm->wcnz];

			/* If the stress tracer then store in box stress attribute */
			if (strcmp(bm->tinfo[i].name, "Stress") == 0) {
				if (bm->sedtr[b][0][i] > 0) {
					bm->boxes[b].stress = bm->sedtr[b][0][i];                    
                    bm->boxes[b].init_stress = bm->sedtr[b][0][i];
					bm->supplied_stress = 1;
				}
			}
		}
	} //for

	if (bm->supplied_stress)
		printf("Bottom stress values found in input file\n");

	/* Free temporary storage */
	free2dInput((doubleINPUT **) val);
	//free2d((double **)val);
    
}

/*******************************************************************//**
 Routine to write the tracer data to a netCDF file. This routine
 assumes that the tracer information in the MSEBoxModel is valid and
 corresponds with the netCDF file. Minimal checking for consistency
 is done here. Does check to see if going to correct output file, depending
 on whether fisheries or general data type.
 *********************************************************************/
void writeBMTracerData(int fid, int dump, MSEBoxModel *bm, int dtype) {
	int i = 0;
	doubleINPUT **val;
	long start[3];
	long count[3];
	int n = 0;

    /*
    int pid = FunctGroupArray[8].contamPropTracers[3][0];
    fprintf(bm->logFile, "Time: %e at start of writeBMTracerData for box%d-%d - test propContam %s-%d for %s in box%d-%d: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[8].groupCode, 3, bm->contaminantStructure[0]->contaminant_name, 3, 2, bm->boxes[3].tr[2][pid]);
     */
     
	if (verbose > 0)
		fprintf(stderr, "Entering writeBMTracerData\n");

	/* Set netCDF library error handling */
	ncopts = NC_VERBOSE | NC_FATAL;

	/* Allocate temporary storage for one tracer */
	val = (doubleINPUT **) alloc2dInput(bm->wcnz + bm->sednz, bm->nbox);

	/* Set indices for writing tracers */
	start[0] = dump;
	start[1] = 0;
	start[2] = 0;
	count[0] = 1;
	count[1] = bm->nbox;
	count[2] = bm->wcnz + bm->sednz;

	/* Loop over each tracer */
	for (i = 0; i < bm->ntracer; i++) {
		int vid;
		int b;
		int k;
		int kdiff;

		/* Ice tracers are written out seperately */
		if(bm->ice_on){
			if(bm->tinfo[i].inice == TRUE ){
				continue;
			}
		}
		//fprintf(bm->logFile, "writeBMTracerData - bm->tinfo[i].name = %s dtype: %d flagid: %d isUsed: %d\n",bm->tinfo[i].name, bm->tinfo[i].dtype, bm->tinfo[i].flagid, bm->tinfo[i].isUsed);

		/* Check to see which output stream being dealt with */
		if ((dtype == bm->tinfo[i].dtype) && bm->tinfo[i].flagid && bm->tinfo[i].isUsed) {
			/* Get netCDF variable id */
			vid = ncvarid(fid, bm->tinfo[i].name);

			/* Check double compatibility */
			if ((n = ncvarsize(fid, vid)) != sizeof(doubleINPUT))
				quit("writeBMTracerData: %s has %d bytes per value in file, code compiled with %d\n", bm->tinfo[i].name, n, sizeof(doubleINPUT));

			/* Get water column data from model storage */
			for (b = 0; b < bm->nbox; b++) {
				kdiff = bm->wcnz - bm->boxes[b].nz;
				for (k = 0; k < bm->wcnz; k++) {
                    if (k < kdiff) {
						val[b][k] = 0.0;
                    } else {
						val[b][k] = (double)(bm->wctr[b][k - kdiff][i]);
                    
                        /*
                        if(b==3) {
                            fprintf(bm->logFile, "writeBMTracerData - bm->tinfo[i].name = %s dtype: %d flagid: %d isUsed: %d val: %e vs %e\n",bm->tinfo[i].name, bm->tinfo[i].dtype, bm->tinfo[i].flagid, bm->tinfo[i].isUsed, val[b][k], bm->boxes[3].tr[2][pid]);
                        }
                         */
                    }
                    
				}

				if(bm->boxes[b].type == LAND){
					val[b][bm->wcnz - 1] = (double)(bm->wctr[b][0][i]);
				}
			}

			/* Get sediment data from model storage */
			for (b = 0; b < bm->nbox; b++)
				for (k = 0; k < bm->sednz; k++)
					val[b][k + bm->wcnz] = (double)bm->sedtr[b][k][i];

			/* Write data */
			ncvarput(fid, vid, start, count, val[0]);
		}
	}

	/* Free temporary storage */
	free2dInput((doubleINPUT **) val);

}

