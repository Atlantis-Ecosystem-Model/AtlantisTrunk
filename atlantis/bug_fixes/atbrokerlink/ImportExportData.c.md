# atbrokerlink/ImportExportData.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **7 findings** (severity H: 3; confidence H and severity H: 2; on the priority list: 2).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [LINK-58](#link-58) ⭐ | 312, 229 | H/H | wrong index | `linkageWCDetritusFlux[b][k][guild] = boxValues[b] / 100 * ...` — `boxValues` has `numBoxes` entries indexed b… |  | [ ] |
| [LINK-59](#link-59) ⭐ | 126 | H/H | accumulator not reset | `epi = 0` is only set in the first (fraction) loop. In the second loop `epi += boxValues[i]` keeps accumulatin… |  | [ ] |
| [LINK-60](#link-60) | 103, 108 | M/M | missing dimension | `columnFractions[b][k]` / `sedColumnFractions[b][k]` have no cohort dimension. They are computed inside the co… |  | [ ] |
| [LINK-61](#link-61) | 224 | M/H | unit mismatch | Link_GetBiomass returns `tr * mg_2_tonne * dz` (t/m2), but Link_SetBiomass distributes the incoming value dire… |  | [ ] |
| [LINK-62](#link-62) | 128 | M/M | wrong condition | The WC update loop runs for every non-EPIFAUNA group regardless of `habitatCoeffs[WC]`. For a sediment-only gr… |  | [ ] |
| [LINK-63](#link-63) | 51, 149 | L/L | logic | With `tolerance = 1.0`, `+= tolerance * difference` sets the value equal to the imported one, so the "pruning"… |  | [ ] |
| [LINK-64](#link-64) | 351 | H/L | copy-paste message | Link_SetDetritus logs "Link_SetBiomass"; Link_SetMortality reports "Link_GetBiomass - group code not recognise… |  | [ ] |

---

## LINK-58

**wrong index** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `ImportExportData.c` 312, `ImportExportData.c` 229
**Lines at review (Oct 2026):** 312, 318

**Function:** `Link_SetDetritus()`

**Why it's thought to be a bug**

`linkageWCDetritusFlux[b][k][guild] = boxValues[b] / 100 * ...` — `boxValues` has `numBoxes` entries indexed by the polygon position `i` (compare `boxValues[i]` in Link_SetBiomass:126-135 and Link_SetMortality:401-410), but it is indexed by the box id `b`. This gives wrong values, or out-of-bounds reads when b >= numBoxes. Function: `Link_SetDetritus()`.

**Code at issue** — `ImportExportData.c` line 312  
Search for: `linkageWCDetritusFlux[b][k][guild] = boxValues[b] / 100 *`

```c
  310: 			if (FunctGroupArray[guild].habitatCoeffs[WC] > 0) {
  311: 				for (k = 0; k < bm->boxes[b].nz; k++) {
  312→ 					bm->linkageInterface->linkageWCDetritusFlux[b][k][guild] = boxValues[b] / 100 * wcColumnFractions[b][k] / bm->boxes[b].dz[k]; /* Bec change 18th Oct */
  313: 
  314: 				}
```

**Code at issue** — `ImportExportData.c` line 229  
Search for: `boxValues[i]`

```c
  227: 				if (FunctGroupArray[guild].habitatCoeffs[SED] > 0) {
  228: 					for (k = 0; k < bm->boxes[b].sm.nz; k++) {
  229→ 						boxValues[i] += bm->boxes[b].sm.tr[k][bid] * mg_2_tonne * bm->boxes[b].sm.dz[k];
  230: 					}
  231: 				}
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
**Found at (current code):** `ImportExportData.c` 126
**Lines at review (Oct 2026):** 88, 126, 144

**Function:** `Link_SetBiomass()`

**Why it's thought to be a bug**

`epi = 0` is only set in the first (fraction) loop. In the second loop `epi += boxValues[i]` keeps accumulating across boxes and cohorts, so box n is set to the sum of boxes 0..n. Function: `Link_SetBiomass()`.

**Code at issue** — `ImportExportData.c` line 126  
Search for: `epi += boxValues[i]`

```c
  124: 			if (FunctGroupArray[guild].habitatType == EPIFAUNA) {
  125: 				/* bm->boxes[b].epi[bid] += */
  126→ 				epi += boxValues[i];
  127: 			} else {
  128: 				if (FunctGroupArray[guild].habitatCoeffs[WC] > 0) {
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
**Found at (current code):** `ImportExportData.c` 103, `ImportExportData.c` 108
**Lines at review (Oct 2026):** 56-57, 78-109, 130-135

**Function:** `Link_SetBiomass()`

**Why it's thought to be a bug**

`columnFractions[b][k]` / `sedColumnFractions[b][k]` have no cohort dimension. They are computed inside the cohort loop and overwritten, so the second loop uses the last cohort's vertical distribution for every cohort. Function: `Link_SetBiomass()`.

**Code at issue** — `ImportExportData.c` line 103  
Search for: `columnFractions[b][k]`

```c
  101: 				if (FunctGroupArray[guild].habitatCoeffs[WC] > 0) {
  102: 					for (k = 0; k < bm->boxes[b].nz; k++)
  103→ 						columnFractions[b][k] = totalValue[b][cohort] ? (bm->boxes[b].tr[k][bid] / totalValue[b][cohort]) : 0.0;
  104: 				}
  105: 
```

**Code at issue** — `ImportExportData.c` line 108  
Search for: `sedColumnFractions[b][k]`

```c
  106: 				if (FunctGroupArray[guild].habitatCoeffs[SED] > 0) {
  107: 					for (k = 0; k < bm->boxes[b].sm.nz; k++)
  108→ 						sedColumnFractions[b][k] = totalValue[b][cohort] ? (bm->boxes[b].sm.tr[k][bid] / totalValue[b][cohort]) : 0.0;
  109: 				}
  110: 			}
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
**Found at (current code):** `ImportExportData.c` 224
**Lines at review (Oct 2026):** 93-108, 130-135 vs 220-230

**Why it's thought to be a bug**

Link_GetBiomass returns `tr * mg_2_tonne * dz` (t/m2), but Link_SetBiomass distributes the incoming value directly as concentration: `tr[k] = boxValue * (tr[k]/sum(tr))`, with no tonne_2_mg and no /dz, and the fractions are computed from concentrations rather than mass. A get→set round trip changes biomass by about 1e9/dz. Functions: `Link_SetBiomass()`, `Link_GetBiomass()`.

**Code at issue** — `ImportExportData.c` line 224 (closest match to the quoted code; wording differs slightly)  
Search for: `boxValues[i] += bm->boxes[b].tr[k][bid] * mg_2_tonne * bm->boxes[b].dz[k];`

```c
  222: 				if (FunctGroupArray[guild].habitatCoeffs[WC] > 0) {
  223: 					for (k = 0; k < bm->boxes[b].nz; k++) {
  224→ 						boxValues[i] += bm->boxes[b].tr[k][bid] * mg_2_tonne * bm->boxes[b].dz[k];
  225: 					}
  226: 				}
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
**Found at (current code):** `ImportExportData.c` 128
**Lines at review (Oct 2026):** 153-163

**Function:** `Link_SetBiomass()`

**Why it's thought to be a bug**

The WC update loop runs for every non-EPIFAUNA group regardless of `habitatCoeffs[WC]`. For a sediment-only group, `tr[k]` is 0, which can zero or prune existing WC tracer values. Compare the SED block, which is guarded (line 165). Function: `Link_SetBiomass()`.

**Code at issue** — `ImportExportData.c` line 128  
Search for: `habitatCoeffs[WC]`

```c
  126: 				epi += boxValues[i];
  127: 			} else {
  128→ 				if (FunctGroupArray[guild].habitatCoeffs[WC] > 0) {
  129: 					for (k = 0; k < bm->boxes[b].nz; k++) {
  130: 						tr[k] += boxValues[i] * columnFractions[b][k];
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
**Found at (current code):** `ImportExportData.c` 51, `ImportExportData.c` 149
**Lines at review (Oct 2026):** 51, 145-149

**Function:** `Link_SetBiomass()`

**Why it's thought to be a bug**

With `tolerance = 1.0`, `+= tolerance * difference` sets the value equal to the imported one, so the "pruning" branch does exactly what the else branch does and only adds a warning. Function: `Link_SetBiomass()`.

**Code at issue** — `ImportExportData.c` line 51  
Search for: `tolerance = 1.0`

```c
   49: int Link_SetBiomass(MSEBoxModel *bm, char *groupCode, int numBoxes, int *boxIndexValues, double *boxValues) {
   50: 
   51→ 	double tolerance = 1.0;
   52: 	int cohort, guild, i, b, k, bid;
   53: 	double epi, difference;
```

**Code at issue** — `ImportExportData.c` line 149  
Search for: `+= tolerance * difference`

```c
  147: 							epi, bm->boxes[b].epi[bid]);
  148: 					/* note if difference is negative this subtracts from the existing value, which is what we want: */
  149→ 					bm->boxes[b].epi[bid] += tolerance * difference;
  150: 				} else
  151: 					bm->boxes[b].epi[bid] = epi;
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
**Found at (current code):** `ImportExportData.c` 351
**Lines at review (Oct 2026):** 263, 390

**Why it's thought to be a bug**

Link_SetDetritus logs "Link_SetBiomass"; Link_SetMortality reports "Link_GetBiomass - group code not recognised". Functions: `Link_SetDetritus()`, `Link_SetMortality()`.

**Code at issue** — `ImportExportData.c` line 351 (located by name only; check this is the right place)  
Search for: `int Link_SetMortality(MSEBoxModel *bm, char *groupCode, int numBoxes, int *boxIndexValues,`

```c
  349: }
  350: 
  351→ int Link_SetMortality(MSEBoxModel *bm, char *groupCode, int numBoxes, int *boxIndexValues, double *boxValues) {
  352: 	int b, i, cohort, guild, bid, k;
  353: 
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
