# atlink/atlinkexport.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **7 findings** (severity H: 3; confidence H and severity H: 3; on the priority list: 3).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [LINK-33](#link-33) ⭐ | 38-55, 91-103 | H/H | uninitialised use | `initialiseLinkModule()` mallocs `eweLinkModule` and immediately uses `convertPRMS.ingroup`, `outgroup`, `io_n… |  | [ ] |
| [LINK-34](#link-34) ⭐ | 139-140 | H/H | copy-paste | `numrows = nr; numrows = nc;` — the second line should set `numcols`. `numcols` stays uninitialised, and all e… |  | [ ] |
| [LINK-35](#link-35) ⭐ | 134, 152-154 | H/H | wrong index / OOB | `bcellarea[nr][nc] = area;` inside `for r, for c` — writes the out-of-bounds element [4][4] of an array alloca… |  | [ ] |
| [LINK-36](#link-36) | 46-55 | M/M | swapped size | `ImportArray` is sized with `ingroup` and `ExportArray` with `outgroup`, but the init loop (and every use) ite… |  | [ ] |
| [LINK-37](#link-37) | 66-77 | H/M | wrong free / wrong variable | `free(values)` / `free(area)` on alloc2d arrays frees only the row pointers and leaks the data block (should b… |  | [ ] |
| [LINK-38](#link-38) | 208-225 | H/M | swapped args | `setBiomass(name, values, io_ncol, io_nrow)` passes (numRows=io_ncol, numColumns=io_nrow) to a function that i… |  | [ ] |
| [LINK-39](#link-39) | 165, 208, 229 | M/M | uninitialised / copy-paste | `ExportArray[l].parameterName` is never assigned (an uninitialised pointer passed to SOAP). `dataType = -1` hi… |  | [ ] |

---

## LINK-33

**uninitialised use** · Conf H / Sev H ⭐ priority  
**Listed lines:** 38-55, 91-103

**Why it's thought to be a bug**

`initialiseLinkModule()` mallocs `eweLinkModule` and immediately uses `convertPRMS.ingroup`, `outgroup`, `io_ncol` and `io_nrow` (uninitialised) as allocation sizes. LoadCoversionParameters only sets these after calling it (lines 91 vs 100-103). Functions: `initialiseLinkModule()`, `LoadCoversionParameters()`.

**Current code** (`atlink/atlinkexport.c` lines 36–57)

```c
   36: {
   37: 	int group;
   38: 	bm->eweLinkModule = (EwELinkStruct *)malloc(sizeof(EwELinkStruct));
   39: 
   40: 	/* The transport arrays */
   41: 
   42: 	/* Set up Export and Import arrays */
   43: 	//ExportArray = Util_Alloc_Init_4D_Double(bm->eweLinkModule->convertPRMS.io_ncol,bm->eweLinkModule->convertPRMS.io_nrow,2, bm->eweLinkModule->convertPRMS.ingroup, 0.0);
   44: 	//ImportArray = Util_Alloc_Init_4D_Double(bm->eweLinkModule->convertPRMS.io_ncol,bm->eweLinkModule->convertPRMS.io_nrow,2, bm->eweLinkModule->convertPRMS.outgroup, 0.0);
   45: 
   46: 	bm->eweLinkModule->ImportArray = (LinkTransportStruct *)malloc(sizeof(LinkTransportStruct) * bm->eweLinkModule->convertPRMS.ingroup);
   47: 	bm->eweLinkModule->ExportArray = (LinkTransportStruct *)malloc(sizeof(LinkTransportStruct) * bm->eweLinkModule->convertPRMS.outgroup);
   48: 
   49: 	for(group=0; group<bm->eweLinkModule->convertPRMS.ingroup; group++){
   50: 		bm->eweLinkModule->ImportArray[group].values = Util_Alloc_Init_2D_Double(bm->eweLinkModule->convertPRMS.io_ncol,bm->eweLinkModule->convertPRMS.io_nrow, 0.0);
   51: 		bm->eweLinkModule->ExportArray[group].values = Util_Alloc_Init_2D_Double(bm->eweLinkModule->convertPRMS.io_ncol,bm->eweLinkModule->convertPRMS.io_nrow, 0.0);
   52: 
   53: 		bm->eweLinkModule->ImportArray[group].area = Util_Alloc_Init_2D_Double(bm->eweLinkModule->convertPRMS.io_ncol,bm->eweLinkModule->convertPRMS.io_nrow, 0.0);
   54: 		bm->eweLinkModule->ExportArray[group].area = Util_Alloc_Init_2D_Double(bm->eweLinkModule->convertPRMS.io_ncol,bm->eweLinkModule->convertPRMS.io_nrow, 0.0);
   55: 	}
   56: }
   57: 
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
**Listed lines:** 139-140

**Function:** `LoadCoversionParameters()`

**Why it's thought to be a bug**

`numrows = nr; numrows = nc;` — the second line should set `numcols`. `numcols` stays uninitialised, and all export/import loops (`nc < export->numcols`) run over garbage bounds. Function: `LoadCoversionParameters()`.

**Current code** (`atlink/atlinkexport.c` lines 137–142)

```c
  137: 
  138: 		/* Readin numrows and numcols */
  139: 		bm->boxes[b].export->numrows = nr;
  140: 		bm->boxes[b].export->numrows = nc;
  141: 
  142: 		/* Readin in cell ids */
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
**Listed lines:** 134, 152-154

**Function:** `LoadCoversionParameters()`

**Why it's thought to be a bug**

`bcellarea[nr][nc] = area;` inside `for r, for c` — writes the out-of-bounds element [4][4] of an array allocated `alloc2d(nc,nr)` = [nr][nc] = [4][4]. The intended cells [r][c] stay uninitialised. Function: `LoadCoversionParameters()`.

**Current code** (`atlink/atlinkexport.c` lines 132–156)

```c
  132: 
  133: 		/* Create memory allocation for areas */
  134: 		bm->boxes[b].export->bcellarea = (double **)alloc2d(nc,nr);
  135: 
  136: 		/** Now load in data **/
  137: 
  138: 		/* Readin numrows and numcols */
  139: 		bm->boxes[b].export->numrows = nr;
  140: 		bm->boxes[b].export->numrows = nc;
  141: 
  142: 		/* Readin in cell ids */
  143: 		for(r=0; r<nr; r++){
  144: 			bm->boxes[b].export->rows[r] = (int)xxx;
  145: 		}
  146: 		for(c=0; c<nc; c++){
  147: 			bm->boxes[b].export->cols[c] = (int)yyy;
  148: 		}
  149: 
  150: 		/* Readin cell areas */
  151: 		area = bm->eweLinkModule->convertPRMS.io_area;  // For now just assume 100% overlap of cells - TODO NEEDS TO BE FIXED
  152: 		for(r=0; r<nr; r++){
  153: 			for(c=0; c<nc; c++){
  154: 				bm->boxes[b].export->bcellarea[nr][nc] = area;
  155: 			}
  156: 		}
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
**Listed lines:** 46-55

**Function:** `initialiseLinkModule()`

**Why it's thought to be a bug**

`ImportArray` is sized with `ingroup` and `ExportArray` with `outgroup`, but the init loop (and every use) iterates `ingroup` over both. If outgroup < ingroup this overruns ExportArray. Function: `initialiseLinkModule()`.

**Current code** (`atlink/atlinkexport.c` lines 44–57)

```c
   44: 	//ImportArray = Util_Alloc_Init_4D_Double(bm->eweLinkModule->convertPRMS.io_ncol,bm->eweLinkModule->convertPRMS.io_nrow,2, bm->eweLinkModule->convertPRMS.outgroup, 0.0);
   45: 
   46: 	bm->eweLinkModule->ImportArray = (LinkTransportStruct *)malloc(sizeof(LinkTransportStruct) * bm->eweLinkModule->convertPRMS.ingroup);
   47: 	bm->eweLinkModule->ExportArray = (LinkTransportStruct *)malloc(sizeof(LinkTransportStruct) * bm->eweLinkModule->convertPRMS.outgroup);
   48: 
   49: 	for(group=0; group<bm->eweLinkModule->convertPRMS.ingroup; group++){
   50: 		bm->eweLinkModule->ImportArray[group].values = Util_Alloc_Init_2D_Double(bm->eweLinkModule->convertPRMS.io_ncol,bm->eweLinkModule->convertPRMS.io_nrow, 0.0);
   51: 		bm->eweLinkModule->ExportArray[group].values = Util_Alloc_Init_2D_Double(bm->eweLinkModule->convertPRMS.io_ncol,bm->eweLinkModule->convertPRMS.io_nrow, 0.0);
   52: 
   53: 		bm->eweLinkModule->ImportArray[group].area = Util_Alloc_Init_2D_Double(bm->eweLinkModule->convertPRMS.io_ncol,bm->eweLinkModule->convertPRMS.io_nrow, 0.0);
   54: 		bm->eweLinkModule->ExportArray[group].area = Util_Alloc_Init_2D_Double(bm->eweLinkModule->convertPRMS.io_ncol,bm->eweLinkModule->convertPRMS.io_nrow, 0.0);
   55: 	}
   56: }
   57: 
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
**Listed lines:** 66-77

**Function:** `freeLinkModule()`

**Why it's thought to be a bug**

`free(values)` / `free(area)` on alloc2d arrays frees only the row pointers and leaks the data block (should be free2d). `free(eweLinkModule)` frees the never-assigned global, not `bm->eweLinkModule` (which leaks). Function: `freeLinkModule()`.

**Current code** (`atlink/atlinkexport.c` lines 64–79)

```c
   64: 	int group;
   65: 
   66: 	for(group=0; group<bm->eweLinkModule->convertPRMS.ingroup; group++){
   67: 		free(bm->eweLinkModule->ImportArray[group].values);
   68: 		free(bm->eweLinkModule->ExportArray[group].values);
   69: 
   70: 		free(bm->eweLinkModule->ImportArray[group].area);
   71: 		free(bm->eweLinkModule->ExportArray[group].area);
   72: 
   73: 	}
   74: 	free(bm->eweLinkModule->ImportArray);
   75: 	free(bm->eweLinkModule->ExportArray);
   76: 
   77: 	free(eweLinkModule);
   78: }
   79: 
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
**Listed lines:** 208-225

**Function:** `SendArrays()`

**Why it's thought to be a bug**

`setBiomass(name, values, io_ncol, io_nrow)` passes (numRows=io_ncol, numColumns=io_nrow) to a function that indexes `dataArray[rows][columns]`, but `values` is `[io_nrow][io_ncol]` (alloc2d(io_ncol, io_nrow), line 51). Columns run to 71 when the row length is 60, so the read runs past the row and past the end of the last row. The same applies to all set* calls here and to all get* calls in atlinkimport.c:136-148. Function: `SendArrays()`.

**Current code** (`atlink/atlinkexport.c` lines 206–227)

```c
  206: 		switch(bm->eweLinkModule->ExportArray[l].dataType){
  207: 			case TYPE_BIOMASS:
  208: 				setBiomass(bm->eweLinkModule->ExportArray[l].parameterName, bm->eweLinkModule->ExportArray[l].values,
  209: 				bm->eweLinkModule->convertPRMS.io_ncol, bm->eweLinkModule->convertPRMS.io_nrow);
  210:                 ans++;
  211:                 break;
  212: 			case TYPE_PRODUCTION:
  213: 				setProduction(bm->eweLinkModule->ExportArray[l].parameterName, bm->eweLinkModule->ExportArray[l].values, bm->eweLinkModule->convertPRMS.io_ncol, bm->eweLinkModule->convertPRMS.io_nrow);
  214:                 ans++;
  215:                 break;
  216: 			case TYPE_MORTALITY:
  217: 				setMortality(bm->eweLinkModule->ExportArray[l].parameterName, bm->eweLinkModule->ExportArray[l].values, bm->eweLinkModule->convertPRMS.io_ncol, bm->eweLinkModule->convertPRMS.io_nrow);
  218:                 ans++;
  219:                 break;
  220: 			case TYPE_NUTRIENTS:
  221: 				setNutrients(bm->eweLinkModule->ExportArray[l].parameterName, bm->eweLinkModule->ExportArray[l].values, bm->eweLinkModule->convertPRMS.io_ncol, bm->eweLinkModule->convertPRMS.io_nrow);
  222:                 ans++;
  223:                 break;
  224: 			case TYPE_PHYSCHEM:
  225: 				setProperties(bm->eweLinkModule->ExportArray[l].parameterName, bm->eweLinkModule->ExportArray[l].values, bm->eweLinkModule->convertPRMS.io_ncol, bm->eweLinkModule->convertPRMS.io_nrow);
  226:                 ans++;
  227:                 break;
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
**Listed lines:** 165, 208, 229

**Function:** `SendArrays()`

**Why it's thought to be a bug**

`ExportArray[l].parameterName` is never assigned (an uninitialised pointer passed to SOAP). `dataType = -1` hits the `default: quit("WeightedAvg: ...")` branch if no group set it, and the quit message names the wrong function. Function: `SendArrays()`.

**Current code** (`atlink/atlinkexport.c` lines 163–167)

```c
  163: 	for(l=0; l<bm->eweLinkModule->convertPRMS.ingroup; l++){
  164: 		bm->eweLinkModule->ImportArray[l].dataType = TYPE_BIOMASS;
  165: 		bm->eweLinkModule->ExportArray[l].dataType = -1;
  166: 
  167: 
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
