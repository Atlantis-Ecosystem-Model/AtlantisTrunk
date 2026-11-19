# atharvest/include/*.h — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [HRV2-19](#hrv2-19) | 104 | M/L | multiple definition | `int first_data_done;` is a definition, not a declaration, in a header included by every atharvest .c file, so… |  | [ ] |

## Reviewer notes for this file

atHarvestIO.h, atHarvestLib.h, atHarvestPrivate.h: no other issues.

---

## HRV2-19

**multiple definition** · Conf M / Sev L  
**Found at (current code):** `atHarvest.h` 104
**Lines at review (Oct 2026):** atHarvest.h:106

**Why it's thought to be a bug**

`int first_data_done;` is a definition, not a declaration, in a header included by every atharvest .c file, so each translation unit gets its own tentative definition. It links only under -fcommon; GCC>=10 and clang>=11 default to -fno-common, which gives a "duplicate symbol" link error. atHarvestImposedCatch.c:≈39 has the real definition commented out.

**Code at issue** — `atHarvest.h` line 104 (located by name only; check this is the right place)  
Search for: `extern int first_data_done;`

```c
  102: extern int *tsdiscardid; /**< Array matching boxmodel species ids to discard time series entry ids */
  103: extern FisheryTimeSeries *tsDiscard; /**< List of discard time series (one per box) */
  104→ extern int first_data_done;
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
