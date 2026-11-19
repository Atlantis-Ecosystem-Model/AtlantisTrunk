# atassess/atassessParamIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **17 findings** (severity H: 6; confidence H and severity H: 2; on the priority list: 2).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [API-01](#api-01) ✔ ⭐ | 647, 632 | H/H | wrong variable | `speciesParamStructArray[sp]` is the parameter-descriptor table `[tot_prms]`, indexed here by species. `Multis… |  | [ ] |
| [API-02](#api-02) | 614 | M/H | NULL deref | `RBCspeciesParam` is only allocated when `useRBCTiers`; it's used when `useMultispAssess` is set, so it crashe… |  | [ ] |
| [API-02b](#api-02b) | 589 | H/L | no NULL check | `xmlReadFileDestFolder` result is not checked, so a missing file segfaults. |  | [ ] |
| [API-03](#api-03) | 1502 | M/H | unchecked bounds | Stock loop bound is `NumRegions` from XML, never checked against `K_num_stocks_per_sp`. `CvLA0`/`CvLAmax` are … |  | [ ] |
| [API-03b](#api-03b) | 741 | H/L | cosmetic | `\n` embedded in the error-path string. |  | [ ] |
| [API-04](#api-04) ✔ ⭐ | 893 | H/H | wrong variable | `FunctGroupArray[sp].speciesParams[sp] == TRUE`: species index used as a parameter id. Can also write to an un… |  | [ ] |
| [API-04b](#api-04b) | 897 | H/L | dead check | `scale_index = b; if (scale_index != -1)` is always true. |  | [ ] |
| [API-05](#api-05) | 923 | H/L | copy-paste | Opens `assessprmIfname` but the error message prints `forceIfname`. Line 916 duplicates line 918. |  | [ ] |
| [API-06](#api-06) | 966 | M/M | wrong check type | `Num_Sexes` is read with `binary_check`, so 2 sexes is impossible and 0 is allowed. |  | [ ] |
| [API-06b](#api-06b) | 1083 | H/L | copy-paste | Error says 'Tier6/TriggerPoints' but the value is read from 'Tier_Assessment'. |  | [ ] |
| [API-06c](#api-06c) | 1011 | L/L | uninitialised | `CloseKinEst` is a per-species malloc but only `[0]` is filled. Seeds at 1007-1009 come from consecutive `cloc… |  | [ ] |
| [API-07](#api-07) | 601 | L/H | bounds | 4 `detrit_import` rows are written but `K_num_detritus*2` are allocated, so it overruns if there is only 1 det… |  | [ ] |
| [API-08](#api-08) | 1489 | M/H | unchecked bounds | Arrays sized `K_num_fisheries` are filled with XML `NumFisheries`; `Mzero`/`MeanLenAge`/`Fecund` are sized `Ov… |  | [ ] |
| [API-09](#api-09) | 1165 | M/L | leak | `RAssessRscriptName` is re-allocated and the first allocation leaks. |  | [ ] |
| [API-10](#api-10) | 1150 | L/M | truncation | `min_TACchange = (int)...` would truncate a proportion to 0. |  | [ ] |
| [API-11](#api-11) | 1485 | L/M | possibly inverted condition | `MeanLenAge` is read when `!flagLAdirect`, which looks opposite to the SigmaLenAge pattern. |  | [ ] |
| [API-12](#api-12) | 496 | M/L | float == TRUE | `assess_flag == TRUE` here; elsewhere the test is `> 0`. |  | [ ] |

---

## API-01

**wrong variable** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Found at (current code):** `atassessParamIO.c` 647, `atassessParamIO.c` 632
**Lines at review (Oct 2026):** 647-652

**Why it's thought to be a bug**

`speciesParamStructArray[sp]` is the parameter-descriptor table `[tot_prms]`, indexed here by species. `MultispAssessType_id` is an RBC parameter (read on line 1052 into `RBCspeciesParam`), but is read from `FunctGroupArray[sp].speciesParams`. So `PGMSY_on` gets set wrongly.

**Code at issue** — `atassessParamIO.c` line 647  
Search for: `speciesParamStructArray[sp]`

```c
  645:         
  646:         for(sp=0; sp<bm->K_num_tot_sp; sp++){
  647→             // Nov 2026: was testing speciesParamStructArray[sp] (indexed by species not parameter) and reading
  648:             // MultispAssessType from the biological speciesParams array rather than RBCspeciesParam
  649:             if (FunctGroupArray[sp].isImpacted == TRUE) {
```

**Code at issue** — `atassessParamIO.c` line 632  
Search for: `FunctGroupArray[sp].speciesParams`

```c
  630:         /* Just for testing purposes
  631:         for(sp=0; sp<bm->K_num_tot_sp; sp++){
  632→             if(FunctGroupArray[sp].speciesParams[assess_flag_id] > 0)
  633:                 GenData(bm, sp, 0);
  634:         }
```

**Suggested fix (from review):** use `isImpacted == TRUE` and `RBCspeciesParam[sp][MultispAssessType_id]`

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

## API-02

**NULL deref** · Conf M / Sev H  
**Found at (current code):** `atassessParamIO.c` 614
**Lines at review (Oct 2026):** 639-660 (alloc 614-617)

**Why it's thought to be a bug**

`RBCspeciesParam` is only allocated when `useRBCTiers`; it's used when `useMultispAssess` is set, so it crashes if RBC tiers are off.

**Code at issue** — `atassessParamIO.c` line 614 (located by name only; check this is the right place)  
Search for: `if(bm->useRBCTiers){`

```c
  612:     readRAssessFlagXML(bm, filename, inputDoc->children);
  613: 
  614→ 	if(bm->useRBCTiers){
  615:         /* Preallocate the arrays and load parameters */
  616:         bm->RBCestimation.RBCspeciesArray = (RBCarrays *) malloc(sizeof(RBCarrays) * (unsigned long int)bm->K_num_tot_sp);
```

**Suggested fix (from review):** require useRBCTiers, or nest the block

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

## API-02b

**no NULL check** · Conf H / Sev L  
**Found at (current code):** `atassessParamIO.c` 589
**Lines at review (Oct 2026):** 589

**Why it's thought to be a bug**

`xmlReadFileDestFolder` result is not checked, so a missing file segfaults.

**Code at issue** — `atassessParamIO.c` line 589 (the line the review pointed to, found by its text)  
Search for: `inputDoc = xmlReadFileDestFolder(bm->destFolder, filename, NULL, 0);`

```c
  587:     int sp;
  588: 	xmlDocPtr inputDoc;
  589→ 	inputDoc = xmlReadFileDestFolder(bm->destFolder, filename, NULL, 0);
  590: 
  591: 	printf("Assessment parameter file loading\n");
```

**Suggested fix (from review):** `if (!inputDoc) quit(...)`

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

## API-03

**unchecked bounds** · Conf M / Sev H  
**Found at (current code):** `atassessParamIO.c` 1502
**Lines at review (Oct 2026):** 722-815

**Why it's thought to be a bug**

Stock loop bound is `NumRegions` from XML, never checked against `K_num_stocks_per_sp`. `CvLA0`/`CvLAmax` are allocated with `K_num_reg` but filled by the stock loop.

**Code at issue** — `atassessParamIO.c` line 1502 (located by name only; check this is the right place)  
Search for: `attributeGroup = getParentNode(bm, fileName, groupingNode, "CvLA0", errorString, sizeof(er`

```c
 1500:     
 1501:     // by sex and stock
 1502→ 	attributeGroup = getParentNode(bm, fileName, groupingNode, "CvLA0", errorString, sizeof(errorString));
 1503:     for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
 1504:         if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
```

**Suggested fix (from review):** check bound; allocate with `K_num_stocks_per_sp`

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

## API-03b

**cosmetic** · Conf H / Sev L  
**Found at (current code):** `atassessParamIO.c` 741
**Lines at review (Oct 2026):** 741, 799

**Why it's thought to be a bug**

`\n` embedded in the error-path string.

**Code at issue** — `atassessParamIO.c` line 741 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `for (ns = 0; ns < bm->K_num_sexes; ns++) {`

```c
  739:             quit("%s/%s species attribute group not found.\n", errorString, FunctGroupArray[guild].groupCode);
  740:             
  741→         for (ns = 0; ns < bm->K_num_sexes; ns++) {
  742:             snprintf(str, sizeof(str), "%s", sexStrings[ns]);
  743:             snprintf(eString, sizeof(eString), "%s/%s/%s\n", errorString, FunctGroupArray[guild].groupCode, str);
```

**Suggested fix (from review):** drop `\n`

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

## API-04

**wrong variable** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Found at (current code):** `atassessParamIO.c` 893
**Lines at review (Oct 2026):** 891

**Why it's thought to be a bug**

`FunctGroupArray[sp].speciesParams[sp] == TRUE`: species index used as a parameter id. Can also write to an unallocated `TAC_mult`.

**Code at issue** — `atassessParamIO.c` line 893  
Search for: `FunctGroupArray[sp].speciesParams[sp] == TRUE`

```c
  891: 		/* Set up the indexing on the names in the forcing file */
  892: 		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  893→ 			if ((FunctGroupArray[sp].speciesParams[flag_id] == TRUE) && ((FunctGroupArray[sp].isImpacted == TRUE) || (FunctGroupArray[sp].speciesParams[sp] == TRUE))) {
  894: 				snprintf(str, sizeof(str), "%s", FunctGroupArray[sp].groupCode);
  895: 				for (b = 0; b < bm->tsTACmult->nv; b++) {
```

**Suggested fix (from review):** `speciesParams[assess_flag_id] > 0` (or `isFished`)

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

## API-04b

**dead check** · Conf H / Sev L  
**Found at (current code):** `atassessParamIO.c` 897
**Lines at review (Oct 2026):** 895-896

**Why it's thought to be a bug**

`scale_index = b; if (scale_index != -1)` is always true.

**Code at issue** — `atassessParamIO.c` line 897  
Search for: `scale_index = b;`

```c
  895: 				for (b = 0; b < bm->tsTACmult->nv; b++) {
  896: 					if (strcmp(str, bm->tsTACmult->varname[b]) == 0){
  897→ 						scale_index = b;
  898: 						if(scale_index != -1){
  899: 							for(titer = 0; titer < bm->RBCestimation.RBCspeciesParam[sp][Nyears_id]; titer++){
```

**Suggested fix (from review):** tidy

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

## API-05

**copy-paste** · Conf H / Sev L  
**Found at (current code):** `atassessParamIO.c` 923
**Lines at review (Oct 2026):** 921-922

**Why it's thought to be a bug**

Opens `assessprmIfname` but the error message prints `forceIfname`. Line 916 duplicates line 918.

**Code at issue** — `atassessParamIO.c` line 923 (located by name only; check this is the right place)  
Search for: `if ((fp = fopen(bm->assessprmIfname, "r")) == NULL)`

```c
  921: 
  922: 	/* Open the file */
  923→ 	if ((fp = fopen(bm->assessprmIfname, "r")) == NULL)
  924: 		quit("get_TAC_mult: Can't open %s\n", bm->forceIfname);
  925: 
```

**Suggested fix (from review):** print the right name

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

## API-06

**wrong check type** · Conf M / Sev M  
**Found at (current code):** `atassessParamIO.c` 966
**Lines at review (Oct 2026):** 964

**Why it's thought to be a bug**

`Num_Sexes` is read with `binary_check`, so 2 sexes is impossible and 0 is allowed.

**Code at issue** — `atassessParamIO.c` line 966 (located by name only; check this is the right place)  
Search for: `bm->K_num_sexes = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, g`

```c
  964:     
  965:     bm->assess_share_params = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "assess_share_params");
  966→ 	bm->K_num_sexes = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "Num_Sexes");
  967:     bm->RBCestimation.HistYrMin = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "HistYrMin");
  968: 	bm->RBCestimation.HistYrMax = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "HistYrMax");
```

**Suggested fix (from review):** `integer_check`, 1–2

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

## API-06b

**copy-paste** · Conf H / Sev L  
**Found at (current code):** `atassessParamIO.c` 1083
**Lines at review (Oct 2026):** 1082

**Why it's thought to be a bug**

Error says 'Tier6/TriggerPoints' but the value is read from 'Tier_Assessment'.

**Code at issue** — `atassessParamIO.c` line 1083 (located by name only; check this is the right place)  
Search for: `if(Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "Tier_Assessment", groupingNod`

```c
 1081:     
 1082:     bm->RBCestimation.NumTriggers = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "NumTriggers"));
 1083→     if(Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "Tier_Assessment", groupingNode, no_checking, "TriggerPoints", &TrigPts, bm->RBCestimation.NumTriggers) == FALSE){
 1084: 		quit("Error: Unable to read parameter 'Tier6/TriggerPoints' from input file %s\n", fileName);
 1085: 	}
```

**Suggested fix (from review):** fix message

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

## API-06c

**uninitialised** · Conf L / Sev L  
**Found at (current code):** `atassessParamIO.c` 1011
**Lines at review (Oct 2026):** 1086-1099 (alloc 619)

**Why it's thought to be a bug**

`CloseKinEst` is a per-species malloc but only `[0]` is filled. Seeds at 1007-1009 come from consecutive `clock()` calls, so they are likely identical.

**Code at issue** — `atassessParamIO.c` line 1011  
Search for: `clock()`

```c
 1009: 	if (!bm->RBCestimation.Iseedx) bm->RBCestimation.Iseedx = (int)clock();
 1010: 	if (!bm->RBCestimation.Iseedy) bm->RBCestimation.Iseedy = (int)clock();
 1011→ 	if (!bm->RBCestimation.Iseedz) bm->RBCestimation.Iseedz = (int)clock();
 1012:     
 1013:     /* SS related flags */
```

**Suggested fix (from review):** calloc / separate struct

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

## API-07

**bounds** · Conf L / Sev H  
**Found at (current code):** `atassessParamIO.c` 601
**Lines at review (Oct 2026):** 438-441, 53 (alloc 601)

**Why it's thought to be a bug**

4 `detrit_import` rows are written but `K_num_detritus*2` are allocated, so it overruns if there is only 1 detritus group.

**Code at issue** — `atassessParamIO.c` line 601  
Search for: `K_num_detritus * 2`

```c
  599: 	printf("Create assessment detritus flow vectors\n");
  600: 
  601→ 	detrit_import = (double **) alloc2d(bm->nfzones, bm->K_num_detritus * 2);
  602: 	DiscardFate = (double **) alloc2d(2, bm->K_num_detritus);
  603: 
```

**Suggested fix (from review):** allocate `max(4, 2*K_num_detritus)`

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

## API-08

**unchecked bounds** · Conf M / Sev H  
**Found at (current code):** `atassessParamIO.c` 1489
**Lines at review (Oct 2026):** 1182-1311, 1476, 1483, 1541

**Why it's thought to be a bug**

Arrays sized `K_num_fisheries` are filled with XML `NumFisheries`; `Mzero`/`MeanLenAge`/`Fecund` are sized `OverallMaxAge` but filled to `MaxAge`. The guard in `Tier_Assessment_Setup` runs after this read.

**Code at issue** — `atassessParamIO.c` line 1489 (the line the review pointed to, found by its text)  
Search for: `readStockAgeAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCsp`

```c
 1487:          if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
 1488:              if (!bm->RBCestimation.RBCspeciesParam[guild][flagLAdirect_id])
 1489→                  readStockAgeAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].MeanLenAge, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][MaxAge_id]);
 1490:     }
 1491:     
```

**Suggested fix (from review):** check bounds before reading

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

## API-09

**leak** · Conf M / Sev L  
**Found at (current code):** `atassessParamIO.c` 1165
**Lines at review (Oct 2026):** 1158-1163 (and 565)

**Why it's thought to be a bug**

`RAssessRscriptName` is re-allocated and the first allocation leaks.

**Code at issue** — `atassessParamIO.c` line 1165 (located by name only; check this is the right place)  
Search for: `bm->RAssessRscriptName = c_alloc2d(BMSLEN, bm->NumRAssessScriptNames);`

```c
 1163:         
 1164:         bm->NumRAssessScriptNames = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "NumRAssessScriptNames");
 1165→         bm->RAssessRscriptName = c_alloc2d(BMSLEN, bm->NumRAssessScriptNames);
 1166:         for (nf = 0; nf < bm->NumRAssessScriptNames; nf ++) {
 1167:             snprintf(nameString, sizeof(nameString), "RAssessRscriptName%d", nf);
```

**Suggested fix (from review):** free first / read once

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

## API-10

**truncation** · Conf L / Sev M  
**Found at (current code):** `atassessParamIO.c` 1150
**Lines at review (Oct 2026):** 1148

**Why it's thought to be a bug**

`min_TACchange = (int)...` would truncate a proportion to 0.

**Code at issue** — `atassessParamIO.c` line 1150 (located by name only; check this is the right place)  
Search for: `bm->RBCestimation.min_TACchange = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ec`

```c
 1148: 	get_TAC_mult(bm,"TAC_mult", &bm->tsTACmult, bm->t_units, warn);
 1149: 
 1150→ 	bm->RBCestimation.min_TACchange = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "RBC_TACchange_min");   // Nov 2026: was cast to int so fractions became 0
 1151: 
 1152:     bm->RBCestimation.MULTISPestNspecies = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "MULTISPestNspecies");
```

**Suggested fix (from review):** check field type

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

## API-11

**possibly inverted condition** · Conf L / Sev M  
**Found at (current code):** `atassessParamIO.c` 1485
**Lines at review (Oct 2026):** 1482

**Why it's thought to be a bug**

`MeanLenAge` is read when `!flagLAdirect`, which looks opposite to the SigmaLenAge pattern.

**Code at issue** — `atassessParamIO.c` line 1485 (located by name only; check this is the right place)  
Search for: `attributeGroup = getParentNode(bm, fileName, groupingNode, "MeanLenAge", errorString, size`

```c
 1483:     }
 1484:     
 1485→ 	attributeGroup = getParentNode(bm, fileName, groupingNode, "MeanLenAge", errorString, sizeof(errorString));
 1486:     for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
 1487:          if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
```

**Suggested fix (from review):** confirm meaning

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

## API-12

**float == TRUE** · Conf M / Sev L  
**Found at (current code):** `atassessParamIO.c` 496
**Lines at review (Oct 2026):** 496

**Why it's thought to be a bug**

`assess_flag == TRUE` here; elsewhere the test is `> 0`.

**Code at issue** — `atassessParamIO.c` line 496 (the line the review pointed to, found by its text)  
Search for: `if (FunctGroupArray[sp].speciesParams[assess_flag_id] == TRUE)`

```c
  494: 			case BIOMASS:
  495: 				/* So far only doing estimates for vertebrate species */
  496→ 				if (FunctGroupArray[sp].speciesParams[assess_flag_id] == TRUE)
  497: 					warn("%s originally set to being assessed in assess (sampling) prm, but no biomass pool groups may be assessed as yet\n",
  498: 							FunctGroupArray[sp].groupCode);
```

**Suggested fix (from review):** `> 0`

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
