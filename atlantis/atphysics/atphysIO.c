/*********************************************************************

 File:           atphysIO.c

 Created:        Mon May 15 20:38:29 EST 1995

 Author:         Stephen Walker
 CSIRO Division of Oceanography

 Purpose:        Routines which deal with variables in the
 box model which have physical significance
 are not tracers and are time dependent.
 These files are in netCDF format
 and should be read, written and created
 using the routines below.

 Arguments:      See each routine below.

 Returns:        See each routine below.

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

 30-03-2009 Bec Gorton
 Added code to initialise the start and count arrays in writeBMphysData.

 21-06-2012 Bec Gorton
 Added code to check the values read in from the netcdf files are defined.
 *********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sjwlib.h>
#include <netcdf.h>
#include <atlantisboxmodel.h>
#include <atUtilLib.h>

/*******************************************************************//**
 Routine to read the physical information from a netCDF file.
 This routine assumes that the file has the following
 format (roughly written in CDL).

 dimensions:

 t = unlimited;    // Time dimension
 b = N;            // Number of boxes
 z = N;	    // Number of layers (water column + sediment)

 // z values corresponds to vertical positions as follows:<br>
 // z = 0             Bottom water layer<br>
 // z = wcnz-1        Surface water column layer<br>
 // z = wcnz          Uppermost sediment layer<br>
 // z = wcnz+sednz-1  Deepest sediment layer<br>

 variables:

 // Time value for each record <br>
 double t(t);<br>
 // Units for time value - usually seconds since some epoch<br>
 // Order is year month day hour minute sec timezone<br>
 // For example:<br>
 t:units = "seconds since 1994-11-24 16:09:00 +10";<br>

 // The netCDF file has global attributes as follows
 :title = "Description of this data set";<br>
 :geometry = "Name of geometry file";<br>
 :parameters = "Name of parameter file";<br>
 :wcnz = N;     // Number of water column layers<br>
 :sednz = N;    // Number of sediment layers


 This function sets the following box model data:
 bm->wcnz
 bm->sednz
 bm->t_units


 Memory is allocated for the following arrays for each box.

 bm->boxes[n].gridz
 bm->boxes[n].cellz
 bm->boxes[n].kz

 bm->boxes[n].sm.gridz
 bm->boxes[n].sm.cellz
 bm->boxes[n].sm.filltime
 bm->boxes[n].sm.css
 bm->boxes[n].sm.er
 bm->boxes[n].sm.dissol_kz
 bm->boxes[n].sm.partic_kz

 The following is also set for each box:

 bm->boxes[n].sm.nz = bm->sednz;

 *********************************************************************/
