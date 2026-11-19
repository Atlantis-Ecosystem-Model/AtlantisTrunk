/**
 * \file atFishMIP_OSP_Bridge.c
 * \ingroup ateconomic
 *
 * \brief Reference bridge between Atlantis's native biomass/catch/effort/
 * price arrays and the FishMIP-OSP plug-in economic models -- ordinary
 * Atlantis C, like every other file in this directory.
 *
 * This is a plain-C reimplementation of the original single-file bridge,
 * atFishMIP_OSP_Bridge.cpp (kept in this directory for reference -- see
 * FishMIP_OSP_Bridge_NOTES.md for the full design rationale, the three
 * modelling options, and everything this bridge deliberately does not do).
 * The split exists because only a small part of that file actually needs
 * C++: the FishMIP-OSP plug-in models (FishMIP_OSP_Plugin_Models.hpp/.cpp,
 * namespace fishmip) take and return C++-only types (NdArray, std::vector,
 * std::optional, StockData, DriverData, ...) with no C ABI, so *something*
 * has to be C++ to call them. But everything else the original file did --
 * reading bm->totfishpop/bm->Effort/Harvest_Get_TotCumCatch(), writing
 * bm->SP_FISHERYprms/bm->TACamt/bm->MARKETprms, the per-year control flow --
 * is plain Atlantis-struct bookkeeping with no C++ dependency at all. This
 * file contains all of that, as ordinary Atlantis C; it calls into the
 * plug-in models only through atFishMIP_OSP_PluginAdapter.h's plain-C
 * interface, implemented by the one remaining C++ file,
 * atFishMIP_OSP_PluginAdapter.cpp -- which never sees MSEBoxModel or any
 * other Atlantis type, and so never needs the `#define export ...` collision
 * workaround the single-file version required for atlantisboxmodel.h's
 * `export` struct field (a C++ reserved word).
 *
 * Reproduces the per-year control flow of TOY_calculate_biomass_dynamics()
 * in FishMIP-OSP_Toy_Model.py -- read biomass, read/derive catch, price the
 * catch, update effort and fishing mortality from the priced catch -- but
 * every quantity that step reads or writes is a real Atlantis array
 * (bm->totfishpop, Harvest_Get_TotCumCatch(), bm->Effort,
 * bm->SP_FISHERYprms[...][saleprice_id]) instead of the toy model's own
 * logistic-population arrays; every call into the plug-in model itself goes
 * through the unmodified functions the adapter wraps.
 *
 * Not yet wired into any call site (see FishMIP_OSP_Bridge_NOTES.md section
 * 4) or into ateconomic/Makefile.am's libateconomic_a_SOURCES -- same
 * "reference, not yet deployed" status as the file it replaces.
 *
 * Author: written as a reference/bridging file, 2026-09-08 (split from the
 * single-file C++ bridge the same day).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <sjwlib.h>
#include <atEconomic.h>

#include "atFishMIP_OSP_Bridge.h"
#include "atFishMIP_OSP_PluginAdapter.h"

/* Global Declarations */
static int CollectFishedSpecies(MSEBoxModel *bm);
static int CollectFleets(MSEBoxModel *bm, int sp);

/* ==========================================================================
 * Set up the OSP structure and load params etc
 * ========================================================================== */

