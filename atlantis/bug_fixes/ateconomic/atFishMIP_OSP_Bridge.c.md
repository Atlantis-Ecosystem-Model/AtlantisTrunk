# ateconomic/atFishMIP_OSP_Bridge.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **10 findings** (severity H: 6; confidence H and severity H: 5; on the priority list: 5), plus 2 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [ECIO-13](#ecio-13) ⭐ | 513-520 (and 542, 673, 730) | H/H | index packing / overlap | `catchBySpFishery[k * nfishery + nf]` uses a different `nfishery` for each species. Rows for different k overl… |  | [ ] |
| [ECIO-14](#ecio-14) ⭐ | 538-544 | H/H | stale variable | In Step 3, `totalCatch += catchBySpFishery[k * nfishery + nf]` uses `nfishery` left over from the last species… |  | [ ] |
| [ECIO-15](#ecio-15) ⭐ | 669-691 | H/H | stale variable / accumulator not reset / wrong index | The Step 4 per-fishery loop has three faults. (a) `sp` is stale (the last species from the earlier loops), so … |  | [ ] |
| [ECIO-16](#ecio-16) ⭐ | 728-736 | H/H | stale variable | In Mode C, `c = catchBySpFishery[k * nfishery + nf]` again uses the stale `nfishery` from Step 2 rather than s… |  | [ ] |
| [ECIO-17](#ecio-17) ⭐ | 480, 576-578 | H/H | type truncation | `gp` is declared `int` (l.480), but `FishMIP_Adapter_GetGlobalPriceForYear()` returns `double` (adapter .h:112… |  | [ ] |
| [ECIO-18](#ecio-18) | 134-142, 387 | H/M | uninitialised variable | `g_catch_n_effort_History` is `malloc`'d and `firstcall` is never set, so `if (g_catch_n_effort_History[sp].fi… |  | [ ] |
| [ECIO-19](#ecio-19) | 400, 410, 435-448, 518, 602 | M/H | unit mismatch | Catch comes from `Harvest_Get_TotCumCatch()` (TotCumCatch, in mg N: ateconindicator.c:454-456 compares it to `… |  | [ ] |
| [ECIO-20](#ecio-20) | 321-331 | M/M | defaults overwritten with 0 | `ApplyLoadedDrivers()` guards catchability with `> 0.0` but copies `compliance`, `invest_driver_U` and `invest… |  | [ ] |
| [ECIO-21](#ecio-21) | 370-416, 651-656 | M/L | empty-dimension call | `RunPIMAssessment()` runs for every fished species even when `nfishery == 0` (no fishery targets it per `Colle… |  | [ ] |
| [ECIO-22](#ecio-22) | 148-187 | L/L | memory leak / dead code | `FreeOSPStruct()` is static and never called, and no public free exists, so all OSP arrays leak; it will also … |  | [ ] |

## Reviewer notes for this file

(This file IS listed in libateconomic_a_SOURCES (ateconomic/Makefile.am:23), even though its header comment says it is not. It compiles but is not called from anywhere.)

---

## ECIO-13

**index packing / overlap** · Conf H / Sev H ⭐ priority  
**Listed lines:** 513-520 (and 542, 673, 730)

**Function:** `FishMIP_OSP_Bridge_Annual()`

**Why it's thought to be a bug**

`catchBySpFishery[k * nfishery + nf]` uses a different `nfishery` for each species. Rows for different k overlap: for example, species k=0 with nfishery=3 uses slots 0-2, and species k=2 with nfishery=1 writes slot 2, overwriting k=0's third fishery. The buffer is allocated `K_num_fisheries*K_num_tot_sp` (l.90). Function: `FishMIP_OSP_Bridge_Annual()`.

**Current code** (`ateconomic/atFishMIP_OSP_Bridge.c` lines 511–522)

```c
  511:      * already computed and recorded the actual catch taken this year, so
  512:      * again we read rather than recompute, in every mode.) */
  513:     for (k = 0; k < OSP_PARAMS->fished_nsp; ++k) {
  514:         sp = OSP_PARAMS->fishedSp[k];
  515:         nfishery = g_catch_n_effort_History[sp].nfishery;
  516:         for (nf = 0; nf < g_catch_n_effort_History[sp].nfishery; ++nf) {
  517:             this_nf = g_catch_n_effort_History[sp].targetFishery[nf];
  518:             OSP_PARAMS->catchBySpFishery[k * nfishery + nf] = Harvest_Get_TotCumCatch(OSP_PARAMS->fishedSp[k], this_nf, bm->thisyear);
  519:         }
  520:     }
  521: 
  522:     /* ---- Step 3 (modes A & B & C): price via FishMIP_Adapter_CalcPrice ----
```

**Suggested fix (from review):** Index as `k * bm->K_num_fisheries + nf` everywhere (or `+ this_nf`).

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## ECIO-14

**stale variable** · Conf H / Sev H ⭐ priority  
**Listed lines:** 538-544

**Function:** `FishMIP_OSP_Bridge_Annual()`

**Why it's thought to be a bug**

In Step 3, `totalCatch += catchBySpFishery[k * nfishery + nf]` uses `nfishery` left over from the last species of the Step 2 loop, not species k's own value, so `prod[k]` (and the price) is summed from the wrong slots. Function: `FishMIP_OSP_Bridge_Annual()`.

**Current code** (`ateconomic/atFishMIP_OSP_Bridge.c` lines 536–546)

```c
  536:         Util_Init_1D_Double(OSP_PARAMS->W, bm->K_num_tot_sp, 0.0);
  537: 
  538:         for (k = 0; k < OSP_PARAMS->fished_nsp; ++k) {
  539:             totalCatch = 0.0;
  540:             sp = OSP_PARAMS->fishedSp[k];
  541:             for (nf = 0; nf < g_catch_n_effort_History[sp].nfishery; ++nf) {
  542:                 totalCatch += OSP_PARAMS->catchBySpFishery[k * nfishery + nf];
  543:             }
  544:             OSP_PARAMS->prod[k] = totalCatch;
  545:             /* Demand curve (TOY driv_X_fresh/driv_W_fresh): if
  546:              * FishMIP_OSP_Bridge_LoadDrivers() was called successfully, uses
```

**Suggested fix (from review):** Set `nfishery = g_catch_n_effort_History[sp].nfishery;` inside the loop (plus the ECIO-13 fix).

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## ECIO-15

**stale variable / accumulator not reset / wrong index** · Conf H / Sev H ⭐ priority  
**Listed lines:** 669-691

**Function:** `FishMIP_OSP_Bridge_Annual()`

**Why it's thought to be a bug**

The Step 4 per-fishery loop has three faults. (a) `sp` is stale (the last species from the earlier loops), so `this_nf = g_catch_n_effort_History[sp].targetFishery[nf]` maps every fishery through one species' target list. For nf >= that species' nfishery the entries are 0, so q/compliance/effort all come from fishery 0. (b) `totalCatch`, `revenue` and `ftargetWeighted` are never reset per fishery; `totalCatch` even carries over from Step 3. Y_arr, price_arr and Ftarget_arr are therefore running totals over fisheries. (c) `c = catchBySpFishery[k*nfishery+nf]` reads with nf running up to K_num_fisheries, crossing into other species' rows. Function: `FishMIP_OSP_Bridge_Annual()`.

**Current code** (`ateconomic/atFishMIP_OSP_Bridge.c` lines 667–693)

```c
  667:         }
  668: 
  669:         for (nf = 0; nf < bm->K_num_fisheries; ++nf) {
  670:             this_nf = g_catch_n_effort_History[sp].targetFishery[nf];
  671:             for (k = 0; k < OSP_PARAMS->fished_nsp; ++k) {
  672:                 nfishery = g_catch_n_effort_History[sp].nfishery;
  673:                 c = OSP_PARAMS->catchBySpFishery[k * nfishery + nf];
  674:                 if (c <= 0.0) {
  675:                     continue;
  676:                 }
  677:                 totalCatch += c;
  678:                 revenue += c * OSP_PARAMS->price_by_sp[k];
  679:                 ftargetWeighted += c * OSP_PARAMS->ftargetBySp[k];
  680:             }
  681:             OSP_PARAMS->Y_arr[nf] = totalCatch / OSP_PARAMS->dt;
  682:             OSP_PARAMS->price_arr[nf] = (totalCatch > 0.0) ? revenue / totalCatch : 0.0;
  683:             OSP_PARAMS->effort_arr[nf] = AtlantisFisheryEffortTotal(bm, this_nf);
  684:             OSP_PARAMS->q_arr[nf] = OSP_PARAMS->catchability[this_nf];
  685:             OSP_PARAMS->comply_arr[nf] = OSP_PARAMS->compliance[this_nf];
  686:             OSP_PARAMS->dU_arr[nf] = OSP_PARAMS->invest_driver_U[this_nf];
  687:             OSP_PARAMS->dV_arr[nf] = OSP_PARAMS->invest_driver_V[this_nf];
  688:             
  689:             /* Catch-weighted mean target F across every species fishery nf lands, matching how price is aggregated above. */
  690:             OSP_PARAMS->Ftarget_arr[nf] = (totalCatch > 0.0) ? ftargetWeighted / totalCatch : OSP_PARAMS->Ftarget_placeholder;
  691:         }
  692: 
  693:         M_placeholder = OSP_PARAMS->vessel_attrition_M[0];  // TODO: Fix versus actually what is meant
```

**Suggested fix (from review):** Reset the three accumulators at the top of each nf iteration, use `this_nf = nf`, and index catch as `[k*K_num_fisheries + nf]`.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## ECIO-16

**stale variable** · Conf H / Sev H ⭐ priority  
**Listed lines:** 728-736

**Function:** `FishMIP_OSP_Bridge_Annual()`

**Why it's thought to be a bug**

In Mode C, `c = catchBySpFishery[k * nfishery + nf]` again uses the stale `nfishery` from Step 2 rather than species sp's value, so the catch weights for origprice are taken from the wrong slots. Function: `FishMIP_OSP_Bridge_Annual()`.

**Current code** (`ateconomic/atFishMIP_OSP_Bridge.c` lines 726–738)

```c
  726:                 continue;
  727:             }
  728:             for (nf = 0; nf < g_catch_n_effort_History[sp].nfishery; ++nf) {
  729:                 this_nf = g_catch_n_effort_History[sp].targetFishery[nf];
  730:                 c = OSP_PARAMS->catchBySpFishery[k * nfishery + nf];
  731:                 if (c <= 0.0) {
  732:                     continue;
  733:                 }
  734:                 weightedOrigSum += c * bm->SP_FISHERYprms[sp][this_nf][origprice_id];
  735:                 weightSum += c;
  736:             }
  737:             if (weightSum <= 0.0) {
  738:                 continue;
```

**Suggested fix (from review):** Use the per-species nfishery (with the ECIO-13 indexing fix).

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## ECIO-17

**type truncation** · Conf H / Sev H ⭐ priority  
**Listed lines:** 480, 576-578

**Function:** `FishMIP_OSP_Bridge_Annual()`

**Why it's thought to be a bug**

`gp` is declared `int` (l.480), but `FishMIP_Adapter_GetGlobalPriceForYear()` returns `double` (adapter .h:112). External prices are truncated to whole units, and any price below 1.0 becomes 0, which still passes `gp >= 0.0` and is written as the species price. Function: `FishMIP_OSP_Bridge_Annual()`.

**Current code** (`ateconomic/atFishMIP_OSP_Bridge.c` lines 574–578) — code appears to have moved +96 lines since the review

```c
  574:             overridden = 0;
  575:             for (k = 0; k < OSP_PARAMS->fished_nsp; ++k) {
  576:                 gp = FishMIP_Adapter_GetGlobalPriceForYear(FunctGroupArray[OSP_PARAMS->fishedSp[k]].groupCode, calendarYear);
  577:                 if (gp >= 0.0) {
  578:                     OSP_PARAMS->price_by_sp[k] = gp;
```

**Suggested fix (from review):** Declare `double gp;`.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## ECIO-18

**uninitialised variable** · Conf H / Sev M  
**Listed lines:** 134-142, 387

**Why it's thought to be a bug**

`g_catch_n_effort_History` is `malloc`'d and `firstcall` is never set, so `if (g_catch_n_effort_History[sp].firstcall > 0)` reads garbage. Either the back-fill never runs (past years stay 0 and the assessment sees an effort/catch collapse) or it runs unpredictably. Functions: `Create_OSP_Arrays()`, `RunPIMAssessment()`.

**Current code** (`ateconomic/atFishMIP_OSP_Bridge.c` lines 132–144)

```c
  132:   
  133:     //g_catch_n_effort_HistoryCount = 0;
  134:     g_catch_n_effort_History = (EffortHistoryRecord *)malloc(sizeof(EffortHistoryRecord) * (size_t)(bm->K_num_tot_sp));
  135:     
  136:     for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  137:         g_catch_n_effort_History[sp].targetFishery = Util_Alloc_Init_1D_Int(bm->K_num_fisheries, 0.0);
  138:         g_catch_n_effort_History[sp].nfishery =  CollectFleets(bm, sp) ;
  139:         g_catch_n_effort_History[sp].effortFlat = Util_Alloc_Init_1D_Double(arr_lengtth, 0.0);
  140:         g_catch_n_effort_History[sp].catchFlat = Util_Alloc_Init_1D_Double(arr_lengtth, 0.0);
  141:         g_catch_n_effort_History[sp].recorded = Util_Alloc_Init_1D_Int(SimLength, 0.0);
  142:     }
  143: 
  144:     return;
```

**Suggested fix (from review):** Set `firstcall = 1` in Create_OSP_Arrays (or use calloc).

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## ECIO-19

**unit mismatch** · Conf M / Sev H  
**Listed lines:** 400, 410, 435-448, 518, 602

**Why it's thought to be a bug**

Catch comes from `Harvest_Get_TotCumCatch()` (TotCumCatch, in mg N: ateconindicator.c:454-456 compares it to `TACamt*kg_2_mg/X_CN`). That catch feeds the PIM assessment, and the resulting `MSY` is written straight into `bm->TACamt[sp][nf][now_id]` and `SP_FISHERYprms[..][TAC_id]`, which are in tonnes/kg. The mg N catch is also used as "prod" for the price model and for revenue weighting. Functions: `RunPIMAssessment()`, `FeedPIMAssessmentIntoAtlantisTAC()`, `FishMIP_OSP_Bridge_Annual()`.

**Current code** (`ateconomic/atFishMIP_OSP_Bridge.c` lines 398–437 (truncated))

```c
  398:                 this_nf = g_catch_n_effort_History[sp].targetFishery[nf];
  399:                 g_catch_n_effort_History[sp].effortFlat[iy * nfishery + nf] = OSP_PARAMS->thisYearEffort[this_nf];
  400:                 g_catch_n_effort_History[sp].catchFlat[iy * nfishery + nf] = Harvest_Get_TotCumCatch(sp, this_nf, iy);
  401:             }
  402:             g_catch_n_effort_History[sp].recorded[iy] = 1;
  403:         }
  404:         g_catch_n_effort_History[sp].firstcall = 0;
  405:     } else {
  406:         /* This year only */
  407:         for (nf = 0; nf < g_catch_n_effort_History[sp].nfishery; ++nf) {
  408:             this_nf = g_catch_n_effort_History[sp].targetFishery[nf];
  409:             g_catch_n_effort_History[sp].effortFlat[bm->thisyear * nfishery + nf] = OSP_PARAMS->thisYearEffort[this_nf];
  410:             g_catch_n_effort_History[sp].catchFlat[bm->thisyear * nfishery + nf] = Harvest_Get_TotCumCatch(sp, this_nf, bm->thisyear);
  411:         }
  412:         g_catch_n_effort_History[sp].recorded[bm->thisyear] = 1;
  413:     }
  414:     FishMIP_Adapter_RunAssessment(sp, nyears, g_catch_n_effort_History[sp].nfishery, g_catch_n_effort_History[sp].catchFlat, g_catch_n_effort_History[sp].effortFlat,
  415:                                   OSP_PARAMS->assess_prior_mean_r, OSP_PARAMS->assess_prior_sd_logr, OSP_PARAMS->assess_prior_mean_z,
  416:                                   OSP_PARAMS->assess_prior_sd_logz, OSP_PARAMS->assess_optimizer, /*verbose=*/0, &result);
  417: 
  418:     if (!result.ok) {
  419:         return 0;
  420:     }
  421:     *outFMSY = result.FMSY;
  422:     *outMSY = result.MSY;
  423:     return 1;
  424: }
  425: 
  426: /** The "feed back to TACs" half of option (a): writes the plug-in's MSY
  427:  *  estimate into exactly the fields Atlantis's own TAC machinery reads
  428:  *  (bm->TACamt[sp][nf][now_id], consulted by atharvest/atHarvestIndex.c and
  429:  *  atimplementation/atImplementationAnnual.c; bm->SP_FISHERYprms[sp][nf]
  430:  *  [TAC_id], the "origin" TAC atimplementation/atImplementationAnnual.c's
  431:  *  TACorig scenario-painting path reads). Allocates the total MSY across
  432:  *  fisheries in proportion to each fishery's existing TAC share, the same
  433:  *  rule Atlantis's own Tier system uses to split a stock-wide TAC across
  434:  *  fisheries (atmanage/atManageTier.c:522). */
  435: static void FeedPIMAssessmentIntoAtlantisTAC(MSEBoxModel *bm, FILE *llogfp, int sp, double MSY) {
  436:     double TAC_old_total = 0.0;
  437:     double share, TACnf;
```

**Suggested fix (from review):** Convert catch with `* X_CN * mg_2_tonne` (and back) at the bridge boundary.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## ECIO-20

**defaults overwritten with 0** · Conf M / Sev M  
**Listed lines:** 321-331

**Function:** `ApplyLoadedDrivers()`

**Why it's thought to be a bug**

`ApplyLoadedDrivers()` guards catchability with `> 0.0` but copies `compliance`, `invest_driver_U` and `invest_driver_V` unconditionally. For a reference country with no EEZ, the adapter returns `ok=1` with these fields left at 0 (adapter .cpp:158-165). Compliance then becomes 0 and investment drivers 0 for every fishery. Function: `ApplyLoadedDrivers()`.

**Current code** (`ateconomic/atFishMIP_OSP_Bridge.c` lines 327–341) — code appears to have moved +8 lines since the review

```c
  327:             OSP_PARAMS->catchability[nf] = v.catchability_EEZ;
  328:         }
  329:         OSP_PARAMS->compliance[nf] = v.compliance;
  330:         OSP_PARAMS->invest_driver_U[nf] = v.invest_driver_U;
  331:         OSP_PARAMS->invest_driver_V[nf] = v.invest_driver_V;
  332:         /* All Atlantis fisheries share one M slot -- see "Fleet-dimension
  333:          * mapping" in Step 4's comment below; the last fishery processed
  334:          * here wins if fisheries mix fleet types, same as the original
  335:          * bridge. */
  336:         OSP_PARAMS->vessel_attrition_M[nf] = v.vessel_attrition_M;
  337:     }
  338: }
  339: 
  340: /* ==========================================================================
  341:  * One-time driver / external-price loaders (thin pass-throughs to the
```

**Suggested fix (from review):** Only overwrite when the driver set has EEZ data (expose hasEezPos) or the value is > 0.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## ECIO-21

**empty-dimension call** · Conf M / Sev L  
**Listed lines:** 370-416, 651-656

**Function:** `RunPIMAssessment()`

**Why it's thought to be a bug**

`RunPIMAssessment()` runs for every fished species even when `nfishery == 0` (no fishery targets it per `CollectFleets()`), so `fit_one_stock` gets a [nyears x 0] C/E matrix. Function: `RunPIMAssessment()`.

**Current code** (`ateconomic/atFishMIP_OSP_Bridge.c` lines 368–407 (truncated))

```c
  368:  *  Returns 1 and fills outFMSY/outMSY on success, 0 otherwise (not enough
  369:  *  history yet, or the fit didn't converge). */
  370: static int RunPIMAssessment(MSEBoxModel *bm, int sp, double *outFMSY, double *outMSY) {
  371:     int nyears = bm->thisyear + 1; /* Atlantis years 0..thisyear inclusive */
  372:     FishMIP_AssessmentResult result;
  373:     int iy, nf, this_nf;
  374:     //int SimLength = (int) (ceil((bm->tstop - bm->tstart) / (86400.0)));
  375:     //int arr_length = SimLength * bm-> K_num_fisheries;
  376:     int nfishery = g_catch_n_effort_History[sp].nfishery;
  377: 
  378:     if (nyears < OSP_PARAMS->assess_min_years) {
  379:         return 0;
  380:     }
  381: 
  382:     Util_Init_1D_Double(OSP_PARAMS->thisYearEffort, bm-> K_num_fisheries, 0.0);
  383:     for (nf = 0; nf < bm->K_num_fisheries; ++nf) {
  384:         OSP_PARAMS->thisYearEffort[nf] = AtlantisFisheryEffortTotal(bm, nf);
  385:     }
  386: 
  387:     if(g_catch_n_effort_History[sp].firstcall > 0){
  388:         /* If the bridge only started running mid-simulation, earlier years have
  389:          * no recorded entry -- back-fill (permanently -- recorded[iy] is set so
  390:          * a later year never overwrites it again) with the earliest known
  391:          * effort rather than leaving zeros, so the assessment doesn't see a
  392:          * spurious effort collapse at the start of its time series. */
  393:         for (iy = 0; iy < nyears; ++iy) {
  394:             if (g_catch_n_effort_History[sp].recorded[iy]) {
  395:                 continue;
  396:             }
  397:             for (nf = 0; nf < g_catch_n_effort_History[sp].nfishery; ++nf) {
  398:                 this_nf = g_catch_n_effort_History[sp].targetFishery[nf];
  399:                 g_catch_n_effort_History[sp].effortFlat[iy * nfishery + nf] = OSP_PARAMS->thisYearEffort[this_nf];
  400:                 g_catch_n_effort_History[sp].catchFlat[iy * nfishery + nf] = Harvest_Get_TotCumCatch(sp, this_nf, iy);
  401:             }
  402:             g_catch_n_effort_History[sp].recorded[iy] = 1;
  403:         }
  404:         g_catch_n_effort_History[sp].firstcall = 0;
  405:     } else {
  406:         /* This year only */
  407:         for (nf = 0; nf < g_catch_n_effort_History[sp].nfishery; ++nf) {
```

**Suggested fix (from review):** `if (nfishery == 0) return 0;`.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## ECIO-22

**memory leak / dead code** · Conf L / Sev L  
**Listed lines:** 148-187

**Function:** `FreeOSPStruct()`

**Why it's thought to be a bug**

`FreeOSPStruct()` is static and never called, and no public free exists, so all OSP arrays leak; it will also trigger an unused-function warning. The unused `done` parameter suggests it was meant to be wired up. Function: `FreeOSPStruct()`.

**Current code** (`ateconomic/atFishMIP_OSP_Bridge.c` lines 146–185 (truncated))

```c
  146: 
  147: 
  148: static void FreeOSPStruct(MSEBoxModel *bm, int done) {
  149:     int sp;
  150:     
  151:     free1d(OSP_PARAMS->catchability);
  152:     free1d(OSP_PARAMS->compliance);
  153:     free1d(OSP_PARAMS->invest_driver_U);
  154:     free1d(OSP_PARAMS->invest_driver_V);
  155:     i_free1d(OSP_PARAMS->fisheryFleetType);
  156:     
  157:     free1d(OSP_PARAMS->ftargetBySp);
  158:     free1d(OSP_PARAMS->prod);
  159:     free1d(OSP_PARAMS->X);
  160:     free1d(OSP_PARAMS->W);
  161:     
  162:     free1d(OSP_PARAMS->thisYearEffort);
  163:     free1d(OSP_PARAMS->price_arr);
  164:     free1d(OSP_PARAMS->Y_arr);
  165:     free1d(OSP_PARAMS->effort_arr);
  166:     free1d(OSP_PARAMS->q_arr);
  167:     free1d(OSP_PARAMS->comply_arr);
  168:     free1d(OSP_PARAMS->dU_arr);
  169:     free1d(OSP_PARAMS->dV_arr);
  170:     free1d(OSP_PARAMS->Ftarget_arr);
  171:     free1d(OSP_PARAMS->f_out);
  172:     free1d(OSP_PARAMS->catchBySpFishery);
  173:     free1d(OSP_PARAMS->price_by_sp);
  174:     i_free1d(OSP_PARAMS->fishedSp);
  175:     free1d(OSP_PARAMS->vessel_attrition_M);
  176:     
  177:     free(OSP_PARAMS);
  178:     for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  179:         i_free1d(g_catch_n_effort_History[sp].targetFishery);
  180:         free1d(g_catch_n_effort_History[sp].effortFlat);
  181:         free1d(g_catch_n_effort_History[sp].catchFlat);
  182:         i_free1d(g_catch_n_effort_History[sp].recorded);
  183:     }
  184:     free(g_catch_n_effort_History);
  185:     
```

**Suggested fix (from review):** Expose it (e.g. `Free_OSP_Arrays`) and call it from Economic_Free.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## Lower-confidence notes (speculative — check before acting)

These came from section B.3 of the list. They were not raised as findings. Tick if you want one followed up.

### LC-1

atFishMIP_OSP_Bridge.c:67/141: `SimLength` is in days, but `recorded`/`effortFlat` are indexed by year. That is oversized (no bug), but the naming suggests a years-vs-days mix-up.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atFishMIP_OSP_Bridge.c:443: when there is no prior TAC, MSY is split evenly across all fisheries, including ones that cannot catch the species.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
