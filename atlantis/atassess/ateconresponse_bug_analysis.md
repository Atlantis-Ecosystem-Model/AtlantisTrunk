# Bug Analysis: ateconresponse.c

## Summary
This document lists all potential bugs found in ateconresponse.c, including swapped indices, off-by-one errors, wrong variable names, and copy-paste mistakes.

---

## Bug 1: Port Activity Flag Never Set to True (CRITICAL)
**Location:** Lines 81, 110-113 in `Port_Growth()`

**Issue:** The variable `flagfishhere` is initialized to 0 (line 81) and then conditionally set to 0 again (line 111), but it's NEVER set to 1. This means the condition at line 112 will always be true, causing all ports to be skipped.

```c
Line 81:  flagfishhere = 0;
...
Line 110-111: if ((bm->dayt < bm->Port_info[porti][port_start_id]) || (bm->dayt > bm->Port_info[porti][port_end_id]))
                  flagfishhere = 0;
Line 112-113: if (!flagfishhere)
                  continue;
```

**Why it's a bug:** The logic should set `flagfishhere = 1` when the port IS active (inverse the condition), or there should be an `else` clause. As written, no port activity calculations will ever execute.

**Suggested fix:** Change line 110-111 to:
```c
if ((bm->dayt >= bm->Port_info[porti][port_start_id]) && (bm->dayt <= bm->Port_info[porti][port_end_id]))
    flagfishhere = 1;
else
    flagfishhere = 0;
```

---

## Bug 2: Missing Variable Update in Maximum Search
**Location:** Lines 275-276 in `Update_Vessel_Numbers()`

**Issue:** When finding the most profitable subfleet, the code updates `best_subfleet[nf]` but never updates `max_marg_rent` to the new maximum value.

```c
Line 275-276: if (tot_marg_rent[nf][ns] > max_marg_rent)
                  best_subfleet[nf] = ns;
```

**Why it's a bug:** The maximum rent value should be updated so subsequent comparisons work correctly. Without updating `max_marg_rent`, all subsequent checks will compare against 0 (the initial value from line 247).

**Suggested fix:** Add the assignment:
```c
if (tot_marg_rent[nf][ns] > max_marg_rent) {
    max_marg_rent = tot_marg_rent[nf][ns];
    best_subfleet[nf] = ns;
}
```

---

## Bug 3: Incorrect Use of min() Instead of max() or floor()
**Location:** Line 364 in `Update_Vessel_Numbers()`

**Issue:** The code uses `min(1.0, floor(rndnum2))` which will always produce either 0 or 1, never allowing multiple boats to move.

```c
Line 363-364: rndnum2 = drandom(0.0, bm->prop_supp) * bm->SUBFLEET_ECONprms[nf][ns][nboat_id];
              num_moving = (int) (min(1.0, floor(rndnum2)));
```

**Why it's a bug:** Since `floor(rndnum2)` produces integers and `min(1.0, floor(rndnum2))` caps it at 1, this will only ever allow 0 or 1 boat to move. Compare with line 398 which uses `max(1.0, step2)` to ensure at least 1 boat moves. The pattern suggests this should allow multiple boats but caps it incorrectly.

**Suggested fix:** Change to:
```c
num_moving = (int) floor(min(rndnum2, bm->SUBFLEET_ECONprms[nf][ns][nboat_id]));
```
Or if capping at 1 is intentional, this should be documented or use `max(1.0, ...)` like line 398.

---

## Bug 4: Same min() Issue for Debt-Lost Boats
**Location:** Line 431 in `Update_Vessel_Numbers()`

**Issue:** Same pattern as Bug 3 - using `min(1.0, floor(rndnum2))`.

```c
Line 430-431: rndnum2 = drandom(0.0, 1.0) * bm->SUBFLEET_ECONprms[nf][ns][nboat_id];
              debt_lost = (int) (min(1.0, floor(rndnum2)));
```

**Why it's a bug:** This limits debt_lost to 0 or 1, when the intent (multiplying by nboat_id) suggests it should potentially be more boats.

**Suggested fix:** Similar to Bug 3, likely should be:
```c
debt_lost = (int) floor(min(rndnum2, bm->SUBFLEET_ECONprms[nf][ns][nboat_id]));
```

---

## Bug 5: Logical OR Should Be AND (CRITICAL)
**Location:** Line 568 in `Update_Vessel_Numbers()`

**Issue:** The condition uses OR (||) when it should use AND (&&) to verify both indices are valid.

```c
Line 568: if ((this_fishery > -1) || (this_subfleet > -1)) {
```

