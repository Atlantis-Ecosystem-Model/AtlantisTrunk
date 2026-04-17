/*******************************************************************//**
\file
\brief Routines which deal with 3d hydrodynamic model input.
    File:           athydromod.c

    Created:        Fri Dec 23 08:52:31 EST 1994

    Author:         Stephen Walker
                    CSIRO Division of Oceanography

    Purpose:        Routines which deal with 3d hydrodynamic model input.

    Arguments:

    Returns:        void

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

					22-May-2009 Bec Gorton
					Changed the ncopen calls to use sjw_ncopen. This function checks that the
					netcdf file exits before calling ncopen which will crash if the file is not
					found.
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
void    get_hydro(MSEBoxModel *bm);
void    open_hydro(MSEBoxModel *bm, char *name);
void    close_hydro(MSEBoxModel *bm);

/**
	Read in the information about the hydrodynamic model files.
	This function does not actually read in the model data, it just reads
	information from the force input file about the model input files.
	Memory for the hydro file names are allocated.
	The first hydro file is opened and the first chunk of data is read in.
*/
void hydro_init(MSEBoxModel *bm)
{
    FILE *fp;
    int i = 0;

    /* Open the file containing the list of hydro input files */
    if( (fp=Open_Input_File(bm->inputFolder, bm->forceIfname,"r")) == NULL )
    {
		printf("hydro_init: Can't open %s%s\n",bm->inputFolder, bm->forceIfname);
		quit("hydro_init: Can't open %s%s\n",bm->inputFolder, bm->forceIfname);
	}

    /* Set error routine to quit if parameters not found */
    set_keyprm_errfn(quit);

    /* Read the number of files */
    readkeyprm_i(fp,"nhdfiles",&bm->hd.nfiles);
    if( bm->hd.nfiles < 1 )
		quit("hydro_init: Must be at least 1 hydrodynamic input file (%d in %s)\n",bm->hd.nfiles,bm->forceIfname);
    
    /* While here read what time reference the time series will be using */
    readkeyprm_i(fp,"ts_on_hydro_time",&bm->ts_on_hydro_time);
    
    /* What frequency should we write out the inputs.ts and export.ts files? */
    time_sec_param(fp,"inputs_tout",&bm->inputs_tout);
    bm->inputs_toutNext = 0;

    /* Allocate memory for file names if necessary */
    if( !bm->hd.fname )
		bm->hd.fname = c_alloc2d(BMSLEN,bm->hd.nfiles);

    /* Read the list of files */
    for(i=0; i<bm->hd.nfiles; i++) {
		char key[BMSLEN];
		snprintf(key,sizeof(key),"hd%d.name",i);
		readkeyprm_s(fp,key,bm->hd.fname[i]);
    }

    /* Close the parameter file */
    fclose(fp);

    /*TODO: Calculate current file and record number
     * from box model time - FIX - HOW DO I DO THIS,
     * PARTICULARLY FOR HOT STARTS?
     */
    warn("hydro_init: Starting from beginning of hydro inputs\n"
	 "This will probably be incorrect for a 'hot' start of the model\n\n");

    bm->hd.curfile = 0;

    open_hydro(bm,bm->hd.fname[0]);


    /* Read first set of data */
    get_hydro(bm);
}

/**
 * Routine to get the hydrodynamic data for this time step.
 * This may involve reading the netCDF input file if the
 * data is not already in the memory buffers
 */
