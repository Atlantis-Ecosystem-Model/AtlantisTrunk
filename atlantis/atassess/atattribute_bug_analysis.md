# Bug Analysis for atattribute.c

## Bugs Found

### 1. **Swapped Array Indices - Line 906** 🔴 CRITICAL
**Location:** `Fish_Attributes()` function  
**Lines:** 905-906

```c
/* Numbers in the catch */
stockinfo[scatchnums_id][sp][z][attrib_id] += stockinfo[stotcatch_id][sp][z][attrib_id]
    * individVERTinfo[icatchnums_id][sp][i][z][attrib_id] / (individVERTinfo[iweight_id][i][sp][z][attrib_id] + TINY);
```

**Problem:** The indices `[sp][i]` are swapped on line 906. 

**Evidence:**
- The consistent pattern throughout the file for `individVERTinfo` is: `[type][size][species][zone][attrib_id]`
- Line 383: `individVERTinfo[iweight_id][lbin][sp][z][attrib_id]` ✓
- Line 388: `individVERTinfo[istocknums_id][lbin][sp][z][attrib_id]` ✓
- Line 394: `individVERTinfo[icatchnums_id][0][sp][z][attrib_id]` ✓
- Line 906 same statement: `individVERTinfo[iweight_id][i][sp][z][attrib_id]` ✓ (correct order)
- Line 906: `individVERTinfo[icatchnums_id][sp][i][z][attrib_id]` ✗ (WRONG - swapped!)
- Line 911: `individVERTinfo[idiscards_id][i][sp][z][attrib_id]` ✓ (correct on next similar line)
- Line 969: `individVERTinfo[icatchnums_id][i][sp][z][attrib_id]` ✓ (correct elsewhere)

**Should be:**
```c
stockinfo[scatchnums_id][sp][z][attrib_id] += stockinfo[stotcatch_id][sp][z][attrib_id]
    * individVERTinfo[icatchnums_id][i][sp][z][attrib_id] / (individVERTinfo[iweight_id][i][sp][z][attrib_id] + TINY);
```

**Impact:** This will access the wrong array element, leading to incorrect catch number calculations and potential array bounds violations.

---

### 2. **Inverted Logic Condition - Line 389** 🔴 CRITICAL
**Location:** `Charismatic_Attributes()` function  
**Lines:** 366-390

```c
for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
    if(FunctGroupArray[sp].groupType == BIRD){
        for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
            /* Get information for nestlings, newly fledged and adult seabirds */
            switch (chrt) {
            case 0: /* Nestlings */
            case 1: /* Fledglings */
                lbin = chrt;
                break;
            default: /* All adults together */
                lbin = 2;
                break;
            }
            /* ... */
            /* Numbers in stock */
            raw = biolVERTinfo[bstocknums_id][sp][chrt][b];
            rawN = raw;
            individVERTinfo[istocknums_id][lbin][sp][z][attrib_id] += raw;
            if (chrt)
                oldbaby[sp][z][attrib_id] += raw;
```

**Problem:** The condition `if (chrt)` accumulates fledglings and adults (chrt > 0) into `oldbaby`, but based on the switch statement comments, nestlings (chrt == 0) are the actual babies.

**Evidence:**
- Line 371: `case 0: /* Nestlings */` - clearly identifies chrt=0 as babies
- Line 166: For seabirds, old baby calculation uses `individVERTinfo[istocknums_id][0]` (index 0 = nestlings)
- Line 389: `if (chrt)` means "if chrt != 0", which excludes nestlings but includes fledglings and adults

**Should be:**
```c
if (!chrt)  // or: if (chrt == 0)
    oldbaby[sp][z][attrib_id] += raw;
```

**Impact:** Baby bird counts will be incorrect, accumulating the wrong cohorts. This affects recruitment and population dynamics calculations.

---

## Potential Issues (Lower Severity)

### 3. **Confusing Variable Names - Lines 716, 819**
**Location:** `Fisheries_Attributes()` and `Fish_Attributes()` functions

**Lines 716-722:**
```c
for (i = 0; i < bm->K_num_size; i++) {
    for (b = 0; b < bm->K_num_tot_sp; b++) {  // 'b' used for species
        if (FunctGroupArray[b].isVertebrate == TRUE && (FunctGroupArray[b].groupType != BIRD)
                && (FunctGroupArray[b].groupType != MAMMAL)) {
            for (j = 2; j < 4; j++) {
                totnums[j][b][z] += sizebins[j][b][z][i][1][attrib_id];
```

**Lines 818-822:**
```c
for (j = 0; j < bm->nfzones; j++)      // 'j' used for zones
    for (b = 0; b < bm->K_num_tot_sp; b++)  // 'b' used for species
        if (FunctGroupArray[b].isVertebrate == TRUE && (FunctGroupArray[b].groupType != BIRD)
                && (FunctGroupArray[b].groupType != MAMMAL)) {
            biom[b][j][attrib_id] = 0.0;
```

**Problem:** Throughout the codebase:
- `b` is typically used for boxes (e.g., lines 159, 240, 362, 588, 603, 638)
- `sp` or `k` is typically used for species (e.g., lines 99, 122, 138, 256)
- `j` is typically used as a generic inner loop counter
- `z` is typically used for zones

But in these specific sections, `b` is used for species and `j` is used for zones, which is inconsistent and confusing.

**Impact:** While not a functional bug (the code works correctly), this makes the code very difficult to maintain and increases the risk of future bugs. A developer could easily mistake `b` for a box index.

---

## Summary

**Critical Bugs Found: 2**
1. Line 906: Swapped array indices `[sp][i]` → should be `[i][sp]`
2. Line 389: Inverted logic `if (chrt)` → should be `if (!chrt)` or `if (chrt == 0)`

**Code Quality Issues: 1**
3. Lines 716, 819: Inconsistent variable naming conventions

Both critical bugs could lead to incorrect scientific calculations and should be fixed immediately.
