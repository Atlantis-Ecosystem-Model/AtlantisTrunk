# Bug Analysis: ImportExportData.c

## Summary
Found **9 significant bugs** involving swapped array indices, wrong variable names, uninitialized variables, and inconsistent API usage.

---

## Bug 1: Swapped Array Dimensions in columnFractions
**Location:** Line 56  
**Severity:** Critical (Memory corruption / segmentation fault)

```c
double **columnFractions = Util_Alloc_Init_2D_Double(bm->wcnz, bm->nbox, 0.0);
```

**Problem:** The array is allocated as `[wcnz][nbox]` but accessed as `[box][layer]` throughout the code.

**Evidence:**
- Line 103: `columnFractions[b][k]` where `b` is a box index and `k` is a layer index (0 to `bm->boxes[b].nz`)
- Line 130: `tr[k] += boxValues[i] * columnFractions[b][k];`

**Why it's wrong:** The allocation dimensions are swapped. Should be `[nbox][wcnz]` to match the access pattern `[b][k]`.

**Fix:** 
```c
double **columnFractions = Util_Alloc_Init_2D_Double(bm->nbox, bm->wcnz, 0.0);
```

---

## Bug 2: Swapped Array Dimensions in sedColumnFractions
**Location:** Line 57  
**Severity:** Critical (Memory corruption / segmentation fault)

```c
double **sedColumnFractions = Util_Alloc_Init_2D_Double(bm->sednz, bm->nbox, 0.0);
```

**Problem:** Same as Bug 1 - allocated as `[sednz][nbox]` but accessed as `[box][layer]`.

**Evidence:**
- Line 108: `sedColumnFractions[b][k]` where `b` is box index, `k` is sediment layer index
- Line 135: `smtr[k] += boxValues[i] * sedColumnFractions[b][k];`

**Fix:**
```c
double **sedColumnFractions = Util_Alloc_Init_2D_Double(bm->nbox, bm->sednz, 0.0);
```

---

## Bug 3: Swapped Array Dimensions in totalValue
**Location:** Line 58  
**Severity:** Critical (Memory corruption / segmentation fault)

```c
double **totalValue = Util_Alloc_Init_2D_Double(bm->K_num_max_cohort * bm->K_num_max_genetypes, bm->nbox, 0.0);
```

**Problem:** Allocated as `[cohorts][nbox]` but accessed as `[box][cohort]`.

**Evidence:**
- Line 84: `totalValue[b][cohort] = 0.0;`
- Line 87: `totalValue[b][cohort] = bm->boxes[b].epi[bid];`
- Line 93: `totalValue[b][cohort] += bm->boxes[b].tr[k][bid];`
- Line 111: `fprintf(bm->logFile, "totalValue[%d][%d] = %e\n", b, cohort, totalValue[b][cohort]);`

**Fix:**
```c
double **totalValue = Util_Alloc_Init_2D_Double(bm->nbox, bm->K_num_max_cohort * bm->K_num_max_genetypes, 0.0);
```

---

## Bug 4: Uninitialized epi Variable in Second Loop
**Location:** Lines 122-126  
**Severity:** High (Incorrect calculations)

```c
for (i = 0; i < numBoxes; i++) {
    b = boxIndexValues[i];

    Util_Init_1D_Double(tr, bm->boxes[b].nz, 0.0);
    Util_Init_1D_Double(smtr, bm->boxes[b].sm.nz, 0.0);
    if (FunctGroupArray[guild].habitatType == EPIFAUNA) {
        /* bm->boxes[b].epi[bid] += */
        epi += boxValues[i];  // Line 126
```

**Problem:** The `epi` variable is not reset to 0 for each box iteration, causing it to accumulate across all boxes. Compare with lines 122-123 where `tr` and `smtr` are properly reset for each box.

**Why it's wrong:** 
- Line 88 sets `epi = 0` in the *first* loop (lines 78-113)
- Line 126 does `epi += boxValues[i]` accumulating across all boxes in the *second* loop
- Line 144 compares accumulated `epi` with single box's `bm->boxes[b].epi[bid]`
- This means `epi` contains the sum of all processed boxes but is compared against each individual box's value

**Fix:** Add initialization before line 124:
```c
Util_Init_1D_Double(tr, bm->boxes[b].nz, 0.0);
Util_Init_1D_Double(smtr, bm->boxes[b].sm.nz, 0.0);
epi = 0.0;  // ADD THIS LINE
if (FunctGroupArray[guild].habitatType == EPIFAUNA) {
```

---

## Bug 5: Swapped Array Dimensions in wcColumnFractions (Link_SetDetritus)
**Location:** Line 246  
**Severity:** Critical (Memory corruption / segmentation fault)

```c
double **wcColumnFractions = Util_Alloc_Init_2D_Double(bm->wcnz, bm->nbox, 0.0);
```

**Problem:** Same pattern as Bug 1 - allocated as `[wcnz][nbox]` but accessed as `[box][layer]`.

**Evidence:**
- Line 295: `wcColumnFractions[b][k] = totalValue[b] ? (...) : 0.0;`
- Line 312: `... * wcColumnFractions[b][k] / ...`

