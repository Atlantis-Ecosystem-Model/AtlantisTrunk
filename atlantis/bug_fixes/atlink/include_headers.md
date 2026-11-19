# atlink/include/atComms.h — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [LINK-43](#link-43) | atlantisboxmodel.h:6095 | M/L | global defined in header | `EwELinkStruct *eweLinkModule;` is a definition in a header included by atlantisboxmodel.h (line 301, under LI… |  | [ ] |
| [LINK-44](#link-44) | 26 | M/L | global defined in header | `link_calc_types`, `link_data_types`, `link_data_entries` and `ExportName` are defined (not `extern`) in a hea… |  | [ ] |

## Reviewer notes for this file

No issues found (declares `sayHello`, which has no live definition; harmless unless called). Functions checked: prototypes.

---

## LINK-43

**global defined in header** · Conf M / Sev L  
**Found at (current code):** `atlantisboxmodel.h` 6095
**Lines at review (Oct 2026):** 49

**Why it's thought to be a bug**

`EwELinkStruct *eweLinkModule;` is a definition in a header included by atlantisboxmodel.h (line 301?, under LINK_ENABLED), so it lands in every TU. Under -fno-common (GCC >= 10, Clang default) this gives duplicate-symbol link errors. It is also the stray global misused in LINK-27 and LINK-37.

**Code at issue** — `atlantisboxmodel.h` line 6095  
Search for: `EwELinkStruct *eweLinkModule`

```c
 6093: 	/**< Linking model parameters */
 6094: 	//double *ConvertPRM;			   /**< parameters for linking to models like EwE - loaded from server app */
 6095→ 	EwELinkStruct *eweLinkModule;
 6096: #endif
 6097: 
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
**Found at (current code):** `atComms.h` 26
**Lines at review (Oct 2026):** 26-31

**Why it's thought to be a bug**

`link_calc_types`, `link_data_types`, `link_data_entries` and `ExportName` are defined (not `extern`) in a header included by three .c files. This is a duplicate-symbol error under -fno-common.

**Code at issue** — `atComms.h` line 26 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `int getBiomass(char *species, double **values, int numRows, int numColumns);`

```c
   24: 
   25: /* The get functions */
   26→ int getBiomass(char *species, double **values, int numRows, int numColumns);
   27: int getProduction(char *species, double **values, int numRows, int numColumns);
   28: int getMortality(char *species, double **values, int numRows, int numColumns);
```

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
