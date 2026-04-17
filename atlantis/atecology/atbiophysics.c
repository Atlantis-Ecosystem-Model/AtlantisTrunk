/**
 \file
 \brief C file for running the environmental conditions (light, temperature etc)
 \ingroup atEcology

 File:		atbiophysics.c
 Created:	5/10/2005
 Author:		Beth Fulton,   CSIRO Marine Laboratories,  2005
 Based on work by Ouyang and Fulton for Port Phillip Bay
 Purpose:	C file for running the environmental conditions (light,
 temperature etc) for use in the ecological models in south east.
 Note:
 Revisions: 	5/10/2005 Created from atbiology.c (moved biophysical routines
 to this file)


 02-02-2009 Beth Fulton
 Changed the variable called small to small_num.

 06-04-2009 Bec Gorton
 Changed the Kd calculation in Box_Light_Process to add the biomass of the
 phytoplankton in the same order as the 'new' code so that floating point adding
 issues are not obvious.

 20-07-2009 Bec Gorton
 Fixed a bug in the Box_Light_Process code. This was missing the line:
 Y1 = (double*) pBox->sm.tr[pBox->sm.topk];
 to set the Y1 value to point to teh top of the sediment layer.
 Thus the light value at the last water column (0) was being over written.

 23-07-2009 Bec Gorton
 Uncommented out the new light attenuation due to sediment in the water column coefficient in
 the light process function.

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
#include <assert.h>
#include <math.h>
#include <sjwlib.h>
#include "atecology.h"

/**
 *
 * \brief Light calculations - Note that all degrees must be in radians
 *
 */
void Calculate_Box_Light(MSEBoxModel *bm, Box *pBox, FILE *llogfp) {
	double deg2rad = 3.1415926 / 180.0;

	/* Location in latitude and longitude */
	double latitude = Projection_GetLatitude(bm, pBox->inside.x, pBox->inside.y);
	double longitude = Projection_GetLongitude(bm, pBox->inside.x, pBox->inside.y);
	double LT = bm->timeleft * 24.0;  // Local time

	/* Day corrections */
	double dn = (double)(bm->TofY);
	double DnC = (360.0 * (dn - 81.0) / 365.0) * deg2rad;

	/* Declination */
	double declination = asin(sin(23.45 * deg2rad) * sin(DnC));

	/* Hour Angle (HRA) */
	double delta_T = longitude * 24.0 / 360.0;   // Difference in hours from location to Greenwich Mean Time - assumes longitude >0 for east, <0 for west
	double LocalStdTime = 15 * delta_T;
	double EoT = 9.87 * sin(2.0 * DnC) - 7.53 * cos(DnC) - 1.5 * sin(DnC);
	double TimeCorrection = 4.0 * (longitude - LocalStdTime) + EoT;   // Note stepped through all the calculations for this form source material, but really LocalStdTime = longitude due to cancellation so TimeCorrection really just equals EoT
	double LocalSolarTime = LT + TimeCorrection / 60.0;
	double HRA = 15 * (LocalSolarTime - 12.0) * deg2rad;

	/* Elevation */
	double elevation = max(0.0,(1.0/deg2rad * asin(sin(declination) * sin(latitude * deg2rad) + cos(declination) * cos(latitude * deg2rad) * cos(HRA))));

	/* Zenith */
	double zenith = 90 - elevation;

	/* Intensity - currently assumed to be at sea level */
	double height = 0.0;   // Height above sea level TODO: Actually have this as real height eventually
	double AM = 1.0 / (cos(zenith * deg2rad) + 0.50572 * pow((96.07995 - zenith),(-1.6364)));  // Air mass including curvature of the earth
	double step0 = pow(AM,0.678);
	double step1 = pow(0.7,step0);
	double step2 = 0.14 * height;
	double intensity = 1.353 * ((1.0 - step2) * step1 + step2); // Sunlight intensity at sea level

	/* Exterterrestrial shortwave radiation levels */
	//double Io = 1367.0 * (1.0 + 0.034 * cos((360.0 * dn / 365.0) * deg2rad)) * bm->swr_scalar;
    double Io = bm->swr_const * (1.0 + bm->swr_cos_coefft * cos((360.0 * dn / 365.0 - bm->swr_cos_offset) * deg2rad)) * bm->swr_scalar;

	/* Available irradiance (PAR) - level available from Griffiths et al ice algal model */
	pBox->swr = intensity * Io;

	if (verbose > 0)
		printf("Calculating light\n");

	return;
}

/**
 *
 *	\brief This subroutine calculates light intensity on surfaces of all
 *  cells and on the bottom of the bottom cell in water column
 *
 *	Light falls off exponentially with depth.
 *	The light attenuation coefficient Kd is given by:
 *
 *	Kd = k_w + k_P * (PL + PS) + k_DON * DON + k_DL * (DL + DR) + k_IS + SED * k_sed
 *
 *	In any given cell, with an upper light intensity Itop, and thickness dz,
 *	the bottom light intensity is:
 *
 *	Ibot = Itop * exp(-Kd * dz)
 *
 *	The average light intensity is:
 *
 *	Iav = (Itop - Ibot) / (Kd * dz)
 *
 *	NB. The light intensity for sediment layers below the top sediment layer
 *	should be zero.
 */
