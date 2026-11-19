# atmanage/atManageParamIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **7 findings** (severity H: 2; confidence H and severity H: 1; on the priority list: 1), plus 2 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MNGIO-01](#mngio-01) ⭐ | 1069-1081, 517-524 | H/H | always-true condition / buffer overrun | `POP_changes = 1; ... if (POP_changes > 0) bm->flagchangepop = 0;` is always true, so the flagchangepop settin… |  | [ ] |
| [MNGIO-02](#mngio-02) | 1019-1023, 343-351, 1037-1042 | H/M | buffer overrun (config dependent) | `EFFORTchange` is allocated `[nf][Effort_max_num_changes][4]`, with max = 1 when `flagchangeeffort == 0`. `rea… |  | [ ] |
| [MNGIO-03](#mngio-03) | 662 | H/M | copy-paste / wrong param key | `bm->CPUEshotavgdis = Util_XML_Read_Value(..., "CPUEshotfdis");` reads the integer error-case flag instead of … |  | [ ] |
| [MNGIO-04](#mngio-04) 🔀 | 144-153 | M/H | buffer overrun / index collision | `bm->BiTAC_sp[b][region][guild][old_id]` with `region < bm->K_num_reg`. BiTAC_sp is allocated with a fixed reg… |  | [ ] |
| [MNGIO-05](#mngio-05) | 294-306 | M/M | buffer overrun (unchecked bound) | The loop `co_sp < FunctGroupArray[guild].speciesParams[max_co_sp_id]` writes `co_sp_catch[nf][co_sp]`, but co_… |  | [ ] |
| [MNGIO-06](#mngio-06) | 768-770 | H/L | NULL dereference | `childGroupingNode = Util_XML_Get_Node(..., "Location_And_Timing")` is not NULL-checked before `readPortXMLDat… |  | [ ] |
| [MNGIO-07](#mngio-07) | 335, 748 | H/L | error-message text | `quit("errorString%s attribute group not found.\n", ...)` prints a literal "errorString". Line 748 says "EPUE_… |  | [ ] |

---

## MNGIO-01

**always-true condition / buffer overrun** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 1069-1081, 517-524  
**Lines at review (Oct 2026):** 1069-1081, 517-524

**Why it's thought to be a bug**

`POP_changes = 1; ... if (POP_changes > 0) bm->flagchangepop = 0;` is always true, so the flagchangepop setting from the input file is always switched off. As a result `POP_max_num_changes` is always 1 and `bm->POPchange = alloc3d(3, 1, K_num_ports)` gives `[ports][1][3]`. `readPopulationChangeXML()` then writes `bm->POPchange[index][b][paramID]` for `b < Port_info[index][POP_num_changes_id]`, which overflows whenever a port has 2 or more changes. The TODO comment at 1068 already suspects this. Functions: `readManagementChangeXML()`, `readPopulationChangeXML()`.

**Current code** (`atmanage/atManageParamIO.c` lines 1067–1083)

```c
 1067: 	/* Get the max number of POP changes and then allocate the POP change array.*/
 1068: 	//TODO: This might be a bug - should POP_changes be set to 0 intially?
 1069: 	POP_changes = 1;
 1070: 	for (i = 0; i < bm->K_num_ports; i++) {
 1071: 		POP_changes = (int) max ( (double) POP_changes, bm->Port_info[i][POP_num_changes_id] );
 1072: 	}
 1073: 
 1074: 	if (POP_changes > 0)
 1075: 		bm->flagchangepop = 0;
 1076: 
 1077: 	POP_max_num_changes = 1;
 1078: 	if (bm->flagchangepop)
 1079: 		POP_max_num_changes = POP_changes + 1;
 1080: 
 1081: 	bm->POPchange = (double ***) alloc3d(3, POP_max_num_changes, bm->K_num_ports);
 1082: 
 1083: 	for (i = 0; i < POP_max_num_changes; i++) {
```

**Suggested fix (from review):** Probably `if (POP_changes <= 0) flagchangepop = 0;` (start POP_changes at 0). Always size the array from the real maximum count, or clamp `counter` to POP_max_num_changes.

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

## MNGIO-02

**buffer overrun (config dependent)** · Conf H / Sev M  
**Lines (current code):** 1019-1023, 343-351, 1037-1042  
**Lines at review (Oct 2026):** 1019-1023, 343-351, 1037-1042

**Why it's thought to be a bug**

`EFFORTchange` is allocated `[nf][Effort_max_num_changes][4]`, with max = 1 when `flagchangeeffort == 0`. `readEffortChangeXML()` still reads `numChanges = FISHERYprms[nf][EFF_num_changes_id]` values into `EFFORTchange[nf][b]` for b<numChanges, regardless of the flag, so it overflows if any fishery has >1 changes while the flag is off. `CAPchange` has the same problem with `flagchangecap` through `Util_XML_Read_Change_Values()` (atUtilFisheryXML.c:463-472). Functions: `readEffortChangedXML()`, `readEffortChangeXML()`.

**Current code** (`atmanage/atManageParamIO.c` lines 1017–1044)

```c
 1017: 	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, EFF_num_changes_id);
 1018: 
 1019: 	Effort_max_num_changes = 1;
 1020: 	if (bm->flagchangeeffort)
 1021: 		Effort_max_num_changes = Manage_Get_Max_Fishery_Param(bm, EFF_num_changes_id);
 1022: 
 1023: 	EFFORTchange = Util_Alloc_Init_3D_Double(4, Effort_max_num_changes, bm->K_num_fisheries, 0.0);
 1024: 
 1025: 	readEffortChangeXML(bm, fileName, groupingNode, "StartChangeDay", start_id);
 1026: 	readEffortChangeXML(bm, fileName, groupingNode, "StartChangePeriod", period_id);
 1027: 	readEffortChangeXML(bm, fileName, groupingNode, "EffortChangeProportion", mult_id);
 1028: 	readEffortChangeXML(bm, fileName, groupingNode, "EffortChangePulseFlag", pulse_id);
 1029: 
 1030: 	childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Effort_CAP");
 1031: 	if (childGroupingNode == NULL)
 1032: 		quit("Effort_CAP: Effort_CAP attribute group not found in input file %s.\n", fileName);
 1033: 
 1034: 	Util_XML_Read_Fishery_Param(bm, fileName, childGroupingNode, CAP_num_changes_id);
 1035: 
 1036: 	/* Get the maximum number of CAP changes so we can allocate the array and load the values */
 1037: 	CAP_max_num_changes = 1;
 1038: 	if (bm->flagchangecap)
 1039: 		CAP_max_num_changes = Manage_Get_Max_Fishery_Param(bm, CAP_num_changes_id) + 1;
 1040: 	CAPchange = Util_Alloc_Init_3D_Double(4, CAP_max_num_changes, bm->K_num_fisheries, 0.0);
 1041: 
 1042: 	Util_XML_Read_Change_Values(bm, fileName, childGroupingNode, "CAPchange", CAP_num_changes_id, CAPchange);
 1043: 
 1044: }
```

**Suggested fix (from review):** Always size from `Manage_Get_Max_Fishery_Param()`, or clamp the number of reads to the allocated size.

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

## MNGIO-03

**copy-paste / wrong param key** · Conf H / Sev M  
**Lines (current code):** 662  
**Lines at review (Oct 2026):** 662

**Function:** `readManamentFlagTimeXML()`

**Why it's thought to be a bug**

`bm->CPUEshotavgdis = Util_XML_Read_Value(..., "CPUEshotfdis");` reads the integer error-case flag instead of the bias parameter. ConvertAtlantis/atManagetoXML.c:550 creates `"CPUEshotavgdis"`, and atManage.c:2862 uses it as the bias. Function: `readManamentFlagTimeXML()`.

**Current code** (`atmanage/atManageParamIO.c` lines 660–664)

```c
  660:         bm->PerShotCDFtype = (int)(Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, integer_check, "PerShotCDFtype"));
  661:         bm->CPUEshotfdis = (int)(Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, integer_check, "CPUEshotfdis"));
  662:         bm->CPUEshotavgdis = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, no_checking, "CPUEshotfdis");
  663:         bm->CPUEshotvartdis = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, no_checking, "CPUEshotvartdis");
  664:     
```

**Suggested fix (from review):** Change the key to `"CPUEshotavgdis"`.

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

## MNGIO-04

**buffer overrun / index collision** · Conf M / Sev H  
**Lines (current code):** 144-153  
**Lines at review (Oct 2026):** 144-153

**Function:** `readBiMonthlyXML()`

**Why it's thought to be a bug**

`bm->BiTAC_sp[b][region][guild][old_id]` with `region < bm->K_num_reg`. BiTAC_sp is allocated with a fixed region dimension of `totalreg_id + 1 = 3` (atManageSetup.c:1195, totalreg_id=2), but K_num_reg can be larger (atbiolsetup.c:599/605). Even with K_num_reg==3, region index 2 is the "total" slot that atManageSetup.c:979 overwrites. Function: `readBiMonthlyXML()`.

> 🔀 **Changed by the merge of main (Nov 2026):** BiTAC_sp now zero-initialised. Check whether this finding is now fixed before acting.

**Current code** (`atmanage/atManageParamIO.c` lines 142–155)

```c
  142: 			if (speciesNode == NULL)
  143: 				quit("readBiMonthlyXML - %s attribute group not found.\n", errorString);
  144: 			for (region = 0; region < bm->K_num_reg; region++) {
  145: 
  146: 				snprintf(str, sizeof(str), "region%d", region + 1);
  147: 
  148: 				if (Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, errorString, speciesNode, proportion_check, str, &values, 6) == FALSE) {
  149: 					quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, str, fileName);
  150: 				}
  151: 
  152: 				for (b = 0; b < 6; b++) {
  153: 					bm->BiTAC_sp[b][region][guild][old_id] = values[b];
  154: 				}
  155: 				free(values);
```

**Suggested fix (from review):** Allocate the region dim as K_num_reg+1 and use K_num_reg as the total index, or quit if K_num_reg > 2.

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

## MNGIO-05

**buffer overrun (unchecked bound)** · Conf M / Sev M  
**Lines (current code):** 294-306  
**Lines at review (Oct 2026):** 294-306

**Function:** `Read_Fishery_Companion_Catch_XML()`

**Why it's thought to be a bug**

The loop `co_sp < FunctGroupArray[guild].speciesParams[max_co_sp_id]` writes `co_sp_catch[nf][co_sp]`, but co_sp_catch is allocated `[nf][K_max_co_sp]` (line 933). Nothing checks that max_co_sp ≤ K_max_co_sp. Function: `Read_Fishery_Companion_Catch_XML()`.

**Current code** (`atmanage/atManageParamIO.c` lines 292–308)

```c
  292:                 quit("Companion_Parameters/co_sp_catch/%s species attribute group not found.\n", FunctGroupArray[guild].groupCode);
  293: 
  294:             for (co_sp = 0; co_sp < FunctGroupArray[guild].speciesParams[max_co_sp_id]; co_sp++) {
  295: 
  296:                 snprintf(str, sizeof(str), "companion%d", co_sp + 1);
  297:                 snprintf(errorString, sizeof(errorString), "co_sp_catch/%s/%s", FunctGroupArray[guild].groupCode, str);
  298: 
  299:                 if (Util_XML_Read_Array_Double(ATLANTIS_COMPANION_ATTRIBUTE, fileName, errorString, speciesNode, no_checking, str, &values, bm->K_num_fisheries) == FALSE) {
  300:                     quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, str, fileName);
  301:                 }
  302: 
  303:                 //printf("For %s co_sp %d ", FunctGroupArray[guild].groupCode, co_sp);
  304: 
  305:                 for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  306:                     FunctGroupArray[guild].co_sp_catch[nf][co_sp] = values[nf];
  307:                     
  308:                     //printf(" %e ", values[nf]);
```

**Suggested fix (from review):** Quit if `max_co_sp_id > K_max_co_sp`.

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

## MNGIO-06

**NULL dereference** · Conf H / Sev L  
**Lines (current code):** 768-770  
**Lines at review (Oct 2026):** 768-770

**Function:** `readPortIndexXML()`

**Why it's thought to be a bug**

`childGroupingNode = Util_XML_Get_Node(..., "Location_And_Timing")` is not NULL-checked before `readPortXMLData()` calls `Util_Get_Node_Name(parent)`, unlike every other group lookup in the file. Function: `readPortIndexXML()`.

**Current code** (`atmanage/atManageParamIO.c` lines 766–772)

```c
  766: 	Util_XML_Read_Fishery_Param_Int(bm, fileName, groupingNode, "FisheryPort", no_checking, bm->Port_Fishery, bm->K_num_ports);
  767: 
  768: 	childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Location_And_Timing");
  769: 
  770: 	readPortXMLData(bm, fileName, childGroupingNode, "ports_x", portx_id);
  771: 	readPortXMLData(bm, fileName, childGroupingNode, "ports_y", porty_id);
  772: 	readPortXMLData(bm, fileName, childGroupingNode, "ports_start", port_start_id);
```

**Suggested fix (from review):** Add a NULL check that calls quit.

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

## MNGIO-07

**error-message text** · Conf H / Sev L  
**Lines (current code):** 335, 748  
**Lines at review (Oct 2026):** 335, 748

**Why it's thought to be a bug**

`quit("errorString%s attribute group not found.\n", ...)` prints a literal "errorString". Line 748 says "EPUE_Thresholds" for the CPUE_Thresholds group. Functions: `readEffortChangeXML()`, `readManagementEffortXML()`.

**Current code** (`atmanage/atManageParamIO.c` lines 333–337)

```c
  333: 	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, paramName);
  334: 	if (attributeNode == NULL)
  335: 		quit("errorString%s attribute group not found.\n", errorString);
  336: 
  337: 	if (verbose)
```

**Suggested fix (from review):** Fix the message text.

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

atManageParamIO.c `readPopulationChangeXML()` calls `Util_XML_Read_Array_Double` with size `counter`, which may be 0. Other readers guard with `if (numChanges > 0)`.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atManageParamIO.c:1195 `Read_Manage_Paramaters` does not check `inputDoc` for NULL (depends on whether xmlReadFileDestFolder calls quit itself).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
