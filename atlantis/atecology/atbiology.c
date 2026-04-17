/**
 \file
 \brief The main routine calls a subroutine biology(...) with a pointer pBox
 to a C structure Box, which is defined in boxm.h.
 All tracer data are saved in the structure Box.
 \ingroup atEcology

 File:		atbiology.c
 Created:	21/9/2004
 Author:		Beth Fulton,   CSIRO Marine Laboratories,  2004
 Based on work by Ouyang and Fulton for Port Phillip Bay
 Purpose:	C file for running the ecological models in south east.

 The main routine calls a subroutine biology(...)
 with a pointer pBox to a C structure Box, which is
 defined in boxm.h. All tracer data are saved in
 the structure Box.

 Note:
 1. Some names of variables or subroutines are self-explaining.
 2. In the names or comments, WC means Water Column, Sm Sediment.

 Revisions: 	8/8/2004 Created msebiol.c file from sebiol.c

 14/8/2004 Moved annual events from biology() to Annual_biology()

 15/8/2004 Moved time check routines from biology() to Time_Check(),
 which is called from the main loop in msebm.c

 21/9/2004 Created atbiology.c from msebiol.c. And renamed
 FSL, FSO, FSF, FST and FSG to generic names so apply to
 other models/generic case.
 FSL -> FDC  (Ling -> Demersal_DC_Fish)
 FSO -> FDO  (Orange_Roughy -> Demersal_O_Fish)
 FSF -> FDB  (Flathead -> Demersal_B_Fish)
 FST -> FVO  (Bluefin_Tuna -> Pisciv_O_Fish)
 FSG -> SHB  (Gummy_Shark -> Shark_B)

 22/9/2004 Replaced numbers in switch statements with id
 guild numbers.

 15/10/2004 Added BML and BFF.

 19/10/2004 Added Prawns

 24/10/2004 Added soft sediments

 26/10/2004 Added FBP, FVB, SP, FDE, FDF, SHR, SSK, REP, WDG

 5/12/2004 Added hemisphere check to TemperatureAtDepth() and option to
 use actual rather than crudely represented temperatures

 7/12/2004 Added Check_Gape()

 19/1/2005 Added Microphytobenthos (MB)

 28/3/2005 Added checks using bm->do_availflag (global flag indicating
 whether this option is on at all) Bioflag[ij] (indicating whether
 the variable is one that may be switched off and so should be set
 to 0.0 non min_pool) and Activeflag[ij] (flag indicating
 whether the variable is active or not) so that can better handle
 groups that aen't active, so that avoid rounding errors in
 backwaters where the 1e-8 min_pools can accumulate through
 advection to be a problem.

 30/3/2005 Added baseline temperature for cases where not using the
 regression and not feeding in temperature time series

 31/3/2005 Redid TemperatureAtDepth() so better use of temperature
 in Q10 calculations rather than just assuming baseline of 15degC
 always

 16/8/2005 Put in default case for all switch statements

 1/9/2005 Added multiple stock structure (horizontal and vertical)

 5/10/2005 Moved Annual_biology() to atannualbiology.c and
 Box_Light_Process(), Box_O2_Depth_Process() and TemperatureAtDepth()
 to atbiophysics.c

 14/06/2008 Bec Gorton

 Fixed a bug in Adapt_Diff_Method. The third loop that transfers values to
 the local pool was overwriting some of the diagnostic values.

 15/6/2008 Bec Gorton
 Caught the goof in the counter for the loop dealing with the
 initialisaiton of the fisheries statistics in Adapt_Diff_Method()

 16/7/2008 Bec Gorton
 Changed the Box_Bio_Process code to store the resulting tracer values straight into
 newwctr and newsedtr instead of Yp. This means Yp is no longer needed.
 This also means that the values of Yp do not need to be copied back to newwctr and
 newsedtr in the main loop.

 18/07/2008 Bec Gorton
 Changing the code within and called by Box_Bio_Process to use the temporary data
 stores in the EcologyModule instead of Pool and NetFLUX.

 02-02-2009 Beth Fulton
 Changed the Get_Time_Change to try to fix the issue of not showing the
 epibenthic or sediment tracer issues.
 Also change the small value to now be called small_num.

 03-02-3009 Bec Gorton
 Added code in AdaptDiffMethod to reset the fishFlux and diagFlux tracer values to zero.
 This means we can always use += when using these tracers.

 14-12-2009 Bec Gorton
 Added additional checking in case the tracer value is infinite. Also removed redundant
 checks on the tracer values in Adapt_Diff_Method.

 15-12-2009 Bec Gorton
 Trunk merge 1427 - Added the calcMLinearMort, calcMQuadMort and calcMPredMort arrays to store different mortality values.

 29-01-2010 Bec Gorton
 Updated the code to use the renamed harvest library functions. Also reformatted the code.

 11-02-2010 Bec Gorton
 Change the names of all functions to conform to coding standard. Also change the Check_Tracer_Array to only check the
 value of the given tracer array not the actual box value from the previous time step. This is called within the adaptive
 time step so only the localTracer values will be changing and the actual tracer value from the previous timestep is checked
 when these values are copied to the local arrays.

 26-04-2010 Bec Gorton
 Fixed a memory bug in getTimeChange.

 17-04-2013 Bec Gorton
 ATL-346 - Added support of tracking P:N:C ratios in groups.
 **************************************************************************

 Table 1. Summary of Model State Variables.

 Description	Symbol					Units
 ------------------              ------------
 Large phytoplankton				PL	mg N m-3
 Small phytoplankton (pico)		PS	mg N m-3
 Dinoflagellates					DF	mg N m-3
 Seagrass						SG	mg N m-2
 Macroalgae						MA	mg N m-2
 Microphytobenthos				MB	mg N m-2
 Small planktivorous fish        FPS special
 Large planktivorous fish        FPL special
 Other planktivorous fish        FPO special
 Deep piscivorous fish           FVD special
 Vulnerable piscivorous fish     FVV special
 Shallow piscivorous fish        FVS special
 Other piscivorous fish          FVB special
 Large piscivorous fish (tuna)   FVT special
 Other tuna                      FVO special
 Migratory mesopelagics fish     FMM special
 Non-migratory mesopelagics fish FMN special
 Benthopelagics                  FBP special
 Herbivorous demersal fish       FDE special
 Shallow demersal fish           FDS special
 Other shallow demersal fish     FDB special
 Deep demersal fish              FDD special
 Other deep demersal fish        FDC special
 Longlived deep demersal fish    FDO special
 Flat deep demersal fish         FDF special
 Protected demersal fish         FDP special
 Miscellaneous demersal fish     FDM special
 Demersal sharks                 SHD special
 Dogfish                         SHC special
 Other demersal sharks           SHB special
 Pelagic sharks                  SHP special
 Reef sharks                     SHR special
 Skates and rays                 SSK special
 Seabirds                        SB  special
 Penguins                        SP  special
 Pinnipeds                       PIN special
 Reptiles						REP special
 Baleen whales                   WHB special
 Small toothed whales            WHS special
 Toothed whales                  WHT special
 Dugongs                         WDG special
 Prawns                          PWN mg N m-3
 Cephalopods                     CEP mg N m-3
 Gelatinous zooplankton			ZG	mg N m-3
 Large carnivorous zooplankton	ZL	mg N m-3
 Mesozooplankton					ZM	mg N m-3
 Small zooplankton				ZS	mg N m-3
 Pelagic associated bacteria		PB	mg N m-2
 Sedimentary bacteria			BB	mg N m-2
 Meiobenthos						BO	mg N m-2
 Deposit feeders					BD	mg N m-2
 Benthic infaunal carnivores		BC	mg N m-2
 Benthic grazers					BG	mg N m-2
 Benthic filter feeders shallow	BFS	mg N m-2
 Benthic filter feeders deep		BFD	mg N m-2
 Other benthic filter feeders	BFF	mg N m-2
 Macrozoobenthos shallow			BMS	mg N m-2
 Macrozoobenthos deep			BMD	mg N m-2
 Megazoobenthos      			BML	mg N m-2
 Labile detritus					DL	mg N m-3
 Refractory detritus				DR	mg N m-3
 Carrion							DC	mg N m-3
 Dissolved organic nitrogen		DON	mg N m-3
 Ammonia							NH	mg N m-3
 Nitrate							NO	mg N m-3
 Dissolved silica				Si	mg Si m-3
 Biogenic silica					DSi	mg Si m-3
 Dissolved oxygen				O2	mg O m-3
 Light							IRR	W m-2

 Those marked with a + also have Si pool too.
 Those marked special have three pools, a sructural N (mg N individual-1),
 reserve N (mg N individual-1) and numbers (per box)
 *************/

