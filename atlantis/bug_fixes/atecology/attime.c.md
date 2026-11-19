# atecology/attime.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [EMISC-06](#emisc-06) ⭐ | 492-529 | H/H | logic / wrong variable | For groups with `numMoveEntries == 4`, `index = floor(TofY/91.25)` is compared with `orig_index`, but line 527… |  | [ ] |
| [EMISC-07](#emisc-07) | 125-128, 192-209 | M/L | off-by-one | `PointInYear` can be 365 (rounding up a remainder of 364.5 or more). That gives `DayInQuarter = 93 > DaysInQ =… |  | [ ] |
| [EMISC-08](#emisc-08) | 152-161 | M/L | stale value | `if (bm->dayt < 1)` runs inside `Time_OYr`, but `Ecology_Time_Check` only updates `bm->dayt` afterwards (line … |  | [ ] |

---

## EMISC-06

**logic / wrong variable** · Conf H / Sev H ⭐ priority  
**Listed lines:** 492-529

**Function:** `Calculate_Move_Date_Index()`

**Why it's thought to be a bug**

For groups with `numMoveEntries == 4`, `index = floor(TofY/91.25)` is compared with `orig_index`, but line 527 then overwrites `moveEntryIndex = bm->QofY`, whose boundaries are 90/181/272. Next day `orig_index` is QofY, not the 91.25-based index. Worked through: day 91 has orig=0 and index=0, so no update, yet moveEntryIndex becomes 1; day 92 has orig=1 and index=1, so no update. The same happens at days 182/183 and 273/274. As a result `NeedMoveUpdate` is set only at `TofY==0`, and forced-movement files (`Ecology_Update_Move_Entry`) are read only once a year for 4-season groups. Function: `Calculate_Move_Date_Index()`.

**Current code** (`atecology/attime.c` lines 490–529 (truncated))

```c
  490:                 FunctGroupArray[sp].NeedMoveUpdate[stage] = 0;
  491:             }
  492:             orig_index = FunctGroupArray[sp].moveEntryIndex;
  493:             
  494: 			/* find the length of each period */
  495:             delta = 365.0 / ((double)(FunctGroupArray[sp].numMoveEntries));
  496: 
  497: 			/* now find the index of the current season. */
  498: 			index = floor(bm->TofY / delta);
  499: 
  500: 			if (index >= FunctGroupArray[sp].numMoveEntries)
  501: 				index = FunctGroupArray[sp].numMoveEntries - 1;
  502:             
  503:             if (index < (FunctGroupArray[sp].numMoveEntries - 1))
  504:                 next_index = index + 1;
  505:             else
  506:                 next_index = 0;
  507:             
  508: 			FunctGroupArray[sp].moveEntryIndex = index;
  509:             FunctGroupArray[sp].next_moveEntryIndex = next_index;
  510: 
  511: 			//fprintf(bm->logFile, "Time: %e dayt, bm->TofY: %d, %s moveEntryIndex: %d orig_index: %d\n", bm->dayt, bm->TofY, FunctGroupArray[sp].groupCode, FunctGroupArray[sp].moveEntryIndex, orig_index);
  512: 
  513:             /* Check if need to read in new information */
  514:             if (!bm->TofY || (index != orig_index)) { // This way get annual check for those who don't really move within a year but can shift through time driven by and SDM
  515:                 for (stage = 0; stage < FunctGroupArray[sp].numStages; stage++) {
  516:                     FunctGroupArray[sp].NeedMoveUpdate[stage] = 1;
  517:                     
  518:                     //fprintf(bm->logFile, "Time: %e dayt, bm->TofY: %d, %s-%d set NeedMoveUpdate: %d\n", bm->dayt, bm->TofY, FunctGroupArray[sp].groupCode, stage, FunctGroupArray[sp].NeedMoveUpdate[stage]);
  519:                 }
  520:             }
  521:             
  522:             /* Also got to get HowFar if number of entries isn't one per quarter */
  523:             if (FunctGroupArray[sp].numMoveEntries != 4) {
  524:                 FunctGroupArray[sp].HowFar = (bm->TofY / delta) - floor(bm->TofY / delta);  // As only want the remainder for that movement period;
  525:             } else {
  526:                 FunctGroupArray[sp].HowFar = bm->HowFar;
  527:                 FunctGroupArray[sp].moveEntryIndex = bm->QofY;
  528:                 FunctGroupArray[sp].next_moveEntryIndex = bm->NextQofY;
  529:             }
```

**Suggested fix (from review):** Do the `numMoveEntries==4` override (index=QofY, next=NextQofY) before the `index != orig_index` test

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

## EMISC-07

**off-by-one** · Conf M / Sev L  
**Listed lines:** 125-128, 192-209

**Function:** `Time_OYr()`

**Why it's thought to be a bug**

`PointInYear` can be 365 (rounding up a remainder of 364.5 or more). That gives `DayInQuarter = 93 > DaysInQ = 92` and `HowFar > 1`. Q0 also counts days from 0 while Q1-Q3 count from 1, so `HowFar` reaches exactly 1.0 at the end of Q1-Q3. Function: `Time_OYr()`.

**Current code** (`atecology/attime.c` lines 123–130)

```c
  123: 	int NewWeek = 0;
  124: 
  125: 	if ((thisday / 365.0) < 1.0)
  126: 		PointInYear = (int) (floor(thisday + 0.5));
  127: 	else
  128: 		PointInYear = (int) (floor((thisday / 365.0 - floor(thisday / 365.0)) * 365.0 + 0.5));
  129: 
  130: 	/* Work out day of the week */
```

**Suggested fix (from review):** Wrap 365 to 0; make DayInQuarter consistent (subtract 91/182/273)

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

## EMISC-08

**stale value** · Conf M / Sev L  
**Listed lines:** 152-161

**Function:** `Time_OYr()`

**Why it's thought to be a bug**

`if (bm->dayt < 1)` runs inside `Time_OYr`, but `Ecology_Time_Check` only updates `bm->dayt` afterwards (line 94), so the test uses the previous step's day. Function: `Time_OYr()`.

**Current code** (`atecology/attime.c` lines 150–163)

```c
  150: 
  151: 	if (NewMonth) {
  152: 		if (bm->dayt < 1)
  153: 			bm->lastMofY = 11;
  154: 		else
  155: 			bm->lastMofY = bm->MofY;
  156: 
  157: 		/* Work out BiMonth (i.e. which bimonthly period of the year currently in) */
  158: 		if (bm->dayt < 1)
  159: 			bm->LastBiM = 5;
  160: 		else
  161: 			bm->LastBiM = bm->BiM;
  162: 	}
  163: 
```

**Suggested fix (from review):** Pass/compare `thisday`

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
