# atassess/atAssessTierSetup.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **13 findings** (severity H: 3; confidence H and severity H: 2; on the priority list: 2).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [ATS-01](#ats-01) ✔ ⭐ | 136 | H/H | swapped index | `AgeYears = Util_Alloc_Init_2D_Int(3, nYears, 0)` gives `[nYears][3]`. Used as `AgeYears[it][iyr]` (it<3, iyr<… |  | [ ] |
| [ATS-02](#ats-02) ✔ ⭐ | 452, 455 | H/H | swapped index | `agesel_bysex`/`fval = Util_Alloc_Init_2D_Double(K_num_sexes, numCohorts)` gives `[numCohorts][sexes]`. Used `… |  | [ ] |
| [ATS-03](#ats-03) | 270-311 vs 729-885 | H/L | alloc/free mismatch | Close-kin, SlopeMat, FracLenS, Fupdated, FFs etc. are allocated for `assess_flag \|\| isImpacted`, but freed onl… |  | [ ] |
| [ATS-04](#ats-04) | 582 vs 63 | M/M | wrong variable | Free is gated on `isFished` but allocation is gated on `isImpacted`. `RBCspeciesArray` is malloc'd (not calloc… |  | [ ] |
| [ATS-05](#ats-05) | 1464 | M/H | wrong operator | `isTAC \|\| (TACamt < no_quota)` sums the `no_quota` sentinel into `TACinitial`/`RBC_old`. Elsewhere this test i… |  | [ ] |
| [ATS-06](#ats-06) | 1059-1082 | H/M | off-by-one | `thisfval = i/nfintt` at the end of iteration i, so F=0 is evaluated twice and every step lags by one. F20/40/… |  | [ ] |
| [ATS-07](#ats-07) | 1415 | M/M | wrong variable | `metierArray[mid].metierCode` is copied from `FisheryArray[m]` (assessment fleet index) rather than `FisheryAr… |  | [ ] |
| [ATS-08](#ats-08) | 53, 1413, 1421 | M/M | uninitialised mapping | `speciesRPFleetToMetier` is initialised to 0 and never populated anywhere, so every fleet maps to metier 0. at… |  | [ ] |
| [ATS-09](#ats-09) | 566-908 | H/L | memory leak | Never freed: AgeComp, AFss, LenComp, Xmove, DiscardType, Blocks, Obsageprop, MeanLenAgeS, T5_removal, TAC_mult… |  | [ ] |
| [ATS-10](#ats-10) | 1479 | H/L | printf mismatch | `%d` used for a `double` (`RBCspeciesParam[..][NumFisheries_id]`). |  | [ ] |
| [ATS-11](#ats-11) | 1553-1555 | L/L | logic | `Lastsp` is only incremented when >0, so if the last assessed group is index 0 it's skipped by `sp < Lastsp` l… |  | [ ] |
| [ATS-12](#ats-12) | 926; 963, 979 | L/L | div-by-zero / OOB | `siglen` ≤ 0 when `len[a]` ≤ 0 gives inf/NaN; `YPRlen` with MaxAge=1 indexes `[-1]`. |  | [ ] |
| [ATS-13](#ats-13) | 1152 | L/L | off-by-one | Loop sets `LengthN[0..2]` but array has 4 entries, so `LengthN[3]` stays 0. |  | [ ] |

---

## ATS-01

**swapped index** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Lines (current code):** 136  
**Lines at review (Oct 2026):** 136

**Why it's thought to be a bug**

`AgeYears = Util_Alloc_Init_2D_Int(3, nYears, 0)` gives `[nYears][3]`. Used as `AgeYears[it][iyr]` (it<3, iyr<nYears) in atSS3DataGen.c:159, 838, so any year ≥3 overruns. `LengthYears` on line 167 is correct: `(nYears,4)`.

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
- [Yes] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> Swap the AgeYears[it][iyr] tp AgeYears[iyr][it]

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## ATS-02

**swapped index** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Lines (current code):** 452, 455  
**Lines at review (Oct 2026):** 448, 451

**Why it's thought to be a bug**

`agesel_bysex`/`fval = Util_Alloc_Init_2D_Double(K_num_sexes, numCohorts)` gives `[numCohorts][sexes]`. Used `[s][a]` with a up to numCohorts-1 in atManageTier.c:1048-1070, 1159.

**Current code** (`atassess/atAssessTierSetup.c` lines 450–457)

```c
  450:         bm->RBCestimation.RBCspeciesArray[groupIndex].discrate = Util_Alloc_Init_1D_Double(nYears, 0.0);
  451:         
  452:         bm->RBCestimation.RBCspeciesArray[groupIndex].agesel_bysex = Util_Alloc_Init_2D_Double(bm->K_num_sexes, FunctGroupArray[groupIndex].numCohorts, 0.0);
  453:         
  454:         if (tier == tier1) { // As only used in tier1 routines
  455:             bm->RBCestimation.RBCspeciesArray[groupIndex].fval = Util_Alloc_Init_2D_Double(bm->K_num_sexes, FunctGroupArray[groupIndex].numCohorts, 0.0);
  456:         }
  457:         
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

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## ATS-03

**alloc/free mismatch** · Conf H / Sev L  
**Lines (current code):** 270-311 vs 729-885  
**Lines at review (Oct 2026):** 270-307 vs 724-876

**Why it's thought to be a bug**

Close-kin, SlopeMat, FracLenS, Fupdated, FFs etc. are allocated for `assess_flag || isImpacted`, but freed only after the `continue` at line 729 (tiered + assessed only), so they leak.

**Current code** (`atassess/atAssessTierSetup.c` lines 268–307 (truncated)) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
  268: 		//bm->RBCestimation.RBCspeciesArray[groupIndex].VulBio = Util_Alloc_Init_3D_Double(nYears, bm->K_num_reg, bm->K_num_fisheries, 0.0);        /* Vulnerable biomass by fleet, region, time */
  269:         
  270:         bm->CloseKinEst[groupIndex].fec_expo_s = Util_Alloc_Init_1D_Double(bm->RBCestimation.RBCspeciesParam[groupIndex][Nsexes_id], 1.0);
  271:         bm->CloseKinEst[groupIndex].fec_sa = Util_Alloc_Init_2D_Double(bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id], bm->RBCestimation.RBCspeciesParam[groupIndex][Nsexes_id], 0.0);
  272:         bm->CloseKinEst[groupIndex].mature = Util_Alloc_Init_1D_Double(bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id], 0.0);
  273:         bm->CloseKinEst[groupIndex].lengths = Util_Alloc_Init_1D_Double(bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id], 0.0);
  274:         bm->CloseKinEst[groupIndex].samp_prop_to = Util_Alloc_Init_3D_Double(bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id], bm->CloseKinEst->last_sy, bm->RBCestimation.RBCspeciesParam[groupIndex][Nsexes_id], 0.0);
  275:         
  276:         bm->RBCestimation.RBCspeciesArray[groupIndex].SlopeMat = Util_Alloc_Init_1D_Double(FunctGroupArray[groupIndex].numStocks, 0.0);
  277:         bm->RBCestimation.RBCspeciesArray[groupIndex].Mat50 = Util_Alloc_Init_1D_Double(FunctGroupArray[groupIndex].numStocks, 0.0);
  278:         bm->RBCestimation.RBCspeciesArray[groupIndex].HiLenBin = Util_Alloc_Init_1D_Double(nLen, 0.0);
  279:         bm->RBCestimation.RBCspeciesArray[groupIndex].FracLenS = Util_Alloc_Init_5D_Double(nLen, nYears, bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id], bm->RBCestimation.RBCspeciesParam[groupIndex][Nsexes_id], bm->K_num_stocks_per_sp, 0.0);
  280:         // Used? bm->RBCestimation.RBCspeciesArray[groupIndex].FracLenM = Util_Alloc_Init_4D_Double(nYears, bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id], bm->RBCestimation.RBCspeciesParam[groupIndex][Nsexes_id], bm->K_num_stocks_per_sp, 0.0);
  281:         bm->RBCestimation.RBCspeciesArray[groupIndex].MeanLenAgeM = Util_Alloc_Init_4D_Double(nYears, bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id], bm->RBCestimation.RBCspeciesParam[groupIndex][Nsexes_id], bm->K_num_stocks_per_sp, 0.0);
  282:         bm->RBCestimation.RBCspeciesArray[groupIndex].SigmaLenAgeM = Util_Alloc_Init_4D_Double(nYears, bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id], bm->RBCestimation.RBCspeciesParam[groupIndex][Nsexes_id], bm->K_num_stocks_per_sp, 0.0);
  283:         bm->RBCestimation.RBCspeciesArray[groupIndex].MeanWtAgeS = Util_Alloc_Init_4D_Double(nYears, bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id], bm->RBCestimation.RBCspeciesParam[groupIndex][Nsexes_id], bm->K_num_stocks_per_sp, 0.0);
  284:         bm->RBCestimation.RBCspeciesArray[groupIndex].MeanWtAgeM = Util_Alloc_Init_4D_Double(nYears, bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id], bm->RBCestimation.RBCspeciesParam[groupIndex][Nsexes_id], bm->K_num_stocks_per_sp, 0.0);
  285:         bm->RBCestimation.RBCspeciesArray[groupIndex].MeanLenAgeS = Util_Alloc_Init_4D_Double(nYears, bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id], bm->RBCestimation.RBCspeciesParam[groupIndex][Nsexes_id], bm->K_num_stocks_per_sp, 0.0);
  286: 
  287:         bm->RBCestimation.RBCspeciesArray[groupIndex].NblockperPatt = Util_Alloc_Init_1D_Int(bm->RBCestimation.RBCspeciesParam[groupIndex][NblockPattern_id], 0);
  288:         bm->RBCestimation.RBCspeciesArray[groupIndex].Blocks = Util_Alloc_Init_2D_Double(bm->K_max_BlockPattern, bm->RBCestimation.RBCspeciesParam[groupIndex][NblockPattern_id], 0.0);
  289:         
  290:         bm->RBCestimation.RBCspeciesArray[groupIndex].discfleet = Util_Alloc_Init_1D_Int(bm->K_num_fisheries, 0);
  291:         bm->RBCestimation.RBCspeciesArray[groupIndex].initialEquilCatch = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
  292:         bm->RBCestimation.RBCspeciesArray[groupIndex].R02 = Util_Alloc_Init_1D_Double(Nregions, 0.0);
  293:         bm->RBCestimation.RBCspeciesArray[groupIndex].SSHsteep = Util_Alloc_Init_1D_Double(Nregions, 0.0);
  294:         bm->RBCestimation.RBCspeciesArray[groupIndex].Start_Sel = Util_Alloc_Init_2D_Double(bm->K_num_tot_sp, bm->K_num_fisheries, 0.0);
  295:         bm->RBCestimation.RBCspeciesArray[groupIndex].SelBlock = Util_Alloc_Init_2D_Double(bm->K_num_tot_sp, bm->K_num_fisheries, 0.0);
  296:         bm->RBCestimation.RBCspeciesArray[groupIndex].Sel_Phase = Util_Alloc_Init_2D_Double(bm->K_num_tot_sp, bm->K_num_fisheries, 0.0);
  297:         bm->RBCestimation.RBCspeciesArray[groupIndex].RBC_by_year = Util_Alloc_Init_1D_Double(nYears, 0.0);
  298:         bm->RBCestimation.RBCspeciesArray[groupIndex].TAC_by_year = Util_Alloc_Init_1D_Double(nYears, 0.0);
  299:         bm->RBCestimation.RBCspeciesArray[groupIndex].RBC_proj = Util_Alloc_Init_1D_Double(MAX_RBC_PROJ, 0.0);
  300:         bm->RBCestimation.RBCspeciesArray[groupIndex].nRBC_proj = 0;
  301:         bm->RBCestimation.RBCspeciesArray[groupIndex].lastAssessRBC = 0.0;
  302:         
  303:         bm->RBCestimation.RBCspeciesArray[groupIndex].Mortality = Util_Alloc_Init_1D_Double(2, 0.0);
  304:                 
  305:         bm->RBCestimation.RBCspeciesArray[groupIndex].Fupdated = Util_Alloc_Init_2D_Double(nYears, bm->K_num_fisheries, 0.0);
  306:         bm->RBCestimation.RBCspeciesArray[groupIndex].RBCupdated = Util_Alloc_Init_2D_Double(nYears, bm->K_num_fisheries, 0.0);
  307:         bm->RBCestimation.RBCspeciesArray[groupIndex].AvgCatFleet = Util_Alloc_Init_2D_Double(nYears, bm->K_num_fisheries, 0.0);
```

**Suggested fix (from review):** move these frees above line 729

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
**Lines (current code):** 582 vs 63  
**Lines at review (Oct 2026):** 577 vs 63

**Why it's thought to be a bug**

Free is gated on `isFished` but allocation is gated on `isImpacted`. `RBCspeciesArray` is malloc'd (not calloc'd), so a fished-not-impacted group frees garbage pointers.

**Current code** (`atassess/atAssessTierSetup.c` lines 580–584)

```c
  580:         tier = (int) (FunctGroupArray[groupIndex].speciesParams[tier_id]);
  581:         
  582:         if ((FunctGroupArray[groupIndex].speciesParams[assess_flag_id] == FALSE) && (FunctGroupArray[groupIndex].isFished == FALSE))
  583:             continue;
  584:         
```

**Suggested fix (from review):** use `isImpacted` on line 582; calloc the struct arrays

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
**Lines (current code):** 1464  
**Lines at review (Oct 2026):** 1439

**Why it's thought to be a bug**

`isTAC || (TACamt < no_quota)` sums the `no_quota` sentinel into `TACinitial`/`RBC_old`. Elsewhere this test is `isTAC && ...` or `!isTAC || ...`.

**Current code** (`atassess/atAssessTierSetup.c` lines 1462–1466)

```c
 1462: 		bm->RBCestimation.RBCspeciesParam[groupIndex][RBC_old_id] = totTAC;
 1463: 		bm->RBCestimation.RBCspeciesParam[groupIndex][EstB0_id] = bm->estBo[groupIndex];
 1464: 		bm->RBCestimation.RBCspeciesParam[groupIndex][EstBcurr_id] = bm->RBCestimation.RBCspeciesParam[groupIndex][EstB0_id] * bm->RBCestimation.RBCspeciesParam[groupIndex][EstDepletion_id];
 1465:         
 1466:         if (tot_nf > bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]) {
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
**Lines (current code):** 1059-1082  
**Lines at review (Oct 2026):** 1050-1073

**Why it's thought to be a bug**

`thisfval = i/nfintt` at the end of iteration i, so F=0 is evaluated twice and every step lags by one. F20/40/48 come out 1/nfint low and maxF is never reached.

**Current code** (`atassess/atAssessTierSetup.c` lines 1057–1084)

```c
 1057: 	thisalpha = (1.0 - bm->RBCestimation.RBCspeciesParam[species][Tier3_h_id]) / (4.0 * bm->RBCestimation.RBCspeciesParam[species][Tier3_h_id]);    // this is (usual alpha)/B0
 1058: 	thisbeta = (5.0 * bm->RBCestimation.RBCspeciesParam[species][Tier3_h_id] - 1.0) / (4.0 * bm->RBCestimation.RBCspeciesParam[species][Tier3_h_id]);
 1059: 	thisfval = 0.0;
 1060: 
 1061: 	for (i = 0; i < nfint; i++){  // f = i/1000  f goes from 0 to Tier3_maxF
 1062:         bm->RBCestimation.RBCspeciesArray[species].T5_Fval[i] = thisfval;
 1063: 
 1064: 		YPRlen(bm, species, thisfval, bm->RBCestimation.RBCspeciesParam[species][Tier3_M_id], &spbpr, &avlen_atF, bm->RBCestimation.RBCspeciesArray[species].wt, bm->RBCestimation.RBCspeciesArray[species].len, bm->RBCestimation.RBCspeciesArray[species].mat, bm->RBCestimation.RBCspeciesArray[species].agesel, bm->RBCestimation.RBCspeciesArray[species].p_al);
 1065: 
 1066:         bm->RBCestimation.RBCspeciesArray[species].T5_Avlen_atF[i] = avlen_atF;
 1067:         
 1068:         //fprintf(bm->logFile,"i: %d avlen_atF: %e T5_val: %e\n", i, avlen_atF, bm->RBCestimation.RBCspeciesArray[species].T5_Fval[i]);
 1069: 
 1070: 		if (i == 0) spb0 = spbpr + small_num;
 1071: 		spfrac = spbpr / spb0;
 1072: 		rec_prog = spfrac/(thisalpha + thisbeta * spfrac + small_num);
 1073: 		spbf = spbpr * rec_prog;
 1074: 
 1075: 		if (i == 0) spbf0 = spbf;
 1076: 
 1077: 		// F20, F40 F48
 1078: 		if (spbf > (spbf0 * 0.2)) bm->RBCestimation.RBCspeciesParam[species][F20_id] = thisfval;
 1079: 		if (spbf > (spbf0 * 0.4)) bm->RBCestimation.RBCspeciesParam[species][F40_id] = thisfval;
 1080: 		if (spbf > (spbf0 * 0.48)) bm->RBCestimation.RBCspeciesParam[species][F48_id] = thisfval;
 1081: 
 1082: 		thisfval = i / nfintt;
 1083: 	}
 1084: 
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
**Lines (current code):** 1415  
**Lines at review (Oct 2026):** 1406

**Why it's thought to be a bug**

`metierArray[mid].metierCode` is copied from `FisheryArray[m]` (assessment fleet index) rather than `FisheryArray[mid]`.

**Current code** (`atassess/atAssessTierSetup.c` lines 1413–1417)

```c
 1413:                 mid = bm->RBCestimation.speciesRPFleetToMetier[m][groupIndex];  // metier for this fishery
 1414:                 bm->RBCestimation.speciesMetierToRPFleet[groupIndex][mid] = m;
 1415:                 strcpy(bm->RBCestimation.metierArray[mid].metierCode, FisheryArray[m].fisheryCode);
 1416:                 bm->RBCestimation.metierArray[mid].PGMSYlinks = PGMSYLinksInputs[mid];
 1417:             }
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
**Lines (current code):** 53, 1413, 1421  
**Lines at review (Oct 2026):** 53, 1404, 1412

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
**Lines (current code):** 566-908  
**Lines at review (Oct 2026):** 561-899

**Why it's thought to be a bug**

Never freed: AgeComp, AFss, LenComp, Xmove, DiscardType, Blocks, Obsageprop, MeanLenAgeS, T5_removal, TAC_mult, Tier3_Linf/k/t0, Tier5_Bstart, Tier5_r, MULTISPestIDs, the CloseKinEst array.

**Current code** (`atassess/atAssessTierSetup.c` lines 564–603 (truncated))

```c
  564:  * Free the Tier assessment data structures.
  565:  */
  566: int Tier_Assessment_Free(MSEBoxModel *bm) {
  567: 
  568: 	int groupIndex, tier;
  569:     
  570:     free1d(bm->RBCestimation.catchwghtCPUE);
  571:     free1d(bm->RBCestimation.catchwght);
  572:     i_free1d(bm->RBCestimation.count_sp_in_buffer);
  573:     
  574:     if(bm->RBCestimation.speciesMetierToRPFleet)
  575:         i_free2d(bm->RBCestimation.speciesMetierToRPFleet);
  576:     if(bm->RBCestimation.speciesRPFleetToMetier)
  577:         i_free2d(bm->RBCestimation.speciesRPFleetToMetier);
  578:     
  579: 	for (groupIndex = 0; groupIndex < bm->K_num_tot_sp; groupIndex++) {
  580:         tier = (int) (FunctGroupArray[groupIndex].speciesParams[tier_id]);
  581:         
  582:         if ((FunctGroupArray[groupIndex].speciesParams[assess_flag_id] == FALSE) && (FunctGroupArray[groupIndex].isFished == FALSE))
  583:             continue;
  584:         
  585:         printf("Doing %s assess free\n", FunctGroupArray[groupIndex].groupCode);
  586: 
  587:         i_free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].regID);
  588:         i_free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].Ia);
  589:         free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].Future_catchprop);
  590:         
  591:         free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].CvLA0);
  592:         free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].CvLAmax);
  593:         free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].Fecund);
  594:         
  595:         free3d(bm->RBCestimation.RBCspeciesArray[groupIndex].MeanLenAge);
  596:         free4d(bm->RBCestimation.RBCspeciesArray[groupIndex].MeanWtAge);
  597:         free3d(bm->RBCestimation.RBCspeciesArray[groupIndex].Mzero);
  598:         free5d(bm->RBCestimation.RBCspeciesArray[groupIndex].Mnat);
  599:         
  600:         free3d(bm->RBCestimation.RBCspeciesArray[groupIndex].WtLen);
  601:         free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_a);
  602:         free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_b);
  603:         
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
**Lines (current code):** 1479  
**Lines at review (Oct 2026):** 1454

