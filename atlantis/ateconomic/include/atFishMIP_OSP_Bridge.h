#ifndef AT_FISHMIP_OSP_BRIDGE_H
#define AT_FISHMIP_OSP_BRIDGE_H

/**
 * \file atFishMIP_OSP_Bridge.h
 * \ingroup ateconomic
 *
 * Public entry points for the FishMIP-OSP <-> Atlantis bridge
 * (atFishMIP_OSP_Bridge.c). This is a plain Atlantis C header -- like every
 * other header in ateconomic/include/, it assumes atlantisboxmodel.h (for
 * MSEBoxModel and the OSP_MODE_*, OSP_FTARGET_* and OSP_PRICE_* constants it
 * defines) and <stdio.h> (for FILE) are already visible at the point of
 * inclusion (see atEconomicLib.h for the same convention).
 *
 * atFishMIP_OSP_Bridge.c does all of the Atlantis-side work (reading
 * bm->totfishpop/bm->Effort/Harvest_Get_TotCumCatch(), writing
 * bm->SP_FISHERYprms/bm->TACamt/bm->MARKETprms, ...); it calls into the
 * FishMIP-OSP plug-in models only through atFishMIP_OSP_PluginAdapter.h's
 * plain-C interface, never touching the plug-in library's C++ types
 * directly. See ateconomic/FishMIP_OSP_Bridge_NOTES.md for what this bridge
 * does, the three modelling options (mode) and the two orthogonal switches
 * (ftargetSource, priceSource), and for the call-site wiring (into
 * Economic_Annual()/Economic_Init()) that has intentionally not been applied
 * yet -- this header/file pair is not called from anywhere else in Atlantis.
 */