/*  Open library routines */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <sjwlib.h>
#include "atecology.h"
#include "atbiologyPrivate.h"
#include "atbiology.h"
#include <atHarvestLib.h>

static void Accumulate_Mortality_Estimates(MSEBoxModel *bm, double dtsz, FILE *llogfp);
static void UpdateMdt(MSEBoxModel *bm, double dtsz, FILE *llogfp);
static void Box_Bio_Process(MSEBoxModel *bm, Box *pBox, double dt, FILE *llogfp);
static void Adapt_Diff_Method(MSEBoxModel *bm, int flagModel, double tsz, BoxLayerValues *boxLayerInfo, FILE *llogfp);

/**
 *
 *	\brief This routine is the main procedure in the ecological module.
 *
 *	It is called by main() in the transport module.
 *
 *	@input	Box	*pBox :  pointer to a Box structure
 *	@input	double	t     :  current time  (seconds)
 *	@input	double  dt    :  time range
 *	@input	double	**Yp  :  vector of pointers to tracers at time t+dt
 *
 */
void Ecology_Box_Biology(MSEBoxModel *bm, Box *pBox, double dt, FILE *llogfp) {
	double midpoint = pBox->inside.y;
    int this_top_layer = bm->top_layer;
    
    if (verbose > 0){
        printf("processing box %d\n", pBox->n);
        fflush(stdout);
        fflush(stderr);
    }

    if(!bm->dtsz_stored) {
        bm->dtsz_stored = dt;
    }
    
    /* Set current box */
	bm->current_box = pBox->n;
    
	/* Calculate temperature sensitive parameters of models */
	Parameter_Q10(bm, pBox, bm->dayt, 1, 0, pBox->nz - 1, midpoint, WC, llogfp);
    
	/* Calculate light intensity on surfaces or bottoms of water column cells */
	if (bm->lim_sun_hours)
		Calculate_Box_Light(bm, pBox, llogfp);
    
	/* Need to do ice light calculations so we know how much light gets through to the water column under the ice */
	if(bm->ice_on) {
		Box_Ice_Light_Level(bm, pBox, llogfp);				// Light inside the ice
	}
	Box_Light_Process(bm, pBox, llogfp);
    
	/* Calculate oxygen depth */
	Box_O2_Depth_Process(bm, pBox);

	/* Get local rugosity */
    Box_Rugosity(bm, pBox, 0, llogfp);
    
    /* Get Local Wind */
    if( bm->track_wind) {
        this_top_layer = pBox->nz - 1;
        current_WIND =  pBox->tr[this_top_layer][Wind_i];
    }
 
	/* Ice related biologically relevant physical ice properties */
	if(bm->ice_on) {
		Box_Ice_Flux(bm, pBox, llogfp);  					// Flux of dissolved properties into ice

		Box_Ice_Temperature_Related(bm, pBox, llogfp);		// Temperature inside the ice
	}
    
    // Initialise diet data content - if required
    if (bm->flagdietcheck) {
        ReInitDietData(bm, pBox->n);
    }

	/* Run biological process */
	Box_Bio_Process(bm, pBox, dt, llogfp);

	return;
}

/**
 *	\brief Box_Bio_Process
 *
 */
