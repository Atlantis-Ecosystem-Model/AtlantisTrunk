# atecology/atbiology.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 2; confidence H and severity H: 2; on the priority list: 2), plus 1 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [BIOL-01](#biol-01) ⭐ | 479-507 | H/H | wrong index / buffer overrun | `newsedtr[bm->current_box][ij - numwclayer][k] = ...` — the loop variable `ij` runs over sediment layer indice… |  | [ ] |
| [BIOL-02](#biol-02) ⭐ | 1474-1481 | H/H | buffer overrun | `for (cohort = 0; cohort < numCohortsXnumGenes * numStocks; cohort++) bm->totDiet[b][k][sp][cohort][prey] = 0.… |  | [ ] |
| [BIOL-03](#biol-03) | 774-786 vs 1350 | M/M | wrong offset | Ice stability check calls `Get_Max_Flux_Change(..., localICETracers, ..., bm->ntracer /*offset*/, ...)`, so `T… |  | [ ] |
| [BIOL-04](#biol-04) | 398-404, 469-642 | L/M | logic / scoping | The ice-layer processing (`if(bm->ice_on) { FlagModel = 4; ... }`) sits inside `if (!oceanic_only)`. In boxes … |  | [ ] |

---

## BIOL-01

**wrong index / buffer overrun** · Conf H / Sev H ⭐ priority  
**Listed lines:** 479-507

**Function:** `Box_Bio_Process()`

**Why it's thought to be a bug**

`newsedtr[bm->current_box][ij - numwclayer][k] = ...` — the loop variable `ij` runs over sediment layer indices (`pBox->sm.topk+1 .. sm.nz-1`) and `Copy_SED_Tracers()` reads `sm.tr[bm->current_layer = ij]`. But the write-back subtracts the *water-column* layer count, so it gives a negative or wrong layer. `newsedtr` is `alloc3d(ntracer, sednz, nbox)` => `[nbox][sednz][ntracer]` (atlantismain.c:650) and is indexed in sediment-layer space everywhere else (atboundary.c:66, atbiophysics.c:940). Any model with >1 sediment layer writes out of bounds, or loses the sediment-layer results. Function: `Box_Bio_Process()`.

**Current code** (`atecology/atbiology.c` lines 477–509)

```c
  477: 		smLayerThick = -pBox->sm.dz[0];
  478: 
  479: 		/* Give index of amount of suspended sediment in the water column */
  480: 		if (oceanic_only || !bm->resuspension || bm->cell_vol / pBox->area > 25)
  481: 			Susp_Sed = 1.0;
  482: 		else
  483: 			Susp_Sed = pBox->stress * pBox->area / bm->cell_vol;
  484: 
  485:         /* Run Adaptive Difference Method */
  486: 		Adapt_Diff_Method(bm, FlagModel, dt, boxLayerInfo, llogfp);
  487: 
  488: 		/* Spawning */
  489: 		Vertebrate_Reproduction(bm, ij, maxdeep, totaldeep, numwclayer, boxLayerInfo->localWCTracers, llogfp);
  490: 		Invertebrate_Reproduction(bm, ij, maxdeep, totaldeep, boxLayerInfo->localWCTracers, llogfp);
  491: 
  492: 		/* Transfer all the temporary values back to their final locations */
  493: 		for (k = 0; k < numwcvar; k++)
  494: 			newwctr[bm->current_box][ij][k] = boxLayerInfo->localWCTracers[k]; /* To WC */
  495: 		Return_Grazed_ICE_Tracers(bm, boxLayerInfo); /* Ice grazed from below - before the ice layers are processed */
  496: 		for (k = 0; k < numdiagvar; k++)
  497: 			pBox->diagnost[k] = boxLayerInfo->localDiagTracers[k]; /* To diagnostics */
  498: 		for (k = 0; k < numfstatvar; k++)
  499: 			pBox->fishstat[k] = boxLayerInfo->localFishTracers[k]; /* To fisheries statistics */
  500: 	}
  501:     
  502:     /** Now process sediment layers not in contact with water. **/
  503: 	if (!oceanic_only) {
  504: 		FlagModel = 2;
  505: 
  506: 		/* Get depth of sediment surface and then
  507: 		 if need be determine depth specific biological parameters */
  508: 		cell_depth = cell_depth - pBox->dz[0];
  509: 		if (numwclayer > 1)
```

**Suggested fix (from review):** `newsedtr[bm->current_box][ij][k] = ...`

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

## BIOL-02

**buffer overrun** · Conf H / Sev H ⭐ priority  
**Listed lines:** 1474-1481

**Function:** `ReInitDietData()`

**Why it's thought to be a bug**

`for (cohort = 0; cohort < numCohortsXnumGenes * numStocks; cohort++) bm->totDiet[b][k][sp][cohort][prey] = 0.0;` — `totDiet` is `alloc5d(K_num_tot_sp, K_num_max_cohort, K_num_tot_sp, wcnz+sednz, nbox)` => cohort dim = `K_num_max_cohort` (atlantismain.c:1176), and K_num_max_cohort is the max of `numCohorts` (atUtilGroupIO.c:302). Any group with >1 stock or >1 genotype writes past the cohort dimension. That corrupts other entries, and past the end of the block for the last box. Runs only when `flagdietcheck` is on. Function: `ReInitDietData()`.

**Current code** (`atecology/atbiology.c` lines 1529–1540) — code appears to have moved +57 lines since the review

```c
 1529:     int k, sp, cohort, prey;
 1530:     
 1531:     for (k = 0; k < bm->wcnz+bm->sednz; k++) {
 1532:         for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 1533:             if(FunctGroupArray[sp].speciesParams[active_id]) {
 1534:                 for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes * FunctGroupArray[sp].numStocks; cohort++) {
 1535:                     for (prey=0; prey < bm->K_num_tot_sp; prey++) {
 1536:                         bm->totDiet[b][k][sp][cohort][prey] = 0.0;
 1537:                     }
 1538:                 }
 1539:             }
 1540:         }
```

**Suggested fix (from review):** Loop `cohort < FunctGroupArray[sp].numCohortsXnumGenes` and allocate the cohort dim as `K_num_max_cohort*K_num_max_genetypes` (also needed for atprocess.c:2356)

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

## BIOL-03

**wrong offset** · Conf M / Sev M  
**Listed lines:** 774-786 vs 1350

**Function:** `Get_Time_Change()`

**Why it's thought to be a bug**

Ice stability check calls `Get_Max_Flux_Change(..., localICETracers, ..., bm->ntracer /*offset*/, ...)`, so `Tolflag`/`Fluxflag` are looked up in the *sediment* flag slots. Integration (`Integrate_Tracer_Variables(..., ICE_BASED, ..., 0 /*offset*/)`, l.1350) and `Copy_ICE_Tracers()` (`Fluxflag[i]`) both use offset 0. The adaptive time step for ice is therefore based on different flags than the ones used to integrate (the l.778 comment already questions this). Function: `Get_Time_Change()`.

**Current code** (`atecology/atbiology.c` lines 768–784) — code appears to have moved -4 lines since the review

```c
  768: 
  769: 		/* Get the max change in the tracer variables */
  770: 		Get_Max_Flux_Change(bm, boxLayerInfo->localWCTracers, boxLayerInfo->localWCFlux, bm->ntracer, 0, del, &maxTracer_del, &tracer_ind_unstable_var, llogfp);
  771: 		offset_int = 0;
  772: 		if (maxTracer_del > max_del) {
  773: 			max_del = maxTracer_del;
  774: 			ind_unstable_var = tracer_ind_unstable_var;
  775: 			tracerValue = boxLayerInfo->localWCTracers[ind_unstable_var];
  776: 			fluxValue = boxLayerInfo->localWCFlux[ind_unstable_var];
  777: 			unstable_offset = offset_int;
  778: 		}
  779: 	}
  780: 
  781: 	if(flagModel == SED || flagModel == EPIFAUNA){
  782: 		/* Get the max change in the sediment variables */
  783: 		Get_Max_Flux_Change(bm, boxLayerInfo->localSEDTracers, boxLayerInfo->localSEDFlux, bm->ntracer, bm->ntracer, del, &maxSed_del, &sed_ind_unstable_var,
  784: 				llogfp);
```

**Suggested fix (from review):** Pass offset 0 for the ice case, the same as integration

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

## BIOL-04

**logic / scoping** · Conf L / Sev M  
**Listed lines:** 398-404, 469-642

**Function:** `Box_Bio_Process()`

**Why it's thought to be a bug**

The ice-layer processing (`if(bm->ice_on) { FlagModel = 4; ... }`) sits inside `if (!oceanic_only)`. In boxes deeper than `bm->max_depth` (oceanic_only), ice biology is never run and `newicetr` is never updated, although sea ice can occur over deep boxes. Function: `Box_Bio_Process()`.

**Current code** (`atecology/atbiology.c` lines 396–406)

```c
  396:     boxLayerInfo->DIN = 0.0;
  397: 
  398: 	//	int den, cohort;
  399: 	bm->max_depth = bm->maxwcbotz;
  400: 
  401: 	/** Identify the current box */
  402: 	bm->current_box = pBox->n;
  403: 
  404: 	/** Identify current bottom depth */
  405: 	current_depth = pBox->botz;
  406: 
```

**Suggested fix (from review):** Move the ice block outside the `!oceanic_only` block (if ice should be processed everywhere)

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

atbiology.c `UpdateMdt()`: for `isOncePerDt` groups after day 1, Mdt is set to `dtsz` rather than `bm->dt`. On `it_count==1`, `dtsz == tsz`, so this is likely harmless.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
