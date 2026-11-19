# atecology/atcoral.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **8 findings** (severity H: 2; confidence H and severity H: 1; on the priority list: 1), plus 1 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [NUTR-21](#nutr-21) ⭐ | 932-942 | H/H | wrong constant | When the model has no turf, `facilitate_space = 1.0`, then `ans = mum * (1 - compete_space) * (1.0 + facilitat… |  | [ ] |
| [NUTR-22](#nutr-22) | 369, 398-437, 534-543 | H/M | variable only set in gated block | `time_since_last_shock` starts at 0 and is only updated inside `if (do_DHW_now)`. That block is disabled for `… |  | [ ] |
| [NUTR-23](#nutr-23) | 446-457 | M/M | repeated application | The bleaching decrement `PropUnBleached -= prop_bleaching` and the extra mortality are applied on every call w… |  | [ ] |
| [NUTR-24](#nutr-24) | 104-110, 189-191 | M/M | mass balance | Polyp mortality N `M = SPmortNH_polyp` is split as NH `M(1-FDM)(1-H)`, symbiont recycling `M(1-FDM)H` (110) an… |  | [ ] |
| [NUTR-25](#nutr-25) | 1338 (atGroupProcesses.c) vs 686-752, 684 | M/H | flux used as state | In the species-level path, mechanistic/blackwood/bioerosion return a rate of change ("returning a flux so that… |  | [ ] |
| [NUTR-26](#nutr-26) | 45-47, 847 | M/M | index (cohort vs stage) | `cohortSpeciesParams` is allocated `[numStages][params]` (atUtilGroupIO.c:830/855/877), but here it is indexed… |  | [ ] |
| [NUTR-27](#nutr-27) | 852 | H/L | log flooding | Unconditional `fprintf(llogfp, "Destroy_Rugosity: ...")` on every call (every fishery/box/cohort). Function: `… |  | [ ] |
| [NUTR-28](#nutr-28) | 925 | M/L | log flooding | `fprintf(bm->logFile, "DANGER DANGER DANGER ...")` runs on every legitimate maturation-trait update path (when… |  | [ ] |

---

## NUTR-21

**wrong constant** · Conf H / Sev H ⭐ priority  
**Listed lines:** 932-942

**Function:** `Coral_Space_Competition()`

**Why it's thought to be a bug**

When the model has no turf, `facilitate_space = 1.0`, then `ans = mum * (1 - compete_space) * (1.0 + facilitate_space)`, which doubles coral/sponge growth. It also discards any HABITATlike facilitation accumulated at 919-920. The returned scaled_mum is passed into Eat() as the heterotrophic mum (atGroupProcesses.c:1311), so heterotrophic growth doubles in every no-turf model. Function: `Coral_Space_Competition()`.

**Current code** (`atecology/atcoral.c` lines 930–944)

```c
  930: 		}
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
**Listed lines:** 369, 398-437, 534-543

**Function:** `Do_Bleaching()`

**Why it's thought to be a bug**

`time_since_last_shock` starts at 0 and is only updated inside `if (do_DHW_now)`. That block is disabled for `cohort > 0` and for `it_count > 1` (398-402). Recovery needs `time_since_last_shock > 0` (534) or `> recovery_periodB` (537), so `PropUnBleached[cohort>0]` never recovers after bleaching. Cohort 0 recovers only on the first adaptive iteration. Function: `Do_Bleaching()`.

**Current code** (`atecology/atcoral.c` lines 368–372)

```c
  368: void Do_Bleaching(MSEBoxModel *bm, FILE *llogfp, int guild, int cohort, double sp_biom){
  369: 	int d;
  370:     int time_since_last_shock = 0;
  371:     int sp_id = (int) (FunctGroupArray[guild].speciesParams[coralID_id]);
  372:     int recovery_periodA = (int)(FunctGroupArray[guild].speciesParams[bleach_periodA_id]);
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
**Listed lines:** 446-457

**Function:** `Do_Bleaching()`

**Why it's thought to be a bug**

The bleaching decrement `PropUnBleached -= prop_bleaching` and the extra mortality are applied on every call while `DHWsum > thresh`, including every adaptive-step iteration (`it_count > 1`), not once per timestep. Effective bleaching scales with the number of iterations. Function: `Do_Bleaching()`.

**Current code** (`atecology/atcoral.c` lines 444–459)

```c
  444:     */
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
**Listed lines:** 104-110, 189-191

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
**Listed lines:** 1338 (atGroupProcesses.c) vs 686-752, 684

**Function:** `Calculate_Rugosity()`

**Why it's thought to be a bug**

In the species-level path, mechanistic/blackwood/bioerosion return a rate of change ("returning a flux so that will be added to LocalRugosity", 751). The caller atGroupProcesses.c:1338 assigns it straight into the state `localWCTracers[Rugosity_i]`, and each coral/sponge guild/cohort overwrites the previous one. The bozec case returns a state. The georugosity floor (818-821) is applied to the flux value. Function: `Calculate_Rugosity()`.

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
**Listed lines:** 45-47, 847

**Why it's thought to be a bug**

`cohortSpeciesParams` is allocated `[numStages][params]` (atUtilGroupIO.c:830/855/877), but here it is indexed by `cohort` (up to numCohortsXnumGenes-1): `[cohort][mL_id]`, `[cohort][mQ_id]`, `[cohort][mE_id]`, `[cohort][rugosity_dec_id]`. Elsewhere in this file it is correctly `[stage]` (642, 687, 711). Coral groups with more cohorts or genotypes than stages read out of bounds. Functions: `Grow_Coral_Symbionts()`, `Destroy_Rugosity()`.

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
**Listed lines:** 852

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
**Listed lines:** 925

**Function:** `Apply_Evol_Scalar()`

**Why it's thought to be a bug**

`fprintf(bm->logFile, "DANGER DANGER DANGER ...")` runs on every legitimate maturation-trait update path (when `min_li_mat <= 0`). Function: `Apply_Evol_Scalar()` (atdiversity.c, listed here for completeness).

**Current code** (`atecology/atcoral.c` lines 922–926)

```c
  922:                 
  923:                 /*
  924:                 fprintf(llogfp, "Time: %e box%d-%d %s-%d facilitate_space %e as %s cover: %e splike: %e\n",
  925:                         bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, cohort, facilitate_space, FunctGroupArray[sp].groupCode,
  926:                         boxLayerInfo->localEPITracers[FunctGroupArray[sp].CoverTracer], bm->HABITATlike[guild][stage][FunctGroupArray[sp].coverID]);
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

atcoral.c `Do_Bleaching()` (420-424): once all shocks have left the DHW queue, `time_since_last_shock` returns to 0, so slow recovery (`> recovery_periodB`) only happens if recovery_periodB < K_max_num_DHW.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
