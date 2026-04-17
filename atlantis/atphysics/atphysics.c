/*******************************************************************//**
 \file
 \brief Physical routines for general box model
 File:           atphysics.c

 Created:        Fri Nov 25 13:42:58 EST 1994

 Author:         Stephen Walker
 CSIRO Division of Oceanography

 Purpose:        Physical routines for general box model

 Arguments:      See each routine below

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

 24/4/2005 EA Fulton
 Added constrain_wc

 21-07-2009 Bec Gorton
 Changed the error generated when the psschange array in the input file is too small. A more meaningful error
 message is now displayed.

 24-07-2009 Bec Gorton
 Changed the code that reads in the pss_mult, pss_start, pss_period, pss_mult2, pss_start2, and pss_period2 code
 to read in arrays of length npss instead of reading in single values. This allows the user to scale each point source differently.

 28-10-2009 Bec Gorton
 Added code to close the salinity and temperature files a final time. This
 will also free up the array of file names.

 17-09-2009 Bec Gorton
 Added a check of the number of point sources before allocating arrays for the pss_mult, pss_period etc arrays.

 30-09-2009 Bec Gorton
 Added a bit of code to free up new arrays - leaks found by valgrind.

 19-04-2010 Bec Gorton
 Changed the code that read in the pss scaling to allow for each variable in each point source file to be scaled
 independently.
 Also removed the pss_mult2 etc variables and now allow any number of pss scales to be defined in the input file.

 The nutrient_change variable in the input file will now specify the number of changes that will take place,
 instead of it just being a flag.
 *********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sjwlib.h>
#include <ctype.h>
#include <atlantisboxmodel.h>
#include <atPhysics.h>
#include <atUtilLib.h>
#include <atEcologyLib.h>

/* Prototypes for physical routines */
void bioturb(MSEBoxModel *bm, double ***newwctr, double ***newsedtr);
void bioirrig(MSEBoxModel *bm, double ***newwctr, double ***newsedtr);
void check_neg(MSEBoxModel *bm, double ***val, char *msg);
void decayBM(MSEBoxModel *bm, double ***newwctr, double ***newsedtr);
void Eddy_Strength_Update(MSEBoxModel *bm);
void gasExchange(MSEBoxModel *bm, double ***newwctr);
void hdiffBMwc(MSEBoxModel *bm, double ***newwctr, FILE *llogfp);
void filler_hdiffBMwc(MSEBoxModel *bm, double ***newwctr, FILE *llogfp);
void resuspendBM(MSEBoxModel *bm, double ***newwctr, double ***newsedtr, FILE *llogfp);
void Saturation_Check(MSEBoxModel *bm, double ***newwctr, double ***newsedtr);
void settleBMwc(MSEBoxModel *bm, double ***newwctr, double ***newsedtr, FILE *llogfp);
void sourceSink(MSEBoxModel *bm, double ***newwctr, FILE *llogfp);
void solarIrradiance(MSEBoxModel *bm, double ***newwattr, FILE *llogfp);
void transportBM(MSEBoxModel *bm, double ***newwctr, FILE *llogfp);
void tracerForcingBM(MSEBoxModel *bm, double ***newwc, double ***newsedtr, PhyPropertyData *inputData);
void resetForcingBM(MSEBoxModel *bm, double ***newwc, double ***newsedtr, PhyPropertyData *inputData);
void vdiffBMwc(MSEBoxModel *bm, double ***newwctr);
void vertical_mixing(MSEBoxModel *bm, double ***newwctr);

void Get_Ice(MSEBoxModel *bm, FILE *llogfp);

/* Pointers to functions for various processes */
extern double (*decay)(double c, double k, double dt);

void freePhysics(MSEBoxModel *bm) {
	printf("Freeing physics specific arrays\n");

	freeSourceSink(bm);

	free2d(bm->eddy_seasonal);

	/* Close the hydro file */
	closeHydroFinal(bm);
	/* Close the temperature and salinity files */
	freeTempSalt(bm);

	free1d(bm->hc);
	free1d(bm->hx);
	free1d(bm->hk);
    i_free1d(bm->hdone);


	free_diffusion1d(1);

	freePhysicsStruct(bm);

}

void Read_PSS_Change(MSEBoxModel *bm, FILE *pfp) {
	int i, k, s;
	char str[200];
	int counter;

	//printf("Creating pSS change arrays\n");


	/* Read in information about each pss change */
	for (s = 0; s < bm->npss; s++) {
		counter = 0;
		keyprm_verbose = 1;

		/* Read the number of changes */
		snprintf(str, sizeof(str), "pss%d_numchanges", s);
		readkeyprm_i(pfp, str, &bm->pss[s].numPssChanges);

		if(bm->pss[s].numPssChanges > 0){
			//		fprintf(bm->logFile, "bm->pss[s].numPssChanges = %d\n", bm->pss[s].numPssChanges);
			//		fprintf(bm->logFile, "bm->pss[s].ts.nv = %d\n", bm->pss[s].ts.nv);

			/* Now allocate memory for each of these changes */
			bm->pss[s].pssChange = Util_Alloc_Init_2D_Int(bm->pss[s].ts.nv - 1, bm->pss[s].numPssChanges, 0);
			bm->pss[s].pssMult = Util_Alloc_Init_2D_Double(bm->pss[s].ts.nv - 1, bm->pss[s].numPssChanges, 0.0);
			bm->pss[s].pssStart = Util_Alloc_Init_2D_Int(bm->pss[s].ts.nv - 1, bm->pss[s].numPssChanges, 0);
			bm->pss[s].pssPeriod = Util_Alloc_Init_2D_Int(bm->pss[s].ts.nv - 1, bm->pss[s].numPssChanges, 0);

			for (i = 0; i < bm->pss[s].numPssChanges; i++) {

				/* Read the psschange list (pss - point source/sink) */

				snprintf(str, sizeof(str), "pss%d_change%d", s, i);
				readkeyprm_iarray_checked(pfp, integer_check, str, &bm->pss[s].pssChange[i], &counter, bm->pss[s].ts.nv - 1);

				/* Read the rest of the nutrient change parameters */
				snprintf(str, sizeof(str), "pss%d_mult%d", s, i);
				readkeyprm_darray_checked(pfp, no_checking, str, &bm->pss[s].pssMult[i], &counter, bm->pss[s].ts.nv - 1);

				for (k = 0; k < bm->pss[s].ts.nv - 1; k++) {
					//fprintf(bm->logFile, "bm->pss[s].pssMult[i][k] = %e\n", bm->pss[s].pssMult[i][k]);

					if (bm->pss[s].pssChange[i][k] && bm->pss[s].pssMult[i][k] < 0)
						quit("pss_mult values must be >0 for point sources that are being scaled.\n");
				}

				snprintf(str, sizeof(str), "pss%d_period%d", s, i);
				readkeyprm_iarray_checked(pfp, no_checking, str, &bm->pss[s].pssPeriod[i], &counter, bm->pss[s].ts.nv - 1);
				for (k = 0; k < bm->pss[s].ts.nv - 1; k++) {
					if (bm->pss[s].pssChange[i][k] && bm->pss[s].pssPeriod[i][k] <= 0)
						quit("pss_period values must be >0 for point sources that are being scaled.\n");
				}

				snprintf(str, sizeof(str), "pss%d_start%d", s, i);
				readkeyprm_iarray_checked(pfp, no_checking, str, &bm->pss[s].pssStart[i], &counter, bm->pss[s].ts.nv - 1);
			}
		}
	}
}
/*******************************************************************..**
 Physics initialisation routine. This routine should be called
 once, after the box model structure has been set up.
 *********************************************************************/
