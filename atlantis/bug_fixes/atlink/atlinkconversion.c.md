# atlink/atlinkconversion.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **6 findings** (severity H: 3; confidence H and severity H: 3; on the priority list: 3), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [LINK-27](#link-27) ⭐ | 41 | H/H | wrong variable / NULL deref | `Util_Init_2D_Double(..., eweLinkModule->convertPRMS.io_nrow, ...)` uses the global `eweLinkModule` (declared … |  | [ ] |
| [LINK-28](#link-28) ⭐ | 212, 223, 229 | H/H | wrong variable | `nid = *PhysioChemArray[fgIndex].tracerIndex;` is computed but the nutrient and physchem blocks read `tr[k][bi… |  | [ ] |
| [LINK-29](#link-29) | 185 | H/M | wrong variable | Sediment P/B: `bm->boxes[b].sm.tr[k][pid] / (bm->boxes[b].tr[k][bid] + small_num)` divides sediment production… |  | [ ] |
| [LINK-30](#link-30) ⭐ | 266 | H/H | div-by-zero | `values[r][c] /= ExportArray[l].area[r][c]; // area created on readin` — `area` is allocated with 0.0 (atlinke… |  | [ ] |
| [LINK-31](#link-31) | 141, 147 | M/M | unit / placeholder | Nutrients are multiplied by `dz` = 1.0 ("//REMOVE") instead of `bm->boxes[b].dz[k]` / `sm.dz[k]`, unlike the b… |  | [ ] |
| [LINK-32](#link-32) | 34 | M/M | wrong extent | Loops over `ExportArray` use `convertPRMS.ingroup`, but ExportArray is allocated with `outgroup` entries (atli… |  | [ ] |

---

## LINK-27

**wrong variable / NULL deref** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atlinkconversion.c` 41
**Lines at review (Oct 2026):** 41

**Function:** `LinkToModel()`

**Why it's thought to be a bug**

`Util_Init_2D_Double(..., eweLinkModule->convertPRMS.io_nrow, ...)` uses the global `eweLinkModule` (declared in atLinkModule.h:49, never assigned) instead of `bm->eweLinkModule`, so this dereferences NULL. Function: `LinkToModel()`.

**Code at issue** — `atlinkconversion.c` line 41  
Search for: `Util_Init_2D_Double(`

```c
   39: 	if(	bm->eweLinkModule->convertPRMS.refresh == TRUE){
   40: 		for(l=0; l<bm->eweLinkModule->convertPRMS.ingroup; l++){
   41→ 			Util_Init_2D_Double(bm->eweLinkModule->ExportArray[l].values, eweLinkModule->convertPRMS.io_nrow, bm->eweLinkModule->convertPRMS.io_ncol, 0.0);
   42: 		}
   43: 	}
```

**Suggested fix (from review):** Use `bm->eweLinkModule->convertPRMS.io_nrow`.

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

## LINK-28

**wrong variable** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atlinkconversion.c` 212, `atlinkconversion.c` 223, `atlinkconversion.c` 229
**Lines at review (Oct 2026):** 212, 223, 229, 247

**Function:** `WeightedAvg()`

**Why it's thought to be a bug**

`nid = *PhysioChemArray[fgIndex].tracerIndex;` is computed but the nutrient and physchem blocks read `tr[k][bid]` and `sm.tr[k][bid]`. `bid` is the last biological group's tracer from the previous loop, or uninitialised if no group was exported. Function: `WeightedAvg()`.

**Code at issue** — `atlinkconversion.c` line 212  
Search for: `nid = *PhysioChemArray[fgIndex].tracerIndex`

```c
  210: 			/* For physical and chemical properties	*/
  211: 				if(PhysioChemArray[fgIndex].isExported == TRUE){
  212→ 					nid = *PhysioChemArray[fgIndex].tracerIndex; /* Get the tracer index pointed to by the totTracers pointer */
  213: 					switch(PhysioChemArray[fgIndex].exportType){
  214: 						case TYPE_NUTRIENTS:  /* Create nutrient array */
```

**Code at issue** — `atlinkconversion.c` line 223  
Search for: `tr[k][bid]`

```c
  221: 										if(PhysioChemArray[fgIndex].habitat_type == WC){
  222: 											for(k=0; k<bm->boxes[b].nz; k++){
  223→ 												bm->eweLinkModule->ExportArray[layer].values[row][col] += bm->boxes[b].tr[k][bid] * area * mg_2_tonne * dz;
  224: 												bm->eweLinkModule->ExportArray[layer].dataType = TYPE_NUTRIENTS;
  225: 											}
```

**Code at issue** — `atlinkconversion.c` line 229  
Search for: `sm.tr[k][bid]`

```c
  227: 										if(PhysioChemArray[fgIndex].habitat_type == SED){
  228: 											for(k=0; k<bm->boxes[b].sm.nz; k++){
  229→ 												bm->eweLinkModule->ExportArray[layer].values[row][col] += bm->boxes[b].sm.tr[k][bid] * area * mg_2_tonne * dz;
  230: 												bm->eweLinkModule->ExportArray[layer].dataType = TYPE_NUTRIENTS;
  231: 											}
```

**Suggested fix (from review):** Replace `bid` with `nid` in lines 223, 229 and 247.

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

## LINK-29

**wrong variable** · Conf H / Sev M  
**Found at (current code):** `atlinkconversion.c` 185
**Lines at review (Oct 2026):** 185

**Function:** `WeightedAvg()`

**Why it's thought to be a bug**

Sediment P/B: `bm->boxes[b].sm.tr[k][pid] / (bm->boxes[b].tr[k][bid] + small_num)` divides sediment production by the water-column biomass (and indexes the WC layer with a sediment layer k). Compare lines 174 and 179. Function: `WeightedAvg()`.

**Code at issue** — `atlinkconversion.c` line 185  
Search for: `boxes[b].sm.tr[k][pid] / (bm->boxes[b].tr[k][bid] + small_num)`

```c
  183: 													if(FunctGroupArray[fgIndex].habitatType == SED){
  184: 														for(k=0; k<bm->boxes[b].sm.nz; k++){
  185→ 															bm->eweLinkModule->ExportArray[layer].values[row][col] += area * s_2_yr * (bm->boxes[b].sm.tr[k][pid] / (bm->boxes[b].tr[k][bid] + small_num));
  186: 															bm->eweLinkModule->ExportArray[layer].dataType = TYPE_PRODUCTION;
  187: 														}
```

**Suggested fix (from review):** Use `bm->boxes[b].sm.tr[k][bid]`.

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

## LINK-30

**div-by-zero** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atlinkconversion.c` 266
**Lines at review (Oct 2026):** 266

**Function:** `WeightedAvg()`

**Why it's thought to be a bug**

`values[r][c] /= ExportArray[l].area[r][c]; // area created on readin` — `area` is allocated with 0.0 (atlinkexport.c:53-54) and never set anywhere, so every exported value becomes inf/NaN. Function: `WeightedAvg()`.

**Code at issue** — `atlinkconversion.c` line 266 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->eweLinkModule->ExportArray[l].values[r][c] /= bm->eweLinkModule->ExportArray[l].area[r`

```c
  264: 				for(r=0; r<bm->eweLinkModule->convertPRMS.io_nrow; r++){
  265: 					for(c=0; c<bm->eweLinkModule->convertPRMS.io_ncol; c++){
  266→ 						bm->eweLinkModule->ExportArray[l].values[r][c] /= bm->eweLinkModule->ExportArray[l].area[r][c]; // area created on readin
  267: 					}
  268: 				}
```

**Suggested fix (from review):** Fill `area[][]` in LoadCoversionParameters and guard against 0.

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

## LINK-31

**unit / placeholder** · Conf M / Sev M  
**Found at (current code):** `atlinkconversion.c` 141, `atlinkconversion.c` 147
**Lines at review (Oct 2026):** 223, 229

**Function:** `WeightedAvg()`

**Why it's thought to be a bug**

Nutrients are multiplied by `dz` = 1.0 ("//REMOVE") instead of `bm->boxes[b].dz[k]` / `sm.dz[k]`, unlike the biomass code (lines 141, 147). Function: `WeightedAvg()`.

**Code at issue** — `atlinkconversion.c` line 141  
Search for: `boxes[b].dz[k]`

```c
  139: 													if(FunctGroupArray[fgIndex].habitatType == WC){
  140: 														for(k=0; k<bm->boxes[b].nz; k++){
  141→ 															bm->eweLinkModule->ExportArray[layer].values[row][col] += bm->boxes[b].tr[k][bid] * area * mg_2_tonne * bm->boxes[b].dz[k];
  142: 															bm->eweLinkModule->ExportArray[layer].dataType = TYPE_BIOMASS;
  143: 														}
```

**Code at issue** — `atlinkconversion.c` line 147  
Search for: `sm.dz[k]`

```c
  145: 													if(FunctGroupArray[fgIndex].habitatType == SED){
  146: 														for(k=0; k<bm->boxes[b].sm.nz; k++){
  147→ 															bm->eweLinkModule->ExportArray[layer].values[row][col] += bm->boxes[b].sm.tr[k][bid] * area * mg_2_tonne * bm->boxes[b].sm.dz[k];
  148: 															bm->eweLinkModule->ExportArray[layer].dataType = TYPE_BIOMASS;
  149: 														}
```

**Suggested fix (from review):** Use the per-layer dz.

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

## LINK-32

**wrong extent** · Conf M / Sev M  
**Found at (current code):** `atlinkconversion.c` 34
**Lines at review (Oct 2026):** 40, 263

**Why it's thought to be a bug**

Loops over `ExportArray` use `convertPRMS.ingroup`, but ExportArray is allocated with `outgroup` entries (atlinkexport.c:47), and `layer` is incremented once per exported group or tracer with no check against that size. Functions: `LinkToModel()`, `WeightedAvg()`.

**Code at issue** — `atlinkconversion.c` line 34 (located by name only; check this is the right place)  
Search for: `void LinkToModel(MSEBoxModel *bm)`

```c
   32: */
   33: 
   34→ void LinkToModel(MSEBoxModel *bm)
   35: {
   36: 	int l=0;
```

**Suggested fix (from review):** Use outgroup for ExportArray and check layer < outgroup.

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

atlink/Makefile.am lists `atlinkconversion.o` in `libatlink_a_SOURCES` instead of `atlinkconversion.c`. Not one of my assigned files, but it means atlinkconversion.c is probably not built from source.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
