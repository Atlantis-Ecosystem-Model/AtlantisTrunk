# atassess/atassessParamIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **17 findings** (severity H: 6; confidence H and severity H: 2; on the priority list: 2).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [API-01](#api-01) ✔ ⭐ | 647-652 | H/H | wrong variable | `speciesParamStructArray[sp]` is the parameter-descriptor table `[tot_prms]`, indexed here by species. `Multis… |  | [ ] |
| [API-02](#api-02) | 639-660 (alloc 614-617) | M/H | NULL deref | `RBCspeciesParam` is only allocated when `useRBCTiers`; it's used when `useMultispAssess` is set, so it crashe… |  | [ ] |
| [API-02b](#api-02b) | 589 | H/L | no NULL check | `xmlReadFileDestFolder` result is not checked, so a missing file segfaults. |  | [ ] |
| [API-03](#api-03) | 722-815 | M/H | unchecked bounds | Stock loop bound is `NumRegions` from XML, never checked against `K_num_stocks_per_sp`. `CvLA0`/`CvLAmax` are … |  | [ ] |
| [API-03b](#api-03b) | 741, 799 | H/L | cosmetic | `\n` embedded in the error-path string. |  | [ ] |
| [API-04](#api-04) ✔ ⭐ | 891 | H/H | wrong variable | `FunctGroupArray[sp].speciesParams[sp] == TRUE`: species index used as a parameter id. Can also write to an un… |  | [ ] |
| [API-04b](#api-04b) | 895-896 | H/L | dead check | `scale_index = b; if (scale_index != -1)` is always true. |  | [ ] |
| [API-05](#api-05) | 921-922 | H/L | copy-paste | Opens `assessprmIfname` but the error message prints `forceIfname`. Line 916 duplicates line 918. |  | [ ] |
| [API-06](#api-06) | 971 | M/M | wrong check type | `Num_Sexes` is read with `binary_check`, so 2 sexes is impossible and 0 is allowed. |  | [ ] |
| [API-06b](#api-06b) | 1082 | H/L | copy-paste | Error says 'Tier6/TriggerPoints' but the value is read from 'Tier_Assessment'. |  | [ ] |
| [API-06c](#api-06c) | 1086-1099 (alloc 619) | L/L | uninitialised | `CloseKinEst` is a per-species malloc but only `[0]` is filled. Seeds at 1007-1009 come from consecutive `cloc… |  | [ ] |
| [API-07](#api-07) | 438-441, 53 (alloc 601) | L/H | bounds | 4 `detrit_import` rows are written but `K_num_detritus*2` are allocated, so it overruns if there is only 1 det… |  | [ ] |
| [API-08](#api-08) | 1182-1311, 1476, 1483, 1541 | M/H | unchecked bounds | Arrays sized `K_num_fisheries` are filled with XML `NumFisheries`; `Mzero`/`MeanLenAge`/`Fecund` are sized `Ov… |  | [ ] |
| [API-09](#api-09) | 1158-1163 (and 565) | M/L | leak | `RAssessRscriptName` is re-allocated and the first allocation leaks. |  | [ ] |
| [API-10](#api-10) | 1148 | L/M | truncation | `min_TACchange = (int)...` would truncate a proportion to 0. |  | [ ] |
| [API-11](#api-11) | 1482 | L/M | possibly inverted condition | `MeanLenAge` is read when `!flagLAdirect`, which looks opposite to the SigmaLenAge pattern. |  | [ ] |
| [API-12](#api-12) | 496 | M/L | float == TRUE | `assess_flag == TRUE` here; elsewhere the test is `> 0`. |  | [ ] |

---

## API-01

**wrong variable** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Lines (current code):** 647-652  
**Lines at review (Oct 2026):** 647-652

**Why it's thought to be a bug**

`speciesParamStructArray[sp]` is the parameter-descriptor table `[tot_prms]`, indexed here by species. `MultispAssessType_id` is an RBC parameter (read on line 1052 into `RBCspeciesParam`), but is read from `FunctGroupArray[sp].speciesParams`. So `PGMSY_on` gets set wrongly.

**Current code** (`atassess/atassessParamIO.c` lines 645–654)

```c
  645:         
  646:         for(sp=0; sp<bm->K_num_tot_sp; sp++){
  647:             // Nov 2026: was testing speciesParamStructArray[sp] (indexed by species not parameter) and reading
  648:             // MultispAssessType from the biological speciesParams array rather than RBCspeciesParam
  649:             if (FunctGroupArray[sp].isImpacted == TRUE) {
  650:                 if ((bm->RBCestimation.RBCspeciesParam[sp][MultispAssessType_id] > 0) && ((int)(bm->RBCestimation.RBCspeciesParam[sp][MultispAssessType_id]) < (IndicatorSpPGMSY + 1))) {
  651:                     bm->PGMSY_on = TRUE;
  652:                     // Make sure the R file is in place
  653:                 
  654:                     fprintf(bm->logFile,"Set PGMSY_on as %s has MultispAssessType %e vs threshold %d\n", FunctGroupArray[sp].groupCode, bm->RBCestimation.RBCspeciesParam[sp][MultispAssessType_id], (IndicatorSpPGMSY + 1));
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
**Lines (current code):** 639-660 (alloc 614-617)  
**Lines at review (Oct 2026):** 639-660 (alloc 614-617)

**Why it's thought to be a bug**

`RBCspeciesParam` is only allocated when `useRBCTiers`; it's used when `useMultispAssess` is set, so it crashes if RBC tiers are off.

**Current code** (`atassess/atassessParamIO.c` lines 637–662)

```c
  637: 	}
  638:     
  639:     if (bm->useMultispAssess) {
  640:         bm->RBCestimation.earliestYr = MAXINT;
  641:         
  642:         if(bm->RBCestimation.UseAtlantisPGMSY > 0) {
  643:             bm->PGMSY_on = TRUE;
  644:         }
  645:         
  646:         for(sp=0; sp<bm->K_num_tot_sp; sp++){
  647:             // Nov 2026: was testing speciesParamStructArray[sp] (indexed by species not parameter) and reading
  648:             // MultispAssessType from the biological speciesParams array rather than RBCspeciesParam
  649:             if (FunctGroupArray[sp].isImpacted == TRUE) {
  650:                 if ((bm->RBCestimation.RBCspeciesParam[sp][MultispAssessType_id] > 0) && ((int)(bm->RBCestimation.RBCspeciesParam[sp][MultispAssessType_id]) < (IndicatorSpPGMSY + 1))) {
  651:                     bm->PGMSY_on = TRUE;
  652:                     // Make sure the R file is in place
  653:                 
  654:                     fprintf(bm->logFile,"Set PGMSY_on as %s has MultispAssessType %e vs threshold %d\n", FunctGroupArray[sp].groupCode, bm->RBCestimation.RBCspeciesParam[sp][MultispAssessType_id], (IndicatorSpPGMSY + 1));
  655:                 
  656:                 }
  657:             }
  658:             if ((FunctGroupArray[sp].isFished == TRUE)  && ((int)(bm->RBCestimation.RBCspeciesParam[sp][AssessStart_id]) < bm->RBCestimation.earliestYr)) {
  659:                 bm->RBCestimation.earliestYr = bm->RBCestimation.RBCspeciesParam[sp][AssessStart_id];
  660:             }
  661:             
  662:             fprintf(bm->logFile,"Setting earliestYr as %s has isFished: %d AssessStart %d vs earliestYr %d\n", FunctGroupArray[sp].groupCode, FunctGroupArray[sp].isFished, ((int)(bm->RBCestimation.RBCspeciesParam[sp][AssessStart_id])), bm->RBCestimation.earliestYr);
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
**Lines (current code):** 589  
**Lines at review (Oct 2026):** 589

**Why it's thought to be a bug**

`xmlReadFileDestFolder` result is not checked, so a missing file segfaults.

**Current code** (`atassess/atassessParamIO.c` lines 587–591)

```c
  587:     int sp;
  588: 	xmlDocPtr inputDoc;
  589: 	inputDoc = xmlReadFileDestFolder(bm->destFolder, filename, NULL, 0);
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
**Lines (current code):** 722-815  
**Lines at review (Oct 2026):** 722-815

**Why it's thought to be a bug**

Stock loop bound is `NumRegions` from XML, never checked against `K_num_stocks_per_sp`. `CvLA0`/`CvLAmax` are allocated with `K_num_reg` but filled by the stock loop.

**Current code** (`atassess/atassessParamIO.c` lines 720–759 (truncated))

```c
  720:     xmlNodePtr speciesNode, sexNode;
  721:     char str[STRLEN];
  722:     char eString[STRLEN*2];
  723:     
  724:     nloop = (int)bm->RBCestimation.RBCspeciesParam[guild][NumRegions_id];
  725:     if (FunctGroupArray[guild].speciesParams[assess_flag_id] < 1)
  726:         nloop = 1;
  727:     
  728:     if (bm->assess_share_params)
  729:         nloop = 1;
  730:     
  731: 
  732:     if (verbose)
  733:         printf("Reading stock based assessment parameters\n");
  734: 
  735:     if (FunctGroupArray[guild].isFished == TRUE) {
  736:             
  737:         speciesNode = Util_XML_Get_Node(ATLANTIS_GROUP_ATTRIBUTE, attributeGroup, FunctGroupArray[guild].groupCode);
  738:         if (speciesNode == NULL)
  739:             quit("%s/%s species attribute group not found.\n", errorString, FunctGroupArray[guild].groupCode);
  740:             
  741:         for (ns = 0; ns < bm->K_num_sexes; ns++) {
  742:             snprintf(str, sizeof(str), "%s", sexStrings[ns]);
  743:             snprintf(eString, sizeof(eString), "%s/%s/%s\n", errorString, FunctGroupArray[guild].groupCode, str);
  744:                 
  745:             sexNode = Util_XML_Get_Node(ATLANTIS_SEX_ATTRIBUTE, speciesNode, str);
  746:             if (sexNode == NULL)
  747:                 quit("%s gender attribute group not found.\n", eString);
  748:                 
  749:             for (nstock = 0; nstock < nloop; nstock++) {
  750:                 snprintf(str, sizeof(str), "stock%d", nstock+1);
  751:         
  752:                 if (Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, eString, sexNode, checkType, str, &values, length) == FALSE) {
  753:                     quit("Error: Unable to find parameter '%s/%s/%s' in input file %s\n", errorString, FunctGroupArray[guild].groupCode, str, fileName);
  754:                 }
  755: 
  756:                 for (index = 0; index < length; index++) {
  757:                     array[nstock][ns][index] = values[index];
  758:                 }
  759:                 free(values);
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
**Lines (current code):** 741, 799  
**Lines at review (Oct 2026):** 741, 799

**Why it's thought to be a bug**

`\n` embedded in the error-path string.

**Current code** (`atassess/atassessParamIO.c` lines 739–743)

```c
  739:             quit("%s/%s species attribute group not found.\n", errorString, FunctGroupArray[guild].groupCode);
  740:             
  741:         for (ns = 0; ns < bm->K_num_sexes; ns++) {
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
**Lines (current code):** 891  
**Lines at review (Oct 2026):** 891

**Why it's thought to be a bug**

`FunctGroupArray[sp].speciesParams[sp] == TRUE`: species index used as a parameter id. Can also write to an unallocated `TAC_mult`.

**Current code** (`atassess/atassessParamIO.c` lines 889–893)

```c
  889: 
  890: 	if(bm->tsTACmult != NULL){
  891: 		/* Set up the indexing on the names in the forcing file */
  892: 		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  893: 			if ((FunctGroupArray[sp].speciesParams[flag_id] == TRUE) && ((FunctGroupArray[sp].isImpacted == TRUE) || (FunctGroupArray[sp].speciesParams[sp] == TRUE))) {
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
**Lines (current code):** 895-896  
**Lines at review (Oct 2026):** 895-896

**Why it's thought to be a bug**

`scale_index = b; if (scale_index != -1)` is always true.

**Current code** (`atassess/atassessParamIO.c` lines 893–898)

```c
  893: 			if ((FunctGroupArray[sp].speciesParams[flag_id] == TRUE) && ((FunctGroupArray[sp].isImpacted == TRUE) || (FunctGroupArray[sp].speciesParams[sp] == TRUE))) {
  894: 				snprintf(str, sizeof(str), "%s", FunctGroupArray[sp].groupCode);
  895: 				for (b = 0; b < bm->tsTACmult->nv; b++) {
  896: 					if (strcmp(str, bm->tsTACmult->varname[b]) == 0){
  897: 						scale_index = b;
  898: 						if(scale_index != -1){
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
**Lines (current code):** 921-922  
**Lines at review (Oct 2026):** 921-922

**Why it's thought to be a bug**

Opens `assessprmIfname` but the error message prints `forceIfname`. Line 916 duplicates line 918.

**Current code** (`atassess/atassessParamIO.c` lines 919–924)

```c
  919: 	if (verbose > 1)
  920:         printf("Reading in TAC_mult time series forcing file\n");
  921: 
  922: 	/* Open the file */
  923: 	if ((fp = fopen(bm->assessprmIfname, "r")) == NULL)
  924: 		quit("get_TAC_mult: Can't open %s\n", bm->forceIfname);
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
**Lines (current code):** 971  
**Lines at review (Oct 2026):** 964

**Why it's thought to be a bug**

`Num_Sexes` is read with `binary_check`, so 2 sexes is impossible and 0 is allowed.

**Current code** (`atassess/atassessParamIO.c` lines 969–973)

```c
  969: 	bm->RBCestimation.OverallMaxAge = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "OverallMaxAge");
  970:     
  971:     bm->RBCestimation.tier6_to_tier4 = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "tier6_to_tier4");
  972:     bm->RBCestimation.delTAE = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "delTAE");
  973:     
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
**Lines (current code):** 1082  
**Lines at review (Oct 2026):** 1082

**Why it's thought to be a bug**

Error says 'Tier6/TriggerPoints' but the value is read from 'Tier_Assessment'.

**Current code** (`atassess/atassessParamIO.c` lines 1080–1084)

```c
 1080:     Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, TriggerResponseScen_id);
 1081:     
 1082:     bm->RBCestimation.NumTriggers = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "NumTriggers"));
 1083:     if(Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "Tier_Assessment", groupingNode, no_checking, "TriggerPoints", &TrigPts, bm->RBCestimation.NumTriggers) == FALSE){
 1084: 		quit("Error: Unable to read parameter 'Tier6/TriggerPoints' from input file %s\n", fileName);
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
**Lines (current code):** 1086-1099 (alloc 619)  
**Lines at review (Oct 2026):** 1086-1099 (alloc 619)

**Why it's thought to be a bug**

`CloseKinEst` is a per-species malloc but only `[0]` is filled. Seeds at 1007-1009 come from consecutive `clock()` calls, so they are likely identical.

**Current code** (`atassess/atassessParamIO.c` lines 1084–1101)

```c
 1084: 		quit("Error: Unable to read parameter 'Tier6/TriggerPoints' from input file %s\n", fileName);
 1085: 	}
 1086:     
 1087:     /* Close kin parameters */
 1088:     bm->CloseKinEst->UseCloseKin = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "UseCloseKin");
 1089:     bm->CloseKinEst->nTotSamps = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "nTotSamps");
 1090:     bm->CloseKinEst->first_sy = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "first_sy");
 1091:     bm->CloseKinEst->last_sy = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "last_sy");
 1092:     bm->CloseKinEst->first_dy = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "first_dy");
 1093:     bm->CloseKinEst->Nscale = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "Nscale");
 1094:     bm->CloseKinEst->ThreshPercLost = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "ThreshPercLost");
 1095:     bm->CloseKinEst->samp_power = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "samp_power");
 1096:     bm->CloseKinEst->a_ck_thresh = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "a_ck_thresh");
 1097:     bm->CloseKinEst->a_ck_thresh2 = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "a_ck_thresh2");
 1098:     bm->CloseKinEst->a_ck_scalar = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "a_ck_scalar");
 1099:     bm->CloseKinEst->a_ck_scalar2 = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "a_ck_scalar2");
 1100:     bm->CloseKinEst->thresh_mat = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "thresh_mat");
 1101:     bm->CloseKinEst->HSP_false_neg_loss_rate = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "HSP_false_neg_loss_rate");
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
**Lines (current code):** 438-441, 53 (alloc 601)  
**Lines at review (Oct 2026):** 438-441, 53 (alloc 601)

**Why it's thought to be a bug**

4 `detrit_import` rows are written but `K_num_detritus*2` are allocated, so it overruns if there is only 1 detritus group.

**Current code** (`atassess/atassessParamIO.c` lines 436–443) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
  436: 		quit("readDetritusXML: Detritus_Input attribute group not found in input file %s.\n", fileName);
  437: 
  438: 	readZoneXML(bm, fileName, groupingNode, "input_DLwc", 0);
  439: 	readZoneXML(bm, fileName, groupingNode, "input_DRwc", 1);
  440: 	readZoneXML(bm, fileName, groupingNode, "input_DLsed", 2);
  441: 	readZoneXML(bm, fileName, groupingNode, "input_DRsed", 3);
  442: 
  443: 	/* Read in the discard fate values for detritus */
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
**Lines (current code):** 1182-1311, 1476, 1483, 1541  
**Lines at review (Oct 2026):** 1182-1311, 1476, 1483, 1541

**Why it's thought to be a bug**

Arrays sized `K_num_fisheries` are filled with XML `NumFisheries`; `Mzero`/`MeanLenAge`/`Fecund` are sized `OverallMaxAge` but filled to `MaxAge`. The guard in `Tier_Assessment_Setup` runs after this read.

**Current code** (`atassess/atassessParamIO.c` lines 1180–1219 (truncated))

```c
 1180:     
 1181:     childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "CVs");
 1182: 	if (childGroupingNode == NULL)
 1183: 		quit("readTierAssessmentManagementXML: CVs attribute group not found in input file %s.\n", fileName);
 1184:     
 1185:     attributeGroup = getParentNode(bm, fileName, childGroupingNode, "DiscardCV", errorString, sizeof(errorString));
 1186: 	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
 1187:         if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
 1188:             readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].DiscCV, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][NumFisheries_id]);
 1189: 	}
 1190: 
 1191:     attributeGroup = getParentNode(bm, fileName, childGroupingNode, "CatchCV", errorString, sizeof(errorString));
 1192: 	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
 1193:         if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
 1194:             readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].CatchCV, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][NumFisheries_id]);
 1195: 	}
 1196: 
 1197:     attributeGroup = getParentNode(bm, fileName, childGroupingNode, "EffortCV", errorString, sizeof(errorString));
 1198: 	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
 1199:         if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
 1200:             readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].EffortCV, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][NumFisheries_id]);
 1201: 	}
 1202:     
 1203:     
 1204: 	childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "CPUE");
 1205: 	if (childGroupingNode == NULL)
 1206: 		quit("readTierAssessmentManagementXML: CPUE attribute group not found in input file %s.\n", fileName);
 1207: 
 1208: 	attributeGroup = getParentNode(bm, fileName, childGroupingNode, "CPUECV", errorString, sizeof(errorString));
 1209: 	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
 1210:         if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
 1211:             readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].CPUEcv, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][NumFisheries_id]);
 1212: 	}
 1213: 
 1214: 	attributeGroup = getParentNode(bm, fileName, childGroupingNode, "CPUEvar", errorString, sizeof(errorString));
 1215: 	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
 1216:         if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
 1217:             readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].CPUEvar, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][NumFisheries_id]);
 1218: 	}
 1219: 	attributeGroup = getParentNode(bm, fileName, childGroupingNode, "CPUEcorr", errorString, sizeof(errorString));
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
**Lines (current code):** 1158-1163 (and 565)  
**Lines at review (Oct 2026):** 1158-1163 (and 565)