void initPhysics(MSEBoxModel *bm) {
	int b = 0;
	int k = 0;
	int i, this_nz;
	int counter = 0;
	static int called = 0;
    double base_cell_vol = 0.0;
	FILE *pfp;
	char buf[BMSLEN];
	char *reflect = "edge_type";
	char *eddy_S1 = "eddy_S1";
	char *eddy_S2 = "eddy_S2";
	char *eddy_S3 = "eddy_S3";
	char *eddy_S4 = "eddy_S4";
    char *ship_channels = "ship_channels";
    char *ship_mixing = "ship_mixing_rate";
    char *vessel_size = "vessel_size";
    char *dredge_path = "dredge_path";
    char *dscalar = "decay_scalar";
    char *lndreclaim = "land_reclaim";

	/* Create local vectors */
	double *bnd_type = alloc1d(bm->nbox);
	double *eddy_S = alloc1d(bm->nbox);
    double *ship_info = alloc1d(bm->K_num_vessel_sizes);
    double *dcay_scalar = alloc1d(bm->nbox);

	/* Create the physics module structure */
	allocatePhysicsModule(bm);

	/* Check that routine is only called once */
	if (called)
		quit("initPhysics: routine called multiple times\n");
	called = 1;

	/* Create arrays for horizontal diffusion */
	bm->max_nconn = 0;
	for (b = 0; b < bm->nbox; b++) {
		if (bm->boxes[b].nconn > bm->max_nconn)
			bm->max_nconn = bm->boxes[b].nconn;
	}
    bm->max_nconn += 1;  // For the box you start from

	bm->hc = alloc1d(bm->max_nconn);
	bm->hx = alloc1d(bm->max_nconn);
	bm->hk = alloc1d(bm->max_nconn);
    bm->hdone = i_alloc1d(bm->nbox);

	/* Open parameter file */
	if ((pfp = Open_Input_File(bm->inputFolder,bm->physprmIfname, "r")) == NULL)
		quit("initPhysics: can't open parameter file %s%s\n", bm->inputFolder, bm->physprmIfname);

	/* Set decay routine */
	decay = decay_exact;
    

	readkeyprm_i(pfp, "include_atmosphere", &bm->include_atmosphere);

	if(bm->include_atmosphere){
		/* Get atmospheric concentrations */
		readkeyprm_d(pfp, "atmospheric_NH", &bm->atmospheric_NH);
		readkeyprm_d(pfp, "atmospheric_NO", &bm->atmospheric_NO);
		readkeyprm_d(pfp, "atmospheric_F", &bm->atmospheric_F);
		readkeyprm_d(pfp, "atmospheric_O2", &bm->atmospheric_O2);
		readkeyprm_d(pfp, "atmospheric_CO2", &bm->atmospheric_CO2);
		readkeyprm_d(pfp, "atmospheric_P", &bm->atmospheric_P);
		readkeyprm_d(pfp, "atmospheric_Si", &bm->atmospheric_Si);
	}
    
    readkeyprm_i(pfp, "include_outgas", &bm->include_outgas);
    
    if (bm->include_outgas) {
        readkeyprm_d(pfp, "outgas_prop", &bm->outgas_prop);
        readkeyprm_d(pfp, "outgas_NH", &bm->outgas_NH);
        readkeyprm_d(pfp, "outgas_NO", &bm->outgas_NO);
        readkeyprm_d(pfp, "outgas_O2", &bm->outgas_O2);
        readkeyprm_d(pfp, "outgas_CO2", &bm->outgas_CO2);
        readkeyprm_d(pfp, "BGC_bound_scalar", &bm->BGC_bound_scalar);
    } else
        bm->BGC_bound_scalar = 1.0;
    
    readkeyprm_i(pfp, "mix_deep_O2", &bm->mix_deep_O2);

	/* Set vertical diffusion weights */
	readkeyprm_d(pfp, "vdiffwt_wc", &bm->a_wc);
	readkeyprm_d(pfp, "vdiffwt_sed", &bm->a_sed);

	/* Read and store water column vertical diffusion parameter */
	readkeyprm_d(pfp, "wc_kz", &bm->wc_kz);
	for (b = 0; b < bm->nbox; b++) {
		Box *bp = &bm->boxes[b];
		// Set the coefficent for each box layer.
		for (k = 1; k < bp->nz; k++)
			bp->kz[k] = bm->wc_kz;
	}
    for (k = 0; k < bm->max_nconn; k++)
        bm->hk[k] = bm->wc_kz;

    /* Read dz tolerance - water column dz values will be
	 * reset to nominal values whenever they go outside
	 * the range nominal_dz*(1 +- tol)
	 */
	readkeyprm_d(pfp, "wc_dz_tol", &bm->wc_dz_tol);
    if (bm->tidal & (bm->wc_dz_tol < 1.0))
        warn("Tidal model but you have not allowed for cells to completely empty or swell by more than 100%\n");

    /* Read sediment related parameters if necessary */
	if (bm->sednz > 0) {
        /* Read in flag as to whether maintain sediment layer thickness or not */
        readkeyprm_i(pfp, "maintain_sedlayer", &bm->maintain_sedlayer);
        if(bm->maintain_sedlayer && (bm->sednz > 1))
            quit("Should only really use maintain_sedlayer if there is only 1 sediment layer. You have %d layers so please set maintain_sedlayer to 0 in force.prm\n", bm->sednz);
        
		/* Read maximum and minimum sediment layer thickness */
		readkeyprm_d(pfp, "maxseddz", &bm->maxseddz);
		readkeyprm_d(pfp, "minseddz", &bm->minseddz);
        readkeyprm_d(pfp, "stress_silt_thresh", &bm->stress_silt_thresh);

		for (b = 0; b < bm->nbox; b++) {
			bm->boxes[b].sm.maxdz = bm->maxseddz;
			bm->boxes[b].sm.mindz = bm->minseddz;
		}
		readkeyprm_d(pfp, "max_erosion", &bm->max_erosion);
        
        if(bm->track_rugosity_arag)
            readkeyprm_d(pfp, "wgt_georugosity", &bm->wgt_georugosity);

		/* Read bio-irrigation rates */
		readkeyprm_d(pfp, "bi_dissol_kz", &bm->bi_dissol_kz);
		readkeyprm_d(pfp, "bi_exchange", &bm->bi_exchange);
		readkeyprm_d(pfp, "bi_injection", &bm->bi_injection);

		/* Read bio-turbation rates */
		readkeyprm_d(pfp, "bt_partic_kz", &bm->bt_partic_kz);
		readkeyprm_d(pfp, "bt_exchange", &bm->bt_exchange);
		readkeyprm_d(pfp, "bt_expulsion", &bm->bt_expulsion);

		/* Read functional form of depth dependence of
		 * bio-irrigation and bio-turbation processes
		 */
		readkeyprm_s(pfp, "biosedprofile", buf);
		bm->biosedprofile = (char)tolower((int)buf[0]);
		//TODO: Put in error checking here.

		/* Read functional form of depth dependence of
		 * oxygen distribution
		 */
		readkeyprm_s(pfp, "biooxprofile", buf);
		bm->biooxprofile = (char)tolower((int)buf[0]);
		//TODO: Put in error checking here.

        /* Read switch indicating whether or not to use bioturbation model or not */
        readkeyprm_i(pfp, "bioirrigation", &bm->bioirrigation);
        
        /* Read switch indicating whether or not to use bioturbation model */
        readkeyprm_i(pfp, "bioturbation", &bm->bioturbation);
        
        /* Read switch indicating whether or not to use resuspension model */
        readkeyprm_i(pfp, "resuspension", &bm->resuspension);
        
        /* Read switch indicating whether or not to use dynamic stress calculations */
        readkeyprm_i(pfp, "dynamic_stress", &bm->dynamic_stress);
        

        /* Calculate initial sediment parameters such as
         * porosity and critical shear stress.
         */
        for (b = 0; b < bm->nbox; b++) {
            // Get the index of the top layer
            k = bm->boxes[b].sm.topk;
            
            //TODO: Fix strange code.
            if (bm->boxes[b].sm.porosity[k] > 0.0)
                k = 0;
            else
                sedprops(bm);
        }
        
        /* Read switch indicating whether or not to use decay model in sediments */
        readkeyprm_i(pfp, "decay_sed", &bm->decay_sed);
        
        /* Read scalar for decay in sediments */
        readkeyprm_d(pfp, "decay_sed_scale", &bm->decay_sed_scale);
        
    }
    
    /* Read switch indicating whether or not to use decay model in water column */
    readkeyprm_i(pfp, "decay_wc", &bm->decay_wc);
		
    /* Read coefficient used to set baseline temperature for the model
     */
    readkeyprm_d(pfp, "baseline_temp", &bm->baseline_temp);

    /* Read coefficient used to set amplitude of temperature variation for the model
     */
    readkeyprm_d(pfp, "temp_ampltiude", &bm->temp_amplitude);

    /* Read switch indicating whether or not to constrain water depth
     * to a min value of 1m.
     */
    //TODO: Add error checking for flag value. Should use the same function for all
    // switch inputs
    readkeyprm_i(pfp, "constrain_wc", &bm->constrain_wc);

    /* Read vertical upwelling mixing rate */
    readkeyprm_d(pfp, "mix_injection", &bm->mix_injection);

    /* Read seasonal vertical upwelling mixing coefficient */
    readkeyprm_d(pfp, "mix_season_kz", &bm->mix_season_kz);

    /* Read switch indicating whether or not deep ocean mixing occurs */
    readkeyprm_i(pfp, "mix_deep", &bm->mix_deep);

    /* Read depth for deep ocean mixing */
    readkeyprm_d(pfp, "mix_deep_depth", &bm->mix_deep_depth);

    /* Read switch indicating whether or not to use point sources */
    readkeyprm_i(pfp, "injection", &bm->injection);

    /* Read switch indicating whether or not to use atmospheric exchange model */
    readkeyprm_i(pfp, "atmospherics", &bm->atmospherics);

    /* Read switch indicating whether or not to use settling model */
    readkeyprm_i(pfp, "settling", &bm->settling);

    /* Read switch indicating whether or not to use horizontal diffusion model */
    readkeyprm_i(pfp, "horiz_diffusion", &bm->horiz_diffusion);

    /* Read switch indicating whether or not to use vertical diffusion model */
    readkeyprm_i(pfp, "vert_diffusion", &bm->vert_diffusion);

    /* Read switch indicating whether or not to use forced vertical mixing model */
    readkeyprm_i(pfp, "vert_mix", &bm->vert_mix);

    /* Read switch indicating whether or not to use transport model */
    readkeyprm_i(pfp, "advect_diffusion", &bm->advect_diffusion);
        
    /* Read switch indicating whether or not to use fill_zero_exchange
     flag and associated triggering threshold */
    readkeyprm_i(pfp, "fill_zero_exchange", &bm->fill_zero_exchange);
    if(bm->fill_zero_exchange) {
        readkeyprm_i(pfp, "use_fill_horizmix", &bm->use_fill_horizmix);
        readkeyprm_d(pfp, "flush_threshold", &bm->flush_threshold);
    }

    /* Read switch indicating whether or not to scale exchanges */
    readkeyprm_i(pfp, "scale_transport", &bm->scale_transport);

    /* Read coefficient used in constant scaling of exchanges */
    readkeyprm_d(pfp, "prcnt_exchange", &bm->prcnt_exchange);

    /* Read coefficient used in area corrected scaling of exchanges */
    readkeyprm_d(pfp, "ka_exchange", &bm->ka_exchange);

    /* Read switch indicating whether or not flows (exchanges) can cascade down slopes */
    readkeyprm_i(pfp, "cascade_flows", &bm->cascade_flows);

    /* Read in switches identifying boundaries as standard, absorptive or reflective */
    counter = bm->nbox;
    readkeyprm_darray(pfp, reflect, &bnd_type, &counter);
    for (b = 0; b < bm->nbox; b++) {
        bm->boxes[b].edge_type = (int) (bnd_type[b]);
    }
    
    /* Read in box specific decay scalar */
    readkeyprm_darray(pfp, dscalar, &dcay_scalar, &counter);
    for (b = 0; b < bm->nbox; b++) {
        bm->boxes[b].decay_scalar = dcay_scalar[b];
    }

    /* Get seasonal eddy info */
    bm->eddy_seasonal = (double **) alloc2d(4, bm->nbox);
    readkeyprm_darray(pfp, eddy_S1, &eddy_S, &counter);
    for (b = 0; b < bm->nbox; b++) {
        bm->eddy_seasonal[b][0] = eddy_S[b];
    }
    readkeyprm_darray(pfp, eddy_S2, &eddy_S, &counter);
    for (b = 0; b < bm->nbox; b++) {
        bm->eddy_seasonal[b][1] = eddy_S[b];
    }
    readkeyprm_darray(pfp, eddy_S3, &eddy_S, &counter);
    for (b = 0; b < bm->nbox; b++) {
        bm->eddy_seasonal[b][2] = eddy_S[b];
    }
    readkeyprm_darray(pfp, eddy_S4, &eddy_S, &counter);
    for (b = 0; b < bm->nbox; b++) {
        bm->eddy_seasonal[b][3] = eddy_S[b];
    }

    /* Read coefficient used in scaling of vertical exchanges by eddies */
    readkeyprm_d(pfp, "eddy_mixscale", &bm->eddy_mixscale);

    /* Number of gradial nutrient changes that will take place. */
    readkeyprm_i(pfp, "nutrientchange", &bm->nutrientChange);
	readkeyprm_i(pfp, "pulsechange", &bm->pulsechange);

    if (bm->nutrientChange) {
        Read_PSS_Change(bm, pfp);
    }

    /* Catastrophic events parameters */
    readkeyprm_i(pfp, "flag_catastrophe", &bm->flag_catastrophe);    
    if ( bm->flag_catastrophe ) {
        readkeyprm_d(pfp, "catastrophic_duration", &bm->catastrophic_duration);
    }
    
    /* Read in simple industries forcing - really only shipping for now */
    if (bm->flagindustry_on == simple_industry_model) {
        
        /* Read in location and type (small, large etc) of shipping channels */
        counter = bm->nbox;
        readkeyprm_darray(pfp, ship_channels, &bnd_type, &counter);
        for (b = 0; b < bm->nbox; b++) {
            bm->boxes[b].channel_type = (int) (bnd_type[b]);
        }
        //Setup_Channel_info(bm);
        
        /* Dredge time steps */
        readkeyprm_i(pfp, "dredge_dt", &bm->dredge_dt);
        
        /* Now find max dredge period to cover the path */
        counter = bm->nbox;
        readkeyprm_darray(pfp, dredge_path, &bnd_type, &counter);
        bm->dredge_period = 0;
        for (b = 0; b < bm->nbox; b++) {
            bm->boxes[b].dredge_path = (int) (bnd_type[b]);
            if (bm->dredge_period < (bm->boxes[b].dredge_path * bm->dredge_dt )) {
                bm->dredge_period = bm->boxes[b].dredge_path * bm->dredge_dt;
            }
        }
        bm->dredge_started = -1;
        
        /* Read in rate of excavation of dredge */
        readkeyprm_d(pfp, "dredge_rate", &bm->dredge_rate);

        /* Read in area excavated each day dredged */
        readkeyprm_d(pfp, "dredge_area", &bm->dredge_area);
        
        /* Read in mass per cm2 of dredge material */
        readkeyprm_d(pfp, "dredge_fines_mass", &bm->dredge_fines_mass);

        /* Read in particle size of dredge material */
        readkeyprm_d(pfp, "dredge_fines_psize", &bm->dredge_fines_psize);
        bm->entrained_psize = bm->dredge_fines_psize;
        
        /* Read in smotehr decay coefficient */
        readkeyprm_d(pfp, "smother_coefft", &bm->smother_coefft);

        /* Read in smotehr decay coefficient */
        readkeyprm_d(pfp, "smother_thresh", &bm->smother_thresh);
        
        for (b = 0; b < bm->nbox; b++) {
            bm->boxes[b].land.ship_transits = Util_Alloc_Init_1D_Int(bm->K_num_vessel_sizes, 0);
            bm->boxes[b].land.new_ship_num = Util_Alloc_Init_1D_Int(bm->K_num_vessel_sizes, 0);
            bm->boxes[b].land.ship_num = Util_Alloc_Init_1D_Int(bm->K_num_vessel_sizes, 0);
        }
        
        bm->ship_id = Util_Alloc_Init_1D_Int(bm->K_num_vessel_sizes, 0);
        //Setup_Ship_IDs(bm, bm->ships); // Sort time series ids

        bm->ship_mixing_rate = Util_Alloc_Init_1D_Double(bm->K_num_vessel_sizes, 0);
        bm->vessel_size = Util_Alloc_Init_1D_Double(bm->K_num_vessel_sizes, 0);
        
        counter = bm->K_num_vessel_sizes;
        readkeyprm_darray(pfp, ship_mixing, &ship_info, &counter);
        for (b = 0; b < bm->K_num_vessel_sizes; b++) {
            bm->ship_mixing_rate[b] = (int) (ship_info[b]);
        }
        counter = bm->K_num_vessel_sizes;
        readkeyprm_darray(pfp, vessel_size, &ship_info, &counter);
        for (b = 0; b < bm->K_num_vessel_sizes; b++) {
            bm->vessel_size[b] = (int) (ship_info[b]);
        }
        
        /* Read in ship strike mortality scalar - weighting on likelihood of ship strike causing mortality */
        readkeyprm_d(pfp, "strike_mort_scalar", &bm->strike_mort_scalar);

        counter = bm->nbox;
        readkeyprm_darray(pfp, ship_mixing, &ship_info, &counter);
        for (b = 0; b < bm->K_num_vessel_sizes; b++) {
            bm->ship_mixing_rate[b] = (int) (ship_info[b]);
        }

    }
    
    bm->flag_reclaim_land = FALSE;
    if(bm->terrestrial_on) {
        /* Read in land reclamation */
        readkeyprm_darray(pfp, lndreclaim, &dcay_scalar, &counter);
        for (b = 0; b < bm->nbox; b++) {
            bm->boxes[b].land.day_reclaim = dcay_scalar[b];
            
            if (lndreclaim[b] < bm->tstop)
                bm->flag_reclaim_land = TRUE;
        }
        
        /* Read in reclaimed land height */
        readkeyprm_d(pfp, "reclaim_land_dz", &bm->reclaim_land_dz);
       

    }

	/* For nutrients set initial bottom values so can have remixing */

	//TODO: Re-write so that you find the index of the tracer first and
	// then loop through each box.
    base_cell_vol = 0.0;
	for (b = 0; b < bm->nbox; b++) {
	    bm->boxes[b].land.ship_mix = 1.0;  // So initialised even if no ships present
		for (i = 0; i < bm->ntracer; i++) {

			if (bm->mix_deep && (bm->mix_deep_depth > bm->boxes[b].botz)) {
				if (!strcmp(bm->tinfo[i].name, "NH3"))
					bm->boxes[b].bottNH = bm->boxes[b].tr[0][i];
				else if (!strcmp(bm->tinfo[i].name, "NO3"))
					bm->boxes[b].bottNO = bm->boxes[b].tr[0][i];
                else if ((!strcmp(bm->tinfo[i].name, "Oxygen")) && bm->mix_deep_O2)
                    bm->boxes[b].bottO2 = bm->boxes[b].tr[0][i];
                else if (!strcmp(bm->tinfo[i].name, "Si"))
					bm->boxes[b].bottSi = bm->boxes[b].tr[0][i];
				else if (!strcmp(bm->tinfo[i].name, "MicroNut"))
					bm->boxes[b].bottFe = bm->boxes[b].tr[0][i];
				else if (bm->track_atomic_ratio == TRUE && strcmp(bm->tinfo[i].name, "Phosphorus") == 0)
					bm->boxes[b].bottP = bm->boxes[b].tr[0][i];
				else if (bm->track_atomic_ratio == TRUE && strcmp(bm->tinfo[i].name, "Carbon") == 0)
					bm->boxes[b].bottC = bm->boxes[b].tr[0][i];
			}
		}

        this_nz = bm->boxes[b].nz - 1;
        base_cell_vol += (bm->boxes[b].area * bm->boxes[b].dz[this_nz]);
	}
    bm->avg_cell_vol = base_cell_vol / ((double)bm->nbox);

	/* Ice related info */
	//readkeyprm_i(pfp, "ice_on", &bm->ice_on);  Removed as now set in tracerinfo read if it finds icenz in the in.nc file

	if(bm->ice_on){
		//readkeyprm_i(pfp, "icenz", &bm->icenz);
		readkeyprm_i(pfp, "maxicedz", &bm->maxicedz);
		readkeyprm_i(pfp, "minicedz", &bm->minicedz);
		readkeyprm_i(pfp, "num_ice_classes", &bm->K_num_ice_classes);
		readkeyprm_i(pfp, "slush", &bm->slush_id);
		readkeyprm_i(pfp, "kind_ice_model", (int *) &bm->kind_ice_model);


		// Set up ice info
		for (b = 0; b < bm->nbox; b++) {
			bm->boxes[b].ice.max_ice_depth = 0;
			bm->boxes[b].ice.ice_growth_rate = 0;
			bm->boxes[b].ice.is_freezing = 0;
			bm->boxes[b].ice.n_ice_types = bm->K_num_ice_classes;
			bm->boxes[b].ice.maxdz = bm->maxicedz;
			bm->boxes[b].ice.mindz = bm->minicedz;
			bm->boxes[b].ice_prop = Util_Alloc_Init_2D_Double(bm->K_num_ice_classes, bm->wcnz, 0.0);
			bm->boxes[b].ice.wc_layer_match = Util_Alloc_Init_1D_Int(bm->icenz, 0);
			bm->boxes[b].ice.last_depth = 0.0;
			bm->boxes[b].has_ice = 0;
			bm->boxes[b].ice.ice_classes = Util_Alloc_Init_2D_Double(bm->K_num_ice_classes, bm->icenz, 0.0);

			bm->boxes[b].ice.fast_ice = FALSE;
		}

		Load_Ice_Timeseries(bm);
		Get_Ice(bm, bm->logFile);
	} else {
		for (b = 0; b < bm->nbox; b++) {
			bm->boxes[b].ice_prop = NULL;
			bm->boxes[b].has_ice = 0;
		}
	}

	/* Free local vectors */
	free1d(bnd_type);
	free1d(eddy_S);
    free1d(ship_info);
    free1d(dcay_scalar);

	/* close parameter file */
	fclose(pfp);
}

