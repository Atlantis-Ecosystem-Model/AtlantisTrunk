# atlantisUtil/atUtilGroupIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **8 findings** (severity H: 2; confidence H and severity H: 2; on the priority list: 2).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [UTIL-17](#util-17) ⭐ | 909, 1348 | H/H | cohort vs cohort x genotype | `contaminantTracers` and `contamPropTracers` are `Util_Alloc_Init_2D_Int(num_contaminants, numCohorts)`, which… |  | [ ] |
| [UTIL-18](#util-18) ⭐ | 1348 | H/H | wrong dimension (habitat) and cohort vs cohort x genotype | The atomic-ratio arrays hard-code 3 habitats: `ratioLost = 3D(num_atomic_id, nCxG, 3)` gives `[3][nCxG][..]`, … |  | [ ] |
| [UTIL-19](#util-19) | 1428, 1433, 1501 | H/M | index beyond logical array size | `habitatCoeffs` is allocated with `bm->num_active_habitats` entries (3 = EPIFAUNA+1 without ice, 5 = ICE_BASED… |  | [ ] |
| [UTIL-20](#util-20) | 394 | H/M | contradictory check / uninitialised member | `isAssessed` is read with `binary_check` (line 394), so a value >1 quits and `if(isAssessed > 1) isCloseKin = … |  | [ ] |
| [UTIL-21](#util-21) | 421 | H/M | uninitialised member | `isCollisionRisk` is only set when `bm->flagindustry_on`. Otherwise it's uninitialised malloc memory, read at … |  | [ ] |
| [UTIL-22](#util-22) | 534 | H/M | unreachable condition | `if(groupType == TURF) bm->containsTurf = TRUE;` sits inside the `isVertebrate == TRUE` branch, but TURF is ne… |  | [ ] |
| [UTIL-23](#util-23) | 494 | M/L | NULL dereference | If `groupList` is NULL or empty, `FunctGroupArray` is never allocated and the count check is skipped. `Set_Gro… |  | [ ] |
| [UTIL-24](#util-24) | 1084 | H/L | wrong loop variable | `TLorder2[eatIndex++] = i;` should be `guild`, since `i` is stale. This is inside `#ifdef ORIGINAL_GROUP_ORDER… |  | [ ] |

---

## UTIL-17

**cohort vs cohort x genotype** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atUtilGroupIO.c` 909, `atUtilGroupIO.c` 1348
**Lines at review (Oct 2026):** 910-912

**Function:** `Util_Read_Functional_Group_XML()`

**Why it's thought to be a bug**

`contaminantTracers` and `contamPropTracers` are `Util_Alloc_Init_2D_Int(num_contaminants, numCohorts)`, which gives `[numCohorts][ncontam]`. `contaminantSpMort` is 1D with `numCohorts` entries. They are indexed with `cohort < numCohortsXnumGenes` in atecology/atContaminants.c:598-613 (`contaminantSpMort[cohort]`, `contaminantTracers[cohort][cIndex]`) and atmanage/atManage.c:2084-2085. That's an out-of-bounds read/write whenever `numGeneTypes > 1` with contaminants on. Function: `Util_Read_Functional_Group_XML()`.

**Code at issue** — `atUtilGroupIO.c` line 909 (closest match to the quoted code; wording differs slightly)  
Search for: `FunctGroupArray[i].contaminantTracers = Util_Alloc_Init_2D_Int(bm->num_contaminants, Funct`

```c
  907: 
  908: 		if(bm->track_contaminants){
  909→ 			FunctGroupArray[i].contaminantTracers = Util_Alloc_Init_2D_Int(bm->num_contaminants, FunctGroupArray[i].numCohorts, 0);
  910:             FunctGroupArray[i].contamPropTracers = Util_Alloc_Init_2D_Int(bm->num_contaminants, FunctGroupArray[i].numCohorts, 0);
  911: 			FunctGroupArray[i].contaminantSpMort = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohorts, 0.0);
```

**Code at issue** — `atUtilGroupIO.c` line 1348 (closest match to the quoted code; wording differs slightly)  
Search for: `for (cohort = 0; cohort < FunctGroupArray[i].numCohortsXnumGenes; cohort++)`

```c
 1346: 	for (i = 0; i < bm->K_num_tot_sp; i++) {
 1347: 		if (FunctGroupArray[i].isVertebrate == TRUE) {
 1348→ 			for (cohort = 0; cohort < FunctGroupArray[i].numCohortsXnumGenes; cohort++)
 1349: 				fprintf(fp, "FunctGroupArray[%d].totNTracers = %d\n", i, FunctGroupArray[i].totNTracers[cohort]);
 1350: 
```

**Suggested fix (from review):** Allocate with `numCohortsXnumGenes` (or bound the loops by `numCohorts`, depending on design).

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

## UTIL-18

**wrong dimension (habitat) and cohort vs cohort x genotype** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atUtilGroupIO.c` 1348
**Lines at review (Oct 2026):** 894-905

**Function:** `Util_Read_Functional_Group_XML()`

**Why it's thought to be a bug**

The atomic-ratio arrays hard-code 3 habitats: `ratioLost = 3D(num_atomic_id, nCxG, 3)` gives `[3][nCxG][..]`, and `ratioLostGlobal = 4D(..., 3, 3)` gives `[3][3][nCxG][..]`. atecology/atecology.c:3904-3914 and 4036-4052 loop `hab < bm->num_active_habitats`, which is 5 when ice is on (attracerIO.c:239), so they write past the first dimension. `ratioLostPred`, `ratioLostPredGlobal`, `ratioGained` and `ratioGainedGlobal` are also sized `numCohorts` but looped with `cohort < numCohortsXnumGenes` in the same code, so they overflow when `numGeneTypes > 1`. Function: `Util_Read_Functional_Group_XML()`.

**Code at issue** — `atUtilGroupIO.c` line 1348 (closest match to the quoted code; wording differs slightly)  
Search for: `for (cohort = 0; cohort < FunctGroupArray[i].numCohortsXnumGenes; cohort++)`

```c
 1346: 	for (i = 0; i < bm->K_num_tot_sp; i++) {
 1347: 		if (FunctGroupArray[i].isVertebrate == TRUE) {
 1348→ 			for (cohort = 0; cohort < FunctGroupArray[i].numCohortsXnumGenes; cohort++)
 1349: 				fprintf(fp, "FunctGroupArray[%d].totNTracers = %d\n", i, FunctGroupArray[i].totNTracers[cohort]);
 1350: 
```

**Suggested fix (from review):** Use `bm->num_active_habitats` in place of 3 and `numCohortsXnumGenes` in place of `numCohorts` (fix the matching Init/free calls too).

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

## UTIL-19

**index beyond logical array size** · Conf H / Sev M  
**Found at (current code):** `atUtilGroupIO.c` 1428, `atUtilGroupIO.c` 1433, `atUtilGroupIO.c` 1501
**Lines at review (Oct 2026):** 1429-1435, 1440, 1502

**Function:** `Set_Group_Habitats()`

**Why it's thought to be a bug**

`habitatCoeffs` is allocated with `bm->num_active_habitats` entries (3 = EPIFAUNA+1 without ice, 5 = ICE_BASED+1 with ice). The code then writes `[ICE_BASED]`(4), `[LAND_BASED]`(3) and `[MIXED]`(5). There's no heap corruption only because `Util_Alloc_Init_1D_Int` allocates double-sized storage. But the land flag (with terrestrial on and ice off) and the MIXED flag for ice groups fall outside every `hab < num_active_habitats` loop, so they're silently ignored. Function: `Set_Group_Habitats()`.

**Code at issue** — `atUtilGroupIO.c` line 1428  
Search for: `num_active_habitats`

```c
 1426: 	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
 1427: 
 1428→ 		FunctGroupArray[guild].habitatCoeffs = Util_Alloc_Init_1D_Int(bm->num_active_habitats, 0);
 1429: 
 1430: 		FunctGroupArray[guild].habitatCoeffs[WC] = 0;
```

**Code at issue** — `atUtilGroupIO.c` line 1433  
Search for: `[ICE_BASED]`

```c
 1431: 		FunctGroupArray[guild].habitatCoeffs[SED] = 0;
 1432: 		FunctGroupArray[guild].habitatCoeffs[EPIFAUNA] = 0;
 1433→ 		FunctGroupArray[guild].habitatCoeffs[ICE_BASED] = 0;
 1434: 		FunctGroupArray[guild].habitatCoeffs[LAND_BASED] = 0;
 1435: 
```

**Code at issue** — `atUtilGroupIO.c` line 1501  
Search for: `[MIXED]`

```c
 1499: 		case ICE_ZOOBIOTA: 	//(small zooplankton)
 1500: 			FunctGroupArray[guild].habitatType = ICE_BASED;
 1501→             FunctGroupArray[guild].habitatCoeffs[MIXED] = 1;
 1502: 			FunctGroupArray[guild].habitatCoeffs[ICE_BASED] = 1;
 1503: 			break;
```

**Suggested fix (from review):** Allocate `nTotHabTypes` entries, or make `num_active_habitats` cover LAND_BASED/MIXED when those features are on.

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

## UTIL-20

**contradictory check / uninitialised member** · Conf H / Sev M  
**Found at (current code):** `atUtilGroupIO.c` 394
**Lines at review (Oct 2026):** 397-399

**Function:** `Util_Read_Functional_Group_XML()`

**Why it's thought to be a bug**

`isAssessed` is read with `binary_check` (line 394), so a value >1 quits and `if(isAssessed > 1) isCloseKin = 1` can never fire. `FunctGroupArray` comes from `malloc` (line 252) and `isCloseKin` is never set otherwise, so it holds garbage, and atSS3Link/atSS3DataGen.c:85 tests it. Function: `Util_Read_Functional_Group_XML()`.

**Code at issue** — `atUtilGroupIO.c` line 394 (located by name only; check this is the right place)  
Search for: `Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, b`

```c
  392: 			Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, binary_check, "IsSiliconDep", &FunctGroupArray[groupIndex].isSiliconDependant);
  393: 			FunctGroupArray[groupIndex].secondNutrientTracerIndex = -1;
  394→ 			Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, binary_check, "IsAssessed", &FunctGroupArray[groupIndex].isAssessed);
  395: 			Util_XML_Get_Value_Double(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, binary_check, "IsCatchGrazer", &FunctGroupArray[groupIndex].speciesParams[catcheater_id]);
  396:             
```

**Suggested fix (from review):** Initialise `isCloseKin = 0`, and read `IsAssessed` with `integer_check` if values >1 are meant to be allowed.

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

## UTIL-21

**uninitialised member** · Conf H / Sev M  
**Found at (current code):** `atUtilGroupIO.c` 421
**Lines at review (Oct 2026):** 421-423

**Function:** `Util_Read_Functional_Group_XML()`

**Why it's thought to be a bug**

`isCollisionRisk` is only set when `bm->flagindustry_on`. Otherwise it's uninitialised malloc memory, read at atecology/atbiolmessage.c:2273/2303. Function: `Util_Read_Functional_Group_XML()`.

**Code at issue** — `atUtilGroupIO.c` line 421  
Search for: `flagindustry_on`

```c
  419:             }
  420:             
  421→             if(bm->flagindustry_on) {
  422:                 Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "isCollisionRisk", &FunctGroupArray[groupIndex].isCollisionRisk);
  423:             }
```

**Suggested fix (from review):** Add `else FunctGroupArray[groupIndex].isCollisionRisk = 0;`.

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

## UTIL-22

**unreachable condition** · Conf H / Sev M  
**Found at (current code):** `atUtilGroupIO.c` 534
**Lines at review (Oct 2026):** 545-547

**Function:** `Util_Read_Functional_Group_XML()`

**Why it's thought to be a bug**

`if(groupType == TURF) bm->containsTurf = TRUE;` sits inside the `isVertebrate == TRUE` branch, but TURF is never a vertebrate, so `containsTurf` is always FALSE. atecology/atcoral.c:908/932 then forces `facilitate_space = 1.0` and skips the turf facilitation logic. Function: `Util_Read_Functional_Group_XML()`.

**Code at issue** — `atUtilGroupIO.c` line 534  
Search for: `isVertebrate == TRUE`

```c
  532: 	bm->flagWCVert = 1;
  533: 	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
  534→ 		if (FunctGroupArray[guild].isVertebrate == TRUE){
  535: 			FunctGroupArray[guild].groupAgeType = AGE_STRUCTURED;
  536: 
```

**Suggested fix (from review):** Move the TURF test into the invertebrate (`else`) branch or a separate loop.

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

## UTIL-23

**NULL dereference** · Conf M / Sev L  
**Found at (current code):** `atUtilGroupIO.c` 494
**Lines at review (Oct 2026):** 244-252, 489-533

**Function:** `Util_Read_Functional_Group_XML()`

**Why it's thought to be a bug**

If `groupList` is NULL or empty, `FunctGroupArray` is never allocated and the count check is skipped. `Set_Group_Habitats` and the later loops then dereference NULL `FunctGroupArray`. Function: `Util_Read_Functional_Group_XML()`.

**Code at issue** — `atUtilGroupIO.c` line 494 (located by name only; check this is the right place)  
Search for: `Set_Group_Habitats(bm);`

```c
  492: 
  493: 	/* Set the habitats of the groups based on the group type */
  494→ 	Set_Group_Habitats(bm);
  495: 
  496: 
```

**Suggested fix (from review):** Quit with a clear message when no FunctionalGroup nodes are found.

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

## UTIL-24

**wrong loop variable** · Conf H / Sev L  
**Found at (current code):** `atUtilGroupIO.c` 1084
**Lines at review (Oct 2026):** 1083-1085

**Function:** `Util_Read_Functional_Group_XML()`

**Why it's thought to be a bug**

`TLorder2[eatIndex++] = i;` should be `guild`, since `i` is stale. This is inside `#ifdef ORIGINAL_GROUP_ORDER`, which is legacy code and not normally compiled. Function: `Util_Read_Functional_Group_XML()`.

**Code at issue** — `atUtilGroupIO.c` line 1084  
Search for: `TLorder2[eatIndex++] = i`

```c
 1082:     for (guild = 0; guild < bm->K_num_tot_sp; guild++){
 1083:         if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED)
 1084→             TLorder2[eatIndex++] = i;
 1085: 	}
 1086: 	TLorder2[eatIndex++] = DLguild_id;
```

**Suggested fix (from review):** Use `guild`.

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
