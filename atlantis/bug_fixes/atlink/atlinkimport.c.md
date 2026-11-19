# atlink/atlinkimport.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [LINK-40](#link-40) | 62 | M/M | wrong index / array | `area / ExportArray[layer].area[nr][nc]` indexes the EwE-grid area array with the box-local overlap indices `n… |  | [ ] |
| [LINK-41](#link-41) | 62 | M/M | logic | Each overlapping EwE cell overwrites (`=`) the box value instead of accumulating its area-weighted contributio… |  | [ ] |
| [LINK-42](#link-42) | 71 | M/M | div-by-zero | `(origval / totval)` — totval is 0 when the group has no biomass in the box, which gives NaN tracers. Function… |  | [ ] |

---

## LINK-40

**wrong index / array** · Conf M / Sev M  
**Found at (current code):** `atlinkimport.c` 62
**Lines at review (Oct 2026):** 62, 71, 81

**Function:** `UnpackArrays()`

**Why it's thought to be a bug**

`area / ExportArray[layer].area[nr][nc]` indexes the EwE-grid area array with the box-local overlap indices `nr,nc` instead of the grid `row,col`, and uses ExportArray rather than ImportArray. That area array is also all zeros (see LINK-30), so this divides by 0. Function: `UnpackArrays()`.

**Code at issue** — `atlinkimport.c` line 62 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->boxes[b].epi[bid] = bm->eweLinkModule->ImportArray[layer].values[row][col] * tonne_2_m`

```c
   60: 												area = bm->boxes[b].export->bcellarea[nr][nc];  // area of box that contributes to cell [nr][nc] - calculated on startup
   61: 												if(FunctGroupArray[fgIndex].habitatType == EPIFAUNA){
   62→ 													bm->boxes[b].epi[bid] = bm->eweLinkModule->ImportArray[layer].values[row][col] * tonne_2_mg * (area /  bm->eweLinkModule->ExportArray[layer].area[nr][nc]) * per_m2_from_per_km2;
   63: 												} else {
   64: 													if(FunctGroupArray[fgIndex].habitatType == WC){
```

**Suggested fix (from review):** Use `ImportArray[layer].area[row][col]` (once populated), with a zero guard.

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

## LINK-41

**logic** · Conf M / Sev M  
**Found at (current code):** `atlinkimport.c` 62
**Lines at review (Oct 2026):** 62, 71

**Function:** `UnpackArrays()`

**Why it's thought to be a bug**

Each overlapping EwE cell overwrites (`=`) the box value instead of accumulating its area-weighted contribution, so only the last cell counts. Function: `UnpackArrays()`.

**Code at issue** — `atlinkimport.c` line 62 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `bm->boxes[b].epi[bid] = bm->eweLinkModule->ImportArray[layer].values[row][col] * tonne_2_m`

```c
   60: 												area = bm->boxes[b].export->bcellarea[nr][nc];  // area of box that contributes to cell [nr][nc] - calculated on startup
   61: 												if(FunctGroupArray[fgIndex].habitatType == EPIFAUNA){
   62→ 													bm->boxes[b].epi[bid] = bm->eweLinkModule->ImportArray[layer].values[row][col] * tonne_2_mg * (area /  bm->eweLinkModule->ExportArray[layer].area[nr][nc]) * per_m2_from_per_km2;
   63: 												} else {
   64: 													if(FunctGroupArray[fgIndex].habitatType == WC){
```

**Suggested fix (from review):** Sum the cell contributions (zero first, then `+=`).

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

## LINK-42

**div-by-zero** · Conf M / Sev M  
**Found at (current code):** `atlinkimport.c` 71
**Lines at review (Oct 2026):** 71, 81

**Function:** `UnpackArrays()`

**Why it's thought to be a bug**

`(origval / totval)` — totval is 0 when the group has no biomass in the box, which gives NaN tracers. Function: `UnpackArrays()`.

**Code at issue** — `atlinkimport.c` line 71  
Search for: `(origval / totval)`

```c
   69: 														for(k=0; k<bm->boxes[b].nz; k++){
   70: 															origval = bm->boxes[b].tr[k][bid] * bm->boxes[b].dz[k];
   71→ 															bm->boxes[b].tr[k][bid] = bm->eweLinkModule->ImportArray[layer].values[row][col] * tonne_2_mg * (area / bm->eweLinkModule->ExportArray[layer].area[nr][nc]) * per_m2_from_per_km2 * (origval / totval) / bm->boxes[b].dz[k];
   72: 														}
   73: 													}
```

**Suggested fix (from review):** If totval == 0, distribute by dz or assign to one layer.

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
