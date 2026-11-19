# atassess/atAssessTierSetup.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **13 findings** (severity H: 3; confidence H and severity H: 2; on the priority list: 2).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [ATS-01](#ats-01) ✔ ⭐ | 136 | H/H | swapped index | `AgeYears = Util_Alloc_Init_2D_Int(3, nYears, 0)` gives `[nYears][3]`. Used as `AgeYears[it][iyr]` (it<3, iyr<… | B | [x] |
| [ATS-02](#ats-02) ✔ ⭐ | 456 | H/H | swapped index | `agesel_bysex`/`fval = Util_Alloc_Init_2D_Double(K_num_sexes, numCohorts)` gives `[numCohorts][sexes]`. Used `… | A | [x] |
| [ATS-03](#ats-03) | 63 | H/L | alloc/free mismatch | Close-kin, SlopeMat, FracLenS, Fupdated, FFs etc. are allocated for `assess_flag \|\| isImpacted`, but freed onl… | fixed (Beth) | [x] |
| [ATS-04](#ats-04) | 583 | M/M | wrong variable | Free is gated on `isFished` but allocation is gated on `isImpacted`. `RBCspeciesArray` is malloc'd (not calloc… | fixed (Beth) | [x] |
| [ATS-05](#ats-05) | 1455 | M/H | wrong operator | `isTAC \|\| (TACamt < no_quota)` sums the `no_quota` sentinel into `TACinitial`/`RBC_old`. Elsewhere this test i… | C | [x] |
| [ATS-06](#ats-06) | 1085 | H/M | off-by-one | `thisfval = i/nfintt` at the end of iteration i, so F=0 is evaluated twice and every step lags by one. F20/40/… | fixed (Beth) | [x] |
| [ATS-07](#ats-07) | 1418 | M/M | wrong variable | `metierArray[mid].metierCode` is copied from `FisheryArray[m]` (assessment fleet index) rather than `FisheryAr… | C | [x] |
| [ATS-08](#ats-08) | 53, 1416 | M/M | uninitialised mapping | `speciesRPFleetToMetier` is initialised to 0 and never populated anywhere, so every fleet maps to metier 0. at… | A (option b) | [x] |
| [ATS-09](#ats-09) | 315 | H/L | memory leak | Never freed: AgeComp, AFss, LenComp, Xmove, DiscardType, Blocks, Obsageprop, MeanLenAgeS, T5_removal, TAC_mult… | A | [x] |
| [ATS-10](#ats-10) | 1472 | H/L | printf mismatch | `%d` used for a `double` (`RBCspeciesParam[..][NumFisheries_id]`). | A | [x] |
| [ATS-11](#ats-11) | 1545 | L/L | logic | `Lastsp` is only incremented when >0, so if the last assessed group is index 0 it's skipped by `sp < Lastsp` l… | C | [x] |
| [ATS-12](#ats-12) | 928 | L/L | div-by-zero / OOB | `siglen` ≤ 0 when `len[a]` ≤ 0 gives inf/NaN; `YPRlen` with MaxAge=1 indexes `[-1]`. | B | [x] |
| [ATS-13](#ats-13) | 1157 | L/L | off-by-one | Loop sets `LengthN[0..2]` but array has 4 entries, so `LengthN[3]` stays 0. | D → no change | [x] |

---

## ATS-01

**swapped index** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Found at (current code):** `atAssessTierSetup.c` 136
**Lines at review (Oct 2026):** 136

**Why it's thought to be a bug**

`AgeYears = Util_Alloc_Init_2D_Int(3, nYears, 0)` gives `[nYears][3]`. Used as `AgeYears[it][iyr]` (it<3, iyr<nYears) in atSS3DataGen.c:159, 838, so any year ≥3 overruns. `LengthYears` on line 167 is correct: `(nYears,4)`.

**Code at issue** — `atAssessTierSetup.c` line 136  
Search for: `AgeYears = Util_Alloc_Init_2D_Int(3, nYears, 0)`

```c
  134:         bm->RBCestimation.RBCspeciesArray[groupIndex].AgeFuture = Util_Alloc_Init_1D_Int(3, 0);          /* Flag for  future age  data (retained, whole, discard) */
  135:         bm->RBCestimation.RBCspeciesArray[groupIndex].AgeN = Util_Alloc_Init_1D_Int(3, 0);               /* Sample size of age comps  (retained, whole, discard) */
  136→ 		bm->RBCestimation.RBCspeciesArray[groupIndex].AgeYears = Util_Alloc_Init_2D_Int(3, nYears, 0);          /* Flags which years for which to generate age comps (retained, whole, discard). Gives [nYears][3] so index as AgeYears[iyr][it] */
  137: 		bm->RBCestimation.RBCspeciesArray[groupIndex].AgeFltYr = Util_Alloc_Init_3D_Int(nYears, bm->K_num_fisheries, 3, 0);         /* Flags AF years, fleets in past (r,w,d) */
  138: 		bm->RBCestimation.RBCspeciesArray[groupIndex].AgeComp = Util_Alloc_Init_5D_Int(bm->RBCestimation.OverallMaxAge, 4, nYears, bm->K_num_sexes, bm->K_num_fisheries, 0);        /* Generated age composition data */
```

**Suggested fix (from review):** `Util_Alloc_Init_2D_Int(nYears, 3, 0)`

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [Yes] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> Swap the AgeYears[it][iyr] tp AgeYears[iyr][it]

**Outcome:** 2026-11-19, batch 002. Fixed as you asked: the allocation is unchanged (`Util_Alloc_Init_2D_Int(3, nYears, 0)` gives `[nYears][3]`), and both uses in `atSS3DataGen.c` now read `AgeYears[iyr][it]` (search `AgeYears[iyr][it]`). A comment on the allocation records the index order.

---

## ATS-02

**swapped index** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Found at (current code):** `atAssessTierSetup.c` 456
**Lines at review (Oct 2026):** 448, 451

**Why it's thought to be a bug**

`agesel_bysex`/`fval = Util_Alloc_Init_2D_Double(K_num_sexes, numCohorts)` gives `[numCohorts][sexes]`. Used `[s][a]` with a up to numCohorts-1 in atManageTier.c:1048-1070, 1159.

**Code at issue** — `atAssessTierSetup.c` line 456 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->RBCestimation.RBCspeciesArray[groupIndex].fval = Util_Alloc_Init_2D_Double(FunctGroupA`

```c
  454: 
  455:         if (tier == tier1) { // As only used in tier1 routines
  456→             bm->RBCestimation.RBCspeciesArray[groupIndex].fval = Util_Alloc_Init_2D_Double(FunctGroupArray[groupIndex].numCohorts, bm->K_num_sexes, 0.0);
  457:         }
  458:         
```

**Suggested fix (from review):** swap to `(numCohorts, K_num_sexes)`

**Your response** (tick one; add detail in comments)
- [Yes] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome:** 2026-11-19, batch 002. `agesel_bysex` and `fval` are now allocated with `Util_Alloc_Init_2D_Double(numCohorts, K_num_sexes, ...)`, giving `[K_num_sexes][numCohorts]` to match the `[s][a]` use. Also needed: the two resets in `atManageTier.c` (`Util_Init_2D_Double(...agesel_bysex/fval, ...)`) looped `[numCohorts][sexes]`, so they were swapped to `(K_num_sexes, numCohorts)`; otherwise they would overrun the corrected arrays.

---

## ATS-03

**alloc/free mismatch** · Conf H / Sev L  
**Found at (current code):** `atAssessTierSetup.c` 63
**Lines at review (Oct 2026):** 270-307 vs 724-876

**Why it's thought to be a bug**

Close-kin, SlopeMat, FracLenS, Fupdated, FFs etc. are allocated for `assess_flag || isImpacted`, but freed only after the `continue` at line 729 (tiered + assessed only), so they leak.

**Code at issue** — `atAssessTierSetup.c` line 63 (closest match to the quoted code; wording differs slightly)  
Search for: `if ((FunctGroupArray[groupIndex].speciesParams[assess_flag_id] == FALSE) && (FunctGroupArr`

```c
   61:         printf("Doing Tier_Assessment_Allocate for %s\n", FunctGroupArray[groupIndex].groupCode);
   62:         
   63→         if ((FunctGroupArray[groupIndex].speciesParams[assess_flag_id] == FALSE) && (FunctGroupArray[groupIndex].isImpacted == FALSE))
   64:             continue;
   65:                 
```

**Suggested fix (from review):** move these frees above line 729

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [X] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> I already fixed it so no longer an issue

**Outcome:** 2026-11-19. Already fixed by Beth: the `continue` before these frees is commented out, so they now run for every allocated group. Included in batch 002.

---

## ATS-04

**wrong variable** · Conf M / Sev M  
**Found at (current code):** `atAssessTierSetup.c` 583
**Lines at review (Oct 2026):** 577 vs 63

**Why it's thought to be a bug**

Free is gated on `isFished` but allocation is gated on `isImpacted`. `RBCspeciesArray` is malloc'd (not calloc'd), so a fished-not-impacted group frees garbage pointers.

**Code at issue** — `atAssessTierSetup.c` line 583 (located by name only; check this is the right place)  
Search for: `if ((FunctGroupArray[groupIndex].speciesParams[assess_flag_id] == FALSE) && (FunctGroupArr`

```c
  581:         tier = (int) (FunctGroupArray[groupIndex].speciesParams[tier_id]);
  582:         
  583→         if ((FunctGroupArray[groupIndex].speciesParams[assess_flag_id] == FALSE) && (FunctGroupArray[groupIndex].isImpacted == FALSE))
  584:             continue;
  585:         
```

**Suggested fix (from review):** use `isImpacted` on line 582; calloc the struct arrays

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [Yes] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> Fixed already

**Outcome:** 2026-11-19. Already fixed by Beth: the free is now gated on `isImpacted`, matching the allocation. Included in batch 002.

---

## ATS-05

**wrong operator** · Conf M / Sev H  
**Found at (current code):** `atAssessTierSetup.c` 1455
**Lines at review (Oct 2026):** 1439

**Why it's thought to be a bug**

`isTAC || (TACamt < no_quota)` sums the `no_quota` sentinel into `TACinitial`/`RBC_old`. Elsewhere this test is `isTAC && ...` or `!isTAC || ...`.

**Code at issue** — `atAssessTierSetup.c` line 1455 (closest match to the quoted code; wording differs slightly)  
Search for: `if(FunctGroupArray[groupIndex].isTAC || (bm->TACamt[groupIndex][n][now_id] < no_quota)) {`

```c
 1453: 		totTAC = 0.0;
 1454: 		for (n = 0; n < bm->K_num_fisheries; n++){
 1455→             if(FunctGroupArray[groupIndex].isTAC || (bm->TACamt[groupIndex][n][now_id] < no_quota)) {
 1456:                 totTAC += bm->TACamt[groupIndex][n][now_id];
 1457:             }
```

**Suggested fix (from review):** `isTAC && (TACamt < no_quota)`

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [Yes] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome:** 2026-11-20. Left as is, per your decision.

---

## ATS-06

**off-by-one** · Conf H / Sev M  
**Found at (current code):** `atAssessTierSetup.c` 1085
**Lines at review (Oct 2026):** 1050-1073

**Why it's thought to be a bug**

`thisfval = i/nfintt` at the end of iteration i, so F=0 is evaluated twice and every step lags by one. F20/40/48 come out 1/nfint low and maxF is never reached.

**Code at issue** — `atAssessTierSetup.c` line 1085  
Search for: `thisfval = i / nfintt`

```c
 1083: 		if (spbf > (spbf0 * 0.48)) bm->RBCestimation.RBCspeciesParam[species][F48_id] = thisfval;
 1084: 
 1085→ 		thisfval = i / nfintt;
 1086: 	}
 1087: 
```

**Suggested fix (from review):** `thisfval = (i+1)/nfintt;`

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [X] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> Already fixed

**Outcome:** 2026-11-20. Already fixed by Beth. Your line `thisfval = double((i + 1)) / nfintt;` used C++ cast syntax, which the C compiler rejects (the build failed). Claude changed it to `thisfval = (double)(i + 1) / nfintt;` (same meaning). Batch 002.

---

## ATS-07

**wrong variable** · Conf M / Sev M  
**Found at (current code):** `atAssessTierSetup.c` 1418
**Lines at review (Oct 2026):** 1406

**Why it's thought to be a bug**

`metierArray[mid].metierCode` is copied from `FisheryArray[m]` (assessment fleet index) rather than `FisheryArray[mid]`.

**Code at issue** — `atAssessTierSetup.c` line 1418  
Search for: `metierArray[mid].metierCode`

```c
 1416:                 mid = bm->RBCestimation.speciesRPFleetToMetier[m][groupIndex];  // metier for this fishery
 1417:                 bm->RBCestimation.speciesMetierToRPFleet[groupIndex][mid] = m;
 1418→                 strcpy(bm->RBCestimation.metierArray[mid].metierCode, FisheryArray[m].fisheryCode);
 1419:                 bm->RBCestimation.metierArray[mid].PGMSYlinks = PGMSYLinksInputs[mid];
 1420:             }
```

**Suggested fix (from review):** use `mid`

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [X] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> I looked at the code and I think this is right as written as the array is findng a fleet code for a species-fleet mapping. We may need to revisit in future.

**Outcome:** 2026-11-20. Left as is, per your decision. Note from the ATS-08 investigation: if `speciesRPFleetToMetier` is populated as proposed there, `mid` becomes an Atlantis fishery index. Copying `FisheryArray[m]` (the assessment fleet index) would then label metier `mid` with another fishery's code, so this line should be revisited when ATS-08 is decided.

---

## ATS-08

**uninitialised mapping** · Conf M / Sev M  
**Found at (current code):** `atAssessTierSetup.c` 53, `atAssessTierSetup.c` 1416
**Lines at review (Oct 2026):** 53, 1404, 1412

**Why it's thought to be a bug**

`speciesRPFleetToMetier` is initialised to 0 and never populated anywhere, so every fleet maps to metier 0. atSS3LinkIO tests `> -1`, which is always true.

**Code at issue** — `atAssessTierSetup.c` line 53 (the line the review pointed to, found by its text)  
Search for: `bm->RBCestimation.speciesRPFleetToMetier = Util_Alloc_Init_2D_Int(bm->K_num_tot_sp, bm->K_`

```c
   51:     
   52:     bm->RBCestimation.speciesMetierToRPFleet = Util_Alloc_Init_2D_Int(bm->K_num_fisheries, bm->K_num_tot_sp, 0);
   53→     bm->RBCestimation.speciesRPFleetToMetier = Util_Alloc_Init_2D_Int(bm->K_num_tot_sp, bm->K_num_fisheries, 0);
   54:     bm->RBCestimation.metierArray = (METarrays *) malloc(sizeof(METarrays) * (unsigned long int)bm->K_num_fisheries);
   55:     
```

**Code at issue** — `atAssessTierSetup.c` line 1416 (the line the review pointed to, found by its text)  
Search for: `mid = bm->RBCestimation.speciesRPFleetToMetier[m][groupIndex];  // metier for this fishery`

```c
 1414:         if (bm->useMultispAssess) {
 1415:             for (m = 0; m < Nfleet; m++) {
 1416→                 mid = bm->RBCestimation.speciesRPFleetToMetier[m][groupIndex];  // metier for this fishery
 1417:                 bm->RBCestimation.speciesMetierToRPFleet[groupIndex][mid] = m;
 1418:                 strcpy(bm->RBCestimation.metierArray[mid].metierCode, FisheryArray[m].fisheryCode);
```

**Suggested fix (from review):** init to -1 and populate (check for lost read code)

**Your response** (tick one; add detail in comments)
- [Yes] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> Please use option (b) of your proposed fix

**Outcome:** 2026-11-20. Fixed with option (b), per your decision. Batch 003.

- **Initialised to -1:** both `speciesMetierToRPFleet` and `speciesRPFleetToMetier` now start at -1 (no mapping), not 0. Search `initialise to -1 (no mapping)`.
- **Filled from `assess_nf_`:** this happens in `Tier_Assessment_Setup`, just before the `if (bm->useMultispAssess)` loop. Search `populate the metier (Atlantis fishery) <-> assessment fleet mapping`.
  - `speciesMetierToRPFleet[sp][nf] = assess_nf[sp][nf]` for every fishery.
  - `speciesRPFleetToMetier[f][sp]` is the **first fishery mapped to fleet `f` that actually catches the species**. "Catches" means `FISHERYtarget[nf][sp]`, `q > 0`, or `mFC > 0`. If no fishery in fleet `f` catches the species, the entry stays -1.
  - Fleet values outside 0..K_num_fisheries-1 are skipped.
- **Guards against -1:**
  - In the `useMultispAssess` loop, fleets with no metier are skipped.
  - In the `LastMetier` loop, -1 is ignored. It already was in effect, but the check is now explicit.
  - In atRlinkRBC.c, `Catch_by_Metier[...][nf]` is skipped for -1. Search `nowhere to store its catch`.
- **SS3 writers:** the atSS3LinkIO.c tests (`speciesRPFleetToMetier[...] > -1`) now mean something, but they only choose between two comment labels, so the SS3 file layout is unchanged.
- **Knock-ons to note, not changed:**
  - **ATS-07 line:** the line you kept (`strcpy(metierArray[mid].metierCode, FisheryArray[m].fisheryCode)`) now gives metier `mid` the code of fishery `m`, where `m` is a fleet number. The SS3 writer comments print `FisheryArray[fleet]` in the same way.
  - **R input file:** `speciesRPFleetToMetier` is written to it ("Which metiers related to which fleets"), so the R side will now see real fishery indices and possibly -1, where before it saw all 0.
  - **`[sp+1]` index:** atSS3LinkIO.c indexes `speciesRPFleetToMetier[f][sp+1]` in two places. That is already logged as SS3IO-27 in atSS3LinkIO.c.md.

Built clean in a scratch copy; not run.

---

## ATS-09

**memory leak** · Conf H / Sev L  
**Found at (current code):** `atAssessTierSetup.c` 315
**Lines at review (Oct 2026):** 561-899

**Why it's thought to be a bug**

Never freed: AgeComp, AFss, LenComp, Xmove, DiscardType, Blocks, Obsageprop, MeanLenAgeS, T5_removal, TAC_mult, Tier3_Linf/k/t0, Tier5_Bstart, Tier5_r, MULTISPestIDs, the CloseKinEst array.

**Code at issue** — `atAssessTierSetup.c` line 315 (located by name only; check this is the right place)  
Search for: `bm->RBCestimation.MULTISPestIDs = Util_Alloc_Init_1D_Int(bm->K_num_tot_sp, 0);`

```c
  313: 	}
  314:     
  315→     bm->RBCestimation.MULTISPestIDs = Util_Alloc_Init_1D_Int(bm->K_num_tot_sp, 0);
  316:     bm->RBCestimation.Catch_by_Metier = Util_Alloc_Init_4D_Double(bm->K_num_fisheries, maxNYears, maxNYears, bm->K_num_tot_sp, 0.0);
  317: 
```

**Suggested fix (from review):** add matching frees

**Your response** (tick one; add detail in comments)
- [Yes] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome:** 2026-11-20, batch 002. Added the missing frees at the end of the species loop in `Tier_Assessment_Free` (search `never freed (ATS-09)`):
- `Xmove`, `DiscardType`, `AgeComp`, `AFss`, `LenComp`, `Tier3_Linf`/`k`/`t0`, `Tier5_Bstart`, `Tier5_r`, `T5_removal`, `TAC_mult`, `Obsageprop`, `MeanLenAgeS` and `Blocks`;
- after the loop, `MULTISPestIDs` and the `bm->CloseKinEst` struct array (its member arrays were already freed per species).

All of these are allocated under the same test as the free loop (your ATS-04 fix), so no extra conditions are needed. `AgeComp` and `LenComp` are 5-D int arrays built on `alloc5d`; there is no `i_free5d`, so they are released with `free5d`.

---

## ATS-10

**printf mismatch** · Conf H / Sev L  
**Found at (current code):** `atAssessTierSetup.c` 1472
**Lines at review (Oct 2026):** 1454

**Why it's thought to be a bug**

`%d` used for a `double` (`RBCspeciesParam[..][NumFisheries_id]`).

**Code at issue** — `atAssessTierSetup.c` line 1472 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id] = tot_nf;`

```c
 1470:             quit("max number of fisheries from assess_nf (%d) exceeds defined NumFisheries (%d), please correct in assess.prm\n", tot_nf, bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]);
 1471:         } else {
 1472→             bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id] = tot_nf;
 1473:             // To save iteration space
 1474:         }
```

**Suggested fix (from review):** cast to `(int)`

**Your response** (tick one; add detail in comments)
- [Yes] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome:** 2026-11-20, batch 002. The `quit()` message now casts `NumFisheries` with `(int)` (search `exceeds defined NumFisheries`).

---

## ATS-11

**logic** · Conf L / Sev L  
**Found at (current code):** `atAssessTierSetup.c` 1545
**Lines at review (Oct 2026):** 1528-1530

**Why it's thought to be a bug**

`Lastsp` is only incremented when >0, so if the last assessed group is index 0 it's skipped by `sp < Lastsp` loops; also never reset.

**Code at issue** — `atAssessTierSetup.c` line 1545 (located by name only; check this is the right place)  
Search for: `bm->RBCestimation.Lastsp++;  // So don't cut off the final species`

```c
 1543:     
 1544:     if (bm->RBCestimation.Lastsp > 0) {
 1545→         bm->RBCestimation.Lastsp++;  // So don't cut off the final species
 1546:     }
 1547:     
```

**Suggested fix (from review):** review

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [Yes] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome:** 2026-11-20. Left as is, per your decision.

---

## ATS-12

**div-by-zero / OOB** · Conf L / Sev L  
**Found at (current code):** `atAssessTierSetup.c` 928
**Lines at review (Oct 2026):** 917; 954, 970

**Why it's thought to be a bug**

`siglen` ≤ 0 when `len[a]` ≤ 0 gives inf/NaN; `YPRlen` with MaxAge=1 indexes `[-1]`.

**Code at issue** — `atAssessTierSetup.c` line 928  
Search for: `len[a]`

```c
  926: 	// calc proportion of age a in length bin l
  927: 	for (a = 0; a < bm->RBCestimation.RBCspeciesParam[species][MaxAge_id]; a++) {
  928→ 		meanlen = len[a];
  929: 		siglen = bm->RBCestimation.RBCspeciesParam[species][Tier5_cv_id] * meanlen;
  930: 		xlim = (bm->RBCestimation.RBCspeciesArray[species].LoLenBin[1] - meanlen) / siglen;
```

**Suggested fix (from review):** guard

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [Yes] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> Either fix as you recommend or use the small_num guard used elsewhere in the code if appropriate.

**Outcome:** 2026-11-20, batch 002. Two guards were added:
- `Prop_LatA`: `if (siglen < small_num) siglen = small_num;`. If `len[a]` ≤ 0 or the CV is 0, all of age `a` now goes into the length bin containing `meanlen`, instead of producing inf/NaN.
- `YPRlen`: the plus-group line now runs only `if (maxage > 0)`. With `MaxAge = 1` there is one age class with `num[0] = 1`, rather than a read of `num[-1]`.

---

## ATS-13

**off-by-one** · Conf L / Sev L  
**Found at (current code):** `atAssessTierSetup.c` 1157
**Lines at review (Oct 2026):** 1143

**Why it's thought to be a bug**

Loop sets `LengthN[0..2]` but array has 4 entries, so `LengthN[3]` stays 0.

**Code at issue** — `atAssessTierSetup.c` line 1157 (located by name only; check this is the right place)  
Search for: `bm->RBCestimation.RBCspeciesArray[species].LengthN[j] = ssize;`

```c
 1155:     for (j=0; j < 3; j++){
 1156:         bm->RBCestimation.RBCspeciesArray[species].AgeN[j] = ssize;
 1157→         bm->RBCestimation.RBCspeciesArray[species].LengthN[j] = ssize;
 1158:         
 1159:         fprintf(bm->logFile, "%s %d has samplesize %d (AgeN: %d, LengthN: %d\n", FunctGroupArray [species].groupCode, j, ssize, bm->RBCestimation.RBCspeciesArray[species].AgeN[j], bm->RBCestimation.RBCspeciesArray[species].LengthN[j]);
```

**Suggested fix (from review):** check intent

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [Yes] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> I think it might be ok to change to 3 but so many otehrs have 4 in that section that it might be that some aggregate values are being stored in 4 for most of these variables so for consistency sake it might need to stay 4 in the initial allocation

**Outcome:** 2026-11-20. Investigated: **not a bug, and no change recommended.**

- `LengthN` holds one sample size per data type (retained, whole, discard). Every read and write uses a 0–2 index: the loop here, the `for (it = 0; it < 3; it++)` loops in atSS3DataGen.c, and `FindRawData`, which is only called with those `it` values.
- Slot 3 is never read or written by any code, so no aggregate is stored there. The same applies to the other 4-slot length arrays (`LengthFuture`, `LengthYears`, `LengthFltYr`) and to the type dimension of `LenComp`, `AgeComp`, `LFss` and `AFss`. All the SS3 writers loop `it < 3`.
- So the 4th slot is just unused padding, and leaving `LengthN[3] = 0` is harmless. Keeping the allocation at 4, as you suggested, keeps it consistent with its siblings.

---
