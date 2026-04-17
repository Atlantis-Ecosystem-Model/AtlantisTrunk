# Bug Analysis for atAssessTierSetup.c

## Summary
Found 10 distinct bugs including swapped indices, wrong variable names, type mismatches, and undefined variable usage.

---

## Bug 1: Incorrect File Name in Header Comment
**Location:** Line 2  
**Severity:** Low (documentation error)

```c
*  File:           atlantisutil.c
```

**Issue:** The comment says the file is `atlantisutil.c` but the actual filename is `atAssessTierSetup.c`. This is a copy-paste error.

**Fix:** Should be:
```c
*  File:           atAssessTierSetup.c
```

---

## Bug 2: Integer Array Initialized with Float Value
**Location:** Line 172  
**Severity:** Medium (type mismatch)

```c
bm->RBCestimation.RBCspeciesArray[groupIndex].mirrored_fleet = Util_Alloc_Init_1D_Int(bm->K_num_fisheries, 0.0);
```

**Issue:** Initializing an integer array with a double value `0.0`. The function expects an int but receives a double.

**Fix:** Should be:
```c
bm->RBCestimation.RBCspeciesArray[groupIndex].mirrored_fleet = Util_Alloc_Init_1D_Int(bm->K_num_fisheries, 0);
```

---

## Bug 3: Integer Array Initialized with Float Value
**Location:** Line 183  
**Severity:** Medium (type mismatch)

```c
bm->RBCestimation.RBCspeciesArray[groupIndex].Sel_Pattern = Util_Alloc_Init_1D_Int(bm->K_num_fisheries, 0.0);
```

**Issue:** Same as Bug 2 - initializing integer array with double value.

**Fix:** Should be:
```c
bm->RBCestimation.RBCspeciesArray[groupIndex].Sel_Pattern = Util_Alloc_Init_1D_Int(bm->K_num_fisheries, 0);
```

---

## Bug 4: Suspicious Duplicate Dimension in 4D Array
**Location:** Line 312  
**Severity:** High (likely logic error)

```c
bm->RBCestimation.Catch_by_Metier = Util_Alloc_Init_4D_Double(bm->K_num_fisheries, maxNYears, maxNYears, bm->K_num_tot_sp, 0.0);
```

**Issue:** The array has `maxNYears` repeated twice as consecutive dimensions. This is very likely a copy-paste error. The dimensions appear to be [fisheries][years][years][species], which doesn't make logical sense.

**Expected:** One of those `maxNYears` should probably be a different dimension (perhaps regions, fleets, or another relevant parameter).

---

## Bug 5: Swapped Array Indices for Mnat
**Location:** Lines 1099, 1101  
**Severity:** Critical (index order mismatch)

**Allocation (Line 103):**
```c
bm->RBCestimation.RBCspeciesArray[groupIndex].Mnat = Util_Alloc_Init_5D_Double(nYears, bm->RBCestimation.OverallMaxAge, bm->K_num_sexes, bm->K_num_reg, bm->K_num_stocks_per_sp, 0.0);
```
Order: `[nYears][OverallMaxAge][K_num_sexes][K_num_reg][K_num_stocks_per_sp]`

**Access (Lines 1099, 1101):**
```c
bm->RBCestimation.RBCspeciesArray[species].Mnat[j][r][s][a][yr1] = ...
bm->RBCestimation.RBCspeciesArray[species].Mnat[j][r][s][a][t] = ...
```
Access order: `[j][r][s][a][yr1]` = `[stock][region][sex][age][year]`

**Issue:** The indices are completely swapped! The allocation order is [year][age][sex][region][stock] but access order is [stock][region][sex][age][year].

**Fix:** Should be accessed as:
```c
bm->RBCestimation.RBCspeciesArray[species].Mnat[yr1][a][s][r][j] = ...
bm->RBCestimation.RBCspeciesArray[species].Mnat[t][a][s][r][j] = ...
```

---

## Bug 6: Swapped Array Indices for MeanWtAge
**Location:** Lines 1124, 1136  
**Severity:** Critical (index order mismatch)

**Allocation (Line 101):**
```c
bm->RBCestimation.RBCspeciesArray[groupIndex].MeanWtAge = Util_Alloc_Init_4D_Double(nYears+2, bm->RBCestimation.OverallMaxAge, bm->K_num_sexes, bm->K_num_stocks_per_sp, 0.0);
```
Order: `[nYears+2][OverallMaxAge][K_num_sexes][K_num_stocks_per_sp]`

**Access (Lines 1124, 1136):**
```c
bm->RBCestimation.RBCspeciesArray[species].MeanWtAge[j][s][a][yr1] = ...
bm->RBCestimation.RBCspeciesArray[species].MeanWtAge[j][s][a][t] = ...
```
Access order: `[j][s][a][yr1]` = `[stock][sex][age][year]`

**Issue:** The indices are swapped! Allocation is [year][age][sex][stock] but access is [stock][sex][age][year].

