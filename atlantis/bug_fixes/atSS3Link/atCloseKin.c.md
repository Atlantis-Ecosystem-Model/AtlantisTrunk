# atSS3Link/atCloseKin.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **15 findings** (severity H: 7; confidence H and severity H: 6; on the priority list: 6).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SS3-26](#ss3-26) ✔ ⭐ | 77-100 (uses 122-487) | H/H | swapped dims (alloc convention) | All local arrays are allocated with dims in *index* order instead of reversed. `nsamps_sya = Alloc_3D(Nsexes,n… |  | [ ] |
| [SS3-27](#ss3-27) | 77, 121-122, 165-177 | M/M | OOB | `nsamps_y` and `nsamps_sya` are sized `nsampy = last_sy-first_sy+1` but indexed with absolute `y` up to `last_… |  | [ ] |
| [SS3-28](#ss3-28) ⭐ | 72, 140 | H/H | year value as index | `FracLenS[j][s][a][this_min][l]` with `this_min = HistYrMin_id` uses a calendar year (e.g. 1990) as the year i… |  | [ ] |
| [SS3-29](#ss3-29) | 130-144 | M/L | overwritten per stock | `mature[l]` and `fec_sa[s][a]` are recomputed (=0 then +=) for each stock `j`, so only the last stock's fecund… |  | [ ] |
| [SS3-30](#ss3-30) ⭐ | 154-163, 490-498 | H/H | use after close | `Util_Close_Output_File(bm->closekinCFile)` and the closes of the samp/PO/HS files fclose the stream, but the … |  | [ ] |
| [SS3-31](#ss3-31) ⭐ | 241, 270-273, 343-353 | H/H | off-by-one OOB | `psurv_syay[s][y-1][this_maxage][y]` with `this_maxage = MaxAge`, `curr_a` allowed to reach MaxAge (`if (curr_… |  | [ ] |
| [SS3-32](#ss3-32) | 288-299 | H/M | accumulator not reset | `stocktot` is never reset inside the s/y loops, so `inv_totfec_sy` becomes 1/(cumulative total over all earlie… |  | [ ] |
| [SS3-33](#ss3-33) ⭐ | 382 | H/H | off-by-one loop start | `for (int s1 = 1; s1 < Nsexes; s1++) comps += ...` skips offspring sex 0, although the comment says "sex of of… |  | [ ] |
| [SS3-34](#ss3-34) ⭐ | 496 vs 630 | H/H | swapped arguments | The call `writeCloseKinNcompsfiles(bm, sp, year, ...)` does not match the signature `(bm, int year, int sp, ..… |  | [ ] |
| [SS3-35](#ss3-35) | 54-55, 109-121, 171, 590-591 | M/H | uninitialised per-species params | The code reads `CloseKinEst[sp].first_sy/last_sy/first_dy/samp_power/a_ck_thresh...`, but these are only read … |  | [ ] |
| [SS3-36](#ss3-36) | 238 | M/M | division by zero | `n_sya[s][y][a] / n_sya[s][y-1][a-1]` has no guard. Zero numbers give inf or NaN survival, which then propagat… |  | [ ] |
| [SS3-37](#ss3-37) | 337-358 | M/L | brace placement | The `} // for ap_b1` brace actually closes the `if`, so `Pr_HS_syaya = demog_Pr_HSP; Pr_HS_yaya += ...` runs o… |  | [ ] |
| [SS3-38](#ss3-38) | 589-591 | M/M | wrong index / compounding | `ageprops[s][a]` uses sex `s` as the stock index; ageprops is `[stock][age]`. Also `samp_prop_to += ...` is ne… |  | [ ] |
| [SS3-39](#ss3-39) | 645, 647 | M/L | copy-paste | The HS output prints `(int)sim_nkin_HS[s][y][a][y2][a2]` twice. The TODO at 649 suggests the second value (or … |  | [ ] |
| [SS3-40](#ss3-40) | 146-151 | L/M | logic | `amat` becomes the *last* age whose fecundity is below the threshold, i.e. an immature age, not the first matu… |  | [ ] |

---

## SS3-26

**swapped dims (alloc convention)** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Lines (current code):** 77-100 (uses 122-487)  
**Lines at review (Oct 2026):** 77-100 (uses 122-487)

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

All local arrays are allocated with dims in *index* order instead of reversed. `nsamps_sya = Alloc_3D(Nsexes,nsampy,MaxAge)` gives `[MaxAge][nsampy][Nsexes]` but is used `[s][y][a]`. `inv_totfec_sy` gives `[last_dy][Nsexes]` and is used `[s][y]`. `n_sya`, `psurv_syay`, `Pr_PO/HS_syaya`, `ncomps_PO_syaya`, `Ekin_*`, `sim_nkin_*` (giving `[MaxAge][last_sy][MaxAge][last_sy][Nsexes]`) and `Pr_GG/HS_yaya` and `ncomps_HS_yaya` (giving `[MaxAge][last_sy][MaxAge][last_sy]`) are all indexed the other way round. Any index past a dimension's size goes out of bounds whenever Nsexes, last_sy and MaxAge differ. The TODO at line 76 flags this. The same pattern is at atAssessTierSetup.c:541-554. Function: `CKsimulator()`.

**Current code** (`atSS3Link/atCloseKin.c` lines 75–114 (truncated))

```c
   75:     /* Leave here for now */
   76:     //TODO: CHECK ORDER OF DIMENSIONS IN THESE DECLARARIONS VS THEIR USE
   77:     double *nsamps_y = Util_Alloc_Init_1D_Double(nsampy, 0.0);
   78:     double ***nsamps_sya = Util_Alloc_Init_3D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], nsampy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
   79: 
   80:     double *x = Util_Alloc_Init_1D_Double(bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
   81:     double *sx = Util_Alloc_Init_1D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], 0.0);
   82:     double **inv_totfec_sy = Util_Alloc_Init_2D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], last_dy, 0.0);
   83: 
   84:     double ***n_sya = Util_Alloc_Init_3D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], last_dy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
   85:     double ****psurv_syay = Util_Alloc_Init_4D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], last_dy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], last_dy, 0.0);
   86:     
   87:     double *****Pr_PO_syaya = Util_Alloc_Init_5D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
   88:     double *****Pr_HS_syaya = Util_Alloc_Init_5D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
   89: 
   90:     double ****Pr_GG_yaya = Util_Alloc_Init_4D_Double(bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
   91:     double ****Pr_HS_yaya = Util_Alloc_Init_4D_Double(bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
   92: 
   93:     double *****ncomps_PO_syaya = Util_Alloc_Init_5D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
   94:     double ****ncomps_HS_yaya = Util_Alloc_Init_4D_Double(bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
   95:     
   96:     double *****Ekin_HS_syaya = Util_Alloc_Init_5D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
   97:     double *****Ekin_PO_syaya = Util_Alloc_Init_5D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
   98:     
   99:     double *****sim_nkin_PO = Util_Alloc_Init_5D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
  100:     double *****sim_nkin_HS = Util_Alloc_Init_5D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
  101: 
  102:     
  103:     Util_Init_1D_Double(bm->CloseKinEst[sp].fec_expo_s, bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], 1.0);
  104:     Util_Init_2D_Double(bm->CloseKinEst[sp].fec_sa, bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id],  0.0);
  105:     Util_Init_1D_Double(bm->CloseKinEst[sp].mature, bm->RBCestimation.RBCspeciesParam[sp][Nlen_id], 0.0);
  106:     Util_Init_1D_Double(bm->CloseKinEst[sp].lengths, bm->RBCestimation.RBCspeciesParam[sp][Nlen_id], 0.0);
  107: 
  108:     /**
  109:     for (y = bm->CloseKinEst[sp].first_sy; y < bm->CloseKinEst[sp].last_sy; y++) {
  110:         nsamps_y[y] = int(bm->CloseKinEst.nTotSamps / nsampy);
  111:         fprintf(bm->logFile, "Time: %e %s CKsimulator: nsamps_y[%d] : %d\n", bm->dayt, FunctGroupArray[sp].groupCode, y, nsamps_y[y]);
  112:     }
  113:      **/
  114:     
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
**Lines (current code):** 77, 121-122, 165-177  
**Lines at review (Oct 2026):** 77, 121-122, 165-177

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

`nsamps_y` and `nsamps_sya` are sized `nsampy = last_sy-first_sy+1` but indexed with absolute `y` up to `last_sy-1`. This overflows when `first_sy >= 2`. Function: `CKsimulator()`.

**Current code** (`atSS3Link/atCloseKin.c` lines 75–79)

```c
   75:     /* Leave here for now */
   76:     //TODO: CHECK ORDER OF DIMENSIONS IN THESE DECLARARIONS VS THEIR USE
   77:     double *nsamps_y = Util_Alloc_Init_1D_Double(nsampy, 0.0);
   78:     double ***nsamps_sya = Util_Alloc_Init_3D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], nsampy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
   79: 
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
**Lines (current code):** 72, 140  
**Lines at review (Oct 2026):** 72, 140

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

`FracLenS[j][s][a][this_min][l]` with `this_min = HistYrMin_id` uses a calendar year (e.g. 1990) as the year index. FracLenS's year dimension is nYears (atAssessTierSetup.c:279), so this reads far out of bounds. Function: `CKsimulator()`.

**Current code** (`atSS3Link/atCloseKin.c` lines 70–74)

```c
   70:     double comps = 0, nsamps1 = 0, nsamps2 = 0;
   71:     int nLinesKinPO = 0, nLinesKinHS = 0;
   72:     int this_min = (int)(bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]);
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
**Lines (current code):** 130-144  
**Lines at review (Oct 2026):** 130-144

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

`mature[l]` and `fec_sa[s][a]` are recomputed (=0 then +=) for each stock `j`, so only the last stock's fecundity survives. Function: `CKsimulator()`.

**Current code** (`atSS3Link/atCloseKin.c` lines 128–146)

```c
  128:     }
  129: 
  130:     for (j = 0; j < FunctGroupArray[sp].numStocks; j++){
  131:     // Maturity as a function of length
  132:         for (l = 0 ; l < bm->RBCestimation.RBCspeciesParam[sp][Nlen_id]; l++) {
  133:             bm->CloseKinEst[sp].mature[l] = 1.0 / (1.0 + exp(bm->RBCestimation.RBCspeciesArray[sp].SlopeMat[j] * (bm->CloseKinEst[sp].lengths[l] - bm->RBCestimation.RBCspeciesArray[sp].Mat50[j])));
  134:         }
  135: 
  136:         for (s = 0; s < bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id]; s++){
  137:             for (a = 0; a < bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]; a++) {
  138:                 bm->CloseKinEst[sp].fec_sa[s][a] = 0;
  139:                 for (int l = 0; l < bm->RBCestimation.RBCspeciesParam[sp][Nlen_id]; l++) {
  140:                     bm->CloseKinEst[sp].fec_sa[s][a] += bm->RBCestimation.RBCspeciesArray[sp].FracLenS[j][s][a][this_min][l] * bm->CloseKinEst[sp].mature[l];
  141:                 }
  142:             }
  143:         }
  144:     }
  145: 
  146:     int amat = 0;
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
**Lines (current code):** 154-163, 490-498  
**Lines at review (Oct 2026):** 154-163, 490-498

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

`Util_Close_Output_File(bm->closekinCFile)` and the closes of the samp/PO/HS files fclose the stream, but the bm pointers are not set to NULL (atUtil.c:200 takes `FILE*` by value). GenData calls CKsimulator every year, so from the second call on it fprintf's to a closed FILE* (UB/crash). Function: `CKsimulator()`.

**Current code** (`atSS3Link/atCloseKin.c` lines 152–165)

```c
  152: 
  153:     // Writeout closekin catch
  154:     if(!bm->closekinCFile) {
  155:         bm->closekinCFile = initCloseKinCatchFile(bm);
  156:     }
  157:     if(!bm->closekinSampFile) {
  158:         bm->closekinSampFile = initCloseKinSampFile(bm);
  159:     }
  160:     //writeCloseKinCatchFile(bm, year, sp, bm->closekinCFile);
  161:     writeCloseKinSampFile(bm, year, sp, bm->closekinSampFile);
  162:     Util_Close_Output_File(bm->closekinCFile);
  163:     Util_Close_Output_File(bm->closekinSampFile);
  164:     
  165:     for (y = bm->CloseKinEst[sp].first_sy; y < bm->CloseKinEst[sp].last_sy; y++){
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
**Lines (current code):** 241, 270-273, 343-353  
**Lines at review (Oct 2026):** 241, 270-273, 343-353

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

`psurv_syay[s][y-1][this_maxage][y]` with `this_maxage = MaxAge`, `curr_a` allowed to reach MaxAge (`if (curr_a < MaxAge) curr_a++`), and `ap_b2 = MaxAge` (then `fec_sa[s][ap_b2]`) all index one past the age extent MaxAge (fec_sa: atAssessTierSetup.c:271). Function: `CKsimulator()`.

**Current code** (`atSS3Link/atCloseKin.c` lines 239–275)

```c
  239:             }
  240: 
  241:             psurv_syay[s][y-1][this_maxage][y] = n_sya[s][y][this_maxage] / n_sya[s][y- 1][this_maxage-1];
  242: 
  243:             /*
  244:             for (a = 1;a < bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]; a++) {
  245:                 fprintf(bm->logFile, "surv_syay[s][y-1][a-1][y] ", s, y-1, a-1, y, surv_syay[s][y-1][a-1][y]);
  246:             }
  247:             fprintf(bm->logFile, "\n");
  248:             */
  249:         }
  250:     }
  251: 
  252:     //multi-year surv probs have already done one year ahead, so just multiply psurv_syay
  253:     //fprintf(bm->logFile, "Time: %e %s CKsimulator: ", bm->dayt, FunctGroupArray[sp].groupCode);
  254:     for (s = 0; s < bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id]; s++){
  255:         //fprintf(bm->logFile, "s %d  ", s);
  256:         for (y = bm->CloseKinEst[sp].first_dy; y < last_dy - 1; y++) {
  257:             /*
  258:             fprintf(bm->logFile, "y %d ", y);
  259:             for (y2 = y; y2 < last_dy; y2++) {
  260:                 fprintf(bm->logFile, "y2 %d", y2);
  261:             }
  262:             fprintf(bm->logFile, "\n");
  263:             */
  264:             
  265:             for (a = 0; a < bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]; a++){
  266:                 psurv_syay[s][y][a][y] = 1; //diagonal
  267:                 cumul_psurv = 1;
  268:                 curr_a = a;
  269:                 for (y2 = y+1; y2 < last_dy; y2++) {
  270:                     cumul_psurv *= psurv_syay[s][y2-1][curr_a][y2];
  271:                     psurv_syay[s][y][curr_a][y2] = cumul_psurv;
  272:                     if (curr_a < bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]) {
  273:                         curr_a++; // ROBIN what does curr_a do?
  274:                     }
  275:                 }
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
**Lines (current code):** 288-299  
**Lines at review (Oct 2026):** 288-299

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

`stocktot` is never reset inside the s/y loops, so `inv_totfec_sy` becomes 1/(cumulative total over all earlier years and sexes). Function: `CKsimulator()`.

**Current code** (`atSS3Link/atCloseKin.c` lines 286–301)

```c
  286:     }
  287: 
  288:     for (s = 0; s < bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id]; s++){
  289:         for (y = bm->CloseKinEst[sp].first_dy + 1; y < last_dy; y++){
  290:             for (a = 0; a < bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]; a++){
  291:                 stocktot += n_sya[s][y][a] * bm->CloseKinEst[sp].fec_sa[s][a];
  292:             }
  293:             if (stocktot > 0) {
  294:                 inv_totfec_sy[s][y] = 1 / stocktot;
  295:             } else {
  296:                 fprintf(bm->logFile, "Time: %e %s  CKsim stocktot <= 0\n", bm->dayt, FunctGroupArray[sp].groupCode);
  297:                 //return;  // If this return is real remember to empty out the arrays
  298:             }
  299:         }
  300:     }
  301: 
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
**Lines (current code):** 382  
**Lines at review (Oct 2026):** 382

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

`for (int s1 = 1; s1 < Nsexes; s1++) comps += ...` skips offspring sex 0, although the comment says "sex of offspring doesn't matter". With 1 sex, comps is 0 and no PO comparisons are generated. Function: `CKsimulator()`.

**Current code** (`atSS3Link/atCloseKin.c` lines 380–384)

```c
  380:                         b2 = y2 - a2;
  381:                         comps = 0;
  382:                         for (int s1 = 1; s1 < bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id]; s1++)
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
**Lines (current code):** 496 vs 630  
**Lines at review (Oct 2026):** 496 vs 630

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

The call `writeCloseKinNcompsfiles(bm, sp, year, ...)` does not match the signature `(bm, int year, int sp, ...)`. `year` is used as the species index into `CloseKinEst[sp]` and `FunctGroupArray[sp]` (OOB), and the species is printed as the year. Function: `CKsimulator()`.

**Current code** (`atSS3Link/atCloseKin.c` lines 494–498)

```c
  494:         bm->CloseKinHSFile = initCloseKinHSFile(bm);
  495:     }
  496:     writeCloseKinNcompsfiles(bm, sp, year, bm->CloseKinPOFile,  bm->CloseKinHSFile, sim_nkin_HS, sim_nkin_PO, ncomps_HS_yaya, ncomps_PO_syaya);
  497:     Util_Close_Output_File(bm->CloseKinPOFile);
  498:     Util_Close_Output_File(bm->CloseKinHSFile);
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
**Lines (current code):** 54-55, 109-121, 171, 590-591  
**Lines at review (Oct 2026):** 54-55, 109-121, 171, 590-591

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

The code reads `CloseKinEst[sp].first_sy/last_sy/first_dy/samp_power/a_ck_thresh...`, but these are only read into element 0 (`bm->CloseKinEst->first_sy`, atassessParamIO.c:1088-1094). The struct array is `malloc`'d uninitialised (atassessParamIO.c:619), so for sp≠0 these are garbage loop bounds and sizes. Function: `CKsimulator()`.

**Current code** (`atSS3Link/atCloseKin.c` lines 52–57)

```c
   52:     //double *tmpVec;
   53: 
   54:     int last_dy = bm->CloseKinEst[sp].last_sy;
   55:     int nsampy = bm->CloseKinEst[sp].last_sy - bm->CloseKinEst[sp].first_sy + 1;
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
**Lines (current code):** 238  
**Lines at review (Oct 2026):** 238

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

`n_sya[s][y][a] / n_sya[s][y-1][a-1]` has no guard. Zero numbers give inf or NaN survival, which then propagates into psurv and Pr_HS. Function: `CKsimulator()`.

**Current code** (`atSS3Link/atCloseKin.c` lines 236–240)

```c
  236: 
  237:             for (a = 1; a < bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]; a++){
  238:                   psurv_syay[s][y-1][a-1][y] = n_sya[s][y][a] / n_sya[s][y-1][a-1];
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
**Lines (current code):** 337-358  
**Lines at review (Oct 2026):** 337-358

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

The `} // for ap_b1` brace actually closes the `if`, so `Pr_HS_syaya = demog_Pr_HSP; Pr_HS_yaya += ...` runs on every ap_b1 iteration. Pr_HS_yaya accumulates partial sums. Pr_HS_syaya ends up correct, and Pr_HS_yaya is unused. Function: `CKsimulator()`.

**Current code** (`atSS3Link/atCloseKin.c` lines 335–360)

```c
  335:                                 // *Putative* parental age in b1
  336:                                 // Propto numbers-at-age * fec-at-age by sex in b1
  337:                                 for (ap_b1 = amat; ap_b1 < bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]; ap_b1++) {
  338:     
  339:                                     Pr_par1_was_ap_b1 = n_sya[s][b1][ap_b1] * bm->CloseKinEst[sp].fec_sa[s][ap_b1] * inv_totfec_sy[s][b1];
  340:     
  341:                                     // Parent's age in b2, could be in the plus group
  342:                                     int ap_b2 = 0;
  343:                                     if ((ap_b1 + (b2 - b1)) > bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]) {
  344:                                         ap_b2 = bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id];
  345:                                     } else {
  346:                                         ap_b2 = ap_b1 + (b2 - b1);
  347:                                     }
  348:                                     if( ap_b2 >= amat)  {
  349:                                         // note *no* n_sya term next because it's just 1 candidate animal
  350:                                         Pr_par2_is_par1_if_alive = bm->CloseKinEst[sp].fec_sa[s][ap_b2] * inv_totfec_sy[s][b2];
  351: 
  352:                                         // Cumul survival prob of parent from b1 to b2, given age ap_b1 in b1
  353:                                         demog_Pr_HSP +=    Pr_par1_was_ap_b1 * Pr_par2_is_par1_if_alive * psurv_syay[s][b1][ap_b1][b2];  // if b2>b1 this will be zero instead of 1; but I think those cases are overwritten anyway
  354: 
  355:                                     } // for ap_b1
  356:     
  357:                                     Pr_HS_syaya[s][y][a][y2][a2] = demog_Pr_HSP; // *bm->CloseKinEst.bm->CloseKinEst.HSP_false_neg_loss_rate; // moved to Likelihood calcs
  358:                                     Pr_HS_yaya[y][a][y2][a2] += Pr_HS_syaya[s][y][a][y2][a2]; //ROBIN: Pr_HS_yaya[y][a][y2][a2] isn;t really used anywhere. It's a bit complicated in the original. Is it needed?
  359:     
  360:                                 } // if parent bm->CloseKinEst[sp].mature in b2
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
**Lines (current code):** 589-591  
**Lines at review (Oct 2026):** 589-591

**Function:** `writeCloseKinSampFile()`

**Why it's thought to be a bug**

`ageprops[s][a]` uses sex `s` as the stock index; ageprops is `[stock][age]`. Also `samp_prop_to += ...` is never reset, and the `*= a_ck_scalar` scaling is reapplied to the cumulative value on every call, so it compounds. Function: `writeCloseKinSampFile()`.

**Current code** (`atSS3Link/atCloseKin.c` lines 587–593)

```c
  587:                 }
  588:                 */
  589:                 bm->CloseKinEst[sp].samp_prop_to[s][y][a] += bm->RBCestimation.RBCspeciesArray[sp].ageprops[s][a] *  FunctGroupArray[sp].speciesParams[samplesize_id];
  590:                 if (a <  bm->CloseKinEst[sp].a_ck_thresh) bm->CloseKinEst[sp].samp_prop_to[s][y][a] *= bm->CloseKinEst[sp].a_ck_scalar;
  591:                 else if (a < bm->CloseKinEst[sp].a_ck_thresh2) bm->CloseKinEst[sp].samp_prop_to[s][y][a] *= bm->CloseKinEst[sp].a_ck_scalar2;
  592: 
  593:                 fprintf(bm->logFile, "%e %d %s %d %d %d CloseKinEst: %e\n", bm->dayt, year, FunctGroupArray[sp].groupCode, s, y, a, bm->CloseKinEst[sp].samp_prop_to[s][y][a]);
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
**Lines (current code):** 645, 647  
**Lines at review (Oct 2026):** 645, 647

**Function:** `writeCloseKinNcompsfiles()`

**Why it's thought to be a bug**

The HS output prints `(int)sim_nkin_HS[s][y][a][y2][a2]` twice. The TODO at 649 suggests the second value (or the swapped `[y2][a2][y][a]` entry) was intended to differ. Function: `writeCloseKinNcompsfiles()`.

**Current code** (`atSS3Link/atCloseKin.c` lines 643–649)

```c
  643:                      
  644:                          if (b1<b2) {
  645:                              fprintf(fidHS, "%e %d %s %d %d %d %d %d %d %d %e\n", bm->dayt, year, FunctGroupArray[sp].groupCode, s, y, a, y2, a2, ((int)sim_nkin_HS[s][y][a][y2][a2]), ((int)sim_nkin_HS[s][y][a][y2][a2]), ncomps_HS_yaya[y][a][y2][a2]);
  646:                         } else {
  647:                             fprintf(fidHS, "%e %d %s %d %d %d %d %d %d %d %e\n", bm->dayt, year, FunctGroupArray[sp].groupCode, s, y2, a2, y, a, ((int)sim_nkin_HS[s][y][a][y2][a2]), ((int)sim_nkin_HS[s][y][a][y2][a2]), ncomps_HS_yaya[y][a][y2][a2]);
  648:                             
  649:                             //TODO: ASK Robin if this should be ((int)sim_nkin_HS[s][y2][a2][y][a]), ((int)sim_nkin_HS[s][y2][a2][y][a]).....
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
**Lines (current code):** 146-151  
**Lines at review (Oct 2026):** 146-151

**Function:** `CKsimulator()`

**Why it's thought to be a bug**

`amat` becomes the *last* age whose fecundity is below the threshold, i.e. an immature age, not the first mature age. Function: `CKsimulator()`.

**Current code** (`atSS3Link/atCloseKin.c` lines 144–153)

```c
  144:     }
  145: 
  146:     int amat = 0;
  147:     for (a = 0; a < bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]; a++) {
  148:         if (bm->CloseKinEst[sp].fec_sa[FEMALE_ID][a] < bm->RBCestimation.RBCspeciesParam[sp][thresh_mat_id]) {
  149:             amat = a;
  150:         }
  151:     }
  152: 
  153:     // Writeout closekin catch
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