void Create_OSP_Arrays(MSEBoxModel *bm, FILE *llogfp) {
    int sp, nf;
    int SimLength = (int) (ceil((bm->tstop - bm->tstart) / (86400.0)));
    int arr_lengtth;
    SimLength += 1; // Just to be safe
    arr_lengtth = SimLength * bm->K_num_fisheries;
    
    if(verbose > 1) {
        printf("Doing Create_OSP_Arrays\n");
    }
    
    OSP_PARAMS = (OSPStruct *) malloc(sizeof(OSPStruct));
    
    OSP_PARAMS->catchability = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
    OSP_PARAMS->compliance = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
    OSP_PARAMS->invest_driver_U = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
    OSP_PARAMS->invest_driver_V = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
    OSP_PARAMS->fisheryFleetType = Util_Alloc_Init_1D_Int(bm->K_num_fisheries, 0.0);
    
    OSP_PARAMS->ftargetBySp = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
    OSP_PARAMS->prod = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
    OSP_PARAMS->X = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
    OSP_PARAMS->W = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
    OSP_PARAMS->price_by_sp =  Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
    
    OSP_PARAMS->catchBySpFishery = Util_Alloc_Init_1D_Double(bm->K_num_fisheries * bm->K_num_tot_sp, 0.0);
    
    OSP_PARAMS->thisYearEffort = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
    OSP_PARAMS->price_arr = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
    OSP_PARAMS->Y_arr = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
    OSP_PARAMS->effort_arr = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
    OSP_PARAMS->q_arr = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
    OSP_PARAMS->comply_arr = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
    OSP_PARAMS->dU_arr = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
    OSP_PARAMS->dV_arr = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
    OSP_PARAMS->Ftarget_arr = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
    OSP_PARAMS->f_out = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
    OSP_PARAMS->vessel_attrition_M = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
    
    for (nf = 0; nf < bm->K_num_fisheries; ++nf) {
        OSP_PARAMS->catchability[nf] = 1e-5;
        OSP_PARAMS->compliance[nf] = 1.0;
        OSP_PARAMS->invest_driver_U[nf] = 1.0;
        OSP_PARAMS->invest_driver_V[nf] = 0.0;
        OSP_PARAMS->fisheryFleetType[nf] = 0;
        OSP_PARAMS->vessel_attrition_M[nf] = 0.15;
    }
    OSP_PARAMS->risk_invest = 5.0;
    OSP_PARAMS->effort_growth_lim = 1;
    OSP_PARAMS->f_growth_rate_max = 1.3797296614612;
    OSP_PARAMS->f_growth_rate_half_sat = 10.0;
    OSP_PARAMS->dt = 1.0;
    OSP_PARAMS->epsilon = -1.5;
    OSP_PARAMS->Ftarget_placeholder = 0.2;

    OSP_PARAMS->assess_prior_mean_r = 0.3;
    OSP_PARAMS->assess_prior_sd_logr = 1.0;
    OSP_PARAMS->assess_prior_mean_z = 1.0;
    OSP_PARAMS->assess_prior_sd_logz = 1.0;
    OSP_PARAMS->assess_optimizer = 4;
    OSP_PARAMS->assess_min_years = 5;

    OSP_PARAMS->demand_loaded = 0;
    OSP_PARAMS->demand_X_uniform = 0.0;
    OSP_PARAMS->demand_W_uniform = 0.0;
    
    OSP_PARAMS->fished_nsp = CollectFishedSpecies(bm);
  
    //g_catch_n_effort_HistoryCount = 0;
    g_catch_n_effort_History = (EffortHistoryRecord *)malloc(sizeof(EffortHistoryRecord) * (size_t)(bm->K_num_tot_sp));
    
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        g_catch_n_effort_History[sp].targetFishery = Util_Alloc_Init_1D_Int(bm->K_num_fisheries, 0.0);
        g_catch_n_effort_History[sp].nfishery =  CollectFleets(bm, sp) ;
        g_catch_n_effort_History[sp].effortFlat = Util_Alloc_Init_1D_Double(arr_lengtth, 0.0);
        g_catch_n_effort_History[sp].catchFlat = Util_Alloc_Init_1D_Double(arr_lengtth, 0.0);
        g_catch_n_effort_History[sp].recorded = Util_Alloc_Init_1D_Int(SimLength, 0.0);
    }

    return;
}


static void FreeOSPStruct(MSEBoxModel *bm, int done) {
    int sp;
    
    free1d(OSP_PARAMS->catchability);
    free1d(OSP_PARAMS->compliance);
    free1d(OSP_PARAMS->invest_driver_U);
    free1d(OSP_PARAMS->invest_driver_V);
    i_free1d(OSP_PARAMS->fisheryFleetType);
    
    free1d(OSP_PARAMS->ftargetBySp);
    free1d(OSP_PARAMS->prod);
    free1d(OSP_PARAMS->X);
    free1d(OSP_PARAMS->W);
    
    free1d(OSP_PARAMS->thisYearEffort);
    free1d(OSP_PARAMS->price_arr);
    free1d(OSP_PARAMS->Y_arr);
    free1d(OSP_PARAMS->effort_arr);
    free1d(OSP_PARAMS->q_arr);
    free1d(OSP_PARAMS->comply_arr);
    free1d(OSP_PARAMS->dU_arr);
    free1d(OSP_PARAMS->dV_arr);
    free1d(OSP_PARAMS->Ftarget_arr);
    free1d(OSP_PARAMS->f_out);
    free1d(OSP_PARAMS->catchBySpFishery);
    free1d(OSP_PARAMS->price_by_sp);
    i_free1d(OSP_PARAMS->fishedSp);
    free1d(OSP_PARAMS->vessel_attrition_M);
    
    free(OSP_PARAMS);
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        i_free1d(g_catch_n_effort_History[sp].targetFishery);
        free1d(g_catch_n_effort_History[sp].effortFlat);
        free1d(g_catch_n_effort_History[sp].catchFlat);
        i_free1d(g_catch_n_effort_History[sp].recorded);
    }
    free(g_catch_n_effort_History);
    
    return;
}



/* ==========================================================================
 * Helpers: Atlantis array <-> plug-in model array conversions.
 * ========================================================================== */

/** Every functional group Atlantis considers fished (FunctGroupArray[sp]
 *  .isFished, exactly the guard Get_Fish_Prices() and every other
 *  ateconomic C file already uses, e.g. ateconomic/atquota.c:89). This is
 *  the toy model's "nstocks" dimension, restricted to species Atlantis
 *  actually fishes. Returns a malloc'd array of length *nsp_out (caller
 *  frees it); *nsp_out is 0 (and the returned pointer NULL) if nothing is
 *  fished. */
static int CollectFishedSpecies(MSEBoxModel *bm) {
    int sp, nsp_out = 0, count = 0;

    for (sp = 0; sp < bm->K_num_tot_sp; ++sp) {
        if (FunctGroupArray[sp].isFished) {
            ++count;
        }
    }

    nsp_out = count;
    OSP_PARAMS->fishedSp = Util_Alloc_Init_1D_Int(nsp_out, 0.0);
    
    if (count == 0) {
        return 0;
    }

    count = 0;
    for (sp = 0; sp < bm->K_num_tot_sp; ++sp) {
        if (FunctGroupArray[sp].isFished) {
            OSP_PARAMS->fishedSp[count++] = sp;
        }
    }
    
    return nsp_out;
}

