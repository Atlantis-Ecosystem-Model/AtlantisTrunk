# atlantisUtil/atUtilhelp.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [UTL2-11](#utl2-11) | 157-161 | H/M | off-by-one / buffer overrun | `istack = i_alloc1d(NSTACK)` gives valid indices 0..49. The check `if (jstack > NSTACK) quit` allows jstack ==… |  | [ ] |
| [UTL2-12](#utl2-12) | 162-170 | H/M | copy error from NR | `if (ir - 1 + i >= j - 1)` should be NR's `if (ir - i + 1 >= j - l)` (letter l, not 1). As written it is alway… |  | [ ] |

---

## UTL2-11

**off-by-one / buffer overrun** · Conf H / Sev M  
**Listed lines:** 157-161

**Function:** `Quicksort_Dir()`

**Why it's thought to be a bug**

`istack = i_alloc1d(NSTACK)` gives valid indices 0..49. The check `if (jstack > NSTACK) quit` allows jstack == NSTACK, so `istack[jstack] = ...` writes istack[50], one past the end. This was carried over from Numerical Recipes' 1-based `ivector(1,NSTACK)` without adjusting. Index 0 is never used. Function: `Quicksort_Dir()`.

**Current code** (`atlantisUtil/atUtilhelp.c` lines 155–163)

```c
  155: 			ddx[j] = d;
  156: 			eex[j] = e;
  157: 			jstack += 2;
  158: 
  159: 			/* Push pointers to larger subarray on stack, process smaller subarray immediately */
  160: 			if (jstack > NSTACK)
  161: 				quit("NSTACK too small in Quicksort_Dir() in economics library. Recode define statement.\n");
  162: 			if (ir - 1 + i >= j - 1) {
  163: 				istack[jstack] = ir;
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
**Listed lines:** 162-170

**Function:** `Quicksort_Dir()`

**Why it's thought to be a bug**

`if (ir - 1 + i >= j - 1)` should be NR's `if (ir - i + 1 >= j - l)` (letter l, not 1). As written it is always true (ir >= j, i >= 1), so the larger subarray is never pushed and the smaller is never processed first. Stack depth is then no longer bounded by log2(n), which makes the NSTACK quit/overrun in UTL2-11 reachable for large or adversarial inputs (atphysics/atgeomIO.c:879 sorts nbox entries; ateconomic/atquota.c:911, 1356). The dead else-branch is also wrong: `istack[jstack - 1] = i` should be `= l`. Function: `Quicksort_Dir()`.

**Current code** (`atlantisUtil/atUtilhelp.c` lines 160–172)

```c
  160: 			if (jstack > NSTACK)
  161: 				quit("NSTACK too small in Quicksort_Dir() in economics library. Recode define statement.\n");
  162: 			if (ir - 1 + i >= j - 1) {
  163: 				istack[jstack] = ir;
  164: 				istack[jstack - 1] = i;
  165: 				ir = j - 1;
  166: 			} else {
  167: 				istack[jstack] = j - 1;
  168: 				istack[jstack - 1] = i;
  169: 				l = i;
  170: 			}
  171: 		}
  172: 	}
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
