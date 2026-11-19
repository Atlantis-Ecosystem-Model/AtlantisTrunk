# atbrokerlink/ImportExportData.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **7 findings** (severity H: 3; confidence H and severity H: 2; on the priority list: 2).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [LINK-58](#link-58) ⭐ | 312, 318 | H/H | wrong index | `linkageWCDetritusFlux[b][k][guild] = boxValues[b] / 100 * ...` — `boxValues` has `numBoxes` entries indexed b… |  | [ ] |
| [LINK-59](#link-59) ⭐ | 88, 126, 144 | H/H | accumulator not reset | `epi = 0` is only set in the first (fraction) loop. In the second loop `epi += boxValues[i]` keeps accumulatin… |  | [ ] |
| [LINK-60](#link-60) | 56-57, 78-109, 130-135 | M/M | missing dimension | `columnFractions[b][k]` / `sedColumnFractions[b][k]` have no cohort dimension. They are computed inside the co… |  | [ ] |
| [LINK-61](#link-61) | 93-108, 130-135 vs 220-230 | M/H | unit mismatch | Link_GetBiomass returns `tr * mg_2_tonne * dz` (t/m2), but Link_SetBiomass distributes the incoming value dire… |  | [ ] |
| [LINK-62](#link-62) | 153-163 | M/M | wrong condition | The WC update loop runs for every non-EPIFAUNA group regardless of `habitatCoeffs[WC]`. For a sediment-only gr… |  | [ ] |
| [LINK-63](#link-63) | 51, 145-149 | L/L | logic | With `tolerance = 1.0`, `+= tolerance * difference` sets the value equal to the imported one, so the "pruning"… |  | [ ] |
| [LINK-64](#link-64) | 263, 390 | H/L | copy-paste message | Link_SetDetritus logs "Link_SetBiomass"; Link_SetMortality reports "Link_GetBiomass - group code not recognise… |  | [ ] |

---

## LINK-58

**wrong index** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 312, 318  
**Lines at review (Oct 2026):** 312, 318

**Function:** `Link_SetDetritus()`

**Why it's thought to be a bug**

`linkageWCDetritusFlux[b][k][guild] = boxValues[b] / 100 * ...` — `boxValues` has `numBoxes` entries indexed by the polygon position `i` (compare `boxValues[i]` in Link_SetBiomass:126-135 and Link_SetMortality:401-410), but it is indexed by the box id `b`. This gives wrong values, or out-of-bounds reads when b >= numBoxes. Function: `Link_SetDetritus()`.

**Current code** (`atbrokerlink/ImportExportData.c` lines 310–320)

```c
  310: 			if (FunctGroupArray[guild].habitatCoeffs[WC] > 0) {
  311: 				for (k = 0; k < bm->boxes[b].nz; k++) {
  312: 					bm->linkageInterface->linkageWCDetritusFlux[b][k][guild] = boxValues[b] / 100 * wcColumnFractions[b][k] / bm->boxes[b].dz[k]; /* Bec change 18th Oct */
  313: 
  314: 				}
  315: 			}
  316: 			if (FunctGroupArray[guild].habitatCoeffs[SED] > 0) {
  317: 				for (k = 0; k < bm->boxes[b].sm.nz; k++) {
  318: 					bm->linkageInterface->linkageSEDDetritusFlux[b][k][guild] = boxValues[b] / 100 * sedColumnFractions[b][k] / bm->boxes[b].sm.dz[k]; /* Bec change 18th Oct */
  319: 				}
  320: 			}
```

**Suggested fix (from review):** Use `boxValues[i]`.

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

## LINK-59

**accumulator not reset** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 88, 126, 144  
**Lines at review (Oct 2026):** 88, 126, 144

**Function:** `Link_SetBiomass()`

**Why it's thought to be a bug**

`epi = 0` is only set in the first (fraction) loop. In the second loop `epi += boxValues[i]` keeps accumulating across boxes and cohorts, so box n is set to the sum of boxes 0..n. Function: `Link_SetBiomass()`.

**Current code** (`atbrokerlink/ImportExportData.c` lines 86–125 (truncated)) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
   86: 			if (FunctGroupArray[guild].habitatType == EPIFAUNA) {
   87: 				totalValue[b][cohort] = bm->boxes[b].epi[bid];
   88: 				epi = 0;
   89: 			} else {
   90: 
   91: 				if (FunctGroupArray[guild].habitatCoeffs[WC] > 0) {
   92: 					for (k = 0; k < bm->boxes[b].nz; k++)
   93: 						totalValue[b][cohort] += bm->boxes[b].tr[k][bid];
   94: 				}
   95: 				if (FunctGroupArray[guild].habitatCoeffs[SED] > 0) {
   96: 					for (k = 0; k < bm->boxes[b].sm.nz; k++)
   97: 						totalValue[b][cohort] += bm->boxes[b].sm.tr[k][bid];
   98: 				}
   99: 
  100: 				/* Now work out the fractions */
  101: 				if (FunctGroupArray[guild].habitatCoeffs[WC] > 0) {
  102: 					for (k = 0; k < bm->boxes[b].nz; k++)
  103: 						columnFractions[b][k] = totalValue[b][cohort] ? (bm->boxes[b].tr[k][bid] / totalValue[b][cohort]) : 0.0;
  104: 				}
  105: 
  106: 				if (FunctGroupArray[guild].habitatCoeffs[SED] > 0) {
  107: 					for (k = 0; k < bm->boxes[b].sm.nz; k++)
  108: 						sedColumnFractions[b][k] = totalValue[b][cohort] ? (bm->boxes[b].sm.tr[k][bid] / totalValue[b][cohort]) : 0.0;
  109: 				}
  110: 			}
  111: 			fprintf(bm->logFile, "totalValue[%d][%d] = %e\n", b, cohort, totalValue[b][cohort]);
  112: 		}
  113: 	}
  114: 
  115: 	/* Now divide up the biomass given into each column based on columnFraction values */
  116: 	for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
  117: 		bid = FunctGroupArray[guild].totNTracers[cohort];
  118: 
  119: 		for (i = 0; i < numBoxes; i++) {
  120: 			b = boxIndexValues[i];
  121: 
  122: 			Util_Init_1D_Double(tr, bm->boxes[b].nz, 0.0);
  123: 			Util_Init_1D_Double(smtr, bm->boxes[b].sm.nz, 0.0);
  124: 			if (FunctGroupArray[guild].habitatType == EPIFAUNA) {
  125: 				/* bm->boxes[b].epi[bid] += */
```

**Suggested fix (from review):** Set `epi = boxValues[i];` (or reset epi = 0 at the start of each box iteration).

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

## LINK-60

**missing dimension** · Conf M / Sev M  
**Lines (current code):** 56-57, 78-109, 130-135  
**Lines at review (Oct 2026):** 56-57, 78-109, 130-135

**Function:** `Link_SetBiomass()`

**Why it's thought to be a bug**

`columnFractions[b][k]` / `sedColumnFractions[b][k]` have no cohort dimension. They are computed inside the cohort loop and overwritten, so the second loop uses the last cohort's vertical distribution for every cohort. Function: `Link_SetBiomass()`.

**Current code** (`atbrokerlink/ImportExportData.c` lines 54–93 (truncated))

```c
   54: 	double *tr = Util_Alloc_Init_1D_Double(bm->wcnz, 0.0);
   55: 	double *smtr = Util_Alloc_Init_1D_Double(bm->sednz, 0.0);
   56: 	double **columnFractions = Util_Alloc_Init_2D_Double(bm->wcnz, bm->nbox, 0.0);
   57: 	double **sedColumnFractions = Util_Alloc_Init_2D_Double(bm->sednz, bm->nbox, 0.0);
   58: 	double **totalValue = Util_Alloc_Init_2D_Double(bm->K_num_max_cohort * bm->K_num_max_genetypes, bm->nbox, 0.0);
   59: 
   60: 	/* Grab the group code */
   61: 	guild = Util_Get_FG_Index(bm, groupCode);
   62: 	if (guild == -1) {
   63: 		fprintf(stderr, "Linkage Link_SetBiomass - group code %s not recognised\n", groupCode);
   64: 		quit("");
   65: 	}
   66: 
   67: 	fprintf(bm->logFile, "Link_SetBiomass \n");
   68: 
   69: 	/* Initialise column fractions outside main logic, because
   70: 	 * they are need for all types of calculations: we may
   71: 	 * need to initialse the array containing the original
   72: 	 * fraction of biomass for that particular level in the
   73: 	 * water column, for those whose habitat is vertically
   74: 	 * distributed, to redistribute the imported value in the
   75: 	 * same proportions.  For safety, if there is no biomass
   76: 	 * left in that box (ie, a total of 0) we initialise to 0
   77: 	 * to avoid NaN or div-by-zero errors. */
   78: 	for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
   79: 		bid = FunctGroupArray[guild].totNTracers[cohort];
   80: 
   81: 		for (i = 0; i < numBoxes; i++) {
   82: 			b = boxIndexValues[i];
   83: 
   84: 			totalValue[b][cohort] = 0.0;
   85: 
   86: 			if (FunctGroupArray[guild].habitatType == EPIFAUNA) {
   87: 				totalValue[b][cohort] = bm->boxes[b].epi[bid];
   88: 				epi = 0;
   89: 			} else {
   90: 
   91: 				if (FunctGroupArray[guild].habitatCoeffs[WC] > 0) {
   92: 					for (k = 0; k < bm->boxes[b].nz; k++)
   93: 						totalValue[b][cohort] += bm->boxes[b].tr[k][bid];
```

**Suggested fix (from review):** Compute the fractions inside the second cohort loop, or add a cohort dimension.

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

## LINK-61

**unit mismatch** · Conf M / Sev H  
**Lines (current code):** 93-108, 130-135 vs 220-230  
**Lines at review (Oct 2026):** 93-108, 130-135 vs 220-230

**Why it's thought to be a bug**

Link_GetBiomass returns `tr * mg_2_tonne * dz` (t/m2), but Link_SetBiomass distributes the incoming value directly as concentration: `tr[k] = boxValue * (tr[k]/sum(tr))`, with no tonne_2_mg and no /dz, and the fractions are computed from concentrations rather than mass. A get→set round trip changes biomass by about 1e9/dz. Functions: `Link_SetBiomass()`, `Link_GetBiomass()`.

**Current code** (`atbrokerlink/ImportExportData.c` lines 91–130 (truncated))

```c
   91: 				if (FunctGroupArray[guild].habitatCoeffs[WC] > 0) {
   92: 					for (k = 0; k < bm->boxes[b].nz; k++)
   93: 						totalValue[b][cohort] += bm->boxes[b].tr[k][bid];
   94: 				}
   95: 				if (FunctGroupArray[guild].habitatCoeffs[SED] > 0) {
   96: 					for (k = 0; k < bm->boxes[b].sm.nz; k++)
   97: 						totalValue[b][cohort] += bm->boxes[b].sm.tr[k][bid];
   98: 				}
   99: 
  100: 				/* Now work out the fractions */
  101: 				if (FunctGroupArray[guild].habitatCoeffs[WC] > 0) {
  102: 					for (k = 0; k < bm->boxes[b].nz; k++)
  103: 						columnFractions[b][k] = totalValue[b][cohort] ? (bm->boxes[b].tr[k][bid] / totalValue[b][cohort]) : 0.0;
  104: 				}
  105: 
  106: 				if (FunctGroupArray[guild].habitatCoeffs[SED] > 0) {
  107: 					for (k = 0; k < bm->boxes[b].sm.nz; k++)
  108: 						sedColumnFractions[b][k] = totalValue[b][cohort] ? (bm->boxes[b].sm.tr[k][bid] / totalValue[b][cohort]) : 0.0;
  109: 				}
  110: 			}
  111: 			fprintf(bm->logFile, "totalValue[%d][%d] = %e\n", b, cohort, totalValue[b][cohort]);
  112: 		}
  113: 	}
  114: 
  115: 	/* Now divide up the biomass given into each column based on columnFraction values */
  116: 	for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
  117: 		bid = FunctGroupArray[guild].totNTracers[cohort];
  118: 
  119: 		for (i = 0; i < numBoxes; i++) {
  120: 			b = boxIndexValues[i];
  121: 
  122: 			Util_Init_1D_Double(tr, bm->boxes[b].nz, 0.0);
  123: 			Util_Init_1D_Double(smtr, bm->boxes[b].sm.nz, 0.0);
  124: 			if (FunctGroupArray[guild].habitatType == EPIFAUNA) {
  125: 				/* bm->boxes[b].epi[bid] += */
  126: 				epi += boxValues[i];
  127: 			} else {
  128: 				if (FunctGroupArray[guild].habitatCoeffs[WC] > 0) {
  129: 					for (k = 0; k < bm->boxes[b].nz; k++) {
  130: 						tr[k] += boxValues[i] * columnFractions[b][k];
```

**Suggested fix (from review):** Convert units symmetrically: fractions from tr*dz, then `tr[k] = val*tonne_2_mg*frac/dz[k]`.

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

## LINK-62

**wrong condition** · Conf M / Sev M  
**Lines (current code):** 153-163  
**Lines at review (Oct 2026):** 153-163

**Function:** `Link_SetBiomass()`

**Why it's thought to be a bug**

The WC update loop runs for every non-EPIFAUNA group regardless of `habitatCoeffs[WC]`. For a sediment-only group, `tr[k]` is 0, which can zero or prune existing WC tracer values. Compare the SED block, which is guarded (line 165). Function: `Link_SetBiomass()`.

**Current code** (`atbrokerlink/ImportExportData.c` lines 151–165)

```c
  151: 					bm->boxes[b].epi[bid] = epi;
  152: 			} else {
  153: 				for (k = 0; k < bm->boxes[b].nz; k++) {
  154: 					difference = tr[k] - bm->boxes[b].tr[k][bid];
  155: 					if (fabs(difference) / (bm->boxes[b].tr[k][bid] + small_num) > tolerance) {
  156: 						warn("box %2d:group %s cohort %d: WC, layer %d: Imported value %lf and current value %lf are too far apart; pruning.\n", b, groupCode,
  157: 								cohort, k, tr[k], bm->boxes[b].tr[k][bid]);
  158: 
  159: 						/* note if difference is negative this subtracts from the existing value, which is what we want: */
  160: 						bm->boxes[b].tr[k][bid] += tolerance * difference;
  161: 					} else
  162: 						bm->boxes[b].tr[k][bid] = tr[k];
  163: 				}
  164: 
  165: 				if (FunctGroupArray[guild].habitatCoeffs[SED] > 0) {
```

**Suggested fix (from review):** Wrap it in `if (habitatCoeffs[WC] > 0)`.

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

## LINK-63

**logic** · Conf L / Sev L  
**Lines (current code):** 51, 145-149  
**Lines at review (Oct 2026):** 51, 145-149

**Function:** `Link_SetBiomass()`

**Why it's thought to be a bug**

With `tolerance = 1.0`, `+= tolerance * difference` sets the value equal to the imported one, so the "pruning" branch does exactly what the else branch does and only adds a warning. Function: `Link_SetBiomass()`.

**Current code** (`atbrokerlink/ImportExportData.c` lines 49–53)

```c
   49: int Link_SetBiomass(MSEBoxModel *bm, char *groupCode, int numBoxes, int *boxIndexValues, double *boxValues) {
   50: 
   51: 	double tolerance = 1.0;
   52: 	int cohort, guild, i, b, k, bid;
   53: 	double epi, difference;
```

**Suggested fix (from review):** Use a tolerance < 1 or cap the step as intended.

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

## LINK-64

**copy-paste message** · Conf H / Sev L  
**Lines (current code):** 263, 390  
**Lines at review (Oct 2026):** 263, 390

**Why it's thought to be a bug**

Link_SetDetritus logs "Link_SetBiomass"; Link_SetMortality reports "Link_GetBiomass - group code not recognised". Functions: `Link_SetDetritus()`, `Link_SetMortality()`.

**Current code** (`atbrokerlink/ImportExportData.c` lines 261–265) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
  261: 	}
  262: 
  263: 	fprintf(bm->logFile, "Link_SetBiomass \n");
  264: 
  265: 	/* Initialise column fractions outside main logic, because
```

**Suggested fix (from review):** Fix the function names in the messages.

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
