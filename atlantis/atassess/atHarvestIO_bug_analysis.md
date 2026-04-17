# Bug Analysis: atHarvestIO.c

## Confirmed Bugs

### 1. Wrong Function Name in Error Message (Copy-Paste Error)
**Location:** Line 267  
**Function:** [`initAnnCatchPerFisheryFile()`](../atharvest/atHarvestIO.c:256)  
**Issue:** The error message incorrectly refers to "initAnnCatchFile" instead of "initAnnCatchPerFisheryFile"

```c
if ((fid = Util_fopen(bm, fname, "w")) == NULL)
    quit("initAnnCatchFile: Can't open %s\n", fname);  // WRONG FUNCTION NAME
```

**Why this is an error:** This is a classic copy-paste mistake. The error message should identify the actual function where the error occurred for proper debugging. When this error is triggered, it will mislead developers about where the file opening failed.

**Fix:** Change to:
```c
quit("initAnnCatchPerFisheryFile: Can't open %s\n", fname);
```

---

### 2. Wrong Function Name in Error Message (Copy-Paste Error)
**Location:** Line 299  
**Function:** [`initAnnDiscardPerFisheryFile()`](../atharvest/atHarvestIO.c:286)  
**Issue:** The error message incorrectly refers to "initAnnDiscardFile" instead of "initAnnDiscardPerFisheryFile"

```c
if ((fid = Util_fopen(bm, fname, "w")) == NULL)
    quit("initAnnDiscardFile: Can't open %s\n", fname);  // WRONG FUNCTION NAME
```

**Why this is an error:** Same as above - this is a copy-paste mistake that will mislead debugging efforts if a file cannot be opened.

**Fix:** Change to:
```c
quit("initAnnDiscardPerFisheryFile: Can't open %s\n", fname);
```

---

### 3. Unused Variable Declaration
**Location:** Line 559  
**Function:** [`writeDisplaceEffort()`](../atharvest/atHarvestIO.c:557)  
**Issue:** Variable `toteffort` is declared but never used

```c
void writeDisplaceEffort(FILE *fid, MSEBoxModel *bm, FILE *llogfp) {
    int nf, b;
    double toteffort = 0;  // DECLARED BUT NEVER USED
```

**Why this is an error:** This appears to be copy-paste residue from [`writeAnnEffort()`](../atharvest/atHarvestIO.c:534) (lines 534-555), which does use `toteffort` to accumulate effort across boxes. The variable serves no purpose in `writeDisplaceEffort()` and suggests incomplete adaptation of copied code.

**Fix:** Remove the unused variable:
```c
void writeDisplaceEffort(FILE *fid, MSEBoxModel *bm, FILE *llogfp) {
    int nf, b;
    // toteffort removed as it's not used
```

---

## Potentially Suspicious Code

### 4. Possible Index Order Inconsistency
**Location:** Line 570  
**Function:** [`writeDisplaceEffort()`](../atharvest/atHarvestIO.c:557)  
**Issue:** Potential index swap in array access compared to similar code in [`writeAnnEffort()`](../atharvest/atHarvestIO.c:534)

**Comparison:**
- Line 548 in `writeAnnEffort()`: `bm->CumEffort[nf][b]` - indexed as [fishery][box]
- Line 570 in `writeDisplaceEffort()`: `bm->CumDisplaceEffort[b][nf]` - indexed as [box][fishery]

```c
// In writeAnnEffort (line 548):
toteffort += bm->CumEffort[nf][b];

// In writeDisplaceEffort (line 570):
fprintf(fid, " %f", bm->CumDisplaceEffort[b][nf]);
```

**Why this might be an error:** The inconsistent index ordering between similar arrays (`CumEffort` vs `CumDisplaceEffort`) could indicate a copy-paste error where indices were not properly adjusted. However, this could also be intentional if the arrays are dimensioned differently in the struct definition.

**To verify:** Check the `MSEBoxModel` struct definition to confirm whether:
- `CumEffort` is declared as `[K_num_fisheries][nbox]`
- `CumDisplaceEffort` is declared as `[nbox][K_num_fisheries]` or vice versa

If both arrays use the same dimension order, then line 570 has swapped indices and should be:
```c
fprintf(fid, " %f", bm->CumDisplaceEffort[nf][b]);
```

**Note:** Without access to the struct definition, I cannot definitively confirm this is a bug, but the inconsistency is suspicious and warrants investigation.

---

## Summary

**Confirmed bugs:** 3
- 2 copy-paste errors in error messages (lines 267, 299)
- 1 unused variable declaration (line 559)

**Needs verification:** 1
- Possible index swap at line 570 (requires struct definition to confirm)

These are all subtle bugs that would not cause compilation errors but could lead to confusion during debugging (wrong error messages), code clutter (unused variables), or potentially runtime errors (if indices are indeed swapped).
