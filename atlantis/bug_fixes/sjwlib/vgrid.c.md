# sjwlib/vgrid.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW3-10](#sjw3-10) | 41, 44 | H/L | scanf format mismatch | `scanf("%e",&c); ... scanf("%e",&d1);` — `c`, `d1` are `double` but `%e` expects `float*`, so only 4 bytes are… |  | [ ] |

## Reviewer notes for this file

(Standalone program, not built into libsjwlib.)

---

## SJW3-10

**scanf format mismatch** · Conf H / Sev L  
**Found at (current code):** `vgrid.c` 41, `vgrid.c` 44
**Lines at review (Oct 2026):** 41, 44

**Function:** `main()`

**Why it's thought to be a bug**

`scanf("%e",&c); ... scanf("%e",&d1);` — `c`, `d1` are `double` but `%e` expects `float*`, so only 4 bytes are written and the values are garbage. Also `n = nl-1` gives division by zero at line 51 when nl==1. Function: `main()`.

**Code at issue** — `vgrid.c` line 41  
Search for: `scanf("%e",&c)`

```c
   39:     n = nl-1;
   40:     fprintf(stderr,"Max depth (+ve)> ");
   41→     scanf("%e",&c);
   42:     c = -fabs(c);
   43:     fprintf(stderr,"Depth of bottom of surface layer (+ve)> ");
```

**Code at issue** — `vgrid.c` line 44  
Search for: `scanf("%e",&d1)`

```c
   42:     c = -fabs(c);
   43:     fprintf(stderr,"Depth of bottom of surface layer (+ve)> ");
   44→     scanf("%e",&d1);
   45:     d1 = fabs(d1);
   46: 
```

**Suggested fix (from review):** Use `%le`; guard `nl > 1`.

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
