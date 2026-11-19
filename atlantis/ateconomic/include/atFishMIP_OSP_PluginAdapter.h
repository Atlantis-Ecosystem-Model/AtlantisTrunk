#ifndef AT_FISHMIP_OSP_PLUGIN_ADAPTER_H
#define AT_FISHMIP_OSP_PLUGIN_ADAPTER_H

/**
 * \file atFishMIP_OSP_PluginAdapter.h
 * \ingroup ateconomic
 *
 * Plain-C interface onto the FishMIP-OSP plug-in economic models
 * (FishMIP_OSP_Plugin_Models.hpp/.cpp, namespace fishmip). This header and
 * its implementation (atFishMIP_OSP_PluginAdapter.cpp) are the ONLY place in
 * Atlantis that touches the plug-in library's C++ types (NdArray, StockData,
 * DriverData, std::vector, std::optional, ...) -- every function declared
 * below takes and returns plain C scalars/arrays/structs only, so it can be
 * called from an ordinary Atlantis .c file. atFishMIP_OSP_Bridge.c does all
 * the Atlantis-side orchestration (reading bm->..., writing bm->..., the
 * per-year control flow) and talks to the plug-in models only through this
 * interface -- it never sees an NdArray, a std::vector, or any other
 * fishmip:: type.
 *
 * Do not add fishmip:: types, STL containers, or other C++-only syntax to
 * this header -- that would defeat the point of the split. See
 * ateconomic/FishMIP_OSP_Bridge_NOTES.md for the single-file (.cpp) bridge
 * this was split out of, and the reasoning behind the split.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* ==========================================================================
 * One-time (not annual) driver / external-price loading.
 * ========================================================================== */

/**
 * Loads PIM_configure_simulation()/PIM_load_drivers()/
 * PIM_get_control_parameters() for one "reference country" standing in for
 * Atlantis's whole model domain, and caches them for
 * FishMIP_Adapter_GetDriverValuesForYear() to draw on every year afterwards.
 * Same parameters/semantics as the original FishMIP_OSP_Bridge_LoadDrivers()
 * (see FishMIP_OSP_Bridge_NOTES.md section 8.1):
 *
 *   dataDir              directory containing the driver .nc/.csv files.
 *   controlParamsCsvPath control_parameters.csv path.
 *   referenceCountryISO3 ISO3 code of the country whose driver time series
 *                        stands in for Atlantis's whole model domain.
 *   commod               0/1/2 (small pelagics / large pelagics / demersal
 *                        & benthic).
 *   nosp                 0 for historical drivers; >0 for an SSP/OSP
 *                        scenario extension.
 *   baseCalendarYear     the real-world calendar year that Atlantis model
 *                        year 0 corresponds to.
 *
 * Returns 1 on success, 0 on failure (bad path, or referenceCountryISO3 not
 * found in the loaded country list).
 */
int FishMIP_Adapter_LoadDrivers(const char *dataDir, const char *controlParamsCsvPath,
                                 const char *referenceCountryISO3, int commod, int nosp, int baseCalendarYear);

/** 1 if FishMIP_Adapter_LoadDrivers() has succeeded, 0 otherwise (including
 *  "never called"). */
int FishMIP_Adapter_DriversLoaded(void);

/** The baseCalendarYear passed to the most recent successful
 *  FishMIP_Adapter_LoadDrivers() call, or 0 if none has succeeded. Callers
 *  add bm->thisyear to this to get the real calendar year to pass to
 *  FishMIP_Adapter_GetDriverValuesForYear(). */
int FishMIP_Adapter_GetDriverBaseCalendarYear(void);

/**
 * One year's worth of resolved driver values for the loaded reference
 * country / fleet type (0 == artisanal, 1 == industrial -- the plug-in
 * model's own nfleet convention). `ok` is 0 if no driver set has been
 * loaded; a loaded-but-landlocked reference country still returns
 * demand-curve values (fields not documented "EEZ-indexed" below) with the
 * EEZ-indexed fields left at 0 -- see FishMIP_OSP_Bridge_NOTES.md section
 * 8.1 for why.
 */
typedef struct {
    int ok;
    double demand_X_fresh, demand_W_fresh;   /* driver_X_fresh/driver_W_fresh */
    double demand_X_caned, demand_W_caned;   /* driver_X_caned/driver_W_caned */
    double exported_frac;                    /* EEZ-indexed */
    double catchability_EEZ;                 /* EEZ-indexed, fleet-type selected */
    double compliance;                       /* EEZ-indexed, fleet-type selected */
    double invest_driver_U;                  /* EEZ-indexed, fleet-type selected */
    double invest_driver_V;                  /* EEZ-indexed, fleet-type selected */
    double epsilon;
    double risk_invest;
    double vessel_attrition_M;               /* fleet-type selected */
    double f_growth_rate_max;
    double f_growth_rate_half_sat;
    int effort_growth_lim;
} FishMIP_DriverYearValues;

