# Bug Analysis for atsurvey.c

## Summary
Found **4 critical bugs** involving swapped indices, wrong variable usage, and copy-paste errors.

---

## Bug 1: Wrong error parameters for temperature sampling (Line 195)
**Severity:** Medium - Incorrect sampling variance  
**Location:** Line 195 in `InvertebrateSampling()`

### Issue:
```c
SamplePhysicalProperty(bm, temperature_id, bb, b, z, k_avgsalt, k_varsalt);
```

### Problem:
Temperature is being sampled using **salt parameters** (`k_avgsalt`, `k_varsalt`) instead of temperature parameters. This is a clear copy-paste error from line 194 which correctly samples salinity.

### Expected:
```c
SamplePhysicalProperty(bm, temperature_id, bb, b, z, k_avgtemp, k_vartemp);
```

### Evidence:
Line 194 correctly uses salt parameters for salinity:
```c
SamplePhysicalProperty(bm, salinity_id, bb, b, z, k_avgsalt, k_varsalt);
```
Temperature should have its own variance parameters.

---

## Bug 2: Swapped array indices (Line 489)
**Severity:** Critical - Incorrect memory access  
**Location:** Line 489 in `CharismaticEstimates()`

### Issue:
```c
raw = biolVERTinfo[bres_id][chrt][b][sp];
```

### Problem:
The array indices are in the wrong order: `[bres_id][chrt][b][sp]`  
All other accesses to `biolVERTinfo` in this file use the order: `[id][sp][chrt][b]`

### Expected:
```c
raw = biolVERTinfo[bres_id][sp][chrt][b];
```

### Evidence:
Compare with nearby lines in the same function:
- Line 464: `raw = biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b];`
- Line 470: `raw = biolVERTinfo[bstocknums_id][sp][chrt][b];`
- Line 476: `raw = biolVERTinfo[bcatchnums_id][sp][chrt][b];`
- Line 484: `raw = biolVERTinfo[bdiscards_id][sp][chrt][b];`

All consistently use `[id][sp][chrt][b]` order.

---

## Bug 3: Using uninitialized/wrong variable (Line 599)
**Severity:** Critical - Incorrect calculation  
**Location:** Line 599 in `CharismaticEstimates()`

### Issue:
```c
rawn = biolVERTinfo[bdiscards_id][sp][chrt][b] / (raw + TINY);
```

### Problem:
The variable `raw` is being used but hasn't been set in this loop context. Looking at the function structure:
- Lines 590-603: First loop processing sample numbers
- Line 599 is in this first loop
- The variable `raw` is only initialized to 0 at declaration (line 388)
- `raw` is not set to a meaningful value before line 599 in this loop

This should be dividing by the weight of the individual, not an undefined `raw` value.

### Expected:
```c
rawwgt = biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b];
rawn = biolVERTinfo[bdiscards_id][sp][chrt][b] / (rawwgt + TINY);
```

### Evidence:
Compare with the correct pattern at lines 946-948 in `FisheriesRecords()`:
```c
rawwgt = biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b];
raw = biolVERTinfo[bdiscards_id][sp][chrt][b] / (rawwgt + TINY);
```
This shows the proper pattern of calculating weight before dividing.

---

## Bug 4: Duplicate/incorrect record keeping (Line 953)
**Severity:** Medium - Incorrect data tracking  
**Location:** Line 953 in `FisheriesRecords()`

### Issue:
```c
/* Collect assessment data */
bm->CatchRecord[Yr][sp][chrt][commerical_id] += sample;
```

### Problem:
This is in the section handling **discards** (lines 945-953), but it's adding to the same `CatchRecord` that was already updated for actual catch at line 943. This means discards are being counted as catch.

### Context:
```c
/* Numbers in catch */
raw = biolVERTinfo[bcatchnums_id][sp][chrt][b];
sample = Assess_Add_Error(bm, flagcount, raw, k_avgcobs, k_varcobs);
samplenums[chrt][samplecatch_id] = ceil(sample);
totn[samplecatch_id] += samplenums[chrt][samplecatch_id];

/* Collect assessment data */
bm->CatchRecord[Yr][sp][chrt][commerical_id] += sample;  // Line 943

/* Numbers in discards */
rawwgt = biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b];
raw = biolVERTinfo[bdiscards_id][sp][chrt][b] / (rawwgt + TINY);
sample = Assess_Add_Error(bm, flagcount, raw, k_avgcobs, k_varcobs);
samplenums[chrt][samplediscard_id] = ceil(sample);
totn[samplediscard_id] += samplenums[chrt][samplediscard_id];

/* Collect assessment data */
bm->CatchRecord[Yr][sp][chrt][commerical_id] += sample;  // Line 953 - DUPLICATE!
```

### Expected:
Either remove line 953, or it should write to a different array/field for discards (e.g., `DiscardRecord` if it exists), or use a different index to distinguish catch from discards.

---

## Additional Observations (Not Bugs, but Worth Noting)

### Redundant rounding (Lines 639, 1259)
**Severity:** Low - Inefficiency, not incorrectness  
**Locations:** Lines 639 and 1259

Both lines have redundant `+0.5` inside `round()`:
```c
cage = (int)(round((chrt + drandom(0.0, 1.0)) * FunctGroupArray[sp].ageClassSize + 0.5));
```

The `+0.5` is redundant because `round()` already rounds to nearest integer. However, this doesn't cause incorrect behavior, just unnecessary computation.

---

## Inconsistent array indexing (Lines 118 vs 120)
**Severity:** Low - Possibly intentional  
**Location:** Lines 118-120 in `SamplePhysicalProperty()`

```c
samplephys[propertyIndex - 2][zone][sample_id] += sample * samplearea[bb];

cvphys[0][propertyIndex][zone] += sample;
cvphys[1][propertyIndex][zone] += sample * sample;
```

`samplephys` uses `propertyIndex - 2` while `cvphys` uses `propertyIndex` directly. This might be intentional due to different array sizing, but it's worth verifying the array dimensions are correct.

---

## Summary of Critical Issues

1. **Line 195**: Copy-paste error using salt parameters for temperature
2. **Line 489**: Swapped array indices `[bres_id][chrt][b][sp]` should be `[bres_id][sp][chrt][b]`
3. **Line 599**: Using uninitialized/wrong `raw` variable instead of calculating weight
4. **Line 953**: Discards incorrectly added to catch record

All four bugs should be fixed to ensure correct sampling and data collection behavior.
