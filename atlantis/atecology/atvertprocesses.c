/**
 \file
 \brief C file for vertebrate subroutines for south east ecological model
 \ingroup atecology

 File:	atvertprocesses.c
 Author:	Beth Fulton
 Created:	21/9/2004
 Purpose:	C file for vertebrate subroutines for south east ecological model
 Revisions: 8/8/2004 Created mseecomvert.c from the seecomvert.c file

 17/8/2004 Generalised and updated to reflect parameters now
 in bm->SP_prms and counters (e.g. bm->K_num_vert_sp) now
 in MSEBoxModel structure

 21/9/2004 Created atvertprocesses.c from mseecomvert.c. And renamed
 FSL, FSO, FSF, FST and FSG to generic names so apply to
 other models/generic case.
 FSL -> FDC  (Ling -> Demersal_DC_Fish)
 FSO -> FDO  (Orange_Roughy -> Demersal_O_Fish)
 FSF -> FDB  (Flathead -> Demersal_B_Fish)
 FST -> FVO  (Bluefin_Tuna -> Pisciv_O_Fish)
 FSG -> SHB  (Gummy_Shark -> Shark_B)

 22/9/2004 Replaced numbers in switch statements with id
 guild numbers. Also added Ricker reproduction equation
 and replaced Beverton-Holt reproduction case with more
 conventional representation.

 15/10/2004 Added BML and BFF.

 19/10/2004 Added Prawns

 24/10/2004 Added soft sediments

 26/10/2004 Added FBP, FVB, SP, FDE, FDF, SHR, SSK, REP, WDG

 27/10/2004 Added cohort distribution per age class (VERTpopratio)

 2/11/2004 Added standard Beverton Holt stock-recruitment curve

 7/12/2004 Added initialisation of RC and DEN entries in shiftVERT
 in Vert_Reproduction()

 25/3/2005 Added code so groups with age class < 1yr long don't get
 anomalously jagged growth curves (for record keeping purposes
 store last valid entry until new age class sweeps through that
 age record)

 7/4/2005 In Vertebrate_Reproduction() changed shiftVERT[] of youngest
 age class from 0 to a formula matching the shift for other age
 classes. Zero only works in the age classes are annual or less, if
 they span multiple years then the original formulation was acting like
 MASSIVE juvenile mortality

 1/9/2005 Added multiple stock structure (horizontal and vertical)

 2/9/2005 Moved KDENR to array to accomodate recruitment to multiple stocks

 22/9/2005 Added linear pupping (and calving) recruitment option for vertebrates

 25/9/2005 Added environmental forcing (from times series) option to vertebrate
 reproduction routine

 10/10/2005 Corrected typos to do with external reproduction code

 8/12/2005 Fixed error with "cohort within age class" aging. Basically
 the original version was updating after every layer so later layers
 were smooshed if earlier layers had no individuals in them (i.e.
 if strong depth stratification and strongly demersal). The fix was to
 add Update_Age_Distrib() and Update_RecAge_Distrib() so that the
 cohort distributions were recalculated at correct spatial scale.
 Also put in check so lowest age classes of single cohort
 year classes weren't emptied (as no recruits meant empty cohort meant
 a zero that then flowed through all cohorts). Lastly needed to add
 array so that cohort aging up to mature age classes is actually shifted
 in the Updated_Aged_Cohorts() (just before movement code is called).
 This was because in species with strong juvenile-adult splits that
 cohort was bing lost which then carried on in a ripple effect through
 the rest of the stock eventually. Use recruitRange and recruitRangeFlat
 to see steady transference of individuals to older age classes (this
 does dilute absolute strength of very high cohorts (year classes),
 but is justifiable given that the code is following the average size
 of the entire age phase/class and there can be a large variation in size
 across individuals of the same age.

 24/2/2006 Added code to Ecology_Update_Vertebrate_Cohorts() and Vertebrate_Reproduction()
 and agingMIG array to better handle aging when only part of the population
 is within the model domain (previous code only dealt with when both juveniles
 and adults were both outside or both inside the model domain at time of aging).

 11/4/2006 Corrected aging problem (shiftVERTON trigger mis-set so had been
 adding newly aged into next age, but not taking them away from the current
 age). Created Prepare_Age_Distrib() to handle distribution calculations
 ahead of time (because when the entire age class ages up there is nothing
 left to do the distributional calculations with!). Had to add AGEnewden and
 AGE_stock_struct_prop to allow for this information to be carried through
 until needed (probably could have overloaded newden and init_stock_struct_prop,
 but chose not to incase timing of events is rewritten in future and movement,
 which also uses newden etc happens between Prepare_age_Distrib() and
 Ecology_Update_Vertebrate_Cohorts().

 28/8/2006 Added recSTOCK array so that can have stock specific reproduction
 parameters.

 23/8/2007 Modified the plankton based recruitment equation from
 VERTembryo[species][stock_id] += recSTOCK[species][stock_id] * PP_sp * (CHLa / ref_chl)
 to
 VERTembryo[species][stock_id] += recSTOCK[species][stock_id] * PP_sp * bm->cell_vol * (CHLa / ref_chl)
 so the units of PP_sp are clearer (larvae support per unit chl)

 23/05/2008 Bec Gorton
 Using setInitalVertVectors(bm, Pool); in atbiolUtil to set the initial value of
 VERTinfo instead of setting the values for each functional group.

 26-05-2008 Bec Gorton
 Changed all references from bm->VERTind to the tracer arrays associated with
 each functional group.

 18/07/2008 Bec Gorton
 Changing the code within and called by Box_Bio_Process to use the temporary data
 stores in the EcologyModule instead of Pool and NetFLUX.

 20-01-2009 Bec Gorton
 Changed the DIET_check array to store habitat level information. This
 array is now in the atEcologyStruct and its contents are output to
 a seperate file called DietCheck.txt if the flagdietcheck flag is TRUE.

 21-01-2009 Bec Gorton
 Got rid of the jmQ_T15_id, jmL_T15_id and jmL_id and jmQ_id species parameters.
 These values are now stored in the juv_id slot in the cohortSpeciesParams array.

 03-02-2009 Bec Gorton
 Added additional debugging messages in Vert_Mortality.

 19-03-2009 Bec Gorton
 Changed the definition of the Do_Vertebrate_Living function to no longer
 expect the biomass of BO, BD, BC and MBsed - these were nor used anymore.

 10-06-2009 Bec Gorton
 Added some new debugging code to print out the spawning information for the
 which_check species if debugging is activated.

 10-06-2009 Bec Gorton
 Added the SEAGRASS invertebrate type.

 05-11-2009 Bec Gorton
 Trunk Merge 4064 - Changed the Vertebrate_Reproduction function to use the species specific
 value calculated in Parameter_Q10 instead of the general temp_influence.

 05-11-2009 Bec Gorton
 Trunk Merge 1126 - Added minimum size of reproduction for vertebrates

 14-12-2009 Bec Gorton
 Trunk merge 1425 - Added the calcMLinearMort, calcMQuadMort and calcMPredMort arrays to store different mortality values.

 02-02-2010 Bec Gorton
 Changed Do_Vertebrate_Living to return the total numbers instead of storing in totVert.

 01-03-2010 Bec Gorton
 Added code for vertebrate larval dispersion.

 04-05-2010 Bec Gorton
 Changed the Vertebrate_Assess_Enviro function to use the new Util_Get_Accumulative_Change_Scale to calculate the habitat
 scaling value for each habitat type. This could potentially change output as more than one scaling event can take
 place at any time.

 10-05-2010 Bec Gorton
 Got rid of the Ecology_Get_Species_LMort function. These calculation are now done in ParameterQ10.

 26-03-2011 Beth Fulton
 Added species specific starvation coefficients - mStarve

 13-07-2011 Beth Fulton
 Fixed a bug where there was an assignment of a value to current_layer rather than a test against the value of
 current_layer (i.e. used = instead of == on line 2098 in the vertebrate aging stuff)

 08-12-2012 Beth Fulton
 Replaced agingMIG with MIGRATION[sp].aging[cohort]

 17-04-2013 Bec Gorton
 ATL-346 - Added support of tracking P:N:C ratios in groups.
 
 8-05-2014 Beth Fulton
 Changed the DIET_check array to store diet information rather than a simple on/off check.
 
 ***************************************************************************************

 Table 1. Summary of Model State Variables.

 Description	Symbol					Units
 ------------------              ------------
 Large phytoplankton				PL	mg N m-3
 Small phytoplankton	(pico)		PS	mg N m-3
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
 Reptiles						REP special
 Pinnipeds                       PIN special
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
 Macrozoobenthos	shallow			BMS	mg N m-2
 Macrozoobenthos	deep			BMD	mg N m-2
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
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <sjwlib.h>
#include "atecology.h"

static void Determine_Fish_Feeding_Prms(MSEBoxModel *bm, int guildcase, double SN, double RN, double X_RS, double *vl, double *ht);
static void Vert_Mortality(MSEBoxModel *bm, int guildcase, int chrt, double SN, double RN, double Biom, double Dens, double Wgt, double *mort, double *waste, FILE *llogfp);
static void Print_Diagnostic_Grazing(MSEBoxModel *bm, FILE *llogfp, int guildcase, double ***spGRAZEinfo);
static void Vertebrate_Activities(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitatType, FILE *llogfp, int guildcase, double SN, double RN, double NUMS, int predcase, int chrt, double E1_sp, double E2_sp, double E3_sp, double E4_sp, double PB_scale, double BB_scale, double ***spPREYinfo, double ***spGRAZEinfo, double **spCATCHGRAZEinfo, double ***spSPinfo, double *Growth, double *GrazeLive, double *FRCsp, double *Tot_sp, double *Mort, double *ReleaseNH, double *ProdnDL, double *ProdnDR);


/**
 *	\brief Group size subroutine
 *
 *	Now used for invertebrates as well as vertebrates.
 *
 *	The cohort parameters is only used for invert groups.
 *
 *	@returns The length (li) of an individual of weight Wgt
 */