void Box_Light_Process(MSEBoxModel *bm, Box *pBox, FILE *llogfp) {
	double DepthBotm = 0; /* Depth from water surface to the current cell bottom */
	int ij = 0;
	double *Y1 = 0; /*Pointer to initial variables of a WC cell */
	double Kd = 0; /* Light Attenuation */
	double Io = 0; /* Defult light depth - not used yet so set to zero */
	double dz = 0; /* depth of a cell */
	double lighttop = 0, lightbot = 0, lightsurf = 0, midwater_dz = 0;
	double DON, DL, DR;
	double totdepth = pBox->botz;
	double phytoBiomass = 0.0;
	int guild, sp;
	double layerSEDValue = 0.0;

	int numwclayer = pBox->nz; /* Number of cells in WC */
	int numsmlayer = pBox->sm.nz; /* Number of cells in Sm */

	double addepth, adfactor, derivative, ddepth, KI_aint;
	//double KI_PL_aint, KI_PS_aint, KI_MB_aint, KI_DF_aint;

	if (verbose > 1) {
		printf("Light levels are assessed\n");
	}

	/* 	Io and flagseason are global data */

	/*	Get light intensity on the surface of a box from transport module,
	 or from a constant data Io.
	 */
	if (bm->flagseason == 1){
		lighttop = pBox->swr;

		if(bm->ice_on == TRUE){
			if(pBox->ice.currentnz > 0){
				lighttop = pBox->tot_ice_prop * pBox->ice.bottom_light + (1.0 - pBox->tot_ice_prop ) *lighttop;
			}
		}
	}else {
		/* Default light level - FIX put this in if required by some one */
		quit("No non-seasonal light options available as yet. Either make the model seasonal (flagseason = 1 in biol.prm) or code in Io\n");
		lighttop = Io;
	}

	/* If just using blanket night-day settings then set night time irradiance to low value */
	if ((!bm->flagday) && (!bm->lim_sun_hours)) {
		lighttop = 0.00000001;
	} else {
		/* Do nothing here and calculate light as if daytime
		 as prop_light_time will handle truncation of light
		 hours for purposes of primary production */
	}

	lightsurf = lighttop;
	/* Calculate thickness and light intensity on surfaces of water column cells */
	/* Loop for all water column cells */
	for (ij = numwclayer - 1; ij > -1; ij--) {
		Y1 = (double*) pBox->tr[ij];

		DON = Y1[DON_i];
		DL = Y1[FunctGroupArray[LabDetIndex].totNTracers[0]];
		DR = Y1[FunctGroupArray[RefDetIndex].totNTracers[0]];

		phytoBiomass = 0.0;
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if(FunctGroupArray[guild].speciesParams[flag_id]){
				if (FunctGroupArray[guild].groupType == LG_PHY || FunctGroupArray[guild].groupType == SM_PHY || FunctGroupArray[guild].groupType == DINOFLAG
						|| FunctGroupArray[guild].groupType == MICROPHTYBENTHOS) {
					phytoBiomass += Y1[FunctGroupArray[guild].totNTracers[0]];
					if(!(_finite(Y1[FunctGroupArray[guild].totNTracers[0]]))){
						fprintf(bm->logFile, "Box_Light_Process - box: %d, layer: %d Phyto biomass value is not finite. Group %s\n",
											pBox->n, ij, FunctGroupArray[guild].groupCode);
						quit( "Box_Light_Process - box: %d, layer: %d Phyto biomass value is not finite. Group %s\n",
								pBox->n, ij, FunctGroupArray[guild].groupCode);
					}
				}
			}
		}

		if (!(_finite(phytoBiomass))) {
			fprintf(bm->logFile, "Box_Light_Process - box: %d, layer: %d Invalid Light value calculated. PhytoBiomass is not finite.\n",
								pBox->n, ij);
			quit("Box_Light_Process - box: %d, layer: %d Invalid Light value calculated. PhytoBiomass is not finite.\n",
					pBox->n, ij);
		}
		/*
		 * Calculate the light attenuation due to sediments in the water column
		 * If flagIsEstuary is false this value will just be 0.0.
		 *
		 * This applies to case II waters.
		 *
		 * Means we shouldn't really be using remote sensed chla data.
		 */
		if (bm->flagIsEstuary){
			layerSEDValue = Y1[SED_i] * k_SED;

			if (verbose > 2) {
				fprintf(llogfp, "Calculating light coefficient due to sediment box %d:%d - k_SED = %e, Sed Tracer = %e, sediment light coefficient = %e\n",
					pBox->n, ij, k_SED, Y1[SED_i], layerSEDValue);
			}
		}

		/*
		 Depth related water absorption of light to reflect difference between
		 coastal k_w and open ocean k_w */
		if (totdepth < k_w_depth)  // Noting that totdepth and k_w_depth are negative numbers
			Kd = k_w_deep + k_PN * phytoBiomass + k_DON * DON + k_DL * (DL + DR) + k_IS + layerSEDValue;
		else{
			Kd = k_w_shallow + k_PN * phytoBiomass + k_DON * DON + k_DL * (DL + DR) + k_IS + layerSEDValue;
		}
		if (bm->debug == debug_prey_biology_process && bm->dayt >= bm->checkstart && bm->dayt < bm->checkstop) {
			fprintf(llogfp, "phytoBiomass = %.20e, Kd = %.20e, k_PN = %.20e, k_DON = %.20e, DON = %.20e, k_DL = %.20e\n", phytoBiomass, Kd, k_PN, k_DON, DON, k_DL);
			fprintf(bm->logFile, "DL = %.20e, DR = %.20e, k_IS = %.20e, layerSEDValue = %.20e\n", DL, DR, k_IS, layerSEDValue);
		}

		DepthBotm = -pBox->gridz[ij];
		dz = pBox->dz[ij];

		if (ij == numwclayer - 1)
			midwater_dz = DepthBotm / 2.0;

		if (bm->flaglight) {
			/**
			 Optional - Light adaptation so that production is not so tightly bound
			 irradiance and does capture algae's ability to adapt to a wide range of light
			 conditions - adaptation calculated here.
			 **/
			ddepth = DepthBotm - dz / 2.0;
			addepth = min(ddepth,K_addepth);
			adfactor = exp(-Kd * addepth);
			derivative = max(pBox->swr * KI_avail * adfactor,KIOP_min);

			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if(FunctGroupArray[sp].speciesParams[flag_id]){
					if (FunctGroupArray[sp].groupType == MICROPHTYBENTHOS || FunctGroupArray[sp].groupType == DINOFLAG || FunctGroupArray[sp].groupType
							== LG_PHY || FunctGroupArray[sp].groupType == SM_PHY) {
						KI_aint = derivative - Y1[FunctGroupArray[sp].Light_Adaptn_TracerID];
						Y1[FunctGroupArray[sp].Light_Adaptn_TracerID] += KIOP_shift * KI_aint;
					}
				}
			}
		}

		lightbot = lighttop * exp(-Kd * dz);

		/* The neglible value is used at night and in very deep water */

		switch (bm->flaglightopt) {
		case light_orig:
		case light_mid:
			if (dz != 0) {
				Y1[Light_i] = max(small_num,(lighttop-lightbot)/(Kd*dz));
			} else
				Y1[Light_i] = max(small_num,lighttop);

			break;
		case light_bott:
			Y1[Light_i] = max(small_num,lightbot);
			break;
		case light_top:
			Y1[Light_i] = max(small_num,lighttop);
			break;
		default:
			quit("No such flaglightopt setting\n");
			break;
		}

		/* Check we have calculated a valid light values */
		if (!(_finite(Y1[Light_i]))) {
			fprintf(bm->logFile, "Box_Light_Process - box: %d, layer: %d Invalid Light value calculated.\n",
					pBox->n, ij);
			fprintf(bm->logFile, "bm->flaglightopt = %d, lighttop = %e, lightbot = %e\n", bm->flaglightopt, lighttop, lightbot);
			quit("Box_Light_Process - box: %d, layer: %d Invalid Light value calculated.\n",
					pBox->n, ij);
		}
		lighttop = lightbot;
        
        if(bm->flagday)
            Y1[DayLight_i] = Y1[Light_i];  // Store DayLight values

	}

	/* Save light intensity on surface of the first sediment cell. */
	Y1 = (double*) pBox->sm.tr[pBox->sm.topk];
	switch (bm->flaglightopt) {
	case light_orig:
		lightbot = lightsurf * exp(-Kd * midwater_dz);

		/* For estuarine and deep water use the surface light levels,
		 for the coastal water use the midwater light levels to reflect
		 fact some coastal reefs are in shallow enough water to recieve
		 some light. This is only really appropriate for use where
		 the shelf boxes are set quite deep with minimal layering */

		if (totdepth < k_w_cdepth)
			Y1[Light_i] = max(small_num,lighttop);
		else {
			if (totdepth > k_w_depth)
				Y1[Light_i] = max(small_num,lighttop);
			else
				Y1[Light_i] = max(small_num,lightbot);
		}
		break;
	case light_bott: /* Light from bottom of water column */
		Y1[Light_i] = max(small_num,lightbot);
		break;
	case light_top:
	case light_mid:
		/* Light at top of sediment (same as lightbot as
		 lighttop set = lightbot at end of watercolumn
		 layer calcs */
		Y1[Light_i] = max(small_num,lighttop);
		break;
	default:
		quit("No such flaglightopt setting\n");
		break;
	}

	if (!(_finite(Y1[Light_i]))) {
		fprintf(bm->logFile, "Box_Light_Process - box: %d. Invalid light value calculated for the sediment layer.\n",
								pBox->n);
				quit("Box_Light_Process - box: %d. Invalid light value calculated for the sediment layer.\n",
						pBox->n);
			}

	/* from the second sediment cell, LightIntensity is 0. */
	for (ij = pBox->sm.topk + 1; ij < numsmlayer; ij++) {
		Y1 = (double*) pBox->sm.tr[ij];
		Y1[Light_i] = 0.0;
	}
	return;
}


