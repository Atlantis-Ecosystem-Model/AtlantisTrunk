# Bug Analysis for atindices.c

## Summary
Found 6 bugs involving swapped indices, wrong operators, copy-paste errors, and inconsistent conditionals.

---

## Bug 1: Wrong Assignment Operator (Line 289)
**Location:** [`Calc_Max_Size()`](atindices.c:289)

**Code:**
```c
maxlngth = +(invstockinfo[fcatch_id][i][z][id] / (zonearea[z] + TINY)) * FunctGroupArray[i].speciesParams[max_length_id] / (totcatch + TINY);
```

**Issue:** Uses `maxlngth = +` instead of `maxlngth +=`

**Why it's wrong:** 
- The unary plus operator `+` doesn't make sense here in context
- Earlier in the function (lines 245-256), the code accumulates `maxlngth` using `+=` for vertebrates and invertebrates
- This line should be **adding** to `maxlngth`, not **assigning** with a unary plus
- This causes all previous vertebrate contributions to be overwritten

**Expected:** `maxlngth += (invstockinfo...`

---

## Bug 2: Duplicate Break Statement (Lines 498-499)
**Location:** [`Calc_Size_Spectra()`](atindices.c:498)

**Code:**
```c
case MICROPHTYBENTHOS:
    sedbin[1][z] += samplebiom[sp][z][id];
    break;
    break;
```

**Issue:** Two consecutive `break;` statements

**Why it's wrong:**
- The second `break;` is unreachable code
- This is a copy-paste error
- While not causing runtime errors, it's incorrect and confusing

---

## Bug 3: Swapped Array Indices (Line 885)
**Location:** [`Calc_ABC()`](atindices.c:885)

**Code:**
```c
for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
    for (chrt = 0; chrt < FunctGroupArray[sp].numStages; chrt++)
        nums[chrt][sp] = 0;
}
```

**Issue:** Array indices are swapped: `nums[chrt][sp]` should be `nums[sp][chrt]`

**Why it's wrong:**
- Throughout the rest of the function, the array is consistently accessed as `nums[sp][chrt]` (lines 857, 858, 861, 862, 891, 892, 896, 897)
- Line 873 also uses `globalnums[sp][chrt][0]` with species first
- This inconsistency will cause incorrect memory access and data corruption
- The declaration and usage pattern indicates species should be the first index

**Expected:** `nums[sp][chrt] = 0;`

---

## Bug 4: Inconsistent Condition Check (Line 816)
**Location:** [`sort_global_values_big()`](atindices.c:816)

**Code:**
```c
for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
    if (FunctGroupArray[sp].isVertebrate == TRUE || FunctGroupArray[sp].isMacroFauna == TRUE) {
        rank = 0;
        for (chrt = 0; chrt < bm->K_num_tot_sp; chrt++) {
            if (FunctGroupArray[chrt].isMacroFauna == TRUE) {  // Line 816
                if (globalnums[sp][index][typeIndex] > globalnums[chrt][index][typeIndex])
                    rank++;
            }
        }
```

**Issue:** Inner loop only checks `isMacroFauna` for `chrt`, but outer loop checks both `isVertebrate` OR `isMacroFauna` for `sp`

**Why it's wrong:**
- The function is supposed to sort "big" values (vertebrates + macrofauna)
- The outer condition (line 813) includes vertebrates: `FunctGroupArray[sp].isVertebrate == TRUE || FunctGroupArray[sp].isMacroFauna == TRUE`
- But the inner comparison (line 816) only compares against macrofauna, excluding vertebrates
- This means vertebrate species will be ranked incorrectly because they're not compared against other vertebrates
- Compare with `sort_global_values()` at line 777 which correctly checks all three conditions

**Expected:** `if (FunctGroupArray[chrt].isVertebrate == TRUE || FunctGroupArray[chrt].isMacroFauna == TRUE)`

---

## Bug 5: Wrong Trophic Level Index (Line 1507)
**Location:** [`Calc_Trophic_Spectra()`](atindices.c:1507)

**Code:**
```c
/* Zone specific and total, static trophic level through time */
tlindx = (int) (floor((tl[a][sp][stage] - 1.0) / 0.5));
```

**Issue:** Uses `tl[a]` for static trophic level calculation, should use `tl[a+3]`

**Why it's wrong:**
- The comment explicitly says "static trophic level through time"
- Lines 1497-1505 handle **dynamic** trophic level using `tl[a][sp][stage]`
- Lines 1506-1514 handle **static** trophic level, but line 1507 incorrectly uses `tl[a][sp][stage]`
- The pattern throughout the function:
  - `tl[a]` or `tl[2]` = dynamic (changes through time)
  - `tl[a+3]` or `tl[5]` = static (fixed at initial level)
- Line 1547 correctly uses `tl[a + 3][sp][stage]` for the same "static" calculation

**Expected:** `tlindx = (int) (floor((tl[a + 3][sp][stage] - 1.0) / 0.5));`

---

## Bug 6: Wrong Variable Used (Line 1384)
**Location:** [`Calc_Avgtl()`](atindices.c:1384)

**Code:**
```c
avgtl[0][bm->nfzones] += catches * totalarea[bb] / (totcatch + TINY);
avgtl[0][bm->nfzones + 1] += totcatchAST * samplearea[bb] / (totcatch + TINY);  // Line 1384
avgtl[2][z] += catchesST * samplearea[bb] / (totcatch + TINY);
avgtl[2][bm->nfzones] += catchesST * totalarea[bb] / (totcatch + TINY);
avgtl[2][bm->nfzones + 1] += totcatchAST * samplearea[bb] / (totcatch + TINY);
```

**Issue:** Line 1384 uses `totcatchAST` (static) but should use `totcatchA` (dynamic)

**Why it's wrong:**
- The index pattern indicates:
  - `avgtl[0]` = dynamic trophic level of catch
  - `avgtl[2]` = static trophic level of catch
- Line 1382 correctly uses `catches` (dynamic) for `avgtl[0][z]`
- Line 1383 correctly uses `catches` (dynamic) for `avgtl[0][bm->nfzones]`
- Line 1384 should use `totcatchA` (overall dynamic) for `avgtl[0][bm->nfzones + 1]`
- But it incorrectly uses `totcatchAST` (overall static), which is the wrong metric
- Lines 1353 and 1354 show that `totcatchA` accumulates with `tl[2]` (overall dynamic)
- Lines 1353 and 1354 show that `totcatchAST` accumulates with `tl[5]` (overall static)

**Expected:** `avgtl[0][bm->nfzones + 1] += totcatchA * samplearea[bb] / (totcatch + TINY);`

---

## Summary Table

| Line | Function | Bug Type | Description |
|------|----------|----------|-------------|
| 289 | `Calc_Max_Size` | Wrong operator | `= +` should be `+=` |
| 498-499 | `Calc_Size_Spectra` | Duplicate code | Two `break;` statements |
| 885 | `Calc_ABC` | Swapped indices | `nums[chrt][sp]` should be `nums[sp][chrt]` |
| 816 | `sort_global_values_big` | Inconsistent condition | Missing `isVertebrate` check for `chrt` |
| 1507 | `Calc_Trophic_Spectra` | Wrong variable | `tl[a]` should be `tl[a+3]` for static |
| 1384 | `Calc_Avgtl` | Wrong variable | `totcatchAST` should be `totcatchA` |
