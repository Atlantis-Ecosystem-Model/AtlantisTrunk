# atlink/atlinkimport.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [LINK-40](#link-40) | 62, 71, 81 | M/M | wrong index / array | `area / ExportArray[layer].area[nr][nc]` indexes the EwE-grid area array with the box-local overlap indices `n… |  | [ ] |
| [LINK-41](#link-41) | 62, 71 | M/M | logic | Each overlapping EwE cell overwrites (`=`) the box value instead of accumulating its area-weighted contributio… |  | [ ] |
| [LINK-42](#link-42) | 71, 81 | M/M | div-by-zero | `(origval / totval)` — totval is 0 when the group has no biomass in the box, which gives NaN tracers. Function… |  | [ ] |

---

## LINK-40

**wrong index / array** · Conf M / Sev M  
**Listed lines:** 62, 71, 81

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
**Listed lines:** 62, 71

**Function:** `UnpackArrays()`

**Why it's thought to be a bug**

Each overlapping EwE cell overwrites (`=`) the box value instead of accumulating its area-weighted contribution, so only the last cell counts. Function: `UnpackArrays()`.

**Current code** (`atlink/atlinkimport.c` lines 33–46) — code appears to have moved -27 lines since the review

```c
   33: */
   34: 
   35: void UnpackArrays(MSEBoxModel *bm)
   36: {
   37: 	int fgIndex, layer=0, b=0, nr=0, nc=0, row=0, col=0, k=0;
   38: 	double area=0, totval=0, origval;
   39: 	int cohort, bid;
   40: 
   41: 	/* Update the array */
   42: 	switch(bm->eweLinkModule->convertPRMS.spacehandling){
   43: 		case WEIGHTED_AVERAGE:
   44: 			for(fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++){
   45: 			/* Note only running for biomass pools for now.
   46: 			   More consultation needed with Villy for age structured groups as will
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
**Listed lines:** 71, 81

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
