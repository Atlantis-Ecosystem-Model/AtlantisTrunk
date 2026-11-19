# atmanage/atManageParamIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **7 findings** (severity H: 2; confidence H and severity H: 1; on the priority list: 1), plus 2 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MNGIO-01](#mngio-01) ⭐ | 1069, 1074, 1081 | H/H | always-true condition / buffer overrun | `POP_changes = 1; ... if (POP_changes > 0) bm->flagchangepop = 0;` is always true, so the flagchangepop settin… |  | [ ] |
| [MNGIO-02](#mngio-02) | 343 | H/M | buffer overrun (config dependent) | `EFFORTchange` is allocated `[nf][Effort_max_num_changes][4]`, with max = 1 when `flagchangeeffort == 0`. `rea… |  | [ ] |
| [MNGIO-03](#mngio-03) | 662 | H/M | copy-paste / wrong param key | `bm->CPUEshotavgdis = Util_XML_Read_Value(..., "CPUEshotfdis");` reads the integer error-case flag instead of … |  | [ ] |
| [MNGIO-04](#mngio-04) 🔀 | 153, 144 | M/H | buffer overrun / index collision | `bm->BiTAC_sp[b][region][guild][old_id]` with `region < bm->K_num_reg`. BiTAC_sp is allocated with a fixed reg… |  | [ ] |
| [MNGIO-05](#mngio-05) | 294, 306 | M/M | buffer overrun (unchecked bound) | The loop `co_sp < FunctGroupArray[guild].speciesParams[max_co_sp_id]` writes `co_sp_catch[nf][co_sp]`, but co_… |  | [ ] |
| [MNGIO-06](#mngio-06) | 768, 503 | H/L | NULL dereference | `childGroupingNode = Util_XML_Get_Node(..., "Location_And_Timing")` is not NULL-checked before `readPortXMLDat… |  | [ ] |
| [MNGIO-07](#mngio-07) | 335 | H/L | error-message text | `quit("errorString%s attribute group not found.\n", ...)` prints a literal "errorString". Line 748 says "EPUE_… |  | [ ] |

---

## MNGIO-01

**always-true condition / buffer overrun** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageParamIO.c` 1069, `atManageParamIO.c` 1074, `atManageParamIO.c` 1081
**Lines at review (Oct 2026):** 1069-1081, 517-524

**Why it's thought to be a bug**

`POP_changes = 1; ... if (POP_changes > 0) bm->flagchangepop = 0;` is always true, so the flagchangepop setting from the input file is always switched off. As a result `POP_max_num_changes` is always 1 and `bm->POPchange = alloc3d(3, 1, K_num_ports)` gives `[ports][1][3]`. `readPopulationChangeXML()` then writes `bm->POPchange[index][b][paramID]` for `b < Port_info[index][POP_num_changes_id]`, which overflows whenever a port has 2 or more changes. The TODO comment at 1068 already suspects this. Functions: `readManagementChangeXML()`, `readPopulationChangeXML()`.

**Code at issue** — `atManageParamIO.c` line 1069  
Search for: `POP_changes = 1`

```c
 1067: 	/* Get the max number of POP changes and then allocate the POP change array.*/
 1068: 	//TODO: This might be a bug - should POP_changes be set to 0 intially?
 1069→ 	POP_changes = 1;
 1070: 	for (i = 0; i < bm->K_num_ports; i++) {
 1071: 		POP_changes = (int) max ( (double) POP_changes, bm->Port_info[i][POP_num_changes_id] );
```

**Code at issue** — `atManageParamIO.c` line 1074  
Search for: `if (POP_changes > 0)`

```c
 1072: 	}
 1073: 
 1074→ 	if (POP_changes > 0)
 1075: 		bm->flagchangepop = 0;
 1076: 
```

**Code at issue** — `atManageParamIO.c` line 1081 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->POPchange = (double ***) alloc3d(3, POP_max_num_changes, bm->K_num_ports);`

```c
 1079: 		POP_max_num_changes = POP_changes + 1;
 1080: 
 1081→ 	bm->POPchange = (double ***) alloc3d(3, POP_max_num_changes, bm->K_num_ports);
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
**Found at (current code):** `atManageParamIO.c` 343
**Lines at review (Oct 2026):** 1019-1023, 343-351, 1037-1042

**Why it's thought to be a bug**

`EFFORTchange` is allocated `[nf][Effort_max_num_changes][4]`, with max = 1 when `flagchangeeffort == 0`. `readEffortChangeXML()` still reads `numChanges = FISHERYprms[nf][EFF_num_changes_id]` values into `EFFORTchange[nf][b]` for b<numChanges, regardless of the flag, so it overflows if any fishery has >1 changes while the flag is off. `CAPchange` has the same problem with `flagchangecap` through `Util_XML_Read_Change_Values()` (atUtilFisheryXML.c:463-472). Functions: `readEffortChangedXML()`, `readEffortChangeXML()`.

**Code at issue** — `atManageParamIO.c` line 343 (closest match to the quoted code; wording differs slightly)  
Search for: `numChanges = (int)bm->FISHERYprms[fisheryIndex][EFF_num_changes_id];`

```c
  341: 	for (fisheryIndex = 0; fisheryIndex < bm->K_num_fisheries; fisheryIndex++) {
  342: 
  343→ 		numChanges = (int)bm->FISHERYprms[fisheryIndex][EFF_num_changes_id];
  344: 		if (numChanges > 0) {
  345: 			if (Util_XML_Read_Array_Double(ATLANTIS_FISHERY_ATTRIBUTE, fileName, errorString, attributeNode, no_checking, FisheryArray[fisheryIndex].fisheryCode, &values,
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
**Found at (current code):** `atManageParamIO.c` 662
**Lines at review (Oct 2026):** 662

**Function:** `readManamentFlagTimeXML()`

**Why it's thought to be a bug**

`bm->CPUEshotavgdis = Util_XML_Read_Value(..., "CPUEshotfdis");` reads the integer error-case flag instead of the bias parameter. ConvertAtlantis/atManagetoXML.c:550 creates `"CPUEshotavgdis"`, and atManage.c:2862 uses it as the bias. Function: `readManamentFlagTimeXML()`.

**Code at issue** — `atManageParamIO.c` line 662  
Search for: `CPUEshotavgdis = Util_XML_Read_Value(`

```c
  660:         bm->PerShotCDFtype = (int)(Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, integer_check, "PerShotCDFtype"));
  661:         bm->CPUEshotfdis = (int)(Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, integer_check, "CPUEshotfdis"));
  662→         bm->CPUEshotavgdis = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, no_checking, "CPUEshotfdis");
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
**Found at (current code):** `atManageParamIO.c` 153, `atManageParamIO.c` 144
**Lines at review (Oct 2026):** 144-153

**Function:** `readBiMonthlyXML()`

**Why it's thought to be a bug**

`bm->BiTAC_sp[b][region][guild][old_id]` with `region < bm->K_num_reg`. BiTAC_sp is allocated with a fixed region dimension of `totalreg_id + 1 = 3` (atManageSetup.c:1195, totalreg_id=2), but K_num_reg can be larger (atbiolsetup.c:599/605). Even with K_num_reg==3, region index 2 is the "total" slot that atManageSetup.c:979 overwrites. Function: `readBiMonthlyXML()`.

> 🔀 **Changed by the merge of main (Nov 2026):** BiTAC_sp now zero-initialised. Check whether this finding is now fixed before acting.

**Code at issue** — `atManageParamIO.c` line 153  
Search for: `BiTAC_sp[b][region][guild][old_id]`

```c
  151: 
  152: 				for (b = 0; b < 6; b++) {
  153→ 					bm->BiTAC_sp[b][region][guild][old_id] = values[b];
  154: 				}
  155: 				free(values);
```

**Code at issue** — `atManageParamIO.c` line 144  
Search for: `region < bm->K_num_reg`

```c
  142: 			if (speciesNode == NULL)
  143: 				quit("readBiMonthlyXML - %s attribute group not found.\n", errorString);
  144→ 			for (region = 0; region < bm->K_num_reg; region++) {
  145: 
  146: 				snprintf(str, sizeof(str), "region%d", region + 1);
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
**Found at (current code):** `atManageParamIO.c` 294, `atManageParamIO.c` 306
**Lines at review (Oct 2026):** 294-306

**Function:** `Read_Fishery_Companion_Catch_XML()`

**Why it's thought to be a bug**

The loop `co_sp < FunctGroupArray[guild].speciesParams[max_co_sp_id]` writes `co_sp_catch[nf][co_sp]`, but co_sp_catch is allocated `[nf][K_max_co_sp]` (line 933). Nothing checks that max_co_sp ≤ K_max_co_sp. Function: `Read_Fishery_Companion_Catch_XML()`.

**Code at issue** — `atManageParamIO.c` line 294  
Search for: `co_sp < FunctGroupArray[guild].speciesParams[max_co_sp_id]`

```c
  292:                 quit("Companion_Parameters/co_sp_catch/%s species attribute group not found.\n", FunctGroupArray[guild].groupCode);
  293: 
  294→             for (co_sp = 0; co_sp < FunctGroupArray[guild].speciesParams[max_co_sp_id]; co_sp++) {
  295: 
  296:                 snprintf(str, sizeof(str), "companion%d", co_sp + 1);
```

**Code at issue** — `atManageParamIO.c` line 306  
Search for: `co_sp_catch[nf][co_sp]`

```c
  304: 
  305:                 for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  306→                     FunctGroupArray[guild].co_sp_catch[nf][co_sp] = values[nf];
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
**Found at (current code):** `atManageParamIO.c` 768, `atManageParamIO.c` 503
**Lines at review (Oct 2026):** 768-770

**Function:** `readPortIndexXML()`

**Why it's thought to be a bug**

`childGroupingNode = Util_XML_Get_Node(..., "Location_And_Timing")` is not NULL-checked before `readPortXMLData()` calls `Util_Get_Node_Name(parent)`, unlike every other group lookup in the file. Function: `readPortIndexXML()`.

**Code at issue** — `atManageParamIO.c` line 768  
Search for: `childGroupingNode = Util_XML_Get_Node(`

```c
  766: 	Util_XML_Read_Fishery_Param_Int(bm, fileName, groupingNode, "FisheryPort", no_checking, bm->Port_Fishery, bm->K_num_ports);
  767: 
  768→ 	childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Location_And_Timing");
  769: 
  770: 	readPortXMLData(bm, fileName, childGroupingNode, "ports_x", portx_id);
```

**Code at issue** — `atManageParamIO.c` line 503  
Search for: `Util_Get_Node_Name(parent)`

```c
  501: 	char str[STRLEN];
  502: 	char errorString[STRLEN];
  503→ 	char *nodeName =  Util_Get_Node_Name(parent);
  504: 
  505: 	snprintf(errorString, sizeof(errorString), "%s/%s", nodeName, paramName);
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
**Found at (current code):** `atManageParamIO.c` 335
**Lines at review (Oct 2026):** 335, 748

**Why it's thought to be a bug**

`quit("errorString%s attribute group not found.\n", ...)` prints a literal "errorString". Line 748 says "EPUE_Thresholds" for the CPUE_Thresholds group. Functions: `readEffortChangeXML()`, `readManagementEffortXML()`.

**Code at issue** — `atManageParamIO.c` line 335  
Search for: `quit("errorString%s attribute group not found.\n",`

```c
  333: 	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, paramName);
  334: 	if (attributeNode == NULL)
  335→ 		quit("errorString%s attribute group not found.\n", errorString);
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
