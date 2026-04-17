# Bug Analysis: atHarvest.c

## Summary
Analysis of atHarvest.c reveals 3 subtle bugs involving incorrect index usage, undefined variables, and missing variable assignments.

---

## Bug 1: Wrong Index in RecCatch Array Access

**Location:** Lines 186-191 in [`Harvest_Init_Box_Arrays()`](../atharvest/atHarvest.c:186)

**Code:**
```c
for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
    if (FunctGroupArray[sp].isImpacted == TRUE) {
        for (n = 0; n < bm->K_num_fisheries; n++) {
            bm->TotCumRecCatch[sp][ij] += bm->RecCatch[ij][sp][ij];  // LINE 188
            bm->RecCatch[ij][sp][ij] = 0;                             // LINE 189
        }
    }
}
```

**Problem:**
The loop iterates over `n` (fisheries index from 0 to `K_num_fisheries`), but the loop variable `n` is never used in the loop body. Instead, the box index `ij` is used twice in the array access: `bm->RecCatch[ij][sp][ij]`. 

**Why it's a bug:**
- The array should be indexed as `[box][species][fishery]` based on the loop structure
- The third index should be the fishery index `n`, not the box index `ij`
- This causes the code to repeatedly access and update the same array element `ij` times, ignoring all other fisheries
- Only fishery index equal to the current box number would be processed

**Expected code:**
```c
for (n = 0; n < bm->K_num_fisheries; n++) {
    bm->TotCumRecCatch[sp][ij] += bm->RecCatch[ij][sp][n];
    bm->RecCatch[ij][sp][n] = 0;
}
```

---

## Bug 2: Uninitialized Variable Usage in AGE_STRUCTURED_BIOMASS Case

**Location:** Lines 1587-1593 in [`Harvest_Skip_biology()`](../atharvest/atHarvest.c:1587)

**Code:**
```c
case AGE_STRUCTURED_BIOMASS:
    for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
        pid = FunctGroupArray[sp].totNTracers[n];
        spbiom = bm->boxes[ij].tr[k][pid];

        /* Get catch per fishery */
        sel = bm->selectivity[sp][nf][stage];  // LINE 1593
        q = bm->SP_FISHERYprms[sp][nf][q_id];
        loadDetFC = bm->SP_FISHERYprms[sp][nf][FFCDR_id];
        spfishing = FCpressure * sel * q * spbiom * (1.0 - loadDetFC) * swept_area;
```

**Problem:**
The variable `stage` is used on line 1593 but is never assigned in the `AGE_STRUCTURED_BIOMASS` case block.

**Why it's a bug:**
- In the previous case (`AGE_STRUCTURED`, line 1573), `stage` is properly assigned: `stage = FunctGroupArray[sp].cohort_stage[n];`
- In the `AGE_STRUCTURED_BIOMASS` case, this assignment is missing
- The code uses whatever value `stage` had from previous iterations, leading to unpredictable behavior
- This is a classic copy-paste error where the assignment line was omitted

**Expected code:**
```c
case AGE_STRUCTURED_BIOMASS:
    for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
        pid = FunctGroupArray[sp].totNTracers[n];
        spbiom = bm->boxes[ij].tr[k][pid];
        stage = FunctGroupArray[sp].cohort_stage[n];  // MISSING LINE

        /* Get catch per fishery */
        sel = bm->selectivity[sp][nf][stage];
```

---

## Bug 3: Wrong Layer Index for Epibenthos Catch

**Location:** Line 1630 in [`Harvest_Skip_biology()`](../atharvest/atHarvest.c:1630)

**Code:**
```c
/* Epibenthos - Get catch per fishery */
for (nf = 0; nf < bm->K_num_fisheries; nf++) {
    bm->cell_vol = bm->boxes[ij].area * bm->boxes[ij].dz[0];
    swept_area = bm->FISHERYprms[nf][swept_area_id] / (bm->cell_vol * FC_hdistrib[ij][nf] + small_num);
    vertdistrib = Effort_vdistrib[ij][0][nf];  // Layer 0 explicitly used
    FCpressure = bm->Effort[ij][nf] * vertdistrib / 86400.0;

    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].habitatType == EPIFAUNA) {
            for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
                // ... calculations ...
                bm->Catch[ij][sp][nf][k] += spfishing * bm->cell_vol * bm->dt;  // LINE 1630
```

**Problem:**
The variable `k` is used as the layer index, but `k` is not defined in this scope. The variable `k` was used in the previous loop (lines 1556-1609) which has already ended.

**Why it's a bug:**
- This section is for epibenthos processing, which occurs in layer 0 (as evidenced by `dz[0]` on line 1612 and `Effort_vdistrib[ij][0][nf]` on line 1614)
- The variable `k` is out of scope and contains an undefined value (likely the last value from the previous loop)
- The comment "/* Epibenthos - Get catch per fishery */" indicates this is a separate processing block
- All other references in this section explicitly use index 0 for the layer

**Expected code:**
```c
bm->Catch[ij][sp][nf][0] += spfishing * bm->cell_vol * bm->dt;
CatchSum[sp][tscocatch_id] += spfishing * bm->cell_vol * bm->dt;
```

---

## Summary Table

| Bug # | Location | Type | Severity | Description |
|-------|----------|------|----------|-------------|
| 1 | Lines 188-189 | Index swap | High | Using box index `ij` instead of fishery index `n` in RecCatch array |
| 2 | Line 1593 | Uninitialized variable | High | Variable `stage` used without being assigned in AGE_STRUCTURED_BIOMASS case |
| 3 | Line 1630 | Wrong variable | Medium | Using out-of-scope variable `k` instead of constant `0` for epibenthos layer |

All three bugs follow the pattern of subtle indexing errors and copy-paste mistakes that the user requested be identified.
