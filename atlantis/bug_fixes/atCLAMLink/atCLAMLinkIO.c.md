# atCLAMLink/atCLAMLinkIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **8 findings** (severity H: 3; confidence H and severity H: 3; on the priority list: 3).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [LINK-08](#link-08) ⭐ | 342, 542-544 | H/H | wrong variable | `indicators[indicatorID].scenarioLinks[linkID] = scenarioID;` — `linkID` is initialised to 0 and never increme… |  | [ ] |
| [LINK-09](#link-09) ⭐ | 396, 423-458 | H/H | uninitialised memory | `CLAMScenarioOptions = malloc(...)`: only the strategies present in the XML are filled, and `isLocked`/`timeLo… |  | [ ] |
| [LINK-10](#link-10) ⭐ | 413, 424, 147 | H/H | missing bounds check | `maxOptionValue`, `numStrategyOptions` and `numLevers` come from XML with no limit, but `strategies[]` is fixe… |  | [ ] |
| [LINK-11](#link-11) | 470-471 | H/M | NULL dereference | `printf("...", indicatorList->nodesetval->nodeNr); if (indicatorList && ...)` — dereferenced before the NULL c… |  | [ ] |
| [LINK-12](#link-12) | 344, 421, 565 | M/M | uninitialised / leak | `xmlXPathFreeObject(strategyList);` frees only the last per-scenario list (the others leak). If `scenarioList`… |  | [ ] |
| [LINK-13](#link-13) | 101 | H/L | format/arg mismatch | `quit("Lever name '%s' not recognised\n");` — `%s` with no argument (UB, garbage or crash in the error path). … |  | [ ] |
| [LINK-14](#link-14) | 396, 476 | M/L | unchecked malloc | `malloc` results for CLAMScenarioOptions and indicators are not checked. Function: `Read_CLAM_Link_Params()`. |  | [ ] |
| [LINK-15](#link-15) | 220-222 | M/M | uninitialised field | For Q_CHANGE levers, `groupCode = Util_Get_FG_Index(...)` is not checked for -1 (the LINEAR_MORT_CHANGE branch… |  | [ ] |

---

## LINK-08

**wrong variable** · Conf H / Sev H ⭐ priority  
**Listed lines:** 342, 542-544

**Function:** `Read_CLAM_Link_Params()`

**Why it's thought to be a bug**

`indicators[indicatorID].scenarioLinks[linkID] = scenarioID;` — `linkID` is initialised to 0 and never incremented, while `numScenarioLinks++` counts up. Every link overwrites `scenarioLinks[0]`, and entries 1..n-1 stay uninitialised (malloc'd). Run_CLAM (atCLAMManage.c:241-245) then uses those garbage values as `CLAMScenarioOptions[scenarioID]` indices: out-of-bounds write and wrong scenarios run. There is also no check against MAXSCENARIOS. Function: `Read_CLAM_Link_Params()`.

**Current code** (`atCLAMLink/atCLAMLinkIO.c` lines 340–344)

```c
  340: 	xmlNodePtr StrategyListNode, strategyNode;
  341: 	xmlDocPtr inputDoc;
  342: 	int indicatorID = 0, linkID = 0, scenarioID;
  343: 	char *scenarioName;
  344: 	xmlXPathObjectPtr indicatorList, scenarioList, strategyList;
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
**Listed lines:** 396, 423-458

**Function:** `Read_CLAM_Link_Params()`

**Why it's thought to be a bug**

`CLAMScenarioOptions = malloc(...)`: only the strategies present in the XML are filled, and `isLocked`/`timeLockOutStarted` are never initialised anywhere (grep shows them set only in atCLAMImplement.c:56-57 after a strategy is implemented). Run_CLAM tests `isLocked == TRUE/FALSE` (atCLAMManage.c:262, 311, 410) on garbage, so strategies can be silently skipped or treated as locked. Run_CLAM also loops `strategyIndex < maxOptionValue`, which can exceed the number of strategies read (garbage `isEnabled`, `numLevers`). Function: `Read_CLAM_Link_Params()`.

**Current code** (`atCLAMLink/atCLAMLinkIO.c` lines 394–398)

```c
  394: 
  395: 		numScenarioOptions = scenarioList->nodesetval->nodeNr;
  396: 		CLAMScenarioOptions = malloc(sizeof(CLAMScenarioOptionStructure) * numScenarioOptions);
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
**Listed lines:** 413, 424, 147

**Why it's thought to be a bug**

`maxOptionValue`, `numStrategyOptions` and `numLevers` come from XML with no limit, but `strategies[]` is fixed at maxStrategyOptions (7) (atCLAMLinkPrivate.h:59) and `strategyLevers[]` at 20 (line 47). More than 7 strategies or 20 levers in the XML overruns the struct. Run_CLAM also indexes `strategies[strategyIndex]` up to maxOptionValue-1. Functions: `Read_CLAM_Link_Params()`, `Read_Lever_List()`.

**Current code** (`atCLAMLink/atCLAMLinkIO.c` lines 411–415)

```c
  411: 				quit("Read_CLAM_Link_Params: Scenario %s is not recognised\n", CLAMScenarioOptions[scenarioID].scenarioOptionName);
  412: 			}
  413: 			Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, scenarioNode, integer_check, "maxOptionValue",
  414: 					&CLAMScenarioOptions[scenarioID].maxOptionValue);
  415: 
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
**Listed lines:** 470-471

**Function:** `Read_CLAM_Link_Params()`

**Why it's thought to be a bug**

`printf("...", indicatorList->nodesetval->nodeNr); if (indicatorList && ...)` — dereferenced before the NULL check. Function: `Read_CLAM_Link_Params()`.

**Current code** (`atCLAMLink/atCLAMLinkIO.c` lines 468–473)

```c
  468: 	indicatorList = Util_XML_Get_Node_List(ATLANTIS_ATTRIBUTE_SUB_GROUP, IndicatorListNode, "Indicator");
  469: 
  470: 	printf("indicatorList->nodesetval->nodeNr = %d\n", indicatorList->nodesetval->nodeNr);
  471: 	if (indicatorList && indicatorList->nodesetval->nodeNr > 0) {
  472: 		if (bm->debug == clam_debug)
  473: 			printf("DEBUG: Reading in %d CLAM indicators.\n", indicatorList->nodesetval->nodeNr);
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
**Listed lines:** 344, 421, 565

**Function:** `Read_CLAM_Link_Params()`

**Why it's thought to be a bug**

`xmlXPathFreeObject(strategyList);` frees only the last per-scenario list (the others leak). If `scenarioList` is NULL or empty, `strategyList` is uninitialised and freeing it is UB. Function: `Read_CLAM_Link_Params()`.

**Current code** (`atCLAMLink/atCLAMLinkIO.c` lines 342–346)

```c
  342: 	int indicatorID = 0, linkID = 0, scenarioID;
  343: 	char *scenarioName;
  344: 	xmlXPathObjectPtr indicatorList, scenarioList, strategyList;
  345: 	xmlNodePtr indicatorScenarioList;
  346: 	int numDays, index;
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
**Listed lines:** 101

**Function:** `Get_Lever_Type()`

**Why it's thought to be a bug**

`quit("Lever name '%s' not recognised\n");` — `%s` with no argument (UB, garbage or crash in the error path). Function: `Get_Lever_Type()`.

**Current code** (`atCLAMLink/atCLAMLinkIO.c` lines 99–103)

```c
   99: 		return LINEAR_MORT_CHANGE;
  100: 
  101: 	quit("Lever name '%s' not recognised\n");
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
**Listed lines:** 396, 476

**Function:** `Read_CLAM_Link_Params()`

**Why it's thought to be a bug**

`malloc` results for CLAMScenarioOptions and indicators are not checked. Function: `Read_CLAM_Link_Params()`.

**Current code** (`atCLAMLink/atCLAMLinkIO.c` lines 394–398)

```c
  394: 
  395: 		numScenarioOptions = scenarioList->nodesetval->nodeNr;
  396: 		CLAMScenarioOptions = malloc(sizeof(CLAMScenarioOptionStructure) * numScenarioOptions);
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
**Listed lines:** 220-222

**Function:** `Read_Lever_List()`

**Why it's thought to be a bug**

For Q_CHANGE levers, `groupCode = Util_Get_FG_Index(...)` is not checked for -1 (the LINEAR_MORT_CHANGE branch does check, lines 202-204). Q_CHANGE also never reads "Location", yet Implement_Strategy passes `lever->location` (uninitialised) to Harvest_Scale_Q (atCLAMImplement.c:40). Function: `Read_Lever_List()`.

**Current code** (`atCLAMLink/atCLAMLinkIO.c` lines 218–224)

```c
  218: 
  219: 				/* Now set the groupCode value */
  220: 				CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].groupCode = Util_Get_FG_Index(bm,
  221: 						CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].targetName);
  222: 
  223: 				break;
  224: 			}
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