static void Box_Bio_Process(MSEBoxModel *bm, Box *pBox, double dt, FILE *llogfp) {
    int ij, FlagModel, k, stopij, oceanic_only;
    //int pid;
	double midpoint = pBox->inside.y;
	double sedC, ratioC;
	int maxdeep = bm->wcnz;
	int totaldeep = pBox->nz;
	int numwclayer = pBox->nz; /* Number of cells in WC */
	int numsmlayer = pBox->sm.nz; /* Number of cells in Sm */
	int numicelayer = pBox->ice.currentnz; /* Number of cells in ICE */
    int totout = bm->K_num_tot_sp + 2; // Extra entries for remineralisation and final flux
    //int totfluxout = bm->K_num_tot_sp + num_nut_flux_id; // Extra entries for nutrient fluxes
    int totfluxout = bm->K_num_tot_sp + bm->K_num_physiochem;

    if (verbose > 1)
		printf("Doing Box_Bio_Process\n");
    
    /** Allocate or initialise memory to store all of the box layer information */
	//printf("Creating boxLayer arrays\n");
    Util_Init_2D_Long_Double(boxLayerInfo->NutsProd, bm->num_active_habitats, K_num_nutrients, 0.0);
    Util_Init_3D_Long_Double(boxLayerInfo->NutsProdGlobal, bm->num_active_habitats, bm->num_active_habitats, K_num_nutrients, 0.0);
    Util_Init_2D_Long_Double(boxLayerInfo->NutsLost, bm->num_active_habitats, K_num_nutrients, 0.0);
    Util_Init_3D_Long_Double(boxLayerInfo->NutsLostGlobal, bm->num_active_habitats, bm->num_active_habitats, K_num_nutrients, 0.0);
    Util_Init_2D_Long_Double(boxLayerInfo->DetritusProd, bm->num_active_habitats, K_num_nutrients, 0.0);
    Util_Init_3D_Long_Double(boxLayerInfo->DetritusProdGlobal, bm->num_active_habitats, bm->num_active_habitats, K_num_nutrients, 0.0);
    Util_Init_2D_Long_Double(boxLayerInfo->DetritusLost, bm->num_active_habitats, K_num_nutrients, 0.0);
    Util_Init_3D_Long_Double(boxLayerInfo->DetritusLostGlobal, bm->num_active_habitats, bm->num_active_habitats, K_num_nutrients, 0.0);

    Util_Init_1D_Double(boxLayerInfo->localWCTracers, (2 * numwcvar + numepivar), 0.0);
    Util_Init_1D_Double(boxLayerInfo->localWCFlux, (2 * numwcvar + numepivar), 0.0);

    Util_Init_1D_Double(boxLayerInfo->localSEDTracers, numwcvar, 0.0);
    Util_Init_1D_Double(boxLayerInfo->localSEDFlux, numwcvar, 0.0);

    Util_Init_1D_Double(boxLayerInfo->localEPITracers, numwcvar, 0.0);
    Util_Init_1D_Double(boxLayerInfo->localEPIFlux, numwcvar, 0.0);

    Util_Init_1D_Double(boxLayerInfo->localICETracers, numwcvar, 0.0);
    Util_Init_1D_Double(boxLayerInfo->localICEFlux, numwcvar, 0.0);

    Util_Init_1D_Double(boxLayerInfo->localLANDTracers, numwcvar, 0.0);
    Util_Init_1D_Double(boxLayerInfo->localLANDFlux, numwcvar, 0.0);

    Util_Init_1D_Double(boxLayerInfo->localDiagTracers, numdiagvar, 0.0);
    Util_Init_1D_Double(boxLayerInfo->localDiagFlux, numdiagvar, 0.0);

    Util_Init_1D_Double(boxLayerInfo->localFishTracers, numfstatvar, 0.0);
    Util_Init_1D_Double(boxLayerInfo->localFishFlux, numfstatvar, 0.0);
    
    Util_Init_3D_Double(boxLayerInfo->DebugInfo, totout, bm->num_active_habitats, Diagnostnlevel_id, 0.0);
    Util_Init_3D_Double(boxLayerInfo->DebugFluxInfo, totfluxout, bm->num_active_habitats, 2, 0.0);

    boxLayerInfo->BB_DL = 0.0;
    boxLayerInfo->BB_DR = 0.0;
    boxLayerInfo->PB_DL = 0.0;
    boxLayerInfo->PB_DR = 0.0;

    boxLayerInfo->DIN = 0.0;

	//	int den, cohort;
	bm->max_depth = bm->maxwcbotz;

	/** Identify the current box */
	bm->current_box = pBox->n;

	/** Identify current bottom depth */
	current_depth = pBox->botz;

	/** Set extra bioirrigation enhancement to one */
	BioirrigEnh = 1.0;

	/** Set extra bioturbation enhancement to one */
	BioturbEnh = 1.0;

	/** Set maximum depth of detritus */
	DRdepth = bm->boxes[bm->current_box].sm.detdepth;

	/** Reset waterboundary **/
	waterboundary = 0;

	/** Determine local substrate type */
	area_reef = bm->boxes[bm->current_box].reef;
	area_flat = bm->boxes[bm->current_box].flat;
	area_soft = bm->boxes[bm->current_box].soft;
	area_box = bm->boxes[bm->current_box].area;
	eddy_strength = bm->boxes[bm->current_box].eddy;

	/** Initialise catch, discard and bycatch arrays for the box **/
	if(bm->flag_fisheries_on)
		Harvest_Init_Box_Arrays(bm, bm->current_box, llogfp);

	/** If in open ocean pelagic only*/
	if (current_depth < bm->max_depth) {
		oceanic_only = 1;
		stopij = -1;
	} else {
		oceanic_only = 0;
		stopij = 0;
	}

	/** Process layers **/
	Susp_Sed = 1.0;
	cell_depth = 0.0;

	/** First, process water column layers not in contact with sediment: **/
	/* From top cell to the second last cell in WC */

	FlagModel = 1;
    
	for (ij = numwclayer - 1; ij > stopij; ij--) {
		if (verbose > 1)
        printf("processing water column layer %d\n", ij);
        
		/* Get layer's physical characteristics */
		bm->current_layer = ij;
		cell_depth = cell_depth - pBox->dz[ij];
		wcLayerThick = pBox->dz[ij];
		bm->cell_vol = pBox->area * pBox->dz[ij];
		if(bm->cell_vol <= 0)
			quit("ERROR: Volume of cell box %d: layer %d is zero. Area = %e, depth = %e\n",
					bm->current_box, bm->current_layer, pBox->area, pBox->dz[ij]);

		assert((_finite(bm->cell_vol)));

		if (!wcLayerThick) {
			fprintf(llogfp, "WARNING box %d layer %d has water depth of %e\n", bm->current_box, bm->current_layer, wcLayerThick);
			wcLayerThick += small_num;
		}
        current_layer_sed = -1;

		/* If need be determine depth specific biological parameters */
		if (numwclayer > 1)
			Parameter_Q10(bm, pBox, bm->dayt, numwclayer, current_depth, bm->current_layer, midpoint, WC, llogfp);

		if(bm->flag_fisheries_on)
			Harvest_Init_Layer_Arrays(bm, bm->current_box, bm->current_layer, llogfp);

		/* Get sediment depth in case need it for fisheries statistics calculations */
		smLayerThick = -pBox->sm.dz[0];

		/* Give index of amount of suspended sediment in the water column */
		if (oceanic_only || !bm->resuspension || bm->cell_vol / pBox->area > 25)
			Susp_Sed = 1.0;
		else
			Susp_Sed = pBox->stress * pBox->area / bm->cell_vol;

        /* Run Adaptive Difference Method */
		Adapt_Diff_Method(bm, FlagModel, dt, boxLayerInfo, llogfp);

		/* Spawning */
		Vertebrate_Reproduction(bm, ij, maxdeep, totaldeep, numwclayer, boxLayerInfo->localWCTracers, llogfp);
		Invertebrate_Reproduction(bm, ij, maxdeep, totaldeep, boxLayerInfo->localWCTracers, llogfp);

		/* Transfer all the temporary values back to their final locations */
		for (k = 0; k < numwcvar; k++)
			newwctr[bm->current_box][ij][k] = boxLayerInfo->localWCTracers[k]; /* To WC */
		for (k = 0; k < numdiagvar; k++)
			pBox->diagnost[k] = boxLayerInfo->localDiagTracers[k]; /* To diagnostics */
		for (k = 0; k < numfstatvar; k++)
			pBox->fishstat[k] = boxLayerInfo->localFishTracers[k]; /* To fisheries statistics */
	}
    
    /** Now process sediment layers not in contact with water. **/
	if (!oceanic_only) {
		FlagModel = 2;

		/* Get depth of sediment surface and then
		 if need be determine depth specific biological parameters */
		cell_depth = cell_depth - pBox->dz[0];
		if (numwclayer > 1)
			Parameter_Q10(bm, pBox, bm->dayt, numwclayer, current_depth, 0, midpoint, SED, llogfp);

		/* From the second top cell to the last cell */
		for (ij = pBox->sm.topk + 1; ij < numsmlayer; ij++) {

			if (verbose > 1)
				fprintf(llogfp, "processing sediment layer ij, %d\n", ij);

			bm->current_layer = ij;
			sporosity = (double) pBox->sm.porosity[ij];

			/** Set benthos stimulation of bacteria - compound of current
			 bioturbation levels and sediment porosity effect, the
			 porosity effect is from Fig 7 (bottom plot) in
			 Blackburn, 1987. Microbial food webs in sediments. In:
			 Sleigh (ed), Microbes in the sea. Ellis Horwood:New York. pp 39
			 **/
			sedC = (sporosity - 0.225) / 0.004;
			ratioC = sedC / 193.75;
			if (flagbactstim)
				Bact_stim = bm->boxes[bm->current_box].sm.turbenh * ratioC;
			else
				Bact_stim = 1.0; // Originally introduced while model being balanced

			current_layer_sed = ij;

			/* Run Adaptive Difference Method */
			Adapt_Diff_Method(bm, FlagModel, dt, boxLayerInfo, llogfp);

			/* Transfer all of the values back to the final locations */
			for (k = 0; k < numwcvar; k++)
				newsedtr[bm->current_box][ij - numwclayer][k] = boxLayerInfo->localSEDTracers[k]; /* To WC */
			for (k = 0; k < numdiagvar; k++)
				pBox->diagnost[k] = boxLayerInfo->localDiagTracers[k]; /* To diagnostics */
			for (k = 0; k < numfstatvar; k++)
				pBox->fishstat[k] = boxLayerInfo->localFishTracers[k]; /* To fisheries statistics */
		}
        
        /** Now process epibenthic layer and adjacent water and sediment layers. **/
		FlagModel = 3;

		/** Get data at the bottom cell in water column **/
		bm->current_layer = 0;
		bm->cell_vol = pBox->area * pBox->dz[bm->current_layer];

		if (verbose > 1)
			fprintf(llogfp, "processing epibenthic layer %d\n", 0);

		/* Get depth of sediment surface and then
		 if need be determine depth specific biological parameters */
		cell_depth = cell_depth - pBox->dz[bm->current_layer];
		if (numwclayer > 1)
			Parameter_Q10(bm, pBox, bm->dayt, numwclayer, current_depth, bm->current_layer, midpoint, WC, llogfp);

		if(bm->flag_fisheries_on)
			Harvest_Init_Layer_Arrays(bm, bm->current_box, bm->current_layer, llogfp);

		/** Get data at the top cell in sediment **/
		ij = pBox->sm.topk;
		current_layer_sed = ij;
		sporosity = (double) pBox->sm.porosity[ij];

		if (!bm->supplied_stress)
			surf_stress = (double) pBox->stress / 1000.0;
		else
			surf_stress = (double) pBox->stress;

		/* Note pBox->stress are the surface stresses from the hydrodunamic model
		 and the division by 1000 just gets them into the correct
		 units (i.e. its a unit conversion constant) - based on the
		 same conversion made in the transport model. */
		/*   surf_stress = pBox->stress * pBox->sm.er[ij]; */

		/*  Use this if want to use erosion rate as a measure
		 of stress on the sediment and want absolute consistency
		 with the transport library. If want to use actual
		 hydrodynamic stresses (from hydrodynamic model) then use
		 line above instead. (i.e. surf_stress = (double) pBox->stress / 1000) */

		/* Give index of amount of suspended sediment in the water column */
		/* If no resuspension inmodel then this means no nitrifcation potential
		 in the watercolumn sogive a default valueof 1.0 inthat case */
		if (!bm->resuspension)
			Susp_Sed = 1.0;
		else
			Susp_Sed = pBox->stress * pBox->area / bm->cell_vol;

		/** Set benthos stimulation of bacteria - compound of current
		 bioturbation levels and sediment porosity effect, the
		 porosity effect is from Fig 7 (bottom plot) in
		 Blackburn, 1987. Microbial food webs in sediments. In:
		 Sleigh (ed), Microbes in the sea. Ellis Horwood:New York. pp 39*/
		sedC = (sporosity - 0.225) / 0.004;
		ratioC = sedC / 193.75;
		if (flagbactstim)
			Bact_stim = bm->boxes[bm->current_box].sm.turbenh * ratioC;
		else
			Bact_stim = 1.0; // Originally introduced while model being balanced

		/* Get layer thicknesses */
		smLayerThick = -pBox->sm.gridz[1];
		wcLayerThick = -pBox->gridz[0] - (-pBox->gridz[1]);
		/* Use this if get model anomalies with nutrients in water <1m due to epibenthos */
		if (bm->constrain_wc)
			wcLayerThick = max(wcLayerThick, 1.0);

		if (!wcLayerThick) {
			fprintf(llogfp, "WARNING box %d layer %d has water depth of %e\n", bm->current_box, bm->current_layer, wcLayerThick);
			wcLayerThick += small_num;
		}

		if (!smLayerThick) {
			fprintf(llogfp, "WARNING box %d layer %d has sediment depth of %e\n", bm->current_box, bm->current_layer, smLayerThick);
			smLayerThick += small_num;
		}

		/* Run Adaptive Difference Method */
		Adapt_Diff_Method(bm, FlagModel, dt, boxLayerInfo, llogfp);

		/* Spawning */
		Vertebrate_Reproduction(bm, 0, maxdeep, totaldeep, numwclayer, boxLayerInfo->localWCTracers, llogfp);
		Invertebrate_Reproduction(bm, 0, maxdeep, totaldeep, boxLayerInfo->localWCTracers, llogfp);

		/* Transfer all values in localWCTracers and localSed to the appropriate variables and
		 reset pointers Y1 and Y2 to newwctr and newsed tr pointing to returned value space
		 (rest straight update stored values) */

		for (ij = 0; ij < numwcvar; ij++) {
			newwctr[bm->current_box][0][ij] = boxLayerInfo->localWCTracers[ij]; /* To bottom cell in WC */
			newsedtr[bm->current_box][0][ij] = boxLayerInfo->localSEDTracers[ij]; /* To top cell in sediment*/
		}

		for (ij = 0; ij < numepivar; ij++) /* To epibenthic variables*/
			pBox->epi[ij] = boxLayerInfo->localEPITracers[ij];
		for (k = 0; k < numdiagvar; k++)
			pBox->diagnost[k] = boxLayerInfo->localDiagTracers[k]; /* To diagnostics */
		for (k = 0; k < numfstatvar; k++)
			pBox->fishstat[k] = boxLayerInfo->localFishTracers[k]; /* To fisheries statistics */
        
        /** Now process ice layers. ****************************************************************************************/
		if(bm->ice_on) {
			FlagModel = 4;

			for (ij = numicelayer - 1; ij >= 0; ij--) {
				if (verbose > 1)
					fprintf(llogfp, "processing ice layer %d\n", ij);

				bm->current_icelayer = ij;

				/* Get temperature corrections to parameters for groups in the ice */
				Box_Ice_Q10(bm, pBox, ij, llogfp);

				/* TODO: Need to be able to harvest in the ice? */

				/* Run Adaptive Difference Method */
				Adapt_Diff_Method(bm, FlagModel, dt, boxLayerInfo, llogfp);

				/* TODO Spawning and reproduction of ice dependent groups */

				/* Transfer all the temporary values back to their final locations */
				for (k = 0; k < numwcvar; k++) /* To epibenthic variables*/
					newicetr[bm->current_box][ij][k] = boxLayerInfo->localICETracers[k];

			}
		}
		/** Move on to other activities per box. **********************************************************************/
	}

	/* Calculate bioirrigation and bioturbation enhancements */
	Irrig_and_Turb(bm, &BioirrigEnh, &BioturbEnh);

	/* Update bioirrigation and bioturbation enhancement */
	bm->boxes[bm->current_box].sm.irrigenh = BioirrigEnh;
	bm->boxes[bm->current_box].sm.turbenh = BioturbEnh;

	/* Calculate new detrital depth */
	bm->boxes[bm->current_box].sm.detdepth = bm->boxes[bm->current_box].sm.detdepth + Enviro_turb * BioturbEnh / DRdepth * (1.0 - exp(-K_TUR_DEP / (DRdepth
			+ small_num)));
    
    /*
    pid = FunctGroupArray[8].contamPropTracers[3][0];
    fprintf(bm->logFile, "Time: %e at end of Box_Bio_Process for box%d-%d - test propContam %s-%d for %s in box%d-%d: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[8].groupCode, 3, bm->contaminantStructure[0]->contaminant_name, 3, 2, bm->boxes[3].tr[2][pid]);
    printf("Time: %e at end of Box_Bio_Process for box%d-%d - test propContam %s-%d for %s in box%d-%d: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[8].groupCode, 3, bm->contaminantStructure[0]->contaminant_name, 3, 2, bm->boxes[3].tr[2][pid]);
     */
	return;
}

