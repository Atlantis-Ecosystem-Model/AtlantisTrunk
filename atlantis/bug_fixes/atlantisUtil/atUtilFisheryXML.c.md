# atlantisUtil/atUtilFisheryXML.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 3 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [UTL2-01](#utl2-01) | 461-473 | M/M | buffer overrun | `counter = (int) bm->FISHERYprms[fisheryIndex][paramID]; ... array[fisheryIndex][b][index] = values[b];` for b… |  | [ ] |
| [UTL2-02](#utl2-02) | 504-521 | M/M | buffer overrun | Same pattern for groups: `for (b = 0; b < bm->SP_FISHERYprms[sp][i][paramID]; b++) array[sp][i][b][index] = va… |  | [ ] |
| [UTL2-03](#utl2-03) | 218-219 | H/L | wrong variable (log) | `printf("Fishery parameter: %s = %e\n", FisheryArray[fisheryIndex].fisheryCode, value);` prints the fishery co… |  | [ ] |

---

## UTL2-01

**buffer overrun** · Conf M / Sev M  
**Lines (current code):** 461-473  
**Lines at review (Oct 2026):** 461-473

**Function:** `Util_XML_Read_Change_Values()`

**Why it's thought to be a bug**

`counter = (int) bm->FISHERYprms[fisheryIndex][paramID]; ... array[fisheryIndex][b][index] = values[b];` for b < counter, no check against the allocated size. Callers size the array as 1 when the change flag is off: atharvest/atHarvestParamIO.c:860-864 `P_max_num_changes = 1; if (bm->flagchangep) ... = max+1; Pchange = Util_Alloc_Init_3D_Double(4, P_max_num_changes, K_num_fisheries)` (=> [nf][P_max][4]). The same applies to SWEPTchange at :873-880. If a fishery has `_p_changes`/`_swept_changes` >= 2 while flagchangep/flagchangeswept = 0, the writes go past `[nf][0]` and corrupt the heap. Function: `Util_XML_Read_Change_Values()`.

**Current code** (`atlantisUtil/atUtilFisheryXML.c` lines 459–475)

```c
  459: 			quit("Util_XML_Read_Change_Values attributeNode not found %s\n", errorString);
  460: 		}
  461: 		for (fisheryIndex = 0; fisheryIndex < bm->K_num_fisheries; fisheryIndex++) {
  462: 
  463: 			counter = (int) bm->FISHERYprms[fisheryIndex][paramID];
  464: 
  465: 			if (Util_XML_Read_Array_Double(ATLANTIS_FISHERY_ATTRIBUTE, fileName, errorString, groupNode, no_checking, FisheryArray[fisheryIndex].fisheryCode, &values,
  466: 					counter) == FALSE) {
  467: 				quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FisheryArray[fisheryIndex].fisheryCode, fileName);
  468: 			}
  469: 
  470: 			for (b = 0; b < counter; b++) {
  471: 				array[fisheryIndex][b][index] = values[b];
  472: 			}
  473: 			free(values);
  474: 		}
  475: 	}
```

**Suggested fix (from review):** Pass the allocated max into the reader and quit if counter > max (or always size the array from the max param, whatever the flag).

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

## UTL2-02

**buffer overrun** · Conf M / Sev M  
**Lines (current code):** 504-521  
**Lines at review (Oct 2026):** 504-521

**Function:** `Util_XML_Read_Change_Fished_Groups()`

**Why it's thought to be a bug**

Same pattern for groups: `for (b = 0; b < bm->SP_FISHERYprms[sp][i][paramID]; b++) array[sp][i][b][index] = values[b];`. Qchange is allocated with `Q_max_num_changes = 1` unless `bm->flagchangeq` (atharvest/atHarvestParamIO.c:905-908). If `_q_changes` > 1 while the flag is off, the write overflows the [sp][nf][1][3] allocation. TACchange (atimplementation/atImplementationParamIO.c:35) should be checked the same way. Function: `Util_XML_Read_Change_Fished_Groups()`.

**Current code** (`atlantisUtil/atUtilFisheryXML.c` lines 502–523)

```c
  502: 		}
  503: 
  504: 		for (speciesIndex = 0; speciesIndex < bm->K_num_tot_sp; speciesIndex++) {
  505: 
  506: 			if (FunctGroupArray[speciesIndex].isImpacted == TRUE) {
  507: 				counter = 0;
  508:                 for (i = 0; i < bm->K_num_fisheries; i++) {
  509:                     counter = max ( counter,(int) bm->SP_FISHERYprms[speciesIndex][i][paramID] );
  510:                 }
  511: 
  512: 				if(counter > 0){
  513: 					if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, groupNode, check, FunctGroupArray[speciesIndex].groupCode, &values,
  514: 							counter) == FALSE) {
  515: 						quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FunctGroupArray[speciesIndex].groupCode, fileName);
  516: 					}
  517: 
  518: 					for (i = 0; i < bm->K_num_fisheries; i++)
  519: 						for (b = 0; b < bm->SP_FISHERYprms[speciesIndex][i][paramID]; b++)
  520: 							array[speciesIndex][i][b][index] = values[b];
  521: 
  522: 					free(values);
  523: 				}
```

**Suggested fix (from review):** Bound b by the allocated max_num_changes, or quit when the count is larger.

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

## UTL2-03

**wrong variable (log)** · Conf H / Sev L  
**Lines (current code):** 218-219  
**Lines at review (Oct 2026):** 218-219

**Function:** `Get_Fisheries_Node_Value()`

**Why it's thought to be a bug**

`printf("Fishery parameter: %s = %e\n", FisheryArray[fisheryIndex].fisheryCode, value);` prints the fishery code but not the parameter tag, so the verbose log can't tell which parameter was read. Function: `Get_Fisheries_Node_Value()`.

**Current code** (`atlantisUtil/atUtilFisheryXML.c` lines 216–221)

```c
  216: 	}
  217: 
  218: 	if (verbose > 1)
  219: 		printf("Fishery parameter: %s = %e\n", FisheryArray[fisheryIndex].fisheryCode, value);
  220: 
  221: 	Check_Value(FisheryArray[fisheryIndex].fisheryCode, value, paramStruct[paramID].checkType, paramStruct[paramID].tag);
```

**Suggested fix (from review):** Print `paramStruct[paramID].tag` as well.

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

atUtilFisheryXML.c:22/134: `numFisheryParams` = 79 but there are 78 initialisers, and `numGroupFisheryParams` = 44 but there are 42. The trailing entries are zero-filled (NULL tag, paramID 0, i.e. `sel_id` for the group enum). `Util_XML_Get_Fishery_Param_Index()` returns the first match, so it is harmless today, but any loop over the full array that dereferences `.tag`/`.regEx` would crash. A lookup of an unlisted enum value equal to 0 would return the empty entry instead of quitting.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atUtilFisheryXML.c:513-520: every fishery i gets the same `values[0..n_i)` vector for a group (the read size is the max over fisheries), so per-fishery change schedules can't differ. This may be the intended file format.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-3

atUtilFisheryXML.c:269-331: error strings say "readFisheryGroupParamXML", and the `_Double` header comment says "integer data" (cosmetic).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