double Ecology_Get_Size(MSEBoxModel *bm, int species, double Wgt, int cohort) {
    double li = 0.0;
    double SN_sp = 0.0;
    int done_it = 0;
    double li_a = FunctGroupArray[species].speciesParams[li_a_id];
    double li_b = 1.0 / (FunctGroupArray[species].speciesParams[li_b_id] + small_num);
    double wi, w_on_a;

    if (FunctGroupArray[species].groupAgeType != AGE_STRUCTURED) {
        if (FunctGroupArray[species].sn[cohort] > 0) {
            if (bm->flag_lengthSN) { // If length is calculated based on SN pool only, asign SN to Wgt
                Wgt = FunctGroupArray[species].sn[cohort];
            } else {
                if (bm->flag_dynamicXRS)
                    Wgt = FunctGroupArray[species].X_RS[cohort] * FunctGroupArray[species].sn[cohort];
                else
                    Wgt = (1.0 + FunctGroupArray[species].X_RS[cohort]) * FunctGroupArray[species].sn[cohort];
                li_a = bm->li_a_invert;
                li_b = bm->li_b_invert;
                done_it = 1;
            }
        }
    }

    // If vertebrate length is calculated based on SN pool only, get their wgt now
    if (bm->flag_lengthSN) {
        li_a = li_a / (1 + FunctGroupArray[species].X_RS[cohort]); // transform the li_a parameter by X_RS, to get only SN based length
        SN_sp = VERTinfo[species][cohort][SN_id]; // weight calculation is only based on SN
        wi = (SN_sp * bm->k_wetdry * bm->X_CN) / 1000.0; // do normal conversion from mg of N to grams of wet weight

        // After that proceed as normal, but remember that li_a and li_b parameters have different meaning depending on flag_lengthSN value.
        
    } else { // if using default weights as RN+SN calculate grams of wet weight based on total weight
        wi = (Wgt * bm->k_wetdry * bm->X_CN) / 1000.0;
    }
	w_on_a = wi / (li_a + small_num);
	li = pow(w_on_a, li_b);

    if ((FunctGroupArray[species].groupAgeType != AGE_STRUCTURED) && !done_it)
        li = 1.0;

    return li;
}

/**
 *	\brief Vertebrate reproductive subroutines
 *
 *	@returns The amount (C) spawned by an individual
 */
double Ecology_Spawn(int species, double KSPA_SP, double FSP_SP, double SN, double RN, double X_RS, double FSPB_SP, FILE *llogfp) {
	double WforS, Wgt, Spwn, Wgtcheck;

	WforS = (1.0 + X_RS) * SN;
	Wgt = SN + RN;
	Spwn = FSP_SP * WforS - KSPA_SP;
	Wgtcheck = FSPB_SP * RN;
	if (Wgt < WforS)
		Spwn = Spwn - (WforS - Wgt);
	if (Spwn < 0.0)
		Spwn = 0.0;
	if (Wgtcheck > RN) {
		/* This was adopted from ERSEM way back when, it should never actually happen
		 as FSPB_SP <= 1 (or should be at any rate!!!!) */
		if (FSPB_SP != 0)
			Spwn = RN / FSPB_SP;
		else
			Spwn = RN;
	}

	return (Spwn);
}

/**
 *	\brief Vertebrates assessing environment as refuge so can adjust availability
 *	parameters according during feeding.
 *
 *
 *	Pelagic fish, pelagic sharks, pinnipeds, whales and seabirds are assumed
 *	to be habitat independent, whereas some demersal fish and sharks have differing
 *	habitat requirements with maturity state
 *
 */
void Vertebrate_Assess_Enviro(MSEBoxModel *bm, int guildcase, int boxin, double **cover_info, double cdepth, int rugosity_check_done, FILE *llogfp) {

	double Kcov_juv, Bcov_juv, Acov_juv, Kcov_ad, Bcov_ad, Acov_ad, cover;
	double sp_num_biogen_hab_juv, sp_num_biogen_hab_ad, sp_num_biogen_hab, geo_habitats, tot_habitats;
	double 	Rcoefft, Rconst, Rcap, Rscalar;
	double sp_likeREEF, sp_likeFLAT, sp_likeCANYON, sp_likeSOFT;
	int habitat_dependent, stage, kij;
	int sp;
	int like_coral = 0;
	double sp_like, step1;
    
    /*
    if(verbose > 1){
    	printf("Vertebrate_Assess_Enviro for %s\n", FunctGroupArray[guildcase].groupCode);
    }
    */

	Box_Rugosity(bm, &bm->boxes[boxin], rugosity_check_done, llogfp);

	habitat_dependent = FunctGroupArray[guildcase].isHabDepend;
	Kcov_juv = FunctGroupArray[guildcase].speciesParams[Kcov_juv_id];
	Bcov_juv = FunctGroupArray[guildcase].speciesParams[Bcov_juv_id];
	Acov_juv = FunctGroupArray[guildcase].speciesParams[Acov_juv_id];
	Kcov_ad = FunctGroupArray[guildcase].speciesParams[Kcov_ad_id];
	Bcov_ad = FunctGroupArray[guildcase].speciesParams[Bcov_ad_id];
	Acov_ad = FunctGroupArray[guildcase].speciesParams[Acov_ad_id];
	Rscalar = FunctGroupArray[guildcase].speciesParams[RugCover_scalar_id];
	sp_num_biogen_hab_ad = FunctGroupArray[guildcase].speciesParams[K_num_biogen_habad_id];
	sp_num_biogen_hab_juv = FunctGroupArray[guildcase].speciesParams[K_num_biogen_habjuv_id];

	if (bm->flaghabdepend && habitat_dependent) {

		/* Determine coastal habitat degradation effects */
		for (kij = 0; kij < bm->K_num_bed_types; kij++) {
			if (Box_degradedi[boxin] && (cdepth < -250) && bm->flagdegrade) {
				BED_scale[kij] = Util_Get_Accumulative_Change_Scale(bm, BEDchange_max_num, BEDchange[kij]);
			} else {
				BED_scale[kij] = 1.0;
			}
		}

		for (stage = 0; stage < FunctGroupArray[guildcase].numStages; stage++) {
			if(stage)
				sp_num_biogen_hab = sp_num_biogen_hab_ad;
			else
				sp_num_biogen_hab = sp_num_biogen_hab_juv;

			/* For those groups which are habitat dependent, the habitat level is the sum
			 of the habitats, but if canyon loving than cover effectiveness is enhanced the
			 more canyons there are */
			cover = 0.0;

			/* Habitat preferences */
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isCover == TRUE) {
					//if (FunctGroupArray[sp].speciesParams[flag_id] == TRUE) {
						sp_like = bm->HABITATlike[guildcase][stage][FunctGroupArray[sp].coverID];

						if (sp_like) {
							cover += (sp_like * cover_info[FunctGroupArray[sp].coverID][boxin]);
						}

						if(((FunctGroupArray[sp].groupType == CORAL) || (FunctGroupArray[sp].groupType == SPONGE)) && (sp_like > 0)) {
							like_coral = 1;
						}
					//}
				}
			}

			sp_likeREEF = bm->HABITATlike[guildcase][stage][bm->REEFcover_id];
			sp_likeFLAT = bm->HABITATlike[guildcase][stage][bm->FLATcover_id];
			sp_likeSOFT = bm->HABITATlike[guildcase][stage][bm->SOFTcover_id];
			sp_likeCANYON = bm->HABITATlike[guildcase][stage][bm->CANYONcover_id];

			geo_habitats = 0;
			if (sp_likeREEF){
				cover += (sp_likeREEF * cover_info[bm->REEFcover_id][boxin] * BED_scale[reef_id]);
				geo_habitats++;
			}
			if (sp_likeFLAT){
				cover += (sp_likeFLAT * cover_info[bm->FLATcover_id][boxin] * BED_scale[flat_id]);
				geo_habitats++;
			}
			if (sp_likeSOFT){
				cover += (sp_likeSOFT * cover_info[bm->SOFTcover_id][boxin] * BED_scale[soft_id]);
				geo_habitats++;
			}
			if (sp_likeCANYON)
				cover *= (1.0 + sp_likeCANYON * cover_info[bm->CANYONcover_id][boxin]);
            
			tot_habitats = geo_habitats + sp_num_biogen_hab;

			if(bm->flag_rel_cover)
				cover /= (tot_habitats + small_num);

			if(bm->ice_on){
				// If use other habitat make ice a scalar
				if(sp_likeREEF || sp_likeFLAT || sp_likeSOFT || sp_likeCANYON)
					cover *= Get_Ice_Vertebrate_Habitat_Rating(bm, guildcase, stage, boxin);
				else // Otherwise ice only
					cover = Get_Ice_Vertebrate_Habitat_Rating(bm, guildcase, stage, boxin);
			}

			switch(bm->flag_refuge_model){
			case no_refuge_model:
				bm->refuge_status[guildcase][boxin][stage] = 1.0;
				break;
			case orig_refuge_model:
				if (stage)
					bm->refuge_status[guildcase][boxin][stage] = Acov_ad * (exp(-Kcov_ad * cover + Bcov_ad) + 1.0 / Kcov_ad);
				else
					bm->refuge_status[guildcase][boxin][stage] = Acov_juv * (exp(-Kcov_juv * cover + Bcov_juv) + 1.0 / Kcov_juv);
				break;
			case rugosity_refuge_model:
				if(like_coral) {
					Rcoefft = bm->RugCover_Coefft;
					Rconst = bm->RugCover_Const;
					Rcap = bm->RugCover_Cap;

					step1 = Rcoefft * log(LocalRugosity) + Rconst;
					bm->refuge_status[guildcase][boxin][stage] = min(Rcap, Rscalar / step1);
					if(bm->refuge_status[guildcase][boxin][stage] < 0.0)
						bm->refuge_status[guildcase][boxin][stage] = 0.0;

					//fprintf(llogfp,"Time: %e %s-%d in box %d refuge_status: %e Rcap: %e Rscalar: %e step1: %e Rcoefft: %e LocalRugosity: %e Rconst: %e\n",
					//		bm->dayt, FunctGroupArray[guildcase].groupCode, stage, boxin, bm->refuge_status[guildcase][boxin][stage], Rcap, Rscalar, step1, Rcoefft, LocalRugosity, Rconst);

				} else {
					// If still habitat dependent but not coral loving use a different habitat dependency function here
                    if (stage)
                        bm->refuge_status[guildcase][boxin][stage] = Acov_ad * (exp(-Kcov_ad * cover + Bcov_ad) + 1.0 / Kcov_ad);
                    else
                        bm->refuge_status[guildcase][boxin][stage] = Acov_juv * (exp(-Kcov_juv * cover + Bcov_juv) + 1.0 / Kcov_juv);
				}
				break;
			default:
				quit("No such refuge model set flag_refuge_model to 0, 1 or 2 (currently %d)\n", bm->flag_refuge_model);
				break;
			}

		}
	} else {
		bm->refuge_status[guildcase][boxin][0] = 1.0;
		bm->refuge_status[guildcase][boxin][1] = 1.0;
	}
    
	return;
    
}

