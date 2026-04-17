# Bug Analysis: ateffortquota.c

## Summary
This file contains two functions, with most of the second function commented out. I found **no bugs in the active code**, but discovered **multiple bugs in the commented-out code** that would cause issues if uncommented.

---

## Active Code Analysis

### Function: `Total_Effort_Quota_Price` (lines 26-48)
**Status:** ✅ No bugs found

The function correctly:
- Iterates through fisheries and subfleets with consistent indexing `[nf][ns]`
- Checks for empty subfleets before processing
- Calculates and stores effort quota prices
- Accumulates lease costs

---

## Commented-Out Code Analysis

### Function: `Effort_Quota_trade` (lines 55-395)

#### BUG 1: Undeclared Variable (Line 78)
**Severity:** High (compilation error if uncommented)

```c
spp_id = bm->SPid[sp];
```

**Issue:** Variable `sp` is used but never declared or defined in this function. This would cause a compilation error.

**Impact:** Code would not compile if uncommented.

---

#### BUG 2: Month Index Off-by-One Error (Lines 192, 228, 357)
**Severity:** High (incorrect array indexing)

**Line 192:**
```c
buyermonth = (int)floor(ROUNDGUARD + (linearPI[sp][idd_id][b])) % nm;
```

**Line 228:**
```c
sellermonth = (int)floor(ROUNDGUARD + (linearPI[sp][idd_id][s])) % nm;
```

**Line 357:**
```c
buyermonth = (int)floor(ROUNDGUARD + (linearPI[sp][idd_id][b])) % nm;
```

**Issue:** These lines compute a **relative month offset** (0 to nm-1) but the resulting value is used to index arrays that expect **absolute month values** (bm->MofY to 11).

**Evidence:** Line 125 shows that `EffortSchedule` is indexed by absolute month:
```c
for(month=bm->MofY; month<12; month++){
    scheduledEffort = bm->EffortSchedule[nf][ns][month][expect_id];
```

Lines 197, 199, 231, 233, 361, 363 use `buyermonth`/`sellermonth` to index `EffortSchedule`:
```c
bm->EffortSchedule[buyernf][buyerns][buyermonth][expect_id]
```

**Why it's wrong:** 
- Line 133 creates the ID as: `idd = ns*tnf*nm + nm*nf + month - bm->MofY`
- The stored value is `month - bm->MofY` (relative offset)
- Extracting it with `% nm` gives the relative offset, not the absolute month
- Using this to index `EffortSchedule[...][...][buyermonth]` accesses wrong months

**Correct fix:** Should add `bm->MofY` to convert relative to absolute:
```c
buyermonth = ((int)floor(ROUNDGUARD + (linearPI[sp][idd_id][b])) % nm) + bm->MofY;
```

**Impact:** Would access wrong month indices, causing incorrect effort schedule lookups and quota calculations.

---

#### BUG 3: Inconsistent Division-by-Zero Protection (Lines 272, 282)
**Severity:** Medium (potential crash)

**Line 272:**
```c
bm->SUBFLEET_ECONprms[sellernf][sellerns][totPlanCatch_id] += scheduledEffort * ExpCatch / ExpEffort;
```

**Line 282:**
```c
bm->SUBFLEET_ECONprms[buyernf][buyerns][totPlanCatch_id] += scheduledEffort * ExpCatch / ExpEffort;
```

**Issue:** These divisions by `ExpEffort` lack protection against zero values, while similar calculations elsewhere in the code add `small_num` for safety.

**Evidence:** Compare with lines 212, 235, 247, 375 which use the same formula but add protection:
```c
expectedCatch += scheduledEffort * ExpCatch / (ExpEffort + small_num);
```

**Why it's wrong:** If `ExpEffort` is zero, this causes division by zero (undefined behavior in C, typically results in NaN or crash).

**Correct fix:** Add `small_num` protection:
```c
bm->SUBFLEET_ECONprms[sellernf][sellerns][totPlanCatch_id] += scheduledEffort * ExpCatch / (ExpEffort + small_num);
```

**Impact:** Potential division by zero errors leading to NaN values or crashes.

---

#### BUG 4: Misleading/Incorrect Comments (Lines 189, 224, 322, 352)
**Severity:** Low (documentation error)

**Lines 189, 224, 322, 352 all contain:**
```c
/* Deconstructing ids from single sorted array - assuming ids in
the sorted array are of the form ns*tnf*(12 - month) + (12 - month)*nf + month - bm->MofY */
```

**Issue:** The comment describes the ID formula as `ns*tnf*(12 - month) + (12 - month)*nf + month - bm->MofY`, but line 133 actually computes:
```c
idd = ns*tnf*nm + nm*nf + month - bm->MofY;
```
where `nm = 12 - bm->MofY` (from line 59).

**Why it's wrong:** 
- Comment formula: `ns*tnf*(12 - month) + (12 - month)*nf + month - bm->MofY`
- Actual formula: `ns*tnf*nm + nm*nf + (month - bm->MofY)` where `nm = 12 - bm->MofY`
- The comment suggests month-dependent coefficients `(12 - month)`, but the code uses a constant `nm`

**Correct comment:** Should read:
```c
/* Deconstructing ids from single sorted array - assuming ids in
the sorted array are of the form ns*tnf*nm + nm*nf + (month - bm->MofY)
where nm = 12 - bm->MofY */
```

**Impact:** Confusion for developers trying to understand or modify the code.

---

## Summary of Bugs

| Line(s) | Severity | Type | Description |
|---------|----------|------|-------------|
| 78 | High | Undeclared variable | Variable `sp` used without declaration |
| 192, 228, 357 | High | Index error | Month index off-by-one: using relative offset instead of absolute month |
| 272, 282 | Medium | Division by zero | Missing `small_num` protection in division |
| 189, 224, 322, 352 | Low | Documentation | Comments incorrectly describe ID formula |

---

## Recommendations

1. **Before uncommenting code:** Fix all identified bugs, especially the high-severity index errors
2. **Add declaration:** Determine proper scope and type for variable `sp` on line 78
3. **Fix month calculations:** Add `+ bm->MofY` to lines 192, 228, 357
4. **Add safety checks:** Add `small_num` to denominators on lines 272, 282
5. **Update comments:** Correct formula descriptions on lines 189, 224, 322, 352
