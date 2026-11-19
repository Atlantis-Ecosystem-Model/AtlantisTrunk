# atecology/atImposeRecruit.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [BMSG-18](#bmsg-18) | 76-83 | M/M | uninitialised mapping | When imposed recruits are on, only boxes that have a time series get `BoxKeyMap[b][rects_key_id] = sp`. Every … |  | [ ] |
| [BMSG-19](#bmsg-19) | 62-74 | M/L | over-strict check | The species-id loop quits for any age-structured group missing from the MultRec file, even groups that use nei… |  | [ ] |

---

## BMSG-18

**uninitialised mapping** · Conf M / Sev M  
**Lines (current code):** 76-83  
**Lines at review (Oct 2026):** 76-83

**Function:** `Load_Imposed_Recruits()`

**Why it's thought to be a bug**

When imposed recruits are on, only boxes that have a time series get `BoxKeyMap[b][rects_key_id] = sp`. Every other box keeps the allocation default of 0 (`Util_Alloc_Init_2D_Int(3, nbox, 0)`, atlantismain.c:2058). So boxes with no series get recruits from series 0 instead of 0 recruits, and the `boxkey_id > -1` warning in `Get_Imposed_MultRecruits()` can never fire. Function: `Load_Imposed_Recruits()`.

**Current code** (`atecology/atImposeRecruit.c` lines 74–85)

```c
   74: 			}
   75: 
   76: 			/* Assign box map ids */
   77:             for (b = 0; b < bm->nbox; b++) {
   78:                 for (sp = 0; sp < ntsRecruits; sp++) {
   79:                     if (b == tsRecruits[sp].b) {
   80:                         bm->BoxKeyMap[b][rects_key_id] = sp;
   81:                     }
   82:                 }
   83:             }
   84: 		} else
   85: 			quit("Expecting recruit time series but didn't find any\n");
```

**Suggested fix (from review):** Set `BoxKeyMap[b][rects_key_id] = -1` for all boxes before the mapping loop.

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

## BMSG-19

**over-strict check** · Conf M / Sev L  
**Lines (current code):** 62-74  
**Lines at review (Oct 2026):** 62-74

**Function:** `Load_Imposed_Recruits()`

**Why it's thought to be a bug**

The species-id loop quits for any age-structured group missing from the MultRec file, even groups that use neither `multiple_ts_recruit` nor stocking. Users have to add dummy columns for every vertebrate. Function: `Load_Imposed_Recruits()`.

**Current code** (`atecology/atImposeRecruit.c` lines 60–76) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
   60: 
   61: 			/* Assign species ids */
   62: 			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
   63: 				if ((FunctGroupArray[sp].groupAgeType == AGE_STRUCTURED) || (FunctGroupArray[sp].groupAgeType == AGE_STRUCTURED_BIOMASS)) {
   64: 					tsRecruitsid[sp] = -1;
   65: 					for (b = 0; b < this_ts->ts.nv; b++) {
   66: 						if (strcmp(FunctGroupArray[sp].groupCode, this_ts->ts.varname[b]) == 0) {
   67: 							tsRecruitsid[sp] = b;
   68: 							break;
   69: 						}
   70: 					}
   71: 					if (tsRecruitsid[sp] < 0)
   72: 						quit("Did not find time series of recruits for %s - check MultRec ts files contain it\n", FunctGroupArray[sp].groupCode);
   73: 				}
   74: 			}
   75: 
   76: 			/* Assign box map ids */
```

**Suggested fix (from review):** Only require (and map) groups where `flagrecruit == multiple_ts_recruit` or `flagstocking` is set; set others to -1.

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

## Lower-confidence notes (speculative — check before acting)

These came from section B.3 of the list. They were not raised as findings. Tick if you want one followed up.

### LC-1

atImposeRecruit.c 59-68 / 126-134: the column index `tsRecruitsid[sp]` is taken from `tsRecruits[0]`'s varnames and then used for every box's series. This silently assumes every MultRec box file has the same column order.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
