# atlink/include/atComms.h — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [LINK-43](#link-43) | 49 | M/L | global defined in header | `EwELinkStruct *eweLinkModule;` is a definition in a header included by atlantisboxmodel.h (line 301, under LI… |  | [ ] |
| [LINK-44](#link-44) | 26-31 | M/L | global defined in header | `link_calc_types`, `link_data_types`, `link_data_entries` and `ExportName` are defined (not `extern`) in a hea… |  | [ ] |

## Reviewer notes for this file

No issues found (declares `sayHello`, which has no live definition; harmless unless called). Functions checked: prototypes.

---

## LINK-43

**global defined in header** · Conf M / Sev L  
**Listed lines:** 49

**Why it's thought to be a bug**

`EwELinkStruct *eweLinkModule;` is a definition in a header included by atlantisboxmodel.h (line 301, under LINK_ENABLED), so it lands in every TU. Under -fno-common (GCC >= 10, Clang default) this gives duplicate-symbol link errors. It is also the stray global misused in LINK-27 and LINK-37.

**Current code** (`atlantismain/include/atlantisboxmodel.h` lines 290–294) — code appears to have moved +243 lines since the review

```c
  290:  and any existing value is overwritten.
  291:  
  292:  Also added support for an external mortality to be applied to a group. At present these values are never read in we just have the stub code to apply the mortality.
  293: 
  294: 
```

**Suggested fix (from review):** Remove it (the struct member bm->eweLinkModule exists), or make it `extern`.

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

## LINK-44

**global defined in header** · Conf M / Sev L  
**Listed lines:** 26-31

**Why it's thought to be a bug**

`link_calc_types`, `link_data_types`, `link_data_entries` and `ExportName` are defined (not `extern`) in a header included by three .c files. This is a duplicate-symbol error under -fno-common.

**Suggested fix (from review):** Declare them `extern` in the header and define them once in a .c file, or delete them (unused).

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
