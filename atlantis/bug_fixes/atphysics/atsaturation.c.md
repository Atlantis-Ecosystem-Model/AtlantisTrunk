# atphysics/atsaturation.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY3-25](#phy3-25) | 73 | H/L | uninitialised variable | `do_break++` runs on an uninitialised `int do_break` (UB). The value is reset before use at line 98, so the re… |  | [ ] |

---

## PHY3-25

**uninitialised variable** · Conf H / Sev L  
**Found at (current code):** `atsaturation.c` 73
**Lines at review (Oct 2026):** 44,73,89

**Function:** `Saturation_Check()`

**Why it's thought to be a bug**

`do_break++` runs on an uninitialised `int do_break` (UB). The value is reset before use at line 98, so the result is harmless. Function: `Saturation_Check()`.

**Code at issue** — `atsaturation.c` line 73  
Search for: `do_break++`

```c
   71: 							newsed[b][k][n] = 8000.0;
   72: 					}
   73→ 					do_break++;
   74:                 } else if (strcmp(bm->tinfo[n].name, "Si") == 0) {
   75:                     for (k = 0; k < bp->nz; k++) {
```

**Suggested fix (from review):** Initialise do_break=0

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