**Fix:** Should be accessed as:
```c
bm->RBCestimation.RBCspeciesArray[species].MeanWtAge[yr1][a][s][j] = ...
bm->RBCestimation.RBCspeciesArray[species].MeanWtAge[t][a][s][j] = ...
```

---

## Bug 7: Use of Undefined Variable
**Location:** Line 1402  
**Severity:** Critical (undefined variable)

```c
for (m = 0; m < Nfleet; m++) {
    mid = bm->RBCestimation.speciesRPFleetToMetier[m][groupIndex];
    bm->RBCestimation.speciesMetierToRPFleet[groupIndex][mid] = m;
    strcpy(bm->RBCestimation.metierArray[mid].metierCode, FisheryArray[m].fisheryCode);
    bm->RBCestimation.metierArray[mid].PGMSYlinks = PGMSYLinksInputs[mid];
}
```

**Issue:** Variable `Nfleet` is used but not defined until line 1409:
```c
Nfleet = bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id];
```

**Fix:** Move the `Nfleet` definition before its usage, or use the already available value from line 1381 where `Nfleet` may have been intended to be defined earlier.

---

## Bug 8: Wrong Variable Name in Array Access
**Location:** Line 1428  
**Severity:** High (wrong variable)

```c
for (n = 0; n < bm->K_num_fisheries; n++){
    if(!FunctGroupArray[groupIndex].isTAC || (bm->TACamt[groupIndex][nf][now_id] < no_quota)) {
        totTAC += bm->TACamt[groupIndex][n][now_id];
    }
    ...
}
```

**Issue:** Inside the loop with index variable `n`, the code uses `nf` (which is from a different context, line 1412) instead of `n` for array indexing.

**Fix:** Should be:
```c
if(!FunctGroupArray[groupIndex].isTAC || (bm->TACamt[groupIndex][n][now_id] < no_quota)) {
```

---

## Bug 9: Swapped Array Indices for TriggerPoints
**Location:** Lines 1489, 1510  
**Severity:** Critical (index order mismatch)

**Allocation (Line 213):**
```c
bm->RBCestimation.RBCspeciesArray[groupIndex].TriggerPoints = Util_Alloc_Init_2D_Double(bm->K_num_reg+1, Ntriggers, 0.0);
```
Order: `[K_num_reg+1][Ntriggers]` = `[regions][triggers]`

**Access (Lines 1489, 1510):**
```c
bm->RBCestimation.RBCspeciesArray[groupIndex].TriggerPoints[nt][Nregions] = TrigPts[nt];
bm->RBCestimation.RBCspeciesArray[groupIndex].TriggerPoints[nt][nreg] = ...
```
Access order: `[nt][Nregions]` and `[nt][nreg]` = `[trigger][region]`

**Issue:** The indices are swapped! Allocation is [region][trigger] but access is [trigger][region].

**Fix:** Should be accessed as:
```c
bm->RBCestimation.RBCspeciesArray[groupIndex].TriggerPoints[Nregions][nt] = TrigPts[nt];
bm->RBCestimation.RBCspeciesArray[groupIndex].TriggerPoints[nreg][nt] = ...
```

---

## Bug 10: Swapped Array Indices for p_al
**Location:** Lines 341 (allocation), 920, 927, 935, 986, 987 (access)  
**Severity:** Critical (index order mismatch throughout)

**Allocation (Line 341):**
```c
bm->RBCestimation.RBCspeciesArray[groupIndex].p_al = Util_Alloc_Init_2D_Double(Nlen, maxA, 0.0);
```
Order: `[Nlen][maxA]` = `[length][age]`

**Access in Prop_LatA function (Lines 920, 927, 935):**
```c
p_al[a][0] = accum;
p_al[a][l] = integral - accum;
p_al[a][nlen] = 1.0 - accum;
```

**Access in YPRlen function (Lines 986, 987):**
```c
yp += bm->RBCestimation.RBCspeciesArray[species].yield[a] * p_al[a][l];
ypl += bm->RBCestimation.RBCspeciesArray[species].yield[a] * p_al[a][l] * ...
```

**Issue:** The array is allocated as [length][age] but accessed consistently as [age][length] throughout the code. This is a systematic index swap error.

**Fix:** Either change allocation to:
```c
bm->RBCestimation.RBCspeciesArray[groupIndex].p_al = Util_Alloc_Init_2D_Double(maxA, Nlen, 0.0);
```

Or change all accesses from `p_al[a][l]` to `p_al[l][a]`.

---

## Conclusion

The most critical bugs are the swapped array indices (Bugs 5, 6, 9, 10) which will cause memory access violations or incorrect data storage/retrieval. Bug 7 (undefined variable) will cause compilation errors or runtime crashes. Bug 8 (wrong variable) will cause incorrect array accesses. The duplicate dimension in Bug 4 suggests a logic error in the data structure design.
