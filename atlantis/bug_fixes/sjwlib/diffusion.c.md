# sjwlib/diffusion.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW3-08](#sjw3-08) | 204-213 | H/L | static state not reset / dangling pointer | `free_diffusion1d()` frees `Cm1, C, Cp1, rhs, ud` but does not set them to NULL nor reset `max_n = -1`. A late… |  | [ ] |

---

## SJW3-08

**static state not reset / dangling pointer** · Conf H / Sev L  
**Lines (current code):** 204-213  
**Lines at review (Oct 2026):** 204-213

**Function:** `free_diffusion1d()`

**Why it's thought to be a bug**

`free_diffusion1d()` frees `Cm1, C, Cp1, rhs, ud` but does not set them to NULL nor reset `max_n = -1`. A later `diffusion1d()` call in the same process (e.g. a second run when Atlantis is embedded/re-invoked, or a call after `atphysics.c:113` teardown) would use freed memory if `n <= max_n`, or double-free them if `n > max_n`. Function: `free_diffusion1d()`.

**Current code** (`sjwlib/diffusion.c` lines 202–214)

```c
  202:  *
  203:  */
  204: void free_diffusion1d(int dummy) {
  205: 
  206: 	if (Cm1 != NULL) {
  207: 		free1d(Cm1);
  208: 		free1d(C);
  209: 		free1d(Cp1);
  210: 		free1d(rhs);
  211: 		free1d(ud);
  212: 	}
  213: }
  214: 
```

**Suggested fix (from review):** After freeing, set all five pointers to NULL and `max_n = -1`.

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
