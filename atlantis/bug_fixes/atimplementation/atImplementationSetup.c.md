# atimplementation/atImplementationSetup.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [HRV2-24](#hrv2-24) | 24 | L/L | fixed buffer | `convertedXMLFileName[50]` uses a hard-coded name, so this is fine today. Note only: if it is ever built from … |  | [ ] |

---

## HRV2-24

**fixed buffer** · Conf L / Sev L  
**Found at (current code):** `atImplementationSetup.c` 24
**Lines at review (Oct 2026):** 24, 30

**Function:** `Implementation_Init()`

**Why it's thought to be a bug**

`convertedXMLFileName[50]` uses a hard-coded name, so this is fine today. Note only: if it is ever built from the fishprm filename (as in Harvest_Init, which uses STRLEN), it will overflow. Function: `Implementation_Init()`.

**Code at issue** — `atImplementationSetup.c` line 24  
Search for: `convertedXMLFileName[50]`

```c
   22: void Implementation_Init(MSEBoxModel *bm, FILE *llogfp){
   23: 
   24→ 	char convertedXMLFileName[50];
   25: 
   26: 	if ( !bm->flag_fisheries_on ){
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
