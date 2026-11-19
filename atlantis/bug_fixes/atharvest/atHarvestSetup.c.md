# atharvest/atHarvestSetup.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [HRV2-07](#hrv2-07) | 644-651 | M/M | cohort×genotype confusion | The comparison uses the age class (`basechrt < age50`, and `age50 = basechrt`), but the stored value is the ge… |  | [ ] |
| [HRV2-08](#hrv2-08) | 78 | M/L | NULL dereference | `*(strstr(convertedXMLFileName, ".prm")) = '\0';` crashes if the fisheries prm filename has no ".prm" extensio… |  | [ ] |
| [HRV2-09](#hrv2-09) | 393-412 | H/L | copy-paste / metadata | Average catch size tracers get the long name "Total effort of (recreational) fishery ...", copied from the eff… |  | [ ] |
| [HRV2-10](#hrv2-10) | 172-205 | H/L | memory leak | `Harvest_Free()` frees tsCatch but never `tsDiscard`, and never frees `qSTOCK` (allocated at line 454). Functi… |  | [ ] |

---

## HRV2-07

**cohort×genotype confusion** · Conf M / Sev M  
**Listed lines:** 644-651

**Function:** `Calculate_Age_Vulnerability()`

**Why it's thought to be a bug**

The comparison uses the age class (`basechrt < age50`, and `age50 = basechrt`), but the stored value is the genotype-expanded index: `speciesParams[Age50pcntV_id] = chrt`. Age95 has the same problem. Consumers treat it as an age class (`va50` in atassess/atclassical.c:839,1143,1410). With numGeneTypes>1 the stored age is wrong and can be >= numCohorts. Function: `Calculate_Age_Vulnerability()`.

**Current code** (`atharvest/atHarvestSetup.c` lines 642–653)

```c
  642: 
  643: 						/* 50% vulnerability to the fisheries */
  644: 						if ((sel > 0.50) && (basechrt < age50)) {
  645: 							FunctGroupArray[sp].speciesParams[Age50pcntV_id] = chrt;
  646: 							age50 = basechrt;
  647: 						}
  648: 						/* 95% vulnerability to the fisheries */
  649: 						if ((sel > 0.95) && (basechrt < age95)) {
  650: 							FunctGroupArray[sp].speciesParams[Age95pcntV_id] = chrt;
  651: 							age95 = basechrt;
  652: 						}
  653: 					}
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
**Listed lines:** 78

**Function:** `Harvest_Init()`

**Why it's thought to be a bug**

`*(strstr(convertedXMLFileName, ".prm")) = '\0';` crashes if the fisheries prm filename has no ".prm" extension. Function: `Harvest_Init()`.

**Current code** (`atharvest/atHarvestSetup.c` lines 76–80)

```c
   76: 	/* Build the converted filename */
   77: 	snprintf(convertedXMLFileName, sizeof(convertedXMLFileName), "%s", bm->fishprmIfname);
   78: 	*(strstr(convertedXMLFileName, ".prm")) = '\0';
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
**Listed lines:** 393-412

**Function:** `Build_Fishery_Tracers()`

**Why it's thought to be a bug**

Average catch size tracers get the long name "Total effort of (recreational) fishery ...", copied from the effort tracer block. Line 408 even says "This should be different but is not!". Units "mg N" are questionable too. Function: `Build_Fishery_Tracers()`.

**Current code** (`atharvest/atHarvestSetup.c` lines 391–414) — ⚠ the quoted code was not found in the current file (changed, or already fixed?). These are just the listed line numbers, which may now hold unrelated code; `Build_Fishery_Tracers()` now starts at line 261

```c
  391: 		Ecology_Add_FStat_Tracer(bm, diagIndex++, name, longName, "Days", 0, 1, &FisheryArray[nf].totalEffortTracer, 0, 2, 0);
  392: 	}
  393: 	/* The average catch tracer */
  394: 	FCindex = 0;
  395: 	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  396: 		if (FisheryArray[nf].isRec == TRUE) {
  397: 			if(recCount > 1){
  398: 				snprintf(name, sizeof(name), "Avg_Catch_Sze_Recfish%d", FCindex + 1);
  399: 				snprintf(longName, sizeof(longName), "Total effort of recreational fishery %d (%s)", FCindex + 1, FisheryArray[nf].fisheryCode);
  400: 				FCindex++;
  401: 			}else{
  402: 				snprintf(name, sizeof(name), "Avg_Catch_Sze_Recfish");
  403: 				snprintf(longName, sizeof(longName), "Total effort of recreational fishery");
  404: 			}
  405: 		} else {
  406: 			snprintf(name, sizeof(name), "Avg_Catch_Sze_FC%d", FCindex + 1);
  407: 
  408: 			/* This should be different but is not!*/
  409: 			snprintf(longName, sizeof(longName), "Total effort of fishery %d (%s)", FCindex + 1, FisheryArray[nf].fisheryCode);
  410: 			FCindex++;
  411: 		}
  412: 		Ecology_Add_FStat_Tracer(bm, diagIndex++, name, longName, "mg N", 0, 1, &FisheryArray[nf].averageCatchSizeTracer, 0, 2, 0);
  413: 	}
  414: 
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
**Listed lines:** 172-205

**Function:** `Harvest_Free()`

**Why it's thought to be a bug**

`Harvest_Free()` frees tsCatch but never `tsDiscard`, and never frees `qSTOCK` (allocated at line 454). Function: `Harvest_Free()`.

**Current code** (`atharvest/atHarvestSetup.c` lines 170–207)

```c
  170:  *
  171:  */
  172: void Harvest_Free(MSEBoxModel *bm) {
  173: 
  174: 	if (!bm->flag_fisheries_on) {
  175: 		return;
  176: 	}
  177: 
  178: 	i_free2d(FisherySpeciesCatchFlags);
  179: 
  180: 	/* Harvest Index arrays */
  181: 	c_free2d(harvestindxNAME);
  182: 	free2d(harvestindx);
  183: 	free3d(TotCumCatch);
  184: 
  185: 	Close_Harvest_Output_Files(bm);
  186: 
  187: 	/* Free the arrays */
  188: 	free3d(bm->selectivity);
  189: 	free2d(k_cover);
  190: 	free3d(FFCDR);
  191: 	free2d(CatchSum);
  192:     free2d(OldCatchSum);
  193: 	i_free1d(checkedBox);
  194: 
  195: 	free3d(SELchange);
  196: 	free3d(Pchange);
  197: 	free3d(SWEPTchange);
  198: 	free4d(Qchange);
  199: 	free4d(mFCchange);
  200: 	free4d(DISCRDchange);
  201: 
  202: 	Harvest_Free_Time_Series(tsCatch, ntsCatch);
  203: 	i_free1d(tscatchid);
  204: 	i_free1d(tsdiscardid);
  205: 
  206: }
  207: 
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
