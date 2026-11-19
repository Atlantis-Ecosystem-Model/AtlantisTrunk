# sjwlib/weight_fn.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW3-09](#sjw3-09) | 66-67 | H/M | wrong formula (missing fabs) | `if( fabs(x) <= scale ) return(1.0 - x/scale);` — the bound uses `fabs(x)` but the value uses signed `x`, so f… |  | [ ] |

---

## SJW3-09

**wrong formula (missing fabs)** · Conf H / Sev M  
**Listed lines:** 66-67

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
