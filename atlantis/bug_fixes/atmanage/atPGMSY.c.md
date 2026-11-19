# atmanage/atPGMSY.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **18 findings** (severity H: 11; confidence H and severity H: 10; on the priority list: 10).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MNGIO-08](#mngio-08) ⭐ | 40-66, 452 | H/H | uninitialised state / NULL dereference | Nothing in the repo calls `Initialise_PGMSY()` (grep finds only its definition). `ASSESS_PROJECTION` (global, … |  | [ ] |
| [MNGIO-09](#mngio-09) ⭐ | 55-56, 127, 131, 338, 341 | H/H | wrong extent / off-by-one | `SSB` and `Depleted` are allocated with length `maxage` but indexed by projection year `iYr` (up to ProjYr-1),… |  | [ ] |
| [MNGIO-10](#mngio-10) ⭐ | 146-153 | H/H | accumulator not reset / div by zero | `RBCspeciesParam[sp][AveF_id] += F_actFleet[sp][nf]` is never zeroed first. GetMultipliers is called several t… |  | [ ] |
| [MNGIO-11](#mngio-11) | 146, 131, 283 | M/M | mixed index spaces | GetMultipliers reads `Fmatrix[nf][this_year]` with nf = fishery and stores `F_actFleet[sp][nf]`. RunPGMSYProje… |  | [ ] |
| [MNGIO-12](#mngio-12) | 264-270 | M/H | incomplete initialisation | The starting population sets only `age = ageclasssize*cohort`, the first age of each cohort, for DEN/LENGTH/WE… |  | [ ] |
| [MNGIO-13](#mngio-13) ⭐ | 319-320 | H/H | wrong year index | `totavailB[nf] += BIO[age][iYr]*sel*q; catch = BIO[age][iYr]*...` but BIO is only ever set for year 0 (line 26… |  | [ ] |
| [MNGIO-14](#mngio-14) ⭐ | 290, 321, 330 | H/H | accumulator in wrong loop | `totF += Fused[nf]` runs inside the age and cohort loops and is only reset once per year. Mortality `exp(-(nat… |  | [ ] |
| [MNGIO-15](#mngio-15) ⭐ | 329-334, 338-340 | H/H | wrong logic / stale value | SSB is accumulated only in the `age == 0` branch and uses `DEN[0][iYr]`, which is not set until line 340, afte… |  | [ ] |
| [MNGIO-16](#mngio-16) | 323-325 | M/M | logic | The catch is capped at the fleet's total RBC per age (`if (catch > RBC[nf]) catch = RBC[nf]`), so the total su… |  | [ ] |
| [MNGIO-17](#mngio-17) | 350-367, 278, 532, 675 | M/M | index-space mixing / div by zero | `RBCupdated[nf][this_year]`, `Fupdated[nf]` and `CatchStore[nf]` are overwritten for `nf < Nfleets` (assessmen… |  | [ ] |
| [MNGIO-18](#mngio-18) ✔ ⭐ | 284, 402, 420, 429, 538-545 | H/H | swapped indices | `AvgCatFleet = Util_Alloc_Init_2D_Double(nYears, K_num_fisheries)` gives `[nf][nYears]` (atAssessTierSetup.c:3… |  | [ ] |
| [MNGIO-19](#mngio-19) | 399-405, 420, 429 | M/M | wrong year index / unpopulated data | `AveF` is built from `FFs[1..AveYr)`, but FFs is only allocated (atAssessTierSetup.c:307) and never filled any… |  | [ ] |
| [MNGIO-20](#mngio-20) ⭐ | 605 | H/H | stale loop variable / OOB | `CatchData[f][r][this_year]` uses `r` left over from the loop at 547-549, so it equals NumRegions of the last … |  | [ ] |
| [MNGIO-21](#mngio-21) ⭐ | 605-607 | H/H | wrong index | `CscalarMetier[nf]` is computed and then read as `CscalarMetier[f]` (f = assessment fleet) to set `RBCupdated[… |  | [ ] |
| [MNGIO-22](#mngio-22) ⭐ | 652-659 | H/H | stale loop variable / OOB | The F-rescaling block sits after the `for (sp...)` loop, so `sp == K_num_tot_sp`. `RBCspeciesParam[sp]`, `RBCs… |  | [ ] |
| [MNGIO-23](#mngio-23) | 652-659, 363 | M/M | logic (bisection) | `Fupdated *= MultBest` compounds across iterations, but bisection on [MultMin, MultMax] assumes an absolute mu… |  | [ ] |
| [MNGIO-24](#mngio-24) | 463, 540-542 | M/M | negative index | `MinCyr = this_year - TriggerCheckPeriod` is not clamped, so early in a run `CatchData[nf][r][Iyr]` is read wi… |  | [ ] |
| [MNGIO-25](#mngio-25) | 114, 131 | H/L | header/format mismatch | The header has 11 column names, but each row prints 12 values (CatchStore is printed with no header column). F… |  | [ ] |

---

## MNGIO-08

**uninitialised state / NULL dereference** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 40-66, 452  
**Lines at review (Oct 2026):** 40-66, 452

**Why it's thought to be a bug**

Nothing in the repo calls `Initialise_PGMSY()` (grep finds only its definition). `ASSESS_PROJECTION` (global, atlantismain.c:225) and the static `PGMSYfp` are therefore NULL when `Do_Atlantis_PGMSY()` runs (from `Call_PGMSY()` when `UseAtlantisPGMSY` is set), giving a crash in `RunPGMSYProjection`/`WritePGMSYStep`. `Free_PGMSY()` is never called either, and PGMSYfp is never closed.

**Current code** (`atmanage/atPGMSY.c` lines 38–68)

```c
   38:  *
   39:  */
   40: void Initialise_PGMSY(MSEBoxModel *bm) {
   41:     int maxage, sp;
   42:     
   43:     ASSESS_PROJECTION = (AssessProjectionStruct *) malloc(sizeof(AssessProjectionStruct) * (size_t)(bm->K_num_tot_sp + 1));
   44:     
   45:     for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
   46:         if( FunctGroupArray[sp].isImpacted == TRUE) {
   47:             maxage = FunctGroupArray[sp].ageClassSize * FunctGroupArray[sp].numCohortsXnumGenes;
   48:             
   49:             ASSESS_PROJECTION[sp].DEN = Util_Alloc_Init_2D_Double(bm->RBCestimation.ProjYr, maxage, 0.0);
   50:             ASSESS_PROJECTION[sp].LENGTH = Util_Alloc_Init_1D_Double(maxage, 0);
   51:             ASSESS_PROJECTION[sp].WEIGHT = Util_Alloc_Init_1D_Double(maxage, 0);
   52:             ASSESS_PROJECTION[sp].BIO = Util_Alloc_Init_2D_Double(bm->RBCestimation.ProjYr, maxage, 0.0);
   53:             ASSESS_PROJECTION[sp].totavailB = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0);
   54:             ASSESS_PROJECTION[sp].Catch = Util_Alloc_Init_2D_Double(bm->RBCestimation.ProjYr, bm->K_num_fisheries, 0.0);
   55:             ASSESS_PROJECTION[sp].SSB = Util_Alloc_Init_1D_Double(maxage, 0);
   56:             ASSESS_PROJECTION[sp].Depleted = Util_Alloc_Init_1D_Double(maxage, 0);
   57:             ASSESS_PROJECTION[sp].Fout = Util_Alloc_Init_2D_Double(bm->RBCestimation.ProjYr, bm->K_num_fisheries, 0.0);
   58:             ASSESS_PROJECTION[sp].Fused = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0);
   59:             ASSESS_PROJECTION[sp].RBC = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0);
   60:         }
   61:     }
   62:     
   63:     PGMSYfp = Init_PGMSY_File(bm);
   64:     
   65:     return;
   66: }
   67: 
   68: /************************************************************************************************************
```

**Suggested fix (from review):** Call Initialise_PGMSY in setup when UseAtlantisPGMSY>0, and call Free_PGMSY plus fclose at shutdown.

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

## MNGIO-09

**wrong extent / off-by-one** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 55-56, 127, 131, 338, 341  
**Lines at review (Oct 2026):** 55-56, 127, 131, 338, 341

**Why it's thought to be a bug**

`SSB` and `Depleted` are allocated with length `maxage` but indexed by projection year `iYr` (up to ProjYr-1), which overflows when ProjYr > maxage. `WritePGMSYStep()` also reads `Depleted[iYr]` with `iYr = ProjYr`, one past the end even if the arrays were sized ProjYr. Functions: `Initialise_PGMSY()`, `WritePGMSYStep()`, `RunPGMSYProjection()`.

**Current code** (`atmanage/atPGMSY.c` lines 53–58)

```c
   53:             ASSESS_PROJECTION[sp].totavailB = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0);
   54:             ASSESS_PROJECTION[sp].Catch = Util_Alloc_Init_2D_Double(bm->RBCestimation.ProjYr, bm->K_num_fisheries, 0.0);
   55:             ASSESS_PROJECTION[sp].SSB = Util_Alloc_Init_1D_Double(maxage, 0);
   56:             ASSESS_PROJECTION[sp].Depleted = Util_Alloc_Init_1D_Double(maxage, 0);
   57:             ASSESS_PROJECTION[sp].Fout = Util_Alloc_Init_2D_Double(bm->RBCestimation.ProjYr, bm->K_num_fisheries, 0.0);
   58:             ASSESS_PROJECTION[sp].Fused = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0);
```

**Suggested fix (from review):** Allocate SSB/Depleted with ProjYr and read `Depleted[ProjYr-1]`.

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

## MNGIO-10

**accumulator not reset / div by zero** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 146-153  
**Lines at review (Oct 2026):** 146-153

**Function:** `GetMultipliers()`

**Why it's thought to be a bug**

`RBCspeciesParam[sp][AveF_id] += F_actFleet[sp][nf]` is never zeroed first. GetMultipliers is called several times per year (lines 562, 575, 599), so AveF keeps growing and FratioFleet is wrong. `/= nfleets` divides by 0 if no fleet has q>0. Function: `GetMultipliers()`.

**Current code** (`atmanage/atPGMSY.c` lines 144–155)

```c
  144:     for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  145:         // Find current fishable biomass so can find F per fishery
  146:         bm->RBCestimation.F_actFleet[sp][nf] = Fmatrix[nf][this_year];
  147:         
  148:         if ( bm->SP_FISHERYprms[sp][nf][q_id] > 0 ) {
  149:             bm->RBCestimation.RBCspeciesParam[sp][AveF_id] += bm->RBCestimation.F_actFleet[sp][nf];
  150:             nfleets += 1.0;
  151:         }
  152:     }
  153:     bm->RBCestimation.RBCspeciesParam[sp][AveF_id] /= nfleets;
  154:     if (bm->RBCestimation.RBCspeciesParam[sp][AveF_id] > 0) {
  155:         for (nf = 0; nf < bm->K_num_fisheries; nf++) {
```

**Suggested fix (from review):** Set AveF_id = 0 before the loop and guard nfleets>0.

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

## MNGIO-11

**mixed index spaces** · Conf M / Sev M  
**Lines (current code):** 146, 131, 283  
**Lines at review (Oct 2026):** 146, 131, 283

**Function:** `GetMultipliers()`

**Why it's thought to be a bug**

GetMultipliers reads `Fmatrix[nf][this_year]` with nf = fishery and stores `F_actFleet[sp][nf]`. RunPGMSYProjection and WritePGMSYStep treat Fmatrix rows and F_actFleet as assessment-fleet indices (`f = SP_FISHERYprms[sp][nf][assess_nf_id]`, `Fmatrix[f]`, `F_actFleet[sp][f]`). Function: `GetMultipliers()`, `WritePGMSYStep()`.

**Current code** (`atmanage/atPGMSY.c` lines 144–148)

```c
  144:     for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  145:         // Find current fishable biomass so can find F per fishery
  146:         bm->RBCestimation.F_actFleet[sp][nf] = Fmatrix[nf][this_year];
  147:         
  148:         if ( bm->SP_FISHERYprms[sp][nf][q_id] > 0 ) {
```

**Suggested fix (from review):** Use one index space (assessment fleet f) throughout.

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

## MNGIO-12

**incomplete initialisation** · Conf M / Sev H  
**Lines (current code):** 264-270  
**Lines at review (Oct 2026):** 264-270

**Function:** `RunPGMSYProjection()`

**Why it's thought to be a bug**

The starting population sets only `age = ageclasssize*cohort`, the first age of each cohort, for DEN/LENGTH/WEIGHT/BIO, although DEN is divided by `age_class_part` (meant to spread over the ages in the cohort). Other ages keep stale values from earlier calls or 0, and get LENGTH=0 for selectivity. maxage is also built from numCohortsXnumGenes, so genotype cohorts are treated as older ages. Function: `RunPGMSYProjection()`.

**Current code** (`atmanage/atPGMSY.c` lines 262–272)

```c
  262: 
  263:     // Set up starting population
  264:     for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
  265:         age = ageclasssize * cohort;
  266:         ASSESS_PROJECTION[species].DEN[age][0] = bm->tot_cohort[species][cohort] * age_class_part;
  267:         ASSESS_PROJECTION[species].LENGTH[age] = Length = linf - (linf * exp(-1.0 * Kbert * (age - tzero)));
  268:         ASSESS_PROJECTION[species].WEIGHT[age] = li_a * pow(Length,li_b) * 1000.0 / (bm->k_wetdry * bm->X_CN);
  269:         ASSESS_PROJECTION[species].BIO[age][0] = ASSESS_PROJECTION[species].DEN[age][0] * ASSESS_PROJECTION[species].WEIGHT[age];
  270:     }
  271: 
  272:     for (nf = 0; nf < Nfleets; nf++) {
```

**Suggested fix (from review):** Loop over every age in each cohort, using numCohorts and treating genotypes separately.

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

## MNGIO-13

**wrong year index** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 319-320  
**Lines at review (Oct 2026):** 319-320

**Function:** `RunPGMSYProjection()`

**Why it's thought to be a bug**

`totavailB[nf] += BIO[age][iYr]*sel*q; catch = BIO[age][iYr]*...` but BIO is only ever set for year 0 (line 269). For iYr≥1 it is 0 or stale, so projected catch and available biomass are 0, and `Fout = Catch/totavailB` (345) is 0/0. Function: `RunPGMSYProjection()`.

**Current code** (`atmanage/atPGMSY.c` lines 317–322)

```c
  317:                     // get final catch
  318:                     q = bm->RBCestimation.RBCspeciesArray[species].PGMSY_q[nf];
  319:                     ASSESS_PROJECTION[species].totavailB[nf] += ASSESS_PROJECTION[species].BIO[age][iYr] * sel * q;
  320:                     catch = ASSESS_PROJECTION[species].BIO[age][iYr] * sel * q * ASSESS_PROJECTION[species].Fused[nf];
  321:                     totF += ASSESS_PROJECTION[species].Fused[nf];
  322:                     
```

**Suggested fix (from review):** Use `DEN[age][iYr-1]*WEIGHT[age]`, or update BIO[age][iYr] each year.

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

## MNGIO-14

**accumulator in wrong loop** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 290, 321, 330  
**Lines at review (Oct 2026):** 290, 321, 330

**Function:** `RunPGMSYProjection()`

**Why it's thought to be a bug**

`totF += Fused[nf]` runs inside the age and cohort loops and is only reset once per year. Mortality `exp(-(nat_mort+totF))` for later ages therefore includes the summed F of every earlier age, and F is not weighted by selectivity. Function: `RunPGMSYProjection()`.

**Current code** (`atmanage/atPGMSY.c` lines 288–327 (truncated))

```c
  288:     for (iYr = 1; iYr < bm->RBCestimation.ProjYr ; iYr++) {
  289:         SSB = 0.0;
  290:         totF = 0.0;
  291:         for (f=0; f < Nfleets; f++) {
  292:             ASSESS_PROJECTION[species].totavailB[f] = 0.0;
  293:             ASSESS_PROJECTION[species].Catch[f][iYr] = 0.0;
  294:         }
  295:         for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
  296:             //  Natural Mortality calculation
  297:             if (cohort < age_mat) {
  298:                 stage = juv_id;
  299:             } else {
  300:                 stage = adult_id;
  301:             }
  302:             base_chort = cohort * ageclasssize;
  303:             max_base_cohort = (cohort + 1) * ageclasssize;
  304: 
  305:             for (age = base_chort; age < max_base_cohort; age++) {
  306:                 nums = ASSESS_PROJECTION[species].DEN[age][iYr-1];
  307:                 
  308:                 // Get projected catch by applying F
  309:                 for (nf = 0; nf < Nfleets; nf++) {
  310:                     lsm = bm->RBCestimation.RBCspeciesArray[species].PGMSY_sel_lsm[nf];
  311:                     sigma = bm->RBCestimation.RBCspeciesArray[species].PGMSY_sel_sigma[nf];
  312:                     sel_curve = (int)(bm->RBCestimation.RBCspeciesArray[species].PGMSY_selcurve[nf]);
  313:                     
  314:                     // get selectivity based on size at age
  315:                     sel = Get_Projection_Selectivity(bm, species, ASSESS_PROJECTION[species].LENGTH[age], sel_curve, lsm, sigma);
  316:                     
  317:                     // get final catch
  318:                     q = bm->RBCestimation.RBCspeciesArray[species].PGMSY_q[nf];
  319:                     ASSESS_PROJECTION[species].totavailB[nf] += ASSESS_PROJECTION[species].BIO[age][iYr] * sel * q;
  320:                     catch = ASSESS_PROJECTION[species].BIO[age][iYr] * sel * q * ASSESS_PROJECTION[species].Fused[nf];
  321:                     totF += ASSESS_PROJECTION[species].Fused[nf];
  322:                     
  323:                     if (catch > ASSESS_PROJECTION[species].RBC[nf])
  324:                         catch = ASSESS_PROJECTION[species].RBC[nf];
  325:                     ASSESS_PROJECTION[species].Catch[nf][iYr] += catch;
  326:                 }
  327:         
```

**Suggested fix (from review):** Compute `totF = Σ_nf sel*q*Fused[nf]` per age, resetting it for each age.

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

## MNGIO-15

**wrong logic / stale value** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 329-334, 338-340  
**Lines at review (Oct 2026):** 329-334, 338-340

**Function:** `RunPGMSYProjection()`

**Why it's thought to be a bug**

SSB is accumulated only in the `age == 0` branch and uses `DEN[0][iYr]`, which is not set until line 340, after SSB has been used. Mature ages never add to SSB, so recruitment and depletion are wrong. Function: `RunPGMSYProjection()`.

**Current code** (`atmanage/atPGMSY.c` lines 327–342)

```c
  327:         
  328:                 // Final numbers
  329:                 if (age > 0) {
  330:                     ASSESS_PROJECTION[species].DEN[age][iYr] = ASSESS_PROJECTION[species].DEN[age-1][iYr-1] * exp(-1.0 * (nat_mort + totF));
  331:                 } else {
  332:                     // Get info for recruiment
  333:                     SSB += FunctGroupArray[species].scaled_FSPB[cohort] * ASSESS_PROJECTION[species].DEN[age][iYr];
  334:                 }
  335:             }
  336:         }
  337:         // Do recruitment and check against Bo
  338:         ASSESS_PROJECTION[species].SSB[iYr] = SSB;
  339:         recruitNum = BHalpha_sp * SSB / (BHbeta_sp + SSB);
  340:         ASSESS_PROJECTION[species].DEN[0][iYr] = recruitNum;
  341:         ASSESS_PROJECTION[species].Depleted[iYr] = SSB / bm->estBo[species];
  342:         
```

**Suggested fix (from review):** Add `scaled_FSPB*DEN[age][iYr]*WEIGHT` for all ages after updating DEN, then compute the recruitment.

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

## MNGIO-16

**logic** · Conf M / Sev M  
**Lines (current code):** 323-325  
**Lines at review (Oct 2026):** 323-325

**Function:** `RunPGMSYProjection()`

**Why it's thought to be a bug**

The catch is capped at the fleet's total RBC per age (`if (catch > RBC[nf]) catch = RBC[nf]`), so the total summed over ages can still exceed the RBC. Function: `RunPGMSYProjection()`.

**Current code** (`atmanage/atPGMSY.c` lines 321–327)

```c
  321:                     totF += ASSESS_PROJECTION[species].Fused[nf];
  322:                     
  323:                     if (catch > ASSESS_PROJECTION[species].RBC[nf])
  324:                         catch = ASSESS_PROJECTION[species].RBC[nf];
  325:                     ASSESS_PROJECTION[species].Catch[nf][iYr] += catch;
  326:                 }
  327:         
```

**Suggested fix (from review):** Apply the cap to the total fleet catch after the age loop.

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

## MNGIO-17

**index-space mixing / div by zero** · Conf M / Sev M  
**Lines (current code):** 350-367, 278, 532, 675  
**Lines at review (Oct 2026):** 350-367, 278, 532, 675

**Function:** `RunPGMSYProjection()`

**Why it's thought to be a bug**

`RBCupdated[nf][this_year]`, `Fupdated[nf]` and `CatchStore[nf]` are overwritten for `nf < Nfleets` (assessment fleets), but RBCupdated is indexed by fishery elsewhere (278, 532, 675). `ncount` is 0 when `AveYr ≤ 1`, and `Fout[nf][iYr]` overflows when `AveYr > ProjYr`. Function: `RunPGMSYProjection()`.

**Current code** (`atmanage/atPGMSY.c` lines 348–369)

```c
  348:     
  349:     // Store updated F
  350:     for (nf = 0; nf < Nfleets; nf++) {
  351:         bm->RBCestimation.RBCspeciesArray[species].RBCupdated[nf][this_year] = 0.0;
  352:         AveF = 0.0;
  353:         AveC = 0.0;
  354:         ncount = 0.0;
  355:         for (iYr = 1; iYr < AveYr ; iYr++) {
  356:             AveF += ASSESS_PROJECTION[species].Fout[nf][iYr];
  357:             AveC += ASSESS_PROJECTION[species].Catch[nf][iYr];
  358:             bm->RBCestimation.RBCspeciesArray[species].RBCupdated[nf][this_year] += ASSESS_PROJECTION[species].Catch[nf][iYr];
  359:             ncount += 1.0;
  360:         }
  361:         AveF /= ncount;
  362:         AveC /= ncount;
  363:         bm->RBCestimation.RBCspeciesArray[species].Fupdated[nf][this_year] = AveF;
  364:         bm->RBCestimation.RBCspeciesArray[species].CatchStore[nf][this_year] = AveC;
  365:         
  366:         // Store RBCs (by fleet)
  367:         bm->RBCestimation.RBCspeciesArray[species].RBCupdated[nf][this_year] /= ncount;
  368:     }
  369: 
```

**Suggested fix (from review):** Map fleets back to fisheries, and clamp AveYr to [2, ProjYr].

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

## MNGIO-18

**swapped indices** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Lines (current code):** 284, 402, 420, 429, 538-545  
**Lines at review (Oct 2026):** 284, 402, 420, 429, 538-545

**Why it's thought to be a bug**

`AvgCatFleet = Util_Alloc_Init_2D_Double(nYears, K_num_fisheries)` gives `[nf][nYears]` (atAssessTierSetup.c:307), but every use is `AvgCatFleet[this_year][nf]` or `[iYr][nf]`. This overflows once the year reaches K_num_fisheries. Functions: `Do_Atlantis_PGMSY()`, `RunPGMSYProjection()`, `RunTier4Projection()`.

**Current code** (`atmanage/atPGMSY.c` lines 282–286)

```c
  282:     for (f = 0; f < Nfleets; f++) {
  283:         ASSESS_PROJECTION[species].Fused[f] = Fmatrix[f][this_year];
  284:         ASSESS_PROJECTION[species].Catch[f][0] = bm->RBCestimation.RBCspeciesArray[species].AvgCatFleet[this_year][f];
  285:     }
  286: 
```

**Suggested fix (from review):** Index as `AvgCatFleet[nf][year]`.

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

## MNGIO-19

**wrong year index / unpopulated data** · Conf M / Sev M  
**Lines (current code):** 399-405, 420, 429  
**Lines at review (Oct 2026):** 399-405, 420, 429

**Function:** `RunTier4Projection()`

**Why it's thought to be a bug**

`AveF` is built from `FFs[1..AveYr)`, but FFs is only allocated (atAssessTierSetup.c:307) and never filled anywhere, so AveF is always 0.5. The comment says "last 10 years", but the loop uses the earliest years. The projected catch for year iYr uses the historical `AvgCatFleet[iYr]` (model year iYr) instead of the current-year fleet split, and `totcatch` sums over years and fleets, which normalises the split wrongly. Function: `RunTier4Projection()`.

**Current code** (`atmanage/atPGMSY.c` lines 397–431)

```c
  397: 
  398:     // Compute average F over the last 10 years
  399:     for (iYr = 1; iYr < AveYr ; iYr++) {
  400:         AveF += 1.0 / (1.0 + exp(-1.0 * bm->RBCestimation.RBCspeciesArray[species].FFs[iYr]));
  401:         for (nf = 0; nf < Nfleets; nf++) {
  402:             totcatch += bm->RBCestimation.RBCspeciesArray[species].AvgCatFleet[iYr][nf];
  403:         }
  404:         ncount += 1.0;
  405:     }
  406:     AveF /= ncount;
  407: 
  408:     // project forward - use age 0 slot for convenience
  409:     ASSESS_PROJECTION[species].BIO[0][0] = bm->RBCestimation.RBCspeciesParam[species][EstBcurr_id];
  410:     for (nf = 0; nf < Nfleets; nf++) {
  411:         ASSESS_PROJECTION[species].RBC[nf] = 0.0;
  412:     }
  413:     for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  414:         f = (int)bm->SP_FISHERYprms[species][nf][assess_nf_id];
  415:         if (bm->RBCestimation.RBCspeciesArray[species].RBCupdated[nf][this_year] < no_quota) {
  416:             ASSESS_PROJECTION[species].RBC[f] += bm->RBCestimation.RBCspeciesArray[species].RBCupdated[nf][this_year];
  417:         }
  418:     }
  419:     for (nf = 0; nf < Nfleets; nf++) {
  420:         ASSESS_PROJECTION[species].Catch[nf][0] = AveF * ASSESS_PROJECTION[species].BIO[0][0] * bm->RBCestimation.RBCspeciesArray[species].AvgCatFleet[0][nf] / (totcatch + small_num);
  421:         if (ASSESS_PROJECTION[species].RBC[nf] > ASSESS_PROJECTION[species].Catch[nf][0])
  422:             ASSESS_PROJECTION[species].RBC[nf] = ASSESS_PROJECTION[species].Catch[nf][0];
  423:     }
  424:     
  425:     minDepletion = 1.0;
  426:     for (iYr = 1; iYr < bm->RBCestimation.ProjYr ; iYr++) {
  427:         thistotcatch = 0.0;
  428:         for (nf = 0; nf < Nfleets; nf++) {
  429:             ASSESS_PROJECTION[species].Catch[nf][iYr] = AveF * ASSESS_PROJECTION[species].BIO[0][iYr-1] * bm->RBCestimation.RBCspeciesArray[species].AvgCatFleet[iYr][nf] / (totcatch + small_num);
  430:             if (ASSESS_PROJECTION[species].Catch[nf][iYr] > ASSESS_PROJECTION[species].RBC[nf])
  431:                 ASSESS_PROJECTION[species].Catch[nf][iYr] = ASSESS_PROJECTION[species].RBC[nf];
```

**Suggested fix (from review):** Populate FFs from the Tier4 fit, use recent years, and use the `AvgCatFleet[nf][this_year]/Σ` split.

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

## MNGIO-20

**stale loop variable / OOB** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 605  
**Lines at review (Oct 2026):** 605

**Function:** `Do_Atlantis_PGMSY()`

**Why it's thought to be a bug**

`CatchData[f][r][this_year]` uses `r` left over from the loop at 547-549, so it equals NumRegions of the last fished species. CatchData is `[nf][K_num_reg+1][nYears]` (atAssessTierSetup.c:115), so this reads the wrong region or out of bounds. Function: `Do_Atlantis_PGMSY()`.

**Current code** (`atmanage/atPGMSY.c` lines 603–607)

```c
  603:                 for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  604:                     f = (int)bm->SP_FISHERYprms[sp][nf][assess_nf_id];
  605:                     bm->RBCestimation.RBCspeciesArray[sp].CscalarMetier[nf] = bm->RBCestimation.RBCspeciesArray[sp].CatchStore[f][this_year] / (bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][r][this_year] + small_num);
  606:                 
  607:                     bm->RBCestimation.RBCspeciesArray[sp].RBCupdated[nf][this_year] = bm->RBCestimation.RBCspeciesArray[sp].CscalarMetier[f] * bm->TACamt[sp][nf][now_id];
```

**Suggested fix (from review):** Sum CatchData over regions (or use the total-region index) explicitly.

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

## MNGIO-21

**wrong index** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 605-607  
**Lines at review (Oct 2026):** 605-607

**Function:** `Do_Atlantis_PGMSY()`

**Why it's thought to be a bug**

`CscalarMetier[nf]` is computed and then read as `CscalarMetier[f]` (f = assessment fleet) to set `RBCupdated[nf]`. Function: `Do_Atlantis_PGMSY()`.

**Current code** (`atmanage/atPGMSY.c` lines 603–609)

```c
  603:                 for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  604:                     f = (int)bm->SP_FISHERYprms[sp][nf][assess_nf_id];
  605:                     bm->RBCestimation.RBCspeciesArray[sp].CscalarMetier[nf] = bm->RBCestimation.RBCspeciesArray[sp].CatchStore[f][this_year] / (bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][r][this_year] + small_num);
  606:                 
  607:                     bm->RBCestimation.RBCspeciesArray[sp].RBCupdated[nf][this_year] = bm->RBCestimation.RBCspeciesArray[sp].CscalarMetier[f] * bm->TACamt[sp][nf][now_id];
  608:                 }
  609:             }
```

**Suggested fix (from review):** Use `CscalarMetier[nf]`.

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

## MNGIO-22

**stale loop variable / OOB** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 652-659  
**Lines at review (Oct 2026):** 652-659

**Function:** `Do_Atlantis_PGMSY()`

**Why it's thought to be a bug**

The F-rescaling block sits after the `for (sp...)` loop, so `sp == K_num_tot_sp`. `RBCspeciesParam[sp]`, `RBCspeciesArray[sp]` and `ASSESS_PROJECTION[sp]` (whose extra entry is uninitialised) are read out of range, and no real species gets rescaled. Function: `Do_Atlantis_PGMSY()`.

**Current code** (`atmanage/atPGMSY.c` lines 650–661)

```c
  650:             }
  651:             if (Finished == FALSE) {
  652:                 MultBest = (MultMax + MultMin) / 2.0;
  653:                 
  654:                 Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]);
  655:                 for (nf = 0; nf < Nfleets; nf++) {
  656:                     bm->RBCestimation.RBCspeciesArray[sp].Fupdated[nf][this_year] *= MultBest;
  657:                 }
  658:                 Iteration += 1;
  659:                 RunPGMSYProjection(bm, sp, this_year, bm->RBCestimation.RBCspeciesArray[sp].Fupdated);  // So ready for the new iteration
  660:             }
  661:         }
```

**Suggested fix (from review):** Wrap the block in a loop over the tier1/tier4 fished species.

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

## MNGIO-23

**logic (bisection)** · Conf M / Sev M  
**Lines (current code):** 652-659, 363  
**Lines at review (Oct 2026):** 652-659, 363

**Function:** `Do_Atlantis_PGMSY()`

**Why it's thought to be a bug**

`Fupdated *= MultBest` compounds across iterations, but bisection on [MultMin, MultMax] assumes an absolute multiplier on a fixed base F. RunPGMSYProjection then overwrites Fupdated[nf][this_year] with AveF (line 363), so the multiplier is lost. Function: `Do_Atlantis_PGMSY()`.

**Current code** (`atmanage/atPGMSY.c` lines 650–661)

```c
  650:             }
  651:             if (Finished == FALSE) {
  652:                 MultBest = (MultMax + MultMin) / 2.0;
  653:                 
  654:                 Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]);
  655:                 for (nf = 0; nf < Nfleets; nf++) {
  656:                     bm->RBCestimation.RBCspeciesArray[sp].Fupdated[nf][this_year] *= MultBest;
  657:                 }
  658:                 Iteration += 1;
  659:                 RunPGMSYProjection(bm, sp, this_year, bm->RBCestimation.RBCspeciesArray[sp].Fupdated);  // So ready for the new iteration
  660:             }
  661:         }
```

**Suggested fix (from review):** Keep a base F vector and set `Fupdated = Fbase*MultBest`.

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

## MNGIO-24

**negative index** · Conf M / Sev M  
**Lines (current code):** 463, 540-542  
**Lines at review (Oct 2026):** 463, 540-542

**Function:** `Do_Atlantis_PGMSY()`

**Why it's thought to be a bug**

`MinCyr = this_year - TriggerCheckPeriod` is not clamped, so early in a run `CatchData[nf][r][Iyr]` is read with Iyr<0. `Call_PGMSY` only checks `earliestYr > year`. Function: `Do_Atlantis_PGMSY()`.

**Current code** (`atmanage/atPGMSY.c` lines 461–465)

```c
  461:     double TAC = 0.0;
  462: 
  463:     int MinCyr = this_year - bm->RBCestimation.TriggerCheckPeriod;
  464:     int MaxCyr = this_year;
  465:     double numYear = (double)(MaxCyr - MinCyr);
```

**Suggested fix (from review):** `MinCyr = max(0, ...)` and recompute numYear.

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

## MNGIO-25

**header/format mismatch** · Conf H / Sev L  
**Lines (current code):** 114, 131  
**Lines at review (Oct 2026):** 114, 131

**Why it's thought to be a bug**

The header has 11 column names, but each row prints 12 values (CatchStore is printed with no header column). Functions: `Init_PGMSY_File()`, `WritePGMSYStep()`.

**Current code** (`atmanage/atPGMSY.c` lines 112–133)

```c
  112: 
  113:     /** Column definitions **/
  114:     fprintf(fid,"Time Species Iteration Fleet AssessF Factual Fupdate FratioFleet RBCupdate Depletion ProjectedDepletion\n");
  115:     
  116:     /* Return file pointer */
  117:     return (fid);
  118: }
  119: 
  120: /************************************************************************************************************
  121:  *
  122:  *    \brief Routine to iwrite out the data to the PGMSY file.
  123:  *
  124:  */
  125: void WritePGMSYStep(FILE *fid, MSEBoxModel *bm, int sp, int this_year, int iteration, double **Fmatrix) {
  126:     int nf, f;
  127:     int iYr = bm->RBCestimation.ProjYr;
  128:     
  129:     for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  130:         f = (int)bm->SP_FISHERYprms[sp][nf][assess_nf_id];
  131:         fprintf(fid,"%d %s %d %s %d %e %e %e %e %e %e %e\n", this_year, FunctGroupArray[sp].groupCode, iteration, FisheryArray[nf].fisheryCode, f, bm->RBCestimation.F_actFleet[sp][f], Fmatrix[f][this_year], bm->RBCestimation.FratioFleet[sp][f], bm->RBCestimation.RBCspeciesArray[sp].RBCupdated[nf][this_year], bm->RBCestimation.RBCspeciesArray[sp].CatchStore[f][this_year], ASSESS_PROJECTION[sp].Depleted[1], ASSESS_PROJECTION[sp].Depleted[iYr]);
  132:     }
  133: }
```

**Suggested fix (from review):** Add a "Catch" column to the header.

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
