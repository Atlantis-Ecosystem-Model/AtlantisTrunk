# atharvest/atHarvestSetup.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [HRV2-07](#hrv2-07) | 644 | M/M | cohort×genotype confusion | The comparison uses the age class (`basechrt < age50`, and `age50 = basechrt`), but the stored value is the ge… |  | [ ] |
| [HRV2-08](#hrv2-08) | 78 | M/L | NULL dereference | `*(strstr(convertedXMLFileName, ".prm")) = '\0';` crashes if the fisheries prm filename has no ".prm" extensio… |  | [ ] |
| [HRV2-09](#hrv2-09) | 395 | H/L | copy-paste / metadata | Average catch size tracers get the long name "Total effort of (recreational) fishery ...", copied from the eff… |  | [ ] |
| [HRV2-10](#hrv2-10) | 424 | H/L | memory leak | `Harvest_Free()` frees tsCatch but never `tsDiscard`, and never frees `qSTOCK` (allocated at line 454). Functi… |  | [ ] |

---

## HRV2-07

**cohort×genotype confusion** · Conf M / Sev M  
**Found at (current code):** `atHarvestSetup.c` 644
**Lines at review (Oct 2026):** 644-651

**Function:** `Calculate_Age_Vulnerability()`

**Why it's thought to be a bug**

The comparison uses the age class (`basechrt < age50`, and `age50 = basechrt`), but the stored value is the genotype-expanded index: `speciesParams[Age50pcntV_id] = chrt`. Age95 has the same problem. Consumers treat it as an age class (`va50` in atassess/atclassical.c:839,1143,1410). With numGeneTypes>1 the stored age is wrong and can be >= numCohorts. Function: `Calculate_Age_Vulnerability()`.

**Code at issue** — `atHarvestSetup.c` line 644  
Search for: `basechrt < age50`

```c
  642: 
  643: 						/* 50% vulnerability to the fisheries */
  644→ 						if ((sel > 0.50) && (basechrt < age50)) {
  645: 							FunctGroupArray[sp].speciesParams[Age50pcntV_id] = chrt;
  646: 							age50 = basechrt;
```

**Suggested fix (from review):** Store `basechrt`

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

## HRV2-08

**NULL dereference** · Conf M / Sev L  
**Found at (current code):** `atHarvestSetup.c` 78
**Lines at review (Oct 2026):** 78

**Function:** `Harvest_Init()`

**Why it's thought to be a bug**

`*(strstr(convertedXMLFileName, ".prm")) = '\0';` crashes if the fisheries prm filename has no ".prm" extension. Function: `Harvest_Init()`.

**Code at issue** — `atHarvestSetup.c` line 78  
Search for: `*(strstr(convertedXMLFileName, ".prm")) = '\0'`

```c
   76: 	/* Build the converted filename */
   77: 	snprintf(convertedXMLFileName, sizeof(convertedXMLFileName), "%s", bm->fishprmIfname);
   78→ 	*(strstr(convertedXMLFileName, ".prm")) = '\0';
   79: 	strcat(convertedXMLFileName, ".xml");
   80: 
```

**Suggested fix (from review):** Check strstr result and quit with a message

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

## HRV2-09

**copy-paste / metadata** · Conf H / Sev L  
**Found at (current code):** `atHarvestSetup.c` 395
**Lines at review (Oct 2026):** 393-412

**Function:** `Build_Fishery_Tracers()`

**Why it's thought to be a bug**

Average catch size tracers get the long name "Total effort of (recreational) fishery ...", copied from the effort tracer block. Line 408 even says "This should be different but is not!". Units "mg N" are questionable too. Function: `Build_Fishery_Tracers()`.

**Code at issue** — `atHarvestSetup.c` line 395 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `for (nf = 0; nf < bm->K_num_fisheries; nf++) {`

```c
  393: 	/* The average catch tracer */
  394: 	FCindex = 0;
  395→ 	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  396: 		if (FisheryArray[nf].isRec == TRUE) {
  397: 			if(recCount > 1){
```

**Suggested fix (from review):** Use "Average catch size of fishery %d (%s)"

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

## HRV2-10

**memory leak** · Conf H / Sev L  
**Found at (current code):** `atHarvestSetup.c` 424
**Lines at review (Oct 2026):** 172-205

**Function:** `Harvest_Free()`

**Why it's thought to be a bug**

`Harvest_Free()` frees tsCatch but never `tsDiscard`, and never frees `qSTOCK` (allocated at line 454). Function: `Harvest_Free()`.

**Code at issue** — `atHarvestSetup.c` line 424  
Search for: `Harvest_Free()`

```c
  422:  * \brief Allocate the memory needed by the harvest module.
  423:  *
  424→  * These arrays are freed in Harvest_Free().
  425:  *
  426:  */
```

**Suggested fix (from review):** Add `Harvest_Free_Time_Series(tsDiscard, ntsDiscard)` and `free3d(qSTOCK)`

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

atHarvestSetup.c `Calculate_Age_Vulnerability()` (634): the size used for selectivity comes from box 0 layer 0 initial tracers. If box 0 is a boundary or empty box, Wgt=0 and li is meaningless. Also, if no cohort reaches sel>0.5, Age50pcntV keeps whatever was read in (it is not set to numCohorts-1).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