/**
 *	\brief Calculate amount of available forage from vertebrate prey
 *
 */
double Avail_Fish(MSEBoxModel *bm, int guildcase, int chrt, int chrtstage, int prey, int bpreychrt, double SN, double ***SP, FILE *llogfp) {
	double eatthis, step1;
	double fish_available = 0.0;
	double KLP_SN, KUP_SN, KLP_SP, KUP_SP, prey_SN, prey_RN, pred_RN, prey_len, pred_len, rel_size, Kmax_coefft = 0.0, sizeScalar = 0.0, maxavail, xmid, li_a, li_b, invert_weight;
	int preyage, boxin, layerin, preystock, pHsensitive_sp;
    //int maxstock_id;
	double pHscalar, prey_turbid_scalar;
	int do_test = 0;
    //double max_scalar_SN = FunctGroupArray[guildcase].max_scalar[chrt][prey];

	boxin = bm->current_box;
    layerin = bm->current_layer;
    KLP_SN = FunctGroupArray[guildcase].speciesParams[KLP_id] * SN;
	KUP_SN = FunctGroupArray[guildcase].speciesParams[KUP_id] * SN;

	step1 = 0;

	if (FunctGroupArray[guildcase].speciesParams[overwintering_id] > 0) {
		/* Don't do anything as overwintering and in status */
		return 0.0;

	}

	if (FunctGroupArray[prey].isVertebrate == TRUE && FunctGroupArray[prey].speciesParams[flag_id]) {
		if (SP[prey][bpreychrt][DEN_id] > 0) {
			preyage = FunctGroupArray[prey].cohort_stage[bpreychrt];


			if (bm->pSPVERTeat[guildcase][prey][chrtstage][preyage] > 0.0) {
				do_test = 1;
				if ( bm->UseHardFeedingWindow ){
					if (SP[prey][bpreychrt][SN_id] >= KLP_SN && SP[prey][bpreychrt][SN_id] <= KUP_SN) {
						do_test = 1;
					} else {
						do_test = 0;
					}
				}

				if (do_test) {
				
					//initavailfish = (fish_available);   // needed for commented out bits below
					
					preystock = bm->group_stock[prey][bm->current_box][bm->current_layer];
					
                    /**
                    if((guildcase == bm->which_check) && (bm->checkbox == bm->current_box)) {
                        fprintf(llogfp, "UseHardFeedingWindow %d, UseBiLogisticFeedingWindow: %d\n", bm->UseHardFeedingWindow, bm->UseBiLogisticFeedingWindow);
                    }
                    **/

					if ( bm->UseHardFeedingWindow ){
						sizeScalar = 1.0;
					} else {
                        if ( !bm->UseBiLogisticFeedingWindow ) {  // Using humped relaitonship with potential skew
                            Kmax_coefft = FunctGroupArray[guildcase].speciesParams[Kmax_coefft_id];
                            rel_size = SP[prey][bpreychrt][SN_id] / KUP_SN; // This used to be (SN *  KUP_SN) but "SN *" not needed as alrady in KLP_SN
                            sizeScalar = rel_size * exp(Kmax_coefft * (1.0 - rel_size));
                            
                            /**
                            if((guildcase == bm->which_check) && (bm->checkbox == bm->current_box)) {
                                fprintf(llogfp, "Kmax_coefft %e, rel_size: %e, SN: %e, prey_SN: %e, KUP_SN: %e\n", Kmax_coefft, rel_size, SN, SP[prey][bpreychrt][SN_id], KUP_SN);
                            }
                            **/
                            
                        } else { // Use bi-logistic form developed by Asta Audzijonyte
                            //sizeScalar = 1.0;
                            Kmax_coefft = FunctGroupArray[guildcase].speciesParams[Kmax_coefft_id];
                            KLP_SP = FunctGroupArray[guildcase].speciesParams[KLP_id];
                            KUP_SP = FunctGroupArray[guildcase].speciesParams[KUP_id];
                            
                            //get predator length information, as bilogistic feeding is based on length
                            if (FunctGroupArray[guildcase].groupAgeType == AGE_STRUCTURED) {
                                pred_RN = SP[guildcase][chrt][RN_id];
                                pred_len = Ecology_Get_Size(bm, guildcase, (SN + pred_RN), chrt);				// returns length in cm
                            } else {
                                //	pred_len = Ecology_Get_Size(bm, guildcase, SN, chrt);
                                li_a = bm->li_a_invert;
                                li_b = bm->li_b_invert;
                                invert_weight = (SN * bm->k_wetdry * 2) / 1000.0;  // We assumed that RN/SN ratio in invertebrates is 1:1. 
                                pred_len = pow((invert_weight / (li_a + small_num)), li_b);
                            }

                            /*
                            maxavail = KLP_SN + (KUP_SN - KLP_SN) * 0.5;
                            if (SP[prey][bpreychrt][SN_id] <= maxavail){  // was (maxavail * SN)
                                xmid = KLP_SN + (maxavail - KLP_SN ) * 0.5;   // was (KLP_SN + (maxavail - KLP_SN ) * 0.5) * SN but final SN not needed as alrady in maxavail, KLP_SN and KUP_SN
                                sizeScalar = 1.0 / (1.0 + exp(-Kmax_coefft * (SP[prey][bpreychrt][SN_id] - xmid)));
                            }
                            if (SP[prey][bpreychrt][SN_id] > maxavail * SN){  // was (maxavail * SN)
                                xmid = (KUP_SN - (KUP_SN - maxavail) * 0.5); // was (KUP_SN - (KUP_SN - maxavail) * 0.5) * SN but final SN not needed as alrady in maxavail, KLP_SN and KUP_SN
                                sizeScalar = 1.0 / (1.0 + exp(Kmax_coefft * (SP[prey][bpreychrt][SN_id] - xmid)));
                            }
                            */
                            
                            //get prey length information as bilogistic feeding is based on length
                            if (FunctGroupArray[prey].groupAgeType == AGE_STRUCTURED) {
                                prey_SN = SP[prey][bpreychrt][SN_id];
                                prey_RN = SP[prey][bpreychrt][RN_id];
                                prey_len = Ecology_Get_Size(bm, prey, (prey_SN + prey_RN), bpreychrt);				// returns length in cm
                            } else {
                                prey_len = 0;  // size based feeding only applies to vertebrate prey, so prey's length is set to 0
                            }
                            
                            maxavail = KLP_SP + (KUP_SP - KLP_SP) * 0.5;
                            
                            if (prey_len <= maxavail * pred_len) {
                                xmid = (KLP_SP + (maxavail - KLP_SP ) * 0.5) * pred_len;
                                sizeScalar = 1.0 / (1.0 + exp(-Kmax_coefft * (prey_len - xmid)));
                            }
                            
                            if (prey_len > maxavail * pred_len) {
                                xmid = (KUP_SP - (KUP_SP - maxavail) * 0.5) * pred_len;
                                sizeScalar = 1.0 / (1.0 + exp(Kmax_coefft * (prey_len - xmid)));
                            }

                            // Correct with pre-calculated max_salar - this is crude as not dynamic, but faster than claculating over all age classes with every Eat() iteration
                            //sizeScalar /= max_scalar_SN;
                            
                        }
                        
                        if(sizeScalar > 1.0)
                            sizeScalar = 1.0;
                        if(sizeScalar < 0.0)
                            sizeScalar = 0.0;

					}

					pHsensitive_sp = (int) (FunctGroupArray[prey].speciesParams[flagpHsensitive_id]);
					if(pHsensitive_sp){
						pHscalar = FunctGroupArray[prey].pHcorr;   // Positive as assume availability being higher as pH drops
					} else {
						pHscalar = 1.0;
					}

                    if (FunctGroupArray[guildcase].speciesParams[turbid_refuge_id])
                        prey_turbid_scalar = bm->turbid_effect[guildcase][boxin][layerin][chrtstage];
                    else
                        prey_turbid_scalar = 1.0;
                    
                    step1 = bm->pSPVERTeat[guildcase][prey][chrtstage][preyage] * pSTOCK[prey][preystock][preyage]
							* bm->refuge_status[prey][boxin][preyage] * prey_turbid_scalar * pHscalar * sizeScalar;

					if (!bm->flag_olddiet && (step1 > 1.0))
						step1 = 1.0;

					/* The amount of fish available - this value is a biomass value */
					eatthis = step1 * ((SP[prey][bpreychrt][SN_id] + SP[prey][bpreychrt][RN_id]) * SP[prey][bpreychrt][DEN_id] / (bm->cell_vol + small_num));
					fish_available += eatthis;


					/**
					if((guildcase == bm->which_check) && (bm->checkbox == bm->current_box)) {
					//if((guildcase == bm->which_check) || (prey == bm->which_check)) {
						fprintf(llogfp,
								"pred: %s, availfish: %.20e, chrtstage: %d, prey: %s, preychrt: %d, bm->cell_vol: %e, preySN: %.20e, preyRN: %.20e, preyDEN: %.20e, preyBiom: %.20e\n",
								FunctGroupArray[guildcase].groupCode, fish_available, chrtstage, FunctGroupArray[prey].groupCode,
								bpreychrt, bm->cell_vol, SP[prey][bpreychrt][SN_id], SP[prey][bpreychrt][RN_id], SP[prey][bpreychrt][DEN_id],
								((SP[prey][bpreychrt][SN_id] + SP[prey][bpreychrt][RN_id]) * SP[prey][bpreychrt][DEN_id] / (bm->cell_vol + small_num)));
						fprintf(llogfp,
								"(%s-%d vs %s-%d) bm->pSPVERTeat[%s][%s][%d][%d]: %.20e, eatthis: %.20e, step1: %.20e, pSTOCK[%d][%d][%d]: %.20e, refuge_status[%d][%d][%d]: %.20e, pHscalar: %e, sizeScalar: %e\n",
								FunctGroupArray[guildcase].groupCode, chrt, FunctGroupArray[prey].groupCode, bpreychrt,
								FunctGroupArray[guildcase].groupCode, FunctGroupArray[prey].groupCode, chrtstage,
								preyage, bm->pSPVERTeat[guildcase][prey][chrtstage][preyage], eatthis, step1, prey, preystock, preyage,
								pSTOCK[prey][preystock][preyage], prey, boxin, preyage, bm->refuge_status[prey][boxin][preyage], pHscalar, sizeScalar);
					}
					/*/


				}
			}
		}
	}

	if ((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)) {
		fprintf(llogfp, "Avail_Fish outcomes: guildcase = %s, chrt = %d, fish_available = %.20e\n", FunctGroupArray[guildcase].groupCode, chrt, fish_available);
	}

	return fish_available;

}