/**
 * \brief Find the most unstable variable and its maximum change rate.
 *
 *
 **/
static void Get_Max_Flux_Change(MSEBoxModel *bm, double *tracerArray, double *fluxArray, int numVariables, int offset, double del, double *max_del,
		int *ind_unstable_var, FILE *llogfp) {
	int ij;

	for (ij = 0; ij < numVariables; ij++) {

		if(Fluxflag[offset + ij] == 2)
			continue;

		/* List fluxs and pools of every variable */
		if ((bm->ecotest == 4) && (bm->dayt >= bm->checkstart)){
			if(fabs(fluxArray[ij]) > 0)
				fprintf(llogfp, "Box: %d-%d (it_count: %d) Vari ij: %d (%s) has Netflux %e and local pool is %e\n", bm->current_box, bm->current_layer, it_count,
						ij, Varname[ij + offset], fluxArray[ij], tracerArray[ij]);
        }

		/* Find the most unstable variable and its maximum change rate */
		if ((Tolflag[ij + offset] == 1) && (tracerArray[ij] > 0.01)) {
			del = fabs(fluxArray[ij] / tracerArray[ij]);
		}

		if (del > *max_del) {
			*max_del = del;
			*ind_unstable_var = ij;
		}
	}
}

/**
 * \brief Get the time change.
 *
 *	Find the max change in the water column, sediment and/or epibenthic tracers/fluxes.
 *
 *	The time change is returned in the dtsz variable and the time_left variable will
 *	be the time left.
 *
 *	The time step is dependant on the maximum change in the tracers as returned
 *	by Get_Max_Flux_Change().
 *
 *	The doWC, doSed and doEpi specify is the water column, sediment layer
 *	and/or epibenthic layers are checked.
 *
 *
 */
