# atecology/attime.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [EMISC-06](#emisc-06) ⭐ | 498, 527 | H/H | logic / wrong variable | For groups with `numMoveEntries == 4`, `index = floor(TofY/91.25)` is compared with `orig_index`, but line 527… |  | [ ] |
| [EMISC-07](#emisc-07) | 209 | M/L | off-by-one | `PointInYear` can be 365 (rounding up a remainder of 364.5 or more). That gives `DayInQuarter = 93 > DaysInQ =… |  | [ ] |
| [EMISC-08](#emisc-08) | 152 | M/L | stale value | `if (bm->dayt < 1)` runs inside `Time_OYr`, but `Ecology_Time_Check` only updates `bm->dayt` afterwards (line … |  | [ ] |

---

## EMISC-06

**logic / wrong variable** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `attime.c` 498, `attime.c` 527
**Lines at review (Oct 2026):** 492-529

**Function:** `Calculate_Move_Date_Index()`

**Why it's thought to be a bug**

For groups with `numMoveEntries == 4`, `index = floor(TofY/91.25)` is compared with `orig_index`, but line 527 then overwrites `moveEntryIndex = bm->QofY`, whose boundaries are 90/181/272. Next day `orig_index` is QofY, not the 91.25-based index. Worked through: day 91 has orig=0 and index=0, so no update, yet moveEntryIndex becomes 1; day 92 has orig=1 and index=1, so no update. The same happens at days 182/183 and 273/274. As a result `NeedMoveUpdate` is set only at `TofY==0`, and forced-movement files (`Ecology_Update_Move_Entry`) are read only once a year for 4-season groups. Function: `Calculate_Move_Date_Index()`.

**Code at issue** — `attime.c` line 498 (closest match to the quoted code; wording differs slightly)  
Search for: `index = floor(bm->TofY / delta);`

```c
  496: 
  497: 			/* now find the index of the current season. */
  498→ 			index = floor(bm->TofY / delta);
  499: 
  500: 			if (index >= FunctGroupArray[sp].numMoveEntries)
```

**Code at issue** — `attime.c` line 527  
Search for: `moveEntryIndex = bm->QofY`

```c
  525:             } else {
  526:                 FunctGroupArray[sp].HowFar = bm->HowFar;
  527→                 FunctGroupArray[sp].moveEntryIndex = bm->QofY;
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
**Found at (current code):** `attime.c` 209
**Lines at review (Oct 2026):** 125-128, 192-209

**Function:** `Time_OYr()`

**Why it's thought to be a bug**

`PointInYear` can be 365 (rounding up a remainder of 364.5 or more). That gives `DayInQuarter = 93 > DaysInQ = 92` and `HowFar > 1`. Q0 also counts days from 0 while Q1-Q3 count from 1, so `HowFar` reaches exactly 1.0 at the end of Q1-Q3. Function: `Time_OYr()`.

**Code at issue** — `attime.c` line 209 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->HowFar = ((double)(DayInQuarter)) / ((double)(DaysInQ));`

```c
  207: 	else
  208: 		DaysInQ = 92;
  209→ 	bm->HowFar = ((double)(DayInQuarter)) / ((double)(DaysInQ));
  210: 	if (QuarterOfYear < 3)
  211: 		NextQuarterOfYear = QuarterOfYear + 1;
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
**Found at (current code):** `attime.c` 152
**Lines at review (Oct 2026):** 152-161

**Function:** `Time_OYr()`

**Why it's thought to be a bug**

`if (bm->dayt < 1)` runs inside `Time_OYr`, but `Ecology_Time_Check` only updates `bm->dayt` afterwards (line 94), so the test uses the previous step's day. Function: `Time_OYr()`.

**Code at issue** — `attime.c` line 152  
Search for: `if (bm->dayt < 1)`

```c
  150: 
  151: 	if (NewMonth) {
  152→ 		if (bm->dayt < 1)
  153: 			bm->lastMofY = 11;
  154: 		else
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