void FishMIP_Adapter_GetDriverValuesForYear(int calendarYear, int fleetType, FishMIP_DriverYearValues *out);

/**
 * Loads an externally-supplied global-market price time series (see the
 * original FishMIP_OSP_Bridge_LoadGlobalPrice() doc comment /
 * FishMIP_OSP_Bridge_NOTES.md section 8.2 for the CSV format). Returns 1 on
 * success, 0 on failure (bad path, or no usable price column found).
 */
int FishMIP_Adapter_LoadGlobalPrice(const char *csvPath, const char *yearColumn, int baseCalendarYear);

/** The baseCalendarYear passed to the most recent successful
 *  FishMIP_Adapter_LoadGlobalPrice() call, or 0 if none has succeeded. */
int FishMIP_Adapter_GetPriceBaseCalendarYear(void);

/** Returns the loaded external price for this species/year, or a negative
 *  value if no usable series/column is available (caller should fall back
 *  to FishMIP_Adapter_CalcPrice()'s locally-computed price). */
double FishMIP_Adapter_GetGlobalPriceForYear(const char *speciesGroupCode, int calendarYear);

/* ==========================================================================
 * Per-year plug-in model calls.
 * ========================================================================== */

/**
 * Wraps plugin_model_1_calc_price(), instantiated the way the bridge uses
 * it: each of the nsp species is given its OWN local market (geo_agreg==0,
 * region_idx[i]==subregion_idx[i]==i, nregion==nsubregion==nctry==nsp; aqua
 * all zero -- see FishMIP_OSP_Bridge_NOTES.md section 6, "Making markets
 * cross-species", for how a fuller wiring could widen this).
 *
 * prod/X/W are length nsp; price_out (caller-allocated, length nsp)
 * receives the resulting per-species price.
 */
void FishMIP_Adapter_CalcPrice(int nsp, const double *prod, const double *X, const double *W, double epsilon,
                                double *price_out);

/**
 * Wraps plugin_model_2_calc_effort(), instantiated the way the bridge uses
 * it: nfleet_type collapsed to 1 (an Atlantis fishery already IS one
 * discrete fleet -- see FishMIP_OSP_Bridge_NOTES.md section 1).
 *
 * price/Y/effo/q/Ftarget/comply/d_U/d_V are length nfishery; M is the single
 * vessel-attrition scalar (all Atlantis fisheries share one M slot -- see
 * the notes doc's "Fleet-dimension mapping"). f_out (caller-allocated,
 * length nfishery) receives the updated per-fishery effort.
 */
void FishMIP_Adapter_CalcEffort(int nfishery, const double *price, const double *Y, const double *effo,
                                 const double *q, const double *Ftarget, const double *comply, double risk,
                                 const double *d_U, const double *d_V, double M, double dt, int effort_growth_lim,
                                 double f_growth_rate_max, double f_growth_rate_half_sat, double *f_out);

/** Result of FishMIP_Adapter_RunAssessment(): `ok` is 0 if the fit did not
 *  converge or produced a non-positive FMSY/MSY. */
typedef struct {
    int ok;
    double FMSY;
    double MSY;
} FishMIP_AssessmentResult;

/**
 * Wraps fit_one_stock() -- the Pella & Tomlinson stock-assessment fit, the
 * same function PIM_assess_stocks() calls internally per stock (see
 * FishMIP_OSP_Bridge_NOTES.md sections 2 and 7(i) for why fit_one_stock() is
 * called directly here instead of going through PIM_assess_stocks()).
 *
 * catchFlat/effortFlat are row-major [nyears][nfishery] (i.e.
 * catchFlat[iy*nfishery+nf]), built by the caller from
 * Harvest_Get_TotCumCatch() and its own effort-history record respectively
 * (Atlantis keeps no native per-fishery multi-year effort record -- see the
 * notes doc section 7(i)).
 *
 * speciesId selects this species' warm-start cache slot (mirrors the
 * original bridge's per-species x0_prev threading) -- pass the same
 * Atlantis species index (sp) every year for a given species. optimizer is
 * fit_one_stock's `mini` selector (0-5; see FishMIP_OSP_Plugin_Models.hpp's
 * doc comment on fit_one_stock for what each value means).
 */
void FishMIP_Adapter_RunAssessment(int speciesId, int nyears, int nfishery, const double *catchFlat,
                                    const double *effortFlat, double prior_mean_r, double prior_sd_logr,
                                    double prior_mean_z, double prior_sd_logz, int optimizer, int verbose,
                                    FishMIP_AssessmentResult *out);

#ifdef __cplusplus
}
#endif

#endif /* AT_FISHMIP_OSP_PLUGIN_ADAPTER_H */