/**
 *    \brief make sure nutrients are all >= 0
 *
 */
void Box_Nutrient_Check(MSEBoxModel *bm, Box *pBox) {
    int ij;
    double *Y1, /* Pointer to initial variables of a water column cell */
    *Y2; /* Pointer to initial variables of a sediment cell */

    if (bm->flag_sanity_check) {
    
        // Water column tracers
        for (ij = 0; ij < pBox->nz; ij++){
            Y1 = (double*) pBox->tr[ij];
            if(Y1[NH3_i] < 0) Y1[NH3_i] = 0;
            if(Y1[NO3_i] < 0) Y1[NO3_i] = 0;
            if(Y1[Light_i] < 0) Y1[Light_i] = 0;
            if(Y1[Si_i] < 0) Y1[Si_i] = 0;
            if(Y1[MicroNut_i] < 0) Y1[MicroNut_i] = 0;
        }
        
        // Sediment tracers
        for(ij = 0; ij < pBox->sm.nz; ij++){
            Y2 = (double*) pBox->sm.tr[ij];
            if(Y2[NH3_i] < 0) Y2[NH3_i] = 0;
            if(Y2[NO3_i] < 0) Y2[NO3_i] = 0;
            if(Y2[Light_i] < 0) Y2[Light_i] = 0;
            if(Y2[Si_i] < 0) Y2[Si_i] = 0;
            if(Y2[MicroNut_i] < 0) Y2[MicroNut_i] = 0;
        }
        
    }
    return;
}