static void Get_Time_Change(MSEBoxModel *bm, FILE *llogfp, int flagModel, double *dtsz, double *time_left, BoxLayerValues *boxLayerInfo) {
	double maxTracer_del, maxSed_del, maxEpi_del, maxIce_del;
	int tracer_ind_unstable_var, sed_ind_unstable_var, epi_ind_unstable_var, ice_ind_unstable_var;
	double fluxValue = 0.0, tracerValue = 0.0;
	double del, max_del;
	double old_time_left;
	int ind_unstable_var = -1;
	int offset_int;
	int unstable_offset = -1;
    //double orig_dtsz = *dtsz;

	/* Check integration results */
	del = RelTol / *dtsz;
	max_del = del;
	maxTracer_del = max_del;
	maxSed_del = max_del;
	maxEpi_del = max_del;
	maxIce_del = max_del;
	tracer_ind_unstable_var = 0;

	/* Always check WC values */
	if(flagModel == WC || flagModel == SED || flagModel == EPIFAUNA){

		/* Get the max change in the tracer variables */
		Get_Max_Flux_Change(bm, boxLayerInfo->localWCTracers, boxLayerInfo->localWCFlux, bm->ntracer, 0, del, &maxTracer_del, &tracer_ind_unstable_var, llogfp);
		offset_int = 0;
		if (maxTracer_del > max_del) {
			max_del = maxTracer_del;
			ind_unstable_var = tracer_ind_unstable_var;
			tracerValue = boxLayerInfo->localWCTracers[ind_unstable_var];
			fluxValue = boxLayerInfo->localWCFlux[ind_unstable_var];
			unstable_offset = offset_int;
		}
	}

	if(flagModel == SED || flagModel == EPIFAUNA){
		/* Get the max change in the sediment variables */
		Get_Max_Flux_Change(bm, boxLayerInfo->localSEDTracers, boxLayerInfo->localSEDFlux, bm->ntracer, bm->ntracer, del, &maxSed_del, &sed_ind_unstable_var,
				llogfp);
		offset_int = bm->ntracer;
		if (maxSed_del > max_del) {
			max_del = maxSed_del;
			ind_unstable_var = sed_ind_unstable_var;
			tracerValue = boxLayerInfo->localSEDTracers[ind_unstable_var];
			fluxValue = boxLayerInfo->localSEDFlux[ind_unstable_var];
			unstable_offset = offset_int;
		}
	}

	if(flagModel == EPIFAUNA){
		/* Get the max change in the epibenthic variables */
		Get_Max_Flux_Change(bm, boxLayerInfo->localEPITracers, boxLayerInfo->localEPIFlux, bm->nepi, 2 * bm ->ntracer, del, &maxEpi_del, &epi_ind_unstable_var, llogfp);
		offset_int = 2 * bm->ntracer;
		if (maxEpi_del > max_del) {
			max_del = maxEpi_del;
			ind_unstable_var = epi_ind_unstable_var;
			tracerValue = boxLayerInfo->localEPITracers[ind_unstable_var];
			fluxValue = boxLayerInfo->localEPIFlux[ind_unstable_var];
			unstable_offset = offset_int;
		}
	}

	if(flagModel == ICE_BASED ){
		/* Get the max change in the ice-based variables */
		Get_Max_Flux_Change(bm, boxLayerInfo->localICETracers, boxLayerInfo->localICEFlux, bm->ntracer, bm ->ntracer, del, &maxIce_del, &ice_ind_unstable_var,
				llogfp);
		offset_int = 0;  // Its assumed to be zero in Adapt_Diff_Method().... should it actually be "3 * bm->ntracer" so not accidentally overwriting anything?
		if (maxIce_del > max_del) {
			max_del = maxIce_del;
			ind_unstable_var = ice_ind_unstable_var;
			tracerValue = boxLayerInfo->localICETracers[ind_unstable_var];
			fluxValue = boxLayerInfo->localICEFlux[ind_unstable_var];
			unstable_offset = offset_int;
		}
	}
	/* Set a new time step size */
	*dtsz = RelTol / max_del;
    
    /*
    if(isnan(*dtsz)){
        printf("flagModel: %d RelTol: %e max_del: %e orig_dtsz: %e\n", flagModel, RelTol, max_del, orig_dtsz);
    }
     */
    
	/* Check if the new step size (unit: second) is too small_num, or not. */
	/* minimum time step reduced */
	if (*dtsz < 0.1) {
        
        if (bm->boxes[bm->current_box].relax_tol){
            fprintf(llogfp, "Time step size %e (seconds) is too small, but in box %d where relax_tol > 0 so lets keep going!\n", *dtsz, bm->current_box);
            fprintf(llogfp, "Time: %e The UNstable vari. %d is %s (offset: %d %s), its  Pool = %g, Netflux =%g on day: %e in box: %d layer %d\n", bm->dayt, ind_unstable_var, Varname[ind_unstable_var + unstable_offset], unstable_offset, Varname[ind_unstable_var], tracerValue, fluxValue, bm->dayt,
                    bm->current_box, bm->current_layer);
            
            warn("Time: %e The UNstable vari. %d is %s (offset: %d %s), its  Pool = %g, Netflux =%g on day: %e in box: %d layer %d\n", bm->dayt, ind_unstable_var, Varname[ind_unstable_var + unstable_offset], unstable_offset, Varname[ind_unstable_var], tracerValue, fluxValue, bm->dayt,
                 bm->current_box, bm->current_layer);
            
            *dtsz = 0.1;
            
        } else {
            fprintf(llogfp, "Time step size %e (seconds) is too small.\n", *dtsz);

            fprintf(llogfp, "Time: %e The UNstable vari. %d is %s (offset: %d %s), its  Pool = %g, Netflux = %g on day: %e in box: %d layer %d\n", bm->dayt,
				ind_unstable_var, Varname[ind_unstable_var + unstable_offset], unstable_offset, Varname[ind_unstable_var], tracerValue, fluxValue, bm->dayt,
				bm->current_box, bm->current_layer);

            printf("Time: %e The UNstable vari. %d is %s (offset: %d %s), its  Pool = %g, Netflux = %g on day: %e in box: %d layer %d\n", bm->dayt,
				ind_unstable_var, Varname[ind_unstable_var + unstable_offset], unstable_offset, Varname[ind_unstable_var], tracerValue, fluxValue, bm->dayt,
				bm->current_box, bm->current_layer);

            printf("Try setting ecotest to '4' or higher in your biology input file. Redo your run and look at the bottom of your log.txt output file\n");
            fflush(llogfp);
            Textfile_Dump(bm, llogfp);

            /* This quit is broken go for something simpler
             quit("Time step size %e (seconds) is too small due to UNstable vari. %s in box %d-%d\n",
             *dtsz, Varname[ind_unstable_var].name, bm->current_box, bm->current_layer);
             */
            quit(" ");
        }
	}

	/* deduct the time left (if <0.1 second remaining include it also) */
	old_time_left = *time_left;
	*time_left = *time_left - *dtsz;
	if (*time_left < 0.1) {
		*dtsz = old_time_left;
		*time_left = 0.0;
	}
}
/**
 * \brief Dynamic integeration - use one-step method to integrate all dynamic
 *	variables
 *
 *	Integrate the specified tracerArray and fluxArray numVariables variables.
 *	The offset indicates the offset of these variables in the Activeflag, Tolflag
 *	and Bioflag arrays.
 *
 *
 */
static void Integrate_Tracer_Variables(MSEBoxModel *bm, int flagModel, double *tracerArray, double *fluxArray, int numVariables, int offset, double tsz, double dtsz,
		FILE *llogfp) {
	int ij, recheck;
	double old_local_pool;
	double dtstep = 0;

	/** Dynamic integration **/
	/* Use one-step method to integrate all dynamic variables */
	for (ij = 0; ij < numVariables; ij++) {
        recheck = 0;
        
        /**
        if((bm->dayt < 1.0) && (bm->current_box == 1) && (bm->current_layer == 0)) {
            fprintf(llogfp, "ij: %d offset: %d %s Tolflag: %d\n", ij, offset, Varname[ij + offset], Tolflag[offset + ij]);
        }

		//if((bm->checkbox == bm->current_box) && (bm->checkstart <= bm->dayt)){
		if(bm->checkstart <= bm->dayt){
			fprintf(llogfp,"day %e, box %d-%d %s: start local_pool:%.20e, netflux:%.20e, (it_count %d, old_pool: %.20e, flux: %.20e, dt: %.20e)\n", bm->dayt, bm->current_box,
				bm->current_layer, Varname[ij], tracerArray[ij], fluxArray[ij], it_count, old_local_pool, fluxArray[ij], dtstep);
		}
		**/
		if(Fluxflag[offset + ij] == 2)
			continue;

		/* Check for those groups not active */
		if (bm->do_availflag && Bioflag[offset + ij]) {

			if ((!Activeflag[offset + ij] && tracerArray[ij])) {
				/* Checkif non-zero flux */
				if (fluxArray[ij] != 0) {
					printf("ij = %d, fluxArray[ij] = %e\n", ij, fluxArray[ij]);
					quit("Non-zero flux of %s (%d) in box %d layer %d on day %e, it_count = %d, flagModel= %d\n", Varname[ij + offset], offset + ij, bm->current_box, bm->current_layer, bm->dayt,
							it_count, flagModel);
				}
				/* Reset pool */
				tracerArray[ij] = 0.0;
				/* Skip to next variable */

				continue;
			}
		}


		/* Do once per day first */
		old_local_pool = tracerArray[ij];
        
        /**
		if(!(_finite(tracerArray[ij]))) {
			quit("Integrate_Tracer_Variables tracer %d - %s is nan before integration in box %d:layer %d, it_count = %d (flux: %e, tsz: %e)\n",
					ij, Varname[offset + ij], bm->current_box, bm->current_layer, it_count, fluxArray[ij], tsz);
		}
        **/
        
		if (it_count == 1) {
			if (Tolflag[offset + ij] >= 2) {
				tracerArray[ij] = tracerArray[ij] + fluxArray[ij] * tsz;
				dtstep = tsz;

			}
		}

		/* Do sensitive ones that are used to determine adaptive times step */
		if (Tolflag[offset + ij] < 2) {
			tracerArray[ij] = tracerArray[ij] + fluxArray[ij] * dtsz;
			dtstep = dtsz;
		}

        /* Now caught in check tracer step below
		if (!(_finite(tracerArray[ij]))) {
			if(fabs(fluxArray[ij]) > 0){
				fprintf(llogfp, "day: %e, box: %d, layer: %d %s localpool set to: %.20e (old_pool: %.20e, flux: %.20e, dt: %.20e), it_count = %d\n", bm->dayt, bm->current_box,
					bm->current_layer, Varname[offset + ij], tracerArray[ij], old_local_pool, fluxArray[ij], dtstep, it_count);
			}
		}
        
        if ((bm->current_box == 84) && (Tolflag[offset + ij] > 3)) {
            fprintf(llogfp, "Time: %e in box %d-%d, Integrate_Tracer_Variables tracer %d - %s is %e it_count = %d (old_pool: %e, flux: %e, tsz: %e dt: %e)\n",
                    bm->dayt, bm->current_box, bm->current_layer, ij, Varname[offset + ij], tracerArray[ij], it_count, old_local_pool, fluxArray[ij], tsz, dtstep);
        }
            
        //if ((Contamflag[offset + ij] > 0) && ((old_local_pool > 0) || (fluxArray[ij] > 0))){
        if (Contamflag[offset + ij] > 0) {
            recheck = 1;
            fprintf(llogfp, "Time: %e in box %d-%d, Integrate_Tracer_Variables tracer %d - %s is %e it_count = %d (old_pool: %e, flux: %e, dtsz: %e)\n",
                    bm->dayt, bm->current_box, bm->current_layer, ij, Varname[offset + ij], tracerArray[ij], it_count, old_local_pool, fluxArray[ij], dtsz);
        }
        */

        /* If pool is less than minimum value then set it to the minimum value */
		if (tracerArray[ij] < bm->min_pool) {

            if (Contamflag[offset + ij] > 0) {
                // Contaminants are allowed to stay < min_pool - expressed by Contamflag == 1
                if (tracerArray[ij] < 0.0) {
                    tracerArray[ij] = 0.0;
                }
            } else if (!bm->do_availflag || !Bioflag[offset + ij] || (bm->do_availflag && Bioflag[offset + ij] && Activeflag[offset + ij])) {
				if ((Tolflag[offset + ij] > 0) && (Tolflag[offset + ij] < 3))
					tracerArray[ij] = bm->min_pool; // Most pools just bounded below by minpool
				else if (Tolflag[offset + ij] == 3)
					tracerArray[ij] = bm->min_pool / 100000000.0;
				/* Vertebrate densities and carrion (discards) should be able to reach zero, but due to divide by
				 zero issues elsewhere in the code pull them up a wee bit short of zero */
			}
		}

		if (!Tolflag[offset + ij] && !Fluxflag[offset + ij]) {
			if (tracerArray[ij] < 0)
				tracerArray[ij] = bm->min_pool;
		}

		/* Check the tracers */
		if (!(_finite(fluxArray[ij])) || !(_finite(tracerArray[ij]))) {
			quit("day %e, box %d-%d %s: end local_pool:%.10f, netflux:%.10f, (it_count %d, old_pool: %e, flux: %e, dt: %e)\n", bm->dayt, bm->current_box,
					bm->current_layer, Varname[offset + ij], tracerArray[ij], fluxArray[ij], it_count, old_local_pool, fluxArray[ij], dtstep);
		}

        /**
        if ((bm->current_box ==84) && (Tolflag[offset + ij] > 3)) {
            fprintf(llogfp, "Time: %e in box %d-%d, Integrate_Tracer_Variables tracer %d - %s is now %e (basically negatives removed)\n",
                    bm->dayt, bm->current_box, bm->current_layer, ij, Varname[offset + ij], tracerArray[ij]);
        }
        
        **/
		//if(bm->checkstart <= bm->dayt){
        //if ((offset + ij) == 999)
        if (recheck) {
			fprintf(llogfp,"day %e, box %d-%d id: %d %s: local_pool:%.20f, netflux:%.20f, (it_count %d, old_pool: %e, flux: %e, dt: %e)\n", bm->dayt, bm->current_box,
				bm->current_layer, offset + ij, Varname[offset + ij], tracerArray[ij], fluxArray[ij], it_count, old_local_pool, fluxArray[ij], dtstep);
		}
		/**/
	}
	// If appropriate make sure Rugosity hasn't exploded
	if(flagModel == WC && bm->track_rugosity_arag == TRUE && !bm->containsCoral){
        tracerArray[Rugosity_i] = Calculate_Rugosity(bm, 0, 0, llogfp, 0);  // If do calculations based on empirical overall relationships rather than species by species
		BoundRugosity(bm, tracerArray);
        
        //fprintf(llogfp,"Integrate_Tracer_Variables: Time: %e box%d-%d Rugosity: %e\n", bm->dayt, bm->current_box, bm->current_layer, tracerArray[Rugosity_i]);
        
	}
    
}
/**
 *	\brief Set the localWCTracers and tracerFlux values.
 *
 */
