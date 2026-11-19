# atSS3Link/atCloseKin.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **15 findings** (severity H: 7; confidence H and severity H: 6; on the priority list: 6).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SS3-26](#ss3-26) ✔ ⭐ | 78, 87 | H/H | swapped dims (alloc convention) | All local arrays are allocated with dims in *index* order instead of reversed. `nsamps_sya = Alloc_3D(Nsexes,n… |  | [ ] |
| [SS3-27](#ss3-27) | 55, 425 | M/M | OOB | `nsamps_y` and `nsamps_sya` are sized `nsampy = last_sy-first_sy+1` but indexed with absolute `y` up to `last_… |  | [ ] |
| [SS3-28](#ss3-28) ⭐ | 140, 72 | H/H | year value as index | `FracLenS[j][s][a][this_min][l]` with `this_min = HistYrMin_id` uses a calendar year (e.g. 1990) as the year i… |  | [ ] |
| [SS3-29](#ss3-29) | 133, 138 | M/L | overwritten per stock | `mature[l]` and `fec_sa[s][a]` are recomputed (=0 then +=) for each stock `j`, so only the last stock's fecund… |  | [ ] |
| [SS3-30](#ss3-30) ⭐ | 162 | H/H | use after close | `Util_Close_Output_File(bm->closekinCFile)` and the closes of the samp/PO/HS files fclose the stream, but the … |  | [ ] |
| [SS3-31](#ss3-31) ⭐ | 241, 73, 344 | H/H | off-by-one OOB | `psurv_syay[s][y-1][this_maxage][y]` with `this_maxage = MaxAge`, `curr_a` allowed to reach MaxAge (`if (curr_… |  | [ ] |
| [SS3-32](#ss3-32) | 294 | H/M | accumulator not reset | `stocktot` is never reset inside the s/y loops, so `inv_totfec_sy` becomes 1/(cumulative total over all earlie… |  | [ ] |
| [SS3-33](#ss3-33) ⭐ | 382 | H/H | off-by-one loop start | `for (int s1 = 1; s1 < Nsexes; s1++) comps += ...` skips offspring sex 0, although the comment says "sex of of… |  | [ ] |
| [SS3-34](#ss3-34) ⭐ | 496, 161, 472 | H/H | swapped arguments | The call `writeCloseKinNcompsfiles(bm, sp, year, ...)` does not match the signature `(bm, int year, int sp, ..… |  | [ ] |
| [SS3-35](#ss3-35) | 55 | M/H | uninitialised per-species params | The code reads `CloseKinEst[sp].first_sy/last_sy/first_dy/samp_power/a_ck_thresh...`, but these are only read … |  | [ ] |
| [SS3-36](#ss3-36) | 238 | M/M | division by zero | `n_sya[s][y][a] / n_sya[s][y-1][a-1]` has no guard. Zero numbers give inf or NaN survival, which then propagat… |  | [ ] |
| [SS3-37](#ss3-37) | 355 | M/L | brace placement | The `} // for ap_b1` brace actually closes the `if`, so `Pr_HS_syaya = demog_Pr_HSP; Pr_HS_yaya += ...` runs o… |  | [ ] |
| [SS3-38](#ss3-38) | 589 | M/M | wrong index / compounding | `ageprops[s][a]` uses sex `s` as the stock index; ageprops is `[stock][age]`. Also `samp_prop_to += ...` is ne… |  | [ ] |
| [SS3-39](#ss3-39) | 645, 649 | M/L | copy-paste | The HS output prints `(int)sim_nkin_HS[s][y][a][y2][a2]` twice. The TODO at 649 suggests the second value (or … |  | [ ] |
| [SS3-40](#ss3-40) | 148 | L/M | logic | `amat` becomes the *last* age whose fecundity is below the threshold, i.e. an immature age, not the first matu… |  | [ ] |

---

## SS3-26

**swapped dims (alloc convention)** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Found at (current code):** `atCloseKin.c` 78, `atCloseKin.c` 87
**Lines at review (Oct 2026):** 77-100 (uses 122-487)

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

All local arrays are allocated with dims in *index* order instead of reversed. `nsamps_sya = Alloc_3D(Nsexes,nsampy,MaxAge)` gives `[MaxAge][nsampy][Nsexes]` but is used `[s][y][a]`. `inv_totfec_sy` gives `[last_dy][Nsexes]` and is used `[s][y]`. `n_sya`, `psurv_syay`, `Pr_PO/HS_syaya`, `ncomps_PO_syaya`, `Ekin_*`, `sim_nkin_*` (giving `[MaxAge][last_sy][MaxAge][last_sy][Nsexes]`) and `Pr_GG/HS_yaya` and `ncomps_HS_yaya` (giving `[MaxAge][last_sy][MaxAge][last_sy]`) are all indexed the other way round. Any index past a dimension's size goes out of bounds whenever Nsexes, last_sy and MaxAge differ. The TODO at line 76 flags this. The same pattern is at atAssessTierSetup.c:541-554. Function: `CKsimulator()`.

**Code at issue** — `atCloseKin.c` line 78 (closest match to the quoted code; wording differs slightly)  
Search for: `double ***nsamps_sya = Util_Alloc_Init_3D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nse`

```c
   76:     //TODO: CHECK ORDER OF DIMENSIONS IN THESE DECLARARIONS VS THEIR USE
   77:     double *nsamps_y = Util_Alloc_Init_1D_Double(nsampy, 0.0);
   78→     double ***nsamps_sya = Util_Alloc_Init_3D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], nsampy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
   79: 
   80:     double *x = Util_Alloc_Init_1D_Double(bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
```

**Code at issue** — `atCloseKin.c` line 87 (closest match to the quoted code; wording differs slightly)  
Search for: `double *****Pr_PO_syaya = Util_Alloc_Init_5D_Double(bm->RBCestimation.RBCspeciesParam[sp][`

```c
   85:     double ****psurv_syay = Util_Alloc_Init_4D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], last_dy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], last_dy, 0.0);
   86:     
   87→     double *****Pr_PO_syaya = Util_Alloc_Init_5D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
   88:     double *****Pr_HS_syaya = Util_Alloc_Init_5D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
   89: 
```

**Suggested fix (from review):** Reverse every dimension list, e.g. `Alloc_3D(MaxAge, nsampy, Nsexes)`

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

## SS3-27

**OOB** · Conf M / Sev M  
**Found at (current code):** `atCloseKin.c` 55, `atCloseKin.c` 425
**Lines at review (Oct 2026):** 77, 121-122, 165-177

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

`nsamps_y` and `nsamps_sya` are sized `nsampy = last_sy-first_sy+1` but indexed with absolute `y` up to `last_sy-1`. This overflows when `first_sy >= 2`. Function: `CKsimulator()`.

**Code at issue** — `atCloseKin.c` line 55 (closest match to the quoted code; wording differs slightly)  
Search for: `int nsampy = bm->CloseKinEst[sp].last_sy - bm->CloseKinEst[sp].first_sy + 1;`

```c
   53: 
   54:     int last_dy = bm->CloseKinEst[sp].last_sy;
   55→     int nsampy = bm->CloseKinEst[sp].last_sy - bm->CloseKinEst[sp].first_sy + 1;
   56:     //int Asamp_propto = 0; //0 for catch
   57:     //int age_power = 1;
```

**Code at issue** — `atCloseKin.c` line 425  
Search for: `last_sy - 1`

```c
  423:                         comps = nsamps1 * nsamps2;
  424: 
  425→                         if ((y == (bm->CloseKinEst[sp].last_sy - 1)) && (a == (bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id] - 1)) && (y2 == (bm->CloseKinEst[sp].last_sy - 2)) && (a2 == (bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id] - 2))) {
  426:                             fprintf(bm->logFile, "Time: %e %s nsamps_sya[FEMALE][%d][%d]: %e nsamps_sya[MALE]: %e nsamps1: %e nsamps_sya[FEMALE][%d][%d]: %e nsamps_sya[MALE]: %e nsamps2: %e comps: %e\n", bm->dayt, FunctGroupArray[sp].groupCode, y, a, nsamps_sya[FEMALE_ID][y][a], nsamps_sya[MALE_ID][y][a], nsamps1, y2, a2, nsamps_sya[FEMALE_ID][y2][a2], nsamps_sya[MALE_ID][y2][a2], nsamps2, comps);
  427:                         }
```

**Suggested fix (from review):** Size by `last_sy` (like the other arrays) or index `y-first_sy`

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

## SS3-28

**year value as index** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atCloseKin.c` 140, `atCloseKin.c` 72
**Lines at review (Oct 2026):** 72, 140

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

`FracLenS[j][s][a][this_min][l]` with `this_min = HistYrMin_id` uses a calendar year (e.g. 1990) as the year index. FracLenS's year dimension is nYears (atAssessTierSetup.c:279), so this reads far out of bounds. Function: `CKsimulator()`.

**Code at issue** — `atCloseKin.c` line 140  
Search for: `FracLenS[j][s][a][this_min][l]`

```c
  138:                 bm->CloseKinEst[sp].fec_sa[s][a] = 0;
  139:                 for (int l = 0; l < bm->RBCestimation.RBCspeciesParam[sp][Nlen_id]; l++) {
  140→                     bm->CloseKinEst[sp].fec_sa[s][a] += bm->RBCestimation.RBCspeciesArray[sp].FracLenS[j][s][a][this_min][l] * bm->CloseKinEst[sp].mature[l];
  141:                 }
  142:             }
```

**Code at issue** — `atCloseKin.c` line 72 (closest match to the quoted code; wording differs slightly)  
Search for: `int this_min = (int)(bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]);`

```c
   70:     double comps = 0, nsamps1 = 0, nsamps2 = 0;
   71:     int nLinesKinPO = 0, nLinesKinHS = 0;
   72→     int this_min = (int)(bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]);
   73:     int this_maxage = (int)(bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]);
   74:         
```

**Suggested fix (from review):** Use a year *index* (e.g. 0 or `year`)

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

## SS3-29

**overwritten per stock** · Conf M / Sev L  
**Found at (current code):** `atCloseKin.c` 133, `atCloseKin.c` 138
**Lines at review (Oct 2026):** 130-144

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

`mature[l]` and `fec_sa[s][a]` are recomputed (=0 then +=) for each stock `j`, so only the last stock's fecundity survives. Function: `CKsimulator()`.

**Code at issue** — `atCloseKin.c` line 133  
Search for: `mature[l]`

```c
  131:     // Maturity as a function of length
  132:         for (l = 0 ; l < bm->RBCestimation.RBCspeciesParam[sp][Nlen_id]; l++) {
  133→             bm->CloseKinEst[sp].mature[l] = 1.0 / (1.0 + exp(bm->RBCestimation.RBCspeciesArray[sp].SlopeMat[j] * (bm->CloseKinEst[sp].lengths[l] - bm->RBCestimation.RBCspeciesArray[sp].Mat50[j])));
  134:         }
  135: 
```

**Code at issue** — `atCloseKin.c` line 138  
Search for: `fec_sa[s][a]`

```c
  136:         for (s = 0; s < bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id]; s++){
  137:             for (a = 0; a < bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]; a++) {
  138→                 bm->CloseKinEst[sp].fec_sa[s][a] = 0;
  139:                 for (int l = 0; l < bm->RBCestimation.RBCspeciesParam[sp][Nlen_id]; l++) {
  140:                     bm->CloseKinEst[sp].fec_sa[s][a] += bm->RBCestimation.RBCspeciesArray[sp].FracLenS[j][s][a][this_min][l] * bm->CloseKinEst[sp].mature[l];
```

**Suggested fix (from review):** Accumulate across stocks (weighted) or add a stock dimension

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

## SS3-30

**use after close** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atCloseKin.c` 162
**Lines at review (Oct 2026):** 154-163, 490-498

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

`Util_Close_Output_File(bm->closekinCFile)` and the closes of the samp/PO/HS files fclose the stream, but the bm pointers are not set to NULL (atUtil.c:200 takes `FILE*` by value). GenData calls CKsimulator every year, so from the second call on it fprintf's to a closed FILE* (UB/crash). Function: `CKsimulator()`.

**Code at issue** — `atCloseKin.c` line 162  
Search for: `Util_Close_Output_File(bm->closekinCFile)`

```c
  160:     //writeCloseKinCatchFile(bm, year, sp, bm->closekinCFile);
  161:     writeCloseKinSampFile(bm, year, sp, bm->closekinSampFile);
  162→     Util_Close_Output_File(bm->closekinCFile);
  163:     Util_Close_Output_File(bm->closekinSampFile);
  164:     
```

**Suggested fix (from review):** Close only at model end, or set the pointers to NULL after closing

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

## SS3-31

**off-by-one OOB** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atCloseKin.c` 241, `atCloseKin.c` 73, `atCloseKin.c` 344
**Lines at review (Oct 2026):** 241, 270-273, 343-353

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

`psurv_syay[s][y-1][this_maxage][y]` with `this_maxage = MaxAge`, `curr_a` allowed to reach MaxAge (`if (curr_a < MaxAge) curr_a++`), and `ap_b2 = MaxAge` (then `fec_sa[s][ap_b2]`) all index one past the age extent MaxAge (fec_sa: atAssessTierSetup.c:271). Function: `CKsimulator()`.

**Code at issue** — `atCloseKin.c` line 241  
Search for: `psurv_syay[s][y-1][this_maxage][y]`

```c
  239:             }
  240: 
  241→             psurv_syay[s][y-1][this_maxage][y] = n_sya[s][y][this_maxage] / n_sya[s][y- 1][this_maxage-1];
  242: 
  243:             /*
```

**Code at issue** — `atCloseKin.c` line 73 (closest match to the quoted code; wording differs slightly)  
Search for: `int this_maxage = (int)(bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]);`

```c
   71:     int nLinesKinPO = 0, nLinesKinHS = 0;
   72:     int this_min = (int)(bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]);
   73→     int this_maxage = (int)(bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]);
   74:         
   75:     /* Leave here for now */
```

**Code at issue** — `atCloseKin.c` line 344 (closest match to the quoted code; wording differs slightly)  
Search for: `ap_b2 = bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id];`

```c
  342:                                     int ap_b2 = 0;
  343:                                     if ((ap_b1 + (b2 - b1)) > bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]) {
  344→                                         ap_b2 = bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id];
  345:                                     } else {
  346:                                         ap_b2 = ap_b1 + (b2 - b1);
```

**Suggested fix (from review):** Use the plus group `MaxAge-1`, or allocate MaxAge+1

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

## SS3-32

**accumulator not reset** · Conf H / Sev M  
**Found at (current code):** `atCloseKin.c` 294
**Lines at review (Oct 2026):** 288-299

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

`stocktot` is never reset inside the s/y loops, so `inv_totfec_sy` becomes 1/(cumulative total over all earlier years and sexes). Function: `CKsimulator()`.

**Code at issue** — `atCloseKin.c` line 294 (the line the review pointed to, found by its text)  
Search for: `inv_totfec_sy[s][y] = 1 / stocktot;`

```c
  292:             }
  293:             if (stocktot > 0) {
  294→                 inv_totfec_sy[s][y] = 1 / stocktot;
  295:             } else {
  296:                 fprintf(bm->logFile, "Time: %e %s  CKsim stocktot <= 0\n", bm->dayt, FunctGroupArray[sp].groupCode);
```

**Suggested fix (from review):** Set `stocktot = 0` at the top of the y loop

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

## SS3-33

**off-by-one loop start** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atCloseKin.c` 382
**Lines at review (Oct 2026):** 382

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

`for (int s1 = 1; s1 < Nsexes; s1++) comps += ...` skips offspring sex 0, although the comment says "sex of offspring doesn't matter". With 1 sex, comps is 0 and no PO comparisons are generated. Function: `CKsimulator()`.

**Code at issue** — `atCloseKin.c` line 382 (closest match to the quoted code; wording differs slightly)  
Search for: `for (int s1 = 1; s1 < bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id]; s1++)`

```c
  380:                         b2 = y2 - a2;
  381:                         comps = 0;
  382→                         for (int s1 = 1; s1 < bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id]; s1++)
  383:                             comps += nsamps1 * nsamps_sya[s1][y2][a2];  // sex of offspring doesn't matter
  384: 
```

**Suggested fix (from review):** Start `s1 = 0`

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

## SS3-34

**swapped arguments** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atCloseKin.c` 496, `atCloseKin.c` 161, `atCloseKin.c` 472
**Lines at review (Oct 2026):** 496 vs 630

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

The call `writeCloseKinNcompsfiles(bm, sp, year, ...)` does not match the signature `(bm, int year, int sp, ...)`. `year` is used as the species index into `CloseKinEst[sp]` and `FunctGroupArray[sp]` (OOB), and the species is printed as the year. Function: `CKsimulator()`.

**Code at issue** — `atCloseKin.c` line 496  
Search for: `writeCloseKinNcompsfiles(bm, sp, year,`

```c
  494:         bm->CloseKinHSFile = initCloseKinHSFile(bm);
  495:     }
  496→     writeCloseKinNcompsfiles(bm, sp, year, bm->CloseKinPOFile,  bm->CloseKinHSFile, sim_nkin_HS, sim_nkin_PO, ncomps_HS_yaya, ncomps_PO_syaya);
  497:     Util_Close_Output_File(bm->CloseKinPOFile);
  498:     Util_Close_Output_File(bm->CloseKinHSFile);
```

**Code at issue** — `atCloseKin.c` line 161 (closest match to the quoted code; wording differs slightly)  
Search for: `writeCloseKinSampFile(bm, year, sp, bm->closekinSampFile);`

```c
  159:     }
  160:     //writeCloseKinCatchFile(bm, year, sp, bm->closekinCFile);
  161→     writeCloseKinSampFile(bm, year, sp, bm->closekinSampFile);
  162:     Util_Close_Output_File(bm->closekinCFile);
  163:     Util_Close_Output_File(bm->closekinSampFile);
```

**Code at issue** — `atCloseKin.c` line 472  
Search for: `CloseKinEst[sp]`

```c
  470:     for (y = bm->CloseKinEst[sp].first_sy; y < bm->CloseKinEst[sp].last_sy; y++) {    // NB skip plus-group since its earlier age is uncertain
  471:         for (a = 0; a < bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]; a++) {
  472→             for (y2 = bm->CloseKinEst[sp].first_sy; y2 < bm->CloseKinEst[sp].last_sy; y2++) {
  473:                 for (a2 = 0; a2 < bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]; a2++) {
  474:                     for (s = 0; s < bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id]; s++) {
```

**Suggested fix (from review):** Call `(bm, year, sp, ...)`

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

## SS3-35

**uninitialised per-species params** · Conf M / Sev H  
**Found at (current code):** `atCloseKin.c` 55
**Lines at review (Oct 2026):** 54-55, 109-121, 171, 590-591

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

The code reads `CloseKinEst[sp].first_sy/last_sy/first_dy/samp_power/a_ck_thresh...`, but these are only read into element 0 (`bm->CloseKinEst->first_sy`, atassessParamIO.c:1088-1094). The struct array is `malloc`'d uninitialised (atassessParamIO.c:619), so for sp≠0 these are garbage loop bounds and sizes. Function: `CKsimulator()`.

**Code at issue** — `atCloseKin.c` line 55 (closest match to the quoted code; wording differs slightly)  
Search for: `int nsampy = bm->CloseKinEst[sp].last_sy - bm->CloseKinEst[sp].first_sy + 1;`

```c
   53: 
   54:     int last_dy = bm->CloseKinEst[sp].last_sy;
   55→     int nsampy = bm->CloseKinEst[sp].last_sy - bm->CloseKinEst[sp].first_sy + 1;
   56:     //int Asamp_propto = 0; //0 for catch
   57:     //int age_power = 1;
```

**Suggested fix (from review):** Read the values per species, or use `CloseKinEst[0]` consistently

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

## SS3-36

**division by zero** · Conf M / Sev M  
**Found at (current code):** `atCloseKin.c` 238
**Lines at review (Oct 2026):** 238

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

`n_sya[s][y][a] / n_sya[s][y-1][a-1]` has no guard. Zero numbers give inf or NaN survival, which then propagates into psurv and Pr_HS. Function: `CKsimulator()`.

**Code at issue** — `atCloseKin.c` line 238  
Search for: `n_sya[s][y][a] / n_sya[s][y-1][a-1]`

```c
  236: 
  237:             for (a = 1; a < bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]; a++){
  238→                   psurv_syay[s][y-1][a-1][y] = n_sya[s][y][a] / n_sya[s][y-1][a-1];
  239:             }
  240: 
```

**Suggested fix (from review):** Guard the denominator

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

## SS3-37

**brace placement** · Conf M / Sev L  
**Found at (current code):** `atCloseKin.c` 355
**Lines at review (Oct 2026):** 337-358

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

The `} // for ap_b1` brace actually closes the `if`, so `Pr_HS_syaya = demog_Pr_HSP; Pr_HS_yaya += ...` runs on every ap_b1 iteration. Pr_HS_yaya accumulates partial sums. Pr_HS_syaya ends up correct, and Pr_HS_yaya is unused. Function: `CKsimulator()`.

**Code at issue** — `atCloseKin.c` line 355  
Search for: `} // for ap_b1`

```c
  353:                                         demog_Pr_HSP +=    Pr_par1_was_ap_b1 * Pr_par2_is_par1_if_alive * psurv_syay[s][b1][ap_b1][b2];  // if b2>b1 this will be zero instead of 1; but I think those cases are overwritten anyway
  354: 
  355→                                     } // for ap_b1
  356:     
  357:                                     Pr_HS_syaya[s][y][a][y2][a2] = demog_Pr_HSP; // *bm->CloseKinEst.bm->CloseKinEst.HSP_false_neg_loss_rate; // moved to Likelihood calcs
```

**Suggested fix (from review):** Move both statements after the ap_b1 loop

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

## SS3-38

**wrong index / compounding** · Conf M / Sev M  
**Found at (current code):** `atCloseKin.c` 589
**Lines at review (Oct 2026):** 589-591

**Function:** `writeCloseKinSampFile()`

**Why it's thought to be a bug**

`ageprops[s][a]` uses sex `s` as the stock index; ageprops is `[stock][age]`. Also `samp_prop_to += ...` is never reset, and the `*= a_ck_scalar` scaling is reapplied to the cumulative value on every call, so it compounds. Function: `writeCloseKinSampFile()`.

**Code at issue** — `atCloseKin.c` line 589  
Search for: `ageprops[s][a]`

```c
  587:                 }
  588:                 */
  589→                 bm->CloseKinEst[sp].samp_prop_to[s][y][a] += bm->RBCestimation.RBCspeciesArray[sp].ageprops[s][a] *  FunctGroupArray[sp].speciesParams[samplesize_id];
  590:                 if (a <  bm->CloseKinEst[sp].a_ck_thresh) bm->CloseKinEst[sp].samp_prop_to[s][y][a] *= bm->CloseKinEst[sp].a_ck_scalar;
  591:                 else if (a < bm->CloseKinEst[sp].a_ck_thresh2) bm->CloseKinEst[sp].samp_prop_to[s][y][a] *= bm->CloseKinEst[sp].a_ck_scalar2;
```

**Suggested fix (from review):** Use a stock index, reset per call, and scale only the new increment

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

## SS3-39

**copy-paste** · Conf M / Sev L  
**Found at (current code):** `atCloseKin.c` 645, `atCloseKin.c` 649
**Lines at review (Oct 2026):** 645, 647

**Function:** `writeCloseKinNcompsfiles()`

**Why it's thought to be a bug**

The HS output prints `(int)sim_nkin_HS[s][y][a][y2][a2]` twice. The TODO at 649 suggests the second value (or the swapped `[y2][a2][y][a]` entry) was intended to differ. Function: `writeCloseKinNcompsfiles()`.

**Code at issue** — `atCloseKin.c` line 645  
Search for: `(int)sim_nkin_HS[s][y][a][y2][a2]`

```c
  643:                      
  644:                          if (b1<b2) {
  645→                              fprintf(fidHS, "%e %d %s %d %d %d %d %d %d %d %e\n", bm->dayt, year, FunctGroupArray[sp].groupCode, s, y, a, y2, a2, ((int)sim_nkin_HS[s][y][a][y2][a2]), ((int)sim_nkin_HS[s][y][a][y2][a2]), ncomps_HS_yaya[y][a][y2][a2]);
  646:                         } else {
  647:                             fprintf(fidHS, "%e %d %s %d %d %d %d %d %d %d %e\n", bm->dayt, year, FunctGroupArray[sp].groupCode, s, y2, a2, y, a, ((int)sim_nkin_HS[s][y][a][y2][a2]), ((int)sim_nkin_HS[s][y][a][y2][a2]), ncomps_HS_yaya[y][a][y2][a2]);
```

**Code at issue** — `atCloseKin.c` line 649  
Search for: `[y2][a2][y][a]`

```c
  647:                             fprintf(fidHS, "%e %d %s %d %d %d %d %d %d %d %e\n", bm->dayt, year, FunctGroupArray[sp].groupCode, s, y2, a2, y, a, ((int)sim_nkin_HS[s][y][a][y2][a2]), ((int)sim_nkin_HS[s][y][a][y2][a2]), ncomps_HS_yaya[y][a][y2][a2]);
  648:                             
  649→                             //TODO: ASK Robin if this should be ((int)sim_nkin_HS[s][y2][a2][y][a]), ((int)sim_nkin_HS[s][y2][a2][y][a]).....
  650:                         }
  651:                     }
```

**Suggested fix (from review):** Print the intended pair

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

## SS3-40

**logic** · Conf L / Sev M  
**Found at (current code):** `atCloseKin.c` 148
**Lines at review (Oct 2026):** 146-151

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

`amat` becomes the *last* age whose fecundity is below the threshold, i.e. an immature age, not the first mature age. Function: `CKsimulator()`.

**Code at issue** — `atCloseKin.c` line 148 (the line the review pointed to, found by its text)  
Search for: `if (bm->CloseKinEst[sp].fec_sa[FEMALE_ID][a] < bm->RBCestimation.RBCspeciesParam[sp][thres`

```c
  146:     int amat = 0;
  147:     for (a = 0; a < bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]; a++) {
  148→         if (bm->CloseKinEst[sp].fec_sa[FEMALE_ID][a] < bm->RBCestimation.RBCspeciesParam[sp][thresh_mat_id]) {
  149:             amat = a;
  150:         }
```

**Suggested fix (from review):** Take the first `a` with `fec_sa >= thresh`

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
