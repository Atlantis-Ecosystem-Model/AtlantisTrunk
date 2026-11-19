# atharvest/atHarvestDiscards.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **6 findings** (severity H: 2; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [HRV2-01](#hrv2-01) | 213, 332, 363 | H/M | cohort×genotype confusion | `Do_Dependent_Discards(..., basechrt, ...)` passes a cohort index that has already been divided by the genotyp… |  | [ ] |
| [HRV2-02](#hrv2-02) | 339-340, 442, 449 | H/M | cohort×genotype confusion | `icohort = floor(bcohort / numGeneTypes)` (339) is already an age class. `Calc_Dependent_Discards_For_Other_Sp… |  | [ ] |
| [HRV2-03](#hrv2-03) | 176-177, 380-381 | M/H | wrong operator / unit (inverted scale) | When the fishery is always active (`fisheriesflagactive==2`), the discard code sets `active_scale = 86400.0 / … |  | [ ] |
| [HRV2-04](#hrv2-04) | 191-192, 254 | M/H | division by zero / NaN | `loadDetFC = tsDetFC / (tsDetFC + loadFC);` has no small_num guard. When the discard time series is 0 and load… |  | [ ] |
| [HRV2-05](#hrv2-05) | 654-655, 666-667, 747-748, 762 | H/M | unit mismatch | In the adjacent_impose catch-up, each box is reduced by `step1 * bm->cell_vol * FCtsCarryOver[box] / (biomCarr… |  | [ ] |
| [HRV2-06](#hrv2-06) | 227, 490, 597 | H/L | copy-paste / log text | The quit text says "value must be between 0 and 2", but cases up to depend_discard exist. Line 597 says "reset… |  | [ ] |

---

## HRV2-01

**cohort×genotype confusion** · Conf H / Sev M  
**Lines (current code):** 213, 332, 363  
**Lines at review (Oct 2026):** 213, 332, 363

**Why it's thought to be a bug**

`Do_Dependent_Discards(..., basechrt, ...)` passes a cohort index that has already been divided by the genotype count (line 71). `Calc_Dependent_Discards_For_Species()` then divides again: `basechrt = chrt / numGeneTypes` (363). When numGeneTypes>1, `DiscardTS_agedistrib[nf][sp][basechrt]` reads the wrong (younger) age class. Functions: `Get_Discards()` -> `Do_Dependent_Discards()` -> `Calc_Dependent_Discards_For_Species()`.

**Current code** (`atharvest/atHarvestDiscards.c` lines 211–215)

```c
  211: 
  212: 		catch_step = 0.0;
  213: 		Do_Dependent_Discards(bm, impose_catch_depend_discard, guildcase, boxkey_id, nf, basechrt, vert_scale, *SPtoFC, &loadDetFC, &catch_step, llogfp);
  214: 		*SPtoFC += catch_step;  // In case any extra catch occurs as a result - due to catch of other species
  215: 
```

**Suggested fix (from review):** Use `chrt` directly in line 370, or pass the raw `chrt` from Get_Discards

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

## HRV2-02

**cohort×genotype confusion** · Conf H / Sev M  
**Lines (current code):** 339-340, 442, 449  
**Lines at review (Oct 2026):** 339-340, 442, 449

**Function:** `Calc_Dependent_Discards_For_Other_Species()`

**Why it's thought to be a bug**

`icohort = floor(bcohort / numGeneTypes)` (339) is already an age class. `Calc_Dependent_Discards_For_Other_Species()` divides it again: `ibasechrt = ichrt / numGeneTypes` (442). With genotypes it indexes the wrong age in `DiscardTS_agedistrib` (alloc [nf][sp][K_num_max_cohort], atlantismain.c:2146). Function: `Calc_Dependent_Discards_For_Other_Species()`.

**Current code** (`atharvest/atHarvestDiscards.c` lines 337–342)

```c
  337:             if(bm->FCcocatch[guildcase][i] > 0) {
  338:                 for(bcohort=0; bcohort<FunctGroupArray[i].numCohortsXnumGenes; bcohort++){
  339:                     icohort = (int)floor(bcohort / FunctGroupArray[i].numGeneTypes);
  340:                     step1 = Calc_Dependent_Discards_For_Other_Species(bm, i, icohort, guildcase, chrt, nf, vert_scale, base_catch_sp, llogfp);
  341:                     bm->DependDiscardsTot[nf][i][bcohort] += step1;
  342:                 }
```

**Suggested fix (from review):** `ibasechrt = ichrt;`

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

## HRV2-03

**wrong operator / unit (inverted scale)** · Conf M / Sev H  
**Lines (current code):** 176-177, 380-381  
**Lines at review (Oct 2026):** 176-177, 380-381

**Why it's thought to be a bug**

When the fishery is always active (`fisheriesflagactive==2`), the discard code sets `active_scale = 86400.0 / bm->dt`. The imposed-catch code uses the inverse, `active_scale = bm->dt / 86400.0` (atHarvestImposedCatch.c:255-256). At 12 h dt the imposed discards (and dependent discards at 381) for always-active fisheries come out 4x larger than the matching catch (2x vs 0.5x, before ts_dtscale). The comment at 374-379 ("if it calls twice then must halve the result") also says the result should be halved, not doubled. Functions: `Get_Discards()`, `Calc_Dependent_Discards_For_Species()`.

**Current code** (`atharvest/atHarvestDiscards.c` lines 174–179)

```c
  174: 
  175: 			/* If always active (vs only diurnally active) then adjust accordingly */
  176: 			if (bm->FISHERYprms[nf][fisheriesflagactive_id] == 2)
  177: 				active_scale = 86400.0 / bm->dt;
  178:             else {
  179:                 active_scale = 1.0;
```

**Suggested fix (from review):** Use `bm->dt / 86400.0` to match Get_Loaded_FC

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

## HRV2-04

**division by zero / NaN** · Conf M / Sev H  
**Lines (current code):** 191-192, 254  
**Lines at review (Oct 2026):** 191-192, 254

**Function:** `Get_Discards()`

**Why it's thought to be a bug**

`loadDetFC = tsDetFC / (tsDetFC + loadFC);` has no small_num guard. When the discard time series is 0 and loadFC is 0 the result is 0/0 = NaN. The later clamp `min(1.0,max(0.0,loadDetFC))` does not remove NaN, because comparisons with NaN are false and the macro returns NaN. The NaN discard fraction is then returned. Function: `Get_Discards()`.

**Current code** (`atharvest/atHarvestDiscards.c` lines 189–194)

```c
  189: 			else
  190: 				tsEvaled = tsEvalEx(&this_tsDiscard->ts, ts_id, bm->t);
  191: 			tsDetFC = active_scale * vert_scale * age_scale_catch * tsEvaled / (bm->cell_vol + small_num);
  192: 			loadDetFC = tsDetFC / (tsDetFC + loadFC);
  193: 		} else {
  194: 			if (!tsDiscardwarned) {
```

**Suggested fix (from review):** `/ (tsDetFC + loadFC + small_num)`

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

## HRV2-05

**unit mismatch** · Conf H / Sev M  
**Lines (current code):** 654-655, 666-667, 747-748, 762  
**Lines at review (Oct 2026):** 654-655, 666-667, 747-748, 762

**Function:** `Get_Imposed_Discards()`

**Why it's thought to be a bug**

In the adjacent_impose catch-up, each box is reduced by `step1 * bm->cell_vol * FCtsCarryOver[box] / (biomCarryOver + small_num)`. `biomCarryOver` is already the summed carry-over divided by cell_vol (579-590), so this has an extra factor of cell_vol. The intended total removal, `step1*cell_vol`, is what the global record gets (651). Per-box carry-overs are effectively zeroed every time, so the per-box and global (nbox) records drift apart. The discard version (747, 762) has the same problem. Function: `Get_Imposed_Discards()`.

**Current code** (`atharvest/atHarvestDiscards.c` lines 652–669)

```c
  652: 
  653: 					/* Update current cell */
  654: 					bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase] -= step1 * bm->cell_vol
  655: 							* bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase] / (biomCarryOver + small_num);
  656: 					if (bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase] < 0)
  657: 						bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase] = 0;
  658: 					/* Update adjacent cells */
  659: 					for (adbox = 0; adbox < bm->boxes[bm->current_box].nconn; adbox++) {
  660: 						chkbox = bm->boxes[bm->current_box].ibox[adbox];
  661: 						checkedBox[chkbox] = 0;
  662: 					}
  663: 					for (adbox = 0; adbox < bm->boxes[bm->current_box].nconn; adbox++) {
  664: 						chkbox = bm->boxes[bm->current_box].ibox[adbox];
  665: 						if (!checkedBox[chkbox]) {
  666: 							bm->FCtsCarryOver[chkbox][tscocatch_id][guildcase] -= step1 * bm->cell_vol * bm->FCtsCarryOver[chkbox][tscocatch_id][guildcase]
  667: 									/ (biomCarryOver + small_num);
  668: 							if (bm->FCtsCarryOver[chkbox][tscocatch_id][guildcase] < 0)
  669: 								bm->FCtsCarryOver[chkbox][tscocatch_id][guildcase] = 0;
```

**Suggested fix (from review):** Per box: `-= step1 * FCtsCarryOver[box] / (biomCarryOver + small_num)` (and the same with step3/disCarryOver)

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

## HRV2-06

**copy-paste / log text** · Conf H / Sev L  
**Lines (current code):** 227, 490, 597  
**Lines at review (Oct 2026):** 227, 490, 597

**Why it's thought to be a bug**

The quit text says "value must be between 0 and 2", but cases up to depend_discard exist. Line 597 says "reset to 0, 1 or 2", but cases 1-4 are valid (atlantisboxmodel.h:1268-1272). The debug line 490 prints the label "loadFC = %e" twice, and the second value is tsDetFC. Functions: `Get_Discards()`, `Get_Imposed_Discards()`.

**Current code** (`atharvest/atHarvestDiscards.c` lines 225–229) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
  225: 
  226: 	default:
  227: 		quit("No such discard option defined (%d) - value must be between 0 and 2 currently\n", flagspdiscard);
  228: 		break;
  229: 	}
```

**Suggested fix (from review):** Fix message text

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

atHarvestDiscards.c `Do_Dependent_Discards()` (338-341): in the non-impose branch, every genotype `bcohort` of species i gets the full `FCcocatch*base_catch_sp*age_scale` amount. With numGeneTypes>1 the dependent discards are multiplied by numGeneTypes, unless DependDiscardsTot is later divided by numGeneTypes (not checked; the consumer is outside the assigned files).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
