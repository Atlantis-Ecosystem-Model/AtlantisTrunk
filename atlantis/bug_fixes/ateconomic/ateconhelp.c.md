# ateconomic/ateconhelp.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [ECON-28](#econ-28) | 51, 86, 160 | H/L | off-by-one (1-based NR port) | `istack = i_alloc1d(NSTACK)` gives indices 0..49, but this follows Numerical Recipes' 1-based stack: `jstack +… |  | [ ] |
| [ECON-29](#econ-29) | 161 | H/L | typo vs reference algorithm | `if (ir - 1 + i >= j - l)`: NR has `ir - i + 1`. The wrong test picks the wrong subarray to push, so the stack… |  | [ ] |

---

## ECON-28

**off-by-one (1-based NR port)** · Conf H / Sev L  
**Found at (current code):** `ateconhelp.c` 51, `ateconhelp.c` 86, `ateconhelp.c` 160
**Lines at review (Oct 2026):** 51, 156-163

**Function:** `Quicksort_Dir()`

**Why it's thought to be a bug**

`istack = i_alloc1d(NSTACK)` gives indices 0..49, but this follows Numerical Recipes' 1-based stack: `jstack += 2; if (jstack > NSTACK) quit` lets `jstack == 50` through, and `istack[jstack]` then writes `istack[50]`. Function: `Quicksort_Dir()`.

**Code at issue** — `ateconhelp.c` line 51 (closest match to the quoted code; wording differs slightly)  
Search for: `istack = (int *) i_alloc1d(NSTACK);`

```c
   49: 	int *istack;
   50: 
   51→ 	istack = (int *) i_alloc1d(NSTACK);
   52: 
   53: 	/* Ascending sort - taken from page 333 of Numerical Recipes in C
```

**Code at issue** — `ateconhelp.c` line 86  
Search for: `istack[jstack]`

```c
   84: 				break;
   85: 			}
   86→ 			ir = istack[jstack]; /* Pop stack and begin a new round of partitioning */
   87: 			l = istack[jstack - 1];
   88: 			jstack -= 2;
```

**Code at issue** — `ateconhelp.c` line 160  
Search for: `Quicksort_Dir()`

```c
  158: 			/* Push pointers to larger subarray on stack, process smaller subarray immediately */
  159: 			if (jstack > NSTACK)
  160→ 				quit("NSTACK too small in Quicksort_Dir() in economics library. Recode define statement.\n");
  161: 			if (ir - 1 + i >= j - l) {
  162: 				istack[jstack] = ir;
```

**Suggested fix (from review):** Allocate `NSTACK + 1` or test `jstack >= NSTACK`

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

## ECON-29

**typo vs reference algorithm** · Conf H / Sev L  
**Found at (current code):** `ateconhelp.c` 161
**Lines at review (Oct 2026):** 161

**Function:** `Quicksort_Dir()`

**Why it's thought to be a bug**

`if (ir - 1 + i >= j - l)`: NR has `ir - i + 1`. The wrong test picks the wrong subarray to push, so the stack can grow linearly and hit the NSTACK quit or ECON-28. The sort order itself is still correct. Function: `Quicksort_Dir()`.

**Code at issue** — `ateconhelp.c` line 161  
Search for: `if (ir - 1 + i >= j - l)`

```c
  159: 			if (jstack > NSTACK)
  160: 				quit("NSTACK too small in Quicksort_Dir() in economics library. Recode define statement.\n");
  161→ 			if (ir - 1 + i >= j - l) {
  162: 				istack[jstack] = ir;
  163: 				istack[jstack - 1] = i;
```

**Suggested fix (from review):** `if (ir - i + 1 >= j - l)`

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