/**
 *	\brief Calculate O2depth
 *
 *	using
 *
 *	O2depth = 2 * [O2]sed * sediment_layer_thickness / [O2]bottom_H20
 *
 *	Based on formulation used by John Parlsow for Moreton Bay work.
 *
 */
void Box_O2_Depth_Process(MSEBoxModel *bm, Box *pBox) {
	int ij = 0;
	double smLayerThick; /* Thickness of sediment layer */
	double *Y1, /* Pointer to initial variables of a water column cell */
	*Y2; /* Pointer to initial variables of a sediment cell */
	double O2_bottomH2O, O2_sed, O2activity, irrigenh, turbenh, newO2;

	if (verbose > 1)
		printf("Oxygen levels are assessed\n");

	/* Check the oxygen levels are valid at each point */

	/* Identify the current box */
	bm->current_box = pBox->n;

	/* Get value of oxygen in the bottom water */
	ij = 0;
	Y1 = (double*) pBox->tr[ij];

	O2_bottomH2O = Y1[Oxygen_i];

	if(O2_bottomH2O == 0) {
		//quit("Oxygen is zero in bottom layer of box %d\n", pBox->n);
        fprintf(bm->logFile,"Time %e box%d Oxygen is zero in bottom layer\n", bm->dayt, pBox->n);
		warn("Oxygen is zero in bottom layer of box %d\n", pBox->n);
        O2_bottomH2O = small_num;
    }

	/* Get value of oxygen in the sediment */
	ij = pBox->sm.topk;
	Y2 = (double*) pBox->sm.tr[ij];
	smLayerThick = -pBox->sm.gridz[ij + 1];
	if(smLayerThick <= 0)
		quit("Sediment layer thickness is zero in box %d\n", pBox->n);

	O2_sed = Y2[Oxygen_i];

	/* Update the sediment oxygen levels */
	irrigenh = pBox->sm.irrigenh;
	turbenh = pBox->sm.turbenh;
	O2activity = min(1.0,max(irrigenh,turbenh));
	newO2 = (O2_sed * O2activity + O2_bottomH2O) / 2.0;
	O2_sed = newO2;

	O2depth = 2.0 * O2_sed * smLayerThick / (O2_bottomH2O + small_num);

	if (O2depth < 0.0){
		O2depth = 0.0;
		warn("Setting oxygen to 0 in box %d\n", pBox->n);
	}

	if (O2depth > 10.0)
		O2depth = 10.0;

	/* Update new oxygen concentration and horizon */
	bm->boxes[bm->current_box].sm.oxdepth = O2depth;
	Y2[Oxygen_i] = O2_sed;

	return;

}

/**
 *
 *	\brief Calculate current temperature at current depth at current time
 *
 */