void Copy_WC_Tracers(MSEBoxModel *bm, double *localWCTracers, double *localWCFlux, FILE *llogfp) {
	int i;

	for (i = 0; i < bm->ntracer; i++) {

		/* If this is a diagnostic tracer then set to 0 - this means the values written to the output files is
		 * actually the flux in this timestep
		 */
        localWCTracers[i] = 0.0;
		if (Fluxflag[i] == 1 || strlen(Varname[i]) == 0){
			// Do nothing here now
            //localWCTracers[i] = 0.0; Moved outside the if so it's initialised
		} else {
			localWCTracers[i] = bm->boxes[bm->current_box].tr[bm->current_layer][i];
		}

		/**
		if(it_count == 1 && strcmp(Varname[i], "Arsenic in wc") == 0)
			fprintf(bm->logFile, "WC - %d, box: %d, layer: %d, %s - bm->atEcologyModule->localTracer[i] = %.20e, fluxFlag = %d\n",
					i, bm->current_box, bm->current_layer, Varname[i], localWCTracers[i], Fluxflag[i]);
		*/

		/**
		if(localWCTracers[i] > 0 && bm->current_box == 1 && bm->current_layer == 1 && (i == 34 || i == 35))
			fprintf(bm->logFile, "box: %d, layer: %d, %s - bm->atEcologyModule->localTracer[%d] = %.20e, fluxFlag = %d\n",bm->current_box, bm->current_layer, Varname[i], i, localWCTracers[i], Fluxflag[i]);
		**/

		if (!(_finite(localWCTracers[i])) || isnan(localWCTracers[i])) {
			fprintf(llogfp,"Copy_WC_Tracers - day: %e, box: %d, layer: %d %s (%d) localpool set to: %e.\n",
					bm->dayt, bm->current_box, bm->current_layer, Varname[i], i, localWCTracers[i]);
			fflush(llogfp);
			quit("Copy_WC_Tracers - day: %e, box: %d, layer: %d %s (%d) localpool set to: %e.\n",
					bm->dayt, bm->current_box, bm->current_layer, Varname[i], i, localWCTracers[i]);
		}

		localWCFlux[i] = 0;
	}
}
/**
 *	\brief Set the localSed and sedFlux values.
 *
 **/
static void Copy_SED_Tracers(MSEBoxModel *bm, double *localTracers, double *localFlux) {
	int i;
	for (i = 0; i < bm->ntracer; i++) {
        if (Fluxflag[i + bm->ntracer] == 1) {
			localTracers[i] = 0.0;
        } else {
			localTracers[i] = bm->boxes[bm->current_box].sm.tr[bm->current_layer][i];
        }
		localFlux[i] = 0;

		/*if(it_count == 0 && localTracers[i] > 0)
			fprintf(bm->logFile, "SED - %s - bm->atEcologyModule->localSed[%d] = %.20e\n",Varname[i], i, localTracers[i]);*/

 		if (!(_finite(localTracers[i]))) {
            fflush(bm->logFile);
			quit("Copy_SED_Tracers - day: %e, box: %d, layer: %d %s (i: %d) localpool set to: %e.\n", bm->dayt, bm->current_box, bm->current_layer, Varname[i], i, localTracers[i]);
		}

	}
}
/**
 *	\brief Set the localEpi and epiFlux values.
 *
 */
static void Copy_EPI_Tracers(MSEBoxModel *bm, double *localTracers, double *localFlux) {
	int i;
	for (i = 0; i < bm->nepi; i++) {
        if (Fluxflag[i + 2 * bm ->ntracer] == 1) {
			localTracers[i] = 0.0;
        } else {
			localTracers[i] = bm->boxes[bm->current_box].epi[i];
        }
		localFlux[i] = 0;

//		if(it_count == 1 && localTracers[i] > 0)
//			fprintf(bm->logFile, "EPI -%d -  %s - bm->atEcologyModule->localEpi[i] = %.20e\n",i, Varname[i + 2 * numwcvar], localTracers[i]);


		if (!(_finite(localTracers[i]))) {
            fflush(bm->logFile);
			quit("Copy_EPI_Tracers - day: %e, box: %d, layer: %d %s localpool set to: %e.\n", bm->dayt, bm->current_box, bm->current_layer, Varname[i + 2 * numwcvar],
					localTracers[i]);
		}

	}
}
/**
 *	\brief Set the localICETracers and tracerFlux values.
 *
 */
static void Copy_ICE_Tracers(MSEBoxModel *bm, double *localICETracers, double *localICEFlux) {
	int i;
//	int icestart = 3 * numwcvar;

	for (i = 0; i < bm->ntracer; i++) {

		/* If this is a diagnostic tracer then set to 0 - this means the values written to the output files is
		 * actually the flux in this timestep
		 */
		if (Fluxflag[i] == 1)
			localICETracers[i] = 0.0;
		else{
//			localICETracers[i] = bm->boxes[bm->current_box].tr[bm->current_icelayer][i];
			localICETracers[i] = bm->boxes[bm->current_box].ice.tr[bm->current_icelayer][i];

		}

//		fprintf(bm->logFile, "%s - bm->atEcologyModule->localTracer[i] = %.20e\n",Varname[i], localWCTracers[i]);

		if (!(_finite(localICETracers[i]))) {
			printf("bm->boxes[%d].ice.tr[%d][%d] = %e\n", bm->current_box, bm->current_icelayer, i,  bm->boxes[bm->current_box].ice.tr[bm->current_icelayer][i]);

			quit("Copy_ICE_Tracers - day: %e, box: %d, layer: %d %s (%d) localpool set to: %e.\n",
					bm->dayt, bm->current_box, bm->current_icelayer, Varname[i], i, localICETracers[i]);
		}

		localICEFlux[i] = 0;
	}
}

