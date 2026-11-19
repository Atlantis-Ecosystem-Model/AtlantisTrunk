# atSS3Link/atSS3Test.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SS3-01](#ss3-01) | 39, 1420-1421, 1750, 1807-1808, 1904, 1978, 1985-1990 | H/L | leak / dimension mismatch | The code re-allocates arrays that `Tier_Assessment_Allocate` already allocated, never frees the originals, and… |  | [ ] |

---

## SS3-01

**leak / dimension mismatch** · Conf H / Sev L  
**Listed lines:** 39, 1420-1421, 1750, 1807-1808, 1904, 1978, 1985-1990

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