/*******************************************************************//**
 Routine to implement physical processes for 1 timestep
 *********************************************************************/
void physics(MSEBoxModel *bm, double ***newwctr, double ***newsedtr, FILE *llogfp) {  // Removed "double ***newicetr, double **newlandtr," for now as not used here and tripping compiler. Put it back in once actually need it

	int tracerIndex;
    
    /* If using simple industry models apply effects now *
    if (bm->flagindustry_on == simple_industry_model) {
        Do_Simple_Shipping(bm, newwctr, newsedtr);
        Do_Simple_Dredging(bm, newwctr, newsedtr);
    }
    */
    
    /* Do any land reclamation *
    if (bm->flag_reclaim_land) {
        Do_Land_Reclaimation(bm, newwctr, newsedtr);
    }
    */
    
	/* Add source/sink terms for this time */
	if (bm->injection == 1)
		sourceSink(bm, newwctr, llogfp);

    /* Handle light from input forcing files*/
    solarIrradiance(bm, newwctr, llogfp);
    
    /* Gas exchange with atmosphere */
    if (bm->atmospherics == 1) {
        gasExchange(bm, newwctr);
    }
    
    /* Decay in water column and sediments */
    if (bm->decay_wc == 1 || bm->decay_sed == 1) {
        decayBM(bm, newwctr, newsedtr);
    }

    if (!_finite(newwctr[24][0][Salinity_i])) {
        printf("Time: %e Salinity in bottom water is nan - E\n", bm->dayt);
    }

    /* Resuspension, sediments.
	 * This should be done before settling so that coarse
	 * particles which are resuspended immediately fall
	 * out again and are not transported elsewhere in
	 * the model by the transport process below
	 */
    if (bm->resuspension == 1) {
        resuspendBM(bm, newwctr, newsedtr, llogfp);
    }

    /* Settling, water column. This routine
	 * calls the deposit() routine to
	 * add material to the sediments.
	 */
    if (bm->settling == 1) {
        settleBMwc(bm, newwctr, newsedtr, llogfp);
    }
     
    /* FIX - Compaction, sediments */

	/* Bioirrigation */
    if (bm->bioirrigation == 1) {
        bioirrig(bm, newwctr, newsedtr);
    }
    
    /* Bioturbation - only do it if switched on */
    if (bm->bioturbation == 1){
        bioturb(bm, newwctr, newsedtr);
    }
    
    /* FIX - There may be other non-biological processes,
	 * such as pressure gradient driven flows, which cause
	 * diffusion and exchange - see Ian Websters summary
	 * paper
	 */

	/* Horizontal diffusion - this should really be covered
	 * in the transport model, but if all else fails in fine
	 * estuarine boxes use this rather than panic.
	 */
    if (bm->horiz_diffusion == 1) {
        hdiffBMwc(bm, newwctr, llogfp);
    }
    
    /* Horizontal diffusion to fill holes in transport models. */
    if (bm->fill_zero_exchange == 1) {
        filler_hdiffBMwc(bm, newwctr, llogfp);
    }
    
    /* Vertical diffusion within water column.
	 * This process should be done after all other non-forced
	 * vertical processes, so that vertical gradients
	 * introduced by sources and sinks (or other things)
	 * have a chance to be smeared out before the next
	 * time step.
	 */
    if (bm->vert_diffusion == 1) {
        vdiffBMwc(bm, newwctr);
    }
    
    /* Vertical mixing. This is forced mixing to compensate for
	 * the lack of vertical mixing and exchange in some sets
	 * of calculated exchanges in multilayed model geometries.
	 * Allows for upwelling even when only explicitly
	 * have horizontal transport information */
    if (bm->vert_mix == 1){
        if (bm->use_VertMixfiles) {
             tracerForcingBM(bm, newwctr, newsedtr, &(bm->VertMixinput));
        }
		vertical_mixing(bm, newwctr);
    }

    /* Transport, water column.
	 * This should be the last of the explicit physics processes
	 * to be done.
	 */

    if (bm->advect_diffusion == 1) {
        transportBM(bm, newwctr, llogfp);
    }

    /* Temperature forcing in the water column.
	 */
	if (bm->use_tempfiles){
		tracerForcingBM(bm, newwctr,newsedtr,  &(bm->tempinput));
	}
    
    /* Salinity forcing in the water column.
	 */
	if (bm->use_saltfiles){
		tracerForcingBM(bm, newwctr, newsedtr, &(bm->saltinput));
	}
    
    /* pH forcing */
	if(bm->use_pHfiles){
		tracerForcingBM(bm, newwctr, newsedtr, &(bm->pHinput));
	}
    
    /* Pollutant forcing for light and noise */
    if(bm->use_pollutantfiles){
        tracerForcingBM(bm, newwctr, newsedtr, &(bm->noiseinput));
        tracerForcingBM(bm, newwctr, newsedtr, &(bm->lightpinput));
    }
    
    /* Force other tracers */
	if (bm->use_forceTracers) {
		for (tracerIndex = 0; tracerIndex < bm->numForceTracers; tracerIndex++) {
            if (!bm->forceTracerInput[tracerIndex].use_resets[bm->forceTracerInput[tracerIndex].curFile]) {
                tracerForcingBM(bm, newwctr, newsedtr, &(bm->forceTracerInput[tracerIndex]));
            } else {
                resetForcingBM(bm, newwctr, newsedtr, &(bm->forceTracerInput[tracerIndex]));
            }
		}
	}

    /* Apply the scalars */
	Ecology_Apply_Environ_Scalars(bm, newwctr, newsedtr);
    
    /* Update eddy values */
    if(bm->use_eddy_NC == FALSE) {
        Eddy_Strength_Update(bm);
    }
    
    /* Saturation check - make sure everything
	 is still in whack before continuing */
	Saturation_Check(bm, newwctr, newsedtr);

    /* Ice state check */
	if(bm->ice_on){
		Get_Ice(bm, llogfp);
	}

    /* Update the next time to write out the inputs or exports file - do it once both files are written out.*/
	if (bm->t >= bm->inputs_toutNext){
		bm->inputs_toutNext = bm->t + bm->inputs_tout;
	}

    return;
}

