# atCLAMLink/atCLAMImplement.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [LINK-06](#link-06) | 116 | H/M | wrong check / OOB write | `variableIndex = tsIndex(&ts, varName) - 1; if (variableIndex == -1) quit(...)` — `tsIndex()` (dfIndex, sjwlib… |  | [ ] |
| [LINK-07](#link-07) | 95 | M/M | missing bounds check | `pssIndex` comes straight from the XML lever "Location" (atCLAMLinkIO.c:178) and indexes `bm->pss[pssIndex]` w… |  | [ ] |

---

## LINK-06

**wrong check / OOB write** · Conf H / Sev M  
**Found at (current code):** `atCLAMImplement.c` 116
**Lines at review (Oct 2026):** 116-118

**Function:** `Scale_PointSource()`

**Why it's thought to be a bug**

`variableIndex = tsIndex(&ts, varName) - 1; if (variableIndex == -1) quit(...)` — `tsIndex()` (dfIndex, sjwlib/datafile.c:571) returns -1 when the name is not found, so variableIndex becomes -2 and the check misses it. Lines 124-127 then write to `[...][-2]` (heap corruption). Only a match on the time variable (index 0) is caught. Function: `Scale_PointSource()`.

**Code at issue** — `atCLAMImplement.c` line 116 (the line the review pointed to, found by its text)  
Search for: `variableIndex = tsIndex(&bm->pss[pssIndex].ts, varName) - 1;`

```c
  114: 
  115: 	/* Find the index of the variables in the source list of variables */
  116→ 	variableIndex = tsIndex(&bm->pss[pssIndex].ts, varName) - 1;
  117: 	if (variableIndex == -1)
  118: 		quit("Scale_PointSource - The variable %s is not in the point source %s\n", varName, bm->pss[pssIndex].name);
```

**Suggested fix (from review):** Test `tsIndex(...) < 1` before subtracting 1.

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

## LINK-07

**missing bounds check** · Conf M / Sev M  
**Found at (current code):** `atCLAMImplement.c` 95
**Lines at review (Oct 2026):** 87, 95, 98

**Function:** `Scale_PointSource()`

**Why it's thought to be a bug**

`pssIndex` comes straight from the XML lever "Location" (atCLAMLinkIO.c:178) and indexes `bm->pss[pssIndex]` without a range check against the number of point sources. Function: `Scale_PointSource()`.

**Code at issue** — `atCLAMImplement.c` line 95  
Search for: `pss[pssIndex]`

```c
   93: 	int i, j, variableIndex;
   94: 
   95→ 	fprintf(llogfp, "Scale_PointSource: Scaling %s: %s by mult %e from %e over %e\n", bm->pss[pssIndex].name, varName, mult,
   96: 			start, period);
   97: 
```

**Suggested fix (from review):** Validate `0 <= pssIndex < bm->npss` (or equivalent) in Read_Lever_List.

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
