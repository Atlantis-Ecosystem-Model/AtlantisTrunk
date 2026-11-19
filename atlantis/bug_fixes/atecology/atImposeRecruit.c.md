# atecology/atImposeRecruit.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [BMSG-18](#bmsg-18) | 76-83 | M/M | uninitialised mapping | When imposed recruits are on, only boxes that have a time series get `BoxKeyMap[b][rects_key_id] = sp`. Every … |  | [ ] |
| [BMSG-19](#bmsg-19) | 62-74 | M/L | over-strict check | The species-id loop quits for any age-structured group missing from the MultRec file, even groups that use nei… |  | [ ] |

---

## BMSG-18

**uninitialised mapping** · Conf M / Sev M  
**Listed lines:** 76-83

**Function:** `Load_Imposed_Recruits()`

**Why it's thought to be a bug**

When imposed recruits are on, only boxes that have a time series get `BoxKeyMap[b][rects_key_id] = sp`. Every other box keeps the allocation default of 0 (`Util_Alloc_Init_2D_Int(3, nbox, 0)`, atlantismain.c:1911). So boxes with no series get recruits from series 0 instead of 0 recruits, and the `boxkey_id > -1` warning in `Get_Imposed_MultRecruits()` can never fire. Function: `Load_Imposed_Recruits()`.

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
**Listed lines:** 62-74

**Function:** `Load_Imposed_Recruits()`

**Why it's thought to be a bug**

The species-id loop quits for any age-structured group missing from the MultRec file, even groups that use neither `multiple_ts_recruit` nor stocking. Users have to add dummy columns for every vertebrate. Function: `Load_Imposed_Recruits()`.

**Current code** (`atecology/atImposeRecruit.c` lines 46–62) — code appears to have moved -14 lines since the review

```c
   46:         recruit_sp = (int) (FunctGroupArray[sp].speciesParams[flagrecruit_id]);
   47:         stocking_sp = (int) (FunctGroupArray[sp].speciesParams[flagstocking_id]);
   48:         if ((recruit_sp == multiple_ts_recruit) || (stocking_sp)){
   49:             flagimposerec = 1;
   50: 		}
   51: 	}
   52: 
   53: 	/* Load recruit time series */
   54: 	if (flagimposerec) {
   55: 		tsRecruitswarned = 0;
   56: 		Harvest_Read_Time_Series(bm->inputFolder, bm->forceIfname, "MultRec", bm->t_units, &tsRecruits, &ntsRecruits, &tsRecruitstype); // Reuse this as populates a list of time series
   57: 
   58: 		if (ntsRecruits) {
   59: 			FisheryTimeSeries *this_ts = &tsRecruits[0];
   60: 
   61: 			/* Assign species ids */
   62: 			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
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
