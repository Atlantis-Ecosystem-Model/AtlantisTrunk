# Bug Analysis: atsamplesetup.c

## Summary
Found **5 distinct bug issues** involving swapped array indices and duplicate/incorrect function calls.

---

## Bug 1: Swapped indices in `bm->tassPatchy` array access (Lines 471-472, 488-489, 505-506)

**Location:** Lines 471-472, 488-489, 505-506

**Issue:** Array indices are swapped when accessing `bm->tassPatchy`

**Allocation (Line 459):**
```c
bm->tassPatchy = (double **) alloc2d(2, numsamples + 2);
```
The array is allocated as `[2][numsamples + 2]`, where first dimension is type (2 types) and second dimension is sample number.

**Incorrect Usage (Lines 471-472):**
```c
for (b = 0; b < numsamples; b++) {
    bm->tassPatchy[b][tass_id] = bm->tassessinc / drandom(bm->minfreq, bm->maxfreq);
    bm->tassPatchy[b][tasseat_id] = bm->teatassessinc / drandom(bm->minfreq, bm->maxfreq);
}
```

**Why it's a bug:** The code accesses `bm->tassPatchy[b][tass_id]` where `b` goes from 0 to numsamples-1, but the first dimension is only size 2. This will cause out-of-bounds access when `b >= 2`.

**Correct Usage Should Be:**
```c
for (b = 0; b < numsamples; b++) {
    bm->tassPatchy[tass_id][b] = bm->tassessinc / drandom(bm->minfreq, bm->maxfreq);
    bm->tassPatchy[tasseat_id][b] = bm->teatassessinc / drandom(bm->minfreq, bm->maxfreq);
}
```

**Same bug appears in:**
- Lines 488-489 (no randomization case)
- Lines 505-506 (no patch sampling case)

---

## Bug 2: Swapped indices in `bm->rand` array access (Lines 477-479, 494-496)

**Location:** Lines 477-479, 494-496

**Issue:** Array indices are swapped when accessing `bm->rand`

**Allocation (Line 456):**
```c
bm->rand = (double ***) alloc3d(numyears, nrand_id, bm->K_num_tot_sp);
```
The array is allocated as `[numyears][nrand_id][K_num_tot_sp]`.

**Incorrect Usage (Lines 477-479):**
```c
for (i = 0; i < bm->K_num_tot_sp; i++) {
    if (FunctGroupArray[i].isFished == TRUE) {
        for (b = 0; b < numyears; b++) {
            bm->rand[i][startN_id][b] = Assess_Add_Error(bm, flagprms, 1.0, k_avgprms, k_varprms);
            bm->rand[i][calcF_id][b] = Assess_Add_Error(bm, flagprms, 1.0, k_avgprms, k_varprms);
            bm->rand[i][calcM_id][b] = Assess_Add_Error(bm, flagprms, 1.0, k_avgprms, k_varprms);
        }
    }
}
```

**Why it's a bug:** The code accesses `bm->rand[i][startN_id][b]` where dimensions should be `[b][startN_id][i]` based on the allocation order. The first index should be the year (b), second is the random type, third is the species (i).

**Correct Usage Should Be:**
```c
for (i = 0; i < bm->K_num_tot_sp; i++) {
    if (FunctGroupArray[i].isFished == TRUE) {
        for (b = 0; b < numyears; b++) {
            bm->rand[b][startN_id][i] = Assess_Add_Error(bm, flagprms, 1.0, k_avgprms, k_varprms);
            bm->rand[b][calcF_id][i] = Assess_Add_Error(bm, flagprms, 1.0, k_avgprms, k_varprms);
            bm->rand[b][calcM_id][i] = Assess_Add_Error(bm, flagprms, 1.0, k_avgprms, k_varprms);
        }
    }
}
```

**Same bug appears in:**
- Lines 494-496 (no randomization case, setting to 1.0)

---

## Bug 3: Swapped indices in `avgtl` array access (Line 414)

**Location:** Line 414

**Issue:** Array indices are swapped when initializing `avgtl`

**Allocation (Line 325):**
```c
avgtl = (double **) alloc2d(bm->nfzones + 2, 8);
```
The array is allocated as `[nfzones + 2][8]`.

**Incorrect Usage (Lines 411-415):**
```c
for (i = 0; i < bm->nfzones; i++) {
    zonearea[i] = 0.0;
    for (b = 0; b < 8; b++)
        avgtl[b][i] = 0.0;
}
```

