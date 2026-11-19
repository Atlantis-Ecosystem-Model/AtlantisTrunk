# atimplementation/atImplementationSetup.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [HRV2-24](#hrv2-24) | 24, 30 | L/L | fixed buffer | `convertedXMLFileName[50]` uses a hard-coded name, so this is fine today. Note only: if it is ever built from … |  | [ ] |

---

## HRV2-24

**fixed buffer** · Conf L / Sev L  
**Lines (current code):** 24, 30  
**Lines at review (Oct 2026):** 24, 30

**Function:** `Implementation_Init()`

**Why it's thought to be a bug**

`convertedXMLFileName[50]` uses a hard-coded name, so this is fine today. Note only: if it is ever built from the fishprm filename (as in Harvest_Init, which uses STRLEN), it will overflow. Function: `Implementation_Init()`.

**Current code** (`atimplementation/atImplementationSetup.c` lines 22–32)

```c
   22: void Implementation_Init(MSEBoxModel *bm, FILE *llogfp){
   23: 
   24: 	char convertedXMLFileName[50];
   25: 
   26: 	if ( !bm->flag_fisheries_on ){
   27: 		return;
   28: 	}
   29: 
   30: 	strcpy(convertedXMLFileName, "implementation.xml");
   31: 
   32: 	/* Convert the input file to XML */
```

**Suggested fix (from review):** Use STRLEN

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
