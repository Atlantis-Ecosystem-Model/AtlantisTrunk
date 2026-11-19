# ateconomic/ateconeffort.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **12 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [QUOT-16](#quot-16) | 131-136, 292, 329, 390-413 | H/M | accumulator not reset / double application | `old_effort`/`new_effort`/`redo_effort`/`max_eff_ratio` are reset once per fishery but accumulated over subfle… |  | [ ] |
| [QUOT-17](#quot-17) | 138, 274, 305/317, 210 | H/M | flag not reset | `redo_needed` is cleared at 274 and then set to 1 in the month loop (305/317). It is not cleared before the ne… |  | [ ] |
| [QUOT-18](#quot-18) | 106, 213-215, 225 | M/M | accumulator not reset | `oldtotpi` is initialised only at declaration. When `MonthlyProfitUpdate` is off it accumulates over every mon… |  | [ ] |
| [QUOT-19](#quot-19) | 282 | M/M | integer truncation | `choice_buffer = (int)(bm->SUBFLEET_ECONprms[nf][ns][choicebuffer_id]);` truncates fractional buffers (the com… |  | [ ] |
| [QUOT-20](#quot-20) | 257 | M/L | division by zero | `totppi *= (oldtotpi / (TripCost * ExpEffort));` has no small_num guard, unlike line 249. Function: `Monthly_E… |  | [ ] |
| [QUOT-21](#quot-21) | 561, 679-685 | H/M | accumulator not reset | `totDIS = 0, totVPUE = 0` are initialised only at declaration. Only `totCPUE` is reset per subfleet (679), so … |  | [ ] |
| [QUOT-22](#quot-22) | 824, 853-878 | M/M | inconsistent accumulation | `new_today_effort` always gets the unscaled subfleet effort (824). In the cap branch `today_effort +=` the sca… |  | [ ] |
| [QUOT-23](#quot-23) ⭐ | 1048-1052, 1365 | H/H | reset inside wrong loop | `bm->Effort_hdistrib[ij][nf][today_effort] = 0;` sits inside the per-subfleet loop, but today_effort is fisher… |  | [ ] |
| [QUOT-24](#quot-24) | 1033-1035, 1146 | M/M | accumulator not reset | `totland[sp][regid] += catch_here` accumulates across subfleets, fisheries and every trip day, and is reset on… |  | [ ] |
| [QUOT-25](#quot-25) | 1070-1076 | M/M | unit mismatch / repeated accumulation | `diff_catch` (kg) is compared with `catch_allowed` (mg N; see 1475 conversion). Also `leased_id += diff_catch`… |  | [ ] |
| [QUOT-26](#quot-26) | 1392-1395, 1440-1478 | H/M | overwritten in loop | `bm->SP_FISHERYprms[sp][nf][catch_allowed] = ...` is assigned inside `for(regid...)`, but catch_allowed is not… |  | [ ] |
| [QUOT-27](#quot-27) | 1458 | H/L | wrong variable (log) | Debug fprintf prints `ns` (stale; equals nsubfleets after the loop) instead of `nss`. Function: `Holland_Effor… |  | [ ] |

---

## QUOT-16

**accumulator not reset / double application** · Conf H / Sev M  
**Listed lines:** 131-136, 292, 329, 390-413

**Function:** `Monthly_Effort_Schedule()`

**Why it's thought to be a bug**

`old_effort`/`new_effort`/`redo_effort`/`max_eff_ratio` are reset once per fishery but accumulated over subfleets (`+=`). The fishery-level `Effort_hdistrib[ij][nf][month]` is rescaled inside the ns loop, so subfleet 0's ratio is applied, then the cumulative (sf0+sf1) ratio is applied again, compounding. `max_eff_ratio` (including the choice_buffer floor at 283-284) also carries from one subfleet to the next. Function: `Monthly_Effort_Schedule()`.

**Current code** (`ateconomic/ateconeffort.c` lines 129–138)

```c
  129: 					do_debug = 0;
  130: 
  131: 				max_eff_ratio = 0;
  132: 				for (month = bm->MofY; month < 12; month++) {
  133: 					old_effort[month] = 0;
  134: 					new_effort[month] = 0;
  135: 					redo_effort[month] = 0;
  136: 				}
  137: 
  138: 				redo_needed = 0;
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
**Listed lines:** 138, 274, 305/317, 210

**Function:** `Monthly_Effort_Schedule()`

**Why it's thought to be a bug**

`redo_needed` is cleared at 274 and then set to 1 in the month loop (305/317). It is not cleared before the next subfleet's species loop, so for ns>=1 the deemed-value profit recalculation (210) runs even when no DV/TAX was triggered. Function: `Monthly_Effort_Schedule()`.

**Current code** (`ateconomic/ateconeffort.c` lines 136–140)

```c
  136: 				}
  137: 
  138: 				redo_needed = 0;
  139: 				for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
  140: 					/* If no boats in the subfleet currently skip ahead */
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
**Listed lines:** 106, 213-215, 225

**Function:** `Monthly_Effort_Schedule()`

**Why it's thought to be a bug**

`oldtotpi` is initialised only at declaration. When `MonthlyProfitUpdate` is off it accumulates over every month, subfleet and fishery in the call, so `this_profit = totppi/oldtotpi` (263) shrinks with loop position. Function: `Monthly_Effort_Schedule()`.

**Current code** (`ateconomic/ateconeffort.c` lines 104–108)

```c
  104: 	int nf, ns, month, sp, ij, nt, do_debug_base, do_debug, boats_tied_up, boats_semi_active, redo_needed = 0, ntarg;
  105: 	double max_eff_ratio, max_month_effort, cumcatch, tolerable_debt = 0, expectedcatch, eff_ratio, effort_adjust, down_time, choice_buffer,
  106: 			effort_correction, check_time, step1 = 0, ans, ExpEffort, oldtotpi = 0, this_profit, totppi, TripCost, potential_totppi = 0, stepE;
  107: 
  108: 	/* If no monthly effort plan skip ahead */
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
**Listed lines:** 282

**Function:** `Monthly_Effort_Schedule()`

**Why it's thought to be a bug**

`choice_buffer = (int)(bm->SUBFLEET_ECONprms[nf][ns][choicebuffer_id]);` truncates fractional buffers (the comment describes partial reduction) to 0, so the floor is always 1 for any value below 1. Function: `Monthly_Effort_Schedule()`.

**Current code** (`ateconomic/ateconeffort.c` lines 280–284)

```c
  280: 
  281: 					 */
  282: 					choice_buffer = (int) (bm->SUBFLEET_ECONprms[nf][ns][choicebuffer_id]);
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
**Listed lines:** 257

**Function:** `Monthly_Effort_Schedule()`

**Why it's thought to be a bug**

`totppi *= (oldtotpi / (TripCost * ExpEffort));` has no small_num guard, unlike line 249. Function: `Monthly_Effort_Schedule()`.

**Current code** (`ateconomic/ateconeffort.c` lines 255–259)

```c
  255: 								} else if (potential_totppi < 0.0) {
  256: 									/* Else just make sure contract effort appropriately */
  257: 									totppi *= (oldtotpi / (TripCost * ExpEffort));
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
**Listed lines:** 561, 679-685

**Function:** `Multi_Plan_Effort_Final_Allocation()`

**Why it's thought to be a bug**

`totDIS = 0, totVPUE = 0` are initialised only at declaration. Only `totCPUE` is reset per subfleet (679), so totDIS/totVPUE grow across subfleets and fisheries and the `UseVPUE` (711) and `SpatialBycatchAvoid` (719) weights are wrong for every subfleet after the first. Function: `Multi_Plan_Effort_Final_Allocation()`.

**Current code** (`ateconomic/ateconeffort.c` lines 559–563)

```c
  559: 
  560: 	int ij, nf, ns, sp, end_run, days_left, do_debug_base, do_debug, any_quota, all_done, porti, flagfishhere;
  561: 	double down_time, max_month_effort, cumcatch, prop_olease, prop_tlease, totCPUE, totDIS = 0, totVPUE = 0, toteffort, step1, CPUEbased, FCweight, Costscale,
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
**Listed lines:** 824, 853-878

**Function:** `Multi_Plan_Effort_Final_Allocation()`

**Why it's thought to be a bug**

`new_today_effort` always gets the unscaled subfleet effort (824). In the cap branch `today_effort +=` the scaled effort, but the else branch sets `today_effort = new_today_effort`, which brings back earlier capped subfleets' unscaled effort. The cap scale `permisible_left/this_effort` also ignores the `*7` weekly factor used at 843, so it allows 7x the permitted effort. Function: `Multi_Plan_Effort_Final_Allocation()`.

**Current code** (`ateconomic/ateconeffort.c` lines 822–861 (truncated))

```c
  822: 									* (bm->Effort_hdistrib[ij][nf][calc_effort] / (toteffort + small_num));
  823: 							/* Effort_hdistrib - value per fishery not a subfleet value */
  824: 							bm->Effort_hdistrib[ij][nf][new_today_effort] += FCpressure;
  825: 							this_effort += FCpressure;
  826: 
  827:                             /*
  828: 							if (do_debug) {
  829: 								fprintf(
  830: 										llogfp,
  831: 										"Time: %e %s-%d Effort_hdistrib-box%d: %e, level: %e, fishablelength: %e, Effort_hdistrib: %e (effort per boat per day: %e - nboat: %e), this_effort: %e\n",
  832: 										bm->dayt, FisheryArray[nf].fisheryCode, ns, ij, FCpressure, (bm->EffortSchedule[nf][ns][bm->MofY][expect_id] / 30.0),
  833: 										bm->SUBFLEET_ECONprms[nf][ns][FishableLength_id], (bm->Effort_hdistrib[ij][nf][calc_effort] / (toteffort + small_num)),
  834: 										((bm->EffortSchedule[nf][ns][bm->MofY][expect_id] / 30.0) / bm->SUBFLEET_ECONprms[nf][ns][nboat_id]),
  835: 										bm->SUBFLEET_ECONprms[nf][ns][nboat_id], this_effort);
  836: 							}
  837:                              */
  838: 						}
  839: 					}
  840: 				}
  841: 				/* Store realised effort - *7 as updated once per week */
  842: 				old_realised = bm->EffortSchedule[nf][ns][bm->MofY][current_id];
  843: 				bm->EffortSchedule[nf][ns][bm->MofY][current_id] += (this_effort * 7);
  844: 
  845: 				/* Check whether have exceeded maximum total effort */
  846: 				if (do_debug) {
  847: 					fprintf(llogfp,
  848: 							"Time: %e %s-%d mth: %d check EffortSchedule: %e (e: %e), max_month_effort: %e, this_effort: %e, toteffort: %e old_realised: %e\n",
  849: 							bm->dayt, FisheryArray[nf].fisheryCode, ns, bm->MofY, bm->EffortSchedule[nf][ns][bm->MofY][current_id],
  850: 							bm->EffortSchedule[nf][ns][bm->MofY][expect_id], max_month_effort, this_effort, toteffort, old_realised);
  851: 				}
  852: 
  853: 				if ((!bm->hist_only) && (bm->EffortSchedule[nf][ns][bm->MofY][current_id] > max_month_effort)) {
  854: 					permisible_left = max_month_effort - old_realised;
  855: 					if (permisible_left < 0)
  856: 						permisible_left = 0;
  857: 					for (ij = 0; ij < bm->nbox; ij++) {
  858: 						if (bm->boxes[ij].type != BOUNDARY) {
  859: 							bm->Effort_hdistrib[ij][nf][today_effort] += bm->SUBFLEET_ECONprms[nf][ns][FishableLength_id]
  860: 									* (bm->EffortSchedule[nf][ns][bm->MofY][expect_id] / 30.0) * (bm->Effort_hdistrib[ij][nf][calc_effort] / (toteffort
  861: 									+ small_num)) * permisible_left / (this_effort + small_num);
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
**Listed lines:** 1048-1052, 1365

**Function:** `Holland_Effort_Final_Allocation()`

**Why it's thought to be a bug**

`bm->Effort_hdistrib[ij][nf][today_effort] = 0;` sits inside the per-subfleet loop, but today_effort is fishery-level and accumulated with `+=` at 1365. Each subfleet wipes the previous subfleets' effort, so only the last active subfleet's effort is applied. Function: `Holland_Effort_Final_Allocation()`.

**Current code** (`ateconomic/ateconeffort.c` lines 1046–1054)

```c
 1046:                     }
 1047:                     
 1048:                     for (ij = 0; ij < bm->nbox; ij++) {
 1049:                         /* More initialisation */
 1050:                         bm->Effort_hdistrib[ij][nf][today_effort] = 0;
 1051:                         bm->Effort_hdistrib[ij][nf][calc_effort] = 0;
 1052:                         bm->Effort_hdistrib[ij][nf][new_today_effort] = 0;
 1053:                         bm->SpatialBlackBook[nf][ns][bm->MofY][ij][expect_id] = 0;
 1054:                         if (bm->boxes[ij].type != BOUNDARY) {
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
**Listed lines:** 1033-1035, 1146

**Function:** `Holland_Effort_Final_Allocation()`

**Why it's thought to be a bug**

`totland[sp][regid] += catch_here` accumulates across subfleets, fisheries and every trip day, and is reset only when BiM changes. It is then used as one subfleet's expected landings (1170/1186/1200) and as the totland_overall normaliser, so it inflates over the bimonth. Function: `Holland_Effort_Final_Allocation()`.

**Current code** (`ateconomic/ateconeffort.c` lines 1031–1037)

```c
 1031:                                     bm->ECONexprice[nf][ns][sp][ij][expect_id] = bm->ECONexprice[nf][ns][sp][ij][hist_id];
 1032:                                 }
 1033:                                 for (ij = 0; ij < bm->K_num_active_reg; ij++) {
 1034:                                     totland[sp][ij] = 0;
 1035:                                 }
 1036:                             }
 1037:                         }
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
**Listed lines:** 1070-1076

**Function:** `Holland_Effort_Final_Allocation()`

**Why it's thought to be a bug**

`diff_catch` (kg) is compared with `catch_allowed` (mg N; see 1475 conversion). Also `leased_id += diff_catch` re-adds the whole overage on every trip-day call, so leased quota grows without bound. Function: `Holland_Effort_Final_Allocation()`.

**Current code** (`ateconomic/ateconeffort.c` lines 1068–1078)

```c
 1068:                                         
 1069:                                         /* Update how much quota has been leased */
 1070:                                         if (cumcatch > bm->QuotaAlloc[nf][ns][sp][owned_id]) {
 1071:                                             diff_catch = cumcatch - bm->QuotaAlloc[nf][ns][sp][owned_id];
 1072:                                             if (diff_catch > bm->SP_FISHERYprms[sp][nf][catch_allowed])
 1073:                                                 bm->QuotaAlloc[nf][ns][sp][leased_id] += (bm->SP_FISHERYprms[sp][nf][catch_allowed] * bm->X_CN * mg_2_kg);
 1074:                                             else
 1075:                                                 bm->QuotaAlloc[nf][ns][sp][leased_id] += diff_catch;
 1076:                                         }
 1077:                                         
 1078:                                         /* Store amount of total leased quota that was leased in the last round of calculations - since calculating lease prices */
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
**Listed lines:** 1392-1395, 1440-1478

**Function:** `Holland_Effort_Final_Allocation()`

**Why it's thought to be a bug**

`bm->SP_FISHERYprms[sp][nf][catch_allowed] = ...` is assigned inside `for(regid...)`, but catch_allowed is not region-indexed, so only the last region's value survives (wrong whenever K_num_active_reg>1). Function: `Holland_Effort_Final_Allocation()`.

**Current code** (`ateconomic/ateconeffort.c` lines 1390–1397)

```c
 1390:                         for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 1391:                             if (FunctGroupArray[sp].isFished == TRUE) {
 1392:                                 for (regid = 0; regid < bm->K_num_active_reg; regid++) {
 1393:                                     totallowedcatch
 1394:                                     = max(0.0,((bm->BiTAC_sp[bm->BiM][regid][sp][now_id] * kg_2_mg / bm->X_CN) - (bm->TotCumBiCatch[sp][nf] * bm->RegionalData[sp][regid][reg_catch_id])));
 1395:                                     bm->SP_FISHERYprms[sp][nf][catch_allowed] = totallowedcatch;
 1396:                                     
 1397:                                     /* Check for individual trip limits */
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
**Listed lines:** 1458

**Function:** `Holland_Effort_Final_Allocation()`

**Why it's thought to be a bug**

Debug fprintf prints `ns` (stale; equals nsubfleets after the loop) instead of `nss`. Function: `Holland_Effort_Final_Allocation()`.

**Current code** (`ateconomic/ateconeffort.c` lines 897–901) — code appears to have moved -559 lines since the review

```c
  897:  * \brief Dan Holland effort model - originally developed for the northern california current
  898:  */
  899: void Holland_Effort_Final_Allocation(MSEBoxModel *bm, FILE *llogfp) {
  900: 	int nf, ij, ns, sp, do_debug_base, do_debug = 0, regid, trip_length, do_biTAC, mth, nss, use_history, need_to_init, do_consoldation = 0;
  901: 	double cumcatch, toteffort, FCpressure, spbiom, q, tolerable_debt, TotCC, use_minprob, minprob, maxprob, totallowedcatch, totscore, max_hold, regTAC_scale, totTAC, catchleft, totcatch, regcatch_nf, regcatch, diff_catch, max_quota_left, totmaxland, totland_overall, maxland, catch_here, hold_size;
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