void get_hydro(MSEBoxModel *bm)
{
	long offset = 0;
    long stepnum = bm->hd.nextrec;
    doubleINPUT *value;
    int i, j, k, l;
    //int b, d;
    doubleINPUT ****array;

    value = alloc1dInput(bm->hd.nbuf);

    ncopts = NC_VERBOSE | NC_FATAL;

    /* Have we finished this file? */
    if( stepnum >= bm->hd.nstep ) {
		/* Yes - close it and open the next one */
		close_hydro(bm);
		bm->hd.curfile = (bm->hd.curfile+1)%bm->hd.nfiles;
		open_hydro(bm,bm->hd.fname[bm->hd.curfile]);
		stepnum = bm->hd.nextrec;

	 }
    
	/* Is the requested data already in the memory buffers? */
    if( stepnum < bm->hd.bufstart || stepnum > bm->hd.bufend ) {
		/* Data must be read from file, so we might as well
		* read bm->hd.nstep records, starting at the requested
		* record number (ensuring, of course, that we don't try
		* to read past the last record in the file).
		*/
		long start[4];
		long count[4];

		start[0] = stepnum;
		start[1] = 0;
		start[2] = 0;
		start[3] = 0;
		count[0] = min(bm->hd.nbuf, bm->hd.nstep-stepnum);
		count[1] = bm->nbox;
		count[2] = bm->wcnz;
		count[3] = bm->hd.dsize;

		array = alloc4dInput(count[3], count[2], count[1], count[0]);

		ncvarget(bm->hd.fid,bm->hd.t_vid,start,count, value);
        for(i = 0; i < bm->hd.nbuf; i++)
            bm->hd.tbuf[i] = (double)value[i];

		ncvarget(bm->hd.fid,bm->hd.e_vid,start,count,array[0][0][0]);
        for(i = 0; i < count[0]; i++)
            for(j = 0; j < count[1]; j++)
                for(k = 0; k < count[2] ; k++)
                    for(l = 0; l < count[3]; l++)
                        bm->hd.ebuf[i][j][k][l] = (double)array[i][j][k][l];

      //  ncvarget(bm->hd.fid,bm->hd.e_vid,start,count,bm->hd.ebuf[0][0][0]);
		ncvarget(bm->hd.fid,bm->hd.b_vid,start,count,bm->hd.bbuf[0][0][0]);
		ncvarget(bm->hd.fid,bm->hd.k_vid,start,count,bm->hd.kbuf[0][0][0]);




		bm->hd.bufstart = stepnum;
		bm->hd.bufend = stepnum + count[0] - 1;
		free4dInput(array);

	}


    /* Data must now be in buffers, so adjust pointers */
    offset = stepnum - bm->hd.bufstart;
    bm->hd.exch = bm->hd.ebuf[offset];
    bm->hd.b = bm->hd.bbuf[offset];
    bm->hd.k = bm->hd.kbuf[offset];

	/* Debug check to see what exchanges are read in *
	if(bm->t/ 86400.0 > 580) {
		for(b=0; b<bm->nbox; b++){
			for(k=0; k<bm->wcnz; k++){
				for(d=0; d<bm->hd.dsize; d++){
					printf("box%d-k%d (nz: %d vs numlayer: %d), bm->hd.exch[%d][%d][%d]: %.12f, bm->hd.b[b][k][d]: %d, bm->hd.k[b][k][d]: %d\n",
						b, k, bm->boxes[b].nz, bm->boxes[b].numlayers, b, k, d, bm->hd.exch[b][k][d], bm->hd.b[b][k][d], bm->hd.k[b][k][d]);
				}
			}
		}
	}
    */

	/* */
    /* Store hydrodynamic time value */
    bm->hd.t = bm->hd.tbuf[offset] + bm->hd.dt/2.0;

    /* Reset time left for this hydrodynamic step */
    bm->hd.tleft = bm->hd.dt;

    /* Increment next record number */
    bm->hd.nextrec++;

    free1dInput(value);

}

/* Maximum amount of memory to allocate for exchange values */
#define MAXBUFMEM (2L*1024L*1024L)

/**
 *	/brief Routine to open a hydrodynamic input file and check
 * that it has appropriate structure.
 *
 *	@param bm - box model
 *	@param name - Name of the hydro file to open.
 */
