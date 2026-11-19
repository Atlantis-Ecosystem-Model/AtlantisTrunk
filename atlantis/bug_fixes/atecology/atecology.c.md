# atecology/atecology.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **23 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 6 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [ECOL-01](#ecol-01) | 3280, 3330, 3979 | H/M | copy-paste / wrong id constant | `Reset_Arrays(bm, SED, it_count, boxLayerInfo, llogfp);` inside the ice box. Because of this the `[ICE_BASED]`… |  | [ ] |
| [ECOL-02](#ecol-02) | 624, 31, 545 | H/M | unit mismatch (biomass vs numbers) | `SizeNumDiscard[cohort][nf][box] += DependDiscardsTot*dt*cell_vol` and `FCdiscard[guild][cohort] += ...` add m… |  | [ ] |
| [ECOL-03](#ecol-03) | 2297, 2332, 2360 | H/M | accumulator not reset in right loop | `sp_scalar = 1.0` is set once per guild (2310), but for EPIFAUNA groups `sp_scalar *= wcLayerThick;` (2345) ru… |  | [ ] |
| [ECOL-04](#ecol-04) | 2609, 3094 | H/M | copy-paste / wrong habitat index | In the old-diet multi-cohort branch for EPIFAUNA groups: `grazeEaten += preyEaten[ct][WC]; grazeEaten *= local… |  | [ ] |
| [ECOL-05](#ecol-05) | 2671, 2740, 3094 | H/M | copy-paste / wrong tracer array | In the SED branch (old diet, multi-cohort): `grazeEaten *= (localWCTracers[...totNTracers[cohort]] / totalBiom… |  | [ ] |
| [ECOL-06](#ecol-06) | 2619, 2777, 3094 | H/M | swapped indices | `grazeEaten = FunctGroupArray[guild].preyEaten[0][cohort];` (TODO in the code says "check this is really aroun… |  | [ ] |
| [ECOL-07](#ecol-07) | 1127, 31 | H/M | wrong operator (= vs +=) | Layers run top->bottom (atbiology.c:414). The top layer sets `PelDem_ratio_i = PDratio`, but the else branch "… |  | [ ] |
| [ECOL-08](#ecol-08) | 3900, 3602 | M/M | off-by-one / range | `for (kij = 0; kij < EPIFAUNA; kij++)` reconciles NutsProd/NutsLost/DetritusProd/DetritusLost only for WC and … |  | [ ] |
| [ECOL-09](#ecol-09) | 3308, 3312 | M/M | copy-paste | The ice box sets `SP_IRR` for groups with `habitatType == SED` and SED phyto types (copied from Sediment_Box, … |  | [ ] |
| [ECOL-10](#ecol-10) | 2313, 2331, 2346 | M/M | unit mismatch | Dependent-discard reality check: `tot_dis` is the sum of `DependDiscardsTot*dt` (mg N m-3), but for EPIFAUNA `… |  | [ ] |
| [ECOL-11](#ecol-11) | 2949, 3094 | H/L | wrong operator (= vs +=) | ICE_BASED cover biomass: `biomass = initialIceBiomass[...totNTracers[cohort]];` inside the cohort loop keeps o… |  | [ ] |
| [ECOL-12](#ecol-12) | 1324, 1338, 31 | H/L | always-false condition / unreachable case | `if (flux_model_id > 0) { switch ... case WC: ...}`. WC == 0 (atlantisboxmodel.h:390), and Max_Flux_Check retu… |  | [ ] |
| [ECOL-13](#ecol-13) | 1140, 2633, 31 | M/L | cohort vs cohort x genotype | `fished_chrt = (int)speciesParams[Age95pcntV_id]` is an age-class index, but it is used directly to index `dea… |  | [ ] |
| [ECOL-14](#ecol-14) | 1903, 1909, 2622 | M/L | wrong habitat index / wrong cohort | Sediment pseudo-M estimate: detritus uses `preyEaten[0][WC]` in the sediment box (should be SED; WC predation … |  | [ ] |
| [ECOL-15](#ecol-15) | 2910, 2913, 3094 | M/L | cohort index | The epibenthic pseudo-M only uses `mortality[0]` and `preyEaten[0][...]`, so multi-cohort epifauna (e.g. age-s… |  | [ ] |
| [ECOL-16](#ecol-16) | 3835 | M/L | buffer extent mismatch | `for (k = 0; k < numStages; k++) grow[kij][k] = 0.0;`. `grow` is allocated `[numCohortsXnumGenes][RN_id+1]` (a… |  | [ ] |
| [ECOL-17](#ecol-17) | 2415, 3094 | M/L | wrong tracer array | In the Infauna/Epifauna ratio, MB (habitatType SED, see 1499 and comment at 2952) is added as `initialWaterBio… |  | [ ] |
| [ECOL-18](#ecol-18) | 3665 | H/L | copy-paste | The ice box QNAN guard tests `localSEDFlux[NH3_i]` and calls `Call_Diagnostics(..., SED, ...)`, so NaNs in `lo… |  | [ ] |
| [ECOL-19](#ecol-19) | 1349, 31 | H/L | copy-paste | In the flux-imbalance path, `Call_Diagnostics(bm, boxLayerInfo, EPIFAUNA, llogfp, 0)` is called from `Water_Co… |  | [ ] |
| [ECOL-20](#ecol-20) | 1233, 31, 1095 | H/L | ungated debug output (log size) | `fprintf(llogfp, "day: %e box%d-%d doing %s with flux entering...")` runs for every physio-chem tracer, layer … |  | [ ] |
| [ECOL-21](#ecol-21) | 3330 | H/L | accumulator not reset | PREYinfo is reset only for cohort 0, but AGE_STRUCTURED_BIOMASS cohorts use `PREYinfo[guild][cohort][ICE_BASED… |  | [ ] |
| [ECOL-22](#ecol-22) | 2582, 864, 3094 | M/L | wrong operator / habitat index (diagnostics) | The DebugFluxInfo for EPIFAUNA groups is stored under `[guild][WC]`, and `gain/loss = ...` uses `=` inside the… |  | [ ] |
| [ECOL-23](#ecol-23) | 1361, 1341, 31 | H/L | wrong error message | The `quit("... in the Epibenthic_Box calculations?")` is inside `Water_Column_Box()`. |  | [ ] |

---

## ECOL-01

**copy-paste / wrong id constant** · Conf H / Sev M  
**Found at (current code):** `atecology.c` 3280, `atecology.c` 3330, `atecology.c` 3979
**Lines at review (Oct 2026):** 3304

**Function:** `Ice_Box()`

**Why it's thought to be a bug**

`Reset_Arrays(bm, SED, it_count, boxLayerInfo, llogfp);` inside the ice box. Because of this the `[ICE_BASED]` slices of `preyEatenGlobal[cohort][habitat_type][hab]`, `NutsProdGlobal/NutsLostGlobal/DetritusProdGlobal/DetritusLostGlobal[habitat_type]`, `ratio*Global[habitat_type]` and `DebugFluxInfo[which_check][ICE_BASED]` are never zeroed at it_count==1. The SED slices get zeroed instead. `preyEatenGlobal` is accumulated with `+=` (atprocess.c:2464,2468), so `Reconcile_Global_Values(bm,...,ICE_BASED)` (line 3448) adds global values for it_count>1 that grow with every step and every box. Function: `Ice_Box()`.

**Code at issue** — `atecology.c` line 3280  
Search for: `Reset_Arrays(bm, SED, it_count, boxLayerInfo, llogfp)`

```c
 3278: 
 3279: 	/* Initialise diagnostic tracer */
 3280→ 	Reset_Arrays(bm, SED, it_count, boxLayerInfo, llogfp);
 3281: 
 3282: 
```

**Code at issue** — `atecology.c` line 3330  
Search for: `[ICE_BASED]`

```c
 3328: 				if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS) {
 3329: 					for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
 3330→ 						PREYinfo[guild][cohort][ICE_BASED] += boxLayerInfo->localICETracers[FunctGroupArray[guild].totNTracers[cohort]]
 3331: 								* FunctGroupArray[guild].habitatCoeffs[ICE_BASED];
 3332: 					}
```

**Code at issue** — `atecology.c` line 3979  
Search for: `preyEatenGlobal[cohort][habitat_type][hab]`

```c
 3977: 					/* Set the global value to zero on the first iteration in this time step */
 3978: 					if (it_count == 1)
 3979→ 						FunctGroupArray[guild].preyEatenGlobal[cohort][habitat_type][hab] = 0.0;
 3980: 				}
 3981: 			}
```

**Suggested fix (from review):** `Reset_Arrays(bm, ICE_BASED, it_count, boxLayerInfo, llogfp);`

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

## ECOL-02

**unit mismatch (biomass vs numbers)** · Conf H / Sev M  
**Found at (current code):** `atecology.c` 624, `atecology.c` 31, `atecology.c` 545
**Lines at review (Oct 2026):** 625-626, 2368-2369

**Why it's thought to be a bug**

`SizeNumDiscard[cohort][nf][box] += DependDiscardsTot*dt*cell_vol` and `FCdiscard[guild][cohort] += ...` add mg N to arrays that hold numbers at age. The normal path stores `age_discard*dt*cell_vol` with `age_discard = discards / Wgt` (atharvest/atHarvest.c:587,596 -> atprocess.c:2171,2178). Line 2369 also leaves out `*dt*cell_vol`, which is inconsistent with line 626. Only matters when `flagdepend_dis` is on. Functions: `Water_Column_Box()`, `Epibenthic_Box()`.

**Code at issue** — `atecology.c` line 624 (closest match to the quoted code; wording differs slightly)  
Search for: `FunctGroupArray[guild].SizeNumDiscard[cohort][nf][bm->current_box] += (bm->DependDiscardsT`

```c
  622:                                 bm->Discards[bm->current_box][guild][nf] += (bm->DependDiscardsTot[nf][guild][cohort] * bm->dt * bm->cell_vol);
  623:                                 if(FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED){
  624→                                     FunctGroupArray[guild].SizeNumDiscard[cohort][nf][bm->current_box] += (bm->DependDiscardsTot[nf][guild][cohort] * bm->dt * bm->cell_vol);
  625:                                     bm->FCdiscard[guild][cohort] += (bm->DependDiscardsTot[nf][guild][cohort] * bm->dt * bm->cell_vol);
  626:                                 }
```

**Code at issue** — `atecology.c` line 31  
Search for: `Water_Column_Box()`

```c
   29: 
   30:  1/10/2004 Reorganised variable declarations so easier to see what
   31→  variables are associated with what groups in Water_Column_Box(),
   32:  Sediment_Box(), Epibenthic_Box(). And moved all invertbrate
   33:  structural equivalency parameters (XX_sn) to core boxmodel
```

**Code at issue** — `atecology.c` line 545  
Search for: `Epibenthic_Box()`

```c
  543: 
  544: 		/**
  545→ 		 Fisheries and bycatch (other groups are dealt with in Epibenthic_Box().
  546: 		 **/
  547: 		for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
```

**Suggested fix (from review):** Use `DependDiscardsTot[nf][guild][cohort] * bm->dt * sp_scalar` (sp_scalar = cell_vol/Wgt) for both arrays in both boxes

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

## ECOL-03

**accumulator not reset in right loop** · Conf H / Sev M  
**Found at (current code):** `atecology.c` 2297, `atecology.c` 2332, `atecology.c` 2360
**Lines at review (Oct 2026):** 2310-2345

**Function:** `Epibenthic_Box()`

**Why it's thought to be a bug**

`sp_scalar = 1.0` is set once per guild (2310), but for EPIFAUNA groups `sp_scalar *= wcLayerThick;` (2345) runs inside the cohort loop. For multi-cohort epifauna (AGE_STRUCTURED_BIOMASS) the scalar becomes wcLayerThick^n, so `dead[cohort] += DependDiscardsTot*sp_scalar` (2373) is wrong for cohorts after the first one with discards. Function: `Epibenthic_Box()`.

**Code at issue** — `atecology.c` line 2297  
Search for: `sp_scalar = 1.0`

```c
 2295: 				flag_sp = (int)FunctGroupArray[guild].speciesParams[flag_id];
 2296: 				if(flag_sp){
 2297→ 					sp_scalar = 1.0;
 2298: 					for(cohort=0; cohort<FunctGroupArray[guild].numCohortsXnumGenes; cohort++){
 2299: 						/* Determine local biomass for vertebrates (invertebrates assume SC entry = Biomass */
```

**Code at issue** — `atecology.c` line 2332  
Search for: `sp_scalar *= wcLayerThick`

```c
 2330:                                 case EPIFAUNA:
 2331:                                     this_bio = initialEpiBiomass[FunctGroupArray[guild].totNTracers[cohort]];
 2332→                                     sp_scalar *= wcLayerThick; // To get in to mg m-2 from mg-3
 2333:                                     break;
 2334:                                 case LAND_BASED:
```

**Code at issue** — `atecology.c` line 2360 (closest match to the quoted code; wording differs slightly)  
Search for: `FunctGroupArray[guild].dead[cohort] += bm->DependDiscardsTot[nf][guild][cohort] * sp_scala`

```c
 2358:                                 boxLayerInfo->DetritusProd[SED][DCdet_id] += bm->DependDiscardsTot[nf][guild][cohort];
 2359: 
 2360→                                 FunctGroupArray[guild].dead[cohort] += bm->DependDiscardsTot[nf][guild][cohort] * sp_scalar;
 2361:                                 
 2362:                                 // Update Catchsum - moved here to remove an additional loop and so it has the reality check in place
```

**Suggested fix (from review):** Reset `sp_scalar = 1.0;` at the top of the cohort loop

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

## ECOL-04

**copy-paste / wrong habitat index** · Conf H / Sev M  
**Found at (current code):** `atecology.c` 2609, `atecology.c` 3094
**Lines at review (Oct 2026):** 2616-2622

**Function:** `Epibenthic_Box()`

**Why it's thought to be a bug**

In the old-diet multi-cohort branch for EPIFAUNA groups: `grazeEaten += preyEaten[ct][WC]; grazeEaten *= localWCTracers[...totNTracers[cohort]] / totalBiomass;`. But `totalBiomass` is summed from `localEPITracers` (2602) and the single-cohort branch uses `preyEaten[0][EPIFAUNA]` (2624). So predation is taken from the wrong habitat and split by the wrong tracer. Function: `Epibenthic_Box()`.

**Code at issue** — `atecology.c` line 2609  
Search for: `totNTracers[cohort]] / totalBiomass`

```c
 2607: 											grazeEaten += FunctGroupArray[guild].preyEaten[ct][WC];
 2608: 										}
 2609→ 										grazeEaten *= (boxLayerInfo->localWCTracers[FunctGroupArray[guild].totNTracers[cohort]] / totalBiomass);
 2610: 									}else{
 2611: 										grazeEaten = FunctGroupArray[guild].preyEaten[0][EPIFAUNA];
```

**Code at issue** — `atecology.c` line 3094  
Search for: `Epibenthic_Box()`

```c
 3092: 				for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
 3093: 					if (FunctGroupArray[guild].speciesParams[flag_id] && FunctGroupArray[guild].isImpacted == TRUE) {
 3094→ 						/* Skip over those done in Epibenthic_Box() */
 3095: 						/* select the vertebrate, age structured groups and large zooplankton */
 3096: 						if (FunctGroupArray[guild].habitatType == EPIFAUNA || FunctGroupArray[guild].groupType == LG_INF || FunctGroupArray[guild].groupType
```

**Suggested fix (from review):** Use `preyEaten[ct][EPIFAUNA]` and `localEPITracers[...]`

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

## ECOL-05

**copy-paste / wrong tracer array** · Conf H / Sev M  
**Found at (current code):** `atecology.c` 2671, `atecology.c` 2740, `atecology.c` 3094
**Lines at review (Oct 2026):** 2747-2753

**Function:** `Epibenthic_Box()`

**Why it's thought to be a bug**

In the SED branch (old diet, multi-cohort): `grazeEaten *= (localWCTracers[...totNTracers[cohort]] / totalBiomass);` while `totalBiomass` is summed from `localSEDTracers` (2734). Function: `Epibenthic_Box()`.

**Code at issue** — `atecology.c` line 2671 (closest match to the quoted code; wording differs slightly)  
Search for: `grazeEaten *= (boxLayerInfo->localWCTracers[FunctGroupArray[guild].totNTracers[cohort]] / `

```c
 2669: 												grazeEaten += FunctGroupArray[guild].preyEaten[ct][WC];
 2670: 											}
 2671→ 											grazeEaten *= (boxLayerInfo->localWCTracers[FunctGroupArray[guild].totNTracers[cohort]] / totalBiomass);
 2672: 										} else {
 2673: 											grazeEaten = FunctGroupArray[guild].preyEaten[cohort][WC];
```

**Code at issue** — `atecology.c` line 2740  
Search for: `totNTracers[cohort]] / totalBiomass)`

```c
 2738: 												grazeEaten += FunctGroupArray[guild].preyEaten[ct][SED];
 2739: 											}
 2740→ 											grazeEaten *= (boxLayerInfo->localSEDTracers[FunctGroupArray[guild].totNTracers[cohort]] / totalBiomass);
 2741: 										} else {
 2742: 											grazeEaten = FunctGroupArray[guild].preyEaten[cohort][SED];
```

**Code at issue** — `atecology.c` line 3094  
Search for: `Epibenthic_Box()`

```c
 3092: 				for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
 3093: 					if (FunctGroupArray[guild].speciesParams[flag_id] && FunctGroupArray[guild].isImpacted == TRUE) {
 3094→ 						/* Skip over those done in Epibenthic_Box() */
 3095: 						/* select the vertebrate, age structured groups and large zooplankton */
 3096: 						if (FunctGroupArray[guild].habitatType == EPIFAUNA || FunctGroupArray[guild].groupType == LG_INF || FunctGroupArray[guild].groupType
```

**Suggested fix (from review):** Use `boxLayerInfo->localSEDTracers[...]`

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

## ECOL-06

**swapped indices** · Conf H / Sev M  
**Found at (current code):** `atecology.c` 2619, `atecology.c` 2777, `atecology.c` 3094
**Lines at review (Oct 2026):** 2631-2632

**Function:** `Epibenthic_Box()`

**Why it's thought to be a bug**

`grazeEaten = FunctGroupArray[guild].preyEaten[0][cohort];` (TODO in the code says "check this is really around the right way"). `preyEaten` is `Util_Alloc_Init_2D_Long_Double(num_active_habitats, numCohortsXnumGenes)`, i.e. `[cohort][hab]` (atUtilGroupIO.c:820). This indexes habitat by cohort: it reads the wrong value, and for cohort >= num_active_habitats it reads past row 0. Applies to seagrass with `flag_macro_model`. Function: `Epibenthic_Box()`.

**Code at issue** — `atecology.c` line 2619  
Search for: `grazeEaten = FunctGroupArray[guild].preyEaten[0][cohort]`

```c
 2617: 
 2618: 								if(FunctGroupArray[guild].groupType == SEAGRASS && bm->flag_macro_model == TRUE){
 2619→ 									grazeEaten = FunctGroupArray[guild].preyEaten[0][cohort];   // TODO: check this is really around the right way
 2620: 								}
 2621: 								boxLayerInfo->localEPIFlux[FunctGroupArray[guild].totNTracers[cohort]] = FunctGroupArray[guild].growth[cohort]
```

**Code at issue** — `atecology.c` line 2777  
Search for: `[cohort][hab]`

```c
 2775: 						for (hab = WC; hab < bm->num_active_habitats; hab++){
 2776: 							for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
 2777→ 								boxLayerInfo->DebugFluxInfo[guild][WC][loss_id] += (double)FunctGroupArray[guild].preyEaten[cohort][hab];
 2778: 							}
 2779: 						}
```

**Code at issue** — `atecology.c` line 3094  
Search for: `Epibenthic_Box()`

```c
 3092: 				for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
 3093: 					if (FunctGroupArray[guild].speciesParams[flag_id] && FunctGroupArray[guild].isImpacted == TRUE) {
 3094→ 						/* Skip over those done in Epibenthic_Box() */
 3095: 						/* select the vertebrate, age structured groups and large zooplankton */
 3096: 						if (FunctGroupArray[guild].habitatType == EPIFAUNA || FunctGroupArray[guild].groupType == LG_INF || FunctGroupArray[guild].groupType
```

**Suggested fix (from review):** `preyEaten[cohort][EPIFAUNA]` (or whatever habitat the macro model stores into)

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

## ECOL-07

**wrong operator (= vs +=)** · Conf H / Sev M  
**Found at (current code):** `atecology.c` 1127, `atecology.c` 31
**Lines at review (Oct 2026):** 1136-1147

**Function:** `Water_Column_Box()`

**Why it's thought to be a bug**

Layers run top->bottom (atbiology.c:449). The top layer sets `PelDem_ratio_i = PDratio`, but the else branch "increment[s] the global average" with `=`: `localDiagTracers[PelDem_ratio_i] = PDratio / nz;` (same for PiscPlank). So the final indicator is just the bottom layer ratio/nz. The top-layer value is also not divided by nz. Function: `Water_Column_Box()`.

**Code at issue** — `atecology.c` line 1127 (closest match to the quoted code; wording differs slightly)  
Search for: `boxLayerInfo->localDiagTracers[PelDem_ratio_i] = PDratio / bm->boxes[bm->current_box].nz;`

```c
 1125: 		} else {
 1126: 			/* Else increment the global average values */
 1127→ 			boxLayerInfo->localDiagTracers[PelDem_ratio_i] = PDratio / bm->boxes[bm->current_box].nz;
 1128: 			boxLayerInfo->localDiagTracers[PiscPlank_ratio_i] = FVFPratio / bm->boxes[bm->current_box].nz;
 1129: 			if (DivHere > boxLayerInfo->localDiagTracers[DivCount_i])
```

**Code at issue** — `atecology.c` line 31  
Search for: `Water_Column_Box()`

```c
   29: 
   30:  1/10/2004 Reorganised variable declarations so easier to see what
   31→  variables are associated with what groups in Water_Column_Box(),
   32:  Sediment_Box(), Epibenthic_Box(). And moved all invertbrate
   33:  structural equivalency parameters (XX_sn) to core boxmodel
```

**Suggested fix (from review):** Top: `= PDratio / nz`; else: `+= PDratio / nz` (same for FVFPratio)

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

## ECOL-08

**off-by-one / range** · Conf M / Sev M  
**Found at (current code):** `atecology.c` 3900, `atecology.c` 3602
**Lines at review (Oct 2026):** 3923-3930

**Function:** `Reconcile_Global_Values()`

**Why it's thought to be a bug**

`for (kij = 0; kij < EPIFAUNA; kij++)` reconciles NutsProd/NutsLost/DetritusProd/DetritusLost only for WC and SED. When called with `ICE_BASED`, the `[ICE_BASED]` entries used by atNutrient.c:480,486 never get their once-per-dt (Global) contributions for it_count>1, while Reset_Arrays zeroes them on every call (up to num_active_habitats). Function: `Reconcile_Global_Values()`.

**Code at issue** — `atecology.c` line 3900  
Search for: `for (kij = 0; kij < EPIFAUNA; kij++)`

```c
 3898: 		}
 3899: 		for (guild = 0; guild < K_num_nutrients; guild++) {
 3900→ 			for (kij = 0; kij < EPIFAUNA; kij++) {
 3901: 				boxLayerInfo->NutsProd[kij][guild] += boxLayerInfo->NutsProdGlobal[habitatType][kij][guild];
 3902: 				boxLayerInfo->NutsLost[kij][guild] += boxLayerInfo->NutsLostGlobal[habitatType][kij][guild];
```

**Code at issue** — `atecology.c` line 3602  
Search for: `[ICE_BASED]`

```c
 3600: 		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
 3601: 			if(FunctGroupArray[guild].speciesParams[flag_id]){
 3602→ 				if (FunctGroupArray[guild].habitatCoeffs[ICE_BASED] > 0 ){
 3603: 					iceFlux += boxLayerInfo->localICEFlux[FunctGroupArray[guild].totNTracers[0]];
 3604: 				}
```

**Suggested fix (from review):** Loop `kij < bm->num_active_habitats` (or at least include `habitatType`)

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

## ECOL-09

**copy-paste** · Conf M / Sev M  
**Found at (current code):** `atecology.c` 3308, `atecology.c` 3312
**Lines at review (Oct 2026):** 3331-3340

**Function:** `Ice_Box()`

**Why it's thought to be a bug**

The ice box sets `SP_IRR` for groups with `habitatType == SED` and SED phyto types (copied from Sediment_Box, lines 1498-1507), using `localICETracers[Light_Adaptn_TracerID]`. Ice primary producers (ICE_DIATOMS/ICE_MIXOTROPHS) never get `SP_IRR` set here and keep stale values. Function: `Ice_Box()`.

**Code at issue** — `atecology.c` line 3308  
Search for: `habitatType == SED`

```c
 3306: 
 3307: 	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
 3308→ 		if (FunctGroupArray[guild].isVertebrate == FALSE && FunctGroupArray[guild].habitatType == SED && (FunctGroupArray[guild].groupType
 3309: 				== MICROPHTYBENTHOS || FunctGroupArray[guild].groupType == LG_PHY || FunctGroupArray[guild].groupType == DINOFLAG
 3310: 				|| FunctGroupArray[guild].groupType == SM_PHY)) {
```

**Code at issue** — `atecology.c` line 3312 (closest match to the quoted code; wording differs slightly)  
Search for: `FunctGroupArray[guild].SP_IRR = boxLayerInfo->localICETracers[FunctGroupArray[guild].Light`

```c
 3310: 				|| FunctGroupArray[guild].groupType == SM_PHY)) {
 3311: 			if (FunctGroupArray[guild].Light_Adaptn_TracerID > 0)
 3312→ 				FunctGroupArray[guild].SP_IRR = boxLayerInfo->localICETracers[FunctGroupArray[guild].Light_Adaptn_TracerID];
 3313: 			else
 3314: 				FunctGroupArray[guild].SP_IRR = 1.0;
```

**Suggested fix (from review):** Test `habitatType == ICE_BASED` and ice producer group types

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

## ECOL-10

**unit mismatch** · Conf M / Sev M  
**Found at (current code):** `atecology.c` 2313, `atecology.c` 2331, `atecology.c` 2346
**Lines at review (Oct 2026):** 2329-2360

**Function:** `Epibenthic_Box()`

**Why it's thought to be a bug**

Dependent-discard reality check: `tot_dis` is the sum of `DependDiscardsTot*dt` (mg N m-3), but for EPIFAUNA `this_bio = initialEpiBiomass[...]` is mg N m-2. Line 2345 itself notes the m-3 -> m-2 conversion is needed. So `if (tot_dis > this_bio)` compares quantities in different units. Function: `Epibenthic_Box()`.

**Code at issue** — `atecology.c` line 2313 (closest match to the quoted code; wording differs slightly)  
Search for: `tot_dis += bm->DependDiscardsTot[nf][guild][cohort] * bm->dt;`

```c
 2311:                         tot_dis = 0.0;
 2312:                         for(nf=0; nf<bm->K_num_fisheries; nf++){
 2313→                             tot_dis += bm->DependDiscardsTot[nf][guild][cohort] * bm->dt;
 2314:                         }
 2315:                         
```

**Code at issue** — `atecology.c` line 2331  
Search for: `this_bio = initialEpiBiomass[`

```c
 2329:                                     break;
 2330:                                 case EPIFAUNA:
 2331→                                     this_bio = initialEpiBiomass[FunctGroupArray[guild].totNTracers[cohort]];
 2332:                                     sp_scalar *= wcLayerThick; // To get in to mg m-2 from mg-3
 2333:                                     break;
```

**Code at issue** — `atecology.c` line 2346  
Search for: `if (tot_dis > this_bio)`

```c
 2344:                         }
 2345:                         realism_scalar = 1.0;
 2346→                         if (tot_dis > this_bio)
 2347:                             realism_scalar = this_bio / (tot_dis + small_num);
 2348:                         
```

**Suggested fix (from review):** Compare `tot_dis * wcLayerThick` with `this_bio` for EPIFAUNA

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

## ECOL-11

**wrong operator (= vs +=)** · Conf H / Sev L  
**Found at (current code):** `atecology.c` 2949, `atecology.c` 3094
**Lines at review (Oct 2026):** 2974-2977

**Function:** `Epibenthic_Box()`

**Why it's thought to be a bug**

ICE_BASED cover biomass: `biomass = initialIceBiomass[...totNTracers[cohort]];` inside the cohort loop keeps only the last cohort. The EPIFAUNA/LAND cases use `+=`. Function: `Epibenthic_Box()`.

**Code at issue** — `atecology.c` line 2949  
Search for: `biomass = initialIceBiomass[`

```c
 2947: 			case ICE_BASED:
 2948: 				for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
 2949→ 					biomass = initialIceBiomass[FunctGroupArray[guild].totNTracers[cohort]];
 2950: 				}
 2951: 				break;
```

**Code at issue** — `atecology.c` line 3094  
Search for: `Epibenthic_Box()`

```c
 3092: 				for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
 3093: 					if (FunctGroupArray[guild].speciesParams[flag_id] && FunctGroupArray[guild].isImpacted == TRUE) {
 3094→ 						/* Skip over those done in Epibenthic_Box() */
 3095: 						/* select the vertebrate, age structured groups and large zooplankton */
 3096: 						if (FunctGroupArray[guild].habitatType == EPIFAUNA || FunctGroupArray[guild].groupType == LG_INF || FunctGroupArray[guild].groupType
```

**Suggested fix (from review):** `biomass += ...`

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

## ECOL-12

**always-false condition / unreachable case** · Conf H / Sev L  
**Found at (current code):** `atecology.c` 1324, `atecology.c` 1338, `atecology.c` 31
**Lines at review (Oct 2026):** 1340, 3164 (also 1985, 3642)

**Why it's thought to be a bug**

`if (flux_model_id > 0) { switch ... case WC: ...}`. WC == 0 (atlantisboxmodel.h:390), and Max_Flux_Check returns `fluxnum = WC` for water-column culprits, so the WC diagnostic branch can never run and "Can't give details" is printed instead. Note: if fixed to `>= 0`, nutrient culprits set `flux_sp = bm->K_num_tot_sp` (atfluxbreakdown.c:117), and `FunctGroupArray[flux_sp].groupCode` would then read out of bounds. Functions: `Water_Column_Box()`, `Epibenthic_Box()`.

**Code at issue** — `atecology.c` line 1324  
Search for: `if (flux_model_id > 0) {`

```c
 1322: 					boxLayerInfo->localEPIFlux, llogfp);
 1323: 
 1324→ 			if (flux_model_id > 0) {
 1325: 				switch (flux_model_id) {
 1326: 				case WC:
```

**Code at issue** — `atecology.c` line 1338  
Search for: `FunctGroupArray[flux_sp].groupCode`

```c
 1336: 					break;
 1337: 				case death_id:
 1338→ 					fprintf(llogfp, "Water_Column_Box: biggest flux is due to the death of %s-%d\n", FunctGroupArray[flux_sp].groupCode, fluxnum);
 1339: 					break;
 1340: 				default:
```

**Code at issue** — `atecology.c` line 31  
Search for: `Water_Column_Box()`

```c
   29: 
   30:  1/10/2004 Reorganised variable declarations so easier to see what
   31→  variables are associated with what groups in Water_Column_Box(),
   32:  Sediment_Box(), Epibenthic_Box(). And moved all invertbrate
   33:  structural equivalency parameters (XX_sn) to core boxmodel
```

**Suggested fix (from review):** Use `>= 0` and guard `flux_sp < bm->K_num_tot_sp` before printing groupCode

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

## ECOL-13

**cohort vs cohort x genotype** · Conf M / Sev L  
**Found at (current code):** `atecology.c` 1140, `atecology.c` 2633, `atecology.c` 31
**Lines at review (Oct 2026):** 1156-1169

**Function:** `Water_Column_Box()`

**Why it's thought to be a bug**

`fished_chrt = (int)speciesParams[Age95pcntV_id]` is an age-class index, but it is used directly to index `dead[]`/`preyEaten[]`, which are sized numCohortsXnumGenes, and is compared with `numCohortsXnumGenes/2`. Elsewhere it is converted (atassess/atclassical.c:860 `nc = va95 * numGeneTypes`). With numGeneTypes>1 this picks the wrong cohort for pseudo-F/M. There is no upper bound check either. Function: `Water_Column_Box()`.

**Code at issue** — `atecology.c` line 1140 (closest match to the quoted code; wording differs slightly)  
Search for: `fished_chrt = (int) (FunctGroupArray[guild].speciesParams[Age95pcntV_id]);`

```c
 1138: 			if (FunctGroupArray[guild].isVertebrate == TRUE) {
 1139: 				/* For use in pseudo F estimate */
 1140→ 				fished_chrt = (int) (FunctGroupArray[guild].speciesParams[Age95pcntV_id]);
 1141: 				if (fished_chrt < ((FunctGroupArray[guild].numCohortsXnumGenes) / 2))
 1142: 					fished_chrt = ((FunctGroupArray[guild].numCohortsXnumGenes)/ 2);
```

**Code at issue** — `atecology.c` line 2633  
Search for: `dead[]`

```c
 2631: 								/**
 2632: 								if(bm->which_check == guild){
 2633→ 									fprintf(llogfp,"day: %f box%d-%d, %s-%d epiflux: %.20e grow: %.20e mort: %.20e dead: %.20e eaten: %Le preyEaten: %Le dead[]: %e, deadGlobal[]: %e\n",
 2634: 										bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, cohort,
 2635: 										boxLayerInfo->localEPIFlux[FunctGroupArray[guild].totNTracers[cohort]],
```

**Code at issue** — `atecology.c` line 31  
Search for: `Water_Column_Box()`

```c
   29: 
   30:  1/10/2004 Reorganised variable declarations so easier to see what
   31→  variables are associated with what groups in Water_Column_Box(),
   32:  Sediment_Box(), Epibenthic_Box(). And moved all invertbrate
   33:  structural equivalency parameters (XX_sn) to core boxmodel
```

**Suggested fix (from review):** `fished_chrt = Age95pcntV * numGeneTypes`; clamp to `< numCohortsXnumGenes`

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

## ECOL-14

**wrong habitat index / wrong cohort** · Conf M / Sev L  
**Found at (current code):** `atecology.c` 1903, `atecology.c` 1909, `atecology.c` 2622
**Lines at review (Oct 2026):** 1914-1923

**Function:** `Sediment_Box()`

**Why it's thought to be a bug**

Sediment pseudo-M estimate: detritus uses `preyEaten[0][WC]` in the sediment box (should be SED; WC predation is already counted in Water_Column_Box at 1178-1180). The non-detritus loop adds `mortality[fished_chrt]` (=cohort 0) once per cohort instead of `mortality[cohort]`. Function: `Sediment_Box()`.

**Code at issue** — `atecology.c` line 1903  
Search for: `preyEaten[0][WC]`

```c
 1901: 				if (FunctGroupArray[guild].isDetritus == TRUE) {
 1902: 					bm->calcMnum[guild][current_id] +=
 1903→ 							(FunctGroupArray[guild].mortality[fished_chrt] + (double)FunctGroupArray[guild].preyEaten[0][WC]);
 1904: 				} else {
 1905: 					hab = SED;
```

**Code at issue** — `atecology.c` line 1909  
Search for: `mortality[fished_chrt]`

```c
 1907: 					for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
 1908: 						bm->calcMnum[guild][current_id] +=
 1909→ 							(FunctGroupArray[guild].mortality[fished_chrt]  + (double)FunctGroupArray[guild].preyEaten[cohort][hab]);
 1910: 					}
 1911: 				}
```

**Code at issue** — `atecology.c` line 2622  
Search for: `mortality[cohort]`

```c
 2620: 								}
 2621: 								boxLayerInfo->localEPIFlux[FunctGroupArray[guild].totNTracers[cohort]] = FunctGroupArray[guild].growth[cohort]
 2622→ 										- FunctGroupArray[guild].mortality[cohort] - dead - (double)grazeEaten;
 2623: 
 2624: 								if(bm->track_atomic_ratio == TRUE){
```

**Suggested fix (from review):** Use `preyEaten[0][SED]` and `mortality[cohort]`

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

## ECOL-15

**cohort index** · Conf M / Sev L  
**Found at (current code):** `atecology.c` 2910, `atecology.c` 2913, `atecology.c` 3094
**Lines at review (Oct 2026):** 2937-2940

**Function:** `Epibenthic_Box()`

**Why it's thought to be a bug**

The epibenthic pseudo-M only uses `mortality[0]` and `preyEaten[0][...]`, so multi-cohort epifauna (e.g. age-structured seagrass) lose all cohorts except the first. Function: `Epibenthic_Box()`.

**Code at issue** — `atecology.c` line 2910  
Search for: `mortality[0]`

```c
 2908: 			} else {
 2909: 				/* For use in pseudo M estimate */
 2910→ 				bm->calcMnum[guild][current_id] += FunctGroupArray[guild].mortality[0]
 2911: 					+ ((FunctGroupArray[guild].habitatType == EPIFAUNA) * (double)FunctGroupArray[guild].preyEaten[0][WC] )
 2912: 					+ ((FunctGroupArray[guild].habitatType == EPIFAUNA) * (double)FunctGroupArray[guild].preyEaten[0][SED] )
```

**Code at issue** — `atecology.c` line 2913  
Search for: `preyEaten[0][`

```c
 2911: 					+ ((FunctGroupArray[guild].habitatType == EPIFAUNA) * (double)FunctGroupArray[guild].preyEaten[0][WC] )
 2912: 					+ ((FunctGroupArray[guild].habitatType == EPIFAUNA) * (double)FunctGroupArray[guild].preyEaten[0][SED] )
 2913→ 					+ ((FunctGroupArray[guild].habitatType == EPIFAUNA) * (double)FunctGroupArray[guild].preyEaten[0][EPIFAUNA] );
 2914: 
 2915: 			}
```

**Code at issue** — `atecology.c` line 3094  
Search for: `Epibenthic_Box()`

```c
 3092: 				for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
 3093: 					if (FunctGroupArray[guild].speciesParams[flag_id] && FunctGroupArray[guild].isImpacted == TRUE) {
 3094→ 						/* Skip over those done in Epibenthic_Box() */
 3095: 						/* select the vertebrate, age structured groups and large zooplankton */
 3096: 						if (FunctGroupArray[guild].habitatType == EPIFAUNA || FunctGroupArray[guild].groupType == LG_INF || FunctGroupArray[guild].groupType
```

**Suggested fix (from review):** Loop over cohorts

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

## ECOL-16

**buffer extent mismatch** · Conf M / Sev L  
**Found at (current code):** `atecology.c` 3835
**Lines at review (Oct 2026):** 3985-3988

**Function:** `Reset_Arrays()`

**Why it's thought to be a bug**

`for (k = 0; k < numStages; k++) grow[kij][k] = 0.0;`. `grow` is allocated `[numCohortsXnumGenes][RN_id+1]` (atUtilGroupIO.c:747), and the index is SN_id/RN_id, not life stage. If numStages==1, `grow[][RN_id]` is not reset. If numStages>2, the loop writes past the row. Function: `Reset_Arrays()`.

**Code at issue** — `atecology.c` line 3835 (closest match to the quoted code; wording differs slightly)  
Search for: `FunctGroupArray[guild].grow[cohort][SN_id], FunctGroupArray[guild].grow[cohort][RN_id]);`

```c
 3833: 									(FunctGroupArray[guild].dead[cohort] + FunctGroupArray[guild].preyEaten[cohort][WC]),
 3834: 									FunctGroupArray[guild].dead[cohort], FunctGroupArray[guild].preyEaten[cohort][WC],
 3835→ 									FunctGroupArray[guild].grow[cohort][SN_id], FunctGroupArray[guild].grow[cohort][RN_id]);
 3836:                             fflush(llogfp);
 3837: 						 }
```

**Suggested fix (from review):** Loop `k <= RN_id`

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

## ECOL-17

**wrong tracer array** · Conf M / Sev L  
**Found at (current code):** `atecology.c` 2415, `atecology.c` 3094
**Lines at review (Oct 2026):** 2424-2430

**Function:** `Epibenthic_Box()`

**Why it's thought to be a bug**

In the Infauna/Epifauna ratio, MB (habitatType SED, see 1499 and comment at 2952) is added as `initialWaterBiomass[...] * smLayerThick`: a water-column concentration multiplied by sediment thickness. It should be the sediment biomass. Function: `Epibenthic_Box()`.

**Code at issue** — `atecology.c` line 2415  
Search for: `initialWaterBiomass[`

```c
 2413: 					} else if (FunctGroupArray[guild].habitatCoeffs[EPIFAUNA] > 0) {
 2414: 						if (FunctGroupArray[guild].groupType == MICROPHTYBENTHOS)
 2415→ 							Epifauna += initialWaterBiomass[FunctGroupArray[guild].totNTracers[cohort]] * smLayerThick;
 2416: 						else
 2417: 							Epifauna += initialEpiBiomass[FunctGroupArray[guild].totNTracers[cohort]];
```

**Code at issue** — `atecology.c` line 3094  
Search for: `Epibenthic_Box()`

```c
 3092: 				for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
 3093: 					if (FunctGroupArray[guild].speciesParams[flag_id] && FunctGroupArray[guild].isImpacted == TRUE) {
 3094→ 						/* Skip over those done in Epibenthic_Box() */
 3095: 						/* select the vertebrate, age structured groups and large zooplankton */
 3096: 						if (FunctGroupArray[guild].habitatType == EPIFAUNA || FunctGroupArray[guild].groupType == LG_INF || FunctGroupArray[guild].groupType
```

**Suggested fix (from review):** `initialSedBiomass[...] * smLayerThick`

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

## ECOL-18

**copy-paste** · Conf H / Sev L  
**Found at (current code):** `atecology.c` 3665
**Lines at review (Oct 2026):** 3689-3691

**Function:** `Ice_Box()`

**Why it's thought to be a bug**

The ice box QNAN guard tests `localSEDFlux[NH3_i]` and calls `Call_Diagnostics(..., SED, ...)`, so NaNs in `localICEFlux` are never caught. Function: `Ice_Box()`.

**Code at issue** — `atecology.c` line 3665  
Search for: `localSEDFlux[NH3_i]`

```c
 3663: 		}
 3664: 	}
 3665→ 	if (!(_finite(boxLayerInfo->localSEDFlux[NH3_i]))) {
 3666: 		/* Check diagnostic terms */
 3667: 		Call_Diagnostics(bm, boxLayerInfo, SED, llogfp, 1);
```

**Suggested fix (from review):** Test `localICEFlux[NH3_i]`, use ICE_BASED

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

## ECOL-19

**copy-paste** · Conf H / Sev L  
**Found at (current code):** `atecology.c` 1349, `atecology.c` 31
**Lines at review (Oct 2026):** 1364-1365, 2005-2006

**Why it's thought to be a bug**

In the flux-imbalance path, `Call_Diagnostics(bm, boxLayerInfo, EPIFAUNA, llogfp, 0)` is called from `Water_Column_Box()` and `Sediment_Box()` (should be WC / SED). The guard also uses `(current_box == checkbox) || (dayt >= checkstart)`, whereas the normal call (1249/1955) uses `&&`.

**Code at issue** — `atecology.c` line 1349  
Search for: `Call_Diagnostics(bm, boxLayerInfo, EPIFAUNA, llogfp, 0)`

```c
 1347: 			/* Check diagnostic terms */
 1348: 			if ((bm->checkNH || bm->checkDL || bm->checkDR) && ((bm->current_box == bm->checkbox) || (bm->dayt >= bm->checkstart)))
 1349→ 				Call_Diagnostics(bm, boxLayerInfo, EPIFAUNA, llogfp, 0);
 1350: 
 1351: 			/* List all figures */
```

**Code at issue** — `atecology.c` line 31  
Search for: `Water_Column_Box()`

```c
   29: 
   30:  1/10/2004 Reorganised variable declarations so easier to see what
   31→  variables are associated with what groups in Water_Column_Box(),
   32:  Sediment_Box(), Epibenthic_Box(). And moved all invertbrate
   33:  structural equivalency parameters (XX_sn) to core boxmodel
```

**Suggested fix (from review):** Pass WC / SED; use `&&`

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

## ECOL-20

**ungated debug output (log size)** · Conf H / Sev L  
**Found at (current code):** `atecology.c` 1233, `atecology.c` 31, `atecology.c` 1095
**Lines at review (Oct 2026):** 1077-1098, 2806-2831

**Why it's thought to be a bug**

`fprintf(llogfp, "day: %e box%d-%d doing %s with flux entering...")` runs for every physio-chem tracer, layer and adaptive substep whenever `dayt >= checkstart && checkbox == current_box`, with no `bm->debug`/verbose gate. This can produce very large logs, similar to the printf removed in commit f56b3e7. Functions: `Water_Column_Box()`, `Epibenthic_Box()`.

**Code at issue** — `atecology.c` line 1233 (closest match to the quoted code; wording differs slightly)  
Search for: `if ((bm->checkNH || bm->checkDL || bm->checkDR) && ((bm->dayt >= bm->checkstart) && ((bm->`

```c
 1231: 	}
 1232:     
 1233→ 	if ((bm->checkNH || bm->checkDL || bm->checkDR) && ((bm->dayt >= bm->checkstart) && ((bm->checkbox > bm->nbox) || (bm->current_box == bm->checkbox))))
 1234: 		Call_Diagnostics(bm, boxLayerInfo, WC, llogfp, 0);
 1235: 
```

**Code at issue** — `atecology.c` line 31  
Search for: `Water_Column_Box()`

```c
   29: 
   30:  1/10/2004 Reorganised variable declarations so easier to see what
   31→  variables are associated with what groups in Water_Column_Box(),
   32:  Sediment_Box(), Epibenthic_Box(). And moved all invertbrate
   33:  structural equivalency parameters (XX_sn) to core boxmodel
```

**Code at issue** — `atecology.c` line 1095  
Search for: `Epibenthic_Box()`

```c
 1093:         /**
 1094: 		 Vertebrates - if in lowest watercolumn layer than this will all actually happen
 1095→ 		 in Epibenthic_Box()
 1096: 		 **/
 1097:         Update_Vertebrate_Tracers(bm, llogfp, WC, boxLayerInfo);
```

**Suggested fix (from review):** Gate on a debug flag (e.g. `bm->debug == debug_biology_process`)

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

## ECOL-21

**accumulator not reset** · Conf H / Sev L  
**Found at (current code):** `atecology.c` 3330
**Lines at review (Oct 2026):** 3348-3356

**Function:** `Ice_Box()`

**Why it's thought to be a bug**

PREYinfo is reset only for cohort 0, but AGE_STRUCTURED_BIOMASS cohorts use `PREYinfo[guild][cohort][ICE_BASED] += ...`. Latent for now: such groups with an ice coefficient >0 quit at 3430, and with a coefficient of 0 the term adds 0. Function: `Ice_Box()`.

**Code at issue** — `atecology.c` line 3330  
Search for: `PREYinfo[guild][cohort][ICE_BASED] +=`

```c
 3328: 				if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS) {
 3329: 					for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
 3330→ 						PREYinfo[guild][cohort][ICE_BASED] += boxLayerInfo->localICETracers[FunctGroupArray[guild].totNTracers[cohort]]
 3331: 								* FunctGroupArray[guild].habitatCoeffs[ICE_BASED];
 3332: 					}
```

**Suggested fix (from review):** Use `=` (as in Sediment_Box line 1519)

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

## ECOL-22

**wrong operator / habitat index (diagnostics)** · Conf M / Sev L  
**Found at (current code):** `atecology.c` 2582, `atecology.c` 864, `atecology.c` 3094
**Lines at review (Oct 2026):** 2594-2655, 3723-3725

**Why it's thought to be a bug**

The DebugFluxInfo for EPIFAUNA groups is stored under `[guild][WC]`, and `gain/loss = ...` uses `=` inside the cohort loop, so only the last cohort is kept (the WC/SED branches at 2719-2720 use `+=`). The same `=`-in-cohort-loop pattern appears in `Update_Vertebrate_Tracers()` 3723-3725. This only affects DiagNGain/Loss tracking. Functions: `Epibenthic_Box()`, `Update_Vertebrate_Tracers()`.

**Code at issue** — `atecology.c` line 2582  
Search for: `[guild][WC]`

```c
 2580: 
 2581: 						boxLayerInfo->DebugFluxInfo[guild][WC][loss_id] = 0;
 2582→ 						boxLayerInfo->DebugFluxInfo[guild][WC][gain_id] = 0;
 2583: 
 2584: 						if (FunctGroupArray[guild].habitatType == EPIFAUNA) {
```

**Code at issue** — `atecology.c` line 864  
Search for: `Update_Vertebrate_Tracers()`

```c
  862: 			case AGE_STRUCTURED:
  863: 
  864→ 				/* the rest of the vertebrate tracers are updated in Update_Vertebrate_Tracers()*/
  865: 				/* For bottom water column layer save this info in Epibenthic_Box() */
  866: 				if (it_count == 1) {
```

**Code at issue** — `atecology.c` line 3094  
Search for: `Epibenthic_Box()`

```c
 3092: 				for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
 3093: 					if (FunctGroupArray[guild].speciesParams[flag_id] && FunctGroupArray[guild].isImpacted == TRUE) {
 3094→ 						/* Skip over those done in Epibenthic_Box() */
 3095: 						/* select the vertebrate, age structured groups and large zooplankton */
 3096: 						if (FunctGroupArray[guild].habitatType == EPIFAUNA || FunctGroupArray[guild].groupType == LG_INF || FunctGroupArray[guild].groupType
```

**Suggested fix (from review):** Use `+=` after zeroing; index by EPIFAUNA

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

## ECOL-23

**wrong error message** · Conf H / Sev L  
**Found at (current code):** `atecology.c` 1361, `atecology.c` 1341, `atecology.c` 31
**Lines at review (Oct 2026):** 1357

**Why it's thought to be a bug**

The `quit("... in the Epibenthic_Box calculations?")` is inside `Water_Column_Box()`.

**Code at issue** — `atecology.c` line 1361  
Search for: `quit("`

```c
 1359: 
 1360: 			Textfile_Dump(bm, llogfp);
 1361→ 			quit("Error on flux balance in water column cell on day: %e, in box: %d, layer: %d, wcFlux: %e\n", bm->dayt, bm->current_box, bm->current_layer,
 1362: 					wcFlux);
 1363: 
```

**Code at issue** — `atecology.c` line 1341 (closest match to the quoted code; wording differs slightly)  
Search for: `quit("No such flux_model defined - how did it get here in the Epibenthic_Box calculations?`

```c
 1339: 					break;
 1340: 				default:
 1341→ 					quit("No such flux_model defined - how did it get here in the Epibenthic_Box calculations?\n");
 1342: 					break;
 1343: 				}
```

**Code at issue** — `atecology.c` line 31  
Search for: `Water_Column_Box()`

```c
   29: 
   30:  1/10/2004 Reorganised variable declarations so easier to see what
   31→  variables are associated with what groups in Water_Column_Box(),
   32:  Sediment_Box(), Epibenthic_Box(). And moved all invertbrate
   33:  structural equivalency parameters (XX_sn) to core boxmodel
```

**Suggested fix (from review):** Say Water_Column_Box

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

`Water_Column_Box()` 604-617: the dependent-discard reality check uses `initialBiomass` (WC tracers) as `this_bio` for every non-EPIFAUNA, non-vertebrate group, including SED groups. For a SED group this is ~0, so `realism_scalar` goes to ~0 and dependent discards of sediment groups caused by WC fleets vanish. `Epibenthic_Box()` then re-zeroes DependDiscardsTot for non-EPIFAUNA groups (2238), so those discards are never applied anywhere. Worth checking whether SED groups should be handled in Epibenthic instead.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

`Epibenthic_Box()` 2371: dependent discards go to `DetritusProd[SED][DCdet_id]`, while the WC box uses `[WC]` (628). DependDiscardsTot is per m3 of water, so adding it to sediment carrion production may need a wcLayerThick/smLayerThick conversion.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-3

`Epibenthic_Box()` 3009-3012: DiagN gain/loss is added to `localWCTracers` for any `habitat_check` with no `habitat_check` test. When habitat_check==SED it goes to the WC tracer, and for WC it may double count what the nested Water_Column_Box call already stored.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-4

`Sediment_Box()` 1781-1782: the debug loss term leaves out lysis and dead, which are included in the WC version (1008-1009). It also adds `linkageSEDDetritusFlux` to `loss_id` instead of `gain_id` (1805), but that branch is unreachable because detritus is skipped at 1723 (same unreachable detritus branch at 1030-1037).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-5

`Ice_Box()` 3519-3528: broker linkage mortality is applied to `localSEDFlux` rather than `localICEFlux` (only under BROKER_LINK_ENABLED).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-6

`Epibenthic_Box()` 2478-2480: `DIN = NO + NH + small_num` means the `if (!DIN)` reset can never fire (harmless).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