/** Every fleet thath fishes each species so can hand only meaningfl fisheries to the assessment */
static int CollectFleets(MSEBoxModel *bm, int sp) {
    int nf, nf_out, count = 0;

    for (nf = 0; nf < bm->K_num_fisheries; ++nf) {
        if (bm->FISHERYtarget[nf][sp] > 0) {
            ++count;
        }
    }

    nf_out = count;
    if (count == 0) {
        return 0;
    }

    count = 0;
    for (nf = 0; nf < bm->K_num_fisheries; ++nf) {
        if (bm->FISHERYtarget[nf][sp] > 0) {
            g_catch_n_effort_History[sp].targetFishery[count++] = nf;
        }
    }
    
    return nf_out;
}

/** bm->Effort is indexed [box][fishery] (confirmed at every dereference
 *  site, e.g. atharvest/atHarvestCatch.c, atmanage/atManageAnnual.c), in
 *  effective boat-days/day (ateconomic/ateconeffort.c:479). The plug-in
 *  model works at the whole-fishery level (one scalar effort per fishery,
 *  like the toy model's effort_EEZ[fleet, eez, t]), so this sums Atlantis's
 *  per-box effort into that one scalar. */
static double AtlantisFisheryEffortTotal(MSEBoxModel *bm, int nf) {
    double total = 0.0;
    int ij;
    for (ij = 0; ij < bm->nbox; ++ij) {
        total += bm->Effort[ij][nf];
    }
    return total;
}

/** The inverse of AtlantisFisheryEffortTotal(): rescales bm->Effort[*][nf]
 *  to a new fishery-wide total while preserving its existing per-box
 *  spatial distribution. If the fishery currently has no effort anywhere
 *  the new total is spread evenly across all boxes instead. */
static void SetAtlantisFisheryEffortTotal(MSEBoxModel *bm, int nf, double newTotal) {
    double oldTotal = AtlantisFisheryEffortTotal(bm, nf);
    int ij;
    double scale, share;
    if (oldTotal <= 0.0) {
        share = newTotal / (double)(bm->nbox);
        for (ij = 0; ij < bm->nbox; ++ij){
            bm->Effort[ij][nf] = share;
        }
        return;
    }
    scale = newTotal / oldTotal; // TODO: Do we need to trap this with small_num inclusion
    for (ij = 0; ij < bm->nbox; ++ij) {
        bm->Effort[ij][nf] *= scale;
    }
}


/** Overlays the loaded driver set (if any) onto an already-built
 *  OSPStruct, per fishery (using OSP_PARAMS->fisheryFleetType[nf] to pick
 *  artisanal vs industrial) and, for the demand curve, once uniformly
 *  across every fished species -- the loaded driver set carries only ONE
 *  commodity's demand curve for ONE reference country, whereas Atlantis
 *  distinguishes many species; applying it uniformly is the simplest
 *  defensible choice, not a claim that every fished species shares the
 *  reference commodity's real-world demand. No-op if
 *  FishMIP_OSP_Bridge_LoadDrivers() was never called or did not succeed
 *  (FishMIP_Adapter_GetDriverValuesForYear() reports ok == 0). */
static void ApplyLoadedDrivers(MSEBoxModel *bm, int calendarYear) {
    FishMIP_DriverYearValues ref;
    FishMIP_DriverYearValues v;
    int nf;

    FishMIP_Adapter_GetDriverValuesForYear(calendarYear, 0, &ref);
    OSP_PARAMS->demand_loaded = ref.ok;
    if (!ref.ok) { // Check information returned from cpp Plug-In-Model
        return;
    }

    OSP_PARAMS->demand_X_uniform = ref.demand_X_fresh;
    OSP_PARAMS->demand_W_uniform = ref.demand_W_fresh;
    OSP_PARAMS->epsilon = ref.epsilon;
    OSP_PARAMS->risk_invest = ref.risk_invest;
    OSP_PARAMS->f_growth_rate_max = ref.f_growth_rate_max;
    OSP_PARAMS->f_growth_rate_half_sat = ref.f_growth_rate_half_sat;
    OSP_PARAMS->effort_growth_lim = ref.effort_growth_lim;

    // TODO: Need to map the params based on mapping of PIM fleet to Atlantis fleet
    
    for (nf = 0; nf < bm->K_num_fisheries; ++nf) {
        FishMIP_Adapter_GetDriverValuesForYear(calendarYear, OSP_PARAMS->fisheryFleetType[nf], &v);
        if (!v.ok) { // Check information returned from cpp Plug-In-Model
            continue;
        }
        if (v.catchability_EEZ > 0.0) {
            OSP_PARAMS->catchability[nf] = v.catchability_EEZ;
        }
        OSP_PARAMS->compliance[nf] = v.compliance;
        OSP_PARAMS->invest_driver_U[nf] = v.invest_driver_U;
        OSP_PARAMS->invest_driver_V[nf] = v.invest_driver_V;
        /* All Atlantis fisheries share one M slot -- see "Fleet-dimension
         * mapping" in Step 4's comment below; the last fishery processed
         * here wins if fisheries mix fleet types, same as the original
         * bridge. */
        OSP_PARAMS->vessel_attrition_M[nf] = v.vessel_attrition_M;
    }
}

