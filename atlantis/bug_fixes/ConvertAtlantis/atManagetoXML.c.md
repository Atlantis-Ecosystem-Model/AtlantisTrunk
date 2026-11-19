# ConvertAtlantis/atManagetoXML.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **5 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [CNV2-01](#cnv2-01) | 911 (with 176-211) | H/M | wrong regex / key mismatch | `"^[A-Z]{2,3}_co_sp_catch[1-9]{1,2}"` requires a trailing digit and exactly 2-3 uppercase letters for the grou… |  | [ ] |
| [CNV2-02](#cnv2-02) | 692-698 | H/M | parsing / wrong index | `PortXMLFunction()` treats the text after the last `_` as the fishery code. Fishery codes that contain `_` (e.… |  | [ ] |
| [CNV2-03](#cnv2-03) | 46-56 | H/L | logic (latent) | `if (flag) strcat(newValueStr, " "); flag = FALSE;` adds a separator only after the first token, so later toke… |  | [ ] |
| [CNV2-04](#cnv2-04) | 112-127 | M/L | NULL deref / overflow | `strcpy(groupStr, strstr(indexStr, "_") + 1)` dereferences NULL+1 if the key has no `_` after the region numbe… |  | [ ] |
| [CNV2-05](#cnv2-05) | 507, 590, 742, 749, 788 | H/L | copy-paste metadata | Descriptions copied without editing. `flagcap_peak_id` uses the same text as `flagcap_id`. `flagchangeEFF_id` … |  | [ ] |

---

## CNV2-01

**wrong regex / key mismatch** · Conf H / Sev M  
**Listed lines:** 911 (with 176-211)

**Function:** `TACXML()`

**Why it's thought to be a bug**

`"^[A-Z]{2,3}_co_sp_catch[1-9]{1,2}"` requires a trailing digit and exactly 2-3 uppercase letters for the group code. `Co_Sp_CatchXMLFunction()` handles keys with no digit ("no digits = companion 1", line 200-202), but the regex never passes those keys to it. Real prm files do use bare keys (e.g. `BB_co_sp_catch`, `BC_co_sp_catch` in /Users/ful083/runs). Four-character codes are dropped too. The `companionN` nodes then stay empty, and `Read_Fishery_Companion_Catch_XML()` (atManageParamIO.c:299) fails for them. Function: `TACXML()`.

**Current code** (`ConvertAtlantis/atManagetoXML.c` lines 909–913)

```c
  909:         Create_Species_ParamXML(bm, fileName, fp, groupingNode, coType_id, "Type of companion TAC (0 = weakest link dictates quota, 1 = strongest link dictates quota)", "", XML_TYPE_BOOLEAN,"1");
  910:         
  911:         Parse_File(bm, fp, fileName, groupingNode, "co_sp_catch", "^[A-Z]{2,3}_co_sp_catch[1-9]{1,2}","Ratio of catch of companions in companion TAC - for all entries must have as many entries as there are fisheries. \nOne vector is required per companion species.", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE, Init_Co_Sp_CatchXML, Co_Sp_CatchXMLFunction);
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
**Listed lines:** 692-698

**Function:** `PortXMLFunction()`

**Why it's thought to be a bug**

`PortXMLFunction()` treats the text after the last `_` as the fishery code. Fishery codes that contain `_` (e.g. `Astoria_Generic`, `Com_boat`, which exist in /Users/ful083/runs prm files) resolve to `Generic`/`boat`. `Util_Get_Fishery_Index()` then quits (atUtil.c:228), or maps to the wrong fishery if a fishery with that name exists. Function: `PortXMLFunction()`.

**Current code** (`ConvertAtlantis/atManagetoXML.c` lines 687–697) — code appears to have moved -3 lines since the review

```c
  687: 			strcmp(str, "ports_start") == 0 || strcmp(str, "ports_end") == 0 ||
  688: 			strcmp(str, "ports_pop") == 0	|| strcmp(str, "ports_markets") == 0
  689: 			|| strcmp(str, "ports_popchange") == 0 || strcmp(str, "ports_popnumchange") == 0 )
  690: 		return;
  691: 
  692: 	fisheryStr = strchr(str, '_');
  693: 	fisheryStr++;
  694: 	while (strchr(fisheryStr, '_') != NULL) {
  695: 		fisheryStr = strchr(fisheryStr, '_');
  696: 		fisheryStr++;
  697: 	}
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
**Listed lines:** 46-56

**Function:** `Fix_Not_Fished_Species_Last_XMLFunction()`

**Why it's thought to be a bug**

`if (flag) strcat(newValueStr, " "); flag = FALSE;` adds a separator only after the first token, so later tokens run together (`"1 2345"`). It also hard-codes `45` as the not-fished id (the comment itself says this is worrying). The function is currently only referenced from commented-out code (line 905), so this is latent. Function: `Fix_Not_Fished_Species_Last_XMLFunction()`.

**Current code** (`ConvertAtlantis/atManagetoXML.c` lines 44–58)

```c
   44: 	strPtr = valueStr;
   45: 
   46: 	while ((varstr = strtok(strPtr, seps)) != NULL) {
   47: 		strPtr = NULL;
   48: 		if (atoi(varstr) == 45) /* Very worring as the Not_fished_id value should be 51 */
   49: 			strcat(newValueStr, "-1");
   50: 		else
   51: 			strcat(newValueStr, varstr);
   52: 
   53: 		if (flag)
   54: 			strcat(newValueStr, " ");
   55: 		flag = FALSE;
   56: 	}
   57: 	Species_Last_XMLFunction(bm, fileName, parent, str, newValueStr);
   58: }
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
**Listed lines:** 112-127

**Function:** `BiMonthlyXML()`

**Why it's thought to be a bug**

`strcpy(groupStr, strstr(indexStr, "_") + 1)` dereferences NULL+1 if the key has no `_` after the region number. It also copies into `groupStr[MAX_GROUPCODE_LENGTH]` without a bound. The error text says `FBoMonthlyXML`, and `index < 0` is checked after it has already been used. Function: `BiMonthlyXML()`.

**Current code** (`ConvertAtlantis/atManagetoXML.c` lines 110–129)

```c
  110: 	char regionStr[STRLEN];
  111: 
  112: 	indexStr = str + strlen("BiTAC_Reg");
  113: 	strcpy(groupStr, strstr(indexStr, "_") + 1);
  114: 	*(strstr(indexStr, "_")) = '\0';
  115: 
  116: 	index = atoi(indexStr);
  117: 
  118: 	/* get the fisheryIndex */
  119: 	speciesIndex = Util_Get_FG_Index_From_Token(bm, groupStr, fileName, str, TRUE);
  120: 
  121: 
  122: 	if (index < 0)
  123: 		quit("FBoMonthlyXML - Failed to get the index value %s\n", indexStr);
  124: 
  125: 	speciesNode = Util_XML_Get_Or_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode);
  126: 	snprintf(regionStr, sizeof(regionStr), "region%d", index);
  127: 	Util_XML_Set_Node_Value(ATLANTIS_ATTRIBUTE, speciesNode, regionStr, valueStr);
  128: }
  129: 
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
**Listed lines:** 507, 590, 742, 749, 788

**Why it's thought to be a bug**

Descriptions copied without editing. `flagcap_peak_id` uses the same text as `flagcap_id`. `flagchangeEFF_id` says "0=no, 0=yes". `targ_refE`/`forage_refE` say "tier4" (should be tier5). `Fref_High` uses the same text as `Fref`. This affects comments in the generated XML only. Functions: `ManamentFlagTimeXML()`, `ManagementSenarioFlagsXML()`, `TACXML()`.

**Current code** (`ConvertAtlantis/atManagetoXML.c` lines 505–509)

```c
  505: 			XML_TYPE_BOOLEAN, "0");
  506: 	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, flagcap_id, "Flags indicating whether effort caps are used. 0=none, 1=yes.", "", XML_TYPE_BOOLEAN,"0");
  507: 	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, flagcap_peak_id, "Flags indicating whether effort caps are used. 0=none, 1=yes..", "", XML_TYPE_BOOLEAN,"0");
  508: 	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, flaguseall_id,
  509: 			"Flags indicating whether use all species caught to define CPUE for CPUE based effort models (1=yes) or not (0=use target species only).", "",
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