/**
 *
 *	\brief Calculate amount of available catch from vertebrate prey
 *
 */
double Avail_Catch(MSEBoxModel *bm, int guildcase, int chrt, int chrtstage, int prey, int bpreychrt, double SN, double ***SP, FILE *llogfp) {
	double eatthis, step1;
	double catch_available = 0.0;
	int preyage, preychrt, fleet;

	if (FunctGroupArray[guildcase].speciesParams[overwintering_id] > 0) {
		/* Don't do anything as overwintering and in status */
		return 0.0;

	}

	preychrt = bpreychrt / FunctGroupArray[prey].numGeneTypes;
	if (FunctGroupArray[prey].speciesParams[flag_id] == TRUE && FunctGroupArray[prey].isVertebrate == TRUE) {
		preyage = FunctGroupArray[prey].cohort_stage[bpreychrt];
		if (bm->pSPFCeat[guildcase][prey][chrtstage][preyage] > 0.0) {
			if (SP[prey][bpreychrt][SN_id] >= FunctGroupArray[guildcase].speciesParams[KLP_id] * SN) {
				if (SP[prey][bpreychrt][SN_id] <= FunctGroupArray[guildcase].speciesParams[KUP_id] * SN) {
					step1 = 0;
					for (fleet = 0; fleet < bm->K_num_fisheries; fleet++) {
						step1 += bm->FCcaughttemp[prey][fleet][preychrt] * bm->pFLEET[guildcase][fleet];
					}

					if(!bm->flag_olddiet)
						step1 *= bm->pSPFCeat[guildcase][prey][chrtstage][preyage];

					eatthis = step1 * (SP[prey][bpreychrt][SN_id] + SP[prey][bpreychrt][RN_id]) / (bm->cell_vol + small_num);
					catch_available += eatthis;
				}
			}
		}
	}

    if ((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)) {
		fprintf(llogfp, "Avail_Catch outcomes: guildcase = %d, chrt = %d,  catch_available = %.20e\n", guildcase, chrt, catch_available);
	}

	return catch_available;

}

/**
 *	\brief Partition vertebrate weight between reserve and structural pools
 *
 *	Calculates fraction of net weight change to be assigned to structural
 *	weight change. The larger pR_SP the more fish "concentrates" on
 *	replenishing reserves rather than structural growth when underweight.
 *
 */
void Partition_Weight(MSEBoxModel *bm, int sp, double pR_SP, double SN, double RN, double X_RS, double *FRC_sp, FILE *llogfp) {
	double temp0, temp1, temp2;

	temp0 = RN / (X_RS * SN);
	temp1 = (1.0 / X_RS) + pR_SP * (temp0 - 1.0);
	if (temp1 < 0.0)
		temp1 = 0.0;
	temp2 = (1.0 / X_RS) + RN / (X_RS * SN);
	*FRC_sp = temp1 / temp2;
    
    if (!bm->flag_shrinkfat){  // Have this as an option so can have consistency with legacy models (sanity check that should never be tripped but needed for consistency with old code just in case)
        if (*FRC_sp < 0.0)
            *FRC_sp = 0.0;
    }
    if (*FRC_sp > 1.0)
        *FRC_sp = 1.0;

    /*
    if(sp == WHT_id)
		fprintf(llogfp,"SN: %e, RN: %e, pR_SP: %e, X_RS: %e, temp0: %e, temp1: %e, temp2: %e, FRC_sp: %e\n",
			SN, RN, pR_SP, X_RS, temp0, temp1, temp2, *FRC_sp);
     */


	return;
}

/**
 *	\brief New function to partition vertebrate weight between reserve and structural pools
 *
 * The pR_SP parameter is not used like in the standard Partition_Weight(). Instead it is assumed that all energy is sent to RN,
 * until the maximum RN/SN ratio is reached (given in RSmax_sp parameter) and then energy is allocated to both pools to maintain the dynamically
 * calculated X_RS ratio. Allocation of energy takes into account the inefficiencies of energy allocation to SN and RN (cost of growth used in DEB)
 * This option does not use flag_shrinkfat, because energy deficit is taken out from reserves by default
 */

void Partition_Weight_Dynamic(MSEBoxModel *bm, int sp, double SN, double RN, double X_RS, double *FRC_sp, double avail_intake, FILE *llogfp) {
    double SNcost_SP, RNcost_SP, maxR, maxS, intake, toR, toS;
    
    SNcost_SP = FunctGroupArray[sp].speciesParams[SNcost_id];
    RNcost_SP = FunctGroupArray[sp].speciesParams[RNcost_id];
    
    toR = 0;
    toS = 0;
    intake = avail_intake;
    
    maxR = intake * RNcost_SP; // this is the maximum possible allocation to RN, when accounting for conversion efficiency
    maxS = intake * SNcost_SP;  // this is the maximum possible allocation to SN, when accounting for conversion efficiency, which for structure is usually low
    
    if (X_RS * SN > RN) { // reserves are below the optimal/maximum level
        toR = X_RS * SN - RN;
        if (toR > maxR) {
            toR = maxR; // temp is now the amount that can be sent to RN to attempt to reach X_RS ratio
        }
        intake -= toR / RNcost_SP; // this is how much food is left after allocating to RN to reach the optimal ratio
    } 	else {					// structure is below optimal. this should never happen, if RN is allocated properly but is here just in case
        toS = RN / X_RS - SN;
        if (toS > maxS) {
            toS = maxS;
        }
        intake -= toS / SNcost_SP; // this is how much food is left after allocating to SN to reach the optimal ratio
    }
    
    // partition remaining mass to keep desired ratio
    //toR += (X_RS * RNcost_SP * SNcost_SP * intake / (RNcost_SP + X_RS * SNcost_SP));
    toS += (RNcost_SP * SNcost_SP * intake / (RNcost_SP + X_RS * SNcost_SP));
    
    *FRC_sp = toS / avail_intake; // this is the proportion of total original intake (avail_intake) that goes to SN
    
    if (*FRC_sp > 1.0)
        *FRC_sp = 1.0;
    
    //	fprintf(llogfp,"PARweightDYN %s, SN: %e, RN: %e, X_RS: %e, intake: %e, avail_intake: %e, maxR: %e, maxS: %e, toR: %e, toS: %e, FRC_sp: %e\n",
    //		FunctGroupArray[sp].groupCode, SN, RN, X_RS, intake, avail_intake, maxR, maxS, toR, toS, *FRC_sp);
    
    
    return;
}


/**
 *	\brief Natural and implicit predation mortality for vertebrates
 *
 */