**Why it's thought to be a bug**

`%d` used for a `double` (`RBCspeciesParam[..][NumFisheries_id]`).

**Current code** (`atassess/atAssessTierSetup.c` lines 1477–1481) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
 1477: 		kpar = 0.0;
 1478: 		t0 = 0.0;
 1479: 		for (s = 0; s < bm->K_num_sexes; s++){
 1480: 			linf += bm->RBCestimation.RBCspeciesArray[groupIndex].Tier3_Linf[s];
 1481: 			kpar += bm->RBCestimation.RBCspeciesArray[groupIndex].Tier3_k[s];
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
**Lines (current code):** 1553-1555  
**Lines at review (Oct 2026):** 1528-1530

**Why it's thought to be a bug**

`Lastsp` is only incremented when >0, so if the last assessed group is index 0 it's skipped by `sp < Lastsp` loops; also never reset.

**Current code** (`atassess/atAssessTierSetup.c` lines 1551–1557) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
 1551:         free1d(TrigPts);
 1552:     }
 1553:     if(PGMSYLinksInputs) {
 1554:         i_free1d(PGMSYLinksInputs);
 1555:     }
 1556:     
 1557: 	return 0;
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
**Lines (current code):** 926; 963, 979  
**Lines at review (Oct 2026):** 917; 954, 970