/**
 *	\brief Set the localDiagTracers and localDiagFlux values.
 *
 */
static void Copy_Diag_Tracers(MSEBoxModel *bm, double *localTracers, double *localFlux) {
	int i;
	for (i = 0; i < bm->ndiag; i++) {
		if ((DiagFluxflag[i] == 1) || (bm->flagday && (!DiagFluxflag[i]) && (bm->current_layer > (bm->boxes[bm->current_box].nz - 2))))
			localTracers[i] = 0.0;
		else
			localTracers[i] = bm->boxes[bm->current_box].diagnost[i];
		localFlux[i] = 0;
	}
}
/**
 *	\brief Set the localFishTracers and localFishFlux values.
 *
 */
static void Copy_FishStat_Tracers(MSEBoxModel *bm, double *localTracers, double *localFlux) {
	int i;
	for (i = 0; i < bm->nfstat; i++) {
		localTracers[i] = bm->boxes[bm->current_box].fishstat[i];
		localFlux[i] = 0;
	}
}
/**
 *	\brief Check the tracer values in localWCTracers and
 *	bm->boxes[bm->current_box].tr[bm->current_layer].
 *
 *	If any values in either of these two arrays are not finite then an error
 *	message is written to the log file.
 *
 */
static void Check_Tracer_Values(MSEBoxModel *bm, double *localTracer, FILE *llogfp) {
	int i;
    for (i = 0; i < bm->ntracer; i++) {
        if (!(_finite(localTracer[i]))) {
            fprintf(llogfp, "day %e, box %d-%d %s (%d) starts:%.10f \n", bm->dayt, bm->current_box, bm->current_layer, Varname[i], i, localTracer[i]);
        }
    }
}
/**
 *	\brief Diagnostic integration. Use one-step method to integrate all diagnostic variables
 *
 */
static void Integrate_Diag_Variables(MSEBoxModel *bm, double *tracerArray, double *fluxArray, int numVariables, int offset, double tsz, double dtsz) {
	int i;

	for (i = 0; i < numVariables; i++) {
		/* Check for those groups notactive */
		if (bm->do_availflag && DiagBioflag[i + offset])
			if (!DiagActiveflag[i + offset])
				/* Skip to next variable */
				continue;

		/* Do once per day first */
        if (it_count == 1) {
            if (DiagTolflag[i + offset] >= 2) {
                tracerArray[i] = tracerArray[i] + fluxArray[i] * tsz;
            }
        }

		/* Do finer scale ones */
        if (DiagTolflag[i + offset] < 2) {
            tracerArray[i] = tracerArray[i] + fluxArray[i] * dtsz;
        }
	}
}

/**
 *
 *	\brief The time-step size controlling routine. This routine calls models of
 *	water-column, sediment or epibenthos and then determines a time step
 *	size to retain model stability.
 *
 *	This routine numerically integrates the dynamic model over 1 time step.
 *	An adaptive one-step difference method is used.
 *
 *	The ecologyModule structure is used to store the temporary tracer and
 *	flux values. The final values are then copied back to the final destinations
 *	in Box_Bio_Process after this function is called.
 *
 *	Inputs:
 *		FlagModel:	Flag of three ecological models,
 *		tsz:		Time step size,
 *		maxdeep:   Maximum number of water column layers in the model
 *		totaldeep: Maximum number of water column layers in this box
 *
 */
static void Adapt_Diff_Method(MSEBoxModel *bm, int flagModel, double tsz, BoxLayerValues *boxLayerInfo, FILE *llogfp) {
	double dtsz, time_left;
	double dzz, numsec = 86400.0;
	int i;
    
    if (verbose > 1) {
        printf("Time: %e Doing adaptive timestep for box: %d, layer %d (Water Column) - flagModel : %d \n", bm->dayt, bm->current_box, bm->current_layer, flagModel);
        fprintf(llogfp, "\nTime: %e Doing adaptive timestep for box: %d, layer %d (Water Column) - flagModel: %d\n", bm->dayt, bm->current_box, bm->current_layer, flagModel);
	}
    
	/* Initialize adaptive time step dtsz, time_left*/
	dtsz = tsz;
	time_left = dtsz;

    /* Assign initial values to local vectors in boxLayerInfo and check the tracer values.
	 * This set the boxLayerInfo tracer arrays to the tracer values stored in the
	 * bm->boxes.tr arrays. In effect these are the values that were calculated at the
	 * end of the previous time step.
	 */
	if(flagModel < 4){
		Copy_WC_Tracers(bm, boxLayerInfo->localWCTracers, boxLayerInfo->localWCFlux, llogfp);

		if (flagModel > 1) {
			Copy_SED_Tracers(bm, boxLayerInfo->localSEDTracers, boxLayerInfo->localSEDFlux);
		}

		if (flagModel > 2) {
			if(bm->ice_on){
				Copy_ICE_Tracers(bm, boxLayerInfo->localICETracers, boxLayerInfo->localICEFlux);
			}
			Copy_EPI_Tracers(bm, boxLayerInfo->localEPITracers, boxLayerInfo->localEPIFlux);

			/* Add old code to ensure old models will get the same results */
			if(bm->include_atmosphere == FALSE){
				/** Get NO3 & NH3 values from atmosphare on the surface cell **/
				dzz = bm->boxes[bm->current_box].dz[0];
				if (dzz != 0.0) {
					boxLayerInfo->localWCTracers[NO3_i] = boxLayerInfo->localWCTracers[NO3_i] + 0.685 / dzz * tsz / numsec;
					boxLayerInfo->localWCTracers[NH3_i] = boxLayerInfo->localWCTracers[NH3_i] + 0.685 / dzz * tsz / numsec;
				}
			}
		}
		if(bm->ice_on){
			/* If this is the top water column layers setup ice tracers so we can get predation */
			if (bm->current_layer == (bm->boxes[bm->current_box].nz - 1)) {
				Copy_ICE_Tracers(bm, boxLayerInfo->localICETracers, boxLayerInfo->localICEFlux);
			}
		}

		Copy_Diag_Tracers(bm, boxLayerInfo->localDiagTracers, boxLayerInfo->localDiagFlux);
		Copy_FishStat_Tracers(bm, boxLayerInfo->localFishTracers, boxLayerInfo->localFishFlux);
	}else{
		Copy_ICE_Tracers(bm, boxLayerInfo->localICETracers, boxLayerInfo->localICEFlux);
	}

	it_count = 0;
    
    /* Loop for testing model results */
	while (time_left > 0) {
		it_count = it_count + 1;

		/* Check the tracer values */
		Check_Tracer_Values(bm, boxLayerInfo->localWCTracers, llogfp);
		if (flagModel > 1)
			Check_Tracer_Values(bm, boxLayerInfo->localSEDTracers, llogfp);

		if (flagModel > 2) {
			Check_Tracer_Values(bm, boxLayerInfo->localEPITracers, llogfp);
		}

        /* Get Mdt - the dt to multiply the ongoingM tracker by.
           Array will get too large if try and do it the same way
           as for estimates done in UpdateMortEsts */
		UpdateMdt(bm, dtsz, llogfp);
        
        /* Run the box and get the next time step */
		switch (flagModel) {
		case 1:
			Water_Column_Box(bm, dtsz, boxLayerInfo, llogfp);
			Get_Time_Change(bm, llogfp, WC, &dtsz, &time_left, boxLayerInfo);
			break;
		case 2:
			Sediment_Box(bm, dtsz, boxLayerInfo, llogfp);
			Get_Time_Change(bm, llogfp, SED, &dtsz, &time_left, boxLayerInfo);
			break;
		case 3:
			Epibenthic_Box(bm, dtsz, boxLayerInfo, llogfp);
			Get_Time_Change(bm, llogfp, EPIFAUNA, &dtsz, &time_left, boxLayerInfo);
			break;
		case 4:
			Ice_Box(bm, dtsz, boxLayerInfo, llogfp);
			Get_Time_Change(bm, llogfp, ICE_BASED, &dtsz, &time_left, boxLayerInfo);
			break;
		default:
			quit("FlagModel %d not recognised in Adapt_Diff_Method\n", flagModel);
			break;
		}
        bm->dtsz_stored = dtsz;
        
        if(isnan(bm->dtsz_stored)){
            quit("dtsz_stored returned nan - dtsz: %e, time_left: %e\n", dtsz, time_left);
        }
        
        if (verbose > 1)
			fprintf(llogfp, "dtsz: %e, time_left: %e\n", dtsz, time_left);

        /** Update mortality estimates */
		Accumulate_Mortality_Estimates(bm, dtsz, llogfp);

		/** Dynamic variable integration **/
		switch (flagModel) {
		case 1:
 			Integrate_Tracer_Variables(bm, WC, boxLayerInfo->localWCTracers, boxLayerInfo->localWCFlux, bm->ntracer, 0, tsz, dtsz, llogfp);
			if(bm->track_atomic_ratio == TRUE){
				Integrate_Ratio_Variables(bm, boxLayerInfo->localWCTracers, boxLayerInfo->localWCFlux, tsz, dtsz, WC);
			}
			if(bm->ice_on){
				/* If this is the top water column layers setup ice tracers so we can get predation */
				if (bm->current_layer == (bm->boxes[bm->current_box].nz - 1)) {

				}
			}
			break;
		case 2:
			Integrate_Tracer_Variables(bm, SED,boxLayerInfo->localSEDTracers, boxLayerInfo->localSEDFlux, bm->ntracer, bm->ntracer, tsz, dtsz, llogfp);
			if(bm->track_atomic_ratio == TRUE){
				Integrate_Ratio_Variables(bm, boxLayerInfo->localSEDTracers, boxLayerInfo->localSEDFlux, tsz, dtsz, SED);
			}
			break;
		case 3:
			Integrate_Tracer_Variables(bm, WC, boxLayerInfo->localWCTracers, boxLayerInfo->localWCFlux, bm->ntracer, 0, tsz, dtsz, llogfp);
			Integrate_Tracer_Variables(bm, SED, boxLayerInfo->localSEDTracers, boxLayerInfo->localSEDFlux, bm->ntracer, bm->ntracer, tsz, dtsz, llogfp);
			Integrate_Tracer_Variables(bm, EPIFAUNA, boxLayerInfo->localEPITracers, boxLayerInfo->localEPIFlux, bm->nepi, 2 * bm->ntracer, tsz, dtsz, llogfp);

			if(bm->track_atomic_ratio == TRUE){
				Integrate_Ratio_Variables(bm, boxLayerInfo->localWCTracers, boxLayerInfo->localWCFlux, tsz, dtsz, WC);
				Integrate_Ratio_Variables(bm, boxLayerInfo->localSEDTracers, boxLayerInfo->localSEDFlux, tsz, dtsz, SED);
				Integrate_Ratio_Variables(bm, boxLayerInfo->localEPITracers, boxLayerInfo->localEPIFlux, tsz, dtsz, EPIFAUNA);
			}
			break;
		case 4:
			Integrate_Tracer_Variables(bm, ICE_BASED, boxLayerInfo->localICETracers, boxLayerInfo->localICEFlux, bm->ntracer, 0, tsz, dtsz, llogfp);

			break;
		default:
			quit("FlagModel %d not recognised in Adapt_Diff_Method\n", flagModel);
			break;
		}

		/** Diagnostic integration **/
		Integrate_Diag_Variables(bm, boxLayerInfo->localDiagTracers, boxLayerInfo->localDiagFlux, numdiagvar, 0, tsz, dtsz);
		Integrate_Diag_Variables(bm, boxLayerInfo->localFishTracers, boxLayerInfo->localFishFlux, numfstatvar, numdiagvar, tsz, dtsz);

        /* setup dtsz for next iteration */
		if (time_left > 100.0)
			dtsz = time_left;

		/* Degugging aids */
		if (bm->ecotest == 5) {
			quit("One iteration done. Current time_left is %e\n", time_left);
		}
		/* Reset the fishFlux and diagFlux flux values to zero */
		for (i = 0; i < numdiagvar; i++)
			boxLayerInfo->localDiagFlux[i] = 0.0;
		for (i = 0; i < numfstatvar; i++)
			boxLayerInfo->localFishFlux[i] = 0.0;

	} /* End of while loop */
    
	//if (bm->debug_it && (bm->debug != debug_econeffort))
	//	fprintf(llogfp, "Time: %e, box:%d-%d it_count: %d\n", bm->dayt, bm->current_box, bm->current_layer, it_count);

	return;
}

