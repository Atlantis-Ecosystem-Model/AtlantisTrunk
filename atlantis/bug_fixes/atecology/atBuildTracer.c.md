# atecology/atBuildTracer.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [EINIT-19](#einit-19) | 76 | M/L | buffer overrun (latent) | `strcpy((*tracerArray)[index].name, name);` writes into `Namelist.name[100]` (atlantisboxmodel.h:6063), but th… |  | [ ] |
| [EINIT-20](#einit-20) | 435-443 vs 466-498 | L/M | size/count mismatch | The size count adds `numCohortsXnumGenes` for every EPIFAUNA group, but the build adds 0 for AGE_STRUCTURED ep… |  | [ ] |

---

## EINIT-19

**buffer overrun (latent)** · Conf M / Sev L  
**Listed lines:** 76

**Function:** `Add_Tracer()`

**Why it's thought to be a bug**

`strcpy((*tracerArray)[index].name, name);` writes into `Namelist.name[100]` (atlantisboxmodel.h:6063), but the names are built in `str[STRLEN*2]` (4000 bytes) from group names plus contaminant names (e.g. `"%s%d_Prop_%s"`, lines 381/540). A long group name plus contaminant name overflows the struct. Function: `Add_Tracer()`.

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
**Listed lines:** 435-443 vs 466-498

**Function:** `Build_EPI_NameList()`

**Why it's thought to be a bug**

The size count adds `numCohortsXnumGenes` for every EPIFAUNA group, but the build adds 0 for AGE_STRUCTURED epifauna and only 1 for BIOMASS epifauna unless `flag_macro_model==1 && SEAGRASS`. Unlike Build_TrName_NameList:400 there is no index-vs-size check, so `nEpiSize` can exceed the number of built entries. atbiolsetup.c:1295 then dereferences the terminator entry or uninitialised entries (`epinamelist[ij].name` and `.index`). Function: `Build_EPI_NameList()`.

**Current code** (`atecology/atBuildTracer.c` lines 436–448) — code appears to have moved +3 lines since the review

```c
  436:  *
  437:  */
  438: void Build_EPI_NameList(MSEBoxModel *bm) {
  439: 	char str[STRLEN*2];
  440: 	int size = 1, fgIndex, index = 0, cohort, contamIndex;
  441: 
  442: 	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
  443: 		if (FunctGroupArray[fgIndex].habitatType == EPIFAUNA){
  444: 			size += FunctGroupArray[fgIndex].numCohortsXnumGenes;
  445: 		}
  446: 
  447: 		if (FunctGroupArray[fgIndex].isCover == TRUE)
  448: 			size += 1;
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
