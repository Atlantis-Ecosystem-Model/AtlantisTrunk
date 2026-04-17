# Bug Analysis: atHarvestDiscards.c

## Bugs Found

### 1. **Line 170: Inconsistent Index Check**
**Location:** Line 170
```c
if (FunctGroupArray[guildcase].groupAgeType == AGE_STRUCTURED || FunctGroupArray[ts_id].groupAgeType == AGE_STRUCTURED_BIOMASS)
```
**Issue:** The left side of the OR checks `FunctGroupArray[guildcase]` but the right side checks `FunctGroupArray[ts_id]`. This is inconsistent.

**Expected:** Both should check the same species index. Should likely be:
```c
if (FunctGroupArray[guildcase].groupAgeType == AGE_STRUCTURED || FunctGroupArray[guildcase].groupAgeType == AGE_STRUCTURED_BIOMASS)
```
**Impact:** May apply incorrect age scaling when imposing discards.

---

### 2. **Line 384: Duplicate Condition (Copy-Paste Error)**
**Location:** Line 384
```c
if (bm->newmonth && ((bm->dt < 43200.0 ) || (bm->dt < 43200.0 )))
```
**Issue:** The condition `bm->dt < 43200.0` is checked twice with OR. This is clearly a copy-paste error.

**Expected:** One condition should likely check `bm->dt > 43200.0` or a different value. Based on the context (checking if dt is between 12-24 hours), it should probably be:
```c
if (bm->newmonth && ((bm->dt < 43200.0 ) || (bm->dt > 86400.0 )))
```
**Impact:** The warning about dt mismatch may not trigger when it should.

---

### 3. **Line 390: Wrong Array Indices**
**Location:** Line 390
```c
if(bm->FCcocatch[i][i] > 0){
```
**Issue:** Inside a loop over species `i` (line 388), this checks the diagonal element `FCcocatch[i][i]`. However, the function is calculating dependent discards for species `sp`, and later at line 423 it correctly uses `FCcocatch[sp][i]`. The comment at line 420-422 confirms this should be checking catch of species `sp` due to catch of species `i`.

**Expected:**
```c
if(bm->FCcocatch[sp][i] > 0){
```
**Impact:** May skip dependent discard calculations when the target species has non-zero cocatch with species i, but species i doesn't self-cocatch.

---

### 4. **Line 442: Wrong Variable Name**
**Location:** Line 442 in `Calc_Dependent_Discards_For_Other_Species()`
```c
int ibasechrt = chrt / FunctGroupArray[i].numGeneTypes;
```
**Issue:** The function has parameters `int ichrt` (cohort of species i) and `int chrt` (cohort of base species sp). This line should use `ichrt` since it's calculating the base cohort for species `i`, not the base species.

Looking at the caller (line 340):
```c
step1 = Calc_Dependent_Discards_For_Other_Species(bm, i, icohort, guildcase, chrt, nf, vert_scale, base_catch_sp, llogfp);
```
- Parameter `ichrt` receives `icohort` (cohort of species i)
- Parameter `chrt` receives `chrt` (cohort of guildcase/sp)

**Expected:**
```c
int ibasechrt = ichrt / FunctGroupArray[i].numGeneTypes;
```
**Impact:** Uses wrong cohort number for age scaling, causing incorrect discard calculations for other species.

---

### 5. **Line 559: Logic Error with AND vs OR**
**Location:** Line 559
```c
if (!age_scale_discard && !age_scale_catch)  // This size isn't caught so it can't be discarded
    return 0;
```
**Issue:** This returns 0 only if BOTH `age_scale_discard` AND `age_scale_catch` are zero. However, if either one is zero, no discarding should occur. The comment says "This size isn't caught so it can't be discarded" - if `age_scale_catch` is 0, there's no catch to discard.

**Expected:**
```c
if (!age_scale_discard || !age_scale_catch)  // This size isn't caught so it can't be discarded
    return 0;
```
Or more clearly:
```c
if (age_scale_discard == 0.0 || age_scale_catch == 0.0)
    return 0;
```
**Impact:** May attempt to calculate discards for cohorts that aren't actually caught or don't have discard scaling.

---

### 6. **Multiple Lines: Typo in Debug Variable Name**
**Locations:** Lines 82, 146, 217, 239, 258, 270, 283, 295, 308
```c
fprintf(llogfp, "... loatDetFC ...", ...);  // Should be "loadDetFC"
```
**Issue:** Debug statements use `loatDetFC` instead of `loadDetFC` (missing 'd').

**Expected:** Change all instances to `loadDetFC`

**Impact:** Minor - only affects debug output readability, doesn't affect calculations.

---

### 7. **Line 746: Inconsistent Pattern (Possible Logic Error)**
**Location:** Line 746
```c
bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase] = 0;
```
**Issue:** In the `adjacent_impose` case, this sets the catch carryover to 0 directly, while everywhere else the code subtracts values. This is followed by line 747 which calculates discard carryover as a proportion of the original value (which is now 0).

Looking at the similar code block at lines 654-657 (in the biomCarryOver > step1 branch), the catch carryover is decremented, not zeroed. The pattern at line 746 breaks this consistency.

**Expected:** Should probably subtract the used amount:
```c
bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase] -= biomCarryOver * bm->cell_vol
    * bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase] / (biomCarryOver + small_num);
```
(Similar to line 747 for discards)

**Impact:** May incorrectly reset catch carryover to zero instead of decreasing it proportionally.

---

## Summary

**Critical Bugs:**
1. Line 390: Wrong array indices - checks `FCcocatch[i][i]` instead of `FCcocatch[sp][i]`
2. Line 442: Wrong variable - uses `chrt` instead of `ichrt`
3. Line 170: Inconsistent species index in condition
4. Line 559: Wrong logical operator (AND instead of OR)

**Moderate Bugs:**
5. Line 384: Duplicate condition (copy-paste error)
6. Line 746: Inconsistent pattern for carryover handling

**Minor Issues:**
7. Multiple lines: Typo in debug output (`loatDetFC` vs `loadDetFC`)
