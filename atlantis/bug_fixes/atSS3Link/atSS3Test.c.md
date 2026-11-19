# atSS3Link/atSS3Test.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SS3-01](#ss3-01) | 39, 1420-1421, 1750, 1807-1808, 1904, 1978, 1985-1990 | H/L | leak / dimension mismatch | The code re-allocates arrays that `Tier_Assessment_Allocate` already allocated, never frees the originals, and… |  | [ ] |

---

## SS3-01

**leak / dimension mismatch** · Conf H / Sev L  
**Lines (current code):** 39, 1420-1421, 1750, 1807-1808, 1904, 1978, 1985-1990  
**Lines at review (Oct 2026):** 39, 1420-1421, 1750, 1807-1808, 1904, 1978, 1985-1990

**Function:** `SS3_Test_Init()`

**Why it's thought to be a bug**

The code re-allocates arrays that `Tier_Assessment_Allocate` already allocated, never frees the originals, and uses different extents. `LenComp` uses 3 data types where the setup uses 4 (atAssessTierSetup.c:169). `AgeComp` uses 3 types and MaxAge where the setup uses 4 and OverallMaxAge (:138). `AgeFltYr` uses 4 where the setup uses 3 (:137). `Ageing_error` uses AccumAge where the setup uses AccumAge+1 (:175). `WtLen`, `LenRetain` and `SelLen` are also re-allocated. Line 39 `nYears = HistYrMax - HistYrMin` is missing the +1. The file is not compiled. Function: `SS3_Test_Init()`.

**Current code** (`atSS3Link/atSS3Test.c` lines 37–41)

```c
   37: 	bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMax_id] = 2008;
   38: 
   39: 	nYears = bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMax_id] - bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id];
   40: 
   41: 	bm->RBCestimation.RBCspeciesArray[groupIndex].CatchData[0][sumregion][0] = 445.6;
```

**Suggested fix (from review):** Delete the file, or drop the re-allocations and use the setup extents

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---