/* ==========================================================================
 * One-time driver / external-price loaders (thin pass-throughs to the
 * adapter, declared in atFishMIP_OSP_Bridge.h).
 * ========================================================================== */
int FishMIP_OSP_Bridge_LoadDrivers(const char *dataDir, const char *controlParamsCsvPath,
                                    const char *referenceCountryISO3, int commod, int nosp, int baseCalendarYear) {
    return FishMIP_Adapter_LoadDrivers(dataDir, controlParamsCsvPath, referenceCountryISO3, commod, nosp,
                                        baseCalendarYear);
}

int FishMIP_OSP_Bridge_LoadGlobalPrice(const char *csvPath, const char *yearColumn, int baseCalendarYear) {
    return FishMIP_Adapter_LoadGlobalPrice(csvPath, yearColumn, baseCalendarYear);
}


/* ==========================================================================
 * Step 3b helpers: sourcing plugin_model_2_calc_effort's fishing-mortality
 * target from a stock assessment, in either direction.
 * ========================================================================== */

/** "(a) use the stock assessment from PIM to set [the effort target], then
 *  [that is] used to set the effort and to feed back to TACs": builds this
 *  species' full-history catch (from Atlantis's own per-year storage) and
 *  effort (from this bridge's own rolling record, since Atlantis keeps none
 *  in this shape -- see EffortHistoryFor()), fits the Pella & Tomlinson
 *  model via the adapter's FishMIP_Adapter_RunAssessment() (fit_one_stock()
 *  under the hood -- see FishMIP_OSP_Bridge_NOTES.md sections 2 and 7(i) for
 *  why fit_one_stock() is used directly rather than PIM_assess_stocks()).
 *  Returns 1 and fills outFMSY/outMSY on success, 0 otherwise (not enough
 *  history yet, or the fit didn't converge). */
static int RunPIMAssessment(MSEBoxModel *bm, int sp, double *outFMSY, double *outMSY) {
    int nyears = bm->thisyear + 1; /* Atlantis years 0..thisyear inclusive */
    FishMIP_AssessmentResult result;
    int iy, nf, this_nf;
    //int SimLength = (int) (ceil((bm->tstop - bm->tstart) / (86400.0)));
    //int arr_length = SimLength * bm-> K_num_fisheries;
    int nfishery = g_catch_n_effort_History[sp].nfishery;

    if (nyears < OSP_PARAMS->assess_min_years) {
        return 0;
    }

    Util_Init_1D_Double(OSP_PARAMS->thisYearEffort, bm-> K_num_fisheries, 0.0);
    for (nf = 0; nf < bm->K_num_fisheries; ++nf) {
        OSP_PARAMS->thisYearEffort[nf] = AtlantisFisheryEffortTotal(bm, nf);
    }

    if(g_catch_n_effort_History[sp].firstcall > 0){
        /* If the bridge only started running mid-simulation, earlier years have
         * no recorded entry -- back-fill (permanently -- recorded[iy] is set so
         * a later year never overwrites it again) with the earliest known
         * effort rather than leaving zeros, so the assessment doesn't see a
         * spurious effort collapse at the start of its time series. */
        for (iy = 0; iy < nyears; ++iy) {
            if (g_catch_n_effort_History[sp].recorded[iy]) {
                continue;
            }
            for (nf = 0; nf < g_catch_n_effort_History[sp].nfishery; ++nf) {
                this_nf = g_catch_n_effort_History[sp].targetFishery[nf];
                g_catch_n_effort_History[sp].effortFlat[iy * nfishery + nf] = OSP_PARAMS->thisYearEffort[this_nf];
                g_catch_n_effort_History[sp].catchFlat[iy * nfishery + nf] = Harvest_Get_TotCumCatch(sp, this_nf, iy);
            }
            g_catch_n_effort_History[sp].recorded[iy] = 1;
        }
        g_catch_n_effort_History[sp].firstcall = 0;
    } else {
        /* This year only */
        for (nf = 0; nf < g_catch_n_effort_History[sp].nfishery; ++nf) {
            this_nf = g_catch_n_effort_History[sp].targetFishery[nf];
            g_catch_n_effort_History[sp].effortFlat[bm->thisyear * nfishery + nf] = OSP_PARAMS->thisYearEffort[this_nf];
            g_catch_n_effort_History[sp].catchFlat[bm->thisyear * nfishery + nf] = Harvest_Get_TotCumCatch(sp, this_nf, bm->thisyear);
        }
        g_catch_n_effort_History[sp].recorded[bm->thisyear] = 1;
    }
    FishMIP_Adapter_RunAssessment(sp, nyears, g_catch_n_effort_History[sp].nfishery, g_catch_n_effort_History[sp].catchFlat, g_catch_n_effort_History[sp].effortFlat,
                                  OSP_PARAMS->assess_prior_mean_r, OSP_PARAMS->assess_prior_sd_logr, OSP_PARAMS->assess_prior_mean_z,
                                  OSP_PARAMS->assess_prior_sd_logz, OSP_PARAMS->assess_optimizer, /*verbose=*/0, &result);

    if (!result.ok) {
        return 0;
    }
    *outFMSY = result.FMSY;
    *outMSY = result.MSY;
    return 1;
}