void Vert_Mortality(MSEBoxModel *bm, int guildcase, int chrt, double SN, double RN, double Biom, double Dens, double Wgt, double *mort, double *waste, FILE *llogfp) {
	double mL, mQ, mE, mS, FSBDR, loadFSB, loadFFDS, SPtoSB, SPtoFDS, SBtoDR, FDStoDR, opt_cond, starving, nat_mort, pred_mort, nums, mStarve, RSstarve,
        orig_mort, final_mort, mort_scalar, X_RS;
	int stock_id;
	int stage = FunctGroupArray[guildcase].cohort_stage[chrt];
	double mortality_scalar;

	FSBDR = FunctGroupArray[guildcase].speciesParams[FSBDR_id];

	mL = Ecology_Get_Linear_Mortality(bm, guildcase, chrt, stage);
	mQ = FunctGroupArray[guildcase].cohortSpeciesParams[stage][mQ_id];
	mE = FunctGroupArray[guildcase].cohortSpeciesParams[stage][mE_id];
    mS = Acidif_Mort(bm, guildcase);
    X_RS = FunctGroupArray[guildcase].X_RS[chrt];

	mStarve = FunctGroupArray[guildcase].speciesParams[mStarve_id];
    RSstarve = FunctGroupArray[guildcase].speciesParams[RSstarve_id];

	/* Pressures due to fish and seabirds not represented explicitly in the model*/
	//	loadImplicitMortality = bm->HowFar * (bm->mS[guildcase][bm->NextQofY] - bm->mS[guildcase][bm->QofY])
	//	+ bm->mS[guildcase][bm->QofY];

	loadFSB = bm->HowFar * (bm->mS[guildcase][bm->NextQofY][0] - bm->mS[guildcase][bm->QofY][0]) + bm->mS[guildcase][bm->QofY][0];
	loadFFDS = bm->HowFar * (bm->mS[guildcase][bm->NextQofY][1] - bm->mS[guildcase][bm->QofY][1]) + bm->mS[guildcase][bm->QofY][1];

	//loadImplicitMortality = loadImplicitMortality/86400.0;

	loadFSB = loadFSB / 86400.0;
	loadFFDS = loadFFDS / 86400.0;

	/* Fluxes to implicit birds, mammals */
	//SPtoImplicitMortality = loadImplicitMortality * Biom;

	SPtoSB = loadFSB * Biom;
	SPtoFDS = loadFFDS * Biom;

	/* Returns from predators to detritus through sloppy eating, and other waste */
	//ImplicitMortalitytoDR =
	SBtoDR = FSBDR * SPtoSB;
	FDStoDR = FFDDR * SPtoFDS;

	*waste = SBtoDR + FDStoDR;

	/* Natural mortality (disease, starvation, implicit predation) */
    if (bm ->flag_dynamicXRS) {
        opt_cond = X_RS * SN; // this is the optimal (maximum) RN
        starving = Kthreshm * exp(-RSstarve * (RN /opt_cond));
    } else {
        // Original version of starvation mortality
        opt_cond = (1.0 + X_RS) * SN;
        if(mStarve > 0){
            starving = mStarve * (Kthreshm * opt_cond - Wgt) / opt_cond;
        } else {
            starving = 0;
        }
	}

	if (starving <= 0.0)
		starving = 0.0;
	else {
		starve_vert[guildcase][bm->current_box] = 1;

	}

	nums = Dens * bm->cell_vol;
	nat_mort = (starving + mL + mE + mS + mQ * nums) * nums;

	/* Explicit static predation (due to seabirds and mammals,
	 fishing and large demersal fish) */
	pred_mort = (SPtoSB + SPtoFDS) * bm->cell_vol / Wgt;

	/* If there is a scalar to apply then grab it now */
	if(bm->scale_all_mortality == TRUE){
		mortality_scalar = Ecology_Get_Mortality_Scalar(bm, guildcase, chrt);
	}else{
		mortality_scalar = 1.0;
	}

	/* Total mortality */
	*mort = (mortality_scalar * (nat_mort + pred_mort));

    orig_mort = *mort;
    if (*mort > nums)
        *mort = nums;
    final_mort = *mort;
    if(orig_mort > 0)
        mort_scalar = final_mort / orig_mort;
    else
        mort_scalar = 1.0;

    /* Update the linear mortality, quad mortality and mortality due to explict birds and mammals - pro-rata any corrections */
	bm->calcMLinearMort[guildcase][current_id] += (mL * nums) * mort_scalar * mortality_scalar;
	bm->calcELinearMort[guildcase][current_id] += ((mE + mS) * nums) * mort_scalar * mortality_scalar;
	bm->calcMQuadMort[guildcase][current_id] += (mQ * nums * nums) * mort_scalar * mortality_scalar;
	bm->calcMPredMort[guildcase][current_id] += (pred_mort) * mort_scalar * mortality_scalar;

	if(bm->track_contaminants){
		Contaminant_Record_Death(bm, guildcase, chrt, nums);
	}

	stock_id = bm->group_stock[guildcase][bm->current_box][bm->current_layer];
	bm->calcTrackedMort[guildcase][chrt][stock_id][ongoingM1_id] += (mortality_scalar * ((nat_mort + pred_mort) * FunctGroupArray[guildcase].speciesParams[Mdt_id]));

	/**
	 if((bm->dayt > bm->checkstart) && ((guildcase == FMM_id) || (guildcase == FMN_id)) && bm->newweek){
	 fprintf(llogfp, "time: %e, sp: %d, nat_mort (st: %e, mL: %e, mQ: %e): %e, pred_mort: %e\n",
	 bm->dayt, guildcase, starving* nums, mL*nums, mQ*nums*nums, nat_mort, pred_mort);
	 }
     **/

    /**
	if ((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)) {
		fprintf(llogfp,
				"Vert_Mortality outcomes: guildcase = %d, chrt = %d, stage = %d, nums = %e, starving = %e, mL = %e, mQ = %e, pred_mort = %e, nat_mort = %e, mortality = %e, SBtoDR = %.20e, FDStoDR = %.20e, waste = %.20e\n",
				guildcase, chrt, stage, nums, starving, mL, mQ, pred_mort, nat_mort, *mort, SBtoDR, FDStoDR, *waste);
	}
    **/

	return;

}

/**
 *	\brief Top level handling of vertebrates - putting all repetitive calls and
 *assignments here instead of Water_Column_Box and Epibenthic_Box()
 *
 */