/***************************************************************************//*
 *	\brief Updating of the Mdt for use in calcTrackedMort and calcTrackedPredMort
 *
 *****************************************************************************/

static void UpdateMdt(MSEBoxModel *bm, double dtsz, FILE *llogfp)
{
	int sp_id = 0;

	for(sp_id=0; sp_id<bm->K_num_tot_sp; sp_id++){
		if((FunctGroupArray[sp_id].isOncePerDt == TRUE) && (it_count == 1) && (bm->dayt < 1.0)){
			FunctGroupArray[sp_id].speciesParams[Mdt_id] = bm->dt; // Only update in the first day as no need to continually update a fixed value
		} else {
			FunctGroupArray[sp_id].speciesParams[Mdt_id] = dtsz;
		}
	}
}

/***************************************************************************//*
 *	\brief Update the mortality estimates. This is called once per adaptive timestep
 *	to add the mortality values from the timestep to the cumulative total.
 *
 */

static void Accumulate_Mortality_Estimates(MSEBoxModel *bm, double dtsz, FILE *llogfp) {
	int prey, pred;

	for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
        /* Add the current entry to the cumulative total - correcting for the period of time
         the mortality has been calculated for (in most cases one time step, but in some it
         will be one adaptive time step)

         Also reset for next iteration.
         */
		if ((FunctGroupArray[prey].isOncePerDt == TRUE) && (it_count == 1)) {
			for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
				bm->calcMnumPerPred[prey][pred][expect_id] += (bm->calcMnumPerPred[prey][pred][current_id] * bm->dt);
				bm->calcMnumPerPred[prey][pred][current_id] = 0;
			}

			/* Update the linear mortality, quad mortality and mortality due to explict birds and mammals */
			bm->calcMLinearMort[prey][expect_id] += bm->calcMLinearMort[prey][current_id] * bm->dt;
			bm->calcMQuadMort[prey][expect_id] += bm->calcMQuadMort[prey][current_id] * bm->dt;
			bm->calcMPredMort[prey][expect_id] += bm->calcMPredMort[prey][current_id] * bm->dt;
			bm->calcELinearMort[prey][expect_id] += bm->calcELinearMort[prey][current_id] * bm->dt;

			bm->calcMLinearMort[prey][current_id] = 0;
			bm->calcELinearMort[prey][current_id] = 0;
			bm->calcMQuadMort[prey][current_id] = 0;
			bm->calcMPredMort[prey][current_id] = 0;

			bm->calcMnum[prey][expect_id] += bm->calcMnum[prey][current_id] * bm->dt;
			bm->calcMnum[prey][current_id] = 0;
			bm->calcFnum[prey][expect_id] += bm->calcFnum[prey][current_id] * bm->dt;
			bm->calcFnum[prey][current_id] = 0;

		} else {
			for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
				bm->calcMnumPerPred[prey][pred][expect_id] += bm->calcMnumPerPred[prey][pred][current_id] * dtsz;
				bm->calcMnumPerPred[prey][pred][current_id] = 0;
			}

            bm->calcMLinearMort[prey][expect_id] += bm->calcMLinearMort[prey][current_id] * dtsz;
            bm->calcELinearMort[prey][expect_id] += bm->calcELinearMort[prey][current_id] * dtsz;
            bm->calcMQuadMort[prey][expect_id] += bm->calcMQuadMort[prey][current_id] * dtsz;
            bm->calcMPredMort[prey][expect_id] += bm->calcMPredMort[prey][current_id] * dtsz;
            
            bm->calcMLinearMort[prey][current_id] = 0;
			bm->calcELinearMort[prey][current_id] = 0;
			bm->calcMQuadMort[prey][current_id] = 0;
			bm->calcMPredMort[prey][current_id] = 0;

			bm->calcMnum[prey][expect_id] += bm->calcMnum[prey][current_id] * dtsz;
			bm->calcMnum[prey][current_id] = 0;
			if (it_count == 1) { /* Fishing only done once per timestep */
				bm->calcFnum[prey][expect_id] += bm->calcFnum[prey][current_id] * bm->dt;
				bm->calcFnum[prey][current_id] = 0;
			}
		}
	}

	return;
}

/*******************************************************************//**
Routine to re-initialise the diet data per box.
*********************************************************************/
void ReInitDietData(MSEBoxModel *bm, int b) {
    int k, sp, cohort, prey;
    
    for (k = 0; k < bm->wcnz+bm->sednz; k++) {
        for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
            if(FunctGroupArray[sp].speciesParams[active_id]) {
                for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes * FunctGroupArray[sp].numStocks; cohort++) {
                    for (prey=0; prey < bm->K_num_tot_sp; prey++) {
                        bm->totDiet[b][k][sp][cohort][prey] = 0.0;
                    }
                }
            }
        }
    }
    
}