void Properties_At_Depth(MSEBoxModel *bm, Box *pBox, double dayt, int numwclayer, double cdepth, int clayer, double midpoint,
		int flagmodel, FILE *llogfp) {
	int checked_already;
	double surfH2Otemp, deepH2Otemp, basetemp, T_scale;
	double base_pH;
	double *Y1; /* Pointer to initial variables of a WC or SED cell */


	/* make sure we grab the tracers for the correct layer type */
	if(flagmodel == WC)
		Y1 = (double*) pBox->tr[clayer];
	else
		Y1 = (double*) pBox->sm.tr[clayer];


	checked_already = bm->checkedalready[pBox->n][clayer + flagmodel * bm->wcnz];
    
	/* Regression and southern hemisphere */
	if ((bm->flagmodeltemp == 1) && (!bm->flaghemisphere)) {
		/* Determine sea surface temperature using linear regression
		 equations determined from CARS data. Assumes midpoint is
		 the value in metres of the latitude (-ve metres as -ve latitude) */
		if (cdepth < -250.0) {
			basetemp = 0.000005 * midpoint + 38.158;
		} else {
			basetemp = 0.000006 * midpoint + 38.365;
		}
	} else if (bm->flagmodeltemp == 2) {
		basetemp = bm->baseline_temp;
	} else {
		basetemp = Y1[Temp_i];
	}

	if (!checked_already) { /* Only do calculations if haven't already */

		/* if this is the sediment use the Tchange value in the first water column layer */
		if(flagmodel == SED){
			T_scale = bm->TempLayerScalar[0];
		}else{
			T_scale = bm->TempLayerScalar[clayer];
		}

		//fprintf(bm->logFile, "Time %e, box %d, layer %d, T_scale = %e\n", bm->dayt, pBox->n, clayer, T_scale);

		/* If appropriate use a regression to define the seasonal signal
		 Note that would normally convert degrees to radians as 2pi/360, but as got 365 days
		 in a revolution (one turn through the calendar) then divide by 365 instead. Also offset
		 the peak value by 31 days as don't peak summer heat in the water column until after mid summer
		 (so in February not January).
		 */
		if (bm->flagseason && bm->flagmodeltemp > 0) {
			if (!bm->flaghemisphere) // Southern hemisphere
				surfH2Otemp = (bm->temp_amplitude * cos(2.0 * 3.1415926 * (dayt - 31.0) / 365.0) + basetemp);
			else
				// Northern hemisphere - allow for variation in season.
				surfH2Otemp = (bm->temp_amplitude * sin(2.0 * 3.1415926 * (dayt - 124.0) / 365.0) + basetemp);

			/* Scale these values - data values are scaled on read-in */
			surfH2Otemp = surfH2Otemp * T_scale;
		} else { // From data
			/* If we are using flagmodeltemp == 0 then the temperature data should be read in from data files - any temperature scaling is done then */
			if(bm->flagmodeltemp == 0)
				surfH2Otemp = basetemp ;
			else
				surfH2Otemp = basetemp * T_scale;
		}

//		fprintf(llogfp,"Time: %e, box: %d, basetemp: %e, flagmodeltemp: %d, bm->flagseason: %d, surfH2Otemp: %e, T_scale: %e\n",
//			bm->dayt, pBox->n, basetemp, bm->flagmodeltemp, bm->flagseason, surfH2Otemp, T_scale);


		H2Otemp = surfH2Otemp;

		//fprintf(bm->logFile, "Time %e, box %d, layer %d, H2Otemp = %e\n", bm->dayt, pBox->n, clayer, H2Otemp);


		/* If need be determine correction for depth */
		if ((numwclayer > 1)  && (bm->flagmodeltemp > 0)){
			/* Shape of this depth temperature relationship is due to fitting trendline
			 to data from Bax and Williams 2000 SE ecosystem report and Parlsow et al 1996
			 Tasmanian slope trophodynamics temperature-at-depth data (r-squared = 0.98) */
			deepH2Otemp = surfH2Otemp * exp(0.0011 * cell_depth);

			H2Otemp = deepH2Otemp;
		}

		/* Save water temperature */
		Y1[Temp_i] = H2Otemp;

        if(Y1[Temp_i] > 40){
			fprintf(stderr,"Time: %e, box: %d, basetemp: %e, flagmodeltemp: %d, bm->flagseason: %d, surfH2Otemp: %e, T_scale: %e\n",
			bm->dayt, pBox->n, basetemp, bm->flagmodeltemp, bm->flagseason, surfH2Otemp, T_scale);

			warn("Your ocean is a bit warm. Time %e, box %d, Temperature is %e\n", bm->dayt, pBox->n, H2Otemp);
		}

		/* Salinity is no longer scaled now - this is done when its read in *.
		 */
		current_SALT = Y1[Salinity_i];
        
        if (bm->track_pH) {
            /* Get pH values  - if required values are scaled in this function*/
            base_pH = pH_At_Depth(bm, pBox, dayt, numwclayer, cdepth, clayer, midpoint, flagmodel, llogfp);

            Y1[pH_i] = base_pH;
        } else {
            base_pH = 8.0;
        }
        current_PH = base_pH;
        
        //fprintf(llogfp, "Time: %e box-%d-%d base_pH: %e\n", bm->dayt, bm->current_box, bm->current_layer, base_pH);

		/* Update checked counter */
		bm->checkedalready[pBox->n][clayer + flagmodel * bm->wcnz]++;

	}

	return;
}

