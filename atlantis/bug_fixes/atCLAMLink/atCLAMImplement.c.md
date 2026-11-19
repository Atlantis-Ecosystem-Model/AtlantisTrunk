# atCLAMLink/atCLAMImplement.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [LINK-06](#link-06) | 116-118 | H/M | wrong check / OOB write | `variableIndex = tsIndex(&ts, varName) - 1; if (variableIndex == -1) quit(...)` — `tsIndex()` (dfIndex, sjwlib… |  | [ ] |
| [LINK-07](#link-07) | 87, 95, 98 | M/M | missing bounds check | `pssIndex` comes straight from the XML lever "Location" (atCLAMLinkIO.c:178) and indexes `bm->pss[pssIndex]` w… |  | [ ] |

---

## LINK-06

**wrong check / OOB write** · Conf H / Sev M  
**Listed lines:** 116-118

**Function:** `Scale_PointSource()`

**Why it's thought to be a bug**

`variableIndex = tsIndex(&ts, varName) - 1; if (variableIndex == -1) quit(...)` — `tsIndex()` (dfIndex, sjwlib/datafile.c:571) returns -1 when the name is not found, so variableIndex becomes -2 and the check misses it. Lines 124-127 then write to `[...][-2]` (heap corruption). Only a match on the time variable (index 0) is caught. Function: `Scale_PointSource()`.

**Current code** (`atCLAMLink/atCLAMImplement.c` lines 114–120)

```c
  114: 
  115: 	/* Find the index of the variables in the source list of variables */
  116: 	variableIndex = tsIndex(&bm->pss[pssIndex].ts, varName) - 1;
  117: 	if (variableIndex == -1)
  118: 		quit("Scale_PointSource - The variable %s is not in the point source %s\n", varName, bm->pss[pssIndex].name);
  119: 
  120: 	//fprintf(llogfp, "variableIndex = %d\n", variableIndex);
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
**Listed lines:** 87, 95, 98

**Function:** `Scale_PointSource()`

**Why it's thought to be a bug**

`pssIndex` comes straight from the XML lever "Location" (atCLAMLinkIO.c:178) and indexes `bm->pss[pssIndex]` without a range check against the number of point sources. Function: `Scale_PointSource()`.

**Current code** (`atCLAMLink/atCLAMImplement.c` lines 85–100)

```c
   85:  *
   86:  */
   87: void Scale_PointSource(MSEBoxModel *bm, char *varName, int pssIndex, double start, double period, double mult, FILE *llogfp) {
   88: 
   89: 	double **multArray;
   90: 	int **startArray;
   91: 	int **periodArray;
   92: 	int **changeFlagArray;
   93: 	int i, j, variableIndex;
   94: 
   95: 	fprintf(llogfp, "Scale_PointSource: Scaling %s: %s by mult %e from %e over %e\n", bm->pss[pssIndex].name, varName, mult,
   96: 			start, period);
   97: 
   98: 	bm->pss[pssIndex].numPssChanges++;
   99: 	/* Now increase the array sizes */
  100: 	changeFlagArray = Util_Alloc_Init_2D_Int(bm->pss[pssIndex].ts.nv - 1, bm->pss[pssIndex].numPssChanges, FALSE);
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
