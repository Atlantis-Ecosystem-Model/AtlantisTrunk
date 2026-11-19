# atimplementation/atImplementationParamIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [HRV2-25](#hrv2-25) ⭐ | 31, 35, 38 | H/H | wrong function / wrong array -> heap overflow | `TAC_num_changes_id` is a species-fishery parameter: it is in the SP enum (atlantisboxmodel.h:985), read by Ut… |  | [ ] |
| [HRV2-26](#hrv2-26) | 50-52 | M/L | NULL dereference | `inputDoc = xmlReadFileDestFolder(...)` is not checked before `inputDoc->children`, so it crashes if the conve… |  | [ ] |
| [HRV2-27](#hrv2-27) | 25-27 | L/L | copy-paste (node name) | TAC changes are read from a node called "Fishing_Catchability_Change", and ConvertAtlantis/atImplementationtoX… |  | [ ] |

---

## HRV2-25

**wrong function / wrong array -> heap overflow** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 31, 35, 38  
**Lines at review (Oct 2026):** 31, 35, 38

**Function:** `readManagementImplementationChangeXML()`

**Why it's thought to be a bug**

`TAC_num_changes_id` is a species-fishery parameter: it is in the SP enum (atlantisboxmodel.h:985), read by Util_XML_Read_Impacted_Group_Param, and used as `SP_FISHERYprms[sp][nf][TAC_num_changes_id]` (atImplementationAnnual.c:338). But `Manage_Get_Max_Fishery_Param()` reads `bm->FISHERYprms[i][paramIndex]` (atManage.c:2633), so the max comes from an unrelated per-fishery parameter. TACchange is then allocated with that change count. `Util_XML_Read_Change_Fished_Groups` writes `array[sp][i][b]` for `b < SP_FISHERYprms[sp][i][TAC_num_changes_id]` (atUtilFisheryXML.c:519-521), which overflows when a species has more TAC changes than that bogus max. Function: `readManagementImplementationChangeXML()`.

**Current code** (`atimplementation/atImplementationParamIO.c` lines 29–40)

```c
   29: 
   30: 	Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, TAC_num_changes_id);
   31: 	TAC_max_num_changes = Manage_Get_Max_Fishery_Param(bm, TAC_num_changes_id ) + 1;
   32: 
   33: 
   34: 	/** Allocate the storage to hold the information on the TAC change */
   35: 	TACchange = Util_Alloc_Init_4D_Double ( 3, TAC_max_num_changes, bm->K_num_fisheries, bm->K_num_tot_sp, 0.0 );
   36: 
   37: 	/* Now load the data */
   38: 	Util_XML_Read_Change_Fished_Groups(bm, fileName, groupingNode, "TACchange", TAC_num_changes_id, TACchange);
   39: }
   40: 
```

**Suggested fix (from review):** `TAC_max_num_changes = Manage_Get_Max_Species_Fishery_Param(bm, TAC_num_changes_id) + 1;`

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

## HRV2-26

**NULL dereference** · Conf M / Sev L  
**Lines (current code):** 50-52  
**Lines at review (Oct 2026):** 50-52

**Function:** `readModelManageImplParameters()`

**Why it's thought to be a bug**

`inputDoc = xmlReadFileDestFolder(...)` is not checked before `inputDoc->children`, so it crashes if the converted XML is missing or malformed. Function: `readModelManageImplParameters()`.

**Current code** (`atimplementation/atImplementationParamIO.c` lines 48–54)

```c
   48: int readModelManageImplParameters(MSEBoxModel *bm, char *filename) {
   49: 	xmlDocPtr inputDoc;
   50: 	inputDoc = xmlReadFileDestFolder(bm->destFolder, filename, NULL, 0);
   51: 
   52: 	readManagementImplementationChangeXML(bm, filename, inputDoc->children);
   53: 	xmlFreeDoc(inputDoc);
   54: 	/* Shutdown libxml */
```

**Suggested fix (from review):** `if (inputDoc == NULL) quit(...)`

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

## HRV2-27

**copy-paste (node name)** · Conf L / Sev L  
**Lines (current code):** 25-27  
**Lines at review (Oct 2026):** 25-27

**Why it's thought to be a bug**

TAC changes are read from a node called "Fishing_Catchability_Change", and ConvertAtlantis/atImplementationtoXML.c:29,33 labels them "catchability". The reader and writer agree, so it works, but the name and description are copy-paste from the q-change code and misleading.

**Current code** (`atimplementation/atImplementationParamIO.c` lines 23–29)

```c
   23: 	xmlNodePtr groupingNode;
   24: 	int TAC_max_num_changes;
   25: 	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Fishing_Catchability_Change");
   26: 	if(groupingNode == NULL){
   27: 		quit("readManagementImplementationChangeXML node 'Fishing_Catchability_Change' not found.\n");
   28: 	}
   29: 
```

**Suggested fix (from review):** Rename consistently in both (optional)

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
