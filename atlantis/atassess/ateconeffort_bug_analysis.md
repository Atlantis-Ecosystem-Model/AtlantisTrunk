# Bug Analysis for ateconeffort.c

## Bugs Found

### Bug 1: Uninitialized accumulation of `oldtotpi` (Line 222)
**Location:** Line 222 in `Monthly_Effort_Schedule()`

**Issue:** The variable `oldtotpi` is accumulated inside nested loops (months and species) without being reset:
```c
// Line 211: Start of month loop
for (month = bm->MofY; month < 12; month++) {
    totppi = 0; /* Total expected profit */
    ExpEffort = bm->EffortSchedule[nf][ns][month][expect_id];
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        // ...
        // Line 222:
        oldtotpi += pi[nf][ns][month][sp]; // Get the sum of the old values before you update them (one at a time)
        pi[nf][ns][month][sp] = ans;
        totppi += pi[nf][ns][month][sp]; // Sum of new values
    }
```

**Why it's a bug:** `oldtotpi` is declared at line 106 and initialized to 0, but it accumulates across all months (line 211-269 loop) and all species. However, at line 260, it's used as `this_profit = totppi / (oldtotpi + small_num);` where `totppi` is the total profit for the current month only. The comparison should be between profit for the same month, but `oldtotpi` keeps accumulating across months. 

**Fix:** Reset `oldtotpi = 0;` at line 211 (start of month loop) or at line 214 (start of species loop).

---

### Bug 2: Incorrect profit adjustment formula (Line 254)
**Location:** Line 254 in `Monthly_Effort_Schedule()`

**Issue:** The formula for adjusting `totppi` when `potential_totppi < 0.0` appears incorrect:
```c
} else if (potential_totppi < 0.0) {
    /* Else just make sure contract effort appropriately */
    totppi *= (oldtotpi / (TripCost * ExpEffort));
}
```

**Why it's a bug:** 
- At line 250, when contracting effort: `totppi = oldtotpi * ExpEffort / (stepE + small_num);` - This makes sense (scaling old profit by effort ratio).
- At line 254, the code multiplies the NEW totppi (sum of updated pi values from line 224) by `oldtotpi / (TripCost * ExpEffort)`. This doesn't match the pattern and likely produces incorrect results.
- Line 224 already calculated `totppi` as sum of new pi values, so multiplying it by a ratio of old profits doesn't make mathematical sense.

**Expected behavior:** Should probably subtract costs or recalculate based on the effort-profit relationship, similar to line 256.

---

### Bug 3: `days_left` set to 0 in else branch (Line 579)
**Location:** Lines 572-580 in `Multi_Plan_Effort_Final_Allocation()`

**Issue:** 
```c
if ((bm->TofY > 330) || (bm->DofM > 23)) {
    end_run = 1;
    days_left = 30 - bm->DofM;
    if (days_left < 0)
        days_left = 1;
} else {
    end_run = 0;
    days_left = 0;  // Line 579: Set to 0
}
```

Then at line 651:
```c
if (all_done && any_quota && end_run && (!bm->hist_only)) {
    bm->EffortSchedule[nf][ns][bm->MofY][expect_id] += days_left;  // Adds 0 when end_run is 0
    all_done = 0;
}
```

**Why it's a bug:** While the condition at line 650 checks `end_run`, so `days_left` won't be used when it's 0, setting it to 0 at line 579 is misleading and could cause issues if the logic changes. The variable should either not be assigned in the else branch, or be set to a sentinel value, or the line 651 addition should be guarded more explicitly.

**Note:** This may be defensive programming rather than a bug, but it's worth noting as the value `days_left = 0` never serves a useful purpose.

---

### Bug 4: Incorrect TAC logic negation (Line 910)
**Location:** Line 910 in `Holland_Effort_Final_Allocation()`

**Issue:**
```c
/* Get total current TAC - leave in kg as tot_cumcatch is in kg here */
if(!FunctGroupArray[sp].isTAC || (bm->TACamt[sp][nf][now_id] < no_quota)) {
    totTAC += regTAC_scale * bm->TACamt[sp][nf][now_id];
}
```

**Why it's a bug:** The condition says "if species is NOT under TAC management OR the TAC amount is less than no_quota, then add to totTAC". This is backwards:
- We should be accumulating TAC amounts when the species IS under TAC management (not when it ISN'T)
- The negation operator `!` on `isTAC` appears incorrect

**Expected logic:** Should be `if(FunctGroupArray[sp].isTAC && (bm->TACamt[sp][nf][now_id] >= no_quota))` or similar - only add to totTAC when the species has TAC management AND has a valid quota amount.

---

### Bug 5: Inconsistent subfleet looping for historical catch (Lines 1418-1434)
**Location:** Lines 1413-1440 in `Holland_Effort_Final_Allocation()`

**Issue:** When calculating regional catch proportion, the code handles historical data differently from actual catch data:

**Historical case (use_history=1, lines 1418-1434):**
```c
for (ij = 0; ij < bm->nbox; ij++) {
    if (bm->boxes[ij].type != BOUNDARY) {
        if (bm->regID[ij] == regid) {
            regcatch_nf += bm->SpatialBlackBook[nf][ns][bm->MofY][ij][hist_id]
                    * bm->BlackBook[nf][ns][sp][bm->MofY][hist_id];
```
Uses only the current subfleet `ns`.

**Actual catch case (use_history=0, lines 1437-1439):**
```c
for (nss = 0; nss < bm->FISHERYprms[nf][nsubfleets_id]; nss++) {
    regcatch_nf += RegCatch[nf][nss][regid][sp];
}
```
Correctly loops over ALL subfleets using `nss`.

**Why it's a bug:** The comment at lines 1410-1412 explicitly states: "In each region find proportion of catch taken by the current fleet **(summing across all its subfleets)**". The historical case should also sum across all subfleets (loop over `nss`), not just use the current subfleet `ns`. This creates an asymmetry where historical data only includes one subfleet while actual catch includes all subfleets, leading to incorrect proportional calculations.

**Fix:** Lines 1418-1434 should include an outer loop: `for (nss = 0; nss < bm->FISHERYprms[nf][nsubfleets_id]; nss++)` and use `nss` instead of `ns` when accessing the arrays.

---

## Summary

Five bugs found:
1. **Line 222**: `oldtotpi` accumulates incorrectly across months without reset
2. **Line 254**: Incorrect formula for profit adjustment uses wrong calculation
3. **Line 579**: `days_left = 0` assignment is misleading (minor issue)
4. **Line 910**: Wrong negation in TAC logic - should check `isTAC` not `!isTAC`
5. **Lines 1418-1434**: Historical catch calculation only uses one subfleet instead of summing across all subfleets like the actual catch case

Bugs 1, 2, 4, and 5 are likely to cause incorrect calculations and unexpected behavior. Bug 3 is more of a code quality issue.