**Why it's thought to be a bug**

`siglen` ≤ 0 when `len[a]` ≤ 0 gives inf/NaN; `YPRlen` with MaxAge=1 indexes `[-1]`.

**Current code** (`atassess/atAssessTierSetup.c` lines 924–963 (truncated))

```c
  924: 	for (a = 0; a < bm->RBCestimation.RBCspeciesParam[species][MaxAge_id]; a++) {
  925: 		meanlen = len[a];
  926: 		siglen = bm->RBCestimation.RBCspeciesParam[species][Tier5_cv_id] * meanlen;
  927: 		xlim = (bm->RBCestimation.RBCspeciesArray[species].LoLenBin[1] - meanlen) / siglen;
  928: 		accum = Assess_errfun(xlim);
  929: 		p_al[a][0] = accum;
  930:         
  931:         //fprintf(bm->logFile,"a: %d meanlen: %e siglen: %e cv: %e xlim: %e p_al[0] and accum: %e\n", a, meanlen, siglen, bm->RBCestimation.RBCspeciesParam[species][Tier5_cv_id], xlim, accum);
  932:         
  933: 		for (l = 1; l < bm->RBCestimation.RBCspeciesParam[species][Nlen_id] - 1; l++){
  934: 			xlim = (bm->RBCestimation.RBCspeciesArray[species].LoLenBin[l+1] - meanlen) / siglen;
  935: 			integral = Assess_errfun(xlim);
  936: 			p_al[a][l] = integral - accum;
  937: 			accum = integral;
  938:             
  939:             //fprintf(bm->logFile,"a: %d l: %d xlim: %e integral: %e accum: %e p_al[l]: %e\n", a, l, xlim, integral, accum, p_al[a][l]);
  940:             
  941: 		}
  942: 		nlen = (int)(bm->RBCestimation.RBCspeciesParam[species][Nlen_id]);
  943:         nlen--; // So don't over-write the end of the array
  944: 		p_al[a][nlen] = 1.0 - accum;
  945:         
  946:         //fprintf(bm->logFile,"a: %d l: %d accum: %e p_al[l]: %e\n", a, nlen, accum, p_al[a][nlen]);
  947:         
  948: 	}
  949: }
  950: 
  951: 
  952: /******************************************************************************
  953:  *
  954:  * Name:  YPRlen
  955:  * Description: yield-per-recruit and average length calculations for given F
  956:  * called by : getF20_40_48_len
  957: ******************************************************************************/
  958: void YPRlen(MSEBoxModel *bm, int species, double fval, double natM, double *spbpr,
  959: 		double *avlen_atF, double *wt, double *len, double *mat, double *sel, double **p_al){
  960: 
  961: 	int a,l;
  962: 	double yp,ypl;
  963: 	int maxage = (int)(bm->RBCestimation.RBCspeciesParam[species][MaxAge_id] - 1);
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
**Lines (current code):** 1152  
**Lines at review (Oct 2026):** 1143

**Why it's thought to be a bug**

Loop sets `LengthN[0..2]` but array has 4 entries, so `LengthN[3]` stays 0.

**Current code** (`atassess/atAssessTierSetup.c` lines 1150–1154)

```c
 1150: 
 1151:     
 1152:     for (j=0; j < 3; j++){
 1153:         bm->RBCestimation.RBCspeciesArray[species].AgeN[j] = ssize;
 1154:         bm->RBCestimation.RBCspeciesArray[species].LengthN[j] = ssize;
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