void readBMphysInfo(int fid, MSEBoxModel *bm) {
	int ndims = 0; /* Number of dimensions */
	int nvars = 0; /* Number of variables */
	int ngatts = 0; /* Number of global attributes */
	int recdim = 0; /* ID of unlimited dimension */
	int t_id = 0; /* Dimension ids */
	int b_id = 0;
	int z_id = 0;
	long n = 0;
	nc_type ty;
	int len = 0;

	if (verbose > 1)
		fprintf(stderr, "Entering readBMphysInfo\n");

	/* Set netCDF library error handling */
	ncopts = NC_VERBOSE | NC_FATAL;

	/* Inquire about this file */
	ncinquire(fid, &ndims, &nvars, &ngatts, &recdim);
	if (ndims < 3)
		quit("readBMphysInfo: not enough dimensions\n");
	if (nvars < 2)
		quit("readBMphysInfo: not enough variables\n");
	if (ngatts < 2)
		quit("readBMphysInfo: not enough attributes\n");

	/* Check dimensions are as expected */
	if ((t_id = ncdimid(fid, "t")) == -1)
		quit("readBMphysInfo: no t dimension");
	if (t_id != recdim)
		quit("readBMphysInfo: t dimension not unlimited\n");
	if ((b_id = ncdimid(fid, "b")) == -1)
		quit("readBMphysInfo: no b dimension\n");
	if ((z_id = ncdimid(fid, "z")) == -1)
		quit("readBMphysInfo: no z dimension\n");

	/* Get max numbers of layers from attributes */
	/* Check the wcnz attribute is the correct type and length */
	ncattinq(fid, NC_GLOBAL, "wcnz", &ty, &len);
	if (nctypelen(ty) != sizeof(bm->wcnz) || len != 1)
		quit("readBMphysInfo: wcnz attribute has wrong type\n");

	/* Check the sednz attribute is the correct type and length */
	ncattinq(fid, NC_GLOBAL, "sednz", &ty, &len);
	if (nctypelen(ty) != sizeof(bm->sednz) || len != 1)
		quit("readBMphysInfo: sednz attribute has wrong type\n");

	/* Get the attribute values */
	ncattget(fid, NC_GLOBAL, "wcnz", &bm->wcnz);
	ncattget(fid, NC_GLOBAL, "sednz", &bm->sednz);

	if (bm->wcnz < 1)
		quit("readBMphysInfo: Must be at least 1 water column layer\n");

	if (bm->sednz < 0)
		quit("readBMphysInfo: Must be at least 0 sediment layer\n");

	if (verbose > 1)
		printf("readBMphysInfo: %d water column layers, %d sediment layers\n", bm->wcnz, bm->sednz);

	/* Get dimension sizes and check against geometry */
	ncdiminq(fid, b_id, NULL, &n);
	if (n != bm->nbox)
		quit("readBMphysInfo: Number of boxes (%ld) doesn't match geometry (%d)\n", n, bm->nbox);

	ncdiminq(fid, z_id, NULL, &n);
	if (n != bm->wcnz + bm->sednz)
		quit("readBMphysInfo: Size of z dimension (%ld) doesn't match geometry (%d+%d)\n", n, bm->wcnz, bm->sednz);

	/* Read time units */
	ncattget(fid, ncvarid(fid, "t"), "units", bm->t_units);

	/* Allocate memory for vertical arrays */
	for (n = 0; n < bm->nbox; n++) {
		bm->boxes[n].gridz = alloc1d(bm->wcnz + 1);
		bm->boxes[n].cellz = alloc1d(bm->wcnz);
		bm->boxes[n].kz = alloc1d(bm->wcnz + 1);
		bm->boxes[n].sm.nz = bm->sednz;
		bm->boxes[n].sm.gridz = alloc1d(bm->sednz + 1);
		bm->boxes[n].sm.cellz = alloc1d(bm->sednz);
		bm->boxes[n].sm.filltime = alloc1d(bm->sednz);
		bm->boxes[n].sm.css = alloc1d(bm->sednz);
		bm->boxes[n].sm.er = alloc1d(bm->sednz);
		bm->boxes[n].sm.dissol_kz = alloc1d(bm->sednz + 1);
		bm->boxes[n].sm.partic_kz = alloc1d(bm->sednz + 1);

	}
}

/**** Added free routine for physical info.
 ****/
void freeBMphysInfo(MSEBoxModel *bm) {
	int n = 0;

	/* Deallocate memory for vertical arrays */
	for (n = 0; n < bm->nbox; n++) {
		free1d(bm->boxes[n].gridz);
		free1d(bm->boxes[n].cellz);
		free1d(bm->boxes[n].kz);
		free1d(bm->boxes[n].sm.gridz);
		free1d(bm->boxes[n].sm.cellz);
		free1d(bm->boxes[n].sm.filltime);
		free1d(bm->boxes[n].sm.css);
		free1d(bm->boxes[n].sm.er);
		free1d(bm->boxes[n].sm.dissol_kz);
		free1d(bm->boxes[n].sm.partic_kz);
        
//		if(bm->ice_on){
//			free1d(bm->boxes[n].ice.gridz);
//		}
        
        if (bm->flagindustry_on == simple_industry_model) {
            i_free1d(bm->boxes[n].land.ship_transits);
            i_free1d(bm->boxes[n].land.new_ship_num);
            i_free1d(bm->boxes[n].land.ship_num);
        }

	}
    
    if (bm->flagindustry_on == simple_industry_model) {
        i_free1d(bm->ship_id);
        free1d(bm->ship_mixing_rate);
        free1d(bm->vessel_size);
    }
    
}

