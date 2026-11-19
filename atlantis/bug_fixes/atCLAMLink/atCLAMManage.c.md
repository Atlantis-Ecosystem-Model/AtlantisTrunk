# atCLAMLink/atCLAMManage.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **7 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1), plus 2 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [LINK-16](#link-16) ⭐ | 440 | H/H | off-by-one / wrong condition | `if (bestScenarioIndex > 0 && bestStrategyIndex > 0) Implement_Strategy(...)` — indices are 0-based and -1 mea… |  | [ ] |
| [LINK-17](#link-17) | 592 | H/M | copy-paste | `"Change in Seagrass Area"` returns `Assess_Get_Physical_Sampled_Value(bm, o2_wc_id, b)`, the dissolved oxygen… |  | [ ] |
| [LINK-18](#link-18) | 163 | M/M | uninitialised / div-by-zero | `currentValue` is only set in the STANDARD_DEV branch (line 163), not for PERCENT indicators. The REACTIVE_MAN… |  | [ ] |
| [LINK-19](#link-19) | 117, 101, 348 | M/M | stale value / div-by-zero | In the second loop every indicator gets `runScenarios = TRUE` (line 117; the comment above says "not running" … |  | [ ] |
| [LINK-20](#link-20) | 157 | M/M | div-by-zero | `requiredChange = requiredChange / indicatorValue * 100;` — no guard if the sampled indicator value is 0 (e.g.… |  | [ ] |
| [LINK-21](#link-21) | 96 | L/M | logic | `if (change > requiredChange \|\| requiredChange == 0)` selects the most positive change, not the largest magnit… |  | [ ] |
| [LINK-22](#link-22) | 335 | L/L | div-by-zero | `normaliseVar = variance/varSum` (varSum can be 0; the value is only logged). The distance divides by `(1 - va… |  | [ ] |

---

## LINK-16

**off-by-one / wrong condition** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atCLAMManage.c` 440
**Lines at review (Oct 2026):** 440

**Function:** `Run_CLAM()`

**Why it's thought to be a bug**

`if (bestScenarioIndex > 0 && bestStrategyIndex > 0) Implement_Strategy(...)` — indices are 0-based and -1 means "not found" (lines 405-406). A best result in scenario 0 or strategy 0 is never implemented. Function: `Run_CLAM()`.

**Code at issue** — `atCLAMManage.c` line 440  
Search for: `if (bestScenarioIndex > 0 && bestStrategyIndex > 0)`

```c
  438: 
  439: 	fprintf(llogfp, "CLAM: bestScenarioIndex %d, bestStrategyIndex %d\n", bestScenarioIndex, bestStrategyIndex);
  440→ 	if (bestScenarioIndex > 0 && bestStrategyIndex > 0)
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
**Found at (current code):** `atCLAMManage.c` 592
**Lines at review (Oct 2026):** 592-594

**Function:** `Get_Box_Indicator_Value()`

**Why it's thought to be a bug**

`"Change in Seagrass Area"` returns `Assess_Get_Physical_Sampled_Value(bm, o2_wc_id, b)`, the dissolved oxygen tracer copied from the block above. The seagrass indicator is therefore driven by O2. Function: `Get_Box_Indicator_Value()`.

**Code at issue** — `atCLAMManage.c` line 592  
Search for: `"Change in Seagrass Area"`

```c
  590: 		return Assess_Get_Physical_Sampled_Value(bm, o2_wc_id, b);
  591: 	}
  592→ 	if (strcmp(indicators[indicatorIndex].CLAMOutputName, "Change in Seagrass Area") == 0) {
  593: 		return Assess_Get_Physical_Sampled_Value(bm, o2_wc_id, b);
  594: 	}
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
**Found at (current code):** `atCLAMManage.c` 163
**Lines at review (Oct 2026):** 155-163, 616-617

**Why it's thought to be a bug**

`currentValue` is only set in the STANDARD_DEV branch (line 163), not for PERCENT indicators. The REACTIVE_MANAGER weighting divides by `indicators[i].currentValue` (uninitialised malloc memory, or 0). Functions: `CLAM_Check_Indicators()`, `Determine_Indicator_Weightings()`.

**Code at issue** — `atCLAMManage.c` line 163  
Search for: `indicators[i].currentValue`

```c
  161: 					requiredChange = requiredChange / SD;
  162: 					indicators[i].targetChange = requiredChange;
  163→ 					indicators[i].currentValue = indicatorValue;
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
**Found at (current code):** `atCLAMManage.c` 117, `atCLAMManage.c` 101, `atCLAMManage.c` 348
**Lines at review (Oct 2026):** 114-119, 152, 298, 348

**Why it's thought to be a bug**

In the second loop every indicator gets `runScenarios = TRUE` (line 117; the comment above says "not running" — a copy-paste of line 74). `targetChange` is only updated when `requiredChange != 0`, so an indicator in range keeps a stale or uninitialised `targetChange`. Run_CLAM then divides by `fabs(targetChange)` (line 348), which gives inf/NaN when it is 0. Functions: `CLAM_Check_Indicators()`, `Run_CLAM()`.

**Code at issue** — `atCLAMManage.c` line 117  
Search for: `runScenarios = TRUE`

```c
  115: 
  116: 			/* By default we are not running the scenarios attached to this indicator */
  117→ 			indicators[i].runScenarios = TRUE;
  118: 			//numOutputStates = CLAMOutputOptions[indicators[i].CLAMOutputIndex].numStates;
  119: 			requiredChange = 0;
```

**Code at issue** — `atCLAMManage.c` line 101  
Search for: `requiredChange != 0`

```c
   99: 			}
  100: 		}
  101→ 		if (requiredChange != 0) {
  102: 			runCLAM = TRUE;
  103: 
```

**Code at issue** — `atCLAMManage.c` line 348  
Search for: `fabs(targetChange)`

```c
  346: 							} else {
  347: 								distance[scenarioIndex][strategyIndex][indicatorIndex] = //variance[scenarioIndex][strategyIndex][indicatorIndex] / varSum *
  348→ 										fabs(targetChange - expectedValue[scenarioIndex][strategyIndex][indicatorIndex]) / (fabs(targetChange) * (1
  349: 												- variance[scenarioIndex][strategyIndex][indicatorIndex]));
  350: 								//variance[scenarioIndex][strategyIndex][indicatorIndex] / fabs(targetChange));
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
**Found at (current code):** `atCLAMManage.c` 157
**Lines at review (Oct 2026):** 157

**Function:** `CLAM_Check_Indicators()`

**Why it's thought to be a bug**

`requiredChange = requiredChange / indicatorValue * 100;` — no guard if the sampled indicator value is 0 (e.g. algae or seagrass absent). Function: `CLAM_Check_Indicators()`.

**Code at issue** — `atCLAMManage.c` line 157  
Search for: `requiredChange = requiredChange / indicatorValue * 100`

```c
  155: 				if (CLAMOutputOptions[indicators[i].CLAMOutputIndex].outputType == PERCENT) {
  156: 					/* Calculate the percentage change required */
  157→ 					requiredChange = requiredChange / indicatorValue * 100;
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
**Found at (current code):** `atCLAMManage.c` 96
**Lines at review (Oct 2026):** 96, 138

**Function:** `CLAM_Check_Indicators()`

**Why it's thought to be a bug**

`if (change > requiredChange || requiredChange == 0)` selects the most positive change, not the largest magnitude, so when all changes are negative it keeps the smallest decrease. This contradicts the comment "Save if its the max required change". Function: `CLAM_Check_Indicators()`.

**Code at issue** — `atCLAMManage.c` line 96  
Search for: `if (change > requiredChange || requiredChange == 0)`

```c
   94: 
   95: 				/* Save if its the max required change - might need other ways of doing this - might want to do weighted average over sites */
   96→ 				if (change > requiredChange || requiredChange == 0) {
   97: 					requiredChange = change;
   98: 				}
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
**Found at (current code):** `atCLAMManage.c` 335
**Lines at review (Oct 2026):** 335, 348-349

**Function:** `Run_CLAM()`

**Why it's thought to be a bug**

`normaliseVar = variance/varSum` (varSum can be 0; the value is only logged). The distance divides by `(1 - variance)`, which is 0 when variance == 1. Function: `Run_CLAM()`.

**Code at issue** — `atCLAMManage.c` line 335 (closest match to the quoted code; wording differs slightly)  
Search for: `double normaliseVar = variance[scenarioIndex][strategyIndex][indicatorIndex] / varSum;`

```c
  333: 								&& CLAMScenarioOptions[scenarioIndex].strategies[strategyIndex].isEnabled > 0) {
  334: 
  335→ 							double normaliseVar = variance[scenarioIndex][strategyIndex][indicatorIndex] / varSum;
  336: 
  337: 							fprintf(llogfp, "normaliseVar = %e\n", normaliseVar);
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