**Why it's a bug:** The code accesses `avgtl[b][i]` where `b` ranges from 0-7 and `i` ranges from 0 to nfzones-1. However, the allocation is `[nfzones + 2][8]`, so it should be `avgtl[i][b]`.

**Correct Usage Should Be:**
```c
for (i = 0; i < bm->nfzones; i++) {
    zonearea[i] = 0.0;
    for (b = 0; b < 8; b++)
        avgtl[i][b] = 0.0;
}
```

---

## Bug 4: Duplicate/overwriting `Set_Species_Biased_Value` calls (Lines 771-776)

**Location:** Lines 771-776

**Issue:** `LAB_DET` and `CARRION` are set twice with different values, overwriting the first settings

**Code:**
```c
Set_Species_Biased_Value(spErrorStructure, LAB_DET, flagdetbiom, k_avgldet, k_varldet, 0, 0, 0, 0, 0, 0);
Set_Species_Biased_Value(spErrorStructure, CARRION, flagdetbiom, k_avgldet, k_varldet, 0, 0, 0, 0, 0, 0);

Set_Species_Biased_Value(spErrorStructure, REF_DET, flagdetbiom, k_avgrdet, k_varrdet, k_avgrdet, k_varrdet, 0, 0, 0, 0);
Set_Species_Biased_Value(spErrorStructure, LAB_DET, flagdetbiom, k_avgrdet, k_varrdet, k_avgrdet, k_varrdet, 0, 0, 0, 0);
Set_Species_Biased_Value(spErrorStructure, CARRION, flagdetbiom, k_avgrdet, k_varrdet, k_avgrdet, k_varrdet, 0, 0, 0, 0);
```

**Why it's a bug:** 
- Lines 771-772 set `LAB_DET` and `CARRION` with `k_avgldet/k_varldet` parameters
- Lines 775-776 immediately overwrite them with `k_avgrdet/k_varrdet` parameters
- This makes lines 771-772 dead code, or lines 775-776 are erroneous duplicates

**Likely Fix:** Remove lines 771-772 as they appear to be copy-paste errors. The correct values should use `k_avgrdet/k_varrdet` based on the variable naming pattern (REF_DET, LAB_DET, CARRION all being detritus types).

---

## Bug 5: Copy-paste error in parameter order (Line 751)

**Location:** Line 751

**Issue:** Wrong parameter passed - appears to be a copy-paste error

**Code:**
```c
Set_Species_Biased_Value(spErrorStructure, MOB_EP_OTHER, flagepibiom, k_avgepif2, k_varepif2, 0, 0, k_avgepi2p, k_varepi2p, k_varepi2eat, k_varepi2eat);
```

**Why it's a bug:** The last two parameters are both `k_varepi2eat`. Looking at the function signature:
```c
static void Set_Species_Biased_Value(ErrorStructure *spErrorStructure, GROUP_TYPES type, int flag, 
    double WC_k_avg_bb, double WC_k_var_bb,
    double SED_k_avg_bb, double SED_k_var_bb, 
    double k_avg_pp, double k_var_pp, 
    double k_avg_eat, double k_var_eat)
```

The last two parameters should be `k_avg_eat` and `k_var_eat`. All other similar calls in the file use the pattern `k_avg*eat, k_var*eat` (avg first, then var), but line 751 has `k_varepi2eat, k_varepi2eat` (var twice).

**Correct Usage Should Be:**
```c
Set_Species_Biased_Value(spErrorStructure, MOB_EP_OTHER, flagepibiom, k_avgepif2, k_varepif2, 0, 0, k_avgepi2p, k_varepi2p, k_avgepi2eat, k_varepi2eat);
```

---

## Summary Table

| Bug # | Lines | Type | Description |
|-------|-------|------|-------------|
| 1 | 471-472, 488-489, 505-506 | Swapped indices | `bm->tassPatchy[b][type]` should be `[type][b]` |
| 2 | 477-479, 494-496 | Swapped indices | `bm->rand[i][type][b]` should be `[b][type][i]` |
| 3 | 414 | Swapped indices | `avgtl[b][i]` should be `avgtl[i][b]` |
| 4 | 771-776 | Duplicate calls | `LAB_DET` and `CARRION` set twice, first settings overwritten |
| 5 | 751 | Wrong parameter | Missing `k_avgepi2eat`, has `k_varepi2eat` twice |

All of these bugs are subtle and could cause runtime errors (array out-of-bounds), incorrect behavior (wrong data storage/retrieval), or dead code (duplicate assignments).