void writeBMphysInfo(int fid, MSEBoxModel *bm, int dtype) {
	int i = 0;
	nc_type dt;
	int dim[3];

	if (verbose > 1)
		fprintf(stderr, "Entering writeBMphysInfo\n");

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

	if (!dtype) {
		/* The following are only required in the general data set not the
		 fisheries data or summary data */

		/* Define the hydrodynamics source variable */
		i = ncvardef(fid, "hdsource", dt, 3, dim);
		ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("phys") + 1, "phys");
		ncattput(fid, i, "units", NC_CHAR, (int) strlen("1") + 1, "1");
		ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Hydrodynamic sources") + 1, "Hydrodynamic sources");

		/* Define the hydrodynamics sink - only included in general data set */
		i = ncvardef(fid, "hdsink", dt, 3, dim);
		ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("phys") + 1, "phys");
		ncattput(fid, i, "units", NC_CHAR, (int) strlen("1") + 1, "1");
		ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Hydrodynamic sinks") + 1, "Hydrodynamic sinks");

		/* Define the exchange horizontal flux variable */
		i = ncvardef(fid, "eflux", dt, 3, dim);
		ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("phys") + 1, "phys");
		ncattput(fid, i, "units", NC_CHAR, (int) strlen("1") + 1, "1");
		ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Hydrodynamic net exchanges") + 1, "Hydrodynamic net exchanges");

		/* Define the vertical exchange flux variable */
		i = ncvardef(fid, "vflux", dt, 3, dim);
		ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("phys") + 1, "phys");
		ncattput(fid, i, "units", NC_CHAR, (int) strlen("1") + 1, "1");
		ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Hydrodynamic net vertical exchanges") + 1, "Hydrodynamic net vertical exchanges");

		/* Define the porosity variable - only included in general data set */
		i = ncvardef(fid, "porosity", dt, 3, dim);
		ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("phys") + 1, "phys");
		ncattput(fid, i, "units", NC_CHAR, (int) strlen("1") + 1, "1");
		ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Porosity") + 1, "Porosity");
	}

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

	/* Define the sediment top index variable */
	i = ncvardef(fid, "numlayers", NC_SHORT, 2, dim);
	ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("phys") + 1, "phys");
	ncattput(fid, i, "units", NC_CHAR, (int) strlen("") + 1, "");
	ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Number of layers") + 1, "Number of layers");

	if (!dtype) {
		/* The following are only required in the general data set not the
		 fisheries data or summary data */

		/* Define the sediment biological depth variable */
		i = ncvardef(fid, "sedbiodepth", dt, 2, dim);
		ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("phys") + 1, "phys");
		ncattput(fid, i, "units", NC_CHAR, (int) strlen("m") + 1, "m");
		ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Depth of biological activity") + 1, "Depth of biological activity");

		/* Define the detrital depth variable */
		i = ncvardef(fid, "seddetdepth", dt, 2, dim);
		ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("phys") + 1, "phys");
		ncattput(fid, i, "units", NC_CHAR, (int) strlen("m") + 1, "m");
		ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Max depth of detritus") + 1, "Max depth of detritus");

		/* Define the oxygen horizon depth variable */
		i = ncvardef(fid, "sedoxdepth", dt, 2, dim);
		ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("phys") + 1, "phys");
		ncattput(fid, i, "units", NC_CHAR, (int) strlen("m") + 1, "m");
		ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Depth of oxygen horizon") + 1, "Depth of oxygen horizon");

		/* Define the sediment biological activity variable */
		i = ncvardef(fid, "sedbiodens", dt, 2, dim);
		ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("phys") + 1, "phys");
		ncattput(fid, i, "units", NC_CHAR, (int) strlen("Animals m-2") + 1, "Animals m-2");
		ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Biological activity") + 1, "Biological activity");

		/* Define the sediment biological activity (bioirrigation) enhancement variable */
		i = ncvardef(fid, "sedirrigenh", dt, 2, dim);
		ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("phys") + 1, "phys");
		ncattput(fid, i, "units", NC_CHAR, (int) strlen("1") + 1, "1");
		ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Bioirrigation enhancement") + 1, "Bioirrigation enhancement");

		/* Define the sediment biological activity (bioturbation) enhancement variable */
		i = ncvardef(fid, "sedturbenh", dt, 2, dim);
		ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("phys") + 1, "phys");
		ncattput(fid, i, "units", NC_CHAR, (int) strlen("1") + 1, "1");
		ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Bioturbation enhancement") + 1, "Bioturbation enhancement");

		/* Define the erosion rate variable */
		i = ncvardef(fid, "erosion_rate", dt, 2, dim);
		ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("phys") + 1, "phys");
		ncattput(fid, i, "units", NC_CHAR, (int) strlen("m s-1") + 1, "m s-1");
		ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Erosion rate") + 1, "Erosion rate");

		/* Define the percent bottom cover due to reefs variable */
		i = ncvardef(fid, "reef", dt, 2, dim);
		ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("phys") + 1, "phys");
		ncattput(fid, i, "units", NC_CHAR, (int) strlen("1") + 1, "1");
		ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Percent reef cover") + 1, "Percent reef cover");

		/* Define the percent bottom cover due to flat ground variable */
		i = ncvardef(fid, "flat", dt, 2, dim);
		ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("phys") + 1, "phys");
		ncattput(fid, i, "units", NC_CHAR, (int) strlen("1") + 1, "1");
		ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Percent flat cover") + 1, "Percent flat cover");

		/* Define the percent bottom cover due to soft ground variable */
		i = ncvardef(fid, "soft", dt, 2, dim);
		ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("phys") + 1, "phys");
		ncattput(fid, i, "units", NC_CHAR, (int) strlen("1") + 1, "1");
		ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Percent soft cover") + 1, "Percent soft cover");

		/* Define the percent canyon area variable */
		i = ncvardef(fid, "canyon", dt, 2, dim);
		ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("phys") + 1, "phys");
		ncattput(fid, i, "units", NC_CHAR, (int) strlen("1") + 1, "1");
		ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Percent canyon") + 1, "Percent canyon");

		/* Define the eddy strength variable */
		i = ncvardef(fid, "eddy", dt, 2, dim);
		ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("phys") + 1, "phys");
		ncattput(fid, i, "units", NC_CHAR, (int) strlen("1") + 1, "1");
		ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Eddy strength") + 1, "Eddy strength");

        /* Define the coastal strip variable */
        i = ncvardef(fid, "coastal", dt, 2, dim);
        ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("phys") + 1, "phys");
        ncattput(fid, i, "units", NC_CHAR, (int) strlen("1") + 1, "1");
        ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Percent coastal strip cover") + 1, "Percent coastal cover");

	}
}

