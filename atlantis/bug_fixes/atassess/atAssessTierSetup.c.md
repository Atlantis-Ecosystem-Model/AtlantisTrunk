# atassess/atAssessTierSetup.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **13 findings** (severity H: 3; confidence H and severity H: 2; on the priority list: 2).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [ATS-01](#ats-01) ✔ ⭐ | 136 | H/H | swapped index | `AgeYears = Util_Alloc_Init_2D_Int(3, nYears, 0)` gives `[nYears][3]`. Used as `AgeYears[it][iyr]` (it<3, iyr<… |  | [ ] |
| [ATS-02](#ats-02) ✔ ⭐ | 448, 451 | H/H | swapped index | `agesel_bysex`/`fval = Util_Alloc_Init_2D_Double(K_num_sexes, numCohorts)` gives `[numCohorts][sexes]`. Used `… |  | [ ] |
| [ATS-03](#ats-03) | 270-307 vs 724-876 | H/L | alloc/free mismatch | Close-kin, SlopeMat, FracLenS, Fupdated, FFs etc. are allocated for `assess_flag \|\| isImpacted`, but freed onl… |  | [ ] |
| [ATS-04](#ats-04) | 577 vs 63 | M/M | wrong variable | Free is gated on `isFished` but allocation is gated on `isImpacted`. `RBCspeciesArray` is malloc'd (not calloc… |  | [ ] |
| [ATS-05](#ats-05) | 1439 | M/H | wrong operator | `isTAC \|\| (TACamt < no_quota)` sums the `no_quota` sentinel into `TACinitial`/`RBC_old`. Elsewhere this test i… |  | [ ] |
| [ATS-06](#ats-06) | 1050-1073 | H/M | off-by-one | `thisfval = i/nfintt` at the end of iteration i, so F=0 is evaluated twice and every step lags by one. F20/40/… |  | [ ] |
| [ATS-07](#ats-07) | 1406 | M/M | wrong variable | `metierArray[mid].metierCode` is copied from `FisheryArray[m]` (assessment fleet index) rather than `FisheryAr… |  | [ ] |
| [ATS-08](#ats-08) | 53, 1404, 1412 | M/M | uninitialised mapping | `speciesRPFleetToMetier` is initialised to 0 and never populated anywhere, so every fleet maps to metier 0. at… |  | [ ] |
| [ATS-09](#ats-09) | 561-899 | H/L | memory leak | Never freed: AgeComp, AFss, LenComp, Xmove, DiscardType, Blocks, Obsageprop, MeanLenAgeS, T5_removal, TAC_mult… |  | [ ] |
| [ATS-10](#ats-10) | 1454 | H/L | printf mismatch | `%d` used for a `double` (`RBCspeciesParam[..][NumFisheries_id]`). |  | [ ] |
| [ATS-11](#ats-11) | 1528-1530 | L/L | logic | `Lastsp` is only incremented when >0, so if the last assessed group is index 0 it's skipped by `sp < Lastsp` l… |  | [ ] |
| [ATS-12](#ats-12) | 917; 954, 970 | L/L | div-by-zero / OOB | `siglen` ≤ 0 when `len[a]` ≤ 0 gives inf/NaN; `YPRlen` with MaxAge=1 indexes `[-1]`. |  | [ ] |
| [ATS-13](#ats-13) | 1143 | L/L | off-by-one | Loop sets `LengthN[0..2]` but array has 4 entries, so `LengthN[3]` stays 0. |  | [ ] |

---

## ATS-01

**swapped index** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Listed lines:** 136

**Why it's thought to be a bug**

`AgeYears = Util_Alloc_Init_2D_Int(3, nYears, 0)` gives `[nYears][3]`. Used as `AgeYears[it][iyr]` (it<3, iyr<nYears) in atSS3DataGen.c:159, 833, so any year ≥3 overruns. `LengthYears` on line 167 is correct: `(nYears,4)`.

**Current code** (`atassess/atAssessTierSetup.c` lines 134–138)

```c
  134:         bm->RBCestimation.RBCspeciesArray[groupIndex].AgeFuture = Util_Alloc_Init_1D_Int(3, 0);          /* Flag for  future age  data (retained, whole, discard) */
  135:         bm->RBCestimation.RBCspeciesArray[groupIndex].AgeN = Util_Alloc_Init_1D_Int(3, 0);               /* Sample size of age comps  (retained, whole, discard) */
  136: 		bm->RBCestimation.RBCspeciesArray[groupIndex].AgeYears = Util_Alloc_Init_2D_Int(3, nYears, 0);          /* Flags which years for which to generate age comps (retained, whole, discard) */
  137: 		bm->RBCestimation.RBCspeciesArray[groupIndex].AgeFltYr = Util_Alloc_Init_3D_Int(nYears, bm->K_num_fisheries, 3, 0);         /* Flags AF years, fleets in past (r,w,d) */
  138: 		bm->RBCestimation.RBCspeciesArray[groupIndex].AgeComp = Util_Alloc_Init_5D_Int(bm->RBCestimation.OverallMaxAge, 4, nYears, bm->K_num_sexes, bm->K_num_fisheries, 0);        /* Generated age composition data */
```

**Suggested fix (from review):** `Util_Alloc_Init_2D_Int(nYears, 3, 0)`

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

## ATS-02

**swapped index** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Listed lines:** 448, 451

**Why it's thought to be a bug**

`agesel_bysex`/`fval = Util_Alloc_Init_2D_Double(K_num_sexes, numCohorts)` gives `[numCohorts][sexes]`. Used `[s][a]` with a up to numCohorts-1 in atManageTier.c:941-963, 1020.

**Current code** (`atassess/atAssessTierSetup.c` lines 450–457) — code appears to have moved +4 lines since the review

```c
  450:         bm->RBCestimation.RBCspeciesArray[groupIndex].discrate = Util_Alloc_Init_1D_Double(nYears, 0.0);
  451:         
  452:         bm->RBCestimation.RBCspeciesArray[groupIndex].agesel_bysex = Util_Alloc_Init_2D_Double(bm->K_num_sexes, FunctGroupArray[groupIndex].numCohorts, 0.0);
  453:         
  454:         if (tier == tier1)  // As only used in tier1 routines
  455:             bm->RBCestimation.RBCspeciesArray[groupIndex].fval = Util_Alloc_Init_2D_Double(bm->K_num_sexes, FunctGroupArray[groupIndex].numCohorts, 0.0);
  456:         
  457:         if (Tier3maxage < maxage)
```

**Suggested fix (from review):** swap to `(numCohorts, K_num_sexes)`

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

## ATS-03

**alloc/free mismatch** · Conf H / Sev L  
**Listed lines:** 270-307 vs 724-876

**Why it's thought to be a bug**

Close-kin, SlopeMat, FracLenS, Fupdated, FFs etc. are allocated for `assess_flag || isImpacted`, but freed only after the `continue` at line 724 (tiered + assessed only), so they leak.

**Current code** (`atassess/atAssessTierSetup.c` lines 328–367 (truncated)) — code appears to have moved +60 lines since the review

```c
  328: 	int nFtier3 = (int)(1000 * bm->RBCestimation.RBCspeciesParam[groupIndex][Tier3_maxF_id]);
  329:     
  330:     if ((FunctGroupArray[groupIndex].speciesParams[tier_id] == 0) || (FunctGroupArray[groupIndex].speciesParams[assess_flag_id] == FALSE))
  331:         return 0;
  332: 
  333:     /* From atManageTier.c and those arrays called in Tier_Assessment_Setup() */
  334:     if (bm->RBCestimation.RBCspeciesParam[groupIndex][Tier3_maxage_id] < bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id])
  335:         maxA = (int)bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id];
  336:     else
  337:         maxA = (int)bm->RBCestimation.RBCspeciesParam[groupIndex][Tier3_maxage_id];
  338: 
  339:     bm->RBCestimation.RBCspeciesArray[groupIndex].agesel = Util_Alloc_Init_1D_Double(maxA, 0.0);
  340:     bm->RBCestimation.RBCspeciesArray[groupIndex].mat = Util_Alloc_Init_1D_Double(maxA, 0.0);
  341:     bm->RBCestimation.RBCspeciesArray[groupIndex].wt = Util_Alloc_Init_1D_Double(maxA, 0.0);
  342:     bm->RBCestimation.RBCspeciesArray[groupIndex].fvalA = Util_Alloc_Init_1D_Double(maxA, 0.0);
  343:     bm->RBCestimation.RBCspeciesArray[groupIndex].ageref = Util_Alloc_Init_1D_Double(maxA, 0.0);
  344:     bm->RBCestimation.RBCspeciesArray[groupIndex].len = Util_Alloc_Init_1D_Double(maxA, 0.0);
  345:     bm->RBCestimation.RBCspeciesArray[groupIndex].p_al = Util_Alloc_Init_2D_Double(Nlen, maxA, 0.0);
  346: 
  347:     bm->RBCestimation.RBCspeciesArray[groupIndex].yield = Util_Alloc_Init_1D_Double(maxA, 0.0);
  348:     bm->RBCestimation.RBCspeciesArray[groupIndex].yield_mid = Util_Alloc_Init_1D_Double(maxA, 0.0);
  349:     bm->RBCestimation.RBCspeciesArray[groupIndex].num = Util_Alloc_Init_1D_Double(maxA, 0.0);
  350: 
  351:     maxIT = maxA;
  352:     if ((tier == tier3) && (Nfleets * 5 > maxIT))
  353:         maxIT = Nfleets * 5;    // absolute max size needed
  354:     bm->RBCestimation.RBCspeciesArray[groupIndex].z = Util_Alloc_Init_1D_Double(maxIT, 0.0);
  355:     
  356:     bm->RBCestimation.RBCspeciesArray[groupIndex].T5_Fval = Util_Alloc_Init_1D_Double(nFtier3, 0.0);          /* Fvals for Tier 5 avlen method */
  357:     bm->RBCestimation.RBCspeciesArray[groupIndex].T5_Avlen_atF = Util_Alloc_Init_1D_Double(nFtier3, 0.0);    /* Average length at F for Tier 5 avlen method */
  358: 
  359:     bm->RBCestimation.RBCspeciesArray[groupIndex].sumcatch = Util_Alloc_Init_1D_Double(Nfleets, 0.0);
  360: 
  361:     return 0;
  362: }
  363: 
  364: 
  365: 
  366: int Tier_Assessment_PostLoad_Allocate(MSEBoxModel *bm) {
  367:     
```

**Suggested fix (from review):** move these frees above line 724

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

## ATS-04

**wrong variable** · Conf M / Sev M  
**Listed lines:** 577 vs 63

**Why it's thought to be a bug**

Free is gated on `isFished` but allocation is gated on `isImpacted`. `RBCspeciesArray` is malloc'd (not calloc'd), so a fished-not-impacted group frees garbage pointers.

**Current code** (`atassess/atAssessTierSetup.c` lines 579–583) — code appears to have moved +4 lines since the review

```c
  579:         tier = (int) (FunctGroupArray[groupIndex].speciesParams[tier_id]);
  580:         
  581:         if ((FunctGroupArray[groupIndex].speciesParams[assess_flag_id] == FALSE) && (FunctGroupArray[groupIndex].isFished == FALSE))
  582:             continue;
  583:         
```

**Suggested fix (from review):** use `isImpacted` on line 577; calloc the struct arrays

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

## ATS-05

**wrong operator** · Conf M / Sev H  
**Listed lines:** 1439

**Why it's thought to be a bug**

`isTAC || (TACamt < no_quota)` sums the `no_quota` sentinel into `TACinitial`/`RBC_old`. Elsewhere this test is `isTAC && ...` or `!isTAC || ...`.

**Current code** (`atassess/atAssessTierSetup.c` lines 1449–1453) — code appears to have moved +12 lines since the review

```c
 1449: 		totTAC = 0.0;
 1450: 		for (n = 0; n < bm->K_num_fisheries; n++){
 1451:             if(FunctGroupArray[groupIndex].isTAC || (bm->TACamt[groupIndex][n][now_id] < no_quota)) {
 1452:                 totTAC += bm->TACamt[groupIndex][n][now_id];
 1453:             }
```

**Suggested fix (from review):** `isTAC && (TACamt < no_quota)`

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

## ATS-06

**off-by-one** · Conf H / Sev M  
**Listed lines:** 1050-1073

**Why it's thought to be a bug**

`thisfval = i/nfintt` at the end of iteration i, so F=0 is evaluated twice and every step lags by one. F20/40/48 come out 1/nfint low and maxF is never reached.

**Current code** (`atassess/atAssessTierSetup.c` lines 1056–1083) — code appears to have moved +8 lines since the review

```c
 1056: 	thisalpha = (1.0 - bm->RBCestimation.RBCspeciesParam[species][Tier3_h_id]) / (4.0 * bm->RBCestimation.RBCspeciesParam[species][Tier3_h_id]);    // this is (usual alpha)/B0
 1057: 	thisbeta = (5.0 * bm->RBCestimation.RBCspeciesParam[species][Tier3_h_id] - 1.0) / (4.0 * bm->RBCestimation.RBCspeciesParam[species][Tier3_h_id]);
 1058: 	thisfval = 0.0;
 1059: 
 1060: 	for (i = 0; i < nfint; i++){  // f = i/1000  f goes from 0 to Tier3_maxF
 1061:         bm->RBCestimation.RBCspeciesArray[species].T5_Fval[i] = thisfval;
 1062: 
 1063: 		YPRlen(bm, species, thisfval, bm->RBCestimation.RBCspeciesParam[species][Tier3_M_id], &spbpr, &avlen_atF, bm->RBCestimation.RBCspeciesArray[species].wt, bm->RBCestimation.RBCspeciesArray[species].len, bm->RBCestimation.RBCspeciesArray[species].mat, bm->RBCestimation.RBCspeciesArray[species].agesel, bm->RBCestimation.RBCspeciesArray[species].p_al);
 1064: 
 1065:         bm->RBCestimation.RBCspeciesArray[species].T5_Avlen_atF[i] = avlen_atF;
 1066:         
 1067:         //fprintf(bm->logFile,"i: %d avlen_atF: %e T5_val: %e\n", i, avlen_atF, bm->RBCestimation.RBCspeciesArray[species].T5_Fval[i]);
 1068: 
 1069: 		if (i == 0) spb0 = spbpr + small_num;
 1070: 		spfrac = spbpr / spb0;
 1071: 		rec_prog = spfrac/(thisalpha + thisbeta * spfrac + small_num);
 1072: 		spbf = spbpr * rec_prog;
 1073: 
 1074: 		if (i == 0) spbf0 = spbf;
 1075: 
 1076: 		// F20, F40 F48
 1077: 		if (spbf > (spbf0 * 0.2)) bm->RBCestimation.RBCspeciesParam[species][F20_id] = thisfval;
 1078: 		if (spbf > (spbf0 * 0.4)) bm->RBCestimation.RBCspeciesParam[species][F40_id] = thisfval;
 1079: 		if (spbf > (spbf0 * 0.48)) bm->RBCestimation.RBCspeciesParam[species][F48_id] = thisfval;
 1080: 
 1081: 		thisfval = i / nfintt;
 1082: 	}
 1083: 
```

**Suggested fix (from review):** `thisfval = (i+1)/nfintt;`

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

## ATS-07

**wrong variable** · Conf M / Sev M  
**Listed lines:** 1406

**Why it's thought to be a bug**

`metierArray[mid].metierCode` is copied from `FisheryArray[m]` (assessment fleet index) rather than `FisheryArray[mid]`.

**Current code** (`atassess/atAssessTierSetup.c` lines 1412–1416) — code appears to have moved +8 lines since the review

```c
 1412:                 mid = bm->RBCestimation.speciesRPFleetToMetier[m][groupIndex];  // metier for this fishery
 1413:                 bm->RBCestimation.speciesMetierToRPFleet[groupIndex][mid] = m;
 1414:                 strcpy(bm->RBCestimation.metierArray[mid].metierCode, FisheryArray[m].fisheryCode);
 1415:                 bm->RBCestimation.metierArray[mid].PGMSYlinks = PGMSYLinksInputs[mid];
 1416:             }
```

**Suggested fix (from review):** use `mid`

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

## ATS-08

**uninitialised mapping** · Conf M / Sev M  
**Listed lines:** 53, 1404, 1412

**Why it's thought to be a bug**

`speciesRPFleetToMetier` is initialised to 0 and never populated anywhere, so every fleet maps to metier 0. atSS3LinkIO tests `> -1`, which is always true.

**Current code** (`atassess/atAssessTierSetup.c` lines 51–55)

```c
   51:     
   52:     bm->RBCestimation.speciesMetierToRPFleet = Util_Alloc_Init_2D_Int(bm->K_num_fisheries, bm->K_num_tot_sp, 0);
   53:     bm->RBCestimation.speciesRPFleetToMetier = Util_Alloc_Init_2D_Int(bm->K_num_tot_sp, bm->K_num_fisheries, 0);
   54:     bm->RBCestimation.metierArray = (METarrays *) malloc(sizeof(METarrays) * (unsigned long int)bm->K_num_fisheries);
   55:     
```

**Suggested fix (from review):** init to -1 and populate (check for lost read code)

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

## ATS-09

**memory leak** · Conf H / Sev L  
**Listed lines:** 561-899

**Why it's thought to be a bug**

Never freed: AgeComp, AFss, LenComp, Xmove, DiscardType, Blocks, Obsageprop, MeanLenAgeS, T5_removal, TAC_mult, Tier3_Linf/k/t0, Tier5_Bstart, Tier5_r, MULTISPestIDs, the CloseKinEst array.

**Current code** (`atassess/atAssessTierSetup.c` lines 559–593 (range truncated))

```c
  559: 	return 0;
  560: }
  561: 
  562: /**
  563:  * Free the Tier assessment data structures.
  564:  */
  565: int Tier_Assessment_Free(MSEBoxModel *bm) {
  566: 
  567: 	int groupIndex, tier;
  568:     
  569:     free1d(bm->RBCestimation.catchwghtCPUE);
  570:     free1d(bm->RBCestimation.catchwght);
  571:     i_free1d(bm->RBCestimation.count_sp_in_buffer);
  572:     
  573:     if(bm->RBCestimation.speciesMetierToRPFleet)
  574:         i_free2d(bm->RBCestimation.speciesMetierToRPFleet);
  575:     if(bm->RBCestimation.speciesRPFleetToMetier)
  576:         i_free2d(bm->RBCestimation.speciesRPFleetToMetier);
  577:     
  578: 	for (groupIndex = 0; groupIndex < bm->K_num_tot_sp; groupIndex++) {
  579:         tier = (int) (FunctGroupArray[groupIndex].speciesParams[tier_id]);
  580:         
  581:         if ((FunctGroupArray[groupIndex].speciesParams[assess_flag_id] == FALSE) && (FunctGroupArray[groupIndex].isFished == FALSE))
  582:             continue;
  583:         
  584:         printf("Doing %s assess free\n", FunctGroupArray[groupIndex].groupCode);
  585: 
  586:         i_free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].regID);
  587:         i_free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].Ia);
  588:         free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].Future_catchprop);
  589:         
  590:         free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].CvLA0);
  591:         free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].CvLAmax);
  592:         free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].Fecund);
  593:         
```

**Suggested fix (from review):** add matching frees

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

## ATS-10

**printf mismatch** · Conf H / Sev L  
**Listed lines:** 1454

**Why it's thought to be a bug**

`%d` used for a `double` (`RBCspeciesParam[..][NumFisheries_id]`).

**Current code** (`atassess/atAssessTierSetup.c` lines 1458–1462) — code appears to have moved +6 lines since the review

```c
 1458:         tot_nf++;
 1459:         
 1460:         bm->RBCestimation.RBCspeciesParam[groupIndex][TACinitial_id] = totTAC;
 1461: 		bm->RBCestimation.RBCspeciesParam[groupIndex][RBC_old_id] = totTAC;
 1462: 		bm->RBCestimation.RBCspeciesParam[groupIndex][EstB0_id] = bm->estBo[groupIndex];
```

**Suggested fix (from review):** cast to `(int)`

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

## ATS-11

**logic** · Conf L / Sev L  
**Listed lines:** 1528-1530

**Why it's thought to be a bug**

`Lastsp` is only incremented when >0, so if the last assessed group is index 0 it's skipped by `sp < Lastsp` loops; also never reset.

**Current code** (`atassess/atAssessTierSetup.c` lines 1538–1544) — code appears to have moved +12 lines since the review

```c
 1538: 	}
 1539:     
 1540:     if (bm->RBCestimation.Lastsp > 0) {
 1541:         bm->RBCestimation.Lastsp++;  // So don't cut off the final species
 1542:     }
 1543:     
 1544:     if (bm->RBCestimation.LastMetier > -1) {
```

**Suggested fix (from review):** review

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

## ATS-12

**div-by-zero / OOB** · Conf L / Sev L  
**Listed lines:** 917; 954, 970

**Why it's thought to be a bug**

`siglen` ≤ 0 when `len[a]` ≤ 0 gives inf/NaN; `YPRlen` with MaxAge=1 indexes `[-1]`.

**Current code** (`atassess/atAssessTierSetup.c` lines 918–957 (truncated)) — code appears to have moved +3 lines since the review

```c
  918: void Prop_LatA(MSEBoxModel *bm, int species, double *len, double **p_al){
  919: 	int a, l, nlen;
  920: 	double xlim, accum, integral, meanlen, siglen;
  921: 
  922: 	// calc proportion of age a in length bin l
  923: 	for (a = 0; a < bm->RBCestimation.RBCspeciesParam[species][MaxAge_id]; a++) {
  924: 		meanlen = len[a];
  925: 		siglen = bm->RBCestimation.RBCspeciesParam[species][Tier5_cv_id] * meanlen;
  926: 		xlim = (bm->RBCestimation.RBCspeciesArray[species].LoLenBin[1] - meanlen) / siglen;
  927: 		accum = Assess_errfun(xlim);
  928: 		p_al[a][0] = accum;
  929:         
  930:         //fprintf(bm->logFile,"a: %d meanlen: %e siglen: %e cv: %e xlim: %e p_al[0] and accum: %e\n", a, meanlen, siglen, bm->RBCestimation.RBCspeciesParam[species][Tier5_cv_id], xlim, accum);
  931:         
  932: 		for (l = 1; l < bm->RBCestimation.RBCspeciesParam[species][Nlen_id] - 1; l++){
  933: 			xlim = (bm->RBCestimation.RBCspeciesArray[species].LoLenBin[l+1] - meanlen) / siglen;
  934: 			integral = Assess_errfun(xlim);
  935: 			p_al[a][l] = integral - accum;
  936: 			accum = integral;
  937:             
  938:             //fprintf(bm->logFile,"a: %d l: %d xlim: %e integral: %e accum: %e p_al[l]: %e\n", a, l, xlim, integral, accum, p_al[a][l]);
  939:             
  940: 		}
  941: 		nlen = (int)(bm->RBCestimation.RBCspeciesParam[species][Nlen_id]);
  942:         nlen--; // So don't over-write the end of the array
  943: 		p_al[a][nlen] = 1.0 - accum;
  944:         
  945:         //fprintf(bm->logFile,"a: %d l: %d accum: %e p_al[l]: %e\n", a, nlen, accum, p_al[a][nlen]);
  946:         
  947: 	}
  948: }
  949: 
  950: 
  951: /******************************************************************************
  952:  *
  953:  * Name:  YPRlen
  954:  * Description: yield-per-recruit and average length calculations for given F
  955:  * called by : getF20_40_48_len
  956: ******************************************************************************/
  957: void YPRlen(MSEBoxModel *bm, int species, double fval, double natM, double *spbpr,
```

**Suggested fix (from review):** guard

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

## ATS-13

**off-by-one** · Conf L / Sev L  
**Listed lines:** 1143

**Why it's thought to be a bug**

Loop sets `LengthN[0..2]` but array has 4 entries, so `LengthN[3]` stays 0.

**Current code** (`atassess/atAssessTierSetup.c` lines 1151–1155) — code appears to have moved +10 lines since the review

```c
 1151:     for (j=0; j < 3; j++){
 1152:         bm->RBCestimation.RBCspeciesArray[species].AgeN[j] = ssize;
 1153:         bm->RBCestimation.RBCspeciesArray[species].LengthN[j] = ssize;
 1154:         
 1155:         fprintf(bm->logFile, "%s %d has samplesize %d (AgeN: %d, LengthN: %d\n", FunctGroupArray [species].groupCode, j, ssize, bm->RBCestimation.RBCspeciesArray[species].AgeN[j], bm->RBCestimation.RBCspeciesArray[species].LengthN[j]);
```

**Suggested fix (from review):** check intent

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
