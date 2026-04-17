# Bug Analysis for atwriting.c

## Summary
This document lists potential bugs found in `atwriting.c`, focusing on subtle issues like swapped indices, off-by-one errors, wrong variable names, and copy-paste mistakes.

---

## CRITICAL BUGS

### Bug 1: Index Inconsistency in Write_Morts() - Lines 2517-2521
**Severity:** HIGH - Potential array access mismatch

**Location:** Function `Write_Morts()`, lines 2517-2526

**Code:**
```c
for (sp = 1; sp < bm->K_num_tot_sp + 1; sp++) {
    if (FunctGroupArray[sp - 1].isAssessed == TRUE) {
        if (FunctGroupArray[sp - 1].isVertebrate == TRUE || FunctGroupArray[sp - 1].isDetritus == TRUE) {
            fprintf(fid, "%.8g ", mortnetwk[sp][0][z]);
            fprintf(fid, "%.8g ", mortnetwk[sp][1][z]);
        } else {
            fprintf(fid, "%.8g ", mortnetwk[sp][0][z]);
        }
    }
}
```

**Issue:** 
The loop variable `sp` ranges from 1 to `K_num_tot_sp` (inclusive), and accesses `FunctGroupArray[sp - 1]` (indices 0 to K_num_tot_sp-1, which is correct for the FunctGroupArray). However, it then accesses `mortnetwk[sp]` (indices 1 to K_num_tot_sp). This creates an inconsistency - if mortnetwk is 0-indexed like most C arrays, this skips mortnetwk[0] and accesses one element beyond the intended range.

**Why this is an error:**
1. All other species loops in this file use 0-based indexing: `for (sp = 0; sp < bm->K_num_tot_sp; sp++)`
2. The FunctGroupArray is clearly 0-indexed (accessed as `sp - 1`)
3. This pattern suggests mortnetwk should also be accessed as `mortnetwk[sp - 1]` for consistency
4. The current code will skip mortnetwk[0] entirely and potentially access mortnetwk[K_num_tot_sp] which could be out of bounds

**Expected behavior:**
The loop should either:
- Start at `sp = 0` and access both arrays consistently as `FunctGroupArray[sp]` and `mortnetwk[sp]`
- OR keep the current loop but access `mortnetwk[sp - 1]` to match the FunctGroupArray access pattern

---

### Bug 2: Swapped Indices in Write_Abcs() - Lines 1698-1704
**Severity:** MEDIUM - Data written to wrong columns

**Location:** Function `Write_Abcs()`, lines 1698-1704

**Code:**
```c
for (j = 0; j < bm->K_num_abcbin; j++)
    /* Write Biomass Info */
    fprintf(fid, "%.8g ", endnums[j][1][z]);

for (j = 0; j < bm->K_num_abcbin; j++)
    /* Write Abundance Info */
    fprintf(fid, "%.8g ", endnums[j][0][z]);
```

**Issue:** 
The second index appears to be swapped. Comparing with similar array accesses in the same file:

- In `Write_Binbioms()` (line 1566): `pelbin[j][0][z]` writes biomass
- In `Write_Bin_Abunds()` (line 1619): `pelbin[j][1][z]` writes abundance

This establishes a pattern where **index 0 = biomass, index 1 = abundance**.

However, in `Write_Abcs()`:
- `endnums[j][1][z]` is labeled as biomass (should be index 0)
- `endnums[j][0][z]` is labeled as abundance (should be index 1)

The same pattern appears in lines 1710 and 1719 with `endnumsbig`:
```c
fprintf(fid, "%.8g ", endnumsbig[index][1][z]);  // After "Write Biomass Info" comment
...
fprintf(fid, "%.8g ", endnumsbig[index][0][z]);  // After "Write Abundance Info" comment
```

**Why this is an error:**
Either the indices are swapped (should be 0 for biomass, 1 for abundance) OR the comments are wrong. Given the consistent pattern throughout the file, the indices are likely wrong.

**Expected behavior:**
Should be:
```c
for (j = 0; j < bm->K_num_abcbin; j++)
    /* Write Biomass Info */
    fprintf(fid, "%.8g ", endnums[j][0][z]);  // Changed 1 to 0

for (j = 0; j < bm->K_num_abcbin; j++)
    /* Write Abundance Info */
    fprintf(fid, "%.8g ", endnums[j][1][z]);  // Changed 0 to 1
```

---

## POTENTIAL BUGS (Require Verification)

### Bug 3: Possible Off-by-One in Write_TrophSpec() - Line 2192
**Severity:** MEDIUM - Potential array overflow

**Location:** Function `Write_TrophSpec()`, line 2192

**Code:**
```c
for (z = 0; z < bm->nfzones + 2; z++) {
    for (k = 0; k < 2; k++) {
        // ... writes trophspect[j][k][z]
```

