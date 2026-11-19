# atharvest/atHarvestParamIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 1; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [HARV-08](#harv-08) | 931 | M/H | buffer overrun | The change arrays are allocated with max size 1 unless the global flag (`flagchangesel`, `flagchangep`, `flagc… |  | [ ] |
| [HARV-09](#harv-09) | 696 | M/L | copy-paste duplicate | `Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, id);` is called twice (lines 696 and 698), so … |  | [ ] |
| [HARV-10](#harv-10) | 933 | M/L | NULL dereference | Unlike every other reader in this file, `attributeGroupNode` (`Fishing_Scenario_Flags`), `groupingNode`/`selGr… |  | [ ] |

---

## HARV-08

**buffer overrun** · Conf M / Sev H  
**Found at (current code):** `atHarvestParamIO.c` 931
**Lines at review (Oct 2026):** 841-845, 860-866, 874-880, 887-893, 905-910

**Function:** `readFisheryChangeXML()`

**Why it's thought to be a bug**

The change arrays are allocated with max size 1 unless the global flag (`flagchangesel`, `flagchangep`, `flagchangeswept`, `flagchangediscard`, `flagchangeq`) is set. The readers (`readSelChangeXML` line 394, `Util_XML_Read_Change_Values`, `readDiscardChangeXML` line 438, `Util_XML_Read_Change_Fished_Groups`) still write `num_changes` entries for each fishery/group. If any `*_num_changes > 1` while the global flag is 0, this is a heap overflow. The fix-up that turns on global flags from per-fishery flags (atHarvestSetup.c:111-129) runs after `Read_Harvest_Parameters` (atHarvestSetup.c:94), so it is too late. Function: `readFisheryChangeXML()`.

**Code at issue** — `atHarvestParamIO.c` line 931 (located by name only; check this is the right place)  
Search for: `int Read_Harvest_Parameters(MSEBoxModel *bm, char *filename) {`

```c
  929:  *
  930:  */
  931→ int Read_Harvest_Parameters(MSEBoxModel *bm, char *filename) {
  932: 	xmlDocPtr inputDoc;
  933: 	inputDoc = xmlReadFileDestFolder(bm->destFolder, filename, NULL, 0);
```

**Suggested fix (from review):** Always size by `Manage_Get_Max_*_Param(...) + 1`, as is already done for mFC at line 917.

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

## HARV-09

**copy-paste duplicate** · Conf M / Sev L  
**Found at (current code):** `atHarvestParamIO.c` 696
**Lines at review (Oct 2026):** 696-699

**Function:** `readFishingMortalityXML()`

**Why it's thought to be a bug**

`Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, id);` is called twice (lines 696 and 698), so mFC is read twice. Harmless, but probably meant to read something else. Function: `readFishingMortalityXML()`.

**Code at issue** — `atHarvestParamIO.c` line 696  
Search for: `Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, id)`

```c
  694: 		quit("Fishing_Mortality attribute group not found in input file %s.\n", fileName);
  695: 
  696→ 	Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, id);
  697: 	readAgeStructuredBiomassFisheryParamXML(bm, fileName, groupingNode, mFC_start_age_id);
  698:     Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, id);
```

**Suggested fix (from review):** Remove the second call.

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

## HARV-10

**NULL dereference** · Conf M / Sev L  
**Found at (current code):** `atHarvestParamIO.c` 933
**Lines at review (Oct 2026):** 583-585, 831-835, 933-935

**Why it's thought to be a bug**

Unlike every other reader in this file, `attributeGroupNode` (`Fishing_Scenario_Flags`), `groupingNode`/`selGroupNode` (`Fishery_Change` and its children) and `inputDoc` (from `xmlReadFileDestFolder`) are not NULL-checked. A missing XML section or file crashes instead of giving a clear `quit`. Functions: `readFisheryScenarioFlags()`, `readFisheryChangeXML()`, `Read_Harvest_Parameters()`.

**Code at issue** — `atHarvestParamIO.c` line 933 (the line the review pointed to, found by its text)  
Search for: `inputDoc = xmlReadFileDestFolder(bm->destFolder, filename, NULL, 0);`

```c
  931: int Read_Harvest_Parameters(MSEBoxModel *bm, char *filename) {
  932: 	xmlDocPtr inputDoc;
  933→ 	inputDoc = xmlReadFileDestFolder(bm->destFolder, filename, NULL, 0);
  934: 
  935: 	readFishingFlagValues(bm, filename, inputDoc->children);
```

**Suggested fix (from review):** Add `if (x == NULL) quit(...)`.

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
