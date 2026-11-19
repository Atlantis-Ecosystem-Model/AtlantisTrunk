# atlink/atlinkimport.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [LINK-40](#link-40) | 62, 71, 81 | M/M | wrong index / array | `area / ExportArray[layer].area[nr][nc]` indexes the EwE-grid area array with the box-local overlap indices `n… |  | [ ] |
| [LINK-41](#link-41) | 62, 71 | M/M | logic | Each overlapping EwE cell overwrites (`=`) the box value instead of accumulating its area-weighted contributio… |  | [ ] |
| [LINK-42](#link-42) | 71, 81 | M/M | div-by-zero | `(origval / totval)` — totval is 0 when the group has no biomass in the box, which gives NaN tracers. Function… |  | [ ] |

---

## LINK-40

**wrong index / array** · Conf M / Sev M  
**Lines (current code):** 62, 71, 81  
**Lines at review (Oct 2026):** 62, 71, 81

**Function:** `UnpackArrays()`

**Why it's thought to be a bug**

`area / ExportArray[layer].area[nr][nc]` indexes the EwE-grid area array with the box-local overlap indices `nr,nc` instead of the grid `row,col`, and uses ExportArray rather than ImportArray. That area array is also all zeros (see LINK-30), so this divides by 0. Function: `UnpackArrays()`.

**Current code** (`atlink/atlinkimport.c` lines 60–83)

```c
   60: 												area = bm->boxes[b].export->bcellarea[nr][nc];  // area of box that contributes to cell [nr][nc] - calculated on startup
   61: 												if(FunctGroupArray[fgIndex].habitatType == EPIFAUNA){
   62: 													bm->boxes[b].epi[bid] = bm->eweLinkModule->ImportArray[layer].values[row][col] * tonne_2_mg * (area /  bm->eweLinkModule->ExportArray[layer].area[nr][nc]) * per_m2_from_per_km2;
   63: 												} else {
   64: 													if(FunctGroupArray[fgIndex].habitatType == WC){
   65: 														totval = 0;
   66: 														for(k=0; k<bm->boxes[b].nz; k++){
   67: 															totval += bm->boxes[b].tr[k][bid] * bm->boxes[b].dz[k];
   68: 														}
   69: 														for(k=0; k<bm->boxes[b].nz; k++){
   70: 															origval = bm->boxes[b].tr[k][bid] * bm->boxes[b].dz[k];
   71: 															bm->boxes[b].tr[k][bid] = bm->eweLinkModule->ImportArray[layer].values[row][col] * tonne_2_mg * (area / bm->eweLinkModule->ExportArray[layer].area[nr][nc]) * per_m2_from_per_km2 * (origval / totval) / bm->boxes[b].dz[k];
   72: 														}
   73: 													}
   74: 													if(FunctGroupArray[fgIndex].habitatType == SED){
   75: 														totval = 0;
   76: 														for(k=0; k<bm->boxes[b].sm.nz; k++){
   77: 															totval += bm->boxes[b].sm.tr[k][bid] * bm->boxes[b].sm.dz[k];
   78: 														}
   79: 														for(k=0; k<bm->boxes[b].sm.nz; k++){
   80: 															origval = bm->boxes[b].sm.tr[k][bid] * bm->boxes[b].sm.dz[k];
   81: 															bm->boxes[b].sm.tr[k][bid] = bm->eweLinkModule->ImportArray[layer].values[row][col] * tonne_2_mg * (area / bm->eweLinkModule->ExportArray[layer].area[nr][nc]) * per_m2_from_per_km2 * (origval / totval) / bm->boxes[b].sm.dz[k];
   82: 														}
   83: 													}
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
**Lines (current code):** 62, 71  
**Lines at review (Oct 2026):** 62, 71

**Function:** `UnpackArrays()`

**Why it's thought to be a bug**

Each overlapping EwE cell overwrites (`=`) the box value instead of accumulating its area-weighted contribution, so only the last cell counts. Function: `UnpackArrays()`.

**Current code** (`atlink/atlinkimport.c` lines 60–73)

```c
   60: 												area = bm->boxes[b].export->bcellarea[nr][nc];  // area of box that contributes to cell [nr][nc] - calculated on startup
   61: 												if(FunctGroupArray[fgIndex].habitatType == EPIFAUNA){
   62: 													bm->boxes[b].epi[bid] = bm->eweLinkModule->ImportArray[layer].values[row][col] * tonne_2_mg * (area /  bm->eweLinkModule->ExportArray[layer].area[nr][nc]) * per_m2_from_per_km2;
   63: 												} else {
   64: 													if(FunctGroupArray[fgIndex].habitatType == WC){
   65: 														totval = 0;
   66: 														for(k=0; k<bm->boxes[b].nz; k++){
   67: 															totval += bm->boxes[b].tr[k][bid] * bm->boxes[b].dz[k];
   68: 														}
   69: 														for(k=0; k<bm->boxes[b].nz; k++){
   70: 															origval = bm->boxes[b].tr[k][bid] * bm->boxes[b].dz[k];
   71: 															bm->boxes[b].tr[k][bid] = bm->eweLinkModule->ImportArray[layer].values[row][col] * tonne_2_mg * (area / bm->eweLinkModule->ExportArray[layer].area[nr][nc]) * per_m2_from_per_km2 * (origval / totval) / bm->boxes[b].dz[k];
   72: 														}
   73: 													}
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
**Lines (current code):** 71, 81  
**Lines at review (Oct 2026):** 71, 81

**Function:** `UnpackArrays()`

**Why it's thought to be a bug**

`(origval / totval)` — totval is 0 when the group has no biomass in the box, which gives NaN tracers. Function: `UnpackArrays()`.

**Current code** (`atlink/atlinkimport.c` lines 69–83)

```c
   69: 														for(k=0; k<bm->boxes[b].nz; k++){
   70: 															origval = bm->boxes[b].tr[k][bid] * bm->boxes[b].dz[k];
   71: 															bm->boxes[b].tr[k][bid] = bm->eweLinkModule->ImportArray[layer].values[row][col] * tonne_2_mg * (area / bm->eweLinkModule->ExportArray[layer].area[nr][nc]) * per_m2_from_per_km2 * (origval / totval) / bm->boxes[b].dz[k];
   72: 														}
   73: 													}
   74: 													if(FunctGroupArray[fgIndex].habitatType == SED){
   75: 														totval = 0;
   76: 														for(k=0; k<bm->boxes[b].sm.nz; k++){
   77: 															totval += bm->boxes[b].sm.tr[k][bid] * bm->boxes[b].sm.dz[k];
   78: 														}
   79: 														for(k=0; k<bm->boxes[b].sm.nz; k++){
   80: 															origval = bm->boxes[b].sm.tr[k][bid] * bm->boxes[b].sm.dz[k];
   81: 															bm->boxes[b].sm.tr[k][bid] = bm->eweLinkModule->ImportArray[layer].values[row][col] * tonne_2_mg * (area / bm->eweLinkModule->ExportArray[layer].area[nr][nc]) * per_m2_from_per_km2 * (origval / totval) / bm->boxes[b].sm.dz[k];
   82: 														}
   83: 													}
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
