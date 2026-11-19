# atharvest/atHarvestTS.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [HRV2-12](#hrv2-12) | 246 | M/M | wrong operator | The expanding branch is documented as "use original distribution", but it does `CatchTS_agedistrib[...][ij] +=… |  | [ ] |
| [HRV2-13](#hrv2-13) | 240 | L/M | logic | In the flagkeepZeroCatchTS branch, when `CatchTS_agedistribOrig==0` the class is set to 0 and `cumtot` is rese… |  | [ ] |
| [HRV2-14](#hrv2-14) | 272 | H/L | logging | `if (do_debug \|\| (CatchTS_agedistrib[fishery_id][sp][0] > 0))` writes to the log for every age class whenever … |  | [ ] |

---

## HRV2-12

**wrong operator** · Conf M / Sev M  
**Found at (current code):** `atHarvestTS.c` 246
**Lines at review (Oct 2026):** 246

**Function:** `Harvest_Recalc_Time_Series_Distrib()`

**Why it's thought to be a bug**

The expanding branch is documented as "use original distribution", but it does `CatchTS_agedistrib[...][ij] += CatchTS_agedistribOrig[...][ij] + cumtot;`. That adds the original distribution on top of the current, contracted one, so the old distribution is never restored after normalisation. The depleted branch (241-242) correctly builds from Orig. Function: `Harvest_Recalc_Time_Series_Distrib()`.

**Code at issue** — `atHarvestTS.c` line 246  
Search for: `CatchTS_agedistrib[`

```c
  244: 				bm->pSPcheck[sp][ij] = 1;
  245:             } else {
  246→ 				bm->CatchTS_agedistrib[fishery_id][sp][ij] += bm->CatchTS_agedistribOrig[fishery_id][sp][ij] + cumtot;
  247: 				bm->DiscardTS_agedistrib[fishery_id][sp][ij] = bm->DiscardTS_agedistribOrig[fishery_id][sp][ij + ncum];
  248: 				cumtot = 0;
```

**Suggested fix (from review):** Use `=` instead of `+=`

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

## HRV2-13

**logic** · Conf L / Sev M  
**Found at (current code):** `atHarvestTS.c` 240
**Lines at review (Oct 2026):** 195-206

**Function:** `Harvest_Recalc_Time_Series_Distrib()`

**Why it's thought to be a bug**

In the flagkeepZeroCatchTS branch, when `CatchTS_agedistribOrig==0` the class is set to 0 and `cumtot` is reset to 0. The catch share accumulated from the depleted older classes is then thrown away instead of being carried down to the next class with catch. Function: `Harvest_Recalc_Time_Series_Distrib()`.

**Code at issue** — `atHarvestTS.c` line 240 (located by name only; check this is the right place)  
Search for: `if ((step1 < (bm->min_dens * bm->nbox * 2)) || (bm->flagkeepZeroCatchTS && !bm->CatchTS_ag`

```c
  238: 				step1 += sptotden[sp][bij];
  239: 			}
  240→ 			if ((step1 < (bm->min_dens * bm->nbox * 2)) || (bm->flagkeepZeroCatchTS && !bm->CatchTS_agedistribOrig[fishery_id][sp][ij])){
  241: 				cumtot += bm->CatchTS_agedistribOrig[fishery_id][sp][ij];
  242: 				bm->CatchTS_agedistrib[fishery_id][sp][ij] = 0;
```

**Suggested fix (from review):** Only reset cumtot when it was actually assigned

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

## HRV2-14

**logging** · Conf H / Sev L  
**Found at (current code):** `atHarvestTS.c` 272
**Lines at review (Oct 2026):** 272

**Function:** `Harvest_Recalc_Time_Series_Distrib()`

**Why it's thought to be a bug**

`if (do_debug || (CatchTS_agedistrib[fishery_id][sp][0] > 0))` writes to the log for every age class whenever the youngest class has any catch share, on every recalculation. This looks like a debug leftover that bloats the log. Function: `Harvest_Recalc_Time_Series_Distrib()`.

**Code at issue** — `atHarvestTS.c` line 272  
Search for: `if (do_debug || (bm->CatchTS_agedistrib[fishery_id][sp][0] > 0))`

```c
  270: 		}
  271: 
  272→ 		if (do_debug || (bm->CatchTS_agedistrib[fishery_id][sp][0] > 0)) {
  273: 			fprintf(llogfp, "Time: %e, %s-%d new_catchTSdistrib: %e\n", bm->dayt, FunctGroupArray[sp].groupCode, ij, bm->CatchTS_agedistrib[fishery_id][sp][ij]);
  274: 		}
```

**Suggested fix (from review):** `if (do_debug)`

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

atHarvestTS.c (202, 210, 247): `DiscardTS_agedistribOrig[..][ij + ncum]`. `ncum` is never reset after a non-depleted class, so the shift is the total number of depleted classes above, not the contiguous run. This stays in bounds; whether it is intended is unclear.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
