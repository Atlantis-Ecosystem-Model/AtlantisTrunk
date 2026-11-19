# atharvest/atHarvest.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **7 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 4 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [HARV-01](#harv-01) | 308-310 | H/M | wrong loop direction | `for (d = 0; d < K-1; d++) CatchQueue[..][d + 1] = CatchQueue[..][d];` is a forward shift, so `CatchQueue[0]` … |  | [ ] |
| [HARV-02](#harv-02) | 625 | L/L | wrong variable | `bm->Discards[...] += bm->FishingResults[nf][Discards_result_id];` reads the global array, not the `FishingRes… |  | [ ] |
| [HARV-03](#harv-03) | 780-792 | M/M | copy-paste / inconsistent table | In the branch where both use reef (`check1`), the fishery uses flats but the group does not (`FC_likeFLAT`), a… |  | [ ] |
| [HARV-04](#harv-04) | 1503 | H/M | wrong parameter id | `Get_Fishery_Change_Scale(bm, nf, P_num_changes_id, P_num_changes_id, Pchange)` passes the number of changes a… |  | [ ] |
| [HARV-05](#harv-05) | 1932, 1940-1949 | H/M | logic / data loss | (a) `Qchange[species][fishery] == 0` compares a pointer to 0, which is never true, so the "overwrite single du… |  | [ ] |
| [HARV-06](#harv-06) | 1981 | H/L | wrong variable (copy-paste) | `tempArray = Util_Alloc_Init_4D_Double(3, Q_max_num_changes, ...)` is the mFC copy of `Harvest_Scale_Q`, but i… |  | [ ] |
| [HARV-07](#harv-07) | 1732 | L/L | wrong error text | The quit message says "value must be between 0 and 6", but there are 9 curve ids (up to `q_binormal_id`). The … |  | [ ] |

---

## HARV-01

**wrong loop direction** · Conf H / Sev M  
**Listed lines:** 308-310

**Function:** `Harvest_Update_Temp_Catch_Array()`

**Why it's thought to be a bug**

`for (d = 0; d < K-1; d++) CatchQueue[..][d + 1] = CatchQueue[..][d];` is a forward shift, so `CatchQueue[0]` gets copied into every slot (the queue collapses to `[new, old0, old0, ...]`). After that, the running `LastCatch` total (subtract slot K-1, add slot 0) no longer matches the queue contents, so the CPUE used in atmanage/atManage.c:291 is wrong whenever `K_num_catchqueue > 2`. Function: `Harvest_Update_Temp_Catch_Array()`.

**Current code** (`atharvest/atHarvest.c` lines 306–312)

```c
  306: 
  307: 						/* Shift everything along the CatchQueue one spot */
  308: 						for (d = 0; d < bm->K_num_catchqueue - 1; d++) {
  309: 							bm->CatchQueue[sp][n][ij][d + 1] = bm->CatchQueue[sp][n][ij][d];
  310: 						}
  311: 						/* Reinitialise first (most recent) spot in the queue as about to fill it */
  312: 						bm->CatchQueue[sp][n][ij][0] = 0;
```

**Suggested fix (from review):** Shift from the top down: `for (d = K-1; d > 0; d--) Q[d] = Q[d-1];`

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

## HARV-02

**wrong variable** · Conf L / Sev L  
**Listed lines:** 625

**Function:** `Harvest_Do_Fishing_And_ByCatch()`

**Why it's thought to be a bug**

`bm->Discards[...] += bm->FishingResults[nf][Discards_result_id];` reads the global array, not the `FishingRes` argument the function fills in at line 595. This only works because the single caller (atecology/atprocess.c:1982) passes `bm->FishingResults`. Function: `Harvest_Do_Fishing_And_ByCatch()`.

**Current code** (`atharvest/atHarvest.c` lines 623–627)

```c
  623: 
  624: 		/* Update the discards values */
  625: 		bm->Discards[bm->current_box][guildcase][nf] += bm->FishingResults[nf][Discards_result_id];
  626: 
  627: 		/* Update any impacts on rugosity */
```

**Suggested fix (from review):** Use `FishingRes[nf][Discards_result_id]`.

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

## HARV-03

**copy-paste / inconsistent table** · Conf M / Sev M  
**Listed lines:** 780-792

**Function:** `Habitat_Overlap()`

**Why it's thought to be a bug**

In the branch where both use reef (`check1`), the fishery uses flats but the group does not (`FC_likeFLAT`), and both use soft (`check3`), the group's habitat (reef+soft) is fully covered by the fishery. Every other "fully covered" case in the table gives 1.0, e.g. lines 825-833 (same group, fishery without flats) and 1124-1134. This case gives `coveramt[REEF]+coveramt[SOFT]` instead, so adding a habitat the group does not use lowers the overlap. Function: `Habitat_Overlap()`.

**Current code** (`atharvest/atHarvest.c` lines 778–794)

```c
  778: 				}
  779: 			}
  780: 		} else if (FC_likeFLAT) { /* Not conincident because fish not on flats */
  781: 			if (check3) { /* Both on soft */
  782: 				if (check4) { /* Both in canyons */
  783: 					p_fish_origi[b][sp][stage][nf] = (bm->coveramt[bm->REEFcover_id][b] + bm->coveramt[bm->SOFTcover_id][b]) * (1.0
  784: 							+ bm->coveramt[bm->CANYONcover_id][b]);
  785: 				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
  786: 					p_fish_origi[b][sp][stage][nf] = (bm->coveramt[bm->REEFcover_id][b] + bm->coveramt[bm->SOFTcover_id][b]) * (1.0
  787: 							- bm->coveramt[bm->CANYONcover_id][b]);
  788: 				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
  789: 					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b] + bm->coveramt[bm->SOFTcover_id][b];
  790: 				} else { /* Neither in canyons */
  791: 					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b] + bm->coveramt[bm->SOFTcover_id][b];
  792: 				}
  793: 			} else if (sp_likeSOFT) { /* Not conincident because fishery not on soft */
  794: 				if (check4) { /* Both in canyons */
```

**Suggested fix (from review):** Set the four sub-cases to the 1.0 / `1.0 - canyon` pattern used at lines 826-833.

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

## HARV-04

**wrong parameter id** · Conf H / Sev M  
**Listed lines:** 1503

**Function:** `Harvest_Update_Habitat_Overlap()`

**Why it's thought to be a bug**

`Get_Fishery_Change_Scale(bm, nf, P_num_changes_id, P_num_changes_id, Pchange)` passes the number of changes as the flag parameter, so the coverage change is applied whenever `P_num_changes > 0`, even when the fishery's `flagchangeP` (`flagchangeP_id`, read at atHarvestParamIO.c:593) is 0. Compare line 773 of atHarvestCatch.c, which passes `flagchangeSWEPT_id, SWEPT_num_changes_id`. Function: `Harvest_Update_Habitat_Overlap()`.

**Current code** (`atharvest/atHarvest.c` lines 1501–1505)

```c
 1501: 			 in days too */
 1502: 
 1503: 		P_scale = Get_Fishery_Change_Scale(bm, nf, P_num_changes_id, P_num_changes_id, Pchange);
 1504: 
 1505: 		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
```

**Suggested fix (from review):** Pass `flagchangeP_id, P_num_changes_id`.

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

## HARV-05

**logic / data loss** · Conf H / Sev M  
**Listed lines:** 1932, 1940-1949

**Function:** `Harvest_Scale_Q()`

**Why it's thought to be a bug**

(a) `Qchange[species][fishery] == 0` compares a pointer to 0, which is never true, so the "overwrite single dummy" shortcut never runs. (b) The new array `tempArray` is zero-initialised, but `Util_Copy_Change_Values` copies only the `[species][fishery]` row, and then the old `Qchange` is freed. Every other species/fishery catchability-change schedule is wiped to zero. Called from atCLAMLink/atCLAMImplement.c:40. Function: `Harvest_Scale_Q()`.

**Current code** (`atharvest/atHarvest.c` lines 1930–1951)

```c
 1930: 
 1931: 	/* Check to see if there is just a single dummy value in the array - if so just overwrite it */
 1932: 	if (Q_max_num_changes == 1 && Qchange[species][fishery] == 0) {
 1933: 		Qchange[species][fishery][0][start_id] = start + bm->dayt;
 1934: 		Qchange[species][fishery][0][period_id] = period;
 1935: 		Qchange[species][fishery][0][mult_id] = mult;
 1936: 		return;
 1937: 	}
 1938: 
 1939: 	/* Allocate a new array */
 1940: 	tempArray = Util_Alloc_Init_4D_Double(3, Q_max_num_changes, bm->K_num_fisheries, bm->K_num_tot_sp, 0.0);
 1941: 
 1942: 	/* Now insert the new value in the correct place */
 1943: 	Util_Copy_Change_Values(bm, Qchange[species][fishery], tempArray[species][fishery], (int)bm->SP_FISHERYprms[species][fishery][Q_num_changes_id], start, period,
 1944: 			mult);
 1945: 
 1946: 	/* Now do some pointer magic and free up the 'old' array*/
 1947: 	tempArrayPointer = Qchange;
 1948: 	Qchange = tempArray;
 1949: 	free4d(tempArrayPointer);
 1950: }
 1951: 
```

**Suggested fix (from review):** Test `Qchange[species][fishery][0][start_id] == 0`, and copy all other `[sp][nf][i][*]` entries into `tempArray` before the swap.

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

## HARV-06

**wrong variable (copy-paste)** · Conf H / Sev L  
**Listed lines:** 1981

**Function:** `Harvest_Scale_mFC()`

**Why it's thought to be a bug**

`tempArray = Util_Alloc_Init_4D_Double(3, Q_max_num_changes, ...)` is the mFC copy of `Harvest_Scale_Q`, but it sizes by `Q_max_num_changes`, not `mFC_max_num_changes` (just updated at line 1969). If mFC changes outnumber Q changes, `Util_Copy_Change_Values` writes past the end. It has the same pointer-compare (line 1973) and row-only-copy problems as HARV-05. No callers found at the moment, so latent. Function: `Harvest_Scale_mFC()`.

**Current code** (`atharvest/atHarvest.c` lines 1979–1983)

```c
 1979: 
 1980: 	/* Allocate a new array */
 1981: 	tempArray = Util_Alloc_Init_4D_Double(3, Q_max_num_changes, bm->K_num_fisheries, bm->K_num_tot_sp, 0.0);
 1982: 
 1983: 	/* Now insert the new value in the correct place */
```

**Suggested fix (from review):** Use `mFC_max_num_changes`, and apply the HARV-05 fixes.

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

## HARV-07

**wrong error text** · Conf L / Sev L  
**Listed lines:** 1732

**Function:** `Get_Selectivity()`

**Why it's thought to be a bug**

The quit message says "value must be between 0 and 6", but there are 9 curve ids (up to `q_binormal_id`). The same stale text is at atHarvestCatch.c:759, and atHarvestCatch.c:631 says "between 0 and 1" for 3 habitat options. Function: `Get_Selectivity()`.

**Current code** (`atharvest/atHarvest.c` lines 1715–1719) — code appears to have moved -15 lines since the review

```c
 1715: 		sel = max(sel1, sel2);
 1716: 		break;
 1717:     case q_binormal_id: /* Bimodal "normal" selectivity curve based on size */
 1718:         sel_lsm = bm->FISHERYprms[nf][sel_bilsm1_id] + addlsm;
 1719:         sel_sigma = bm->FISHERYprms[nf][sel_bisigma_id] + addsigma;
```

**Suggested fix (from review):** Update the message ranges.

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

atHarvest.c `Basic_Habitat_Overlap()` / `Habitat_Overlap()` loop `stage < numStages` into `bm->HABITATlike[sp][stage][...]`, which is allocated with only 2 stages (atlantismain.c:1926). Any impacted group with `numStages > 2` would overrun. The same root cause exists in the biology reader (atBiologyXMLParamIO.c:1486), so it should be fixed at allocation (`K_num_max_stages`).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atHarvest.c:395-396 debug gate uses `which_check == not_tracking_flux`, while the two neighbouring gates use `>=`. This only affects debug output.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-3

atHarvest.c `Harvest_Skip_biology()`: the water-column `AGE_STRUCTURED_BIOMASS` branch reads `tr[k][totNTracers]` even for epifaunal groups, which are also handled in the epibenthic loop. `bm->Catch` is only reset for `isImpacted` groups but is accumulated for all groups. This is a dev/test path.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-4

atHarvest.c `Harvest_Update_Habitat_Overlap()` stores into `p_fishi[sp][stage][nf]`, which has no box dimension, so it depends on being called immediately before each box's biology (it currently is: atlantismain.c:511).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