**Why it's a bug:** Both `this_fishery` and `this_subfleet` are initialized to -1 (lines 546-547). If no suitable alternative is found, they remain -1. The condition should verify BOTH are valid (> -1), not just one. Using OR means the code will execute even if one index is -1, leading to incorrect array access with index -1.

**Suggested fix:** Change to:
```c
if ((this_fishery > -1) && (this_subfleet > -1)) {
```

---

## Bug 6: Inverted Port Activity Check
**Location:** Line 695 in `Update_Vessel_Numbers()`

**Issue:** The condition checks if a port is INACTIVE and then adds weight, which seems backwards.

```c
Line 695-698: if (bm->Port_Fishery[nf][porti] && ((bm->dayt < bm->Port_info[porti][port_start_id]) || (bm->dayt > bm->Port_info[porti][port_end_id]))) {
                  portweight[porti]++;
                  totportweight++;
              }
```

**Why it's a bug:** The date comparison `(bm->dayt < port_start) || (bm->dayt > port_end)` means the port is OUTSIDE its active period. It doesn't make sense to add weight to inactive ports.

**Suggested fix:** Change to check when port IS active:
```c
if (bm->Port_Fishery[nf][porti] && ((bm->dayt >= bm->Port_info[porti][port_start_id]) && (bm->dayt <= bm->Port_info[porti][port_end_id]))) {
```

---

## Bug 7: Inverted Market Weight Logic
**Location:** Lines 708-711 in `Update_Vessel_Numbers()`

**Issue:** If a port is the prime market (flag is true), the weight is set to 0; if not prime, weight is 1. This seems backwards.

```c
Line 708-711: if (bm->Port_info[porti][prime_market_id])
                  marketwgt = 0;
              else
                  marketwgt = 1;
```

**Why it's a bug:** Intuitively, the prime market should have weight 1 (or higher), not 0. Unless the semantics are intentionally inverted (e.g., prime_market_id = 0 means market 0 is prime, and they want weight for market 1), this appears incorrect. The variable name suggests it's a boolean flag where true = prime market.

**Suggested fix:** Verify the intended semantics. If prime market should have higher weight:
```c
if (bm->Port_info[porti][prime_market_id])
    marketwgt = 1;
else
    marketwgt = 0;
```

---

## Bug 8: Unused Variable totCumCatch (Dead Code)
**Location:** Line 1190, 1240 in `Allocate_Catch()`

**Issue:** Variable `totCumCatch` is declared and initialized to 0 but never updated before being subtracted from `totallowedcatch`.

```c
Line 1190: double totCumCatch = 0, ...
...
Line 1240: totallowedcatch -= totCumCatch;
```

**Why it's a bug:** Subtracting a value that's always 0 is a no-op. Either this variable was meant to be calculated earlier (and the calculation was forgotten), or this line should be removed.

**Suggested fix:** Either:
1. Remove line 1240 if totCumCatch is not needed, or
2. Add code to calculate totCumCatch before line 1240

---

## Minor Style Issues (Not Bugs but Worth Noting)

### Issue A: Inconsistent Spacing
**Location:** Lines 236, 590, 743

Extra space in `bm-> K_max_num_boats` (with space after `->` operator). While syntactically valid, it's inconsistent with the rest of the code.

```c
Line 236: orig_max_num_boats = bm-> K_max_num_boats;
Line 590: bm-> K_max_num_boats = 0;
Line 743: if(( bm->K_max_num_boats > orig_max_num_boats) && ...
```

### Issue B: Double Parentheses
**Location:** Line 743

```c
Line 743: if(( bm->K_max_num_boats > orig_max_num_boats) && bm->flagStoreShotCPUE)
```

Extra opening parenthesis after `if(`.

---

## Summary of Critical Bugs

1. **Bug 1 (Line 110-113):** Port activity logic - `flagfishhere` never set to 1, all ports skipped
2. **Bug 2 (Line 275-276):** Missing `max_marg_rent` update in maximum search
3. **Bug 5 (Line 568):** OR should be AND when checking valid fishery/subfleet indices
4. **Bug 6 (Line 695):** Inverted port activity check - adding weight to inactive ports
5. **Bug 7 (Lines 708-711):** Questionable market weight logic (may be inverted)

## Summary of Likely Bugs

1. **Bug 3 (Line 364):** Incorrect use of `min(1.0, floor(...))` limiting boats to 0-1
2. **Bug 4 (Line 431):** Same min/floor issue for debt-lost boats
3. **Bug 8 (Line 1240):** Dead code - unused `totCumCatch` variable

---

**Total Bugs Found:** 8 (5 critical, 3 likely)
