# atmanage/atPGMSY.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **18 findings** (severity H: 11; confidence H and severity H: 10; on the priority list: 10).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MNGIO-08](#mngio-08) ⭐ | 137 | H/H | uninitialised state / NULL dereference | Nothing in the repo calls `Initialise_PGMSY()` (grep finds only its definition). `ASSESS_PROJECTION` (global, … |  | [ ] |
| [MNGIO-09](#mngio-09) ⭐ | 131, 127, 165 | H/H | wrong extent / off-by-one | `SSB` and `Depleted` are allocated with length `maxage` but indexed by projection year `iYr` (up to ProjYr-1),… |  | [ ] |
| [MNGIO-10](#mngio-10) ⭐ | 149, 153 | H/H | accumulator not reset / div by zero | `RBCspeciesParam[sp][AveF_id] += F_actFleet[sp][nf]` is never zeroed first. GetMultipliers is called several t… |  | [ ] |
| [MNGIO-11](#mngio-11) | 146, 131 | M/M | mixed index spaces | GetMultipliers reads `Fmatrix[nf][this_year]` with nf = fishery and stores `F_actFleet[sp][nf]`. RunPGMSYProje… |  | [ ] |
| [MNGIO-12](#mngio-12) | 265, 165 | M/H | incomplete initialisation | The starting population sets only `age = ageclasssize*cohort`, the first age of each cohort, for DEN/LENGTH/WE… |  | [ ] |
| [MNGIO-13](#mngio-13) ⭐ | 345, 165 | H/H | wrong year index | `totavailB[nf] += BIO[age][iYr]*sel*q; catch = BIO[age][iYr]*...` but BIO is only ever set for year 0 (line 26… |  | [ ] |
| [MNGIO-14](#mngio-14) ⭐ | 321, 330, 165 | H/H | accumulator in wrong loop | `totF += Fused[nf]` runs inside the age and cohort loops and is only reset once per year. Mortality `exp(-(nat… |  | [ ] |
| [MNGIO-15](#mngio-15) ⭐ | 340, 165 | H/H | wrong logic / stale value | SSB is accumulated only in the `age == 0` branch and uses `DEN[0][iYr]`, which is not set until line 340, afte… |  | [ ] |
| [MNGIO-16](#mngio-16) | 165 | M/M | logic | The catch is capped at the fleet's total RBC per age (`if (catch > RBC[nf]) catch = RBC[nf]`), so the total su… |  | [ ] |
| [MNGIO-17](#mngio-17) | 351, 363, 345 | M/M | index-space mixing / div by zero | `RBCupdated[nf][this_year]`, `Fupdated[nf]` and `CatchStore[nf]` are overwritten for `nf < Nfleets` (assessmen… |  | [ ] |
| [MNGIO-18](#mngio-18) ✔ ⭐ | 538, 402, 236 | H/H | swapped indices | `AvgCatFleet = Util_Alloc_Init_2D_Double(nYears, K_num_fisheries)` gives `[nf][nYears]` (atAssessTierSetup.c:3… |  | [ ] |
| [MNGIO-19](#mngio-19) | 402 | M/M | wrong year index / unpopulated data | `AveF` is built from `FFs[1..AveYr)`, but FFs is only allocated (atAssessTierSetup.c:307) and never filled any… |  | [ ] |
| [MNGIO-20](#mngio-20) ⭐ | 605, 376 | H/H | stale loop variable / OOB | `CatchData[f][r][this_year]` uses `r` left over from the loop at 547-549, so it equals NumRegions of the last … |  | [ ] |
| [MNGIO-21](#mngio-21) ⭐ | 605, 376 | H/H | wrong index | `CscalarMetier[nf]` is computed and then read as `CscalarMetier[f]` (f = assessment fleet) to set `RBCupdated[… |  | [ ] |
| [MNGIO-22](#mngio-22) ⭐ | 671, 654, 131 | H/H | stale loop variable / OOB | The F-rescaling block sits after the `for (sp...)` loop, so `sp == K_num_tot_sp`. `RBCspeciesParam[sp]`, `RBCs… |  | [ ] |
| [MNGIO-23](#mngio-23) | 656, 376 | M/M | logic (bisection) | `Fupdated *= MultBest` compounds across iterations, but bisection on [MultMin, MultMax] assumes an absolute mu… |  | [ ] |
| [MNGIO-24](#mngio-24) | 463, 542, 376 | M/M | negative index | `MinCyr = this_year - TriggerCheckPeriod` is not clamped, so early in a run `CatchData[nf][r][Iyr]` is read wi… |  | [ ] |
| [MNGIO-25](#mngio-25) | 102 | H/L | header/format mismatch | The header has 11 column names, but each row prints 12 values (CatchStore is printed with no header column). F… |  | [ ] |

---

## MNGIO-08

**uninitialised state / NULL dereference** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atPGMSY.c` 137
**Lines at review (Oct 2026):** 40-66, 452

**Why it's thought to be a bug**

Nothing in the repo calls `Initialise_PGMSY()` (grep finds only its definition). `ASSESS_PROJECTION` (global, atlantismain.c:225) and the static `PGMSYfp` are therefore NULL when `Do_Atlantis_PGMSY()` runs (from `Call_PGMSY()` when `UseAtlantisPGMSY` is set), giving a crash in `RunPGMSYProjection`/`WritePGMSYStep`. `Free_PGMSY()` is never called either, and PGMSYfp is never closed.

**Code at issue** — `atPGMSY.c` line 137  
Search for: `Do_Atlantis_PGMSY()`

```c
  135: /********************************************************************************************************
  136:  * \brief  Getting multipliers across fleets
  137→  * Called from Do_Atlantis_PGMSY()
  138:  */
  139: void GetMultipliers(MSEBoxModel *bm, int sp, int this_year, double **Fmatrix) {
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
**Found at (current code):** `atPGMSY.c` 131, `atPGMSY.c` 127, `atPGMSY.c` 165
**Lines at review (Oct 2026):** 55-56, 127, 131, 338, 341

**Why it's thought to be a bug**

`SSB` and `Depleted` are allocated with length `maxage` but indexed by projection year `iYr` (up to ProjYr-1), which overflows when ProjYr > maxage. `WritePGMSYStep()` also reads `Depleted[iYr]` with `iYr = ProjYr`, one past the end even if the arrays were sized ProjYr. Functions: `Initialise_PGMSY()`, `WritePGMSYStep()`, `RunPGMSYProjection()`.

**Code at issue** — `atPGMSY.c` line 131  
Search for: `Depleted[iYr]`

```c
  129:     for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  130:         f = (int)bm->SP_FISHERYprms[sp][nf][assess_nf_id];
  131→         fprintf(fid,"%d %s %d %s %d %e %e %e %e %e %e %e\n", this_year, FunctGroupArray[sp].groupCode, iteration, FisheryArray[nf].fisheryCode, f, bm->RBCestimation.F_actFleet[sp][f], Fmatrix[f][this_year], bm->RBCestimation.FratioFleet[sp][f], bm->RBCestimation.RBCspeciesArray[sp].RBCupdated[nf][this_year], bm->RBCestimation.RBCspeciesArray[sp].CatchStore[f][this_year], ASSESS_PROJECTION[sp].Depleted[1], ASSESS_PROJECTION[sp].Depleted[iYr]);
  132:     }
  133: }
```

**Code at issue** — `atPGMSY.c` line 127 (closest match to the quoted code; wording differs slightly)  
Search for: `int iYr = bm->RBCestimation.ProjYr;`

```c
  125: void WritePGMSYStep(FILE *fid, MSEBoxModel *bm, int sp, int this_year, int iteration, double **Fmatrix) {
  126:     int nf, f;
  127→     int iYr = bm->RBCestimation.ProjYr;
  128:     
  129:     for (nf = 0; nf < bm->K_num_fisheries; nf++) {
```

**Code at issue** — `atPGMSY.c` line 165  
Search for: `RunPGMSYProjection()`

```c
  163: /**==================================================================================
  164:  \brief  Get_Selectivity
  165→     Called from RunPGMSYProjection()
  166:  **/
  167: 
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
**Found at (current code):** `atPGMSY.c` 149, `atPGMSY.c` 153
**Lines at review (Oct 2026):** 146-153

**Function:** `GetMultipliers()`

**Why it's thought to be a bug**

`RBCspeciesParam[sp][AveF_id] += F_actFleet[sp][nf]` is never zeroed first. GetMultipliers is called several times per year (lines 562, 575, 599), so AveF keeps growing and FratioFleet is wrong. `/= nfleets` divides by 0 if no fleet has q>0. Function: `GetMultipliers()`.

**Code at issue** — `atPGMSY.c` line 149 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->RBCestimation.RBCspeciesParam[sp][AveF_id] += bm->RBCestimation.F_actFleet[sp][nf];`

```c
  147:         
  148:         if ( bm->SP_FISHERYprms[sp][nf][q_id] > 0 ) {
  149→             bm->RBCestimation.RBCspeciesParam[sp][AveF_id] += bm->RBCestimation.F_actFleet[sp][nf];
  150:             nfleets += 1.0;
  151:         }
```

**Code at issue** — `atPGMSY.c` line 153  
Search for: `/= nfleets`

```c
  151:         }
  152:     }
  153→     bm->RBCestimation.RBCspeciesParam[sp][AveF_id] /= nfleets;
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
**Found at (current code):** `atPGMSY.c` 146, `atPGMSY.c` 131
**Lines at review (Oct 2026):** 146, 131, 283

**Function:** `GetMultipliers()`

**Why it's thought to be a bug**

GetMultipliers reads `Fmatrix[nf][this_year]` with nf = fishery and stores `F_actFleet[sp][nf]`. RunPGMSYProjection and WritePGMSYStep treat Fmatrix rows and F_actFleet as assessment-fleet indices (`f = SP_FISHERYprms[sp][nf][assess_nf_id]`, `Fmatrix[f]`, `F_actFleet[sp][f]`). Function: `GetMultipliers()`, `WritePGMSYStep()`.

**Code at issue** — `atPGMSY.c` line 146  
Search for: `Fmatrix[nf][this_year]`

```c
  144:     for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  145:         // Find current fishable biomass so can find F per fishery
  146→         bm->RBCestimation.F_actFleet[sp][nf] = Fmatrix[nf][this_year];
  147:         
  148:         if ( bm->SP_FISHERYprms[sp][nf][q_id] > 0 ) {
```

**Code at issue** — `atPGMSY.c` line 131  
Search for: `Fmatrix[f]`

```c
  129:     for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  130:         f = (int)bm->SP_FISHERYprms[sp][nf][assess_nf_id];
  131→         fprintf(fid,"%d %s %d %s %d %e %e %e %e %e %e %e\n", this_year, FunctGroupArray[sp].groupCode, iteration, FisheryArray[nf].fisheryCode, f, bm->RBCestimation.F_actFleet[sp][f], Fmatrix[f][this_year], bm->RBCestimation.FratioFleet[sp][f], bm->RBCestimation.RBCspeciesArray[sp].RBCupdated[nf][this_year], bm->RBCestimation.RBCspeciesArray[sp].CatchStore[f][this_year], ASSESS_PROJECTION[sp].Depleted[1], ASSESS_PROJECTION[sp].Depleted[iYr]);
  132:     }
  133: }
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
**Found at (current code):** `atPGMSY.c` 265, `atPGMSY.c` 165
**Lines at review (Oct 2026):** 264-270

**Function:** `RunPGMSYProjection()`

**Why it's thought to be a bug**

The starting population sets only `age = ageclasssize*cohort`, the first age of each cohort, for DEN/LENGTH/WEIGHT/BIO, although DEN is divided by `age_class_part` (meant to spread over the ages in the cohort). Other ages keep stale values from earlier calls or 0, and get LENGTH=0 for selectivity. maxage is also built from numCohortsXnumGenes, so genotype cohorts are treated as older ages. Function: `RunPGMSYProjection()`.

**Code at issue** — `atPGMSY.c` line 265  
Search for: `age = ageclasssize * cohort`

```c
  263:     // Set up starting population
  264:     for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
  265→         age = ageclasssize * cohort;
  266:         ASSESS_PROJECTION[species].DEN[age][0] = bm->tot_cohort[species][cohort] * age_class_part;
  267:         ASSESS_PROJECTION[species].LENGTH[age] = Length = linf - (linf * exp(-1.0 * Kbert * (age - tzero)));
```

**Code at issue** — `atPGMSY.c` line 165  
Search for: `RunPGMSYProjection()`

```c
  163: /**==================================================================================
  164:  \brief  Get_Selectivity
  165→     Called from RunPGMSYProjection()
  166:  **/
  167: 
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
**Found at (current code):** `atPGMSY.c` 345, `atPGMSY.c` 165
**Lines at review (Oct 2026):** 319-320

**Function:** `RunPGMSYProjection()`

**Why it's thought to be a bug**

`totavailB[nf] += BIO[age][iYr]*sel*q; catch = BIO[age][iYr]*...` but BIO is only ever set for year 0 (line 269). For iYr≥1 it is 0 or stale, so projected catch and available biomass are 0, and `Fout = Catch/totavailB` (345) is 0/0. Function: `RunPGMSYProjection()`.

**Code at issue** — `atPGMSY.c` line 345 (closest match to the quoted code; wording differs slightly)  
Search for: `ASSESS_PROJECTION[species].Fout[nf][iYr] = ASSESS_PROJECTION[species].Catch[nf][iYr] / ASS`

```c
  343:         // Update F
  344:         for (nf = 0; nf < Nfleets; nf++) {
  345→             ASSESS_PROJECTION[species].Fout[nf][iYr] = ASSESS_PROJECTION[species].Catch[nf][iYr] / ASSESS_PROJECTION[species].totavailB[nf];
  346:         }
  347:     }
```

**Code at issue** — `atPGMSY.c` line 165  
Search for: `RunPGMSYProjection()`

```c
  163: /**==================================================================================
  164:  \brief  Get_Selectivity
  165→     Called from RunPGMSYProjection()
  166:  **/
  167: 
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
**Found at (current code):** `atPGMSY.c` 321, `atPGMSY.c` 330, `atPGMSY.c` 165
**Lines at review (Oct 2026):** 290, 321, 330

**Function:** `RunPGMSYProjection()`

**Why it's thought to be a bug**

`totF += Fused[nf]` runs inside the age and cohort loops and is only reset once per year. Mortality `exp(-(nat_mort+totF))` for later ages therefore includes the summed F of every earlier age, and F is not weighted by selectivity. Function: `RunPGMSYProjection()`.

**Code at issue** — `atPGMSY.c` line 321 (closest match to the quoted code; wording differs slightly)  
Search for: `totF += ASSESS_PROJECTION[species].Fused[nf];`

```c
  319:                     ASSESS_PROJECTION[species].totavailB[nf] += ASSESS_PROJECTION[species].BIO[age][iYr] * sel * q;
  320:                     catch = ASSESS_PROJECTION[species].BIO[age][iYr] * sel * q * ASSESS_PROJECTION[species].Fused[nf];
  321→                     totF += ASSESS_PROJECTION[species].Fused[nf];
  322:                     
  323:                     if (catch > ASSESS_PROJECTION[species].RBC[nf])
```

**Code at issue** — `atPGMSY.c` line 330 (closest match to the quoted code; wording differs slightly)  
Search for: `ASSESS_PROJECTION[species].DEN[age][iYr] = ASSESS_PROJECTION[species].DEN[age-1][iYr-1] * `

```c
  328:                 // Final numbers
  329:                 if (age > 0) {
  330→                     ASSESS_PROJECTION[species].DEN[age][iYr] = ASSESS_PROJECTION[species].DEN[age-1][iYr-1] * exp(-1.0 * (nat_mort + totF));
  331:                 } else {
  332:                     // Get info for recruiment
```

**Code at issue** — `atPGMSY.c` line 165  
Search for: `RunPGMSYProjection()`

```c
  163: /**==================================================================================
  164:  \brief  Get_Selectivity
  165→     Called from RunPGMSYProjection()
  166:  **/
  167: 
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
**Found at (current code):** `atPGMSY.c` 340, `atPGMSY.c` 165
**Lines at review (Oct 2026):** 329-334, 338-340

**Function:** `RunPGMSYProjection()`

**Why it's thought to be a bug**

SSB is accumulated only in the `age == 0` branch and uses `DEN[0][iYr]`, which is not set until line 340, after SSB has been used. Mature ages never add to SSB, so recruitment and depletion are wrong. Function: `RunPGMSYProjection()`.

**Code at issue** — `atPGMSY.c` line 340  
Search for: `DEN[0][iYr]`

```c
  338:         ASSESS_PROJECTION[species].SSB[iYr] = SSB;
  339:         recruitNum = BHalpha_sp * SSB / (BHbeta_sp + SSB);
  340→         ASSESS_PROJECTION[species].DEN[0][iYr] = recruitNum;
  341:         ASSESS_PROJECTION[species].Depleted[iYr] = SSB / bm->estBo[species];
  342:         
```

**Code at issue** — `atPGMSY.c` line 165  
Search for: `RunPGMSYProjection()`

```c
  163: /**==================================================================================
  164:  \brief  Get_Selectivity
  165→     Called from RunPGMSYProjection()
  166:  **/
  167: 
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
**Found at (current code):** `atPGMSY.c` 165
**Lines at review (Oct 2026):** 323-325

**Function:** `RunPGMSYProjection()`

**Why it's thought to be a bug**

The catch is capped at the fleet's total RBC per age (`if (catch > RBC[nf]) catch = RBC[nf]`), so the total summed over ages can still exceed the RBC. Function: `RunPGMSYProjection()`.

**Code at issue** — `atPGMSY.c` line 165  
Search for: `RunPGMSYProjection()`

```c
  163: /**==================================================================================
  164:  \brief  Get_Selectivity
  165→     Called from RunPGMSYProjection()
  166:  **/
  167: 
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
**Found at (current code):** `atPGMSY.c` 351, `atPGMSY.c` 363, `atPGMSY.c` 345
**Lines at review (Oct 2026):** 350-367, 278, 532, 675

**Function:** `RunPGMSYProjection()`

**Why it's thought to be a bug**

`RBCupdated[nf][this_year]`, `Fupdated[nf]` and `CatchStore[nf]` are overwritten for `nf < Nfleets` (assessment fleets), but RBCupdated is indexed by fishery elsewhere (278, 532, 675). `ncount` is 0 when `AveYr ≤ 1`, and `Fout[nf][iYr]` overflows when `AveYr > ProjYr`. Function: `RunPGMSYProjection()`.

**Code at issue** — `atPGMSY.c` line 351  
Search for: `RBCupdated[nf][this_year]`

```c
  349:     // Store updated F
  350:     for (nf = 0; nf < Nfleets; nf++) {
  351→         bm->RBCestimation.RBCspeciesArray[species].RBCupdated[nf][this_year] = 0.0;
  352:         AveF = 0.0;
  353:         AveC = 0.0;
```

**Code at issue** — `atPGMSY.c` line 363  
Search for: `Fupdated[nf]`

```c
  361:         AveF /= ncount;
  362:         AveC /= ncount;
  363→         bm->RBCestimation.RBCspeciesArray[species].Fupdated[nf][this_year] = AveF;
  364:         bm->RBCestimation.RBCspeciesArray[species].CatchStore[nf][this_year] = AveC;
  365:         
```

**Code at issue** — `atPGMSY.c` line 345  
Search for: `Fout[nf][iYr]`

```c
  343:         // Update F
  344:         for (nf = 0; nf < Nfleets; nf++) {
  345→             ASSESS_PROJECTION[species].Fout[nf][iYr] = ASSESS_PROJECTION[species].Catch[nf][iYr] / ASSESS_PROJECTION[species].totavailB[nf];
  346:         }
  347:     }
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
**Found at (current code):** `atPGMSY.c` 538, `atPGMSY.c` 402, `atPGMSY.c` 236
**Lines at review (Oct 2026):** 284, 402, 420, 429, 538-545

**Why it's thought to be a bug**

`AvgCatFleet = Util_Alloc_Init_2D_Double(nYears, K_num_fisheries)` gives `[nf][nYears]` (atAssessTierSetup.c:307), but every use is `AvgCatFleet[this_year][nf]` or `[iYr][nf]`. This overflows once the year reaches K_num_fisheries. Functions: `Do_Atlantis_PGMSY()`, `RunPGMSYProjection()`, `RunTier4Projection()`.

**Code at issue** — `atPGMSY.c` line 538  
Search for: `AvgCatFleet[this_year][nf]`

```c
  536:             for (nf = 0; nf < Nfleets; nf++) {
  537:                 // Reinitilise depletion output vector
  538→                 bm->RBCestimation.RBCspeciesArray[sp].AvgCatFleet[this_year][nf] = 0.0;
  539: 
  540:                 for (Iyr = MinCyr; Iyr < MaxCyr; Iyr++ ) {
```

**Code at issue** — `atPGMSY.c` line 402  
Search for: `[iYr][nf]`

```c
  400:         AveF += 1.0 / (1.0 + exp(-1.0 * bm->RBCestimation.RBCspeciesArray[species].FFs[iYr]));
  401:         for (nf = 0; nf < Nfleets; nf++) {
  402→             totcatch += bm->RBCestimation.RBCspeciesArray[species].AvgCatFleet[iYr][nf];
  403:         }
  404:         ncount += 1.0;
```

**Code at issue** — `atPGMSY.c` line 236  
Search for: `Do_Atlantis_PGMSY()`

```c
  234: /**==================================================================================
  235:  \brief  RunPGMSYProgection for PGMSY calculations
  236→     Called from Do_Atlantis_PGMSY()
  237:  **/
  238: 
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
**Found at (current code):** `atPGMSY.c` 402
**Lines at review (Oct 2026):** 399-405, 420, 429

**Function:** `RunTier4Projection()`

**Why it's thought to be a bug**

`AveF` is built from `FFs[1..AveYr)`, but FFs is only allocated (atAssessTierSetup.c:307) and never filled anywhere, so AveF is always 0.5. The comment says "last 10 years", but the loop uses the earliest years. The projected catch for year iYr uses the historical `AvgCatFleet[iYr]` (model year iYr) instead of the current-year fleet split, and `totcatch` sums over years and fleets, which normalises the split wrongly. Function: `RunTier4Projection()`.

**Code at issue** — `atPGMSY.c` line 402  
Search for: `AvgCatFleet[iYr]`

```c
  400:         AveF += 1.0 / (1.0 + exp(-1.0 * bm->RBCestimation.RBCspeciesArray[species].FFs[iYr]));
  401:         for (nf = 0; nf < Nfleets; nf++) {
  402→             totcatch += bm->RBCestimation.RBCspeciesArray[species].AvgCatFleet[iYr][nf];
  403:         }
  404:         ncount += 1.0;
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
**Found at (current code):** `atPGMSY.c` 605, `atPGMSY.c` 376
**Lines at review (Oct 2026):** 605

**Function:** `Do_Atlantis_PGMSY()`

**Why it's thought to be a bug**

`CatchData[f][r][this_year]` uses `r` left over from the loop at 547-549, so it equals NumRegions of the last fished species. CatchData is `[nf][K_num_reg+1][nYears]` (atAssessTierSetup.c:115), so this reads the wrong region or out of bounds. Function: `Do_Atlantis_PGMSY()`.

**Code at issue** — `atPGMSY.c` line 605  
Search for: `CatchData[f][r][this_year]`

```c
  603:                 for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  604:                     f = (int)bm->SP_FISHERYprms[sp][nf][assess_nf_id];
  605→                     bm->RBCestimation.RBCspeciesArray[sp].CscalarMetier[nf] = bm->RBCestimation.RBCspeciesArray[sp].CatchStore[f][this_year] / (bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][r][this_year] + small_num);
  606:                 
  607:                     bm->RBCestimation.RBCspeciesArray[sp].RBCupdated[nf][this_year] = bm->RBCestimation.RBCspeciesArray[sp].CscalarMetier[f] * bm->TACamt[sp][nf][now_id];
```

**Code at issue** — `atPGMSY.c` line 376  
Search for: `Do_Atlantis_PGMSY()`

```c
  374: /**==================================================================================
  375:  \brief  RunPGMSYProjection for PGMSY calculations
  376→     Called from Do_Atlantis_PGMSY()
  377:  **/
  378: 
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
**Found at (current code):** `atPGMSY.c` 605, `atPGMSY.c` 376
**Lines at review (Oct 2026):** 605-607

**Function:** `Do_Atlantis_PGMSY()`

**Why it's thought to be a bug**

`CscalarMetier[nf]` is computed and then read as `CscalarMetier[f]` (f = assessment fleet) to set `RBCupdated[nf]`. Function: `Do_Atlantis_PGMSY()`.

**Code at issue** — `atPGMSY.c` line 605  
Search for: `CscalarMetier[nf]`

```c
  603:                 for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  604:                     f = (int)bm->SP_FISHERYprms[sp][nf][assess_nf_id];
  605→                     bm->RBCestimation.RBCspeciesArray[sp].CscalarMetier[nf] = bm->RBCestimation.RBCspeciesArray[sp].CatchStore[f][this_year] / (bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][r][this_year] + small_num);
  606:                 
  607:                     bm->RBCestimation.RBCspeciesArray[sp].RBCupdated[nf][this_year] = bm->RBCestimation.RBCspeciesArray[sp].CscalarMetier[f] * bm->TACamt[sp][nf][now_id];
```

**Code at issue** — `atPGMSY.c` line 376  
Search for: `Do_Atlantis_PGMSY()`

```c
  374: /**==================================================================================
  375:  \brief  RunPGMSYProjection for PGMSY calculations
  376→     Called from Do_Atlantis_PGMSY()
  377:  **/
  378: 
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
**Found at (current code):** `atPGMSY.c` 671, `atPGMSY.c` 654, `atPGMSY.c` 131
**Lines at review (Oct 2026):** 652-659

**Function:** `Do_Atlantis_PGMSY()`

**Why it's thought to be a bug**

The F-rescaling block sits after the `for (sp...)` loop, so `sp == K_num_tot_sp`. `RBCspeciesParam[sp]`, `RBCspeciesArray[sp]` and `ASSESS_PROJECTION[sp]` (whose extra entry is uninitialised) are read out of range, and no real species gets rescaled. Function: `Do_Atlantis_PGMSY()`.

**Code at issue** — `atPGMSY.c` line 671  
Search for: `for (sp`

```c
  669:     
  670:     // Store updated RCBs and TACs
  671→     for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  672:         if (FunctGroupArray[sp].isFished == TRUE) {  // TODO: Could this be made to be isImpacted?
  673:             TAC = 0.0;
```

**Code at issue** — `atPGMSY.c` line 654  
Search for: `RBCspeciesParam[sp]`

```c
  652:                 MultBest = (MultMax + MultMin) / 2.0;
  653:                 
  654→                 Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]);
  655:                 for (nf = 0; nf < Nfleets; nf++) {
  656:                     bm->RBCestimation.RBCspeciesArray[sp].Fupdated[nf][this_year] *= MultBest;
```

**Code at issue** — `atPGMSY.c` line 131  
Search for: `ASSESS_PROJECTION[sp]`

```c
  129:     for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  130:         f = (int)bm->SP_FISHERYprms[sp][nf][assess_nf_id];
  131→         fprintf(fid,"%d %s %d %s %d %e %e %e %e %e %e %e\n", this_year, FunctGroupArray[sp].groupCode, iteration, FisheryArray[nf].fisheryCode, f, bm->RBCestimation.F_actFleet[sp][f], Fmatrix[f][this_year], bm->RBCestimation.FratioFleet[sp][f], bm->RBCestimation.RBCspeciesArray[sp].RBCupdated[nf][this_year], bm->RBCestimation.RBCspeciesArray[sp].CatchStore[f][this_year], ASSESS_PROJECTION[sp].Depleted[1], ASSESS_PROJECTION[sp].Depleted[iYr]);
  132:     }
  133: }
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
**Found at (current code):** `atPGMSY.c` 656, `atPGMSY.c` 376
**Lines at review (Oct 2026):** 652-659, 363

**Function:** `Do_Atlantis_PGMSY()`

**Why it's thought to be a bug**

`Fupdated *= MultBest` compounds across iterations, but bisection on [MultMin, MultMax] assumes an absolute multiplier on a fixed base F. RunPGMSYProjection then overwrites Fupdated[nf][this_year] with AveF (line 363), so the multiplier is lost. Function: `Do_Atlantis_PGMSY()`.

**Code at issue** — `atPGMSY.c` line 656 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->RBCestimation.RBCspeciesArray[sp].Fupdated[nf][this_year] *= MultBest;`

```c
  654:                 Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]);
  655:                 for (nf = 0; nf < Nfleets; nf++) {
  656→                     bm->RBCestimation.RBCspeciesArray[sp].Fupdated[nf][this_year] *= MultBest;
  657:                 }
  658:                 Iteration += 1;
```

**Code at issue** — `atPGMSY.c` line 376  
Search for: `Do_Atlantis_PGMSY()`

```c
  374: /**==================================================================================
  375:  \brief  RunPGMSYProjection for PGMSY calculations
  376→     Called from Do_Atlantis_PGMSY()
  377:  **/
  378: 
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
**Found at (current code):** `atPGMSY.c` 463, `atPGMSY.c` 542, `atPGMSY.c` 376
**Lines at review (Oct 2026):** 463, 540-542

**Function:** `Do_Atlantis_PGMSY()`

**Why it's thought to be a bug**

`MinCyr = this_year - TriggerCheckPeriod` is not clamped, so early in a run `CatchData[nf][r][Iyr]` is read with Iyr<0. `Call_PGMSY` only checks `earliestYr > year`. Function: `Do_Atlantis_PGMSY()`.

**Code at issue** — `atPGMSY.c` line 463 (closest match to the quoted code; wording differs slightly)  
Search for: `int MinCyr = this_year - bm->RBCestimation.TriggerCheckPeriod;`

```c
  461:     double TAC = 0.0;
  462: 
  463→     int MinCyr = this_year - bm->RBCestimation.TriggerCheckPeriod;
  464:     int MaxCyr = this_year;
  465:     double numYear = (double)(MaxCyr - MinCyr);
```

**Code at issue** — `atPGMSY.c` line 542  
Search for: `CatchData[nf][r][Iyr]`

```c
  540:                 for (Iyr = MinCyr; Iyr < MaxCyr; Iyr++ ) {
  541:                     for (r = 0; r < bm->RBCestimation.RBCspeciesParam[sp][NumRegions_id]; r++) {
  542→                         bm->RBCestimation.RBCspeciesArray[sp].AvgCatFleet[this_year][nf] += bm->RBCestimation.RBCspeciesArray[sp].CatchData[nf][r][Iyr];
  543:                     }
  544:                 }
```

**Code at issue** — `atPGMSY.c` line 376  
Search for: `Do_Atlantis_PGMSY()`

```c
  374: /**==================================================================================
  375:  \brief  RunPGMSYProjection for PGMSY calculations
  376→     Called from Do_Atlantis_PGMSY()
  377:  **/
  378: 
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
**Found at (current code):** `atPGMSY.c` 102
**Lines at review (Oct 2026):** 114, 131

**Why it's thought to be a bug**

The header has 11 column names, but each row prints 12 values (CatchStore is printed with no header column). Functions: `Init_PGMSY_File()`, `WritePGMSYStep()`.

**Code at issue** — `atPGMSY.c` line 102 (located by name only; check this is the right place)  
Search for: `static FILE * Init_PGMSY_File(MSEBoxModel *bm) {`

```c
  100:  *
  101:  */
  102→ static FILE * Init_PGMSY_File(MSEBoxModel *bm) {
  103:     FILE *fid;
  104:     char fname[STRLEN];
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