/**
 *
 *	\brief Calculate current pH at current depth at current time
 *
 */
double pH_At_Depth(MSEBoxModel *bm, Box *pBox, double dayt, int numwclayer, double cdepth, int clayer,
		double midpoint, int flagmodel, FILE *llogfp) {
	int depth_class = -1;
    double ans_pH = 8.0;
    double step1, pCO2, ans_carbonate, ans_arag_sat,
    	current_O2, alk, read_pCO2, coefft_T = 0, coefft_S = 0, coefft_O = 0,
    	const_coefft = 0, abs_temp, K0, K1, K2, step2;

	double *Y1; /* Pointer to initial variables of a WC cell */
	double pH_scale;

	/* make sure we grab the tracers for the correct layer type */
	if(flagmodel == WC)
		Y1 = (double*) pBox->tr[clayer];
	else
		Y1 = (double*) pBox->sm.tr[clayer];

	/* Get the P_scale value - this is calculated in the physics code - only apply here if
	 * pH value is calculated - if pH data is read in from netcdf file the scaling is applied
	 * when the data is read in.
	 */

	/* if this is the sediment use the Tchange value in the first water column layer */
	if(flagmodel == SED){
		pH_scale = bm->pHLayerScalar[0];
	}else{
		pH_scale = bm->pHLayerScalar[clayer];
	}

	if(bm->flagmodelpH == ph_regression){
		if(bm->tspCO2 == NULL) {
			quit("You can not use the setting flagmodelpH = 1 and fail to supply the pCO2 time series\n");
		}

		/* Get depth */
		if (cell_depth < bm->pH_surface_depth){
			depth_class = pH_surface_water;
		} else if (cell_depth < bm->pH_mid_depth){
			depth_class = pH_mid_water;
		} else {
			depth_class = pH_deep_water;
		}

		/* Calculate alkalinity using an empirical relationship
		 * First get the coefficients - from Bostock et al 2012 poster
		 */
		switch (depth_class) {
		case pH_surface_water:
			coefft_T = bm->pH_surface_coefft_T;
			coefft_S = bm->pH_surface_coefft_S;
			coefft_O = bm->pH_surface_coefft_O;
			const_coefft = bm->pH_surface_const;
			break;
		case pH_mid_water:
			coefft_T = bm->pH_mid_coefft_T;
			coefft_S = bm->pH_mid_coefft_S;
			coefft_O = bm->pH_mid_coefft_O;
			const_coefft = bm->pH_mid_const;
			break;
		case pH_deep_water:
			coefft_T = bm->pH_deep_coefft_T;
			coefft_S = bm->pH_deep_coefft_S;
			coefft_O = bm->pH_deep_coefft_O;
			const_coefft = bm->pH_deep_const;
			break;
		default:
			quit("No such depth option (%d) available for the calculated pH level\n", depth_class);
			break;
		}

		/* Second get the actual alkalinity result */
		current_O2 = Y1[Oxygen_i];
		current_O2 /= 31.988; // Converting from mg/m3 to umol/L
		alk = coefft_T * H2Otemp + coefft_S * current_SALT + coefft_O * current_O2 + const_coefft;
		alk /= (1000.0 * 1000.0);  // Converting umol/kg -> meq/L -> M/kg

		/* Convert alkalinity to pH - equations taken from
		 * Chapter 9 "Chemistry of Carbonic Acid in Water"
		 * IAEA Volume on "Environmental Isotopes in the Hydrological Cycle : Principles and Applications" */
		abs_temp = H2Otemp + 273.15;  // Temperature in Kelvin
		step1 = -60.2409 + (9345.17 / abs_temp) + 23.3585 * log(0.01 * abs_temp)
		 + current_SALT * (0.023517 - 0.023656 * 0.01 * abs_temp + 0.0047036 * (0.01 * abs_temp) * (0.01 * abs_temp));
		K0 = exp(step1);
		step1 = -1.0 * ((3670.7 / abs_temp) - 62.008 + 9.7944 * log(abs_temp) - 0.0118 * current_SALT + 0.000116 * current_SALT * current_SALT);
		K1 = pow(10.0,step1);
		step1 = -1.0 * ((1394.7 / abs_temp) + 4.777 - 0.0184 * current_SALT + 0.000118 * current_SALT * current_SALT);
		K2 = pow(10.0,step1);
		read_pCO2 = tsEval(bm->tspCO2, bm->pCO2_id, bm->t); // Assume pCO2 is in ppm CO2 equivalence concentrations (as for RCP files in 2012)
		pCO2 = read_pCO2 / 1000000.0;   // convert from ppm to correct units
		step1 = K1 * K0 * pCO2;
		step2 = K1 * K2 * K0 * pCO2;
		ans_pH = -log10((step1 + sqrt(step1 * step1 + 8.0 * alk * step2)) / (2 * alk));

		ans_pH = ans_pH * pH_scale;
		fprintf(llogfp, "Time: %e box%d-%d pH: %e CO2: %e\n", bm->dayt, bm->current_box, bm->current_layer, ans_pH, read_pCO2);

	} else if (bm->flagmodelpH == ph_data) {
		/* Read from file with salinity and temperature */
		/* Do not scale now - this is done on read-in */
		ans_pH = Y1[pH_i];
        
        //fprintf(llogfp, "Time: %e %d-%d has base_pH: %e and current_pH: %e\n", bm->dayt, bm->current_box, bm->current_layer, Y1[pH_i], ans_pH);
        
        
	} else if (bm->flagmodelpH == ph_disabled) {
        ans_pH = 8.0;
        ans_pH = ans_pH * pH_scale;
    }

    //fprintf(llogfp, "Time: %e box%d-%d containsCoral: %d flagmodelArag: %d\n", bm->dayt, bm->current_box, bm->current_layer, bm->containsCoral, bm->flagmodelArag);
    
    if(bm->containsCoral){
        switch(bm->flagmodelArag) {
            case no_arag:
                /* Nothing to do */
                break;
            case ts_arag:
                current_ARAG = Y1[AragoniteSat_i];
                
                //fprintf(llogfp, "Time: %e box%d-%d current_ARAG: %e\n", bm->dayt, bm->current_box, bm->current_layer, current_ARAG);
                break;
            case model_arag:
                ans_carbonate = bm->Karag_A / (1.0 + bm->Karag_B * exp(-bm->Karag_C * (ans_pH - bm->Karag_pH)));
                ans_carbonate *= (0.000186 / 0.111);  // This step is to calibrate to the midpoint of 186 micromol/kg reported for modern sea at 20 degrees with pH 8.05 from Kleypas 2006
                ans_arag_sat = log10(bm->Kca_const * ans_carbonate / bm->K_Ks);  //As assume calcium concentration static and only carbonate shifts with acidification
                current_ARAG = ans_arag_sat;
                Y1[AragoniteSat_i] = current_ARAG;

                //fprintf(llogfp, "Time: %e box%d-%d current_ARAG: %e K_Ks: %e\n", bm->dayt, bm->current_box, bm->current_layer, current_ARAG, bm->K_Ks);
                break;
        }
	}


	return ans_pH;
}

