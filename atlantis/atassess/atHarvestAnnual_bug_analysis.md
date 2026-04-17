# Bug Analysis: atHarvestAnnual.c

## Summary
Found 3 potential bugs involving inconsistent use of variables in calculations and potentially mismatched array indices.

---

## Bug 1: Inconsistent use of `bm->dt` in underdiscard calculation (Line 100)

**Location:** Line 100 in function `Report_Annual_Harvest()`

**Issue:** The calculation for underdiscard includes `bm->dt` but the equivalent calculations for undercatch and other underdiscard calculations do not.

**Code:**
```c
// Line 93 - undercatch (no bm->dt)
fprintf(llogfp, "Time: %e, box %d %s had %e t undercatch", bm->dayt, b, FunctGroupArray[sp].groupCode,
        bm->FCtsCarryOver[b][tscocatch_id][sp] * bm->cell_vol * bm->X_CN * mg_2_tonne);

// Line 97 - underdiscard in same context (no bm->dt)
fprintf(llogfp, " and %e t underdiscard", bm->FCtsCarryOver[b][tscodiscard_id][sp] * bm->cell_vol * bm->X_CN * mg_2_tonne);

// Line 100 - underdiscard in else branch (HAS bm->dt - INCONSISTENT)
fprintf(llogfp, "Time: %e, box %d %s had %e t underdiscard", bm->dayt, b, FunctGroupArray[sp].groupCode,
        bm->FCtsCarryOver[b][tscodiscard_id][sp] * bm->cell_vol * bm->dt * bm->X_CN * mg_2_tonne);
```

**Why this is likely an error:**
- Lines 93 and 97 report undercatch and underdiscard for the same box using the same base calculation (without `bm->dt`)
- Line 100 reports underdiscard for the same type of data but includes `bm->dt` in the calculation
- This inconsistency appears to be a copy-paste error where `bm->dt` was incorrectly included
- Since `FCtsCarryOver` represents carry-over amounts (not rates), multiplying by `bm->dt` (time step) would give incorrect units

**Recommendation:** Remove `bm->dt` from line 100 to match the pattern in lines 93 and 97.

---

## Bug 2: Inconsistent use of `bm->dt` in global underdiscard calculation (Line 112-113)

**Location:** Lines 112-113 in function `Report_Annual_Harvest()`

**Issue:** Similar to Bug 1, the global underdiscard calculation includes `bm->dt` but the undercatch calculation does not.

**Code:**
```c
// Line 111-112 - undercatch (no bm->dt)
fprintf(llogfp, "Time: %e, %s had %e t undercatch and %e t underdiscard\n", bm->dayt, FunctGroupArray[sp].groupCode,
        bm->FCtsCarryOver[bm->nbox][tscocatch_id][sp] * bm->X_CN * mg_2_tonne, 
        // underdiscard below (HAS bm->dt - INCONSISTENT)
        bm->FCtsCarryOver[bm->nbox][tscodiscard_id][sp] * bm->dt * bm->X_CN * mg_2_tonne);
```

**Why this is likely an error:**
- This reports the global totals (using index `bm->nbox`) for both undercatch and underdiscard
- The undercatch uses: `FCtsCarryOver * X_CN * mg_2_tonne`
- The underdiscard uses: `FCtsCarryOver * dt * X_CN * mg_2_tonne`
- These should be calculated consistently since they represent the same type of quantity (carry-over amounts)
- The inclusion of `bm->dt` appears to be a copy-paste error

**Recommendation:** Remove `bm->dt` from line 112-113 to match the undercatch calculation.

---

## Bug 3: Inconsistent array index ordering (Lines 180-181)

**Location:** Lines 180-181 in function `Harvest_Annual_Reset()`

**Issue:** Two related arrays use different index orderings, which could indicate swapped indices.

**Code:**
```c
// Line 180
bm->Catch[b][k][n][ij] = 0.0;

// Line 181
bm->CumCatch[k][n][b][ij] = 0.0;
```

**Context:** These lines are inside nested loops:
- Outer: `for (n = 0; n < bm->K_num_fisheries; n++)` (fishery)
- Middle: `for (k = 0; k < bm->K_num_tot_sp; k++)` (species)
- Inner: `for (b = 0; b < bm->nbox; b++)` (box)
- Innermost: `for (ij = 0; ij < bm->wcnz; ij++)` (water column zone)

**Why this might be an error:**
- `Catch` uses index order: `[box][species][fishery][zone]`
- `CumCatch` uses index order: `[species][fishery][box][zone]`
- These arrays logically represent related quantities (catch and cumulative catch)
- The inconsistent ordering could indicate:
  1. A design choice where arrays are declared differently (possible but unusual)
  2. A bug where indices are swapped on one of these lines
- Without seeing the array declarations, this is suspicious and worth verifying
- If the arrays are meant to have the same structure, one of these lines has indices in the wrong order

**Recommendation:** 
1. Verify the actual array declarations for `Catch` and `CumCatch`
2. If they should have the same structure, correct the index ordering on whichever line is wrong
3. If they are intentionally different, consider adding comments explaining why

---

## Additional Observations

### Potential Off-by-One Context
- Line 202: `TotCumCatch[k][n][bm->thisyear+1] = 0.0;` - Using `thisyear+1` to "ready for the new year". This assumes the array is sized appropriately. Should verify that the array size accounts for this +1 access.

### Variable Reuse
- Lines 84 and 89: `did_printout` is initialized before the species loop (line 84) and then again inside the box loop (line 89). The initialization on line 84 appears redundant since it's overwritten before use.

---

## Summary of Findings

**Critical Issues:**
1. **Lines 100, 112-113**: Inconsistent use of `bm->dt` multiplier in discard calculations (likely copy-paste errors)
2. **Lines 180-181**: Inconsistent array index ordering between related arrays (potential swapped indices)

**Recommended Actions:**
1. Remove `bm->dt` from lines 100 and 112-113 to maintain consistency
2. Verify array declarations and correct index ordering if needed
3. Add bounds checking or comments for the `thisyear+1` array access
