# ConvertAtlantis/atManagetoXML.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **5 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [CNV2-01](#cnv2-01) | 911 | H/M | wrong regex / key mismatch | `"^[A-Z]{2,3}_co_sp_catch[1-9]{1,2}"` requires a trailing digit and exactly 2-3 uppercase letters for the grou… |  | [ ] |
| [CNV2-02](#cnv2-02) | 713 | H/M | parsing / wrong index | `PortXMLFunction()` treats the text after the last `_` as the fishery code. Fishery codes that contain `_` (e.… |  | [ ] |
| [CNV2-03](#cnv2-03) | 49 | H/L | logic (latent) | `if (flag) strcat(newValueStr, " "); flag = FALSE;` adds a separator only after the first token, so later toke… |  | [ ] |
| [CNV2-04](#cnv2-04) | 113, 105, 122 | M/L | NULL deref / overflow | `strcpy(groupStr, strstr(indexStr, "_") + 1)` dereferences NULL+1 if the key has no `_` after the region numbe… |  | [ ] |
| [CNV2-05](#cnv2-05) | 507, 742 | H/L | copy-paste metadata | Descriptions copied without editing. `flagcap_peak_id` uses the same text as `flagcap_id`. `flagchangeEFF_id` … |  | [ ] |

---

## CNV2-01

**wrong regex / key mismatch** · Conf H / Sev M  
**Found at (current code):** `atManagetoXML.c` 911
**Lines at review (Oct 2026):** 911 (with 176-211)

**Function:** `TACXML()`

**Why it's thought to be a bug**

`"^[A-Z]{2,3}_co_sp_catch[1-9]{1,2}"` requires a trailing digit and exactly 2-3 uppercase letters for the group code. `Co_Sp_CatchXMLFunction()` handles keys with no digit ("no digits = companion 1", line 200-202), but the regex never passes those keys to it. Real prm files do use bare keys (e.g. `BB_co_sp_catch`, `BC_co_sp_catch` in /Users/ful083/runs). Four-character codes are dropped too. The `companionN` nodes then stay empty, and `Read_Fishery_Companion_Catch_XML()` (atManageParamIO.c:299) fails for them. Function: `TACXML()`.

**Code at issue** — `atManagetoXML.c` line 911  
Search for: `"^[A-Z]{2,3}_co_sp_catch[1-9]{1,2}"`

```c
  909:         Create_Species_ParamXML(bm, fileName, fp, groupingNode, coType_id, "Type of companion TAC (0 = weakest link dictates quota, 1 = strongest link dictates quota)", "", XML_TYPE_BOOLEAN,"1");
  910:         
  911→         Parse_File(bm, fp, fileName, groupingNode, "co_sp_catch", "^[A-Z]{2,3}_co_sp_catch[1-9]{1,2}","Ratio of catch of companions in companion TAC - for all entries must have as many entries as there are fisheries. \nOne vector is required per companion species.", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE, Init_Co_Sp_CatchXML, Co_Sp_CatchXMLFunction);
  912: 
  913:     }
```

**Suggested fix (from review):** Use regex `^[A-Za-z0-9]+_co_sp_catch[0-9]*$`

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

## CNV2-02

**parsing / wrong index** · Conf H / Sev M  
**Found at (current code):** `atManagetoXML.c` 713
**Lines at review (Oct 2026):** 692-698

**Function:** `PortXMLFunction()`

**Why it's thought to be a bug**

`PortXMLFunction()` treats the text after the last `_` as the fishery code. Fishery codes that contain `_` (e.g. `Astoria_Generic`, `Com_boat`, which exist in /Users/ful083/runs prm files) resolve to `Generic`/`boat`. `Util_Get_Fishery_Index()` then quits (atUtil.c:228), or maps to the wrong fishery if a fishery with that name exists. Function: `PortXMLFunction()`.

**Code at issue** — `atManagetoXML.c` line 713 (located by name only; check this is the right place)  
Search for: `"^ports_.*",`

```c
  711: 			groupingNode,
  712: 			"FisheryPort",
  713→ 			"^ports_.*",
  714: 			"Vectors indicating which ports each fishery can operate from initially (other ports may be activated using the flagnewport option in the switch and flag section at the top of the file). \n1 = from the port, 0 = not active from the port. \nThe list of ports are listed at the top of the parameter file - there must be as many entries as there are ports (K_num_ports)",
  715: 			"", XML_TYPE_BOOLEANARRAY, bm->K_num_ports, TRUE, Create_Fishery_XMLNodes, PortXMLFunction);
```

**Suggested fix (from review):** Strip the known `ports_` prefix (or match against FisheryArray codes) instead of taking the last `_`

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

## CNV2-03

**logic (latent)** · Conf H / Sev L  
**Found at (current code):** `atManagetoXML.c` 49
**Lines at review (Oct 2026):** 46-56

**Function:** `Fix_Not_Fished_Species_Last_XMLFunction()`

**Why it's thought to be a bug**

`if (flag) strcat(newValueStr, " "); flag = FALSE;` adds a separator only after the first token, so later tokens run together (`"1 2345"`). It also hard-codes `45` as the not-fished id (the comment itself says this is worrying). The function is currently only referenced from commented-out code (line 905), so this is latent. Function: `Fix_Not_Fished_Species_Last_XMLFunction()`.

**Code at issue** — `atManagetoXML.c` line 49 (the line the review pointed to, found by its text)  
Search for: `strcat(newValueStr, "-1");`

```c
   47: 		strPtr = NULL;
   48: 		if (atoi(varstr) == 45) /* Very worring as the Not_fished_id value should be 51 */
   49→ 			strcat(newValueStr, "-1");
   50: 		else
   51: 			strcat(newValueStr, varstr);
```

**Suggested fix (from review):** Always append `" "` between tokens and use the real `Not_fished_id`, or delete the function

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

## CNV2-04

**NULL deref / overflow** · Conf M / Sev L  
**Found at (current code):** `atManagetoXML.c` 113, `atManagetoXML.c` 105, `atManagetoXML.c` 122
**Lines at review (Oct 2026):** 112-127

**Function:** `BiMonthlyXML()`

**Why it's thought to be a bug**

`strcpy(groupStr, strstr(indexStr, "_") + 1)` dereferences NULL+1 if the key has no `_` after the region number. It also copies into `groupStr[MAX_GROUPCODE_LENGTH]` without a bound. The error text says `FBoMonthlyXML`, and `index < 0` is checked after it has already been used. Function: `BiMonthlyXML()`.

**Code at issue** — `atManagetoXML.c` line 113  
Search for: `strcpy(groupStr, strstr(indexStr, "_") + 1)`

```c
  111: 
  112: 	indexStr = str + strlen("BiTAC_Reg");
  113→ 	strcpy(groupStr, strstr(indexStr, "_") + 1);
  114: 	*(strstr(indexStr, "_")) = '\0';
  115: 
```

**Code at issue** — `atManagetoXML.c` line 105  
Search for: `groupStr[MAX_GROUPCODE_LENGTH]`

```c
  103: void BiMonthlyXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {
  104: 
  105→ 	char groupStr[MAX_GROUPCODE_LENGTH];
  106: 	char *indexStr;
  107: 	int index;
```

**Code at issue** — `atManagetoXML.c` line 122  
Search for: `index < 0`

```c
  120: 
  121: 
  122→ 	if (index < 0)
  123: 		quit("FBoMonthlyXML - Failed to get the index value %s\n", indexStr);
  124: 
```

**Suggested fix (from review):** Check the `strstr` result and use a bounded copy

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

## CNV2-05

**copy-paste metadata** · Conf H / Sev L  
**Found at (current code):** `atManagetoXML.c` 507, `atManagetoXML.c` 742
**Lines at review (Oct 2026):** 507, 590, 742, 749, 788

**Why it's thought to be a bug**

Descriptions copied without editing. `flagcap_peak_id` uses the same text as `flagcap_id`. `flagchangeEFF_id` says "0=no, 0=yes". `targ_refE`/`forage_refE` say "tier4" (should be tier5). `Fref_High` uses the same text as `Fref`. This affects comments in the generated XML only. Functions: `ManamentFlagTimeXML()`, `ManagementSenarioFlagsXML()`, `TACXML()`.

**Code at issue** — `atManagetoXML.c` line 507 (the line the review pointed to, found by its text)  
Search for: `Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, flagcap_peak_id, "Flags indicating`

```c
  505: 			XML_TYPE_BOOLEAN, "0");
  506: 	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, flagcap_id, "Flags indicating whether effort caps are used. 0=none, 1=yes.", "", XML_TYPE_BOOLEAN,"0");
  507→ 	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, flagcap_peak_id, "Flags indicating whether effort caps are used. 0=none, 1=yes..", "", XML_TYPE_BOOLEAN,"0");
  508: 	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, flaguseall_id,
  509: 			"Flags indicating whether use all species caught to define CPUE for CPUE based effort models (1=yes) or not (0=use target species only).", "",
```

**Code at issue** — `atManagetoXML.c` line 742 (the line the review pointed to, found by its text)  
Search for: `Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "targ_refE", "Target reference`

```c
  740: 	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "targ_refC", "Target reference point - tier3", "", XML_TYPE_FLOAT, "");
  741: 	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "targ_refD", "Target reference point - tier4", "", XML_TYPE_FLOAT, "");
  742→     Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "targ_refE", "Target reference point - tier4", "", XML_TYPE_FLOAT, "");
  743: 	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "lim_ref", "Limit reference point", "", XML_TYPE_FLOAT, "");
  744: 
```

**Suggested fix (from review):** Correct the description strings

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