double Do_Vertebrate_Living(MSEBoxModel *bm, FILE *llogfp, int guildcase, HABITAT_TYPES level_id, BoxLayerValues *boxLayerInfo, double DLsed, double DRsed, double DCsed,
		double ***spPREYinfo, double ***spGRAZEinfo, double **spCATCHGRAZEinfo, double ***spSPinfo) {
	double TotSP, E1_sp, E2_sp, E3_sp, E4_sp, avail_BB, PB_scale, BB_scale, VGrowth, VGrazelive, FRC_V, Vtotchrt, Vmortality, VreleaseNHchrt,
			VprodDLchrt, VprodDRchrt, test_value, SNcost_sp, RNcost_sp;
	int flag_sp, active_sp, predcase_sp, cohort, preyID, prey_chrt, hab;
	int do_level = 0;
	int isGlobal = (FunctGroupArray[guildcase].diagTol == 2 && it_count == 1);
	double avail_Ice_Bact;

    if ((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)) {
		fprintf(llogfp, "Do_Vertebrate_Living parameters: guildcase = %d, level_id = %d, PB_DL = %.20e, PB_DR = %.20e, BB_DL = %.20e, BB_DR = %.20e, DLsed = %.20e, DRsed = %.20e, DCsed = %.20e\n", guildcase, level_id, boxLayerInfo->PB_DL, boxLayerInfo->PB_DR, boxLayerInfo->BB_DL, boxLayerInfo->BB_DR, DLsed, DRsed, DCsed);
	}
    
    flag_sp = (int) (FunctGroupArray[guildcase].speciesParams[flag_id]);
    active_sp = (int) (FunctGroupArray[guildcase].speciesParams[active_id]);
    predcase_sp = (int) (FunctGroupArray[guildcase].speciesParams[predcase_id]);
    
    if (level_id == EPIFAUNA)
		do_level = 1;
	else if ((bm->current_layer != 0) || waterboundary)
		do_level = 1;

	TotSP = 0.0;
    
    if (flag_sp && active_sp && do_level) {
		/* Initialise cumulative quantities */
        
        E1_sp = FunctGroupArray[guildcase].speciesParams[E1_id];
		E2_sp = FunctGroupArray[guildcase].speciesParams[E2_id];
		E3_sp = FunctGroupArray[guildcase].speciesParams[E3_id];
		E4_sp = FunctGroupArray[guildcase].speciesParams[E4_id];

		PB_scale = FunctGroupArray[guildcase].speciesParams[PBscale_id];
		BB_scale = FunctGroupArray[guildcase].speciesParams[BBscale_id];

		/* Diagnostic information storage */
		boxLayerInfo->DebugInfo[guildcase][level_id][DiagnostNH_id] = 0.0;
		boxLayerInfo->DebugInfo[guildcase][level_id][DiagnostDL_id] = 0.0;
		boxLayerInfo->DebugInfo[guildcase][level_id][DiagnostDR_id] = 0.0;
		boxLayerInfo->DebugInfo[guildcase][level_id][DiagnostDLsed_id] = 0.0;
		boxLayerInfo->DebugInfo[guildcase][level_id][DiagnostDRsed_id] = 0.0;

		/* If in epibenthic layer find availability of benthic groups in sediment */
		if (level_id != WC)
			Calculate_Sediment_Prey_Avail(bm, boxLayerInfo, guildcase, spPREYinfo, &avail_BB);
		else
			avail_BB = 0.0;

		/* If this is the top layer and ice is active allow grazing into the ice layer */
		if(bm->ice_on == TRUE){
			if(level_id == WC && bm->current_layer == bm->boxes[bm->current_box].nz - 1){
				Calculate_Ice_Prey_Avail(bm, boxLayerInfo, guildcase, spPREYinfo, &avail_Ice_Bact);
				/* for now ignoring the amount of ice bacteria available in ice - we are not scaling based on nutrient depth  so the avail_Ice_Bact value is ignore */
			}
		}

		/* Do fish feeding and mortality processes */
		for (cohort = 0; cohort < FunctGroupArray[guildcase].numCohortsXnumGenes; cohort++) {
            if ((spSPinfo[guildcase][cohort][SN_id] > bm->min_pool) && (spSPinfo[guildcase][cohort][DEN_id] > bm->min_dens)) {


				/* Note that only non-vertebrate prey read in here, vertebrate prey
				 dealt with directly in the Vertebrate_Activities() subroutine */

				Vertebrate_Activities(bm, boxLayerInfo, level_id, llogfp, guildcase, spSPinfo[guildcase][cohort][SN_id], spSPinfo[guildcase][cohort][RN_id], spSPinfo[guildcase][cohort][DEN_id], predcase_sp, cohort, E1_sp, E2_sp, E3_sp, E4_sp, PB_scale, avail_BB * BB_scale, spPREYinfo,
						spGRAZEinfo, spCATCHGRAZEinfo, spSPinfo, &VGrowth, &VGrazelive, &FRC_V, &Vtotchrt, &Vmortality, &VreleaseNHchrt,
						&VprodDLchrt, &VprodDRchrt);

				FunctGroupArray[guildcase].GrazeLive[cohort] = VGrazelive;
                
                test_value = VGrowth;
                if(bm->flag_replicated_old)
                    test_value = FRC_V;
                
                if ((test_value >= 0.0) || !bm->flag_shrinkfat) {   // Should this be a test on test_value not FRC_V? (can FRC_V ever be < 0? Yes if shrinkfat is not on... I think
                    if (bm->flag_dynamicXRS) {
                        SNcost_sp = FunctGroupArray[guildcase].speciesParams[SNcost_id];
                        RNcost_sp = FunctGroupArray[guildcase].speciesParams[RNcost_id];
                        FunctGroupArray[guildcase].grow[cohort][SN_id] = SNcost_sp * FRC_V * VGrowth; // allocation to structure entails cost of growth, so there is inefficiency in addition to assimilation inefficiency
                        FunctGroupArray[guildcase].grow[cohort][RN_id] = RNcost_sp * (VGrowth - FRC_V * VGrowth);
                    } else {
                        FunctGroupArray[guildcase].grow[cohort][SN_id] = FRC_V * VGrowth;
                        FunctGroupArray[guildcase].grow[cohort][RN_id] = VGrowth - FRC_V * VGrowth;
                    }
                } else if (bm->flag_shrinkfat && (test_value < 0.0)) { // No flag_shrinkfat check here as will be trapped by first half of if statement regardless
                    if (bm -> flag_dynamicXRS) {
                        RNcost_sp = FunctGroupArray[guildcase].speciesParams[RNcost_id];
                        FunctGroupArray[guildcase].grow[cohort][RN_id] = 1.0 / RNcost_sp * VGrowth; // if there is a cost in converting energy to RN, the same inverse cost applies when
                    } else {
                        FunctGroupArray[guildcase].grow[cohort][RN_id] = VGrowth;  // Take deficit from the reserve so this represents living off fat
                    }
                }
                
                
				FunctGroupArray[guildcase].dead[cohort] += Vmortality;
				TotSP += Vtotchrt;
                
                /**
                if (guildcase == bm->which_check){
                //if ((guildcase == 33) || ((guildcase > 8) && (guildcase < 12))) {
                    fprintf(llogfp,"Time: %e box%d-%d %s-%d has new dead %e as Vmortality: %e\n",
                        bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guildcase].groupCode, cohort, FunctGroupArray[guildcase].dead[cohort], Vmortality);
                }
                **/
                
				boxLayerInfo->DetritusProd[WC][DLdet_id] += VprodDLchrt;
				boxLayerInfo->DetritusProd[WC][DRdet_id] += VprodDRchrt;

				if(bm->track_atomic_ratio == TRUE){

					/* Keep track of P ratio in DR and DL */
					Gain_Element(bm, boxLayerInfo, WC, RefDetIndex, 0, guildcase, cohort, VprodDRchrt, level_id, isGlobal);
					Gain_Element(bm, boxLayerInfo, WC, LabDetIndex, 0, guildcase, cohort, VprodDLchrt, level_id, isGlobal);

					/* P release due to respiration and excretion*/
					Calculate_Element_Release(bm, boxLayerInfo, guildcase, VreleaseNHchrt, WC, level_id);
				}

				/* Contaminants transfer */
				if(bm->track_contaminants){
                    
                    /* Gains in DL and DR due to mortality 0- pass biomass as 0.0 as pulling information from spSPinfo */
                    Group_Transfer_Contaminant(bm, boxLayerInfo, WC, WC, LabDetIndex, 0, guildcase, cohort, VprodDLchrt, spSPinfo, 0.0, bm->dtsz_stored, 0, 33);
					Group_Transfer_Contaminant(bm, boxLayerInfo, WC, WC, RefDetIndex, 0, guildcase, cohort, VprodDRchrt, spSPinfo, 0.0, bm->dtsz_stored, 0, 34);

					/* Release contaminants back into the water column - pass 0.0 for biomass as claculation it off spSPinfo */
                    //fprintf(bm->logFile,"Calling Gain_Contaminants from Do_Vertebrate_Living\n");
					Gain_Contaminants(bm, boxLayerInfo, WC, WC, guildcase, cohort, VreleaseNHchrt, spSPinfo, 0.0);
				}

				boxLayerInfo->NutsProd[WC][NH_id] += VreleaseNHchrt;

				/* Update global fluxes */
				boxLayerInfo->DetritusProdGlobal[level_id][WC][DLdet_id] += VprodDLchrt;
				boxLayerInfo->DetritusProdGlobal[level_id][WC][DRdet_id] += VprodDRchrt;

				boxLayerInfo->NutsProdGlobal[level_id][WC][NH_id] += VreleaseNHchrt;

				for (preyID = 0; preyID < bm->K_num_tot_sp; preyID++) {
					for (prey_chrt = 0; prey_chrt < FunctGroupArray[preyID].numCohortsXnumGenes; prey_chrt++) {
                        if(bm->flagWCVert && (FunctGroupArray[preyID].groupAgeType == AGE_STRUCTURED)) {
                        	if(GRAZEinfo[preyID][prey_chrt][WC] > 0){
                        		UpdateTrackedMort(bm, llogfp, guildcase, cohort, level_id, WC, preyID, prey_chrt, boxLayerInfo, 1.0, 1);
                        	}
						} else {
							for (hab = WC; hab < bm->num_active_habitats; hab++) {
								if(GRAZEinfo[preyID][prey_chrt][hab] > 0){
									UpdateTrackedMort(bm, llogfp, guildcase, cohort, level_id, (HABITAT_TYPES)hab, preyID, prey_chrt, boxLayerInfo, 1.0, 1);
								}
							}
							/*
							 if((bm->dayt > 100.0)& (bm->current_box == bm->checkbox)){
							 fprintf(llogfp,"Time: %e, it_count: %d, calcM-%s-pred%s: %e, GRAZEinfo: %e bm->INVERT_PREYeaten: %e bm->INVERT_PREYeatenGlobal: %e (sp: %d, availfish: %d)\n",
							 bm->dayt, it_count, FunctGroupArray[guild].groupCode[prey], FunctGroupArray[guildcase].groupCode, bm->calcMnumPerPred[prey][guildcase][current_id], GRAZEinfo[sp], bm->INVERT_PREYeaten[sp], bm->INVERT_PREYeatenGlobal[level_id][sp], sp, availfish_id);
							 }
							 */

						}
					}
				}

				/* Diagnostic information storage */
				boxLayerInfo->DebugInfo[guildcase][level_id][DiagnostNH_id] += VreleaseNHchrt;
				boxLayerInfo->DebugInfo[guildcase][level_id][DiagnostDL_id] += VprodDLchrt - spGRAZEinfo[LabDetIndex][0][WC];
				boxLayerInfo->DebugInfo[guildcase][level_id][DiagnostDR_id] += VprodDRchrt - spGRAZEinfo[RefDetIndex][0][WC];
				boxLayerInfo->DebugInfo[guildcase][level_id][DiagnostDLsed_id] += -spGRAZEinfo[LabDetIndex][0][SED];
				boxLayerInfo->DebugInfo[guildcase][level_id][DiagnostDRsed_id] += -spGRAZEinfo[RefDetIndex][0][SED];
			} else {
				FunctGroupArray[guildcase].grow[cohort][SN_id] = 0.0;
				FunctGroupArray[guildcase].grow[cohort][RN_id] = 0.0;
				FunctGroupArray[guildcase].dead[cohort] += 0.0;
				FunctGroupArray[guildcase].GrazeLive[cohort] = 0.0;
			}

		}
	} else {
		TotSP = 0.0;

		for (cohort = 0; cohort < FunctGroupArray[guildcase].numCohortsXnumGenes; cohort++) {
			FunctGroupArray[guildcase].grow[cohort][SN_id] = 0.0;
			FunctGroupArray[guildcase].grow[cohort][RN_id] = 0.0;
			FunctGroupArray[guildcase].dead[cohort] += 0.0;
			FunctGroupArray[guildcase].GrazeLive[cohort] = 0.0;

			if (flag_sp)
				TotSP += (spSPinfo[guildcase][cohort][SN_id] + spSPinfo[guildcase][cohort][RN_id]) * spSPinfo[guildcase][cohort][DEN_id] / bm->cell_vol;
		}
	}
	if (bm->debug == debug_prey_biology_process && bm->dayt >= bm->checkstart && bm->dayt < bm->checkstop) {
		fprintf(llogfp, "Do_Vertebrate_Living %d\n", guildcase);
		Print_Eat_Diagnostics(bm, logfp, guildcase, level_id, 2);
		Print_Eat_Diagnostics(bm, logfp, guildcase, level_id, 1);
	}

	return TotSP;
}

/**
 *	\brief Vertebrate activity routine (i.e. growth, feeding, mortality etc)
 *
 */
