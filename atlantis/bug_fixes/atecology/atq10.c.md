# atecology/atq10.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1), plus 1 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [EINIT-21](#einit-21) ⭐ | 151, 194, 202 | H/H | global state clobbered | `Get_Tcorr(bm, sp, H2Otemp, &bm->current_corr)` passes the global `bm->current_corr`, and the humped-Griffith … |  | [ ] |
| [EINIT-22](#einit-22) | 163-170 | H/M | ordering / lost update | `Scale_Linear_Mortality` multiplies `cohortSpeciesParams[stage][mL_id]` by the forcing and change scalars. Whe… |  | [ ] |
| [EINIT-23](#einit-23) | 412-433, 566-579 | L/M | division by zero | `Get_Pollutant_Corrections` can return 0 (when noise_coefft or light_coefft is 0) or a very large value, with … |  | [ ] |
| [EINIT-24](#einit-24) | 254 | H/L | stale message | The quit text says "q10 effect method can only be 0-2", but there are 4 methods (base, humped Griffith, Heinic… |  | [ ] |

---

## EINIT-21

**global state clobbered** · Conf H / Sev H ⭐ priority  
**Listed lines:** 151, 194, 202

**Function:** `Parameter_Q10()`

**Why it's thought to be a bug**

`Get_Tcorr(bm, sp, H2Otemp, &bm->current_corr)` passes the global `bm->current_corr`, and the humped-Griffith branch writes `*current_corr = current_temp - opt_temp;`. Every later species in the loop that uses `base_q10_id` then gets `pow(q10, T - Topt)` instead of `pow(q10, (T - baseline)/10)`. For example, with T=15 and Topt=2 the exponent is 13, giving Q10^13. `Apply_Q10_Corrections` (lines 522/527/532) also uses the clobbered `bm->current_corr` for its warm/cool sign tests. Results depend on species ordering. Function: `Parameter_Q10()` / `Get_Tcorr()`.

**Current code** (`atecology/atq10.c` lines 149–188 (truncated))

```c
  149: 
  150: 	/* Calculate the TCorr value for each group */
  151: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  152:         FunctGroupArray[sp].current_corr = bm->current_corr;
  153: 		FunctGroupArray[sp].Tcorr = Get_Tcorr(bm, sp, H2Otemp, &FunctGroupArray[sp].current_corr);
  154:         FunctGroupArray[sp].TcorrEff = FunctGroupArray[sp].Tcorr;
  155:         
  156: 		/** Include pH and salinity modifiers if desired **/
  157: 		FunctGroupArray[sp].Scorr = Get_Scorr(bm, sp, current_SALT);
  158: 		FunctGroupArray[sp].pHcorr = Get_pHcorr(bm, sp, current_PH, pBox->n, clayer);
  159:         
  160:         FunctGroupArray[sp].PolluteCorr = Get_Pollutant_Corrections(bm, sp, pBox->n, clayer);
  161: 	}
  162: 
  163: 	/** Apply parameter corrections **/
  164: 	/* If we are not tracking contaminants then do q10 calcs now - otherwise they will be done in WaterColumnBox etc each iteration to allow for changes in contaminant levels */
  165: 	if(bm->track_contaminants == FALSE){
  166: 		Apply_Q10_Corrections(bm);
  167: 	}
  168: 
  169: 	/* Now also allow for other factors that effect mortality such as changes due to environmental forcing
  170: 	 * and any changes specified in the change array
  171: 	 */
  172: 	Scale_Linear_Mortality(bm, llogfp, pBox->n);
  173: 
  174: 	return;
  175: }
  176: 
  177: /*
  178:  * \brief Get the temperature corrections to apply to ecophysiology given current water temperature
  179:  * Note that from Griffiths original model fitting exercise
  180:  * K_temp_const_A = 0.851
  181:  * K_temp_const_B = 1.066
  182:  *
  183:  * Equation from Gary G - Tdep_di=log(2)*0.851*(1.066.^T_i).*exp(-((abs(T_i-T_opt_nitzschia)).^3)./1000);
  184:  */
  185: double Get_Tcorr(MSEBoxModel *bm, int sp, double current_temp, double *current_corr) {
  186: 	double ans = 1.0;
  187: 	double step1, step2, step3, step4, stepA, stepB, stepC;
  188: 	double opt_temp;
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
**Listed lines:** 163-170

**Function:** `Parameter_Q10()`

**Why it's thought to be a bug**

`Scale_Linear_Mortality` multiplies `cohortSpeciesParams[stage][mL_id]` by the forcing and change scalars. When `track_contaminants` is TRUE, `Apply_Q10_Corrections` is called later from atecology.c:451/1484/2148, and it recomputes `mL_id` from `mL_T15_id` (lines 570/578). The linear-mortality time series and change scaling is therefore discarded whenever contaminants are tracked. Function: `Parameter_Q10()`.

**Current code** (`atecology/atq10.c` lines 161–172)

```c
  161: 	}
  162: 
  163: 	/** Apply parameter corrections **/
  164: 	/* If we are not tracking contaminants then do q10 calcs now - otherwise they will be done in WaterColumnBox etc each iteration to allow for changes in contaminant levels */
  165: 	if(bm->track_contaminants == FALSE){
  166: 		Apply_Q10_Corrections(bm);
  167: 	}
  168: 
  169: 	/* Now also allow for other factors that effect mortality such as changes due to environmental forcing
  170: 	 * and any changes specified in the change array
  171: 	 */
  172: 	Scale_Linear_Mortality(bm, llogfp, pBox->n);
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
**Listed lines:** 412-433, 566-579

**Function:** `Get_Pollutant_Corrections()`

**Why it's thought to be a bug**

`Get_Pollutant_Corrections` can return 0 (when noise_coefft or light_coefft is 0) or a very large value, with no `small_num` floor, unlike Get_Tcorr and Get_pHcorr. The result is used as a divisor in `(1.0 / FunctGroupArray[sp].PolluteCorr)` for mS, mL and mQ. Function: `Get_Pollutant_Corrections()` / `Apply_Q10_Corrections()`.

**Current code** (`atecology/atq10.c` lines 410–435)

```c
  410: /*
  411:  * \brief Get correction factors based on light and noise pollution
  412:  *
  413:  */
  414: double Get_Pollutant_Corrections(MSEBoxModel *bm, int sp, int b, int clayer) {
  415:     double ans = 1.0;
  416:     
  417:     if (!FunctGroupArray[sp].isNoiseEffected && !FunctGroupArray[sp].isLightEffected)
  418:         return ans;
  419:     
  420:     if (FunctGroupArray[sp].isNoiseEffected && (bm->boxes[b].tr[clayer][Noise_Pollution_i] > 0)) {
  421:         ans = FunctGroupArray[sp].speciesParams[noise_coefft_id] / bm->boxes[b].tr[clayer][Noise_Pollution_i];
  422:     }
  423:     
  424:     if (FunctGroupArray[sp].isLightEffected && (bm->boxes[b].tr[clayer][Light_Pollution_i] > 0)) {
  425:         ans *= FunctGroupArray[sp].speciesParams[light_coefft_id] / bm->boxes[b].tr[clayer][Light_Pollution_i];
  426:     }
  427:     
  428:     /*
  429:     if ((bm->newmonth) && (sp == 1)){
  430:         fprintf(bm->logFile, "Time: %e box-layer: %d-%d %s ans: %e, isNoiseEffected: %d noise_coefft: %e, Noise_Pollution: %e, isLightEffected: %d, light_coefft: %e Light_Pollution: %e\n",
  431:                 bm->dayt, b, clayer, FunctGroupArray[sp].groupCode, ans, FunctGroupArray[sp].isNoiseEffected, FunctGroupArray[sp].speciesParams[noise_coefft_id], bm->boxes[b].tr[clayer][Noise_Pollution_i], FunctGroupArray[sp].isLightEffected, FunctGroupArray[sp].speciesParams[light_coefft_id], bm->boxes[b].tr[clayer][Light_Pollution_i]);
  432:     }
  433:     */
  434:     
  435:     return ans;
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
**Listed lines:** 254

**Function:** `Get_Tcorr()`

**Why it's thought to be a bug**

The quit text says "q10 effect method can only be 0-2", but there are 4 methods (base, humped Griffith, Heinichen, CEATTLE). Function: `Get_Tcorr()`.

**Current code** (`atecology/atq10.c` lines 183–187) — code appears to have moved -69 lines since the review

```c
  183:  * Equation from Gary G - Tdep_di=log(2)*0.851*(1.066.^T_i).*exp(-((abs(T_i-T_opt_nitzschia)).^3)./1000);
  184:  */
  185: double Get_Tcorr(MSEBoxModel *bm, int sp, double current_temp, double *current_corr) {
  186: 	double ans = 1.0;
  187: 	double step1, step2, step3, step4, stepA, stepB, stepC;
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

atq10.c:469 `Apply_Q10_Corrections()`: `contamGrowthScalar = C_growth_corr[0]` is applied to every cohort. This may be intended, but looks like a cohort-0-only value.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