/*******************************************************************//**
 Routine to read the physical data from a netCDF file.
 This routine assumes that the file has the following
 format (roughly written in CDL).

 dimensions:

 t = unlimited;    // Time dimension
 b = N;            // Number of boxes
 z = N;	    // Number of layers (water column + sediment)

 // z values correspond to vertical positions as follows:
 // z = 0             Bottom water layer
 // z = wcnz-1        Surface water column layer
 // z = wcnz          Uppermost sediment layer
 // z = wcnz+sednz-1  Deepest sediment layer

 variables:

 // Time value for each record
 double t(t);
 // Units for time value - usually seconds since some epoch
 // Order is year month day hour minute sec timezone
 // For example:
 t:units = "seconds since 1994-11-24 16:09:00 +10";

 // Nominal cell thicknesses
 double nominal_dz(b,z);
 nominal_dz:bmtype = "phys";      // This is a physical variable
 nominal_dz:units = "m";          // Units
 nominal_dz:long_name = "Thickness"; // Long name

 // Time varying cell thicknesses
 double dz(t,b,z);
 dz:bmtype = "phys";      // This is a physical variable
 dz:units = "m";          // Units
 dz:long_name = "Thickness"; // Long name

 // Top sediment layer index
 short topk(t,b);
 topk:bmtype = "phys";      // This is a physical variable
 topk:units = "";           // Units
 topk:long_name = "Top sediment index"; // Long name

 // Depth of biological activity in sediment
 double sedbiodepth(t, b);
 sedbiodepth:bmtype = "phys"; // This is a physical variable!
 sedbiodepth:units = "m";     // Units are metres
 sedbiodepth:long_name = "Depth of biological activity";  // Long name

 // Amount of biological activity
 double sedbiodens(t, b);
 sedbiodens:bmtype = "phys";  // This is a physical variable!
 sedbiodens:units = "Animals per m2";      // Units are animals per m2
 sedbiodens:long_name = "Biological activity";  // Long name

 // Erosion rate
 double erosion_rate(t, b);
 erosion_rate:bmtype = "phys";  // This is a physical variable!
 erosion_rate:
 units = "m s-1";      // Units are m s-1
 erosion_rate:long_name = "Erosion rate";  // Long name

 // The netCDF file has global attributes as follows
 :title = "Description of this data set";
 :geometry = "Name of geometry file";
 :parameters = "Name of parameter file";
 :wcnz = N;     // Number of water column layers
 :sednz = N;    // Number of sediment layers

 @param fid file ID
 @param dump input file dump - file start index.
 @bm reference to box model
 *********************************************************************/