void Vertebrate_Activities(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitatType, FILE *llogfp, int guildcase, double SN, double RN, double NUMS, int predcase, int chrt, double E1_sp, double E2_sp, double E3_sp, double E4_sp, double PB_scale, double BB_scale, double ***spPREYinfo, double ***spGRAZEinfo, double **spCATCHGRAZEinfo, double ***spSPinfo, double *Growth, double *GrazeLive, double *FRCsp, double *Tot_sp, double *Mort, double *ReleaseNH, double *ProdnDL, double *ProdnDR) {
	int flagdem, i, sp_feed_while_spawn, sp_spawn_now, spawn_period, spawn_date;
	double C_sp, mum_sp, KL_sp, KU_sp, pR_sp, FDM_sp, FDG_sp, FDL_sp, Weight, FDGDL_sp, Density, Biomass, vl_sp = 1.0, ht_sp = 1.0, FDGDR_sp, eatBact,
			sp_Growth, Resp = 0.0, FRC_sp, sp_Mortality, sp_Wastes, Mort_biomass, sp_GrazePlant, SPgrazeDR, SPgrazeDL, eatBactBB, eatBactPB, sp_mat_pcnt,
			sp_GrazeFeed, X_RS, avail_intake;
	int habitat, preyID, h, kij;
	int overall_checkday = (int) (floor(bm->dayt));
	double dummy = MAXDOUBLE;
	double overwinter_leave = 0.0;
	double overwinter_return = 0.0;
	double growth_scalar = Ecology_Get_Growth_Scalar(bm, guildcase, chrt);
	double eatIceBact;

	*Growth = 0;
	*GrazeLive = 0;
	*FRCsp = 0;
	*Tot_sp = 0;
	*Mort = 0;
	*ReleaseNH = 0;
	*ProdnDL = 0;
	*ProdnDR = 0;

	if (!bm->flagfishrates) {
		C_sp = FunctGroupArray[guildcase].scaled_C[chrt];
		mum_sp = FunctGroupArray[guildcase].scaled_mum[chrt];
	} else {
		C_sp = (SN + RN) * FunctGroupArray[guildcase].scaled_C[chrt];
		mum_sp = (SN + RN) * FunctGroupArray[guildcase].scaled_mum[chrt];
	}

	/* Apply growth scalar */
	mum_sp = mum_sp * growth_scalar;

    X_RS = FunctGroupArray[guildcase].X_RS[chrt];
    KL_sp = FunctGroupArray[guildcase].speciesParams[KL_id];
	KU_sp = FunctGroupArray[guildcase].speciesParams[KU_id];
	pR_sp = FunctGroupArray[guildcase].speciesParams[pR_id];
	FDM_sp = FunctGroupArray[guildcase].speciesParams[FDMort_id];
	FDL_sp = FunctGroupArray[guildcase].speciesParams[FDL_id];
	FDG_sp = FunctGroupArray[guildcase].speciesParams[FDG_id];
	FDGDL_sp = FunctGroupArray[guildcase].speciesParams[FDGDL_id];
	FDGDR_sp = FunctGroupArray[guildcase].speciesParams[FDGDR_id];
	flagdem = (int) (FunctGroupArray[guildcase].speciesParams[flagdem_id]);
	sp_feed_while_spawn = (int) (FunctGroupArray[guildcase].speciesParams[feed_while_spawn_id]);

	spawn_date = EMBRYO[guildcase].Spawn_Now[chrt][EMBRYO[guildcase].next_spawn];
	spawn_period = (int) (FunctGroupArray[guildcase].speciesParams[spawn_period_id]);

	if((overall_checkday >= spawn_date) && (overall_checkday <= (spawn_date + spawn_period))){
		sp_spawn_now = 1;
	} else {
		sp_spawn_now = 0;
	}

	if ((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)) {
		fprintf(llogfp, "Vertebrate_Activities: guildcase = %d, chrt = %d, C_sp = %e, mum_sp = %e, sp_spawn_now = %d, E1_sp = %e, E2_sp = %e, E3_sp = %e, E4_sp = %e, FDG_sp = %e\n",
				guildcase, chrt, C_sp, mum_sp, sp_spawn_now, E1_sp, E2_sp, E3_sp, E4_sp, FDG_sp );
	}

	if (sp_spawn_now)
		sp_mat_pcnt = FunctGroupArray[guildcase].scaled_FSPB[chrt];
	else
		sp_mat_pcnt = 0.0;

	//fprintf(llogfp, "%s-%d sp_mat_pcnt = %e\n", FunctGroupArray[guildcase].groupCode, chrt, sp_mat_pcnt);

	/* Local biomass for the cohort */
    Weight = SN + RN;
	Density = NUMS / bm->cell_vol;
	Biomass = Weight * Density;

	if (Biomass < 0)
		quit("biomass of vertebrate group %d, cohort %d is negative on day %e, in box %d with weight = %e (sn = %e, rn = %e) and density = %e (nums = %e, vol = %e) \n", guildcase, chrt, bm->dayt, bm->current_box, Weight, SN, RN, Density, NUMS, bm->cell_vol);
	*Tot_sp = Biomass;

	/* Feeding */
	if (predcase > 4)
		Determine_Fish_Feeding_Prms(bm, guildcase, SN, RN, X_RS, &vl_sp, &ht_sp);
	else if (predcase == 3) {
		quit("Invalid predcase for a vertebrate\n");
	} else {
		vl_sp = 1.0;
		ht_sp = 1.0;
	}

	if (flagdem)
		vl_sp = vl_sp * 0.5;

	Eat(bm, llogfp, predcase, guildcase, chrt, Density, C_sp, mum_sp, KL_sp, KU_sp, vl_sp, ht_sp, E1_sp, E2_sp, E3_sp, E4_sp, sp_feed_while_spawn, sp_spawn_now, sp_mat_pcnt, spPREYinfo, spGRAZEinfo, spCATCHGRAZEinfo, Biomass);

	//fprintf(llogfp,"Time: %e box%d-%d %s-%d\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guildcase].groupCode, chrt);

	eatBactPB = 0;
	eatBactBB = 0.0;
	eatIceBact = 0.0;

	/* Add labile det */
	eatBactPB += spGRAZEinfo[LabDetIndex][0][WC] * boxLayerInfo->PB_DL / (spPREYinfo[LabDetIndex][0][WC] + small_num);
	eatBactBB += spGRAZEinfo[LabDetIndex][0][SED] * boxLayerInfo->BB_DL / (spPREYinfo[LabDetIndex][0][SED] + small_num);

	if(bm->ice_on == TRUE){

		/* Should we check the other habitats? Can ice bacteria be in the water column? What happens when we get melt? */
		eatIceBact += spGRAZEinfo[LabDetIndex][0][ICE_BASED] * boxLayerInfo->ICEB_DL / (spPREYinfo[LabDetIndex][0][ICE_BASED] + small_num);
		eatIceBact += spGRAZEinfo[RefDetIndex][0][ICE_BASED] * boxLayerInfo->ICEB_DR / (spPREYinfo[RefDetIndex][0][ICE_BASED] + small_num);
	}

	/* Add ref det */
	eatBactPB += spGRAZEinfo[RefDetIndex][0][WC] * boxLayerInfo->PB_DR / (spPREYinfo[RefDetIndex][0][WC] + small_num);
	eatBactBB += spGRAZEinfo[RefDetIndex][0][SED] * boxLayerInfo->BB_DR / (spPREYinfo[RefDetIndex][0][SED] + small_num);

	eatBactPB = eatBactPB * PB_scale;
	eatBactBB = eatBactBB * BB_scale;

	SPgrazeDR = 0.0;
	SPgrazeDL = 0.0;

	for (habitat = WC; habitat <  bm->num_active_habitats; habitat++) {
		SPgrazeDR += spGRAZEinfo[RefDetIndex][0][habitat];
		SPgrazeDL += spGRAZEinfo[LabDetIndex][0][habitat];
	}

	spGRAZEinfo[pelagicBactIndex][0][WC] = eatBactPB;
	spGRAZEinfo[SedBactIndex][0][SED] = eatBactBB;
	eatBact = eatBactPB + eatBactBB + eatIceBact;
	FunctGroupArray[guildcase].GrazeLive[chrt] += eatBact;


	/* handle grazing of plants */
	sp_GrazePlant = 0.0;

	for (i = 0; i < bm->K_num_tot_sp; i++) {
	preyID = i;
		if (FunctGroupArray[preyID].isPrimaryProducer == TRUE) {
			for (h = WC; h <  bm->num_active_habitats; h++) {
				habitat = h;

				/* if this prey is epibenthic scale to get units in m^2 not m^3*/
				for (kij = 0; kij < FunctGroupArray[preyID].numCohortsXnumGenes; kij++) {
					if (FunctGroupArray[preyID].habitatType == EPIFAUNA){
						sp_GrazePlant += spGRAZEinfo[preyID][kij][habitat] / smLayerThick;
					}else{
						sp_GrazePlant += spGRAZEinfo[preyID][kij][habitat];
					}
				}
			}
		}
	}

	/**/
	if (_isnan((double)FunctGroupArray[guildcase].GrazeLive[chrt])) {
	//if ((bm->which_check == guildcase) && (bm->current_box == bm->checkbox) && (chrt == 0)) {
		fprintf(llogfp, "%s, sp_GrazeLive: %Le, spGRAZEinfo[DC][SED]: %.20e, spGRAZEinfo[DC][WC]: %.20e, eatBact: %.20e\n",
				FunctGroupArray[guildcase].groupCode, FunctGroupArray[guildcase].GrazeLive[chrt], spGRAZEinfo[CarrionIndex][0][SED],
				spGRAZEinfo[CarrionIndex][0][WC], eatBact);

		/* Print the diagnostic grazing information to the log file */
		Print_Diagnostic_Grazing(bm, llogfp, guildcase, spGRAZEinfo);
	}
	/**/


	*GrazeLive = (double)FunctGroupArray[guildcase].GrazeLive[chrt];

	/* Handle grazing of aquaculture feed */
	if(FunctGroupArray[guildcase].isCultured || FunctGroupArray[guildcase].isSupplemented)
		sp_GrazeFeed = spGRAZEinfo[AquacultFeedIndex][0][WC];
	else
		sp_GrazeFeed = 0.0;

#ifndef VERTS_TURN_OFF_GROWTH

	sp_Growth = growth_scalar * (E1_sp * (((double)FunctGroupArray[guildcase].GrazeLive[chrt] - sp_GrazePlant) + sp_GrazeFeed) + E2_sp * sp_GrazePlant + E3_sp * SPgrazeDL + E4_sp * SPgrazeDR);

	/* Track growth due to non-vertebrates - vertebrates are handled in Partition_Grazed_fish */
	if(bm->track_atomic_ratio == TRUE){
		Vertebrates_Transfer_To_Pred(bm, boxLayerInfo, spGRAZEinfo, guildcase, chrt, growth_scalar, E1_sp, E2_sp, E3_sp, E4_sp, SPgrazeDL, SPgrazeDR, WC, Density);
	}

#else
	sp_Growth = growth_scalar *  0;

#endif
	/*
	//if ((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)) {
	if ((bm->which_check == guildcase) && (bm->current_box == bm->checkbox) && (chrt == 0)) {
		fprintf(bm->logFile, "%s:%d Vert Activities. GrazeLive = %.20Le, grazePlant = %.20e, grazeDL = %.20e, grazeDR = %.20e\n", FunctGroupArray[guildcase].groupCode,
				chrt, FunctGroupArray[guildcase].GrazeLive[chrt], sp_GrazePlant, SPgrazeDL, SPgrazeDR);
	}
	**/
    
    switch (flagresp) { //ASTAFIX - make sure all respirations cases are read, because now it just assumes boolean flag
        case no_resp:
            Resp = 0.0; /* Turning off respiration means the fish model is essentially/very similar to a Deriso-Schnute */
            break;
        case base_resp:
            Fish_Respiration(bm, guildcase, SN, RN, X_RS, Density, &Resp);
            break;
        case deb_resp: // DEB model for respitaiton
            Fish_Respiration_DEB(bm, guildcase, SN, RN, X_RS, Density, &Resp);
            break;
    }

	*Growth = (sp_Growth - Resp) / (Density + small_num);
    avail_intake = *Growth;
    
    if (bm->flag_dynamicXRS && (avail_intake > 0)) {  // Test needs to be here for consistency with old code
        Partition_Weight_Dynamic(bm, guildcase, SN, RN, X_RS, &FRC_sp, avail_intake, llogfp);
    }
    else {
        Partition_Weight(bm, guildcase, pR_sp, SN, RN, X_RS, &FRC_sp, llogfp);
    }
	*FRCsp = FRC_sp;

	/**/
	if (!(_finite(*Growth))) {
		fprintf(llogfp, "%s, Growth: %e, Resp: %e, Density: %e\n", FunctGroupArray[guildcase].groupCode, sp_Growth, Resp, Density);
		/* Print the diagnostic grazing information to the log file */
		Print_Diagnostic_Grazing(bm, llogfp, guildcase, spGRAZEinfo);
	}

	/**
	if ((bm->which_check == guildcase) && (bm->current_box == bm->checkbox) && (chrt == 0)) {
		fprintf(llogfp, "Time: %e, %s-%d, Growth: %.20e, Resp: %.20e, Density: %.20e\n", 
			bm->dayt, FunctGroupArray[guildcase].groupCode, chrt, sp_Growth, Resp, Density);
	}
	**/

	assert((_finite(*Growth)));

	Vert_Mortality(bm, guildcase, chrt, SN, RN, Biomass, Density, Weight, &sp_Mortality, &sp_Wastes, llogfp);
	Mort_biomass = sp_Mortality * Weight / bm->cell_vol;

	/**
	if ((bm->which_check == guildcase) && (bm->current_box == bm->checkbox)) {
		 fprintf(llogfp,"Time: %e, box%d-%d, %s-%d, sp_mort: %.20e, Mort_biomass = %.20e\n",
				 bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guildcase].groupCode, chrt, sp_Mortality, Mort_biomass);
	 }
	 **/

	/* Check whether overwintering or not */
	FunctGroupArray[guildcase].speciesParams[overwintering_id] = Check_OverWintering(bm, llogfp, guildcase);
	if(FunctGroupArray[guildcase].speciesParams[overwintering_id] > 0)
		Encystment(bm, llogfp, guildcase, NUMS, dummy, &overwinter_return, &overwinter_leave);  // Enter nutrients as MAXDOUBLE as invertebrate consumers don't trigger on this
	else {
		overwinter_return = 0.0;
		overwinter_leave = 0.0;
	}

	*Mort = sp_Mortality + overwinter_leave - overwinter_return;  /* Returning animals would lead to negative mortality
	 (so potential bug source if things go awry and overwintering not really happening, will need to keep an eye on this */

	/**
	if ((bm->which_check == guildcase) && (bm->current_box == bm->checkbox)) {
		fprintf(llogfp,"Time: %e box%d-%d %s-%d, NUMS: %.20e, Mort: %.20e, sp_Mort: %.20e, leave: %.20e, return: %.20e\n",
			bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guildcase].groupCode, chrt, NUMS, (*Mort), sp_Mortality, overwinter_leave, overwinter_return);
	}
	**/

	*ReleaseNH = Mort_biomass * (1.0 - FDM_sp) + ((double)FunctGroupArray[guildcase].GrazeLive[chrt] + sp_GrazeFeed) * (1.0 - E1_sp) * (1.0 - FDG_sp) + sp_GrazePlant
			* (1.0 - E2_sp) * (1.0 - FDG_sp) + SPgrazeDL * (1.0 - E3_sp) * (1.0 - FDGDL_sp) + SPgrazeDR * (1.0 - E4_sp) * (1.0 - FDGDR_sp) + Resp;
	*ProdnDL = (((double)FunctGroupArray[guildcase].GrazeLive[chrt] + sp_GrazeFeed) * (1.0 - E1_sp) * FDG_sp + sp_GrazePlant * (1.0 - E2_sp) * FDG_sp + SPgrazeDL * (1.0
			- E3_sp) * FDGDL_sp + SPgrazeDR * (1.0 - E4_sp) * FDGDR_sp + Mort_biomass * FDM_sp) * FDL_sp;
	*ProdnDR = (((double)FunctGroupArray[guildcase].GrazeLive[chrt] + sp_GrazeFeed) * (1.0 - E1_sp) * FDG_sp + sp_GrazePlant * (1.0 - E2_sp) * FDG_sp + SPgrazeDL * (1.0
			- E3_sp) * FDGDL_sp + SPgrazeDR * (1.0 - E4_sp) * FDGDR_sp + Mort_biomass * FDM_sp) * (1.0 - FDL_sp) + sp_Wastes;

	/**
	assert((_finite(*ReleaseNH)));
	//if(!(_finite(*ReleaseNH)) || !(_finite(*Mort))){
	if ((bm->dayt > bm->checkstart) && (bm->debug == debug_deposit) && ((strcmp(FunctGroupArray[guildcase].groupCode, "FMM") == 0) || (strcmp(
		FunctGroupArray[guildcase].groupCode, "FMN") == 0))) {
		fprintf(llogfp, "Time: %e Box%d-%d %s, Growth: %.20e, GrazeLive: %.20e, FRCsp: %.20e, Tot_sp: %.20e, Mort: %.20e (%.20e), ReleaseNH: %.20e, ProdnDL: %.20e, ProdnDR: %.20e\n",
				bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guildcase].groupCode, *Growth, *GrazeLive, *FRCsp, *Tot_sp, *Mort, Mort_biomass,
				*ReleaseNH, *ProdnDL, *ProdnDR);
		fprintf(
				llogfp,
				"%s Density: %.20e, sp_Growth: %.20e, E1_sp: %.20e, sp_GrazeLive: %Le, E2_sp: %.20e, sp_GrazePlant: %.20e, E3_sp: %.20e, eatlabdet: %.20e, E4_sp: %.20e, eatrefdet: %.20e, Resp: %.20e\n",
				FunctGroupArray[guildcase].groupCode, Density, sp_Growth, E1_sp, FunctGroupArray[guildcase].GrazeLive[chrt], E2_sp, sp_GrazePlant,
				E3_sp, SPgrazeDL, E4_sp, SPgrazeDR, Resp);
		// Print the diagnostic grazing information to the log file
		//Print_Diagnostic_Grazing(bm, llogfp, guildcase, spGRAZEinfo);

	}
	**/

	if ((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)) {
		fprintf(llogfp,
				"Vertebrate_Activities outcomes: guildcase = %d, chrt = %d, eatBactPB = %.20e, eatBactBB = %.20e, GrazeLive = %.20e, Growth = %.20e, FRCsp = %.20e, Mort = %.20e, ReleaseNH = %.20e, ProdnDL = %.20e, ProdnDR = %.20e\n",
				guildcase, chrt, eatBactPB, eatBactBB, *GrazeLive, *Growth, *FRCsp, *Mort, *ReleaseNH, *ProdnDL, *ProdnDR);
	}

	return;
}