void check_vals(MSEBoxModel *bm, double ***val, int nz, char *msg) {
	int b, k, n;

	/*
	 fprintf(stderr,"check_neg start: %s\n",msg);
	 */
	for (b = 0; b < bm->nbox; b++) {
		for (k = 0; k < nz; k++) {
			for (n = 0; n < bm->ntracer; n++) {
				if (isnan(val[b][k][n]))
					fprintf(stderr, "%s NaN (%e) b=%d k=%d, %s\n", bm->tinfo[n].name, val[b][k][n], b, k, msg);

				if (strcmp(bm->tinfo[n].name, "Oxygen") == 0)
					continue;
				if (strcmp(bm->tinfo[n].name, "X2") == 0)
					continue;

				if (val[b][k][n] < 0)
					fprintf(stderr, "%s negative (%e) b=%d k=%d, %s\n", bm->tinfo[n].name, val[b][k][n], b, k, msg);
			}
		}
	}
}

/*********************************************************************//**
 Routine to determine current strength of seasonally changing eddies
 */
void Eddy_Strength_Update(MSEBoxModel *bm) {
	int b = 0;
	int qrt = bm->QofY;

	for (b = 0; b < bm->nbox; b++) {
		if (qrt < 3)
			bm->boxes[b].eddy = bm->HowFar * (bm->eddy_seasonal[b][qrt + 1] - bm->eddy_seasonal[b][qrt]) + bm->eddy_seasonal[b][qrt];
		else
			bm->boxes[b].eddy = bm->HowFar * (bm->eddy_seasonal[b][0] - bm->eddy_seasonal[b][qrt]) + bm->eddy_seasonal[b][qrt];

	}
	return;
}



