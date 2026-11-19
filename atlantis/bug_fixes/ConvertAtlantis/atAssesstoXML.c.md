# ConvertAtlantis/atAssesstoXML.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1), plus 1 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [CNV2-06](#cnv2-06) ⭐ | 689-690 | H/H | wrong check type | `Num_Sexes` is declared `XML_TYPE_BOOLEAN` and read with `binary_check`. `Check_Value` quits for any value oth… |  | [ ] |
| [CNV2-07](#cnv2-07) | 549-554 | M/L | buffer overrun / NULL deref | `i = strcspn(speciesStr, "0-9"); strncpy(strPtr, speciesStr, i); strPtr[i] = '\0';` writes into `char strPtr[1… |  | [ ] |
| [CNV2-08](#cnv2-08) | 980, ≈1041 | M/L | duplicate parameter | `trigger_threshold_id` is created twice from the same prm keys, once under `Tier6` and once under `TIERsigma`,… |  | [ ] |
| [CNV2-09](#cnv2-09) | 247, 471, 777, 986-990 | H/L | copy-paste metadata | `flagfishbiom` is described as "detritus". `NumRAssessScriptNames` is a count but typed `XML_TYPE_BOOLEAN`. Fo… |  | [ ] |

---

## CNV2-06

**wrong check type** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 689-690  
**Lines at review (Oct 2026):** 689-690

**Function:** `TierAssessmentXML()`

**Why it's thought to be a bug**

`Num_Sexes` is declared `XML_TYPE_BOOLEAN` and read with `binary_check`. `Check_Value` quits for any value other than 0 or 1 (atUtilIO.c:171-175). A normal two-sex setup (`Num_Sexes 2`; sexStrings has F/M) therefore aborts the conversion whenever `useRBCTiers` is on. The reader has the same bug (atassessParamIO.c:971). Function: `TierAssessmentXML()`.

**Current code** (`ConvertAtlantis/atAssesstoXML.c` lines 687–692)

```c
  687: 	Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "assess_share_params", &bm->assess_share_params);
  688:     
  689:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Num_Sexes", "Number of sexes in the model.", "", XML_TYPE_BOOLEAN, "0");
  690: 	Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "Num_Sexes", &bm->K_num_sexes);
  691: 
  692:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "UseAtlantisPGMSY", "Flag indicating whether using Atlantis version of PGMSY (1) or R version (0)", "", XML_TYPE_BOOLEAN, "0");
```

**Suggested fix (from review):** Use `XML_TYPE_INTEGER` with `integer_check` here and in atassessParamIO.c:971

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

## CNV2-07

**buffer overrun / NULL deref** · Conf M / Sev L  
**Lines (current code):** 549-554  
**Lines at review (Oct 2026):** 549-554

**Function:** `AssessStockXMLFunction()`

**Why it's thought to be a bug**

`i = strcspn(speciesStr, "0-9"); strncpy(strPtr, speciesStr, i); strPtr[i] = '\0';` writes into `char strPtr[10]`. If a matching key has no digit before the parameter name, `i` is the full key length (overflow) and `sexnum` is NULL, so `atoi(NULL)` crashes. Group codes containing digits are also mis-split. Function: `AssessStockXMLFunction()`.

**Current code** (`ConvertAtlantis/atAssesstoXML.c` lines 547–556)

```c
  547:     
  548:     // Assumes string has the form FXXi_STRStockj where STR is the parameter name, i is the sex (either 0 or 1) and j is the stock number 1+
  549:     i = (int)strcspn (speciesStr, keys);  // location of first number
  550:     sexnum = strpbrk (speciesStr, keys);
  551:     ns = atoi(sexnum);
  552:     strncpy ( strPtr, speciesStr, (size_t)i);
  553: 	strPtr[i] = '\0';  // null character added manually
  554: 	speciesIndex = Util_Get_FG_Index_From_Token(bm, strPtr, fileName, str, TRUE);
  555: 
  556: 	if (strstr(str, "Mzero") != NULL) {
```

**Suggested fix (from review):** Bound `i` to `sizeof(strPtr)-1`, check `sexnum != NULL`, and split on `_`

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

## CNV2-08

**duplicate parameter** · Conf M / Sev L  
**Lines (current code):** 980, ≈1041  
**Lines at review (Oct 2026):** 975, 1037

**Function:** `TierAssessmentXML()`

**Why it's thought to be a bug**

`trigger_threshold_id` is created twice from the same prm keys, once under `Tier6` and once under `TIERsigma`, with different descriptions. The reader uses only `TIERsigma` (atassessParamIO.c:1561), so the `Tier6` copy is dead and confusing. Function: `TierAssessmentXML()`.

**Current code** (`ConvertAtlantis/atAssesstoXML.c` lines 978–982) — ≈ the code here has changed since the review; lines are approximate

```c
  978:     Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, TriggerResponseScen_id, "WHat to do if Tier 6 triggered", "", XML_TYPE_INTEGER, "0");
  979:     Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, trigger_threshold_id, "WHat to the proportional change for triggered action", "", XML_TYPE_FLOAT, "0");    
  980:     
  981:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "NumTriggers", "NumTriggers", "", XML_TYPE_INTEGER, "0");
  982: 	Util_XML_Create_Node_Next_Line(fp, fileName, groupingNode, "TriggerPoints", "Trigger points versus historical catch levels", "", XML_TYPE_FLOAT);
```

**Suggested fix (from review):** Remove one copy

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

## CNV2-09

**copy-paste metadata** · Conf H / Sev L  
**Lines (current code):** 247, 471, 777, 986-990  
**Lines at review (Oct 2026):** 247, 471, 772, 981-985

**Why it's thought to be a bug**

`flagfishbiom` is described as "detritus". `NumRAssessScriptNames` is a count but typed `XML_TYPE_BOOLEAN`. For `UseCloseKin`, `nTotSamps`, `first_sy`, `last_sy` and `first_dy`, the description and units arguments are swapped (the description goes into units), and `UseCloseKin` has recommended value `"4000"`. Functions: `ErrorStructureXML()`, `RAssessFlagXML()`, `TierAssessmentXML()`.

**Current code** (`ConvertAtlantis/atAssesstoXML.c` lines 245–249)

```c
  245: 			"Flag for setting error structure used for benthic infauna invertebrate biomass: 0 = uniform, 1 = normal, 2 = lognormal", "", XML_TYPE_INTEGER, "1");
  246: 	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagdetbiom", "Flag for setting error structure used for detritus: 0 = uniform, 1 = normal, 2 = lognormal", "", XML_TYPE_INTEGER, "1");
  247: 	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagfishbiom", "Flag for setting error structure used for detritus: 0 = uniform, 1 = normal, 2 = lognormal", "", XML_TYPE_INTEGER, "1");
  248: 	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagverts", "Flag for setting error structure used for vertebrate info: 0 = uniform, 1 = normal, 2 = lognormal", "", XML_TYPE_INTEGER,
  249: 			"1");
```

**Suggested fix (from review):** Fix the argument order, types and texts

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

The atAssesstoXML.c Parse_File regexes `"li_a"`, `"samplesize"`, `"R_max"`, `"q_lsm"` and `"selcurve"` are unanchored. They could match unrelated keys that contain those substrings. I found no collisions in the sample prm files.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