**Issue:**
This loop iterates `z` from 0 to `nfzones + 1` (inclusive), accessing `trophspect[j][k][z]`. However, most other similar loops in the file use `z < bm->nfzones + 1`, iterating from 0 to `nfzones` (inclusive). Examples:
- Line 1445, 1502, 1556, 1609, 1690, 1762, 1811, 1862, 1929, 1989, 2040, 2093, 2142, 2313, 2363, 2508, 2566, 2617: all use `z < bm->nfzones + 1`

**Why this might be an error:**
If the `trophspect` array is allocated with size `[...][...][nfzones+1]` like most other similar arrays, this would cause an off-by-one buffer overflow, accessing `trophspect[...][...][nfzones+1]` when the maximum valid index is `nfzones`.

**Why this might be correct:**
The `trophspect` array might be intentionally allocated with size `[...][...][nfzones+2]` to accommodate an extra zone for aggregated data.

**Verification needed:**
Check the allocation of the `trophspect` array to determine if it has size `nfzones+1` or `nfzones+2`.

---

### Bug 4: Reverse Order Loop - Line 941
**Severity:** LOW - Possible unintended order

**Location:** Function `Write_Physs()`, lines 941-942

**Code:**
```c
for (j = 2; j >= 0; j--)
    fprintf(fid, "%.8g ", sampleprocess[j][z][id]);
```

**Issue:**
This loop writes the `sampleprocess` array in reverse order (indices 2, 1, 0). This is immediately after a forward loop at lines 939-940 that writes `samplephys` in forward order (0, 1, 2, 3, 4, 5).

**Why this might be an error:**
The reverse ordering seems unusual and might be a copy-paste mistake where someone forgot to change the loop direction.

**Why this might be correct:**
The ordering could be intentional based on the specific data representation requirements for this output file.

**Verification needed:**
Verify with documentation or output file specification whether the reverse order is intentional.

---

### Bug 5: Potential Index Mismatch in Write_Stock_Values() - Line 256
**Severity:** LOW - Questionable parameter design

**Location:** Function `Write_Stock_Values()`, lines 248-261

**Code:**
```c
static void Write_Stock_Values(MSEBoxModel *bm, FILE *fid, int z, int id, int paramID) {
    int sp;

    if (paramID == sbiomass_id) {
        quit("Cannot function Write_Stock_Values use with sbiomass_id");
    }
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].isVertebrate == TRUE) {
            fprintf(fid, "%.8g ", stockinfo[paramID][sp][z][id]);

        }
    }

}
```

**Issue:**
The function explicitly prevents using `sbiomass_id` as a paramID, but at line 599 in `Write_StockNums()`, the code writes biomass using a different approach:
```c
fprintf(fid, "%.8g ", stockinfo[sbiomass_id][sp][z][id] * mg_2_tonne / area_correct);
```

This suggests there might be special handling needed for biomass, but it's handled outside this function. The prohibition seems odd.

**Why this might be an error:**
The error message has awkward grammar ("Cannot function Write_Stock_Values use") suggesting it might have been hastily added or is a placeholder. The prohibition might be unnecessary or indicate incomplete refactoring.

**Why this might be correct:**
The biomass calculation requires special unit conversion and area correction that doesn't apply to other parameters, so the prohibition is intentional.

---

## STYLE ISSUES (Not bugs, but worth noting)

### Issue 1: Inconsistent Loop Variable Names
Throughout the file, nested loops sometimes reuse variable names in non-conflicting scopes, which can be confusing:
- `j` is used for different purposes in different functions
- Sometimes species index is `sp`, sometimes `pred`, sometimes `prey`

### Issue 2: Magic Numbers
Line 2175 has hardcoded column headers that could get out of sync with actual data:
```c
fprintf(fid, "#Time Zone StaticTL bin1 bin15 bin2 bin25 bin3 bin35 bin4 bin45 bin5 bin55 bin6 bin65 bin7 bin75 bin8 bin85 bin9 bin95 bin10 #\n");
```

---

## SUMMARY OF FINDINGS

**Critical Issues:**
1. **Line 2517-2521**: Index inconsistency in `Write_Morts()` - `mortnetwk[sp]` vs `FunctGroupArray[sp-1]`
2. **Lines 1698-1704**: Swapped indices in `Write_Abcs()` - biomass/abundance confusion

**Issues Requiring Verification:**
3. **Line 2192**: Possible off-by-one in `Write_TrophSpec()` - `nfzones + 2` vs `nfzones + 1`
4. **Line 941**: Reverse order loop in `Write_Physs()` - intentional or error?

**Total Bugs Found:** 2 definite, 2 probable (pending verification)
