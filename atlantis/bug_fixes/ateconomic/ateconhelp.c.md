# ateconomic/ateconhelp.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [ECON-28](#econ-28) | 51, 156-163 | H/L | off-by-one (1-based NR port) | `istack = i_alloc1d(NSTACK)` gives indices 0..49, but this follows Numerical Recipes' 1-based stack: `jstack +… |  | [ ] |
| [ECON-29](#econ-29) | 161 | H/L | typo vs reference algorithm | `if (ir - 1 + i >= j - l)`: NR has `ir - i + 1`. The wrong test picks the wrong subarray to push, so the stack… |  | [ ] |

---

## ECON-28

**off-by-one (1-based NR port)** · Conf H / Sev L  
**Lines (current code):** 51, 156-163  
**Lines at review (Oct 2026):** 51, 156-163

**Function:** `Quicksort_Dir()`

**Why it's thought to be a bug**

`istack = i_alloc1d(NSTACK)` gives indices 0..49, but this follows Numerical Recipes' 1-based stack: `jstack += 2; if (jstack > NSTACK) quit` lets `jstack == 50` through, and `istack[jstack]` then writes `istack[50]`. Function: `Quicksort_Dir()`.

**Current code** (`ateconomic/ateconhelp.c` lines 49–53)

```c
   49: 	int *istack;
   50: 
   51: 	istack = (int *) i_alloc1d(NSTACK);
   52: 
   53: 	/* Ascending sort - taken from page 333 of Numerical Recipes in C
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
**Lines (current code):** 161  
**Lines at review (Oct 2026):** 161

**Function:** `Quicksort_Dir()`

**Why it's thought to be a bug**

`if (ir - 1 + i >= j - l)`: NR has `ir - i + 1`. The wrong test picks the wrong subarray to push, so the stack can grow linearly and hit the NSTACK quit or ECON-28. The sort order itself is still correct. Function: `Quicksort_Dir()`.

**Current code** (`ateconomic/ateconhelp.c` lines 159–163)

```c
  159: 			if (jstack > NSTACK)
  160: 				quit("NSTACK too small in Quicksort_Dir() in economics library. Recode define statement.\n");
  161: 			if (ir - 1 + i >= j - l) {
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