void readBMphysData(int fid, int dump, MSEBoxModel *bm, FILE *llogfp) {
	long start[3];
	long int count[3];
	int b = 0;
	long n = 0;
	int id = 0;
	short *topk;
	short *numnz;
	doubleINPUT *fptmp;
	doubleINPUT **array;
	int i;

	if (verbose > 1)
		fprintf(stderr, "Entering readBMphysData\n");

	/* Allocate temporary arrays */
	fptmp = (doubleINPUT *) alloc1dInput(bm->nbox);
	array = (doubleINPUT **) alloc2dInput(bm->wcnz + bm->sednz, bm->nbox);

	topk = s_alloc1d(bm->nbox);
	numnz = s_alloc1d(bm->nbox);

	/* Set netCDF library error handling */
	ncopts = NC_VERBOSE | NC_FATAL;

	/* Check that requested dump exists in file */
	ncdiminq(fid, ncdimid(fid, "t"), NULL, &n);
	if (dump >= n)
		quit("readBMphysData: dump %d not in file (%ld records)\n", dump, n);

	/* Read time value */
	start[0] = dump;
	count[0] = 1L;
	ncvarget(fid, ncvarid(fid, "t"), start, count, (void *) &bm->t);

	/* Indices for reading 3D values */
	start[0] = dump;
	start[1] = 0;
	start[2] = 0;
	count[0] = 1;
	count[1] = bm->nbox;
	count[2] = bm->wcnz + bm->sednz;

	/* Read physical variables */
	ncvarread(fid, "nominal_dz", sizeof(doubleINPUT), &start[1], &count[1], (void *) array[0]);
	checkNetCDFData2D("readBMphysData", "nominal_dz", array,  bm->nbox, bm->wcnz + bm->sednz);

	for (b = 0; b < bm->nbox; b++) {
		for (i = 0; i < bm->wcnz + bm->sednz; i++) {
			bm->nom_dz[b][i] = array[b][i];
 		}
	}

	/* Check if time varying dz is present */
	ncopts = 0;
	id = ncvarid(fid, "dz");
	ncopts = NC_VERBOSE | NC_FATAL;
	if (id >= 0) {
		ncvarread(fid, "dz", sizeof(doubleINPUT), start, count, (void *) array[0]);
		checkNetCDFData2D("readBMphysData", "dz", array,  bm->nbox, bm->wcnz + bm->sednz);
		for (b = 0; b < bm->nbox; b++)
			for (i = 0; i < bm->wcnz + bm->sednz; i++) {
				bm->dz[b][i] = array[b][i];
			}
	} else {
		ncvarread(fid, "nominal_dz", sizeof(doubleINPUT), &start[1], &count[1], (void *) array[0]);
		for (b = 0; b < bm->nbox; b++)
			for (i = 0; i < bm->wcnz + bm->sednz; i++)
				bm->dz[b][i] = array[b][i];
	}

	/* Index of the first non-empty sediment layer */
	ncvarread(fid, "topk", sizeof(short), start, count, (void *) topk);
	checkNetCDFData1DShort("readBMphysData", "topk", topk, bm->nbox);
	for (b = 0; b < bm->nbox; b++)
		bm->boxes[b].sm.topk = topk[b];

	/* Number of water column layers*/
	ncvarread(fid, "numlayers", sizeof(short), start, count, (void *) numnz);
	checkNetCDFData1DShort("readBMphysData", "numlayers", numnz,  bm->nbox);
	for (b = 0; b < bm->nbox; b++)
		bm->boxes[b].numlayers = numnz[b];

	/* Read in the sediment variables. */

	/* Max depth for biological activity */
	ncvarread(fid, "sedbiodepth", sizeof(doubleINPUT), start, count, (void *) fptmp);
	checkNetCDFData1D("readBMphysData", "sedbiodepth", fptmp,  bm->nbox);
	for (b = 0; b < bm->nbox; b++)
		bm->boxes[b].sm.biodepth = (double) fptmp[b];

	/* Max depth for detritus (changes with time) */
	ncvarread(fid, "seddetdepth", sizeof(doubleINPUT), start, count, (void *) fptmp);
	checkNetCDFData1D("readBMphysData", "seddetdepth", fptmp,  bm->nbox);

	for (b = 0; b < bm->nbox; b++)
		bm->boxes[b].sm.detdepth = fptmp[b];

	/* Oxygen horizon (changes with time) */
	ncvarread(fid, "sedoxdepth", sizeof(doubleINPUT), start, count, (void *) fptmp);
	checkNetCDFData1D("readBMphysData", "sedoxdepth", fptmp,  bm->nbox);

	for (b = 0; b < bm->nbox; b++)
		bm->boxes[b].sm.oxdepth = fptmp[b];

	/* Measure of biological activity (animals per square metre?)*/
	ncvarread(fid, "sedbiodens", sizeof(doubleINPUT), start, count, (void *) fptmp);
	checkNetCDFData1D("readBMphysData", "sedbiodens", fptmp,  bm->nbox);
	for (b = 0; b < bm->nbox; b++)
		bm->boxes[b].sm.biodens = fptmp[b];

	/* Scalar of biological activity enhancement due to the actions of infaunal species. */
	ncvarread(fid, "sedirrigenh", sizeof(doubleINPUT), start, count, (void *) fptmp);
	checkNetCDFData1D("readBMphysData", "sedirrigenh", fptmp,  bm->nbox);
	for (b = 0; b < bm->nbox; b++)
		bm->boxes[b].sm.irrigenh = fptmp[b];

	/* Scalar of biological activity enhancement due to the actions of infaunal species.  */
	ncvarread(fid, "sedturbenh", sizeof(doubleINPUT), start, count, (void *) fptmp);
	checkNetCDFData1D("readBMphysData", "sedturbenh", fptmp,  bm->nbox);
	for (b = 0; b < bm->nbox; b++)
		bm->boxes[b].sm.turbenh = fptmp[b];

	/* Erosion rate (m s-1) */
	ncvarread(fid, "erosion_rate", sizeof(doubleINPUT), start, count, (void *) fptmp);
	checkNetCDFData1D("readBMphysData", "erosion_rate", fptmp,  bm->nbox);
	for (b = 0; b < bm->nbox; b++)
		bm->boxes[b].erosion_rate = fptmp[b];

	/* Percent rocky bottom */
	ncvarread(fid, "reef", sizeof(doubleINPUT), start, count, (void *) fptmp);
	checkNetCDFData1D("readBMphysData", "reef", fptmp,  bm->nbox);
	for (b = 0; b < bm->nbox; b++)
		bm->boxes[b].reef = fptmp[b];

	/* Percent flat sediment */
	ncvarread(fid, "flat", sizeof(doubleINPUT), start, count, (void *) fptmp);
	checkNetCDFData1D("readBMphysData", "flat", fptmp,  bm->nbox);
	for (b = 0; b < bm->nbox; b++)
		bm->boxes[b].flat = fptmp[b];

	/* Percent soft sediment */
	ncvarread(fid, "soft", sizeof(doubleINPUT), start, count, (void *) fptmp);
	checkNetCDFData1D("readBMphysData", "soft", fptmp,  bm->nbox);
	for (b = 0; b < bm->nbox; b++)
		bm->boxes[b].soft = fptmp[b];

	/* Percent area made up of canyons */
	ncvarread(fid, "canyon", sizeof(doubleINPUT), start, count, (void *) fptmp);
	checkNetCDFData1D("readBMphysData", "canyon", fptmp,  bm->nbox);
	for (b = 0; b < bm->nbox; b++)
		bm->boxes[b].canyon = fptmp[b];

	/* Strenght of eddy in the box */
	ncvarread(fid, "eddy", sizeof(doubleINPUT), start, count, (void *) fptmp);
	checkNetCDFData1D("readBMphysData", "eddy", fptmp,  bm->nbox);
	for (b = 0; b < bm->nbox; b++)
		bm->boxes[b].eddy = fptmp[b];
    
    printf("About to read coastal\n");
    
    /* Percent coastal strip */
    id = ncvarid(fid, "coastal");
    if (id >= 0) {
        ncvarread(fid, "coastal", sizeof(doubleINPUT), start, count, (void *) fptmp);
        checkNetCDFData1D("readBMphysData", "coastal", fptmp,  bm->nbox);
        for (b = 0; b < bm->nbox; b++) {
            bm->boxes[b].coastal = fptmp[b];
        }
    }
    
    /* Read hydrodynamic sources if present */
	id = ncvarid(fid, "hdsource");
	if (id >= 0) {
		ncvarread(fid, "hdsource", sizeof(doubleINPUT), start, count, (void *) array[0]);
		checkNetCDFData2D("readBMphysData", "hdsource", array,  bm->nbox,bm->wcnz + bm->sednz);
		for (b = 0; b < bm->nbox; b++)
			for (i = 0; i < (bm->wcnz + bm->sednz); i++) {
				bm->hddrain[b][i] = (double)array[b][i];
			}
	}

	/* Read hydrodynamic sinks if present */
	id = ncvarid(fid, "hdsink");
	if (id >= 0) {
		ncvarread(fid, "hdsink", sizeof(doubleINPUT), start, count, (void *) array[0]);
		checkNetCDFData2D("readBMphysData", "hdsink", array,  bm->nbox,bm->wcnz + bm->sednz);
		for (b = 0; b < bm->nbox; b++)
			for (i = 0; i < bm->wcnz + bm->sednz; i++)
				bm->hdpool[b][i] = (double)array[b][i];
	}

	/* Read hydrodynamic exchanges if present */
	id = ncvarid(fid, "eflux");
	if (id >= 0) {
		ncvarread(fid, "eflux", sizeof(doubleINPUT), start, count, (void *) array[0]);
		checkNetCDFData2D("readBMphysData", "eflux", array,  bm->nbox,bm->wcnz + bm->sednz);
		for (b = 0; b < bm->nbox; b++)
			for (i = 0; i < bm->wcnz + bm->sednz; i++)
				bm->efluxes[b][i] = (double)array[b][i];
	}

	/* Read hydrodynamic vertical exchanges if present */
	id = ncvarid(fid, "vflux");
	if (id >= 0) {
		ncvarread(fid, "vflux", sizeof(doubleINPUT), start, count, (void *) array[0]);
		checkNetCDFData2D("readBMphysData", "vflux", array,  bm->nbox,bm->wcnz + bm->sednz);
		for (b = 0; b < bm->nbox; b++)
			for (i = 0; i < bm->wcnz + bm->sednz; i++)
				bm->vfluxes[b][i] = (double)array[b][i];
	}

	/* Read porosity if it is present */
	id = ncvarid(fid, "porosity");
	if (id >= 0) {
		ncvarread(fid, "porosity", sizeof(doubleINPUT), start, count, (void *) array[0]);
		checkNetCDFData2D("readBMphysData", "porosity", array,  bm->nbox,bm->wcnz + bm->sednz);
		for (b = 0; b < bm->nbox; b++)
			for (i = 0; i < bm->wcnz + bm->sednz; i++) {
				bm->por[b][i] = (double)array[b][i];

			}
	}

	/* Calculate number of layers in each box from nominal dz values */
	nz_from_nomdz(bm);

	/* Calculate volumes from cell thicknesses. */
	volume_from_dz(bm);

	/* Calculate gridz and cellz coordinates */
	layer_coords(bm, llogfp);

	/* Free temporary arrays */
	free1dInput(fptmp);
	free2dInput(array);

	s_free1d(topk);
	s_free1d(numnz);
}

