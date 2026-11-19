# atecology/atecology.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **23 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 6 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [ECOL-01](#ecol-01) | 3304 | H/M | copy-paste / wrong id constant | `Reset_Arrays(bm, SED, it_count, boxLayerInfo, llogfp);` inside the ice box. Because of this the `[ICE_BASED]`… |  | [ ] |
| [ECOL-02](#ecol-02) | 625-626, 2368-2369 | H/M | unit mismatch (biomass vs numbers) | `SizeNumDiscard[cohort][nf][box] += DependDiscardsTot*dt*cell_vol` and `FCdiscard[guild][cohort] += ...` add m… |  | [ ] |
| [ECOL-03](#ecol-03) | 2310-2345 | H/M | accumulator not reset in right loop | `sp_scalar = 1.0` is set once per guild (2310), but for EPIFAUNA groups `sp_scalar *= wcLayerThick;` (2345) ru… |  | [ ] |
| [ECOL-04](#ecol-04) | 2616-2622 | H/M | copy-paste / wrong habitat index | In the old-diet multi-cohort branch for EPIFAUNA groups: `grazeEaten += preyEaten[ct][WC]; grazeEaten *= local… |  | [ ] |
| [ECOL-05](#ecol-05) | 2747-2753 | H/M | copy-paste / wrong tracer array | In the SED branch (old diet, multi-cohort): `grazeEaten *= (localWCTracers[...totNTracers[cohort]] / totalBiom… |  | [ ] |
| [ECOL-06](#ecol-06) | 2631-2632 | H/M | swapped indices | `grazeEaten = FunctGroupArray[guild].preyEaten[0][cohort];` (TODO in the code says "check this is really aroun… |  | [ ] |
| [ECOL-07](#ecol-07) | 1136-1147 | H/M | wrong operator (= vs +=) | Layers run top->bottom (atbiology.c:414). The top layer sets `PelDem_ratio_i = PDratio`, but the else branch "… |  | [ ] |
| [ECOL-08](#ecol-08) | 3923-3930 | M/M | off-by-one / range | `for (kij = 0; kij < EPIFAUNA; kij++)` reconciles NutsProd/NutsLost/DetritusProd/DetritusLost only for WC and … |  | [ ] |
| [ECOL-09](#ecol-09) | 3331-3340 | M/M | copy-paste | The ice box sets `SP_IRR` for groups with `habitatType == SED` and SED phyto types (copied from Sediment_Box, … |  | [ ] |
| [ECOL-10](#ecol-10) | 2329-2360 | M/M | unit mismatch | Dependent-discard reality check: `tot_dis` is the sum of `DependDiscardsTot*dt` (mg N m-3), but for EPIFAUNA `… |  | [ ] |
| [ECOL-11](#ecol-11) | 2974-2977 | H/L | wrong operator (= vs +=) | ICE_BASED cover biomass: `biomass = initialIceBiomass[...totNTracers[cohort]];` inside the cohort loop keeps o… |  | [ ] |
| [ECOL-12](#ecol-12) | 1340, 3164 (also 1985, 3642) | H/L | always-false condition / unreachable case | `if (flux_model_id > 0) { switch ... case WC: ...}`. WC == 0 (atlantisboxmodel.h:390), and Max_Flux_Check retu… |  | [ ] |
| [ECOL-13](#ecol-13) | 1156-1169 | M/L | cohort vs cohort x genotype | `fished_chrt = (int)speciesParams[Age95pcntV_id]` is an age-class index, but it is used directly to index `dea… |  | [ ] |
| [ECOL-14](#ecol-14) | 1914-1923 | M/L | wrong habitat index / wrong cohort | Sediment pseudo-M estimate: detritus uses `preyEaten[0][WC]` in the sediment box (should be SED; WC predation … |  | [ ] |
| [ECOL-15](#ecol-15) | 2937-2940 | M/L | cohort index | The epibenthic pseudo-M only uses `mortality[0]` and `preyEaten[0][...]`, so multi-cohort epifauna (e.g. age-s… |  | [ ] |
| [ECOL-16](#ecol-16) | 3985-3988 | M/L | buffer extent mismatch | `for (k = 0; k < numStages; k++) grow[kij][k] = 0.0;`. `grow` is allocated `[numCohortsXnumGenes][RN_id+1]` (a… |  | [ ] |
| [ECOL-17](#ecol-17) | 2424-2430 | M/L | wrong tracer array | In the Infauna/Epifauna ratio, MB (habitatType SED, see 1499 and comment at 2952) is added as `initialWaterBio… |  | [ ] |
| [ECOL-18](#ecol-18) | 3689-3691 | H/L | copy-paste | The ice box QNAN guard tests `localSEDFlux[NH3_i]` and calls `Call_Diagnostics(..., SED, ...)`, so NaNs in `lo… |  | [ ] |
| [ECOL-19](#ecol-19) | 1364-1365, 2005-2006 | H/L | copy-paste | In the flux-imbalance path, `Call_Diagnostics(bm, boxLayerInfo, EPIFAUNA, llogfp, 0)` is called from `Water_Co… |  | [ ] |
| [ECOL-20](#ecol-20) | 1077-1098, 2806-2831 | H/L | ungated debug output (log size) | `fprintf(llogfp, "day: %e box%d-%d doing %s with flux entering...")` runs for every physio-chem tracer, layer … |  | [ ] |
| [ECOL-21](#ecol-21) | 3348-3356 | H/L | accumulator not reset | PREYinfo is reset only for cohort 0, but AGE_STRUCTURED_BIOMASS cohorts use `PREYinfo[guild][cohort][ICE_BASED… |  | [ ] |
| [ECOL-22](#ecol-22) | 2594-2655, 3723-3725 | M/L | wrong operator / habitat index (diagnostics) | The DebugFluxInfo for EPIFAUNA groups is stored under `[guild][WC]`, and `gain/loss = ...` uses `=` inside the… |  | [ ] |
| [ECOL-23](#ecol-23) | 1357 | H/L | wrong error message | The `quit("... in the Epibenthic_Box calculations?")` is inside `Water_Column_Box()`. |  | [ ] |

---

## ECOL-01

**copy-paste / wrong id constant** · Conf H / Sev M  
**Listed lines:** 3304

**Function:** `Ice_Box()`

**Why it's thought to be a bug**

`Reset_Arrays(bm, SED, it_count, boxLayerInfo, llogfp);` inside the ice box. Because of this the `[ICE_BASED]` slices of `preyEatenGlobal[cohort][habitat_type][hab]`, `NutsProdGlobal/NutsLostGlobal/DetritusProdGlobal/DetritusLostGlobal[habitat_type]`, `ratio*Global[habitat_type]` and `DebugFluxInfo[which_check][ICE_BASED]` are never zeroed at it_count==1. The SED slices get zeroed instead. `preyEatenGlobal` is accumulated with `+=` (atprocess.c:2304,2308), so `Reconcile_Global_Values(bm,...,ICE_BASED)` (line 3448) adds global values for it_count>1 that grow with every step and every box. Function: `Ice_Box()`.

**Current code** (`atecology/atecology.c` lines 3302–3306)

```c
 3302: 
 3303: 	//DON = boxLayerInfo->localICETracers[DON_i];
 3304: 	NH = boxLayerInfo->localICETracers[NH3_i];
 3305: 	NO = boxLayerInfo->localICETracers[NO3_i];
 3306: 
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
**Listed lines:** 625-626, 2368-2369

**Why it's thought to be a bug**

`SizeNumDiscard[cohort][nf][box] += DependDiscardsTot*dt*cell_vol` and `FCdiscard[guild][cohort] += ...` add mg N to arrays that hold numbers at age. The normal path stores `age_discard*dt*cell_vol` with `age_discard = discards / Wgt` (atharvest/atHarvest.c:587,596 -> atprocess.c:2008,2015). Line 2369 also leaves out `*dt*cell_vol`, which is inconsistent with line 626. Only matters when `flagdepend_dis` is on. Functions: `Water_Column_Box()`, `Epibenthic_Box()`.

**Current code** (`atecology/atecology.c` lines 623–628)

```c
  623:                                 if(FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED){
  624:                                     FunctGroupArray[guild].SizeNumDiscard[cohort][nf][bm->current_box] += (bm->DependDiscardsTot[nf][guild][cohort] * bm->dt * bm->cell_vol);
  625:                                     bm->FCdiscard[guild][cohort] += (bm->DependDiscardsTot[nf][guild][cohort] * bm->dt * bm->cell_vol);
  626:                                 }
  627:                                 boxLayerInfo->DetritusProd[WC][DCdet_id] += bm->DependDiscardsTot[nf][guild][cohort];
  628: 
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
**Listed lines:** 2310-2345

**Function:** `Epibenthic_Box()`

**Why it's thought to be a bug**

`sp_scalar = 1.0` is set once per guild (2310), but for EPIFAUNA groups `sp_scalar *= wcLayerThick;` (2345) runs inside the cohort loop. For multi-cohort epifauna (AGE_STRUCTURED_BIOMASS) the scalar becomes wcLayerThick^n, so `dead[cohort] += DependDiscardsTot*sp_scalar` (2373) is wrong for cohorts after the first one with discards. Function: `Epibenthic_Box()`.

**Current code** (`atecology/atecology.c` lines 2311–2350) — code appears to have moved +3 lines since the review

```c
 2311:                         tot_dis = 0.0;
 2312:                         for(nf=0; nf<bm->K_num_fisheries; nf++){
 2313:                             tot_dis += bm->DependDiscardsTot[nf][guild][cohort] * bm->dt;
 2314:                         }
 2315:                         
 2316:                         if(!tot_dis)
 2317:                             continue;
 2318:                         
 2319:                         // See if taking more biomass than present and fix accordingly
 2320:                         if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED){
 2321:                             this_bio = Wgt * VERTinfo[guild][cohort][DEN_id];
 2322:                         } else {
 2323:                             switch (FunctGroupArray[guild].habitatType) {
 2324:                                 case WC:
 2325:                                     this_bio = initialWaterBiomass[FunctGroupArray[guild].totNTracers[cohort]];
 2326:                                     break;
 2327:                                 case SED:
 2328:                                     this_bio = initialSedBiomass[FunctGroupArray[guild].totNTracers[cohort]];
 2329:                                     break;
 2330:                                 case EPIFAUNA:
 2331:                                     this_bio = initialEpiBiomass[FunctGroupArray[guild].totNTracers[cohort]];
 2332:                                     sp_scalar *= wcLayerThick; // To get in to mg m-2 from mg-3
 2333:                                     break;
 2334:                                 case LAND_BASED:
 2335:                                     this_bio = initialLandBiomass[FunctGroupArray[guild].totNTracers[cohort]];
 2336:                                     break;
 2337:                                 case ICE_BASED:
 2338:                                     this_bio = initialIceBiomass[FunctGroupArray[guild].totNTracers[cohort]];
 2339:                                     break;
 2340:                                 case MIXED:
 2341:                                     quit("How did we get here as should come through a primary habitat\n");
 2342:                                     break;
 2343:                             }
 2344:                         }
 2345:                         realism_scalar = 1.0;
 2346:                         if (tot_dis > this_bio)
 2347:                             realism_scalar = this_bio / (tot_dis + small_num);
 2348:                         
 2349: 						for(nf=0; nf<bm->K_num_fisheries; nf++){
 2350:                             if(bm->DependDiscardsTot[nf][guild][cohort]) {
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
**Listed lines:** 2616-2622

**Function:** `Epibenthic_Box()`

**Why it's thought to be a bug**

In the old-diet multi-cohort branch for EPIFAUNA groups: `grazeEaten += preyEaten[ct][WC]; grazeEaten *= localWCTracers[...totNTracers[cohort]] / totalBiomass;`. But `totalBiomass` is summed from `localEPITracers` (2602) and the single-cohort branch uses `preyEaten[0][EPIFAUNA]` (2624). So predation is taken from the wrong habitat and split by the wrong tracer. Function: `Epibenthic_Box()`.

**Current code** (`atecology/atecology.c` lines 2613–2623)

```c
 2613: 
 2614: 								} else {
 2615: 									grazeEaten = FunctGroupArray[guild].preyEaten[cohort][EPIFAUNA];
 2616: 								}
 2617: 
 2618: 								if(FunctGroupArray[guild].groupType == SEAGRASS && bm->flag_macro_model == TRUE){
 2619: 									grazeEaten = FunctGroupArray[guild].preyEaten[0][cohort];   // TODO: check this is really around the right way
 2620: 								}
 2621: 								boxLayerInfo->localEPIFlux[FunctGroupArray[guild].totNTracers[cohort]] = FunctGroupArray[guild].growth[cohort]
 2622: 										- FunctGroupArray[guild].mortality[cohort] - dead - (double)grazeEaten;
 2623: 
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
**Listed lines:** 2747-2753

**Function:** `Epibenthic_Box()`

**Why it's thought to be a bug**

In the SED branch (old diet, multi-cohort): `grazeEaten *= (localWCTracers[...totNTracers[cohort]] / totalBiomass);` while `totalBiomass` is summed from `localSEDTracers` (2734). Function: `Epibenthic_Box()`.

**Current code** (`atecology/atecology.c` lines 2745–2755)

```c
 2745: 										grazeEaten = FunctGroupArray[guild].preyEaten[cohort][SED];
 2746: 									}
 2747: 									boxLayerInfo->localSEDFlux[FunctGroupArray[guild].totNTracers[cohort]] += (-dead - (double)grazeEaten);
 2748: 
 2749: 									if(bm->track_atomic_ratio == TRUE){
 2750: 
 2751: 										/* Predation and growth due to predation is handled separately */
 2752: 										Loose_Element(bm, boxLayerInfo, SED, guild, cohort,
 2753: 												(dead),
 2754: 												EPIFAUNA, isGlobal);
 2755: 									}
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
**Listed lines:** 2631-2632

**Function:** `Epibenthic_Box()`

**Why it's thought to be a bug**

`grazeEaten = FunctGroupArray[guild].preyEaten[0][cohort];` (TODO in the code says "check this is really around the right way"). `preyEaten` is `Util_Alloc_Init_2D_Long_Double(num_active_habitats, numCohortsXnumGenes)`, i.e. `[cohort][hab]` (atUtilGroupIO.c:821). This indexes habitat by cohort: it reads the wrong value, and for cohort >= num_active_habitats it reads past row 0. Applies to seagrass with `flag_macro_model`. Function: `Epibenthic_Box()`.

**Current code** (`atecology/atecology.c` lines 2629–2634)

```c
 2629: 								}
 2630: 
 2631: 								/**
 2632: 								if(bm->which_check == guild){
 2633: 									fprintf(llogfp,"day: %f box%d-%d, %s-%d epiflux: %.20e grow: %.20e mort: %.20e dead: %.20e eaten: %Le preyEaten: %Le dead[]: %e, deadGlobal[]: %e\n",
 2634: 										bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, cohort,
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
**Listed lines:** 1136-1147

**Function:** `Water_Column_Box()`

**Why it's thought to be a bug**

Layers run top->bottom (atbiology.c:414). The top layer sets `PelDem_ratio_i = PDratio`, but the else branch "increment[s] the global average" with `=`: `localDiagTracers[PelDem_ratio_i] = PDratio / nz;` (same for PiscPlank). So the final indicator is just the bottom layer ratio/nz. The top-layer value is also not divided by nz. Function: `Water_Column_Box()`.

**Current code** (`atecology/atecology.c` lines 1128–1143) — code appears to have moved -6 lines since the review

```c
 1128: 			boxLayerInfo->localDiagTracers[PiscPlank_ratio_i] = FVFPratio / bm->boxes[bm->current_box].nz;
 1129: 			if (DivHere > boxLayerInfo->localDiagTracers[DivCount_i])
 1130: 				boxLayerInfo->localDiagTracers[DivCount_i] = DivHere;
 1131: 		}
 1132: 	}
 1133:     
 1134: 	//TODO: Rewrite to allow for age structured biomass groups.
 1135: 	/* Update mortality estimates - Start with vertebrates */
 1136: 	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
 1137: 		if(FunctGroupArray[guild].speciesParams[flag_id] == TRUE){
 1138: 			if (FunctGroupArray[guild].isVertebrate == TRUE) {
 1139: 				/* For use in pseudo F estimate */
 1140: 				fished_chrt = (int) (FunctGroupArray[guild].speciesParams[Age95pcntV_id]);
 1141: 				if (fished_chrt < ((FunctGroupArray[guild].numCohortsXnumGenes) / 2))
 1142: 					fished_chrt = ((FunctGroupArray[guild].numCohortsXnumGenes)/ 2);
 1143: 
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
**Listed lines:** 3923-3930

**Function:** `Reconcile_Global_Values()`

**Why it's thought to be a bug**

`for (kij = 0; kij < EPIFAUNA; kij++)` reconciles NutsProd/NutsLost/DetritusProd/DetritusLost only for WC and SED. When called with `ICE_BASED`, the `[ICE_BASED]` entries used by atNutrient.c:480,486 never get their once-per-dt (Global) contributions for it_count>1, while Reset_Arrays zeroes them on every call (up to num_active_habitats). Function: `Reconcile_Global_Values()`.

**Current code** (`atecology/atecology.c` lines 3921–3932)

```c
 3921: 	int hab, guild, cohort;
 3922: 
 3923: 	for (hab = 0; hab < bm->num_active_habitats; hab++) {
 3924: 		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
 3925: 			for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
 3926: 				if (FunctGroupArray[guild].speciesParams[flag_id] && FunctGroupArray[guild].isVertebrate == FALSE){
 3927: 					if (!(_finite(PREYinfo[guild][cohort][hab]))) {
 3928: 						fprintf(llogfp, "%e box%d-%d %s is %e in PREYinfo\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode,
 3929: 								PREYinfo[guild][cohort][hab]);
 3930: 					}
 3931: 				}
 3932: 			}
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
**Listed lines:** 3331-3340

**Function:** `Ice_Box()`

**Why it's thought to be a bug**

The ice box sets `SP_IRR` for groups with `habitatType == SED` and SED phyto types (copied from Sediment_Box, lines 1498-1507), using `localICETracers[Light_Adaptn_TracerID]`. Ice primary producers (ICE_DIATOMS/ICE_MIXOTROPHS) never get `SP_IRR` set here and keep stale values. Function: `Ice_Box()`.

**Current code** (`atecology/atecology.c` lines 3329–3342)

```c
 3329: 					for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
 3330: 						PREYinfo[guild][cohort][ICE_BASED] += boxLayerInfo->localICETracers[FunctGroupArray[guild].totNTracers[cohort]]
 3331: 								* FunctGroupArray[guild].habitatCoeffs[ICE_BASED];
 3332: 					}
 3333: 				} else {
 3334: 					PREYinfo[guild][0][ICE_BASED] = boxLayerInfo->localICETracers[FunctGroupArray[guild].totNTracers[0]] * FunctGroupArray[guild].habitatCoeffs[ICE_BASED];
 3335: 				}
 3336: 			}
 3337: 		}
 3338: 	}
 3339: 
 3340: 	/* Check that the prey values are finite */
 3341: 	Check_Prey_Is_Finite(bm, llogfp, habitat_type);
 3342: 
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
**Listed lines:** 2329-2360

**Function:** `Epibenthic_Box()`

**Why it's thought to be a bug**

Dependent-discard reality check: `tot_dis` is the sum of `DependDiscardsTot*dt` (mg N m-3), but for EPIFAUNA `this_bio = initialEpiBiomass[...]` is mg N m-2. Line 2345 itself notes the m-3 -> m-2 conversion is needed. So `if (tot_dis > this_bio)` compares quantities in different units. Function: `Epibenthic_Box()`.

**Current code** (`atecology/atecology.c` lines 2327–2362)

```c
 2327:                                 case SED:
 2328:                                     this_bio = initialSedBiomass[FunctGroupArray[guild].totNTracers[cohort]];
 2329:                                     break;
 2330:                                 case EPIFAUNA:
 2331:                                     this_bio = initialEpiBiomass[FunctGroupArray[guild].totNTracers[cohort]];
 2332:                                     sp_scalar *= wcLayerThick; // To get in to mg m-2 from mg-3
 2333:                                     break;
 2334:                                 case LAND_BASED:
 2335:                                     this_bio = initialLandBiomass[FunctGroupArray[guild].totNTracers[cohort]];
 2336:                                     break;
 2337:                                 case ICE_BASED:
 2338:                                     this_bio = initialIceBiomass[FunctGroupArray[guild].totNTracers[cohort]];
 2339:                                     break;
 2340:                                 case MIXED:
 2341:                                     quit("How did we get here as should come through a primary habitat\n");
 2342:                                     break;
 2343:                             }
 2344:                         }
 2345:                         realism_scalar = 1.0;
 2346:                         if (tot_dis > this_bio)
 2347:                             realism_scalar = this_bio / (tot_dis + small_num);
 2348:                         
 2349: 						for(nf=0; nf<bm->K_num_fisheries; nf++){
 2350:                             if(bm->DependDiscardsTot[nf][guild][cohort]) {
 2351:                                 bm->DependDiscardsTot[nf][guild][cohort] *= realism_scalar;
 2352:                                 bm->Discards[bm->current_box][guild][nf] += (bm->DependDiscardsTot[nf][guild][cohort] * bm->dt * bm->cell_vol);
 2353: 
 2354:                                 if(FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED){
 2355:                                     FunctGroupArray[guild].SizeNumDiscard[cohort][nf][bm->current_box] += (bm->DependDiscardsTot[nf][guild][cohort] * bm->dt * bm->cell_vol);  // Added "* bm->dt * bm->cell_vol" to be consistent with water column box case and CatchSum case
 2356:                                     bm->FCdiscard[guild][cohort] += bm->DependDiscardsTot[nf][guild][cohort];
 2357:                                 }
 2358:                                 boxLayerInfo->DetritusProd[SED][DCdet_id] += bm->DependDiscardsTot[nf][guild][cohort];
 2359: 
 2360:                                 FunctGroupArray[guild].dead[cohort] += bm->DependDiscardsTot[nf][guild][cohort] * sp_scalar;
 2361:                                 
 2362:                                 // Update Catchsum - moved here to remove an additional loop and so it has the reality check in place
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
**Listed lines:** 2974-2977

**Function:** `Epibenthic_Box()`

**Why it's thought to be a bug**

ICE_BASED cover biomass: `biomass = initialIceBiomass[...totNTracers[cohort]];` inside the cohort loop keeps only the last cohort. The EPIFAUNA/LAND cases use `+=`. Function: `Epibenthic_Box()`.

**Current code** (`atecology/atecology.c` lines 2959–2966) — code appears to have moved -13 lines since the review

```c
 2959: 			} else if (FunctGroupArray[guild].speciesParams[flag_lim_id] == ersem_ben_lim) {
 2960: 				boxLayerInfo->localEPITracers[FunctGroupArray[guild].CoverTracer]
 2961: 						= min(1.0, biomass / (50.0 * FunctGroupArray[guild].speciesParams[thresh_id]));
 2962: 			}
 2963: 		}
 2964: 	}
 2965: 
 2966: 
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
**Listed lines:** 1340, 3164 (also 1985, 3642)

**Why it's thought to be a bug**

`if (flux_model_id > 0) { switch ... case WC: ...}`. WC == 0 (atlantisboxmodel.h:390), and Max_Flux_Check returns `fluxnum = WC` for water-column culprits, so the WC diagnostic branch can never run and "Can't give details" is printed instead. Note: if fixed to `>= 0`, nutrient culprits set `flux_sp = bm->K_num_tot_sp` (atfluxbreakdown.c:117), and `FunctGroupArray[flux_sp].groupCode` would then read out of bounds. Functions: `Water_Column_Box()`, `Epibenthic_Box()`.

**Current code** (`atecology/atecology.c` lines 1336–1340)

```c
 1336: 					break;
 1337: 				case death_id:
 1338: 					fprintf(llogfp, "Water_Column_Box: biggest flux is due to the death of %s-%d\n", FunctGroupArray[flux_sp].groupCode, fluxnum);
 1339: 					break;
 1340: 				default:
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
**Listed lines:** 1156-1169

**Function:** `Water_Column_Box()`

**Why it's thought to be a bug**

`fished_chrt = (int)speciesParams[Age95pcntV_id]` is an age-class index, but it is used directly to index `dead[]`/`preyEaten[]`, which are sized numCohortsXnumGenes, and is compared with `numCohortsXnumGenes/2`. Elsewhere it is converted (atassess/atclassical.c:860 `nc = va95 * numGeneTypes`). With numGeneTypes>1 this picks the wrong cohort for pseudo-F/M. There is no upper bound check either. Function: `Water_Column_Box()`.

**Current code** (`atecology/atecology.c` lines 1154–1171)

```c
 1154: 
 1155: 			} else {
 1156: 				for(cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++){
 1157: 					//fished_chrt = 0;
 1158: 		   			bm->calcTrackedMort[guild][0][0][ongoingF_id] += (FunctGroupArray[guild].dead[cohort] * bm->dt);
 1159: 					bm->calcFnum[guild][current_id] += FunctGroupArray[guild].dead[cohort];
 1160: 
 1161: 					/* For use in pseudo M estimate */
 1162: 					bm->calcMnum[guild][current_id] += (FunctGroupArray[guild].mortality[cohort]
 1163: 					  + (double)FunctGroupArray[guild].preyEaten[cohort][WC] + (double)FunctGroupArray[guild].preyEaten[cohort][SED]
 1164: 					  + (double)FunctGroupArray[guild].preyEaten[cohort][EPIFAUNA]);
 1165: 
 1166: #ifdef BROKER_LINK_ENABLED
 1167: 					if(do_BrokerLinkage){
 1168: 						bm->calcMnum[guild][current_id] += bm->linkageInterface->linkageWCMortality[bm->current_box][bm->current_layer][guild][cohort];
 1169: 					}
 1170: #endif
 1171: 				}
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
**Listed lines:** 1914-1923

**Function:** `Sediment_Box()`

**Why it's thought to be a bug**

Sediment pseudo-M estimate: detritus uses `preyEaten[0][WC]` in the sediment box (should be SED; WC predation is already counted in Water_Column_Box at 1178-1180). The non-detritus loop adds `mortality[fished_chrt]` (=cohort 0) once per cohort instead of `mortality[cohort]`. Function: `Sediment_Box()`.

**Current code** (`atecology/atecology.c` lines 1912–1925)

```c
 1912: #ifdef BROKER_LINK_ENABLED
 1913: 				if(do_BrokerLinkage){
 1914: 					bm->calcMnum[guild][current_id] += bm->linkageInterface->linkageSEDMortality[bm->current_box][bm->current_layer][guild][fished_chrt];
 1915: 				}
 1916: #endif
 1917: 			}
 1918: 		}
 1919: 	}
 1920: 
 1921: 	/* check diagnostics */
 1922: 	if (bm->habitat_check == SED && bm->which_check < not_tracking_flux) {
 1923: 		flux_correct = sporosity; // Correct nutrient fluxes in the sediment for sediment porosity
 1924: 	} else
 1925: 		flux_correct = 1.0;
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
**Listed lines:** 2937-2940

**Function:** `Epibenthic_Box()`

**Why it's thought to be a bug**

The epibenthic pseudo-M only uses `mortality[0]` and `preyEaten[0][...]`, so multi-cohort epifauna (e.g. age-structured seagrass) lose all cohorts except the first. Function: `Epibenthic_Box()`.

**Current code** (`atecology/atecology.c` lines 2911–2918) — code appears to have moved -24 lines since the review

```c
 2911: 					+ ((FunctGroupArray[guild].habitatType == EPIFAUNA) * (double)FunctGroupArray[guild].preyEaten[0][WC] )
 2912: 					+ ((FunctGroupArray[guild].habitatType == EPIFAUNA) * (double)FunctGroupArray[guild].preyEaten[0][SED] )
 2913: 					+ ((FunctGroupArray[guild].habitatType == EPIFAUNA) * (double)FunctGroupArray[guild].preyEaten[0][EPIFAUNA] );
 2914: 
 2915: 			}
 2916: 		}
 2917: 	}
 2918: 
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
**Listed lines:** 3985-3988

**Function:** `Reset_Arrays()`

**Why it's thought to be a bug**

`for (k = 0; k < numStages; k++) grow[kij][k] = 0.0;`. `grow` is allocated `[numCohortsXnumGenes][RN_id+1]` (atUtilGroupIO.c:748), and the index is SN_id/RN_id, not life stage. If numStages==1, `grow[][RN_id]` is not reset. If numStages>2, the loop writes past the row. Function: `Reset_Arrays()`.

**Current code** (`atecology/atecology.c` lines 3972–3979) — code appears to have moved -11 lines since the review

```c
 3972: 
 3973: 			for (hab = 0; hab < bm->num_active_habitats; hab++) {
 3974: 				for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
 3975: 					PREYinfo[guild][cohort][hab] = 0.0;
 3976: 					FunctGroupArray[guild].preyEaten[cohort][hab] = 0.0;
 3977: 					/* Set the global value to zero on the first iteration in this time step */
 3978: 					if (it_count == 1)
 3979: 						FunctGroupArray[guild].preyEatenGlobal[cohort][habitat_type][hab] = 0.0;
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
**Listed lines:** 2424-2430

**Function:** `Epibenthic_Box()`

**Why it's thought to be a bug**

In the Infauna/Epifauna ratio, MB (habitatType SED, see 1499 and comment at 2952) is added as `initialWaterBiomass[...] * smLayerThick`: a water-column concentration multiplied by sediment thickness. It should be the sediment biomass. Function: `Epibenthic_Box()`.

**Current code** (`atecology/atecology.c` lines 2417–2427) — code appears to have moved -5 lines since the review

```c
 2417: 							Epifauna += initialEpiBiomass[FunctGroupArray[guild].totNTracers[cohort]];
 2418: 					} else if (FunctGroupArray[guild].isInfauna == TRUE){
 2419: 						Infauna += initialSedBiomass[FunctGroupArray[guild].totNTracers[cohort]];
 2420: 					}
 2421: 				}
 2422: 			}
 2423: 		}
 2424: 
 2425: 		/* Infauna vs Epifauna ratio - convert infauna to m-2 from m-3 first.
 2426: 		 Note that MB count as epifauna here as live on sediment surface
 2427: 		 */
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
**Listed lines:** 3689-3691

**Function:** `Ice_Box()`

**Why it's thought to be a bug**

The ice box QNAN guard tests `localSEDFlux[NH3_i]` and calls `Call_Diagnostics(..., SED, ...)`, so NaNs in `localICEFlux` are never caught. Function: `Ice_Box()`.

**Current code** (`atecology/atecology.c` lines 3665–3671) — code appears to have moved -22 lines since the review

```c
 3665: 	if (!(_finite(boxLayerInfo->localSEDFlux[NH3_i]))) {
 3666: 		/* Check diagnostic terms */
 3667: 		Call_Diagnostics(bm, boxLayerInfo, SED, llogfp, 1);
 3668: 
 3669: 		/* List all figures */
 3670: 		Print_Flux(bm, ICE_BASED, VERTinfo, qnancheck, 0.0, 0.0, 0.0, 0.0, 0.0, iceFlux, 0.0, iceFlux, 0.0, boxLayerInfo->localWCFlux, boxLayerInfo->localSEDFlux,
 3671: 				boxLayerInfo->localEPIFlux, llogfp);
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
**Listed lines:** 1364-1365, 2005-2006

**Why it's thought to be a bug**

In the flux-imbalance path, `Call_Diagnostics(bm, boxLayerInfo, EPIFAUNA, llogfp, 0)` is called from `Water_Column_Box()` and `Sediment_Box()` (should be WC / SED). The guard also uses `(current_box == checkbox) || (dayt >= checkstart)`, whereas the normal call (1249/1955) uses `&&`.

**Current code** (`atecology/atecology.c` lines 1368–1373) — code appears to have moved +6 lines since the review

```c
 1368: 			/* List all figures */
 1369: 
 1370: 			fprintf(llogfp, "debug Water_Column_Box, bm->current_box = %d, currentLayer = %d, it_count = %d\n", bm->current_box, bm->current_layer, it_count);
 1371: 			Print_Flux(bm, WC, VERTinfo, noqnanverbose, wcFlux, wcFlux1, wcFlux2, wcFlux3, 0, 0, 0, wcFlux, wcFishing, boxLayerInfo->localWCFlux,
 1372: 					boxLayerInfo->localSEDFlux, boxLayerInfo->localEPIFlux, llogfp);
 1373: 
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
**Listed lines:** 1077-1098, 2806-2831

**Why it's thought to be a bug**

`fprintf(llogfp, "day: %e box%d-%d doing %s with flux entering...")` runs for every physio-chem tracer, layer and adaptive substep whenever `dayt >= checkstart && checkbox == current_box`, with no `bm->debug`/verbose gate. This can produce very large logs, similar to the printf removed in commit f56b3e7. Functions: `Water_Column_Box()`, `Epibenthic_Box()`.

**Current code** (`atecology/atecology.c` lines 1078–1103) — code appears to have moved +3 lines since the review

```c
 1078: 				/* Check the tracer values */
 1079: 				if(!_finite(boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex])){
 1080: 					quit("Water_Column_Box - Nutrient %s is infinite in box %d:%d\n", PhysioChemArray[index].name, bm->current_box, bm->current_layer);
 1081: 				}
 1082: 			}
 1083: 		}
 1084: 	}
 1085: 
 1086: 	if (it_count == 1) {
 1087: 		/**
 1088: 		 Carcasses in the water column
 1089: 		 **/
 1090: 		Carrion_ROC(bm, llogfp, WC, boxLayerInfo);
 1091: 
 1092: 
 1093:         /**
 1094: 		 Vertebrates - if in lowest watercolumn layer than this will all actually happen
 1095: 		 in Epibenthic_Box()
 1096: 		 **/
 1097:         Update_Vertebrate_Tracers(bm, llogfp, WC, boxLayerInfo);
 1098: 
 1099: 		/** Update overall catch at age distributions with those from the current
 1100: 		 water column layer
 1101: 		 **/
 1102: 		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
 1103: 			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
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
**Listed lines:** 3348-3356

**Function:** `Ice_Box()`

**Why it's thought to be a bug**

PREYinfo is reset only for cohort 0, but AGE_STRUCTURED_BIOMASS cohorts use `PREYinfo[guild][cohort][ICE_BASED] += ...`. Latent for now: such groups with an ice coefficient >0 quit at 3430, and with a coefficient of 0 the term adds 0. Function: `Ice_Box()`.

**Current code** (`atecology/atecology.c` lines 3352–3364) — code appears to have moved +6 lines since the review

```c
 3352: 		/* Local diversity proxy */
 3353: 		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
 3354: 			if (FunctGroupArray[guild].habitatType == ICE_BASED) {
 3355: 				if (FunctGroupArray[guild].speciesParams[flag_id] && (initialBiomass[FunctGroupArray[guild].totNTracers[0]] > bm->min_pool))
 3356: 					DivHere++;
 3357: 			}
 3358: 
 3359: 		}
 3360: 	}
 3361: 
 3362: 	/**
 3363: 	 * Initial the invertebrate variables.
 3364: 	 */
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
**Listed lines:** 2594-2655, 3723-3725

**Why it's thought to be a bug**

The DebugFluxInfo for EPIFAUNA groups is stored under `[guild][WC]`, and `gain/loss = ...` uses `=` inside the cohort loop, so only the last cohort is kept (the WC/SED branches at 2719-2720 use `+=`). The same `=`-in-cohort-loop pattern appears in `Update_Vertebrate_Tracers()` 3723-3725. This only affects DiagNGain/Loss tracking. Functions: `Epibenthic_Box()`, `Update_Vertebrate_Tracers()`.

**Current code** (`atecology/atecology.c` lines 2580–2614 (range truncated)) — code appears to have moved -12 lines since the review

```c
 2580: 
 2581: 						boxLayerInfo->DebugFluxInfo[guild][WC][loss_id] = 0;
 2582: 						boxLayerInfo->DebugFluxInfo[guild][WC][gain_id] = 0;
 2583: 
 2584: 						if (FunctGroupArray[guild].habitatType == EPIFAUNA) {
 2585: 
 2586: 							/* Get the total biomass */
 2587: 							totalBiomass = 0.0;
 2588: 							for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
 2589: 								totalBiomass += boxLayerInfo->localEPITracers[FunctGroupArray[guild].totNTracers[cohort]];
 2590: 							}
 2591: 							totalBiomass += small_num;
 2592: 
 2593: 							for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
 2594: 								double dead;
 2595: 								long double grazeEaten;
 2596: 
 2597: 								/* Get the dead due to fishing - If this is a finer scale group - use the global values else use the values that were calculated in this time step. */
 2598: 								if (FunctGroupArray[guild].diagTol < 2)
 2599: 									dead = FunctGroupArray[guild].deadGlobal[cohort];
 2600: 								else
 2601: 									dead = FunctGroupArray[guild].dead[cohort];
 2602: 
 2603: 								if(bm->flag_olddiet){
 2604: 									if((FunctGroupArray[guild].numCohortsXnumGenes) > 1){
 2605: 										grazeEaten = 0.0;
 2606: 										for (ct = 0; ct < FunctGroupArray[guild].numCohortsXnumGenes; ct++){
 2607: 											grazeEaten += FunctGroupArray[guild].preyEaten[ct][WC];
 2608: 										}
 2609: 										grazeEaten *= (boxLayerInfo->localWCTracers[FunctGroupArray[guild].totNTracers[cohort]] / totalBiomass);
 2610: 									}else{
 2611: 										grazeEaten = FunctGroupArray[guild].preyEaten[0][EPIFAUNA];
 2612: 									}
 2613: 
 2614: 								} else {
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
**Listed lines:** 1357

**Why it's thought to be a bug**

The `quit("... in the Epibenthic_Box calculations?")` is inside `Water_Column_Box()`.

**Current code** (`atecology/atecology.c` lines 1359–1363) — code appears to have moved +4 lines since the review

```c
 1359: 
 1360: 			Textfile_Dump(bm, llogfp);
 1361: 			quit("Error on flux balance in water column cell on day: %e, in box: %d, layer: %d, wcFlux: %e\n", bm->dayt, bm->current_box, bm->current_layer,
 1362: 					wcFlux);
 1363: 
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
