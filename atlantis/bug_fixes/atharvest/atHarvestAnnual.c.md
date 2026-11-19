# atharvest/atHarvestAnnual.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 1; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [HRV2-16](#hrv2-16) | 201 | M/H | buffer overrun | `TotCumCatch[k][n][bm->thisyear+1] = 0.0;`. TotCumCatch is allocated with `max_year = (int)(tstop/yr)+1` years… |  | [ ] |
| [HRV2-17](#hrv2-17) | 195 | M/M | wrong index (year) | `bm->TotOldCumCatch[k][n] += TotCumCatch[k][n][bm->thisyear];` runs after `thisyear++` (atlantismain.c:385), s… |  | [ ] |
| [HRV2-18](#hrv2-18) | 92 | H/L | unit mismatch (log) | FCtsCarryOver is already stored multiplied by cell_vol (mg s-1, see atHarvestDiscards.c:503). The per-box repo… |  | [ ] |

---

## HRV2-16

**buffer overrun** · Conf M / Sev H  
**Found at (current code):** `atHarvestAnnual.c` 201
**Lines at review (Oct 2026):** 201

**Function:** `Harvest_Annual_Reset()`

**Why it's thought to be a bug**

`TotCumCatch[k][n][bm->thisyear+1] = 0.0;`. TotCumCatch is allocated with `max_year = (int)(tstop/yr)+1` years (atHarvestSetup.c:428-442). The main loop runs `while t < tstop` (atlantismain.c:822), and thisyear is incremented at each year start. If tstop is not a whole number of years (e.g. 10.5 yr), the last annual call has thisyear = floor(tstop/yr) = max_year-1, so thisyear+1 = max_year is out of range. This writes into the next fishery's year-0 entry, or past the block end for the last sp/nf. Function: `Harvest_Annual_Reset()`.

**Code at issue** — `atHarvestAnnual.c` line 201  
Search for: `TotCumCatch[k][n][bm->thisyear+1] = 0.0`

```c
  199: 
  200: 			if (FunctGroupArray[k].isTAC) {
  201→ 				TotCumCatch[k][n][bm->thisyear+1] = 0.0;  // Ready for the new year (just to be sure)
  202: 				bm->TotCumRecCatch[k][n] = 0.0;
  203: 			}
```

**Suggested fix (from review):** Allocate `ceil(tstop/yr)+1` years, or guard `if (thisyear+1 < max_year)`

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

## HRV2-17

**wrong index (year)** · Conf M / Sev M  
**Found at (current code):** `atHarvestAnnual.c` 195
**Lines at review (Oct 2026):** 195

**Function:** `Harvest_Annual_Reset()`

**Why it's thought to be a bug**

`bm->TotOldCumCatch[k][n] += TotCumCatch[k][n][bm->thisyear];` runs after `thisyear++` (atlantismain.c:451), so it adds the new, empty year. The comment says the intent is the year just finished ("Could use TotCumCatch[thisyear-1] instead"). With bulkTAC the multi-year cumulative catch is never accumulated, which breaks bulk-TAC over-quota checks (atManage.c:809, atHarvestDiscards.c:279). Function: `Harvest_Annual_Reset()`.

**Code at issue** — `atHarvestAnnual.c` line 195  
Search for: `TotOldCumCatch[k][n] += TotCumCatch[k][n][bm->thisyear]`

```c
  193: 					bm->TotOldCumCatch[k][n] = 0; // If just do test above then will fail as will have new TAC setting in between which resets the counters
  194: 				}
  195→ 				bm->TotOldCumCatch[k][n] += TotCumCatch[k][n][bm->thisyear];  // Could use TotCumCatch[k][n][bm->thisyear-1] instead of TotOldCumCatch, but then would have to add exception checks so keep TotOldCumCatch (at least for now)
  196: 				if(!bm->bulkTAC)
  197: 					bm->TotOldCumCatch[k][n] = 0.0; // As not summing over years, so don't count previous years in cumulative sums
```

**Suggested fix (from review):** Use `TotCumCatch[k][n][bm->thisyear-1]` when thisyear>0

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

## HRV2-18

**unit mismatch (log)** · Conf H / Sev L  
**Found at (current code):** `atHarvestAnnual.c` 92
**Lines at review (Oct 2026):** 91-99, 110-112

**Function:** `Report_Annual_Harvest()`

**Why it's thought to be a bug**

FCtsCarryOver is already stored multiplied by cell_vol (mg s-1, see atHarvestDiscards.c:503). The per-box report multiplies by `cell_vol` again. Undercatch (92) and underdiscard (96) leave out `dt`, while the alternate underdiscard (99) includes it. The global line (111) applies `dt` only to discards. The reported tonnages are inconsistent and wrong. Function: `Report_Annual_Harvest()`.

**Code at issue** — `atHarvestAnnual.c` line 92 (the line the review pointed to, found by its text)  
Search for: `bm->FCtsCarryOver[b][tscocatch_id][sp] * bm->cell_vol * bm->X_CN * mg_2_tonne);`

```c
   90: 					did_printout = 1;
   91: 					fprintf(llogfp, "Time: %e, box %d %s had %e t undercatch", bm->dayt, b, FunctGroupArray[sp].groupCode,
   92→ 							bm->FCtsCarryOver[b][tscocatch_id][sp] * bm->cell_vol * bm->X_CN * mg_2_tonne);
   93: 				}
   94: 				if (bm->FCtsCarryOver[b][tscodiscard_id][sp] > 0) {
```

**Suggested fix (from review):** Use `* bm->dt * bm->X_CN * mg_2_tonne` (no cell_vol) consistently

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
