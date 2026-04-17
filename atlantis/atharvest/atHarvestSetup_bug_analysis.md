# Bug Analysis for atHarvestSetup.c

## Critical Bugs Found

### 1. **Array Dimension Mismatch - FisherySpeciesCatchFlags** (Line 438)
**Location:** `Allocate_Harvest_Memory()` function  
**Line 438:**
```c
FisherySpeciesCatchFlags = Util_Alloc_Init_2D_Int(bm->K_num_fisheries, bm->K_num_tot_sp, 0);
```

**Problem:** The array is allocated with dimensions `[K_num_fisheries][K_num_tot_sp]`, but it's consistently accessed throughout the code as `[species][fishery]`.

**Usage examples showing the mismatch:**
- Line 153: `FisherySpeciesCatchFlags[sp][nf]` (sp=species, nf=fishery)
- Line 314: `FisherySpeciesCatchFlags[fgIndex][nf]` (fgIndex=species, nf=fishery)
- Line 329: `FisherySpeciesCatchFlags[fgIndex][nf]` 
- Line 479: `FisherySpeciesCatchFlags[sp][nf] = FALSE;`
- Line 493: `FisherySpeciesCatchFlags[sp][nf] = TRUE;`

**Why this is an error:** The allocation order doesn't match the access pattern. This will cause incorrect indexing and potential out-of-bounds access if `K_num_fisheries != K_num_tot_sp`.

**Fix:** Should be allocated as:
```c
FisherySpeciesCatchFlags = Util_Alloc_Init_2D_Int(bm->K_num_tot_sp, bm->K_num_fisheries, 0);
```

---

### 2. **Array Dimension Mismatch - selectivity** (Line 446)
**Location:** `Allocate_Harvest_Memory()` function  
**Line 446:**
```c
bm->selectivity = Util_Alloc_Init_3D_Double(bm->K_num_max_stages, bm->K_num_fisheries, bm->K_num_tot_sp, 0.0);
```

**Problem:** The array is allocated with dimensions `[K_num_max_stages][K_num_fisheries][K_num_tot_sp]`, but accessed as `[species][fishery][stage]`.

**Usage showing the mismatch:**
- Line 101: `bm->selectivity[sp][i][b]` where:
  - `sp` loops over species (0 to K_num_tot_sp, line 97)
  - `i` loops over fisheries (0 to K_num_fisheries, line 98)
  - `b` loops over stages (0 to numStages, line 99)

**Why this is an error:** The allocation order `[stages][fisheries][species]` doesn't match the access pattern `[species][fishery][stage]`. This is a serious bug that will cause memory corruption.

**Fix:** Should be allocated as:
```c
bm->selectivity = Util_Alloc_Init_3D_Double(bm->K_num_tot_sp, bm->K_num_fisheries, bm->K_num_max_stages, 0.0);
```

---

### 3. **Copy-Paste Error in String Descriptions** (Lines 399, 403, 409)
**Location:** `Build_Fishery_Tracers()` function  
**Lines 399, 403, 409:**
```c
// Line 399 (recreational fishery with multiple rec fisheries):
snprintf(longName, sizeof(longName), "Total effort of recreational fishery %d (%s)", FCindex + 1, FisheryArray[nf].fisheryCode);

// Line 403 (recreational fishery with single rec fishery):
snprintf(longName, sizeof(longName), "Total effort of recreational fishery");

// Line 409 (commercial fishery):
snprintf(longName, sizeof(longName), "Total effort of fishery %d (%s)", FCindex + 1, FisheryArray[nf].fisheryCode);
```

**Problem:** These are describing the tracer for **average catch size** (see variable name on line 398: `"Avg_Catch_Sze_"`), but the description says "Total effort".

**Evidence:** 
- Line 408 has a comment: `/* This should be different but is not!*/` - confirming this is a known issue
- Line 398: The variable name is `"Avg_Catch_Sze_Recfish%d"` or `"Avg_Catch_Sze_FC%d"`
- Line 412: The tracer being added is `FisheryArray[nf].averageCatchSizeTracer`
- Compare with lines 379-390 which correctly describe the "Total effort" tracers

**Why this is an error:** Misleading documentation - the tracer description doesn't match what's actually being tracked.

**Fix:** Should say something like:
```c
// Line 399:
snprintf(longName, sizeof(longName), "Average catch size of recreational fishery %d (%s)", FCindex + 1, FisheryArray[nf].fisheryCode);

// Line 403:
snprintf(longName, sizeof(longName), "Average catch size of recreational fishery");

// Line 409:
snprintf(longName, sizeof(longName), "Average catch size of fishery %d (%s)", FCindex + 1, FisheryArray[nf].fisheryCode);
```

---

### 4. **Wrong Variable Used - Age Vulnerability** (Lines 645, 650)
**Location:** `Calculate_Age_Vulnerability()` function  
**Lines 645, 650:**
```c
// Line 645:
FunctGroupArray[sp].speciesParams[Age50pcntV_id] = chrt;

// Line 650:
FunctGroupArray[sp].speciesParams[Age95pcntV_id] = chrt;
```

**Problem:** The code uses `chrt` but should use `basechrt` to be consistent with the tracking variables.

**Evidence:**
- Line 628: `for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++)`
- Line 629: `basechrt = chrt / FunctGroupArray[sp].numGeneTypes;`
- Line 644: Condition checks `(basechrt < age50)` 
- Line 646: `age50 = basechrt;` - stores `basechrt`, not `chrt`
- Line 649: Condition checks `(basechrt < age95)`
- Line 651: `age95 = basechrt;` - stores `basechrt`, not `chrt`

**Why this is an error:** For species with multiple gene types, `chrt` includes all cohorts across all gene types, while `basechrt` is the actual age/cohort number. Lines 646 and 651 correctly use `basechrt` for the age tracking variables, so lines 645 and 650 should also use `basechrt` for consistency. Using `chrt` could result in storing values that are larger than the actual number of cohorts.

**Fix:**
```c
// Line 645:
FunctGroupArray[sp].speciesParams[Age50pcntV_id] = basechrt;

// Line 650:
FunctGroupArray[sp].speciesParams[Age95pcntV_id] = basechrt;
```

---

## Summary

**Total bugs found: 4**

1. **Line 438** - Critical: Array dimension mismatch for `FisherySpeciesCatchFlags`
2. **Line 446** - Critical: Array dimension mismatch for `selectivity` 
3. **Lines 399, 403, 409** - Moderate: Copy-paste error in tracer descriptions
4. **Lines 645, 650** - Moderate: Wrong variable used (`chrt` instead of `basechrt`)

The first two bugs are critical and could cause memory corruption, incorrect calculations, or crashes. The latter two are less severe but still represent logic errors that should be fixed.