#ifdef __cplusplus
extern "C" {
#endif

void Create_OSP_Arrays(MSEBoxModel *bm, FILE *llogfp);

/**
 * Call once per year, at the same point Economic_Annual() would call it --
 * see FishMIP_OSP_Bridge_NOTES.md section 1's call-site diagram for the
 * exact insertion point per `mode` (not applied here).
 *
 *   mode           OSP_MODE_ATLANTIS_NATIVE / OSP_MODE_A_EFFORT_ECONOMICS /
 *                  OSP_MODE_B_MARKET_VALUE / OSP_MODE_C_NATIVE_WITH_DRIVERS
 *                  (atlantisboxmodel.h).
 *   ftargetSource  OSP_FTARGET_PLACEHOLDER / OSP_FTARGET_PIM_ASSESSMENT /
 *                  OSP_FTARGET_ATLANTIS_ASSESSMENT (atlantisboxmodel.h) --
 *                  only has an effect when mode == OSP_MODE_A_EFFORT_ECONOMICS.
 *   priceSource    OSP_PRICE_LOCAL_PLUGIN / OSP_PRICE_EXTERNAL_GLOBAL
 *                  (atlantisboxmodel.h).
 */
void FishMIP_OSP_Bridge_Annual(MSEBoxModel *bm, FILE *llogfp, int mode, int ftargetSource, int priceSource);

/**
 * One-time (not annual) driver load: reads the SAME real driver files the
 * toy model's PIM_load_drivers()/PIM_get_control_parameters() read, and
 * caches them for FishMIP_OSP_Bridge_Annual() to draw on every year
 * afterwards. See FishMIP_OSP_Bridge_NOTES.md section 8.1 for the parameters
 * and exactly which placeholder each loaded driver replaces.
 *
 * Returns 1 on success, 0 on failure (e.g. bad path, or
 * referenceCountryISO3 not found) -- check the return value;
 * FishMIP_OSP_Bridge_Annual() silently falls back to its placeholders if
 * this was never called or did not succeed.
 */
int FishMIP_OSP_Bridge_LoadDrivers(const char *dataDir, const char *controlParamsCsvPath,
                                    const char *referenceCountryISO3, int commod, int nosp, int baseCalendarYear);

/**
 * One-time (not annual) load of an externally-supplied global-market price
 * time series, for OSP_PRICE_EXTERNAL_GLOBAL. See FishMIP_OSP_Bridge_NOTES.md
 * section 8.2 for the CSV format. Returns 1 on success, 0 on failure.
 */
int FishMIP_OSP_Bridge_LoadGlobalPrice(const char *csvPath, const char *yearColumn, int baseCalendarYear);


/* ==========================================================================
 * Parameters the plug-in calls below need that Atlantis does not (yet)
 * expose natively per species/fishery: catchability, target fishing
 * mortality, compliance, investment drivers, demand-curve intercept/slope.
 * See FishMIP_OSP_Bridge_NOTES.md, "Parameters with no Atlantis-native
 * source", for what a production wiring would need to add. Everything below
 * is a clearly-labelled placeholder so the numeric example is well-defined,
 * not a claim that these values are meaningful for any particular Atlantis
 * parameterisation.
 * ========================================================================== */
typedef struct {
    
    double *catchability;      /* [nfishery]; q in plugin_model_2_calc_effort */
    double *compliance;        /* [nfishery], in [0,1] */
    double *invest_driver_U;   /* [nfishery]; revenue-side investment driver (TOY driv_U_national) */
    double *invest_driver_V;   /* [nfishery]; cost-side divestment driver (TOY driv_V_national) */
    int *fisheryFleetType;     /* [nfishery]; 0 == artisanal, 1 == industrial (toy model's own nfleet
                                 * convention). Atlantis fisheries have no native artisanal/industrial
                                 * label, so this defaults every fishery to "artisanal" (0). */
    double *vessel_attrition_M; /* rather than using single slot -- see "Fleet-dimension mapping" */
    
    // Per species arrays
    double *ftargetBySp;
    double *prod;
    double *X;
    double *W;
    int *fishedSp;   /* compact list of species indices to iterate over (the "nstocks" dimension for the plug-in model), instead of looping over all K_num_tot_sp groups and re-checking isFished every time the OSP is called */
    double *price_by_sp;
    double *catchBySpFishery;


    // Per fishery arrays
    double *price_arr;
    double *Y_arr;
    double *effort_arr;
    double *q_arr;
    double *comply_arr;
    double *dU_arr;
    double *dV_arr;
    double *Ftarget_arr;
    double *f_out;
    double *thisYearEffort;
    
    int fished_nsp;
    
    double risk_invest;        /* mirrors PIM_load_drivers' risk_invest default */
    int effort_growth_lim;
    double f_growth_rate_max;      /* == 25^(1/10), see the toy model's control_parameters.csv */
    double f_growth_rate_half_sat;
    double dt;                     /* this bridge runs once/year, like Annual_Effort_Schedule() */
    double epsilon;                /* demand elasticity for FishMIP_Adapter_CalcPrice (price falls as supply rises) */
    double Ftarget_placeholder;    /* used when ftargetSource == OSP_FTARGET_PLACEHOLDER */

    /* ---- fit_one_stock() priors, used only when ftargetSource ==
     * OSP_FTARGET_PIM_ASSESSMENT. Atlantis has no single field these map
     * onto directly; a generic Schaefer-like default is used here rather
     * than a per-species biological judgement call. */
    double assess_prior_mean_r;
    double assess_prior_sd_logr;
    double assess_prior_mean_z;
    double assess_prior_sd_logz;
    int assess_optimizer;   /* "mini" selector; 4 == PIM_assess_stocks's own default (parallel) branch */
    int assess_min_years;   /* minimum catch/effort history length before attempting a fit */

    /* ---- populated by ApplyLoadedDrivers() when a driver set has been
     * loaded (FishMIP_OSP_Bridge_LoadDrivers()); left at defaults
     * (demand_loaded == 0) otherwise, in which case Step 3 falls back to
     * its own catch-scaled placeholder demand curve. */
    int demand_loaded;
    double demand_X_uniform;
    double demand_W_uniform;
    

}OSPStruct;
extern OSPStruct *OSP_PARAMS;

/** Atlantis keeps no per-fishery, multi-year effort history in the shape
 *  fit_one_stock() needs (n years x nfleets): bm->EffortRecord is indexed
 *  [year][species][dataid] (see atassess/atclassical.c), with no fishery
 *  axis at all -- it was built for Atlantis's OWN classical/Tier assessment
 *  input conventions, not this bridge's per-fishery breakdown. So this
 *  bridge keeps its own rolling per-species record instead, appended to
 *  once per year the bridge runs. (Catch needs no equivalent buffer --
 *  Harvest_Get_TotCumCatch(sp, nf, iy) already gives any past year directly
 *  from Atlantis's own storage.) */
typedef struct {
    int firstcall;  /* whether time series has previously been pre-filled for historical years when assessment begins mid run */
    int nfishery;
    int *targetFishery;     /* Mapping of Atlantis fleet to PIM fleet and back */
    int *recorded;          /* 1 once a year's row has been written (by the current
                            * year or by the mid-simulation-start back-fill)
                            * and should never be touched again */
    double *effortFlat;   /* per fishery */
    double *catchFlat;    /* per fishery */
    
} EffortHistoryRecord;
extern EffortHistoryRecord *g_catch_n_effort_History;  /* per fished species */

extern int g_effortHistoryCount; /* number of species slots allocated in g_effortHistory */


#ifdef __cplusplus
}
#endif

#endif /* AT_FISHMIP_OSP_BRIDGE_H */
