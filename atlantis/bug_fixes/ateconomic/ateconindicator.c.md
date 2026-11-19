# ateconomic/ateconindicator.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 2 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [ECIO-05](#ecio-05) | 625 vs 284 | M/M | unit mismatch | `Expected_Indicators()` computes `fuel_cost = (...) * fuel_cost_scalar;`. The same formula in `Dynamic_Econ_In… |  | [ ] |
| [ECIO-06](#ecio-06) | 553-555 | L/L | side effect on shared state | `if (CurrentCatch_id < 1.0) SUBFLEET_ECONprms[nf][ns][CurrentCatch_id] = 1.0;` writes a floor into the real st… |  | [ ] |
| [ECIO-07](#ecio-07) | 764, 778, 789-792 | L/M | mismatched accumulation period | `RevPerT_id = econindx[nf][SubfleetCash_id] / totcatch`. SubfleetCash is cumulative (reset only `if (bm->dayt … |  | [ ] |

---

## ECIO-05

**unit mismatch** · Conf M / Sev M  
**Lines (current code):** 625 vs 284  
**Lines at review (Oct 2026):** 625 vs 284

**Function:** `Expected_Indicators()`

**Why it's thought to be a bug**

`Expected_Indicators()` computes `fuel_cost = (...) * fuel_cost_scalar;`. The same formula in `Dynamic_Econ_Indicators()` (l.284) divides by 100.0, with the comment "converting cents to $". So in the expected-indicator path, fuel cost (and therefore `cost_ind_id`, `cost_ton_ind_id` and `boat_cash_ind_id`) comes out 100x too large. Function: `Expected_Indicators()`.

**Current code** (`ateconomic/ateconindicator.c` lines 623–627)

```c
  623: 	resid = tsEval(&this_fuel->ts, bm->FuelCost_id, bm->t);
  624: 	fuel_cost_scalar = bm->SUBFLEET_ECONprms[nf][ns][fuel_cost_id];
  625: 	fuel_cost = (fuel_intercept + trend_coefft * bm->thisyear + month_coefft + auto_coefft * resid) * fuel_cost_scalar;
  626: 	capital_cost = bm->SUBFLEET_ECONprms[nf][ns][capital_cost_id];
  627: 	gear_cost = bm->SUBFLEET_ECONprms[nf][ns][gear_cost_id];
```

**Suggested fix (from review):** Add `/ 100.0`, matching l.284.

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

## ECIO-06

**side effect on shared state** · Conf L / Sev L  
**Lines (current code):** 553-555  
**Lines at review (Oct 2026):** 553-555

**Function:** `Dynamic_Econ_Indicators()`

**Why it's thought to be a bug**

`if (CurrentCatch_id < 1.0) SUBFLEET_ECONprms[nf][ns][CurrentCatch_id] = 1.0;` writes a floor into the real state variable instead of a local copy. Anything that reads CurrentCatch later in the month (before the reset at ateconomic.c:229) sees an invented 1 t catch. Function: `Dynamic_Econ_Indicators()`.

**Current code** (`ateconomic/ateconindicator.c` lines 551–557)

```c
  551: 	 landed - if nothing landed little difference anyway in reality.
  552: 	 */
  553: 	if (bm->SUBFLEET_ECONprms[nf][ns][CurrentCatch_id] < 1.0)
  554: 		bm->SUBFLEET_ECONprms[nf][ns][CurrentCatch_id] = 1.0;
  555: 	bm->SUBFLEET_ECONprms[nf][ns][rev_land_ind_id] = revenue / (bm->SUBFLEET_ECONprms[nf][ns][CurrentCatch_id] + small_num);
  556: 
  557: 	/* Return on investment - profit / total investment (costs and capital dollars put into it) */
```

**Suggested fix (from review):** Use a local `catch_floor = max(1.0, CurrentCatch)` for the division.

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

## ECIO-07

**mismatched accumulation period** · Conf L / Sev M  
**Lines (current code):** 764, 778, 789-792  
**Lines at review (Oct 2026):** 764, 778, 789-792

**Function:** `Update_Econ_Indx()`

**Why it's thought to be a bug**

`RevPerT_id = econindx[nf][SubfleetCash_id] / totcatch`. SubfleetCash is cumulative (reset only `if (bm->dayt < 1.0)` and at ateconomic.c:73), while `totcatch` is rebuilt every call from `cumboatcatch_id`. If the two cover different periods, revenue per tonne is biased. The same applies to `RevPerEffort_id`, which uses `CumEffort`. Function: `Update_Econ_Indx()`.

**Current code** (`ateconomic/ateconindicator.c` lines 762–794)

```c
  762: 
  763: 			/* Boat cash */
  764: 			bm->econindx[nf][SubfleetCash_id] += bm->SUBFLEET_ECONprms[nf][ns][boat_cash_ind_id];
  765: 
  766: 			/* Leased quota value - only if commercial */
  767: 			if (!flagrecfish)
  768: 				bm->econindx[nf][LeaseVal_id] += bm->SUBFLEET_ECONprms[nf][ns][leased_Q_val_ind_id];
  769: 			else
  770: 				bm->econindx[nf][LeaseVal_id] = 0;
  771: 
  772: 			/* Return on investment */
  773: 			bm->econindx[nf][InvestReturn_id] += bm->SUBFLEET_ECONprms[nf][ns][invest_return_ind_id];
  774: 
  775: 			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  776: 				if (FunctGroupArray[sp].isFished == TRUE) {
  777: 					/* Revenue per tonne */
  778: 					totcatch += bm->QuotaAlloc[nf][ns][sp][cumboatcatch_id];
  779: 
  780: 					/* Trading extent */
  781: 					prop_olease = bm->QuotaAlloc[nf][ns][sp][permolease_id];
  782: 					prop_tlease = bm->QuotaAlloc[nf][ns][sp][templease_id];
  783: 
  784: 					bm->econindx[nf][TradeExtent_id] += (prop_olease + prop_tlease);
  785: 					numexchange++;
  786: 				}
  787: 			}
  788: 		}
  789: 		if (!totcatch)
  790: 			bm->econindx[nf][RevPerT_id] = 0;
  791: 		else
  792: 			bm->econindx[nf][RevPerT_id] = bm->econindx[nf][SubfleetCash_id] / (totcatch + small_num);
  793: 		if (!numexchange)
  794: 			bm->econindx[nf][TradeExtent_id] = 0;
```

**Suggested fix (from review):** Make sure numerator and denominator cover the same period.

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

ateconindicator.c:529 (Holland branch) computes management cost as `gvp * (1 - mgmt_cost_id)`. If `mgmt_cost` is the cost-recovery fraction (as the comment "cost recovery costs only" suggests), the cost would be `gvp * mgmt_cost`. Check the parameter's definition.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

ateconindicator.c:439 divides spatial gvp by `nsubfleets` even though `ECONexprice`/`SpatialBlackBook` are already per subfleet. Possible double scaling, by design or not.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
