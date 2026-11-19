# ateconomic/ateconindicator.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 2 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [ECIO-05](#ecio-05) | 625, 17 | M/M | unit mismatch | `Expected_Indicators()` computes `fuel_cost = (...) * fuel_cost_scalar;`. The same formula in `Dynamic_Econ_In… |  | [ ] |
| [ECIO-06](#ecio-06) | 349, 17 | L/L | side effect on shared state | `if (CurrentCatch_id < 1.0) SUBFLEET_ECONprms[nf][ns][CurrentCatch_id] = 1.0;` writes a floor into the real st… |  | [ ] |
| [ECIO-07](#ecio-07) | 792, 701 | L/M | mismatched accumulation period | `RevPerT_id = econindx[nf][SubfleetCash_id] / totcatch`. SubfleetCash is cumulative (reset only `if (bm->dayt … |  | [ ] |

---

## ECIO-05

**unit mismatch** · Conf M / Sev M  
**Found at (current code):** `ateconindicator.c` 625, `ateconindicator.c` 17
**Lines at review (Oct 2026):** 625 vs 284

**Function:** `Expected_Indicators()`

**Why it's thought to be a bug**

`Expected_Indicators()` computes `fuel_cost = (...) * fuel_cost_scalar;`. The same formula in `Dynamic_Econ_Indicators()` (l.284) divides by 100.0, with the comment "converting cents to $". So in the expected-indicator path, fuel cost (and therefore `cost_ind_id`, `cost_ton_ind_id` and `boat_cash_ind_id`) comes out 100x too large. Function: `Expected_Indicators()`.

**Code at issue** — `ateconindicator.c` line 625  
Search for: `fuel_cost = (`

```c
  623: 	resid = tsEval(&this_fuel->ts, bm->FuelCost_id, bm->t);
  624: 	fuel_cost_scalar = bm->SUBFLEET_ECONprms[nf][ns][fuel_cost_id];
  625→ 	fuel_cost = (fuel_intercept + trend_coefft * bm->thisyear + month_coefft + auto_coefft * resid) * fuel_cost_scalar;
  626: 	capital_cost = bm->SUBFLEET_ECONprms[nf][ns][capital_cost_id];
  627: 	gear_cost = bm->SUBFLEET_ECONprms[nf][ns][gear_cost_id];
```

**Code at issue** — `ateconindicator.c` line 17  
Search for: `Dynamic_Econ_Indicators()`

```c
   15:  21/11/2006 removed "- cost" from calculation of
   16:  bm->SUBFLEET_ECONprms[nf][ns][boat_cash_ind_id]
   17→  in Dynamic_Econ_Indicators() as costs already
   18:  included in calculation of the revenue term.
   19: 
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
**Found at (current code):** `ateconindicator.c` 349, `ateconindicator.c` 17
**Lines at review (Oct 2026):** 553-555

**Function:** `Dynamic_Econ_Indicators()`

**Why it's thought to be a bug**

`if (CurrentCatch_id < 1.0) SUBFLEET_ECONprms[nf][ns][CurrentCatch_id] = 1.0;` writes a floor into the real state variable instead of a local copy. Anything that reads CurrentCatch later in the month (before the reset at ateconomic.c:229) sees an invented 1 t catch. Function: `Dynamic_Econ_Indicators()`.

**Code at issue** — `ateconindicator.c` line 349 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->SUBFLEET_ECONprms[nf][ns][cpue_ind_id], bm->SUBFLEET_ECONprms[nf][ns][CurrentCatch_id]`

```c
  347: 	if (do_debug) {
  348: 		fprintf(llogfp, "Time: %e, %s-%d has cpue_ind = %e (catch: %e, effort: %e)\n", bm->dayt, FisheryArray[nf].fisheryCode, ns,
  349→ 				bm->SUBFLEET_ECONprms[nf][ns][cpue_ind_id], bm->SUBFLEET_ECONprms[nf][ns][CurrentCatch_id], bm->SUBFLEET_ECONprms[nf][ns][CurrentEffort_id]);
  350: 	}
  351:      */
```

**Code at issue** — `ateconindicator.c` line 17  
Search for: `Dynamic_Econ_Indicators()`

```c
   15:  21/11/2006 removed "- cost" from calculation of
   16:  bm->SUBFLEET_ECONprms[nf][ns][boat_cash_ind_id]
   17→  in Dynamic_Econ_Indicators() as costs already
   18:  included in calculation of the revenue term.
   19: 
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
**Found at (current code):** `ateconindicator.c` 792, `ateconindicator.c` 701
**Lines at review (Oct 2026):** 764, 778, 789-792

**Function:** `Update_Econ_Indx()`

**Why it's thought to be a bug**

`RevPerT_id = econindx[nf][SubfleetCash_id] / totcatch`. SubfleetCash is cumulative (reset only `if (bm->dayt < 1.0)` and at ateconomic.c:73), while `totcatch` is rebuilt every call from `cumboatcatch_id`. If the two cover different periods, revenue per tonne is biased. The same applies to `RevPerEffort_id`, which uses `CumEffort`. Function: `Update_Econ_Indx()`.

**Code at issue** — `ateconindicator.c` line 792 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->econindx[nf][RevPerT_id] = bm->econindx[nf][SubfleetCash_id] / (totcatch + small_num);`

```c
  790: 			bm->econindx[nf][RevPerT_id] = 0;
  791: 		else
  792→ 			bm->econindx[nf][RevPerT_id] = bm->econindx[nf][SubfleetCash_id] / (totcatch + small_num);
  793: 		if (!numexchange)
  794: 			bm->econindx[nf][TradeExtent_id] = 0;
```

**Code at issue** — `ateconindicator.c` line 701  
Search for: `if (bm->dayt < 1.0)`

```c
  699: 	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  700: 		/* Initialise for the first year */
  701→ 		if (bm->dayt < 1.0) {
  702: 			bm->econindx[nf][fleet_switch_id] = 0;
  703: 			bm->econindx[nf][overallrent_id] = 0;
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