/**
 * Static Functions.
 *
 */

/**
 * \brief Determine handling and search times for fish feeding
 *
 */
static void Determine_Fish_Feeding_Prms(MSEBoxModel *bm, int guildcase, double SN, double RN, double X_RS, double *vl, double *ht) {
	double Relative_reserve, wgteffect, vla, vlb, hta, htb;
	double numsec = 86400.0; /* number of seconds a day */

	vla = FunctGroupArray[guildcase].speciesParams[vla_id];
	vlb = FunctGroupArray[guildcase].speciesParams[vlb_id];
	hta = FunctGroupArray[guildcase].speciesParams[hta_id];
	htb = FunctGroupArray[guildcase].speciesParams[htb_id];

	/* Condition of fish/vertebrate and its effect on ability to handle and find food */
	Relative_reserve = RN / (X_RS * SN);
	wgteffect = 1.0;
	if (Relative_reserve < Kthresh1)
		wgteffect = KHTD;
	if (Relative_reserve < Kthresh2)
		wgteffect = KHTI;

	/* Search volume - convert N to C here so can use existing parameter values */
	*vl = vla * pow(SN * bm->X_CN, vlb) / numsec;

	/* Handling time - convert N to C here so can use existing parameter values */
	*ht = wgteffect * hta * pow(SN * bm->X_CN, -htb) * numsec;

	return;
}

/**
 * \brief Respiration (basal) for vertebrates
 *
 */
void Fish_Respiration(MSEBoxModel *bm, int guildcase, double SN, double RN, double X_RS, double Dens, double *respire) {
	double restresp, KA, KB, KST, relres, Ktmp;

	KA = FunctGroupArray[guildcase].speciesParams[KA_id];
	KB = FunctGroupArray[guildcase].speciesParams[KB_id];
	KST = FunctGroupArray[guildcase].speciesParams[KST_id];
	Ktmp = FunctGroupArray[guildcase].speciesParams[Ktmp_id];

	/* Calculate relative reserves */
	relres = RN / (X_RS * SN);

	/* Calculate rest respiration */
	restresp = exp(Ktmp * H2Otemp) * KA * pow((SN + RN) * bm->X_CN, KB) / 86400.0;
	if (relres < Kthresh2)
		restresp = KST * restresp;


	/* Individual rate now taken to level of the population as doing growth at population level */
	*respire = restresp * Dens;

	return;

}

/**
 * \brief DEB based respiration (basal) for vertebrates - respiration is not reduced at low condition and
 * the power exponent is 1. Instead the KB parameter that serves as an exponent in basal respiration is used as mass specific maintenance cost of reserve
 * So the main difference is that reserve has different constant (mass specific cost) than structure, and reserve is usually much lower (up to 10x smaller)
 *
 */
void Fish_Respiration_DEB(MSEBoxModel *bm, int guildcase, double SN, double RN, double X_RS, double Dens, double *respire) {
    double restresp, KA, KB, Ktmp;
    
    KA = FunctGroupArray[guildcase].speciesParams[KA_id];
    KB = FunctGroupArray[guildcase].speciesParams[KB_id];
    Ktmp = FunctGroupArray[guildcase].speciesParams[Ktmp_id];
    
    /* Calculate rest respiration */
    restresp = ( exp(Ktmp * H2Otemp) * ( KA * SN + KB * RN )) / 86400.0;
    
    /* Individual rate now taken to level of the population as doing growth at population level */
    *respire = restresp * Dens;

    return;
    
}

static void Print_Diagnostic_Grazing(MSEBoxModel *bm, FILE *llogfp, int guildcase, double ***spGRAZEinfo) {
    int habitat, i, kij;
    
    for (habitat = WC; habitat < bm->num_active_habitats; habitat++) {
        for (i = 0; i < bm->K_num_tot_sp; i++) {
            for (kij = 0; kij < FunctGroupArray[i].numCohortsXnumGenes; kij++) {
                fprintf(llogfp, "%s grazes on %s = %e\n", FunctGroupArray[guildcase].groupCode, FunctGroupArray[i].groupCode, spGRAZEinfo[i][kij][habitat]);
            }
        }
    }
}

