# ConvertAtlantis/atHarvesttoXML.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **5 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [CNV2-10](#cnv2-10) | 47-54 | H/M | parsing / wrong index | `Fishery_Last_XMLFunction()` takes the text after the last `_` as the fishery code. Real models have fishery c… |  | [ ] |
| [CNV2-11](#cnv2-11) | 330 | H/L | copy-paste | The lookup for option 4 ("Conditional on catch of other species") is created with value `"2"`, which duplicate… |  | [ ] |
| [CNV2-12](#cnv2-12) | 312, 469 | H/L | wrong type metadata | `selcurve_id` is typed `XML_TYPE_BOOLEAN` but has 7 lookup values (0-6). `k_mismatch` is typed `XML_TYPE_BOOLE… |  | [ ] |
| [CNV2-13](#cnv2-13) | 167-185 | M/L | logic / dead code | `Incidental_MortalityXML()` builds `newValueStr` from the prm value and then ignores it, writing `"0.0"` for e… |  | [ ] |
| [CNV2-14](#cnv2-14) | 483, 653 | M/L | inconsistent length | `CatchAgeDistributions` expects length `K_num_max_cohort*K_num_max_genetypes`, `DiscardAgeDistributions` expec… |  | [ ] |

---

## CNV2-10

**parsing / wrong index** · Conf H / Sev M  
**Lines (current code):** 47-54  
**Lines at review (Oct 2026):** 47-54

**Function:** `Fishery_Last_XMLFunction()`

**Why it's thought to be a bug**

`Fishery_Last_XMLFunction()` takes the text after the last `_` as the fishery code. Real models have fishery codes with underscores (`EffortChange_start_Astoria_Generic`, `..._Com_boat` in /Users/ful083/runs), which resolve to `Generic`/`boat`. `Util_Get_Fishery_Index()` then quits or picks the wrong fishery. This affects every caller: mEff_, habitat_, SELchange_*, EffortChange_*, Effort_mult_, among others. Function: `Fishery_Last_XMLFunction()`.

**Current code** (`ConvertAtlantis/atHarvesttoXML.c` lines 45–56)

```c
   45: 	int fisheryIndex;
   46: 
   47: 	fisheryStr = strchr(str, '_');
   48: 	fisheryStr++;
   49: 	while (strchr(fisheryStr, '_') != NULL) {
   50: 		fisheryStr = strchr(fisheryStr, '_');
   51: 		fisheryStr++;
   52: 	}
   53: 	fisheryIndex = Util_Get_Fishery_Index(bm, fisheryStr);
   54: 	Util_XML_Set_Node_Value(ATLANTIS_FISHERY_ATTRIBUTE, parent, FisheryArray[fisheryIndex].fisheryCode, valueStr);
   55: }
   56: 
```

**Suggested fix (from review):** Strip the known parameter prefix (pass it in) instead of using the last `_`

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

## CNV2-11

**copy-paste** · Conf H / Sev L  
**Lines (current code):** 330  
**Lines at review (Oct 2026):** 330

**Function:** `FisheryFlagXML()`

**Why it's thought to be a bug**

The lookup for option 4 ("Conditional on catch of other species") is created with value `"2"`, which duplicates the "Size based" entry. Function: `FisheryFlagXML()`.

**Current code** (`ConvertAtlantis/atHarvesttoXML.c` lines 328–332)

```c
  328: 	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "Size based");
  329: 	Util_XML_Create_Lookup_Value_Node(lookupNode, "3", "Loaded from timeseries file");
  330: 	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "Conditional on catch of other species");
  331: 
  332: 	node = Create_Harvest_Fishery_Group_ParamXML(bm, fp, fileName, groupingNode, flagescapement_id,
```

**Suggested fix (from review):** Change `"2"` to `"4"`

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

## CNV2-12

**wrong type metadata** · Conf H / Sev L  
**Lines (current code):** 312, 469  
**Lines at review (Oct 2026):** 312, 469

**Why it's thought to be a bug**

`selcurve_id` is typed `XML_TYPE_BOOLEAN` but has 7 lookup values (0-6). `k_mismatch` is typed `XML_TYPE_BOOLEAN` but is a real number (recommended 0.25; the reader uses a double at atHarvestParamIO.c:602). Functions: `FisheryFlagXML()`, `FisheryScenarioFlags()`.

**Current code** (`ConvertAtlantis/atHarvesttoXML.c` lines 310–314)

```c
  310: 	node = Create_Fishery_ParamXML( bm, fp, fileName, groupingNode, selcurve_id,
  311: 					"Flag indicating what form of selectivity a fishery starts with (this can change through time if the appropriate changeSEL is set to 1 below). \n0=groups specific constant, \n1=cohort specific constant for each group, \n2=size based logistic curve, \n3=size based normal curve, \n4=size based lognormal curve, \n5=size based gamma curve, \n6=knife edged",
  312: 					"", XML_TYPE_BOOLEAN,"0");
  313: 	lookupNode = Util_XML_Create_Lookup_Node(node);
  314: 	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "Groups specific constant");
```

**Suggested fix (from review):** Use `XML_TYPE_LOOKUP` / `XML_TYPE_FLOAT`

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

## CNV2-13

**logic / dead code** · Conf M / Sev L  
**Lines (current code):** 167-185  
**Lines at review (Oct 2026):** 167-185

**Why it's thought to be a bug**

`Incidental_MortalityXML()` builds `newValueStr` from the prm value and then ignores it, writing `"0.0"` for every group. The `IncidentalMortality` node is never read (no reader match), so the prm value is silently lost.

**Current code** (`ConvertAtlantis/atHarvesttoXML.c` lines 165–187)

```c
  165: }
  166: 
  167: static void Incidental_MortalityXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {
  168: 	int guild;
  169: 	char newValueStr[5000];
  170: 	int fisheryIndex;
  171: 
  172: 	strcpy(newValueStr, "");
  173: 
  174: 	for (fisheryIndex = 0; fisheryIndex < bm->K_num_fisheries; fisheryIndex++) {
  175: 		strcat(newValueStr, " ");
  176: 		strcat(newValueStr, valueStr);
  177: 	}
  178: 
  179: 	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
  180: 		if (FunctGroupArray[guild].isImpacted == TRUE) {
  181: 			//Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, newValueStr);
  182: 			Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "0.0");
  183: 		}
  184: 	}
  185: }
  186: 
  187: 
```

**Suggested fix (from review):** Use `valueStr` or remove the block

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

## CNV2-14

**inconsistent length** · Conf M / Sev L  
**Lines (current code):** 483, 653  
**Lines at review (Oct 2026):** 483, 653

**Function:** `FisheryScenarioFlags()`

**Why it's thought to be a bug**

`CatchAgeDistributions` expects length `K_num_max_cohort*K_num_max_genetypes`, `DiscardAgeDistributions` expects `K_num_max_cohort`, and the reader reads `numCohorts` values (atHarvestParamIO.c:148). When there is more than one genotype, Parse_File warns about a length mismatch for every catch key. Function: `FisheryScenarioFlags()`.

**Current code** (`ConvertAtlantis/atHarvesttoXML.c` lines 481–485)

```c
  481: 
  482: 	Parse_File(bm, fp, fileName, groupingNode, "CatchAgeDistributions", "^CatchTS_agedistrib",
  483: 			"Catch by age distribution for conditioning time series of catch (if used)", "", XML_TYPE_FLOATARRAY, bm->K_num_max_cohort * bm->K_num_max_genetypes, TRUE,
  484: 			Init_Age_Structured_SpeciesXML, Age_Catch_DistributionXML);
  485: 
```

**Suggested fix (from review):** Use `K_num_max_cohort` for both

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

`Group_Catch_Selectivity_XMLFunction()` (atHarvesttoXML.c:203-217) copies the key into `char tempStr[50]` without a bound. Also, `strstr(strPtr,"ad")` is unchecked; it is safe only because the regex `^sel_.*[dv]$` guarantees one of "juv"/"ad".

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
