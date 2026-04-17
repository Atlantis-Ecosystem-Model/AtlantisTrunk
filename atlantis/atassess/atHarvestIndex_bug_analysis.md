# Bug Analysis: atHarvestIndex.c

## CRITICAL BUGS FOUND

### Bug 1: Incorrect Loop Nesting for CumDiscards - Line 151
**Location:** Line 151 in `Update_Harvest_Index_Values()`

**Code:**
```c
for (ij = 0; ij < bm->boxes[b].nz; ij++) {
    /* Total landings */
    harvestindx[nf][tot_land_id] += bm->CumCatch[sp][nf][b][ij] * bm->X_CN * mg_2_tonne;
```

**Problem:**
Line 151 is:
```c
harvestindx[nf][tot_trash_id] += bm->CumDiscards[sp][nf][b] * bm->X_CN * mg_2_tonne;
```

This line is **inside** the `ij` loop (which starts at line 152), but `bm->CumDiscards[sp][nf][b]` is a 3D array that does NOT have an `ij` dimension (vertical layer index). This means the same discard value is being added to the total multiple times - once for each vertical layer `ij` in box `b`.

**Why it's a bug:** The total discards are being overcounted by a factor of `bm->boxes[b].nz` for each box. If a box has 5 vertical layers, the discards are counted 5 times instead of once.

**Fix:** Line 151 should be moved outside the `ij` loop but remain inside the `b` loop.

---

### Bug 2: Incorrect Loop Nesting for TEP Kill Discards (Cute Species) - Line 161
**Location:** Line 161 in `Update_Harvest_Index_Values()`

**Code:**
```c
if (is_cute) {
    if (FunctGroupArray[sp].isFished == TRUE)
        harvestindx[nf][tepkill_id] += bm->tepcoefft * bm->CumCatch[sp][nf][b][ij] * bm->X_CN * mg_2_tonne;
    if (FunctGroupArray[sp].isImpacted == TRUE)
        harvestindx[nf][tepkill_id] += bm->tepcoefft * bm->CumDiscards[sp][nf][b] * bm->X_CN * mg_2_tonne;
}
```

**Problem:**
Line 161 uses `bm->CumDiscards[sp][nf][b]` (without `ij` dimension) but is inside the `ij` loop (line 152). The discard contribution to TEP kills is being counted multiple times for each vertical layer.

**Why it's a bug:** Same as Bug 1 - overcounting by a factor of `bm->boxes[b].nz` per box.

**Fix:** Line 161 should be moved outside the `ij` loop but remain inside the `b` loop.

---

### Bug 3: Incorrect Loop Nesting for TEP Kill Discards (Sharks) - Line 167
**Location:** Line 167 in `Update_Harvest_Index_Values()`

**Code:**
```c
if (is_shark) {
    if (FunctGroupArray[sp].isFished == TRUE)
        harvestindx[nf][tepkill_id] += bm->sharkcoefft * bm->CumCatch[sp][nf][b][ij] * bm->X_CN * mg_2_tonne;
    if (FunctGroupArray[sp].isImpacted == TRUE)
        harvestindx[nf][tepkill_id] += bm->sharkcoefft * bm->CumDiscards[sp][nf][b] * bm->X_CN * mg_2_tonne;
}
```

**Problem:**
Line 167 uses `bm->CumDiscards[sp][nf][b]` (without `ij` dimension) but is inside the `ij` loop (line 152).

**Why it's a bug:** Same issue - discards are being counted once per vertical layer instead of once per box.

**Fix:** Line 167 should be moved outside the `ij` loop but remain inside the `b` loop.

---

### Bug 4: Incorrect Loop Nesting for Habitat Kill Discards - Line 173
**Location:** Line 173 in `Update_Harvest_Index_Values()`

**Code:**
```c
if (is_hab) {
    if (FunctGroupArray[sp].isFished == TRUE)
        harvestindx[nf][habkill_id] += bm->CumCatch[sp][nf][b][ij] * bm->X_CN * mg_2_tonne;
    if (FunctGroupArray[sp].isImpacted == TRUE)
        harvestindx[nf][habkill_id] += bm->CumDiscards[sp][nf][b] * bm->X_CN * mg_2_tonne;
}
```

**Problem:**
Line 173 uses `bm->CumDiscards[sp][nf][b]` (without `ij` dimension) but is inside the `ij` loop (line 152).

**Why it's a bug:** Same overcounting issue as the previous bugs.

**Fix:** Line 173 should be moved outside the `ij` loop but remain inside the `b` loop.

---

## Summary

All four bugs have the same root cause: **Incorrect loop nesting**. 

The problematic structure (lines 149-176) is:
```c
for (nf = 0; nf < bm->K_num_fisheries; nf++) {
    for (b = 0; b < bm->nbox; b++) {
        /* Line 151: WRONG - CumDiscards[sp][nf][b] inside ij loop */
        harvestindx[nf][tot_trash_id] += bm->CumDiscards[sp][nf][b] * ...;
        
        for (ij = 0; ij < bm->boxes[b].nz; ij++) {
            /* Line 154: CORRECT - CumCatch[sp][nf][b][ij] has ij dimension */
            harvestindx[nf][tot_land_id] += bm->CumCatch[sp][nf][b][ij] * ...;
            
            /* Lines 161, 167, 173: WRONG - all use CumDiscards[sp][nf][b] */
            // These are inside ij loop but shouldn't be
        }
    }
}
```

The `CumDiscards` array has dimensions `[sp][nf][b]` (species, fishery, box) but NOT vertical layer `ij`. Therefore, all calculations involving `CumDiscards` should be at the box level (`b` loop) but NOT inside the vertical layer loop (`ij` loop).

**Impact:** These bugs cause significant overcounting of:
1. Total discards
2. TEP kills from discards (for birds, mammals, and sharks)
3. Habitat impacts from discards

The magnitude of overcounting depends on the number of vertical layers in each box (`bm->boxes[b].nz`), which could be 5-7 layers typically, meaning discards are counted 5-7 times more than they should be.
