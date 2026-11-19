# atlantisUtil/atUtilGroupIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **8 findings** (severity H: 2; confidence H and severity H: 2; on the priority list: 2).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [UTIL-17](#util-17) ⭐ | 910-912 | H/H | cohort vs cohort x genotype | `contaminantTracers` and `contamPropTracers` are `Util_Alloc_Init_2D_Int(num_contaminants, numCohorts)`, which… |  | [ ] |
| [UTIL-18](#util-18) ⭐ | 894-905 | H/H | wrong dimension (habitat) and cohort vs cohort x genotype | The atomic-ratio arrays hard-code 3 habitats: `ratioLost = 3D(num_atomic_id, nCxG, 3)` gives `[3][nCxG][..]`, … |  | [ ] |
| [UTIL-19](#util-19) | 1429-1435, 1440, 1502 | H/M | index beyond logical array size | `habitatCoeffs` is allocated with `bm->num_active_habitats` entries (3 = EPIFAUNA+1 without ice, 5 = ICE_BASED… |  | [ ] |
| [UTIL-20](#util-20) | 397-399 | H/M | contradictory check / uninitialised member | `isAssessed` is read with `binary_check` (line 394), so a value >1 quits and `if(isAssessed > 1) isCloseKin = … |  | [ ] |
| [UTIL-21](#util-21) | 421-423 | H/M | uninitialised member | `isCollisionRisk` is only set when `bm->flagindustry_on`. Otherwise it's uninitialised malloc memory, read at … |  | [ ] |
| [UTIL-22](#util-22) | 545-547 | H/M | unreachable condition | `if(groupType == TURF) bm->containsTurf = TRUE;` sits inside the `isVertebrate == TRUE` branch, but TURF is ne… |  | [ ] |
| [UTIL-23](#util-23) | 244-252, 489-533 | M/L | NULL dereference | If `groupList` is NULL or empty, `FunctGroupArray` is never allocated and the count check is skipped. `Set_Gro… |  | [ ] |
| [UTIL-24](#util-24) | 1083-1085 | H/L | wrong loop variable | `TLorder2[eatIndex++] = i;` should be `guild`, since `i` is stale. This is inside `#ifdef ORIGINAL_GROUP_ORDER… |  | [ ] |

---

## UTIL-17

**cohort vs cohort x genotype** · Conf H / Sev H ⭐ priority  
**Listed lines:** 910-912

**Function:** `Util_Read_Functional_Group_XML()`

**Why it's thought to be a bug**

`contaminantTracers` and `contamPropTracers` are `Util_Alloc_Init_2D_Int(num_contaminants, numCohorts)`, which gives `[numCohorts][ncontam]`. `contaminantSpMort` is 1D with `numCohorts` entries. They are indexed with `cohort < numCohortsXnumGenes` in atecology/atContaminants.c:592-607 (`contaminantSpMort[cohort]`, `contaminantTracers[cohort][cIndex]`) and atmanage/atManage.c:2069-2070. That's an out-of-bounds read/write whenever `numGeneTypes > 1` with contaminants on. Function: `Util_Read_Functional_Group_XML()`.

**Current code** (`atlantisUtil/atUtilGroupIO.c` lines 908–914)

```c
  908: 
  909: 		if(bm->track_contaminants){
  910: 			FunctGroupArray[i].contaminantTracers = Util_Alloc_Init_2D_Int(bm->num_contaminants, FunctGroupArray[i].numCohorts, 0);
  911:             FunctGroupArray[i].contamPropTracers = Util_Alloc_Init_2D_Int(bm->num_contaminants, FunctGroupArray[i].numCohorts, 0);
  912: 			FunctGroupArray[i].contaminantSpMort = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohorts, 0.0);
  913: 			FunctGroupArray[i].calcCLinearMort = Util_Alloc_Init_2D_Double(3, FunctGroupArray[i].numCohortsXnumGenes, 0.0);
  914:             FunctGroupArray[i].agingContam = Util_Alloc_Init_4D_Double((bm->wcnz+bm->sednz), bm->nbox, bm->num_contaminants, FunctGroupArray[i].numCohorts, 0);
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
**Listed lines:** 894-905

**Function:** `Util_Read_Functional_Group_XML()`

**Why it's thought to be a bug**

The atomic-ratio arrays hard-code 3 habitats: `ratioLost = 3D(num_atomic_id, nCxG, 3)` gives `[3][nCxG][..]`, and `ratioLostGlobal = 4D(..., 3, 3)` gives `[3][3][nCxG][..]`. atecology/atecology.c:3899-3909 and 4036-4052 loop `hab < bm->num_active_habitats`, which is 5 when ice is on (attracerIO.c:239), so they write past the first dimension. `ratioLostPred`, `ratioLostPredGlobal`, `ratioGained` and `ratioGainedGlobal` are also sized `numCohorts` but looped with `cohort < numCohortsXnumGenes` in the same code, so they overflow when `numGeneTypes > 1`. Function: `Util_Read_Functional_Group_XML()`.

**Current code** (`atlantisUtil/atUtilGroupIO.c` lines 892–907)

```c
  892: 			FunctGroupArray[i].addRatioFluxes = Util_Alloc_Init_2D_Double(num_atomic_id + 1, FunctGroupArray[i].numCohortsXnumGenes, 0.0);
  893: 
  894: 			FunctGroupArray[i].ratioLost = Util_Alloc_Init_3D_Long_Double(num_atomic_id, FunctGroupArray[i].numCohortsXnumGenes, 3, 0.0);
  895: 			FunctGroupArray[i].ratioLostGlobal = Util_Alloc_Init_4D_Long_Double(num_atomic_id, FunctGroupArray[i].numCohortsXnumGenes, 3, 3, 0.0);
  896: 			FunctGroupArray[i].ratioGainedPred = Util_Alloc_Init_2D_Long_Double(num_atomic_id, FunctGroupArray[i].numCohortsXnumGenes, 0.0);
  897: 			//FunctGroupArray[i].ratioGainedPredGlobal = Util_Alloc_Init_4D_Long_Double(num_atomic_id, FunctGroupArray[i].numCohorts, 3, 3, 0.0);
  898: 
  899: 			FunctGroupArray[i].addRatioLost = Util_Alloc_Init_2D_Double(num_atomic_id, FunctGroupArray[i].numCohortsXnumGenes, 0);
  900: 
  901: 			FunctGroupArray[i].ratioLostPred = Util_Alloc_Init_3D_Long_Double(num_atomic_id, FunctGroupArray[i].numCohorts, 3, 0.0);
  902: 			FunctGroupArray[i].ratioLostPredGlobal = Util_Alloc_Init_4D_Long_Double(num_atomic_id, FunctGroupArray[i].numCohorts, 3, 3, 0.0);
  903: 
  904: 			FunctGroupArray[i].ratioGained = Util_Alloc_Init_3D_Long_Double(num_atomic_id, FunctGroupArray[i].numCohorts, 3, 0.0);
  905: 			FunctGroupArray[i].ratioGainedGlobal = Util_Alloc_Init_4D_Long_Double(num_atomic_id, FunctGroupArray[i].numCohorts, 3, 3, 0.0);
  906: 
  907: 		}
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
**Listed lines:** 1429-1435, 1440, 1502

**Function:** `Set_Group_Habitats()`

**Why it's thought to be a bug**

`habitatCoeffs` is allocated with `bm->num_active_habitats` entries (3 = EPIFAUNA+1 without ice, 5 = ICE_BASED+1 with ice). The code then writes `[ICE_BASED]`(4), `[LAND_BASED]`(3) and `[MIXED]`(5). There's no heap corruption only because `Util_Alloc_Init_1D_Int` allocates double-sized storage. But the land flag (with terrestrial on and ice off) and the MIXED flag for ice groups fall outside every `hab < num_active_habitats` loop, so they're silently ignored. Function: `Set_Group_Habitats()`.

**Current code** (`atlantisUtil/atUtilGroupIO.c` lines 1427–1437)

```c
 1427: 	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
 1428: 
 1429: 		FunctGroupArray[guild].habitatCoeffs = Util_Alloc_Init_1D_Int(bm->num_active_habitats, 0);
 1430: 
 1431: 		FunctGroupArray[guild].habitatCoeffs[WC] = 0;
 1432: 		FunctGroupArray[guild].habitatCoeffs[SED] = 0;
 1433: 		FunctGroupArray[guild].habitatCoeffs[EPIFAUNA] = 0;
 1434: 		FunctGroupArray[guild].habitatCoeffs[ICE_BASED] = 0;
 1435: 		FunctGroupArray[guild].habitatCoeffs[LAND_BASED] = 0;
 1436: 
 1437: 
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
**Listed lines:** 397-399

**Function:** `Util_Read_Functional_Group_XML()`

**Why it's thought to be a bug**

`isAssessed` is read with `binary_check` (line 394), so a value >1 quits and `if(isAssessed > 1) isCloseKin = 1` can never fire. `FunctGroupArray` comes from `malloc` (line 252) and `isCloseKin` is never set otherwise, so it holds garbage, and atSS3Link/atSS3DataGen.c:85 tests it. Function: `Util_Read_Functional_Group_XML()`.

**Current code** (`atlantisUtil/atUtilGroupIO.c` lines 395–401)

```c
  395: 			Util_XML_Get_Value_Double(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, binary_check, "IsCatchGrazer", &FunctGroupArray[groupIndex].speciesParams[catcheater_id]);
  396:             
  397:             if(FunctGroupArray[groupIndex].isAssessed > 1) {
  398:                 FunctGroupArray[groupIndex].isCloseKin = 1;
  399:             }
  400: 
  401: 			Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, binary_check, "OverWinters", &FunctGroupArray[groupIndex].isOverWinter);
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
**Listed lines:** 421-423

**Function:** `Util_Read_Functional_Group_XML()`

**Why it's thought to be a bug**

`isCollisionRisk` is only set when `bm->flagindustry_on`. Otherwise it's uninitialised malloc memory, read at atecology/atbiolmessage.c:2273/2303. Function: `Util_Read_Functional_Group_XML()`.

**Current code** (`atlantisUtil/atUtilGroupIO.c` lines 419–425)

```c
  419:             }
  420:             
  421:             if(bm->flagindustry_on) {
  422:                 Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "isCollisionRisk", &FunctGroupArray[groupIndex].isCollisionRisk);
  423:             }
  424: 
  425:             if(bm->terrestrial_on){
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
**Listed lines:** 545-547

**Function:** `Util_Read_Functional_Group_XML()`

**Why it's thought to be a bug**

`if(groupType == TURF) bm->containsTurf = TRUE;` sits inside the `isVertebrate == TRUE` branch, but TURF is never a vertebrate, so `containsTurf` is always FALSE. atecology/atcoral.c:907/932 then forces `facilitate_space = 1.0` and skips the turf facilitation logic. Function: `Util_Read_Functional_Group_XML()`.

**Current code** (`atlantisUtil/atUtilGroupIO.c` lines 543–549)

```c
  543: 			}
  544: 
  545: 			if(FunctGroupArray[guild].groupType == TURF){
  546: 				bm->containsTurf = TRUE;
  547: 			}
  548: 
  549: 			if(FunctGroupArray[guild].habitatType != WC)
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
**Listed lines:** 244-252, 489-533

**Function:** `Util_Read_Functional_Group_XML()`

**Why it's thought to be a bug**

If `groupList` is NULL or empty, `FunctGroupArray` is never allocated and the count check is skipped. `Set_Group_Habitats` and the later loops then dereference NULL `FunctGroupArray`. Function: `Util_Read_Functional_Group_XML()`.

**Current code** (`atlantisUtil/atUtilGroupIO.c` lines 242–254)

```c
  242: 	groupList = Util_XML_Get_Node_List(ATLANTIS_ATTRIBUTE_SUB_GROUP, inputDoc->children, "FunctionalGroup");
  243: 
  244: 	if (groupList && groupList->nodesetval->nodeNr > 0) {
  245: 
  246: 		/* Check the number of groups match the values in the run input file*/
  247: 		numGroups = groupList->nodesetval->nodeNr;
  248: 		if (numGroups != bm->K_num_tot_sp) {
  249: 			quit("Util_Read_Functional_Group_XML: Number of groups specified (%d) in your group definition file %s does not match the K_num_tot_sp value (%d) in your run file\n", numGroups, convertedXMLFileName, bm->K_num_tot_sp);
  250: 		}
  251: 
  252: 		FunctGroupArray = (FunctionalGroupStruct *) malloc(sizeof(FunctionalGroupStruct) * (size_t)(bm->K_num_tot_sp + 1));
  253: 
  254: #ifdef ORIGINAL_GROUP_ORDER
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
**Listed lines:** 1083-1085

**Function:** `Util_Read_Functional_Group_XML()`

**Why it's thought to be a bug**

`TLorder2[eatIndex++] = i;` should be `guild`, since `i` is stale. This is inside `#ifdef ORIGINAL_GROUP_ORDER`, which is legacy code and not normally compiled. Function: `Util_Read_Functional_Group_XML()`.

**Current code** (`atlantisUtil/atUtilGroupIO.c` lines 1081–1087)

```c
 1081: 
 1082: 	//for (i = 0; i < bm->K_num_vert_sp; i++) {
 1083:     for (guild = 0; guild < bm->K_num_tot_sp; guild++){
 1084:         if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED)
 1085:             TLorder2[eatIndex++] = i;
 1086: 	}
 1087: 	TLorder2[eatIndex++] = DLguild_id;
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
