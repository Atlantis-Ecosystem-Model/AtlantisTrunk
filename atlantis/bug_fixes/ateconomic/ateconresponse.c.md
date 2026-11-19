# ateconomic/ateconresponse.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **11 findings** (severity H: 2; confidence H and severity H: 2; on the priority list: 2), plus 3 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [ECON-07](#econ-07) ⭐ | 488, 609-619 | H/H | accumulator never reset | `boats_new[nf][ns] += num_new;` is never zeroed: grep finds no reset anywhere, and the initialise block at 222… |  | [ ] |
| [ECON-08](#econ-08) | 530-549 | H/M | accumulator / stale variable | In the `flagsupp_allowed > 1` branch, `boats_switching` is overwritten for each destination subfleet and each … |  | [ ] |
| [ECON-09](#econ-09) | 530, 532 | M/M | stale variable (wrong subfleet) | `supp_cost` and `maxp_flexgear` used here are left over from the last subfleet handled in the earlier leaving … |  | [ ] |
| [ECON-10](#econ-10) | 370, 379, 445-473 | M/M | logic | `boats_free[nf][ns] += num_moving` is recorded before `num_moving` is reduced by the fleet_thresh, boat-bound … |  | [ ] |
| [ECON-11](#econ-11) | 801-805 | H/M | always-false logic | Inside `if ((owned_quota + tradeable_quota[sp]) > totTACnew)`, `tradeable_quota[sp] = totTACnew - (owned_quota… |  | [ ] |
| [ECON-12](#econ-12) | 963 | H/M | wrong index | `nreg = bm->regID[ij];` uses the layer index `ij`, but `bm->regID` is per box ("ID for each box", atlantisboxm… |  | [ ] |
| [ECON-13](#econ-13) | 919, 983, 1050-1053 | H/M | accumulator not reset | `totval` is zeroed once, before the fishery loop, then accumulated over all fisheries, subfleets and boxes. `n… |  | [ ] |
| [ECON-14](#econ-14) | 935, 995, 1046 | H/M | accumulator not reset | `totdiscard` is reset per subfleet but accumulated across boxes, yet `newDISpue = totdiscard / effort[b]` is c… |  | [ ] |
| [ECON-15](#econ-15) | 1078 | H/M | wrong id constant / slot collision | `bm->SUBFLEET_ECONprms[nf][ns][within_id] = 1`: `within_id` (=15) is a QuotaAlloc (per-species) index. In the … |  | [ ] |
| [ECON-16](#econ-16) | 1142 | M/M | division by zero | `cpueval += BlackBook[..][expect_id] / bm->EffortSchedule[nf][ns][bm->lastMofY][hist_id];` has no `small_num`,… |  | [ ] |
| [ECON-17](#econ-17) ⭐ | 1259 | H/H | unit conversion wrong direction | With deemed values on, "unlimited" allowed catch is set as `totallowedcatch = no_quota * bm->X_CN * mg_2_kg`. … |  | [ ] |

---

## ECON-07

**accumulator never reset** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 488, 609-619  
**Lines at review (Oct 2026):** 488, 609-619

**Function:** `Update_Vessel_Numbers()`

**Why it's thought to be a bug**

`boats_new[nf][ns] += num_new;` is never zeroed: grep finds no reset anywhere, and the initialise block at 222-238 resets only `boats_free`. Every monthly call re-adds the whole accumulated `boats_new` to `nboat_id`/`nvessel_id`/`newboat_id` (616-619), so new entrants are counted again every month. Function: `Update_Vessel_Numbers()`.

**Current code** (`ateconomic/ateconresponse.c` lines 486–490)

```c
  486: 					rndnum2 = drandom(0.0, 1.0) * (bm->FISHERYprms[nf][nlicence_id] - bm->FISHERYprms[nf][nvessel_id]);
  487: 					num_new = (int) (min(1.0, floor(rndnum2)));
  488: 					boats_new[nf][ns] += num_new;
  489: 				}
  490: 				if (boats_new[nf][ns] < 0)
```

**Suggested fix (from review):** Zero `boats_new[nf][ns]` in the initialise loop, or after it is applied at 619

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

## ECON-08

**accumulator / stale variable** · Conf H / Sev M  
**Lines (current code):** 530-549  
**Lines at review (Oct 2026):** 530-549

**Function:** `Update_Vessel_Numbers()`

**Why it's thought to be a bug**

In the `flagsupp_allowed > 1` branch, `boats_switching` is overwritten for each destination subfleet and each value is added to that destination's `nboat_id`. Only the last value is then subtracted from `boats_free` (546), and the inflated remainder is added back to the origin (549), so boats are created. `boats_switching` is also never reset per (nf,ns), so a stale value is subtracted when no destination qualifies. Function: `Update_Vessel_Numbers()`.

**Current code** (`ateconomic/ateconresponse.c` lines 528–551)

```c
  528: 								/* Only allowed to swap with other dynamic fisheries */
  529: 								if (bm->FISHERYprms[fishery_id][flageffortmodel_id] > rec_econ_model) {
  530: 									rent_ratio = tot_marg_rent[fishery_id][subf_id] / (tot_marg_rent[nf][ns] - supp_cost + small_num);
  531: 									if (rent_ratio > 1.0) {
  532: 										p_flexgear = ((1.0 - 1.0 / (rent_ratio + small_num)) / (maxp_flexgear + small_num));
  533: 										boats_switching = floor(p_flexgear * boats_free[nf][ns] + 0.5); // 0.5 so rounding to nearest
  534: 										bm->SUBFLEET_ECONprms[fishery_id][subf_id][nboat_id] += boats_switching;
  535: 										bm->FISHERYprms[fishery_id][nvessel_id] += boats_switching;
  536: 										bm->SUBFLEET_ECONprms[fishery_id][subf_id][switchboat_id] += boats_switching;
  537: 									}
  538: 								}
  539: 							}
  540: 						}
  541: 					}
  542: 
  543: 					/* If not enough better offers than stay where they are for now */
  544: 					fprintf(llogfp, "Time: %e, %s-%d had %d boats wishing to switch, but only %e found a location so stay where they are\n", bm->dayt,
  545: 							FisheryArray[nf].fisheryCode, ns, boats_free[nf][ns], boats_switching);
  546: 					boats_free[nf][ns] -= (int) (boats_switching);
  547: 					if (boats_free[nf][ns] < 0)
  548: 						boats_free[nf][ns] = 0;
  549: 					bm->SUBFLEET_ECONprms[nf][ns][nboat_id] += boats_free[nf][ns];
  550: 				} else {
  551: 					this_fishery = -1;
```

**Suggested fix (from review):** Accumulate into a per-origin total (reset to 0 for each ns) and subtract that total

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

## ECON-09

**stale variable (wrong subfleet)** · Conf M / Sev M  
**Lines (current code):** 530, 532  
**Lines at review (Oct 2026):** 530, 532

**Function:** `Update_Vessel_Numbers()`

**Why it's thought to be a bug**

`supp_cost` and `maxp_flexgear` used here are left over from the last subfleet handled in the earlier leaving loop (326/356), possibly a different fishery, not origin (nf,ns). Function: `Update_Vessel_Numbers()`.

**Current code** (`ateconomic/ateconresponse.c` lines 528–534)

```c
  528: 								/* Only allowed to swap with other dynamic fisheries */
  529: 								if (bm->FISHERYprms[fishery_id][flageffortmodel_id] > rec_econ_model) {
  530: 									rent_ratio = tot_marg_rent[fishery_id][subf_id] / (tot_marg_rent[nf][ns] - supp_cost + small_num);
  531: 									if (rent_ratio > 1.0) {
  532: 										p_flexgear = ((1.0 - 1.0 / (rent_ratio + small_num)) / (maxp_flexgear + small_num));
  533: 										boats_switching = floor(p_flexgear * boats_free[nf][ns] + 0.5); // 0.5 so rounding to nearest
  534: 										bm->SUBFLEET_ECONprms[fishery_id][subf_id][nboat_id] += boats_switching;
```

**Suggested fix (from review):** Recompute `supp_cost = SUBFLEET_ECONprms[nf][ns][supp_cost_id]` and store maxp_flexgear per subfleet

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

## ECON-10

**logic** · Conf M / Sev M  
**Lines (current code):** 370, 379, 445-473  
**Lines at review (Oct 2026):** 370, 379, 445-473

**Function:** `Update_Vessel_Numbers()`

**Why it's thought to be a bug**

`boats_free[nf][ns] += num_moving` is recorded before `num_moving` is reduced by the fleet_thresh, boat-bound and `hist_only` checks (445-470). Those boats are still moved to other subfleets (534/574) but are not removed from the origin, which creates boats (e.g. with `hist_only` set, num_moving goes to 0 but boats_free stays at 1). Function: `Update_Vessel_Numbers()`.

**Current code** (`ateconomic/ateconresponse.c` lines 368–381)

```c
  368: 					rndnum2 = drandom(0.0, bm->prop_supp) * bm->SUBFLEET_ECONprms[nf][ns][nboat_id];
  369: 					num_moving = (int) (min(1.0, floor(rndnum2)));  // Curently capped at 1 boat a month moving.... could set ghis to max to allow more
  370: 					boats_free[nf][ns] += num_moving;
  371: 
  372: 					fprintf(llogfp, "Time: %e %d boats supplementing from %s-%d (num_moving; %d, boats_free: %d)\n", bm->dayt, num_moving,
  373: 							FisheryArray[nf].fisheryCode, ns, num_moving, boats_free[nf][ns]);
  374: 
  375: 				} else if (rndnum < p_switch) {
  376: 					/* Simply switching fleets */
  377: 					rndnum2 = drandom(0.0, bm->prop_switch) * bm->SUBFLEET_ECONprms[nf][ns][nboat_id];
  378: 					num_moving = (int) (min(1.0, floor(rndnum2)));
  379: 					boats_free[nf][ns] += num_moving;
  380: 
  381: 					fprintf(llogfp, "Time: %e %d boats switching from %s-%d (num_moving; %d, boats_free: %d)\n", bm->dayt, num_moving,
```

**Suggested fix (from review):** Cap `boats_free[nf][ns]` at the final num_moving after line 470

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

## ECON-11

**always-false logic** · Conf H / Sev M  
**Lines (current code):** 801-805  
**Lines at review (Oct 2026):** 801-805

**Function:** `Annual_Quota_Allocation()`

**Why it's thought to be a bug**

Inside `if ((owned_quota + tradeable_quota[sp]) > totTACnew)`, `tradeable_quota[sp] = totTACnew - (owned_quota + tradeable_quota[sp])` is always negative and so always reset to 0. The branch can only zero tradeable quota. Function: `Annual_Quota_Allocation()`.

**Current code** (`ateconomic/ateconresponse.c` lines 799–807)

```c
  799: 			if (!bm->stochastic_effort) {
  800: 				if (totTACnew < totTACold) {
  801: 					if ((owned_quota + tradeable_quota[sp]) > totTACnew) {
  802: 						tradeable_quota[sp] = totTACnew - (owned_quota + tradeable_quota[sp]);
  803: 						if (tradeable_quota[sp] < 0)
  804: 							tradeable_quota[sp] = 0;
  805: 					}
  806: 				}
  807: 				if (bm->dayt < 1)
```

**Suggested fix (from review):** Probably `tradeable_quota[sp] = totTACnew - owned_quota` (then clamp at 0)

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

## ECON-12

**wrong index** · Conf H / Sev M  
**Lines (current code):** 963  
**Lines at review (Oct 2026):** 963

**Function:** `Allocate_Catch()`

**Why it's thought to be a bug**

`nreg = bm->regID[ij];` uses the layer index `ij`, but `bm->regID` is per box ("ID for each box", atlantisboxmodel.h:5489). RegCatch then books catch to the wrong region when there is more than one region. Function: `Allocate_Catch()`.

**Current code** (`ateconomic/ateconresponse.c` lines 961–965)

```c
  961: 
  962: 								for (ij = 0; ij < bm->boxes[b].nz; ij++) {
  963: 									nreg = bm->regID[ij];
  964: 									if ((bm->Catch[b][sp][nf][ij]) || ((bm->flagTACincludeDiscard) && (!ij) && (bm->Discards[b][sp][nf]))) {
  965: 										/*oldspcatch = totthisspcatch;*/
```

**Suggested fix (from review):** `nreg = bm->regID[b];`

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

## ECON-13

**accumulator not reset** · Conf H / Sev M  
**Lines (current code):** 919, 983, 1050-1053  
**Lines at review (Oct 2026):** 919, 983, 1050-1053

**Function:** `Allocate_Catch()`

**Why it's thought to be a bug**

`totval` is zeroed once, before the fishery loop, then accumulated over all fisheries, subfleets and boxes. `newVpue = totval / effort[b]` therefore uses a running total, not this box's or subfleet's value, so SpatialVPUE (used for deemed-value behaviour) inflates. Function: `Allocate_Catch()`.

**Current code** (`ateconomic/ateconresponse.c` lines 917–921)

```c
  917: 	}
  918: 
  919: 	totval = 0;
  920: 	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  921: 		if (bm->FISHERYprms[nf][flageffortmodel_id] > rec_econ_model) {
```

**Suggested fix (from review):** Reset `totval = 0` per box (next to `totcatch = 0` at 938)

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

## ECON-14

**accumulator not reset** · Conf H / Sev M  
**Lines (current code):** 935, 995, 1046  
**Lines at review (Oct 2026):** 935, 995, 1046

**Function:** `Allocate_Catch()`

**Why it's thought to be a bug**

`totdiscard` is reset per subfleet but accumulated across boxes, yet `newDISpue = totdiscard / effort[b]` is computed per box, so later boxes get the cumulative discards of earlier boxes. Unlike catch, it is also not scaled by the subfleet's `SpatialBlackBook` share. Function: `Allocate_Catch()`.

**Current code** (`ateconomic/ateconresponse.c` lines 933–937)

```c
  933: 
  934: 				tottotcatch = 0;
  935: 				totdiscard = 0;
  936: 				/* Allocate catch from fleet amongst subfleets */
  937: 				for (b = 0; b < bm->nbox; b++) {
```

**Suggested fix (from review):** Use a per-box discard accumulator (reset at 938) for DisPUE, scaled by the subfleet share

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

## ECON-15

**wrong id constant / slot collision** · Conf H / Sev M  
**Lines (current code):** 1078  
**Lines at review (Oct 2026):** 1078

**Function:** `Allocate_Catch()`

**Why it's thought to be a bug**

`bm->SUBFLEET_ECONprms[nf][ns][within_id] = 1`: `within_id` (=15) is a QuotaAlloc (per-species) index. In the SUBFLEET_PARAMS enum, 15 is `res_cost_ind_id`, which ateconindicator.c:533 overwrites with `reswork`. atquota.c:1172 then reads this "within" flag, so it is corrupted by the indicator value and vice versa (also set at atEconomicAnnual.c:241 and atquota.c:1955). Function: `Allocate_Catch()`.

**Current code** (`ateconomic/ateconresponse.c` lines 1076–1080)

```c
 1076: 						if (sp_avail < (1.0 - propstop) * (ownQuota + leaseQuota)) {
 1077: 							bm->QuotaAlloc[nf][ns][sp][within_id] = 1;
 1078: 							bm->SUBFLEET_ECONprms[nf][ns][within_id] = 1;
 1079: 						} else
 1080: 							bm->QuotaAlloc[nf][ns][sp][within_id] = 0;
```

**Suggested fix (from review):** Add a dedicated `sf_within_id` to SUBFLEET_PARAMS and use it at all 5 sites

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

## ECON-16

**division by zero** · Conf M / Sev M  
**Lines (current code):** 1142  
**Lines at review (Oct 2026):** 1142

**Function:** `Allocate_Catch()`

**Why it's thought to be a bug**

`cpueval += BlackBook[..][expect_id] / bm->EffortSchedule[nf][ns][bm->lastMofY][hist_id];` has no `small_num`, and the guard at 1113 only checks `expect_id > 0`, so hist effort can be 0 and give Inf/NaN in ExpectCPUE. Function: `Allocate_Catch()`.

**Current code** (`ateconomic/ateconresponse.c` lines 1140–1144)

```c
 1140: 										* (bm->QuotaAlloc[nf][ns][sp][cummonthcatch_id] - bm->BlackBook[nf][ns][sp][bm->lastMofY][expect_id])
 1141: 										+ bm->BlackBook[nf][ns][sp][bm->lastMofY][expect_id];
 1142: 								cpueval += bm->BlackBook[nf][ns][sp][bm->lastMofY][expect_id] / bm->EffortSchedule[nf][ns][bm->lastMofY][hist_id];
 1143: 
 1144: 								/* For species of concern update bycatch in the BlackBook */
```

**Suggested fix (from review):** Add `+ small_num` to the denominator

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

## ECON-17

**unit conversion wrong direction** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 1259  
**Lines at review (Oct 2026):** 1259

**Function:** `Consolidate_Allowed_Catches()`

**Why it's thought to be a bug**

With deemed values on, "unlimited" allowed catch is set as `totallowedcatch = no_quota * bm->X_CN * mg_2_kg`. `catch_allowed` is in mg N (1238 converts kg to mg N with `kg_2_mg / X_CN`; same as ateconomicsetup.c:249), so this shrinks 1e12 to about 1.1e8 mg N (about 13 t wet), a binding cap instead of no limit. Function: `Consolidate_Allowed_Catches()`.

**Current code** (`ateconomic/ateconresponse.c` lines 1257–1261)

```c
 1257: 						case simpleDV_id: /* Deemed value is simple proportion of price, using constant proportions per species per fishery */
 1258: 						case complexDV_id: /* Deemed value is a proportion estimated explicitly from costs of landing, price of stock, crew share. */
 1259: 							totallowedcatch = no_quota * bm->X_CN * mg_2_kg;
 1260: 							break;
 1261: 						default:
```

**Suggested fix (from review):** `totallowedcatch = no_quota;` (or `no_quota * kg_2_mg / bm->X_CN`)

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

ateconresponse.c:1035: `CurrentEffort_id += Effort_hdistrib[b][nf][today_effort]` adds the whole fleet's effort to every subfleet, while catch is split by the SpatialBlackBook share. Subfleet CPUE (1041) may therefore be understated.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

ateconresponse.c:636-674: the "new subfleet opening up" branch sets `this_subfleet = 0` but never uses it, so no distribution is copied, as the comment says it should be. The `MofY > 10` block also runs for non-economic fisheries.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-3

ateconresponse.c:296-301: `log(resale/...)` and `log(crew_size)` give -inf if a subfleet has 0 resale value or crew.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
