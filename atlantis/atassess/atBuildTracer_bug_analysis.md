# Bug Analysis: atBuildTracer.c

## Bug 1: Line 401 - Incorrect error message parameter in Build_TrName_NameList

**Location:** [`Build_TrName_NameList`](../atecology/atBuildTracer.c:152)

**Line 401:**
```c
quit("ERROR: Build_TrName_NameList - Mismatch in the number of tracers we are expected and the number we have built. We are expecting %d, but we have built %d tracers\n", nTrSize, index);
```

**Issue:** The error message says "we have built %d tracers" and passes `index` as the second parameter. However, `index` has already been incremented past the last tracer that was added. The actual number of tracers built (excluding the NULL terminator) is `index - 1`, not `index`.

**Context:**
- Line 212: `nTrSize = size - 1;` (the last valid tracer index, excluding NULL)
- Line 398: `Add_Tracer(&trnamelist, index++, NULL, NULL, 0, 0, 0, 0);` (adds NULL and increments index)
- Line 400: `if ((index - 1) != nTrSize)` (checks if counts match)

**Why it's an error:** The error message compares `nTrSize` (last valid index) with `index` (total count including NULL terminator). This is confusing because they represent different things. The second parameter should be `index - 1` to properly compare like values, or the message should clarify that it's comparing an index vs a count.

**Corrected version:**
```c
quit("ERROR: Build_TrName_NameList - Mismatch in the number of tracers we are expected and the number we have built. We are expecting %d, but we have built %d tracers\n", nTrSize, index - 1);
```

---

## Bug 2: Line 644 - Swapped array dimensions in Ecology_Allocate_Diag_Arrays

**Location:** [`Ecology_Allocate_Diag_Arrays`](../atecology/atBuildTracer.c:622)

**Line 644:**
```c
bm->diagnost = Util_Alloc_Init_2D_Double(bm->ndiag, bm->nbox, 0.0);
```

**Issue:** The array dimensions are swapped. The allocation uses `(ndiag, nbox)` but the array is accessed as `diagnost[box][diag]` throughout the code.

**Evidence from usage:**
- **Line 648:** `bm->boxes[b].diagnost = bm->diagnost[b];` 
  - This assigns the pointer `diagnost[b]` to each box, where `b` ranges from 0 to `nbox-1`
  - This means the first dimension must be `nbox`

- **Lines 652-656:**
  ```c
  for (i = 0; i < bm->ndiag; i++) {
      for (b = 0; b < bm->nbox; b++) {
          bm->diagnost[b][i] = 0.0;
      }
  }
  ```
  - Access pattern is `diagnost[b][i]` where `b < nbox` and `i < ndiag`
  - This confirms first dimension should be `nbox`, second dimension should be `ndiag`

**Why it's an error:** When the array is allocated with swapped dimensions and then accessed using the expected index order, it will either cause out-of-bounds access (if ndiag ≠ nbox) or produce incorrect memory layout. This will lead to segmentation faults or data corruption.

**Corrected version:**
```c
bm->diagnost = Util_Alloc_Init_2D_Double(bm->nbox, bm->ndiag, 0.0);
```

---

## Summary

Two bugs found:
1. **Line 401:** Error message parameter mismatch - should use `index - 1` instead of `index`
2. **Line 644:** Swapped array dimensions - should be `(nbox, ndiag)` instead of `(ndiag, nbox)`

Both bugs could cause runtime issues, with Bug #2 being more critical as it can cause memory corruption and crashes.
