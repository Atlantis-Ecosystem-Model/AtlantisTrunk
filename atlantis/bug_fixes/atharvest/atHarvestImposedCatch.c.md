# atharvest/atHarvestImposedCatch.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [HRV2-11](#hrv2-11) | 154-157 vs 82-88 | L/L | inconsistency | Discard species-id matching has no `break`, so the last matching column wins, while catch matching takes the f… |  | [ ] |

## Reviewer notes for this file

(See HRV2-03 for the active_scale disagreement between this file and atHarvestDiscards.c. This file's `dt/86400` appears to be the correct one.)

---

## HRV2-11

**inconsistency** · Conf L / Sev L  
**Lines (current code):** 154-157 vs 82-88  
**Lines at review (Oct 2026):** 154-157 vs 82-88

**Function:** `Load_Imposed_Discards()`

**Why it's thought to be a bug**

Discard species-id matching has no `break`, so the last matching column wins, while catch matching takes the first match. This only matters if a ts file has duplicate column names. Function: `Load_Imposed_Discards()`.

**Current code** (`atharvest/atHarvestImposedCatch.c` lines 152–159)

```c
  152: 				if (FunctGroupArray[sp].isFished == TRUE) {
  153: 					tsdiscardid[sp] = -1;
  154: 					for (b = 0; b < this_ts->ts.nv; b++) {
  155: 						if (strcmp(FunctGroupArray[sp].groupCode, this_ts->ts.varname[b]) == 0)
  156: 							tsdiscardid[sp] = b;
  157: 					}
  158: 					if (tsdiscardid[sp] < 0)
  159: 						quit("Did not find time series of discards for %s - check discard ts files contain it\n", FunctGroupArray[sp].groupCode);
```

**Suggested fix (from review):** Add `break`

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
