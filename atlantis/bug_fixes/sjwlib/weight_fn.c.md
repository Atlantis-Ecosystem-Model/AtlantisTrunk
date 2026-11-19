# sjwlib/weight_fn.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW3-09](#sjw3-09) | 66 | H/M | wrong formula (missing fabs) | `if( fabs(x) <= scale ) return(1.0 - x/scale);` — the bound uses `fabs(x)` but the value uses signed `x`, so f… |  | [ ] |

---

## SJW3-09

**wrong formula (missing fabs)** · Conf H / Sev M  
**Found at (current code):** `weight_fn.c` 66
**Lines at review (Oct 2026):** 66-67

**Why it's thought to be a bug**

`if( fabs(x) <= scale ) return(1.0 - x/scale);` — the bound uses `fabs(x)` but the value uses signed `x`, so for negative x the weight is `1 +

**Code at issue** — `weight_fn.c` line 66  
Search for: `if( fabs(x) <= scale )`

```c
   64: )
   65: {
   66→     if( fabs(x) <= scale )
   67: 	return(1.0 - x/scale);
   68:     else
```

**Suggested fix (from review):** x

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
