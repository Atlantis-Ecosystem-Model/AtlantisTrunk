# ateconomic/ateconeffort.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **12 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [QUOT-16](#quot-16) | 403 | H/M | accumulator not reset / double application | `old_effort`/`new_effort`/`redo_effort`/`max_eff_ratio` are reset once per fishery but accumulated over subfle… |  | [ ] |
| [QUOT-17](#quot-17) | 138, 274 | H/M | flag not reset | `redo_needed` is cleared at 274 and then set to 1 in the month loop (305/317). It is not cleared before the ne… |  | [ ] |
| [QUOT-18](#quot-18) | 263 | M/M | accumulator not reset | `oldtotpi` is initialised only at declaration. When `MonthlyProfitUpdate` is off it accumulates over every mon… |  | [ ] |
| [QUOT-19](#quot-19) | 282 | M/M | integer truncation | `choice_buffer = (int)(bm->SUBFLEET_ECONprms[nf][ns][choicebuffer_id]);` truncates fractional buffers (the com… |  | [ ] |
| [QUOT-20](#quot-20) | 257 | M/L | division by zero | `totppi *= (oldtotpi / (TripCost * ExpEffort));` has no small_num guard, unlike line 249. Function: `Monthly_E… |  | [ ] |
| [QUOT-21](#quot-21) | 561 | H/M | accumulator not reset | `totDIS = 0, totVPUE = 0` are initialised only at declaration. Only `totCPUE` is reset per subfleet (679), so … |  | [ ] |
| [QUOT-22](#quot-22) | 878, 861 | M/M | inconsistent accumulation | `new_today_effort` always gets the unscaled subfleet effort (824). In the cap branch `today_effort +=` the sca… |  | [ ] |
| [QUOT-23](#quot-23) ⭐ | 1050 | H/H | reset inside wrong loop | `bm->Effort_hdistrib[ij][nf][today_effort] = 0;` sits inside the per-subfleet loop, but today_effort is fisher… |  | [ ] |
| [QUOT-24](#quot-24) | 1146 | M/M | accumulator not reset | `totland[sp][regid] += catch_here` accumulates across subfleets, fisheries and every trip day, and is reset on… |  | [ ] |
| [QUOT-25](#quot-25) | 1075 | M/M | unit mismatch / repeated accumulation | `diff_catch` (kg) is compared with `catch_allowed` (mg N; see 1475 conversion). Also `leased_id += diff_catch`… |  | [ ] |
| [QUOT-26](#quot-26) | 1395, 1392 | H/M | overwritten in loop | `bm->SP_FISHERYprms[sp][nf][catch_allowed] = ...` is assigned inside `for(regid...)`, but catch_allowed is not… |  | [ ] |
| [QUOT-27](#quot-27) | 1459 | H/L | wrong variable (log) | Debug fprintf prints `ns` (stale; equals nsubfleets after the loop) instead of `nss`. Function: `Holland_Effor… |  | [ ] |

---

## QUOT-16

**accumulator not reset / double application** · Conf H / Sev M  
**Found at (current code):** `ateconeffort.c` 403
**Lines at review (Oct 2026):** 131-136, 292, 329, 390-413

**Function:** `Monthly_Effort_Schedule()`

**Why it's thought to be a bug**

`old_effort`/`new_effort`/`redo_effort`/`max_eff_ratio` are reset once per fishery but accumulated over subfleets (`+=`). The fishery-level `Effort_hdistrib[ij][nf][month]` is rescaled inside the ns loop, so subfleet 0's ratio is applied, then the cumulative (sf0+sf1) ratio is applied again, compounding. `max_eff_ratio` (including the choice_buffer floor at 283-284) also carries from one subfleet to the next. Function: `Monthly_Effort_Schedule()`.

**Code at issue** — `ateconeffort.c` line 403  
Search for: `Effort_hdistrib[ij][nf][month]`

```c
  401: 							for (ij = 0; ij < bm->nbox; ij++) {
  402: 								if (bm->boxes[ij].type != BOUNDARY) {
  403→ 									bm->Effort_hdistrib[ij][nf][month] *= (new_effort[month] / (old_effort[month] + small_num));
  404: 
  405: 									if (do_debug) {
```

**Suggested fix (from review):** Reset max_eff_ratio per subfleet; move the Effort_hdistrib rescale loop after the ns loop

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

## QUOT-17

**flag not reset** · Conf H / Sev M  
**Found at (current code):** `ateconeffort.c` 138, `ateconeffort.c` 274
**Lines at review (Oct 2026):** 138, 274, 305/317, 210

**Function:** `Monthly_Effort_Schedule()`

**Why it's thought to be a bug**

`redo_needed` is cleared at 274 and then set to 1 in the month loop (305/317). It is not cleared before the next subfleet's species loop, so for ns>=1 the deemed-value profit recalculation (210) runs even when no DV/TAX was triggered. Function: `Monthly_Effort_Schedule()`.

**Code at issue** — `ateconeffort.c` line 138 (the line the review pointed to, found by its text)  
Search for: `redo_needed = 0;`

```c
  136: 				}
  137: 
  138→ 				redo_needed = 0;
  139: 				for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
  140: 					/* If no boats in the subfleet currently skip ahead */
```

**Code at issue** — `ateconeffort.c` line 274 (the line the review pointed to, found by its text)  
Search for: `redo_needed = 0;`

```c
  272: 						}
  273: 					}
  274→ 					redo_needed = 0;
  275: 
  276: 					/* Only reduce effort if applicable to model. Unless regulated otherwise, some models
```

**Suggested fix (from review):** Set `redo_needed = 0` at the top of the ns loop

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

## QUOT-18

**accumulator not reset** · Conf M / Sev M  
**Found at (current code):** `ateconeffort.c` 263
**Lines at review (Oct 2026):** 106, 213-215, 225

**Function:** `Monthly_Effort_Schedule()`

**Why it's thought to be a bug**

`oldtotpi` is initialised only at declaration. When `MonthlyProfitUpdate` is off it accumulates over every month, subfleet and fishery in the call, so `this_profit = totppi/oldtotpi` (263) shrinks with loop position. Function: `Monthly_Effort_Schedule()`.

**Code at issue** — `ateconeffort.c` line 263 (closest match to the quoted code; wording differs slightly)  
Search for: `this_profit = totppi / (oldtotpi + small_num);`

```c
  261: 
  262: 							/* Calculate the prop of this profit compared to past profit */
  263→ 							this_profit = totppi / (oldtotpi + small_num);
  264: 
  265: 							if (do_debug) {
```

**Suggested fix (from review):** Reset oldtotpi per subfleet/month, or keep a persistent per-(nf,ns) store if annual memory is intended

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

## QUOT-19

**integer truncation** · Conf M / Sev M  
**Found at (current code):** `ateconeffort.c` 282
**Lines at review (Oct 2026):** 282

**Function:** `Monthly_Effort_Schedule()`

**Why it's thought to be a bug**

`choice_buffer = (int)(bm->SUBFLEET_ECONprms[nf][ns][choicebuffer_id]);` truncates fractional buffers (the comment describes partial reduction) to 0, so the floor is always 1 for any value below 1. Function: `Monthly_Effort_Schedule()`.

**Code at issue** — `ateconeffort.c` line 282  
Search for: `choice_buffer = (int) (bm->SUBFLEET_ECONprms[nf][ns][choicebuffer_id])`

```c
  280: 
  281: 					 */
  282→ 					choice_buffer = (int) (bm->SUBFLEET_ECONprms[nf][ns][choicebuffer_id]);
  283: 					if (max_eff_ratio < (1 - choice_buffer))
  284: 						max_eff_ratio = (1 - choice_buffer);
```

**Suggested fix (from review):** Remove the `(int)` cast

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

## QUOT-20

**division by zero** · Conf M / Sev L  
**Found at (current code):** `ateconeffort.c` 257
**Lines at review (Oct 2026):** 257

**Function:** `Monthly_Effort_Schedule()`

**Why it's thought to be a bug**

`totppi *= (oldtotpi / (TripCost * ExpEffort));` has no small_num guard, unlike line 249. Function: `Monthly_Effort_Schedule()`.

**Code at issue** — `ateconeffort.c` line 257  
Search for: `totppi *= (oldtotpi / (TripCost * ExpEffort))`

```c
  255: 								} else if (potential_totppi < 0.0) {
  256: 									/* Else just make sure contract effort appropriately */
  257→ 									totppi *= (oldtotpi / (TripCost * ExpEffort));
  258: 								} else
  259: 									totppi -= (TripCost * ExpEffort);
```

**Suggested fix (from review):** Add `+ small_num`

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

## QUOT-21

**accumulator not reset** · Conf H / Sev M  
**Found at (current code):** `ateconeffort.c` 561
**Lines at review (Oct 2026):** 561, 679-685

**Function:** `Multi_Plan_Effort_Final_Allocation()`

**Why it's thought to be a bug**

`totDIS = 0, totVPUE = 0` are initialised only at declaration. Only `totCPUE` is reset per subfleet (679), so totDIS/totVPUE grow across subfleets and fisheries and the `UseVPUE` (711) and `SpatialBycatchAvoid` (719) weights are wrong for every subfleet after the first. Function: `Multi_Plan_Effort_Final_Allocation()`.

**Code at issue** — `ateconeffort.c` line 561  
Search for: `totDIS = 0, totVPUE = 0`

```c
  559: 
  560: 	int ij, nf, ns, sp, end_run, days_left, do_debug_base, do_debug, any_quota, all_done, porti, flagfishhere;
  561→ 	double down_time, max_month_effort, cumcatch, prop_olease, prop_tlease, totCPUE, totDIS = 0, totVPUE = 0, toteffort, step1, CPUEbased, FCweight, Costscale,
  562: 			tempEffort, FCpressure, this_effort, old_realised, permisible_left;
  563: 
```

**Suggested fix (from review):** Reset `totDIS = totVPUE = 0` next to `totCPUE = 0`

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

## QUOT-22

**inconsistent accumulation** · Conf M / Sev M  
**Found at (current code):** `ateconeffort.c` 878, `ateconeffort.c` 861
**Lines at review (Oct 2026):** 824, 853-878

**Function:** `Multi_Plan_Effort_Final_Allocation()`

**Why it's thought to be a bug**

`new_today_effort` always gets the unscaled subfleet effort (824). In the cap branch `today_effort +=` the scaled effort, but the else branch sets `today_effort = new_today_effort`, which brings back earlier capped subfleets' unscaled effort. The cap scale `permisible_left/this_effort` also ignores the `*7` weekly factor used at 843, so it allows 7x the permitted effort. Function: `Multi_Plan_Effort_Final_Allocation()`.

**Code at issue** — `ateconeffort.c` line 878 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->Effort_hdistrib[ij][nf][today_effort] = bm->Effort_hdistrib[ij][nf][new_today_effort];`

```c
  876: 					for (ij = 0; ij < bm->nbox; ij++) {
  877: 						if (bm->boxes[ij].type != BOUNDARY) {
  878→ 							bm->Effort_hdistrib[ij][nf][today_effort] = bm->Effort_hdistrib[ij][nf][new_today_effort];
  879: 
  880:                             /*
```

**Code at issue** — `ateconeffort.c` line 861 (closest match to the quoted code; wording differs slightly)  
Search for: `+ small_num)) * permisible_left / (this_effort + small_num);`

```c
  859: 							bm->Effort_hdistrib[ij][nf][today_effort] += bm->SUBFLEET_ECONprms[nf][ns][FishableLength_id]
  860: 									* (bm->EffortSchedule[nf][ns][bm->MofY][expect_id] / 30.0) * (bm->Effort_hdistrib[ij][nf][calc_effort] / (toteffort
  861→ 									+ small_num)) * permisible_left / (this_effort + small_num);
  862: 
  863:                             /*
```

**Suggested fix (from review):** Accumulate per subfleet consistently (`today_effort += scaled or unscaled`); divide by `7*this_effort`

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

## QUOT-23

**reset inside wrong loop** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `ateconeffort.c` 1050
**Lines at review (Oct 2026):** 1048-1052, 1365

**Function:** `Holland_Effort_Final_Allocation()`

**Why it's thought to be a bug**

`bm->Effort_hdistrib[ij][nf][today_effort] = 0;` sits inside the per-subfleet loop, but today_effort is fishery-level and accumulated with `+=` at 1365. Each subfleet wipes the previous subfleets' effort, so only the last active subfleet's effort is applied. Function: `Holland_Effort_Final_Allocation()`.

**Code at issue** — `ateconeffort.c` line 1050  
Search for: `Effort_hdistrib[ij][nf][today_effort] = 0`

```c
 1048:                     for (ij = 0; ij < bm->nbox; ij++) {
 1049:                         /* More initialisation */
 1050→                         bm->Effort_hdistrib[ij][nf][today_effort] = 0;
 1051:                         bm->Effort_hdistrib[ij][nf][calc_effort] = 0;
 1052:                         bm->Effort_hdistrib[ij][nf][new_today_effort] = 0;
```

**Suggested fix (from review):** Zero today_effort once per fishery, before the ns loop (as at 974-978)

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

## QUOT-24

**accumulator not reset** · Conf M / Sev M  
**Found at (current code):** `ateconeffort.c` 1146
**Lines at review (Oct 2026):** 1033-1035, 1146

**Function:** `Holland_Effort_Final_Allocation()`

**Why it's thought to be a bug**

`totland[sp][regid] += catch_here` accumulates across subfleets, fisheries and every trip day, and is reset only when BiM changes. It is then used as one subfleet's expected landings (1170/1186/1200) and as the totland_overall normaliser, so it inflates over the bimonth. Function: `Holland_Effort_Final_Allocation()`.

**Code at issue** — `ateconeffort.c` line 1146  
Search for: `totland[sp][regid] += catch_here`

```c
 1144:                                     catch_here = spbiom * q * prop_tow_time[nf][ns][ij];
 1145:                                     bm->SpatialBlackBook[nf][ns][bm->MofY][ij][expect_id] += catch_here;
 1146→                                     totland[sp][regid] += catch_here;
 1147:                                     
 1148:                                     //fprintf(llogfp,"Time: %e, %s-%d mth: %d box: %d SpatialBlackBook: %e, catch_here: %e, spbiom: %e, q: %e, prop_tow_time: %e\n",
```

**Suggested fix (from review):** Reset totland per subfleet trip calculation

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

## QUOT-25

**unit mismatch / repeated accumulation** · Conf M / Sev M  
**Found at (current code):** `ateconeffort.c` 1075
**Lines at review (Oct 2026):** 1070-1076

**Function:** `Holland_Effort_Final_Allocation()`

**Why it's thought to be a bug**

`diff_catch` (kg) is compared with `catch_allowed` (mg N; see 1475 conversion). Also `leased_id += diff_catch` re-adds the whole overage on every trip-day call, so leased quota grows without bound. Function: `Holland_Effort_Final_Allocation()`.

**Code at issue** — `ateconeffort.c` line 1075 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->QuotaAlloc[nf][ns][sp][leased_id] += diff_catch;`

```c
 1073:                                                 bm->QuotaAlloc[nf][ns][sp][leased_id] += (bm->SP_FISHERYprms[sp][nf][catch_allowed] * bm->X_CN * mg_2_kg);
 1074:                                             else
 1075→                                                 bm->QuotaAlloc[nf][ns][sp][leased_id] += diff_catch;
 1076:                                         }
 1077:                                         
```

**Suggested fix (from review):** Compare with `catch_allowed*X_CN*mg_2_kg`; set leased to `max(leased, diff_catch)` rather than adding

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

## QUOT-26

**overwritten in loop** · Conf H / Sev M  
**Found at (current code):** `ateconeffort.c` 1395, `ateconeffort.c` 1392
**Lines at review (Oct 2026):** 1392-1395, 1440-1478

**Function:** `Holland_Effort_Final_Allocation()`

**Why it's thought to be a bug**

`bm->SP_FISHERYprms[sp][nf][catch_allowed] = ...` is assigned inside `for(regid...)`, but catch_allowed is not region-indexed, so only the last region's value survives (wrong whenever K_num_active_reg>1). Function: `Holland_Effort_Final_Allocation()`.

**Code at issue** — `ateconeffort.c` line 1395  
Search for: `SP_FISHERYprms[sp][nf][catch_allowed] =`

```c
 1393:                                     totallowedcatch
 1394:                                     = max(0.0,((bm->BiTAC_sp[bm->BiM][regid][sp][now_id] * kg_2_mg / bm->X_CN) - (bm->TotCumBiCatch[sp][nf] * bm->RegionalData[sp][regid][reg_catch_id])));
 1395→                                     bm->SP_FISHERYprms[sp][nf][catch_allowed] = totallowedcatch;
 1396:                                     
 1397:                                     /* Check for individual trip limits */
```

**Code at issue** — `ateconeffort.c` line 1392  
Search for: `for (regid`

```c
 1390:                         for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 1391:                             if (FunctGroupArray[sp].isFished == TRUE) {
 1392→                                 for (regid = 0; regid < bm->K_num_active_reg; regid++) {
 1393:                                     totallowedcatch
 1394:                                     = max(0.0,((bm->BiTAC_sp[bm->BiM][regid][sp][now_id] * kg_2_mg / bm->X_CN) - (bm->TotCumBiCatch[sp][nf] * bm->RegionalData[sp][regid][reg_catch_id])));
```

**Suggested fix (from review):** Sum over regions, or store per region

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

## QUOT-27

**wrong variable (log)** · Conf H / Sev L  
**Found at (current code):** `ateconeffort.c` 1459
**Lines at review (Oct 2026):** 1458

**Function:** `Holland_Effort_Final_Allocation()`

**Why it's thought to be a bug**

Debug fprintf prints `ns` (stale; equals nsubfleets after the loop) instead of `nss`. Function: `Holland_Effort_Final_Allocation()`.

**Code at issue** — `ateconeffort.c` line 1459 (located by name only; check this is the right place)  
Search for: `regcatch_nf, bm->SpatialBlackBook[nf][nss][bm->MofY][ij][hist_id],`

```c
 1457:                                                             "Time: %e, reg: %d (box: %d), %s-%d on %s in month %d regcatch: %e, SpatialBB: %e, BB: %e)\n",
 1458:                                                             bm->dayt, regid, ij, FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, bm->MofY,
 1459→                                                             regcatch_nf, bm->SpatialBlackBook[nf][nss][bm->MofY][ij][hist_id],
 1460:                                                             bm->BlackBook[nf][nss][sp][bm->MofY][hist_id]);
 1461:                                                 }
```

**Suggested fix (from review):** Use `nss`

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
