# atecology/atq10.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [EINIT-21](#einit-21) ⭐ | 153, 204 | H/H | global state clobbered | `Get_Tcorr(bm, sp, H2Otemp, &bm->current_corr)` passes the global `bm->current_corr`, and the humped-Griffith … |  | [ ] |
| [EINIT-22](#einit-22) | 572 | H/M | ordering / lost update | `Scale_Linear_Mortality` multiplies `cohortSpeciesParams[stage][mL_id]` by the forcing and change scalars. Whe… |  | [ ] |
| [EINIT-23](#einit-23) | 568 | L/M | division by zero | `Get_Pollutant_Corrections` can return 0 (when noise_coefft or light_coefft is 0) or a very large value, with … |  | [ ] |
| [EINIT-24](#einit-24) | 256 | H/L | stale message | The quit text says "q10 effect method can only be 0-2", but there are 4 methods (base, humped Griffith, Heinic… |  | [ ] |

---

## EINIT-21

**global state clobbered** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atq10.c` 153, `atq10.c` 204
**Lines at review (Oct 2026):** 151, 194, 202

**Function:** `Parameter_Q10()`

**Why it's thought to be a bug**

`Get_Tcorr(bm, sp, H2Otemp, &bm->current_corr)` passes the global `bm->current_corr`, and the humped-Griffith branch writes `*current_corr = current_temp - opt_temp;`. Every later species in the loop that uses `base_q10_id` then gets `pow(q10, T - Topt)` instead of `pow(q10, (T - baseline)/10)`. For example, with T=15 and Topt=2 the exponent is 13, giving Q10^13. `Apply_Q10_Corrections` (lines ≈522/≈534/≈534) also uses the clobbered `bm->current_corr` for its warm/cool sign tests. Results depend on species ordering. Function: `Parameter_Q10()` / `Get_Tcorr()`.

**Code at issue** — `atq10.c` line 153 (closest match to the quoted code; wording differs slightly)  
Search for: `FunctGroupArray[sp].Tcorr = Get_Tcorr(bm, sp, H2Otemp, &FunctGroupArray[sp].current_corr);`

```c
  151: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  152:         FunctGroupArray[sp].current_corr = bm->current_corr;
  153→ 		FunctGroupArray[sp].Tcorr = Get_Tcorr(bm, sp, H2Otemp, &FunctGroupArray[sp].current_corr);
  154:         FunctGroupArray[sp].TcorrEff = FunctGroupArray[sp].Tcorr;
  155:         
```

**Code at issue** — `atq10.c` line 204  
Search for: `*current_corr = current_temp - opt_temp`

```c
  202:                 step1 = log(2) * temp_const_A * pow(bm->temp_const_B,current_temp);
  203:                 step2 = exp(-bm->temp_const_C * (pow(fabs(current_temp - opt_temp), bm->temp_const_D) / FunctGroupArray[sp].speciesParams[q10_correction_id]));
  204→                 *current_corr = current_temp - opt_temp;
  205: 
  206:                 ans = step1 * step2;
```

**Suggested fix (from review):** Pass a per-species local copy: `double cc = bm->current_corr; Tcorr = Get_Tcorr(bm, sp, H2Otemp, &cc);`

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

## EINIT-22

**ordering / lost update** · Conf H / Sev M  
**Found at (current code):** `atq10.c` 572
**Lines at review (Oct 2026):** 163-170

**Function:** `Parameter_Q10()`

**Why it's thought to be a bug**

`Scale_Linear_Mortality` multiplies `cohortSpeciesParams[stage][mL_id]` by the forcing and change scalars. When `track_contaminants` is TRUE, `Apply_Q10_Corrections` is called later from atecology.c:450/1484/2148, and it recomputes `mL_id` from `mL_T15_id` (lines 572/580). The linear-mortality time series and change scaling is therefore discarded whenever contaminants are tracked. Function: `Parameter_Q10()`.

**Code at issue** — `atq10.c` line 572  
Search for: `cohortSpeciesParams[stage][mL_id]`

```c
  570: 			case AGE_STRUCTURED_BIOMASS:
  571: 				for(stage = 0; stage < FunctGroupArray[sp].numStages; stage++) {
  572→ 					FunctGroupArray[sp].cohortSpeciesParams[stage][mL_id] = FunctGroupArray[sp].cohortSpeciesParams[stage][mL_T15_id] * FunctGroupArray[sp].Tcorr * FunctGroupArray[sp].Scorr * (1.0 / pHscalar) * (1.0 / FunctGroupArray[sp].PolluteCorr);
  573: 					FunctGroupArray[sp].cohortSpeciesParams[stage][mQ_id] = FunctGroupArray[sp].cohortSpeciesParams[stage][mQ_T15_id] * FunctGroupArray[sp].Tcorr * FunctGroupArray[sp].Scorr * (1.0 / pHscalar) * (1.0 / FunctGroupArray[sp].PolluteCorr);
  574: 					FunctGroupArray[sp].cohortSpeciesParams[stage][mE_id] = 0; //1e-05;
```

**Suggested fix (from review):** Call `Scale_Linear_Mortality` after `Apply_Q10_Corrections` in the contaminant path too (e.g. move it inside Apply_Q10_Corrections)

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

## EINIT-23

**division by zero** · Conf L / Sev M  
**Found at (current code):** `atq10.c` 568
**Lines at review (Oct 2026):** 412-433, 566-579

**Function:** `Get_Pollutant_Corrections()`

**Why it's thought to be a bug**

`Get_Pollutant_Corrections` can return 0 (when noise_coefft or light_coefft is 0) or a very large value, with no `small_num` floor, unlike Get_Tcorr and Get_pHcorr. The result is used as a divisor in `(1.0 / FunctGroupArray[sp].PolluteCorr)` for mS, mL and mQ. Function: `Get_Pollutant_Corrections()` / `Apply_Q10_Corrections()`.

**Code at issue** — `atq10.c` line 568  
Search for: `(1.0 / FunctGroupArray[sp].PolluteCorr)`

```c
  566: 
  567: 			/* Mortality - no additional contaminant mortality here as added directly in linear mortality call */
  568→ 			FunctGroupArray[sp].speciesParams[mS_id] = FunctGroupArray[sp].speciesParams[mS_T15_id] * FunctGroupArray[sp].Tcorr * FunctGroupArray[sp].Scorr * (1.0 / pHscalar) * (1.0 / FunctGroupArray[sp].PolluteCorr);
  569: 			switch (FunctGroupArray[sp].groupAgeType) {
  570: 			case AGE_STRUCTURED_BIOMASS:
```

**Suggested fix (from review):** Floor `ans` at `small_num` before returning

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

## EINIT-24

**stale message** · Conf H / Sev L  
**Found at (current code):** `atq10.c` 256
**Lines at review (Oct 2026):** 254

**Function:** `Get_Tcorr()`

**Why it's thought to be a bug**

The quit text says "q10 effect method can only be 0-2", but there are 4 methods (base, humped Griffith, Heinichen, CEATTLE). Function: `Get_Tcorr()`.

**Code at issue** — `atq10.c` line 256 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `quit("How got here as code option not possible - q10 effect method can only be 0-2 and you`

```c
  254:                 break;
  255:             default:
  256→                 quit("How got here as code option not possible - q10 effect method can only be 0-2 and you have %d\n", (int)(FunctGroupArray[sp].speciesParams[q10_method_id]));
  257: 		}
  258: 	} else {
```

**Suggested fix (from review):** Update the text

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

atq10.c:471 `Apply_Q10_Corrections()`: `contamGrowthScalar = C_growth_corr[0]` is applied to every cohort. This may be intended, but looks like a cohort-0-only value.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