void writeBMphysData(int fid, int dump, MSEBoxModel *bm, int dtype) {
	long start[3];
	long count[3];
	short *topk;
	short *numnz;
	doubleINPUT *fptmp;
	doubleINPUT **valsrc;
	doubleINPUT **valsink;
	doubleINPUT **valeflux;
	doubleINPUT **valvflux;
	doubleINPUT **valdz;
	doubleINPUT **valvol;
	int b, k, kdiff;
	doubleINPUT value;

	if (verbose > 0)
		printf("Entering writeBMphysData\n");

	/* Allocate temporary arrays */
	fptmp = (doubleINPUT *) alloc1dInput(bm->nbox);
	valsrc = (doubleINPUT **) alloc2dInput(bm->wcnz + bm->sednz, bm->nbox);
	valsink = (doubleINPUT **) alloc2dInput(bm->wcnz + bm->sednz, bm->nbox);
	valeflux = (doubleINPUT **) alloc2dInput(bm->wcnz + bm->sednz, bm->nbox);
	valvflux = (doubleINPUT **) alloc2dInput(bm->wcnz + bm->sednz, bm->nbox);
	valdz = (doubleINPUT **) alloc2dInput(bm->wcnz + bm->sednz, bm->nbox);
	valvol = (doubleINPUT **) alloc2dInput(bm->wcnz + bm->sednz, bm->nbox);

	topk = s_alloc1d(bm->nbox);
	numnz = s_alloc1d(bm->nbox);

	/* Set netCDF library error handling */
	ncopts = NC_VERBOSE | NC_FATAL;

	/* Write time value */
	start[0] = dump;
	start[1] = 0;
	start[2] = 0;
	count[0] = 1L;
	count[1] = 1L;
	count[2] = 1L;
	value = (double) bm->t;
	ncvarput(fid, ncvarid(fid, "t"), start, count, &value);

	/* Indices for writing 3D values */
	start[0] = dump;
	start[1] = 0;
	start[2] = 0;
	count[0] = 1;
	count[1] = bm->nbox;
	count[2] = bm->wcnz + bm->sednz;

	/* Write physical variables */
	ncvarput(fid, ncvarid(fid, "nominal_dz"), &start[1], &count[1], bm->nom_dz[0]);

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

	if (!dtype) {

		/* Get water column data from model storage */
		for (b = 0; b < bm->nbox; b++) {
			kdiff = bm->wcnz - bm->boxes[b].nz;
			for (k = 0; k < bm->wcnz; k++) {
				if (k < kdiff) {
					valsrc[b][k] = 0.0;
					valsink[b][k] = 0.0;
					valeflux[b][k] = 0.0;
					valvflux[b][k] = 0.0;
				} else {
					valsrc[b][k] = bm->boxes[b].hdsource[k - kdiff];
					valsink[b][k] = bm->boxes[b].hdsink[k - kdiff];
					valeflux[b][k] = bm->boxes[b].eflux[k - kdiff];
					valvflux[b][k] = bm->boxes[b].vflux[k - kdiff];
				}
			}
		}

		/* Get sediment data from model storage */
		for (b = 0; b < bm->nbox; b++) {
			for (k = 0; k < bm->sednz; k++) {
				valsrc[b][k + bm->wcnz] = 0.0;
				valsink[b][k + bm->wcnz] = 0.0;
				valeflux[b][k + bm->wcnz] = 0.0;
				valvflux[b][k + bm->wcnz] = 0.0;
			}
		}

		/* Write data */
		ncvarput(fid, ncvarid(fid, "hdsource"), start, count, valsrc[0]);
		ncvarput(fid, ncvarid(fid, "hdsink"), start, count, valsink[0]);
		ncvarput(fid, ncvarid(fid, "eflux"), start, count, valeflux[0]);
		ncvarput(fid, ncvarid(fid, "vflux"), start, count, valvflux[0]);
		ncvarput(fid, ncvarid(fid, "porosity"), start, count, bm->por[0]);
	}

	for (b = 0; b < bm->nbox; b++)
		topk[b] = (short) bm->boxes[b].sm.topk;
	ncvarput(fid, ncvarid(fid, "topk"), start, count, topk);

	for (b = 0; b < bm->nbox; b++)
		numnz[b] = (short)bm->boxes[b].numlayers;
	ncvarput(fid, ncvarid(fid, "numlayers"), start, count, numnz);

	if (!dtype) {
		/* The following are only written for the general data set */
        for (b = 0; b < bm->nbox; b++){
            fptmp[b] = bm->boxes[b].sm.biodepth;
        }
		ncvarput(fid, ncvarid(fid, "sedbiodepth"), start, count, fptmp);

        for (b = 0; b < bm->nbox; b++){
            fptmp[b] = bm->boxes[b].sm.detdepth;
        }
		ncvarput(fid, ncvarid(fid, "seddetdepth"), start, count, fptmp);

        for (b = 0; b < bm->nbox; b++){
            fptmp[b] = bm->boxes[b].sm.oxdepth;
        }
		ncvarput(fid, ncvarid(fid, "sedoxdepth"), start, count, fptmp);

        for (b = 0; b < bm->nbox; b++){
            fptmp[b] = bm->boxes[b].sm.biodens;
        }
		ncvarput(fid, ncvarid(fid, "sedbiodens"), start, count, fptmp);

        for (b = 0; b < bm->nbox; b++){
            fptmp[b] = bm->boxes[b].sm.irrigenh;
        }
		ncvarput(fid, ncvarid(fid, "sedirrigenh"), start, count, fptmp);

        for (b = 0; b < bm->nbox; b++){
            fptmp[b] = bm->boxes[b].sm.turbenh;
        }
		ncvarput(fid, ncvarid(fid, "sedturbenh"), start, count, fptmp);

        for (b = 0; b < bm->nbox; b++){
            fptmp[b] = bm->boxes[b].erosion_rate;
        }
		ncvarput(fid, ncvarid(fid, "erosion_rate"), start, count, fptmp);

        for (b = 0; b < bm->nbox; b++){
            fptmp[b] = bm->boxes[b].reef;
        }
		ncvarput(fid, ncvarid(fid, "reef"), start, count, fptmp);

        for (b = 0; b < bm->nbox; b++){
            fptmp[b] = bm->boxes[b].flat;
        }
		ncvarput(fid, ncvarid(fid, "flat"), start, count, fptmp);

        for (b = 0; b < bm->nbox; b++){
            fptmp[b] = bm->boxes[b].soft;
        }
		ncvarput(fid, ncvarid(fid, "soft"), start, count, fptmp);

        for (b = 0; b < bm->nbox; b++){
            fptmp[b] = bm->boxes[b].canyon;
        }
		ncvarput(fid, ncvarid(fid, "canyon"), start, count, fptmp);

        for (b = 0; b < bm->nbox; b++){
            fptmp[b] = bm->boxes[b].eddy;
        }
		ncvarput(fid, ncvarid(fid, "eddy"), start, count, fptmp);

        for (b = 0; b < bm->nbox; b++){
            fptmp[b] = bm->boxes[b].coastal;
        }
        ncvarput(fid, ncvarid(fid, "coastal"), start, count, fptmp);

	}

	free1dInput((doubleINPUT *) fptmp);
	free2dInput((doubleINPUT **) valsrc);
	free2dInput((doubleINPUT **) valsink);
	free2dInput((doubleINPUT **) valeflux);
	free2dInput((doubleINPUT **) valvflux);
	free2dInput((doubleINPUT **) valdz);
	free2dInput((doubleINPUT **) valvol);

	s_free1d(topk);
	s_free1d(numnz);
}
