# Bug Analysis: atHarvestCatch.c

## Summary
Found 5 potential bugs including duplicate parameter IDs, inconsistent selectivity modifications, and unused variable computation.

---

## Bug 1: Duplicate Parameter ID in Get_Fishing_Mortality
**Location:** Line 232  
**Severity:** High  
**Type:** Copy-paste error / Wrong parameter

### Issue
```c
mFC_scale = Get_Fishery_Group_Change_Scale(bm, nf,sp, mFC_num_changes_id, mFC_num_changes_id, mFCchange[sp]);
```

The same parameter ID `mFC_num_changes_id` is passed twice to the function. Looking at the function signature:
```c
Get_Fishery_Group_Change_Scale(MSEBoxModel *bm, int nf, int sp, int paramID, int numChangeParamID, double ***changeArray)
```

The function expects:
- `paramID`: flag to check if changes are enabled (should be something like `flagchangemFC_id`)
- `numChangeParamID`: parameter for the number of changes (correctly `mFC_num_changes_id`)

### Expected
```c
mFC_scale = Get_Fishery_Group_Change_Scale(bm, nf, sp, flagchangemFC_id, mFC_num_changes_id, mFCchange[sp]);
```
Or similar flag parameter for the first argument.

### Why it's a bug
The function implementation (lines 119-133) shows that it first checks `flagchange` using the first parameter, then uses the second parameter to get `num_changes`. Using the same parameter ID for both would cause incorrect behavior - it would check if "number of changes" is non-zero (wrong logic) instead of checking if the "change flag" is enabled.

---

## Bug 2: Duplicate Parameter ID in Get_Swept_Area
**Location:** Line 773  
**Severity:** High  
**Type:** Copy-paste error / Wrong parameter

### Issue
```c
SWEPT_scale = Get_Fishery_Change_Scale(bm, nf, flagchangeSWEPT_id, flagchangeSWEPT_id, SWEPTchange);
```

Same issue as Bug 1 - the parameter ID `flagchangeSWEPT_id` is used twice.

Looking at the function signature (lines 134-147):
```c
Get_Fishery_Change_Scale(MSEBoxModel *bm, int nf, int paramID, int numChangeParamID, double ***changeArray)
```

### Expected
```c
SWEPT_scale = Get_Fishery_Change_Scale(bm, nf, flagchangeSWEPT_id, SWEPT_num_changes_id, SWEPTchange);
```

### Why it's a bug
Same reasoning as Bug 1. The second parameter should be the "number of changes" parameter ID, not the flag parameter ID.

---

## Bug 3: Inconsistent addlsm Application in q_bimodal_id Case
**Location:** Lines 708-730  
**Severity:** Medium  
**Type:** Logic inconsistency / Potential copy-paste error

### Issue
In the bimodal selectivity case, the modification logic is inconsistent:

**Original (unmodified) calculation:**
```c
// Line 709-713: First peak - no addlsm
sel_lsm = bm->FISHERYprms[nf][sel_bilsm1_id];

// Line 714: Second peak - WITH addlsm
sel_lsm = bm->FISHERYprms[nf][sel_bilsm2_id] + addlsm;
```

**Modified (changed gear) calculation:**
```c
// Line 719-723: First peak - WITH addlsm
sel_lsm = bm->FISHERYprms[nf][sel_bilsm1_id] + addlsm;

// Line 724-727: Second peak - WITH addlsm
sel_lsm = bm->FISHERYprms[nf][sel_bilsm2_id] + addlsm;
```

### Why it's a bug
In the original calculation, only the second peak gets `addlsm` applied. In the modified calculation, BOTH peaks get `addlsm` applied. This is inconsistent - if you're trying to measure the difference caused by the gear change, you should apply the modification consistently. Either:
1. Both peaks should get it in both calculations (original AND modified), or  
2. Only one peak should get it in the modified calculation

The current code applies it to peak 2 only in original, then to BOTH peaks in modified. This means the difference calculated on line 729 is not just the effect of addlsm on peak 2, but the combined effect on both peaks.

### Expected Pattern
Based on other selectivity cases (logistic, normal, etc.), only the second peak should get the modification:
```c
// First peak - no addlsm
sel_lsm = bm->FISHERYprms[nf][sel_bilsm1_id];
sel_sigma = bm->FISHERYprms[nf][sel_bisigma_id] + addsigma;  // Keep addsigma on line 720
```

---

## Bug 4: Unused Variable Computation in Harvest_How_Much_Fishery_Access
**Location:** Lines 851-852  
**Severity:** Medium  
**Type:** Dead code / Logic error

### Issue
```c
} else if (flagF) {
    if( bm->flag_sel_with_mFC) {
        sel = Get_Selectivity(bm, species, stage, nf, li, sel_curve, 0.0, 0.0);  // Line 851
        ans = 1.0;  // Line 852 - immediately overwrites without using sel!
```

The code recomputes `sel` on line 851 but then immediately sets `ans = 1.0` on line 852 without using the recomputed `sel` value.

### Why it's a bug
1. The `sel` variable computed on line 836 is overwritten on line 851
2. The newly computed `sel` value is never used
3. This suggests either:
   - Line 851 is dead code and should be removed, OR
   - Line 852 should use `sel` in the calculation: `ans = sel;` or similar

### Expected
Likely should be:
```c
if( bm->flag_sel_with_mFC) {
    sel = Get_Selectivity(bm, species, stage, nf, li, sel_curve, 0.0, 0.0);
    ans = sel;  // Use the recomputed value
```

---

## Bug 5: Similar Issue in q_binormal_id Case
**Location:** Lines 731-757  
**Severity:** Low  
**Type:** Potential inconsistency

### Issue
Similar to Bug 3, in the binormal case, both peaks get `addlsm` and `addsigma` applied in the modified calculation:

**Original calculation:**
- Peak 1: No modifications (lines 732-736)
- Peak 2: WITH modifications (lines 737-742)

**Modified calculation:**
- Peak 1: WITH modifications (lines 744-748)  
- Peak 2: WITH modifications (lines 749-754)

### Why it might be a bug
The same inconsistency as Bug 3 - applying modifications to both peaks in the modified version when only one peak had it in the original. However, this case is slightly different because each peak has its own sigma parameter (`sel_bisigma_id` vs `sel_bisigma2_id`), so it might be intentional design for binormal curves.

---

## Recommendations

1. **Priority 1 (High):** Fix Bugs 1 and 2 - these are clear copy-paste errors that will cause incorrect change scaling behavior
2. **Priority 2 (Medium):** Review Bugs 3, 4, and 5 - these may be intentional design decisions, but should be verified with the original developer or through testing
3. Consider adding comments explaining the intended behavior in complex calculations like the bimodal selectivity cases
4. Add assertions or validation to catch parameter ID mismatches at runtime