/*********************************************************************//**
Routines to do sanity check on tracers after fluxes 
*
void Tracer_Sanity_Check(MSEBoxModel *bm, double ***newwc, double ***newsed) {
    int b, n, k ;
    
    for (b = 0; b < bm->nbox; b++) {
        
        // Water column tracers
        for (n = 0; n < bm->ntracer; n++) {
            if (bm->tinfo[n].inwc) {
                for (k = 0; k < bm->boxes[b].nz; k++) {
                    if (newwc[b][k][n] < 0.0) {
                        newwc[b][k][n] = bm->min_pool;
                    }
                }
            }
        
        
            if( bm->tinfo[n].insed ) {
                for (k = 0; k < bm->boxes[b].sm.nz; k++) {
                    if (newsed[b][k][n] < 0.0) {
                        newsed[b][k][n] = bm->min_pool;
                    }
                }
            }
        }
        
    }
    
    return;
}
*/
/*********************************************************************//**
Routines to spit out value of specific group (or DIN if 
which_check == bm->K_num_tot_sp) to help track conservation of mass
 *
void ConservationMatterCheck(MSEBoxModel *bm, int spotid) {
	int ij, n, k, sn, rn, den, pid;
    double biomass = 0.0, NH3biom = 0.0, NO3biom = 0.0, DONbiom = 0.0;
    
    // Set the thing to check
    int sp = bm->which_check;
    
    if (!bm->masscheck)
        return;
    
    
    // Get toal value for the variable
    
	for (ij = 0; ij < bm->nbox; ij++) {
        
        // Water column contributions
        for (k = 0; k < bm->boxes[ij].nz; k++) {
            if (sp < bm->K_num_tot_sp){
                if(FunctGroupArray[sp].speciesParams[flag_id]){
                    for (n = 0; n < FunctGroupArray[sp].numCohorts; n++) {
                        switch (FunctGroupArray[sp].groupAgeType) {
                            case AGE_STRUCTURED:
                                sn = FunctGroupArray[sp].structNTracers[n];
                                rn = FunctGroupArray[sp].resNTracers[n];
                                den = FunctGroupArray[sp].NumsTracers[n];
                                biomass += (bm->boxes[ij].tr[k][sn] + bm->boxes[ij].tr[k][rn]) * bm->boxes[ij].tr[k][den] * FunctGroupArray[sp].habitatCoeffs[WC];
                                break;
                            case BIOMASS:				// Intentional follow through
                            case AGE_STRUCTURED_BIOMASS:
                                pid = FunctGroupArray[sp].totNTracers[n];
                                biomass += bm->boxes[ij].tr[k][pid] * bm->boxes[ij].dz[k] * bm->boxes[ij].area * FunctGroupArray[sp].habitatCoeffs[WC];
                                break;
                        }
                    }
                }
            } else {
                pid = NH3_i;
                NH3biom += bm->boxes[ij].tr[k][pid] * bm->boxes[ij].dz[k] * bm->boxes[ij].area;
                biomass += bm->boxes[ij].tr[k][pid] * bm->boxes[ij].dz[k] * bm->boxes[ij].area;
                pid = NO3_i;
                NH3biom += bm->boxes[ij].tr[k][pid] * bm->boxes[ij].dz[k] * bm->boxes[ij].area;
                biomass += bm->boxes[ij].tr[k][pid] * bm->boxes[ij].dz[k] * bm->boxes[ij].area;
                pid = DON_i;
                NH3biom += bm->boxes[ij].tr[k][pid] * bm->boxes[ij].dz[k] * bm->boxes[ij].area;
                biomass += bm->boxes[ij].tr[k][pid] * bm->boxes[ij].dz[k] * bm->boxes[ij].area;
            }
        }
        
        // Sediment contributions
        if (bm->boxes[ij].botz >= bm->maxwcbotz) {
            for (k = 0; k < bm->boxes[ij].sm.nz; k++) {
                if (sp < bm->K_num_tot_sp){
                    if(FunctGroupArray[sp].speciesParams[flag_id]){
                        for (n = 0; n < FunctGroupArray[sp].numCohorts; n++) {
                            switch (FunctGroupArray[sp].groupAgeType) {
                                case AGE_STRUCTURED:        // Nothng to do as yet
                                    break;
                                case BIOMASS:				// Intentional follow through
                                case AGE_STRUCTURED_BIOMASS:
 									pid = FunctGroupArray[sp].totNTracers[n];
									biomass += bm->boxes[ij].sm.tr[k][pid] * bm->boxes[ij].sm.dz[k] * bm->boxes[ij].area;
                                    break;
                            }
                        }
                    }
                } else {
                    pid = NH3_i;
                    NH3biom += bm->boxes[ij].sm.tr[k][pid] * bm->boxes[ij].sm.dz[k] * bm->boxes[ij].area;
                    biomass += bm->boxes[ij].sm.tr[k][pid] * bm->boxes[ij].sm.dz[k] * bm->boxes[ij].area;
                    pid = NO3_i;
                    NO3biom += bm->boxes[ij].sm.tr[k][pid] * bm->boxes[ij].sm.dz[k] * bm->boxes[ij].area;
                    biomass += bm->boxes[ij].sm.tr[k][pid] * bm->boxes[ij].sm.dz[k] * bm->boxes[ij].area;
                    pid = DON_i;
                    DONbiom += bm->boxes[ij].sm.tr[k][pid] * bm->boxes[ij].sm.dz[k] * bm->boxes[ij].area;
                    biomass += bm->boxes[ij].sm.tr[k][pid] * bm->boxes[ij].sm.dz[k] * bm->boxes[ij].area;
                }
            }
        }
        
        // Epibenthic contributions
        if ((sp < bm->K_num_tot_sp) && (FunctGroupArray[sp].habitatType == EPIFAUNA) && (FunctGroupArray[sp].speciesParams[flag_id])){
            for (n = 0; n < FunctGroupArray[sp].numCohorts; n++) {
                switch (FunctGroupArray[sp].groupAgeType) {
                    case AGE_STRUCTURED:        // Nothng to do as yet
                        break;
                    case BIOMASS:				// Intentional follow through
                    case AGE_STRUCTURED_BIOMASS:
                        pid = FunctGroupArray[sp].totNTracers[n];
                        // Calculate the biomass of this group in this cell
                        biomass = bm->boxes[ij].epi[pid] * bm->boxes[ij].area;
                        break;
                }
            }
            
        }
        
    }
    
    // Output results
    fprintf(bm->logFile, "Time: %e spot %d has biomass: %f (and if relevant NH3biom: %f NO3biom: %f DONbiom: %f\n",
            bm->dayt, spotid, biomass, NH3biom, NO3biom, DONbiom);
    
	return;
}

void NewMatterCheck(MSEBoxModel *bm, double ***newwctr, double ***newsedtr, int spotid) {
	int ij, n, k, sn, rn, den, pid;
    double biomass = 0.0, NH3biom = 0.0, NO3biom = 0.0, DONbiom = 0.0;
    
    // Set the thing to check
    int sp = bm->which_check;
    
    if (!bm->masscheck)
        return;
    
    // Get toal value for the variable
    
	for (ij = 0; ij < bm->nbox; ij++) {
        
        // Water column contributions
        for (k = 0; k < bm->boxes[ij].nz; k++) {
            if (sp < bm->K_num_tot_sp){
                if(FunctGroupArray[sp].speciesParams[flag_id]){
                    for (n = 0; n < FunctGroupArray[sp].numCohorts; n++) {
                        switch (FunctGroupArray[sp].groupAgeType) {
                            case AGE_STRUCTURED:
                                sn = FunctGroupArray[sp].structNTracers[n];
                                rn = FunctGroupArray[sp].resNTracers[n];
                                den = FunctGroupArray[sp].NumsTracers[n];
                                biomass += (newwctr[ij][k][sn] + newwctr[ij][k][rn]) * newwctr[ij][k][den] * FunctGroupArray[sp].habitatCoeffs[WC];
                                break;
                            case BIOMASS:				// Intentional follow through
                            case AGE_STRUCTURED_BIOMASS:
                                pid = FunctGroupArray[sp].totNTracers[n];
                                biomass += newwctr[ij][k][pid] * bm->boxes[ij].dz[k] * bm->boxes[ij].area * FunctGroupArray[sp].habitatCoeffs[WC];
                                break;
                        }
                    }
                }
            } else {
                pid = NH3_i;
                NH3biom += newwctr[ij][k][pid] * bm->boxes[ij].dz[k] * bm->boxes[ij].area;
                biomass += newwctr[ij][k][pid] * bm->boxes[ij].dz[k] * bm->boxes[ij].area;
                pid = NO3_i;
                NH3biom += newwctr[ij][k][pid] * bm->boxes[ij].dz[k] * bm->boxes[ij].area;
                biomass += newwctr[ij][k][pid] * bm->boxes[ij].dz[k] * bm->boxes[ij].area;
                pid = DON_i;
                NH3biom += newwctr[ij][k][pid] * bm->boxes[ij].dz[k] * bm->boxes[ij].area;
                biomass += newwctr[ij][k][pid] * bm->boxes[ij].dz[k] * bm->boxes[ij].area;
            }
        }
        
        // Sediment contributions
        if (bm->boxes[ij].botz >= bm->maxwcbotz) {
            for (k = 0; k < bm->boxes[ij].sm.nz; k++) {
                if (sp < bm->K_num_tot_sp){
                    if(FunctGroupArray[sp].speciesParams[flag_id]){
                        for (n = 0; n < FunctGroupArray[sp].numCohorts; n++) {
                            switch (FunctGroupArray[sp].groupAgeType) {
                                case AGE_STRUCTURED:        // Nothng to do as yet
                                    break;
                                case BIOMASS:				// Intentional follow through
                                case AGE_STRUCTURED_BIOMASS:
 									pid = FunctGroupArray[sp].totNTracers[n];
									biomass += newsedtr[ij][k][pid] * bm->boxes[ij].sm.dz[k] * bm->boxes[ij].area;
                                    break;
                            }
                        }
                    }
                } else {
                    pid = NH3_i;
                    NH3biom += newsedtr[ij][k][pid] * bm->boxes[ij].sm.dz[k] * bm->boxes[ij].area;
                    biomass += newsedtr[ij][k][pid] * bm->boxes[ij].sm.dz[k] * bm->boxes[ij].area;
                    pid = NO3_i;
                    NO3biom += newsedtr[ij][k][pid] * bm->boxes[ij].sm.dz[k] * bm->boxes[ij].area;
                    biomass += newsedtr[ij][k][pid] * bm->boxes[ij].sm.dz[k] * bm->boxes[ij].area;
                    pid = DON_i;
                    DONbiom += newsedtr[ij][k][pid] * bm->boxes[ij].sm.dz[k] * bm->boxes[ij].area;
                    biomass += newsedtr[ij][k][pid] * bm->boxes[ij].sm.dz[k] * bm->boxes[ij].area;
                }
            }
        }
        
        // Epibenthic contributions
        
    }
    
    // Output results
    fprintf(bm->logFile, "Time: %e spot %d has newtr biomass: %f (and if relevant NH3biom: %f NO3biom: %f DONbiom: %f\n",
            bm->dayt, spotid, biomass, NH3biom, NO3biom, DONbiom);
    
	return;
}
*/
