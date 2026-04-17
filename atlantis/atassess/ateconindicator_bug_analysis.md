# Bug Analysis: ateconindicator.c

## Bugs Found

### 1. **Line 491: Incorrect accumulation of deemed values**
**Severity:** HIGH - Logic Error

**Location:** `Dynamic_Econ_Indicators()` function, inside the species loop

**Code:**
```c
/* Deemed values */
if (catch_count > quota) {
    dvprice = bm->SP_FISHERYprms[sp][nf][deemprice_id];
    current_catch = bm->QuotaAlloc[nf][ns][sp][cummonthcatch_id];
    dv_sum += dvprice * current_catch;  // Line 487: accumulating across species
}

/* Save the dv_sum for writing to output file */
bm->QuotaAlloc[nf][ns][sp][deemed_value_id] += dv_sum;  // Line 491: BUG!
```

**Problem:** 
The variable `dv_sum` is initialized at line 390 before the species loop, then accumulated for each species at line 487. However, at line 491, the **cumulative sum** is added to EACH species individually. This means:
- Species 0 gets its own deemed value
- Species 1 gets species 0's + species 1's deemed values  
- Species 2 gets species 0's + species 1's + species 2's deemed values
- And so on...

This causes incorrect double/triple/multiple counting of deemed values.

**Fix:** Line 491 should either:
1. Be moved inside the `if` block and add only the species-specific value:
```c
if (catch_count > quota) {
    dvprice = bm->SP_FISHERYprms[sp][nf][deemprice_id];
    current_catch = bm->QuotaAlloc[nf][ns][sp][cummonthcatch_id];
    double species_dv = dvprice * current_catch;
    dv_sum += species_dv;
    bm->QuotaAlloc[nf][ns][sp][deemed_value_id] += species_dv;  // Add species-specific value
}
```

2. Or be moved outside the loop if it's meant to store a fleet total somewhere else.

---

### 2. **Line 545: Overwriting revenue with revenue per day**
**Severity:** MEDIUM - Wrong Variable Name / Logic Error

**Location:** `Dynamic_Econ_Indicators()` function

**Code:**
```c
/* Revenue */
lease_cost = bm->SUBFLEET_ECONprms[nf][ns][leased_Q_val_ind_id];
revenue = gvp - lease_cost - cost;
bm->SUBFLEET_ECONprms[nf][ns][revenue_ind_id] = revenue;  // Line 539

// ... other code ...

/* Revenue per day */
bm->SUBFLEET_ECONprms[nf][ns][revenue_ind_id] = revenue / (bm->SUBFLEET_ECONprms[nf][ns][CurrentEffort_id] + small_num);  // Line 545: BUG!
```

**Problem:**
Line 539 stores the absolute revenue value in `revenue_ind_id`, but then line 545 immediately overwrites it with "revenue per day" (revenue divided by effort). The absolute revenue value is lost. Based on the economic indicators list at lines 140-151, these should be separate indicators:
- "Revenue / day" 
- "Cash income = (revenue - costs)"

**Fix:** Line 545 should use a different index variable, likely `rev_day_ind_id` or similar:
```c
bm->SUBFLEET_ECONprms[nf][ns][rev_day_ind_id] = revenue / (bm->SUBFLEET_ECONprms[nf][ns][CurrentEffort_id] + small_num);
```

Or if the intent was to only store revenue per day, then line 539 should be removed or use a different index.

---

### 3. **Line 558: Extra semicolon**
**Severity:** LOW - Style/Typo (doesn't affect functionality)

**Location:** `Dynamic_Econ_Indicators()` function

**Code:**
```c
bm->SUBFLEET_ECONprms[nf][ns][rev_land_ind_id] = revenue / (bm->SUBFLEET_ECONprms[nf][ns][CurrentCatch_id] + small_num);
;  // Line 558: Extra semicolon
```

**Problem:**
There's an extra semicolon on line 558 after the statement. While this doesn't cause a compilation error (it's treated as an empty statement), it's a typo that suggests a copy-paste or typing error.

**Fix:** Remove the extra semicolon:
```c
bm->SUBFLEET_ECONprms[nf][ns][rev_land_ind_id] = revenue / (bm->SUBFLEET_ECONprms[nf][ns][CurrentCatch_id] + small_num);
```

---

## Summary

Three bugs were identified:

1. **Line 491** (HIGH): Deemed values are incorrectly accumulated across all species and added to each species individually, causing significant overcounting
2. **Line 545** (MEDIUM): Revenue indicator is overwritten with revenue-per-day, losing the absolute revenue value
3. **Line 558** (LOW): Extra semicolon (cosmetic issue only)

The most critical bug is #1, which will cause incorrect economic calculations for deemed values. Bug #2 will cause incorrect economic indicator outputs, potentially affecting decision-making in the model.
