# atecology/atcoral.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **8 findings** (severity H: 2; confidence H and severity H: 1; on the priority list: 1), plus 1 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [NUTR-21](#nutr-21) ⭐ | 933-943 | H/H | wrong constant | When the model has no turf, `facilitate_space = 1.0`, then `ans = mum * (1 - compete_space) * (1.0 + facilitat… |  | [ ] |
| [NUTR-22](#nutr-22) | 370, 399-438, 535-544 | H/M | variable only set in gated block | `time_since_last_shock` starts at 0 and is only updated inside `if (do_DHW_now)`. That block is disabled for `… |  | [ ] |
| [NUTR-23](#nutr-23) | 447-458 | M/M | repeated application | The bleaching decrement `PropUnBleached -= prop_bleaching` and the extra mortality are applied on every call w… |  | [ ] |
| [NUTR-24](#nutr-24) | 104-110, 189-191 | M/M | mass balance | Polyp mortality N `M = SPmortNH_polyp` is split as NH `M(1-FDM)(1-H)`, symbiont recycling `M(1-FDM)H` (110) an… |  | [ ] |
| [NUTR-25](#nutr-25) | 1338? (atGroupProcesses.c) vs 699-765, 697 | M/H | flux used as state | In the species-level path, mechanistic/blackwood/bioerosion return a rate of change ("returning a flux so that… |  | [ ] |
| [NUTR-26](#nutr-26) | 45-47, 848 | M/M | index (cohort vs stage) | `cohortSpeciesParams` is allocated `[numStages][params]` (atUtilGroupIO.c:830/855/877), but here it is indexed… |  | [ ] |
| [NUTR-27](#nutr-27) | 853 | H/L | log flooding | Unconditional `fprintf(llogfp, "Destroy_Rugosity: ...")` on every call (every fishery/box/cohort). Function: `… |  | [ ] |
| [NUTR-28](#nutr-28) | 925 | M/L | log flooding | `fprintf(bm->logFile, "DANGER DANGER DANGER ...")` runs on every legitimate maturation-trait update path (when… |  | [ ] |

---

## NUTR-21

**wrong constant** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 933-943  
**Lines at review (Oct 2026):** 932-942

**Function:** `Coral_Space_Competition()`

**Why it's thought to be a bug**

When the model has no turf, `facilitate_space = 1.0`, then `ans = mum * (1 - compete_space) * (1.0 + facilitate_space)`, which doubles coral/sponge growth. It also discards any HABITATlike facilitation accumulated at 919-920. The returned scaled_mum is passed into Eat() as the heterotrophic mum (atGroupProcesses.c:1329), so heterotrophic growth doubles in every no-turf model. Function: `Coral_Space_Competition()`.

**Current code** (`atecology/atcoral.c` lines 931–945)

```c
  931: 	}
  932: 
  933: 	if(!bm->containsTurf) {
  934: 		facilitate_space = 1.0;
  935: 	}
  936: 
  937: 	/* Sanity check */
  938: 	if(compete_space > 1.0)
  939: 		compete_space = 1.0;
  940: 	if(facilitate_space > 1.0)
  941: 		facilitate_space = 1.0;
  942: 
  943: 	ans = mum * (1.0 - compete_space) * (1.0 + facilitate_space);
  944: 
  945: 	if ((bm->which_check == guild) && (bm->current_box == bm->checkbox)) {
```

**Suggested fix (from review):** Set `facilitate_space = 0.0` (neutral for the `1+f` form) or only reset it when no epiff facilitation applies.

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

## NUTR-22

**variable only set in gated block** · Conf H / Sev M  
**Lines (current code):** 370, 399-438, 535-544  
**Lines at review (Oct 2026):** 369, 398-437, 534-543

**Function:** `Do_Bleaching()`

**Why it's thought to be a bug**

`time_since_last_shock` starts at 0 and is only updated inside `if (do_DHW_now)`. That block is disabled for `cohort > 0` and for `it_count > 1` (399-403). Recovery needs `time_since_last_shock > 0` (535) or `> recovery_periodB` (538), so `PropUnBleached[cohort>0]` never recovers after bleaching. Cohort 0 recovers only on the first adaptive iteration. Function: `Do_Bleaching()`.

**Current code** (`atecology/atcoral.c` lines 368–407 (truncated))

```c
  368: void Do_Bleaching(MSEBoxModel *bm, FILE *llogfp, int guild, int cohort, double sp_biom){
  369: 	int d;
  370:     int time_since_last_shock = 0;
  371:     int sp_id = (int) (FunctGroupArray[guild].speciesParams[coralID_id]);
  372:     int recovery_periodA = (int)(FunctGroupArray[guild].speciesParams[bleach_periodA_id]);
  373: 	int recovery_periodB = (int)(FunctGroupArray[guild].speciesParams[bleach_periodB_id]);
  374:     double timestep_correction = FunctGroupArray[guild].speciesParams[Mdt_id] / 86400;
  375: 	double amt_recovering, period_effect, prop_bleaching, lost_biom, dead_biom;
  376:     double bleach_mort_rate = FunctGroupArray[guild].speciesParams[mBleach_id];
  377: 	double bleach_rate = FunctGroupArray[guild].speciesParams[bleaching_rate_id] * FunctGroupArray[guild].speciesParams[Mdt_id];
  378: 	double recovery_rate = FunctGroupArray[guild].speciesParams[bleach_recovery_rate_id];
  379: 	double temp_shift_rate = FunctGroupArray[guild].speciesParams[bleach_tempshift_id];
  380: 	double grow_shift_rate = FunctGroupArray[guild].speciesParams[bleach_growshift_id];
  381: 	double local_temp_thresh = FunctGroupArray[guild].speciesParams[bleach_temp_id] * CORALREEF[sp_id].TempShift[bm->current_box];
  382: 	double zooxanth_biom = sp_biom * (FunctGroupArray[guild].speciesParams[prop_zooxanth_id] * CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box]);
  383: 	double polyp_biom = sp_biom - zooxanth_biom;
  384:     double min_bleach_salt_sp = FunctGroupArray[guild].speciesParams[min_bleach_salt_id];
  385:     double max_bleach_salt_sp = FunctGroupArray[guild].speciesParams[max_bleach_salt_id];
  386:     //double min_bleach_temp_sp = FunctGroupArray[guild].speciesParams[min_bleach_temp_id];
  387:     int do_DHW_now = 1;
  388:     int sp_active = (int) (FunctGroupArray[guild].speciesParams[flagactive_id]);
  389:     
  390:     if (bm->dt < 86400) {  // Assumes 1 = day time, 0 = night time
  391:         if (sp_active == 2) { /* No preference for night or day */
  392:             if(!bm->flagday)
  393:                 do_DHW_now = 0;   // Don't do it at night - want it in the day time if have night and day
  394:         } else if (sp_active != bm->flagday) {  /* Base it on when active */
  395:             do_DHW_now = 0;
  396:         }
  397:     }
  398:     
  399:     if (cohort || (it_count > 1)) {
  400:         // Only want it for the youngest - as then know done for all - and in the first iteration of the adaptive
  401:         // timestep as don't need to do it for every age group as the answer is identical for all
  402:         do_DHW_now = 0;
  403:     }
  404:     
  405: 	/* Look at past week of temperatures - if 1-2 deg C above threshold temperature then
  406: 	 * flag as "degree heating week", if get a month of those (i.e. 4 continuous weeks)
  407: 	 * then bleach with associated mortality and biomass (non-terminal) losses.
```

**Suggested fix (from review):** Store time-since-shock per box in CORALREEF (computed when the queue updates) and read it for all cohorts and iterations.

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

## NUTR-23

**repeated application** · Conf M / Sev M  
**Lines (current code):** 447-458  
**Lines at review (Oct 2026):** 446-457

**Function:** `Do_Bleaching()`

**Why it's thought to be a bug**

The bleaching decrement `PropUnBleached -= prop_bleaching` and the extra mortality are applied on every call while `DHWsum > thresh`, including every adaptive-step iteration (`it_count > 1`), not once per timestep. Effective bleaching scales with the number of iterations. Function: `Do_Bleaching()`.

**Current code** (`atecology/atcoral.c` lines 445–460)

```c
  445:     
  446: 	/* See if potential bleaching event triggered */
  447: 	if(CORALREEF[sp_id].DHWsum[bm->current_box] > FunctGroupArray[guild].speciesParams[DHW_thresh_id]) {
  448: 
  449: 		/* Find proportion bleaching */
  450: 		period_effect = CORALREEF[sp_id].DHWsum[bm->current_box] - FunctGroupArray[guild].speciesParams[DHW_thresh_id]; // i.e. how much it exceeds the threshold by
  451: 		prop_bleaching = bleach_rate * (1.0 + period_effect);
  452:         if(prop_bleaching < 0.0)
  453:             prop_bleaching = 0.0;
  454:         
  455: 		CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] -= prop_bleaching;
  456:         if(CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] < 0.0)
  457:             CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] = 0.0;
  458:         
  459:        // Sanity check - could be much greater > 1.0 from calculations if super high temperature
  460:         if (prop_bleaching > 1.0)
```

**Suggested fix (from review):** Gate the PropUnBleached update with `it_count == 1` (mortality can stay as a rate).

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

## NUTR-24

**mass balance** · Conf M / Sev M  
**Lines (current code):** 104-110, 189-191  
**Lines at review (Oct 2026):** 104-110, 189-191

**Function:** `Grow_Coral_Symbionts()`

**Why it's thought to be a bug**

Polyp mortality N `M = SPmortNH_polyp` is split as NH `M(1-FDM)(1-H)`, symbiont recycling `M(1-FDM)H` (110) and detritus `M FDM (1-H)` (190-191). These sum to `M - M·H·FDM`, so N leaks whenever Host_Resp_Remin>0 and FDM>0. Also `SP_zooxanth_mort /= smLayerThick` and `SP_polyp_mort /= smLayerThick` (105, 107) are computed and never used. Function: `Grow_Coral_Symbionts()`.

**Current code** (`atecology/atcoral.c` lines 102–112)

```c
  102:     
  103: 	/* Convert into useful units for handing to nutrient and detritus pools */
  104: 	SPmortNH_zx = SP_zooxanth_mort / wcLayerThick;
  105: 	SP_zooxanth_mort /= smLayerThick;
  106: 	SPmortNH_polyp = SP_polyp_mort / wcLayerThick;
  107: 	SP_polyp_mort /= smLayerThick;
  108: 
  109: 	/* Calculate limitation factors */
  110: 	host_resp_N_for_symbiont = SPmortNH_polyp * (1.0 - FDM_SP) * Host_Resp_Remin_SP;
  111: 	tot_N = DIN + host_resp_N_for_symbiont;
  112: 	hN_sp = Nutrient_Lim(bm, lim_case, micro_case, tot_N, Si, Fe, P, PRatio, P0, P1,
```

**Suggested fix (from review):** Detritus terms should be `SPmortNH_polyp * FDM_SP * FDL_SP` (no `(1-H)`).

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

## NUTR-25

**flux used as state** · Conf M / Sev H  
**Lines (current code):** 1338? (atGroupProcesses.c) vs 699-765, 697  
**Lines at review (Oct 2026):** 1338 (atGroupProcesses.c) vs 686-752, 684

**Function:** `Calculate_Rugosity()`

**Why it's thought to be a bug**

In the species-level path, mechanistic/blackwood/bioerosion return a rate of change ("returning a flux so that will be added to LocalRugosity", 751). The caller atGroupProcesses.c:1356 assigns it straight into the state `localWCTracers[Rugosity_i]`, and each coral/sponge guild/cohort overwrites the previous one. The bozec case returns a state. The georugosity floor (819-822) is applied to the flux value. Function: `Calculate_Rugosity()`.

**Current code** (`atecology/atcoral.c` lines 1336–986) — ⚠ the quoted code was not found in the current file (changed, or already fixed?). These are just the listed line numbers, which may now hold unrelated code; `Calculate_Rugosity()` now starts at line 572

```c

```

**Suggested fix (from review):** Caller should accumulate into `localWCFlux[Rugosity_i]` for flux models, and only assign for bozec.

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

## NUTR-26

**index (cohort vs stage)** · Conf M / Sev M  
**Lines (current code):** 45-47, 848  
**Lines at review (Oct 2026):** 45-47, 847

**Why it's thought to be a bug**

`cohortSpeciesParams` is allocated `[numStages][params]` (atUtilGroupIO.c:829/855/877), but here it is indexed by `cohort` (up to numCohortsXnumGenes-1): `[cohort][mL_id]`, `[cohort][mQ_id]`, `[cohort][mE_id]`, `[cohort][rugosity_dec_id]`. Elsewhere in this file it is correctly `[stage]` (643, 688, 712). Coral groups with more cohorts or genotypes than stages read out of bounds. Functions: `Grow_Coral_Symbionts()`, `Destroy_Rugosity()`.

**Current code** (`atecology/atcoral.c` lines 43–49)

```c
   43: 	double hN_sp, hI_sp, sp_grow, scale_uptake, uptakeNH, uptakeNO, uptakeSi, uptakeFe, uptakeP = 0, uptakeC = 0;
   44: 	double tot_N, host_resp_N_for_symbiont, SP_zooxanth_mort, SP_polyp_mort, SPmortNH_zx, SPmortNH_polyp;
   45: 	double mL_SP = FunctGroupArray[guild].cohortSpeciesParams[cohort][mL_id] + mL_other;
   46: 	double mQ_SP = FunctGroupArray[guild].cohortSpeciesParams[cohort][mQ_id];
   47: 	double mE_SP = FunctGroupArray[guild].cohortSpeciesParams[cohort][mE_id];
   48: 	double FDM_SP = FunctGroupArray[guild].speciesParams[FDMort_id];
   49: 	double Host_Resp_Remin_SP = FunctGroupArray[guild].speciesParams[HostRemin_id];
```

**Suggested fix (from review):** Use `FunctGroupArray[guild].cohort_stage[cohort]` as the first index.

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

## NUTR-27

**log flooding** · Conf H / Sev L  
**Lines (current code):** 853  
**Lines at review (Oct 2026):** 852

**Function:** `Destroy_Rugosity()`

**Why it's thought to be a bug**

Unconditional `fprintf(llogfp, "Destroy_Rugosity: ...")` on every call (every fishery/box/cohort). Function: `Destroy_Rugosity()`.

**Current code** (`atecology/atcoral.c` lines 851–855)

```c
  851:     
  852:     
  853:     fprintf(llogfp,"Destroy_Rugosity: Time: %e box%d-%d Rugosity: %e dead_biom: %e rate_rug_crumble: %e\n", bm->dayt, bm->current_box, bm->current_layer, boxLayerInfo->localWCFlux[Rugosity_i], dead_biom, rate_rug_crumble);
  854: 
  855: }
```

**Suggested fix (from review):** Guard with debug flag.

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

## NUTR-28

**log flooding** · Conf M / Sev L  
**Lines (current code):** 925  
**Lines at review (Oct 2026):** 925

**Function:** `Apply_Evol_Scalar()`

**Why it's thought to be a bug**

`fprintf(bm->logFile, "DANGER DANGER DANGER ...")` runs on every legitimate maturation-trait update path (when `min_li_mat <= 0`). Function: `Apply_Evol_Scalar()` (atdiversity.c, listed here for completeness).

**Current code** (`atecology/atcoral.c` lines 923–927)

```c
  923:                 /*
  924:                 fprintf(llogfp, "Time: %e box%d-%d %s-%d facilitate_space %e as %s cover: %e splike: %e\n",
  925:                         bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, cohort, facilitate_space, FunctGroupArray[sp].groupCode,
  926:                         boxLayerInfo->localEPITracers[FunctGroupArray[sp].CoverTracer], bm->HABITATlike[guild][stage][FunctGroupArray[sp].coverID]);
  927:                 */
```

**Suggested fix (from review):** Remove or guard.

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

atcoral.c `Do_Bleaching()` (421-425): once all shocks have left the DHW queue, `time_since_last_shock` returns to 0, so slow recovery (`> recovery_periodB`) only happens if recovery_periodB < K_max_num_DHW.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
