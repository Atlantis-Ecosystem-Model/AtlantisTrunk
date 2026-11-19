# atecology/atBuildTracer.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [EINIT-19](#einit-19) | 76 | M/L | buffer overrun (latent) | `strcpy((*tracerArray)[index].name, name);` writes into `Namelist.name[100]` (atlantisboxmodel.h:6063), but th… |  | [ ] |
| [EINIT-20](#einit-20) | 442-450 vs 473-505 | L/M | size/count mismatch | The size count adds `numCohortsXnumGenes` for every EPIFAUNA group, but the build adds 0 for AGE_STRUCTURED ep… |  | [ ] |

---

## EINIT-19

**buffer overrun (latent)** · Conf M / Sev L  
**Lines (current code):** 76  
**Lines at review (Oct 2026):** 76

**Function:** `Add_Tracer()`

**Why it's thought to be a bug**

`strcpy((*tracerArray)[index].name, name);` writes into `Namelist.name[100]` (atlantisboxmodel.h:6255), but the names are built in `str[STRLEN*2]` (4000 bytes) from group names plus contaminant names (e.g. `"%s%d_Prop_%s"`, lines 388/547). A long group name plus contaminant name overflows the struct. Function: `Add_Tracer()`.

**Current code** (`atecology/atBuildTracer.c` lines 74–78)

```c
   74: 		(*tracerArray)[index].index = NULL;
   75: 	} else {
   76: 		strcpy((*tracerArray)[index].name, name);
   77: 		(*tracerArray)[index].index = tracerIndex;
   78: 	}
```

**Suggested fix (from review):** `snprintf((*tracerArray)[index].name, sizeof((*tracerArray)[index].name), "%s", name);`

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

## EINIT-20

**size/count mismatch** · Conf L / Sev M  
**Lines (current code):** 442-450 vs 473-505  
**Lines at review (Oct 2026):** 435-443 vs 466-498

**Function:** `Build_EPI_NameList()`

**Why it's thought to be a bug**

The size count adds `numCohortsXnumGenes` for every EPIFAUNA group, but the build adds 0 for AGE_STRUCTURED epifauna and only 1 for BIOMASS epifauna unless `flag_macro_model==1 && SEAGRASS`. Unlike Build_TrName_NameList:400 there is no index-vs-size check, so `nEpiSize` can exceed the number of built entries. atbiolsetup.c:1295 then dereferences the terminator entry or uninitialised entries (`epinamelist[ij].name` and `.index`). Function: `Build_EPI_NameList()`.

**Current code** (`atecology/atBuildTracer.c` lines 440–479 (truncated))

```c
  440: 	int size = 1, fgIndex, index = 0, cohort, contamIndex;
  441: 
  442: 	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
  443: 		if (FunctGroupArray[fgIndex].habitatType == EPIFAUNA){
  444: 			size += FunctGroupArray[fgIndex].numCohortsXnumGenes;
  445: 		}
  446: 
  447: 		if (FunctGroupArray[fgIndex].isCover == TRUE)
  448: 			size += 1;
  449: 
  450: 	}
  451: 	if(bm->track_atomic_ratio == TRUE){
  452: 		for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
  453: 			if (FunctGroupArray[fgIndex].speciesParams[flag_id] == TRUE) {
  454: 				if (FunctGroupArray[fgIndex].habitatType == EPIFAUNA) {
  455: 					size += (FunctGroupArray[fgIndex].numCohortsXnumGenes * 2);
  456: 				}
  457: 			}
  458: 		}
  459: 	}
  460: 	if (bm->track_contaminants == TRUE) {
  461: 		for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
  462: 			if (FunctGroupArray[fgIndex].speciesParams[flag_id] == TRUE) {
  463: 				if (FunctGroupArray[fgIndex].habitatType == EPIFAUNA) {
  464: 					size += (FunctGroupArray[fgIndex].numCohorts * bm->num_contaminants * 2);
  465: 				}
  466: 			}
  467: 		}
  468: 	}
  469: 	epinamelist = (Namelist *) malloc((size_t)size * sizeof(Namelist));
  470: 
  471: 	nEpiSize = size - 1;
  472: 
  473: 	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
  474: 		FunctGroupArray[fgIndex].CoverTracer = -1;
  475: 		if (FunctGroupArray[fgIndex].habitatType == EPIFAUNA) {
  476: 			switch (FunctGroupArray[fgIndex].groupAgeType) {
  477: 			case AGE_STRUCTURED_BIOMASS:
  478: 				// Allow for multiple cohorts so can have age structure reefs and multiple macrophyte pools
  479: 				for (cohort = 0; cohort < FunctGroupArray[fgIndex].numCohortsXnumGenes; cohort++) {
```

**Suggested fix (from review):** Make the size count mirror the build branches and add the same `(index-1) != nEpiSize` quit check

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