/**
 *	\brief Get local box rugosity
 *
 *
 */
void Box_Rugosity(MSEBoxModel *bm, Box *pBox, int rugosity_check_done, FILE *llogfp) {
	int ij = 0;
	double *Y1; /* Pointer to initial variables of a water column cell */

    if (rugosity_check_done){
        return; // Already done so don't redo
    }
    
	if (verbose > 3)
		printf("Rugosity level assessed\n");

	if(bm->track_rugosity_arag == FALSE) {
		if(bm->flag_refuge_model == rugosity_refuge_model){
			quit("You can't have track_rugosity_arag off (0 in run.prm) while also setting flag_refuge_model to %d\n", rugosity_refuge_model);
		}
		LocalRugosity = 0.0;
		return;
	}

	/* Get rugosity in the bottom water */
	Y1 = (double*) pBox->tr[ij];
	LocalRugosity = Y1[Rugosity_i];
    
    // Make sure the epibenthic layer Rugosity is reflected in all other water column layers (just in case)
    for(ij = 1; ij < pBox->nz; ij++){
        Y1 = (double *)pBox->tr[ij];
        Y1[Rugosity_i] = LocalRugosity;
    }
        

	return;

}

/*****************************************************************************************************************
 *
 * \brief Get environment scalar
 *
 */
double Get_Enviro_Scalar(MSEBoxModel *bm, double **Xchange, int Xchange_max_num, int X_may_change, int scale_id, int clayer, double timeNow){
	double X_scale = 1.0;
	int i, past_change;
	int now_change = 0;
	double end_date, XmultA, XmultB, Vmult, Xstart, Xperiod;

	if (X_may_change && (timeNow >= Xchange[0][start_id]) && (Xchange[0][start_id] != 0)) {
        
        for (i = 0; i < Xchange_max_num; i++) {
			if (timeNow >= Xchange[i][start_id])
				now_change = i;
		}
        
        end_date = (Xchange[now_change][start_id] + Xchange[now_change][period_id]);
		past_change = now_change - 1;
		XmultA = Xchange[now_change][mult_id];
		Vmult = Vchange[now_change][clayer][scale_id];
		if (now_change > 0)
			XmultB = Xchange[past_change][mult_id];
		else
			XmultB = 1;
		Xstart = Xchange[now_change][start_id];
		Xperiod = Xchange[now_change][period_id] + small_num;
        
        if (end_date < timeNow) {
			/* Change over */
			X_scale = XmultA;
		} else {
			/* Change still underway - calculate scaling based on time elapsed
			 since change started */
			if (XmultA >= XmultB) {
				X_scale = XmultB + (XmultA - XmultB) * (timeNow - Xstart) / Xperiod;
			} else {
				X_scale = XmultB - (XmultB - XmultA) * (timeNow - Xstart) / Xperiod;
			}
		}
        
        /* Correct for differential effects with depth */
		X_scale *= Vmult;
		
        //fprintf(bm->logFile, "Time: %e box%d-%d now_change: %d X_scale = %.20e\n", bm->dayt, bm->current_box, bm->current_layer, now_change, X_scale);
        
	} else {
		X_scale = 1.0;
        
        //fprintf(bm->logFile, "Time: %e box%d-%d nothing to do so X_scale = %.20e\n", bm->dayt, bm->current_box, bm->current_layer, X_scale);
	}
    
    return X_scale;

}