**Fix:**
```c
double **wcColumnFractions = Util_Alloc_Init_2D_Double(bm->nbox, bm->wcnz, 0.0);
```

---

## Bug 6: Swapped Array Dimensions in sedColumnFractions (Link_SetDetritus)
**Location:** Line 247  
**Severity:** Critical (Memory corruption / segmentation fault)

```c
double **sedColumnFractions = Util_Alloc_Init_2D_Double(bm->sednz, bm->nbox, 0.0);
```

**Problem:** Same as Bug 2 - allocated as `[sednz][nbox]` but accessed as `[box][layer]`.

**Evidence:**
- Line 300: `sedColumnFractions[b][k] = totalValue[b] ? (...) : 0.0;`
- Line 318: `... * sedColumnFractions[b][k] / ...`

**Fix:**
```c
double **sedColumnFractions = Util_Alloc_Init_2D_Double(bm->nbox, bm->sednz, 0.0);
```

---

## Bug 7: Wrong Array Index in Link_SetDetritus
**Location:** Line 312  
**Severity:** Critical (Wrong data / potential segmentation fault)

```c
for (i = 0; i < numBoxes; i++) {
    b = boxIndexValues[i];
    
    if (FunctGroupArray[guild].habitatType != EPIFAUNA) {
        if (FunctGroupArray[guild].habitatCoeffs[WC] > 0) {
            for (k = 0; k < bm->boxes[b].nz; k++) {
                bm->linkageInterface->linkageWCDetritusFlux[b][k][guild] = boxValues[b] / 100 * wcColumnFractions[b][k] / bm->boxes[b].dz[k];
```

**Problem:** Using `boxValues[b]` instead of `boxValues[i]`.

**Why it's wrong:**
- `i` is the iteration index (0 to numBoxes-1) for the input arrays
- `b = boxIndexValues[i]` is the actual box index in the model
- `boxValues` is indexed by the input array position `i`, not the box number `b`
- If `boxIndexValues` contains non-sequential values (e.g., [5, 12, 8]), then `boxValues[b]` will access wrong elements or go out of bounds

**Fix:**
```c
bm->linkageInterface->linkageWCDetritusFlux[b][k][guild] = boxValues[i] / 100 * wcColumnFractions[b][k] / bm->boxes[b].dz[k];
```

---

## Bug 8: Wrong Array Index in Link_SetDetritus (Sediment)
**Location:** Line 318  
**Severity:** Critical (Wrong data / potential segmentation fault)

```c
for (k = 0; k < bm->boxes[b].sm.nz; k++) {
    bm->linkageInterface->linkageSEDDetritusFlux[b][k][guild] = boxValues[b] / 100 * sedColumnFractions[b][k] / bm->boxes[b].sm.dz[k];
```

**Problem:** Same as Bug 7 - using `boxValues[b]` instead of `boxValues[i]`.

**Fix:**
```c
bm->linkageInterface->linkageSEDDetritusFlux[b][k][guild] = boxValues[i] / 100 * sedColumnFractions[b][k] / bm->boxes[b].sm.dz[k];
```

---

## Bug 9: Inconsistent Habitat Check in Link_SetMortality
**Location:** Line 400  
**Severity:** Medium (Logic error / copy-paste mistake)

```c
if (FunctGroupArray[guild].habitatCoeffs[EPIFAUNA] > 0) {
    bm->linkageInterface->linkageEPIMortality[b][guild][cohort] = (boxValues[i] / 100) * bm->boxes[b].epi[bid];
```

**Problem:** Using `habitatCoeffs[EPIFAUNA]` instead of `habitatType == EPIFAUNA`.

**Why it's wrong:** Throughout the entire codebase, epifauna checks use `habitatType == EPIFAUNA`:
- Line 86: `if (FunctGroupArray[guild].habitatType == EPIFAUNA)`
- Line 124: `if (FunctGroupArray[guild].habitatType == EPIFAUNA)`
- Line 143: `if (FunctGroupArray[guild].habitatType == EPIFAUNA)`
- Line 219: `if (FunctGroupArray[guild].habitatType == EPIFAUNA)`
- Line 281: `if (FunctGroupArray[guild].habitatType != EPIFAUNA)`

This is inconsistent and likely a copy-paste error where someone copied the check for `habitatCoeffs[WC]` (line 403) and changed WC to EPIFAUNA without realizing the check type should be different.

**Fix:**
```c
if (FunctGroupArray[guild].habitatType == EPIFAUNA) {
    bm->linkageInterface->linkageEPIMortality[b][guild][cohort] = (boxValues[i] / 100) * bm->boxes[b].epi[bid];
```

---

## Impact Summary

### Critical Bugs (7):
- Bugs 1-3, 5-8: Will cause memory corruption, segmentation faults, or incorrect data access
- These bugs may not immediately crash but will cause subtle data corruption

### High Severity (1):
- Bug 4: Causes incorrect mathematical calculations for EPIFAUNA biomass

### Medium Severity (1):
- Bug 9: May cause incorrect logic flow depending on data structure values

All bugs should be fixed before this code is used in production.
