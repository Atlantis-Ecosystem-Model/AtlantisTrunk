# Bug Analysis for atclassical.c

## Bugs Found

### 1. **Line 163: Wrong variable name**
```c
//fprintf(ofp,"Doing %s with YrMax: %d, ROUNDGUARD: %e, dayt: %e, tassessstart: %e, bracket: %e\n", FunctGroupArray[sp].groupCode, YrMax, ROUNDGUARD, bm->dt, bm->tassessstart, ((bm->dt - bm->tassessstart)/365.0));
```
**Error**: Uses `bm->dt` but should be `bm->dayt`  
**Reason**: Line 159 correctly uses `bm->dayt` in the calculation. The debug print statement should also use `bm->dayt` for consistency. This appears to be a copy-paste error where `dt` (delta-t/timestep) was used instead of `dayt` (day-time).

---

### 2. **Lines 172 and 410: Logical operator error**
**Line 172:**
```c
if ((FunctGroupArray[sp].groupType == BIRD) && (FunctGroupArray[sp].groupType == MAMMAL)) {
```
**Line 410:**
```c
if ((FunctGroupArray[sp].groupType == BIRD) && (FunctGroupArray[sp].groupType == MAMMAL)) {
```
**Error**: Uses `&&` (AND) but should be `||` (OR)  
**Reason**: A group cannot simultaneously be both BIRD and MAMMAL. The condition will always be false. Based on the comment "Current code only allows for survey data of birds and mammals," this should use OR (`||`) to check if the group is either a bird or a mammal.

---

### 3. **Lines 274 and 498: Bootstrap index calculation error**
**Line 274:**
```c
ipnt = (int) floor(ROUNDGUARD + (icnt * drandom(0.0, 1.0) + 1.0));
```
**Line 498:**
```c
ipnt = (int) floor(ROUNDGUARD + (icnt * drandom(0.0, 1.0) + 1.0));
```
**Error**: The `+ 1.0` produces out-of-bounds indices  
**Reason**: `drandom(0.0, 1.0)` returns values in [0, 1). With `icnt * drandom(0.0, 1.0) + 1.0`, the range becomes [1, icnt+1), but array `BootResu` has valid indices [0, icnt-1]. This should be:
```c
ipnt = (int) floor(ROUNDGUARD + (icnt * drandom(0.0, 1.0)));
```
or
```c
ipnt = (int) floor((icnt - 1) * drandom(0.0, 1.0));
```

---

### 4. **Lines 294-297: Off-by-one error in bootstrap loop**
```c
if ((NResult[YrMax][i][nbs + 1] < MAXDOUBLE) && ((i == est_r_id) || (i == est_K_id) || (i == est_B_id))) {
    for (j = 0; j < bootstrap; j++) {
        ResultToSort[j] = NResult[YrMax][i][j];
    }
```
**Error**: Loop bounds mismatch  
**Reason**: The bootstrap loop (line 196) runs from `nbs = -1` to `nbs < bootstrap`, storing results at indices `nbs + 1` (i.e., 0 to bootstrap inclusive). However, line 296 only copies indices 0 to bootstrap-1. Should be:
```c
for (j = 0; j <= bootstrap; j++) {
```
Same issue at **lines 524-527**.

---

### 5. **Line 888: Wrong variable name**
```c
start_N += individVERTinfo[istocknums_id][size_nc][sp][z][sample_id] * stockinfo[sstocknums_id][sp][z][sample_id] * popratio;
```
**Error**: Uses `size_nc` instead of `size_nc_start`  
**Reason**: Line 886 calls `Sort_Length_Weight(..., &size_nc_start, ofp)` which updates `size_nc_start`, not `size_nc`. The variable `size_nc` is uninitialized or has a stale value. Should be:
```c
start_N += individVERTinfo[istocknums_id][size_nc_start][sp][z][sample_id] * ...
```

---

### 6. **Line 906-907: Wrong variable name**
```c
if (end_N && (max_yrs_included < ((end_nc - va95) * sp_AgeClassSize))) {
    max_yrs_included = (end_nc - va95) * sp_AgeClassSize;
}
```
**Error**: Uses `end_nc` instead of `nc_end`  
**Reason**: The loop variable is `nc_end` (line 893), not `end_nc`. The variable `end_nc` is defined on line 874 but is intended for a different purpose. This should be:
```c
if (end_N && (max_yrs_included < ((nc_end - va95) * sp_AgeClassSize))) {
    max_yrs_included = (nc_end - va95) * sp_AgeClassSize;
}
```

---

### 7. **Line 1008: Logic error with all_done flag**
```c
for (ai = 0; ai < sp_AgeClassSize; ai++) {
    all_done = 0;  // <-- This resets the flag every iteration!
    thisai = chrt * sp_AgeClassSize + ai;
    if (thisai > oldest_age) {
        all_done = 1;
        break;
    }
```
**Error**: `all_done` is reset to 0 at the start of each inner loop iteration  
**Reason**: This defeats the purpose of the check on line 1005 (`if (all_done) break;`). The flag gets reset before it's checked. The line `all_done = 0;` on line 1008 should be removed, or the logic needs restructuring.

---

### 8. **Line 1548: Index mismatch**
```c
num_nyr[na][est1_id] = num_nyr[na - 1][est2_id] * exp(-M - V * tempF);
```
**Error**: Uses `est2_id` on the right side but stores to `est1_id`  
**Reason**: For est1_id calculations, it should reference the previous est1_id value. Compare with line 1549 which correctly uses est2_id for both sides. Should be:
```c
num_nyr[na][est1_id] = num_nyr[na - 1][est1_id] * exp(-M - V * tempF);
```

---

### 9. **Lines 1225-1226: Wrong variable in Beverton-Holt recruitment**
**Line 1225:**
```c
num_rec1 = (0.5 * spawn_biom1 * Balpha / (Bbeta + 0.5 * tbiom1)) / (KWRR_sp + KWSR_sp);
```
**Line 1226:**
```c
num_rec2 = (0.5 * spawn_biom2 * Balpha / (Bbeta + 0.5 * tbiom2)) / (KWRR_sp + KWSR_sp);
```
**Error**: Uses `tbiom1` and `tbiom2` instead of `spawn_biom1` and `spawn_biom2`  
**Reason**: Comparing with the pattern elsewhere in the code (e.g., line 1117 uses `spawn_biom1` twice, and line 1572 uses `tbiom` for recruitment denominator), this is inconsistent. Based on Beverton-Holt recruitment formulas, the denominator should typically use spawning biomass, not total biomass. However, this could also be intentional depending on the model specification. Worth verifying against the intended formula.

---

## Summary

**Critical bugs (will cause incorrect behavior):**
- Lines 172, 410: Logic error (AND vs OR)
- Lines 274, 498: Array index out of bounds
- Line 888: Uninitialized/wrong variable
- Line 906-907: Wrong variable name
- Line 1548: Index mismatch
- Line 1008: Logic flag error

**Moderate bugs (likely errors):**
- Line 163: Wrong variable name (but commented out)
- Lines 294-297, 524-527: Off-by-one in loop bounds

**Questionable (may be intentional but worth checking):**
- Lines 1225-1226: Variable choice in recruitment formula

Total: **9-11 distinct bugs found**
