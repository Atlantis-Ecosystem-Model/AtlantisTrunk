# atecology/atBiologyXMLParamIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **13 findings** (severity H: 1; confidence H and severity H: 0; on the priority list: 0), plus 5 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [BXML-01](#bxml-01) | 2959-2971 | H/M | wrong node / wrong check type | `bm->shrink_tol = Util_XML_Read_Value(..., attributeGroupNode, integer_check, "shrink_tol")`: (a) the value is… |  | [ ] |
| [BXML-02](#bxml-02) | 3825-3826 | H/M | wrong check type | `dissolv_coefft = Util_XML_Read_Value(..., integer_check, "<contam>_dissolv_coefft")`: this is a continuous co… |  | [ ] |
| [BXML-03](#bxml-03) | 3631-3642 | M/M | index mismatch | `InvaderStartNums` is read into slots `[0 .. n-1]` with `n = maxInvaderAge - minInvaderAge`, but the only cons… |  | [ ] |
| [BXML-04](#bxml-04) | 4229-4242 | M/H | wrong parameter key / NULL dereference | The reader looks up `"KMIG_INVERT%d"` (KMIG_INVERT0, KMIG_INVERT1, ...) via `parameterNameMigNum`, but the con… |  | [ ] |
| [BXML-05](#bxml-05) | 1506, 1596, 1642, 1793, 1896, 2042, 2068, 2131, 2137, 2446, 4147 | M/M | buffer overrun (latent) | These loops run `cohort < numStages` and index `cohortString[cohort]` (a 2-element array, line 95). Several al… |  | [ ] |
| [BXML-06](#bxml-06) | 2033-2036 | M/L | always-false condition | `pSPEat[cohort][prey][SED] = values[b]; if ((pSPEat[cohort][prey][SED] == 0) && values[b] > 0) pSPEat[0][prey]… |  | [ ] |
| [BXML-07](#bxml-07) | 2121 | H/L | format/argument mismatch | `quit("Error: Unable to find parameter '%s/%s/%s' in input file %s\n", errorString, cohortString[preyCohort], … |  | [ ] |
| [BXML-08](#bxml-08) | 977-989, 1007-1019, 1037-1049 | H/L | always-true check | The "incomplete set of entries" checks can never fire. For T: `counter = base = Tchange_max_num*wcnz`, so `dif… |  | [ ] |
| [BXML-09](#bxml-09) | 855, 964, 981, 1041, 1314, 1332, 1349, 1366, 1604, 3652 | H/L | copy-paste error/log text | Error messages name the wrong node. 855: "Chemical_Limitation" (node is PhysicalChemical). 964: "Physical_Chan… |  | [ ] |
| [BXML-10](#bxml-10) | 3637 | H/L | format/argument mismatch | `quit("...'Invading_Species/InvaderStartNums' in input file %s\n", "InvaderStartNums", fileName)`: there is on… |  | [ ] |
| [BXML-11](#bxml-11) | 3398 | H/L | format specifier vs type | `quit("... ageClassSize for %s of %e\n", ..., FunctGroupArray[guild].ageClassSize)`: `ageClassSize` is an `int… |  | [ ] |
| [BXML-12](#bxml-12) | 2807-2810 | M/L | NULL dereference | `attributeGroup = Util_XML_Get_Node(..., "Turbidity")` is not NULL-checked before `Read_Cohort_Species_Param_V… |  | [ ] |
| [BXML-13](#bxml-13) | 3828-3837 | H/L | memory leak | `values` from `Util_XML_Read_Array_Double(... "_contam_interaction" ...)` is never freed, so it leaks once per… |  | [ ] |

---

## BXML-01

**wrong node / wrong check type** · Conf H / Sev M  
**Lines (current code):** 2959-2971  
**Lines at review (Oct 2026):** 2959-2971

**Function:** `Read_Diet_Information_Values()`

**Why it's thought to be a bug**

`bm->shrink_tol = Util_XML_Read_Value(..., attributeGroupNode, integer_check, "shrink_tol")`: (a) the value is looked up under `attributeGroupNode` (the "Diet" group), but the converter writes it inside the "Shrinking related" sub-group (ConvertAtlantis/atBioltoXML.c:3025-3030). `Util_XML_Get_Node` only searches direct children (`parent/Attribute[@name=..]`, atUtilXML.c:1004-1011), so with `isRequired=1` every run with `size_shrinking_type > 0` stops with "Cannot find atlantis parameter 'shrink_tol'". (b) `shrink_tol` is a double proportion ("Tolerated proportion of a month", atlantisboxmodel.h:5302, XML_TYPE_FLOAT), so `integer_check` rejects any fractional value. The species params on the next lines correctly use `attributeGroup`. Function: `Read_Diet_Information_Values()`.

**Current code** (`atecology/atBiologyXMLParamIO.c` lines 2957–2973)

```c
 2957:         Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, SNcost_id);
 2958:         Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, RNcost_id);
 2959:         Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, RSstarve_id);
 2960:     }
 2961:     
 2962:     /* Starvation and shrinking associated with feeding */
 2963:     attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Shrinking related");
 2964:     if (attributeGroup == NULL) {
 2965:         quit("Shrinking/Starvation related shrinking information input file %s.\n", fileName);
 2966:     }
 2967:     
 2968:     bm->size_shrinking_type = (int)(Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, integer_check, "size_shrinking_type"));
 2969:     if (bm->size_shrinking_type > 0){
 2970:         bm->shrink_tol = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "shrink_tol");
 2971:         Util_XML_Read_Species_Param(bm, fileName, attributeGroup, shrinkStarve_id);
 2972:         Util_XML_Read_Species_Param(bm, fileName, attributeGroup, shrinkProp_id);
 2973:     }
```

**Suggested fix (from review):** Use `attributeGroup` and `no_checking` (or `proportion_check`).

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

## BXML-02

**wrong check type** · Conf H / Sev M  
**Lines (current code):** 3825-3826  
**Lines at review (Oct 2026):** 3825-3826

**Function:** `Read_Contaminant_Values()`

**Why it's thought to be a bug**

`dissolv_coefft = Util_XML_Read_Value(..., integer_check, "<contam>_dissolv_coefft")`: this is a continuous coefficient (converter declares XML_TYPE_FLOAT, atBioltoXML.c:3731-3733; used as a multiplier in atContaminants.c:715). `Check_Value` with `integer_check` quits on any non-integer (atUtilIO.c:181-186), so realistic values such as 0.1 are rejected. Function: `Read_Contaminant_Values()`.

**Current code** (`atecology/atBiologyXMLParamIO.c` lines 3823–3828)

```c
 3823: 
 3824: 		snprintf(varStr, sizeof(varStr), "%s_half_life", bm->contaminantStructure[cIndex]->contaminant_name);
 3825: 		bm->contaminantStructure[cIndex]->half_life = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);
 3826: 
 3827:         snprintf(varStr, sizeof(varStr), "%s_temp_depend", bm->contaminantStructure[cIndex]->contaminant_name);
 3828:         bm->contaminantStructure[cIndex]->contam_temp_depend = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, varStr);
```

**Suggested fix (from review):** Use `no_checking` (or `proportion_check`).

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

## BXML-03

**index mismatch** · Conf M / Sev M  
**Lines (current code):** 3631-3642  
**Lines at review (Oct 2026):** 3631-3642

**Function:** `Read_Invading_Species_Values()`

**Why it's thought to be a bug**

`InvaderStartNums` is read into slots `[0 .. n-1]` with `n = maxInvaderAge - minInvaderAge`, but the only consumer reads `bm->InvaderStartNums[n]` for `n = minInvaderAge .. maxInvaderAge-1` (atmovement.c:3738-3741). When `minInvaderAge > 0` the wrong entries are used, and the top `minInvaderAge` cohorts get 0 (the array is zero-initialised with size K_num_max_cohort, atlantismain.c:2009). Function: `Read_Invading_Species_Values()`.

**Current code** (`atecology/atBiologyXMLParamIO.c` lines 3629–3644)

```c
 3629: 	bm->InvaderSpeed = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "InvaderSpeed") / numsechr;
 3630:     
 3631:     if (bm->InvadersEntering ) {
 3632:         bm->InvaderEntryLayer = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "InvaderEntryLayer");
 3633:         
 3634:         n = bm->maxInvaderAge - bm->minInvaderAge;
 3635:         if (n <= 0)
 3636:             n = 1;
 3637:         
 3638:         /* Load number of invaders to add to the system - in first invasion pulse */
 3639:         if (Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "InvaderStartNums", attributeGroupNode, integer_check, "InvaderStartNums", &values, n) == FALSE) {
 3640:             quit("Error: Unable to find parameter 'Invading_Species/InvaderStartNums' in input file %s\n", "InvaderStartNums", fileName);
 3641:         }
 3642:     
 3643:         for (cohort = 0; cohort < n; cohort++) {
 3644:             bm->InvaderStartNums[cohort] = values[cohort];
```

**Suggested fix (from review):** Store `bm->InvaderStartNums[minInvaderAge + cohort] = values[cohort]` (or change the consumer to index `[n - minInvaderAge]`).

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

## BXML-04

**wrong parameter key / NULL dereference** · Conf M / Sev H  
**Lines (current code):** 4229-4242  
**Lines at review (Oct 2026):** 4229-4242

**Function:** `Read_KMIG_INVERT_XML()`

**Why it's thought to be a bug**

The reader looks up `"KMIG_INVERT%d"` (KMIG_INVERT0, KMIG_INVERT1, ...) via `parameterNameMigNum`, but the converter only creates one attribute called `"KMIG_INVERT"` (atBioltoXML.c:2807-2809, InitKMIGInvertXML at 1425-1444). The returned `attributeNode` is never NULL-checked, so it is passed as `parent` to `Util_XML_Read_Array_Double` -> `Util_XML_Get_Node`, which dereferences `parent->doc` (atUtilXML.c:1093). Any invertebrate with `sp_geo_move` and `num_migrate >= 1` would segfault. The existing comment ("This wasn't originally set - was that intentional?") suggests the change was unverified. Function: `Read_KMIG_INVERT_XML()`.

**Current code** (`atecology/atBiologyXMLParamIO.c` lines 4227–4244)

```c
 4227:     
 4228:     snprintf(errorString, sizeof(errorString), "Migration/%s", parameterName);
 4229:     attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, parameterName);
 4230:     if (attributeNode == NULL)
 4231:         quit("%s attribute group not found.\n", errorString);
 4232:     
 4233:     /* Create a node for each functional group*/
 4234:     for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
 4235:         for (qid = 0; qid < FunctGroupArray[guild].num_migrate; qid++) {
 4236:             snprintf(errorString, sizeof(errorString), "%s%d", parameterName,qid);  // This used to be "Migration/%s%d" but parent is "Migration" so don't need it again as otherwise doubled up in attributeNode code
 4237:             snprintf(parameterNameMigNum, sizeof(parameterNameMigNum), "%s%d", parameterName, qid); // This wasn't originally set - was that intentional?
 4238:             attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, parameterNameMigNum);
 4239:             
 4240:             // Load those migrations that do happen
 4241:             if (FunctGroupArray[guild].isVertebrate == FALSE && FunctGroupArray[guild].sp_geo_move == TRUE) {
 4242:                 
 4243:                 printf("Reading %s values for %s with %d genotypes and %d cohorts - %s\n", parameterName, FunctGroupArray[guild].groupCode, FunctGroupArray[guild].numGeneTypes, FunctGroupArray[guild].numCohortsXnumGenes, errorString);
 4244:                 
```

**Suggested fix (from review):** Either look up `parameterName` (matching the converter) or make the converter write per-queue nodes; also add `if (attributeNode == NULL) quit(...)`.

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

## BXML-05

**buffer overrun (latent)** · Conf M / Sev M  
**Lines (current code):** 1506, 1596, 1642, 1793, 1896, 2042, 2068, 2131, 2137, 2446, 4147  
**Lines at review (Oct 2026):** 1486, 1576, 1622, 1773, 1876, 2022, 2048, 2111, 2117, 2425, 4103

**Why it's thought to be a bug**

These loops run `cohort < numStages` and index `cohortString[cohort]` (a 2-element array, line 95). Several also index arrays sized 2 in the stage dimension: `HABITATlike`/`ICE_HABITATlike*` (atlantismain.c:2073-2076, `[sp][2][..]`), `distrib_VERTICAL`/`distrib_OVERWINTER` (atUtilGroupIO.c:775-778), and `distrib` (atUtilGroupIO.c:917). `numStages` is only checked to be >= 1 (atUtilGroupIO.c:294), with no upper bound of 2, so a group with 3+ stages reads past `cohortString` (passing a garbage `char*` as the XML key) and writes out of bounds. Functions: `Read_Habitat_XML()`, `Read_Ice_Habitat_XML()`, `Read_VERT_Function()`, `Read_Sesonal_Dist_XML()`, `Read_Detritus_Sediment_Food_Avail()`, `Read_Vertebrate_Food_Avail()`, `Read_PStock_XML()`, `Read_MigrateIO_Box_XML()`.

**Current code** (`atecology/atBiologyXMLParamIO.c` lines 1504–1508)

```c
 1504: 					sum_hab -= bm->HABITATlike[guild][cohort][bm->CANYONcover_id];
 1505: 					if(cohort)
 1506: 						FunctGroupArray[guild].speciesParams[K_num_biogen_habad_id] = sum_hab;
 1507: 					else
 1508: 						FunctGroupArray[guild].speciesParams[K_num_biogen_habjuv_id] = sum_hab;
```

**Suggested fix (from review):** Add a check at group load that `numStages <= 2`, or size these arrays and strings by `K_num_max_stages`.

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

## BXML-06

**always-false condition** · Conf M / Sev L  
**Lines (current code):** 2033-2036  
**Lines at review (Oct 2026):** 2033-2036

**Function:** `Read_Detritus_Sediment_Food_Avail()`

**Why it's thought to be a bug**

`pSPEat[cohort][prey][SED] = values[b]; if ((pSPEat[cohort][prey][SED] == 0) && values[b] > 0) pSPEat[0][prey][SED] = values[b];`: the test runs on the value just assigned, so it can never be true and the fallback copy to stage 0 is dead code. The intended test was probably on `pSPEat[0][prey][SED]` (fill juveniles from adults when juveniles are 0). Function: `Read_Detritus_Sediment_Food_Avail()`.

**Current code** (`atecology/atBiologyXMLParamIO.c` lines 2031–2038)

```c
 2031: 						for (b = 0; b < bm->K_num_detritus; b++) {
 2032: 							/* Work out which detritus group we are looking at*/
 2033: 							prey = Util_Get_Detritus_Index(bm, b);
 2034: 
 2035: 							FunctGroupArray[guild].pSPEat[cohort][prey][SED] = values[b];
 2036:                             if ((FunctGroupArray[guild].pSPEat[cohort][prey][SED] == 0) && values[b] > 0) {
 2037:                                 FunctGroupArray[guild].pSPEat[0][prey][SED] = values[b];
 2038:                             }
```

**Suggested fix (from review):** Test `pSPEat[0][prey][SED] == 0`, or delete the block if it isn't wanted.

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

## BXML-07

**format/argument mismatch** · Conf H / Sev L  
**Lines (current code):** 2121  
**Lines at review (Oct 2026):** 2121

**Function:** `Read_Vertebrate_Food_Avail()`

**Why it's thought to be a bug**

`quit("Error: Unable to find parameter '%s/%s/%s' in input file %s\n", errorString, cohortString[preyCohort], fileName)`: 4 `%s` but only 3 arguments, which is undefined behaviour (likely a crash) on this error path. Function: `Read_Vertebrate_Food_Avail()`.

**Current code** (`atecology/atBiologyXMLParamIO.c` lines 2119–2123)

```c
 2119: 				for (preyCohort = 0; preyCohort < FunctGroupArray[guild].numStages; preyCohort++) {
 2120: 
 2121: 					if (Util_XML_Read_Array_Double(ATLANTIS_AGE_CLASS_ATTRIBUTE, fileName, errorString, cohortNode, proportion_check, cohortString[preyCohort], &values,
 2122: 							bm->K_num_tot_sp) == FALSE) {
 2123: 						quit("Error: Unable to find parameter '%s/%s/%s' in input file %s\n", errorString, cohortString[preyCohort], fileName);
```

**Suggested fix (from review):** Use `'%s/%s' in input file %s`.

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

## BXML-08

**always-true check** · Conf H / Sev L  
**Lines (current code):** 977-989, 1007-1019, 1037-1049  
**Lines at review (Oct 2026):** 977-989, 1007-1019, 1037-1049

**Function:** `Read_Physical_Chem_Values()`

**Why it's thought to be a bug**

The "incomplete set of entries" checks can never fire. For T: `counter = base = Tchange_max_num*wcnz`, so `diff` is always 0. For S and pH: `nset = counter / bm->wcnz` is integer division of `X*wcnz` by `wcnz`, so `floor(nset) == nset`. `Parse_Into_Array` already enforces the count, so these checks are dead code. Function: `Read_Physical_Chem_Values()`.

**Current code** (`atecology/atBiologyXMLParamIO.c` lines 975–1014 (truncated))

```c
  975:         
  976:         printf("Created Tchange\n");
  977: 
  978: 		//TODO: Check this - we might need to quit if there are not the right number of entries. It might be safer.
  979: 		counter = Tchange_max_num * bm->wcnz;
  980: 		if (Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "PhysicalChemical/Temperature_Effects", attributeGroup, no_checking, "vertTchange_mult", &vertTchange_multi, Tchange_max_num
  981: 				* bm->wcnz) == FALSE) {
  982: 
  983: 			quit("Error: Unable to find parameter 'PhysicalChemical/Salinity_Effects/vertTchange_mult' in input file %s\n", fileName);
  984: 		}
  985: 		base = Tchange_max_num * bm->wcnz;
  986: 		nset = counter / 1.0;
  987: 		diff = fabs(base - nset);
  988: 		if (diff > 0) {
  989: 			quit("Incomplete set of entries for vertTchange_mult (says there are %e entries instead of %e = %d * %d (Vchange_num * wcnz)\n", nset, base,
  990: 					Tchange_max_num, bm->wcnz);
  991: 		}
  992: 	}else{
  993: 		Tchange_max_num = 0;
  994: 	}
  995: 	Vchange_max_num = Tchange_max_num;
  996: 
  997: 	/* Salinity effects */
  998: 	if(bm->flagsaltchange){
  999: 		attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Salinity_Effects");
 1000: 		if (attributeGroup == NULL)
 1001: 			quit("Salinity_Effects attribute group not found in input file %s.\n", fileName);
 1002: 
 1003: 		Schange_max_num = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "Schange_max_num");
 1004: 
 1005: 		if(Schange_max_num > 0)
 1006: 			Schange = (double **) alloc2d(3, Schange_max_num);
 1007: 		Read_Change_DataXML(bm, fileName, attributeGroup, "Schange", Schange, Schange_max_num, no_checking);
 1008: 
 1009: 		counter = Schange_max_num * bm->wcnz;
 1010: 
 1011: 		if (Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "Salinity_Effects", attributeGroup, no_checking, "vertSchange_mult",
 1012: 				&vertSchange_multi, Schange_max_num * bm->wcnz) == FALSE) {
 1013: 			quit("Error: Unable to find parameter 'PhysicalChemical/Salinity_Effects/vertSchange_mult' in input file %s\n", fileName);
 1014: 		}
```

**Suggested fix (from review):** Remove them, or check the real number of tokens parsed.

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

## BXML-09

**copy-paste error/log text** · Conf H / Sev L  
**Lines (current code):** 855, 964, 981, 1041, 1314, 1332, 1349, 1366, 1604, 3652  
**Lines at review (Oct 2026):** 855, 964, 981, 1041, 1314, 1332, 1349, 1366, 1604, 3652

**Why it's thought to be a bug**

Error messages name the wrong node. 855: "Chemical_Limitation" (node is PhysicalChemical). 964: "Physical_Change" (node is Temperature_Effects). 981 and 1041: "Salinity_Effects/vertTchange_mult" and ".../vertPHchange_mult" (should be Temperature_Effects and pH_Effects). 1314, 1332, 1349 and 1366: all say "Bleaching" (should be Calcification, ReefGrowth, ReefRecruit and RugosityModel). 1604: "Group_Ice_Preference" (should be Group_IceReprod_Preference). 3652: prints "Reading the Reproduction values" in `Read_Ice_Values()`. Functions: as listed.

**Current code** (`atecology/atBiologyXMLParamIO.c` lines 853–857) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
  853: 	/** Biophysical constants ************/
  854: 	xmlNodePtr attributeGroupNode, attributeGroup;
  855: 	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "PhysicalChemical");
  856: 	if (attributeGroupNode == NULL)
  857: 		quit("Chemical_Limitation attribute group not found in input file %s.\n", fileName);
```

**Suggested fix (from review):** Correct the strings.

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

## BXML-10

**format/argument mismatch** · Conf H / Sev L  
**Lines (current code):** 3637  
**Lines at review (Oct 2026):** 3637

**Function:** `Read_Invading_Species_Values()`

**Why it's thought to be a bug**

`quit("...'Invading_Species/InvaderStartNums' in input file %s\n", "InvaderStartNums", fileName)`: there is one `%s` but the first argument is the literal "InvaderStartNums", so the message prints that instead of the file name. Function: `Read_Invading_Species_Values()`.

**Current code** (`atecology/atBiologyXMLParamIO.c` lines 3635–3639)

```c
 3635:         if (n <= 0)
 3636:             n = 1;
 3637:         
 3638:         /* Load number of invaders to add to the system - in first invasion pulse */
 3639:         if (Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "InvaderStartNums", attributeGroupNode, integer_check, "InvaderStartNums", &values, n) == FALSE) {
```

**Suggested fix (from review):** Drop the extra `"InvaderStartNums"` argument.

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

## BXML-11

**format specifier vs type** · Conf H / Sev L  
**Lines (current code):** 3398  
**Lines at review (Oct 2026):** 3398

**Function:** `Read_Popratio_Stock_XML()`

**Why it's thought to be a bug**

`quit("... ageClassSize for %s of %e\n", ..., FunctGroupArray[guild].ageClassSize)`: `ageClassSize` is an `int` printed with `%e` (UB). It is unreachable in practice because atUtilGroupIO.c:288 already quits when it is < 1. Function: `Read_Popratio_Stock_XML()`.

**Current code** (`atecology/atBiologyXMLParamIO.c` lines 3396–3400)

```c
 3396: 
 3397: 				for (i = 0; i < FunctGroupArray[guild].numStocks; i++) {
 3398: 					counter = FunctGroupArray[guild].ageClassSize;
 3399:                     if (FunctGroupArray[guild].ageClassSize < 1) {
 3400:                         quit("Currently the popratioStock code will not work for ageclasses < 1 - you have ageClassSize for %s of %e\n", FunctGroupArray[guild].groupCode, FunctGroupArray[guild].ageClassSize);
```

**Suggested fix (from review):** Use `%d`.

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

## BXML-12

**NULL dereference** · Conf M / Sev L  
**Lines (current code):** 2807-2810  
**Lines at review (Oct 2026):** 2807-2810

**Function:** `Read_Diet_Information_Values()`

**Why it's thought to be a bug**

`attributeGroup = Util_XML_Get_Node(..., "Turbidity")` is not NULL-checked before `Read_Cohort_Species_Param_Values(bm, fileName, attributeGroup, ...)`, which calls `Util_Get_Node_Name(parent)` and `Util_XML_Get_Node(..., parent, ...)` on it. If the Turbidity sub-group is missing from an estuary model, this crashes instead of giving a clear message. Every other sub-group lookup in this file is checked. Function: `Read_Diet_Information_Values()`.

**Current code** (`atecology/atBiologyXMLParamIO.c` lines 2805–2812)

```c
 2805:     if(bm->flagIsEstuary) {
 2806:        
 2807:         Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, turbid_refuge_id);
 2808:         
 2809:         attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Turbidity");        
 2810:         Read_Cohort_Species_Param_Values(bm, fileName, attributeGroup, L_turbid_id);
 2811:         Read_Cohort_Species_Param_Values(bm, fileName, attributeGroup, a_turbid_id);
 2812:         Read_Cohort_Species_Param_Values(bm, fileName, attributeGroup, b_turbid_id);
```

**Suggested fix (from review):** Add `if (attributeGroup == NULL) quit("Diet/Turbidity ...")`.

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

## BXML-13

**memory leak** · Conf H / Sev L  
**Lines (current code):** 3828-3837  
**Lines at review (Oct 2026):** 3828-3837

**Function:** `Read_Contaminant_Values()`

**Why it's thought to be a bug**

`values` from `Util_XML_Read_Array_Double(... "_contam_interaction" ...)` is never freed, so it leaks once per contaminant. Function: `Read_Contaminant_Values()`.

**Current code** (`atecology/atBiologyXMLParamIO.c` lines 3826–3839)

```c
 3826: 
 3827:         snprintf(varStr, sizeof(varStr), "%s_temp_depend", bm->contaminantStructure[cIndex]->contaminant_name);
 3828:         bm->contaminantStructure[cIndex]->contam_temp_depend = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, varStr);
 3829: 
 3830:         snprintf(varStr, sizeof(varStr), "%s_dissolv_coefft", bm->contaminantStructure[cIndex]->contaminant_name);
 3831:         bm->contaminantStructure[cIndex]->dissolv_coefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, varStr);
 3832:         
 3833:         if (bm->num_contaminants > 1) {
 3834:             snprintf(varStr, sizeof(varStr), "%s_contam_interaction", bm->contaminantStructure[cIndex]->contaminant_name);
 3835:             if (Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "Contaminants", attributeGroupNode, no_checking, varStr, &values, bm->num_contaminants) == FALSE){
 3836:                 quit("Error: Unable to read parameter %s from input file %s\n", varStr, fileName);
 3837:             }
 3838:             
 3839:             for(contamIndex = 0; contamIndex < bm->num_contaminants; contamIndex++){
```

**Suggested fix (from review):** `free(values);` after the copy loop.

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

`Read_Primary_Producer_Values()` (1220-1253): when `ice_on`, `attributeGroupNode` is reassigned to "IcePrimaryProduction", so the macro-model, coral, sponge and HAB species parameters that follow are read from the ice sub-group. `HAB_box` is read from `parent`, and the "HAB_Location" node that was just fetched is ignored. This matches what the converter writes (atBioltoXML.c:2618-2650), so it works today, but the structure looks accidental (the converter does the same reassignment). If either side is "fixed" alone, both will break.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

`Read_Vertebrate_Food_Avail()` (2127): under `flag_olddiet` the guard tests `pSPEat[0][b][WC]` but writes `pSPEat[cohort]...`. Once stage 0 is set, stage 1 is never filled. It may be deliberate, given the old-diet remapping comment in atmovement.c:286. Also, in the non-old branch `pSPEat[cohort][b]` is overwritten on every `preyCohort` pass, so only the adult-prey value survives.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-3

`Read_Reproduction_Values()` (3565-3569): `AquacultDistribution` is written into the same `recruit_hdistrib[0]` array as `RecruitDistribution`, so for cultured age-structured groups it overwrites the recruit distribution. This may be intended.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-4

`Read_Invert_Growth_Rates_XML()` and `Read_Invert_Clearance_Rates_XML()` set `mum_T15_per_day` and `C_T15_per_day`, while the vertebrate readers set `mum_per_day` and `SP_C_per_day`. A vertebrate with `numCohorts == 1` would go through the invertebrate growth reader and leave `mum_per_day` at 0, which is used as a divisor in atmovement.c:398. This is an edge case.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-5

`Read_Ice_Habitat_XML()` (1529-1538): the `!bm->ice_on` initialisation branch would dereference unallocated `ICE_HABITATlike` (only allocated when `ice_on`, atlantismain.c:2074). It is unreachable because the only caller (1699) is guarded by `ice_on`.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
