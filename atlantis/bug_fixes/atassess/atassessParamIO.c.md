# atassess/atassessParamIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **17 findings** (severity H: 6; confidence H and severity H: 2; on the priority list: 2).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [API-01](#api-01) ✔ ⭐ | 647-652 | H/H | wrong variable | `speciesParamStructArray[sp]` is the parameter-descriptor table `[tot_prms]`, indexed here by species. `Multis… |  | [ ] |
| [API-02](#api-02) | 639-660 (alloc 614-617) | M/H | NULL deref | `RBCspeciesParam` is only allocated when `useRBCTiers`; it's used when `useMultispAssess` is set, so it crashe… |  | [ ] |
| [API-02b](#api-02b) | 589 | H/L | no NULL check | `xmlReadFileDestFolder` result is not checked, so a missing file segfaults. |  | [ ] |
| [API-03](#api-03) | 722-815 | M/H | unchecked bounds | Stock loop bound is `NumRegions` from XML, never checked against `K_num_stocks_per_sp`. `CvLA0`/`CvLAmax` are … |  | [ ] |
| [API-03b](#api-03b) | 741, 799 | H/L | cosmetic | `\n` embedded in the error-path string. |  | [ ] |
| [API-04](#api-04) ✔ ⭐ | 891 | H/H | wrong variable | `FunctGroupArray[sp].speciesParams[sp] == TRUE`: species index used as a parameter id. Can also write to an un… |  | [ ] |
| [API-04b](#api-04b) | 895-896 | H/L | dead check | `scale_index = b; if (scale_index != -1)` is always true. |  | [ ] |
| [API-05](#api-05) | 921-922 | H/L | copy-paste | Opens `assessprmIfname` but the error message prints `forceIfname`. Line 916 duplicates line 918. |  | [ ] |
| [API-06](#api-06) | 964 | M/M | wrong check type | `Num_Sexes` is read with `binary_check`, so 2 sexes is impossible and 0 is allowed. |  | [ ] |
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
**Listed lines:** 647-652

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
**Listed lines:** 639-660 (alloc 614-617)

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
**Listed lines:** 589

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
**Listed lines:** 722-815

**Why it's thought to be a bug**

Stock loop bound is `NumRegions` from XML, never checked against `K_num_stocks_per_sp`. `CvLA0`/`CvLAmax` are allocated with `K_num_reg` but filled by the stock loop.

**Current code** (`atassess/atassessParamIO.c` lines 720–754 (range truncated))

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
**Listed lines:** 741, 799

**Why it's thought to be a bug**

`\n` embedded in the error-path string.

**Current code** (`atassess/atassessParamIO.c` lines 739–778 (truncated))

```c
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
  760:             }
  761:         }
  762:         
  763:         
  764:         // Copy to other stocks
  765:         if ((bm->assess_share_params) && (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)) {
  766:             for (nstock = nloop; nstock < bm->RBCestimation.RBCspeciesParam[guild][NumRegions_id]; nstock++) {
  767:                 for (ns = 0; ns < bm->K_num_sexes; ns++) {
  768:                     for (index = 0; index < length; index++) {
  769:                         array[nstock][ns][index] = array[0][ns][index];
  770:                     }
  771:             	}
  772:             }
  773:         }
  774: 	}
  775: }
  776: 
  777: /**
  778:  * \brief Read in the stock, sex and age specific values
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
**Listed lines:** 891

**Why it's thought to be a bug**

`FunctGroupArray[sp].speciesParams[sp] == TRUE`: species index used as a parameter id. Can also write to an unallocated `TAC_mult`.

**Current code** (`atassess/atassessParamIO.c` lines 891–895)

```c
  891: 		/* Set up the indexing on the names in the forcing file */
  892: 		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  893: 			if ((FunctGroupArray[sp].speciesParams[flag_id] == TRUE) && ((FunctGroupArray[sp].isImpacted == TRUE) || (FunctGroupArray[sp].speciesParams[sp] == TRUE))) {
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
**Listed lines:** 895-896

**Why it's thought to be a bug**

`scale_index = b; if (scale_index != -1)` is always true.

**Current code** (`atassess/atassessParamIO.c` lines 895–900)

```c
  895: 				for (b = 0; b < bm->tsTACmult->nv; b++) {
  896: 					if (strcmp(str, bm->tsTACmult->varname[b]) == 0){
  897: 						scale_index = b;
  898: 						if(scale_index != -1){
  899: 							for(titer = 0; titer < bm->RBCestimation.RBCspeciesParam[sp][Nyears_id]; titer++){
  900: 								tstit = titer * 365.0 * 86400.0;
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
**Listed lines:** 921-922

**Why it's thought to be a bug**

Opens `assessprmIfname` but the error message prints `forceIfname`. Line 916 duplicates line 918.

**Current code** (`atassess/atassessParamIO.c` lines 921–926)

```c
  921: 
  922: 	/* Open the file */
  923: 	if ((fp = fopen(bm->assessprmIfname, "r")) == NULL)
  924: 		quit("get_TAC_mult: Can't open %s\n", bm->forceIfname);
  925: 
  926: 	/** Get time series **/
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
**Listed lines:** 964

**Why it's thought to be a bug**

`Num_Sexes` is read with `binary_check`, so 2 sexes is impossible and 0 is allowed.

**Current code** (`atassess/atassessParamIO.c` lines 963–967)

```c
  963: 		quit("readTierAssessmentManagementXML: Tier_Assessment attribute group not found in input file %s.\n", fileName);
  964:     
  965:     bm->assess_share_params = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "assess_share_params");
  966: 	bm->K_num_sexes = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "Num_Sexes");
  967:     bm->RBCestimation.HistYrMin = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "HistYrMin");
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
**Listed lines:** 1082

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
**Listed lines:** 1086-1099 (alloc 619)

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
**Listed lines:** 438-441, 53 (alloc 601)

**Why it's thought to be a bug**

4 `detrit_import` rows are written but `K_num_detritus*2` are allocated, so it overruns if there is only 1 detritus group.

**Current code** (`atassess/atassessParamIO.c` lines 599–606) — code appears to have moved +163 lines since the review

```c
  599: 	printf("Create assessment detritus flow vectors\n");
  600: 
  601: 	detrit_import = (double **) alloc2d(bm->nfzones, bm->K_num_detritus * 2);
  602: 	DiscardFate = (double **) alloc2d(2, bm->K_num_detritus);
  603: 
  604: 	/* Read the rest of the model run parameters */
  605: 	readErrorStructureXML(bm, filename, inputDoc->children);
  606: 	readBiasedSampleXML(bm, filename,inputDoc->children);
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
**Listed lines:** 1182-1311, 1476, 1483, 1541

**Why it's thought to be a bug**

Arrays sized `K_num_fisheries` are filled with XML `NumFisheries`; `Mzero`/`MeanLenAge`/`Fecund` are sized `OverallMaxAge` but filled to `MaxAge`. The guard in `Tier_Assessment_Setup` runs after this read.

**Current code** (`atassess/atassessParamIO.c` lines 967–1001 (range truncated)) — code appears to have moved -213 lines since the review

```c
  967:     bm->RBCestimation.HistYrMin = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "HistYrMin");
  968: 	bm->RBCestimation.HistYrMax = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "HistYrMax");
  969: 	bm->RBCestimation.OverallMaxAge = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "OverallMaxAge");
  970:     
  971:     bm->RBCestimation.tier6_to_tier4 = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "tier6_to_tier4");
  972:     bm->RBCestimation.delTAE = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "delTAE");
  973:     
  974: 	/* Now allocate the rest of the memory */
  975:     bm->RBCestimation.UseAtlantisPGMSY = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "UseAtlantisPGMSY"));
  976: 	bm->RBCestimation.UseSS = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "UseSS"));
  977:     bm->RBCestimation.AssessDelay = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "AssessDelay"));
  978:     bm->RBCestimation.UseTierBuffers = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "UseTierBuffers"));
  979:     bm->RBCestimation.myTACbuffer = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "myTACbuffer");   // Nov 2026: was cast to int so fractions became 0
  980:     bm->RBCestimation.myTACperiod = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "myTACperiod"));
  981:     bm->RBCestimation.myTACperiod_MCT = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "myTACperiod_MCT"));
  982:     
  983:     bm->RBCestimation.sexratio_cv = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "sexratio_cv");
  984:     
  985:     bm->RBCestimation.ReviewPeriod = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "ReviewPeriod");
  986:     bm->RBCestimation.TriggerCheckPeriod = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "TriggerCheckPeriod");
  987:     bm->RBCestimation.LagPeriod = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "LagPeriod");
  988:     bm->RBCestimation.GradientWindow = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "GradientWindow");
  989:     bm->RBCestimation.GradientPeriod = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "GradientPeriod");
  990:     bm->RBCestimation.GradientBuffer = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "GradientBuffer");
  991:     bm->RBCestimation.UseCategory = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "UseCategory");
  992:     bm->RBCestimation.UseClosest = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "UseClosest");
  993:     bm->RBCestimation.UseTriggerMgmt = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "UseTriggerMgmt");
  994:     bm->RBCestimation.ProjYr = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "ProjYr");
  995:     
  996:     bm->RBCestimation.ThresholdDepletion = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "ThresholdDepletion");
  997:     bm->RBCestimation.ThresholdBound = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "ThresholdBound");
  998:     bm->RBCestimation.MaxIteration = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "MaxIteration");
  999:     
 1000: 	childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Seeds");
 1001: 	if (childGroupingNode == NULL)
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
**Listed lines:** 1158-1163 (and 565)

**Why it's thought to be a bug**

`RAssessRscriptName` is re-allocated and the first allocation leaks.

**Current code** (`atassess/atassessParamIO.c` lines 1163–1172) — code appears to have moved +7 lines since the review

```c
 1163:         
 1164:         bm->NumRAssessScriptNames = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "NumRAssessScriptNames");
 1165:         bm->RAssessRscriptName = c_alloc2d(BMSLEN, bm->NumRAssessScriptNames);
 1166:         for (nf = 0; nf < bm->NumRAssessScriptNames; nf ++) {
 1167:             snprintf(nameString, sizeof(nameString), "RAssessRscriptName%d", nf);
 1168:             Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, 1, groupingNode, nameString, bm->RAssessRscriptName[nf]);
 1169:         }
 1170:         
 1171:         if(Util_XML_Read_Array_Integer(ATLANTIS_ATTRIBUTE, fileName, groupingNode, integer_check, "PGMSYlinks", &PGMSYLinksInputs, bm->K_num_fisheries) == FALSE){
 1172:             quit("Error: Unable to read parameter 'PGMSYlinks' from input file %s\n", fileName);
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
**Listed lines:** 1148

**Why it's thought to be a bug**

`min_TACchange = (int)...` would truncate a proportion to 0.

**Current code** (`atassess/atassessParamIO.c` lines 1148–1152)

```c
 1148: 	get_TAC_mult(bm,"TAC_mult", &bm->tsTACmult, bm->t_units, warn);
 1149: 
 1150: 	bm->RBCestimation.min_TACchange = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "RBC_TACchange_min");   // Nov 2026: was cast to int so fractions became 0
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
**Listed lines:** 1482

**Why it's thought to be a bug**

`MeanLenAge` is read when `!flagLAdirect`, which looks opposite to the SigmaLenAge pattern.

**Current code** (`atassess/atassessParamIO.c` lines 1483–1487) — code appears to have moved +3 lines since the review

```c
 1483:     }
 1484:     
 1485: 	attributeGroup = getParentNode(bm, fileName, groupingNode, "MeanLenAge", errorString, sizeof(errorString));
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
**Listed lines:** 496

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
