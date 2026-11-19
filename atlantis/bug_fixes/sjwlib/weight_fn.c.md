# sjwlib/weight_fn.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW3-09](#sjw3-09) | 66-67 | H/M | wrong formula (missing fabs) | `if( fabs(x) <= scale ) return(1.0 - x/scale);` — the bound uses `fabs(x)` but the value uses signed `x`, so f… |  | [ ] |

---

## SJW3-09

**wrong formula (missing fabs)** · Conf H / Sev M  
**Lines (current code):** 66-67  
**Lines at review (Oct 2026):** 66-67

**Why it's thought to be a bug**

`if( fabs(x) <= scale ) return(1.0 - x/scale);` — the bound uses `fabs(x)` but the value uses signed `x`, so for negative x the weight is `1 +

**Current code** (`sjwlib/weight_fn.c` lines 64–69)

```c
   64: )
   65: {
   66:     if( fabs(x) <= scale )
   67: 	return(1.0 - x/scale);
   68:     else
   69: 	return(0.0);
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
