# Bug Analysis: atexternalpop.c

## Errors Found

### 1. **CRITICAL BUG: Wrong variable used in age comparison (Line 192)**

**Location:** Line 192
```c
if (cohort < age_mat) {
```

**Issue:** This compares `cohort` (an array index/cohort number) with `age_mat` (an age value in years). This is a type mismatch - comparing an index with a biological age value.

**Evidence:**
- Line 68: `age_mat = FunctGroupArray[species].speciesParams[age_mat_id];` - age_mat is loaded as a species parameter representing maturation age in years
- Line 91: `age = ageclasssize * cohort + ageclasssize / 2.0;` - shows that age is calculated from cohort index
- Line 108: `age = ageclasssize * cohort + prop_year_elapsed;` - alternative age calculation from cohort index

**Why it's wrong:** If `age_mat = 3.0` years and `ageclasssize = 0.5` years, then an animal in cohort 7 would have age = 3.5 years (adult), but the comparison `cohort < age_mat` would be `7 < 3.0` = FALSE, incorrectly classifying it as adult. Conversely, cohort 2 (age = 1.0 year, juvenile) would pass `2 < 3.0` = TRUE, correctly classifying as juvenile. The logic is backwards and scale-dependent.

**Correct code should be:**
```c
if (age < age_mat) {
```

**Impact:** This causes incorrect mortality parameters (mLext and mPext) to be applied - juveniles may get adult mortality rates and vice versa, depending on the cohort numbering scheme and age_mat value.

---

### 2. **Minor Issue: Duplicate variable in debug format string (Line 157-158)**

**Location:** Lines 157-158 (commented out debug code)
```c
fprintf(bm->logFile, "growth Kbert: %e, linf: %e, Length: %e, Kbert: %e, tzero: %e ",
    Kbert, linf, Length, Kbert, tzero);
```

**Issue:** The format string includes `Kbert` twice - once as "growth Kbert: %e" and again as "Kbert: %e". The variable `Kbert` is printed twice.

**Why it's suspicious:** This looks like a copy-paste error. The first instance probably should be a different variable, or one should be removed.

**Impact:** Low - this is commented-out debug code, so it doesn't affect runtime behavior, but if uncommented would print confusing/redundant output.

---

## Summary

**Critical Bugs: 1**
- Line 192: Comparing cohort index with age value instead of comparing age with age_mat

**Minor Issues: 1**  
- Lines 157-158: Duplicate variable in commented debug output (copy-paste mistake)

The main bug at line 192 is a serious logic error that would cause incorrect classification of juveniles vs adults for mortality calculations.
