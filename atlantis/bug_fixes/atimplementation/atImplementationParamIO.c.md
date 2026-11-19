# atimplementation/atImplementationParamIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [HRV2-25](#hrv2-25) ⭐ | 38 | H/H | wrong function / wrong array -> heap overflow | `TAC_num_changes_id` is a species-fishery parameter: it is in the SP enum (atlantisboxmodel.h:985), read by Ut… |  | [ ] |
| [HRV2-26](#hrv2-26) | 50 | M/L | NULL dereference | `inputDoc = xmlReadFileDestFolder(...)` is not checked before `inputDoc->children`, so it crashes if the conve… |  | [ ] |
| [HRV2-27](#hrv2-27) | 25 | L/L | copy-paste (node name) | TAC changes are read from a node called "Fishing_Catchability_Change", and ConvertAtlantis/atImplementationtoX… |  | [ ] |

---

## HRV2-25

**wrong function / wrong array -> heap overflow** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atImplementationParamIO.c` 38
**Lines at review (Oct 2026):** 31, 35, 38

**Function:** `readManagementImplementationChangeXML()`

**Why it's thought to be a bug**

`TAC_num_changes_id` is a species-fishery parameter: it is in the SP enum (atlantisboxmodel.h:985), read by Util_XML_Read_Impacted_Group_Param, and used as `SP_FISHERYprms[sp][nf][TAC_num_changes_id]` (atImplementationAnnual.c:338). But `Manage_Get_Max_Fishery_Param()` reads `bm->FISHERYprms[i][paramIndex]` (atManage.c:2633), so the max comes from an unrelated per-fishery parameter. TACchange is then allocated with that change count. `Util_XML_Read_Change_Fished_Groups` writes `array[sp][i][b]` for `b < SP_FISHERYprms[sp][i][TAC_num_changes_id]` (atUtilFisheryXML.c:519-521), which overflows when a species has more TAC changes than that bogus max. Function: `readManagementImplementationChangeXML()`.

**Code at issue** — `atImplementationParamIO.c` line 38 (the line the review pointed to, found by its text)  
Search for: `Util_XML_Read_Change_Fished_Groups(bm, fileName, groupingNode, "TACchange", TAC_num_change`

```c
   36: 
   37: 	/* Now load the data */
   38→ 	Util_XML_Read_Change_Fished_Groups(bm, fileName, groupingNode, "TACchange", TAC_num_changes_id, TACchange);
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
**Found at (current code):** `atImplementationParamIO.c` 50
**Lines at review (Oct 2026):** 50-52

**Function:** `readModelManageImplParameters()`

**Why it's thought to be a bug**

`inputDoc = xmlReadFileDestFolder(...)` is not checked before `inputDoc->children`, so it crashes if the converted XML is missing or malformed. Function: `readModelManageImplParameters()`.

**Code at issue** — `atImplementationParamIO.c` line 50  
Search for: `inputDoc = xmlReadFileDestFolder(`

```c
   48: int readModelManageImplParameters(MSEBoxModel *bm, char *filename) {
   49: 	xmlDocPtr inputDoc;
   50→ 	inputDoc = xmlReadFileDestFolder(bm->destFolder, filename, NULL, 0);
   51: 
   52: 	readManagementImplementationChangeXML(bm, filename, inputDoc->children);
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
**Found at (current code):** `atImplementationParamIO.c` 25
**Lines at review (Oct 2026):** 25-27

**Why it's thought to be a bug**

TAC changes are read from a node called "Fishing_Catchability_Change", and ConvertAtlantis/atImplementationtoXML.c:29,33 labels them "catchability". The reader and writer agree, so it works, but the name and description are copy-paste from the q-change code and misleading.

**Code at issue** — `atImplementationParamIO.c` line 25 (located by name only; check this is the right place)  
Search for: `groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Fishing_Catchabi`

```c
   23: 	xmlNodePtr groupingNode;
   24: 	int TAC_max_num_changes;
   25→ 	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Fishing_Catchability_Change");
   26: 	if(groupingNode == NULL){
   27: 		quit("readManagementImplementationChangeXML node 'Fishing_Catchability_Change' not found.\n");
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
