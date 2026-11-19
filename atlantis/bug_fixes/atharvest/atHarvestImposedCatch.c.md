# atharvest/atHarvestImposedCatch.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [HRV2-11](#hrv2-11) | 154 | L/L | inconsistency | Discard species-id matching has no `break`, so the last matching column wins, while catch matching takes the f… |  | [ ] |

## Reviewer notes for this file

(See HRV2-03 for the active_scale disagreement between this file and atHarvestDiscards.c. This file's `dt/86400` appears to be the correct one.)

---

## HRV2-11

**inconsistency** · Conf L / Sev L  
**Found at (current code):** `atHarvestImposedCatch.c` 154
**Lines at review (Oct 2026):** 154-157 vs 82-88

**Function:** `Load_Imposed_Discards()`

**Why it's thought to be a bug**

Discard species-id matching has no `break`, so the last matching column wins, while catch matching takes the first match. This only matters if a ts file has duplicate column names. Function: `Load_Imposed_Discards()`.

**Code at issue** — `atHarvestImposedCatch.c` line 154 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `for (b = 0; b < this_ts->ts.nv; b++) {`

```c
  152: 				if (FunctGroupArray[sp].isFished == TRUE) {
  153: 					tsdiscardid[sp] = -1;
  154→ 					for (b = 0; b < this_ts->ts.nv; b++) {
  155: 						if (strcmp(FunctGroupArray[sp].groupCode, this_ts->ts.varname[b]) == 0)
  156: 							tsdiscardid[sp] = b;
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
