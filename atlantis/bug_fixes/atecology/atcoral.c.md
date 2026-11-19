# atecology/atcoral.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **8 findings** (severity H: 2; confidence H and severity H: 1; on the priority list: 1), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [NUTR-21](#nutr-21) ⭐ | 934, 943 | H/H | wrong constant | When the model has no turf, `facilitate_space = 1.0`, then `ans = mum * (1 - compete_space) * (1.0 + facilitat… |  | [ ] |
| [NUTR-22](#nutr-22) | 411, 399, 535 | H/M | variable only set in gated block | `time_since_last_shock` starts at 0 and is only updated inside `if (do_DHW_now)`. That block is disabled for `… |  | [ ] |
| [NUTR-23](#nutr-23) | 455, 399 | M/M | repeated application | The bleaching decrement `PropUnBleached -= prop_bleaching` and the extra mortality are applied on every call w… |  | [ ] |
| [NUTR-24](#nutr-24) | 105 | M/M | mass balance | Polyp mortality N `M = SPmortNH_polyp` is split as NH `M(1-FDM)(1-H)`, symbiont recycling `M(1-FDM)H` (110) an… |  | [ ] |
| [NUTR-25](#nutr-25) | atGroupProcesses.c:1349 | M/H | flux used as state | In the species-level path, mechanistic/blackwood/bioerosion return a rate of change ("returning a flux so that… |  | [ ] |
| [NUTR-26](#nutr-26) | 45, 848, 688 | M/M | index (cohort vs stage) | `cohortSpeciesParams` is allocated `[numStages][params]` (atUtilGroupIO.c:830/855/877), but here it is indexed… |  | [ ] |
| [NUTR-27](#nutr-27) | 853 | H/L | log flooding | Unconditional `fprintf(llogfp, "Destroy_Rugosity: ...")` on every call (every fishery/box/cohort). Function: `… |  | [ ] |
| [NUTR-28](#nutr-28) | 488 | M/L | log flooding | `fprintf(bm->logFile, "DANGER DANGER DANGER ...")` runs on every legitimate maturation-trait update path (when… |  | [ ] |

---

## NUTR-21

**wrong constant** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atcoral.c` 934, `atcoral.c` 943
**Lines at review (Oct 2026):** 932-942

**Function:** `Coral_Space_Competition()`

**Why it's thought to be a bug**

When the model has no turf, `facilitate_space = 1.0`, then `ans = mum * (1 - compete_space) * (1.0 + facilitate_space)`, which doubles coral/sponge growth. It also discards any HABITATlike facilitation accumulated at 919-920. The returned scaled_mum is passed into Eat() as the heterotrophic mum (atGroupProcesses.c:1329), so heterotrophic growth doubles in every no-turf model. Function: `Coral_Space_Competition()`.

**Code at issue** — `atcoral.c` line 934  
Search for: `facilitate_space = 1.0`

```c
  932: 
  933: 	if(!bm->containsTurf) {
  934→ 		facilitate_space = 1.0;
  935: 	}
  936: 
```

**Code at issue** — `atcoral.c` line 943 (closest match to the quoted code; wording differs slightly)  
Search for: `ans = mum * (1.0 - compete_space) * (1.0 + facilitate_space);`

```c
  941: 		facilitate_space = 1.0;
  942: 
  943→ 	ans = mum * (1.0 - compete_space) * (1.0 + facilitate_space);
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
**Found at (current code):** `atcoral.c` 411, `atcoral.c` 399, `atcoral.c` 535
**Lines at review (Oct 2026):** 369, 398-437, 534-543

**Function:** `Do_Bleaching()`

**Why it's thought to be a bug**

`time_since_last_shock` starts at 0 and is only updated inside `if (do_DHW_now)`. That block is disabled for `cohort > 0` and for `it_count > 1` (399-403). Recovery needs `time_since_last_shock > 0` (535) or `> recovery_periodB` (538), so `PropUnBleached[cohort>0]` never recovers after bleaching. Cohort 0 recovers only on the first adaptive iteration. Function: `Do_Bleaching()`.

**Code at issue** — `atcoral.c` line 411  
Search for: `if(do_DHW_now)`

```c
  409: 
  410: 	/** Update the current temperature status **/
  411→ 	if(do_DHW_now) {
  412:         if (bleach_rate < 0.0) // Sanity check - needed here as used later in a couple of spots - for temperature and salinity so better than 2 tests there
  413:             bleach_rate = 0.0;
```

**Code at issue** — `atcoral.c` line 399  
Search for: `it_count > 1`

```c
  397:     }
  398:     
  399→     if (cohort || (it_count > 1)) {
  400:         // Only want it for the youngest - as then know done for all - and in the first iteration of the adaptive
  401:         // timestep as don't need to do it for every age group as the answer is identical for all
```

**Code at issue** — `atcoral.c` line 535  
Search for: `time_since_last_shock > 0`

```c
  533: 
  534: 	/* Deal with any recovery of zooxanthellae by host polys post bleaching */
  535→ 	if ((time_since_last_shock > 0) && (time_since_last_shock < recovery_periodA)) {
  536: 		// Quick recover so thermal toleerance and growth rates uneffected as have taken back in the same zooxanthellae clade
  537: 		CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] += recovery_rate * timestep_correction;
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
**Found at (current code):** `atcoral.c` 455, `atcoral.c` 399
**Lines at review (Oct 2026):** 446-457

**Function:** `Do_Bleaching()`

**Why it's thought to be a bug**

The bleaching decrement `PropUnBleached -= prop_bleaching` and the extra mortality are applied on every call while `DHWsum > thresh`, including every adaptive-step iteration (`it_count > 1`), not once per timestep. Effective bleaching scales with the number of iterations. Function: `Do_Bleaching()`.

**Code at issue** — `atcoral.c` line 455 (closest match to the quoted code; wording differs slightly)  
Search for: `CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] -= prop_bleaching;`

```c
  453:             prop_bleaching = 0.0;
  454:         
  455→ 		CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] -= prop_bleaching;
  456:         if(CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] < 0.0)
  457:             CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] = 0.0;
```

**Code at issue** — `atcoral.c` line 399  
Search for: `it_count > 1`

```c
  397:     }
  398:     
  399→     if (cohort || (it_count > 1)) {
  400:         // Only want it for the youngest - as then know done for all - and in the first iteration of the adaptive
  401:         // timestep as don't need to do it for every age group as the answer is identical for all
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
**Found at (current code):** `atcoral.c` 105
**Lines at review (Oct 2026):** 104-110, 189-191

**Function:** `Grow_Coral_Symbionts()`

**Why it's thought to be a bug**

Polyp mortality N `M = SPmortNH_polyp` is split as NH `M(1-FDM)(1-H)`, symbiont recycling `M(1-FDM)H` (110) and detritus `M FDM (1-H)` (190-191). These sum to `M - M·H·FDM`, so N leaks whenever Host_Resp_Remin>0 and FDM>0. Also `SP_zooxanth_mort /= smLayerThick` and `SP_polyp_mort /= smLayerThick` (105, 107) are computed and never used. Function: `Grow_Coral_Symbionts()`.

**Code at issue** — `atcoral.c` line 105  
Search for: `SP_zooxanth_mort /= smLayerThick`

```c
  103: 	/* Convert into useful units for handing to nutrient and detritus pools */
  104: 	SPmortNH_zx = SP_zooxanth_mort / wcLayerThick;
  105→ 	SP_zooxanth_mort /= smLayerThick;
  106: 	SPmortNH_polyp = SP_polyp_mort / wcLayerThick;
  107: 	SP_polyp_mort /= smLayerThick;
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
**Found at (current code):** `atGroupProcesses.c` 1349
**Lines at review (Oct 2026):** 1338 (atGroupProcesses.c) vs 686-752, 684

**Function:** `Calculate_Rugosity()`

**Why it's thought to be a bug**

In the species-level path, mechanistic/blackwood/bioerosion return a rate of change ("returning a flux so that will be added to LocalRugosity", 751). The caller atGroupProcesses.c:1356 assigns it straight into the state `localWCTracers[Rugosity_i]`, and each coral/sponge guild/cohort overwrites the previous one. The bozec case returns a state. The georugosity floor (819-822) is applied to the flux value. Function: `Calculate_Rugosity()`.

**Code at issue** — `atGroupProcesses.c` line 1349  
Search for: `localWCTracers[Rugosity_i]`

```c
 1347:         Coral_Limiting_Growth_Factors(bm, llogfp, guild, cohort, sed_level);
 1348: 
 1349→         boxLayerInfo->localWCTracers[Rugosity_i] = Calculate_Rugosity(bm, guild, cohort, llogfp, 1);
 1350:                 
 1351:         //fprintf(llogfp,"Coral_Process: Time: %e box%d-%d %s-%d Rugosity: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, cohort, boxLayerInfo->localWCTracers[Rugosity_i]);
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
**Found at (current code):** `atcoral.c` 45, `atcoral.c` 848, `atcoral.c` 688
**Lines at review (Oct 2026):** 45-47, 847

**Why it's thought to be a bug**

`cohortSpeciesParams` is allocated `[numStages][params]` (atUtilGroupIO.c:829/855/877), but here it is indexed by `cohort` (up to numCohortsXnumGenes-1): `[cohort][mL_id]`, `[cohort][mQ_id]`, `[cohort][mE_id]`, `[cohort][rugosity_dec_id]`. Elsewhere in this file it is correctly `[stage]` (643, 688, 712). Coral groups with more cohorts or genotypes than stages read out of bounds. Functions: `Grow_Coral_Symbionts()`, `Destroy_Rugosity()`.

**Code at issue** — `atcoral.c` line 45  
Search for: `[cohort][mL_id]`

```c
   43: 	double hN_sp, hI_sp, sp_grow, scale_uptake, uptakeNH, uptakeNO, uptakeSi, uptakeFe, uptakeP = 0, uptakeC = 0;
   44: 	double tot_N, host_resp_N_for_symbiont, SP_zooxanth_mort, SP_polyp_mort, SPmortNH_zx, SPmortNH_polyp;
   45→ 	double mL_SP = FunctGroupArray[guild].cohortSpeciesParams[cohort][mL_id] + mL_other;
   46: 	double mQ_SP = FunctGroupArray[guild].cohortSpeciesParams[cohort][mQ_id];
   47: 	double mE_SP = FunctGroupArray[guild].cohortSpeciesParams[cohort][mE_id];
```

**Code at issue** — `atcoral.c` line 848  
Search for: `[cohort][rugosity_dec_id]`

```c
  846:  */
  847: void Destroy_Rugosity(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, FILE *llogfp, int guild, int cohort, int fishery, double dead_biom){
  848→ 	double rate_rug_crumble = bm->FISHERYprms[fishery][rugosity_effect_id] * FunctGroupArray[guild].cohortSpeciesParams[cohort][rugosity_dec_id];
  849: 
  850: 	boxLayerInfo->localWCFlux[Rugosity_i] -= (dead_biom * rate_rug_crumble);
```

**Code at issue** — `atcoral.c` line 688  
Search for: `[stage]`

```c
  686:             switch (bm->flag_rugosity_model) {
  687:                 case mechanistic_rugosity: /* Model details from Blackwood et al 2011 */
  688→                 rate_rug_growth = FunctGroupArray[guild].cohortSpeciesParams[stage][rugosity_inc_id] * 86400.0; // Note that the 86400.0 is used here as the rug_growth_rate is s-1 but growth rate is too so was getting s-2 in effect - TODO: May need some more thought
  689:                 
  690:                 // Rugosity increase due to growth, related to size-based cohort so get greater increase of rugosity due to growth of largest (height wise) coral stages
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
**Found at (current code):** `atcoral.c` 853
**Lines at review (Oct 2026):** 852

**Function:** `Destroy_Rugosity()`

**Why it's thought to be a bug**

Unconditional `fprintf(llogfp, "Destroy_Rugosity: ...")` on every call (every fishery/box/cohort). Function: `Destroy_Rugosity()`.

**Code at issue** — `atcoral.c` line 853  
Search for: `fprintf(llogfp,"Destroy_Rugosity:`

```c
  851:     
  852:     
  853→     fprintf(llogfp,"Destroy_Rugosity: Time: %e box%d-%d Rugosity: %e dead_biom: %e rate_rug_crumble: %e\n", bm->dayt, bm->current_box, bm->current_layer, boxLayerInfo->localWCFlux[Rugosity_i], dead_biom, rate_rug_crumble);
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
**Found at (current code):** `atcoral.c` 488
**Lines at review (Oct 2026):** 925

**Function:** `Apply_Evol_Scalar()`

**Why it's thought to be a bug**

`fprintf(bm->logFile, "DANGER DANGER DANGER ...")` runs on every legitimate maturation-trait update path (when `min_li_mat <= 0`). Function: `Apply_Evol_Scalar()` (atdiversity.c, listed here for completeness).

**Code at issue** — `atcoral.c` line 488 (located by name only; check this is the right place)  
Search for: `fprintf(bm->logFile, "Mdt = %e bleaching_has_occurred: %d\n", FunctGroupArray[guild].speci`

```c
  486: 			fprintf(bm->logFile, "zooxanth_biom = %e polyp_biom = %e sp_biom = %e ", zooxanth_biom, polyp_biom, sp_biom);
  487: 			fprintf(bm->logFile, "timestep_correction = %e ", timestep_correction);
  488→             fprintf(bm->logFile, "Mdt = %e bleaching_has_occurred: %d\n", FunctGroupArray[guild].speciesParams[Mdt_id], bleaching_has_occurred[guild]);
  489: 			//abort();
  490: 		//}
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
