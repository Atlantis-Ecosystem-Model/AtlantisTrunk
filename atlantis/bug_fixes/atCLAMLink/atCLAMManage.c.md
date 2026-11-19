# atCLAMLink/atCLAMManage.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **7 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1), plus 2 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [LINK-16](#link-16) ⭐ | 440 | H/H | off-by-one / wrong condition | `if (bestScenarioIndex > 0 && bestStrategyIndex > 0) Implement_Strategy(...)` — indices are 0-based and -1 mea… |  | [ ] |
| [LINK-17](#link-17) | 592-594 | H/M | copy-paste | `"Change in Seagrass Area"` returns `Assess_Get_Physical_Sampled_Value(bm, o2_wc_id, b)`, the dissolved oxygen… |  | [ ] |
| [LINK-18](#link-18) | 155-163, 616-617 | M/M | uninitialised / div-by-zero | `currentValue` is only set in the STANDARD_DEV branch (line 163), not for PERCENT indicators. The REACTIVE_MAN… |  | [ ] |
| [LINK-19](#link-19) | 114-119, 152, 298, 348 | M/M | stale value / div-by-zero | In the second loop every indicator gets `runScenarios = TRUE` (line 117; the comment above says "not running" … |  | [ ] |
| [LINK-20](#link-20) | 157 | M/M | div-by-zero | `requiredChange = requiredChange / indicatorValue * 100;` — no guard if the sampled indicator value is 0 (e.g.… |  | [ ] |
| [LINK-21](#link-21) | 96, 138 | L/M | logic | `if (change > requiredChange \|\| requiredChange == 0)` selects the most positive change, not the largest magnit… |  | [ ] |
| [LINK-22](#link-22) | 335, 348-349 | L/L | div-by-zero | `normaliseVar = variance/varSum` (varSum can be 0; the value is only logged). The distance divides by `(1 - va… |  | [ ] |

---

## LINK-16

**off-by-one / wrong condition** · Conf H / Sev H ⭐ priority  
**Listed lines:** 440

**Function:** `Run_CLAM()`

**Why it's thought to be a bug**

`if (bestScenarioIndex > 0 && bestStrategyIndex > 0) Implement_Strategy(...)` — indices are 0-based and -1 means "not found" (lines 405-406). A best result in scenario 0 or strategy 0 is never implemented. Function: `Run_CLAM()`.

**Current code** (`atCLAMLink/atCLAMManage.c` lines 438–442)

```c
  438: 
  439: 	fprintf(llogfp, "CLAM: bestScenarioIndex %d, bestStrategyIndex %d\n", bestScenarioIndex, bestStrategyIndex);
  440: 	if (bestScenarioIndex > 0 && bestStrategyIndex > 0)
  441: 		Implement_Strategy(bm, bestScenarioIndex, bestStrategyIndex, llogfp);
  442: 	else {
```

**Suggested fix (from review):** Use `>= 0` on both.

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

## LINK-17

**copy-paste** · Conf H / Sev M  
**Listed lines:** 592-594

**Function:** `Get_Box_Indicator_Value()`

**Why it's thought to be a bug**

`"Change in Seagrass Area"` returns `Assess_Get_Physical_Sampled_Value(bm, o2_wc_id, b)`, the dissolved oxygen tracer copied from the block above. The seagrass indicator is therefore driven by O2. Function: `Get_Box_Indicator_Value()`.

**Current code** (`atCLAMLink/atCLAMManage.c` lines 590–596)

```c
  590: 		return Assess_Get_Physical_Sampled_Value(bm, o2_wc_id, b);
  591: 	}
  592: 	if (strcmp(indicators[indicatorIndex].CLAMOutputName, "Change in Seagrass Area") == 0) {
  593: 		return Assess_Get_Physical_Sampled_Value(bm, o2_wc_id, b);
  594: 	}
  595: 	quit("Get_Box_Indicator_Value: Monitoring of indicator %s not yet implemented\n", indicators[indicatorIndex].CLAMOutputName);
  596: 
```

**Suggested fix (from review):** Return the seagrass cover/area value (or quit as "not implemented").

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

## LINK-18

**uninitialised / div-by-zero** · Conf M / Sev M  
**Listed lines:** 155-163, 616-617

**Why it's thought to be a bug**

`currentValue` is only set in the STANDARD_DEV branch (line 163), not for PERCENT indicators. The REACTIVE_MANAGER weighting divides by `indicators[i].currentValue` (uninitialised malloc memory, or 0). Functions: `CLAM_Check_Indicators()`, `Determine_Indicator_Weightings()`.

**Current code** (`atCLAMLink/atCLAMManage.c` lines 153–165)

```c
  153: 				/* Assign the impact values for this indicator */
  154: 				// allocatedImpactWeightings(bm, i, requiredChange);
  155: 				if (CLAMOutputOptions[indicators[i].CLAMOutputIndex].outputType == PERCENT) {
  156: 					/* Calculate the percentage change required */
  157: 					requiredChange = requiredChange / indicatorValue * 100;
  158: 					indicators[i].targetChange = requiredChange;
  159: 				} else {
  160: 
  161: 					requiredChange = requiredChange / SD;
  162: 					indicators[i].targetChange = requiredChange;
  163: 					indicators[i].currentValue = indicatorValue;
  164: 				}
  165: 				/* log the info to the log file */
```

**Suggested fix (from review):** Set `currentValue = indicatorValue` in both branches, and guard against 0.

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

## LINK-19

**stale value / div-by-zero** · Conf M / Sev M  
**Listed lines:** 114-119, 152, 298, 348

**Why it's thought to be a bug**

In the second loop every indicator gets `runScenarios = TRUE` (line 117; the comment above says "not running" — a copy-paste of line 74). `targetChange` is only updated when `requiredChange != 0`, so an indicator in range keeps a stale or uninitialised `targetChange`. Run_CLAM then divides by `fabs(targetChange)` (line 348), which gives inf/NaN when it is 0. Functions: `CLAM_Check_Indicators()`, `Run_CLAM()`.

**Current code** (`atCLAMLink/atCLAMManage.c` lines 59–68) — code appears to have moved -53 lines since the review

```c
   59:  *
   60:  */
   61: void CLAM_Check_Indicators(MSEBoxModel *bm, FILE *llogfp) {
   62: 	int i, b, bb;
   63: 	double indicatorValue, change, requiredChange;
   64: 	double tempIndicatorValue;
   65: 	int runCLAM = FALSE; /* Assume that all the indicators are within the target range */
   66: 	double SD;
   67: 	//double maxScenarioImpact;
   68: 	//int numOutputStates;
```

**Suggested fix (from review):** Set targetChange = 0 for in-range indicators and skip the distance division when it is 0.

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

## LINK-20

**div-by-zero** · Conf M / Sev M  
**Listed lines:** 157

**Function:** `CLAM_Check_Indicators()`

**Why it's thought to be a bug**

`requiredChange = requiredChange / indicatorValue * 100;` — no guard if the sampled indicator value is 0 (e.g. algae or seagrass absent). Function: `CLAM_Check_Indicators()`.

**Current code** (`atCLAMLink/atCLAMManage.c` lines 155–159)

```c
  155: 				if (CLAMOutputOptions[indicators[i].CLAMOutputIndex].outputType == PERCENT) {
  156: 					/* Calculate the percentage change required */
  157: 					requiredChange = requiredChange / indicatorValue * 100;
  158: 					indicators[i].targetChange = requiredChange;
  159: 				} else {
```

**Suggested fix (from review):** Guard `indicatorValue != 0`.

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

## LINK-21

**logic** · Conf L / Sev M  
**Listed lines:** 96, 138

**Function:** `CLAM_Check_Indicators()`

**Why it's thought to be a bug**

`if (change > requiredChange || requiredChange == 0)` selects the most positive change, not the largest magnitude, so when all changes are negative it keeps the smallest decrease. This contradicts the comment "Save if its the max required change". Function: `CLAM_Check_Indicators()`.

**Current code** (`atCLAMLink/atCLAMManage.c` lines 94–133 (truncated))

```c
   94: 
   95: 				/* Save if its the max required change - might need other ways of doing this - might want to do weighted average over sites */
   96: 				if (change > requiredChange || requiredChange == 0) {
   97: 					requiredChange = change;
   98: 				}
   99: 			}
  100: 		}
  101: 		if (requiredChange != 0) {
  102: 			runCLAM = TRUE;
  103: 
  104: 		} else {
  105: 			fprintf(bm->logFile, "CLAM run not required\n");
  106: 		}
  107: 	}
  108: 
  109: 	/* Ok one of our indicators is outside its acceptable range - now take all of the other indicators into account
  110: 	 * so we don't select a strategy that will have nasty effects on the rest of them
  111: 	 */
  112: 	if (runCLAM) {
  113: 		printf("numIndicators = %d\n", numIndicators);
  114: 		for (i = 0; i < numIndicators; i++) {
  115: 
  116: 			/* By default we are not running the scenarios attached to this indicator */
  117: 			indicators[i].runScenarios = TRUE;
  118: 			//numOutputStates = CLAMOutputOptions[indicators[i].CLAMOutputIndex].numStates;
  119: 			requiredChange = 0;
  120: 
  121: 			/* How we calculate the required change depends on the type of indicator.
  122: 			 * If its a standard dev indicator then we calculate the standard deviation over all boxes.
  123: 			 * If its a percent change indicator then we calculate the indicator value in a number of special boxes.
  124: 			 */
  125: 			/* For each site of interest - get the required change. Just look at the intense sites.*/
  126: 			for (bb = 0; bb < bm->nsbox_i; bb++) {
  127: 				b = bm->nsboxes[bb];
  128: 				tempIndicatorValue = Get_Box_Indicator_Value(bm, i, b);
  129: 
  130: 				printf("name = %s, tempIndicatorValue = %e\n", indicators[i].CLAMOutputName, tempIndicatorValue);
  131: 				/* Check that the indicator is inside the allowed range */
  132: 				//if (tempIndicatorValue < indicators[i].minValue || tempIndicatorValue > indicators[i].maxValue) {
  133: 
```

**Suggested fix (from review):** Compare `fabs(change) > fabs(requiredChange)`.

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

## LINK-22

**div-by-zero** · Conf L / Sev L  
**Listed lines:** 335, 348-349

**Function:** `Run_CLAM()`

**Why it's thought to be a bug**

`normaliseVar = variance/varSum` (varSum can be 0; the value is only logged). The distance divides by `(1 - variance)`, which is 0 when variance == 1. Function: `Run_CLAM()`.

**Current code** (`atCLAMLink/atCLAMManage.c` lines 333–351)

```c
  333: 								&& CLAMScenarioOptions[scenarioIndex].strategies[strategyIndex].isEnabled > 0) {
  334: 
  335: 							double normaliseVar = variance[scenarioIndex][strategyIndex][indicatorIndex] / varSum;
  336: 
  337: 							fprintf(llogfp, "normaliseVar = %e\n", normaliseVar);
  338: 							/* Calculate the distance to the required value */
  339: 							if (expectedValue[scenarioIndex][strategyIndex][indicatorIndex] == 0 && variance[scenarioIndex][strategyIndex][indicatorIndex] == 0) {
  340: 								distance[scenarioIndex][strategyIndex][indicatorIndex] = 0.0;
  341: 							} else if (variance[scenarioIndex][strategyIndex][indicatorIndex] == 0) {
  342: 								distance[scenarioIndex][strategyIndex][indicatorIndex] = fabs(targetChange
  343: 										- expectedValue[scenarioIndex][strategyIndex][indicatorIndex]);
  344: 								sum = sum + distance[scenarioIndex][strategyIndex][indicatorIndex];
  345: 								count++;
  346: 							} else {
  347: 								distance[scenarioIndex][strategyIndex][indicatorIndex] = //variance[scenarioIndex][strategyIndex][indicatorIndex] / varSum *
  348: 										fabs(targetChange - expectedValue[scenarioIndex][strategyIndex][indicatorIndex]) / (fabs(targetChange) * (1
  349: 												- variance[scenarioIndex][strategyIndex][indicatorIndex]));
  350: 								//variance[scenarioIndex][strategyIndex][indicatorIndex] / fabs(targetChange));
  351: 								//fprintf(llogfp, "fabs(targetChange - expectedValue[scenarioIndex][strategyIndex][indicatorIndex]) = %e\n", fabs(targetChange - expectedValue[scenarioIndex][strategyIndex][indicatorIndex]));
```

**Suggested fix (from review):** Guard the denominators.

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

atCLAMManage.c Get_Variance loops over MAXNUMOUTPUTSTATES while Get_Expected_Value loops over numStates. This is harmless because the unused OutputStates entries are zero-initialised, but it is inconsistent.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atCLAMManage.c CLAM_Check_Indicators: SD is only set when the first or a larger change is found, and is logged for PERCENT indicators where it may be uninitialised (log only).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