**Why it's thought to be a bug**

`RAssessRscriptName` is re-allocated and the first allocation leaks.

**Current code** (`atassess/atassessParamIO.c` lines 1156–1165)

```c
 1156:     
 1157:     // Nov 2026: ss3Name now also read when using SS for Tier 1 (was only read for multispecies assessments)
 1158:     if (bm->useMultispAssess || bm->RBCestimation.UseSS) {
 1159:         Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, 1, groupingNode, "ss3Name", bm->ss3Name);
 1160:     }
 1161: 
 1162:     if (bm->useMultispAssess) {        
 1163:         
 1164:         bm->NumRAssessScriptNames = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "NumRAssessScriptNames");
 1165:         bm->RAssessRscriptName = c_alloc2d(BMSLEN, bm->NumRAssessScriptNames);
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
**Lines (current code):** 1148  
**Lines at review (Oct 2026):** 1148

**Why it's thought to be a bug**

`min_TACchange = (int)...` would truncate a proportion to 0.

**Current code** (`atassess/atassessParamIO.c` lines 1146–1150)

```c
 1146:     
 1147: 	bm->RBCestimation.Use_TAC_mult = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "Use_TAC_mult");
 1148: 	get_TAC_mult(bm,"TAC_mult", &bm->tsTACmult, bm->t_units, warn);
 1149: 
 1150: 	bm->RBCestimation.min_TACchange = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "RBC_TACchange_min");   // Nov 2026: was cast to int so fractions became 0
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
**Lines (current code):** 1482  
**Lines at review (Oct 2026):** 1482

**Why it's thought to be a bug**

`MeanLenAge` is read when `!flagLAdirect`, which looks opposite to the SigmaLenAge pattern.

**Current code** (`atassess/atassessParamIO.c` lines 1480–1484) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
 1480:     for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
 1481:         if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
 1482:             readStockAgeAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].Mzero, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][MaxAge_id]);
 1483:     }
 1484:     
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
**Lines (current code):** 496  
**Lines at review (Oct 2026):** 496

**Why it's thought to be a bug**

`assess_flag == TRUE` here; elsewhere the test is `> 0`.

**Current code** (`atassess/atassessParamIO.c` lines 494–498)

```c
  494: 			case BIOMASS:
  495: 				/* So far only doing estimates for vertebrate species */
  496: 				if (FunctGroupArray[sp].speciesParams[assess_flag_id] == TRUE)
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
