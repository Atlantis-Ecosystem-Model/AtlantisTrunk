# atCLAMLink/atCLAMLinkIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **8 findings** (severity H: 3; confidence H and severity H: 3; on the priority list: 3).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [LINK-08](#link-08) ⭐ | 542, 405 | H/H | wrong variable | `indicators[indicatorID].scenarioLinks[linkID] = scenarioID;` — `linkID` is initialised to 0 and never increme… |  | [ ] |
| [LINK-09](#link-09) ⭐ | 396 | H/H | uninitialised memory | `CLAMScenarioOptions = malloc(...)`: only the strategies present in the XML are filled, and `isLocked`/`timeLo… |  | [ ] |
| [LINK-10](#link-10) ⭐ | 124 | H/H | missing bounds check | `maxOptionValue`, `numStrategyOptions` and `numLevers` come from XML with no limit, but `strategies[]` is fixe… |  | [ ] |
| [LINK-11](#link-11) | 470 | H/M | NULL dereference | `printf("...", indicatorList->nodesetval->nodeNr); if (indicatorList && ...)` — dereferenced before the NULL c… |  | [ ] |
| [LINK-12](#link-12) | 565 | M/M | uninitialised / leak | `xmlXPathFreeObject(strategyList);` frees only the last per-scenario list (the others leak). If `scenarioList`… |  | [ ] |
| [LINK-13](#link-13) | 101 | H/L | format/arg mismatch | `quit("Lever name '%s' not recognised\n");` — `%s` with no argument (UB, garbage or crash in the error path). … |  | [ ] |
| [LINK-14](#link-14) | 396 | M/L | unchecked malloc | `malloc` results for CLAMScenarioOptions and indicators are not checked. Function: `Read_CLAM_Link_Params()`. |  | [ ] |
| [LINK-15](#link-15) | 220 | M/M | uninitialised field | For Q_CHANGE levers, `groupCode = Util_Get_FG_Index(...)` is not checked for -1 (the LINEAR_MORT_CHANGE branch… |  | [ ] |

---

## LINK-08

**wrong variable** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atCLAMLinkIO.c` 542, `atCLAMLinkIO.c` 405
**Lines at review (Oct 2026):** 342, 542-544

**Function:** `Read_CLAM_Link_Params()`

**Why it's thought to be a bug**

`indicators[indicatorID].scenarioLinks[linkID] = scenarioID;` — `linkID` is initialised to 0 and never incremented, while `numScenarioLinks++` counts up. Every link overwrites `scenarioLinks[0]`, and entries 1..n-1 stay uninitialised (malloc'd). Run_CLAM (atCLAMManage.c:241-245) then uses those garbage values as `CLAMScenarioOptions[scenarioID]` indices: out-of-bounds write and wrong scenarios run. There is also no check against MAXSCENARIOS. Function: `Read_CLAM_Link_Params()`.

**Code at issue** — `atCLAMLinkIO.c` line 542  
Search for: `indicators[indicatorID].scenarioLinks[linkID] = scenarioID`

```c
  540: 
  541: 				/* Store the index of the scenario for later use */
  542→ 				indicators[indicatorID].scenarioLinks[linkID] = scenarioID;
  543: 				node = node->next;
  544: 				indicators[indicatorID].numScenarioLinks++;
```

**Code at issue** — `atCLAMLinkIO.c` line 405  
Search for: `CLAMScenarioOptions[scenarioID]`

```c
  403: 			scenarioNode = scenarioList->nodesetval->nodeTab[scenarioID];
  404: 
  405→ 			Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, scenarioNode, "ScenarioName", CLAMScenarioOptions[scenarioID].scenarioOptionName);
  406: 
  407: 			/* Check that this scenario is recognised */
```

**Suggested fix (from review):** Use `scenarioLinks[indicators[indicatorID].numScenarioLinks]` and quit if it reaches MAXSCENARIOS.

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

## LINK-09

**uninitialised memory** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atCLAMLinkIO.c` 396
**Lines at review (Oct 2026):** 396, 423-458

**Function:** `Read_CLAM_Link_Params()`

**Why it's thought to be a bug**

`CLAMScenarioOptions = malloc(...)`: only the strategies present in the XML are filled, and `isLocked`/`timeLockOutStarted` are never initialised anywhere (grep shows them set only in atCLAMImplement.c:56-57 after a strategy is implemented). Run_CLAM tests `isLocked == TRUE/FALSE` (atCLAMManage.c:262, 311, 410) on garbage, so strategies can be silently skipped or treated as locked. Run_CLAM also loops `strategyIndex < maxOptionValue`, which can exceed the number of strategies read (garbage `isEnabled`, `numLevers`). Function: `Read_CLAM_Link_Params()`.

**Code at issue** — `atCLAMLinkIO.c` line 396  
Search for: `CLAMScenarioOptions = malloc(`

```c
  394: 
  395: 		numScenarioOptions = scenarioList->nodesetval->nodeNr;
  396→ 		CLAMScenarioOptions = malloc(sizeof(CLAMScenarioOptionStructure) * numScenarioOptions);
  397: 
  398: 		printf("numScenarios  %d\n", numScenarioOptions);
```

**Suggested fix (from review):** Use calloc, or explicitly set isLocked=FALSE, timeLockOutStarted=0 and isEnabled=0 for every strategy slot.

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

## LINK-10

**missing bounds check** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atCLAMLinkIO.c` 124
**Lines at review (Oct 2026):** 413, 424, 147

**Why it's thought to be a bug**

`maxOptionValue`, `numStrategyOptions` and `numLevers` come from XML with no limit, but `strategies[]` is fixed at maxStrategyOptions (7) (atCLAMLinkPrivate.h:59) and `strategyLevers[]` at 20 (line 47). More than 7 strategies or 20 levers in the XML overruns the struct. Run_CLAM also indexes `strategies[strategyIndex]` up to maxOptionValue-1. Functions: `Read_CLAM_Link_Params()`, `Read_Lever_List()`.

**Code at issue** — `atCLAMLinkIO.c` line 124 (located by name only; check this is the right place)  
Search for: `for (i = 0; i < maxStrategyOptions; i++) {`

```c
  122: 	int i;
  123: 
  124→ 	for (i = 0; i < maxStrategyOptions; i++) {
  125: 		if (strcmp(name, CLAMScenarios[scenarioIndex].strategyNames[i]) == 0)
  126: 			return i;
```

**Suggested fix (from review):** Quit if numStrategyOptions or maxOptionValue > maxStrategyOptions, or numLevers > 20.

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

## LINK-11

**NULL dereference** · Conf H / Sev M  
**Found at (current code):** `atCLAMLinkIO.c` 470
**Lines at review (Oct 2026):** 470-471

**Function:** `Read_CLAM_Link_Params()`

**Why it's thought to be a bug**

`printf("...", indicatorList->nodesetval->nodeNr); if (indicatorList && ...)` — dereferenced before the NULL check. Function: `Read_CLAM_Link_Params()`.

**Code at issue** — `atCLAMLinkIO.c` line 470  
Search for: `printf("`

```c
  468: 	indicatorList = Util_XML_Get_Node_List(ATLANTIS_ATTRIBUTE_SUB_GROUP, IndicatorListNode, "Indicator");
  469: 
  470→ 	printf("indicatorList->nodesetval->nodeNr = %d\n", indicatorList->nodesetval->nodeNr);
  471: 	if (indicatorList && indicatorList->nodesetval->nodeNr > 0) {
  472: 		if (bm->debug == clam_debug)
```

**Suggested fix (from review):** Move the printf inside the `if`.

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

## LINK-12

**uninitialised / leak** · Conf M / Sev M  
**Found at (current code):** `atCLAMLinkIO.c` 565
**Lines at review (Oct 2026):** 344, 421, 565

**Function:** `Read_CLAM_Link_Params()`

**Why it's thought to be a bug**

`xmlXPathFreeObject(strategyList);` frees only the last per-scenario list (the others leak). If `scenarioList` is NULL or empty, `strategyList` is uninitialised and freeing it is UB. Function: `Read_CLAM_Link_Params()`.

**Code at issue** — `atCLAMLinkIO.c` line 565  
Search for: `xmlXPathFreeObject(strategyList)`

```c
  563: 	xmlXPathFreeObject(indicatorList);
  564: 	xmlXPathFreeObject(scenarioList);
  565→ 	xmlXPathFreeObject(strategyList);
  566: 	xmlFreeDoc(inputDoc);
  567: 	xmlCleanupParser();
```

**Suggested fix (from review):** Free strategyList inside the scenario loop, and initialise it to NULL.

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

## LINK-13

**format/arg mismatch** · Conf H / Sev L  
**Found at (current code):** `atCLAMLinkIO.c` 101
**Lines at review (Oct 2026):** 101

**Function:** `Get_Lever_Type()`

**Why it's thought to be a bug**

`quit("Lever name '%s' not recognised\n");` — `%s` with no argument (UB, garbage or crash in the error path). Function: `Get_Lever_Type()`.

**Code at issue** — `atCLAMLinkIO.c` line 101  
Search for: `quit("Lever name '%s' not recognised\n")`

```c
   99: 		return LINEAR_MORT_CHANGE;
  100: 
  101→ 	quit("Lever name '%s' not recognised\n");
  102: 	return -1;
  103: }
```

**Suggested fix (from review):** Pass `name`.

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

## LINK-14

**unchecked malloc** · Conf M / Sev L  
**Found at (current code):** `atCLAMLinkIO.c` 396
**Lines at review (Oct 2026):** 396, 476

**Function:** `Read_CLAM_Link_Params()`

**Why it's thought to be a bug**

`malloc` results for CLAMScenarioOptions and indicators are not checked. Function: `Read_CLAM_Link_Params()`.

**Code at issue** — `atCLAMLinkIO.c` line 396 (the line the review pointed to, found by its text)  
Search for: `CLAMScenarioOptions = malloc(sizeof(CLAMScenarioOptionStructure) * numScenarioOptions);`

```c
  394: 
  395: 		numScenarioOptions = scenarioList->nodesetval->nodeNr;
  396→ 		CLAMScenarioOptions = malloc(sizeof(CLAMScenarioOptionStructure) * numScenarioOptions);
  397: 
  398: 		printf("numScenarios  %d\n", numScenarioOptions);
```

**Suggested fix (from review):** Check for NULL and quit.

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

## LINK-15

**uninitialised field** · Conf M / Sev M  
**Found at (current code):** `atCLAMLinkIO.c` 220
**Lines at review (Oct 2026):** 220-222

**Function:** `Read_Lever_List()`

**Why it's thought to be a bug**

For Q_CHANGE levers, `groupCode = Util_Get_FG_Index(...)` is not checked for -1 (the LINEAR_MORT_CHANGE branch does check, lines 202-204). Q_CHANGE also never reads "Location", yet Implement_Strategy passes `lever->location` (uninitialised) to Harvest_Scale_Q (atCLAMImplement.c:40). Function: `Read_Lever_List()`.

**Code at issue** — `atCLAMLinkIO.c` line 220  
Search for: `groupCode = Util_Get_FG_Index(`

```c
  218: 
  219: 				/* Now set the groupCode value */
  220→ 				CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].groupCode = Util_Get_FG_Index(bm,
  221: 						CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].targetName);
  222: 
```

**Suggested fix (from review):** Check groupCode == -1, and read "Location" for Q_CHANGE.

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
