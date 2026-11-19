# atharvest/include/*.h — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [HRV2-19](#hrv2-19) | atHarvest.h:≈104 | M/L | multiple definition | `int first_data_done;` is a definition, not a declaration, in a header included by every atharvest .c file, so… |  | [ ] |

## Reviewer notes for this file

atHarvestIO.h, atHarvestLib.h, atHarvestPrivate.h: no other issues.

---

## HRV2-19

**multiple definition** · Conf M / Sev L  
**Lines (current code):** atHarvest.h:≈104  
**Lines at review (Oct 2026):** atHarvest.h:106

**Why it's thought to be a bug**

`int first_data_done;` is a definition, not a declaration, in a header included by every atharvest .c file, so each translation unit gets its own tentative definition. It links only under -fcommon; GCC>=10 and clang>=11 default to -fno-common, which gives a "duplicate symbol" link error. atHarvestImposedCatch.c:≈39 has the real definition commented out.

**Current code** (`atharvest/include/atHarvest.h` lines 102–106) — ≈ the code here has changed since the review; lines are approximate

```c
  102: extern int *tsdiscardid; /**< Array matching boxmodel species ids to discard time series entry ids */
  103: extern FisheryTimeSeries *tsDiscard; /**< List of discard time series (one per box) */
  104: extern int first_data_done;
  105: 
  106: 
```

**Suggested fix (from review):** `extern int first_data_done;` here, and define it once in atHarvestImposedCatch.c

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
