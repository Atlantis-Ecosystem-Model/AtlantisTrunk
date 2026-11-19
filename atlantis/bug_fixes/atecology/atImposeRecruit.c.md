# atecology/atImposeRecruit.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [BMSG-18](#bmsg-18) | 80, 110 | M/M | uninitialised mapping | When imposed recruits are on, only boxes that have a time series get `BoxKeyMap[b][rects_key_id] = sp`. Every … |  | [ ] |
| [BMSG-19](#bmsg-19) | 48 | M/L | over-strict check | The species-id loop quits for any age-structured group missing from the MultRec file, even groups that use nei… |  | [ ] |

---

## BMSG-18

**uninitialised mapping** · Conf M / Sev M  
**Found at (current code):** `atImposeRecruit.c` 80, `atImposeRecruit.c` 110
**Lines at review (Oct 2026):** 76-83

**Function:** `Load_Imposed_Recruits()`

**Why it's thought to be a bug**

When imposed recruits are on, only boxes that have a time series get `BoxKeyMap[b][rects_key_id] = sp`. Every other box keeps the allocation default of 0 (`Util_Alloc_Init_2D_Int(3, nbox, 0)`, atlantismain.c:2058). So boxes with no series get recruits from series 0 instead of 0 recruits, and the `boxkey_id > -1` warning in `Get_Imposed_MultRecruits()` can never fire. Function: `Load_Imposed_Recruits()`.

**Code at issue** — `atImposeRecruit.c` line 80  
Search for: `BoxKeyMap[b][rects_key_id] = sp`

```c
   78:                 for (sp = 0; sp < ntsRecruits; sp++) {
   79:                     if (b == tsRecruits[sp].b) {
   80→                         bm->BoxKeyMap[b][rects_key_id] = sp;
   81:                     }
   82:                 }
```

**Code at issue** — `atImposeRecruit.c` line 110  
Search for: `boxkey_id > -1`

```c
  108: 	 so don't have to have dummy fields in unfished boxes */
  109: 
  110→ 	if (boxkey_id > -1) {
  111: 
  112: 		loadFC = Get_Loaded_MultRecruits(bm, guildcase, boxkey_id, do_debug, llogfp);
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
**Found at (current code):** `atImposeRecruit.c` 48
**Lines at review (Oct 2026):** 62-74

**Function:** `Load_Imposed_Recruits()`

**Why it's thought to be a bug**

The species-id loop quits for any age-structured group missing from the MultRec file, even groups that use neither `multiple_ts_recruit` nor stocking. Users have to add dummy columns for every vertebrate. Function: `Load_Imposed_Recruits()`.

**Code at issue** — `atImposeRecruit.c` line 48 (located by name only; check this is the right place)  
Search for: `if ((recruit_sp == multiple_ts_recruit) || (stocking_sp)){`

```c
   46:         recruit_sp = (int) (FunctGroupArray[sp].speciesParams[flagrecruit_id]);
   47:         stocking_sp = (int) (FunctGroupArray[sp].speciesParams[flagstocking_id]);
   48→         if ((recruit_sp == multiple_ts_recruit) || (stocking_sp)){
   49:             flagimposerec = 1;
   50: 		}
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
