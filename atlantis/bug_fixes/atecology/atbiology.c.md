# atecology/atbiology.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 2; confidence H and severity H: 2; on the priority list: 2), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [BIOL-01](#biol-01) ⭐ | 541, 1083 | H/H | wrong index / buffer overrun | `newsedtr[bm->current_box][ij - numwclayer][k] = ...` — the loop variable `ij` runs over sediment layer indice… |  | [ ] |
| [BIOL-02](#biol-02) ⭐ | 1534 | H/H | buffer overrun | `for (cohort = 0; cohort < numCohortsXnumGenes * numStocks; cohort++) bm->totDiet[b][k][sp][cohort][prey] = 0.… |  | [ ] |
| [BIOL-03](#biol-03) | 810, 885, 1160 | M/M | wrong offset | Ice stability check calls `Get_Max_Flux_Change(..., localICETracers, ..., bm->ntracer /*offset*/, ...)`, so `T… |  | [ ] |
| [BIOL-04](#biol-04) | 650, 503, 431 | L/M | logic / scoping | The ice-layer processing (`if(bm->ice_on) { FlagModel = 4; ... }`) sits inside `if (!oceanic_only)`. In boxes … |  | [ ] |

---

## BIOL-01

**wrong index / buffer overrun** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atbiology.c` 541, `atbiology.c` 1083
**Lines at review (Oct 2026):** 479-507

**Function:** `Box_Bio_Process()`

**Why it's thought to be a bug**

`newsedtr[bm->current_box][ij - numwclayer][k] = ...` — the loop variable `ij` runs over sediment layer indices (`pBox->sm.topk+1 .. sm.nz-1`) and `Copy_SED_Tracers()` reads `sm.tr[bm->current_layer = ij]`. But the write-back subtracts the *water-column* layer count, so it gives a negative or wrong layer. `newsedtr` is `alloc3d(ntracer, sednz, nbox)` => `[nbox][sednz][ntracer]` (atlantismain.c:779) and is indexed in sediment-layer space everywhere else (atboundary.c:66, atbiophysics.c:938). Any model with >1 sediment layer writes out of bounds, or loses the sediment-layer results. Function: `Box_Bio_Process()`.

**Code at issue** — `atbiology.c` line 541  
Search for: `newsedtr[bm->current_box][ij - numwclayer][k] =`

```c
  539: 			/* Transfer all of the values back to the final locations */
  540: 			for (k = 0; k < numwcvar; k++)
  541→ 				newsedtr[bm->current_box][ij - numwclayer][k] = boxLayerInfo->localSEDTracers[k]; /* To WC */
  542: 			for (k = 0; k < numdiagvar; k++)
  543: 				pBox->diagnost[k] = boxLayerInfo->localDiagTracers[k]; /* To diagnostics */
```

**Code at issue** — `atbiology.c` line 1083 (closest match to the quoted code; wording differs slightly)  
Search for: `localTracers[i] = bm->boxes[bm->current_box].sm.tr[bm->current_layer][i];`

```c
 1081: 			localTracers[i] = 0.0;
 1082:         } else {
 1083→ 			localTracers[i] = bm->boxes[bm->current_box].sm.tr[bm->current_layer][i];
 1084:         }
 1085: 		localFlux[i] = 0;
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
**Found at (current code):** `atbiology.c` 1534
**Lines at review (Oct 2026):** 1474-1481

**Function:** `ReInitDietData()`

**Why it's thought to be a bug**

`for (cohort = 0; cohort < numCohortsXnumGenes * numStocks; cohort++) bm->totDiet[b][k][sp][cohort][prey] = 0.0;` — `totDiet` is `alloc5d(K_num_tot_sp, K_num_max_cohort, K_num_tot_sp, wcnz+sednz, nbox)` => cohort dim = `K_num_max_cohort` (atlantismain.c:1318), and K_num_max_cohort is the max of `numCohorts` (atUtilGroupIO.c:302). Any group with >1 stock or >1 genotype writes past the cohort dimension. That corrupts other entries, and past the end of the block for the last box. Runs only when `flagdietcheck` is on. Function: `ReInitDietData()`.

**Code at issue** — `atbiology.c` line 1534 (the line the review pointed to, found by its text)  
Search for: `for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes * FunctGroupArray[sp].nu`

```c
 1532:         for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 1533:             if(FunctGroupArray[sp].speciesParams[active_id]) {
 1534→                 for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes * FunctGroupArray[sp].numStocks; cohort++) {
 1535:                     for (prey=0; prey < bm->K_num_tot_sp; prey++) {
 1536:                         bm->totDiet[b][k][sp][cohort][prey] = 0.0;
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
**Found at (current code):** `atbiology.c` 810, `atbiology.c` 885, `atbiology.c` 1160
**Lines at review (Oct 2026):** 774-786 vs 1350

**Function:** `Get_Time_Change()`

**Why it's thought to be a bug**

Ice stability check calls `Get_Max_Flux_Change(..., localICETracers, ..., bm->ntracer /*offset*/, ...)`, so `Tolflag`/`Fluxflag` are looked up in the *sediment* flag slots. Integration (`Integrate_Tracer_Variables(..., ICE_BASED, ..., 0 /*offset*/)`, l.1350) and `Copy_ICE_Tracers()` (`Fluxflag[i]`) both use offset 0. The adaptive time step for ice is therefore based on different flags than the ones used to integrate (the l.778 comment already questions this). Function: `Get_Time_Change()`.

**Code at issue** — `atbiology.c` line 810  
Search for: `Get_Max_Flux_Change(`

```c
  808: 	if(flagModel == ICE_BASED ){
  809: 		/* Get the max change in the ice-based variables */
  810→ 		Get_Max_Flux_Change(bm, boxLayerInfo->localICETracers, boxLayerInfo->localICEFlux, bm->ntracer, bm->ntracer, del, &maxIce_del, &ice_ind_unstable_var,
  811: 				llogfp);
  812: 		offset_int = 0;  // Its assumed to be zero in Adapt_Diff_Method().... should it actually be "3 * bm->ntracer" so not accidentally overwriting anything?
```

**Code at issue** — `atbiology.c` line 885  
Search for: `Integrate_Tracer_Variables(`

```c
  883:  *
  884:  */
  885→ static void Integrate_Tracer_Variables(MSEBoxModel *bm, int flagModel, double *tracerArray, double *fluxArray, int numVariables, int offset, double tsz, double dtsz,
  886: 		FILE *llogfp) {
  887: 	int ij, recheck;
```

**Code at issue** — `atbiology.c` line 1160  
Search for: `Copy_ICE_Tracers()`

```c
 1158:  *	\brief Put the bottom ice layer tracers back after the water column layer below it has grazed them,
 1159:  *	so the ice layer processing (which copies from ice.tr) starts from the grazed values.
 1160→  *	Diagnostic tracers are skipped as Copy_ICE_Tracers() zeroed them in the local copy.
 1161:  */
 1162: static void Return_Grazed_ICE_Tracers(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo) {
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
**Found at (current code):** `atbiology.c` 650, `atbiology.c` 503, `atbiology.c` 431
**Lines at review (Oct 2026):** 398-404, 469-642

**Function:** `Box_Bio_Process()`

**Why it's thought to be a bug**

The ice-layer processing (`if(bm->ice_on) { FlagModel = 4; ... }`) sits inside `if (!oceanic_only)`. In boxes deeper than `bm->max_depth` (oceanic_only), ice biology is never run and `newicetr` is never updated, although sea ice can occur over deep boxes. Function: `Box_Bio_Process()`.

**Code at issue** — `atbiology.c` line 650  
Search for: `if(bm->ice_on) {`

```c
  648:         
  649:         /** Now process ice layers. ****************************************************************************************/
  650→ 		if(bm->ice_on) {
  651: 			FlagModel = 4;
  652: 
```

**Code at issue** — `atbiology.c` line 503  
Search for: `if (!oceanic_only)`

```c
  501:     
  502:     /** Now process sediment layers not in contact with water. **/
  503→ 	if (!oceanic_only) {
  504: 		FlagModel = 2;
  505: 
```

**Code at issue** — `atbiology.c` line 431  
Search for: `max_depth`

```c
  429: 
  430: 	/** If in open ocean pelagic only*/
  431→ 	if (current_depth < bm->max_depth) {
  432: 		oceanic_only = 1;
  433: 		stopij = -1;
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