/** The "feed back to TACs" half of option (a): writes the plug-in's MSY
 *  estimate into exactly the fields Atlantis's own TAC machinery reads
 *  (bm->TACamt[sp][nf][now_id], consulted by atharvest/atHarvestIndex.c and
 *  atimplementation/atImplementationAnnual.c; bm->SP_FISHERYprms[sp][nf]
 *  [TAC_id], the "origin" TAC atimplementation/atImplementationAnnual.c's
 *  TACorig scenario-painting path reads). Allocates the total MSY across
 *  fisheries in proportion to each fishery's existing TAC share, the same
 *  rule Atlantis's own Tier system uses to split a stock-wide TAC across
 *  fisheries (atmanage/atManageTier.c:522). */
static void FeedPIMAssessmentIntoAtlantisTAC(MSEBoxModel *bm, FILE *llogfp, int sp, double MSY) {
    double TAC_old_total = 0.0;
    double share, TACnf;
    int nf;
    for (nf = 0; nf < bm->K_num_fisheries; ++nf) {
        TAC_old_total += bm->TACamt[sp][nf][now_id];
    }
    for (nf = 0; nf < bm->K_num_fisheries; ++nf) {
        share = (TAC_old_total > 0.0) ? (bm->TACamt[sp][nf][now_id] / TAC_old_total) : (1.0 / (double)(bm->K_num_fisheries));
        TACnf = MSY * share;
        bm->TACamt[sp][nf][old_id] = bm->TACamt[sp][nf][now_id];
        bm->TACamt[sp][nf][now_id] = TACnf;
        bm->SP_FISHERYprms[sp][nf][TAC_id] = TACnf;
    }
    /*
    if (verbose) {
        fprintf(llogfp, "FishMIP_OSP_Bridge: PIM assessment set %s MSY=%e, TAC written to %d fisheries\n", FunctGroupArray[sp].groupCode, MSY, bm->K_num_fisheries);
    }
    */
}

/** "(b) for Atlantis stock assessment to feed the PIM effort calculations":
 *  reads Atlantis's OWN existing stock-assessment output --
 *  bm->RBCestimation.RBCspeciesParam[sp][RBCest_id] (the Tier system's own
 *  Recommended Biological Catch, tonnes) and [sp][EstBcurr_id] (its
 *  estimated current biomass, tonnes) -- and derives an implied
 *  exploitation-rate target, RBCest / EstBcurr, to stand in for the
 *  fishing-mortality target plugin_model_2_calc_effort expects. Returns 0.0
 *  (caller falls back to the placeholder) if Atlantis has not populated a
 *  usable assessment for this species. */
static double FtargetFromAtlantisAssessment(MSEBoxModel *bm, int sp) {
    double RBCest = bm->RBCestimation.RBCspeciesParam[sp][RBCest_id];
    double Bcurr = bm->RBCestimation.RBCspeciesParam[sp][EstBcurr_id];
    if (!(Bcurr > 0.0) || !(RBCest > 0.0)) {
        return 0.0;
    }
    
    return RBCest / Bcurr;
}


/* ==========================================================================
 * Main entry point.
 * ========================================================================== */
