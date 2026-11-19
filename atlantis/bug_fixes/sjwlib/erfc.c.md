# sjwlib/erfc.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW3-14](#sjw3-14) | 2 | M/L | precision / symbol override | File comment says it exists only because MSVC lacks erf/erfc, but it is compiled on all platforms (sjwlib/Make… |  | [ ] |

---

## SJW3-14

**precision / symbol override** · Conf M / Sev L  
**Found at (current code):** `erfc.c` 2
**Lines at review (Oct 2026):** 9-41

**Why it's thought to be a bug**

File comment says it exists only because MSVC lacks erf/erfc, but it is compiled on all platforms (sjwlib/Makefile.am c_sources). Because libsjwlib.a is linked before libm, any Atlantis reference to `erf`/`erfc` resolves to these approximations (A&S 7.1.26, abs. error ~1.5e-7; NR erfcc ~1.2e-7 rel.) rather than the full-precision libm versions, giving platform-dependent results. Functions: `erf()`, `erfc()`.

**Code at issue** — `erfc.c` line 2  
Search for: `erfc()`

```c
    1: /*
    2→  * erf() and erfc() are not included into MSVC math library while they are part of GNU libm.
    3:  * To compile STS using MSVC I had to add implementation of these functions below. Both
    4:  * are taken from Numerical recipes in C.
```

**Suggested fix (from review):** Wrap the file in `#ifdef _MSC_VER` (and the nrcdflib.h prototypes likewise).

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
