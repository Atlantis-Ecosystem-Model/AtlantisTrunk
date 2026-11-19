# atecology/atBuildTracer.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [EINIT-19](#einit-19) | 76, 72, 163 | M/L | buffer overrun (latent) | `strcpy((*tracerArray)[index].name, name);` writes into `Namelist.name[100]` (atlantisboxmodel.h:6063), but th… |  | [ ] |
| [EINIT-20](#einit-20) | 485 | L/M | size/count mismatch | The size count adds `numCohortsXnumGenes` for every EPIFAUNA group, but the build adds 0 for AGE_STRUCTURED ep… |  | [ ] |

---

## EINIT-19

**buffer overrun (latent)** · Conf M / Sev L  
**Found at (current code):** `atBuildTracer.c` 76, `atBuildTracer.c` 72, `atBuildTracer.c` 163
**Lines at review (Oct 2026):** 76

**Function:** `Add_Tracer()`

**Why it's thought to be a bug**

`strcpy((*tracerArray)[index].name, name);` writes into `Namelist.name[100]` (atlantisboxmodel.h:6255), but the names are built in `str[STRLEN*2]` (4000 bytes) from group names plus contaminant names (e.g. `"%s%d_Prop_%s"`, lines 388/547). A long group name plus contaminant name overflows the struct. Function: `Add_Tracer()`.

**Code at issue** — `atBuildTracer.c` line 76  
Search for: `strcpy((*tracerArray)[index].name, name)`

```c
   74: 		(*tracerArray)[index].index = NULL;
   75: 	} else {
   76→ 		strcpy((*tracerArray)[index].name, name);
   77: 		(*tracerArray)[index].index = tracerIndex;
   78: 	}
```

**Code at issue** — `atBuildTracer.c` line 72 (closest match to the quoted code; wording differs slightly)  
Search for: `static void Add_Tracer(Namelist **tracerArray, int index, char *name, int *tracerIndex, in`

```c
   70:  *
   71:  */
   72→ static void Add_Tracer(Namelist **tracerArray, int index, char *name, int *tracerIndex, int flux, int tol, int bio, int contam) {
   73: 	if (name == NULL) {
   74: 		(*tracerArray)[index].index = NULL;
```

**Code at issue** — `atBuildTracer.c` line 163  
Search for: `str[STRLEN*2]`

```c
  161: 	int size = 0;
  162: 	int index = 0;
  163→ 	char str[STRLEN*2];
  164: 
  165: 	/* work out the number of tracers that should be found */
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
**Found at (current code):** `atBuildTracer.c` 485
**Lines at review (Oct 2026):** 435-443 vs 466-498

**Function:** `Build_EPI_NameList()`

**Why it's thought to be a bug**

The size count adds `numCohortsXnumGenes` for every EPIFAUNA group, but the build adds 0 for AGE_STRUCTURED epifauna and only 1 for BIOMASS epifauna unless `flag_macro_model==1 && SEAGRASS`. Unlike Build_TrName_NameList:400 there is no index-vs-size check, so `nEpiSize` can exceed the number of built entries. atbiolsetup.c:1295 then dereferences the terminator entry or uninitialised entries (`epinamelist[ij].name` and `.index`). Function: `Build_EPI_NameList()`.

**Code at issue** — `atBuildTracer.c` line 485 (closest match to the quoted code; wording differs slightly)  
Search for: `if(bm->flag_macro_model == 1 && FunctGroupArray[fgIndex].groupType == SEAGRASS){`

```c
  483: 				break;
  484: 			case BIOMASS:
  485→ 				if(bm->flag_macro_model == 1 && FunctGroupArray[fgIndex].groupType == SEAGRASS){
  486: 					// Allow for multiple cohorts so can have age structure reefs and multiple macrophyte pools
  487: 					for (cohort = 0; cohort < FunctGroupArray[fgIndex].numCohortsXnumGenes; cohort++) {
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