void FishMIP_OSP_Bridge_Annual(MSEBoxModel *bm, FILE *llogfp, int mode, int ftargetSource, int priceSource) {
    int nsp, k, nf, this_nf, sp, nmarket, gp, calendarYear, overridden, nfishery;
    double weightedOrigSum = 0.0, weightSum = 0.0, origprice, ratio, c, M_placeholder;
    double FMSY, MSY, ft, totalCatch = 0.0, revenue = 0.0, ftargetWeighted = 0.0;
    
    if (mode == OSP_MODE_ATLANTIS_NATIVE) {
        return; /* Nothing to do. Atlantis's own routines already ran every aspect */
    }

    if (OSP_PARAMS->fished_nsp == 0) {
        return;
    }
    
    /* Overlays real driver-file values onto the placeholders above, if
     * FishMIP_OSP_Bridge_LoadDrivers() was called successfully at model
     * start; a no-op otherwise. calendarYear maps Atlantis's own model-year
     * counter onto the driver files' real calendar-year time axis. */
    ApplyLoadedDrivers(bm, FishMIP_Adapter_GetDriverBaseCalendarYear() + bm->thisyear);

    /* ---- Step 1: biomass, straight from Atlantis --------------------------
     * (TOY_calculate_biomass_dynamics line 166: biomass_stock[:, t] =
     * b_prev_stock -- there, the toy model owns and integrates its own
     * logistic biomass ODE every substep; here, Atlantis has already
     * integrated real numbers-at-age biomass dynamics for the whole year via
     * its own ecology module. Kept here, unused downstream, purely to mirror
     * the toy model's data flow one-for-one -- see FishMIP_OSP_Bridge_NOTES.md
     * section 1's table for the bm->totfishpop[sp] * bm->X_CN * mg_2_tonne
     * conversion this would use.) */

    /* ---- Step 2: this year's realised catch, straight from Atlantis -------
     * (TOY lines 272-278: catch_step_EEZ/catch_step_DWFN -- there, computed
     * BY the toy model as F * biomass * dt; here, Atlantis's harvest module
     * already computed and recorded the actual catch taken this year, so
     * again we read rather than recompute, in every mode.) */
    for (k = 0; k < OSP_PARAMS->fished_nsp; ++k) {
        sp = OSP_PARAMS->fishedSp[k];
        nfishery = g_catch_n_effort_History[sp].nfishery;
        for (nf = 0; nf < g_catch_n_effort_History[sp].nfishery; ++nf) {
            this_nf = g_catch_n_effort_History[sp].targetFishery[nf];
            OSP_PARAMS->catchBySpFishery[k * nfishery + nf] = Harvest_Get_TotCumCatch(OSP_PARAMS->fishedSp[k], this_nf, bm->thisyear);
        }
    }

    /* ---- Step 3 (modes A & B & C): price via FishMIP_Adapter_CalcPrice ----
     * (TOY lines 190-201: PIM_calculate_prices_consumptions, itself built on
     * plugin_model_1_calc_price -- called here directly rather than via the
     * country/EEZ-level wrapper, since Atlantis has no EEZ/country axis;
     * "market" here is one Atlantis species. Each fished species is given
     * its own local market: the simplest valid instantiation of
     * plugin_model_1_calc_price's four-level market hierarchy. See notes
     * doc "Making markets cross-species" for how to widen this to e.g. one
     * shared national or global market across several species.) */
    if (mode == OSP_MODE_A_EFFORT_ECONOMICS || mode == OSP_MODE_B_MARKET_VALUE ||
        mode == OSP_MODE_C_NATIVE_WITH_DRIVERS) {
        
        Util_Init_1D_Double(OSP_PARAMS->prod, bm->K_num_tot_sp, 0.0);
        Util_Init_1D_Double(OSP_PARAMS->X, bm->K_num_tot_sp, 0.0);
        Util_Init_1D_Double(OSP_PARAMS->W, bm->K_num_tot_sp, 0.0);

        for (k = 0; k < OSP_PARAMS->fished_nsp; ++k) {
            totalCatch = 0.0;
            sp = OSP_PARAMS->fishedSp[k];
            for (nf = 0; nf < g_catch_n_effort_History[sp].nfishery; ++nf) {
                totalCatch += OSP_PARAMS->catchBySpFishery[k * nfishery + nf];
            }
            OSP_PARAMS->prod[k] = totalCatch;
            /* Demand curve (TOY driv_X_fresh/driv_W_fresh): if
             * FishMIP_OSP_Bridge_LoadDrivers() was called successfully, uses
             * the SAME real driver_X_fresh/driver_W_fresh values the toy
             * model itself would read for the loaded reference
             * country/commodity/year (applied uniformly across every fished
             * species -- see ApplyLoadedDrivers()'s doc comment for why).
             * Otherwise falls back to the original placeholder: intercept
             * 50% above current supply, unit slope -- guarantees a
             * well-defined, positive equilibrium price with no real driver
             * data at all. See notes doc section 8. */
            if (OSP_PARAMS->demand_loaded) {
                OSP_PARAMS->X[k] = OSP_PARAMS->demand_X_uniform;
                OSP_PARAMS->W[k] = OSP_PARAMS->demand_W_uniform;
            } else {
                OSP_PARAMS->X[k] = 1.5 * totalCatch + 1.0;
                OSP_PARAMS->W[k] = 1.0;
            }
        }
        FishMIP_Adapter_CalcPrice(OSP_PARAMS->fished_nsp, OSP_PARAMS->prod, OSP_PARAMS->X, OSP_PARAMS->W, OSP_PARAMS->epsilon, OSP_PARAMS->price_by_sp);

        /* External global-market price override (OSP_PRICE_EXTERNAL_GLOBAL):
         * treats "the price a full multi-country PIM run would have
         * produced" as an external input this bridge cannot derive itself
         * (this Atlantis domain is one region, not the whole globe). Any
         * species without a usable value in the loaded series keeps the
         * FishMIP_Adapter_CalcPrice() value just computed above, so a
         * partially-populated external series is always safe to use. */
        if (priceSource == OSP_PRICE_EXTERNAL_GLOBAL) {
            calendarYear = FishMIP_Adapter_GetPriceBaseCalendarYear() + bm->thisyear;
            overridden = 0;
            for (k = 0; k < OSP_PARAMS->fished_nsp; ++k) {
                gp = FishMIP_Adapter_GetGlobalPriceForYear(FunctGroupArray[OSP_PARAMS->fishedSp[k]].groupCode, calendarYear);
                if (gp >= 0.0) {
                    OSP_PARAMS->price_by_sp[k] = gp;
                    ++overridden;
                }
            }
            /*
            if (verbose && overridden > 0) {
                fprintf(llogfp, "FishMIP_OSP_Bridge: external global price overrode %d of %d species\n", overridden, OSP_PARAMS->fished_nsp);
             }
             */
        }
    }

    if (mode == OSP_MODE_A_EFFORT_ECONOMICS || mode == OSP_MODE_B_MARKET_VALUE) {
        /* Write the plug-in price into exactly the field Get_Fish_Prices()
         * would otherwise have set (ateconomic/atquota.c:91,124-149) --
         * insert this call in place of (mode A) or immediately after (mode
         * B) Get_Fish_Prices(bm, llogfp) inside Economic_Annual(), see notes
         * doc "Call-site wiring". */
        for (k = 0; k < OSP_PARAMS->fished_nsp; ++k) {
            sp = OSP_PARAMS->fishedSp[k];
            for (nf = 0; nf < bm->K_num_fisheries; ++nf) {
                if (!bm->SP_FISHERYprms[sp][nf][q_id]) {
                    continue; /* fishery nf cannot catch sp, same guard as Get_Fish_Prices() */
                }
                bm->SP_FISHERYprms[sp][nf][saleprice_id] = OSP_PARAMS->price_by_sp[k];
            }
        }
        /*
        if (verbose) {
            fprintf(llogfp, "FishMIP_OSP_Bridge: mode %d wrote plug-in market price for %d species\n", mode, OSP_PARAMS->fished_nsp);
        }
         */
    }

    /* ---- Step 4 (mode A only): effort & fishing mortality -----------------
     * (TOY lines 209 & 219: plugin_model_2_calc_effort, called once for the
     * EEZ-based fleet and once for the DWFN fleet. Atlantis has no separate
     * EEZ/DWFN split -- an Atlantis fishery already IS one discrete fleet --
     * so this calls the plug-in effort model once per year, across all
     * fisheries at once, with nfleet_type collapsed to 1.
     *
     * Fleet-dimension mapping: Atlantis effort (bm->Effort[box][fishery]) is
     * shared across every species a fishery lands, unlike the toy model's
     * single-commodity-per-run stocks. This aggregates every fished species
     * landed by fishery nf into one revenue-weighted price and one
     * total-catch figure before calling the effort model -- i.e. it mirrors
     * what Atlantis's own Marginal_Rent_Calc() (ateconomic/ateconresponse.c)
     * already does with multi-species profit, just via the plug-in model's
     * effort-dynamics equation instead of Atlantis's own.) */
    if (mode == OSP_MODE_A_EFFORT_ECONOMICS) {
        /* Reinitialise the vectors that are reused each loop */
        Util_Init_1D_Double(OSP_PARAMS->ftargetBySp, bm->K_num_tot_sp, 0.0);
        Util_Init_1D_Double(OSP_PARAMS->price_arr, bm->K_num_fisheries, 0.0);
        Util_Init_1D_Double(OSP_PARAMS->Y_arr, bm->K_num_fisheries, 0.0);
        Util_Init_1D_Double(OSP_PARAMS->effort_arr, bm->K_num_fisheries, 0.0);
        Util_Init_1D_Double(OSP_PARAMS->q_arr, bm->K_num_fisheries, 0.0);
        Util_Init_1D_Double(OSP_PARAMS->comply_arr, bm->K_num_fisheries, 0.0);
        Util_Init_1D_Double(OSP_PARAMS->dU_arr, bm->K_num_fisheries, 0.0);
        Util_Init_1D_Double(OSP_PARAMS->dV_arr, bm->K_num_fisheries, 0.0);
        Util_Init_1D_Double(OSP_PARAMS->Ftarget_arr, bm->K_num_fisheries, 0.0);
        Util_Init_1D_Double(OSP_PARAMS->f_out, bm->K_num_fisheries, 0.0);
        
        /* ---- Step 3b: per-species fishing-mortality target, sourced per
         * ftargetSource (see atFishMIP_OSP_Bridge.h's doc comment). This is
         * the toy model's target_EEZ/target_DWFN_fleet (TOY line 152-154,
         * refreshed yearly via PIM_allocate_target_local_DWFN) -- computed
         * per species here rather than per EEZ/ABNJ stock, then
         * catch-weighted down to one per-fishery number below, same as
         * price. */
        for (k = 0; k < OSP_PARAMS->fished_nsp; ++k) {
            OSP_PARAMS->ftargetBySp[k] = OSP_PARAMS->Ftarget_placeholder;
        }
        if (ftargetSource == OSP_FTARGET_PIM_ASSESSMENT) {
            for (k = 0; k < OSP_PARAMS->fished_nsp; ++k) {
                sp = OSP_PARAMS->fishedSp[k];
                if (RunPIMAssessment(bm, sp, &FMSY, &MSY)) {
                    OSP_PARAMS->ftargetBySp[k] = FMSY;
                    FeedPIMAssessmentIntoAtlantisTAC(bm, llogfp, sp, MSY);
                }
                /* else: not enough history yet / fit didn't converge -- keep the placeholder for this species this year. */
            }
        } else if (ftargetSource == OSP_FTARGET_ATLANTIS_ASSESSMENT) {
            for (k = 0; k < OSP_PARAMS->fished_nsp; ++k) {
                ft = FtargetFromAtlantisAssessment(bm, OSP_PARAMS->fishedSp[k]);
                if (ft > 0.0) {
                    OSP_PARAMS->ftargetBySp[k] = ft;
                }
                /* else: Atlantis has no assessment output for this species yet -- keep the placeholder. */
            }
        }

        for (nf = 0; nf < bm->K_num_fisheries; ++nf) {
            this_nf = g_catch_n_effort_History[sp].targetFishery[nf];
            for (k = 0; k < OSP_PARAMS->fished_nsp; ++k) {
                nfishery = g_catch_n_effort_History[sp].nfishery;
                c = OSP_PARAMS->catchBySpFishery[k * nfishery + nf];
                if (c <= 0.0) {
                    continue;
                }
                totalCatch += c;
                revenue += c * OSP_PARAMS->price_by_sp[k];
                ftargetWeighted += c * OSP_PARAMS->ftargetBySp[k];
            }
            OSP_PARAMS->Y_arr[nf] = totalCatch / OSP_PARAMS->dt;
            OSP_PARAMS->price_arr[nf] = (totalCatch > 0.0) ? revenue / totalCatch : 0.0;
            OSP_PARAMS->effort_arr[nf] = AtlantisFisheryEffortTotal(bm, this_nf);
            OSP_PARAMS->q_arr[nf] = OSP_PARAMS->catchability[this_nf];
            OSP_PARAMS->comply_arr[nf] = OSP_PARAMS->compliance[this_nf];
            OSP_PARAMS->dU_arr[nf] = OSP_PARAMS->invest_driver_U[this_nf];
            OSP_PARAMS->dV_arr[nf] = OSP_PARAMS->invest_driver_V[this_nf];
            
            /* Catch-weighted mean target F across every species fishery nf lands, matching how price is aggregated above. */
            OSP_PARAMS->Ftarget_arr[nf] = (totalCatch > 0.0) ? ftargetWeighted / totalCatch : OSP_PARAMS->Ftarget_placeholder;
        }

        M_placeholder = OSP_PARAMS->vessel_attrition_M[0];  // TODO: Fix versus actually what is meant
        FishMIP_Adapter_CalcEffort(bm->K_num_fisheries, OSP_PARAMS->price_arr, OSP_PARAMS->Y_arr, OSP_PARAMS->effort_arr, OSP_PARAMS->q_arr, OSP_PARAMS->Ftarget_arr, OSP_PARAMS->comply_arr, OSP_PARAMS->risk_invest, OSP_PARAMS->dU_arr, OSP_PARAMS->dV_arr, M_placeholder, OSP_PARAMS->dt, OSP_PARAMS->effort_growth_lim, OSP_PARAMS->f_growth_rate_max, OSP_PARAMS->f_growth_rate_half_sat, OSP_PARAMS->f_out);
        
        for (nf = 0; nf < bm->K_num_fisheries; ++nf) {
            SetAtlantisFisheryEffortTotal(bm, nf, OSP_PARAMS->f_out[nf]);
        }

        /*
        if (verbose) {
            fprintf(llogfp, "FishMIP_OSP_Bridge: mode A wrote plug-in effort for %d fisheries\n", nfishery);
        }
         */
    }

    /* ---- Mode C: leave every native Atlantis output alone; inject a ------
     * plug-in-derived *driver* instead of a plug-in-derived *result*.
     * Get_Fish_Prices() (ateconomic/atquota.c:124-131) computes:
     *   saleprice = market_wgt * (MARKETprms[market][sp][market_intercept_id]
     *               * SP_FISHERYprms[sp][nf][origprice_id]
     *               + trend_coefft*thisyear + month_coefft + auto_coefft*resid)
     * i.e. market_intercept_id is a *ratio* against a fixed reference price
     * (origprice_id), not a price itself. This translates the plug-in
     * model's equilibrium price into that ratio (catch-weighted across
     * fisheries landing sp, since MARKETprms has no fishery axis but
     * origprice_id does), so Atlantis's own trend/seasonal/AR terms still
     * apply on top of a plug-in-informed intercept, rather than the bridge
     * setting the final price itself. */
    if (mode == OSP_MODE_C_NATIVE_WITH_DRIVERS) {
        for (k = 0; k < OSP_PARAMS->fished_nsp; ++k) {
            weightedOrigSum = 0.0;
            weightSum = 0.0;
            sp = OSP_PARAMS->fishedSp[k];
            if (OSP_PARAMS->price_by_sp[k] <= 0.0) {
                continue;
            }
            for (nf = 0; nf < g_catch_n_effort_History[sp].nfishery; ++nf) {
                this_nf = g_catch_n_effort_History[sp].targetFishery[nf];
                c = OSP_PARAMS->catchBySpFishery[k * nfishery + nf];
                if (c <= 0.0) {
                    continue;
                }
                weightedOrigSum += c * bm->SP_FISHERYprms[sp][this_nf][origprice_id];
                weightSum += c;
            }
            if (weightSum <= 0.0) {
                continue;
            }
            origprice = weightedOrigSum / weightSum;
            if (origprice <= 0.0) {
                continue;
            }
            ratio = OSP_PARAMS->price_by_sp[k] / origprice;
            for (nmarket = 0; nmarket < bm->K_num_markets; ++nmarket)
                bm->MARKETprms[nmarket][sp][market_intercept_id] = ratio;
        }
        /*
        if (verbose) {
            fprintf(llogfp, "FishMIP_OSP_Bridge: mode C injected plug-in market_intercept_id drivers for %d species\n", nsp);
        }
        */
    }
    
    return;

}
