# atecology/atbiology.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 2; confidence H and severity H: 2; on the priority list: 2), plus 1 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [BIOL-01](#biol-01) ⭐ | 515-543 | H/H | wrong index / buffer overrun | `newsedtr[bm->current_box][ij - numwclayer][k] = ...` — the loop variable `ij` runs over sediment layer indice… |  | [ ] |
| [BIOL-02](#biol-02) ⭐ | 1533-1540 | H/H | buffer overrun | `for (cohort = 0; cohort < numCohortsXnumGenes * numStocks; cohort++) bm->totDiet[b][k][sp][cohort][prey] = 0.… |  | [ ] |
| [BIOL-03](#biol-03) | 810-822 vs 1411 | M/M | wrong offset | Ice stability check calls `Get_Max_Flux_Change(..., localICETracers, ..., bm->ntracer /*offset*/, ...)`, so `T… |  | [ ] |
| [BIOL-04](#biol-04) | 433-439, 505-678 | L/M | logic / scoping | The ice-layer processing (`if(bm->ice_on) { FlagModel = 4; ... }`) sits inside `if (!oceanic_only)`. In boxes … |  | [ ] |

---

## BIOL-01

**wrong index / buffer overrun** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 515-543  
**Lines at review (Oct 2026):** 479-507

**Function:** `Box_Bio_Process()`

**Why it's thought to be a bug**

`newsedtr[bm->current_box][ij - numwclayer][k] = ...` — the loop variable `ij` runs over sediment layer indices (`pBox->sm.topk+1 .. sm.nz-1`) and `Copy_SED_Tracers()` reads `sm.tr[bm->current_layer = ij]`. But the write-back subtracts the *water-column* layer count, so it gives a negative or wrong layer. `newsedtr` is `alloc3d(ntracer, sednz, nbox)` => `[nbox][sednz][ntracer]` (atlantismain.c:779) and is indexed in sediment-layer space everywhere else (atboundary.c:66, atbiophysics.c:938). Any model with >1 sediment layer writes out of bounds, or loses the sediment-layer results. Function: `Box_Bio_Process()`.

**Current code** (`atecology/atbiology.c` lines 513–545)

```c
  513: 		for (ij = pBox->sm.topk + 1; ij < numsmlayer; ij++) {
  514: 
  515: 			if (verbose > 1)
  516: 				fprintf(llogfp, "processing sediment layer ij, %d\n", ij);
  517: 
  518: 			bm->current_layer = ij;
  519: 			sporosity = (double) pBox->sm.porosity[ij];
  520: 
  521: 			/** Set benthos stimulation of bacteria - compound of current
  522: 			 bioturbation levels and sediment porosity effect, the
  523: 			 porosity effect is from Fig 7 (bottom plot) in
  524: 			 Blackburn, 1987. Microbial food webs in sediments. In:
  525: 			 Sleigh (ed), Microbes in the sea. Ellis Horwood:New York. pp 39
  526: 			 **/
  527: 			sedC = (sporosity - 0.225) / 0.004;
  528: 			ratioC = sedC / 193.75;
  529: 			if (flagbactstim)
  530: 				Bact_stim = bm->boxes[bm->current_box].sm.turbenh * ratioC;
  531: 			else
  532: 				Bact_stim = 1.0; // Originally introduced while model being balanced
  533: 
  534: 			current_layer_sed = ij;
  535: 
  536: 			/* Run Adaptive Difference Method */
  537: 			Adapt_Diff_Method(bm, FlagModel, dt, boxLayerInfo, llogfp);
  538: 
  539: 			/* Transfer all of the values back to the final locations */
  540: 			for (k = 0; k < numwcvar; k++)
  541: 				newsedtr[bm->current_box][ij - numwclayer][k] = boxLayerInfo->localSEDTracers[k]; /* To WC */
  542: 			for (k = 0; k < numdiagvar; k++)
  543: 				pBox->diagnost[k] = boxLayerInfo->localDiagTracers[k]; /* To diagnostics */
  544: 			for (k = 0; k < numfstatvar; k++)
  545: 				pBox->fishstat[k] = boxLayerInfo->localFishTracers[k]; /* To fisheries statistics */
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
**Lines (current code):** 1533-1540  
**Lines at review (Oct 2026):** 1474-1481

**Function:** `ReInitDietData()`

**Why it's thought to be a bug**

`for (cohort = 0; cohort < numCohortsXnumGenes * numStocks; cohort++) bm->totDiet[b][k][sp][cohort][prey] = 0.0;` — `totDiet` is `alloc5d(K_num_tot_sp, K_num_max_cohort, K_num_tot_sp, wcnz+sednz, nbox)` => cohort dim = `K_num_max_cohort` (atlantismain.c:1318), and K_num_max_cohort is the max of `numCohorts` (atUtilGroupIO.c:302). Any group with >1 stock or >1 genotype writes past the cohort dimension. That corrupts other entries, and past the end of the block for the last box. Runs only when `flagdietcheck` is on. Function: `ReInitDietData()`.

**Current code** (`atecology/atbiology.c` lines 1531–1542)

```c
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
 1541:     }
 1542:     
```

**Suggested fix (from review):** Loop `cohort < FunctGroupArray[sp].numCohortsXnumGenes` and allocate the cohort dim as `K_num_max_cohort*K_num_max_genetypes` (also needed for atprocess.c:2516)

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
**Lines (current code):** 810-822 vs 1411  
**Lines at review (Oct 2026):** 774-786 vs 1350

**Function:** `Get_Time_Change()`

**Why it's thought to be a bug**

Ice stability check calls `Get_Max_Flux_Change(..., localICETracers, ..., bm->ntracer /*offset*/, ...)`, so `Tolflag`/`Fluxflag` are looked up in the *sediment* flag slots. Integration (`Integrate_Tracer_Variables(..., ICE_BASED, ..., 0 /*offset*/)`, l.1350) and `Copy_ICE_Tracers()` (`Fluxflag[i]`) both use offset 0. The adaptive time step for ice is therefore based on different flags than the ones used to integrate (the l.778 comment already questions this). Function: `Get_Time_Change()`.

**Current code** (`atecology/atbiology.c` lines 808–824)

```c
  808: 	if(flagModel == ICE_BASED ){
  809: 		/* Get the max change in the ice-based variables */
  810: 		Get_Max_Flux_Change(bm, boxLayerInfo->localICETracers, boxLayerInfo->localICEFlux, bm->ntracer, bm->ntracer, del, &maxIce_del, &ice_ind_unstable_var,
  811: 				llogfp);
  812: 		offset_int = 0;  // Its assumed to be zero in Adapt_Diff_Method().... should it actually be "3 * bm->ntracer" so not accidentally overwriting anything?
  813: 		if (maxIce_del > max_del) {
  814: 			max_del = maxIce_del;
  815: 			ind_unstable_var = ice_ind_unstable_var;
  816: 			tracerValue = boxLayerInfo->localICETracers[ind_unstable_var];
  817: 			fluxValue = boxLayerInfo->localICEFlux[ind_unstable_var];
  818: 			unstable_offset = offset_int;
  819: 		}
  820: 	}
  821: 	/* Set a new time step size */
  822: 	*dtsz = RelTol / max_del;
  823:     
  824:     /*
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
**Lines (current code):** 433-439, 505-678  
**Lines at review (Oct 2026):** 398-404, 469-642

**Function:** `Box_Bio_Process()`

**Why it's thought to be a bug**

The ice-layer processing (`if(bm->ice_on) { FlagModel = 4; ... }`) sits inside `if (!oceanic_only)`. In boxes deeper than `bm->max_depth` (oceanic_only), ice biology is never run and `newicetr` is never updated, although sea ice can occur over deep boxes. Function: `Box_Bio_Process()`.

**Current code** (`atecology/atbiology.c` lines 431–441)

```c
  431: 	if (current_depth < bm->max_depth) {
  432: 		oceanic_only = 1;
  433: 		stopij = -1;
  434: 	} else {
  435: 		oceanic_only = 0;
  436: 		stopij = 0;
  437: 	}
  438: 
  439: 	/** Process layers **/
  440: 	Susp_Sed = 1.0;
  441: 	cell_depth = 0.0;
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
