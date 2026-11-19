# atlantisUtil/atUtilhelp.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [UTL2-11](#utl2-11) | 52, 160, 163 | H/M | off-by-one / buffer overrun | `istack = i_alloc1d(NSTACK)` gives valid indices 0..49. The check `if (jstack > NSTACK) quit` allows jstack ==… |  | [ ] |
| [UTL2-12](#utl2-12) | 162 | H/M | copy error from NR | `if (ir - 1 + i >= j - 1)` should be NR's `if (ir - i + 1 >= j - l)` (letter l, not 1). As written it is alway… |  | [ ] |

---

## UTL2-11

**off-by-one / buffer overrun** · Conf H / Sev M  
**Found at (current code):** `atUtilhelp.c` 52, `atUtilhelp.c` 160, `atUtilhelp.c` 163
**Lines at review (Oct 2026):** 157-161

**Function:** `Quicksort_Dir()`

**Why it's thought to be a bug**

`istack = i_alloc1d(NSTACK)` gives valid indices 0..49. The check `if (jstack > NSTACK) quit` allows jstack == NSTACK, so `istack[jstack] = ...` writes istack[50], one past the end. This was carried over from Numerical Recipes' 1-based `ivector(1,NSTACK)` without adjusting. Index 0 is never used. Function: `Quicksort_Dir()`.

**Code at issue** — `atUtilhelp.c` line 52 (closest match to the quoted code; wording differs slightly)  
Search for: `istack = (int *) i_alloc1d(NSTACK);`

```c
   50: 	int *istack;
   51: 
   52→ 	istack = (int *) i_alloc1d(NSTACK);
   53: 
   54: 	/* Ascending sort - taken from page 333 of Numerical Recipes in C
```

**Code at issue** — `atUtilhelp.c` line 160  
Search for: `if (jstack > NSTACK)`

```c
  158: 
  159: 			/* Push pointers to larger subarray on stack, process smaller subarray immediately */
  160→ 			if (jstack > NSTACK)
  161: 				quit("NSTACK too small in Quicksort_Dir() in economics library. Recode define statement.\n");
  162: 			if (ir - 1 + i >= j - 1) {
```

**Code at issue** — `atUtilhelp.c` line 163  
Search for: `istack[jstack] =`

```c
  161: 				quit("NSTACK too small in Quicksort_Dir() in economics library. Recode define statement.\n");
  162: 			if (ir - 1 + i >= j - 1) {
  163→ 				istack[jstack] = ir;
  164: 				istack[jstack - 1] = i;
  165: 				ir = j - 1;
```

**Suggested fix (from review):** Allocate `NSTACK+1`, or test `jstack >= NSTACK`.

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

## UTL2-12

**copy error from NR** · Conf H / Sev M  
**Found at (current code):** `atUtilhelp.c` 162
**Lines at review (Oct 2026):** 162-170

**Function:** `Quicksort_Dir()`

**Why it's thought to be a bug**

`if (ir - 1 + i >= j - 1)` should be NR's `if (ir - i + 1 >= j - l)` (letter l, not 1). As written it is always true (ir >= j, i >= 1), so the larger subarray is never pushed and the smaller is never processed first. Stack depth is then no longer bounded by log2(n), which makes the NSTACK quit/overrun in UTL2-11 reachable for large or adversarial inputs (atphysics/atgeomIO.c:879 sorts nbox entries; ateconomic/atquota.c:911, 1356). The dead else-branch is also wrong: `istack[jstack - 1] = i` should be `= l`. Function: `Quicksort_Dir()`.

**Code at issue** — `atUtilhelp.c` line 162  
Search for: `if (ir - 1 + i >= j - 1)`

```c
  160: 			if (jstack > NSTACK)
  161: 				quit("NSTACK too small in Quicksort_Dir() in economics library. Recode define statement.\n");
  162→ 			if (ir - 1 + i >= j - 1) {
  163: 				istack[jstack] = ir;
  164: 				istack[jstack - 1] = i;
```

**Suggested fix (from review):** Change the condition to `ir - i + 1 >= j - l` and the else-branch to `istack[jstack-1] = l;`.

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
