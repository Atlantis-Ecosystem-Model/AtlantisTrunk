# ateconomic/atEconomicAnnual.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **10 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [ECON-18](#econ-18) ⭐ | 507, 521, 317 | H/H | accumulator in wrong loop | `tot_expecteffort += bm->EffortSchedule[nf][ns][month][hist_id];` is inside the species loop, so annual effort… |  | [ ] |
| [ECON-19](#econ-19) | 350 | H/M | accumulator never reset | `QuotaAlloc[..][spexpectvalue_id] +=` every year, but only `spexpectcatch_id` is reset (Initialise_Annual_Effo… |  | [ ] |
| [ECON-20](#econ-20) | 818 | H/M | wrong index | `expectedCatch += EffortSchedule[month][expect] * BlackBook[month][expect] / (EffortSchedule[nf][ns][bm->MofY]… |  | [ ] |
| [ECON-21](#econ-21) | 379, 398 | M/M | rank vs species index | `maxTargetCatch` and `tempTarget` are indexed by rank `nt`, but the reset loop only clears positions whose spe… |  | [ ] |
| [ECON-22](#econ-22) | 419 | M/M | logic / parameter overwritten | `ntarg` starts at 1 and grows with every insertion, so every species with positive expected catch becomes a ta… |  | [ ] |
| [ECON-23](#econ-23) | 431 | M/M | sentinel collision | `SUBFISHERYtarget[nf][ns][newtarg] = nt;` stores the rank, with 0 for the top target, but 0 is also the "not a… |  | [ ] |
| [ECON-24](#econ-24) | 1047, 794 | M/M | unit mismatch | `QuotaAlloc[..][permolease_id] *= CatchReScale[sp]` and the same for `templease_id`: `permolease_id`/`templeas… |  | [ ] |
| [ECON-25](#econ-25) | 541 | M/M | division by zero | `totppi *= (oldtotpi / (TripCost * tot_expecteffort));` has no `small_num` (unlike 534/538). If TripCost or ex… |  | [ ] |
| [ECON-26](#econ-26) | 247 | M/L | accumulator never reset | `SUBFLEET_ECONprms[..][AnnualCatch_id] +=` every year and is never zeroed (grep), so the "AnnCatch" output is … |  | [ ] |
| [ECON-27](#econ-27) | 739 | H/L | log text | The message says "100th iteration" but the bail-out is at `itc > 1000`. Function: `Stocastic_Effort_Allocation… |  | [ ] |

---

## ECON-18

**accumulator in wrong loop** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atEconomicAnnual.c` 507, `atEconomicAnnual.c` 521, `atEconomicAnnual.c` 317
**Lines at review (Oct 2026):** 496-515

**Function:** `Dynamic_Effort_Allocation()`

**Why it's thought to be a bug**

`tot_expecteffort += bm->EffortSchedule[nf][ns][month][hist_id];` is inside the species loop, so annual effort is multiplied by the number of catchable species. `potential_totppi = totppi - TripCost*tot_expecteffort` (521, 541, 543) then overstates costs about nsp-fold. Compare `Calculate_Projected_Income()` (295/317), which subtracts `TripCost*ExpEffort` once per month. Function: `Dynamic_Effort_Allocation()`.

**Code at issue** — `atEconomicAnnual.c` line 507  
Search for: `tot_expecteffort += bm->EffortSchedule[nf][ns][month][hist_id]`

```c
  505: 
  506: 				/* Total expected effort */
  507→ 				tot_expecteffort += bm->EffortSchedule[nf][ns][month][hist_id];
  508: 
  509: 				if (do_debug) {
```

**Code at issue** — `atEconomicAnnual.c` line 521 (closest match to the quoted code; wording differs slightly)  
Search for: `potential_totppi = totppi - (TripCost * tot_expecteffort);`

```c
  519: 		//totppi -= TripCost;
  520: 		tolerable_debt = -1.0 * bm->SUBFLEET_ECONprms[nf][ns][tol_debt_id];
  521→ 		potential_totppi = totppi - (TripCost * tot_expecteffort);
  522: 
  523: 		if ((bm->dayt < 1) || (bm->dayt < (bm->hist_only_timeout + 360.0)))
```

**Code at issue** — `atEconomicAnnual.c` line 317  
Search for: `TripCost * ExpEffort`

```c
  315: 
  316: 		if (!bm->OrigEconCalc) {
  317→ 			totexpect_return -= (TripCost * ExpEffort);
  318: 			//totexpect_return -= TripCost;
  319: 		}
```

**Suggested fix (from review):** Move the `tot_expecteffort` line out of the sp loop (directly inside the month loop)

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

## ECON-19

**accumulator never reset** · Conf H / Sev M  
**Found at (current code):** `atEconomicAnnual.c` 350
**Lines at review (Oct 2026):** 350

**Function:** `Calculate_Projected_Income()`

**Why it's thought to be a bug**

`QuotaAlloc[..][spexpectvalue_id] +=` every year, but only `spexpectcatch_id` is reset (Initialise_Annual_Effort 249; grep shows no reset for spexpectvalue_id). Expected value grows without bound, which skews targeting when `UseVPUE` is set (394). Function: `Calculate_Projected_Income()`.

**Code at issue** — `atEconomicAnnual.c` line 350 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->QuotaAlloc[nf][ns][sp][spexpectvalue_id] += bm->BlackBook[nf][ns][sp][month][expect_id`

```c
  348: 					all_pi[sp] += pi[nf][ns][month][sp];
  349: 					bm->QuotaAlloc[nf][ns][sp][spexpectcatch_id] += bm->BlackBook[nf][ns][sp][month][expect_id];
  350→ 					bm->QuotaAlloc[nf][ns][sp][spexpectvalue_id] += bm->BlackBook[nf][ns][sp][month][expect_id] * (bm->SP_FISHERYprms[sp][nf][saleprice_id]
  351: 							- calcTax);
  352: 					bm->MonthAlloc[nf][ns][sp][month] = bm->BlackBook[nf][ns][sp][month][expect_id];
```

**Suggested fix (from review):** Reset `spexpectvalue_id` next to `spexpectcatch_id` at line 249

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

## ECON-20

**wrong index** · Conf H / Sev M  
**Found at (current code):** `atEconomicAnnual.c` 818
**Lines at review (Oct 2026):** 1012

**Function:** `Calculate_Box_Allocation()`

**Why it's thought to be a bug**

`expectedCatch += EffortSchedule[month][expect] * BlackBook[month][expect] / (EffortSchedule[nf][ns][bm->MofY][hist_id] + small_num)` uses the current month instead of the loop `month`. The same formula at 583 and 817-818 uses `month`, and the comment at 818 says MofY was a past mistake. Function: `Calculate_Box_Allocation()`.

**Code at issue** — `atEconomicAnnual.c` line 818 (located by name only; check this is the right place)  
Search for: `/ (bm->EffortSchedule[nf][ns][month][hist_id] + small_num);  // Denominator previously use`

```c
  816: 			expectedCatch
  817: 					+= bm->EffortSchedule[nf][ns][month][expect_id] * bm->BlackBook[nf][ns][sp][month][expect_id]
  818→ 							/ (bm->EffortSchedule[nf][ns][month][hist_id] + small_num);  // Denominator previously used MofY not month
  819: 		}
  820: 		bm->QuotaAlloc[nf][ns][sp][spexpectcatch_id] = expectedCatch;
```

**Suggested fix (from review):** Replace `bm->MofY` with `month`

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

## ECON-21

**rank vs species index** · Conf M / Sev M  
**Found at (current code):** `atEconomicAnnual.c` 379, `atEconomicAnnual.c` 398
**Lines at review (Oct 2026):** 378-381, 398-407

**Function:** `Identify_Target_Species()`

**Why it's thought to be a bug**

`maxTargetCatch` and `tempTarget` are indexed by rank `nt`, but the reset loop only clears positions whose species index `sp` is fished. Rank slots at non-fished species indices keep values from the previous subfleet or fishery, which distorts the insertion sort and can make a species from another fleet a target. Function: `Identify_Target_Species()`.

**Code at issue** — `atEconomicAnnual.c` line 379 (the line the review pointed to, found by its text)  
Search for: `maxTargetCatch[sp] = 0;`

```c
  377: 		if (FunctGroupArray[sp].isFished == TRUE) {
  378: 			tempTarget[sp] = -1;
  379→ 			maxTargetCatch[sp] = 0;
  380: 		}
  381: 	}
```

**Code at issue** — `atEconomicAnnual.c` line 398 (the line the review pointed to, found by its text)  
Search for: `if (check_value > maxTargetCatch[nt]) {`

```c
  396: 					check_value = bm->QuotaAlloc[nf][ns][sp][spexpectcatch_id];
  397: 
  398→ 				if (check_value > maxTargetCatch[nt]) {
  399: 					for (ntt = ntarg - 1; ntt > nt - 1; ntt--) {
  400: 						/* Shuffle existing records out of the way */
```

**Suggested fix (from review):** Reset all K_num_tot_sp entries unconditionally

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

## ECON-22

**logic / parameter overwritten** · Conf M / Sev M  
**Found at (current code):** `atEconomicAnnual.c` 419
**Lines at review (Oct 2026):** 383-419

**Function:** `Identify_Target_Species()`

**Why it's thought to be a bug**

`ntarg` starts at 1 and grows with every insertion, so every species with positive expected catch becomes a target. The `ntargets` parameter (read at 288) never limits it, and `FISHERYprms[nf][ntargets_id]` is overwritten with the growing count, which changes `ntarg` for later subfleets and years (`Get_Expected_Profit` divides TripCost by it). Function: `Identify_Target_Species()`.

**Code at issue** — `atEconomicAnnual.c` line 419  
Search for: `FISHERYprms[nf][ntargets_id]`

```c
  417: 	}
  418: 	/* Update new number of target groups */
  419→ 	bm->FISHERYprms[nf][ntargets_id] = ntarg;
  420: 	fprintf(bm->logFile, "ntarg = %d\n", ntarg);
  421: 
```

**Suggested fix (from review):** Keep the parameter value and cap insertion and ntarg at it

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

## ECON-23

**sentinel collision** · Conf M / Sev M  
**Found at (current code):** `atEconomicAnnual.c` 431
**Lines at review (Oct 2026):** 427, 431, 444-447

**Function:** `Identify_Target_Species()`

**Why it's thought to be a bug**

`SUBFISHERYtarget[nf][ns][newtarg] = nt;` stores the rank, with 0 for the top target, but 0 is also the "not a target" value (447). So the top target is logged as "switching to target" every year (427) and is never stripped (444 `> 0`). ateconeffort.c:185 also gives non-targets the same top-target weight. Function: `Identify_Target_Species()`.

**Code at issue** — `atEconomicAnnual.c` line 431  
Search for: `SUBFISHERYtarget[nf][ns][newtarg] = nt`

```c
  429: 						FunctGroupArray[newtarg].groupCode);
  430: 			}
  431→ 			bm->SUBFISHERYtarget[nf][ns][newtarg] = nt;
  432: 		}
  433: 	}
```

**Suggested fix (from review):** Store `nt + 1` (and use `nt-1` at ateconeffort.c:185), or use -1 for non-target

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

## ECON-24

**unit mismatch** · Conf M / Sev M  
**Found at (current code):** `atEconomicAnnual.c` 1047, `atEconomicAnnual.c` 794
**Lines at review (Oct 2026):** 1047-1048

**Function:** `Do_EconArray_Rescaling()`

**Why it's thought to be a bug**

`QuotaAlloc[..][permolease_id] *= CatchReScale[sp]` and the same for `templease_id`: `permolease_id`/`templease_id` are used as proportions (`1.0 - prop_olease - prop_tlease`, lines 599-601, ateconresponse.c:1065-1067), so scaling them by a catch factor can push them above 1. The tonnage `leased_id` is not rescaled. Function: `Do_EconArray_Rescaling()`.

**Code at issue** — `atEconomicAnnual.c` line 1047 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->QuotaAlloc[nf][ns][sp][permolease_id] *= bm->CatchReScale[sp];`

```c
 1045:             }
 1046:             for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 1047→                 bm->QuotaAlloc[nf][ns][sp][permolease_id] *= bm->CatchReScale[sp];
 1048:                 bm->QuotaAlloc[nf][ns][sp][templease_id] *= bm->CatchReScale[sp];
 1049:                 bm->QuotaAlloc[nf][ns][sp][owned_id] *= bm->CatchReScale[sp];
```

**Code at issue** — `atEconomicAnnual.c` line 794  
Search for: `1.0 - prop_olease - prop_tlease`

```c
  792: 
  793: 			/* Update quota_avail - take away new expected catch to see amount remaining */
  794→ 			quota_avail = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[nf][ns][sp][owned_id] + bm->QuotaAlloc[nf][ns][sp][leased_id] - cumcatch;
  795: 			quota_avail *= 0.01; // Converted from kg to 100kg
  796: 			quota_avail = floor(quota_avail + 0.5); // So will not bother chasing final 50kg of quota
```

**Suggested fix (from review):** Rescale `leased_id` instead of the proportions

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

## ECON-25

**division by zero** · Conf M / Sev M  
**Found at (current code):** `atEconomicAnnual.c` 541
**Lines at review (Oct 2026):** 541

**Function:** `Dynamic_Effort_Allocation()`

**Why it's thought to be a bug**

`totppi *= (oldtotpi / (TripCost * tot_expecteffort));` has no `small_num` (unlike 534/538). If TripCost or expected effort is 0 and totppi < 0, this gives Inf/NaN. Function: `Dynamic_Effort_Allocation()`.

**Code at issue** — `atEconomicAnnual.c` line 541  
Search for: `totppi *= (oldtotpi / (TripCost * tot_expecteffort))`

```c
  539: 		} else if (potential_totppi < 0.0) {
  540: 			/* Else just make sure contract effort appropriately */
  541→ 			totppi *= (oldtotpi / (TripCost * tot_expecteffort));
  542: 		} else
  543: 			totppi -= (TripCost * tot_expecteffort);
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

## ECON-26

**accumulator never reset** · Conf M / Sev L  
**Found at (current code):** `atEconomicAnnual.c` 247
**Lines at review (Oct 2026):** 247

**Function:** `Initialise_Annual_Effort()`

**Why it's thought to be a bug**

`SUBFLEET_ECONprms[..][AnnualCatch_id] +=` every year and is never zeroed (grep), so the "AnnCatch" output is cumulative over the whole run, not annual. Function: `Initialise_Annual_Effort()`.

**Code at issue** — `atEconomicAnnual.c` line 247 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->SUBFLEET_ECONprms[nf][ns][AnnualCatch_id] += bm->QuotaAlloc[nf][ns][sp][cumboatcatch_i`

```c
  245: 						if (!bm->SP_FISHERYprms[sp][nf][q_id])
  246: 							continue;
  247→ 						bm->SUBFLEET_ECONprms[nf][ns][AnnualCatch_id] += bm->QuotaAlloc[nf][ns][sp][cumboatcatch_id];
  248: 						bm->QuotaAlloc[nf][ns][sp][cumboatcatch_id] = 0; // Reset annual total catch
  249: 						bm->QuotaAlloc[nf][ns][sp][spexpectcatch_id] = 0;
```

**Suggested fix (from review):** Zero AnnualCatch_id before the species loop

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

## ECON-27

**log text** · Conf H / Sev L  
**Found at (current code):** `atEconomicAnnual.c` 739
**Lines at review (Oct 2026):** 740

**Function:** `Stocastic_Effort_Allocation()`

**Why it's thought to be a bug**

The message says "100th iteration" but the bail-out is at `itc > 1000`. Function: `Stocastic_Effort_Allocation()`.

**Code at issue** — `atEconomicAnnual.c` line 739  
Search for: `itc > 1000`

```c
  737: 			}
  738: 
  739→ 			if (itc > 1000) {
  740: 				fprintf(llogfp, "Had to bail on 100th iteration for %s-%d sp: %s\n", FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode);
  741: 				quota_avail = 0;
```

**Suggested fix (from review):** Fix the text

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

atEconomicAnnual.c:401-402: the insertion shuffle writes index `ntarg`, which can reach K_num_tot_sp (out of bounds) only if every group is fished and catchable. Unlikely.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