void open_hydro(MSEBoxModel *bm, char *name)
{
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
    long bytes;
    doubleINPUT value;

    /* Set netCDF library error handling */
    ncopts = NC_VERBOSE;

    /* Open the file */
    if( (bm->hd.fid=Util_ncopen(bm->inputFolder, name,NC_NOWRITE)) < 0 )
	   quit("open_hydro: Can't open hydrodynamic model input data file %s\n",name);
    
    fprintf(bm->logFile, "Time: %e, opening hydrofile %s\n", bm->dayt, name);

    /* Inquire about this file */
    ncopts = NC_VERBOSE | NC_FATAL;
    ncinquire(bm->hd.fid,&ndims,&nvars,&natts,&recdim);
    if( ndims < 4 )
        quit("open_hydro: not enough dimensions in %s\n",name);
    if( nvars < 4 )
        quit("open_hydro: not enough variables in %s\n",name);
    if( natts < 3 )
        quit("open_hydro: not enough attributes in %s\n",name);

    /* Check dimensions are as expected */
    if( (bm->hd.t_did = ncdimid(bm->hd.fid,"t"))  == -1 )
        quit("open_hydro: no t dimension in %s\n",name);
    if( bm->hd.t_did != recdim )
        quit("open_hydro: t dimension not unlimited in %s\n",name);

    if( (bm->hd.b_did = ncdimid(bm->hd.fid,"b"))  == -1 )
        quit("open_hydro: no b dimension in %s\n",name);

    if( (bm->hd.z_did = ncdimid(bm->hd.fid,"z"))  == -1 )
        quit("open_hydro: no z dimension in %s\n",name);

    if( (bm->hd.d_did = ncdimid(bm->hd.fid,"dest"))  == -1 )
        quit("open_hydro: no dest dimension in %s\n",name);

    /* Get dimension sizes and check against geometry */
    ncdiminq(bm->hd.fid,bm->hd.b_did,NULL,&n);
    if( n != bm->nbox )
        quit("open_hydro: Number of boxes (%ld) in %s doesn't match geometry (%d)\n",n,name,bm->nbox);

    ncdiminq(bm->hd.fid,bm->hd.z_did,NULL,&n);
    if( n != bm->wcnz )
        quit("open_hydro: Number of layers (%ld) in %s doesn't match geometry (%d)\n",n,name,bm->wcnz);

    ncdiminq(bm->hd.fid,bm->hd.d_did,NULL,&bm->hd.dsize);
    if( bm->hd.dsize < 1 )
        quit("open_hydro: Number of destinations (%ld) in %s is not sensible\n",bm->hd.dsize,name);

    /* Check that time units and steps match this model */
    bm->hd.t_vid = ncvarid(bm->hd.fid,"t");
    if( bm->hd.t_vid < 0 )
		quit("open_hydro: no t variable in %s\n",name);

    memset(bm->hd.t_units,0,BMSLEN);

	// Get the units attribute of the time variable.
    ncattget(bm->hd.fid,bm->hd.t_vid,"units",bm->hd.t_units);

    sscanf(bm->hd.t_units,"%s",hdu);
    sscanf(bm->t_units,"%s",bmu);
    if( strcmp(hdu,bmu) != 0 )
        quit("open_hydro: Time units (%s) don't match model time units (%s)\n",hdu,bmu);

    ncattinq(bm->hd.fid,bm->hd.t_vid,"dt",&daty,&len);
   if( nctypelen(daty) != sizeof(doubleINPUT))
        quit("open_hydro: dt attribute wrong type\n");

    ncattget(bm->hd.fid,bm->hd.t_vid,"dt",&value);
    bm->hd.dt = (double)value;
    if( bm->hd.dt <= 0.0 )
		quit("open_hydro: dt attribute must have positive value\n");

    /* Find out how many time steps are in the file */
    ncdiminq(bm->hd.fid,bm->hd.t_did,NULL,&bm->hd.nstep);

    /* Get other variable ids */

	/* exchange variable id */
    bm->hd.e_vid = ncvarid(bm->hd.fid,"exchange");
    if( bm->hd.e_vid < 0 )
		quit("open_hydro: no exchange variable in %s\n",name);

	/* b dimension id */
    bm->hd.b_vid = ncvarid(bm->hd.fid,"dest_b");
    if( bm->hd.b_vid < 0 )
		quit("open_hydro: no dest_b variable in %s\n",name);

	/* dest_k variable id */
    bm->hd.k_vid = ncvarid(bm->hd.fid,"dest_k");
    if( bm->hd.k_vid < 0 )
		quit("open_hydro: no dest_k variable in %s\n",name);

    /* Check variable types and dimensions */
    ncvarinq(bm->hd.fid,bm->hd.e_vid,NULL,&daty,&ndims,dims,&natts);
    if( nctypelen(daty) != sizeof(doubleINPUT) )
       quit("open_hydro: Type of exchange variable doesn't match model\n");

    if(
			ndims != 4					||
			dims[0] != bm->hd.t_did		||
			dims[1] != bm->hd.b_did		||
			dims[2] != bm->hd.z_did		||
			dims[3] != bm->hd.d_did
		)
        quit("open_hydro: exchange variable has incorrect dimensions\n");

    ncvarinq(bm->hd.fid,bm->hd.b_vid,NULL,&daty,&ndims,dims,&natts);
    if( nctypelen(daty) != sizeof(int) )
        quit("open_hydro: Type of dest_b variable doesn't match model\n");

    if(
			ndims != 4					||
			dims[0] != bm->hd.t_did		||
			dims[1] != bm->hd.b_did		||
			dims[2] != bm->hd.z_did		||
			dims[3] != bm->hd.d_did
		)
        quit("open_hydro: dest_b variable has incorrect dimensions\n");

    ncvarinq(bm->hd.fid,bm->hd.k_vid,NULL,&daty,&ndims,dims,&natts);
    if( nctypelen(daty) != sizeof(int) )
        quit("open_hydro: Type of dest_k variable doesn't match model\n");

    if(
			ndims != 4					||
			dims[0] != bm->hd.t_did		||
			dims[1] != bm->hd.b_did		||
			dims[2] != bm->hd.z_did		||
			dims[3] != bm->hd.d_did
		)
        quit("open_hydro: dest_k variable has incorrect dimensions\n");

    /* Calculate a reasonable number of time steps
     * to store in buffers in memory
     */
    bytes = (long int)sizeof(double)*bm->nbox*bm->wcnz*bm->hd.dsize;
    bm->hd.nbuf = min(MAXBUFMEM/bytes, bm->hd.nstep);
    if( bm->hd.nbuf < 1 )
		bm->hd.nbuf = 1;

    /* Allocate memory for data buffers */
    bm->hd.tbuf = alloc1d(bm->hd.nbuf);
    bm->hd.ebuf = (double ****)alloc4d(bm->hd.dsize,bm->wcnz,bm->nbox,bm->hd.nbuf);

    bm->hd.bbuf = (int ****)i_alloc4d(bm->hd.dsize,bm->wcnz,bm->nbox,bm->hd.nbuf);
    bm->hd.kbuf = (int ****)i_alloc4d(bm->hd.dsize,bm->wcnz,bm->nbox,bm->hd.nbuf);

    /* Signify that buffers are empty */
    bm->hd.bufstart = -1;
    bm->hd.bufend = -1;
    bm->hd.nextrec = 0;

    /* Reset netCDF error handling */
    ncopts = NC_VERBOSE | NC_FATAL;
}

/** Routine to close an already open hydrodynamic input file */
void close_hydro(MSEBoxModel *bm)
{
	ncopts = NC_VERBOSE | NC_FATAL;
    /* Close file */
    if( bm->hd.fid >= 0 ){
    	ncclose(bm->hd.fid);
    }

    bm->hd.fid = -1;

    /* Free buffers */
    free1d(bm->hd.tbuf);
    free4d((double ****)bm->hd.ebuf);

    i_free4d(bm->hd.bbuf);
    i_free4d(bm->hd.kbuf);

    bm->hd.tbuf = NULL;
    bm->hd.ebuf = NULL;
    bm->hd.bbuf = NULL;
    bm->hd.kbuf = NULL;
}

/**
 * This function is called at the end of the model run to close the hydro files and
 * free the bm->hd.fname array.
 *
 */
void closeHydroFinal(MSEBoxModel *bm)
{
	close_hydro(bm);
	c_free2d(bm->hd.fname);

}
