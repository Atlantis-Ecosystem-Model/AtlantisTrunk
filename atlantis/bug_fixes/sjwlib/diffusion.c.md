# sjwlib/diffusion.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW3-08](#sjw3-08) | 31, 102 | H/L | static state not reset / dangling pointer | `free_diffusion1d()` frees `Cm1, C, Cp1, rhs, ud` but does not set them to NULL nor reset `max_n = -1`. A late… |  | [ ] |

---

## SJW3-08

**static state not reset / dangling pointer** · Conf H / Sev L  
**Found at (current code):** `diffusion.c` 31, `diffusion.c` 102
**Lines at review (Oct 2026):** 204-213

**Function:** `free_diffusion1d()`

**Why it's thought to be a bug**

`free_diffusion1d()` frees `Cm1, C, Cp1, rhs, ud` but does not set them to NULL nor reset `max_n = -1`. A later `diffusion1d()` call in the same process (e.g. a second run when Atlantis is embedded/re-invoked, or a call after `atphysics.c:113` teardown) would use freed memory if `n <= max_n`, or double-free them if `n > max_n`. Function: `free_diffusion1d()`.

**Code at issue** — `diffusion.c` line 31  
Search for: `max_n = -1`

```c
   29: 
   30: /* Local temporary storage */
   31→ static int max_n = -1;
   32: static double *Cm1;
   33: static double *C;
```

**Code at issue** — `diffusion.c` line 102  
Search for: `n > max_n`

```c
  100: 
  101:     /* Multiple layers - allocate temporary storage if necessary */
  102→     if( n > max_n ) {
  103: 	if( Cm1 != NULL ) {
  104: 	    free1d(Cm1);
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
