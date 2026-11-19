# atlink/atlinkexport.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **7 findings** (severity H: 3; confidence H and severity H: 3; on the priority list: 3).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [LINK-33](#link-33) ⭐ | 50 | H/H | uninitialised use | `initialiseLinkModule()` mallocs `eweLinkModule` and immediately uses `convertPRMS.ingroup`, `outgroup`, `io_n… |  | [ ] |
| [LINK-34](#link-34) ⭐ | 139 | H/H | copy-paste | `numrows = nr; numrows = nc;` — the second line should set `numcols`. `numcols` stays uninitialised, and all e… |  | [ ] |
| [LINK-35](#link-35) ⭐ | 154, 134 | H/H | wrong index / OOB | `bcellarea[nr][nc] = area;` inside `for r, for c` — writes the out-of-bounds element [4][4] of an array alloca… |  | [ ] |
| [LINK-36](#link-36) | 47 | M/M | swapped size | `ImportArray` is sized with `ingroup` and `ExportArray` with `outgroup`, but the init loop (and every use) ite… |  | [ ] |
| [LINK-37](#link-37) | 77, 66 | H/M | wrong free / wrong variable | `free(values)` / `free(area)` on alloc2d arrays frees only the row pointers and leaks the data block (should b… |  | [ ] |
| [LINK-38](#link-38) | 208 | H/M | swapped args | `setBiomass(name, values, io_ncol, io_nrow)` passes (numRows=io_ncol, numColumns=io_nrow) to a function that i… |  | [ ] |
| [LINK-39](#link-39) | 208, 195, 228 | M/M | uninitialised / copy-paste | `ExportArray[l].parameterName` is never assigned (an uninitialised pointer passed to SOAP). `dataType = -1` hi… |  | [ ] |

---

## LINK-33

**uninitialised use** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atlinkexport.c` 50
**Lines at review (Oct 2026):** 38-55, 91-103

**Why it's thought to be a bug**

`initialiseLinkModule()` mallocs `eweLinkModule` and immediately uses `convertPRMS.ingroup`, `outgroup`, `io_ncol` and `io_nrow` (uninitialised) as allocation sizes. LoadCoversionParameters only sets these after calling it (lines 91 vs 100-103). Functions: `initialiseLinkModule()`, `LoadCoversionParameters()`.

**Code at issue** — `atlinkexport.c` line 50 (located by name only; check this is the right place)  
Search for: `bm->eweLinkModule->ImportArray[group].values = Util_Alloc_Init_2D_Double(bm->eweLinkModule`

```c
   48: 
   49: 	for(group=0; group<bm->eweLinkModule->convertPRMS.ingroup; group++){
   50→ 		bm->eweLinkModule->ImportArray[group].values = Util_Alloc_Init_2D_Double(bm->eweLinkModule->convertPRMS.io_ncol,bm->eweLinkModule->convertPRMS.io_nrow, 0.0);
   51: 		bm->eweLinkModule->ExportArray[group].values = Util_Alloc_Init_2D_Double(bm->eweLinkModule->convertPRMS.io_ncol,bm->eweLinkModule->convertPRMS.io_nrow, 0.0);
   52: 
```

**Suggested fix (from review):** Set the convertPRMS values before allocating the arrays.

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

## LINK-34

**copy-paste** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atlinkexport.c` 139
**Lines at review (Oct 2026):** 139-140

**Function:** `LoadCoversionParameters()`

**Why it's thought to be a bug**

`numrows = nr; numrows = nc;` — the second line should set `numcols`. `numcols` stays uninitialised, and all export/import loops (`nc < export->numcols`) run over garbage bounds. Function: `LoadCoversionParameters()`.

**Code at issue** — `atlinkexport.c` line 139 (the line the review pointed to, found by its text)  
Search for: `bm->boxes[b].export->numrows = nr;`

```c
  137: 
  138: 		/* Readin numrows and numcols */
  139→ 		bm->boxes[b].export->numrows = nr;
  140: 		bm->boxes[b].export->numrows = nc;
  141: 
```

**Suggested fix (from review):** `bm->boxes[b].export->numcols = nc;`

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

## LINK-35

**wrong index / OOB** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atlinkexport.c` 154, `atlinkexport.c` 134
**Lines at review (Oct 2026):** 134, 152-154

**Function:** `LoadCoversionParameters()`

**Why it's thought to be a bug**

`bcellarea[nr][nc] = area;` inside `for r, for c` — writes the out-of-bounds element [4][4] of an array allocated `alloc2d(nc,nr)` = [nr][nc] = [4][4]. The intended cells [r][c] stay uninitialised. Function: `LoadCoversionParameters()`.

**Code at issue** — `atlinkexport.c` line 154  
Search for: `bcellarea[nr][nc] = area`

```c
  152: 		for(r=0; r<nr; r++){
  153: 			for(c=0; c<nc; c++){
  154→ 				bm->boxes[b].export->bcellarea[nr][nc] = area;
  155: 			}
  156: 		}
```

**Code at issue** — `atlinkexport.c` line 134  
Search for: `alloc2d(nc,nr)`

```c
  132: 
  133: 		/* Create memory allocation for areas */
  134→ 		bm->boxes[b].export->bcellarea = (double **)alloc2d(nc,nr);
  135: 
  136: 		/** Now load in data **/
```

**Suggested fix (from review):** `bcellarea[r][c] = area;`

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

## LINK-36

**swapped size** · Conf M / Sev M  
**Found at (current code):** `atlinkexport.c` 47
**Lines at review (Oct 2026):** 46-55

**Function:** `initialiseLinkModule()`

**Why it's thought to be a bug**

`ImportArray` is sized with `ingroup` and `ExportArray` with `outgroup`, but the init loop (and every use) iterates `ingroup` over both. If outgroup < ingroup this overruns ExportArray. Function: `initialiseLinkModule()`.

**Code at issue** — `atlinkexport.c` line 47 (the line the review pointed to, found by its text)  
Search for: `bm->eweLinkModule->ExportArray = (LinkTransportStruct *)malloc(sizeof(LinkTransportStruct)`

```c
   45: 
   46: 	bm->eweLinkModule->ImportArray = (LinkTransportStruct *)malloc(sizeof(LinkTransportStruct) * bm->eweLinkModule->convertPRMS.ingroup);
   47→ 	bm->eweLinkModule->ExportArray = (LinkTransportStruct *)malloc(sizeof(LinkTransportStruct) * bm->eweLinkModule->convertPRMS.outgroup);
   48: 
   49: 	for(group=0; group<bm->eweLinkModule->convertPRMS.ingroup; group++){
```

**Suggested fix (from review):** Allocate and loop each array by its own count.

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

## LINK-37

**wrong free / wrong variable** · Conf H / Sev M  
**Found at (current code):** `atlinkexport.c` 77, `atlinkexport.c` 66
**Lines at review (Oct 2026):** 66-77

**Function:** `freeLinkModule()`

**Why it's thought to be a bug**

`free(values)` / `free(area)` on alloc2d arrays frees only the row pointers and leaks the data block (should be free2d). `free(eweLinkModule)` frees the never-assigned global, not `bm->eweLinkModule` (which leaks). Function: `freeLinkModule()`.

**Code at issue** — `atlinkexport.c` line 77  
Search for: `free(eweLinkModule)`

```c
   75: 	free(bm->eweLinkModule->ExportArray);
   76: 
   77→ 	free(eweLinkModule);
   78: }
   79: 
```

**Code at issue** — `atlinkexport.c` line 66  
Search for: `eweLinkModule`

```c
   64: 	int group;
   65: 
   66→ 	for(group=0; group<bm->eweLinkModule->convertPRMS.ingroup; group++){
   67: 		free(bm->eweLinkModule->ImportArray[group].values);
   68: 		free(bm->eweLinkModule->ExportArray[group].values);
```

**Suggested fix (from review):** Use free2d for the 2D arrays and free(bm->eweLinkModule).

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

## LINK-38

**swapped args** · Conf H / Sev M  
**Found at (current code):** `atlinkexport.c` 208
**Lines at review (Oct 2026):** 208-225

**Function:** `SendArrays()`

**Why it's thought to be a bug**

`setBiomass(name, values, io_ncol, io_nrow)` passes (numRows=io_ncol, numColumns=io_nrow) to a function that indexes `dataArray[rows][columns]`, but `values` is `[io_nrow][io_ncol]` (alloc2d(io_ncol, io_nrow), line 51). Columns run to 71 when the row length is 60, so the read runs past the row and past the end of the last row. The same applies to all set* calls here and to all get* calls in atlinkimport.c:136-148. Function: `SendArrays()`.

**Code at issue** — `atlinkexport.c` line 208 (the line the review pointed to, found by its text)  
Search for: `setBiomass(bm->eweLinkModule->ExportArray[l].parameterName, bm->eweLinkModule->ExportArray`

```c
  206: 		switch(bm->eweLinkModule->ExportArray[l].dataType){
  207: 			case TYPE_BIOMASS:
  208→ 				setBiomass(bm->eweLinkModule->ExportArray[l].parameterName, bm->eweLinkModule->ExportArray[l].values,
  209: 				bm->eweLinkModule->convertPRMS.io_ncol, bm->eweLinkModule->convertPRMS.io_nrow);
  210:                 ans++;
```

**Suggested fix (from review):** Pass `io_nrow, io_ncol`.

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

## LINK-39

**uninitialised / copy-paste** · Conf M / Sev M  
**Found at (current code):** `atlinkexport.c` 208, `atlinkexport.c` 195, `atlinkexport.c` 228
**Lines at review (Oct 2026):** 165, 208, 229

**Function:** `SendArrays()`

**Why it's thought to be a bug**

`ExportArray[l].parameterName` is never assigned (an uninitialised pointer passed to SOAP). `dataType = -1` hits the `default: quit("WeightedAvg: ...")` branch if no group set it, and the quit message names the wrong function. Function: `SendArrays()`.

**Code at issue** — `atlinkexport.c` line 208  
Search for: `ExportArray[l].parameterName`

```c
  206: 		switch(bm->eweLinkModule->ExportArray[l].dataType){
  207: 			case TYPE_BIOMASS:
  208→ 				setBiomass(bm->eweLinkModule->ExportArray[l].parameterName, bm->eweLinkModule->ExportArray[l].values,
  209: 				bm->eweLinkModule->convertPRMS.io_ncol, bm->eweLinkModule->convertPRMS.io_nrow);
  210:                 ans++;
```

**Code at issue** — `atlinkexport.c` line 195  
Search for: `dataType = -1`

```c
  193: 	for(l=0; l<bm->eweLinkModule->convertPRMS.ingroup; l++){
  194: 
  195→ //		bm->eweLinkModule->ExportArray[l].dataType = -1;
  196: 		// Not sure why you would want to do this.
  197: //		for(r=0; r<bm->eweLinkModule->convertPRMS.io_nrow; r++){
```

**Code at issue** — `atlinkexport.c` line 228  
Search for: `default:`

```c
  226:                 ans++;
  227:                 break;
  228→ 			default:
  229: 				quit("WeightedAvg: This data type does not exist\n");
  230: 				break;
```

**Suggested fix (from review):** Set parameterName, and correct the message to "SendArrays".

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
