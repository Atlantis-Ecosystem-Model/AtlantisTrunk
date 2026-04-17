# Bug Analysis for atHarvestTS.c

## Summary
Found **2 confirmed bugs** in this file related to format string mismatches and incorrect operator usage.

---

## Bug 1: Format String Argument Mismatch
**Location:** Line 83  
**Severity:** High - Will cause undefined behavior/crash

**Code:**
```c
quit("read%sTimeSeries: Can't allocate memory for %s time series list\n", tsname);
```

**Issue:**  
The format string contains **2** `%s` format specifiers:
- `read%sTimeSeries` (first %s)
- `for %s time series list` (second %s)

But only **1** argument is provided: `tsname`

**Why this is a bug:**  
This will cause undefined behavior when the `quit()` function tries to format the string. The second `%s` will try to read a non-existent argument, potentially reading garbage from the stack or causing a segmentation fault.

**Expected fix:**
Either remove the second `%s` or provide a second argument:
```c
quit("read%sTimeSeries: Can't allocate memory for time series list\n", tsname);
```
or
```c
quit("read%sTimeSeries: Can't allocate memory for %s time series list\n", tsname, tsname);
```

---

## Bug 2: Incorrect Operator (+= instead of =)
**Location:** Line 246  
**Severity:** Medium - Logic error in distribution expansion

**Code:**
```c
bm->CatchTS_agedistrib[fishery_id][sp][ij] += bm->CatchTS_agedistribOrig[fishery_id][sp][ij] + cumtot;
```

**Issue:**  
Uses compound assignment operator `+=` when it should use simple assignment `=`.

**Why this is a bug:**  
1. This is in the "expanding distribution" case (decdis = false) at lines 222-259
2. The purpose is to **restore** the distribution from the original values, not add to existing values
3. In the parallel "contracting distribution" case (line 209), only `cumtot` is added: `bm->CatchTS_agedistrib[fishery_id][sp][ij] += cumtot;`
4. The expanding case should **set** the value to original + accumulated, not **add** to whatever current value exists

**Context:**
- Lines 240-250 handle the case where an age class is NOT depleted
- Line 242 sets depleted ages to 0: `bm->CatchTS_agedistrib[fishery_id][sp][ij] = 0;`
- Line 246 should restore non-depleted ages to their original value plus any accumulated catch from depleted ages

**Expected fix:**
```c
bm->CatchTS_agedistrib[fishery_id][sp][ij] = bm->CatchTS_agedistribOrig[fishery_id][sp][ij] + cumtot;
```

**Impact:**  
When expanding distributions (e.g., when age classes are reintroduced), the catch distribution values will be incorrectly inflated by adding to pre-existing values instead of resetting to the original baseline. This would compound over multiple calls, causing progressively incorrect fishing pressure distributions.

---

## Notes

### Function: Harvest_Read_Time_Series (lines 57-132)
- One critical bug found (Bug 1)
- Rest of the function appears correct

### Function: Harvest_Free_Time_Series (lines 137-148)
- No bugs detected
- Proper cleanup of fishery time series data

### Function: Harvest_Recalc_Time_Series_Distrib (lines 159-296)
- One logic bug found (Bug 2)
- Complex function with contracting/expanding distribution logic
- Other index usages appear correct (e.g., `ij + ncum` at lines 202, 210, 247)
- Loop boundaries appear correct (starting at fishery_id = 1 to copy from fishery 0)