/**
 * Apply the Temp, salinity and PH scalars.
 *
 * This is now called by the physics code instead of inside the biology code.
 * This means the scald values are visible in the output netcdf file.
 *
 *
 */
void Ecology_Apply_Environ_Scalars(MSEBoxModel *bm, double ***newwc, double ***newsedtr){
	int b = 0, k = 0, sed_k;
	double T_scale, S_scale, pH_scale;
	/* Add dt so we get the time scalar used at the start of the next timestep */
	double timeNow = bm->dayt + (bm->dt/86400.0);


	//timeNow = bm->dayt;
	for(k = 0; k < bm->wcnz; k++){
        
        /* If general change in average base temperature through time (gradual change) calculate
		 new levels here. Note don't need to correct end_date to seconds here as bm->dayt
		 is in days too */
		T_scale = Get_Enviro_Scalar(bm, Tchange, Tchange_max_num, bm->flagtempchange, tempscale_id, k, timeNow);

        bm->TempLayerScalar[k] = T_scale;

		/* If the tempertature data has been read in from data files scale it now - otherwise its scaled later */
		if(bm->flagmodeltemp == 0 && T_scale != 1.0){
			for (b = 0; b < bm->nbox; b++) {
				newwc[b][k][bm->tempinput.tracerID]*= T_scale;

				//fprintf(bm->logFile, "newwc[b][k][bm->tempinput.tracerID] = %e\n", newwc[b][k][bm->tempinput.tracerID]);
			}

			/* Deal with the sediment scaling */
			if (k == 0){
				for(sed_k = 0; sed_k < bm->sednz; sed_k++){
					for (b = 0; b < bm->nbox; b++) {
						newsedtr[b][sed_k][bm->tempinput.tracerID]*= T_scale;

						//fprintf(bm->logFile, "newsedtr[b][sed_k][bm->tempinput.tracerID] = %e\n", newsedtr[b][sed_k][bm->tempinput.tracerID]);
					}
				}
			}
		}

        /* Scale salinity to represent environmental change through time (gradual change).
		 Note don't need to correct end_date to seconds here as bm->dayt
		 is in days too */
		S_scale = Get_Enviro_Scalar(bm, Schange, Schange_max_num, bm->flagsaltchange, saltscale_id, k, timeNow);
		bm->SaltLayerScalar[k] = S_scale;

		if(S_scale != 1.0){
			for (b = 0; b < bm->nbox; b++) {
				newwc[b][k][bm->saltinput.tracerID]*= S_scale;
			}

			/* Deal with the sediment scaling */
			if (k == 0){
				for(sed_k = 0; sed_k < bm->sednz; sed_k++){
					for (b = 0; b < bm->nbox; b++) {
						newsedtr[b][sed_k][bm->saltinput.tracerID]*= S_scale;

						//fprintf(bm->logFile, "newsedtr[b][sed_k][bm->saltinput.tracerID] = %e\n", newsedtr[b][sed_k][bm->saltinput.tracerID]);
					}
				}
			}

		}

        /* Scale pH to represent environmental change through time (gradual change).
		 Note don't need to correct end_date to seconds here as bm->dayt
		 is in days too */
		pH_scale = Get_Enviro_Scalar(bm, PHchange, PHchange_max_num, bm->flagpHchange, pHscale_id, k, timeNow);
		/* Store the scaling value for this layer so it can be used for pH models that don't reply on data */
		bm->pHLayerScalar[k] = pH_scale;

        if(pH_scale != 1.0){
			if(bm->flagmodelpH == ph_data){
				for (b = 0; b < bm->nbox; b++) {
					newwc[b][k][bm->pHinput.tracerID]*= pH_scale;
				}
				/* Deal with the sediment scaling */
				if (k == 0){
					for(sed_k = 0; sed_k < bm->sednz; sed_k++){
						for (b = 0; b < bm->nbox; b++) {
							newsedtr[b][sed_k][bm->pHinput.tracerID]*= pH_scale;

							//fprintf(bm->logFile, "newsedtr[b][sed_k][bm->pHinput.tracerID] = %e\n", newsedtr[b][sed_k][bm->pHinput.tracerID]);
						}
					}
				}

			}
		}
	}
}
