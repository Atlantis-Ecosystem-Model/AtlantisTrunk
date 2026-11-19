# ConvertAtlantis/atAssesstoXML.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [CNV2-06](#cnv2-06) ⭐ | 690 | H/H | wrong check type | `Num_Sexes` is declared `XML_TYPE_BOOLEAN` and read with `binary_check`. `Check_Value` quits for any value oth… |  | [ ] |
| [CNV2-07](#cnv2-07) | 531 | M/L | buffer overrun / NULL deref | `i = strcspn(speciesStr, "0-9"); strncpy(strPtr, speciesStr, i); strPtr[i] = '\0';` writes into `char strPtr[1… |  | [ ] |
| [CNV2-08](#cnv2-08) | 977 | M/L | duplicate parameter | `trigger_threshold_id` is created twice from the same prm keys, once under `Tier6` and once under `TIERsigma`,… |  | [ ] |
| [CNV2-09](#cnv2-09) | 985 | H/L | copy-paste metadata | `flagfishbiom` is described as "detritus". `NumRAssessScriptNames` is a count but typed `XML_TYPE_BOOLEAN`. Fo… |  | [ ] |

---

## CNV2-06

**wrong check type** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atAssesstoXML.c` 690
**Lines at review (Oct 2026):** 689-690

**Function:** `TierAssessmentXML()`

**Why it's thought to be a bug**

`Num_Sexes` is declared `XML_TYPE_BOOLEAN` and read with `binary_check`. `Check_Value` quits for any value other than 0 or 1 (atUtilIO.c:171-175). A normal two-sex setup (`Num_Sexes 2`; sexStrings has F/M) therefore aborts the conversion whenever `useRBCTiers` is on. The reader has the same bug (atassessParamIO.c:971). Function: `TierAssessmentXML()`.

**Code at issue** — `atAssesstoXML.c` line 690 (the line the review pointed to, found by its text)  
Search for: `Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, bin`

```c
  688:     
  689:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Num_Sexes", "Number of sexes in the model.", "", XML_TYPE_BOOLEAN, "0");
  690→ 	Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "Num_Sexes", &bm->K_num_sexes);
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
**Found at (current code):** `atAssesstoXML.c` 531
**Lines at review (Oct 2026):** 549-554

**Function:** `AssessStockXMLFunction()`

**Why it's thought to be a bug**

`i = strcspn(speciesStr, "0-9"); strncpy(strPtr, speciesStr, i); strPtr[i] = '\0';` writes into `char strPtr[10]`. If a matching key has no digit before the parameter name, `i` is the full key length (overflow) and `sexnum` is NULL, so `atoi(NULL)` crashes. Group codes containing digits are also mis-split. Function: `AssessStockXMLFunction()`.

**Code at issue** — `atAssesstoXML.c` line 531  
Search for: `char strPtr[10]`

```c
  529: 	char *secondlastLetter;
  530:     char *sexnum;
  531→ 	char strPtr[10];
  532: 	char speciesStr[STRLEN];
  533: 	char paramStr[STRLEN];
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
**Found at (current code):** `atAssesstoXML.c` 977
**Lines at review (Oct 2026):** 975, 1037

**Function:** `TierAssessmentXML()`

**Why it's thought to be a bug**

`trigger_threshold_id` is created twice from the same prm keys, once under `Tier6` and once under `TIERsigma`, with different descriptions. The reader uses only `TIERsigma` (atassessParamIO.c:1561), so the `Tier6` copy is dead and confusing. Function: `TierAssessmentXML()`.

**Code at issue** — `atAssesstoXML.c` line 977 (located by name only; check this is the right place)  
Search for: `childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Tier`

```c
  975:     
  976:     // tier 6
  977→     childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Tier6", "Tier6", "", "");
  978:     Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, TriggerResponseScen_id, "WHat to do if Tier 6 triggered", "", XML_TYPE_INTEGER, "0");
  979:     Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, trigger_threshold_id, "WHat to the proportional change for triggered action", "", XML_TYPE_FLOAT, "0");    
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
**Found at (current code):** `atAssesstoXML.c` 985
**Lines at review (Oct 2026):** 247, 471, 772, 981-985

**Why it's thought to be a bug**

`flagfishbiom` is described as "detritus". `NumRAssessScriptNames` is a count but typed `XML_TYPE_BOOLEAN`. For `UseCloseKin`, `nTotSamps`, `first_sy`, `last_sy` and `first_dy`, the description and units arguments are swapped (the description goes into units), and `UseCloseKin` has recommended value `"4000"`. Functions: `ErrorStructureXML()`, `RAssessFlagXML()`, `TierAssessmentXML()`.

**Code at issue** — `atAssesstoXML.c` line 985  
Search for: `"4000"`

```c
  983: 
  984:     // Close kin parameters
  985→     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "UseCloseKin", "UseCloseKin", "Whether using close kin", XML_TYPE_INTEGER, "4000");
  986:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "nTotSamps", "nTotSamps", "Sample size for close kin", XML_TYPE_INTEGER, "4000");
  987:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "first_sy", "first_sy", "First year of close kin data collection", XML_TYPE_INTEGER, "0");
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
