# Atlantis — Bug To-Do / Fix List

- **Part A** (generated 2026-10-10): every `.c` and `.h` file in `atlantis/atassess/` (~165 findings).
- **Part B** (generated 2026-10-11/12, at the end of this file): every other folder under `atlantis/` (971 findings). It excludes third-party `externalLibs/` and `netcdf/`, `autom4te.cache/`, and the auto-generated `atbrokerlink/*.pb-c.*`.

No code was changed. Each entry needs confirming before it's fixed. Part B uses the same columns and conventions as Part A below. Its IDs have a per-reviewer prefix explained at the top of Part B.

# PART A — atassess/

**How to read this list**
- **ID**: file prefix + number. ATS = atAssessTierSetup.c, API = atassessParamIO.c, CLS = atclassical.c, IND = atindices.c, SRV = atsurvey.c, WRT = atwriting.c, ATT = atattribute.c, NET = atnetwork.c, SMP = atsample.c, DIE = atdiet.c, DAT = atdatareading.c, SSU = atsamplesetup.c, SOL = atsolve.c, DOO = doolittle.c, HDR = headers, XTR = outside atassess.
- **Conf / Sev**: confidence that it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent).
- **✔ checked**: I re-read the code myself and the claim holds (on top of the reviewer's own check).
- **Array convention**: everything here was checked using the Atlantis reversed-dimension rule. `allocNd(n1..nk)` and `Util_Alloc_Init_ND_*(d1..dk)` return `[dk]...[d1]`. `Util_Init_ND_*(a, d1..dk)` loops in index order `[d1]...[dk]`. Allocations that only *look* back to front were not reported. Those listed as "swapped" are genuine mismatches even after applying this rule.
- **Status** column: tick it once you've confirmed (Y), rejected (N) or fixed (F) the entry.

---

## 1. Priority summary: high confidence and high severity

| ID | File:Line | Problem | Status |
|---|---|---|---|
| ATS-01 ✔ | atAssessTierSetup.c:136 | `AgeYears` allocated `[nYears][3]` but used `[it][iyr]`: out-of-bounds | [ ] |
| ATS-02 ✔ | atAssessTierSetup.c:448,451 | `agesel_bysex`/`fval` allocated `[nCohorts][nSexes]` but used `[s][a]`: heap overrun | [ ] |
| API-01 ✔ | atassessParamIO.c:647-652 | `speciesParamStructArray[sp]` indexed by species; `MultispAssessType` read from wrong array | [ ] |
| API-04 ✔ | atassessParamIO.c:891 | `speciesParams[sp]`: species index used as a parameter id | [ ] |
| CLS-08 ✔ | atclassical.c:668 | zone loop `z < nfzones+1` over arrays sized `nfzones` | [ ] |
| CLS-11 | atclassical.c:1042,1050,1079 | `nagemat` (years) used as a cohort index: out-of-bounds | [ ] |
| CLS-12 | atclassical.c:1002 | `catch_N` reset inside zone loop | [ ] |
| CLS-16 | atclassical.c:1187 | `NumsTracers[na]`: age-in-years used as a cohort index | [ ] |
| CLS-17 ✔ | atclassical.c:1218-19,1555-56 | `Ecology_Spawn` args X_RS and FSPB swapped | [ ] |
| CLS-18 | atclassical.c:1242-1279,1596-1660 | `not_done` overwritten, so Fref loop can end before BrefA converges | [ ] |
| SOL-1 / CLS-05 ✔ | atsolve.c:137-140,193 | Amoeba puts best point in `p[1]` but returns `ilo` | [ ] |
| SOL-2 / CLS-01 | atsolve.c:97-98; atclassical.c:213-219,441-447 | simplex has `ndim` vertices, not `ndim+1`; last parameter never varied | [ ] |
| SOL-6 ✔ | atsolve.c:493; atclassical.c:1352 | `PP` never allocated (NULL), so `Linear_Regression` crashes | [ ] |
| SOL-7 | atsolve.c:483 | q estimated from catch (`CTData`) instead of CPUE (`ITData`) | [ ] |
| SOL-9 | atsolve.c:556 | Schaefer minimises the +log-likelihood (wrong sign) | [ ] |
| IND-02 | atindices.c:321,1019 | age cap allows `j == K_max_agekey` (one past end) | [ ] |
| IND-07 | atindices.c:691,699,702 | vertebrates not ranked in `sort_values`; all overwrite `endnums[0]` | [ ] |
| IND-08 ✔ | atindices.c:814-819 | unfiltered rank, so `numsort` goes negative and `endnumsbig[-n]` is written | [ ] |
| IND-10 | atindices.c:844,869,881,900 | trait index looped over `numStages` | [ ] |
| IND-12 ✔ | atindices.c:1003 | `agelengthkey[k][sp][jj]`: age and species indices swapped | [ ] |
| IND-16 | atindices.c:1276 | CV variance uses `Σx² − mean²` (should be `Σx² − n·mean²`) | [ ] |
| IND-17 | atindices.c:1316 | `avgtl[*][nfzones+1]` never reset | [ ] |
| IND-20 | atindices.c:1505 | "static" TL spectrum uses dynamic TL (copy-paste) | [ ] |
| SRV-01 | atsurvey.c:290-325 | annual `/= numsteps` block sits inside the per-box loop | [ ] |
| SRV-03 / SRV-10 | atsurvey.c:646; 990 | `raw`/`rawwgt` overwritten with reserve-only weight, then reused for catch/discards | [ ] |
| SRV-13 ✔ | atsurvey.c:1123 | `sizebins` reset `j<2`, should be `j<5` (atattribute.c uses 5) | [ ] |
| SRV-17 | atsurvey.c:1560-1568 | sampled age can be negative, giving a negative array index | [ ] |
| DIE-1 | atdiet.c:137 | overall diet composition computed inside the box loop | [ ] |
| DIE-3 ✔ | atdiet.c:243 | `Util_Init_2D_Double(contribvert, 2, K)` should be `(K, 2)`; half of array never reset | [ ] |
| DIE-5 | atdiet.c:668-669 | TL matrix diagonal uses `numStages` stride, so matrix is singular | [ ] |
| DAT-1 | atdatareading.c:332,354 | sediment group biomass stored in `[SED]` but read from `[WC]` | [ ] |
| DAT-2 ✔ | atdatareading.c:227 | `zonearea` accumulates every sampling step, never reset | [ ] |
| WRT-02 | atwriting.c:914,940 | physinfo "DON"/"DIN" columns actually contain O2wc/TSS | [ ] |
| NET-01 ✔ | atnetwork.c:80-86 | juvenile and adult shares swapped | [ ] |
| ATT-13 | atattribute.c:705 (and atsurvey.c:600) | fish discard *numbers* divided by weight | [ ] |
| XTR-01 ✔ | atlantisUtil/atUtilIO.c:178 | `proportion_check` uses `&&`, so it can never fail | [ ] |

---

## 2. Full findings by file

### 2.1 atAssessTierSetup.c

| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| ATS-01 ✔ | 136 | swapped index | H/H | `AgeYears = Util_Alloc_Init_2D_Int(3, nYears, 0)` gives `[nYears][3]`. Used as `AgeYears[it][iyr]` (it<3, iyr<nYears) in atSS3DataGen.c:159, 833, so any year ≥3 overruns. `LengthYears` on line 167 is correct: `(nYears,4)`. | `Util_Alloc_Init_2D_Int(nYears, 3, 0)` | [ ] |
| ATS-02 ✔ | 448, 451 | swapped index | H/H | `agesel_bysex`/`fval = Util_Alloc_Init_2D_Double(K_num_sexes, numCohorts)` gives `[numCohorts][sexes]`. Used `[s][a]` with a up to numCohorts-1 in atManageTier.c:941-963, 1020. | swap to `(numCohorts, K_num_sexes)` | [ ] |
| ATS-03 | 270-307 vs 724-876 | alloc/free mismatch | H/L | Close-kin, SlopeMat, FracLenS, Fupdated, FFs etc. are allocated for `assess_flag \|\| isImpacted`, but freed only after the `continue` at line 724 (tiered + assessed only), so they leak. | move these frees above line 724 | [ ] |
| ATS-04 | 577 vs 63 | wrong variable | M/M | Free is gated on `isFished` but allocation is gated on `isImpacted`. `RBCspeciesArray` is malloc'd (not calloc'd), so a fished-not-impacted group frees garbage pointers. | use `isImpacted` on line 577; calloc the struct arrays | [ ] |
| ATS-05 | 1439 | wrong operator | M/H | `isTAC \|\| (TACamt < no_quota)` sums the `no_quota` sentinel into `TACinitial`/`RBC_old`. Elsewhere this test is `isTAC && ...` or `!isTAC \|\| ...`. | `isTAC && (TACamt < no_quota)` | [ ] |
| ATS-06 | 1050-1073 | off-by-one | H/M | `thisfval = i/nfintt` at the end of iteration i, so F=0 is evaluated twice and every step lags by one. F20/40/48 come out 1/nfint low and maxF is never reached. | `thisfval = (i+1)/nfintt;` | [ ] |
| ATS-07 | 1406 | wrong variable | M/M | `metierArray[mid].metierCode` is copied from `FisheryArray[m]` (assessment fleet index) rather than `FisheryArray[mid]`. | use `mid` | [ ] |
| ATS-08 | 53, 1404, 1412 | uninitialised mapping | M/M | `speciesRPFleetToMetier` is initialised to 0 and never populated anywhere, so every fleet maps to metier 0. atSS3LinkIO tests `> -1`, which is always true. | init to -1 and populate (check for lost read code) | [ ] |
| ATS-09 | 561-899 | memory leak | H/L | Never freed: AgeComp, AFss, LenComp, Xmove, DiscardType, Blocks, Obsageprop, MeanLenAgeS, T5_removal, TAC_mult, Tier3_Linf/k/t0, Tier5_Bstart, Tier5_r, MULTISPestIDs, the CloseKinEst array. | add matching frees | [ ] |
| ATS-10 | 1454 | printf mismatch | H/L | `%d` used for a `double` (`RBCspeciesParam[..][NumFisheries_id]`). | cast to `(int)` | [ ] |
| ATS-11 | 1528-1530 | logic | L/L | `Lastsp` is only incremented when >0, so if the last assessed group is index 0 it's skipped by `sp < Lastsp` loops; also never reset. | review | [ ] |
| ATS-12 | 917; 954, 970 | div-by-zero / OOB | L/L | `siglen` ≤ 0 when `len[a]` ≤ 0 gives inf/NaN; `YPRlen` with MaxAge=1 indexes `[-1]`. | guard | [ ] |
| ATS-13 | 1143 | off-by-one | L/L | Loop sets `LengthN[0..2]` but array has 4 entries, so `LengthN[3]` stays 0. | check intent | [ ] |

### 2.2 atassessParamIO.c

| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| API-01 ✔ | 647-652 | wrong variable | H/H | `speciesParamStructArray[sp]` is the parameter-descriptor table `[tot_prms]`, indexed here by species. `MultispAssessType_id` is an RBC parameter (read on line 1052 into `RBCspeciesParam`), but is read from `FunctGroupArray[sp].speciesParams`. So `PGMSY_on` gets set wrongly. | use `isImpacted == TRUE` and `RBCspeciesParam[sp][MultispAssessType_id]` | [ ] |
| API-02 | 639-660 (alloc 614-617) | NULL deref | M/H | `RBCspeciesParam` is only allocated when `useRBCTiers`; it's used when `useMultispAssess` is set, so it crashes if RBC tiers are off. | require useRBCTiers, or nest the block | [ ] |
| API-02b | 589 | no NULL check | H/L | `xmlReadFileDestFolder` result is not checked, so a missing file segfaults. | `if (!inputDoc) quit(...)` | [ ] |
| API-03 | 722-815 | unchecked bounds | M/H | Stock loop bound is `NumRegions` from XML, never checked against `K_num_stocks_per_sp`. `CvLA0`/`CvLAmax` are allocated with `K_num_reg` but filled by the stock loop. | check bound; allocate with `K_num_stocks_per_sp` | [ ] |
| API-03b | 741, 799 | cosmetic | H/L | `\n` embedded in the error-path string. | drop `\n` | [ ] |
| API-04 ✔ | 891 | wrong variable | H/H | `FunctGroupArray[sp].speciesParams[sp] == TRUE`: species index used as a parameter id. Can also write to an unallocated `TAC_mult`. | `speciesParams[assess_flag_id] > 0` (or `isFished`) | [ ] |
| API-04b | 895-896 | dead check | H/L | `scale_index = b; if (scale_index != -1)` is always true. | tidy | [ ] |
| API-05 | 921-922 | copy-paste | H/L | Opens `assessprmIfname` but the error message prints `forceIfname`. Line 916 duplicates line 918. | print the right name | [ ] |
| API-06 | 964 | wrong check type | M/M | `Num_Sexes` is read with `binary_check`, so 2 sexes is impossible and 0 is allowed. | `integer_check`, 1–2 | [ ] |
| API-06b | 1082 | copy-paste | H/L | Error says 'Tier6/TriggerPoints' but the value is read from 'Tier_Assessment'. | fix message | [ ] |
| API-06c | 1086-1099 (alloc 619) | uninitialised | L/L | `CloseKinEst` is a per-species malloc but only `[0]` is filled. Seeds at 1007-1009 come from consecutive `clock()` calls, so they are likely identical. | calloc / separate struct | [ ] |
| API-07 | 438-441, 53 (alloc 601) | bounds | L/H | 4 `detrit_import` rows are written but `K_num_detritus*2` are allocated, so it overruns if there is only 1 detritus group. | allocate `max(4, 2*K_num_detritus)` | [ ] |
| API-08 | 1182-1311, 1476, 1483, 1541 | unchecked bounds | M/H | Arrays sized `K_num_fisheries` are filled with XML `NumFisheries`; `Mzero`/`MeanLenAge`/`Fecund` are sized `OverallMaxAge` but filled to `MaxAge`. The guard in `Tier_Assessment_Setup` runs after this read. | check bounds before reading | [ ] |
| API-09 | 1158-1163 (and 565) | leak | M/L | `RAssessRscriptName` is re-allocated and the first allocation leaks. | free first / read once | [ ] |
| API-10 | 1148 | truncation | L/M | `min_TACchange = (int)...` would truncate a proportion to 0. | check field type | [ ] |
| API-11 | 1482 | possibly inverted condition | L/M | `MeanLenAge` is read when `!flagLAdirect`, which looks opposite to the SigmaLenAge pattern. | confirm meaning | [ ] |
| API-12 | 496 | float == TRUE | M/L | `assess_flag == TRUE` here; elsewhere the test is `> 0`. | `> 0` | [ ] |

### 2.3 atclassical.c

| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| CLS-01 | 213-219, 441-447 | off-by-one / algorithm | H/H | Starting simplex has only Ndim rows; row 0 is unscaled and the last parameter (Ndim-1) is never perturbed, so Amoeba can't move it. See SOL-2. | build Ndim+1 rows with `P[i+1][i] *= GRD`; allocate P/Y with nk+1 rows | [ ] |
| CLS-02 | 166-169, 207-209 | uninitialised | H/M | With Pella-Tomlinson, Ndim=4 but only X[0..2] are reset; X[3] is stale from the previous fit. | loop `i < Ndim` | [ ] |
| CLS-03 | 274, 498 | off-by-one | H/M | `ipnt = floor(icnt*U + 1.0)` gives 1..icnt, so BootResu[0] is never used and BootResu[icnt] is stale. | drop `+1.0`, clamp | [ ] |
| CLS-04 | 294-297, 524-527 | off-by-one | H/M | Failure test reads `[nbs+1]` = `[bootstrap+1]` (never written). The sort takes slots 0..bootstrap-1, which includes the real-data fit and excludes the last bootstrap. MAXDOUBLE failures are not excluded. | sort `[j+1]`, skip MAXDOUBLE | [ ] |
| CLS-05 | 228-233, 461 | see SOL-1 | M/H | Takes `P[Ilow]` which, because of the Amoeba swap bug, is not the best vertex. | fix in atsolve.c | [ ] |
| CLS-06 | 231, 459 | off-by-one | M/L | `nfunk <= Assess_Max_Int` treats hitting the iteration limit as success. | `<`, or an explicit flag | [ ] |
| CLS-07 | 521-574 | misplaced block | H/M | `if (!assess_failed)` save sits inside the cohort loop, so partial sums are stored if a later cohort fails. | move after the loop | [ ] |
| CLS-08 ✔ | 668-680 | off-by-one | H/H | `z < nfzones+1` over `stockinfo`, `individVERTinfo` and `agelengthkey`, which all have zone dimension `nfzones` (atsamplesetup.c:213, 223, 226). | `z < nfzones` | [ ] |
| CLS-09 | 723, 738-741, 793 | wrong variable | M/M | `numCohorts` used where tracer arrays need `numCohortsXnumGenes`. | use XnumGenes | [ ] |
| CLS-10 | 855, 878, 895, 980, 992, 1045 | unit mismatch | M/M | After line 855, `nagemat` is in years but is compared with cohort indices. | keep a separate cohort-level variable | [ ] |
| CLS-11 | 1042, 1050-1051, 1079-1080 | wrong var / OOB | H/H | `nagemat` (years) used as a cohort index into `scaled_C`, `scaled_mum`, `biolVERTinfo`; e.g. 15 > 10 cohorts. | use cohort-level age_mat | [ ] |
| CLS-12 | 972, 1002 | accumulator reset | H/H | `catch_N = 0` inside the zone loop, while start_N/end_N sum across all zones. | move the reset before the zone loop | [ ] |
| CLS-13 | 893 | loop condition | H/M | `while (!end_N ...)`: end_N accumulates across zones, so later zones skip the search. | per-zone variable | [ ] |
| CLS-14 | 949-951, 1025-1027 | formula | M/M | `totDead = start-end` already includes catch, so `start-catch-totDead` double-counts catch (log of a negative is possible), and the "M" formula is actually Z. | natural deaths = start-end-catch | [ ] |
| CLS-15 | 918-925, 1068, 1076-1079 | wrong variable | M/M | `b` is stale per chrt; calcE uses the running cross-zone calcB; predation uses the prey's catch and clearance instead of the predator's. | fix variables | [ ] |
| CLS-16 | 1145, 1186-1187 | wrong var / OOB | H/H | `NumsTracers[na]` with na up to numCohorts×ageClassSize-1. | `nc = floor(na/nasize)` | [ ] |
| CLS-17 ✔ | 1218-1219, 1555-1556 | swapped args | H/H | Signature is `Ecology_Spawn(..., X_RS, FSPB, log)` (atvertprocesses.c:342), but all four calls pass `scaled_FSPB[nc], X_RS[nc]`. | swap the two arguments | [ ] |
| CLS-18 | 1242-1279, 1596-1660 | logic | H/H | BrefC branch sets `not_done = 0` and overrides BrefA's `not_done = 1`. | separate doneA/doneC flags | [ ] |
| CLS-19 | 1250-1251, 1273-1274, 1285-1293 | numerical | M/M | `diff = 1 - log(1/ratio)` goes negative (negative F) and is unguarded here, unlike line 1608; the NaN check covers tempF only. | `max(small_num, …)`; check tempF2 | [ ] |
| CLS-20 | 1224-1225 vs 1116 | inconsistency | L/M | Beverton-Holt denominator uses `tbiom1` here but `spawn_biom1` at line 1116; cohort counts also differ. | unify | [ ] |
| CLS-21 | 1374-1386; 287-289, 510-512 | sort algorithm | H/M | Rank sort gives ties the same slot, leaving stale entries; percentile index can equal nbs. | tie-break / qsort; clamp | [ ] |
| CLS-22 | 1465-1479 | log domain | M/M | After the sanity resets, Z = log(start/small_num) is huge, or log of a negative. | clamp denominator | [ ] |
| CLS-23 | 1493-1509 | other | L/L | Rescaled M is not written back to `NAssess[est_M_id]`. | confirm intent | [ ] |
| CLS-24 | 1476, 1535-1536, 1565-1566 | log text | H/L | Message says 1.2 (code uses 1.25); `ageClassSize` is labelled nageclass; tempF is printed for est2. | fix text | [ ] |

Lower-confidence notes: `va95` is treated as a cohort at line 860 but as years at 926/1209. `CPUEtrend[sp][ny]` can overflow if pseudo-assessment runs from tstart. VPA "biomass" sums NEst, which may be numbers.

### 2.4 atindices.c

| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| IND-01 | 240, 287-288 | inconsistency | M/M | The denominator includes epi/infauna; methods 2/3 exclude them from the numerator. | same filter | [ ] |
| IND-02 | 321-322, 1019-1020 | off-by-one | H/H | `if (j > K_max_agekey) j = K_max_agekey` lets the index equal the dimension size (`agelengthkey` age dimension = K_max_agekey). | `>=` … `K_max_agekey-1` | [ ] |
| IND-03 | 347-353 | formula | M/M | Bird size-at-maturity sums 3 lengths instead of averaging them. | weight / divide | [ ] |
| IND-04 | 643-645 | wrong variable | H/M | Clamp tests `startbinbig` instead of `bincount`. | `if (bincount > pelbinbird-1)` | [ ] |
| IND-05 | 563/578, 649/653 | index arithmetic | M/M | `+4` plankton offset added to an already-absolute bin; line 653 uses absolute `sizeclass`. | drop `+4`; use `sizeclass - sizeclasstransit` | [ ] |
| IND-06 | 422-425, 625-649 | stale accumulator | M/M | `nbinleft`/`startbinbig` carry over between species; `prop_bin` sums to >1. | reset per species | [ ] |
| IND-07 | 691, 699, 702 vs 713 | copy-paste | H/H | Vertebrates are excluded from ranking but are written, so all land in `endnums[0]`. | add `isVertebrate` | [ ] |
| IND-08 ✔ | 814-819 | copy-paste | H/H | Inner rank loop is unfiltered, so rank > counter-1 and `numsort` < 0, writing `endnumsbig[negative]`. | filter `chrt` like the sibling functions | [ ] |
| IND-09 | 703-707, 741-746, 777-782, 815-819 | ranking | M/M | Ties collide; `endnums` is never cleared, so stale slots remain. | tie-break and clear | [ ] |
| IND-10 | 844-845, 869-870, 881-882, 900-901 | wrong variable | H/H | Loops over the trait index (0=nums, 1=biom) with `numStages` bound, so single-stage groups lose biomass and >2 stages go out of bounds. | `chrt < 2` | [ ] |
| IND-11 | 874-925 | mislabel | M/L | `abundindex_id=1`/`bioindex_id=0`, but the stored data are 0=nums, 1=biomass. | align constants | [ ] |
| IND-12 ✔ | 1003-1004 | swapped index | H/H | `agelengthkey[k][sp][jj]`; layout is `[size][age][sp]`, as line 1004 shows. `scalarsum` is unused. | `[k][jj][sp]` | [ ] |
| IND-13 | 1018 | truncation | L/M | No ROUNDGUARD; can produce -1. | floor + guard + clamp | [ ] |
| IND-14 | 1103-1130 | inverted ratio | M/M | PS:ZP computed as planktivore/piscivore, opposite of the doc; pd[0] includes birds and mammals. | swap / fix doc | [ ] |
| IND-15 | 1255-1257 | div-by-zero | M/M | No zero-count guard; sqrt of a slightly negative value gives NaN. | +TINY, clamp ≥0 | [ ] |
| IND-16 | 1275-1277 | formula | H/H | Variance = (Σx² − **mean²**)/(n−1); should be `n·mean²`. Compare Calculate_Phys_SD line 1255. | `- n*mean*mean` | [ ] |
| IND-17 | 1316-1318 | off-by-one | H/H | `avgtl` is `[8][nfzones+2]` but reset only to `z < nfzones+1`, so `[nfzones+1]` accumulates forever. | `z < nfzones+2` | [ ] |
| IND-18 | 1360-1455 | wrong variable | M/H | Whole-area columns are weighted by `samplearea` (sums to 1 per zone), giving ≈ nfzones × TL. | use `totalarea[bb]` | [ ] |
| IND-19 | 1346-1348 vs 1370-1376 | units | M/M | Vertebrate catch is divided by zonearea; invertebrate catch is not. | divide inverts too | [ ] |
| IND-20 | 1504-1505 | copy-paste | H/H | "Static" block uses `tl[a]` (dynamic); the invertebrate equivalent uses `tl[a+3]`. | `tl[a+3]` | [ ] |
| IND-21 | 1501, 1512, 1543, 1552 | missing weight | M/M | Zone-level catch is added once per box. | × samplearea or move out | [ ] |
| IND-22 | 1638-1665 | logic | H/M | Whole-area functional diversity omits the species count included per zone. | start from `divindx[0][nfzones]` | [ ] |

### 2.5 atsurvey.c

| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SRV-01 | 290-325 (inside bb loop 185-359) | misplaced block | H/H | Per-zone arrays are divided by numsteps once per box, so they are divided N times and results depend on box order. | move after the box loop | [ ] |
| SRV-02 | 192-203, 295-296 (+ atsample.c:238) | scaling | L/M | samplephys may be divided by numsteps twice. | normalise once | [ ] |
| SRV-03 | 620, 646, 668, 688 | reused temp | H/H | `raw` is overwritten with reserve weight (line 646) and then used for catch/discard weights. | separate variable | [ ] |
| SRV-04 | 635, 667, 687, 983, 1010, 1253 | off-by-one | H/M | `while (nsample && i <= nsample)` from i=0 gives nsample+1 draws. | `i < nsample` | [ ] |
| SRV-05 | 420-546, 1467-1473 | misplaced brace | M/H | All bird sampling is inside `if (annual_sample == 1)` (meant only for the bin init), yet the result is still divided by numsteps. | close the block after line 437 | [ ] |
| SRV-06 | 503-517 | loop bound | M/M | Birds fill bins 0-2 but the sum loops to `numCohortsXnumGenes` (can exceed K_num_size). | loop 0..2 | [ ] |
| SRV-07 | 454-461, 640, 1260 | wrong variable | M/M | Age and stage computed from `chrt` (cohort×gene) rather than the base cohort. | `chrt/numGeneTypes` | [ ] |
| SRV-08 | 640, 1260 | off-by-one | M/M | `round(x + 0.5)` = floor(x)+1, so age 0 is never populated. | `floor(x)` | [ ] |
| SRV-09 | 737-738 | denominator | M/M | Per-bin mean weight divided by total numbers over all bins. | per-bin count | [ ] |
| SRV-10 | 969, 990, 1011 | reused temp | H/H | Same as SRV-03, for fish discards. | separate variable | [ ] |
| SRV-11 | 1072-1080 (+511) | double count | M/M | Bird catch numbers are added twice. | one source | [ ] |
| SRV-12 | 794-797 vs 819-829, 876 | condition | L/M | Reset is gated on isImpacted, accumulation on isFished. | unify conditions | [ ] |
| SRV-13 ✔ | 1123-1124 | off-by-one | H/H | `sizebins` reset `j<2` of 5 characteristics; `sizeselectnums_id` (4) grows every year and inflates catchability. atattribute.c:807 uses `j<5`. | `j < 5` | [ ] |
| SRV-14 | 1210, 1220, 1223 | loop bound | M/M | Uses global max cohorts, so additive noise creates phantom fish. | per-species count | [ ] |
| SRV-15 | 1311, 1320-1323, 1360-1361, 1372-1374 | unguarded logging | H/L | Debug fprintf calls are not behind `verbose`, so log files bloat. | wrap in `verbose` | [ ] |
| SRV-16 | 1393-1397 | units | L/L | matage (years) is capped at numCohorts. | cap at numCohorts×ageClassSize | [ ] |
| SRV-17 | 1559-1568, 1574 | random range | M/H | Upper bound is clamped but not lower, so a negative `samplec` writes out of bounds in `agebins`. | `if (samplec < 0) samplec = 0;` | [ ] |
| SRV-18 | 1680-1689 | log(0) | L/L | Box-Muller with U=0 is possible. | redraw / drandom(TINY,1) | [ ] |

Lower-confidence notes: the survey never fills `oldbaby[..][sample_id]`, so the sampled young-of-year index is always 0. Prod/eat gated on `sample_now` are still divided by numsteps.

### 2.6 atwriting.c

| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| WRT-01 | 768 | div-by-zero | M/M | CPUE = catch/effort with no guard (line 1099 adds small_num). | `+ small_num` | [ ] |
| WRT-02 | 914, 940-941 | stale layout | H/H | Header reads `Salt LightWc LightSed O2 DON DIN`, but samplephys[0..5] = salinity, light_wc, light_sed, o2_sed, o2_wc, TSS. DON/DIN/Temp (rows 6, 7, 9) are never written. | write named ids and match the header | [ ] |
| WRT-03 | 1647-1650, 1700-1707 | size mismatch | M/M | Loops to `K_num_abcbin` over `endnums` sized K_num_tot_sp; the parameter is not capped. | cap in ParamIO | [ ] |
| WRT-04 | 854, 1540, 1593, 2179, 1286/1323/1361, 2473, 1486, 2708, 2782 | header text | H/L | Hard-coded bin counts in headers; "Location" label; missing `#`/space/newline. | build headers from loop bounds | [ ] |
| WRT-05 | 2648-2669 | dead code | H/L | `WriteDietOut` is never called, so indiet/offdiet/oi files are never produced. | confirm intent | [ ] |

### 2.7 atattribute.c

| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| ATT-01 | 166, 188, 390, 1011-1013 | averaging | M/M | `oldbaby` is assigned with `=` each step but divided by numsteps; birds are double-added at line 390. | no division / accumulate | [ ] |
| ATT-02 | 153, 220 | wrong variable | H/M | Time column `nt = dt/86400` (step length) instead of year (`dayt/365`, as in atsample.c:59). | `floor(ROUNDGUARD + dayt/365)` | [ ] |
| ATT-03 | 245-289 | missing normalisation | M/H | Invertebrate true values are never divided by numsteps (survey divides them). | add the year-end division | [ ] |
| ATT-04 | 246-249 | range | M/L | Loop `light_wc_id..<don_wc_id` skips DON, DIN and Temp. | include them | [ ] |
| ATT-05 | 268-270 | wrong slot | M/M | Reads `biolbiom[..][WC]` only; sediment groups are stored in SED (see DAT-1). | sum WC+SED | [ ] |
| ATT-06 | 394-395 | wrong index | M/M | Bird catch numbers always go into bin 0, not `lbin`. | `[lbin]` | [ ] |
| ATT-07 | 370-377, 469, 858 | wrong variable | M/M | Stage switch and age use `chrt` (cohort×gene). | base cohort | [ ] |
| ATT-08 | 469, 858 | off-by-one | H/M | `round(x+0.5)` shifts age up by 1 (same as SRV-08). | `round(x)` | [ ] |
| ATT-09 | 460, 852 | off-by-one | L/L | `ceil(ageClassSize+0.5)` gives one too many sub-ages; the remainder is dropped. | `(int)(acs+0.5)` | [ ] |
| ATT-10 | 383-402, 418-435 | accumulator mixing | M/M | Bird raw counts are added onto already-normalised fractions in later rounds. | per-round locals | [ ] |
| ATT-11 | 490, 505, 869 (+atsurvey 535/699) | formula | M/M | Production multiplied by stock numbers again (already included in atdatareading.c:536). | drop the factor | [ ] |
| ATT-12 | 500-512 vs 485, 866 | inconsistency | L/M | Bird prod/eat are not gated by `sample_now`. | gate | [ ] |
| ATT-13 | 705 (+atsurvey.c:600) | units | M/H | `bdiscards` holds numbers (atdatareading.c:575) but is divided by weight, so it is ≈0. | drop `/ wgt` | [ ] |
| ATT-14 | 756-765 (+429) | double count | M/M | Bird catch numbers are added twice. | one source | [ ] |
| ATT-15 | 727, 756 | condition | M/L | `else` also catches invertebrates. | `else if (BIRD)` | [ ] |
| ATT-16 | 934-935 | formula | M/M | Biomass per bin = total biomass × individual weight (mass²). | × `istocknums` fraction | [ ] |

### 2.8 atnetwork.c

| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| NET-01 ✔ | 80-86 | swapped | H/M | `pop_fraction[0/1]` = juvenile fraction (atattribute.c:974-978), but the juv branch uses `1 - pop_fraction`. Also, birds never get pop_fraction. | swap the branches | [ ] |
| NET-02 | 182 | index assumption | L/M | `detrit_import[stage*2 + detIndex]` assumes DL=0, DR=1 in group order. | index by type | [ ] |
| NET-03 | 58-64, 102-198, 250-256 | stale values | L/L | Only `[0][0][z]` is reset; skipped groups keep `minpool` from the previous year. | zero all | [ ] |

### 2.9 atsample.c

| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SMP-01 | 233, 238 | off-by-one | H/M | `if (sample > num_sampled_phy_id)` should be `>=`; no lower bound (sample<2 gives a negative index). | `>=` and `< salinity_id` | [ ] |
| SMP-02 | 258 | off-by-one | H/M | `if (sample > 3)` should be `>= 3` (`sampleprocess` has 3 rows). | `>= 3 \|\| < 0` | [ ] |
| SMP-03 | 117, 134, 143, 173 | schedule | M/M | `teatassess` is not ceil'd like `tassess`, so it drifts and stops firing; next indices are not bounds-checked. | match tassess | [ ] |
| SMP-04 | 186 | always true | M/M | `flagpatchy < 2` but flagpatchy is binary. | `== 1` | [ ] |
| SMP-05 | 202-213 | buffer overflow | M/H | `nsbox = nsbox_o` can exceed the `samplearea`/`totalarea` allocation (nsbox_i). | allocate with the max | [ ] |
| SMP-06 | 238, 265, 279 | normalisation | M/M | Getters divide inconsistently by `annual_sample` (can be 0, giving inf). | unify and guard | [ ] |
| SMP-07 | 273-274 | off-by-one / copy-paste | H/L | `>` should be `>=`; error message names the wrong function. | fix both | [ ] |

### 2.10 atdiet.c

| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| DIE-1 | 115-138 | accumulator in wrong loop | H/H | `Calculate_Overall_Diet_Composition` runs inside the box loop on running totals, so early boxes are counted many times (atattribute.c:1115 does it after the loop). | move after the loop | [ ] |
| DIE-2 | 162-164 | wrong slot | M/M | Prey availability uses `biolbiom[prey][b][WC]` only, so infauna and sediment bacteria are never eaten (see DAT-1). | WC+SED | [ ] |
| DIE-3 ✔ | 243 (alloc atsamplesetup.c:277) | swapped dims in Init | H/H | Allocated `Util_Alloc_Init_2D_Double(2, K)` = `[K][2]`, but reset with `Util_Init_2D_Double(contribvert, 2, K)`, which loops `[0..1][0..K-1]` and covers only flat elements 0..K+1. Entries for pred ≥ (K+2)/2 are never reset and accumulate. | `Util_Init_2D_Double(contribvert, K_num_tot_sp, 2, 0.0)` | [ ] |
| DIE-4 | 547-555 | misplaced break | H/L | `if (checkdone) break;` sits inside the same inner loop, so it never fires (performance only). | move out one level | [ ] |
| DIE-5 | 667-671, 690-692 | wrong variable | H/H | Diagonal uses `pred*numStages+stage`; the rest of the matrix uses `pred*2+stage`. Detritus rows stay zero, so the matrix is singular, and Doolittle's -1 return is ignored. | `pred*2+stage`, loop to 2, check return | [ ] |
| DIE-6 | 767-784 | copy-paste | M/M | Detritus preference is always written to `allStage=0`, so adults never eat detritus. | use `predStage` | [ ] |
| DIE-7 | 804-807 | wrong variable | M/L | `pSPEat[predStage][prey][preyStage]`: the 3rd index is habitat, not stage; `/2` is hard-coded. | use `pSPVERTeat` | [ ] |

### 2.11 atdatareading.c

| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| DAT-1 | 325-333, 348-355 | wrong slot | H/H | SED_BACT, SM_INF, LG_INF, MPB and sediment PHYTOBEN/TURF are stored in `biolbiom[..][SED]`; all non-detritus readers use `[WC]`, so they read zero. | store in WC, or read both | [ ] |
| DAT-2 ✔ | 226-227 | never reset | H/H | `zonearea[z] +=` runs every sampling step; it is zeroed only once (atsamplesetup.c:412). | reset in Init_Data_Array | [ ] |
| DAT-3 | 261-273 | units | M/M | Salinity, light, temperature and O2 are summed over layers, not depth-averaged (Chl a on line 299 is weighted). | weight by dz/depth | [ ] |
| DAT-4 | 536-538 | units | L/M | Production is multiplied by numbers but consumption is not. | confirm units | [ ] |
| DAT-5 | 474 | reused variable | H/L | Zone variable `z` is reused as a cohort loop index. | use `cohort` | [ ] |
| DAT-6 | 550 | wrong condition | M/L | Verbose print uses `isGrazer == FALSE` (should be TRUE). | flip | [ ] |

### 2.12 atsamplesetup.c

| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SSU-1 | 467-500 | missing branch | M/H | `flagrandom=1, flaggen=0` leaves `rand`/`tassPatchy` at 0, so diet sampling never advances and assessment sampling runs daily. | read stored values, or quit | [ ] |
| SSU-2 | 471-472 | formula | L/M | `tassPatchy = tassessinc / freq` is used as a multiplier, giving tassessinc²/freq. | `freq / tassessinc` | [ ] |
| SSU-3 | 382-386 | units | H/L | `nkk = tstop/365` but tstop is in seconds, so allocation is 86400× too big. | divide by 86400*365 | [ ] |
| SSU-4 | 397 | alloc vs use | M/M | `NResult` middle dimension = cohorts only; it is indexed up to `num_est_prm` (NEst uses max). | max(cohorts, num_est_prm) | [ ] |
| SSU-5 | 432-435 | always false | M/L | `numsteps < 0` can never be true; probably meant `< 1`. | `< 1` | [ ] |
| SSU-6 | 129-131 | NULL deref | H/L | `*(strstr(name, ".prm")) = 0` with no NULL check. | check | [ ] |
| SSU-7 | 738, 746, 768 | copy-paste | L/L | DINOFLAG and ICE_MIXOTROPHS use the detritus flag; SED_EP_OTHER mixes epi1 and epi2 errors. | confirm | [ ] |

### 2.13 atsolve.c, doolittle.c, triangular solvers

| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SOL-1 ✔ | 137-140, 193 | 1-based port | H/H | On convergence `SWAP(y[0],y[ilo])` but `SWAP(p[1][i],p[ilo][i])`, then `*ilow = ilo`, which now holds the old p[1]. Callers take a non-best vertex. | swap into p[0], set `*ilow = 0` | [ ] |
| SOL-2 | 97-98, 212 | algorithm | H/H | `mpts = ndim` (NR uses ndim+1), so the simplex is degenerate and the Amotry centroid is biased. | ndim+1 (with callers and allocations) | [ ] |
| SOL-3 | 151, 188, 221 | double count | H/M | `nfunk` is incremented both in Amoeba (+=2/--) and in Amotry, so the effective itmax is halved. | remove one | [ ] |
| SOL-4 | 143-150 | leak | H/L | Early `return` skips `free1d(psum)`. | break / free | [ ] |
| SOL-5 | 143 | return code | M/M | Success is inferred from `nfunk <= itmax` and is ambiguous at the limit. | explicit status | [ ] |
| SOL-5b | 76 | wrong arg | H/L | `AmoebaL` passes NULL instead of `xpar`; time is hard-coded to 1.0. | pass through | [ ] |
| SOL-6 ✔ | 493 (+atclassical.c:1352) | NULL deref | H/H | `PP` is declared `= 0` (atsamplesetup.h:165) and never allocated anywhere. `Linear_Regression` writes `P[i][j]`, so it crashes when prm_sp is 2 or 3. | allocate PP | [ ] |
| SOL-7 | 482-484 | wrong variable | H/H | `q = log(CTData/B)` uses catch, but q = CPUE/B (`ITData`). | use ITData | [ ] |
| SOL-8 | 476 | off-by-one | L/M | Subtracts `CTData[Yr]` instead of `[Yr-1]`. | confirm alignment | [ ] |
| SOL-9 | 556 | sign | H/H | `*SS = -(n/2)(…)` is the log-likelihood, but Amoeba minimises it, so it seeks the worst fit. | negative log-likelihood (+) | [ ] |
| SOL-10 | 260-262 | loop bound | H/M | Copies `X[0..est_B0_id]` only; `p` (est_p_id) is never copied. | `i < ndim` | [ ] |
| SOL-11 | 261 vs 460-462 | units | M/M | `exp(X)` but X starts at 20.0 "linear scale", giving r ≈ 4.9e6. | choose log or linear consistently | [ ] |
| SOL-12 | 296-301 | ignored return | H/M | `EquilF_Funk`/`SurplusProduction` results are discarded (`ans` stays 0). | `ans = ...` | [ ] |
| SOL-13 | 336-347, 367-375 | convergence | M/H | Bisection iterates `ndim` (≈10) times with an absolute tolerance, and writes results only on a hit, otherwise stale. | fixed iterations; always write | [ ] |
| SOL-14 | 615-622 | units | H/M | Fallbacks X[0]=0.8 and X[1]=1.0 are linear values in log space, giving q=2.23 and qinc=e. | log(0.8), 0.0 | [ ] |
| DOO-1 | doolittle.c:93 (+atdiet.c:691-692) | singular check | M/L | Tests for an exact-zero pivot only, and the caller ignores the return value. | tolerance and check | [ ] |

unit_lower_triangular.c and upper_triangular.c: no issues found.

### 2.14 Headers (include/)

| ID | Location | Conf/Sev | Description | Fix | Status |
|---|---|---|---|---|---|
| HDR-1 | atsample.h:103-105 | M/L | Lowercase `#define alpha/beta/gamma` can collide with nrcdflib.h parameters or libm `gamma()`. | rename | [ ] |
| HDR-2 | atsample.h:186 | H/L | `totselectnums_id 4` but `totnums` has 4 rows (0-3); unused today. | allocate 5 / remove | [ ] |
| HDR-3 | atAssessLib.h:50-75 | H/L | The comment on phys ids contradicts the defines (o2_wc/TSS inserted at 6/7). This is the root of WRT-02. | update comment | [ ] |
| HDR-4 | atsample.h:301 | H/L | Dangling `extern double **food;` with no definition. | delete | [ ] |

### 2.15 Outside atassess (found incidentally)

| ID | Location | Conf/Sev | Description | Fix | Status |
|---|---|---|---|---|---|
| XTR-01 ✔ | atlantisUtil/atUtilIO.c:178 | H/M | `proportion_check`: `value < 0.0 && value > 1.0` can never be true, so out-of-range proportions are accepted silently. | `\|\|` | [ ] |

---

## 3. Notes

- **Mirrored bugs:** several bugs appear in both atsurvey.c (sampled) and atattribute.c (true values), e.g. SRV-08/ATT-08, ATT-13/atsurvey.c:600, SRV-11/ATT-14, ATT-11/atsurvey.c:535/699. Fix both together so true and sampled values stay comparable.
- **Fix together:** SOL-1, SOL-2 and CLS-01 all concern the Nelder-Mead implementation and should be fixed as one change (Amoeba, both callers in atclassical.c, Linear_Regression, and the P/Y allocations).
- **Earlier reviews:** the earlier `*_bug_analysis.md` files in this directory (April 2026) overlap with some entries here (e.g. CLS-03/CLS-04). This list was produced independently from the current source; line numbers refer to the files as of today.

---

# PART B — All other folders (reviewed 2026-10-11/12)

Scope: every folder under `atlantis/` except `atassess/` (Part A above), the third-party `externalLibs/` and `netcdf/`, `autom4te.cache/`, and the auto-generated protobuf files `atbrokerlink/*.pb-c.*`.
Same conventions as Part A (ID, Conf/Sev, ✔ = re-checked by hand, reversed-dimension rule applied before reporting any allocation mismatch). IDs carry a per-reviewer prefix (e.g. DEMO = atdemography.c, TIER = atManageTier.c).

Total Part B findings: **971** (severity H = 198, M = 438, L = 335).

## B.1 Priority summary — High confidence AND High severity

| ID | File:Lines | Problem | Status |
|---|---|---|---|
| DEMO-01 | atecology/atdemography.c:1712-1726 (used 455-458, 1751-1754, 2115-2118, 2149-2152; atmovement.c:1298) | `double KWSR_force = 0.0; if (bm->flag_modify_KWSR && (KWSRforceid > -1)) {...} return KWSR_force;`. | [ ] |
| DEMO-02 | atecology/atdemography.c:2730, 2708, 2738-2772 | `nextcid = cohort + numGeneTypes; if (cohort < num_cohorts)` where `num_cohorts = numCohortsXnumGenes - 1`. | [ ] |
| DEMO-03 | atecology/atdemography.c:4453-4460 | Semelparous case: `spawning_biomass_contrib = this_biom * (1.0 - scaled_FSPB[cohort]);`. | [ ] |
| MOVE-01 ✔ | atecology/atmovement.c:2030 | newden is allocated `Util_Alloc_Init_4D_Double(ncells, nlayer, ncoh, nsp)` => `[sp][n][layer][box]` (atbiolsetup.c:2869) and used everywhere else as `newden[sp][n][k][ij]`. | [ ] |
| MOVE-02 | atecology/atmovement.c:2049-2050 | this sets the labile/refractory detritus tracers to the killed biomass instead of adding to it, wiping out the detritus already in the cell. | [ ] |
| BSET-01 | atecology/atbiolsetup.c:3161-3170 | `dist_step1 += sqrt(xdiff*xdiff + ydiff*ydiff); if (dist_step1 > maxdist) maxdist = dist_step1;`: the code is meant to find the largest distance from the entry box to any box. | [ ] |
| BSET-02 | atecology/atbiolsetup.c:3187-3197 | In the no_move invasion spread, `dist_step1` keeps growing from the BSET-01 loop and never resets, so the test `dist_step1 < InvaderSpeed*Invadedt*86400` is effectively always false. | [ ] |
| PROC-01 | atecology/atprocess.c:420, 800, 1096 | `bm->pSPFCeat[sp_id][preyID][cohort][kij]` (and `[predatorID][preyID][cohort][prey_chrt]` at 420). | [ ] |
| PROC-03 | atecology/atprocess.c:1074-1076 | `thisID = (int)speciesParams[coralID_id]; ... FunctGroupArray[thisID].cohortSpeciesParams[kij][rugosity_dec_id]`. | [ ] |
| CONT-01 | atecology/atContaminants.c:307, 480, 592, 707, 768, 1008, 1174, 1498, 1541, 1655, 1780, 1794, 1864, 2000, 2023, 2043, 2212 (and atUtilGroupIO.c:910-914) | `FunctGroupArray[i].contaminantTracers = Util_Alloc_Init_2D_Int(bm->num_contaminants, numCohorts, 0)` (atUtilGroupIO.c:910) gives `[numCohorts][ncont]`; same for `contamPropTracers` (911), `contaminantSpMort` (912, 1D `numCohorts`) and ``... | [ ] |
| CONT-02 | atecology/atContaminants.c:313, 345, 348, 380, 383 | `currentden` passed from atmovement.c:2927 holds *proportions* of the total (`currentden = tr[k][den] / totden`, atmovement.c:1542) taken before movement, while `tr[k][den]` at the call point already holds the post-move *numbers* (atmove... | [ ] |
| CONT-05 | atecology/atContaminants.c:427-431, 435 | For `habitat == EPIFAUNA` (called from atecology.c:2553) `tracerArray = getTracerArray(boxLayerInfo, EPIFAUNA)` = `localEPITracers`, then `cLevel = tracerArray[contaminant_tracer]` indexes the epibenthic array with a water-column tracer ... | [ ] |
| BIOL-01 | atecology/atbiology.c:479-507 | the loop variable `ij` runs over sediment layer indices (`pBox->sm.topk+1 .. sm.nz-1`) and `Copy_SED_Tracers()` reads `sm.tr[bm->current_layer = ij]`. | [ ] |
| BIOL-02 | atecology/atbiology.c:1474-1481 | `totDiet` is `alloc5d(K_num_tot_sp, K_num_max_cohort, K_num_tot_sp, wcnz+sednz, nbox)` => cohort dim = `K_num_max_cohort` (atlantismain.c:1176), and K_num_max_cohort is the max of `numCohorts` (atUtilGroupIO.c:302). | [ ] |
| BIOL-08 | atecology/additionalTracer.c:1311-1346 | `net_desorp = Pads_r * (...)` is a rate (EMS source writes it to derivatives), but here it is added straight to the local *state* with no `dt`. | [ ] |
| NUTR-08 | atecology/atdiversity.c:959-1023, 884-957 | `TRAIT_INDICES` includes `rsmid_trait_id` and `rsprop_trait_id` (atlantisboxmodel.h:2030-2031), which are used elsewhere (atprocess.c:2516, atdemography.c:914, and Get_Orig_Trait_value 1102-1107). | [ ] |
| NUTR-21 | atecology/atcoral.c:932-942 | When the model has no turf, `facilitate_space = 1.0`, then `ans = mum * (1 - compete_space) * (1.0 + facilitate_space)`, which doubles coral/sponge growth. | [ ] |
| EINIT-01 | atecology/atmigration.c:204-207 | `MIGRATION[sp].all_go[cohort][currentID] = MIGRATION[sp].all_go[cohort][migrationID];` copies `all_go` from itself. | [ ] |
| EINIT-03 | atecology/atmigration.c:158-421 | For multi-year migrators the year-0 "start with return" branch writes `this_currentID = currentID + counter*num_migrate` for `counter < MinYearsAway`, then `last_currentID` jumps to that value, so every (stage, migration) at yr 0 uses `(`... | [ ] |
| EINIT-21 | atecology/atq10.c:151, 194, 202 | `Get_Tcorr(bm, sp, H2Otemp, &bm->current_corr)` passes the global `bm->current_corr`, and the humped-Griffith branch writes `*current_corr = current_temp - opt_temp;`. | [ ] |
| EMISC-06 | atecology/attime.c:492-529 | For groups with `numMoveEntries == 4`, `index = floor(TofY/91.25)` is compared with `orig_index`, but line 527 then overwrites `moveEntryIndex = bm->QofY`, whose boundaries are 90/181/272. | [ ] |
| EMISC-22 | atecology/atbiolUtil.c:143-157 | A new zeroed 5D array is allocated and only `[speciesIndex][cohort][box]` is copied (the memcpy is commented out); then the old array is freed. | [ ] |
| EMISC-24 | atecology/atbiolUtil.c:210-219 | Same problem for habitats: only `BEDchange[habitatIndex]` is copied into the new `[K_num_bed_types][max][3]` array, so the scheduled changes of the other habitat types are zeroed. | [ ] |
| EMISC-26 | atecology/include/*.h:atEcologyLib.h:68; atassess/atclassical.c:1218-1219, 1555-1556 | The prototype and definition (atvertprocesses.c:342) are `Ecology_Spawn(sp, KSPA, FSP, SN, RN, X_RS, FSPB, fp)`, but the atclassical.c callers pass `..., scaled_FSPB[nc], X_RS[nc], ofp`, so X_RS and FSPB are swapped. | [ ] |
| QUOT-01 | ateconomic/atquota.c:130-133 | residual series indexed by market, but atmanage/atManageSetup.c:676-679 states "only one time series allowed" and builds `tseconid` from `tsResid[0]` only. | [ ] |
| QUOT-04 | ateconomic/atquota.c:825-827, 871-872, 915-916 | `linearPI` is defined `= 0` in include/atEconomicSetup.h:30 and is never allocated anywhere in the repo (grep finds only atquota.c, the commented-out ateffortquota.c, and the headers). | [ ] |
| QUOT-23 | ateconomic/ateconeffort.c:1048-1052, 1365 | `bm->Effort_hdistrib[ij][nf][today_effort] = 0;` sits inside the per-subfleet loop, but today_effort is fishery-level and accumulated with `+=` at 1365. | [ ] |
| ECON-01 | ateconomic/ateconomicsetup.c:349-351 | `bm->SpatialDisPUE[nf][ns][mth][b] += bm->SpatialCPUE[nf][ns][mth][b] * avg_loadDetFC;` sits after the `sp` loop, so `mth` is whatever the earlier `for (mth=0; mth<12; ...)` loops left: 12 (one past the end of the 12-month dimension, all... | [ ] |
| ECON-07 | ateconomic/ateconresponse.c:488, 609-619 | `boats_new[nf][ns] += num_new;` is never zeroed: grep finds no reset anywhere, and the initialise block at 222-238 resets only `boats_free`. | [ ] |
| ECON-17 | ateconomic/ateconresponse.c:1259 | With deemed values on, "unlimited" allowed catch is set as `totallowedcatch = no_quota * bm->X_CN * mg_2_kg`. | [ ] |
| ECON-18 | ateconomic/atEconomicAnnual.c:496-515 | `tot_expecteffort += bm->EffortSchedule[nf][ns][month][hist_id];` is inside the species loop, so annual effort is multiplied by the number of catchable species. | [ ] |
| ECIO-13 | ateconomic/atFishMIP_OSP_Bridge.c:513-520 (and 542, 673, 730) | `catchBySpFishery[k * nfishery + nf]` uses a different `nfishery` for each species. | [ ] |
| ECIO-14 | ateconomic/atFishMIP_OSP_Bridge.c:538-544 | In Step 3, `totalCatch += catchBySpFishery[k * nfishery + nf]` uses `nfishery` left over from the last species of the Step 2 loop, not species k's own value, so `prod[k]` (and the price) is summed from the wrong slots. | [ ] |
| ECIO-15 | ateconomic/atFishMIP_OSP_Bridge.c:669-691 | The Step 4 per-fishery loop has three faults. | [ ] |
| ECIO-16 | ateconomic/atFishMIP_OSP_Bridge.c:728-736 | In Mode C, `c = catchBySpFishery[k * nfishery + nf]` again uses the stale `nfishery` from Step 2 rather than species sp's value, so the catch weights for origprice are taken from the wrong slots. | [ ] |
| ECIO-17 | ateconomic/atFishMIP_OSP_Bridge.c:480, 576-578 | `gp` is declared `int` (l.480), but `FishMIP_Adapter_GetGlobalPriceForYear()` returns `double` (adapter .h:112). | [ ] |
| HARV-11 | atharvest/atHarvestCatch.c:232 | `Get_Fishery_Group_Change_Scale(bm, nf, sp, bm->flagchangeF, mFC_num_changes_id, mFCchange[sp])` passes the global flag value (1) as the parameter index. | [ ] |
| HRV2-20 | atimplementation/atImplementationAnnual.c:81, 95-103, 164-168 | `high_trigger_tripped` starts at 1 and is only cleared when a species falls between the low and high thresholds. | [ ] |
| HRV2-25 | atimplementation/atImplementationParamIO.c:31, 35, 38 | `TAC_num_changes_id` is a species-fishery parameter: it is in the SP enum (atlantisboxmodel.h:985), read by Util_XML_Read_Impacted_Group_Param, and used as `SP_FISHERYprms[sp][nf][TAC_num_changes_id]` (atImplementationAnnual.c:338). | [ ] |
| TIER-01 | atmanage/atManageTier.c:487-492 | `tac_change = (TAC_old - TACpost)/TAC_old; if (tac_change > inc_check) TAC = TAC_old + inc_check*TAC_old; else if (tac_change < bm->prop_decTAC) TAC = TAC_old - dec_check*TAC_old;` The sign is reversed: a big proposed *decrease* gives a ... | [ ] |
| TIER-02 | atmanage/atManageTier.c:501-503 | `if (TAC < minCatch) TAC = FunctGroupArray[species].speciesParams[MinCatch_id];` sets the TAC to the fraction (e.g. | [ ] |
| TIER-03 | atmanage/atManageTier.c:535-536 | `if (bm->TACamt[species][nf][now_id] > no_quota) bm->TACamt[species][nf][now_id] = no_quota;` is inside the `nfishery` loop but uses the stale `nf`, which equals K_num_fisheries (or Nfleets) left over from earlier loops. | [ ] |
| TIER-04 | atmanage/atManageTier.c:394-399 | `RBC = RBCparam[RBCest_id];` is read *before* `Apply_RBC_Buffers()` changes RBCest_id. | [ ] |
| TIER-07 | atmanage/atManageTier.c:3539-3549 | `case sp_rollover:` has no `break`, so it falls through to `default` and calls `quit("")`. | [ ] |
| TIER-13 | atmanage/atManageTier.c:1131, 1147 | `for (ia = 0; ia <= this_nselage; ia++) ... propgt05[ia] = 1;` reads `numatage[nselage]` and writes `propgt05[nselage]`. | [ ] |
| TIER-15 | atmanage/atManageTier.c:1173-1176 | `nz` is the index of the last stored Z (it starts at -1 and is incremented before use), but `for (i=0; i<nz; i++)` and `meanz = sumz/nz` treat it as a count. | [ ] |
| TIER-17 | atmanage/atManageTier.c:1435 | Restart branch: `fvalF[nF] = UnTransform(simplex[1][1], 0.01);` takes the S95 parameter of the second vertex. | [ ] |
| TIER-18 | atmanage/atManageTier.c:1459, 1472, 1478, 1482 | `nF` is the index of the last fitted fleet (it starts at -1, then ++), but the loops `i < nF` and the divisor `nF` treat it as a count. | [ ] |
| TIER-19 | atmanage/atManageTier.c:1614 | `for (iy=0; iy < Ncage_id; iy++)`: Ncage_id is the last index (it starts at -1), so the last year of catch-at-age is left out of the objective. | [ ] |
| TIER-21 | atmanage/atManageTier.c:1737-1740 (and 2726-2729) | `step1 = (F40-F20)*(Fcurr-F20); frbc = F48 / step1;` is dimensionally wrong (1/F). | [ ] |
| TIER-27 | atmanage/atManageTier.c:2291, 2295, 2388 | `k = simplex[0][0]; xpar[1] = simplex[0][0];` and in the objective `in_K = xpar[0];`. | [ ] |
| TIER-28 | atmanage/atManageTier.c:2397-2398 | `step1 = (1 - (bt/K)^p) - T5_removal[iy-1]; B = bt + bt*r/p*step1;` multiplies the catch removal by `bt*r/p`. | [ ] |
| TIER-32 | atmanage/atManageTier.c:2639-2649 | `for (iy...) { sumcatch = 0.0; ... }` resets the sum every year, so DCAC uses only the final year's catch and not the cumulative catch over `num_years`. | [ ] |
| TIER-35 | atmanage/atManageTier.c:2826-2827, 2835, 2848, 2851, 2863-2867 | `DoCPUE(bm, year, species, llogfp)`, `DoPrager(bm, year, species, ...)`, `Call_Tier3(bm, year, species, ...)` and `TierFiveSAFE(bm, year, species, totsumcatch, ...)` all pass (year, species) where the prototypes (lines 75-76, 1899, 57) e... | [ ] |
| TIER-36 | atmanage/atManageTier.c:2938-2950 | `for (iy = year-1; iy >= 0; iy++)` counts *up* without bound (runs off the end of EffortData). | [ ] |
| TIER-37 | atmanage/atManageTier.c:2963 | `AnnCPUE[5-iy]` with iy going down to 0 writes `AnnCPUE[5]`, but AnnCPUE is allocated with 5 entries (atAssessTierSetup.c:215). | [ ] |
| TIER-38 | atmanage/atManageTier.c:3044, 3078, 3097, 3183, 3196, 3211 | `cosp = FunctGroupArray[sp].co_sp[sp];` uses the loop counter as the group index. | [ ] |
| TIER-39 | atmanage/atManageTier.c:3146-3147 | `if (bm->SP_FISHERYprms[cosp][nf][q_id] > 0.0) toteffort += bm->CumEffort[nf][b];` sits after the nf loops have finished, so nf == K_num_fisheries and both arrays are read out of bounds. | [ ] |
| TIER-41 | atmanage/atManageTier.c:3216 | `Call_Tier3(bm, year, species, llogfp)` has its arguments swapped, and inside the loop over cosp it should assess `cosp`. | [ ] |
| TIER-46 | atmanage/atManageTier.c:3950-3952, 3968-3970, 4022-4024, 4042-4044 | `if (diff < closest_slope) ref_sp = ind_sp;` never updates `closest_slope` (or `closest_slopeB`), so ref_sp ends up as the *last* candidate (possibly `sp` itself), not the closest. | [ ] |
| TIER-49 | atmanage/atManageTier.c:4156, 4164-4172 | `chdir(dirname)` never returns to the parent directory, so later relative file I/O and next year's mkdir end up nested. | [ ] |
| TIER-50 | atmanage/atManageTier.c:4280-4282 | The third block under the "Year, Catch and CPUE" header writes `CatchData` a second time where CPUE (`CPUEgen`) is expected, so the R assessment receives catch as CPUE. | [ ] |
| MNG-01 | atmanage/atManage.c:160-163 | TAC_over is allocated `Util_Alloc_Init_3D_Int(K_num_basket, K_num_fisheries, K_num_tot_sp)` (atManageSetup.c:1260) => `[sp][nf][basket]`, and every other use is `TAC_over[sp][nf][..]` (lines 818, 859, 959, atHarvestDiscards.c:248). | [ ] |
| MNG-02 | atmanage/atManage.c:959, 978, 1111, 1133, 2405 | third dimension is allocated with `K_num_basket` (atManageSetup.c:1260; K_num_basket = max basket size, usually 1, atManageSetup.c:1291-1297) but is indexed by region/stock `nreg` up to `K_num_active_reg` / `numStocks`. | [ ] |
| MNG-07 | atmanage/atManage.c:245-257 | In the `!bm->EffortModelsActive` branch `Effort_Displacement(bm, ..., orig_FCpressure, &FCpressure, ...)` is called with `FCpressure` never initialised (declared line 86); Effort_Displacement reads `*FCpressure` (line 1919-1920) and sets... | [ ] |
| MNG-12 | atmanage/atManage.c:1894-1897 | bm->MPA is an access multiplier (1 = open, applied as `FCpressure * MPA` at 454; default 1.0, atManageSetup.c:1220). | [ ] |
| MNG-13 | atmanage/atManage.c:2067-2089 | read past FunctGroupArray. | [ ] |
| MNG-21 | atmanage/atManage.c:2718-2733 | if `Effort_hdistrib[..][today_effort]` is zero in all boxes (fleet idle/closed that day) then prop_effort is all 0, sum_prob stays 0 and no shot is ever placed, while nshot > 0 (it is computed from boat numbers, not effort) -> hang. | [ ] |
| MNG-31 | atmanage/atSS3assess.c:176, 200 | realpath with a non-NULL buffer returns `buf`, a stack array, so free() of a stack pointer aborts ("pointer being freed was not allocated") on every SS3 run. | [ ] |
| MNGA-01 | atmanage/atManageAnnual.c:333 | `spTotCumCatch = Harvest_Get_TotCumCatch(sp, nf, bm->thisyear);` is called outside any `nf` loop. | [ ] |
| MNGA-02 | atmanage/atManageAnnual.c:365, 371 | `FunctGroupArray[sp].co_sp_catch[nf][co_sp2]`: `co_sp_catch` is allocated `Util_Alloc_Init_2D_Double(K_max_co_sp, K_num_fisheries)` (atManageParamIO.c:933, so `[nf][K_max_co_sp]`) and filled as `co_sp_catch[nf][co_sp]` using the slot ind... | [ ] |
| MNGA-05 | atmanage/atManageAnnual.c:601-605, 898-900, 1001 | `tac_resetcount_id` is incremented only at line 1001. | [ ] |
| MNGA-06 | atmanage/atManageAnnual.c:766-777 (alloc atManageSetup.c:912-913) | `yr_id = floor(FC_period/364 + 0.5) + bm->thisyear; LeverQueue[sp][nf][yr_id] = ...`. | [ ] |
| MNGA-08 | atmanage/atManageAnnual.c:1378-1387 | `Braw += totfishpop[othersp]...; Bcurr += Assess_Add_Error(bm, er_case, Braw, ...)`. | [ ] |
| MNGA-09 | atmanage/atManageAnnual.c:1392-1397, 1482-1488 | Both loops run `groupIndex < max_co_sp` and pick `othersp = sp` only when `groupIndex >= K_max_co_sp`. | [ ] |
| MNGA-10 | atmanage/atManageAnnual.c:1421-1426 | `for (groupIndex = 0; groupIndex < K_max_co_sp + 1; ...) Fcurr += ...; Fcurr_sp[groupIndex] /= counter;`. | [ ] |
| MNGA-13 | atmanage/atManageAnnual.c:1527, 1617-1631 | `Braw` is declared at 1527 without initialisation and accumulated with `Braw += ...` in the SSB branch. | [ ] |
| MNGA-14 | atmanage/atManageAnnual.c:1802, 1864-1866 | `mFC` is computed once per fishery (1802), but inside the cohort loop `mFC = 0` is assigned when a cohort is outside [mFC_start_age, mFC_end_age). | [ ] |
| MNGA-15 | atmanage/atManageAnnual.c:1910-1917 | `case assess_M_est:` has no `break`, so it always falls into `default: quit(...)`. | [ ] |
| MNGA-22 | atmanage/atManageAnnual.c:2069-2139 | `avg_scale`, `num_scale` and `min_scale` are initialised per fleet but never updated from `lever_scale`. | [ ] |
| MNGA-24 | atmanage/atManageSetup.c:1083-1085 | Inside `for (i = 0; i < K_num_fisheries; i++)` the test uses `bm->TACamt[sp][nf][now_id]`. | [ ] |
| MNGIO-01 | atmanage/atManageParamIO.c:1069-1081, 517-524 | `POP_changes = 1; ... if (POP_changes > 0) bm->flagchangepop = 0;` is always true, so the flagchangepop setting from the input file is always switched off. | [ ] |
| MNGIO-08 | atmanage/atPGMSY.c:40-66, 452 | Nothing in the repo calls `Initialise_PGMSY()` (grep finds only its definition). | [ ] |
| MNGIO-09 | atmanage/atPGMSY.c:55-56, 127, 131, 338, 341 | `SSB` and `Depleted` are allocated with length `maxage` but indexed by projection year `iYr` (up to ProjYr-1), which overflows when ProjYr > maxage. | [ ] |
| MNGIO-10 | atmanage/atPGMSY.c:146-153 | `RBCspeciesParam[sp][AveF_id] += F_actFleet[sp][nf]` is never zeroed first. | [ ] |
| MNGIO-13 | atmanage/atPGMSY.c:319-320 | `totavailB[nf] += BIO[age][iYr]*sel*q; catch = BIO[age][iYr]*...` but BIO is only ever set for year 0 (line 269). | [ ] |
| MNGIO-14 | atmanage/atPGMSY.c:290, 321, 330 | `totF += Fused[nf]` runs inside the age and cohort loops and is only reset once per year. | [ ] |
| MNGIO-15 | atmanage/atPGMSY.c:329-334, 338-340 | SSB is accumulated only in the `age == 0` branch and uses `DEN[0][iYr]`, which is not set until line 340, after SSB has been used. | [ ] |
| MNGIO-18 ✔ | atmanage/atPGMSY.c:284, 402, 420, 429, 538-545 | `AvgCatFleet = Util_Alloc_Init_2D_Double(nYears, K_num_fisheries)` gives `[nf][nYears]` (atAssessTierSetup.c:303), but every use is `AvgCatFleet[this_year][nf]` or `[iYr][nf]`. | [ ] |
| MNGIO-20 | atmanage/atPGMSY.c:605 | `CatchData[f][r][this_year]` uses `r` left over from the loop at 547-549, so it equals NumRegions of the last fished species. | [ ] |
| MNGIO-21 | atmanage/atPGMSY.c:605-607 | `CscalarMetier[nf]` is computed and then read as `CscalarMetier[f]` (f = assessment fleet) to set `RBCupdated[nf]`. | [ ] |
| MNGIO-22 | atmanage/atPGMSY.c:652-659 | The F-rescaling block sits after the `for (sp...)` loop, so `sp == K_num_tot_sp`. | [ ] |
| MNGIO-27 | atmanage/atRlinkRBC.c:371-376 | `Util_Close_Output_File(bm->metierFile)` closes the file but cannot NULL the pointer (atUtil.c:200). | [ ] |
| MNGIO-28 | atmanage/atRlinkRBC.c:270-274 | `if (line_size == 1) { ThisSpID = atoi(line_buf); spID = ...}`. | [ ] |
| MNGIO-29 | atmanage/atRlinkRBC.c:284 | `tok = strtok(NULL, "\n")` uses "\n" instead of `seps` for later tokens, so the rest of the line becomes one token. | [ ] |
| MNGIO-30 | atmanage/atRlinkRBC.c:279 | `while (fgets(buffer,...) && (i < nrow))` reads a line before checking i<nrow. | [ ] |
| MNGIO-31 | atmanage/atRlinkRBC.c:267, 294 | `for (int sp = 0; sp < MULTISPestNspecies; ...)` shadows the outer `sp`. | [ ] |
| MNGIO-35 | atmanage/atRlinkRAssess.c:180, 202-207 | `rawwgt` is never initialised and never reset per cohort/box before `if (this_wgt > rawwgt) rawwgt = this_wgt;`. | [ ] |
| MNGIO-36 | atmanage/atRlinkRAssess.c:213 | `rawn = bm->boxes[b].tr[k][den];` runs after the `for (k...)` loop, so k == nz, one layer past the last water layer. | [ ] |
| SS3IO-01 | atSS3Link/atSS3LinkIO.c:61-72 (also 219-230) | `if ((fid = fopen(...)) == NULL) { ... AssessFail_id = 1; }` then falls through to `while (fgets(buf, buflen, fid) ...)` with `fid == NULL` -> crash whenever SS3 fails to produce Report.sso (the very case the warning is for). | [ ] |
| SS3IO-02 | atSS3Link/atSS3LinkIO.c:36, 61; 302, 315 | Both readers open bare `"Report.sso"` / `"ss3.par"` in the current working dir and ignore `folderName`. | [ ] |
| SS3IO-04 | atSS3Link/atSS3LinkIO.c:41, 146-160 | In the `Biology_at_age_in_endyr_with_CV=f(LAA)` block no lines are ever read: `strtok(buffer, seps)` tokenises the never-initialised local `char buffer[1000]`, so `values[12..16]` are garbage (or uninitialised malloc memory). | [ ] |
| SS3IO-05 | atSS3Link/atSS3LinkIO.c:149-153, 169-179 | MeanLenAgeM/SigmaLenAgeM/MeanWtAgeS/MeanWtAgeM are allocated `Util_Alloc_Init_4D_Double(nYears, MaxAge, Nsexes, stocks)` => `[stocks][Nsexes][MaxAge][nYears]` (atAssessTierSetup.c:281-285), and AccumAge = MaxAge + ceil(MaxAge/4) (atAsses... | [ ] |
| SS3IO-09 | atSS3Link/atSS3LinkIO.c:359, 374-375 | Starter file writes `"%s%s.dat", groupCode, fileName` where `fileName` is the starter's own path (`"<dir>//starter.ss"`) -> e.g. | [ ] |
| SS3IO-10 | atSS3Link/atSS3LinkIO.c:397-398 | `"1 # min age for calc of summary biomass"` is written twice. | [ ] |
| SS3IO-14 | atSS3Link/atSS3LinkIO.c:1148/1155-1156, 2281; 1164-1165, 2380 | `WriteSSCtl()` ends with `fclose(fid)` and `WriteSS330Files()` then calls `fclose(fidctl)` on the same FILE*; likewise `WriteSSFor()` closes and the caller closes `fidss` again. | [ ] |
| SS3IO-15 | atSS3Link/atSS3LinkIO.c:1221-1222 | Fleet definition line `" 1  -1  1  1  0  FLEET%d in metier %s\n"` has no `#`, so SS reads fleetname=`FLEET0` and then `in`, `metier`, `<code>` as the start of the next fleet's definition -> whole data file misparsed. | [ ] |
| SS3IO-16 | atSS3Link/atSS3LinkIO.c:1222-1300, 1333, 1346, 1446, 1574, 2018-2023, 2037-2039, 2240-2255 | All SS fleet numbers are written 0-based (`f`/`i`): catch lines `-999 1 %d`, CPUE setup `%d 1 0 0`, CPUE/discard/length/age observations, Q setup, Q parms, variance adjustments. | [ ] |
| SS3IO-17 | atSS3Link/atSS3LinkIO.c:1250-1255 | `fprintf(fid, "%d 1 %d", t, f)` has no trailing space and the next `fprintf(fid, "%e ", catch)` has no leading space, so fleet and catch merge (e.g. | [ ] |
| SS3IO-20 | atSS3Link/atSS3LinkIO.c:1486-1491, 1577-1590 | Writes `MaxAge+1` as `#_N_age_bins` but then only `MaxAge` bin labels (`a < MaxAge`) and `MaxAge` values per age-comp vector. | [ ] |
| SS3IO-21 | atSS3Link/atSS3LinkIO.c:1493-1499 (also 943-949, 3323-3334) | Ageing-error definition rows must have Nages+1 entries (ages 0..accumulator age); loops write `a < AccumAge` = AccumAge entries. | [ ] |
| SS3IO-22 | atSS3Link/atSS3LinkIO.c:1187, 1561-1566 | declared without init at 1187) or stale, `it==1` (whole) is labelled retained, `it==2` (discard) labelled whole. | [ ] |
| SS3IO-25 | atSS3Link/atSS3LinkIO.c:1801 | `Mat_slope_Fem` line `"    -3     3 %f %f %f     0.8    0   -3  0 0 0 0 0 0 0"` has 15 fields (three `%f` plus 0.8), whereas each MG parameter line takes 14 (cf. | [ ] |
| SS3IO-28 | atSS3Link/atSS3LinkIO.c:2115, 2117 | Age-selex pattern lines for fishing fleets (`"%d  0  0  0  # FLEET %d in metier %s"`) have no `\n`, so the next fleet's/survey's `pattern 0 0 0` line lands after the `#` and is discarded by SS -> only one age-selex line is read for all f... | [ ] |
| SS3IO-30 | atSS3Link/atSS3LinkIO.c:2368-2373 | `mgtTAC_old` (atlantisboxmodel.h:2999) is never assigned anywhere in the repo, so forecast catches are 0 (or garbage); the TAC is kept in `RBCspeciesParam[sp][TAC_old_id]` (set atManageTier.c:140, 238; used by `Write_Forecast_File()` 558). | [ ] |
| SS3IO-31 | atSS3Link/atSS3LinkIO.c:2374 | stray comma after the newline, so each following line starts with `,` (e.g. | [ ] |
| SS3-05 | atSS3Link/atSS3DataGen.c:350-383 | `totnum` is accumulated but never used. | [ ] |
| SS3-06 | atSS3Link/atSS3DataGen.c:369-381 | The divisions sit inside `for (ij=0; ij<K_num_sexes; ij++)` but always target `[FEMALE]`. | [ ] |
| SS3-10 | atSS3Link/atSS3DataGen.c:525-532 | `sumcatch += TotCatch[f][r][iyr]`: `TotCatch` is allocated (atAssessTierSetup.c:160) but never written anywhere in compiled code (grep finds only this read). | [ ] |
| SS3-11 | atSS3Link/atSS3DataGen.c:569-573, 582 | `SizeNumCaught/SizeNumDiscard` hold *numbers* (atHarvest.c:586/594, `age_catch = fishing / Wgt`). | [ ] |
| SS3-18 | atSS3Link/atSS3DataGen.c:803, 1384-1391 | Age samples are generated by `GenSample(..., NageVec, ..., agecomp, ...)`, which bins by *length*: `l = floor(rawsizedata/bin_size)`, clamped to `vecsize-1`. | [ ] |
| SS3-21 | atSS3Link/atSS3DataGen.c:1062-1074 | `l = (int)floor(li / bin_size); lenprops[r][l] += ...` has no upper bound check, unlike GenSample:1388. | [ ] |
| SS3-22 | atSS3Link/atSS3DataGen.c:1126-1157, 1171-1174 | `a` is set once per cohort, but `a += aa` runs inside the *box* loop. | [ ] |
| SS3-24 | atSS3Link/atSS3DataGen.c:1242-1301 | `fishableBiomass` mixes units. | [ ] |
| SS3-26 ✔ | atSS3Link/atCloseKin.c:77-100 (uses 122-487) | All local arrays are allocated with dims in *index* order instead of reversed. | [ ] |
| SS3-28 | atSS3Link/atCloseKin.c:72, 140 | `FracLenS[j][s][a][this_min][l]` with `this_min = HistYrMin_id` uses a calendar year (e.g. | [ ] |
| SS3-30 | atSS3Link/atCloseKin.c:154-163, 490-498 | `Util_Close_Output_File(bm->closekinCFile)` and the closes of the samp/PO/HS files fclose the stream, but the bm pointers are not set to NULL (atUtil.c:200 takes `FILE*` by value). | [ ] |
| SS3-31 | atSS3Link/atCloseKin.c:241, 270-273, 343-353 | `psurv_syay[s][y-1][this_maxage][y]` with `this_maxage = MaxAge`, `curr_a` allowed to reach MaxAge (`if (curr_a < MaxAge) curr_a++`), and `ap_b2 = MaxAge` (then `fec_sa[s][ap_b2]`) all index one past the age extent MaxAge (fec_sa: atAsse... | [ ] |
| SS3-33 | atSS3Link/atCloseKin.c:382 | `for (int s1 = 1; s1 < Nsexes; s1++) comps += ...` skips offspring sex 0, although the comment says "sex of offspring doesn't matter". | [ ] |
| SS3-34 | atSS3Link/atCloseKin.c:496 vs 630 | The call `writeCloseKinNcompsfiles(bm, sp, year, ...)` does not match the signature `(bm, int year, int sp, ...)`. | [ ] |
| PHY1-01 | atphysics/aticeIO.c:68, 707, 744, 747 | `int *tsiceid;` is a file-global never allocated anywhere in the repo (grep: only uses are in this file). | [ ] |
| PHY1-02 | atphysics/aticeIO.c:730, 746 | `char key[BMSLEN];` is never set, then `strcmp(key, this_ts->ts.varname[b])` compares garbage. | [ ] |
| PHY1-12 | atphysics/attempsalt.c:497-499 | `if(propInput->input_Varname) c_free1d(propInput->input_Varname);`: `input_Varname` is declared `char input_Varname[BMSLEN]` in the struct (atlantisboxmodel.h:3997). | [ ] |
| PHY1-20 | atphysics/atphysics.c:700-702 | `if (!_finite(newwctr[24][0][Salinity_i]))` hard-codes box 24 and runs every time step. | [ ] |
| PHY2-07 | atphysics/atsourcesink.c:201-225 | In the no-water branch, `nut_scale = Get_PSS_Scalar(bm, s, variableIndex);` is followed by a copy of Get_PSS_Scalar's own loop, which multiplies `nut_scale` again by the same `pssMult` factors. | [ ] |
| PHY2-10 | atphysics/atvertgeom.c:699-704, 596-597 | `for(kk=k; kk<ice->currentnz; kk++) copy_ice_layer(ice,kk,kk+1);` shifts forward in ascending order, so every layer k..currentnz ends up as a copy of layer k and the deeper ice layers are lost. | [ ] |
| PHY2-12 | atphysics/atagetracerIO.c:465-522 | `ncvarput(fid, vid, start, count, val)` (whole-nbox record) is called inside `for b` → `for nf`, after only `val[b]` has been set. | [ ] |
| PHY3-01 | atphysics/atparamIO.c:105 | `snprintf(convertedXMLFileName, sizeof(convertedXMLFileName), "%s", biologyfile)`: `convertedXMLFileName` is a `char *` from malloc (line 89), so `sizeof` gives the pointer size (8). | [ ] |
| PHY3-06 | atphysics/atbioirrig.c:60,69,78,87 | `if (!_finite(newwc[24][0][Salinity_i]))`: leftover debug code with box 24 hard-coded, run for every box. | [ ] |
| PHY3-12 | atphysics/atdietIO.c:117,197,208,212,219 | The loop runs `cohort < numCohortsXnumGenes * numStocks` and indexes `bm->totDiet[b][k][sp][cohort][prey]`. | [ ] |
| PHY3-15 | atphysics/atsettle.c:92 | `if( bm->boxes[b].type != BOUNDARY && bm->boxes[b].type)`: BOX_TYPE has `INTERNALBOX = 0, BOUNDARY, LAND` (atlantisboxmodel.h:3597). | [ ] |
| PHY3-17 | atphysics/atbioturb.c:102-119,150-166,196-213 | `transfer_sed(sm,k,dst,thick)` moves the whole sediment slab (dz and *all* tracers, atvertgeom.c:398-417), but it is called inside `for(n...)` over tracers. | [ ] |
| PHY3-18 | atphysics/atbioturb.c:98,143,189 | `thick[k] = ... * bm->dt * 86400.0`: the comment assumes dt is in days, but `bm->dt` is in seconds (read via Util_XML_Get_Value_Time, atparamIO.c:178; bioirrig uses `bm->dt` directly). | [ ] |
| PHY3-21 | atphysics/atsuspension.c:153-156 | `else if (insed && dissol && can_be_moved)` has no braces, so line 156 `newsed[b][tk][n] -= newsed[b][tk][n]*pwv/(...)` runs for **every** tracer. | [ ] |
| UTIL-06 | atlantisUtil/atUtilXML.c:1601-1603 | `valueCopy = malloc(STRLEN)` (STRLEN=2000, atlantisboxmodel.h:331), then `strcpy(valueCopy, valueStr)` copies the full XML array string with no length check. | [ ] |
| UTIL-17 | atlantisUtil/atUtilGroupIO.c:910-912 | `contaminantTracers` and `contamPropTracers` are `Util_Alloc_Init_2D_Int(num_contaminants, numCohorts)`, which gives `[numCohorts][ncontam]`. | [ ] |
| UTIL-18 | atlantisUtil/atUtilGroupIO.c:894-905 | The atomic-ratio arrays hard-code 3 habitats: `ratioLost = 3D(num_atomic_id, nCxG, 3)` gives `[3][nCxG][..]`, and `ratioLostGlobal = 4D(..., 3, 3)` gives `[3][3][nCxG][..]`. | [ ] |
| CNV1-01 | ConvertAtlantis/atBioltoXML.c:1780-1786 | `tempStr[strlen(secondlastLetter) - strlen("popratioStock") - 2] = '\0';`: `secondlastLetter` points at the last 2 chars, so `strlen(secondlastLetter)` is 2. | [ ] |
| CNV2-06 | ConvertAtlantis/atAssesstoXML.c:689-690 | `Num_Sexes` is declared `XML_TYPE_BOOLEAN` and read with `binary_check`. | [ ] |
| CNV2-19 | ConvertAtlantis/atGroupsXML.c:34-42 vs 261-264 | `isCollisionRisk` is read as an extra column when `bm->flagindustry_on` is set, and the reader requires it (atUtilGroupIO.c:421-422). | [ ] |
| CNV2-20 | ConvertAtlantis/atRunXML.c:49-73 | The reader requires `K_num_vessel_sizes` in `SubModelSetup` when `flagindustry_on == simple_industry_model (1)` (atparamIO.c:136-138, isRequired=1, which quits if absent at atUtilIO.c:71-72). | [ ] |
| LINK-02 | atCLAMLink/atCLAMComm.c:245-254 | `i` has no upper bound, but the last dimension is MAXNUMOUTPUTSTATES (10) (atCLAMLinkSetup.c:68). | [ ] |
| LINK-08 | atCLAMLink/atCLAMLinkIO.c:342, 542-544 | `linkID` is initialised to 0 and never incremented, while `numScenarioLinks++` counts up. | [ ] |
| LINK-09 | atCLAMLink/atCLAMLinkIO.c:396, 423-458 | `CLAMScenarioOptions = malloc(...)`: only the strategies present in the XML are filled, and `isLocked`/`timeLockOutStarted` are never initialised anywhere (grep shows them set only in atCLAMImplement.c:56-57 after a strategy is implement... | [ ] |
| LINK-10 | atCLAMLink/atCLAMLinkIO.c:413, 424, 147 | `maxOptionValue`, `numStrategyOptions` and `numLevers` come from XML with no limit, but `strategies[]` is fixed at maxStrategyOptions (7) (atCLAMLinkPrivate.h:59) and `strategyLevers[]` at 20 (line 47). | [ ] |
| LINK-16 | atCLAMLink/atCLAMManage.c:440 | indices are 0-based and -1 means "not found" (lines 405-406). | [ ] |
| LINK-23 | atlink/atComms.c:321-328 | `str` is a `char *`, so `sizeof(str)` is 8 and the output is truncated to 7 characters. | [ ] |
| LINK-27 | atlink/atlinkconversion.c:41 | `Util_Init_2D_Double(..., eweLinkModule->convertPRMS.io_nrow, ...)` uses the global `eweLinkModule` (declared in atLinkModule.h:49, never assigned) instead of `bm->eweLinkModule`, so this dereferences NULL. | [ ] |
| LINK-28 | atlink/atlinkconversion.c:212, 223, 229, 247 | `nid = *PhysioChemArray[fgIndex].tracerIndex;` is computed but the nutrient and physchem blocks read `tr[k][bid]` and `sm.tr[k][bid]`. | [ ] |
| LINK-30 | atlink/atlinkconversion.c:266 | `area` is allocated with 0.0 (atlinkexport.c:53-54) and never set anywhere, so every exported value becomes inf/NaN. | [ ] |
| LINK-33 | atlink/atlinkexport.c:38-55, 91-103 | `initialiseLinkModule()` mallocs `eweLinkModule` and immediately uses `convertPRMS.ingroup`, `outgroup`, `io_ncol` and `io_nrow` (uninitialised) as allocation sizes. | [ ] |
| LINK-34 | atlink/atlinkexport.c:139-140 | the second line should set `numcols`. | [ ] |
| LINK-35 | atlink/atlinkexport.c:134, 152-154 | writes the out-of-bounds element [4][4] of an array allocated `alloc2d(nc,nr)` = [nr][nc] = [4][4]. | [ ] |
| LINK-52 | atbrokerlink/Deserialiser.c:261, 293, 321, 351, 378, 408 | `strcpy(interface->groupName, request->groupid)` copies an unbounded network string into `char groupName[1024]` (atBrokerLink.h:22): a remote input overflows a fixed buffer. | [ ] |
| LINK-58 | atbrokerlink/ImportExportData.c:312, 318 | `boxValues` has `numBoxes` entries indexed by the polygon position `i` (compare `boxValues[i]` in Link_SetBiomass:126-135 and Link_SetMortality:401-410), but it is indexed by the box id `b`. | [ ] |
| LINK-59 | atbrokerlink/ImportExportData.c:88, 126, 144 | `epi = 0` is only set in the first (fraction) loop. | [ ] |
| SJW1-06 | sjwlib/datafile.c:317-321 | When rewinding is on and `r` is past the last record: `leftover = floor(r / lengthdf); *newt = r - leftover * df->records[ilow];`. | [ ] |
| SJW2-01 | sjwlib/mapproj.c:298-347, 561-604 | `add_argument()` copies the caller's string pointers into the new array (`new_args[i+1] = args[i]`, line 587). | [ ] |

## B.2 Full findings by folder and file

### B.2.1 atecology/

#### atecology/atdemography.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| DEMO-01 | 1712-1726 (used 455-458, 1751-1754, 2115-2118, 2149-2152; atmovement.c:1298) | wrong default / logic | H/H | `double KWSR_force = 0.0; if (bm->flag_modify_KWSR && (KWSRforceid > -1)) {...} return KWSR_force;`. When `flag_modify_KWSR` is on, every species gets `KWSRforce_id = -1` unless it has a column in the KWSR forcing file (atbiolsetup.c:1078-1083; non-vertebrates always stay -1). For those species the function returns 0, so recruits are given SN = RN = 0 (Vertebrate_Reproduction 468-469, atmovement.c recruits). Function: `Get_Recruit_Size_Forcing()`. | Start with `KWSR_force = KWSR_sp` (the unforced value) and only override it when `KWSRforceid > -1`. | [ ] |
| DEMO-02 | 2730, 2708, 2738-2772 | off-by-one / cohort×genotype | H/H | `nextcid = cohort + numGeneTypes; if (cohort < num_cohorts)` where `num_cohorts = numCohortsXnumGenes - 1`. With numGeneTypes > 1, the cohorts in the oldest age class (e.g. cohort = NxG-2) pass this test, so `nextcid >= numCohortsXnumGenes` and MIGRATION DEN/SN/RN/aging/cohort_migrating`[nextcid]` are read and written past the end (allocated with `cohort` rows, atBiologyXMLParamIO.c:4348-4380). This is reachable when flagsenesce is on, because the oldest cohorts are then not skipped (line 383). Those cohorts also miss the plus-group branch. Update_Aging_Numbers (2669) uses the correct test. Function: `Update_Migration_Array()`. | `if (cohort <= (num_cohorts - numGeneTypes))`. | [ ] |
| DEMO-03 | 4453-4460 | wrong formula | H/H | Semelparous case: `spawning_biomass_contrib = this_biom * (1.0 - scaled_FSPB[cohort]);`. In Update_Invert_Aging (5111) `spawning_biomass_contrib` is the biomass removed. The comment says the loss should be "all adults spawning". The code removes the immature fraction instead of the mature one, so a fully mature cohort (FSPB = 1) loses nothing and a partly mature cohort loses its non-spawners. This is the vertebrate survivor factor (Apply_Semelparous_Deaths 1098) used as a loss. Function: `Invertebrate_Reproduction()`. | `this_biom * FunctGroupArray[species].scaled_FSPB[cohort]`. | [ ] |
| DEMO-04 | 4341-4345, 4425 | buffer overrun | H/M | `for (stk = 0; stk < maxstock_id + 1; stk++) tot_yoy[species][stk] = 0.0;` and `tot_yoy[species][maxstock_id] += ...`. `tot_yoy` is `Util_Alloc_Init_2D_Double(nstock, K_num_tot_sp)`, i.e. `[sp][K_num_stocks_per_sp]` (atbiolsetup.c:2931, nstock = K_num_stocks_per_sp at 2799). It has no extra "aggregate" slot. When a species has the maximum number of stocks, this writes into the next species' row, or past the end of the heap block for the last species. Function: `Invertebrate_Reproduction()`. | Allocate `tot_yoy` with nstock+1 columns, or drop the aggregate slot. | [ ] |
| DEMO-05 | 3763 | stale loop variable / OOB | H/M | `if (FunctGroupArray[species].agingVERT[cohort][k][DEN_id] > 0)`. Here `k` is left over from the stock loop at 3697-3703, so `k == maxstock`. agingVERT is `[ncxg][K_num_stocks_per_sp][3]` (atUtilGroupIO.c:838), so this either reads one row out of bounds or reads an unused stock slot (always 0). As a result, juveniles are never pushed into the MIGRATION array. Function: `Ecology_Update_Vertebrate_Cohorts()`. | Sum over stocks (or test `tot_new_mat[cohort] > small_num`) before the inner `for (k...)` loop. | [ ] |
| DEMO-06 | 3717-3718, 3752-3753 | cohort×genotype / wrong index | H/M | `cid = floor(cohort / numGeneTypes); nextcid = cid + numGeneTypes;` combines an age index with the genotype count. The next age class for this cohort is `cohort + numGeneTypes`. The test `(cohort < age_mat) && (nextcid >= age_mat)` also compares the cohort×gene index with age_mat, which is in age classes. Both are only correct when numGeneTypes = 1. With more genotypes, the wrong cohorts trigger the juvenile-to-migration transfer and the wrong box-0 tracers are overwritten (3797-3805). Function: `Ecology_Update_Vertebrate_Cohorts()`. | `nextcid = cohort + numGeneTypes;` and test `(cid < age_mat) && (cid + 1 >= age_mat)`. | [ ] |
| DEMO-07 | 4624-4649 (used 4353) | accumulator overwritten | H/M | `MigSpawn = flag_allow_spawn * spawn_active * prop_mat * MIGRATION[species].DEN[cohort][qid];` is assigned, not accumulated, inside the qid loop. Only the last migration-queue entry's spawn is returned, but DEN is still reduced for every entry. Function: `Ecology_Do_External_InVert_Spawning()`. | Use `MigSpawn +=` with a per-qid temporary for the DEN update. | [ ] |
| DEMO-08 | 4639-4641 | logic | M/M | Semelparous branch: `MIGRATION[species].DEN[cohort][qid] *= (1.0 - spawn_active);` ignores both `flag_allow_spawn` (ReprodAllowed) and `prop_mat`. Migrants that are not allowed to reproduce, or are immature, are still killed in proportion to FSP. Function: `Ecology_Do_External_InVert_Spawning()`. | `DEN *= (1.0 - flag_allow_spawn * prop_mat)` (or the intended spawning fraction). | [ ] |
| DEMO-09 | 4376-4428, 4885-4956 | repeated per-cohort call | H/M | The recruitment block (`overall_checkday == sp_spawnday + additional_day`) runs inside the per-cohort loop with no cohort guard. `Ecology_Find_Invert_Recruits`, `Find_Final_Invert_Recruit_Distribtuion`, `tot_yoy += spawned_biomass` and `calcTrackedMort[..][start_id] +=` therefore run once per cohort×gene. In Find_Final, `totrecruit += ...` runs on every call, and for `external_recruit` so does `MIGRATION.DEN[ngene][mid] += hab_scalar*spawned_biomass`. Recruits are therefore added numCohortsXnumGenes times (and numGeneTypes more times, see DEMO-10). Functions: `Invertebrate_Reproduction()`, `Find_Final_Invert_Recruit_Distribtuion()`. | Do the recruitment block once per gene, e.g. only when `basecohort == 0`. | [ ] |
| DEMO-10 | 4885, 4893 | shadowed parameter | M/M | The `ngene` parameter is immediately overwritten by `for (ngene = 0; ngene < numGeneTypes; ngene++)`. The `spawned_biomass` computed for one genotype (4383) is therefore written to every genotype's num_recruits/totrecruit/MIGRATION entries. With numGeneTypes > 1, the last caller's value wins, or values accumulate with `+=`. Function: `Find_Final_Invert_Recruit_Distribtuion()`. | Remove the inner loop and use the `ngene` argument. | [ ] |
| DEMO-11 | 4302-4322, 4499-4503 | flag not reset | M/M | `sp_Spawn_Now` and `sp_Age_Now` are set to 1 when any cohort matches its Spawn_Now/Age_Now day and are never reset per cohort. Every later cohort in the loop is then treated as spawning and aging, so `Get_Proportion_Aging` and aging shifts are applied to cohorts not due to age today. The second loop inherits the value too. Function: `Invertebrate_Reproduction()`. | Reset `sp_Spawn_Now = sp_Age_Now = 0` at the top of each cohort iteration, and keep a separate "any" flag for the queue update. | [ ] |
| DEMO-12 | 1004-1033 (used 469, 517, 2671) | stale state | M/H | `EMBRYO[species].IndSpawn[cohort]` is set only for cohorts that actually spawn. A cohort that is skipped (`Spawn_Now != today` or `li < min_li_mat`) keeps the IndSpawn value from the previous box on the same spawn day; it is only zeroed on non-spawn days (271-273). Since li depends on the local SN+RN, a cohort can spawn in box A and not in box B. In box B, `Update_Local_dPool` still subtracts the stale IndSpawn from RN (2923), and it also affects aging RN (2671) and recruit RN (469). Function: `Ecology_Do_Internal_Age_Structured_Spawning()`. | Set `EMBRYO[species].IndSpawn[cohort] = 0.0` at the top of each cohort iteration, before the `continue`s. | [ ] |
| DEMO-13 | 921-934 | flag not reset | H/M | `allowed_to_spawn` is set to 0 inside the `mid` loop when one migrant group is too small (`li < min_li_mat`) and is never reset to 1 for the next `mid`. All later migration-queue entries for that cohort are then skipped. Function: `Ecology_Do_External_Age_Structured_Spawning()`. | Use a per-mid local, e.g. `int mid_ok = allowed_to_spawn;` inside the loop. | [ ] |
| DEMO-14 | 1077-1108 | logic | M/M | The death factor `(1 - scaled_FSPB[cohort])` is applied to every cohort with FSPB > 0 whenever any cohort spawned (`semelparous_parents`/`_migrants` are species-level flags). Cohorts whose Spawn_Now was not today, or which failed the size test, still die. It also writes unconditional `fprintf` lines for every cohort, box and spawn event. Function: `Apply_Semelparous_Deaths()`. | Track a per-cohort "spawned" flag (e.g. IndSpawn[cohort] > 0 or per-mid flags) and apply deaths only to those. Guard the fprintf with do_debug. | [ ] |
| DEMO-15 | 1904-1958 | OOB index / logic | M/M | `mid = migIDmatch[qid]; if (mid < 0) recruit_outside = 0; ... else { MIGRATION[species].recruit[ngene][mid] = 0; ...}`. The else branch runs exactly when `mid < 0`, so it writes `recruit[ngene][-1]` (heap corruption). In that same branch, `totrecruit += ...` (1948) is then wiped by the unconditional `totrecruit[...] = 0.0` (1957). Also, for external_recruit, AlignVsMigration never sets -1 (atannualbiology.c:956-963) and migIDmatch defaults to 0, so a missing match quietly sends recruits to queue entry 0. Function: `Find_Final_Recruit_Distribution()`. | Skip the MIGRATION write when `mid < 0`, move the zeroing into the `recruit_outside` branch, and initialise migIDmatch to -1. | [ ] |
| DEMO-16 | 523-532 | stale variable | M/M | The comment says that if general aging happens here too, this step should be skipped. But `need_update > -1` tests the local copy read at 522, before `Update_Age_Distrib(...,0)` sets `speciesParams[need_update_id] = -1` (3396-3397). The test is always true (0 or 1), so the recruit-related update also runs. It then overwrites the youngest cohorts' tempPopRatio/boxPopRatio with `recVERTpopratio`, which was not refreshed this step: when need_update = 1, Get_Latest_Additions takes the tempPopRatio branch (2846). Function: `Vertebrate_Reproduction()`. | Re-read `need_update = (int)speciesParams[need_update_id]` before the second test (or test `need_update != 1`). | [ ] |
| DEMO-17 | 3157-3159, 3290-3292 | stale loop variable | H/M | `do_test = 0; if(rec_related) do_test = 1; else if(cohort \|\| (!cohort && k)) do_test = 1;` is evaluated outside the `for (k...)` loop, where `k == classize` from the previous loop. The test is therefore always true, and the intended exemption for cohort 0 / sub-class 0 never applies: the minimum floor `maxvalue/sp_recruitRange` is forced on it. Update_Invert_Cohorts (5219) evaluates the same test correctly inside the k loop. Function: `Update_Age_Distrib()`. | Move the test inside the k loop: `if ((rec_related \|\| cohort \|\| k) && tempPopRatio < ...)`. | [ ] |
| DEMO-18 | 3081-3114 | flag not reset / wrong index | H/M | `stock_done[s_id]` is reset once per call (2992) but is shared across all `cohort` and `qid` iterations. After the first cohort marks a stock done, no later cohort or queue entry updates tempPopRatio for that stock. The outer `for (i < maxstock_id)` loop is also redundant. In the same block, the `recVERTpopratio[stock_id]...` update (3104) uses the function argument `stock_id` instead of `s_id`. Function: `Update_Age_Distrib()`. | Reset `stock_done` at the start of each cohort/qid iteration, and use `s_id` in 3104. | [ ] |
| DEMO-19 | 5259-5268, 5288-5289 | wrong variable / logic | H/M | The loop runs over `sp` but the body uses `species` throughout (`FunctGroupArray[species].isCover`, `.coverID`). The same term is added K_num_tot_sp times if the recruiting species is itself a cover group; otherwise nothing is added. `isCover == CORAL \|\| isCover == SPONGE` compares a TRUE/FALSE flag to group-type constants, so `like_coral` is effectively never set; it should test `groupType`. `hab_scalar` also starts at 1.0 and is capped at 1.0, so positive habitat preferences can never raise it. Function: `Get_Habitat_Mediated_Recruit_Scalar()`. | Use `FunctGroupArray[sp]` inside the loop, test `FunctGroupArray[sp].groupType == CORAL/SPONGE`, and review the 1.0 start value against the cap. | [ ] |
| DEMO-20 | 2064-2070 | cohort×genotype | H/M | `for (n = 0; n < numCohorts; n++) tot_adult_den += scaled_FSPB[n] * tr[k][NumsTracers[n]];`. scaled_FSPB and NumsTracers are indexed by cohort×gene. With numGeneTypes > 1, this sums the wrong (young) entries and misses the adults, so the larvae-per-adult trim is wrong. Function: `Sanity_Check_Recruits()`. | Loop `n < numCohortsXnumGenes`. | [ ] |
| DEMO-21 | 2122-2126 | accumulator overwritten | H/L | `tot_yoy[species][i] = (...)` sits inside the `ngene` loop, so only the last genotype's YOY is kept (the migrant add at 2141 is `+=`). This affects the YOY diagnostic output (atbiolmessage.c:1631). Function: `Store_Recruitment_Diagnostics()`. | Zero `tot_yoy[species][i]` before the ngene loop and use `+=`. | [ ] |
| DEMO-22 | 2237-2254, 2284-2285 | double count / inconsistent | M/M | `if (fished_chrt == ngene) biomass += tr[k][den]; ... biomass += tr[k][den];` counts the fished cohort twice and counts every youngest-cohort density plus larvae. `calcNstart[expect_id]` is then set to this value unconditionally (2284). Reset_Mortality (4167-4168, 4206) instead defines calcNstart as the fished cohort's numbers only. The two routines overwrite each other's pseudo-assessment starting N with different quantities. The history reset at 2167 is correctly limited to `fished_chrt < numGeneTypes`, but the final assignment is not. The non-AGE_STRUCTURED branch (2257) also does not skip BOUNDARY boxes, unlike the branch above it. Function: `Store_Recruitment_Diagnostics()`. | Remove the extra `biomass +=` at 2247 and only set calcNstart when `fished_chrt < sp_numGeneTypes`. Skip BOUNDARY boxes. | [ ] |
| DEMO-23 | 2550-2581 (used 389-392, 2682) | uninitialised | M/L | In the evolution branch, `*this_p_ageup` is never set. The caller's `this_prop_ageup` (declared without an initial value at 116) is then passed to `Update_Aging_Numbers` and on to `Age_Contaminants_Store(..., this_p_ageup)` when contaminants are tracked. Line 2547 `ng = cohort - basecohort;` is also wrong (missing `* sp_numGeneTypes`), but it is overwritten before use. Function: `Get_Numbers_Aging()`. | Set `*this_p_ageup = num_aging / (VERTinfo DEN + small_num)` in the evolution branch, and initialise this_prop_ageup. | [ ] |
| DEMO-24 | 1742-1826 | dead code / log spam | H/L | `direct_recruit_entry` is only ever set to 1, inside the same `if` that sets `recruit_outside = 1` (1775-1780). So `if (recruit_outside && (!direct_recruit_entry))` (1789) can never be true and the whole MIGRATION add block is dead. In practice delivery happens in atmovement.c:1292. Line 1748 also writes an unconditional `fprintf(bm->logFile, ...)` for every gene × queue entry on every recruiting call. Function: `Recruit_Migration()`. | Remove the dead block, or read `direct_recruit_entry` from speciesParams if it was meant to be live. Guard the fprintf with do_debug. | [ ] |
| DEMO-25 | 2848-2853, 2872-2874, 2883-2885 | swapped index / format args | H/L | The debug check `bm->tempPopRatio[stock_id][ngene][cohort][0]` swaps dimensions: it should be `[stock_id][species][ngene][0]`, because tempPopRatio is `[nstock][sp][ncxg][maxspage]` (atbiolsetup.c:2899). In all three fprintfs the format `"%s-%d in Box%d-%d (ngene %d)"` is given `cohort, ngene, box, layer`, so the labels are misaligned. 2850 also prints every call for which_check without do_debug. Function: `Get_Latest_Additions()`. | Fix the index to `[stock_id][species][ngene][0]` and reorder the args to `cohort, box, layer, ngene`. | [ ] |
| DEMO-26 | 2649-2653 | wrong index / log spam | H/L | The check `!_finite(bm->tempPopRatio[stock_id][species][cohort][i])` uses `i == 0` after the loop and the wrong cohort; it should check `[nextcid][0]`, which is what is printed. The `\|\| (species == bm->which_check)` condition has no `do_debug`, so it prints on every aging event for the checked species. Function: `Update_Aging_Numbers()`. | Check `[nextcid][0]` and add `&& do_debug`. | [ ] |
| DEMO-27 | 1610-1613 | copy-paste message | H/L | The warning says "...as flagtempdepend_move = %d" and passes `bm->flagtempdepend_move`, but the guarding flag is `bm->flagtempdepend_reprod`. The comment also says "movement". Function: `Get_Enviro_Recruit_Forcing()`. | Print `flagtempdepend_reprod`. | [ ] |
| DEMO-28 | 1641-1645 | identical branches | L/L | `if (sp_q10receff == 1) enviro_scalar *= Tscalar; else if (sp_q10receff == 2) enviro_scalar *= Tscalar;` has identical branches. Either option 2 was meant to differ, or the branches should be merged. Function: `Get_Enviro_Recruit_Forcing()`. | Confirm the intended behaviour for flagq10receff = 2. | [ ] |
| DEMO-29 | 5192 | log spam | H/L | `fprintf(bm->logFile, "species %s, totsum = %e\n", ...)` is unconditional and runs for every cohort, every box with activity, and every call. The log grows large (compare the recent commit that removed a similar printf). Function: `Update_Invert_Cohorts()`. | Guard with a debug flag or remove. | [ ] |
| DEMO-30 | 4427 | wrong index | M/L | `bm->calcTrackedMort[species][0][stock_id][start_id] += spawned_biomass;` always credits genotype 0. Elsewhere, recruits for genotype `ngene` live in cohort index `ngene`. Function: `Invertebrate_Reproduction()`. | Use `[ngene]`. | [ ] |


#### atecology/atmovement.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| MOVE-01 ✔ | 2030 | swapped indices | H/H | `newden[sp][n][chkbox][kk] += den_diff;` — newden is allocated `Util_Alloc_Init_4D_Double(ncells, nlayer, ncoh, nsp)` => `[sp][n][layer][box]` (atbiolsetup.c:2869) and used everywhere else as `newden[sp][n][k][ij]`. Here the box index is put in the layer slot, so any `chkbox >= wcnz` writes out of bounds (memory corruption) and the wrong cell gets the displaced fish. Function: `Ecology_Total_Verts_And_Migration()` (flagenviro_displace branch). | `newden[sp][n][kk][chkbox] += den_diff;` | [ ] |
| MOVE-02 | 2049-2050 | wrong operator | H/H | `bm->boxes[ij].tr[k][DL_id] = VERTinfo[..SN_id] * totden * den_diff;` — this sets the labile/refractory detritus tracers to the killed biomass instead of adding to it, wiping out the detritus already in the cell. It also runs once per cohort, so each cohort overwrites the one before. The value is also mg N for the whole cell (per-individual N × numbers × proportion) written into a mg N m-3 tracer with no divide by volume. Function: `Ecology_Total_Verts_And_Migration()` (flagenviro_kill). | Use `+=` and divide by `dz[k]*area` | [ ] |
| MOVE-03 | 1552 | wrong variable | H/M | `(-1.0 * bm->boxes[bm->current_box].botz) <= ...[maxtotdepth_id]` — this is inside the `ij` box loop, but it uses `bm->current_box`, which is old state left over from the process loop. The two checks next to it use `bm->boxes[ij].botz`, and atbiolsetup.c:3290 does the same max-total-depth test with `ij`. So the depth exclusion for forage-dependent movement uses the wrong box. Function: `Ecology_Total_Verts_And_Migration()`. | `bm->boxes[ij].botz` | [ ] |
| MOVE-04 | 2997, 3073-3075 | wrong type / truncation | H/M | `int qrt, next_qrt, this_HowFar, ...` — this_HowFar is an int but gets `bm->HowFar` (a double fraction in [0,1), attime.c:209) or `FunctGroupArray[sp].HowFar`. It truncates to 0, so prescribed invertebrate movement (line 3382) never blends between seasons and jumps when the season changes. The vertebrate routine declares it `double` (line 867). Also, `FunctGroupArray[].HowFar` is itself declared `int` (atlantisUtil/include/atFunctGroup.h:94) but is assigned a fractional remainder (attime.c:524), so it is always 0 for vertebrates too whenever numMoveEntries != 4. Function: `Ecology_Invert_Migration()`. | Declare `double this_HowFar`; change atFunctGroup.h:94 to `double HowFar` | [ ] |
| MOVE-05 | 3382 | wrong index | H/M | `FunctGroupArray[sp].distrib[ij][next_qrt][cohort]` — distrib is allocated `Util_Alloc_Init_3D_Double(2, numMoveEntries, nbox)` => `[nbox][entries][2]`, indexed by stage (atUtilGroupIO.c:918; the vertebrate code uses `[stage]`, e.g. line 1932). Using `cohort` (0..numCohortsXnumGenes-1) reads out of bounds for an invert with more than 2 cohort×gene entries, and reads the wrong stage when genes > 1. Function: `Ecology_Invert_Migration()`. | Use `[stage]` | [ ] |
| MOVE-06 | 3624, 3627, 3637 | wrong index | H/M | `amt * tempdistrib[k][cohort]` — tempdistrib is `[wcnz][K_num_max_stages]` (atbiolsetup.c:2898), and `Get_Vertical_Distribution()` only fills `[k][juv_id]` and `[k][stage]` (tracker_id = stage, line 4154). Line 3620 correctly reads `tempdistrib[k][stage]`. Indexing by cohort goes out of bounds or reads unset columns when cohorts×genes > stages. Function: `Ecology_Invert_Migration()`. | `tempdistrib[k][stage]` | [ ] |
| MOVE-07 | 3498 | wrong index | H/M | `MIGRATION[sp].DEN[cohort][qid] += migbiom * totboxden[invert_biosum];` — the total biomass for this cohort is stored at `totboxden[invert_biosum+cohort]` (lines 3274, 3333, 3550, 3567). Cohort 0's biomass is used for every cohort, so migrant biomass for cohorts > 0 is wrong. Function: `Ecology_Invert_Migration()`. | `totboxden[invert_biosum+cohort]` | [ ] |
| MOVE-08 | 3274, 3333, 3550, 3567, 818 | off-by-one / buffer overrun | H/M | `totboxden[invert_biosum+cohort]` with invert_biosum = 1 (atlantisboxmodel.h:813) reaches index numCohortsXnumGenes. totboxden is allocated with only `K_num_max_cohort*K_num_max_genetypes` (atbiolsetup.c:2922). atbiolsetup.c:2905-2907 computes `totdensize = K_max_invert_cohorts+1` "as need an additional slot for propsum in invertebrate movement code", but applies it to totden/totden_check, not totboxden. So it overruns when an invert has the maximum cohort×gene count. Functions: `Ecology_Invert_Migration()`, `Store_Min_Max_Avg()`. | Allocate totboxden with `totdensize` (atbiolsetup.c:2922) | [ ] |
| MOVE-09 | 3101-3106, 3376-3379, 3397 | off-by-one | H/M | `if (habitatType == EPIFAUNA) maxk = 0; ... for (k = 0; k < maxk; k++)` — with maxk = 0 the loop never runs, so epifauna skip all temperature, salinity, O2, ice, light, noise and contaminant limits, both for non-mobile groups (3095 block) and for prescribed movement. The epifauna `epi[pid]` branches inside those loops can never run. | `maxk = 1` for EPIFAUNA | [ ] |
| MOVE-10 | 3033-3036 | inverted condition | M/H | `if (bm->flagday) day_part = 0; else day_part = 1;` — `distrib_VERTICAL[0]` is "night" and `[1]` is "day" (atBiologyXMLParamIO.c:93-94), and flagday = 1 means daytime (attime.c Time_ODay; comment at line 906). The vertebrate routine passes `day_part = bm->flagday`. Invertebrates therefore use the night profile in the day and the day profile at night (diel migration reversed), unless this reversal is a deliberate legacy choice. Function: `Ecology_Invert_Migration()`. | `day_part = bm->flagday;` | [ ] |
| MOVE-11 | 4067-4072 | off-by-one | H/M | `else year = bm->larvalYears;` — connectivitymatrix only holds data for years `0..larvalYears-1` (read with count `min(larvalYears, numRunYears)`, atbiolsetup.c:3727). Past the end of the data with larval_rewind off, the code indexes a slot that was never filled (all zeros), so larvaldistrib = 0 and there is no recruitment. Function: `Larval_Dispersal()`. | `year = bm->larvalYears - 1;` | [ ] |
| MOVE-12 | 4008-4042 | logic (overwrite instead of AND) | H/M | Each check overwrites `go_there` (temperature fail -> 0, then salinity OK -> 1, then O2 OK -> 1). Only the last active criterion counts. Light and noise set `go_there = ceil(coefft/enviro)`, which can override earlier failures. Invaders can spread into cells that fail temperature or salinity limits. Function: `Invade_Spread()`. | Start with `go_there=1` and set it to 0 on any failure | [ ] |
| MOVE-13 | 3800 | uninitialised/wrong variable | H/M | `bm->boxes[b].tr[k][bm->InvaderPID] *= (1.0 + InvadeScalar);` — in the `conditional_invader` case, InvadeScalar is never set (it is only computed in `simple_invader`), so it stays 0.0. Invertebrate entrants are never scaled, while the vertebrate branch two lines above uses `bm->InvaderScalar`. Function: `Ecology_Invading_Species()`. | Use `bm->InvaderScalar` | [ ] |
| MOVE-14 | 3929-3931, 3938, 3941 | copy-paste / wrong variable | H/M | `bk = k; if(bk >= bm->boxes[nb].nz) bk = bm->boxes[nb].nz - 1;` — k loops `< boxes[nb].nz`, so this test can never be true. It was clearly meant to clamp against the source box `b`, since `boxes[b].tr[bk]` is read next. When nb is deeper than b, unused layers of b are read. Function: `Ecology_Invading_Species()`. | `if (bk >= bm->boxes[b].nz) bk = bm->boxes[b].nz - 1;` | [ ] |
| MOVE-15 | 3810-3813, 3820-3910, 3918-3923 | normalisation error | M/M | The local box's scalar is written to every layer but added to `tot_scalar` only once. Neighbour scalars are added to `tot_scalar` once per invader cohort n (loop at 3820) but stored only once per [nb][k], with the last n winning. Each source layer then keeps `s_b/tot` and sends `s_nb,k/tot` to each neighbour layer, so the fractions for a layer do not add to 1 and invader biomass/numbers are not conserved. Function: `Ecology_Invading_Species()`. | Build the scalars per source layer and normalise per layer, outside the cohort loop | [ ] |
| MOVE-16 | 3830 | copy-paste | M/M | `sp_likeCANYON = HABITATlike[..][CANYONcover_id];` is the only habitat term not multiplied by the box's area fraction. REEF, FLAT and SOFT are multiplied by `boxes[nb].reef/flat/soft`, and `double canyon` exists in the box struct (atlantisboxmodel.h:3710). Any canyon-liking invader therefore treats every box as suitable habitat. Function: `Ecology_Invading_Species()`. | `* bm->boxes[nb].canyon` | [ ] |
| MOVE-17 | 1613-1618 | use before compute | H/M | Sticky ddepend with flagmother: `if ((roc[ij][age_mat] / roc_wgt) > ...)` runs inside the `n` loop while `n < age_mat`, so roc[ij][age_mat] for this species and timestep has not been computed yet. It holds the value from the previous species or timestep. (The same test at 1864 runs after all roc are computed, so it is fine.) Function: `Ecology_Total_Verts_And_Migration()`. | Compute roc for all n first, then do the sticky boxden pass in a second loop | [ ] |
| MOVE-18 | 472-492 | double counting | M/M | `if(!prey_counted[prey][n])` is checked outside the `predstage` loop. Inside it, every predator stage whose gape window fits adds `(sn+rn)*den` to preyamt again, so prey are counted twice when a predator has 2 stages that both match. This always happens under flag_olddiet, where preyavail is the same for both stages. Function: `Get_Vert_Diet()`. | Break out of the predstage loop once `prey_counted[prey][n]=1`, or move the check inside | [ ] |
| MOVE-19 | 1159-1160, 1231-1243, 1246-1253 | dead code | H/L | `DEN` is clamped to 0 at 1159, so `if (DEN < 0.0)` at 1231 can never be true, and the SN/RN reset under flagavgmig never happens. The 1246 test `dayt > Return_Now+Return_Period` can never be true inside the enclosing condition at 1120 (`dayt < Return_Now+Return_Period`), so `Checked[qid]` is never set here. Function: `Ecology_Total_Verts_And_Migration()`. | Test `<= 0.0` at 1231; move the 1246 block outside the return-window `if` | [ ] |
| MOVE-20 | 1270-1278 | logic | M/M | The larval_queue_extension reset happens inside the `n` (gene) and `qid` loops. After the first gene with `stock_id == numStocks-1` resets it to 0, later genes skip their pre-model larvae, which are lost. If `stock_id < 0` it is set to 0 (line 1188), so the reset only fires when numStocks == 1. Function: `Ecology_Total_Verts_And_Migration()`. | Reset after the n/qid loops | [ ] |
| MOVE-21 | 1392 | accumulator in wrong loop | H/L | `sp_amt += totden[sp][n];` sits inside the stock loop `ij < maxij`, so the total is multiplied by K_num_stocks_per_sp. This loosens the "less than one individual present" skip at 1404. Function: `Ecology_Total_Verts_And_Migration()`. | Accumulate outside the stock loop | [ ] |
| MOVE-22 | 1473, 4384/4416 | truncation | M/M | `contract_sp = (int)FunctGroupArray[sp].speciesParams[contract_tol_id];` (and `int contract_sp` in `Check_Conditions()`) truncates the pH tolerance contraction, e.g. 0.5 -> 0. The invertebrate routine (3204) correctly keeps it as a double. Functions: `Ecology_Total_Verts_And_Migration()`, `Check_Conditions()`. | Remove the cast; declare as double | [ ] |
| MOVE-23 | 2383-2386 | order dependency | M/M | `if(!totad[ngene]) recruit_hdistrib = distrib[..][adstage];` is inside the box/layer loop, while totad accumulates across boxes. Boxes processed before the first adult is found get the prescribed distribution; later empty boxes get 0. The fallback is also not added to totad, so the later normalisation is off. Function: `Ecology_Total_Verts_And_Migration()`. | Check `totad` after the box loop and apply the fallback to all boxes | [ ] |
| MOVE-24 | 2378-2379, 2390-2391 | inconsistent normalisation | M/M | The `recruit_hdistrib` numerator leaves out `noise_effect * light_effect`, but the `totad` denominator includes them. After normalisation (2409) the distribution does not sum to 1, and light/noise reduce the denominator instead of steering larvae away. Function: `Ecology_Total_Verts_And_Migration()`. | Apply the same factors to both | [ ] |
| MOVE-25 | 2056, 2060-2097 | stale state | M/M | `LocalPopCount[n]` is only updated when `newden > 0`, so cells with no animals keep the previous cell's value and trigger suckling transfers that should not happen. `suckler_contam_today_id = 1` (2097) is reset inside the per-cell (ij,k) loop, so the "skip on birth day" set in atdemography.c:473 only applies to the first cell. Function: `Ecology_Total_Verts_And_Migration()`. | Reset LocalPopCount every cell; move the 2097 reset after the box loop | [ ] |
| MOVE-26 | 1973-1985 vs 4306-4320 | double application | M/M | When enviro_depend is set, light/noise avoidance is applied to `tempdistrib` in `Get_Vertical_Distribution()` (which feeds `vertdistrib` into newden) and then applied again to newden at 1973-1985. Inverts get the same double effect (3399-3411). | Apply in one place only | [ ] |
| MOVE-27 | 2789 | format mismatch | H/M | `quit("Time: %e box%d-%d %s-%d ...", bm->dayt, groupCode, n, ij, k)` — the arguments are in the wrong order (`%d` gets a char*, `%s` gets an int), so the error path will likely segfault instead of printing. Function: `Ecology_Total_Verts_And_Migration()`. | Pass `bm->dayt, ij, k, groupCode, n` | [ ] |
| MOVE-28 | 2672, 2649, 2511/2515/2769/2773, 3630 | log text | H/L | 2672 prints `period = %e` with the int `Leave_Period[qid]` (UB). 2649 prints `Box %d;%d` with `ij, ij` (should be ij, k). The negative-size diagnostics and the invert quit print `bm->current_box, bm->current_layer` instead of `ij, k`. | Fix the specifiers and arguments | [ ] |
| MOVE-29 | 2080 | off-by-one | M/L | The second suckling loop starts at `ad_cohort = ad_start_cohort`, which the first loop (2066) already handled, so that cohort's contaminants are transferred twice. Function: `Ecology_Total_Verts_And_Migration()`. | Start at `ad_start_cohort - 1` | [ ] |
| MOVE-30 | 3184, 3460 | accumulator not reset | H/L | `numScalar_final` is never reset per cell, so after the first constrained cell every cell counts as "impacted" in the flagenviro_warn log. Function: `Ecology_Invert_Migration()`. | Reset to 1.0 per cell | [ ] |


#### atecology/atBiologyXMLParamIO.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| BXML-01 | 2959-2971 | wrong node / wrong check type | H/M | `bm->shrink_tol = Util_XML_Read_Value(..., attributeGroupNode, integer_check, "shrink_tol")`: (a) the value is looked up under `attributeGroupNode` (the "Diet" group), but the converter writes it inside the "Shrinking related" sub-group (ConvertAtlantis/atBioltoXML.c:3025-3030). `Util_XML_Get_Node` only searches direct children (`parent/Attribute[@name=..]`, atUtilXML.c:951-958), so with `isRequired=1` every run with `size_shrinking_type > 0` stops with "Cannot find atlantis parameter 'shrink_tol'". (b) `shrink_tol` is a double proportion ("Tolerated proportion of a month", atlantisboxmodel.h:5110, XML_TYPE_FLOAT), so `integer_check` rejects any fractional value. The species params on the next lines correctly use `attributeGroup`. Function: `Read_Diet_Information_Values()`. | Use `attributeGroup` and `no_checking` (or `proportion_check`). | [ ] |
| BXML-02 | 3825-3826 | wrong check type | H/M | `dissolv_coefft = Util_XML_Read_Value(..., integer_check, "<contam>_dissolv_coefft")`: this is a continuous coefficient (converter declares XML_TYPE_FLOAT, atBioltoXML.c:3729-3731; used as a multiplier in atContaminants.c:709). `Check_Value` with `integer_check` quits on any non-integer (atUtilIO.c:181-186), so realistic values such as 0.1 are rejected. Function: `Read_Contaminant_Values()`. | Use `no_checking` (or `proportion_check`). | [ ] |
| BXML-03 | 3631-3642 | index mismatch | M/M | `InvaderStartNums` is read into slots `[0 .. n-1]` with `n = maxInvaderAge - minInvaderAge`, but the only consumer reads `bm->InvaderStartNums[n]` for `n = minInvaderAge .. maxInvaderAge-1` (atmovement.c:3735-3738). When `minInvaderAge > 0` the wrong entries are used, and the top `minInvaderAge` cohorts get 0 (the array is zero-initialised with size K_num_max_cohort, atlantismain.c:2009). Function: `Read_Invading_Species_Values()`. | Store `bm->InvaderStartNums[minInvaderAge + cohort] = values[cohort]` (or change the consumer to index `[n - minInvaderAge]`). | [ ] |
| BXML-04 | 4229-4242 | wrong parameter key / NULL dereference | M/H | The reader looks up `"KMIG_INVERT%d"` (KMIG_INVERT0, KMIG_INVERT1, ...) via `parameterNameMigNum`, but the converter only creates one attribute called `"KMIG_INVERT"` (atBioltoXML.c:2807-2809, InitKMIGInvertXML at 1425-1444). The returned `attributeNode` is never NULL-checked, so it is passed as `parent` to `Util_XML_Read_Array_Double` -> `Util_XML_Get_Node`, which dereferences `parent->doc` (atUtilXML.c:1040). Any invertebrate with `sp_geo_move` and `num_migrate >= 1` would segfault. The existing comment ("This wasn't originally set - was that intentional?") suggests the change was unverified. Function: `Read_KMIG_INVERT_XML()`. | Either look up `parameterName` (matching the converter) or make the converter write per-queue nodes; also add `if (attributeNode == NULL) quit(...)`. | [ ] |
| BXML-05 | 1486, 1576, 1622, 1773, 1876, 2022, 2048, 2111, 2117, 2425, 4103 | buffer overrun (latent) | M/M | These loops run `cohort < numStages` and index `cohortString[cohort]` (a 2-element array, line 95). Several also index arrays sized 2 in the stage dimension: `HABITATlike`/`ICE_HABITATlike*` (atlantismain.c:1926-1929, `[sp][2][..]`), `distrib_VERTICAL`/`distrib_OVERWINTER` (atUtilGroupIO.c:776-779), and `distrib` (atUtilGroupIO.c:918). `numStages` is only checked to be >= 1 (atUtilGroupIO.c:294), with no upper bound of 2, so a group with 3+ stages reads past `cohortString` (passing a garbage `char*` as the XML key) and writes out of bounds. Functions: `Read_Habitat_XML()`, `Read_Ice_Habitat_XML()`, `Read_VERT_Function()`, `Read_Sesonal_Dist_XML()`, `Read_Detritus_Sediment_Food_Avail()`, `Read_Vertebrate_Food_Avail()`, `Read_PStock_XML()`, `Read_MigrateIO_Box_XML()`. | Add a check at group load that `numStages <= 2`, or size these arrays and strings by `K_num_max_stages`. | [ ] |
| BXML-06 | 2033-2036 | always-false condition | M/L | `pSPEat[cohort][prey][SED] = values[b]; if ((pSPEat[cohort][prey][SED] == 0) && values[b] > 0) pSPEat[0][prey][SED] = values[b];`: the test runs on the value just assigned, so it can never be true and the fallback copy to stage 0 is dead code. The intended test was probably on `pSPEat[0][prey][SED]` (fill juveniles from adults when juveniles are 0). Function: `Read_Detritus_Sediment_Food_Avail()`. | Test `pSPEat[0][prey][SED] == 0`, or delete the block if it isn't wanted. | [ ] |
| BXML-07 | 2121 | format/argument mismatch | H/L | `quit("Error: Unable to find parameter '%s/%s/%s' in input file %s\n", errorString, cohortString[preyCohort], fileName)`: 4 `%s` but only 3 arguments, which is undefined behaviour (likely a crash) on this error path. Function: `Read_Vertebrate_Food_Avail()`. | Use `'%s/%s' in input file %s`. | [ ] |
| BXML-08 | 977-989, 1007-1019, 1037-1049 | always-true check | H/L | The "incomplete set of entries" checks can never fire. For T: `counter = base = Tchange_max_num*wcnz`, so `diff` is always 0. For S and pH: `nset = counter / bm->wcnz` is integer division of `X*wcnz` by `wcnz`, so `floor(nset) == nset`. `Parse_Into_Array` already enforces the count, so these checks are dead code. Function: `Read_Physical_Chem_Values()`. | Remove them, or check the real number of tokens parsed. | [ ] |
| BXML-09 | 855, 964, 981, 1041, 1314, 1332, 1349, 1366, 1604, 3652 | copy-paste error/log text | H/L | Error messages name the wrong node. 855: "Chemical_Limitation" (node is PhysicalChemical). 964: "Physical_Change" (node is Temperature_Effects). 981 and 1041: "Salinity_Effects/vertTchange_mult" and ".../vertPHchange_mult" (should be Temperature_Effects and pH_Effects). 1314, 1332, 1349 and 1366: all say "Bleaching" (should be Calcification, ReefGrowth, ReefRecruit and RugosityModel). 1604: "Group_Ice_Preference" (should be Group_IceReprod_Preference). 3652: prints "Reading the Reproduction values" in `Read_Ice_Values()`. Functions: as listed. | Correct the strings. | [ ] |
| BXML-10 | 3637 | format/argument mismatch | H/L | `quit("...'Invading_Species/InvaderStartNums' in input file %s\n", "InvaderStartNums", fileName)`: there is one `%s` but the first argument is the literal "InvaderStartNums", so the message prints that instead of the file name. Function: `Read_Invading_Species_Values()`. | Drop the extra `"InvaderStartNums"` argument. | [ ] |
| BXML-11 | 3398 | format specifier vs type | H/L | `quit("... ageClassSize for %s of %e\n", ..., FunctGroupArray[guild].ageClassSize)`: `ageClassSize` is an `int` printed with `%e` (UB). It is unreachable in practice because atUtilGroupIO.c:288 already quits when it is < 1. Function: `Read_Popratio_Stock_XML()`. | Use `%d`. | [ ] |
| BXML-12 | 2807-2810 | NULL dereference | M/L | `attributeGroup = Util_XML_Get_Node(..., "Turbidity")` is not NULL-checked before `Read_Cohort_Species_Param_Values(bm, fileName, attributeGroup, ...)`, which calls `Util_Get_Node_Name(parent)` and `Util_XML_Get_Node(..., parent, ...)` on it. If the Turbidity sub-group is missing from an estuary model, this crashes instead of giving a clear message. Every other sub-group lookup in this file is checked. Function: `Read_Diet_Information_Values()`. | Add `if (attributeGroup == NULL) quit("Diet/Turbidity ...")`. | [ ] |
| BXML-13 | 3828-3837 | memory leak | H/L | `values` from `Util_XML_Read_Array_Double(... "_contam_interaction" ...)` is never freed, so it leaks once per contaminant. Function: `Read_Contaminant_Values()`. | `free(values);` after the copy loop. | [ ] |


#### atecology/atecology.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| ECOL-01 | 3304 | copy-paste / wrong id constant | H/M | `Reset_Arrays(bm, SED, it_count, boxLayerInfo, llogfp);` inside the ice box. Because of this the `[ICE_BASED]` slices of `preyEatenGlobal[cohort][habitat_type][hab]`, `NutsProdGlobal/NutsLostGlobal/DetritusProdGlobal/DetritusLostGlobal[habitat_type]`, `ratio*Global[habitat_type]` and `DebugFluxInfo[which_check][ICE_BASED]` are never zeroed at it_count==1. The SED slices get zeroed instead. `preyEatenGlobal` is accumulated with `+=` (atprocess.c:2304,2308), so `Reconcile_Global_Values(bm,...,ICE_BASED)` (line 3448) adds global values for it_count>1 that grow with every step and every box. Function: `Ice_Box()`. | `Reset_Arrays(bm, ICE_BASED, it_count, boxLayerInfo, llogfp);` | [ ] |
| ECOL-02 | 625-626, 2368-2369 | unit mismatch (biomass vs numbers) | H/M | `SizeNumDiscard[cohort][nf][box] += DependDiscardsTot*dt*cell_vol` and `FCdiscard[guild][cohort] += ...` add mg N to arrays that hold numbers at age. The normal path stores `age_discard*dt*cell_vol` with `age_discard = discards / Wgt` (atharvest/atHarvest.c:587,596 -> atprocess.c:2008,2015). Line 2369 also leaves out `*dt*cell_vol`, which is inconsistent with line 626. Only matters when `flagdepend_dis` is on. Functions: `Water_Column_Box()`, `Epibenthic_Box()`. | Use `DependDiscardsTot[nf][guild][cohort] * bm->dt * sp_scalar` (sp_scalar = cell_vol/Wgt) for both arrays in both boxes | [ ] |
| ECOL-03 | 2310-2345 | accumulator not reset in right loop | H/M | `sp_scalar = 1.0` is set once per guild (2310), but for EPIFAUNA groups `sp_scalar *= wcLayerThick;` (2345) runs inside the cohort loop. For multi-cohort epifauna (AGE_STRUCTURED_BIOMASS) the scalar becomes wcLayerThick^n, so `dead[cohort] += DependDiscardsTot*sp_scalar` (2373) is wrong for cohorts after the first one with discards. Function: `Epibenthic_Box()`. | Reset `sp_scalar = 1.0;` at the top of the cohort loop | [ ] |
| ECOL-04 | 2616-2622 | copy-paste / wrong habitat index | H/M | In the old-diet multi-cohort branch for EPIFAUNA groups: `grazeEaten += preyEaten[ct][WC]; grazeEaten *= localWCTracers[...totNTracers[cohort]] / totalBiomass;`. But `totalBiomass` is summed from `localEPITracers` (2602) and the single-cohort branch uses `preyEaten[0][EPIFAUNA]` (2624). So predation is taken from the wrong habitat and split by the wrong tracer. Function: `Epibenthic_Box()`. | Use `preyEaten[ct][EPIFAUNA]` and `localEPITracers[...]` | [ ] |
| ECOL-05 | 2747-2753 | copy-paste / wrong tracer array | H/M | In the SED branch (old diet, multi-cohort): `grazeEaten *= (localWCTracers[...totNTracers[cohort]] / totalBiomass);` while `totalBiomass` is summed from `localSEDTracers` (2734). Function: `Epibenthic_Box()`. | Use `boxLayerInfo->localSEDTracers[...]` | [ ] |
| ECOL-06 | 2631-2632 | swapped indices | H/M | `grazeEaten = FunctGroupArray[guild].preyEaten[0][cohort];` (TODO in the code says "check this is really around the right way"). `preyEaten` is `Util_Alloc_Init_2D_Long_Double(num_active_habitats, numCohortsXnumGenes)`, i.e. `[cohort][hab]` (atUtilGroupIO.c:821). This indexes habitat by cohort: it reads the wrong value, and for cohort >= num_active_habitats it reads past row 0. Applies to seagrass with `flag_macro_model`. Function: `Epibenthic_Box()`. | `preyEaten[cohort][EPIFAUNA]` (or whatever habitat the macro model stores into) | [ ] |
| ECOL-07 | 1136-1147 | wrong operator (= vs +=) | H/M | Layers run top->bottom (atbiology.c:414). The top layer sets `PelDem_ratio_i = PDratio`, but the else branch "increment[s] the global average" with `=`: `localDiagTracers[PelDem_ratio_i] = PDratio / nz;` (same for PiscPlank). So the final indicator is just the bottom layer ratio/nz. The top-layer value is also not divided by nz. Function: `Water_Column_Box()`. | Top: `= PDratio / nz`; else: `+= PDratio / nz` (same for FVFPratio) | [ ] |
| ECOL-08 | 3923-3930 | off-by-one / range | M/M | `for (kij = 0; kij < EPIFAUNA; kij++)` reconciles NutsProd/NutsLost/DetritusProd/DetritusLost only for WC and SED. When called with `ICE_BASED`, the `[ICE_BASED]` entries used by atNutrient.c:480,486 never get their once-per-dt (Global) contributions for it_count>1, while Reset_Arrays zeroes them on every call (up to num_active_habitats). Function: `Reconcile_Global_Values()`. | Loop `kij < bm->num_active_habitats` (or at least include `habitatType`) | [ ] |
| ECOL-09 | 3331-3340 | copy-paste | M/M | The ice box sets `SP_IRR` for groups with `habitatType == SED` and SED phyto types (copied from Sediment_Box, lines 1498-1507), using `localICETracers[Light_Adaptn_TracerID]`. Ice primary producers (ICE_DIATOMS/ICE_MIXOTROPHS) never get `SP_IRR` set here and keep stale values. Function: `Ice_Box()`. | Test `habitatType == ICE_BASED` and ice producer group types | [ ] |
| ECOL-10 | 2329-2360 | unit mismatch | M/M | Dependent-discard reality check: `tot_dis` is the sum of `DependDiscardsTot*dt` (mg N m-3), but for EPIFAUNA `this_bio = initialEpiBiomass[...]` is mg N m-2. Line 2345 itself notes the m-3 -> m-2 conversion is needed. So `if (tot_dis > this_bio)` compares quantities in different units. Function: `Epibenthic_Box()`. | Compare `tot_dis * wcLayerThick` with `this_bio` for EPIFAUNA | [ ] |
| ECOL-11 | 2974-2977 | wrong operator (= vs +=) | H/L | ICE_BASED cover biomass: `biomass = initialIceBiomass[...totNTracers[cohort]];` inside the cohort loop keeps only the last cohort. The EPIFAUNA/LAND cases use `+=`. Function: `Epibenthic_Box()`. | `biomass += ...` | [ ] |
| ECOL-12 | 1340, 3164 (also 1985, 3642) | always-false condition / unreachable case | H/L | `if (flux_model_id > 0) { switch ... case WC: ...}`. WC == 0 (atlantisboxmodel.h:390), and Max_Flux_Check returns `fluxnum = WC` for water-column culprits, so the WC diagnostic branch can never run and "Can't give details" is printed instead. Note: if fixed to `>= 0`, nutrient culprits set `flux_sp = bm->K_num_tot_sp` (atfluxbreakdown.c:117), and `FunctGroupArray[flux_sp].groupCode` would then read out of bounds. Functions: `Water_Column_Box()`, `Epibenthic_Box()`. | Use `>= 0` and guard `flux_sp < bm->K_num_tot_sp` before printing groupCode | [ ] |
| ECOL-13 | 1156-1169 | cohort vs cohort x genotype | M/L | `fished_chrt = (int)speciesParams[Age95pcntV_id]` is an age-class index, but it is used directly to index `dead[]`/`preyEaten[]`, which are sized numCohortsXnumGenes, and is compared with `numCohortsXnumGenes/2`. Elsewhere it is converted (atassess/atclassical.c:860 `nc = va95 * numGeneTypes`). With numGeneTypes>1 this picks the wrong cohort for pseudo-F/M. There is no upper bound check either. Function: `Water_Column_Box()`. | `fished_chrt = Age95pcntV * numGeneTypes`; clamp to `< numCohortsXnumGenes` | [ ] |
| ECOL-14 | 1914-1923 | wrong habitat index / wrong cohort | M/L | Sediment pseudo-M estimate: detritus uses `preyEaten[0][WC]` in the sediment box (should be SED; WC predation is already counted in Water_Column_Box at 1178-1180). The non-detritus loop adds `mortality[fished_chrt]` (=cohort 0) once per cohort instead of `mortality[cohort]`. Function: `Sediment_Box()`. | Use `preyEaten[0][SED]` and `mortality[cohort]` | [ ] |
| ECOL-15 | 2937-2940 | cohort index | M/L | The epibenthic pseudo-M only uses `mortality[0]` and `preyEaten[0][...]`, so multi-cohort epifauna (e.g. age-structured seagrass) lose all cohorts except the first. Function: `Epibenthic_Box()`. | Loop over cohorts | [ ] |
| ECOL-16 | 3985-3988 | buffer extent mismatch | M/L | `for (k = 0; k < numStages; k++) grow[kij][k] = 0.0;`. `grow` is allocated `[numCohortsXnumGenes][RN_id+1]` (atUtilGroupIO.c:748), and the index is SN_id/RN_id, not life stage. If numStages==1, `grow[][RN_id]` is not reset. If numStages>2, the loop writes past the row. Function: `Reset_Arrays()`. | Loop `k <= RN_id` | [ ] |
| ECOL-17 | 2424-2430 | wrong tracer array | M/L | In the Infauna/Epifauna ratio, MB (habitatType SED, see 1499 and comment at 2952) is added as `initialWaterBiomass[...] * smLayerThick`: a water-column concentration multiplied by sediment thickness. It should be the sediment biomass. Function: `Epibenthic_Box()`. | `initialSedBiomass[...] * smLayerThick` | [ ] |
| ECOL-18 | 3689-3691 | copy-paste | H/L | The ice box QNAN guard tests `localSEDFlux[NH3_i]` and calls `Call_Diagnostics(..., SED, ...)`, so NaNs in `localICEFlux` are never caught. Function: `Ice_Box()`. | Test `localICEFlux[NH3_i]`, use ICE_BASED | [ ] |
| ECOL-19 | 1364-1365, 2005-2006 | copy-paste | H/L | In the flux-imbalance path, `Call_Diagnostics(bm, boxLayerInfo, EPIFAUNA, llogfp, 0)` is called from `Water_Column_Box()` and `Sediment_Box()` (should be WC / SED). The guard also uses `(current_box == checkbox) \|\| (dayt >= checkstart)`, whereas the normal call (1249/1955) uses `&&`. | Pass WC / SED; use `&&` | [ ] |
| ECOL-20 | 1077-1098, 2806-2831 | ungated debug output (log size) | H/L | `fprintf(llogfp, "day: %e box%d-%d doing %s with flux entering...")` runs for every physio-chem tracer, layer and adaptive substep whenever `dayt >= checkstart && checkbox == current_box`, with no `bm->debug`/verbose gate. This can produce very large logs, similar to the printf removed in commit f56b3e7. Functions: `Water_Column_Box()`, `Epibenthic_Box()`. | Gate on a debug flag (e.g. `bm->debug == debug_biology_process`) | [ ] |
| ECOL-21 | 3348-3356 | accumulator not reset | H/L | PREYinfo is reset only for cohort 0, but AGE_STRUCTURED_BIOMASS cohorts use `PREYinfo[guild][cohort][ICE_BASED] += ...`. Latent for now: such groups with an ice coefficient >0 quit at 3430, and with a coefficient of 0 the term adds 0. Function: `Ice_Box()`. | Use `=` (as in Sediment_Box line 1519) | [ ] |
| ECOL-22 | 2594-2655, 3723-3725 | wrong operator / habitat index (diagnostics) | M/L | The DebugFluxInfo for EPIFAUNA groups is stored under `[guild][WC]`, and `gain/loss = ...` uses `=` inside the cohort loop, so only the last cohort is kept (the WC/SED branches at 2719-2720 use `+=`). The same `=`-in-cohort-loop pattern appears in `Update_Vertebrate_Tracers()` 3723-3725. This only affects DiagNGain/Loss tracking. Functions: `Epibenthic_Box()`, `Update_Vertebrate_Tracers()`. | Use `+=` after zeroing; index by EPIFAUNA | [ ] |
| ECOL-23 | 1357 | wrong error message | H/L | The `quit("... in the Epibenthic_Box calculations?")` is inside `Water_Column_Box()`. | Say Water_Column_Box | [ ] |


#### atecology/atbiolsetup.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| BSET-01 | 3161-3170 | wrong accumulator | H/H | `dist_step1 += sqrt(xdiff*xdiff + ydiff*ydiff); if (dist_step1 > maxdist) maxdist = dist_step1;`: the code is meant to find the largest distance from the entry box to any box. Because it uses `+=`, it adds up the distances to all boxes instead (and `dist_step1` is never reset). That makes `maxdist`, and with it `InvaderSatDay` and `Invadendt` (the size of `InvadeArray`), much too large. Function: `Setup_Invade()`. | `dist_step1 = sqrt(...)` | [ ] |
| BSET-02 | 3187-3197 | wrong index / accumulator | H/H | In the no_move invasion spread, `dist_step1` keeps growing from the BSET-01 loop and never resets, so the test `dist_step1 < InvaderSpeed*Invadedt*86400` is effectively always false. Even when it is true, it writes `InvadeArray[b][k]` (the source box, which is already > 0) rather than the neighbour `nb`, and it writes time step `k` rather than `k+1`. The invasion therefore never spreads past the entry box until the forced saturation at the last step. Function: `Setup_Invade()`. | Compute distance per neighbour (`dist = sqrt(...)`) and set `InvadeArray[nb][k+1]` (when `k+1 < Invadendt`); also carry earlier values forward | [ ] |
| BSET-03 | 1902-1907 | double normalisation | H/M | `if (bm->manage_reg) ... if (lastreg_prop[sp][nreg] < no_quota) initreg_prop[sp][nreg] = initreg_prop[sp][nreg] / (regtot + small_num);`: `initreg_prop` was already turned into a proportion at line 1900. `lastreg_prop` was set to 0 at line 1541, so the condition is always true at init. With manage_reg on, the proportions are divided by the total biomass a second time and become close to 0. Function: `Ecology_Init_Populations()`. | Remove the second division (or set no-TAC regions to 0 and renormalise across the rest) | [ ] |
| BSET-04 | 892-895 | copy-paste / ordering | H/M | `speciesParams[vstockstruct_id] = sum;` is stored before `if (sum < 1) sum = 1;`, so the minimum of 1 never reaches the stored value. The horizontal block above (881-883) clamps first and then stores. If `wcnz == 0` the value stays `-MAXDOUBLE`, and if all `v_stock_struct` entries are 0 it is 0, which feeds into the stock-structure logic. Function: `Ecology_Init()`. | Move the clamp above the assignment | [ ] |
| BSET-05 | 675-676 | format mismatch | H/L | `quit("... group %s must use prescribed movement only.  %s\n", FunctGroupArray[b].groupCode);` has two `%s` but only one argument. This is undefined behaviour and can crash while reporting the error. Function: `Ecology_Init()`. | Remove the second `%s` | [ ] |
| BSET-06 | 3929, 3941-3951, 3996-3998 | uninitialised index | M/M | `speciesParamIndex` starts at -1 and is never set for `growth_trait_id` (=0), `consumption_trait_id` or `maturation_trait_id` ("intentionally not handled"). If `DNA[sp].trait_active[]` is set for one of these traits, the loop still reads `speciesParams[-1]` (out of bounds) or reuses the previous trait's index. Function: `Create_Evolution_Parameters()`. | `continue` the trait loop for unhandled traits (or skip when `speciesParamIndex < 0`) | [ ] |
| BSET-07 | 3838-3842 vs 2626-2652, 3996 | alloc/use mismatch | M/M | `Ecology_Setup_Evolution()` returns without allocating `DNA` when `K_num_max_genetypes < 2`. `Create_Evolution_Parameters()` (on `flag_do_evolution`) and `Free_Evolution()` (on `flag_do_evolution \|\| flag_mult_grow_curves`) still dereference `DNA[sp]`, so the run crashes if evolution or multiple growth curves are on with one genotype. The reverse case also leaks: when genotypes ≥ 2 but neither flag is set, all `DNA` arrays are allocated and never freed. Functions: `Ecology_Setup_Evolution()`, `Free_Evolution()`. | Use the same condition for allocation and free, or quit on an inconsistent configuration | [ ] |
| BSET-08 | 1876-1884, 1911 | out-of-range struct use | M/L | The loop goes to `sp == K_num_tot_sp` (the DIN slot). For that slot it prints `FunctGroupArray[sp].groupCode` and reads `FunctGroupArray[sp].habitatType`. `FunctGroupArray` is malloc'd with K_num_tot_sp+1 entries (atlantisUtil/atUtilGroupIO.c:252), so this entry exists but was never initialised. The log gets a garbage name, and the read is undefined behaviour. Function: `Ecology_Init_Populations()`. | Use the literal "DIN" for the extra slot and guard line 1911 with `sp < K_num_tot_sp` | [ ] |
| BSET-09 | 1452-1453 | NULL/negative index | M/L | When verbose > 1, the code prints `FunctGroupArray[guild].groupCode` even if the tracer wasn't found (`guild == -1`, the `checkdone < 0` branch). That is an out-of-bounds read at index -1. Function: `Set_Active_Flag()`. | Guard with `guild >= 0` | [ ] |
| BSET-10 | 1743-1748 | double counting | M/L | In the relaxed-density retry, `sp_abund += bm->boxes[ij].tr[k][den]` adds every box's numbers again. Line 1627 already counted them all, unconditionally. When the retry triggers, `initVERTinfo[sp][n][DEN_id]` (and so `totinitpop`) is too high by the in-domain numbers. Function: `Ecology_Init_Populations()`. | Remove the `sp_abund +=` line in the retry loop | [ ] |
| BSET-11 | 3593-3602 | cohort vs cohort×genotype / negative index | M/L | `nage = age_mat - 1; nmat = age_mat;` index `structNTracers`, which is sized and ordered by numCohortsXnumGenes (cohort = age*numGeneTypes+gene; see line 550). With several genotypes, `[nage]` and `[nmat]` pick the wrong age class. If `age_mat_id == 0`, `nage = -1` reads out of bounds. Line 3603 does use numCohortsXnumGenes-1, which is inconsistent. Function: `Set_spMinMax_Values()`. | Use `max(0,nage)*numGeneTypes` and `nmat*numGeneTypes`, clamped against numCohorts-1 | [ ] |
| BSET-12 | 529-535 vs 2899, 3011-3023, 2827, 951 | alloc size | M/M | `bm->maxspage` is the largest `ageClassSize` over vertebrates only. `bm->tempPopRatio` (and `readinpopratio`, `nSTOCK`, `sumSTOCK`) is allocated with `maxspage` in its age dimension, but `Init_Population_Ratios()` fills `tempPopRatio[..][i]` for `i < ageClassSize` for AGE_STRUCTURED_BIOMASS invertebrates too. If an age-structured invertebrate has a larger `ageClassSize` than every vertebrate, or the model has no vertebrates (maxspage = 0), this writes out of bounds. Function: `Ecology_Init()` / `Init_Population_Ratios()`. | Compute maxspage over all AGE_STRUCTURED/AGE_STRUCTURED_BIOMASS groups | [ ] |
| BSET-13 | 564 | off-by-one | M/L | `if (cohort_stage[cohort] > numStages)`: valid stages run 0..numStages-1, so a stage equal to `numStages` passes the check and then indexes past stage-sized arrays such as `pSPEat`. Function: `Ecology_Init()`. | Use `>=` | [ ] |
| BSET-14 | 3363-3365, 3407-3411 | bounds check after write | L/L | `ids[idnum]` is written before checking `n > nbox`. In the first branch (3363) there is no check at all, and in the second the check comes after the write, so `ids[nbox]` can be written before the quit. Function: `Setup_HomeRanges()`. | Check `idnum >= nbox` before writing | [ ] |
| BSET-15 | 2924 vs 2353 | memory leak | H/L | `totden_check` is allocated in `Allocate_Arrays_Post_Load()` but never freed. `Ecology_Free()` frees `totden` but not `totden_check`. | Add `free2d(totden_check);` | [ ] |
| BSET-16 | 3821 vs 2582-2617 | memory leak | H/L | `EMBRYO[sp].migIDmatch` is allocated in `Ecology_Setup_Reproduction()` but `Free_Embryo()` never frees it. | Add `i_free1d(EMBRYO[sp].migIDmatch);` | [ ] |


#### atecology/atbiolmessage.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| BMSG-01 | 966, 987-994 | accumulator not reset | H/M | `coverindx = 0;` is set once before the box loop (966), but inside the loop `coverindx += epi[CoverTracer]` keeps adding and then `ecolindx[coverindx_id] += coverindx / (nbox-nboundary-nland)` runs for every box. So box 1's cover gets counted once per box, box 2's once per remaining box, and so on. The cover indicator is a weighted cumulative sum, not a mean. Function: `Ecology_Calculate_Total_Abundance()`. | Move `coverindx = 0;` inside the `for (ij...)` loop, before the `k` loop (or add straight into `ecolindx`). | [ ] |
| BMSG-02 | 2736-2778 | double counting | H/M | For vertebrates, the first `n` loop adds `MIGRATION[sp].DEN/SN/RN[n][i]` into `juv_mig_sum`/`ad_mig_sum`. Then the `j` loop uses `index = j*num_migrate + i`, which at `j=0` is `i` again, so queue slot `i` is added twice. Migrating biomass and Prop_Migrating in Migration.txt are too high. Function: `Write_Migration_File()`. | Delete the migrating-sum lines from the first loop (keep only `juv_sum`/`ad_sum` there), or start `j` at 1. | [ ] |
| BMSG-03 | 2697-2703 vs 2790-2808 | header/data mismatch | M/M | For invertebrate migrators the header writes 3 columns for each `n` and for each `i < num_migrate`. The writer writes 3 values for each `n` and each `i` in `[num_in_queue_done, num_in_queue)`, which is a different, time-varying count. Migration.txt columns are misaligned and the row width changes from row to row. Function: `Init_Migration_File()` / `Write_Migration_File()`. | Use the same loop bound in both, e.g. aggregate over the queue per `i < num_migrate` as the vertebrate branch does. | [ ] |
| BMSG-04 | 435, 530-574 | wrong scope | M/M | The ice-layer block (`if(bm->ice_on)`) sits inside `if (bm->boxes[ij].botz >= bm->maxwcbotz)`, which is meant to leave out the non-dynamic sediments and epibenthos of oceanic cells. Sea ice over deep oceanic boxes is dynamic, but it is left out of totbiom, reg_prop and targetspbiom. Function: `Calculate_Box_Biomass()`. | Move the ice block outside the `botz >= maxwcbotz` if-block. | [ ] |
| BMSG-05 | 457-458, 504-505, 552-553 | missing update | M/M | `bm->boxBiomass[ij][sp]` is incremented for water-column groups (281, 360) and land (601), but not in the sediment, epibenthic or ice branches. BoxBiomass.txt (`Write_BoxBiomass`) therefore leaves out all infauna, epibenthos and ice biomass and only shows any WC fraction. Function: `Calculate_Box_Biomass()`. | Add `bm->boxBiomass[ij][sp] += biomass;` in the sed, epi and ice branches (non-diagnostic path). | [ ] |
| BMSG-06 | 2735, 2743, 2763 | cohort vs cohort x genotype | M/M | `n` runs over `numCohortsXnumGenes` but is compared directly with `age_mat` (an age-class index). Elsewhere the convention is `basecohort = n / numGeneTypes` (atdemography.c:373, 500). With `numGeneTypes > 1` the juvenile/adult split is wrong. Function: `Write_Migration_File()`. | Compare `(n / FunctGroupArray[sp].numGeneTypes) < age_mat`. | [ ] |
| BMSG-07 | 266-268, 310 | cohort vs cohort x genotype | M/M | `fished_chrt` comes from `Age95pcntV_id` (a cohort index), and its floor is set from `numCohortsXnumGenes/2`. It is then compared with `n`, the cohort x genotype index. With genotypes, `calcNstart[sp][hist_id]` counts the wrong (single genotype) slot. Function: `Calculate_Box_Biomass()`. | Use `(n / numGeneTypes) == fished_chrt`, with the floor based on `numCohorts/2`. | [ ] |
| BMSG-08 | 1077-1095 | wrong condition | M/L | `if ((flagstockout == 1) \|\| call_type)` also holds for diagnostic box calls (`call_type == 1`, from atecology.c:1253, 1959, 3020, 3616). So `Ecology_Output_Mort_Per_Pred_Estimates()` writes extra MortPerPred/SpecificPredMort rows for every debugged box/layer call. Function: `Ecology_Calculate_Total_Abundance()`. | Wrap the Mort_Per_Pred call in `if (!call_type && !bm->flagannual_Mest)`. | [ ] |
| BMSG-09 | 1026-1036 | out-of-bounds read | M/L | The BSS slope is computed only when `smallID != -1 && largeID != -1`, but the debug fprintf right after it uses `FunctGroupArray[bm->smallID].groupCode` and `totfishpop[bm->smallID]` without that guard, so it reads index -1. Function: `Ecology_Calculate_Total_Abundance()`. | Move the debug print inside the `smallID/largeID != -1` block. | [ ] |
| BMSG-10 | 694, 726 | cohort vs cohort x genotype | M/L | The initial-condition checks loop `n < numCohorts`, while the tracer arrays (`structNTracers`, `totNTracers`) run over `numCohortsXnumGenes`. Genotypes beyond the first are never checked for negative or misplaced biomass. Function: `Check_Layer_Initial_Biomass()`. | Loop to `numCohortsXnumGenes`. | [ ] |
| BMSG-11 | 411-414 | wrong variable | L/L | `bm->diagnosticBiom[sp] += ...NO3...` uses the loop variable `sp`, left over after the species loop, while the NH3 block (395) uses `nid` (`DIN_id`). It works only because `sp == K_num_tot_sp == DIN_id` (atparamIO.c:367). Function: `Calculate_Box_Biomass()`. | Use `bm->diagnosticBiom[nid]`. | [ ] |


#### atecology/atfluxbreakdown.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| BMSG-12 | 67-70, 88-91, 100-102, 113-130, 146-148, 159-175, 195-198, 214-217, 228-247, 258-292 | wrong logic | H/L | Each test is `if (fabs(x) > maxflux)` but then stores `maxflux = x` (signed). After a large negative flux wins, `maxflux` is negative, so every later candidate passes the test and the last one checked is reported as the culprit. Function: `Max_Flux_Check()`. | Store `maxflux = fabs(x)` everywhere. | [ ] |
| BMSG-13 | 53, 295-296 | missing case | H/M | The switch handles only WC, SED and EPIFAUNA. atecology.c:3640 calls `Max_Flux_Check(bm, ICE_BASED, ...)` when the ice flux balance fails (`ecotest > 1`), which hits `default: quit("No such callerid ...")`. The ice diagnostic path ends the run instead of reporting the culprit. Function: `Max_Flux_Check()`. | Add an `ICE_BASED` case that scans `localICEFlux` (the caller passes it in all three slots), or stop calling it for ice. | [ ] |
| BMSG-14 | 637-647, 663-684, 697-713, 725-735, 748-758, 771-781, 869-882 | missing case | M/L | `Print_Eat_Diagnostics()` passes `ICE_BASED`/`LAND_BASED` as `habitat` to the Print_* helpers, but every helper only switches on WC/SED/EPIFAUNA, so nothing is printed. It is called that way at atecology.c:3684. Functions: `Print_PreyInfo_Value()`, `Print_PreyEaten_Value()`, etc. | Add ICE_BASED/LAND_BASED cases to the helpers (the arrays are sized `num_active_habitats`). | [ ] |
| BMSG-15 | 805-826 | wrong file / NULL deref | M/L | The header lines go to the global `logfp` but the values go to `llogfp`. Also `fp` stays NULL if `diagType` is not 0-4, and is then called (NULL function pointer). Function: `Print_Eat_Diagnostics()`. | Use `llogfp` for the headers; add `default: quit(...)`. | [ ] |
| BMSG-16 | 322-327 | wrong label | L/L | Any `level_id` other than WC or SED is labelled "epi", so ICE_BASED calls (atecology.c:3613) are logged as epi. Function: `Call_Diagnostics()`. | Add an "ice" label for ICE_BASED (and "land"). | [ ] |
| BMSG-17 | 572-580 | always-true condition | L/L | `if(numCohorts > 0)` is always true, so the else branch never runs. The WC branch (432) uses `> 1`. The output is still right because the loop covers all cohorts. Function: `Print_Flux()`. | Use `> 1` to match line 432. | [ ] |


#### atecology/atLandProcess.c


No issues found. Functions checked: Land_PrimaryProduction, Land_HabitatState, Ecology_Land_Biology_Process (body is all commented out; unused locals `k`, `maxdeep`, `totaldeep` only).


#### atecology/atImposeRecruit.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| BMSG-18 | 76-83 | uninitialised mapping | M/M | When imposed recruits are on, only boxes that have a time series get `BoxKeyMap[b][rects_key_id] = sp`. Every other box keeps the allocation default of 0 (`Util_Alloc_Init_2D_Int(3, nbox, 0)`, atlantismain.c:1911). So boxes with no series get recruits from series 0 instead of 0 recruits, and the `boxkey_id > -1` warning in `Get_Imposed_MultRecruits()` can never fire. Function: `Load_Imposed_Recruits()`. | Set `BoxKeyMap[b][rects_key_id] = -1` for all boxes before the mapping loop. | [ ] |
| BMSG-19 | 62-74 | over-strict check | M/L | The species-id loop quits for any age-structured group missing from the MultRec file, even groups that use neither `multiple_ts_recruit` nor stocking. Users have to add dummy columns for every vertebrate. Function: `Load_Imposed_Recruits()`. | Only require (and map) groups where `flagrecruit == multiple_ts_recruit` or `flagstocking` is set; set others to -1. | [ ] |


#### atecology/atPhysChemIO.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| BMSG-20 | 77-79 vs 138-141 | inconsistent condition | M/M | The count uses `if(bm->track_rugosity_arag)` (truthy), but the add uses `if(bm->track_rugosity_arag == TRUE)`. The flag is read with `integer_check` (atparamIO.c:209), so a value such as 2 reserves 2 slots that are never filled. Their `tracerIndex` pointers are garbage and get dereferenced in `Print_Flux()` and `Check_Layer_Initial_Biomass()` (`*PhysioChemArray[n].tracerIndex`). Function: `Ecology_Assign_Physio_Chem()`. | Use the same test in both places (e.g. `if (bm->track_rugosity_arag)` at line 138). | [ ] |


#### atecology/atprocess.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| PROC-01 | 420, 800, 1096 | wrong index / buffer overrun | H/H | `bm->pSPFCeat[sp_id][preyID][cohort][kij]` (and `[predatorID][preyID][cohort][prey_chrt]` at 420). pSPFCeat is allocated `Util_Alloc_Init_4D_Double(K_num_max_stages, K_num_max_stages, K_num_tot_sp, K_num_tot_sp)` => `[sp][sp][stage][stage]` (atlantismain/atlantismain.c:1914), and it is indexed by stage in atvertprocesses.c:729/738 (`[chrtstage][preyage]`). Here it uses the cohort x genotype indices `cohort`/`kij`, which go past K_num_max_stages (normally 2), so the read is out of bounds and returns the wrong preference. Live whenever catcheater && flag_fisheries_on && !flag_olddiet. Functions: `Get_Catch_Prey()`, `Eat()`. | Index with `[chrtstage][FunctGroupArray[preyID].cohort_stage[kij]]` | [ ] |
| PROC-02 | 795-801, 1094 | accumulator not reset | H/M | `bm->groupTotCatch[preyID][kij] += FCcaughttemp*pFLEET` and then `*= pSPFCeat`. groupTotCatch is only allocated (atbiolsetup.c:2901) and freed. A repo-wide grep finds no reset, so it keeps growing across predators, cells and timesteps, and the `*=` scales the whole running total, not just this predator's share. The `ratio` denominator at 1094 is therefore far too big, so too little catch is taken out of FCcaughttemp/Catch. Function: `Eat()`. | Before the fleet loop, reset with `groupTotCatch[preyID][kij] = 0` (or use a local), then apply pSPFCeat | [ ] |
| PROC-03 | 1074-1076 | wrong variable / buffer overrun | H/H | `thisID = (int)speciesParams[coralID_id]; ... FunctGroupArray[thisID].cohortSpeciesParams[kij][rugosity_dec_id]`. coralID_id holds the CORALREEF index 0..ncoral-1 (atbiolsetup.c:3908), not a guild, so this reads the parameters of an unrelated group (e.g. guild 0). cohortSpeciesParams is also sized `[numStages]` (atUtilGroupIO.c:830), but here it is indexed by `kij` (cohort x genotype), which can overrun it. Live with flag_adv_habitat whenever coral or sponge is eaten. Function: `Eat()`. | Use `FunctGroupArray[preyID].cohortSpeciesParams[FunctGroupArray[preyID].cohort_stage[kij]][rugosity_dec_id]` | [ ] |
| PROC-04 | 1005-1006, 1013-1014, 1162, 1165 | double division | H/M | For the std_holling/ratio/HV/CM cases, Eat sets `scalar = pHscalar / tot_prey; tprey = tot_prey;`. Do_Extra_Feed then computes `EATINGinfo*CLEAR*scalar/(tprey+small_num)`, so aquaculture feed consumption is divided by tot_prey twice (tot_prey_sq twice for holling3). `scalar` also carries the pH correction of whichever prey group was processed last, which has nothing to do with the feed. Functions: `Eat()`, `Do_Extra_Feed()`. | Pass `scalar=1.0` (or the feed-only scalar) and divide once by tprey | [ ] |
| PROC-05 | 646-652 vs 1143-1165; 1244 | wrong index | H/M | Get_Extra_Feed stores feed availability in `EATINGinfo/FEEDinfo[AquacultFeedIndex][cohort][WC]`, but Do_Extra_Feed always reads `[AquacultFeedIndex][0][WC]` and writes spGRAZEinfo `[...][0]`. Invert_Activities (1244) also reads `GRAZEinfo[AquacultFeedIndex][0][WC]`. For any cohort > 0, a cultured or supplemented group eats feed sized from cohort 0's (stale) availability. Functions: `Get_Extra_Feed()`, `Do_Extra_Feed()`. | Use a consistent index (store at [0], or pass the cohort through) | [ ] |
| PROC-06 | 462-466, 483, 504-509 | variable overwritten in loop | H/M | `pHscalar` is set before the habitat loop (1/pHcorr for the predation-availability effect), but for plant prey it is overwritten inside the loop (`pHscalar = pHcorr` or 1.0). From the 2nd habitat on, `prey_eat = prey_avail * pHscalar` uses the nutritional-value scalar instead of the predation scalar, and the 1/pHcorr effect is lost or inverted for SED/EPIFAUNA plant prey. Function: `Calculate_PreyAvail()`. | Use a separate local (e.g. `nutScalar`) for the plant nutritional-value correction | [ ] |
| PROC-07 | 981-990 | division by zero / wrong loop bound | H/M | eat_minmax: `rel_scalar = EATINGinfo/(EATINGinfo + catch_addition)` is 0/0 = NaN for any prey/habitat with zero availability, which makes spGRAZEinfo NaN and triggers the `_finite` quit at 1083. The loop also runs `EPIFAUNA..0` regardless of `max_hab` (WC only for vertebrate prey, where EATINGinfo[SED/EPI] is never set this call), so stale values get used. Function: `Eat()`. | Add `+ small_num` to the denominator and loop `habitat = max_hab..WC` | [ ] |
| PROC-08 | 1585, 1618 | double-applied scalar | H/M | `mortality[0]` already includes `mortality_scalar` (1585), but calcTrackedMort adds `mortality_scalar * mortality[0] * Mdt`, which applies the scalar twice (including the HAB scalar). Invert_Activities line 1269 correctly omits it. Function: `Primary_Production()`. | Drop the outer `mortality_scalar *` at 1618 | [ ] |
| PROC-09 | 1643-1657 | leftover debug | H/L | `if((bm->dayt > 3633) && (groupType == TURF) && (current_box == 24)) fprintf(llogfp, ...)` is live code (the surrounding `/** ... **/` closes at 1642). Any model with a TURF group and a box 24 writes two long lines per call after day 3633, which bloats the log. Function: `Primary_Production()`. | Comment out, or gate on bm->debug/which_check/checkbox | [ ] |
| PROC-10 | 1685-1689, 1711-1716 | logic / copy-paste | H/M | Wrap handling: when `encyst_TofY + encyst_period > 365`, it sets `encyst_TofY = 0; encyst_period = rollover`, which drops the days from encyst_TofY to 365 from the window entirely. The hatch block then reuses the already-modified `encyst_period`, so the hatch window length is wrong whenever the encyst window wrapped. Function: `Encystment()`. | Test `TofY >= start \|\| TofY <= rollover` for wrapping windows and use separate period variables | [ ] |
| PROC-11 | 1401-1407 | truncation / missing state | M/M | `overStartTemp = (int)(speciesParams[overwinterStartTemp_id])` truncates fractional temperature thresholds (2.5 becomes 2). `ans` is a local starting at 0, so when start < temp < end it returns 0: there is no hysteresis, even though the start/end pair implies one. Function: `Check_OverWintering()`. | Remove the `(int)` casts; return the previous overwintering state when between the thresholds | [ ] |
| PROC-12 | 2179-2193 | formula inconsistent | M/M | `BB_DL = BB*Bact_stim*XBB_DL*hO_BB/(Bact_stim*XBB_DL*DLsed*hO_BB + ...)`: the numerator is missing `DLsed` (same for BB_DR/DRsed), and for habitatType==SED it hard-codes `DLsed = DRsed = 1.0`. Compare Calculate_PelagicBact_Scale (2165, `PB*XPB_DL*DL/(...)`) and Sediment_Bacterica_Process (atGroupProcesses.c:1807, numerator includes DL). Callers then divide by DLsed again (Invert_Activities 1238), so sediment bacteria grazing is mis-scaled. Function: `Calculate_SedBact_Scale()`. | Use the actual sediment DL/DR and include them in the numerator, as at atGroupProcesses.c:1807 | [ ] |
| PROC-13 | 1996, 2041-2042 | double counting | M/M | For invertebrates, `biomassToDL` is added to `DetritusProd[WC][DCdet_id]` (1996) and then again to `DetritusProdGlobal[WC][WC][DLdet_id]` (divided by propSediment) at 2042, so the discard waste nitrogen is created twice. Function: `Calculate_Catch()`. | Add biomassToDL to only one pool | [ ] |
| PROC-14 | 2459-2465 | unit mismatch | M/H | McElhany-Bush: `annualSurvivalNoAcid = exp(-pHmortB)` is annual, so `-log(1-annualSurvivalDecrement)` is an annual instantaneous rate. Dividing by 86400 alone (named `mortDueToAcidPerDay`) treats it as a daily rate, which makes acidification mortality about 365x too high. Function: `Acidif_Mort()`. | Divide by `365.0*86400.0` (or document pHmortB as a daily rate) | [ ] |
| PROC-15 | 1535-1554 | no-op logic | M/M | `scale_uptake_NO = max(scale_uptake, NO/uptakeNO); scale_uptake = min(scale_uptake, scale_uptake_NO);`. The max guarantees scale_uptake_NO >= scale_uptake, so the min never changes scale_uptake. The NO, Si and Fe caps therefore never take effect, and uptake of those nutrients can exceed what is available. Only the NH cap is active. Function: `Primary_Production()`. | Use `min(1.0, NO/uptakeNO)` for each nutrient's own scale, then take the overall min | [ ] |
| PROC-16 | 1517-1520 | division by zero | M/M | `uptakeNH = sp_grow*(NH/(KN+NH))*((KN+DIN)/DIN)`, and uptakeNO also divides by DIN. When DIN == 0, sp_grow is 0, so this is 0*inf = NaN. NaN fails both `>0` and `<0`, so it is stored in uptakeNH and NutsLost. Function: `Primary_Production()`. | Guard with `DIN > 0` (else uptake = 0) | [ ] |
| PROC-17 | 560-568 | wrong variable / index (latent) | H/L | Unreachable for now (Eat quits before calling it), but: the habitat coefficients use `FunctGroupArray[sp_id]` rather than `[predID]`; the else branch indexes `sm.tr[bm->current_layer]` with a water-column layer index (can exceed sediment nz); the epi biomass is indexed by `totNTracers` regardless of tracer type; and the loop includes non-predator groups. Function: `Get_Predator_Competition()`. | Fix before enabling the ratio-dependent/HV/CM cases | [ ] |
| PROC-18 | 316-323, 1476 | division by zero | M/L | Primary_Production (and atcoral/atmacrophytes) pass `PBmax_D = 0.0`. If bm->flaglight is irr_ice_v1, `step2 = 0`, so `step1*amt_irr/step2` is a division by zero (and irr_ice_v2 gives `tanh(x/0)`). Function: `Light_Lim()`. | Reject the ice light cases for flaglight, or pass the real PBmax | [ ] |


#### atecology/atGroupProcesses.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| PROC-19 | 943, 1096, 1216 | wrong semantics | H/M | `spawn_now = (int)EMBRYO[guild].Spawn_Now[cohort][next_spawn]` and `if ((int)(Spawn_Now[...] == TRUE))`. Spawn_Now stores spawn day-of-year (atannualbiology.c:248, 386-390), and atvertprocesses.c:1232 and atdemography.c:894 compare it with the day. So spawn_now is non-zero almost always, and `mat_pcnt` is only set when the spawn day equals 1. The feed-while-spawning reduction in Eat is effectively never applied correctly for AGE_STRUCTURED_BIOMASS inverts. Functions: `Invert_Consumers_Process()`, `Coral_Process()`. | Compute spawning-now as in atvertprocesses.c (compare spawn_date and spawn_period with the current day) | [ ] |
| PROC-20 | 832, 838 | copy-paste / wrong habitat | H/M | Algal epibenthic branch: detritus goes to `DetritusProd[WC][DLdet_id]` (832), but `Group_Transfer_Contaminant(bm, boxLayerInfo, EPIFAUNA, SED, LabDetIndex, ...)` moves the contaminant into the SED habitat (the 4th arg is the habitat, atContaminants.c:940). This was copied from the seagrass branch (784), which does use SED detritus. Compare Update_Detritus 382-383 (`EPIFAUNA, WC`). Function: `Phytoplankton_Process()`. | Change to `EPIFAUNA, WC` | [ ] |
| PROC-21 | 1437, 1526, 1562 | double counting | H/M | With track_atomic_ratio, `Gain_Element(... LabDetIndex ..., lysis[0] ...)` is called at 1437 using the Primary_Production lysis. Lysis is then recomputed at 1526, and Gain_Element for DL is called again at 1562. DL elements (P/C) are gained twice per step. Function: `Dinoflag_Process()`. | Remove the call at 1437 | [ ] |
| PROC-22 | 1533-1550 | wrong habitat index | M/M | Dinoflag_Process handles `case WC: case ICE_BASED:` together, but all flux updates are hard-coded to `[WC]` (NutsLost[WC], DetritusLost[WC], DetritusProd[WC], `GRAZEinfo[RefDetIndex][0][WC]`). For ice dinoflagellates the uptake and detritus fluxes go to the water column, and the ice detritus grazing is not removed. Phytoplankton_Process (865-870) correctly uses `habitatType`. Function: `Dinoflag_Process()`. | Use `habitatType` in place of `WC` | [ ] |
| PROC-23 | 604 vs 616/620; 1550 vs 1562/1571 | inconsistent fluxes | M/M | `DetritusProd[..][DLdet_id] += lysis + mortality[0]`, but the element gain (Gain_Element), and the DebugInfo DL, only use `lysis`. With flag_replicated_old_PPmort off, mortality[0] is non-zero, so the P/C ratios of DL drift (N added without the matching elements). Functions: `Phytoplankton_Process()`, `Dinoflag_Process()`. | Include mortality[0] in Gain_Element and DebugInfo | [ ] |
| PROC-24 | 714 | unit mismatch | M/M | `cysts[...][cystenter_id] = (1.0 - (mS_sp + mL_sp + mE_sp)) * cysts[...]`. The mortality rates are per second (used as `rate * biomass` fluxes elsewhere), so this kills a negligible per-second fraction per call, with no dt, independent of timestep. Function: `Phytoplankton_Process()`. | Use `(1.0 - (mS+mL+mE)*bm->dt)` (or exp(-rate*dt)) | [ ] |
| PROC-25 | 299-303, 338 | inverted condition / double count | M/M | In the EPIFAUNA case, for SED_EP groups: `if (diagTol==2 && it_count==1) NutsProd[SED] += releaseNH; else NutsProdGlobal[EPIFAUNA][SED] += releaseNH;`. Every other case updates the local flux always and the global flux only when diagTol==2 && it_count==1. Here the non-global groups never get a local NH flux, and in the global case NutsProdGlobal[EPIFAUNA][SED] is incremented again at 338. Function: `Update_Detritus()`. | Always update NutsProd[SED] and leave the global flux to the block at 325-344 | [ ] |
| PROC-26 | 2157-2166 | inconsistent reset | M/M | After temporarily overwriting phytoplankton PREYinfo with sediment availability, the reset sets `PREYinfo[sp][chrt][WC] = localWCTracers[...]` without the `habitatCoeffs[WC]` (and ice MIXED) scaling used by Construct_Prey_Info (atprocess.c:2404, 2423), and leaves `PREYinfo[...][SED] = 0`. Predators processed later in the same cell see different phytoplankton availability. Function: `Sediment_Epi_Other_Process()`. | Save and restore the original values (or reapply the habitatCoeffs) | [ ] |
| PROC-27 | 1679-1680, 1811-1812 | division by zero | M/M | With flagkdrop: `pow(PB_DL/(XPB_DL*DL), 3.0)` and `pow(BB_DL/(XBB_DL*DL), 3.0)` have no small_num. When DL (or DR) is 0, the numerator is also 0, giving 0/0 = NaN growth (`max(0.0, NaN)` is unreliable). Functions: `Pelagic_Bacteria_Process()`, `Sediment_Bacterica_Process()`. | Add `+ small_num` to the denominators | [ ] |
| PROC-28 | 317-322, 346-352 | ungated debug | M/L | `if((bm->dayt > bm->checkstart) && (bm->checkbox == bm->current_box)) fprintf(bm->logFile, ...)` is not gated on bm->debug or which_check. It prints two lines for every epibenthic group, every iteration, in the check box once past checkstart, which can flood the log. Function: `Update_Detritus()`. | Gate on `bm->debug == debug_biology_process` / which_check, or comment out | [ ] |


#### atecology/atContaminants.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| CONT-01 | 307, 480, 592, 707, 768, 1008, 1174, 1498, 1541, 1655, 1780, 1794, 1864, 2000, 2023, 2043, 2212 (and atUtilGroupIO.c:910-914) | buffer overrun (cohort vs cohort×genotype) | H/H | `FunctGroupArray[i].contaminantTracers = Util_Alloc_Init_2D_Int(bm->num_contaminants, numCohorts, 0)` (atUtilGroupIO.c:910) gives `[numCohorts][ncont]`; same for `contamPropTracers` (911), `contaminantSpMort` (912, 1D `numCohorts`) and `agingContam` (914, first dim `numCohorts`). atBuildTracer.c:378 also only builds tracers for `cohort < numCohorts`. But every loop in this file runs `cohort < numCohortsXnumGenes` and indexes `contaminantTracers[cohort][cIndex]`, `contamPropTracers[cohort][...]`, `contaminantSpMort[cohort]`, and `agingContam[nextcid]` (nextcid = cohort+numGeneTypes, atdemography.c:2630). With numGeneTypes>1 this reads/writes past the allocations (and uses garbage tracer ids). Functions: `Calculate_Containinant_Flux_Habitat()`, `Calculate_Contaminants_Flux()`, `Species_Contaminant_Uptake()`, `Calculate_Species_Contaminant_Decay()`, `Calculate_Species_Contaminant_Effects()`, `Age_Contaminants_Store()`, `Check_Contam_Totals()`, etc. | Allocate these with `numCohortsXnumGenes` and build tracers per cohort×genotype (or map cohort×gene to age class consistently) | [ ] |
| CONT-02 | 313, 345, 348, 380, 383 | wrong variable / unit mismatch | H/H | `diffden[ij][k] = this_currentden[sp][cohort][k][ij] - bm->boxes[ij].tr[k][den];` — `currentden` passed from atmovement.c:2927 holds *proportions* of the total (`currentden = tr[k][den] / totden`, atmovement.c:1542) taken before movement, while `tr[k][den]` at the call point already holds the post-move *numbers* (atmovement.c:2797). So diffden subtracts numbers from a proportion (almost always negative, and sign is reversed vs the intended "new − old"), and lines 345/348/380/383 multiply proportion by `this_currentden` as if it were a count. The contaminated-proportion reallocation is therefore meaningless. Function: `Move_Vert_Contaminated()`. | Pass/compute old numbers (`currentden*totden`) and use `diffden = new_num - old_num`; use numbers consistently in the averaging | [ ] |
| CONT-03 | 318, 345, 380 | wrong tracer | M/H | `localcontam[...] = bm->boxes[ij].tr[k][cid]` with `cid = contaminant_tracer` (the *environmental* contaminant concentration), and lines 345/380 overwrite `tr[clayer][cid]` / `tr[next_box][clayer][cid]` with a fish-number-weighted average. Moving animals thus rewrites the water-column contaminant concentration (non-conservative). Intended quantity is the group's own tissue tracer `FunctGroupArray[sp].contaminantTracers[cohort][cIndex]`. Function: `Move_Vert_Contaminated()`. | Use `contaminantTracers[cohort][cIndex]` for `cid` | [ ] |
| CONT-04 | 349, 384 | wrong denominator | M/M | `tr[clayer][pid] = new_num_in_box / (bm->boxes[ij].tr[clayer][den] + small_num)` — numerator built from `this_currentden` (lines 348/383) but denominator is `tr[...][den]`; inconsistent sources, proportion is then clamped to 1 hiding the error. Function: `Move_Vert_Contaminated()`. | Use the same (post-move) numbers in numerator and denominator | [ ] |
| CONT-05 | 427-431, 435 | wrong tracer array | H/H | For `habitat == EPIFAUNA` (called from atecology.c:2553) `tracerArray = getTracerArray(boxLayerInfo, EPIFAUNA)` = `localEPITracers`, then `cLevel = tracerArray[contaminant_tracer]` indexes the epibenthic array with a water-column tracer id (wrong value, possible OOB). `Degrade_Contaminants()` then sets `amount_decayed` from this, which is subtracted in the EPIFAUNA branch of `Calculate_Contaminants_Flux()` (lines 570-571). Compare `Species_Contaminant_Uptake()` line 749 and `Calculate_Species_Contaminant_Effects()` line 1617 which use the WC array for the environmental level under EPIFAUNA. Function: `Change_Contaminant_Levels()`. | For EPIFAUNA read `cLevel` from `getTracerArray(boxLayerInfo, WC)` | [ ] |
| CONT-06 | 748-752, 770 | wrong tracer array | M/H | For EPIFAUNA habitat `tracerArray` is set to WC, but `cGroupLevel = tracerArray[FunctGroupArray[sp].contaminantTracers[cohort][cIndex]]` — for epibenthic groups those tracer ids index the epi array (`epi[pid]` is correctly used at line 878). So group level is read from the wrong array, corrupting uptake (linear/sigmoid uses `cLevel - cGroupLevel`). `Gain_Contaminants()` (678-682) handles this correctly. Function: `Species_Contaminant_Uptake()`. | Read group levels from `getTracerArray(boxLayerInfo, EPIFAUNA)` when the group is epifaunal | [ ] |
| CONT-07 | 966, 974, 1049-1050 | wrong variable / unit mismatch | M/M | `amountTransfer` is overwritten with a proportion (`amountTransfer / totalBiomass`) at 966/974, but at 1049 `amt_exchanged = amountTransfer * dtsz; prop_exchanged = amt_exchanged / toGuild_totalBiomass` treats it as a biomass amount — proportion divided by receiver biomass. Function: `Group_Transfer_Contaminant()`. | Keep the original amount in a separate variable and use it for `amt_exchanged` | [ ] |
| CONT-08 | 957-961, 997, 1058 | wrong tracer array | M/M | `tracerArray` chosen by `fromGuild` habitat (EPIFAUNA → epi array) but used for the *toGuild* `totNTracers[toCohort]` (997) and `NumsTracers[toCohort]` (1058). When prey is epifaunal and predator is not (or vice versa) the predator's indices are read from the wrong array. Function: `Group_Transfer_Contaminant()`. | Select a separate tracer array based on `FunctGroupArray[toGuild].habitatType` | [ ] |
| CONT-09 | 1436, 1449-1450 | wrong operator (copy) | M/M | `if (step2 < C_reprod_corr[cohort]) C_reprod_corr[cohort] *= step2;` — comment says "taking the minimum value only" but it multiplies, compounding the effect across contaminants. Function: `Get_ContamReproductionEffects()`. | `C_reprod_corr[cohort] = step2;` | [ ] |
| CONT-10 | 1433-1437 | cohort vs cohort×genotype | M/M | `if (cohort < age_mat) return;` — `cohort` is the cohort×genotype index (caller loops `numCohortsXnumGenes`, line 1642) while `age_mat` is in age classes. Function: `Get_ContamReproductionEffects()`. | Compare `cohort / numGeneTypes` with age_mat | [ ] |
| CONT-11 | 1558 | printf format mismatch | H/L | `quit("No such contaminant interaction option (%d), ...")` has a `%d` but no argument — undefined behaviour. Function: `Get_Contam_Amplification()`. | Pass `bm->flag_contamInteractModel` | [ ] |
| CONT-12 | 1795-1797 | wrong index (cohort×genotype) | H/M | `cid_prev = FunctGroupArray[sp].contaminantTracers[cohort-1][cIndex];` — the source cohort for aging is `cohort - numGeneTypes` (nextcid = cohort + sp_numGeneTypes, atdemography.c:2630); with genotypes, `cohort-1` is a different genotype of the same/younger age. Function: `Age_Contaminants_Update()`. | `cohort - FunctGroupArray[sp].numGeneTypes` | [ ] |
| CONT-13 | 1834-1847 | unit mismatch | M/H | `time_step = bm->dayt - Leave_Now[mid]` is in days (Leave_Now set in days, atmigration.c:386), but `half_life` is in seconds (Degrade_Contaminants line 450 uses `dtsz / half_life` with dtsz in seconds). Decay during migration is underestimated by a factor 86400. Function: `ContaminantMigrationIn()`. | `time_step = (bm->dayt - Leave_Now[mid]) * 86400.0` | [ ] |
| CONT-14 | 1873-1880 | uninitialised logic / incomplete | M/M | `newContam = 0.0` (assignment commented out `//newContam = ;`) so every batch of leavers dilutes the migrant pool's contaminant level towards zero: `(origContam*oldden + 0*num_leaving)/(...)`. Leavers' tissue burden is lost. Function: `ContaminantMigrationOut()`. | Set `newContam` from the leavers' tissue concentration | [ ] |
| CONT-15 | 1909-1910, 1924 | unit mismatch / division by zero | M/M | `reprodContam += cGroupLevel * maternal_transfer_rate` (unweighted concentration sum over cells) while `reprodContamCount += this_den` (numbers); `Store_Recruit_Contaminants()` then divides the two — not a density-weighted mean, and divides by zero when no spawners contributed (NaN). Functions: `Get_Parental_Contaminants()`, `Store_Recruit_Contaminants()`. | Accumulate `cGroupLevel*rate*this_den`; guard count>0 | [ ] |
| CONT-16 | 1993-1995 | cohort vs cohort×genotype | L/M | `drandom(age_mat, numCohortsXnumGenes)` mixes age-class units (age_mat) with cohort×genotype count and ignores `ngene`, so the chosen "mother" cohort may be the wrong genotype/age (also overruns contaminantTracers per CONT-01). Function: `Get_Settler_Contaminants()`. | Pick an adult age class and convert to `age*numGeneTypes + ngene` | [ ] |
| CONT-17 | 2177, 2250, 2254 | wrong index | M/M | `bm->forceTracerInput[cIndex].total_input` indexes the forcing-tracer array by contaminant index, but `Init_Contaminants()` (208-212) shows contaminants are matched to forcing entries by name, not position. If any non-contaminant forcing tracer precedes them, the wrong total is checked (spurious quit or missed check). Function: `Check_Contam_Totals()`. | Store the matched forcing index per contaminant in Init_Contaminants and use it | [ ] |
| CONT-18 | 840, 1266 | wrong index (layer collision) | L/L | `expose_time[sp][cohort][current_box][current_layer]` is sized `wcnz+sednz` layers but in SED habitat `current_layer` is a sediment layer index (0..), colliding with WC layer slots. Same for `sp_point`/`sp_maxDoseToDate`. Functions: `Species_Contaminant_Uptake()`, `Get_ContamMortEffects()`. | Offset sediment layers by `wcnz` | [ ] |
| CONT-19 | 1397 | wrong operator | L/M | `if ((isPrimaryProducer == TRUE) && (isMicroFauna == TRUE))` — a group is unlikely to be both, so the Laender EC50 branch is effectively dead; `||` probably intended. Function: `Get_ContamGrowthEffects()`. | Use `\|\|` if intended for either class | [ ] |


#### atecology/atvertprocesses.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| CONT-20 | 726, 734 | cohort vs cohort×genotype | H/M | `preychrt = bpreychrt / numGeneTypes; ... bm->FCcaughttemp[prey][fleet][preychrt]` — FCcaughttemp is indexed by cohort×genotype everywhere else (atprocess.c:797, 1094, 2005; atecology.c:1122; reset loop atecology.c:3990). With numGeneTypes>1 the wrong cohort's catch is used. Function: `Avail_Catch()`. | Index with `bpreychrt` | [ ] |
| CONT-21 | 1617 | printf format mismatch | H/L | Format string contains `"... or %d (incfemental fixed % drops)"`; `% d` is parsed as a conversion (space flag + `d`) with no matching argument — UB. Function: `Check_for_Starvation()`. | Escape as `%%` | [ ] |
| CONT-22 | 1594-1597, 1610-1613 | wrong formula | M/M | Non-dynamic branch uses `opt_SN = RN / (1.0 + X_RS)` and `opt_RN = (1.0 + X_RS) * newSN`, but optimal reserve is `RN = X_RS*SN` elsewhere in non-dynamic mode (`Partition_Weight()` 766/770 `RN/(X_RS*SN)`; total weight `(1+X_RS)*SN` in `Vert_Mortality()` 896). XRS_shrink shrinks SN too far; incremental_shrink yields negative `diff_RN` so RN *grows* on starvation. Function: `Check_for_Starvation()`. | Use `RN / X_RS` and `X_RS * newSN` | [ ] |
| CONT-23 | 437-439, 383 | accumulator not reset | L/M | `like_coral` is set inside the `stage` loop but never reset per stage, so a coral-liking juvenile stage forces the rugosity model for the adult stage too. Function: `Vertebrate_Assess_Enviro()`. | Reset `like_coral = 0` at top of the stage loop | [ ] |
| CONT-24 | 830 | division by zero | M/M | `*FRC_sp = toS / avail_intake;` with no guard; `Vertebrate_Activities()` guards `avail_intake > 0` (1393) but `External_Box_Ecology()` (atexternalpop.c:164) calls it unconditionally, so avail_intake ≤ 0 gives inf/NaN or negative FRC (no lower clamp). Function: `Partition_Weight_Dynamic()`. | Guard `avail_intake <= 0` (fallback to Partition_Weight) and clamp at 0 | [ ] |


#### atecology/atexternalpop.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| CONT-25 | 91, 104, 108, 192 | cohort vs cohort×genotype | M/M | `age = ageclasssize * cohort ...` and `if (cohort < age_mat)` use the cohort×genotype loop index (line 71 loops `numCohortsXnumGenes`) as if it were the age class; with genotypes ages/maturity are wrong. Function: `External_Box_Ecology()`. | Use `cohort / numGeneTypes` for age | [ ] |
| CONT-26 | 147-153 | unit mismatch | M/M | `Fish_Respiration()` returns a per-second rate (`/86400.0`, atvertprocesses.c:1532) while `Growth_increment` is a per-step (per-dt or per-day) mass increment; `avail_intake = Growth_increment - Resp` mixes units, feeding `Partition_Weight_Dynamic()`. Function: `External_Box_Ecology()`. | Multiply Resp by `bm->dt` (and check Growth units) | [ ] |
| CONT-27 | 97-100, 180-181 | unit mismatch | M/M | Marie's option computes a *daily* increment (`lengthgrowth = Kbert*(linf-Length)/365`) and adds it to SN/RN once per call with no dt scaling (comment says dt not needed as increment already per step, which is only true for the Isaac option). If dt ≠ 1 day growth is mis-scaled. Also `li_a*pow(lengthgrowth, li_b)` converts a length *increment* through the length-weight relation, which is not a weight increment. Function: `External_Box_Ecology()`. | Scale by `dt/86400` and use W(L+dL)-W(L) | [ ] |
| CONT-28 | 199-201 | missing floor | L/M | `MIGRATION[species].DEN[cohort][mid] -= (mort * dt);` with no lower bound; large mL/mP or dt can drive density negative. Function: `External_Box_Ecology()`. | Clamp to ≥ 0 | [ ] |


#### atecology/atbiology.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| BIOL-01 | 479-507 | wrong index / buffer overrun | H/H | `newsedtr[bm->current_box][ij - numwclayer][k] = ...` — the loop variable `ij` runs over sediment layer indices (`pBox->sm.topk+1 .. sm.nz-1`) and `Copy_SED_Tracers()` reads `sm.tr[bm->current_layer = ij]`. But the write-back subtracts the *water-column* layer count, so it gives a negative or wrong layer. `newsedtr` is `alloc3d(ntracer, sednz, nbox)` => `[nbox][sednz][ntracer]` (atlantismain.c:650) and is indexed in sediment-layer space everywhere else (atboundary.c:66, atbiophysics.c:940). Any model with >1 sediment layer writes out of bounds, or loses the sediment-layer results. Function: `Box_Bio_Process()`. | `newsedtr[bm->current_box][ij][k] = ...` | [ ] |
| BIOL-02 | 1474-1481 | buffer overrun | H/H | `for (cohort = 0; cohort < numCohortsXnumGenes * numStocks; cohort++) bm->totDiet[b][k][sp][cohort][prey] = 0.0;` — `totDiet` is `alloc5d(K_num_tot_sp, K_num_max_cohort, K_num_tot_sp, wcnz+sednz, nbox)` => cohort dim = `K_num_max_cohort` (atlantismain.c:1176), and K_num_max_cohort is the max of `numCohorts` (atUtilGroupIO.c:302). Any group with >1 stock or >1 genotype writes past the cohort dimension. That corrupts other entries, and past the end of the block for the last box. Runs only when `flagdietcheck` is on. Function: `ReInitDietData()`. | Loop `cohort < FunctGroupArray[sp].numCohortsXnumGenes` and allocate the cohort dim as `K_num_max_cohort*K_num_max_genetypes` (also needed for atprocess.c:2356) | [ ] |
| BIOL-03 | 774-786 vs 1350 | wrong offset | M/M | Ice stability check calls `Get_Max_Flux_Change(..., localICETracers, ..., bm->ntracer /*offset*/, ...)`, so `Tolflag`/`Fluxflag` are looked up in the *sediment* flag slots. Integration (`Integrate_Tracer_Variables(..., ICE_BASED, ..., 0 /*offset*/)`, l.1350) and `Copy_ICE_Tracers()` (`Fluxflag[i]`) both use offset 0. The adaptive time step for ice is therefore based on different flags than the ones used to integrate (the l.778 comment already questions this). Function: `Get_Time_Change()`. | Pass offset 0 for the ice case, the same as integration | [ ] |
| BIOL-04 | 398-404, 469-642 | logic / scoping | L/M | The ice-layer processing (`if(bm->ice_on) { FlagModel = 4; ... }`) sits inside `if (!oceanic_only)`. In boxes deeper than `bm->max_depth` (oceanic_only), ice biology is never run and `newicetr` is never updated, although sea ice can occur over deep boxes. Function: `Box_Bio_Process()`. | Move the ice block outside the `!oceanic_only` block (if ice should be processed everywhere) | [ ] |


#### atecology/additionalTracer.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| BIOL-05 | 827 | off-by-one / wrong bound | H/M | `for (tracerIndex = 0; tracerIndex <= p_id; tracerIndex++)` — `p_id = 0`, `c_id = 1` (atlantisboxmodel.h:463-468), so the loop runs only for P. The `case c_id:` branch can never run, carbon released by breakdown/excretion is never added to `NutsProd[..][C_id]`, and `Calculate_Carbon_Decomposition()` (l.299) always sees only other contributions. Function: `Calculate_Element_Release()`. | `tracerIndex < num_atomic_id` | [ ] |
| BIOL-06 | 363 | off-by-one / always-false | H/M | `if (bm->current_layer == bm->boxes[bm->current_box].nz)` — layers are 0..nz-1, so this is never true. The air-sea CO2 exchange `*trans` is never computed and `C_trans` stays 0 for the whole run. When it is called for SED/EPIFAUNA, `current_layer` is not a WC index anyway. Function: `Calculate_Carbon_Transformation()`. | `if (habitatType == WC && bm->current_layer == nz - 1)`, else `*trans = 0` | [ ] |
| BIOL-07 | 325 | unit mismatch | H/M | `RTOP = bm->atomicRatioInfo->RTOP / (24.0 * 86400.0); /* divide to get rate per second */` — RTOP is documented as days^-1 (atBioltoXML.c:2377, atNutrient.c:1083), and day^-1 to s^-1 is /86400. The extra /24 makes P transformation 24x too slow. Function: `Calculate_Phosphorus_Transformation()`. | `/ 86400.0` | [ ] |
| BIOL-08 | 1311-1346 | unit mismatch / integration bypass | H/H | `tracerArray[DIP_i] += net_desorp/porosity; tracerArray[PIP_i] -= ...` — `net_desorp = Pads_r * (...)` is a rate (EMS source writes it to derivatives), but here it is added straight to the local *state* with no `dt`. It is also applied on every adaptive sub-step through `Calculate_Phosphorus()`, so the result depends on the number of sub-steps and the units are wrong. Runs when `flag_adv_P >= 1`. Function: `p_adsorption_calc()`. | Add the terms to the flux array (`localWCFlux`/`localSEDFlux`) instead of the tracers, or multiply by `dtsz` | [ ] |
| BIOL-09 | 1296-1309 | dead code / missing init | H/M | `p_adsorption_setup()` is never called anywhere in the repo (grep finds only its definition, and it has no prototype). So `Pads_r_i`, `r_immob_PIP_i` and `Tfactor_i` are never set from `bm->Pads_r_t0`/`r_immob_PIP_t0`, and `p_adsorption_calc()` uses whatever value the tracer has in the initial file (usually 0, which turns adsorption off). Function: `p_adsorption_setup()`. | Call it before `p_adsorption_calc()` in `Calculate_Phosphorus()` | [ ] |
| BIOL-10 | 1308, 1346 | accumulator | M/M | `tracerArray[TIP_i] += PIP + PIPI + PIP_Dust;` — total inorganic P should be the sum (`=`), but it is accumulated on every call/sub-step, so TIP grows without bound. The TIP tracer is registered with flux flag 0 (atPhysChemIO.c:132), so it is not reset each step. Functions: `p_adsorption_setup()`, `p_adsorption_calc()`. | `tracerArray[TIP_i] = ...` | [ ] |
| BIOL-11 | 457 | debug output left live | H/M | An unconditional `fprintf(bm->logFile, "%s-%d Box %d-%d Density: ...")` runs for every age-structured cohort, every atomic tracer, every layer and every adaptive sub-step whenever `track_atomic_ratio` is on. It can create very large log files, the same problem as the printf just removed from atContaminants. Function: `Calculate_Element_Flux()`. | Delete it or guard with a debug flag | [ ] |
| BIOL-12 | 655 | debug output left live | H/M | An unconditional `fprintf(bm->logFile, "Group %s Box %d layer %d newN: ...")` runs for every primary-producer cohort/element on every sub-step. Function: `Integrate_Ratio_Variables()`. | Delete it or guard with a debug flag | [ ] |
| BIOL-13 | 1111-1118 | debug output left live | H/M | `/**/ if(amount > 0 && toGuild == 0){ fprintf(bm->logFile, ...) } /**/` — the empty `/**/` markers leave this debug print active. It logs every predation transfer into group index 0. Function: `Gain_Element_Predation()`. | Comment it out properly or delete it | [ ] |
| BIOL-14 | 755-759 | uninitialised variable / stdout spam | M/L | Inside the `flagratio_warn` block, the `/**/ ... /**/` markers leave a `printf` to stdout plus an `fprintf` live on every ratio update. The fprintf prints `newN`, which is only set in the primary-producer branch (l.643), so it can be uninitialised for non-PP groups. Function: `Integrate_Ratio_Variables()`. | Remove the prints or initialise `newN = 0` | [ ] |
| BIOL-15 | 450, 624 | cohort vs cohort×genotype | M/M | Both ratio loops use `cohort < FunctGroupArray[sp].numCohorts`. `addRatioTracers`/`ratioLost`/`ratioGainedPred` are sized `numCohortsXnumGenes` (atUtilGroupIO.c:891-896) and cohorts are gene-interleaved (`numGeneTypes*bcohort+ngene`). With >1 genotype, half of the ratio tracers are never integrated, and the cohorts that are covered are the wrong ones. `ratioGained`/`ratioLostPred` are sized only `numCohorts` (l.901,904), so the sizes are inconsistent too. Functions: `Calculate_Element_Flux()`, `Integrate_Ratio_Variables()`. | Loop over `numCohortsXnumGenes` and size `ratioGained`/`ratioLostPred` to match | [ ] |


#### atecology/atannualbiology.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| BIOL-16 | 603, 619 | off-by-one | H/M | `layerk = k + diffdeep - 1;` / `layerk = j + diffdeep - 1;` — this is the same vertical rescaling as `Get_Vertical_Recruit_Position()` (atdemography.c:2025-2042), which uses `k + diffdeep` (also atmovement.c:4221, atManageSetup.c:1156). With `-1`, shallow boxes map onto layers shifted down by one and never use the top layer `nlayers-1`, so the initial pre-model recruits get the wrong vertical distribution. Function: `Init_Spawning()`. | Use `k + diffdeep` / `j + diffdeep`, or simply call `Get_Vertical_Recruit_Position(sp, j, nlayers, bm->boxes[i].nz)` | [ ] |
| BIOL-17 | 1157-1159 | wrong box index | M/M | `boxkey_id = bm->BoxKeyMap[bm->current_box][rects_key_id];` — `Get_Init_Embryos()` is called from inside the box loop `i` of `Init_Spawning()`, but it has no box argument, so it uses the stale `bm->current_box` (whatever was last processed) for every box. Function: `Get_Init_Embryos()`. | Pass box `i` into `Get_Init_Embryos()` and use it here | [ ] |
| BIOL-18 | 1098-1181 vs 1238 | missing switch case | M/M | The SR switch handles `case ice_wimd_based`, but the first (larvae) switch has no such case, so the run reaches `default: quit("No such flagrecruit defined...")` whenever `flag_old_embryo_init` is off and a larval-queue extension exists. The same omission appears in atdemography.c's first switch. Function: `Get_Init_Embryos()`. | Add `case ice_wimd_based:` alongside the BevHolt/Ricker cases (Larvae += TotSpawn) | [ ] |
| BIOL-19 | 471, 593-669 | accumulator / loop nesting | M/M | The larval-queue-extension block (recruit setup for k < larval_queue_extension) sits inside the `for nspawn < numSpawns` loop. With `flag_old_embryo_init`, `EMBRYO[sp].Larvae[stock_id][ngene][k] += num_recruits` (l.649) is accumulated again for each spawn event, so it counts recruits numSpawns times. The `num_recruits[..][k]` entries are also overwritten per nspawn, whatever spawn they belong to. Function: `Init_Spawning()`. | Index by the spawn's queue slot, or move the block outside the nspawn loop and reset Larvae first | [ ] |
| BIOL-20 | 1106-1108 | debug output left live | H/L | `if(species == 9) { fprintf(bm->logFile,"Init for %s qid: ..."); }` — a hard-coded species index debug print in live code. Function: `Get_Init_Embryos()`. | Delete it | [ ] |
| BIOL-21 | 924-934 | debug output left live | M/L | The `do_debug` guard is commented out, so `PrintDates()` writes a line for every species × queue slot × cohort×genotype (called from l.718). The queue length is numSpawns*(years+…), so this can be thousands of lines per species at init. Function: `PrintDates()`. | Restore the `if (do_debug && ...)` guard | [ ] |
| BIOL-22 | 493-494 vs 518 | inconsistent constant | L/L | Countback uses `temp_val -= 364` (and l.526-527 for recruits), but insertion subtracts `365 * i` (l.518, 560-561). For dates of about 729-730 days, countback gives 2 years while the insertion assumes 365-day years. Function: `Init_Spawning()`. | Use 365 consistently | [ ] |
| BIOL-23 | 752-755 | unit mismatch in check | L/L | `next_spawn_any_age > ((365.0 * bm->tstop) / 86400.0)` — `tstop/86400` is already days, and multiplying by 365 makes the upper bound meaningless (365x too lax). Function: `Init_Spawning()`. | Compare against `bm->tstop / 86400.0` | [ ] |
| BIOL-24 | 703-710 | wrong formula (latent) | L/L | `num_steps = ceil(364.0 / AgeClassSize_sp)` should be `ceil(1.0 / AgeClassSize_sp)` (steps per year). As written, `stid` reaches far beyond the `Age_Now` queue (`[cohort][max_num]`, atbiolsetup.c:3804). This is currently unreachable because `ageClassSize` is an `int` (atFunctGroup.h:90), so it is always ≥1. Function: `Init_Spawning()`. | `num_steps = (int)ceil(1.0 / AgeClassSize_sp)` and bound by `num_in_aging_queue` | [ ] |


#### atecology/atmacrophytes.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| BIOL-25 | 88 | format specifier mismatch | H/L | `quit("This is not coded for %s group types ...", FunctGroupArray[sp_id].groupType, ...)` — `groupType` is a `GROUP_TYPES` enum (int) (atFunctGroup.h:152), but it is passed to `%s`. This is undefined behaviour and probably a segfault instead of the intended error message (ersem_ben_lim path). Function: `Grow_Macrophytes()`. | Use `%d`, or print the group-type name string | [ ] |
| BIOL-26 | 190-192 vs 118 | wrong parameter | L/M | Epiphyte nutrient limitation uses `KNepi_id` (l.118), but epiphyte NH/NO uptake partitioning uses `KN_id` (the leaf half-saturation). This is probably a copy from the leaf/sediment branch. Function: `Grow_Macrophytes()`. | Use `KNepi_id` in the epiphyte uptake terms (if intended) | [ ] |


#### atecology/atNutrient.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| NUTR-01 | 498-519, 708-725, 808-817, 906-927, 972-981, 1033-1050, 1161-1170 | wrong condition / double counting | H/M | The atmospheric-input blocks are gated only by `bm->current_layer == nz-1`, not by `habitatType == WC`. Each ROC function is called for WC, SED, EPIFAUNA and ICE_BASED (atecology.c:1082, 1847, 2811, 3548). In a box with one water-column layer (nz==1) the WC loop in atbiology.c:414 skips layer 0 (`ij > stopij`, stopij=0), and Epibenthic_Box (atecology.c:2132-2135 then its own ROC loop) runs WC, SED and EPIFAUNA all with `current_layer==0==nz-1`. `localWCFlux` is not reset between those calls (Reset_Arrays atecology.c:3970 does not touch it) and the EPIFAUNA cases use `+=`, so atmospheric NH, NO, Fe, O2, Si, CO2 and P inputs get added 3 times (2 times for Oxygen/Si/DON-like cases that `=` in WC). The SED call also adds to localWCFlux when a sediment layer index equals nz-1. Functions: `Ammonium_ROC()`, `Nitrate_ROC()`, `Micronutrient_ROC()`, `Oxygen_ROC()`, `Dissolved_Silica_ROC()`, `Carbon_ROC()`, `Phosphorus_ROC()`. | Wrap each atmosphere block in `if (habitatType == WC && ...)`. | [ ] |
| NUTR-02 | 895-897 | copy-paste | H/M | `localICEFlux[Oxygen_i] = -bm->X_ON * (FunctGroupArray[LabDetIndex].solDON + ... + FunctGroupArray[IceBactIndex].prodnDON)` is the DON_ROC ICE expression (line 850), not the NH3+NO3 flux used in the WC/SED/EPIFAUNA cases (lines 874, 881, 887). Ice oxygen ends up driven by DON production. No debug info set either. Function: `Oxygen_ROC()`. | `= -bm->X_ON * (boxLayerInfo->localICEFlux[NH3_i] + boxLayerInfo->localICEFlux[NO3_i]);` | [ ] |
| NUTR-03 | 1017-1020 | missing term | M/M | The EPIFAUNA case adds `NutsProd[WC][C_id]` and `NutsProd[SED][C_id]` but never subtracts `NutsLost[..][C_id]`. Compare Phosphorus_ROC EPIFAUNA (1145-1146), which uses Prod − Lost. Epibenthic carbon uptake gets dropped. Function: `Carbon_ROC()`. | Add `- (double)boxLayerInfo->NutsLost[WC][C_id]` (and the SED one). | [ ] |
| NUTR-04 | 1038-1044 | wrong tracer | M/M | Atmospheric CO2 is added to `*PhysioChemArray[index].tracerIndex`, which is `C_i` ("Carbon"). But the outgassing saturation test reads `localWCTracers[CO2_i]`, a separate tracer (atPhysChemIO.c:122-123). Either the input goes to the wrong tracer or the test reads the wrong one. Function: `Carbon_ROC()`. | Use one tracer consistently (probably CO2_i for both, or C_i for both). | [ ] |
| NUTR-05 | 509, 719, 921, 1044 | wrong variable / sign | H/L | The outgassing debug loss uses `atmospheric_X` instead of `outgas_X` (NH 509, O2 921, CO2 1044). In Nitrate (719) it is also `-=` on `loss_id`, so the recorded loss goes negative. Only debug flux output is affected. Functions: `Ammonium_ROC()`, `Nitrate_ROC()`, `Oxygen_ROC()`, `Carbon_ROC()`. | `DebugFluxInfo[..][WC][loss_id] += (bm->outgas_X / dzz) / numsec;` | [ ] |
| NUTR-06 | 603-612 | cohort loop missing | M/L | The ICE case uses only `mortality[0]` and `lysis[0]` with no loop over `numCohortsXnumGenes`, unlike the WC/SED cases (540, 573). Si-dependent groups with genotypes (>1 cohort slot) lose all but slot 0. Function: `Detrital_Silica_ROC()`. | Loop `cohort < numCohortsXnumGenes` as in WC case. | [ ] |
| NUTR-07 | 792-806 | incomplete reset | L/L | When `!bm->flagmicro`, WC and SED fluxes are zeroed but `localICEFlux[MicroNut]` is not, even though the ICE debug values are. Function: `Micronutrient_ROC()`. | Also zero `localICEFlux[*tracerIndex]` when ice_on. | [ ] |


#### atecology/atdiversity.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| NUTR-08 | 959-1023, 884-957 | missing switch case → crash | H/H | `TRAIT_INDICES` includes `rsmid_trait_id` and `rsprop_trait_id` (atlantisboxmodel.h:2030-2031), which are used elsewhere (atprocess.c:2516, atdemography.c:914, and Get_Orig_Trait_value 1102-1107). `Get_Trait_value()` and `Apply_Evol_Scalar()` have no cases for them and fall to `quit(" No such trait as yet")`. Find_Evolution_Stats (151, 184, 199), Find_Evolution_Shift (326), Do_Evolution (702-714) and Do_Aging_Update_Trait (846-867) call these for every active trait, so activating either trait aborts the run. Functions: `Get_Trait_value()`, `Apply_Evol_Scalar()`. | Add cases reading and scaling `bm->evolTraitValues[species][cohort][rsmid/rsprop_trait_id]`. | [ ] |
| NUTR-09 | 306-307 | wrong id constant | H/M | `MIGRATION[species].SN[cohort][expect_id]`, `.RN[...][expect_id]` and `.DEN[cohort][expect_id]` use `expect_id`, a Black Book economics id (`#define expect_id 0`, atlantisboxmodel.h:1570). The second index of these arrays is the migration-event index qid (allocated `[cohort][maxnum]`, atBiologyXMLParamIO.c:4379-4381, used as `[cohort][qid]` in atmigration.c:770). So only migration event 0 counts toward the external spawn contribution, and the rest are ignored. Function: `Find_Evolution_Shift()`. | Sum over `qid < maxnum` (or the active event) instead of `expect_id`. | [ ] |
| NUTR-10 | 1025-1039, 1116-1130 (called at 62) | index unit confusion | H/M | `startage = age_class` and `endage = age_class + numGeneTypes` treat the age class as a flat cohort×gene index. Find_Evolution_Stats line 62 calls `Get_Mean_Orig_Value(bm, species, cohort, ...)` with `cohort` = age class 0..numCohorts-1, so for age class a>0 it averages slots a..a+ngene-1 (mixed ages and genotypes) instead of `a*ngene .. a*ngene+ngene-1`. Calls with 0 are unaffected. Functions: `Get_Mean_Value()`, `Get_Mean_Orig_Value()`. | `startage = age_class * numGeneTypes; endage = startage + numGeneTypes;` | [ ] |
| NUTR-11 | 113-122, 253-260 | loop nesting / overcount | H/M | For BIOMASS/AGE_STRUCTURED_BIOMASS EPIFAUNA groups, `bm->boxes[b].epi[den] * area` is added inside the `for (k < nz)` layer loop, so each box's epibenthic biomass counts nz times. Boxes with more layers get extra weight in tot_num, num and tot_abund. Functions: `Find_Evolution_Stats()`, `Find_Evolution_Shift()`. | Move the EPIFAUNA accumulation outside the k loop (or only when k==0). | [ ] |
| NUTR-12 | 61-68 | overwritten initialisation | H/M | Line 63 sets `trait_shift[ntrait][cohort][0][evol_birth_var_id]` to initial variability. The loop at 65-68 then sets `evol_birth_var_id = 0.0` for every na, including na=0, which wipes it. Initial birth variability is always 0, which later becomes the `std_dev_parents` divisor in Do_Evolution:758. Function: `Find_Evolution_Stats()`. | Do the `na` loop first, then set line 63's value (or start the loop at na=1). | [ ] |
| NUTR-13 | 61-62, 153, 172 | accumulator not reset | M/M | At dayt==0, `evol_theoretmean_id` is first set to `Get_Mean_Orig_Value(...)` (62), then `+= param_value` for each genotype (153), then `/= ngene` (172). The result is (orig_mean + Σvalues)/ngene, which is too high by orig_mean/ngene. Function: `Find_Evolution_Stats()`. | Store the orig mean in a separate field, or don't seed theoretmean before the accumulation. | [ ] |
| NUTR-14 | 635-648 | logic error | M/M | When variable expression is on, the last genotype (`ngene == numGeneTypes-1`) gets `scalar = 1.0`. For every `ng != numGeneTypes-1`, all remaining `dennow` goes to the last genotype before reaching line 648, so nothing stays with its own genotype and the last bin soaks up the rest. Only ng == last keeps residuals. Function: `Do_Phenotyic_Expression()`. | Don't use scalar=1 for the last gene; use random shift for all ngene != ng and leave the remainder for ng. | [ ] |
| NUTR-15 | 171 | division by zero | M/L | `trait[..][evol_mean_id] /= DNA[species].tot_num[cohort]` has no guard. An extinct age class gives NaN that spreads into Find_Evolution_Shift (277) and Write_Evolution. Function: `Find_Evolution_Stats()`. | Divide by `(tot_num + small_num)`. | [ ] |
| NUTR-16 | 756, 758 | division by zero | M/M | `mean_param_val / trait[..][evol_theoretmean_id]` and `/ std_dev_parents` have no small_num guard. `evol_parent_var_id` is only set in Do_Inheritance (406), and initial birth var is 0 (see NUTR-12), so this can give inf/NaN evol_scalar (the clamp at 768-771 passes NaN through). Function: `Do_Evolution()`. | Add `+ small_num` to both denominators. | [ ] |


#### atecology/atbiophysics.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| NUTR-17 | 560, 578-660 | stale globals | M/M | `H2Otemp`, `current_SALT` and `current_PH` are only assigned inside `if (!checked_already)`. When the box/layer has already been checked since the last reset of `checkedalready` (attime.c:84 daily, attempsalt.c on forcing), these globals keep whatever box/layer was computed last. Parameter_Q10 (atq10.c:126-133) then computes Tcorr from that stale value. This hits repeat calls such as atmovement.c:904/3029 (box 1) and atexternalpop.c:42, and every timestep within a day when no forcing reset happens. Function: `Properties_At_Depth()`. | In the `else` branch reload `H2Otemp = Y1[Temp_i]; current_SALT = Y1[Salinity_i]; current_PH = track_pH ? Y1[pH_i] : 8.0;`. | [ ] |
| NUTR-18 | 626 | wrong variable | M/M | The depth correction uses the global `cell_depth` instead of the `cdepth` argument (which line 567 uses). Callers passing an explicit depth (atmovement.c:904 passes 0, atexternalpop.c:42 passes 0) get a correction based on whatever `cell_depth` was left by the last atbiology loop. pH_At_Depth (705) has the same problem. Function: `Properties_At_Depth()`, `pH_At_Depth()`. | Use `cdepth` (and make callers pass the correct depth). | [ ] |
| NUTR-19 | 161-172 | unit / double count | M/L | `intensity = 1.353 * (...)` is already a solar flux in kW m-2, because 1.353 is the solar constant. It is then multiplied by `Io`, which also contains the solar constant (`swr_const`, ~1367), so SWR is too high by about 1.35x. Also at elevation 0 (night) AM ≈ 38 still gives a nonzero intensity. Function: `Calculate_Box_Light()`. | Use `(intensity / 1.353) * Io` (or drop Io's constant). | [ ] |
| NUTR-20 | 764 | log flooding | H/L | An unconditional `fprintf(llogfp, ...)` runs on every pH_At_Depth call in the regression branch (every box/layer/timestep), which produces very large log files. Function: `pH_At_Depth()`. | Guard with debug flag or remove. | [ ] |


#### atecology/atcoral.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| NUTR-21 | 932-942 | wrong constant | H/H | When the model has no turf, `facilitate_space = 1.0`, then `ans = mum * (1 - compete_space) * (1.0 + facilitate_space)`, which doubles coral/sponge growth. It also discards any HABITATlike facilitation accumulated at 919-920. The returned scaled_mum is passed into Eat() as the heterotrophic mum (atGroupProcesses.c:1311), so heterotrophic growth doubles in every no-turf model. Function: `Coral_Space_Competition()`. | Set `facilitate_space = 0.0` (neutral for the `1+f` form) or only reset it when no epiff facilitation applies. | [ ] |
| NUTR-22 | 369, 398-437, 534-543 | variable only set in gated block | H/M | `time_since_last_shock` starts at 0 and is only updated inside `if (do_DHW_now)`. That block is disabled for `cohort > 0` and for `it_count > 1` (398-402). Recovery needs `time_since_last_shock > 0` (534) or `> recovery_periodB` (537), so `PropUnBleached[cohort>0]` never recovers after bleaching. Cohort 0 recovers only on the first adaptive iteration. Function: `Do_Bleaching()`. | Store time-since-shock per box in CORALREEF (computed when the queue updates) and read it for all cohorts and iterations. | [ ] |
| NUTR-23 | 446-457 | repeated application | M/M | The bleaching decrement `PropUnBleached -= prop_bleaching` and the extra mortality are applied on every call while `DHWsum > thresh`, including every adaptive-step iteration (`it_count > 1`), not once per timestep. Effective bleaching scales with the number of iterations. Function: `Do_Bleaching()`. | Gate the PropUnBleached update with `it_count == 1` (mortality can stay as a rate). | [ ] |
| NUTR-24 | 104-110, 189-191 | mass balance | M/M | Polyp mortality N `M = SPmortNH_polyp` is split as NH `M(1-FDM)(1-H)`, symbiont recycling `M(1-FDM)H` (110) and detritus `M FDM (1-H)` (190-191). These sum to `M - M·H·FDM`, so N leaks whenever Host_Resp_Remin>0 and FDM>0. Also `SP_zooxanth_mort /= smLayerThick` and `SP_polyp_mort /= smLayerThick` (105, 107) are computed and never used. Function: `Grow_Coral_Symbionts()`. | Detritus terms should be `SPmortNH_polyp * FDM_SP * FDL_SP` (no `(1-H)`). | [ ] |
| NUTR-25 | 1338 (atGroupProcesses.c) vs 686-752, 684 | flux used as state | M/H | In the species-level path, mechanistic/blackwood/bioerosion return a rate of change ("returning a flux so that will be added to LocalRugosity", 751). The caller atGroupProcesses.c:1338 assigns it straight into the state `localWCTracers[Rugosity_i]`, and each coral/sponge guild/cohort overwrites the previous one. The bozec case returns a state. The georugosity floor (818-821) is applied to the flux value. Function: `Calculate_Rugosity()`. | Caller should accumulate into `localWCFlux[Rugosity_i]` for flux models, and only assign for bozec. | [ ] |
| NUTR-26 | 45-47, 847 | index (cohort vs stage) | M/M | `cohortSpeciesParams` is allocated `[numStages][params]` (atUtilGroupIO.c:830/855/877), but here it is indexed by `cohort` (up to numCohortsXnumGenes-1): `[cohort][mL_id]`, `[cohort][mQ_id]`, `[cohort][mE_id]`, `[cohort][rugosity_dec_id]`. Elsewhere in this file it is correctly `[stage]` (642, 687, 711). Coral groups with more cohorts or genotypes than stages read out of bounds. Functions: `Grow_Coral_Symbionts()`, `Destroy_Rugosity()`. | Use `FunctGroupArray[guild].cohort_stage[cohort]` as the first index. | [ ] |
| NUTR-27 | 852 | log flooding | H/L | Unconditional `fprintf(llogfp, "Destroy_Rugosity: ...")` on every call (every fishery/box/cohort). Function: `Destroy_Rugosity()`. | Guard with debug flag. | [ ] |
| NUTR-28 | 925 | log flooding | M/L | `fprintf(bm->logFile, "DANGER DANGER DANGER ...")` runs on every legitimate maturation-trait update path (when `min_li_mat <= 0`). Function: `Apply_Evol_Scalar()` (atdiversity.c, listed here for completeness). | Remove or guard. | [ ] |


#### atecology/init.c


No issues found. The entire file (lines 1-935) is commented out (every line starts with `//`), so it contains no live code. Functions present but inactive: init_tr2ex, build_box, set_boxes, set_cells, set_btscells, check_boundary_cells, find_link, read_link_weights, add_links, set_fixed_links, set_links, read_tracers, read_forcing, read_weights, set_time, check_time_range, set_work_variables, init.


#### atecology/atmigration.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| EINIT-01 | 204-207 | wrong variable / copy-paste | H/H | `MIGRATION[sp].all_go[cohort][currentID] = MIGRATION[sp].all_go[cohort][migrationID];` copies `all_go` from itself. `all_go` is allocated zeroed (atBiologyXMLParamIO.c:4353), and the parameter values are only ever written to `all_go_Prm[stage][migrate]` (atBiologyXMLParamIO.c:4121, 4143). This line is the only writer of `all_go` anywhere in the repo, so `all_go` stays 0 for every queue entry. The "whole cohort leaves" branches in atdemography.c:3062/3086 (`if (MIGRATION[species].all_go[cohort][qid] && ...)`) therefore never fire. Function: `Init_Migration()`. | `MIGRATION[sp].all_go[cohort][currentID] = MIGRATION[sp].all_go_Prm[stage][migrationID];` (`all_go_Prm` is filled by stage index) | [ ] |
| EINIT-02 | 92-150 | logic / always-true condition | H/M | The pairwise check loops over all ordered pairs (migrationID, migID) and does not skip empty (0/0) slots. (a) The quit test `(sp_Migrate_Time_orig > sp_Migrate_Return_chk) && (sp_Migrate_Time > sp_Migrate_Return_chk)` repeats the same condition (Time is never modified after `_orig` is set). It fires whenever entry migrationID starts after migID returns. So two correctly ordered, non-overlapping migrations (e.g. 10-50 and 100-150) quit when evaluated as (1,0), and so does any real entry compared against an unused 0/0 slot of the same stage. The message also says "happens completely before", which is the opposite of what is tested. (b) The overlap tests at 126 (`Return > Time_chk`) and 134 (`Time < Return_chk`) are each only half of an interval-overlap test, so nearly every species with num_migrate>1 gets `isComplexMigrator = TRUE`. Line 116 is unreachable after `quit`. Function: `Init_Migration()`. | Only test `migID > migrationID`, skip 0/0 slots (as line 164 does), and use `(Time < Return_chk && Return > Time_chk)` for overlap | [ ] |
| EINIT-03 | 158-421 | buffer overrun (queue size) | H/H | For multi-year migrators the year-0 "start with return" branch writes `this_currentID = currentID + counter*num_migrate` for `counter < MinYearsAway`, then `last_currentID` jumps to that value, so every (stage, migration) at yr 0 uses `(Y-1)*nm+1` slots. Total slots = `S*nm*((Y-1)*nm + nyears)`, but the arrays hold `maxnum = nyears*S*(nm+1)` (atBiologyXMLParamIO.c:4334). This overflows when `nm^2*(Y-1) > nyears`, e.g. nm=1, Y=5 on a run of 3 years or less, or nm=2, Y=3 on a run under 8 years. Nothing bounds `currentID`/`this_currentID` against `num_in_queue`, so Leave_Now, Return_Now, all_go, Box, Stagger, etc. are written past their allocation (heap corruption). Function: `Init_Migration()`. | Add `if (this_currentID >= MIGRATION[sp].num_in_queue) quit(...)`, and size maxnum (and the Check_Migration:550 num_in_queue) to include `S*nm*nm*(MaxYearsAway-1)` | [ ] |
| EINIT-04 | 191-202 | cohort vs cohort×genotype | M/M | `nagemat` (age_mat, in cohort units) is used directly as a bound and start index in cohort×genotype space: `for(cohort = 0; cohort < nagemat; ...)` and `for(cohort = nagemat; cohort < numCohortsXnumGenes; ...)`, plus `start_cohort = nagemat`. When numGeneTypes>1, juveniles are flagged only for the first `nagemat` cohort×gene slots, and adult flags and `start_cohort` begin at the wrong slot. This also assumes exactly two stages (`!stage` vs else). The same `start_cohort` feeds `EMBRYO.Age_Now[cohort]` and the aging loops in Init_Migration_Age_Check. Function: `Init_Migration()`. | Use `nagemat * FunctGroupArray[sp].numGeneTypes` for the bounds and `start_cohort` | [ ] |
| EINIT-05 | 164-166 | logic (break vs continue) | L/M | `if(!StartDay_Prm[stage][migrationID] && !EndDay_Prm[stage][migrationID]) break;` stops processing all later migration entries for this stage. If entry 0 of a stage is unused but entry 1 is real, the real migration is silently dropped. Function: `Init_Migration()`. | Use `continue` unless entries are guaranteed to be packed | [ ] |
| EINIT-06 | 585-596 | logic (max overwritten) | M/M | In the loop computing the maximum years away, the `else` branch sets `max_years = 1;` whenever an entry is annual. A larger MaxYearsAway found in an earlier entry is discarded, so `num_away`, `num_cohorts_away` and `needed_for_age_away` are under-computed. Function: `Check_Migration()`. | `else max_years = max(max_years, 1);` | [ ] |
| EINIT-07 | 600 | integer division / cohort vs genotype | M/L | `num_cohorts_away = ceil(b + (max_years / FunctGroupArray[sp].ageClassSize));` divides two ints (`ageClassSize` is int, atFunctGroup.h:90), so the `ceil` does nothing and any max_years < ageClassSize adds 0 cohorts. The cohort offset is also added to a cohort×gene index `b` without multiplying by numGeneTypes. Function: `Check_Migration()`. | `b + numGeneTypes * (int)ceil((double)max_years / ageClassSize)` | [ ] |
| EINIT-08 | 630-631 | unit / genotype split | H/M | `SN[b][counter] = InitSN[b][0] / ngenes; RN[b][counter] = InitRN[b][0] / ngenes;` SN and RN are per-individual sizes (mg N per fish). The KMIG reader already copies the same per-individual value to every genotype (atBiologyXMLParamIO.c:4188-4194, `values[basechrt]`). Dividing the size by ngenes makes returning migrants 1/ngenes the correct weight whenever numGeneTypes>1. Only DEN (numbers) should be split. Function: `Check_Migration()`. | Remove `/ ngenes` from the SN and RN lines and keep it on DEN | [ ] |
| EINIT-09 | 692-694 | wrong variable in message | H/L | `quit("... StaggerYears cannot be set to %d ...", (int)(FunctGroupArray[sp].speciesParams[stagger_years_id]), ...)`. `stagger_years_id` (=1, atlantisboxmodel.h:753) is a column index into `MIGRATION.Stagger`, not a speciesParams id, so the message prints an unrelated species parameter. Function: `Init_Migration_Age_Check()`. | Print `MIGRATION[sp].Stagger[qid][stagger_years_id]` | [ ] |
| EINIT-10 | 740-753 | loop logic / overwritten result | M/M | `end_pt[qid] = cohort + yrs_to_age_pre_model[qid]` is assigned inside a `cohort` loop running from top to 0, so only the cohort 0 value survives and `end_pt = yrs_to_age_pre_model`. The aging loop at 757 (`cohort = end_pt-1 .. start_cohort`) then does nothing for adult-stage entries (start_cohort = nagemat > end_pt-1). For juvenile entries it only ages the lowest few slots, so pre-model aging of migrants is mostly skipped. Function: `Init_Migration_Age_Check()`. | Drop the cohort loop and set end_pt = min(numCohortsXnumGenes-1, top cohort of the migrating stage + yrs_to_age) or similar intended value | [ ] |
| EINIT-11 | 743 | integer division | H/M | `diff_nc = (int)ceil((Return_Now[qid] - Leave_Now[qid]) / 365);` Both operands are int, so the division truncates before `ceil` (400 days gives 1, not 2). This feeds `yrs_to_age_pre_model = sp_Migrate_Years - diff_nc`. Function: `Init_Migration_Age_Check()`. | `/ 365.0` | [ ] |
| EINIT-12 | 175, 304 | log noise | H/L | `printf("Doing migration entry %d ...")` and `printf("Step1 Doing currentID ...")` are unconditional and run for every year × stage × migration of every species. Function: `Init_Migration()`. | Guard with `if (verbose > 1)` | [ ] |


#### atecology/atIceProcesses.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| EINIT-13 | 127-130 | sign / physical constant | M/M | `Y1[Temp_i] = 1.8;` sets every non-slush ice layer to +1.8 °C. Sea ice sits at or below the seawater freezing point (about -1.8 °C), and the slush layer is set to 0.0, which makes the solid ice warmer than the slush. This temperature drives `Get_Tcorr` for ice species in `Box_Ice_Q10`. Function: `Box_Ice_Temperature_Related()`. | `Y1[Temp_i] = -1.8;` | [ ] |
| EINIT-14 | 172-264 | wrong loop direction | M/M | Ice layer 0 is the surface layer (atvertgeom.c:571-575 "Shrink into layer closer to the surface" k to k-1, insert_ice_layer "Move deeper layers down"). `currentnz-1` is the bottom slush layer next to the water (used as the bottom in Box_Ice_Flux:39/57 and as slush in Box_Ice_Temperature_Related). The light loop `for(i = currentnz-1; i >= 0; i--)` starts with `lighttop = pBox->swr` at the bottom layer and attenuates upward, and `ice->bottom_light` ends up as the light below the top layer. Function: `Box_Ice_Light_Level()`. | Loop `for (i = 0; i < ice->currentnz; i++)` | [ ] |
| EINIT-15 | 285, 295 | uninitialised / stale value | H/M | `local_current_corr` is initialised to 1.0 and never derived from `current_temp`. For `base_q10_id` groups, `Get_Tcorr` returns `pow(q10, 1.0) = q10`, a constant regardless of ice temperature. After any humped-Griffith group, `Get_Tcorr` overwrites it with `T - Topt` (atq10.c:202) and later species reuse that value. It also drives the sign tests at 341/346. Function: `Box_Ice_Q10()`. | Inside the sp loop set `local_current_corr = (current_temp - bm->baseline_temp)/10.0;` before calling Get_Tcorr | [ ] |
| EINIT-16 | 310 | wrong index (ice vs water layer) | M/M | `Get_Pollutant_Corrections(bm, sp, pBox->n, ice_layer)` passes an ice-layer index, and the callee reads `bm->boxes[b].tr[clayer][Noise_Pollution_i]` (atq10.c:418), the water-column tracer array. Ice layer 0 is the top, but water layer 0 is the deepest layer. With fast ice (`nz == 0`, atvertgeom.c:612) this indexes past the water-column array. Function: `Box_Ice_Q10()`. | Pass `ice->wc_layer_match[ice_layer]` (and guard nz>0), or use the ice tracers | [ ] |
| EINIT-17 | 604 | copy-paste / wrong group index | H/M | `int flag_sp = (int) (FunctGroupArray[pelagicBactIndex].speciesParams[flag_id]);` was copied from the pelagic version (atprocess.c:2156). The sediment version correctly uses `SedBactIndex` (atprocess.c:2177). Ice bacteria partitioning is therefore switched on or off by the pelagic bacteria flag: if pelagic bacteria are off, ICEB_DL/DR are 0 even when ice bacteria are active. The header comment ("Sediment Bacteria partitioning") is also copy-pasted. Function: `Calculate_IceBact_Scale()`. | Use `IceBactIndex` | [ ] |
| EINIT-18 | 720-742 | swapped variables / unreachable branch | H/M | `BotDpth` = sum of dz above the current layer (i.e. the depth of its top), and `TopDpth = BotDpth + dz` (the depth of its bottom). These are the reverse of the sediment `Avail()` in atprocess.c:1841-1842. As a result `if (BotDpth <= Depth)` gives full availability to any layer that merely starts above Depth, and the partial branch `else if (TopDpth < Depth)` can never be reached (TopDpth > BotDpth). `PercentDpth` is computed from the wrong boundary. Function: `Ice_Avail()`. | `TopDpth = sum; BotDpth = TopDpth + dz[current_icelayer];` | [ ] |


#### atecology/atBuildTracer.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| EINIT-19 | 76 | buffer overrun (latent) | M/L | `strcpy((*tracerArray)[index].name, name);` writes into `Namelist.name[100]` (atlantisboxmodel.h:6063), but the names are built in `str[STRLEN*2]` (4000 bytes) from group names plus contaminant names (e.g. `"%s%d_Prop_%s"`, lines 381/540). A long group name plus contaminant name overflows the struct. Function: `Add_Tracer()`. | `snprintf((*tracerArray)[index].name, sizeof((*tracerArray)[index].name), "%s", name);` | [ ] |
| EINIT-20 | 435-443 vs 466-498 | size/count mismatch | L/M | The size count adds `numCohortsXnumGenes` for every EPIFAUNA group, but the build adds 0 for AGE_STRUCTURED epifauna and only 1 for BIOMASS epifauna unless `flag_macro_model==1 && SEAGRASS`. Unlike Build_TrName_NameList:400 there is no index-vs-size check, so `nEpiSize` can exceed the number of built entries. atbiolsetup.c:1295 then dereferences the terminator entry or uninitialised entries (`epinamelist[ij].name` and `.index`). Function: `Build_EPI_NameList()`. | Make the size count mirror the build branches and add the same `(index-1) != nEpiSize` quit check | [ ] |


#### atecology/atq10.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| EINIT-21 | 151, 194, 202 | global state clobbered | H/H | `Get_Tcorr(bm, sp, H2Otemp, &bm->current_corr)` passes the global `bm->current_corr`, and the humped-Griffith branch writes `*current_corr = current_temp - opt_temp;`. Every later species in the loop that uses `base_q10_id` then gets `pow(q10, T - Topt)` instead of `pow(q10, (T - baseline)/10)`. For example, with T=15 and Topt=2 the exponent is 13, giving Q10^13. `Apply_Q10_Corrections` (lines 522/527/532) also uses the clobbered `bm->current_corr` for its warm/cool sign tests. Results depend on species ordering. Function: `Parameter_Q10()` / `Get_Tcorr()`. | Pass a per-species local copy: `double cc = bm->current_corr; Tcorr = Get_Tcorr(bm, sp, H2Otemp, &cc);` | [ ] |
| EINIT-22 | 163-170 | ordering / lost update | H/M | `Scale_Linear_Mortality` multiplies `cohortSpeciesParams[stage][mL_id]` by the forcing and change scalars. When `track_contaminants` is TRUE, `Apply_Q10_Corrections` is called later from atecology.c:451/1484/2148, and it recomputes `mL_id` from `mL_T15_id` (lines 570/578). The linear-mortality time series and change scaling is therefore discarded whenever contaminants are tracked. Function: `Parameter_Q10()`. | Call `Scale_Linear_Mortality` after `Apply_Q10_Corrections` in the contaminant path too (e.g. move it inside Apply_Q10_Corrections) | [ ] |
| EINIT-23 | 412-433, 566-579 | division by zero | L/M | `Get_Pollutant_Corrections` can return 0 (when noise_coefft or light_coefft is 0) or a very large value, with no `small_num` floor, unlike Get_Tcorr and Get_pHcorr. The result is used as a divisor in `(1.0 / FunctGroupArray[sp].PolluteCorr)` for mS, mL and mQ. Function: `Get_Pollutant_Corrections()` / `Apply_Q10_Corrections()`. | Floor `ans` at `small_num` before returning | [ ] |
| EINIT-24 | 254 | stale message | H/L | The quit text says "q10 effect method can only be 0-2", but there are 4 methods (base, humped Griffith, Heinichen, CEATTLE). Function: `Get_Tcorr()`. | Update the text | [ ] |


#### atecology/atcalibtools.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| EINIT-25 | 364-375, 382-389, 434-440 | off-by-one / OOB read | M/M | `qid = EMBRYO[sp].next_larvae; if (SpawnRecruitOverlap) qid++;` is then used in `Larvae[k][n][qid]` and `StartDay[n][qid]`. Both arrays have queue length `max_num = num_in_spawn_queue` (atbiolsetup.c:3807/3816). The guard `if (next_larvae > num_in_spawn_queue)` permits `next_larvae == num_in_spawn_queue`, and the `++` can add one more, so the reads can go past the end, most likely at `coming_to_end`. Function: `Check_Gape()`. | Guard with `if (qid >= num_in_spawn_queue) qid = num_in_spawn_queue - 1;` (and use `>=` in the check) | [ ] |
| EINIT-26 | 486-538 | buffer overrun | M/M | `strcat(keywithin/keymore/keyless, key)` appends about 25-30 chars per prey group (e.g. `" CODE-12 (>SN 1.234567e+00)"`) into `char[BMSLEN*2]` (1024) buffers with no length check. A predator stage with more than about 35-40 linked vertebrate prey in one category overflows the stack buffer. Function: `Check_Gape()`. | Track remaining space and use `strncat`, or `snprintf` at an offset with a size check | [ ] |


#### atecology/atForcedMovement.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| EMISC-01 | 63, 306 | uninitialised variable | H/M | `if(propInput->curFile + 1 >= propInput->nFiles)` — `forceMoveEntryInput` is `malloc`ed (line 63) and `nFiles` is never assigned (grep: no `forceMoveEntryInput[..].nFiles` anywhere; local `nFiles=1` is never stored). The end-of-file test reads garbage: if garbage > 1, `atEnd` is never set, and the read goes on past `nstep` with `count[0] = nstep - tstepnum <= 0` (line 320/332), so a zero/negative alloc and a fatal ncvarget. Function: `get_move_property()` (set in `init_forceMoveEntries()`). | Add `bm->forceMoveEntryInput[tracerIndex].nFiles = nFiles;` in init (and init `fid=-1`) | [ ] |
| EMISC-02 | 80-92, 115-117 | NULL/invalid index | H/M | `tracerID = -1` stays -1 when no `<code>_stage_<n>` name matches, then `FunctGroupArray[speciesIndex].moveEntryIndex = 0` writes `FunctGroupArray[-1]`; `stageID` is also left uninitialised (malloc). There is no check, so a typo in `MoveGroupCodes` corrupts memory. Function: `init_forceMoveEntries()`. | `if (tracerID < 0) quit("...%s not recognised...")` | [ ] |
| EMISC-03 | 467, 482 | float equality / logic | M/M | `if (bm->dayt != ...movetstart)` uses exact double equality. If `<code>_tstart` is not exactly on a model step, `Init_full_move_array` never runs. Line 482 (`dayt > movetstart`) still starts reading records into `distrib[..][next_qrt]`, so the initial `qrt` slot is never filled from the file. Function: `Ecology_Update_Move_Entry()`. | Use `bm->dayt >= movetstart && !moveInitDone` | [ ] |
| EMISC-04 | 225-255 | wrong order of checks | M/L | The `nctypelen(daty)` type check runs before `status` is checked. When `missing_value`/`valid_max`/`valid_min` is absent, `daty` holds whatever was left from the previous call, so the type test is meaningless (it could spuriously quit). Function: `open_move_prop()`. | Test `status >= 0` first, then the type | [ ] |
| EMISC-05 | 268-281 | leak | H/L | `close_move_prop()` is never called (grep), so netCDF ids, `tbuf`/`valuebuf`, `fname`, `variableName` and `use_resets` are never freed. Function: `close_move_prop()`. | Call it on shutdown / at atEnd | [ ] |


#### atecology/attime.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| EMISC-06 | 492-529 | logic / wrong variable | H/H | For groups with `numMoveEntries == 4`, `index = floor(TofY/91.25)` is compared with `orig_index`, but line 527 then overwrites `moveEntryIndex = bm->QofY`, whose boundaries are 90/181/272. Next day `orig_index` is QofY, not the 91.25-based index. Worked through: day 91 has orig=0 and index=0, so no update, yet moveEntryIndex becomes 1; day 92 has orig=1 and index=1, so no update. The same happens at days 182/183 and 273/274. As a result `NeedMoveUpdate` is set only at `TofY==0`, and forced-movement files (`Ecology_Update_Move_Entry`) are read only once a year for 4-season groups. Function: `Calculate_Move_Date_Index()`. | Do the `numMoveEntries==4` override (index=QofY, next=NextQofY) before the `index != orig_index` test | [ ] |
| EMISC-07 | 125-128, 192-209 | off-by-one | M/L | `PointInYear` can be 365 (rounding up a remainder of 364.5 or more). That gives `DayInQuarter = 93 > DaysInQ = 92` and `HowFar > 1`. Q0 also counts days from 0 while Q1-Q3 count from 1, so `HowFar` reaches exactly 1.0 at the end of Q1-Q3. Function: `Time_OYr()`. | Wrap 365 to 0; make DayInQuarter consistent (subtract 91/182/273) | [ ] |
| EMISC-08 | 152-161 | stale value | M/L | `if (bm->dayt < 1)` runs inside `Time_OYr`, but `Ecology_Time_Check` only updates `bm->dayt` afterwards (line 94), so the test uses the previous step's day. Function: `Time_OYr()`. | Pass/compare `thisday` | [ ] |


#### atecology/dump.c


(Note: this file is not in `libatecology_a_SOURCES` and `tr2ex.h` does not exist, so it is dead code. Severities are latent.)

| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| EMISC-09 | 300 | format specifier | H/L | `fscanf(f, "bnd_vert %e %e ", &p.x, &p.y)` reads into `dpoint` members that are `double` (sjwlib/include/polyline.h:19-20); `%e` writes a float. Function: `init_from_bm_dump()`. | Use `%le` | [ ] |
| EMISC-10 | 445-463 | NULL dereference | H/L | `hd->fname`, `hd->fid`, `hd->nb`... are read in the declarations before the `if (hd == NULL ...)` check. Function: `write_exchange_dump()`. | Move the NULL check before the dereferences | [ ] |
| EMISC-11 | 104, 111, 189-194, 247-252 | leak / uninitialised | M/L | `validu` is never freed; `meco->zgrid[nk]` is never set; for a non-boundary cell with 0 MECO cells, `c->mecocells` is left uninitialised (only the boundary branch sets NULL). Function: `init_from_meco_dump()`. | free(validu); set mecocells=NULL in both branches | [ ] |


#### atecology/atExternalScalar.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| EMISC-12 | 231-235 | logic | H/M | `for (i = length-1; i >= 0; i--) if (isdigit(str[i])) index = i;` has no `break` on a non-digit, so it returns the first digit anywhere in the string, not the start of the trailing number. A group code containing a digit (e.g. "ZM2", or "BB2" + cohort) is split at the wrong place: wrong cohort, or group = -1 and quit. Function: `Get_Group_Cohort_Value()`. | Scan back while `isdigit`, and stop at the first non-digit | [ ] |
| EMISC-13 | 63, 271-277, 285-290 | buffer overrun | H/M | `Species_Cohort_Lookup[group][cohort][..] = variableIndex` checks only `cohort == -1`. The array is `[K_num_tot_sp][K_num_max_cohort][NUM_BIOLOGY_SCALAR]` (line 63, reversed-dims convention), so a variable like `XXX12mort_ff` for a 10-cohort group writes out of bounds. Function: `Match_Scalar_To_Groups()`. | Also quit if `cohort >= FunctGroupArray[group].numCohorts` | [ ] |
| EMISC-14 | 380, 403-406 | off-by-one | M/L | `start[0] = max(tstepnum - 1, 0)`, but `bufstart = tstepnum`, `bufend = tstepnum + count - 1` and `nextrec += count`. Every chunk after the first is read one record early, so the final record `nstep-1` is never read, although `bufend` claims it is loaded. Function: `Get_Scalar_Property()`. | Make start/bufstart/bufend consistent (either no -1, or set bufstart=start) | [ ] |
| EMISC-15 | 428-468 | stale index | H/M | After `Get_Scalar_Property()` reloads the buffer (line 431/440), `currentTimeIndex` is not reset, and the search loop starts from the old index. If that index is `>= current_nbuf` the loop never runs and a stale slot is used; otherwise earlier matching slots are skipped. Function: `Calc_Scalar_Values()`. | Reset `currentTimeIndex = 0` whenever the buffer is reloaded | [ ] |
| EMISC-16 | 430-446 | logic | M/M | The rewind time offset (`+ nstep*rewindCount*dt`) is applied only to the chunk loaded right at rewind. Later chunk loads (line 431) after a rewind keep raw file times, so `bm->t` is always past the buffer and the data jumps or reloads every step. This only matters for multi-chunk files with rewind on. Function: `Calc_Scalar_Values()`. | Apply the offset inside `Get_Scalar_Property()` using `rewindCount` | [ ] |
| EMISC-17 | 156 | format / uninitialised | H/L | The `quit("... %s dt must have positive value", propInput->variableName)` message passes `variableName`, which is `char**` and not allocated until line 165 (struct was malloc'ed). Function: `Open_Scalar_Prop()`. | Print `propInput->fname` instead | [ ] |
| EMISC-18 | 63 | cohort vs cohort x genotype | M/M | The lookup is sized `K_num_max_cohort`, but atq10.c:491-496 calls `Ecology_Get_Growth_Scalar(bm, sp, cohort)` for `cohort < numCohortsXnumGenes`, which reads `Species_Cohort_Lookup[sp][cohort]` out of bounds when the genetics option makes numCohortsXnumGenes > K_num_max_cohort. Function: `Init_External_Scalars()` / `Ecology_Get_Growth_Scalar()`. | Map cohort to base cohort (cohort / numGenes) before lookup, or size by numCohortsXnumGenes | [ ] |


#### atecology/atecologyts.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| EMISC-19 | 351-352 | state not reset | H/M | `if (fabs(scalar - 1.0) > 1e-100) scaled_mum = mum*scalar;` When the time series returns to 1.0, `scaled_mum` keeps the last scaled value. For non-temperature-sensitive vertebrates nothing else resets it: the atq10.c:557-561 else-branch resets only `scaled_C`. Function: `Scale_Growth_Rate()`. | Always assign `scaled_mum = mum * scalar` | [ ] |
| EMISC-20 | 397-401 | state not reset / missing clamp | H/M | Same pattern for FSPB: when the addend returns to 0, `scaled_FSPB` stays at its old value. There is also no lower clamp, so a negative addend can give a negative FSPB. Function: `Scale_FSPB_Rate()`. | Always assign `FSPB + scalar`, then clamp to [0,1] | [ ] |
| EMISC-21 | 45, 91, 131, 460 | global state | M/L | `set_keyprm_errfn(err)` is never restored, so later `readkeyprm_*` calls by the caller inherit `warn`/`quiet` instead of `quit`. Functions: `Read_Ecology_Time_Series()`, `Ecology_Read_Recruit_TS()`, `Ecology_Read_Enviro_Forcing()`, `Ecology_Read_pCO2_TS()`. | Restore `set_keyprm_errfn(quit)` before returning | [ ] |


#### atecology/atbiolUtil.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| EMISC-22 | 143-157 | data loss | H/H | A new zeroed 5D array is allocated and only `[speciesIndex][cohort][box]` is copied (the memcpy is commented out); then the old array is freed. Every other group/stage/box's scheduled linear-mortality changes are wiped, while `numMortChanges` still counts them. Function: `Scale_Group_Linear_Mortality()`. | Copy all [sp][stage][box][0..oldmax) entries into tempArray before inserting | [ ] |
| EMISC-23 | 143 | wrong dimension | H/M | `Util_Alloc_Init_5D_Double(3, maxMortChange, bm->nbox, 2, bm->K_num_tot_sp, 0)` uses stage dimension 2, but the original allocation uses `K_num_max_stages` (atbiolsetup.c:2777). Groups with more than 2 stages index out of bounds after the first realloc. Function: `Scale_Group_Linear_Mortality()`. | Use `bm->K_num_max_stages` | [ ] |
| EMISC-24 | 210-219 | data loss | H/H | Same problem for habitats: only `BEDchange[habitatIndex]` is copied into the new `[K_num_bed_types][max][3]` array, so the scheduled changes of the other habitat types are zeroed. Function: `Scale_Habitat()`. | Copy all bed types' rows before inserting | [ ] |
| EMISC-25 | 138 | debug output | H/L | An unconditional `printf("maxMortChange = %d\n", ...)` runs on every call. Function: `Scale_Group_Linear_Mortality()`. | Guard with verbose | [ ] |


#### atecology/include/*.h


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| EMISC-26 | atEcologyLib.h:68; atassess/atclassical.c:1218-1219, 1555-1556 | swapped arguments | H/H | The prototype and definition (atvertprocesses.c:342) are `Ecology_Spawn(sp, KSPA, FSP, SN, RN, X_RS, FSPB, fp)`, but the atclassical.c callers pass `..., scaled_FSPB[nc], X_RS[nc], ofp`, so X_RS and FSPB are swapped. That gives wrong `WforS = (1+X_RS)*SN` and a wrong FSPB check, so spawning biomass in the classical assessment is wrong. Function: callers in atclassical.c. | Swap to `..., X_RS[nc], scaled_FSPB[nc], ofp` | [ ] |
| EMISC-27 | atExternalScalar.h:19; atecology.h:367, 369, 439, 485; atContaminants.h:39 | stale prototypes | H/L | These prototypes have no definition anywhere and no callers: `Apply_External_Biology_Scalars`, `Get_Aquacult_Feed`, `Update_GrazeInfo`, `Setup_Size_Change_Indices`, `Land_Biology_Process`, `Loose_Contaminant`. They are harmless now but would give link errors if called. | Remove | [ ] |

Header checks done: all 257 other prototypes in atecology/include/*.h were compared against their definitions across the repo (return type, argument count, types, order) with no mismatches found. The `extern` declarations in atbiology.h match the definitions in atbiologyPrivate.h (`cohortParamStrings`/`spawnParamStrings` are defined in atlantisUtil/atUtil.c). There are no conflicting #defines or enums in these headers.


### B.2.2 ateconomic/

#### ateconomic/atquota.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| QUOT-01 | 130-133 | buffer overrun | H/H | `&bm->tsResid[nmarket]` — residual series indexed by market, but atmanage/atManageSetup.c:676-679 states "only one time series allowed" and builds `tseconid` from `tsResid[0]` only. With `K_num_markets`=2 (header comment says two markets are used) and `ntsResid`=1, `tsResid[1]` is read past the end of the array. Function: `Get_Fish_Prices()`. | Use `bm->tsResid[min(nmarket, bm->ntsResid-1)]` or require `ntsResid >= K_num_markets` at setup | [ ] |
| QUOT-02 | 576-578 | static state not reset / copy-paste | H/M | `price_coefft = 0; bind_coefft = 0;` — these are the loop-wide coefficient copies set once at 531/538. Once any species has price < minValue (and UseMinValue off), price and bind terms are switched off for every later species, subfleet and fishery in that call. Function: `Total_Quota_Price()` (NZmodel). | Use per-species locals (e.g. `pc = bm->price_Coefft; bc = bm->bind_Coefft;` inside the sp loop) | [ ] |
| QUOT-03 | 664 | printf format | H/L | `quit("No default (%d) flagLease defined...\n")` — `%d` with no argument (UB). Function: `Total_Quota_Price()`. | Add `, bm->flagLease` | [ ] |
| QUOT-04 | 825-827, 871-872, 915-916 | NULL dereference | H/H | `linearPI[sp][linPI_id][ns] = 0;` — `linearPI` is defined `= 0` in include/atEconomicSetup.h:30 and is never allocated anywhere in the repo (grep finds only atquota.c, the commented-out ateffortquota.c, and the headers). Setting `sp_by_sp`=1 (a documented XML option) crashes straight away. Function: `Quota_trade()`. | Allocate `linearPI` (e.g. `Util_Alloc_Init_3D_Double(totpi, 2, K_num_tot_sp, 0)`) in Post_Load_Array_Alloc and free it | [ ] |
| QUOT-05 | 870, 931, 968, 1098 | wrong index (offset vs absolute) | H/M | `idd = ns*tnf*nm + nm*nf + month - bm->MofY;` so decoding with `% nm` gives `month - MofY`, an offset rather than a month. `buyermonth`/`sellermonth` are then used as absolute month indices into EffortSchedule/BlackBook/MargRent and passed to `Trade()`, so after January the wrong month is used. Function: `Quota_trade()` (sp_by_sp). | `buyermonth = ((int)... % nm) + bm->MofY;` (same for sellermonth) | [ ] |
| QUOT-06 | 920, 960, 962-965, 1055-1064 | off-by-one | H/M | `s = nsort;` then `linearPI[sp][idd_id][s]` — the sorted entries are 0..nsort-1. Index nsort holds a zeroed slot (decodes to fishery 0, subfleet 0, month 0), and when nsort==totpi it reads past the end. Function: `Quota_trade()`. | Start at `s = nsort - 1` | [ ] |
| QUOT-07 | 1101 | stale variable | H/M | Step 2 demand test uses `max_month_effort`, which is still set for the previous buyer (from 1000/1130 or 933). Unlike step 1 (932-933), it is not recomputed for the current `buyernf/buyerns`. Function: `Quota_trade()`. | Recompute down_time/max_month_effort for the buyer inside the loop before line 1101 | [ ] |
| QUOT-08 | 1064-1066, 1131 | uninitialised/stale variable | H/M | Step 2 decodes `sellerns/sellernf` but never `sellermonth`, so `Trade()` gets the stale value from step 1, or 0. Function: `Quota_trade()`. | Decode sellermonth as at 968 (plus the QUOT-05 fix) | [ ] |
| QUOT-09 | 1013, 1023 | division by zero | H/M | `scheduledEffort * ExpCatch / ExpEffort` — no `+ small_num`, unlike every other use of this expression (952, 987). Any month with zero historical effort makes totPlanCatch Inf/NaN. Function: `Quota_trade()`. | Divide by `(ExpEffort + small_num)` | [ ] |
| QUOT-10 | 866 | wrong index | M/M | Inside the `month` loop, `expectedCPUE` uses `BlackBook[..][bm->MofY]` and `EffortSchedule[..][bm->MofY][hist_id]`, not `[month]`, so pi for every future month uses the current month's CPUE. Function: `Quota_trade()`. | Use `[month]` | [ ] |
| QUOT-11 | 1356 | wrong variable | H/M | `Quicksort_Dir(ax, bx, cx, dx, ex, nsort, ascendflag);` — in the package path `nsort` is still 0 (set at 808/811 and only reset in the sp_by_sp branch). The match list (`idp` entries) is never sorted, so trades go in loop order, not best match first. The comment also says matches should be minimised, but `ascendflag`=0 gives descending order. Function: `Quota_trade()`. | Pass `idp`; check whether ascendflag should be 1 here | [ ] |
| QUOT-12 | 1327-1332 | buffer overrun | M/H | `ax[idp] = ...; idp++` runs once per (buyer subfleet, seller subfleet) pair, so idp can reach ~(nf*ns)^2. But ax..ex are sized `totpi = K_max_num_subfleet*K_num_fisheries*12` (ateconomicsetup.c:990-1004). With more than about 13 active fishery-subfleets that all pass the filters, this writes past the end of the arrays. Function: `Quota_trade()`. | Size ax..ex as `(nf*ns)^2`, or bound-check idp | [ ] |
| QUOT-13 | 1166, 1171-1172, 1955 | wrong id constant (array aliasing) | H/M | `bm->SUBFLEET_ECONprms[buyernf][buyerns][within_id]` — `within_id` (#define 15, a QuotaAlloc index, atlantisboxmodel.h:1554) is used to index SUBFLEET_ECONprms, where slot 15 is `res_cost_ind_id` (enum at atlantisboxmodel.h:1386-1400). ateconindicator.c:533 writes reswork there, so the "within" filter really tests research cost, and Trade()/ateconresponse.c:1078/atEconomicAnnual.c:241 overwrite the ResCost indicator with 0/1. The code's own TODO comment admits this. | Add a dedicated subfleet enum entry (e.g. `subfleet_within_id`) and use it at all five sites | [ ] |
| QUOT-14 | 1806-1807, 1651-1655 | missing cap | M/M | Bought quota: in the supply>demand branch there is no cap, so `owned_id -= demand` can push seller owned below 0 and trigger `quit("Negative quota")`. The demand>supply branch caps at total owned, not the un-leased part (1-po-pt)*owned. Latent today, because `keep_trading`=1e9 gives p_buy of about 4e-44. Function: `Trade()`. | Cap at `(1-po-pt)*owned` in both branches | [ ] |
| QUOT-15 | 1449 vs 1271-1272 | inconsistent formula | L/M | spareend is `p+(1-p)*M/12` with threshold `(1-spareend)*Q` when matching, but `p*(1-M/12)` (algebraically) with threshold `spareend*Q` when trading. The matcher and the trader disagree on what is available to sell. Function: `Quota_trade()`. | Use one helper for both | [ ] |


#### ateconomic/ateconeffort.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| QUOT-16 | 131-136, 292, 329, 390-413 | accumulator not reset / double application | H/M | `old_effort`/`new_effort`/`redo_effort`/`max_eff_ratio` are reset once per fishery but accumulated over subfleets (`+=`). The fishery-level `Effort_hdistrib[ij][nf][month]` is rescaled inside the ns loop, so subfleet 0's ratio is applied, then the cumulative (sf0+sf1) ratio is applied again, compounding. `max_eff_ratio` (including the choice_buffer floor at 283-284) also carries from one subfleet to the next. Function: `Monthly_Effort_Schedule()`. | Reset max_eff_ratio per subfleet; move the Effort_hdistrib rescale loop after the ns loop | [ ] |
| QUOT-17 | 138, 274, 305/317, 210 | flag not reset | H/M | `redo_needed` is cleared at 274 and then set to 1 in the month loop (305/317). It is not cleared before the next subfleet's species loop, so for ns>=1 the deemed-value profit recalculation (210) runs even when no DV/TAX was triggered. Function: `Monthly_Effort_Schedule()`. | Set `redo_needed = 0` at the top of the ns loop | [ ] |
| QUOT-18 | 106, 213-215, 225 | accumulator not reset | M/M | `oldtotpi` is initialised only at declaration. When `MonthlyProfitUpdate` is off it accumulates over every month, subfleet and fishery in the call, so `this_profit = totppi/oldtotpi` (263) shrinks with loop position. Function: `Monthly_Effort_Schedule()`. | Reset oldtotpi per subfleet/month, or keep a persistent per-(nf,ns) store if annual memory is intended | [ ] |
| QUOT-19 | 282 | integer truncation | M/M | `choice_buffer = (int)(bm->SUBFLEET_ECONprms[nf][ns][choicebuffer_id]);` truncates fractional buffers (the comment describes partial reduction) to 0, so the floor is always 1 for any value below 1. Function: `Monthly_Effort_Schedule()`. | Remove the `(int)` cast | [ ] |
| QUOT-20 | 257 | division by zero | M/L | `totppi *= (oldtotpi / (TripCost * ExpEffort));` has no small_num guard, unlike line 249. Function: `Monthly_Effort_Schedule()`. | Add `+ small_num` | [ ] |
| QUOT-21 | 561, 679-685 | accumulator not reset | H/M | `totDIS = 0, totVPUE = 0` are initialised only at declaration. Only `totCPUE` is reset per subfleet (679), so totDIS/totVPUE grow across subfleets and fisheries and the `UseVPUE` (711) and `SpatialBycatchAvoid` (719) weights are wrong for every subfleet after the first. Function: `Multi_Plan_Effort_Final_Allocation()`. | Reset `totDIS = totVPUE = 0` next to `totCPUE = 0` | [ ] |
| QUOT-22 | 824, 853-878 | inconsistent accumulation | M/M | `new_today_effort` always gets the unscaled subfleet effort (824). In the cap branch `today_effort +=` the scaled effort, but the else branch sets `today_effort = new_today_effort`, which brings back earlier capped subfleets' unscaled effort. The cap scale `permisible_left/this_effort` also ignores the `*7` weekly factor used at 843, so it allows 7x the permitted effort. Function: `Multi_Plan_Effort_Final_Allocation()`. | Accumulate per subfleet consistently (`today_effort += scaled or unscaled`); divide by `7*this_effort` | [ ] |
| QUOT-23 | 1048-1052, 1365 | reset inside wrong loop | H/H | `bm->Effort_hdistrib[ij][nf][today_effort] = 0;` sits inside the per-subfleet loop, but today_effort is fishery-level and accumulated with `+=` at 1365. Each subfleet wipes the previous subfleets' effort, so only the last active subfleet's effort is applied. Function: `Holland_Effort_Final_Allocation()`. | Zero today_effort once per fishery, before the ns loop (as at 974-978) | [ ] |
| QUOT-24 | 1033-1035, 1146 | accumulator not reset | M/M | `totland[sp][regid] += catch_here` accumulates across subfleets, fisheries and every trip day, and is reset only when BiM changes. It is then used as one subfleet's expected landings (1170/1186/1200) and as the totland_overall normaliser, so it inflates over the bimonth. Function: `Holland_Effort_Final_Allocation()`. | Reset totland per subfleet trip calculation | [ ] |
| QUOT-25 | 1070-1076 | unit mismatch / repeated accumulation | M/M | `diff_catch` (kg) is compared with `catch_allowed` (mg N; see 1475 conversion). Also `leased_id += diff_catch` re-adds the whole overage on every trip-day call, so leased quota grows without bound. Function: `Holland_Effort_Final_Allocation()`. | Compare with `catch_allowed*X_CN*mg_2_kg`; set leased to `max(leased, diff_catch)` rather than adding | [ ] |
| QUOT-26 | 1392-1395, 1440-1478 | overwritten in loop | H/M | `bm->SP_FISHERYprms[sp][nf][catch_allowed] = ...` is assigned inside `for(regid...)`, but catch_allowed is not region-indexed, so only the last region's value survives (wrong whenever K_num_active_reg>1). Function: `Holland_Effort_Final_Allocation()`. | Sum over regions, or store per region | [ ] |
| QUOT-27 | 1458 | wrong variable (log) | H/L | Debug fprintf prints `ns` (stale; equals nsubfleets after the loop) instead of `nss`. Function: `Holland_Effort_Final_Allocation()`. | Use `nss` | [ ] |


#### ateconomic/ateffortquota.c


No issues found. Functions checked: Total_Effort_Quota_Price, Effort_Quota_trade (body entirely commented out; it repeats the QUOT-05/06/07/08/09 patterns, so fix those too if it is ever re-enabled).


#### ateconomic/ateconts.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| QUOT-28 | 84-128 | partial initialisation | M/L | It allocates `nEconts` FisheryTimeSeries but reads only index `i=0`; entries 1..n-1 stay uninitialised. A negative nEconts is not rejected before malloc. It is latent because the only caller (ateconomicsetup.c:503) uses `tsGDP[0]`. Function: `Read_Econ_Time_Series()`. | Loop i over nEconts (or allocate 1) and quit if `nEconts < 1` | [ ] |


#### ateconomic/ateconomicsetup.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| ECON-01 | 349-351 | wrong variable / out-of-bounds | H/H | `bm->SpatialDisPUE[nf][ns][mth][b] += bm->SpatialCPUE[nf][ns][mth][b] * avg_loadDetFC;` sits after the `sp` loop, so `mth` is whatever the earlier `for (mth=0; mth<12; ...)` loops left: 12 (one past the end of the 12-month dimension, allocated at ateconomicsetup.c:968/970 as `[fish][sub][12][nbox]`). If no species is fished, `mth` is uninitialised on the first pass. Starting discards for months 0-11 are never set. Function: `Economic_Init()`. | Wrap it in `for (mth = 0; mth < 12; mth++) { for (b...) ... }` | [ ] |
| ECON-02 | 789-790, 974 | swapped dims (alloc vs use) | H/L | `bm->EffortTrades = Util_Alloc_Init_3D_Int(bm->K_num_fisheries, 12, 2, 0)` gives `[2][12][K_num_fisheries]`, but it is indexed `EffortTrades[nf][i][source_id]` (fishery first). With more than 2 fisheries (or only 1) that writes past the end. Latent: the only use is in `InitialiseEconArrays()`, which nothing calls (grep finds no caller). Function: `Pre_Load_Array_Alloc()` / `InitialiseEconArrays()`. | Allocate `Util_Alloc_Init_3D_Int(2, 12, bm->K_num_fisheries, 0)` | [ ] |
| ECON-03 | 1231-1316 | buffer overrun (input-driven) | M/M | `columnIndexValues` is `[numExpectedTokens][2]` (1232), but `count++` writes `columnIndexValues[count]` in the header loop before the `count != numExpectedTokens` check at 1274. Each data row likewise reads `columnIndexValues[rowIndex]` before the 1315 check. A CSV with extra columns therefore overruns the heap. Also, only the last character is parsed as the subfleet index (1259/1288), so subfleet 10 or higher is misread, and `index` is never bounds-checked before `friendship_netwk[..][..][index][..]` (1310). Function: `Read_FriendShip_Network()`. | Check `count < numExpectedTokens` / `rowIndex < numExpectedTokens` inside the loops and bound-check `index < K_max_num_subfleet` | [ ] |
| ECON-04 | 1043, 1067, 1041 | copy-paste / truncation | M/L | `EconIndicatorInputNames[down_time_id]` is set twice ("DownTime" then "minDownTime"). "triplength(days)" (16 chars) is cut off by `paramLen = 15`. The array is never read (only freed), so this is cosmetic. Function: `Init_Econ_Indicator_Names()`. | Remove the duplicate and enlarge paramLen | [ ] |
| ECON-05 | 1143 | buffer truncation | H/L | "AnnualExpectedProfitInd" (23 chars) goes into a 20-byte name buffer (atlantismain.c:2101), so the output header is truncated. Function: `Init_Econ_Index_Names()`. | Shorten the label or widen the buffer | [ ] |
| ECON-06 | 137 | NULL deref | M/L | `*(strstr(convertedXMLFileName, ".prm")) = '\0';` crashes if the econ file name has no ".prm". Function: `Economic_Init()`. | Check the strstr result and quit with a message | [ ] |


#### ateconomic/ateconresponse.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| ECON-07 | 488, 609-619 | accumulator never reset | H/H | `boats_new[nf][ns] += num_new;` is never zeroed: grep finds no reset anywhere, and the initialise block at 222-238 resets only `boats_free`. Every monthly call re-adds the whole accumulated `boats_new` to `nboat_id`/`nvessel_id`/`newboat_id` (616-619), so new entrants are counted again every month. Function: `Update_Vessel_Numbers()`. | Zero `boats_new[nf][ns]` in the initialise loop, or after it is applied at 619 | [ ] |
| ECON-08 | 530-549 | accumulator / stale variable | H/M | In the `flagsupp_allowed > 1` branch, `boats_switching` is overwritten for each destination subfleet and each value is added to that destination's `nboat_id`. Only the last value is then subtracted from `boats_free` (546), and the inflated remainder is added back to the origin (549), so boats are created. `boats_switching` is also never reset per (nf,ns), so a stale value is subtracted when no destination qualifies. Function: `Update_Vessel_Numbers()`. | Accumulate into a per-origin total (reset to 0 for each ns) and subtract that total | [ ] |
| ECON-09 | 530, 532 | stale variable (wrong subfleet) | M/M | `supp_cost` and `maxp_flexgear` used here are left over from the last subfleet handled in the earlier leaving loop (326/356), possibly a different fishery, not origin (nf,ns). Function: `Update_Vessel_Numbers()`. | Recompute `supp_cost = SUBFLEET_ECONprms[nf][ns][supp_cost_id]` and store maxp_flexgear per subfleet | [ ] |
| ECON-10 | 370, 379, 445-473 | logic | M/M | `boats_free[nf][ns] += num_moving` is recorded before `num_moving` is reduced by the fleet_thresh, boat-bound and `hist_only` checks (445-470). Those boats are still moved to other subfleets (534/574) but are not removed from the origin, which creates boats (e.g. with `hist_only` set, num_moving goes to 0 but boats_free stays at 1). Function: `Update_Vessel_Numbers()`. | Cap `boats_free[nf][ns]` at the final num_moving after line 470 | [ ] |
| ECON-11 | 801-805 | always-false logic | H/M | Inside `if ((owned_quota + tradeable_quota[sp]) > totTACnew)`, `tradeable_quota[sp] = totTACnew - (owned_quota + tradeable_quota[sp])` is always negative and so always reset to 0. The branch can only zero tradeable quota. Function: `Annual_Quota_Allocation()`. | Probably `tradeable_quota[sp] = totTACnew - owned_quota` (then clamp at 0) | [ ] |
| ECON-12 | 963 | wrong index | H/M | `nreg = bm->regID[ij];` uses the layer index `ij`, but `bm->regID` is per box ("ID for each box", atlantisboxmodel.h:5297). RegCatch then books catch to the wrong region when there is more than one region. Function: `Allocate_Catch()`. | `nreg = bm->regID[b];` | [ ] |
| ECON-13 | 919, 983, 1050-1053 | accumulator not reset | H/M | `totval` is zeroed once, before the fishery loop, then accumulated over all fisheries, subfleets and boxes. `newVpue = totval / effort[b]` therefore uses a running total, not this box's or subfleet's value, so SpatialVPUE (used for deemed-value behaviour) inflates. Function: `Allocate_Catch()`. | Reset `totval = 0` per box (next to `totcatch = 0` at 938) | [ ] |
| ECON-14 | 935, 995, 1046 | accumulator not reset | H/M | `totdiscard` is reset per subfleet but accumulated across boxes, yet `newDISpue = totdiscard / effort[b]` is computed per box, so later boxes get the cumulative discards of earlier boxes. Unlike catch, it is also not scaled by the subfleet's `SpatialBlackBook` share. Function: `Allocate_Catch()`. | Use a per-box discard accumulator (reset at 938) for DisPUE, scaled by the subfleet share | [ ] |
| ECON-15 | 1078 | wrong id constant / slot collision | H/M | `bm->SUBFLEET_ECONprms[nf][ns][within_id] = 1`: `within_id` (=15) is a QuotaAlloc (per-species) index. In the SUBFLEET_PARAMS enum, 15 is `res_cost_ind_id`, which ateconindicator.c:533 overwrites with `reswork`. atquota.c:1172 then reads this "within" flag, so it is corrupted by the indicator value and vice versa (also set at atEconomicAnnual.c:241 and atquota.c:1955). Function: `Allocate_Catch()`. | Add a dedicated `sf_within_id` to SUBFLEET_PARAMS and use it at all 5 sites | [ ] |
| ECON-16 | 1142 | division by zero | M/M | `cpueval += BlackBook[..][expect_id] / bm->EffortSchedule[nf][ns][bm->lastMofY][hist_id];` has no `small_num`, and the guard at 1113 only checks `expect_id > 0`, so hist effort can be 0 and give Inf/NaN in ExpectCPUE. Function: `Allocate_Catch()`. | Add `+ small_num` to the denominator | [ ] |
| ECON-17 | 1259 | unit conversion wrong direction | H/H | With deemed values on, "unlimited" allowed catch is set as `totallowedcatch = no_quota * bm->X_CN * mg_2_kg`. `catch_allowed` is in mg N (1238 converts kg to mg N with `kg_2_mg / X_CN`; same as ateconomicsetup.c:249), so this shrinks 1e12 to about 1.1e8 mg N (about 13 t wet), a binding cap instead of no limit. Function: `Consolidate_Allowed_Catches()`. | `totallowedcatch = no_quota;` (or `no_quota * kg_2_mg / bm->X_CN`) | [ ] |


#### ateconomic/atEconomicAnnual.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| ECON-18 | 496-515 | accumulator in wrong loop | H/H | `tot_expecteffort += bm->EffortSchedule[nf][ns][month][hist_id];` is inside the species loop, so annual effort is multiplied by the number of catchable species. `potential_totppi = totppi - TripCost*tot_expecteffort` (521, 541, 543) then overstates costs about nsp-fold. Compare `Calculate_Projected_Income()` (295/317), which subtracts `TripCost*ExpEffort` once per month. Function: `Dynamic_Effort_Allocation()`. | Move the `tot_expecteffort` line out of the sp loop (directly inside the month loop) | [ ] |
| ECON-19 | 350 | accumulator never reset | H/M | `QuotaAlloc[..][spexpectvalue_id] +=` every year, but only `spexpectcatch_id` is reset (Initialise_Annual_Effort 249; grep shows no reset for spexpectvalue_id). Expected value grows without bound, which skews targeting when `UseVPUE` is set (394). Function: `Calculate_Projected_Income()`. | Reset `spexpectvalue_id` next to `spexpectcatch_id` at line 249 | [ ] |
| ECON-20 | 1012 | wrong index | H/M | `expectedCatch += EffortSchedule[month][expect] * BlackBook[month][expect] / (EffortSchedule[nf][ns][bm->MofY][hist_id] + small_num)` uses the current month instead of the loop `month`. The same formula at 583 and 817-818 uses `month`, and the comment at 818 says MofY was a past mistake. Function: `Calculate_Box_Allocation()`. | Replace `bm->MofY` with `month` | [ ] |
| ECON-21 | 378-381, 398-407 | rank vs species index | M/M | `maxTargetCatch` and `tempTarget` are indexed by rank `nt`, but the reset loop only clears positions whose species index `sp` is fished. Rank slots at non-fished species indices keep values from the previous subfleet or fishery, which distorts the insertion sort and can make a species from another fleet a target. Function: `Identify_Target_Species()`. | Reset all K_num_tot_sp entries unconditionally | [ ] |
| ECON-22 | 383-419 | logic / parameter overwritten | M/M | `ntarg` starts at 1 and grows with every insertion, so every species with positive expected catch becomes a target. The `ntargets` parameter (read at 288) never limits it, and `FISHERYprms[nf][ntargets_id]` is overwritten with the growing count, which changes `ntarg` for later subfleets and years (`Get_Expected_Profit` divides TripCost by it). Function: `Identify_Target_Species()`. | Keep the parameter value and cap insertion and ntarg at it | [ ] |
| ECON-23 | 427, 431, 444-447 | sentinel collision | M/M | `SUBFISHERYtarget[nf][ns][newtarg] = nt;` stores the rank, with 0 for the top target, but 0 is also the "not a target" value (447). So the top target is logged as "switching to target" every year (427) and is never stripped (444 `> 0`). ateconeffort.c:185 also gives non-targets the same top-target weight. Function: `Identify_Target_Species()`. | Store `nt + 1` (and use `nt-1` at ateconeffort.c:185), or use -1 for non-target | [ ] |
| ECON-24 | 1047-1048 | unit mismatch | M/M | `QuotaAlloc[..][permolease_id] *= CatchReScale[sp]` and the same for `templease_id`: `permolease_id`/`templease_id` are used as proportions (`1.0 - prop_olease - prop_tlease`, lines 599-601, ateconresponse.c:1065-1067), so scaling them by a catch factor can push them above 1. The tonnage `leased_id` is not rescaled. Function: `Do_EconArray_Rescaling()`. | Rescale `leased_id` instead of the proportions | [ ] |
| ECON-25 | 541 | division by zero | M/M | `totppi *= (oldtotpi / (TripCost * tot_expecteffort));` has no `small_num` (unlike 534/538). If TripCost or expected effort is 0 and totppi < 0, this gives Inf/NaN. Function: `Dynamic_Effort_Allocation()`. | Add `+ small_num` | [ ] |
| ECON-26 | 247 | accumulator never reset | M/L | `SUBFLEET_ECONprms[..][AnnualCatch_id] +=` every year and is never zeroed (grep), so the "AnnCatch" output is cumulative over the whole run, not annual. Function: `Initialise_Annual_Effort()`. | Zero AnnualCatch_id before the species loop | [ ] |
| ECON-27 | 740 | log text | H/L | The message says "100th iteration" but the bail-out is at `itc > 1000`. Function: `Stocastic_Effort_Allocation()`. | Fix the text | [ ] |


#### ateconomic/atEconomicUtil.c


No issues found. Functions checked: Get_Deemed_Value_Scale, Get_Expected_Profit.


#### ateconomic/ateconhelp.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| ECON-28 | 51, 156-163 | off-by-one (1-based NR port) | H/L | `istack = i_alloc1d(NSTACK)` gives indices 0..49, but this follows Numerical Recipes' 1-based stack: `jstack += 2; if (jstack > NSTACK) quit` lets `jstack == 50` through, and `istack[jstack]` then writes `istack[50]`. Function: `Quicksort_Dir()`. | Allocate `NSTACK + 1` or test `jstack >= NSTACK` | [ ] |
| ECON-29 | 161 | typo vs reference algorithm | H/L | `if (ir - 1 + i >= j - l)`: NR has `ir - i + 1`. The wrong test picks the wrong subarray to push, so the stack can grow linearly and hit the NSTACK quit or ECON-28. The sort order itself is still correct. Function: `Quicksort_Dir()`. | `if (ir - i + 1 >= j - l)` | [ ] |


#### ateconomic/ateconomic.c


No issues found. Functions checked: Economic_Annual, Economics.


#### ateconomic/ateconio.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| ECIO-01 | 169-170 | copy-paste (message) | H/L | `quit("Init_Econ_Meas_File: Can't open %s\n",fname)` sits inside `Init_Econ_Ind_File()` (EconSubIndx.txt), so the error names the wrong function/file. Function: `Init_Econ_Ind_File()`. | Change text to `Init_Econ_Ind_File`. | [ ] |
| ECIO-02 | 313-341 | output ambiguity / copy-paste | M/L | `Write_Real_Effort_Schedule()` writes a block of `EffortSchedule[..][expect_id]` rows (a copy of `Write_Effort_Schedule()`) and then a block of `[current_id]` rows. Both blocks have the same key columns (Time Fishery Subfleet) and the header (l.276) has no column saying which is which, so the expected and realised rows can't be told apart in RealEffortSchedIndx.txt. The expected values are already written to EffortSchedIndx.txt. Function: `Write_Real_Effort_Schedule()`. | Drop the first (expect_id) block, or add a "Type" column. | [ ] |
| ECIO-03 | 195, 615, 690, 952, 1011 vs 290, 315, 330 | inconsistent loop bound | L/L | The effort-schedule writers loop `ns < max(1, nsubfleets)`, but the econ-index, price, deemed-value, catch-per-month and expected-catch writers loop `ns < nsubfleets`. A fishery with 0 subfleets therefore gets rows in some output files and not in others. Functions: `Write_Econ_Ind()`, `Write_Prices()`, `Write_Deemed_Values()`, `Write_Temporal_Catches()`, `Write_Expected_CatchPerMonth()`. | Use one convention everywhere. | [ ] |
| ECIO-04 | 119-132, 906-907, 1039-1040 | dangling static pointer | L/L | `Close_Econ_Indx_Files()` closes every static FILE* but does not set it to NULL. `Economic_Output_Temporal_Catches()` and `Economic_Output_ExpectedCatchPerMonth()` lazily reopen only `if (!fp)`, so any write after the close would use a freed FILE*. Latent: depends on call order. Functions: `Close_Econ_Indx_Files()`. | Set each pointer to NULL after closing. | [ ] |


#### ateconomic/ateconindicator.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| ECIO-05 | 625 vs 284 | unit mismatch | M/M | `Expected_Indicators()` computes `fuel_cost = (...) * fuel_cost_scalar;`. The same formula in `Dynamic_Econ_Indicators()` (l.284) divides by 100.0, with the comment "converting cents to $". So in the expected-indicator path, fuel cost (and therefore `cost_ind_id`, `cost_ton_ind_id` and `boat_cash_ind_id`) comes out 100x too large. Function: `Expected_Indicators()`. | Add `/ 100.0`, matching l.284. | [ ] |
| ECIO-06 | 553-555 | side effect on shared state | L/L | `if (CurrentCatch_id < 1.0) SUBFLEET_ECONprms[nf][ns][CurrentCatch_id] = 1.0;` writes a floor into the real state variable instead of a local copy. Anything that reads CurrentCatch later in the month (before the reset at ateconomic.c:229) sees an invented 1 t catch. Function: `Dynamic_Econ_Indicators()`. | Use a local `catch_floor = max(1.0, CurrentCatch)` for the division. | [ ] |
| ECIO-07 | 764, 778, 789-792 | mismatched accumulation period | L/M | `RevPerT_id = econindx[nf][SubfleetCash_id] / totcatch`. SubfleetCash is cumulative (reset only `if (bm->dayt < 1.0)` and at ateconomic.c:73), while `totcatch` is rebuilt every call from `cumboatcatch_id`. If the two cover different periods, revenue per tonne is biased. The same applies to `RevPerEffort_id`, which uses `CumEffort`. Function: `Update_Econ_Indx()`. | Make sure numerator and denominator cover the same period. | [ ] |


#### ateconomic/ateconParamIO.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| ECIO-08 | 733-736 | missing subfleet fill | M/M | `bm->ECONexprice[fisheryIndex][0][sp][i][hist_id] = values[i];` fills subfleet 0 only. ateconomicsetup.c:392 then copies `[nf][ns]..[hist_id]` into `expect_id` for every ns (the copy-from-subfleet-0 line at :391 is commented out). ateconindicator.c:438 and ateconeffort.c:1031/1114 read `[nf][ns]..[hist_id]` for all ns. Result: every subfleet with ns>0 sees an ex-vessel price of 0 in the Holland model (zero gvp and zero marginal rent). Function: `Read_Dan_Holland_XML()`. | Loop `ns < nsubfleets` and set `ECONexprice[fisheryIndex][ns][sp][i][hist_id]`. | [ ] |
| ECIO-09 | 678 | wrong cast | H/M | `bm->high_price = (int) Util_XML_Read_Value(..., no_checking, "high_price");`, but `high_price` is a `double` (atlantisboxmodel.h:5870). The cast truncates the fraction, and a value above INT_MAX (plausible for an "arbitrarily high price") is undefined behaviour. Function: `Read_Dan_Holland_XML()`. | Remove the `(int)` cast. | [ ] |
| ECIO-10 | 158-161 vs 345 | duplicate read / validation bypass | M/M | `bm->minValue` is read from Economic_Flags (l.158) and validated (l.160, must be non-zero for the NZ log model). It is then read again from Quota_Reconciliation/LeaseValues (l.345), which silently overwrites it without re-validating, so a 0 there leads to log(0) in the Newell model. Functions: `Read_Economic_Flags()`, `Read_Quota_XML()`. | Read it in one place, or repeat the check after l.345. | [ ] |
| ECIO-11 | 234, 238, 262, 174, 180, 484, 599, 627, 675 | copy-paste (messages) | H/L | Error strings are copied from other blocks. `Read_Market_Value()` reports "Market_Intercept" for Market_Trend and Market_Autocoefft. `Read_Monthly_Market_Value()` reports path "Market_Intercept/…" for MonthlyMarket. Rescale messages say 'ReScale_Effort'/'ReScale_Catch' but the tags are ReScale_Eff/ReScale_Cat. FuelCosts path is given as "SubfleetEffort/" (it lives under SubfleetCosts). The SpatialEffort and SpatialCPUE checks both say "TemporalBlackBook". `Read_Dan_Holland_XML()` says "Read_Management_Costs_XML". | Correct each message. | [ ] |
| ECIO-12 | 751-757 | NULL dereference | M/L | `inputDoc = xmlReadFileDestFolder(...)` is used as `inputDoc->children` without a NULL check, so a missing or malformed econ XML segfaults instead of quitting with a message. Function: `Read_Economic_Parameters()`. | `if (!inputDoc) quit(...)`. | [ ] |


#### ateconomic/atFishMIP_OSP_Bridge.c


(This file IS listed in libateconomic_a_SOURCES (ateconomic/Makefile.am:23), even though its header comment says it is not. It compiles but is not called from anywhere.)

| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| ECIO-13 | 513-520 (and 542, 673, 730) | index packing / overlap | H/H | `catchBySpFishery[k * nfishery + nf]` uses a different `nfishery` for each species. Rows for different k overlap: for example, species k=0 with nfishery=3 uses slots 0-2, and species k=2 with nfishery=1 writes slot 2, overwriting k=0's third fishery. The buffer is allocated `K_num_fisheries*K_num_tot_sp` (l.90). Function: `FishMIP_OSP_Bridge_Annual()`. | Index as `k * bm->K_num_fisheries + nf` everywhere (or `+ this_nf`). | [ ] |
| ECIO-14 | 538-544 | stale variable | H/H | In Step 3, `totalCatch += catchBySpFishery[k * nfishery + nf]` uses `nfishery` left over from the last species of the Step 2 loop, not species k's own value, so `prod[k]` (and the price) is summed from the wrong slots. Function: `FishMIP_OSP_Bridge_Annual()`. | Set `nfishery = g_catch_n_effort_History[sp].nfishery;` inside the loop (plus the ECIO-13 fix). | [ ] |
| ECIO-15 | 669-691 | stale variable / accumulator not reset / wrong index | H/H | The Step 4 per-fishery loop has three faults. (a) `sp` is stale (the last species from the earlier loops), so `this_nf = g_catch_n_effort_History[sp].targetFishery[nf]` maps every fishery through one species' target list. For nf >= that species' nfishery the entries are 0, so q/compliance/effort all come from fishery 0. (b) `totalCatch`, `revenue` and `ftargetWeighted` are never reset per fishery; `totalCatch` even carries over from Step 3. Y_arr, price_arr and Ftarget_arr are therefore running totals over fisheries. (c) `c = catchBySpFishery[k*nfishery+nf]` reads with nf running up to K_num_fisheries, crossing into other species' rows. Function: `FishMIP_OSP_Bridge_Annual()`. | Reset the three accumulators at the top of each nf iteration, use `this_nf = nf`, and index catch as `[k*K_num_fisheries + nf]`. | [ ] |
| ECIO-16 | 728-736 | stale variable | H/H | In Mode C, `c = catchBySpFishery[k * nfishery + nf]` again uses the stale `nfishery` from Step 2 rather than species sp's value, so the catch weights for origprice are taken from the wrong slots. Function: `FishMIP_OSP_Bridge_Annual()`. | Use the per-species nfishery (with the ECIO-13 indexing fix). | [ ] |
| ECIO-17 | 480, 576-578 | type truncation | H/H | `gp` is declared `int` (l.480), but `FishMIP_Adapter_GetGlobalPriceForYear()` returns `double` (adapter .h:112). External prices are truncated to whole units, and any price below 1.0 becomes 0, which still passes `gp >= 0.0` and is written as the species price. Function: `FishMIP_OSP_Bridge_Annual()`. | Declare `double gp;`. | [ ] |
| ECIO-18 | 134-142, 387 | uninitialised variable | H/M | `g_catch_n_effort_History` is `malloc`'d and `firstcall` is never set, so `if (g_catch_n_effort_History[sp].firstcall > 0)` reads garbage. Either the back-fill never runs (past years stay 0 and the assessment sees an effort/catch collapse) or it runs unpredictably. Functions: `Create_OSP_Arrays()`, `RunPIMAssessment()`. | Set `firstcall = 1` in Create_OSP_Arrays (or use calloc). | [ ] |
| ECIO-19 | 400, 410, 435-448, 518, 602 | unit mismatch | M/H | Catch comes from `Harvest_Get_TotCumCatch()` (TotCumCatch, in mg N: ateconindicator.c:454-456 compares it to `TACamt*kg_2_mg/X_CN`). That catch feeds the PIM assessment, and the resulting `MSY` is written straight into `bm->TACamt[sp][nf][now_id]` and `SP_FISHERYprms[..][TAC_id]`, which are in tonnes/kg. The mg N catch is also used as "prod" for the price model and for revenue weighting. Functions: `RunPIMAssessment()`, `FeedPIMAssessmentIntoAtlantisTAC()`, `FishMIP_OSP_Bridge_Annual()`. | Convert catch with `* X_CN * mg_2_tonne` (and back) at the bridge boundary. | [ ] |
| ECIO-20 | 321-331 | defaults overwritten with 0 | M/M | `ApplyLoadedDrivers()` guards catchability with `> 0.0` but copies `compliance`, `invest_driver_U` and `invest_driver_V` unconditionally. For a reference country with no EEZ, the adapter returns `ok=1` with these fields left at 0 (adapter .cpp:158-165). Compliance then becomes 0 and investment drivers 0 for every fishery. Function: `ApplyLoadedDrivers()`. | Only overwrite when the driver set has EEZ data (expose hasEezPos) or the value is > 0. | [ ] |
| ECIO-21 | 370-416, 651-656 | empty-dimension call | M/L | `RunPIMAssessment()` runs for every fished species even when `nfishery == 0` (no fishery targets it per `CollectFleets()`), so `fit_one_stock` gets a [nyears x 0] C/E matrix. Function: `RunPIMAssessment()`. | `if (nfishery == 0) return 0;`. | [ ] |
| ECIO-22 | 148-187 | memory leak / dead code | L/L | `FreeOSPStruct()` is static and never called, and no public free exists, so all OSP arrays leak; it will also trigger an unused-function warning. The unused `done` parameter suggests it was meant to be wired up. Function: `FreeOSPStruct()`. | Expose it (e.g. `Free_OSP_Arrays`) and call it from Economic_Free. | [ ] |


#### ateconomic/atFishMIP_OSP_PluginAdapter.cpp


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| ECIO-23 | 221-286 | exception across C ABI | M/M | `FishMIP_Adapter_CalcPrice()`, `FishMIP_Adapter_CalcEffort()` and `FishMIP_Adapter_RunAssessment()` are `extern "C"` and called from C, but unlike the two loaders (l.100-128, 174-191) they have no try/catch. Any std::exception from the plug-in (bad_alloc, runtime_error, NdArray bounds) unwinding into C frames ends in std::terminate. | Wrap each body in try/catch and return a failure flag / leave outputs unchanged. | [ ] |
| ECIO-24 | 231 | unchecked buffer copy | L/H | `std::copy(pr.price_ctry.begin(), pr.price_ctry.end(), price_out)` assumes `price_ctry.size() == nsp`, but `price_out` is caller-allocated (bridge: K_num_tot_sp). If the plug-in returns a longer vector it overflows; a shorter one leaves stale values. Function: `FishMIP_Adapter_CalcPrice()`. | Copy `min(size, nsp)` elements. | [ ] |

#### ateconomic/include/ (prototypes vs definitions)

| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| ECIO-25 | atFishMIP_OSP_Bridge.h:170 | definition in header | M/H | `int g_effortHistoryCount;` is a (tentative) definition, not an `extern` declaration, in a header that every ateconomic TU includes via atEconomic.h:37, as does atlantismain.c:137. Under `-fno-common`, which is the default in GCC>=10 and Clang>=11 (no `-fcommon` in PreRules.am/configure.ac), this gives duplicate-symbol link errors. It is also never used. | Delete it, or make it `extern` and define it once. | [ ] |
| ECIO-26 | atEconomic.h:111; atlantisboxmodel.h:6255 | duplicate definition (latent) | L/L | `Quicksort_Dir` is defined in both ateconomic/ateconhelp.c:42 and atlantisUtil/atUtilhelp.c:43. ateconhelp.c is not in libateconomic_a_SOURCES at the moment, so it does not link today, but adding it back would collide. | Delete ateconhelp.c's copy. | [ ] |
| ECIO-27 | atEconomicLib.h; ateconomic.c:35; ateconio.c:1003 | missing prototypes | L/L | `Economic_Output_ExpectedCatchPerMonth()` is public but declared ad hoc in ateconomic.c:35 instead of in atEconomicLib.h (next to the other Economic_Output_*). `Write_Expected_CatchPerMonth()` is non-static and has no prototype anywhere, unlike every other Write_* in ateconio.c. All other prototypes in atEconomic.h, atEconomicLib.h, atFishMIP_OSP_Bridge.h and atFishMIP_OSP_PluginAdapter.h match their definitions in signature. | Add the prototype to atEconomicLib.h and make Write_Expected_CatchPerMonth static. | [ ] |


### B.2.3 atharvest/

#### atharvest/atHarvest.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| HARV-01 | 308-310 | wrong loop direction | H/M | `for (d = 0; d < K-1; d++) CatchQueue[..][d + 1] = CatchQueue[..][d];` is a forward shift, so `CatchQueue[0]` gets copied into every slot (the queue collapses to `[new, old0, old0, ...]`). After that, the running `LastCatch` total (subtract slot K-1, add slot 0) no longer matches the queue contents, so the CPUE used in atmanage/atManage.c:291 is wrong whenever `K_num_catchqueue > 2`. Function: `Harvest_Update_Temp_Catch_Array()`. | Shift from the top down: `for (d = K-1; d > 0; d--) Q[d] = Q[d-1];` | [ ] |
| HARV-02 | 625 | wrong variable | L/L | `bm->Discards[...] += bm->FishingResults[nf][Discards_result_id];` reads the global array, not the `FishingRes` argument the function fills in at line 595. This only works because the single caller (atecology/atprocess.c:1982) passes `bm->FishingResults`. Function: `Harvest_Do_Fishing_And_ByCatch()`. | Use `FishingRes[nf][Discards_result_id]`. | [ ] |
| HARV-03 | 780-792 | copy-paste / inconsistent table | M/M | In the branch where both use reef (`check1`), the fishery uses flats but the group does not (`FC_likeFLAT`), and both use soft (`check3`), the group's habitat (reef+soft) is fully covered by the fishery. Every other "fully covered" case in the table gives 1.0, e.g. lines 825-833 (same group, fishery without flats) and 1124-1134. This case gives `coveramt[REEF]+coveramt[SOFT]` instead, so adding a habitat the group does not use lowers the overlap. Function: `Habitat_Overlap()`. | Set the four sub-cases to the 1.0 / `1.0 - canyon` pattern used at lines 826-833. | [ ] |
| HARV-04 | 1503 | wrong parameter id | H/M | `Get_Fishery_Change_Scale(bm, nf, P_num_changes_id, P_num_changes_id, Pchange)` passes the number of changes as the flag parameter, so the coverage change is applied whenever `P_num_changes > 0`, even when the fishery's `flagchangeP` (`flagchangeP_id`, read at atHarvestParamIO.c:593) is 0. Compare line 773 of atHarvestCatch.c, which passes `flagchangeSWEPT_id, SWEPT_num_changes_id`. Function: `Harvest_Update_Habitat_Overlap()`. | Pass `flagchangeP_id, P_num_changes_id`. | [ ] |
| HARV-05 | 1932, 1940-1949 | logic / data loss | H/M | (a) `Qchange[species][fishery] == 0` compares a pointer to 0, which is never true, so the "overwrite single dummy" shortcut never runs. (b) The new array `tempArray` is zero-initialised, but `Util_Copy_Change_Values` copies only the `[species][fishery]` row, and then the old `Qchange` is freed. Every other species/fishery catchability-change schedule is wiped to zero. Called from atCLAMLink/atCLAMImplement.c:40. Function: `Harvest_Scale_Q()`. | Test `Qchange[species][fishery][0][start_id] == 0`, and copy all other `[sp][nf][i][*]` entries into `tempArray` before the swap. | [ ] |
| HARV-06 | 1981 | wrong variable (copy-paste) | H/L | `tempArray = Util_Alloc_Init_4D_Double(3, Q_max_num_changes, ...)` is the mFC copy of `Harvest_Scale_Q`, but it sizes by `Q_max_num_changes`, not `mFC_max_num_changes` (just updated at line 1969). If mFC changes outnumber Q changes, `Util_Copy_Change_Values` writes past the end. It has the same pointer-compare (line 1973) and row-only-copy problems as HARV-05. No callers found at the moment, so latent. Function: `Harvest_Scale_mFC()`. | Use `mFC_max_num_changes`, and apply the HARV-05 fixes. | [ ] |
| HARV-07 | 1732 | wrong error text | L/L | The quit message says "value must be between 0 and 6", but there are 9 curve ids (up to `q_binormal_id`). The same stale text is at atHarvestCatch.c:759, and atHarvestCatch.c:631 says "between 0 and 1" for 3 habitat options. Function: `Get_Selectivity()`. | Update the message ranges. | [ ] |


#### atharvest/atHarvestParamIO.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| HARV-08 | 841-845, 860-866, 874-880, 887-893, 905-910 | buffer overrun | M/H | The change arrays are allocated with max size 1 unless the global flag (`flagchangesel`, `flagchangep`, `flagchangeswept`, `flagchangediscard`, `flagchangeq`) is set. The readers (`readSelChangeXML` line 394, `Util_XML_Read_Change_Values`, `readDiscardChangeXML` line 438, `Util_XML_Read_Change_Fished_Groups`) still write `num_changes` entries for each fishery/group. If any `*_num_changes > 1` while the global flag is 0, this is a heap overflow. The fix-up that turns on global flags from per-fishery flags (atHarvestSetup.c:111-129) runs after `Read_Harvest_Parameters` (atHarvestSetup.c:94), so it is too late. Function: `readFisheryChangeXML()`. | Always size by `Manage_Get_Max_*_Param(...) + 1`, as is already done for mFC at line 917. | [ ] |
| HARV-09 | 696-699 | copy-paste duplicate | M/L | `Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, id);` is called twice (lines 696 and 698), so mFC is read twice. Harmless, but probably meant to read something else. Function: `readFishingMortalityXML()`. | Remove the second call. | [ ] |
| HARV-10 | 583-585, 831-835, 933-935 | NULL dereference | M/L | Unlike every other reader in this file, `attributeGroupNode` (`Fishing_Scenario_Flags`), `groupingNode`/`selGroupNode` (`Fishery_Change` and its children) and `inputDoc` (from `xmlReadFileDestFolder`) are not NULL-checked. A missing XML section or file crashes instead of giving a clear `quit`. Functions: `readFisheryScenarioFlags()`, `readFisheryChangeXML()`, `Read_Harvest_Parameters()`. | Add `if (x == NULL) quit(...)`. | [ ] |


#### atharvest/atHarvestCatch.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| HARV-11 | 232 | wrong parameter id | H/H | `Get_Fishery_Group_Change_Scale(bm, nf, sp, bm->flagchangeF, mFC_num_changes_id, mFCchange[sp])` passes the global flag value (1) as the parameter index. It reads `SP_FISHERYprms[sp][nf][1]`, which is `flagQchange_id` (enum in atlantisboxmodel.h:950-952). So forced-F (mFC) change schedules are only applied when the Q-change flag is set for that group/fleet, and the per-group `flagFchange_id` (read at atHarvestParamIO.c:597) is ignored. Function: `Get_Fishing_Mortality()`. | Pass `flagFchange_id` as the paramID. | [ ] |
| HARV-12 | 709-718, 732-743 | copy-paste | M/L | The "baseline" selectivity `sel_temp` should be computed without the change offsets. For `q_bimodal_id`, line 714 adds `addlsm` to `sel_bilsm2` for the baseline second mode. For `q_binormal_id`, lines 737-738 add both `addlsm` and `addsigma` to the baseline. So `sel - sel_temp` understates the change. Only affects invertebrates using bimodal curves with selectivity changes. Function: `Get_addlsm_Scale()`. | Drop `+ addlsm` / `+ addsigma` in the baseline (`sel_temp2`) lines. | [ ] |
| HARV-13 | 829, 835 | cohort vs cohort x genotype | M/M | `Ecology_Get_Size(bm, species, Wgt, basechrt)` passes the age class. `Ecology_Get_Size` indexes `sn[]`, `X_RS[]` and `VERTinfo[][]` by cohort x genotype (atUtilGroupIO.c:771,843), and the main fishing path passes `chrt` (atHarvest.c:444). With `numGeneTypes > 1`, this gives the wrong genotype's length, so selectivity in the output access calculation is wrong. Function: `Harvest_How_Much_Fishery_Access()`. | Pass `cohort`, not `basechrt`, to `Ecology_Get_Size`. | [ ] |


#### atharvest/atHarvestDiscards.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| HRV2-01 | 213, 332, 363 | cohort×genotype confusion | H/M | `Do_Dependent_Discards(..., basechrt, ...)` passes a cohort index that has already been divided by the genotype count (line 71). `Calc_Dependent_Discards_For_Species()` then divides again: `basechrt = chrt / numGeneTypes` (363). When numGeneTypes>1, `DiscardTS_agedistrib[nf][sp][basechrt]` reads the wrong (younger) age class. Functions: `Get_Discards()` -> `Do_Dependent_Discards()` -> `Calc_Dependent_Discards_For_Species()`. | Use `chrt` directly in line 370, or pass the raw `chrt` from Get_Discards | [ ] |
| HRV2-02 | 339-340, 442, 449 | cohort×genotype confusion | H/M | `icohort = floor(bcohort / numGeneTypes)` (339) is already an age class. `Calc_Dependent_Discards_For_Other_Species()` divides it again: `ibasechrt = ichrt / numGeneTypes` (442). With genotypes it indexes the wrong age in `DiscardTS_agedistrib` (alloc [nf][sp][K_num_max_cohort], atlantismain.c:1999). Function: `Calc_Dependent_Discards_For_Other_Species()`. | `ibasechrt = ichrt;` | [ ] |
| HRV2-03 | 176-177, 380-381 | wrong operator / unit (inverted scale) | M/H | When the fishery is always active (`fisheriesflagactive==2`), the discard code sets `active_scale = 86400.0 / bm->dt`. The imposed-catch code uses the inverse, `active_scale = bm->dt / 86400.0` (atHarvestImposedCatch.c:255-256). At 12 h dt the imposed discards (and dependent discards at 381) for always-active fisheries come out 4x larger than the matching catch (2x vs 0.5x, before ts_dtscale). The comment at 374-379 ("if it calls twice then must halve the result") also says the result should be halved, not doubled. Functions: `Get_Discards()`, `Calc_Dependent_Discards_For_Species()`. | Use `bm->dt / 86400.0` to match Get_Loaded_FC | [ ] |
| HRV2-04 | 191-192, 254 | division by zero / NaN | M/H | `loadDetFC = tsDetFC / (tsDetFC + loadFC);` has no small_num guard. When the discard time series is 0 and loadFC is 0 the result is 0/0 = NaN. The later clamp `min(1.0,max(0.0,loadDetFC))` does not remove NaN, because comparisons with NaN are false and the macro returns NaN. The NaN discard fraction is then returned. Function: `Get_Discards()`. | `/ (tsDetFC + loadFC + small_num)` | [ ] |
| HRV2-05 | 654-655, 666-667, 747-748, 762 | unit mismatch | H/M | In the adjacent_impose catch-up, each box is reduced by `step1 * bm->cell_vol * FCtsCarryOver[box] / (biomCarryOver + small_num)`. `biomCarryOver` is already the summed carry-over divided by cell_vol (579-590), so this has an extra factor of cell_vol. The intended total removal, `step1*cell_vol`, is what the global record gets (651). Per-box carry-overs are effectively zeroed every time, so the per-box and global (nbox) records drift apart. The discard version (747, 762) has the same problem. Function: `Get_Imposed_Discards()`. | Per box: `-= step1 * FCtsCarryOver[box] / (biomCarryOver + small_num)` (and the same with step3/disCarryOver) | [ ] |
| HRV2-06 | 227, 490, 597 | copy-paste / log text | H/L | The quit text says "value must be between 0 and 2", but cases up to depend_discard exist. Line 597 says "reset to 0, 1 or 2", but cases 1-4 are valid (atlantisboxmodel.h:1262-1266). The debug line 490 prints the label "loadFC = %e" twice, and the second value is tsDetFC. Functions: `Get_Discards()`, `Get_Imposed_Discards()`. | Fix message text | [ ] |


#### atharvest/atHarvestSetup.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| HRV2-07 | 644-651 | cohort×genotype confusion | M/M | The comparison uses the age class (`basechrt < age50`, and `age50 = basechrt`), but the stored value is the genotype-expanded index: `speciesParams[Age50pcntV_id] = chrt`. Age95 has the same problem. Consumers treat it as an age class (`va50` in atassess/atclassical.c:839,1143,1410). With numGeneTypes>1 the stored age is wrong and can be >= numCohorts. Function: `Calculate_Age_Vulnerability()`. | Store `basechrt` | [ ] |
| HRV2-08 | 78 | NULL dereference | M/L | `*(strstr(convertedXMLFileName, ".prm")) = '\0';` crashes if the fisheries prm filename has no ".prm" extension. Function: `Harvest_Init()`. | Check strstr result and quit with a message | [ ] |
| HRV2-09 | 393-412 | copy-paste / metadata | H/L | Average catch size tracers get the long name "Total effort of (recreational) fishery ...", copied from the effort tracer block. Line 408 even says "This should be different but is not!". Units "mg N" are questionable too. Function: `Build_Fishery_Tracers()`. | Use "Average catch size of fishery %d (%s)" | [ ] |
| HRV2-10 | 172-205 | memory leak | H/L | `Harvest_Free()` frees tsCatch but never `tsDiscard`, and never frees `qSTOCK` (allocated at line 454). Function: `Harvest_Free()`. | Add `Harvest_Free_Time_Series(tsDiscard, ntsDiscard)` and `free3d(qSTOCK)` | [ ] |


#### atharvest/atHarvestIO.c


No issues found. Functions checked: Harvest_Report_Annual_Stats, Harvest_Report_Monthly_Stats, Open_Harvest_Output_Files, Close_Harvest_Output_Files, initAnnCatchFile, initAnnRecCatchFile, initAnnDiscardFile, initAnnCatchPerFisheryFile, initAnnDiscardPerFisheryFile, initAnnEffortFile, initAnnTACFile, initDisplacedEffortFile, writeAnnCatch, writeAnnCatchPerFishery, writeAnnDiscardPerFishery, writeAnnEffort, writeDisplaceEffort. Array orders checked against the allocations: CatchSum [sp][3], TotCumRecCatch/TotCumDiscards [sp][nf], CumEffort [nf][b], CumDisplaceEffort [b][nf].


#### atharvest/atHarvestImposedCatch.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| HRV2-11 | 154-157 vs 82-88 | inconsistency | L/L | Discard species-id matching has no `break`, so the last matching column wins, while catch matching takes the first match. This only matters if a ts file has duplicate column names. Function: `Load_Imposed_Discards()`. | Add `break` | [ ] |

(See HRV2-03 for the active_scale disagreement between this file and atHarvestDiscards.c. This file's `dt/86400` appears to be the correct one.)


#### atharvest/atHarvestTS.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| HRV2-12 | 246 | wrong operator | M/M | The expanding branch is documented as "use original distribution", but it does `CatchTS_agedistrib[...][ij] += CatchTS_agedistribOrig[...][ij] + cumtot;`. That adds the original distribution on top of the current, contracted one, so the old distribution is never restored after normalisation. The depleted branch (241-242) correctly builds from Orig. Function: `Harvest_Recalc_Time_Series_Distrib()`. | Use `=` instead of `+=` | [ ] |
| HRV2-13 | 195-206 | logic | L/M | In the flagkeepZeroCatchTS branch, when `CatchTS_agedistribOrig==0` the class is set to 0 and `cumtot` is reset to 0. The catch share accumulated from the depleted older classes is then thrown away instead of being carried down to the next class with catch. Function: `Harvest_Recalc_Time_Series_Distrib()`. | Only reset cumtot when it was actually assigned | [ ] |
| HRV2-14 | 272 | logging | H/L | `if (do_debug \|\| (CatchTS_agedistrib[fishery_id][sp][0] > 0))` writes to the log for every age class whenever the youngest class has any catch share, on every recalculation. This looks like a debug leftover that bloats the log. Function: `Harvest_Recalc_Time_Series_Distrib()`. | `if (do_debug)` | [ ] |


#### atharvest/atHarvestIndex.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| HRV2-15 | 225 | wrong index (year) | M/L | `Update_Harvest_Index_Values()` is called from the annual block right after `bm->thisyear++` (atlantismain.c:385-402). At that point `TotCumCatch[sp][nf][bm->thisyear]` is the new, empty year, so `managementCost` is effectively always 0. The just-finished year is thisyear-1, which writeAnnCatchPerFishery (atHarvestIO.c:495) uses correctly. | Use `bm->thisyear-1` (guarded for thisyear>0) | [ ] |


#### atharvest/atHarvestAnnual.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| HRV2-16 | 201 | buffer overrun | M/H | `TotCumCatch[k][n][bm->thisyear+1] = 0.0;`. TotCumCatch is allocated with `max_year = (int)(tstop/yr)+1` years (atHarvestSetup.c:428-442). The main loop runs `while t < tstop` (atlantismain.c:693), and thisyear is incremented at each year start. If tstop is not a whole number of years (e.g. 10.5 yr), the last annual call has thisyear = floor(tstop/yr) = max_year-1, so thisyear+1 = max_year is out of range. This writes into the next fishery's year-0 entry, or past the block end for the last sp/nf. Function: `Harvest_Annual_Reset()`. | Allocate `ceil(tstop/yr)+1` years, or guard `if (thisyear+1 < max_year)` | [ ] |
| HRV2-17 | 195 | wrong index (year) | M/M | `bm->TotOldCumCatch[k][n] += TotCumCatch[k][n][bm->thisyear];` runs after `thisyear++` (atlantismain.c:385), so it adds the new, empty year. The comment says the intent is the year just finished ("Could use TotCumCatch[thisyear-1] instead"). With bulkTAC the multi-year cumulative catch is never accumulated, which breaks bulk-TAC over-quota checks (atManage.c:797, atHarvestDiscards.c:279). Function: `Harvest_Annual_Reset()`. | Use `TotCumCatch[k][n][bm->thisyear-1]` when thisyear>0 | [ ] |
| HRV2-18 | 91-99, 110-112 | unit mismatch (log) | H/L | FCtsCarryOver is already stored multiplied by cell_vol (mg s-1, see atHarvestDiscards.c:503). The per-box report multiplies by `cell_vol` again. Undercatch (92) and underdiscard (96) leave out `dt`, while the alternate underdiscard (99) includes it. The global line (111) applies `dt` only to discards. The reported tonnages are inconsistent and wrong. Function: `Report_Annual_Harvest()`. | Use `* bm->dt * bm->X_CN * mg_2_tonne` (no cell_vol) consistently | [ ] |


#### atharvest/include/*.h


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| HRV2-19 | atHarvest.h:106 | multiple definition | M/L | `int first_data_done;` is a definition, not a declaration, in a header included by every atharvest .c file, so each translation unit gets its own tentative definition. It links only under -fcommon; GCC>=10 and clang>=11 default to -fno-common, which gives a "duplicate symbol" link error. atHarvestImposedCatch.c:39 has the real definition commented out. | `extern int first_data_done;` here, and define it once in atHarvestImposedCatch.c | [ ] |

atHarvestIO.h, atHarvestLib.h, atHarvestPrivate.h: no other issues.


### B.2.4 atimplementation/

#### atimplementation/atImplementationAnnual.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| HRV2-20 | 81, 95-103, 164-168 | logic / wrong condition | H/H | `high_trigger_tripped` starts at 1 and is only cleared when a species falls between the low and high thresholds. (1) When the low trigger trips (95-100), the loop `break`s with high_trigger_tripped still 1, so the reduction computed at 143-147 is always overwritten by the "recovery" formula at 166-167. (2) The recovery block runs even when `flagADJUSTmanage==0` (it is outside that `if`). (3) `EFF_scale = 1.0 - step1*(1+FC_restrict)` with `step1=(dayt+1-target_day)/FC_period`. target_day is reset to 0 at line 159, so step1 ~ dayt/FC_period and EFF_scale goes strongly negative. This value is stored in `effort_scale[..][target_tac_id]` and used as TAC_scale (319) and mgmt_scale (atHarvestImposedCatch.c:317). The endangered block 264-268 has the same pattern. Function: `Apply_Annual_Fisheries_Mgmt()`. | Run the recovery only when `!trigger_tripped && high_trigger_tripped && flagADJUSTmanage>0`, and use a bounded ramp, e.g. `FC_restrict + min(1,step1)*(1-FC_restrict)` | [ ] |
| HRV2-21 | 197-237, 248-257 | misplaced else / wrong branch | H/M | In the endangered-species loop, the "Use assessment model output" `else` (221) belongs to `if (speciesParams[sp_concern_id])`, not to a `!bm->do_assessing` test like the target-species loop (94/109). Non-concern species are judged on assessment output (TAC_trigger) even when no assessment runs, and concern species always use perfect knowledge. Also, `pseudo_survey_trip=1` (220) is only reached for species above the upper threshold. When the trigger trips via the perfect-knowledge `break` (209), pseudo_survey_trip is 0, so line 257 uses `TAC_trigger[..][triggered_scalar_id]` instead of the ramp. Line 228 prints `fishnowpop/startpop`, which may be uninitialised in that branch. Function: `Apply_Annual_Fisheries_Mgmt()`. | Restructure as `if (sp_concern) { if (!bm->do_assessing) {...; pseudo_survey_trip=1;} else {...} }` | [ ] |
| HRV2-22 | 357-366 | dead code / feature not applied | M/M | In the "Disable TAC check" block, `TAC_scale` is computed via Util_Get_Change_Scale but never used. Nothing is assigned to TACamt, so disabling TACs has no effect. Function: `Apply_Annual_Fisheries_Mgmt()`. | Apply the scale (e.g. set `TACamt[sp][fishery_id][now_id]`) or remove | [ ] |
| HRV2-23 | 372-501, 491-498 | accumulator not reset / loop placement | M/M | For non-vertebrate groups, `RegionalData[sp][nreg][reg_tac_id] += stock_struct_prop[..]/n` is never reset first. The whole regional block also sits inside the `fishery_id` loop, so the value grows by about 1 per managed fishery per year and stops being a proportion. The vertebrate branch also recomputes `initreg_prop` for every guild, for every sp and every fishery: wasteful, but the result is correct. Function: `Apply_Annual_Fisheries_Mgmt()`. | Set RegionalData to 0 before the += loop, and move the regional block outside the fishery loop | [ ] |


#### atimplementation/atImplementationSetup.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| HRV2-24 | 24, 30 | fixed buffer | L/L | `convertedXMLFileName[50]` uses a hard-coded name, so this is fine today. Note only: if it is ever built from the fishprm filename (as in Harvest_Init, which uses STRLEN), it will overflow. Function: `Implementation_Init()`. | Use STRLEN | [ ] |


#### atimplementation/atImplementationParamIO.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| HRV2-25 | 31, 35, 38 | wrong function / wrong array -> heap overflow | H/H | `TAC_num_changes_id` is a species-fishery parameter: it is in the SP enum (atlantisboxmodel.h:985), read by Util_XML_Read_Impacted_Group_Param, and used as `SP_FISHERYprms[sp][nf][TAC_num_changes_id]` (atImplementationAnnual.c:338). But `Manage_Get_Max_Fishery_Param()` reads `bm->FISHERYprms[i][paramIndex]` (atManage.c:2618), so the max comes from an unrelated per-fishery parameter. TACchange is then allocated with that change count. `Util_XML_Read_Change_Fished_Groups` writes `array[sp][i][b]` for `b < SP_FISHERYprms[sp][i][TAC_num_changes_id]` (atUtilFisheryXML.c:519-521), which overflows when a species has more TAC changes than that bogus max. Function: `readManagementImplementationChangeXML()`. | `TAC_max_num_changes = Manage_Get_Max_Species_Fishery_Param(bm, TAC_num_changes_id) + 1;` | [ ] |
| HRV2-26 | 50-52 | NULL dereference | M/L | `inputDoc = xmlReadFileDestFolder(...)` is not checked before `inputDoc->children`, so it crashes if the converted XML is missing or malformed. Function: `readModelManageImplParameters()`. | `if (inputDoc == NULL) quit(...)` | [ ] |
| HRV2-27 | 25-27 | copy-paste (node name) | L/L | TAC changes are read from a node called "Fishing_Catchability_Change", and ConvertAtlantis/atImplementationtoXML.c:29,33 labels them "catchability". The reader and writer agree, so it works, but the name and description are copy-paste from the q-change code and misleading. | Rename consistently in both (optional) | [ ] |


#### atimplementation/include/*.h


No issues found. Checked atImplementation.h, atImplementationLib.h.


### B.2.5 atmanage/

#### atmanage/atManageTier.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| TIER-01 | 487-492 | wrong operator / sign | H/H | `tac_change = (TAC_old - TACpost)/TAC_old; if (tac_change > inc_check) TAC = TAC_old + inc_check*TAC_old; else if (tac_change < bm->prop_decTAC) TAC = TAC_old - dec_check*TAC_old;` The sign is reversed: a big proposed *decrease* gives a positive tac_change, which then *raises* the TAC to the max. The second test compares against the raw multiplier `prop_decTAC` (e.g. 0.5) and not `-dec_check`, so almost every other case is pushed to the minimum TAC. prop_inc/decTAC are multipliers (see atManageAnnual.c:849-850). Also divides by zero when TAC_old==0. Function: `DoTierAssessment()`. | `tac_change = (TACpost - TAC_old)/(TAC_old+small_num); if (tac_change > inc_check) ... else if (tac_change < -dec_check) ...` | [ ] |
| TIER-02 | 501-503 | wrong variable | H/H | `if (TAC < minCatch) TAC = FunctGroupArray[species].speciesParams[MinCatch_id];` sets the TAC to the fraction (e.g. 0.05) and not to the computed `minCatch` (fraction × last historical catch). Function: `DoTierAssessment()`. | `TAC = minCatch;` | [ ] |
| TIER-03 | 535-536 | wrong index / OOB | H/H | `if (bm->TACamt[species][nf][now_id] > no_quota) bm->TACamt[species][nf][now_id] = no_quota;` is inside the `nfishery` loop but uses the stale `nf`, which equals K_num_fisheries (or Nfleets) left over from earlier loops. That is an out-of-bounds read/write on TACamt[species], and the cap is never applied to the fishery actually being updated. Function: `DoTierAssessment()`. | Use `nfishery` | [ ] |
| TIER-04 | 394-399 | ordering / wrong value used | H/H | `RBC = RBCparam[RBCest_id];` is read *before* `Apply_RBC_Buffers()` changes RBCest_id. The TAC is then worked out from the local, unbuffered `RBC` (lines 442/444/527), so tier buffers have no effect on the TAC. Function: `DoTierAssessment()`. | Move the `RBC = ...RBCest_id` assignment to after `Apply_RBC_Buffers()` | [ ] |
| TIER-05 | 325-329 (also 1924-1927) | accumulator / loop | H/M | `for (nf...) for (nreg < Nregions) current_catch += Harvest_Get_TotCumCatch(species, nf, bm->thisyear);` The inner region loop does not depend on nreg, so the total catch is multiplied by Nregions. Functions: `DoTierAssessment()`, `Call_Tier3()`. | Drop the nreg loop | [ ] |
| TIER-06 | 376-384 | switch fall-through | M/M | `case tier8: case tier9: case tier13: RBC = 0.0;` has no break, so it falls into `default` and quits with "No such tier". Function: `DoTierAssessment()`. | Add `break` (or a clear "not implemented" quit) | [ ] |
| TIER-07 | 3539-3549 | switch fall-through / wrong source | H/H | `case sp_rollover:` has no `break`, so it falls through to `default` and calls `quit("")`. It is also unreachable in practice because `tier` is re-read from `speciesParams[tier_id]` and not from the local `tier` that DoTierAssessment sets to sp_rollover (lines 267/277). As a result a rolled-over TAC gets the base tier's buffer (e.g. sig1) and not the MYTAC buffer. Function: `Apply_RBC_Buffers()`. | Add `break`; pass the effective tier in as a parameter | [ ] |
| TIER-08 | 702-712 | off-by-one / sanity logic | M/M | `if (MinCyr > MaxCyr) MaxCyr = MinCyr;` can push MaxCyr past `year` (future, unfilled years), so it should probably be `MinCyr = MaxCyr`. `nyrs` is computed *before* `MaxCyr` is bumped from 1 to 2, so the mean at 763-771 adds an extra year but divides by the old count. Function: `Tier_averages()`. | Clamp MinCyr instead; recompute nyrs after the MaxCyr fix-up | [ ] |
| TIER-09 | 718-743 | accumulator not reset | M/M | `Util_Init_1D_Double(discrate, nyears, 0.0)` zeroes indices 0..nyears-1 (the number of averaging years), not the absolute years `year-3..year` that are used afterwards. Inside the fleet loop `discrate[iy]` is only overwritten when `DiscYears[nf][iy] > 0`, so fleets without discard data inherit the previous fleet's rate (or values from line 807 of the previous call). Function: `Tier_averages()`. | Reset `discrate[startyear..year]` at the start of each fleet iteration | [ ] |
| TIER-10 | 721, 800-803 | uninitialised data / stale variable | M/M | `DiscType = ...DiscardType[nf]`: DiscardType is allocated with size nLen (atAssessTierSetup.c:130), is indexed by fleet, and is never filled anywhere in the repo, so it is always 0. Neither DiscType branch (1/2) ever runs, so discard rates are always 0, or NaN when there is no catch (807). The all-fleet block at 800-803 also uses the `DiscType` left over from the last fleet and not each fleet's own. Function: `Tier_averages()`. | Allocate DiscardType with K_num_fisheries and read it from params; look up per fleet in the 799 loop | [ ] |
| TIER-11 | 644 | wrong divisor | M/M | `avprop_reg[nf][nreg] /= num_avg_year;` but the sum covers `startyear..year` (5 years, or fewer near the start). When num_avg_year != the number of years summed, avprop does not sum to 1. Function: `Tier_averages()`. | Divide by `(year - startyear + 1)` | [ ] |
| TIER-12 | 997-1022 | cohort vs age / double counting | M/M | `natm = Mnat[0][0][s][a][index2]` uses `a` (a cohort×genotype index, up to numCohortsXnumGenes) as the *age-in-years* index of Mnat. `num_individuals` and `bodysize` are also summed over all individuals regardless of sex, and then added once per sex, so the yield is doubled when K_num_sexes=2. Function: `TierOneMimic()`. | Map cohort to age (`ia*ageClassSize`); split numbers by sex or drop the sex loop | [ ] |
| TIER-13 | 1131, 1147 | off-by-one / heap overflow | H/H | `for (ia = 0; ia <= this_nselage; ia++) ... propgt05[ia] = 1;` reads `numatage[nselage]` and writes `propgt05[nselage]`. Both are allocated with size nselage (atAssessTierSetup.c:461-463). The do-while at 1147 likewise reads `propgt05[ia+1]` with ia+1 == this_nselage. This is left over from Sally's 1-based code. Function: `TierThree_CC()`. | Use `ia < this_nselage`; bound the do-while at `ia+1 < this_nselage` | [ ] |
| TIER-14 | 1121-1126 | 1-based leftover / dead code | M/M | `if ((a == AgeSel95+1) && (numatage[1] < numatage[2]))`: at this point `numatage[2]` has not been filled yet (it is 0), so the test can never be true and the "drop first age if lower than next" logic never runs. In 0-based terms it should compare `numatage[0] < numatage[1]`. Function: `TierThree_CC()`. | Translate to 0-based indices | [ ] |
| TIER-15 | 1173-1176 | off-by-one | H/H | `nz` is the index of the last stored Z (it starts at -1 and is incremented before use), but `for (i=0; i<nz; i++)` and `meanz = sumz/nz` treat it as a count. The last Z is dropped, and a single valid Z (nz==0) is reported as an assessment failure. Function: `TierThree_CC()`. | Use `nz+1` as the count (`i <= nz`, `if (nz >= 0)`, `/(nz+1)`) | [ ] |
| TIER-16 | 1345, 1373 | uninitialised / not reset | H/M | `NCA` is initialised to 0 at its declaration and only updated when a year has data. It is not reset per fleet (only `Ncage_id` is), so a fleet with no data reuses the previous fleet's NCA (and the first fleet uses 0). `if (NCA >= 0)` is therefore always true and the minimisation runs on stale or empty Obsageprop. Function: `TierThree_CCsel()`. | Set `NCA = -1` at the top of each fleet iteration | [ ] |
| TIER-17 | 1435 | copy-paste / wrong index | H/H | Restart branch: `fvalF[nF] = UnTransform(simplex[1][1], 0.01);` takes the S95 parameter of the second vertex. The F estimate is `simplex[0][0]`, as used at line 1412. Function: `TierThree_CCsel()`. | `UnTransform(simplex[0][0], 0.01)` | [ ] |
| TIER-18 | 1459, 1472, 1478, 1482 | off-by-one | H/H | `nF` is the index of the last fitted fleet (it starts at -1, then ++), but the loops `i < nF` and the divisor `nF` treat it as a count. With one fleet (nF=0) Fcurr=0 and agesel is all zeros; with more fleets the last fleet is dropped. Function: `TierThree_CCsel()`. | Use `nF+1` as the count | [ ] |
| TIER-19 | 1614 | off-by-one | H/H | `for (iy=0; iy < Ncage_id; iy++)`: Ncage_id is the last index (it starts at -1), so the last year of catch-at-age is left out of the objective. With one year the objective is constant (ss=0), so the fit is meaningless. Function: `EquilF_Funk()`. | `iy <= Ncage_id` | [ ] |
| TIER-20 | 1601-1604 | unreachable code | M/M | The plus-group branch `else if (a == Tier3_maxage)` can never run inside `for (a < Tier3_maxage)`, so the plus group is never accumulated. Function: `EquilF_Funk()`. | Apply the plus-group formula at `a == Tier3_maxage-1` | [ ] |
| TIER-21 | 1737-1740 (and 2726-2729) | wrong formula | H/H | `step1 = (F40-F20)*(Fcurr-F20); frbc = F48 / step1;` is dimensionally wrong (1/F). The linear ramp between F40 and F20 should be `frbc = F48*(F20-Fcurr)/(F20-F40)`. The same code is copied into TierFiveSAFE. Functions: `newTier3Rule()`, `TierFiveSAFE()`. | `frbc = F48 * (F20 - Fcurr) / (F20 - F40);` | [ ] |
| TIER-22 | 1812-1841 | off-by-one | L/L | `fval` is updated at the *end* of the loop as `i/1000.0`, so iterations 0 and 1 both use F=0 and every later F lags by 0.001. Function: `getF20_40_48()`. | Set `fval = i/1000.0` at the top of the loop | [ ] |
| TIER-23 | 1868-1886 | cohort vs age | M/M | YPR loops over `numCohorts`, but the `wt/mat/sel` arrays it is given are indexed by age in years (filled up to MaxAge in TierOneMimic/trueF and up to Tier3_maxage in CC/CCsel). When ageClassSize>1, only the first numCohorts *years* are used, so the plus group is wrong. Function: `YPR()`. | Loop over the age length actually used (MaxAge/Tier3_maxage), passed in as a parameter | [ ] |
| TIER-24 | 1971-1972 | 1-based leftover / OOB | H/M | `maxprop = avprop[1]; maxfleet = 1;` reads out of bounds when Nfleets==1 (avprop has size Nfleets) and defaults to fleet 1 rather than 0. Function: `TierFour()`. | `maxprop = avprop[0]; maxfleet = 0;` | [ ] |
| TIER-25 | 2052, 2062 | off-by-one | M/M | The CPUE target loop `iy < MaxCyr` excludes the last year, but the divisor `step1 = yrmax - yrmin + 1` counts it (and uses the unsanitised params). So CPUEtarg is biased low. Function: `newTierFour()` (same exclusive loop in `WriteCPUE()` 2177). | `iy <= MaxCyr`; divide by `MaxCyr-MinCyr+1` | [ ] |
| TIER-26 | 2078-2095 | off-by-one | H/M | `startyear = year - m; for (iy=startyear; iy<=year; ...)` sums m+1 years but divides by m (or by `year-startyear` when clipped). CPUEav is overstated by (m+1)/m, which makes the RBC multiplier too high. Function: `newTierFour()`. | `startyear = year - m + 1` (or divide by `year-startyear+1`) | [ ] |
| TIER-27 | 2291, 2295, 2388 | copy-paste / wrong index | H/H | `k = simplex[0][0]; xpar[1] = simplex[0][0];` and in the objective `in_K = xpar[0];`. K is parameter 1 (comment: "xpar[1] = K"), but Bstart is used everywhere instead. The K dimension of the simplex is optimised for nothing, and depletion = BcurrSP/Bstart. Functions: `TierFive_SP()`, `SurplusProduction()`. | Use `simplex[0][1]` / `xpar[1]` | [ ] |
| TIER-28 | 2397-2398 | misplaced parenthesis | H/H | `step1 = (1 - (bt/K)^p) - T5_removal[iy-1]; B = bt + bt*r/p*step1;` multiplies the catch removal by `bt*r/p`. The projection loop at 2432-2433 correctly subtracts the removal outside. Function: `SurplusProduction()`. | `B = bt + bt*r/p*(1-(bt/K)^p) - T5_removal[iy-1]` | [ ] |
| TIER-29 | 2430-2438 | off-by-one / uninitialised read | M/H | The projection loop `iy < index2` runs zero times for AssessDelay 0 and only to CurrentYear for delay 1. Then `BcurrSP = expectB[index1]` reads an entry that was never computed (outside the Init range at 2380, so stale or 0). The fitting loop also stops at `iy < CurrentYear`. Function: `SurplusProduction()`. | Project through `iy <= index1`; init expectB to the full size | [ ] |
| TIER-30 | 2475-2480 | off-by-one / heap overflow | H/M | `do { iy++; ... TSbiomass[iy] = ... } while (... && iy < 1000.0);` can write `TSbiomass[1000]`, but the array is allocated with 1000 entries (atAssessTierSetup.c:499). Function: `get_T5_F20_40_48()`. | `while (... && iy < 999)` | [ ] |
| TIER-31 | 2583-2589 | off-by-one / OOB read | H/L | `do { i=i+1; } while ((avlen < T5_Avlen_atF[i]) && (i < nfint));` evaluates `T5_Avlen_atF[nfint]` (array size nFtier3 == nfint) before the bound check, and never tests index 1. Function: `TierFive_AvLen()`. | Put `i < nfint` first and start at i=0 | [ ] |
| TIER-32 | 2639-2649 | accumulator reset in wrong loop | H/H | `for (iy...) { sumcatch = 0.0; ... }` resets the sum every year, so DCAC uses only the final year's catch and not the cumulative catch over `num_years`. Function: `TierFiveDCAC()`. | Remove the reset inside the loop | [ ] |
| TIER-33 | 2768-2774 | accumulator not reset | H/M | `localcatch` is not reset per year, so `HistCatch` ends up as cumulative total catch rather than the peak annual catch (compare TierSevenRUSS 3051, which resets it). Function: `TierSixRUSS()`. | Set `localcatch = 0.0` at the top of the iy loop | [ ] |
| TIER-34 | 2782-2794 | wrong variable reset | M/M | `RBCspeciesParam[species][TriggerReached_id] = 0;` is reset per region, but the array `TriggerReached[nreg]` (and `[Nregions]`) that the following loops set is never reset. Once a trigger has been reached it stays reached forever. Function: `TierSixRUSS()`. | Reset `TriggerReached[nreg] = 0` (and `[Nregions]`) before the trigger loops | [ ] |
| TIER-35 | 2826-2827, 2835, 2848, 2851, 2863-2867 | swapped arguments | H/H | `DoCPUE(bm, year, species, llogfp)`, `DoPrager(bm, year, species, ...)`, `Call_Tier3(bm, year, species, ...)` and `TierFiveSAFE(bm, year, species, totsumcatch, ...)` all pass (year, species) where the prototypes (lines 75-76, 1899, 57) expect (species, year). The wrong species is assessed, or there are out-of-bounds species indices. Function: `TierSixRUSS()`. | Swap to `(bm, species, year, ...)` | [ ] |
| TIER-36 | 2938-2950 | wrong operator / logic | H/H | `for (iy = year-1; iy >= 0; iy++)` counts *up* without bound (runs off the end of EffortData). Also `j = iy` is assigned only when effort was *not* found, so j is not the most recent year with effort (if found immediately, j stays 0). Function: `DoCPUE()`. | `iy--`; set `j = iy` when `is_done` | [ ] |
| TIER-37 | 2963 | off-by-one / heap overflow | H/H | `AnnCPUE[5-iy]` with iy going down to 0 writes `AnnCPUE[5]`, but AnnCPUE is allocated with 5 entries (atAssessTierSetup.c:215). Function: `DoCPUE()`. | `AnnCPUE[4-iy]` | [ ] |
| TIER-38 | 3044, 3078, 3097, 3183, 3196, 3211 | wrong index | H/H | `cosp = FunctGroupArray[sp].co_sp[sp];` uses the loop counter as the group index. It should be the focal species' companion list, `FunctGroupArray[species].co_sp[sp]` (co_sp is per guild: atManageParamIO.c:259/932). Function: `TierSevenRUSS()`. | `FunctGroupArray[species].co_sp[sp]` | [ ] |
| TIER-39 | 3146-3147 | stale index / OOB | H/H | `if (bm->SP_FISHERYprms[cosp][nf][q_id] > 0.0) toteffort += bm->CumEffort[nf][b];` sits after the nf loops have finished, so nf == K_num_fisheries and both arrays are read out of bounds. Function: `TierSevenRUSS()`. | Wrap in `for (nf = 0; nf < K_num_fisheries; nf++)` | [ ] |
| TIER-40 | 3124-3156 | accumulator not reset | M/M | `areafished` and `areasp` (and `toteffort`) are initialised once at function entry and not per co-species, so `relarea` for later species includes earlier species' areas. Function: `TierSevenRUSS()`. | Reset areafished/areasp at the top of each cosp iteration | [ ] |
| TIER-41 | 3216 | swapped args / wrong species | H/H | `Call_Tier3(bm, year, species, llogfp)` has its arguments swapped, and inside the loop over cosp it should assess `cosp`. Function: `TierSevenRUSS()`. | `Call_Tier3(bm, cosp, year, llogfp)` | [ ] |
| TIER-42 | 3718-3736 | accumulator not reset / off-by-one | M/M | `catchwghtCPUE` is zeroed once outside the `lag` loop, so every lag adds into the same slots. The regression for lag k therefore includes data from lags > k. Also, when mgt_rbcYear==this_year, `yy` runs 1..TCP-1, so slot 0 is never filled and a spurious 0 enters the regression. Division by `totCatch` can also be 0. Function: `trigger_species_breakout()`. | Init inside the lag loop; fix the yy offset (drop the `+1`) | [ ] |
| TIER-43 | 3787 | wrong variable | M/M | `(FunctGroupArray[sp].isImpacted == FALSE)` tests the target species and not the candidate indicator `i`. Function: `CalculateNonIndicatorRBC()`. | `FunctGroupArray[i].isImpacted` | [ ] |
| TIER-44 | 3877-3910, 3899 | wrong constant / accumulator | M/M | The `yy` offset uses `GradientPeriod` (the lag count) and not `GradientWindow` (the array length). Indices can exceed the catchwght/catchwghtCPUE size (max(GradientWindow, TriggerCheckPeriod)), or be clipped so that many years pile into slot 0. The arrays are also not reset between lags (same issue as TIER-42). Function: `Check_Indicator_Links()`. | Use GradientWindow in the offset; reset per lag | [ ] |
| TIER-45 | 3927-3929 | copy-paste | H/M | The else-branch sets `CPUEtrend_id = 0` where it should set `CPUEdir_id = 0`, so the direction stays stale and the trend is wiped before it is used in steps B/C. Function: `Check_Indicator_Links()`. | `RBCspeciesParam[sp][CPUEdir_id] = 0;` | [ ] |
| TIER-46 | 3950-3952, 3968-3970, 4022-4024, 4042-4044 | missing update | H/H | `if (diff < closest_slope) ref_sp = ind_sp;` never updates `closest_slope` (or `closest_slopeB`), so ref_sp ends up as the *last* candidate (possibly `sp` itself), not the closest. And `closest_slope < closest_slopeB` (3981/4055) compares MAXDOUBLE with MAXDOUBLE, so ref_spB is always chosen. Function: `Check_Indicator_Links()`. | Add `closest_slope = diff;` / `closest_slopeB = diff;`; skip `ind_sp == sp` | [ ] |
| TIER-47 | 3967 | inconsistency | H/M | The other-category diff is missing `fabs()` (the same-category loop at 3949 has it), so any species with a more negative trend counts as "closest". Function: `Check_Indicator_Links()`. | `diff = fabs(...)` | [ ] |
| TIER-48 | 3987, 4061 | wrong variable | H/M | `maxTier = mgt_category_id;` defaults to the enum index constant and not the species' current category (`my_category`). When no species falls inside the buffer, the category is set to an arbitrary number. Function: `Check_Indicator_Links()`. | `maxTier = my_category;` | [ ] |
| TIER-49 | 4156, 4164-4172 | state not reset / use-after-close | H/H | `chdir(dirname)` never returns to the parent directory, so later relative file I/O and next year's mkdir end up nested. `Util_Close_Output_File(bm->Dyn4File)` closes the file but leaves the pointer non-NULL (atUtil.c:200), so on the next call `if(!bm->Dyn4File)` is false and the code writes to a closed FILE*. Function: `RDynamicTier4Assessment()`. | Set both pointers to NULL after closing; `chdir("..")` at the end | [ ] |
| TIER-50 | 4280-4282 | copy-paste | H/H | The third block under the "Year, Catch and CPUE" header writes `CatchData` a second time where CPUE (`CPUEgen`) is expected, so the R assessment receives catch as CPUE. Function: `writeDynTier4inputfile()`. | Write `CPUEgen[nf][sumregion][iy]` | [ ] |
| TIER-51 | 4269 | uninitialised counter | L/M | `t` is never incremented, so `t <= HistYrMax` is always true and the retro-catch adjustment applies to all years. Function: `writeDynTier4inputfile()`. | Use `iy` (or increment t) | [ ] |
| TIER-52 | 4456 | statement inside comment | H/L | `...= 0.0; // temporary value ...        bm->RBCestimation.RBCspeciesParam[species][EstB0_id] = 999.9;` puts the EstB0 assignment inside the comment, so EstB0 is not flagged on failure and EstDepletion (4467) mixes a valid B0 with Bcurr=999.9. Function: `setRBC()`. | Move the EstB0 assignment onto its own line | [ ] |
| TIER-53 | 4481-4490 | off-by-one / wrong index | H/M | The loop `yr < mgt_rbcYear + n - 1` sums n-1 RBCs but divides by n. The replacement loop writes `RBC_by_year[year]` repeatedly and not `[yr]`. Function: `Averaged_RBC()`. | Loop `yr < mgt_rbcYear + n`; write `RBC_by_year[yr]` | [ ] |
| TIER-54 | 3243 | wrong array | M/L | `tiertype = FunctGroupArray[species].speciesParams[tiertype_id]`: tiertype_id belongs to the RBC_SPECIES_PARAMS enum (atlantisboxmodel.h:2513/2706). DoTierAssessment reads `RBCspeciesParam[species][tiertype_id]`, so the tier5 log branch tests an unrelated parameter. Function: `Evaluation()`. | `bm->RBCestimation.RBCspeciesParam[species][tiertype_id]` | [ ] |
| TIER-55 | 3256-3258, 3292 | wrong index (log only) | M/L | `sb0 = SBioZero[0][0]` is used with `SpawnBio[Nstocks][Nregions]` (elsewhere the overall value is at [Nstocks][Nregions]). `step1 = CPUEgen[fleetid][r][year]` is computed with the stale `r` (=Nregions) outside the loop and printed for every region. Function: `Evaluation()`. | Use [Nstocks][Nregions]; move step1 into the loop | [ ] |
| TIER-56 | 1724-1725, 2528-2529, 2579-2580 | debug leftover | H/L | Hard-coded `if (bm->dayt > 10584) printf(...)` debug prints left in live code. Functions: `newTier3Rule()`, `TierFive_AvLen()`. | Remove | [ ] |


#### atmanage/atManage.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| MNG-01 | 160-163 | swapped indices | H/H | `bm->TAC_over[ij][sp][k] = 0` with `ij` looping over `K_num_fisheries` — TAC_over is allocated `Util_Alloc_Init_3D_Int(K_num_basket, K_num_fisheries, K_num_tot_sp)` (atManageSetup.c:1260) => `[sp][nf][basket]`, and every other use is `TAC_over[sp][nf][..]` (lines 818, 859, 959, atHarvestDiscards.c:248). Here sp indexes the fishery dimension (size K_num_fisheries) -> out-of-bounds writes whenever K_num_tot_sp > K_num_fisheries, and the real flags are never reset so TAC-over discarding/closures are sticky. Function: `Manage_Calculate_Total_Effort()`. | `bm->TAC_over[sp][ij][k] = 0;` | [ ] |
| MNG-02 | 959, 978, 1111, 1133, 2405 | buffer overrun | H/H | `bm->TAC_over[sp][fishery_id][nreg]` — third dimension is allocated with `K_num_basket` (atManageSetup.c:1260; K_num_basket = max basket size, usually 1, atManageSetup.c:1291-1297) but is indexed by region/stock `nreg` up to `K_num_active_reg` / `numStocks`. Any regional-TAC config with >1 region/stock writes/reads past the end (also atHarvestDiscards.c:248 reads `[this_stock]`). Functions: `TAC_Check()`, `Manage_Visit_Council()`. | Allocate third dim as max(K_num_basket, nreg, K_num_stocks_per_sp) (and reset that full extent in MNG-01). | [ ] |
| MNG-03 | 387-417 | uninitialised variable | H/M | `} else if (flagmanage > stock_adapt_mgmt)` — `flagmanage` is only assigned inside the `if (flagspeffortmodel != readts_effort)` branch (line 390); for read-ts effort fisheries it is uninitialised (first fishery) or stale from a previous fishery, so TAC_Check (and TAC_over discards) runs/does not run for the wrong fisheries. Function: `Manage_Calculate_Total_Effort()`. | Move `flagmanage = (int) bm->FISHERYprms[fishery_id][flagmanage_id];` before the if. | [ ] |
| MNG-04 | 498 | wrong index | H/M | `mpa_scale = bm->MPA[bm->current_box][fishery_id];` inside `for (ij...)` loop — `current_box` is the ecology loop's stale global box, not `ij`; every box gets the MPA scalar of one arbitrary box. Compare line 454 `bm->MPA[ij][fishery_id]`. Function: `Manage_Calculate_Total_Effort()`. | `bm->MPA[ij][fishery_id]` | [ ] |
| MNG-05 | 561-563 | always-false condition | M/M | Outer `if (flagspeffortmodel == wgt_effort \|\| == dist_effort)` then inner `if (== array_effort \|\| == dist_effort)` — array_effort can never be true inside, and wgt_effort never passes the inner test, so effort buffering is never applied to wgt_effort (and array_effort is never reached). One of the two lists is a copy-paste error. Function: `Manage_Calculate_Total_Effort()`. | Make the two model lists consistent (decide whether array_effort or wgt_effort is intended). | [ ] |
| MNG-06 | 116-148 | ordering | M/M | `bm->CumDisplaceEffort[ij][nf] += bm->Effort[ij][nf];` runs after `bm->Effort[ij][nf] = 0.0` at line 123 (when EffortModelsActive), so the displaced-effort record (written out in atHarvestIO.c:570) only ever accumulates zeros (or the 1.0 reset value). Function: `Manage_Calculate_Total_Effort()`. | Accumulate CumDisplaceEffort before Effort is zeroed (e.g. use OldEffort). | [ ] |
| MNG-07 | 245-257 | uninitialised variable | H/H | In the `!bm->EffortModelsActive` branch `Effort_Displacement(bm, ..., orig_FCpressure, &FCpressure, ...)` is called with `FCpressure` never initialised (declared line 86); Effort_Displacement reads `*FCpressure` (line 1919-1920) and sets `*FCdisplaced = orig_FCpressure - *FCpressure`, so F_rescale/F_displaced applied to `bm->Effort` are garbage. Function: `Manage_Calculate_Total_Effort()`. | Set `FCpressure = orig_FCpressure;` (scaled by MPA if intended) before the call. | [ ] |
| MNG-08 | 1129-1137 | wrong variable / copy-paste | H/M | `if (regtaccount == nstock)` in the common-pool regional closure check — `nstock` is stale (last species' value from the loop at 1077-1127, possibly K_num_active_reg) while the count loop runs to `FunctGroupArray[sp].numStocks`. The non-common-pool version (976-981) correctly resets `nstock = numStocks`. Fishery closure decision is wrong when species have different stock numbers. Function: `TAC_Check()`. | Add `nstock = FunctGroupArray[sp].numStocks;` before the inner loop. | [ ] |
| MNG-09 | 948, 1087 | wrong operator | L/M | `if (!in_quota \|\| (!sp_reg && (nstock < 1)))` — comment says "Only include those groups under regional TACs with separate stocks", but with `&&` non-regional species (sp_reg = 0) are still processed whenever nstock >= 1 (always), so speciesParams[regionalSP_id] has no effect. Function: `TAC_Check()`. | Likely `(!sp_reg \|\| (nstock < 1))`. | [ ] |
| MNG-10 | 1266-1273 | always-true condition | H/M | `if (flagpulsefish && (end_date < bm->dayt + 1.0))` inside `if (end_date < bm->dayt)` — always true, so the 1/EFFmultA "undo pulse" correction (meant only for the first day after the pulse) is applied every timestep forever, compounding on previous-effort-dependent models. Function: `Forced_Effort_Change()`. | `if (flagpulsefish && (bm->dayt - end_date) <= bm->dt / 86400.0)` (or track that the reset has been done). | [ ] |
| MNG-11 | 1739-1740 | off-by-one / missing wrap | M/M | `bm->Effort_hdistrib[ij][fishery_id][bm->QofY + 1]` without the `QofY < 3` guard used in array_effort/rec_effort (1451-1456, 1536-1541); in Q4 it reads entry 4 (unused quarterly slot, normally 0, or a monthly econ value) instead of wrapping to 0. Function: `Allocate_Immediate_Effort()` (ideal_distrib_effort). | Use `(bm->QofY < 3) ? QofY+1 : 0`. | [ ] |
| MNG-12 | 1894-1897 | inverted logic | H/H | `MPA_check = 1.0 - bm->MPA[nb][fishery_id]; fishthere *= MPA_check;` — bm->MPA is an access multiplier (1 = open, applied as `FCpressure * MPA` at 454; default 1.0, atManageSetup.c:1220). Using 1-MPA zeroes open neighbours and gives full weight to closed ones, so displaced effort is sent INTO closed areas. Function: `Effort_Displacement()`. | `MPA_check = bm->MPA[nb][fishery_id];` | [ ] |
| MNG-13 | 2067-2089 | buffer overrun | H/H | "Break loop" is done with `sp = bm->K_num_tot_sp;` but the inner `cohort`/`cIndex` loops keep running and immediately evaluate `FunctGroupArray[sp].numCohortsXnumGenes` / `FunctGroupArray[sp].contaminantTracers[cohort][cIndex]` with sp == K_num_tot_sp — read past FunctGroupArray. Function: `Check_For_Active_MPA()`. | Use a flag/goto to exit all three loops (or `break` out of each level). | [ ] |
| MNG-14 | 2066-2070 | wrong box | H/M | `tracerArray = boxLayerInfo->localWCTracers;` — this is the tracer set of whatever box/layer the ecology loop last processed, not box `ij`; every box's closure is decided on one box's concentrations (and epibenthic groups' contaminantTracers index the epi tracer list, not WC). Function: `Check_For_Active_MPA()`. | Read concentrations for box ij (e.g. from `bm->boxes[ij].tr[k][...]` / epi arrays). | [ ] |
| MNG-15 | 2068 | cohort vs cohort×genotype | M/H | `for (cohort = 0; cohort < numCohortsXnumGenes; ...) contaminantTracers[cohort][cIndex]` — contaminantTracers is allocated `[numCohorts][num_contaminants]` (atUtilGroupIO.c:910); with numGenes > 1 this reads past the array. Function: `Check_For_Active_MPA()`. | Loop to `numCohorts`. | [ ] |
| MNG-16 | 2117-2143 | state not reset | H/M | `CAP_scale = 1.0;` is set once before the fishery loop; a fishery with `flagchangeP_id == 0` inherits the CAP_scale computed for an earlier fishery. Function: `Check_CAP()`. | Set `CAP_scale = 1.0;` at the top of the fishery loop. | [ ] |
| MNG-17 | 2210-2223 | state not reset | H/M | `double POP_scale = 1.0` initialised once; ports without `popchange_id` get the previous port's change scale applied to their population. Function: `Update_Port_Population()`. | Reset `POP_scale = 1.0;` per port. | [ ] |
| MNG-18 | 2517-2575 | logic / stale variable | M/M | Rescaling of BiTACamt is inside the per-companion loop, so it is applied once per OK companion (compounding `BiTACamt *= 1/catch_ratio` each time) and never if the species has no companions; also `prop_yr_changed = 1 - implement_BiM/6.0` (2539) uses implement_BiM from a previous species/fishery since it is only set at 2557. Comment says rescale "only if companion species would be worse off, so check companions" — i.e. check all then rescale once. Function: `Manage_Visit_Council()`. | Compute companion_ok over all companions first (AND), then do the nf/bim rescale once outside that loop; compute implement_BiM before use. | [ ] |
| MNG-19 | 2379 | wrong message argument | L/L | `quit("...more than 12 months in a year - %d\n", bm->BiM)` while switching on `bm->MofY`. Function: `Manage_Visit_Council()`. | Print `bm->MofY`. | [ ] |
| MNG-20 | 2615-2620 | off-by-one (doc vs code) | L/L | `if (val > maxValue) maxValue = val + 1;` — later fisheries are compared against the +1'd value, so e.g. values {3,4} return 4 not 5 as documented. Callers add +1 so no overrun found, but the function does not return what it claims; Species version (2629) never adds +1. Function: `Manage_Get_Max_Fishery_Param()`. | Track the raw max then return `max(1, raw_max + 1)`. | [ ] |
| MNG-21 | 2718-2733 | infinite loop | H/H | `while (this_shot < nshot) { step1 = drandom(0, max_prob); ... if (step1 <= sum_prob) {...this_shot++;} }` — if `Effort_hdistrib[..][today_effort]` is zero in all boxes (fleet idle/closed that day) then prop_effort is all 0, sum_prob stays 0 and no shot is ever placed, while nshot > 0 (it is computed from boat numbers, not effort) -> hang. Function: `GenerateCPUE()`. | Skip shot generation when tot_effort == 0 (and guard against round-off by assigning the last box with effort). | [ ] |
| MNG-22 | 2688-2704, 2727, 2742 | buffer overrun | M/H | `nshot` is derived from boats × down_time × FishableLength × hr_per_dt/ShotLength and is not clamped to `max_poss_shots` (= K_max_num_subfleet × K_max_num_boats × ceil(24/K_min_shotlength) + nbox + 1, set at setup atManageSetup.c:1591); boat numbers can also grow under the econ model. `shot_loc[this_shot]` / `ShotData[this_shot]` can overrun. Function: `GenerateCPUE()`. | Clamp `nshot` to `max_poss_shots - bm->nbox - 1`. | [ ] |
| MNG-23 | 2852 | wrong operator | H/M | `bm->alloced_discard[sp][ij] -= final_shot_discards;` — alloced_discard is subtracted from available discards at 2808/2940, so it must increase as discards are allocated; decrementing makes available discards grow with each shot. Function: `GenerateCPUE()`. | `+=` | [ ] |
| MNG-24 | 2855-2874, 2984-2986 | accumulator / allocation logic | H/M | `sum_prob` is reset only once before the while loop and the k loop has no `break`, so after the first pass every layer past the threshold (and on later passes all layers) is allocated; and `allocate_catch()` allocates the whole `catch_left` of a layer rather than `min(catch_left, *catch_unalloced)`, so a single small shot consumes all catch in a layer and catch_unalloced goes negative. Shot catches/depths and tot_alloced are wrong. Functions: `GenerateCPUE()`, `allocate_catch()`. | Reset sum_prob each iteration, break after allocating, and allocate `min(catch_left, *catch_unalloced)`. | [ ] |
| MNG-25 | 2992-2996 | wrong array / copy-paste | H/M | `for (ij = 0; ij < bm->nbox; ij++) bm->prop_effort[ij] /= (new_sum + small_num);` — after reducing `prop_depth[k]` the code renormalises the box effort distribution instead of the depth distribution (and reuses parameter `ij` as loop counter). prop_effort is repeatedly inflated; prop_depth never renormalised. Function: `allocate_catch()`. | Loop `for (kk = 0; kk < bm->boxes[ij].nz; kk++) bm->prop_depth[kk] /= new_sum;` | [ ] |
| MNG-26 | 2936 | unit mismatch | M/M | Left-over effort uses `Effort_hdistrib[..][today_effort] / 24.0` whereas shot effort uses `* 24.0` ("originally in days and want hours", 2897); also `alloced_effort` is never incremented, so the residual effort is the full effort, not the left-over. Function: `GenerateCPUE()`. | Use `* 24.0` and update alloced_effort with final_shot_effort per box. | [ ] |
| MNG-27 | 2937-2941 | accumulator in wrong loop | H/M | `ShotData[...][tdiscard_id] += Discards[ij][sp][nf]*... - alloced_discard[sp][ij]` is inside the `k` (layer) loop, but Discards/alloced_discard are per box — discards are added nz times. Function: `GenerateCPUE()`. | Move the tdiscard line outside the k loop. | [ ] |
| MNG-28 | 2938, 2943 | wrong formula | M/L | Left-over `wdepth_id += catch` then `/= tcatch` gives ~1, not a catch-weighted depth (compare 2987 which multiplies by a depth term). Function: `GenerateCPUE()`. | Accumulate `depth(k) * catch_left`. | [ ] |
| MNG-29 | 2817-2830 | logic | M/L | If `step1 >= cpue_cdf[sp][nc][gun]` for all bins (cdf not reaching 1), `this_catchbin` stays 0 giving zero catch instead of the top bin; bin 0 always yields `max_catch = 0`. Function: `GenerateCPUE()`. | Initialise `this_catchbin = K_num_catchbin - 1`. | [ ] |


#### atmanage/atManageIndex.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| MNG-30 | 105-111 | uninitialised flag | M/L | `first_indx = (int) manageindx[fishery_id][frstindx_id]` — manageindx is allocated/initialised to 0.0 (atManageSetup.c:1210) and frstindx is never set to 1 anywhere in the repo, so the "first index" branch never runs and the first mgmtAccess value is averaged with 0 (halved). Function: `Update_Management_Indx()`. | Initialise `manageindx[nf][frstindx_id] = 1` after allocation. | [ ] |


#### atmanage/atSS3assess.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| MNG-31 | 176, 200 | invalid free | H/H | `ss3_abs = realpath(bm->ss3Name, buf); ... free(ss3_abs);` — realpath with a non-NULL buffer returns `buf`, a stack array, so free() of a stack pointer aborts ("pointer being freed was not allocated") on every SS3 run. Function: `Run_SS3()`. | Remove `free(ss3_abs)` (or call `realpath(..., NULL)` and free that). | [ ] |
| MNG-32 | 165-167, 176 | buffer overrun | M/M | `char buf[1024]` passed to `realpath()`, which may write up to PATH_MAX bytes (4096 on Linux). Long paths overflow the stack buffer. Function: `Run_SS3()`. | Use `realpath(bm->ss3Name, NULL)` and free the result. | [ ] |
| MNG-33 | 190, 199 | unchecked return | M/L | `chdir(run_dir)` return not checked — if it fails ss3 runs in the model directory, overwriting/reading the wrong files, and results are then read from run_dir. Function: `Run_SS3()`. | Check return and quit on failure. | [ ] |


#### atmanage/atManageAnnual.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| MNGA-01 | 333 | wrong variable / OOB | H/H | `spTotCumCatch = Harvest_Get_TotCumCatch(sp, nf, bm->thisyear);` is called outside any `nf` loop. `nf` is stale: `K_num_fisheries` (left over from the loop at 286 or the inner loop at 348), so it reads `TotCumCatch[sp][K_num_fisheries][yr]`, past the end of the fishery dimension. It also gives a per-fleet catch ratio that does not depend on fleet, while `co_sp2_TotCumCatch` (358) is per fleet. Function: `Make_Mgmt_Decisions()`. | Move the call inside the `for (nf...)` loop at 348, or sum over nf. | [ ] |
| MNGA-02 | 365, 371 | wrong index / OOB | H/H | `FunctGroupArray[sp].co_sp_catch[nf][co_sp2]`: `co_sp_catch` is allocated `Util_Alloc_Init_2D_Double(K_max_co_sp, K_num_fisheries)` (atManageParamIO.c:933, so `[nf][K_max_co_sp]`) and filled as `co_sp_catch[nf][co_sp]` using the slot index (atManageParamIO.c:306). `co_sp2` is a functional-group id (up to K_num_tot_sp-1), so this reads out of bounds or picks the wrong ratio. Function: `Make_Mgmt_Decisions()`. | Use `co_sp_catch[nf][co_sp]` in both places. | [ ] |
| MNGA-03 | 354-382 | logic | M/M | `co_sp2_TotCumCatch` is reset to 0 and only set when `flagdyn_coupdate` is on. With `flagdyn_coupdate==0` (static read-in ratios), `if (!co_sp2_TotCumCatch && spC_TAC) dont_scale2 = 1;` is true whenever the companion has a TAC, so companion TACs are never adjusted in static-ratio mode. Function: `Make_Mgmt_Decisions()`. | Always fetch `co_sp2_TotCumCatch` (or only apply the zero-catch guard when `flagdyn_coupdate` is set). | [ ] |
| MNGA-04 | 342-345 | wrong index (log) | H/L | `FunctGroupArray[co_sp].groupCode`: `co_sp` is the companion slot index (0..max_co_sp-1), not a group id, so the log names the wrong group. Also `co_sp2 > 0` should be `>= 0` (group 0 is valid; -1 was already skipped at 339). Function: `Make_Mgmt_Decisions()`. | Print `FunctGroupArray[co_sp2].groupCode`; drop the `>0` test. | [ ] |
| MNGA-05 | 601-605, 898-900, 1001 | logic (stuck counter) | H/H | `tac_resetcount_id` is incremented only at line 1001. Line 604 resets it to 0, and the early `return` at 898-900 (TAC change < `TAC_crit_change`) skips the increment. Line 601 returns while `count < period` without incrementing. Once a year's RBC falls inside `TAC_crit_change`, the counter stays at 0 and `0 < tac_resetperiod` (>=1) holds every year, so that species' TAC is never reassessed for the rest of the run. Function: `AMS_Tiered_Assessment()`. | Increment the counter at line 601 before returning, and at the 898 return (or move the increment to just after 605). | [ ] |
| MNGA-06 | 766-777 (alloc atManageSetup.c:912-913) | OOB | H/H | `yr_id = floor(FC_period/364 + 0.5) + bm->thisyear; LeverQueue[sp][nf][yr_id] = ...`. LeverQueue is `[sp][nf][num_years]` with `num_years = ceil((tstop-tstart)/1 yr)`. In the last year(s) of the run, `yr_id >= num_years`, so this writes past the end of the array (heap corruption) whenever tier6 is used. Function: `AMS_Tiered_Assessment()`. | Skip or clamp when `yr_id >= num_years` (store num_years globally), or allocate num_years + max lag. | [ ] |
| MNGA-07 | 909-921 | inverted condition | M/L | The comment says "If phase out date elapsed turn off" but the code is `if (phase_out > dayt) phased_out = 0; else if (phase_start > dayt) {ramp}`. Both comparisons are reversed: a future phase-out date zeroes the TAC now, and the ramp runs before phase_start. Latent only, because `phase_out_id`/`phase_start_id` are never read in (they stay 0, so phased_out=1). The ramp also divides by `period` with no zero check. Function: `AMS_Tiered_Assessment()`. | `if (dayt >= phase_out) 0; else if (dayt > phase_start) ramp; else 1`. | [ ] |
| MNGA-08 | 1378-1387 | accumulation error | H/H | `Braw += totfishpop[othersp]...; Bcurr += Assess_Add_Error(bm, er_case, Braw, ...)`. `Braw` is cumulative (primary plus all previous guild members), and each step adds the error-applied *cumulative* biomass to Bcurr. The guild biomass is massively overcounted (primary counted n+1 times, etc.), which skews the HCR toward FrefA/FrefH. Function: `Guild_Frescale()`. | Use a per-species `double Bsp = totfishpop[othersp]*X_CN*mg_2_tonne; Braw += Bsp; Bcurr += Assess_Add_Error(..., Bsp, ...)`. | [ ] |
| MNGA-09 | 1392-1397, 1482-1488 | off-by-one / wrong bound | H/H | Both loops run `groupIndex < max_co_sp` and pick `othersp = sp` only when `groupIndex >= K_max_co_sp`. Because max_co_sp <= K_max_co_sp (co_sp has K_max_co_sp entries), the primary species is never processed. Its Fcurr is not included, and in the apply loop its `mFC_scale` is never set to F_rescale and `done_Co_sp_id` is never set. The comments ("+1 is for the primary species") show it should be included. Function: `Guild_Frescale()`. | Loop `groupIndex <= max_co_sp` and use `othersp = (groupIndex < max_co_sp) ? co_sp[groupIndex] : sp`. | [ ] |
| MNGA-10 | 1421-1426 | wrong variable / OOB write | H/H | `for (groupIndex = 0; groupIndex < K_max_co_sp + 1; ...) Fcurr += ...; Fcurr_sp[groupIndex] /= counter;`. After the loop `groupIndex == K_max_co_sp+1`, so this writes one past the end of `Fcurr_sp` (allocated K_max_co_sp+1 at 1357), and `Fcurr` is never averaged: it stays a sum. The divisor also counts unused slots rather than guild members. Function: `Guild_Frescale()`. | `Fcurr /= (number of species actually summed);`. | [ ] |
| MNGA-11 | 1354-1357 | memory leak | H/L | `calcM_sp`, `calcF_sp`, `counter_sp` and `Fcurr_sp` are allocated on every call and never freed. Function: `Guild_Frescale()`. | Add `free1d()` for all four before return. | [ ] |
| MNGA-12 | 1428 | inconsistent tier dispatch | M/M | `if (tier < tier8)` sends dyntier4(10), dyntier1B0(11), sp_rollover(12), tier13 and tier14 to the tier8/9 (FrefH) broken stick. `Per_Sp_Frescale()`/`Ecosystem_Cap_Frescale()` treat 10-12 and 14 as the tier1 stick and tier13 as escapement. The F_rescale branch at 1465 is also tier8-specific. Function: `Guild_Frescale()`. | Use the same `switch(tier)` as `Per_Sp_Frescale()`. | [ ] |
| MNGA-13 | 1527, 1617-1631 | uninitialised accumulator | H/H | `Braw` is declared at 1527 without initialisation and accumulated with `Braw += ...` in the SSB branch. It is never reset per species, so the first species uses garbage and later species include all previous species' SSB. (`Per_Sp_Frescale()` initialises `Braw = 0.0` per call.) Function: `Ecosystem_Cap_Frescale()`. | Set `Braw = 0.0;` at the top of the per-species block (before line 1616). | [ ] |
| MNGA-14 | 1802, 1864-1866 | state not reset in loop | H/H | `mFC` is computed once per fishery (1802), but inside the cohort loop `mFC = 0` is assigned when a cohort is outside [mFC_start_age, mFC_end_age). Cohort 0 is normally younger than mFC_start_age, so mFC is zeroed at nc=0 and stays 0 for every older cohort: the expected catch is 0 and the system cap never binds (when `flag_sel_with_mFC==0`). Function: `Ecosystem_Cap_Frescale()`. | Use a per-cohort local, e.g. `double mFC_c = in_age_range ? mFC : 0;`, and use it at 1923/1957. | [ ] |
| MNGA-15 | 1910-1917 | switch fall-through | H/H | `case assess_M_est:` has no `break`, so it always falls into `default: quit(...)`. It also sets `M` rather than `calcM`, so calcM would be uninitialised or stale anyway. The quit text names `syst_cap_calc_method` rather than `M_est_method`. Function: `Ecosystem_Cap_Frescale()`. | `calcM = bm->NAssess[sp][est_M_id]; break;` and fix the message. | [ ] |
| MNGA-16 | 1938-1940 | `=` vs `+=` | H/M | Invertebrate branch: `for (k...) this_Biom = calcTrackedMort[sp][nc][k][start_id] * ...;` overwrites on each stock, so only the last stock's biomass is used (the vertebrate branch at 1933 accumulates). Function: `Ecosystem_Cap_Frescale()`. | Change to `this_Biom +=`. | [ ] |
| MNGA-17 | 1929-1944, 1960 | stale / uninitialised value | H/M | `this_expect_catch` is only assigned when `expectF > 0`, but line 1960 always adds it to `sp_fishery_expected_catch_id`. Cohorts with zero F add the previous cohort's (or previous species', or on first use uninitialised) value. Function: `Ecosystem_Cap_Frescale()`. | Set `this_expect_catch = 0.0;` at the top of the cohort loop. | [ ] |
| MNGA-18 | 1827 | accumulator reset in wrong loop | H/M | `speciesParams[sp_fishery_expected_catch_id] = 0` sits inside the `nf` loop, so each fishery wipes the previous fisheries' expected catch. Only the last flagF fishery's catch survives, while `tot_expect_catch` sums all fisheries. The per-species deduction scalar at 1981-1984 is therefore wrong in multi-fleet models. Function: `Ecosystem_Cap_Frescale()`. | Move the reset to before `for (nf...)` at 1781. | [ ] |
| MNGA-19 | 1982-1988 | missing bound | M/M | `rescale_scalar = (orig - deductions)/orig` goes negative when a species' deduction share exceeds its expected catch, giving a negative `mFC_scale`. Function: `Ecosystem_Cap_Frescale()`. | Clamp `rescale_scalar` to >= 0 (and redistribute the remainder). | [ ] |
| MNGA-20 | 1734, 1771 | copy-paste (message) | H/L | The quit messages say `Per_Sp_Frescale:` inside `Ecosystem_Cap_Frescale()`. | Fix the function name in the text. | [ ] |
| MNGA-21 | 2040 | off-by-one | H/L | `if ((co_sp < 0) \|\| (co_sp > bm->K_num_tot_sp)) continue;` lets `co_sp == K_num_tot_sp` through, which indexes FunctGroupArray out of bounds. Function: `Check_Ecosystem_F_Harvest_Control_Rule()`. | Use `>=`. | [ ] |
| MNGA-22 | 2069-2139 | accumulators never filled / not reset | H/H | `avg_scale`, `num_scale` and `min_scale` are initialised per fleet but never updated from `lever_scale`. So `new_scale` is 0 (avg) or MAXDOUBLE (min), and `effort_scale[nf][target_tac_id]` / season length are set to 0 or MAXDOUBLE whenever an effort, space or season lever is used. Also `update_scale`/`update_season` are initialised once outside the `nf` loop, so once set by one fleet they apply to every later fleet. Function: `Do_MSY_Management()`. | In effort/space/season cases do `avg_scale += lever_scale; num_scale++; min_scale = min(min_scale, lever_scale);`, and reset `update_scale = update_season = 0` at the top of each nf iteration. | [ ] |
| MNGA-23 | 179-197, 204 | wrong expected value | M/M | The post-scaling check compares `newtoteffort` with `(1.0 - effort_scale) * toteffort` whether effort was scaled up by `(1+effort_scale)` or down by `(1-effort_scale/2)`. The check therefore nearly always fires and forces effort to `(1-effort_scale)*toteffort`, so effort is always reduced and the up-scaling is undone. Function: `Annual_Effort_Scale()`. | Store the scalar actually applied (`1+effort_scale` or `1-effort_scale/2`) and use it in both checks. | [ ] |


#### atmanage/atManageSetup.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| MNGA-24 | 1083-1085 | wrong variable / OOB | H/H | Inside `for (i = 0; i < K_num_fisheries; i++)` the test uses `bm->TACamt[sp][nf][now_id]`. `nf` is stale from the earlier loop (== K_num_fisheries, or uninitialised if no impacted species), so it reads `TACamt[sp][K_num_fisheries]`, past the fleet dimension (TACamt `[sp][nfleets][entries]`, line 1197). That is a garbage pointer dereference and a wrong filter. Function: `Calculate_BiTAC()`. | Use `bm->TACamt[sp][i][now_id]`. | [ ] |
| MNGA-25 | 1080 (alloc 1195) | OOB | M/M | `for (nreg = 0; nreg < bm->K_num_active_reg; nreg++) BiTAC_sp[bim][nreg]...`. BiTAC_sp has only `totalreg_id+1 = 3` region rows, but K_num_active_reg can be any number of regions (atbiolsetup.c:583). With more than 3 regions this writes out of bounds, and with exactly 3 it overwrites the total row. The earlier loop clamps to `Kreg` (max 2). Function: `Calculate_BiTAC()`. | Loop `nreg < min(K_num_active_reg, 2)` (the same Kreg clamp). | [ ] |
| MNGA-26 | 1007-1008 | format mismatch | H/L | `warn("... id number %d\n", ..., FunctGroupArray[sp].groupCode)` passes a `char*` to `%d`. Function: `Calculate_BiTAC()`. | Use `%s`. | [ ] |
| MNGA-27 | 1477-1480 | logic (every class incremented) | H/M | `for (nimp...) if (flagimposecatch > 0) impose_classes[nimp] += 1;` increments every class equally, so the max search always picks `nimp = 0` and `flagimposeglobal` is always 0. The intent is to count the class equal to the flag value. Function: `Calculate_Flag_Impose()`. | `nimp = (int)SP_FISHERYprms[sp][fleet][flagimposecatch_id]; if (nimp >= 0 && nimp < K_num_impose_classes) impose_classes[nimp]++;`. | [ ] |
| MNGA-28 | 1552 | copy-paste | H/M | The "top" skipper CDF uses `cpue_cdf_top_r_id` with `cpue_cdf_poor_p_id`. `cpue_cdf_top_p_id` exists (atlantisboxmodel.h:1756). Function: `GenerateCPUEDistribution()` (called from ateconomicsetup.c:640). | Use `cpue_cdf_top_p_id`. | [ ] |
| MNGA-29 | 465-484 | init only in debug | M/M | `speciesParams[TACorig_id] = totTAC` is set only inside `if (bm->debug == debug_assess)`. Otherwise it is set only by the economics setup (ateconomicsetup.c:583). Without econ or debug, `TACorig = 0` in `AMS_Tiered_Assessment()` (618), so with `flagQuotaBound` the check `RBC > TACbound*TACorig` forces RBC to 0, and line 875 divides `0/TACbound`. Function: `Manage_Init()`. | Compute TACorig outside the debug block (keep only the fprintfs debug-gated). | [ ] |
| MNGA-30 | 1068-1069 | wrong condition | M/L | `if (!i) FISHERYprms[i][flagconflict_id] = 0;` resets the flag for fleet 0 only, so the other fleets keep whatever value they had before the `gear_conflict` scan sets it to 1. The intent is to reset for every fleet. Function: `Calculate_BiTAC()`. | Drop the `if (!i)`. | [ ] |
| MNGA-31 | 1362-1463 | memory leak | H/L | `Manage_Free()` never frees `bm->MPA`, `bm->TAC_over` or `bm->sp_basket`. In the fisheries-off path (Manage_Init 329-331) `sp_basket`, `TAC_over` and `rolling_cap_initialised` are allocated, but Manage_Free returns immediately. Function: `Manage_Free()`. | Add the matching free calls in both paths. | [ ] |
| MNGA-32 | 352 | NULL deref | M/L | `*(strstr(convertedXMLFileName, ".nc")) = '\0';` crashes if the output name has no ".nc". `strcpy(fisheriesfile[120], bm->fishprmIfname)` (348) can overflow and the variable is unused. Function: `Manage_Init()`. | Check for NULL before writing; remove `fisheriesfile` or use snprintf. | [ ] |


#### atmanage/atManageParamIO.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| MNGIO-01 | 1069-1081, 517-524 | always-true condition / buffer overrun | H/H | `POP_changes = 1; ... if (POP_changes > 0) bm->flagchangepop = 0;` is always true, so the flagchangepop setting from the input file is always switched off. As a result `POP_max_num_changes` is always 1 and `bm->POPchange = alloc3d(3, 1, K_num_ports)` gives `[ports][1][3]`. `readPopulationChangeXML()` then writes `bm->POPchange[index][b][paramID]` for `b < Port_info[index][POP_num_changes_id]`, which overflows whenever a port has 2 or more changes. The TODO comment at 1068 already suspects this. Functions: `readManagementChangeXML()`, `readPopulationChangeXML()`. | Probably `if (POP_changes <= 0) flagchangepop = 0;` (start POP_changes at 0). Always size the array from the real maximum count, or clamp `counter` to POP_max_num_changes. | [ ] |
| MNGIO-02 | 1019-1023, 343-351, 1037-1042 | buffer overrun (config dependent) | H/M | `EFFORTchange` is allocated `[nf][Effort_max_num_changes][4]`, with max = 1 when `flagchangeeffort == 0`. `readEffortChangeXML()` still reads `numChanges = FISHERYprms[nf][EFF_num_changes_id]` values into `EFFORTchange[nf][b]` for b<numChanges, regardless of the flag, so it overflows if any fishery has >1 changes while the flag is off. `CAPchange` has the same problem with `flagchangecap` through `Util_XML_Read_Change_Values()` (atUtilFisheryXML.c:463-472). Functions: `readEffortChangedXML()`, `readEffortChangeXML()`. | Always size from `Manage_Get_Max_Fishery_Param()`, or clamp the number of reads to the allocated size. | [ ] |
| MNGIO-03 | 662 | copy-paste / wrong param key | H/M | `bm->CPUEshotavgdis = Util_XML_Read_Value(..., "CPUEshotfdis");` reads the integer error-case flag instead of the bias parameter. ConvertAtlantis/atManagetoXML.c:550 creates `"CPUEshotavgdis"`, and atManage.c:2847 uses it as the bias. Function: `readManamentFlagTimeXML()`. | Change the key to `"CPUEshotavgdis"`. | [ ] |
| MNGIO-04 | 144-153 | buffer overrun / index collision | M/H | `bm->BiTAC_sp[b][region][guild][old_id]` with `region < bm->K_num_reg`. BiTAC_sp is allocated with a fixed region dimension of `totalreg_id + 1 = 3` (atManageSetup.c:1195, totalreg_id=2), but K_num_reg can be larger (atbiolsetup.c:599/605). Even with K_num_reg==3, region index 2 is the "total" slot that atManageSetup.c:979 overwrites. Function: `readBiMonthlyXML()`. | Allocate the region dim as K_num_reg+1 and use K_num_reg as the total index, or quit if K_num_reg > 2. | [ ] |
| MNGIO-05 | 294-306 | buffer overrun (unchecked bound) | M/M | The loop `co_sp < FunctGroupArray[guild].speciesParams[max_co_sp_id]` writes `co_sp_catch[nf][co_sp]`, but co_sp_catch is allocated `[nf][K_max_co_sp]` (line 933). Nothing checks that max_co_sp ≤ K_max_co_sp. Function: `Read_Fishery_Companion_Catch_XML()`. | Quit if `max_co_sp_id > K_max_co_sp`. | [ ] |
| MNGIO-06 | 768-770 | NULL dereference | H/L | `childGroupingNode = Util_XML_Get_Node(..., "Location_And_Timing")` is not NULL-checked before `readPortXMLData()` calls `Util_Get_Node_Name(parent)`, unlike every other group lookup in the file. Function: `readPortIndexXML()`. | Add a NULL check that calls quit. | [ ] |
| MNGIO-07 | 335, 748 | error-message text | H/L | `quit("errorString%s attribute group not found.\n", ...)` prints a literal "errorString". Line 748 says "EPUE_Thresholds" for the CPUE_Thresholds group. Functions: `readEffortChangeXML()`, `readManagementEffortXML()`. | Fix the message text. | [ ] |


#### atmanage/atPGMSY.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| MNGIO-08 | 40-66, 452 | uninitialised state / NULL dereference | H/H | Nothing in the repo calls `Initialise_PGMSY()` (grep finds only its definition). `ASSESS_PROJECTION` (global, atlantismain.c:215) and the static `PGMSYfp` are therefore NULL when `Do_Atlantis_PGMSY()` runs (from `Call_PGMSY()` when `UseAtlantisPGMSY` is set), giving a crash in `RunPGMSYProjection`/`WritePGMSYStep`. `Free_PGMSY()` is never called either, and PGMSYfp is never closed. | Call Initialise_PGMSY in setup when UseAtlantisPGMSY>0, and call Free_PGMSY plus fclose at shutdown. | [ ] |
| MNGIO-09 | 55-56, 127, 131, 338, 341 | wrong extent / off-by-one | H/H | `SSB` and `Depleted` are allocated with length `maxage` but indexed by projection year `iYr` (up to ProjYr-1), which overflows when ProjYr > maxage. `WritePGMSYStep()` also reads `Depleted[iYr]` with `iYr = ProjYr`, one past the end even if the arrays were sized ProjYr. Functions: `Initialise_PGMSY()`, `WritePGMSYStep()`, `RunPGMSYProjection()`. | Allocate SSB/Depleted with ProjYr and read `Depleted[ProjYr-1]`. | [ ] |
| MNGIO-10 | 146-153 | accumulator not reset / div by zero | H/H | `RBCspeciesParam[sp][AveF_id] += F_actFleet[sp][nf]` is never zeroed first. GetMultipliers is called several times per year (lines 562, 575, 599), so AveF keeps growing and FratioFleet is wrong. `/= nfleets` divides by 0 if no fleet has q>0. Function: `GetMultipliers()`. | Set AveF_id = 0 before the loop and guard nfleets>0. | [ ] |
| MNGIO-11 | 146, 131, 283 | mixed index spaces | M/M | GetMultipliers reads `Fmatrix[nf][this_year]` with nf = fishery and stores `F_actFleet[sp][nf]`. RunPGMSYProjection and WritePGMSYStep treat Fmatrix rows and F_actFleet as assessment-fleet indices (`f = SP_FISHERYprms[sp][nf][assess_nf_id]`, `Fmatrix[f]`, `F_actFleet[sp][f]`). Function: `GetMultipliers()`, `WritePGMSYStep()`. | Use one index space (assessment fleet f) throughout. | [ ] |
| MNGIO-12 | 264-270 | incomplete initialisation | M/H | The starting population sets only `age = ageclasssize*cohort`, the first age of each cohort, for DEN/LENGTH/WEIGHT/BIO, although DEN is divided by `age_class_part` (meant to spread over the ages in the cohort). Other ages keep stale values from earlier calls or 0, and get LENGTH=0 for selectivity. maxage is also built from numCohortsXnumGenes, so genotype cohorts are treated as older ages. Function: `RunPGMSYProjection()`. | Loop over every age in each cohort, using numCohorts and treating genotypes separately. | [ ] |
| MNGIO-13 | 319-320 | wrong year index | H/H | `totavailB[nf] += BIO[age][iYr]*sel*q; catch = BIO[age][iYr]*...` but BIO is only ever set for year 0 (line 269). For iYr≥1 it is 0 or stale, so projected catch and available biomass are 0, and `Fout = Catch/totavailB` (345) is 0/0. Function: `RunPGMSYProjection()`. | Use `DEN[age][iYr-1]*WEIGHT[age]`, or update BIO[age][iYr] each year. | [ ] |
| MNGIO-14 | 290, 321, 330 | accumulator in wrong loop | H/H | `totF += Fused[nf]` runs inside the age and cohort loops and is only reset once per year. Mortality `exp(-(nat_mort+totF))` for later ages therefore includes the summed F of every earlier age, and F is not weighted by selectivity. Function: `RunPGMSYProjection()`. | Compute `totF = Σ_nf sel*q*Fused[nf]` per age, resetting it for each age. | [ ] |
| MNGIO-15 | 329-334, 338-340 | wrong logic / stale value | H/H | SSB is accumulated only in the `age == 0` branch and uses `DEN[0][iYr]`, which is not set until line 340, after SSB has been used. Mature ages never add to SSB, so recruitment and depletion are wrong. Function: `RunPGMSYProjection()`. | Add `scaled_FSPB*DEN[age][iYr]*WEIGHT` for all ages after updating DEN, then compute the recruitment. | [ ] |
| MNGIO-16 | 323-325 | logic | M/M | The catch is capped at the fleet's total RBC per age (`if (catch > RBC[nf]) catch = RBC[nf]`), so the total summed over ages can still exceed the RBC. Function: `RunPGMSYProjection()`. | Apply the cap to the total fleet catch after the age loop. | [ ] |
| MNGIO-17 | 350-367, 278, 532, 675 | index-space mixing / div by zero | M/M | `RBCupdated[nf][this_year]`, `Fupdated[nf]` and `CatchStore[nf]` are overwritten for `nf < Nfleets` (assessment fleets), but RBCupdated is indexed by fishery elsewhere (278, 532, 675). `ncount` is 0 when `AveYr ≤ 1`, and `Fout[nf][iYr]` overflows when `AveYr > ProjYr`. Function: `RunPGMSYProjection()`. | Map fleets back to fisheries, and clamp AveYr to [2, ProjYr]. | [ ] |
| MNGIO-18 ✔ | 284, 402, 420, 429, 538-545 | swapped indices | H/H | `AvgCatFleet = Util_Alloc_Init_2D_Double(nYears, K_num_fisheries)` gives `[nf][nYears]` (atAssessTierSetup.c:303), but every use is `AvgCatFleet[this_year][nf]` or `[iYr][nf]`. This overflows once the year reaches K_num_fisheries. Functions: `Do_Atlantis_PGMSY()`, `RunPGMSYProjection()`, `RunTier4Projection()`. | Index as `AvgCatFleet[nf][year]`. | [ ] |
| MNGIO-19 | 399-405, 420, 429 | wrong year index / unpopulated data | M/M | `AveF` is built from `FFs[1..AveYr)`, but FFs is only allocated (atAssessTierSetup.c:307) and never filled anywhere, so AveF is always 0.5. The comment says "last 10 years", but the loop uses the earliest years. The projected catch for year iYr uses the historical `AvgCatFleet[iYr]` (model year iYr) instead of the current-year fleet split, and `totcatch` sums over years and fleets, which normalises the split wrongly. Function: `RunTier4Projection()`. | Populate FFs from the Tier4 fit, use recent years, and use the `AvgCatFleet[nf][this_year]/Σ` split. | [ ] |
| MNGIO-20 | 605 | stale loop variable / OOB | H/H | `CatchData[f][r][this_year]` uses `r` left over from the loop at 547-549, so it equals NumRegions of the last fished species. CatchData is `[nf][K_num_reg+1][nYears]` (atAssessTierSetup.c:115), so this reads the wrong region or out of bounds. Function: `Do_Atlantis_PGMSY()`. | Sum CatchData over regions (or use the total-region index) explicitly. | [ ] |
| MNGIO-21 | 605-607 | wrong index | H/H | `CscalarMetier[nf]` is computed and then read as `CscalarMetier[f]` (f = assessment fleet) to set `RBCupdated[nf]`. Function: `Do_Atlantis_PGMSY()`. | Use `CscalarMetier[nf]`. | [ ] |
| MNGIO-22 | 652-659 | stale loop variable / OOB | H/H | The F-rescaling block sits after the `for (sp...)` loop, so `sp == K_num_tot_sp`. `RBCspeciesParam[sp]`, `RBCspeciesArray[sp]` and `ASSESS_PROJECTION[sp]` (whose extra entry is uninitialised) are read out of range, and no real species gets rescaled. Function: `Do_Atlantis_PGMSY()`. | Wrap the block in a loop over the tier1/tier4 fished species. | [ ] |
| MNGIO-23 | 652-659, 363 | logic (bisection) | M/M | `Fupdated *= MultBest` compounds across iterations, but bisection on [MultMin, MultMax] assumes an absolute multiplier on a fixed base F. RunPGMSYProjection then overwrites Fupdated[nf][this_year] with AveF (line 363), so the multiplier is lost. Function: `Do_Atlantis_PGMSY()`. | Keep a base F vector and set `Fupdated = Fbase*MultBest`. | [ ] |
| MNGIO-24 | 463, 540-542 | negative index | M/M | `MinCyr = this_year - TriggerCheckPeriod` is not clamped, so early in a run `CatchData[nf][r][Iyr]` is read with Iyr<0. `Call_PGMSY` only checks `earliestYr > year`. Function: `Do_Atlantis_PGMSY()`. | `MinCyr = max(0, ...)` and recompute numYear. | [ ] |
| MNGIO-25 | 114, 131 | header/format mismatch | H/L | The header has 11 column names, but each row prints 12 values (CatchStore is printed with no header column). Functions: `Init_PGMSY_File()`, `WritePGMSYStep()`. | Add a "Catch" column to the header. | [ ] |


#### atmanage/atRlink.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| MNGIO-26 | 81-89 | buffer overrun / leak | H/L | `malloc(sizeof(char*) * K + 1)` allocates K pointers plus 1 byte, so `names[K]` is out of bounds (operator precedence). `malloc(5)` followed by `strcpy(..., groupCode)` overflows for codes over 4 characters (groupCode is char[20], atFunctGroup.h:80). names is never freed. This is latent: atmanage/Makefile.am:11 lists atRlink.h, not atRlink.c, so this file is not compiled. Its live duplicate is MNGIO-40. Function: `R_Linkage_Start()`. | `malloc(sizeof(char*)*(K+1))`, `strdup(groupCode)`, then free. Or delete the dead file. | [ ] |


#### atmanage/atRlinkRBC.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| MNGIO-27 | 371-376 | use after close | H/H | `Util_Close_Output_File(bm->metierFile)` closes the file but cannot NULL the pointer (atUtil.c:200). In the next assessment year `if(!bm->metierFile)` is false, so writeMetierFile writes to a closed FILE* (undefined behaviour). Function: `Do_PGMSY_R()`. | Set `bm->metierFile = NULL` after closing. | [ ] |
| MNGIO-28 | 270-274 | uninitialised variable | H/H | `if (line_size == 1) { ThisSpID = atoi(line_buf); spID = ...}`. getline counts the newline, so a species-ID line like "5\n" has length 2. The branch only runs for empty lines, so `spID` stays uninitialised and indexes RBCspeciesParam and Catch_by_Metier. Function: `Read_PGMSY_output()`. | Test `line_size > 1` (or parse it), and quit on a bad line. | [ ] |
| MNGIO-29 | 284 | wrong strtok delimiter | H/H | `tok = strtok(NULL, "\n")` uses "\n" instead of `seps` for later tokens, so the rest of the line becomes one token. Only values[0] and values[1] are set, and `values[2+m]` (line 302) reads uninitialised malloc'd memory. Function: `Read_PGMSY_output()`. | `strtok(NULL, seps)` (with "\n" added to seps), and bound j < size. | [ ] |
| MNGIO-30 | 279 | loop condition order | H/H | `while (fgets(buffer,...) && (i < nrow))` reads a line before checking i<nrow. After nrow rows it consumes and drops the next species' ID line, so the following species is misparsed. Function: `Read_PGMSY_output()`. | `while (i < nrow && fgets(...))`. | [ ] |
| MNGIO-31 | 267, 294 | shadowed variable / wrong index | H/H | `for (int sp = 0; sp < MULTISPestNspecies; ...)` shadows the outer `sp`. `RBC_by_year[year]` is written to `RBCspeciesArray[sp]` (loop counter) instead of `[spID]`, and every projection row overwrites the same `[year]` slot although the comment says all years are needed (for the average at 316-317). Function: `Read_PGMSY_output()`. | Use `RBCspeciesArray[spID].RBC_by_year[(int)values[0]]` (or year+i). | [ ] |
| MNGIO-32 | 316-319 | off-by-one | H/M | The average loop runs `yr < mgt_rbcYear + nRBCaverage - 1`, which sums nRBCaverage-1 values, but divides by nRBCaverage. Function: `Read_PGMSY_output()`. | `yr < mgt_rbcYear + nRBCaverage`. | [ ] |
| MNGIO-33 | 202-209, 97-98 | output formatting | M/L | The newline test sits inside the `nreg` loop, so the last species/metier prints Nregions newlines per year row when Nregions>1. Metier codes are printed as `"%s "` plus another `" "`, giving a double space. Function: `writeMetierFile()`. | Move the end-of-row test outside the region loop and check nreg==Nregions-1. | [ ] |
| MNGIO-34 | 359, 40-43 | wrong directory | M/L | `cleanPGMSYFiles()` removes "Metier.dat" and "MULTRBC.RBC" relative to the current working directory (input folder) before `chdir(destFolder)`, but the files are created and read in destFolder, so stale files are not removed. Function: `Do_PGMSY_R()`. | Build the paths with bm->destFolder. | [ ] |


#### atmanage/atRlinkRAssess.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| MNGIO-35 | 180, 202-207 | uninitialised variable / accumulator not reset | H/H | `rawwgt` is never initialised and never reset per cohort/box before `if (this_wgt > rawwgt) rawwgt = this_wgt;`. The first comparison uses garbage, and later boxes and cohorts inherit the previous maximum. Function: `RAssessSurvey()`. | Set `rawwgt = 0.0` before the k loop. | [ ] |
| MNGIO-36 | 213 | out-of-bounds layer | H/H | `rawn = bm->boxes[b].tr[k][den];` runs after the `for (k...)` loop, so k == nz, one layer past the last water layer. Function: `RAssessSurvey()`. | Sum `tr[k][den]` over k inside the loop (or use the intended layer). | [ ] |
| MNGIO-37 | 326-352 | PROTECT imbalance | H/M | Four objects are PROTECTed (infile, outfile, outdir, r_call), but only `UNPROTECT(3)` is called, so R's protect stack leaks on every call and eventually overflows. Function: `Run_RAssess()`. | `UNPROTECT(4)`. | [ ] |
| MNGIO-38 | 118-119 | cohort vs age | M/M | `MeanWtAge[istock][isex][cohort][iyr]` and `MeanLenAge[istock][isex][cohort]` are age-indexed arrays (OverallMaxAge dim, atAssessTierSetup.c:100-101) but are indexed with the cohort number instead of `this_age`, which is wrong when ageClassSize>1. Function: `Populate_RAssessFile()`. | Index with `this_age`. | [ ] |
| MNGIO-39 | 282-287 | string compare / NULL | M/L | `strcmp(buffer, "TAC")` never matches because fgets keeps the "\n". The header is parsed as `atof("TAC")=0`, which is only fixed by the next line. `strtok` can return NULL (for example, a whitespace-only line), and then `strlen(valueStr)` crashes. Function: `Read_RAssess_output()`. | Strip the newline before comparing and check valueStr != NULL. | [ ] |


#### atmanage/atRlinkRedus.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| MNGIO-40 | 39-47 | buffer overrun / leak | H/M | Same bug as MNGIO-26, in compiled code: `malloc(sizeof(char*) * bm->K_num_tot_sp + 1)` leaves `names[K]` out of bounds, `malloc(5)` with `strcpy(groupCode)` overflows for codes over 4 characters, and names is never freed. Function: `Redus_Linkage_Start()`. | `malloc(sizeof(char*)*(K+1))`, `strdup`, then free after use. | [ ] |
| MNGIO-41 | 61, 87-93 | uninitialised variable | H/L | `int error;` is returned uninitialised if the parsed expression has length 0, because R_tryEval is never called. Function: `exec_r()`. | `int error = 0;`. | [ ] |


#### atmanage/atManageIO.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| MNGIO-42 | 158-160, 177 | header/column mismatch | H/L | The header order is `Blim BrefB BrefA`, but the row prints `Blim, BrefA, BrefB` (matching the argument order and the callers in atManageAnnual.c:1334), so the BrefA and BrefB columns are mislabelled. Functions: `initAnnBrokenStickFile()`, `WriteAnnBrokenStickFile()`. | Swap the header labels to `BrefA BrefB`. | [ ] |
| MNGIO-43 | 58-66 | resource leak | H/L | `cpuefp` and `grosscpuefp` are opened lazily in Write_CPUEreport and Write_CPUE but never closed or flushed in `Close_Manage_Output_Files()`, so trailing output may be lost. | Add Util_Close_Output_File for both. | [ ] |


#### atmanage/atManageMPATS.c


No issues found. Functions checked: Setup_MPA_Lists, Load_MPAlist_From_File.


#### atmanage/include/*.h


No issues found. Files checked: atManage.h, atManageLib.h, atManagePrivate.h, atRlink.h. (`EFFORTchange` is defined twice in atManagePrivate.h:54 and :66. This is legal because the second is a tentative definition, but it is redundant.)


### B.2.6 atSS3Link/

#### atSS3Link/atSS3LinkIO.c


Context: the live path is `TierOne()` -> `SS330Assessment()` (atmanage/atSS3assess.c) -> `WriteSS330Files()` (-> `WriteSSDat`, `WriteSSCtl`, `WriteSSFor`, `Create_Starter_File`), then `Run_SS3()`, `Read_SS3_Par_File()`, `Read_SS3_Report_File()`. `WriteSSFiles()` (atSS3Link.c) is commented out in `TierOne()`, so `Write_Forecast_File`, `Write_SS_Data_File`, `Write_SS_Control_File`, the `*_Orig` functions and `Read_SS3_Report_File_EstBo_Only` are currently unreachable (dead). Findings in dead code are rated Sev L.

| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SS3IO-01 | 61-72 (also 219-230) | NULL dereference | H/H | `if ((fid = fopen(...)) == NULL) { ... AssessFail_id = 1; }` then falls through to `while (fgets(buf, buflen, fid) ...)` with `fid == NULL` -> crash whenever SS3 fails to produce Report.sso (the very case the warning is for). Function: `Read_SS3_Report_File()` (same in `Read_SS3_Report_File_EstBo_Only()`). `Read_SS3_Par_File()` correctly returns. | `free(values); return 0.0;` inside the NULL branch | [ ] |
| SS3IO-02 | 36, 61; 302, 315 | wrong path / logic | H/H | Both readers open bare `"Report.sso"` / `"ss3.par"` in the current working dir and ignore `folderName`. In the live path `Run_SS3()` (atSS3assess.c:190-200) does `chdir(run_dir)` and then `chdir(cwd)` back before `Read_SS3_Par_File`/`Read_SS3_Report_File` are called (atSS3assess.c:118-124), so the files SS3 just wrote in `<sp>_SS3_sim_..._year_...` are never read (either "can't open" -> SS3IO-01 crash / par=1000 -> assessment always failed, or a stale file from cwd is read). The old `WriteSSFiles()` path did chdir before reading, the new one doesn't. Functions: `Read_SS3_Report_File()`, `Read_SS3_Par_File()`. | Build path `snprintf(path, ..., "%s%s%s", folderName, FOLDER_SEP, outputFileName)` and fopen that | [ ] |
| SS3IO-03 | 85-104 (also 243-256) | NULL dereference | M/L | `varStr = strtok(NULL, seps); n++; if (n == EstBcurrIndex) ... atof(varStr)` — the NULL check is done before the strtok, so on a short/blank line `atof(NULL)` is called. Function: `Read_SS3_Report_File()`. | Test `varStr != NULL` after each strtok before use | [ ] |
| SS3IO-04 | 41, 146-160 | uninitialised variable / logic | H/H | In the `Biology_at_age_in_endyr_with_CV=f(LAA)` block no lines are ever read: `strtok(buffer, seps)` tokenises the never-initialised local `char buffer[1000]`, so `values[12..16]` are garbage (or uninitialised malloc memory). Also the continuation separator is `"\n"` not `seps`, so after the first token the rest of the line would be a single token; and `j` is unbounded against `values[100]`. Function: `Read_SS3_Report_File()`. | `fgets(buffer, sizeof buffer, fid)` per sex/age row (after skipping header), use `seps` for every strtok, bound `j < size` | [ ] |
| SS3IO-05 | 149-153, 169-179 | buffer overrun (index) | H/H | `for (age = 0; age < RBCspeciesParam[AccumAge_id]; age++) ... MeanLenAgeM[0][sex][age][thisID]` — MeanLenAgeM/SigmaLenAgeM/MeanWtAgeS/MeanWtAgeM are allocated `Util_Alloc_Init_4D_Double(nYears, MaxAge, Nsexes, stocks)` => `[stocks][Nsexes][MaxAge][nYears]` (atAssessTierSetup.c:281-285), and AccumAge = MaxAge + ceil(MaxAge/4) (atAssessTierSetup.c:86) > MaxAge, so `age` overruns the age dimension. Function: `Read_SS3_Report_File()`. | Loop `age < MaxAge_id` (or allocate with AccumAge+1) | [ ] |
| SS3IO-06 | 187-188 | logic (log text) | H/L | "Something has gone wrong reading in the SS3 report file" is printed/logged unconditionally after the loop, including on success. Function: `Read_SS3_Report_File()`. | Only print when the LAA block was not found | [ ] |
| SS3IO-07 | 72-194; 230-292; 326-345 | resource leak | H/M | `fid` is never `fclose`d in `Read_SS3_Report_File()`, `Read_SS3_Report_File_EstBo_Only()` or `Read_SS3_Par_File()`. Two descriptors leak per species per assessment year; on long multi-species runs this can hit the per-process FD limit, after which fopen fails (-> SS3IO-01 crash). | `fclose(fid)` before each return | [ ] |
| SS3IO-08 | 323-338 | logic / parsing | M/H | Searches for `MGparm[16]`, then "ignore the next line" and reads the line after. In an ADMB `.par` file each `# MGparm[16]:` header is followed directly by its value line, so the code skips the value and reads the next `# MGparm[17]:` header -> `atof("#") = 0` -> `MaxConvergCrit` is always 0 and the convergence test in `SS330Assessment()` always passes. Also MGparm[16] is a biology parameter, not a convergence measure (the max gradient is on the `.par` header line "Maximum gradient component = ..."). Function: `Read_SS3_Par_File()`. | Parse "Maximum gradient component" from the first line of ss3.par (or read the value line directly after the header) | [ ] |
| SS3IO-09 | 359, 374-375 | wrong variable | H/H | Starter file writes `"%s%s.dat", groupCode, fileName` where `fileName` is the starter's own path (`"<dir>//starter.ss"`) -> e.g. `FISdir//starter.ss.dat`. The real files written by `WriteSS330Files()` are `<dir>/<groupCode>WriteSS330Files.dat/.ctl` (lines 1142, 1151), and SS3 runs inside `<dir>`, so SS3 cannot find its data/control files. Function: `Create_Starter_File()`. | Pass the dat/ctl base name into `Create_Starter_File` and write `"%s%s.dat"` with groupCode + that base name (no directory) | [ ] |
| SS3IO-10 | 397-398 | copy-paste | H/H | `"1 # min age for calc of summary biomass"` is written twice. SS3.30 starter expects a single value here, so every subsequent entry (Depletion basis, fraction X, SPR basis, F units, ..., check value 3.30) is shifted by one and the starter fails its end-of-file check / is misread. Function: `Create_Starter_File()`. | Delete line 398 | [ ] |
| SS3IO-11 | 419, 423 | buffer size | H/L | `snprintf(fileName, sizeof(fileName), ...)` where `fileName` is a `char *` parameter -> `sizeof` is 8, path truncated to 7 chars. Function: `Create_Starter_File_Orig()` (dead). | Pass the buffer length or use STRLEN | [ ] |
| SS3IO-12 | 552-561 | division by zero / fleet index | M/L | `catch = CatchData[f][sumregion][maxyr] * TAC_old / last_catch` with no guard for `last_catch == 0`; fleets printed 0-based (`f`) where SS needs 1..Nfleets. Function: `Write_Forecast_File()` (dead). | Guard `last_catch > 0`; print `f+1` | [ ] |
| SS3IO-13 | 733, 748, 774, 780, 942 | copy-paste / wrong variable | H/L | Several errors: 780 `for (f = 0; fleetIndex < NumFisheries; f++)` tests the stale `fleetIndex` (== NumFisheries after the loop at 690) so discard fleet specs are never written; 733 writes a stray line `"%e %e"` of NumFisheries/NumSurvey into the CPUE block; 748 CPUE obs `" %d 7 %e %e"` omits the fleet field; 774 writes NumFisheries as N_fleets_with_discard instead of `nfltdisc` (fixed in `WriteSSDat` 1326); 942 no newline after age bins. Function: `Write_SS_Data_File()` (dead). | Use `f` in loop test; remove 733; add fleet to 748; use `nfltdisc` | [ ] |
| SS3IO-14 | 1148/1155-1156, 2281; 1164-1165, 2380 | double free (fclose) | H/H | `WriteSSCtl()` ends with `fclose(fid)` and `WriteSS330Files()` then calls `fclose(fidctl)` on the same FILE*; likewise `WriteSSFor()` closes and the caller closes `fidss` again. Double `fclose` is undefined behaviour (heap corruption / abort on glibc). `WriteSSDat()` correctly leaves closing to the caller. Functions: `WriteSSCtl()`, `WriteSSFor()`, `WriteSS330Files()`. | Remove `fclose(fid)` at 2281 and 2380 | [ ] |
| SS3IO-15 | 1221-1222 | data file format | H/H | Fleet definition line `" 1  -1  1  1  0  FLEET%d in metier %s\n"` has no `#`, so SS reads fleetname=`FLEET0` and then `in`, `metier`, `<code>` as the start of the next fleet's definition -> whole data file misparsed. Because `speciesRPFleetToMetier` is never populated (all 0, so `> -1` is always true) this branch is always taken. The earlier `Write_SS_Data_File()` (line 666) has `# %d in metier`. Function: `WriteSSDat()`. | `" 1 -1 1 1 0 FLEET%d # in metier %s\n"` | [ ] |
| SS3IO-16 | 1222-1300, 1333, 1346, 1446, 1574, 2018-2023, 2037-2039, 2240-2255 | off-by-one (fleet numbering) | H/H | All SS fleet numbers are written 0-based (`f`/`i`): catch lines `-999 1 %d`, CPUE setup `%d 1 0 0`, CPUE/discard/length/age observations, Q setup, Q parms, variance adjustments. SS3.30 numbers fleets 1..Nfleets, so fleet 0 is invalid and every observation is attributed to the previous fleet (the last survey is never referenced). The `*_Orig` writers used `fleetIndex + 1` (e.g. 3087-3110, 3269). Functions: `WriteSSDat()`, `WriteSSCtl()`. | Write `f+1` / `i+1` wherever an SS fleet number is output | [ ] |
| SS3IO-17 | 1250-1255 | data file format | H/H | `fprintf(fid, "%d 1 %d", t, f)` has no trailing space and the next `fprintf(fid, "%e ", catch)` has no leading space, so fleet and catch merge (e.g. `1980 1 01.234e+02  0.01`) -> 4 tokens instead of 5, and the whole catch block is misread. `Write_SS_Data_File()` (line 696) has `"%d 1 %d "`. Function: `WriteSSDat()`. | `"%d 1 %d "` | [ ] |
| SS3IO-18 | 1278, 1280 (also 1300, 1346) | data file format | H/M | CPUE setup lines for non-metier fleets and all surveys (`" %d 1  0  0  # SURVEY%d"`) have no `\n`; the next fleet's setup then sits after the `#` on the same line and is ignored by SS, so with 2+ surveys the specs are missing and the data file is misread. (1300 CPUE obs and 1346 discard obs also lack `\n`; harmless token-wise but they merge with the following line.) Function: `WriteSSDat()`. | Add `\n` to all four formats | [ ] |
| SS3IO-19 | 1218-1528, 2012-2255 | buffer overrun (index) | M/H | Loops run `f < NumFisheries + NumSurvey` and index arrays sized `K_num_fisheries` in the fleet dimension: `CPUEgen[f]`, `CPUEcv[f]`, `LengthFltYr[it][f]`, `LFss[f]`, `LenComp[f]` (atAssessTierSetup.c:149-170, all `[..K_num_fisheries..]`), and in the ctl `CPUEqmu[f]`, `Sel_Pattern[f]`, `mirrored_fleet[f]`, `Start_Sel[f]`, `Sel_Phase[f]`, `SelBlock[f]`, `Varadj_*[f]` (allocated K_num_fisheries / `[K_num_fisheries][K_num_tot_sp]`). NumFisheries is capped near K_num_fisheries (atAssessTierSetup.c:1453-1457) so any surveys push `f` past the end. Functions: `WriteSSDat()`, `WriteSSCtl()`. | Allocate fleet dimension as K_num_fisheries + max surveys, or guard `f < K_num_fisheries` | [ ] |
| SS3IO-20 | 1486-1491, 1577-1590 | off-by-one | H/H | Writes `MaxAge+1` as `#_N_age_bins` but then only `MaxAge` bin labels (`a < MaxAge`) and `MaxAge` values per age-comp vector. SS reads MaxAge+1 bin edges (consuming the `1` of `#_N_ageerror_definitions`) and every age-comp row is one value short -> file misparsed. Function: `WriteSSDat()` (same pattern in `Write_SS_Data_File()` 936-940, 1032-1045). | Make the count and the loops agree (e.g. print `MaxAge` bins, or loop `a <= MaxAge` everywhere with AgeComp sized accordingly) | [ ] |
| SS3IO-21 | 1493-1499 (also 943-949, 3323-3334) | off-by-one | H/H | Ageing-error definition rows must have Nages+1 entries (ages 0..accumulator age); loops write `a < AccumAge` = AccumAge entries. `Ageing_error` is allocated `AccumAge+1` "dimensions 0..AccumAge (SS data file)" (atAssessTierSetup.c:175), and the comment on 1493 says "accumulator age + 1". Each row is one short so the following data is shifted. Function: `WriteSSDat()`. | `a <= AccumAge` in both loops | [ ] |
| SS3IO-22 | 1187, 1561-1566 | copy-paste / uninitialised | H/H | Age-comp partition uses `it==1 -> 2, it==2 -> 0, it==3 -> 1` but `it` runs 0..2: `it==0` (retained) leaves `part` unset (uninitialised in this function — declared without init at 1187) or stale, `it==1` (whole) is labelled retained, `it==2` (discard) labelled whole. The length block a few lines up (1433-1439) uses the correct `!it -> 2, 1 -> 0, else 1`. Function: `WriteSSDat()`. | Copy the mapping from 1433-1439 | [ ] |
| SS3IO-23 | 1680, 2065-2071; 2365-2373 | wrong index (region) | H/M | `WriteSSCtl()` and `WriteSSFor()` set `allregion = 0`, but the sum over regions is stored at index `NumRegions` (`CatchData`/`DiscData` allocated `K_num_reg+1` "so have sum over all regions"; `WriteSSDat()` 1192 and atManageTier.c:598 use `Nregions`). So retention parameters (`dflt`) are decided from region 0 only while the dat file's discard fleets come from the all-region sum (mismatched parameter count -> SS error), and forecast catch splits use region 0 only. | `allregion = (int)RBCspeciesParam[sp][NumRegions_id];` in both | [ ] |
| SS3IO-24 | 1705-1713 | off-by-one / dead data | H/M | Writes `NblockperPatt[i]` blocks per pattern but loops `j < NblockperPatt[i]-1` with `Blocks[i][j*2+1], Blocks[i][j*2+2]` (1-based leftover) -> one begin/end pair too few, so the ctl is misread when `NblockPattern > 0`. Also `Blocks` and `NblockperPatt` are allocated (atAssessTierSetup.c:287-288) but never filled anywhere in the repo. Function: `WriteSSCtl()`. | `for (j = 0; j < NblockperPatt[i]; j++) ... Blocks[i][2*j], Blocks[i][2*j+1]` and populate the arrays | [ ] |
| SS3IO-25 | 1801 | data file format | H/H | `Mat_slope_Fem` line `"    -3     3 %f %f %f     0.8    0   -3  0 0 0 0 0 0 0"` has 15 fields (three `%f` plus 0.8), whereas each MG parameter line takes 14 (cf. Mat50 line 1800). Every later ctl entry is shifted by one token. Function: `WriteSSCtl()`. | Drop one `%f` (and its argument) | [ ] |
| SS3IO-26 | 1852 | wrong operator/precedence | H/M | `vbk = log(VBk[0][1])/VBk[0][0];` — the male K offset should be `log(VBk[0][1]/VBk[0][0])` (cf. 1863, 1874). Wrong male K whenever Nsexes>1 and MG_offset>2. Function: `WriteSSCtl()`. | `log(VBk[0][1] / VBk[0][0])` | [ ] |
| SS3IO-27 | 2017, 2114 | wrong index / overrun | H/M | `speciesRPFleetToMetier[f][sp+1]` — array is `[K_num_fisheries][K_num_tot_sp]` (atAssessTierSetup.c:53) and every other use in this file is `[i][sp]` (1221, 1275, 1385, 1515). `sp+1` reads the next species' column and overruns the row when `sp == K_num_tot_sp-1`. Function: `WriteSSCtl()`. | `[f][sp]` / `[i][sp]` | [ ] |
| SS3IO-28 | 2115, 2117 | ctl file format | H/H | Age-selex pattern lines for fishing fleets (`"%d  0  0  0  # FLEET %d in metier %s"`) have no `\n`, so the next fleet's/survey's `pattern 0 0 0` line lands after the `#` and is discarded by SS -> only one age-selex line is read for all fleets and the rest of the ctl is shifted. The survey branch (2120) has `\n`. Function: `WriteSSCtl()`. | Add `\n` to both formats | [ ] |
| SS3IO-29 | 2088 | ctl file format | M/L | Mirrored size-selex line `"%d %d 0 %d "` has no `\n` (no `#` so tokens still parse, but the next fleet's line is appended). Function: `WriteSSCtl()`. | Add `\n` | [ ] |
| SS3IO-30 | 2368-2373 | uninitialised field / division by zero | H/H | `cat[f] = CatchData[f][allregion][maxyr] * RBCspeciesArray[sp].mgtTAC_old / last_catch;` — `mgtTAC_old` (atlantisboxmodel.h:2999) is never assigned anywhere in the repo, so forecast catches are 0 (or garbage); the TAC is kept in `RBCspeciesParam[sp][TAC_old_id]` (set atManageTier.c:140, 238; used by `Write_Forecast_File()` 558). Also no guard for `last_catch == 0` (NaN/inf written to forecast.ss). Function: `WriteSSFor()`. | Use `RBCspeciesParam[sp][TAC_old_id]` and guard `last_catch > 0` | [ ] |
| SS3IO-31 | 2374 | ctl file format | H/H | `fprintf(fid, "%d 1 %d %f\n,", ...)` — stray comma after the newline, so each following line starts with `,` (e.g. `,-9999 1 1 0`), corrupting the forecast catch list and its terminator. Function: `WriteSSFor()`. | `"%d 1 %d %f\n"` | [ ] |
| SS3IO-32 | 2383-2948, 3444-3981 | copy-paste / wrong index | H/L | (Dead; `Write_SS_Control_File_Orig` is a verbatim copy.) Male parameters are all computed from FEMALE values (2511 `log(SSMort[0][FEMALE]/SSMort[0][FEMALE])` = 0; 2523-2548, 2563, 2587/2591 use `[0][0]` for male CV_old); `Wtlen_2_Fem` written twice (2606, 2609) and male wt-len lines use FEMALE and are labelled `_Fem` (2623-2627); `bm->RBCestimation.RBCspeciesArray->VBt0` (2477, 2485) = species 0, not `groupIndex`; `dflt` loop (2760-2765) overwrites per year so only the last year counts and runs `yearIndex <= maxyr`; 2675-2679, 2798-2846 missing `\n` before `#` comments. Functions: `Write_SS_Control_File()`, `Write_SS_Control_File_Orig()`. | Use MALE indices/`[groupIndex]`, set `dflt` only to 1 inside loop after zero-init | [ ] |
| SS3IO-33 | 2966-3442 | logic / overrun | H/L | (Dead.) `s` loops `0..Nsex_samp-1` with `Nsex_samp = 1` but vectors are only written for `s == 1` / `s == 2` (3272, 3279, 3372, 3379) so no comp data are ever written; `ageIndex <= MaxAge` (3374, 3384) reads `AgeComp[..][MaxAge]`, past the `OverallMaxAge` dimension when MaxAge == OverallMaxAge; `num` not reset before the length/age obs counts (3243, 3342) so counts include discards; 3134 prints stale `fleetIndex+1` instead of `nf+1`; environmental-variable header written twice (3419-3431). Function: `Write_SS_Data_File_Orig()`. | Fix sex tests to `s == 0`/`s == 1`, `<` MaxAge, reset `num` | [ ] |

Functions checked: Read_SS3_Report_File, Read_SS3_Report_File_EstBo_Only, Read_SS3_Par_File, Create_Starter_File, Create_Starter_File_Orig, Write_Forecast_File, Write_SS_Data_File, WriteSS330Files, WriteSSDat, WriteSSCtl, WriteSSFor, Write_SS_Control_File, Write_SS_Data_File_Orig, Write_SS_Control_File_Orig.


#### atSS3Link/atSS3Test.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SS3-01 | 39, 1420-1421, 1750, 1807-1808, 1904, 1978, 1985-1990 | leak / dimension mismatch | H/L | The code re-allocates arrays that `Tier_Assessment_Allocate` already allocated, never frees the originals, and uses different extents. `LenComp` uses 3 data types where the setup uses 4 (atAssessTierSetup.c:169). `AgeComp` uses 3 types and MaxAge where the setup uses 4 and OverallMaxAge (:138). `AgeFltYr` uses 4 where the setup uses 3 (:137). `Ageing_error` uses AccumAge where the setup uses AccumAge+1 (:175). `WtLen`, `LenRetain` and `SelLen` are also re-allocated. Line 39 `nYears = HistYrMax - HistYrMin` is missing the +1. The file is not compiled. Function: `SS3_Test_Init()`. | Delete the file, or drop the re-allocations and use the setup extents | [ ] |


#### atSS3Link/atSS3DataGen.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SS3-02 | 212-216 | accumulator | H/M | `this_SSB += (RN+SN)*DEN*...; tot_SSB += scaled_FSPB[cohort] * this_SSB;`: `this_SSB` is a running total over queue entries and cohorts, and the whole running total is added to `tot_SSB` again on each iteration. External-recruit SSB is therefore over-counted. The in-domain loop at 233 correctly uses `=`. Function: `GetBiomassData()`. | Use a local `val = (...)`, then `this_SSB += val; tot_SSB += FSPB*val` | [ ] |
| SS3-03 | 227, 254, 556, 924 | wrong variable | M/M | `r_id = bm->regID[ij]` uses the biomass *reporting* region (atlantisboxmodel.h:5297). The other functions in this file map boxes with the species *assessment* region `RBCspeciesArray[groupIndex].regID[b]` (lines 122, 480, 877). So SSB, propR, Uage and the raw comp data are split by a different region map than catch and effort. The clamp `r_id >= Nregions -> Nregions-1` hides the mismatch. Functions: `GetBiomassData()`, `GetCatch_n_DiscardData()`, `FindRawData()`. | Use `RBCspeciesArray[groupIndex].regID[ij]` | [ ] |
| SS3-04 | 282-291 | division by zero | M/M | `propR[ij] /= thistot; propSR[ij][s] /= thistot;` has no guard. When the stock is collapsed or absent, `thistot == 0` makes SpawnBio and SBioZero NaN. Function: `GetBiomassData()`. | Divide only if `thistot > 0` | [ ] |
| SS3-05 | 350-383 | wrong denominator / units | H/H | `totnum` is accumulated but never used. MeanWtAge is `sum(W*N*X_CN*mg_2_kg)` divided by `propCount`, which is the number of *cells*. That gives biomass per cell, not the mean weight of an individual. MeanLenAge is `sum(L*N*X_CN*mg_2_kg)/ncells`, so it also carries a spurious mgN-to-kg factor on a length. The comment says the average is "weighted based on numbers". Function: `GetLengthWeightData()`. | Divide by `totnum` per stock. Drop `X_CN*mg_2_kg` from the length sum | [ ] |
| SS3-06 | 369-381 | loop / copy-paste | H/H | The divisions sit inside `for (ij=0; ij<K_num_sexes; ij++)` but always target `[FEMALE]`. With `Num_Sexes=2` the FEMALE mean is divided by propCount twice. Function: `GetLengthWeightData()`. | Divide once, outside the sex loop, or index `[ij]` | [ ] |
| SS3-07 | 369 | wrong index | H/L | The test `propCount[stock_id]` uses the `stock_id` left over from the box/layer loop, before the per-stock loop that follows it. One stock's count gates the division for all stocks. Function: `GetLengthWeightData()`. | Move the propCount test inside the stock loop, as at 378 | [ ] |
| SS3-08 | 337-348, 359-381 | cohort vs genotype | M/M | With `numGeneTypes>1`, several cohorts map to the same `ia`. MeanWtAge[..][ia][iyr] is re-zeroed at 343 for each genotype cohort, so only the last genotype survives. MeanLenAge is not reset, but is divided again for each genotype cohort. Function: `GetLengthWeightData()`. | Loop over basecohort and aggregate all genotypes before averaging | [ ] |
| SS3-09 | 388-441 | state not reset | M/L | `last_wtval/last_wtai/last_lval/last_lai` are not reset per stock or sex. Because the loop order is ia (outer) then stock (inner), gap-filling interpolates using the previous stock's values. Function: `GetLengthWeightData()`. | Make the last_* arrays per stock, or loop stock outermost and reset | [ ] |
| SS3-10 | 525-532 | uninitialised data | H/H | `sumcatch += TotCatch[f][r][iyr]`: `TotCatch` is allocated (atAssessTierSetup.c:160) but never written anywhere in compiled code (grep finds only this read). With `DiscType==2`, the discard fraction becomes `expdisc/(0+0.0001)`, about 10^4 times too large. Function: `GetCatch_n_DiscardData()`. | Use `CatchData[f][r][iyr]` (+ discards) or fill TotCatch | [ ] |
| SS3-11 | 569-573, 582 | unit mismatch | H/H | `SizeNumCaught/SizeNumDiscard` hold *numbers* (atHarvest.c:586/594, `age_catch = fishing / Wgt`). Here they are multiplied by `X_CN*mg_2_kg` and divided by `actualbiom`, which is in kg. So Uage is not a harvest rate. Uage feeds F at atManageTier.c:942 and 1656. Function: `GetCatch_n_DiscardData()`. | Divide catch numbers by numbers present (den), or convert catch numbers to weight | [ ] |
| SS3-12 | 573 | wrong index | M/L | `actualcatchsize[r_id][stock_id][basecohort]` uses the stock_id of the *last layer* from the preceding k loop. Catches are attributed to one stock per box. Function: `GetCatch_n_DiscardData()`. | Pick the stock explicitly, e.g. split catch by stock biomass share | [ ] |
| SS3-13 | 615 | off-by-one / OOB | M/H | `CPUEdevs[f][r][iyr-1]` is read with no `iyr>0` guard. GenData is called with `year` from atManageTier.c:124, which can be 0 in the first year, and CPUEyears is set whenever catch > 0. That reads index -1. Function: `GetFishCPUEData()`. | Use `prev = (iyr>0) ? CPUEdevs[..][iyr-1] : 0` | [ ] |
| SS3-14 | 614-624 | wrong transform | M/M | The AR(1) deviate (normal, mean 0) is used directly as a multiplier, `actualCPUE * cpue_dev`. A hack maps exactly 0 to 1. Generated CPUE can be negative, and with `CPUEvar=0` and `corr<1` it decays as corr^n. Function: `GetFishCPUEData()`. | Use `actualCPUE * exp(cpue_dev)` and remove the 0→1 hack | [ ] |
| SS3-15 | 712-730, 794-796 | overwritten state / missing calc | H/M | On the GenMnomial path, `ageprops[r]` and `lenprops[r]` are 2-D (stock × bin). They are overwritten for every `it` and `f`, so sampling for every data type uses the props from it=2 (discards) and the last fleet. Also `lenprops[Nstocks]` (overall) is never computed; only `ageprops[Nstocks]` is (723-729). Length comps are therefore drawn from all-zero props. Function: `GetFishLength_n_AgeData()`. | Compute the props per it (and fleet) right before sampling, and compute `lenprops[Nstocks]` | [ ] |
| SS3-16 | 714-717, 1033, 1115 | region vs stock | M/M | The loop `r < numStocks` is passed as the region arg `r`. The callee uses it both as stock (`r != stock_id`) and as region in `Ia[f][r]`. Ia is allocated `[nfish][K_num_reg]` (atAssessTierSetup.c:90), so this is OOB when numStocks > K_num_reg, and the wrong flag is read otherwise. Functions: `GetFishLength_n_AgeData()`, `GetLenProps()`, `GetAgeProps()`. | Pass region and stock separately | [ ] |
| SS3-17 | 737-738, 788-796, 816-843 | stale buffer | H/M | `agecomp/lencomp` are reset only per `it`, not per (f,r,s). On the GenMnomial path, when `nage_sample`/`nlen_sample` is 0 for a cell, GenMnomial is skipped. The previous cell's comp is then added into `AgeComp/LenComp[f][s]` for this cell. Function: `GetFishLength_n_AgeData()`. | Zero agecomp/lencomp at the top of the f/r/s body | [ ] |
| SS3-18 | 803, 1384-1391 | wrong quantity | H/H | Age samples are generated by `GenSample(..., NageVec, ..., agecomp, ...)`, which bins by *length*: `l = floor(rawsizedata/bin_size)`, clamped to `vecsize-1`. AgeComp therefore holds length-bin counts piled into age slots, with the plus group absorbing everything above NageVec. This is the default (non-GenMnomial) path used for tier 3. Functions: `GetFishLength_n_AgeData()`/`GenSample()`. | Add an age mode that uses `(nc/numGeneTypes)*ageClassSize` (+ an intra-class draw) as the index | [ ] |
| SS3-19 | 329, 388, 838 | buffer overrun risk | M/H | Writes run up to `Nages = MaxAge` (MeanWtAge/MeanLenAge) and `aa < MaxAge+1` (AgeComp). These arrays are allocated with `OverallMaxAge` (atAssessTierSetup.c:100-101, 138), which is read independently (atassessParamIO.c:967). MaxAge_id is raised to `numCohorts*ageClassSize` at atAssessTierSetup.c:74-75 with no check against OverallMaxAge. Functions: `GetLengthWeightData()`, `GetFishLength_n_AgeData()`. | At setup, enforce `OverallMaxAge >= MaxAge+1` for every assessed species | [ ] |
| SS3-20 | 933-998 | duplicated counts | M/M | `SizeNumCaught/SizeNumDiscard` have no sex dimension, but are added once per `s < Nsexes`. With `Num_Sexes=2`, totnum and tempcatch are doubled, which inflates AgeN/LengthN, and the comps are duplicated across sexes. Function: `FindRawData()`. | Divide by Nsexes or apply a sex ratio | [ ] |
| SS3-21 | 1062-1074 | buffer overrun | H/H | `l = (int)floor(li / bin_size); lenprops[r][l] += ...` has no upper bound check, unlike GenSample:1388. lenprops is allocated `[Nstocks+1][Nlen]` (atAssessTierSetup.c:420). Large fish write past the row and corrupt memory. The `f` parameter is also clobbered by the `for (f...)` loop at 1064, so the fleet is ignored. Function: `GetLenProps()`. | Clamp `l` to Nlen-1. Use a separate loop variable and restrict to the requested fleet | [ ] |
| SS3-22 | 1126-1157, 1171-1174 | accumulator not reset / OOB | H/H | `a` is set once per cohort, but `a += aa` runs inside the *box* loop. It keeps growing across boxes, which yields wrong ages and indices beyond `Nages` into `ageprops[r][a]` (allocated MaxAge+2, atAssessTierSetup.c:422). This happens when ageClassSize > 1. Function: `GetAgeProps()`. | Use `a_box = a_base + aa` per box | [ ] |
| SS3-23 | 1214-1215 | uninitialised source | M/M | `CKSurveyNum` is built from `ageprops`, which is filled only on the `useGenMnomial` path (and then holds the last itype and fleet). Otherwise it is all zeros, so close-kin survey numbers are 0. Function: `GetCloseKinNum()`. | Compute age props explicitly here | [ ] |
| SS3-24 | 1242-1301 | unit mismatch | H/H | `fishableBiomass` mixes units. AGE_STRUCTURED adds a cell total in mg N (`(sn+rn)*den`). AGE_STRUCTURED_BIOMASS and BIOMASS add a concentration (`tr[pid]`, mg N m-3) without `cell_vol`, which is computed at 1245 but unused. There is no `X_CN*mg_2_kg`, and `sel*q*spbiom` is summed once per fishery. It is then divided into CatchData, which is in kg, so Fhist has the wrong scale. There is no zero guard either. Function: `RecordHistF()`. | Convert to kg, multiply concentration by volume, use max or combined selectivity, guard /0 | [ ] |
| SS3-25 | 1286-1290 | copy-paste / loop | H/M | In the BIOMASS case the fishery loop only assigns `sel` and `q`, and the `+=` sits after the loop, so only the last fishery's selectivity and q are used. Function: `RecordHistF()`. | Move the `+=` inside the loop, consistent with the other cases | [ ] |


#### atSS3Link/atCloseKin.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SS3-26 ✔ | 77-100 (uses 122-487) | swapped dims (alloc convention) | H/H | All local arrays are allocated with dims in *index* order instead of reversed. `nsamps_sya = Alloc_3D(Nsexes,nsampy,MaxAge)` gives `[MaxAge][nsampy][Nsexes]` but is used `[s][y][a]`. `inv_totfec_sy` gives `[last_dy][Nsexes]` and is used `[s][y]`. `n_sya`, `psurv_syay`, `Pr_PO/HS_syaya`, `ncomps_PO_syaya`, `Ekin_*`, `sim_nkin_*` (giving `[MaxAge][last_sy][MaxAge][last_sy][Nsexes]`) and `Pr_GG/HS_yaya` and `ncomps_HS_yaya` (giving `[MaxAge][last_sy][MaxAge][last_sy]`) are all indexed the other way round. Any index past a dimension's size goes out of bounds whenever Nsexes, last_sy and MaxAge differ. The TODO at line 76 flags this. The same pattern is at atAssessTierSetup.c:536-549. Function: `CKsimulator()`. | Reverse every dimension list, e.g. `Alloc_3D(MaxAge, nsampy, Nsexes)` | [ ] |
| SS3-27 | 77, 121-122, 165-177 | OOB | M/M | `nsamps_y` and `nsamps_sya` are sized `nsampy = last_sy-first_sy+1` but indexed with absolute `y` up to `last_sy-1`. This overflows when `first_sy >= 2`. Function: `CKsimulator()`. | Size by `last_sy` (like the other arrays) or index `y-first_sy` | [ ] |
| SS3-28 | 72, 140 | year value as index | H/H | `FracLenS[j][s][a][this_min][l]` with `this_min = HistYrMin_id` uses a calendar year (e.g. 1990) as the year index. FracLenS's year dimension is nYears (atAssessTierSetup.c:279), so this reads far out of bounds. Function: `CKsimulator()`. | Use a year *index* (e.g. 0 or `year`) | [ ] |
| SS3-29 | 130-144 | overwritten per stock | M/L | `mature[l]` and `fec_sa[s][a]` are recomputed (=0 then +=) for each stock `j`, so only the last stock's fecundity survives. Function: `CKsimulator()`. | Accumulate across stocks (weighted) or add a stock dimension | [ ] |
| SS3-30 | 154-163, 490-498 | use after close | H/H | `Util_Close_Output_File(bm->closekinCFile)` and the closes of the samp/PO/HS files fclose the stream, but the bm pointers are not set to NULL (atUtil.c:200 takes `FILE*` by value). GenData calls CKsimulator every year, so from the second call on it fprintf's to a closed FILE* (UB/crash). Function: `CKsimulator()`. | Close only at model end, or set the pointers to NULL after closing | [ ] |
| SS3-31 | 241, 270-273, 343-353 | off-by-one OOB | H/H | `psurv_syay[s][y-1][this_maxage][y]` with `this_maxage = MaxAge`, `curr_a` allowed to reach MaxAge (`if (curr_a < MaxAge) curr_a++`), and `ap_b2 = MaxAge` (then `fec_sa[s][ap_b2]`) all index one past the age extent MaxAge (fec_sa: atAssessTierSetup.c:271). Function: `CKsimulator()`. | Use the plus group `MaxAge-1`, or allocate MaxAge+1 | [ ] |
| SS3-32 | 288-299 | accumulator not reset | H/M | `stocktot` is never reset inside the s/y loops, so `inv_totfec_sy` becomes 1/(cumulative total over all earlier years and sexes). Function: `CKsimulator()`. | Set `stocktot = 0` at the top of the y loop | [ ] |
| SS3-33 | 382 | off-by-one loop start | H/H | `for (int s1 = 1; s1 < Nsexes; s1++) comps += ...` skips offspring sex 0, although the comment says "sex of offspring doesn't matter". With 1 sex, comps is 0 and no PO comparisons are generated. Function: `CKsimulator()`. | Start `s1 = 0` | [ ] |
| SS3-34 | 496 vs 630 | swapped arguments | H/H | The call `writeCloseKinNcompsfiles(bm, sp, year, ...)` does not match the signature `(bm, int year, int sp, ...)`. `year` is used as the species index into `CloseKinEst[sp]` and `FunctGroupArray[sp]` (OOB), and the species is printed as the year. Function: `CKsimulator()`. | Call `(bm, year, sp, ...)` | [ ] |
| SS3-35 | 54-55, 109-121, 171, 590-591 | uninitialised per-species params | M/H | The code reads `CloseKinEst[sp].first_sy/last_sy/first_dy/samp_power/a_ck_thresh...`, but these are only read into element 0 (`bm->CloseKinEst->first_sy`, atassessParamIO.c:1088-1094). The struct array is `malloc`'d uninitialised (atassessParamIO.c:619), so for sp≠0 these are garbage loop bounds and sizes. Function: `CKsimulator()`. | Read the values per species, or use `CloseKinEst[0]` consistently | [ ] |
| SS3-36 | 238 | division by zero | M/M | `n_sya[s][y][a] / n_sya[s][y-1][a-1]` has no guard. Zero numbers give inf or NaN survival, which then propagates into psurv and Pr_HS. Function: `CKsimulator()`. | Guard the denominator | [ ] |
| SS3-37 | 337-358 | brace placement | M/L | The `} // for ap_b1` brace actually closes the `if`, so `Pr_HS_syaya = demog_Pr_HSP; Pr_HS_yaya += ...` runs on every ap_b1 iteration. Pr_HS_yaya accumulates partial sums. Pr_HS_syaya ends up correct, and Pr_HS_yaya is unused. Function: `CKsimulator()`. | Move both statements after the ap_b1 loop | [ ] |
| SS3-38 | 589-591 | wrong index / compounding | M/M | `ageprops[s][a]` uses sex `s` as the stock index; ageprops is `[stock][age]`. Also `samp_prop_to += ...` is never reset, and the `*= a_ck_scalar` scaling is reapplied to the cumulative value on every call, so it compounds. Function: `writeCloseKinSampFile()`. | Use a stock index, reset per call, and scale only the new increment | [ ] |
| SS3-39 | 645, 647 | copy-paste | M/L | The HS output prints `(int)sim_nkin_HS[s][y][a][y2][a2]` twice. The TODO at 649 suggests the second value (or the swapped `[y2][a2][y][a]` entry) was intended to differ. Function: `writeCloseKinNcompsfiles()`. | Print the intended pair | [ ] |
| SS3-40 | 146-151 | logic | L/M | `amat` becomes the *last* age whose fecundity is below the threshold, i.e. an immature age, not the first mature age. Function: `CKsimulator()`. | Take the first `a` with `fec_sa >= thresh` | [ ] |


#### atSS3Link/atSS3Util.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SS3-41 | 73-76 | off-by-one (Fortran port) | H/M | `a1 = (int)(ran3*np + 1)` gives the range 1..np: `dum[0]` is never chosen and `dum[np]` (unfilled, 0) can be. `if (a>0) c[a] += 1` then discards category 0 (age 0 or the first length bin) completely; the 0-means-empty sentinel only made sense with 1-based indexing. Function: `GenMnomial()`. | `a1 = (int)(ran3*np)`. Use -1 as the empty sentinel and accept `a >= 0` | [ ] |
| SS3-42 | 36-39, 45-49, 64 | leak / div0 / fixed buffer | M/L | `dum` is allocated before the `tc==0` early return, which leaks it. `tot==0` produces NaN props. `dum[15000]` is fixed while np can reach about 10000+ndim/2, with no bound check. Function: `GenMnomial()`. | Allocate after the check, free on all paths, guard tot, bound np | [ ] |
| SS3-43 | 235-241 | off-by-one (NR port) | H/L | In the 0-based port, `ii=0` serves both as the "not started" flag and as a valid index: if the first nonzero is at i=0, forward substitution never starts. The loop `j < (i-1)` should be `j < i`. INVM gives wrong inverses. INVM/LUBKSB are currently never called (latent). Function: `LUBKSB()`. | Use `ii=-1` as the flag and `for (j=ii; j<i; j++)` | [ ] |
| SS3-44 | 165, 174-177 | leak / div0 | M/L | `VV` is never freed, and on a singular row `VV[i] = 1.0/big` is computed with big==0 after only a printf. Latent (unused). Function: `LUDCMP()`. | `free1d(VV)`. Return or quit on a singular matrix | [ ] |


#### atSS3Link/atSS3Link.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SS3-45 | 236, 248, 255, 264 | year value as index | H/L | `iy` runs over the calendar years `HistYrMin..HistYrMax-1` and is used directly as the year index into `SpawnBio/CatchData/DiscData` (extent nYears, atAssessTierSetup.c:115/161/206), which is OOB. It also skips HistYrMax. Latent: WriteHistory is never called. Function: `WriteHistoryFile()`. | Loop `iy = 0..nPast` and print `HistYrMin+iy` | [ ] |
| SS3-46 | 241, 254, 263 | wrong variable | H/L | The `Nfleets` used in the catch and discard loops is left over from the *last* species of the biomass loop, not the current `groupIndex`. Latent. Function: `WriteHistoryFile()`. | Recompute Nfleets per groupIndex in each loop | [ ] |
| SS3-47 | 240, 252, 261 | NULL deref | M/L | The filter `isFished \|\| isImpacted` includes fished species that are not assessed, but RBCspeciesArray arrays are only allocated when `assess_flag \|\| isImpacted` (atAssessTierSetup.c:63). The fprintf in 248 also repeats "iy region" for every species. Latent. Function: `WriteHistoryFile()`. | Filter on assess_flag/isImpacted. Print iy/region once per row | [ ] |
| SS3-48 | 64-65, 102, 152 | error handling | L/L | `fork()` returning -1 is treated as the parent. `spawn("./child", ..., NULL)` passes a NULL argv to `execvp`, which is UB on many platforms. Functions: `spawn()`, `WriteSSFiles()`. | Handle -1. Pass `{program, NULL}` as arg_list | [ ] |


#### atSS3Link/include/*.h


No issues found. Headers checked: atSS3LinkLib.h, atSS3LinkPrivate.h, atSS3Util.h. Minor: `GenMnorm` is declared in atSS3Util.h but not defined in atSS3Util.c (the real one is `Util_GenMnorm`). It is never called, so this is harmless.


### B.2.7 atphysics/

#### atphysics/aticeIO.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| PHY1-01 | 68, 707, 744, 747 | NULL dereference | H/H | `int *tsiceid;` is a file-global never allocated anywhere in the repo (grep: only uses are in this file). `Load_Ice_Timeseries()` writes `tsiceid[b] = -1` and `get_Loaded_Ice()` reads `tsiceid[box_id]`, so these write and read through a NULL pointer. Any run with `kind_ice_model = ts_file` segfaults. Functions: `Load_Ice_Timeseries()`, `get_Loaded_Ice()`. | `tsiceid = i_alloc1d(bm->nbox);` before the loop (free it in `freeIceFiles`) | [ ] |
| PHY1-02 | 730, 746 | uninitialised variable / wrong index | H/H | `char key[BMSLEN];` is never set, then `strcmp(key, this_ts->ts.varname[b])` compares garbage. It also indexes `varname[b]` (box) where it should be `varname[i]` (the series variable being looped over, `i < ts.nv`), so it can read past `nv`. The box-to-column mapping is wrong in every case. Function: `Load_Ice_Timeseries()`. | Build `key` for box b (e.g. `snprintf(key,...,"%d",b)` or whatever column naming the ts uses) and compare to `varname[i]` | [ ] |
| PHY1-03 | 771 (with 944) | logic / divide-by-zero | H/M | `get_ice_property(bm, &(bm->iceinput))` runs unconditionally at the end. (a) For `cdf_file`, `ice_cdf_init()` already called it at line 944, so the second call increments `nextrec` again and the model starts on the 2nd ice record, one step out of sync. (b) For `ts_file` (and if ice is off), `iceinput` was never opened: `nstep==0` triggers `close_iceprop` on an unopened fid, then `% propInput->nFiles` with `nFiles==0` is an integer divide by zero. Function: `Load_Ice_Timeseries()`. | Delete line 771 (or move it into the `cdf_file` case and drop the call in `ice_cdf_init`) | [ ] |
| PHY1-04 | 454-455 | type mismatch | H/M | `value = bm->tinfo[i].inice; ncattput(fid, vid, "inice", NC_LONG, 1, &value);`: `value` is a `doubleINPUT`, but NC_LONG makes netCDF read the buffer as a 4-byte int. That writes the low half of the double's bit pattern, which is 0 for 1.0 on little-endian. Ice-only tracers get `inice=0` in output and restart files, and `readBMIceInfo` (line 352) reads that back for particulate tracers. Every other int attribute here passes `&bm->tinfo[i].<field>`. Function: `writeBMIceInfo()`. | `ncattput(fid, vid, "inice", NC_LONG, 1, &bm->tinfo[i].inice);` | [ ] |
| PHY1-05 | 1319-1337 | logic / reset in wrong loop | M/M | `ice->wc_layer_match[i] = bm->boxes[b].nz - 1;` (the default) runs inside the outer water-layer loop `z`. On each new `z` the inner loop goes over ice layers from 0 again and resets the layers already matched to a higher `z` back to `nz-1`, because their `last_depth < icedepth` test now fails. So any ice layer that maps to a water layer between 1 and nz-2 ends up recorded as the surface layer. Function: `Enter_IceBM()`. | Set the default once per box, before the `z` loop | [ ] |
| PHY1-06 | 1249-1295, 1322-1330 | stale state | M/M | When ice is present, only layers `0..z-1` of `ice->ice_classes` are written. Layers `z..icenz-1` keep their values from the previous time step when ice thins, and nothing zeroes them. The mapping loop then adds `ice_classes[i][k]` over all `icenz` layers into `ice_prop`, so stale ice fractions get counted. Function: `Enter_IceBM()`. | After filling, zero `ice_classes[zz][*]` for `zz >= z` | [ ] |
| PHY1-07 | 809 | format/arg mismatch | H/L | `quit("read%sTimeSeries: Can't allocate memory for %s time series list\n", tsname);` has two `%s` but only one argument, which is undefined behaviour on this error path. Function: `Ice_Read_Time_Series()`. | Add a second `tsname` argument | [ ] |
| PHY1-08 | 819 | uninitialised read | H/L | `fprintf(..., fts[i].iceclass)` prints a field of a `malloc`ed array that is never set, here or anywhere else. Function: `Ice_Read_Time_Series()`. | Set `fts[i].iceclass = i;` before printing | [ ] |
| PHY1-09 | 629 | format specifier | H/L | `quit("... code compiled with %d\n", ..., sizeof(doubleINPUT))`: `%d` is given a `size_t`. Function: `writeBMIceData()`. | `%zu` or cast to `(int)` | [ ] |
| PHY1-10 | 1203-1207 | dangling pointer | M/L | `close_iceprop` frees `dzvaluebuf` but does not NULL it. `tbuf` and `valuebuf` are NULLed. A second close (for example `freeCDFIceFiles` after a failed reopen) would double-free. Function: `close_iceprop()`. | Add `propInput->dzvaluebuf = NULL;` | [ ] |
| PHY1-11 | 536-547, 370, 516, 534, 537, 542 | dead branch / debug output | H/L | `icestart[]` and `start[]` are identical, so the `insed\|\|inwc` if/else branches do the same thing. Leftover unconditional `printf` debug lines ("in gere", every ice dz, currentnz per box) flood stdout. Functions: `readBMIceData()`, `readBMIceInfo()`. | Drop one branch; guard or remove the printfs | [ ] |


#### atphysics/attempsalt.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| PHY1-12 | 497-499 | invalid free | H/H | `if(propInput->input_Varname) c_free1d(propInput->input_Varname);`: `input_Varname` is declared `char input_Varname[BMSLEN]` in the struct (atlantisboxmodel.h:3997). The test is always true, and `free()` gets a pointer into the middle of `bm` or the `forceTracerInput` malloc block. At shutdown (`freePhysics` -> `freeTempSalt`) this aborts or corrupts the heap for every active property input (temp, salt, pH, forced tracers, ...). This code is new in the uncommitted changes. Function: `free_PhyPropertyData()`. | Delete these 3 lines | [ ] |
| PHY1-13 | 996 | wrong variable (copy-paste) | H/M | The sediment loop accumulates `inputData->total_input += ((new_value - cur_value) * bp->dz[k]);`, using the water-column `dz`. It should use `bp->sm.dz[k]`, as `resetForcingBM` line 1148 does. `total_input` feeds the contaminant mass checks (atContaminants.c:2173-2250). Function: `tracerForcingBM()`. | `bp->sm.dz[k]` | [ ] |
| PHY1-14 | 999, 1011, 1154, 1165 | format/arg order | H/L | `quit("... %s in %d:%d at time %e ...", b, k, bm->dayt, inputData->variableName, ...)`: `%s` receives an int, so this crashes instead of printing the error message. In the 1-D branches (1011, 1165) `k` is also stale. Functions: `tracerForcingBM()`, `resetForcingBM()`. | Pass `variableName` first: `(..., inputData->variableName, b, k, bm->dayt, value)` | [ ] |
| PHY1-15 | 1122, 1148 | wrong quantity | M/M | When a reset fires, `total_input += new_value * dz`, which adds the whole new concentration. `tracerForcingBM` adds the actual change `(new_value - cur_value) * dz`. The reset path therefore overstates the mass added, which inflates the contaminant budget tolerance. Function: `resetForcingBM()`. | `(new_value - cur_value) * dz` | [ ] |
| PHY1-16 | 64-66, 155-163, 191-193 | uninitialised sentinel | M/M | Only `tempid`, `saltid` and `pHid` are set to -1. `windid`, `noiseid` and `lightpid` (and the matching `*input.tracerID` checked in `init_PhyPropertyData` line 303) never are, so the "no windid/noiseid/lightpid set" quits never fire. A missing tracer is silently treated as tracer 0 (or garbage). Function: `tempsalt_init()`. | Set `bm->windid = bm->noiseid = bm->lightpid = -1;` and the `tracerID`s to -1 at the start | [ ] |
| PHY1-17 | 471-480 | config backward-compat | M/M | The new keys `<name>_ignore_units` and `<name>_input_Varname` are read with `set_keyprm_errfn(quit)` in force (set at line 356), so any existing forcing prm file without them now aborts. The new code comment calls `ignore_units` optional behaviour. Function: `init_forceTracers()`. | Wrap these reads with `set_keyprm_errfn(warn)` and default `input_Varname` to the Atlantis name | [ ] |
| PHY1-18 | 888-890 | wrong array in message | H/L | The epibenthic unit check compares against `bm->einfo[tracerID].units`, but the error message prints `bm->tinfo[tracerID].units`. Function: `open_phyprop()`. | Print `bm->einfo[...]` in that branch | [ ] |
| PHY1-19 | 822 | buffer sizing | L/L | `bytes = sizeof(double)*nbox*wcnz`, but each buffered step holds `nbox*(wcnz+sednz)` values (line 830), so the cache can exceed MAXBUFMEM. Function: `open_phyprop()`. | Use `wcnz + sednz` | [ ] |


#### atphysics/atphysics.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| PHY1-20 | 700-702 | out of bounds (debug leftover) | H/H | `if (!_finite(newwctr[24][0][Salinity_i]))` hard-codes box 24 and runs every time step. `newwctr` has `nbox` rows (alloc3d(ntracer,wcnz,nbox)), so any model with fewer than 25 boxes reads past the row-pointer array and will likely segfault. Function: `physics()`. | Delete these debug lines | [ ] |
| PHY1-21 | 573 | wrong variable | H/M | `if (lndreclaim[b] < bm->tstop)`: `lndreclaim` is the key string `"land_reclaim"`, so this compares a character code with `tstop` and reads past the literal when `b >= 13`. It should test the reclaim day just read. Function: `initPhysics()` (only with `terrestrial_on`). | `if (dcay_scalar[b] < bm->tstop)` (i.e. `land.day_reclaim`) | [ ] |
| PHY1-22 | 547, 552, 561 | truncation | M/M | `bm->ship_mixing_rate[b] = (int) (ship_info[b]);` (likewise `vessel_size`) casts to int before storing into double arrays (`Util_Alloc_Init_1D_Double`). Fractional mixing rates such as 0.3 become 0. Function: `initPhysics()` (simple industry model). | Drop the `(int)` casts | [ ] |
| PHY1-23 | 558-562 | copy-paste duplicate | M/L | `ship_mixing_rate` is read a second time with the same key, `ship_mixing`, into the same array (counter set to `nbox`, loop over vessel sizes). It is redundant and was probably meant to read a different parameter. Function: `initPhysics()`. | Remove, or replace with the intended key | [ ] |
| PHY1-24 | 610-611 | index -1 | M/M | `this_nz = nz - 1; base_cell_vol += area * dz[this_nz];`: boxes with `nz == 0` (dry/island boxes, see atvertgeom.c:154 and :613) read `dz[-1]`, which is the previous box's last sediment cell or memory before the allocation. The average also divides by all boxes, including those. Function: `initPhysics()`. | Skip boxes with `nz < 1` and count only wet boxes | [ ] |
| PHY1-25 | 284 | wrong operator | L/L | `if (bm->tidal & (bm->wc_dz_tol < 1.0))` uses bitwise `&`. It works only while `tidal` is 0/1. Function: `initPhysics()`. | `&&` | [ ] |


#### atphysics/atphysIO.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| PHY1-26 | 645, 656, 667, 677, 687, 697 | ineffective optional check | M/L | These optional variables (`coastal`, `hdsource`, `hdsink`, `eflux`, `vflux`, `porosity`) are looked up with `ncopts = NC_VERBOSE \| NC_FATAL`, so a missing variable exits inside `ncvarid` before `if (id >= 0)` is reached. The `dz` lookup at 536-538 does this correctly by setting `ncopts = 0` around it. Function: `readBMphysData()`. | Set `ncopts = 0` around each optional `ncvarid`, then restore | [ ] |
| PHY1-27 | 398 vs 917 | condition mismatch | L/L | `coastal` is defined when `flagIncludesCoastal > 0` but written when `flagIncludesCoastal` is non-zero. A negative value would write to an undefined variable, which is fatal. Functions: `writeBMphysInfo()`, `writeBMphysData()`. | Use the same test in both | [ ] |


#### atphysics/atgeomIO.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| PHY2-01 | 190-223 | wrong counting | H/M | `bm->nboundary++` runs in three separate `if` blocks that are not mutually exclusive. A box that fails `pointinpoly` (counted once) and also has `botz == 0.0` (or `botz > 0` with land off) gets counted two times. `nbox - nboundary - nland` is then used as a divisor for ecological indices (atecology/atbiolmessage.c:982-994) and as `ncells` (atmanage/atManage.c:1382), so those averages come out wrong and the divisor could even reach 0 or go negative. Also, a box outside the polygon with `botz > 0` and terrestrial on is counted in both `nboundary` and `nland`. Function: `readMSEBoxModelGeom()`. | Count once after classification, e.g. only increment when `is_boundary[i]` changes from 0 to 1, and decrement nboundary if the box is later set to LAND | [ ] |
| PHY2-02 | 769-774 | wrong pointer | M/L | `valueStr = strstr(str, "="); return atof(valueStr);` passes a string that starts with `=` to atof, so it always returns 0.0. Currently has no callers (latent). Function: `getProjectionValue()`. | `return atof(valueStr + 1);` | [ ] |
| PHY2-03 | 795-796 | unchecked return | M/L | `fgets(buf, buflen, fp)`'s return value is not checked. If fgets fails, `buf` is uninitialised and `strcmp(buf,"")` reads garbage. Function: `parseProjectionString()`. | `if (fgets(buf, buflen, fp) == NULL \|\| buf[0]=='\0') quit(...)` | [ ] |


#### atphysics/attracerIO.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| PHY2-04 | 223-244 | stale variable | H/M | `ncattget(fid, NC_GLOBAL, "icenz", &n); if(n!= 0)` runs with `ncopts=0`. When the attribute is missing, ncattget fails without writing to `n`, so `n` keeps the `sednz` value read at line 227 (non-zero). So `ice_on` is set TRUE for every model that has no ice. Then every tracer goes through the `inice` read path, and `boxes[b].ice.ntr` is set to ntracer (nice++ for every tracer). In atlantismain.c:1125 this triggers `readBMIceInfo`, which only partly undoes it: it sets ice_on FALSE but `num_active_habitats = ICE_BASED` (4), not the `EPIFAUNA+1` (3) intended here. Function: `readBMTracerInfo()`. | Set `n = 0;` before the icenz ncattget, or use `ncattinq(...)!=-1` as in aticeIO.c:117 | [ ] |
| PHY2-05 | 590, 682, 721 | accumulator not reset | H/L | `found_partial_match_n` is set to 1 but never reset per tracer (only `found_partial_match` is reset at 658). Once one tracer matches `_N`, every later unmatched tracer triggers the "no string matches found" warning. Function: `readBMTracerData()`. | Add `found_partial_match_n = 0;` next to line 658 | [ ] |
| PHY2-06 | 821 | format specifier | H/L | `quit("... %d", ..., sizeof(doubleINPUT))` passes a size_t to `%d`. The same pattern is at atagetracerIO.c:243 and 517. Function: `writeBMTracerData()`. | Use `%zu` or cast to `(int)` | [ ] |


#### atphysics/atsourcesink.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| PHY2-07 | 201-225 | duplicated logic / double application | H/H | In the no-water branch, `nut_scale = Get_PSS_Scalar(bm, s, variableIndex);` is followed by a copy of Get_PSS_Scalar's own loop, which multiplies `nut_scale` again by the same `pssMult` factors. The scaling is applied twice (mult² after the change period, and the ramp is squared). The water branch (line 159) only calls Get_PSS_Scalar. The copied block also tests `bm->t == pssStart` (days) without `*86400` (line 218). Function: `sourceSink()`. | Delete the loop at lines 203-225 and keep only the `Get_PSS_Scalar` call | [ ] |
| PHY2-08 | 209, 508 | wrong index | M/M | `strcasecmp(pss->ts.varname[variableIndex], "Si")` uses `variableIndex`, which counts only valid non-time, non-water tracer columns. `ts.varname` is indexed by the raw time-series column `i` and includes the time column (sjwlib/timeseries.c:146-156, pointsourcesink.c:165). So the Si exemption checks the wrong column, usually shifted by one because time is column 0, and Si can get scaled while the column before it is exempted. Function: `sourceSink()`, `Get_PSS_Scalar()`. | Pass the ts column index `i` to the Si test (e.g. add a parameter to Get_PSS_Scalar) | [ ] |
| PHY2-09 | 706 | format specifier | H/L | `quit("read_bm_ts: Can't open %%s\n", bm->inputFolder, bm->forceIfname)`: the `%%s` prints a literal "%s" and both arguments are ignored. The function name in the message is also wrong. Function: `read_bm_ts_rewind()`. | `quit("read_bm_ts_rewind: Can't open %s%s\n", ...)` | [ ] |


#### atphysics/atvertgeom.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| PHY2-10 | 699-704, 596-597 | loop direction / wrong layer | H/H | `for(kk=k; kk<ice->currentnz; kk++) copy_ice_layer(ice,kk,kk+1);` shifts forward in ascending order, so every layer k..currentnz ends up as a copy of layer k and the deeper ice layers are lost. Then `clear_ice_layer(ice,k)` empties layer k, and the caller `check_ice_dz` does `transfer_ice(ice,k,k+1,maxdz-mindz,...)` from the now-empty layer k. That trips `a > ice->dz[src_k]` → quit("Too much material"). As written, the thick-ice splitting path cannot work. Function: `insert_ice_layer()`, `check_ice_dz()`. | Loop `for(kk=ice->currentnz-1; kk>=k; kk--)`, then transfer from k+1 into k (or clear k+1 and transfer k→k+1) | [ ] |
| PHY2-11 | 728 | off-by-one | H/M | `clear_ice_layer(ice, ice->currentnz); ice->currentnz--;` clears one slot past the last active layer. When `currentnz == nz`, clear_ice_layer quits ("Invalid index"). Otherwise the stale duplicate in slot `currentnz-1` is left behind. Function: `delete_ice_layer()`. | `clear_ice_layer(ice, ice->currentnz - 1);` | [ ] |


#### atphysics/atagetracerIO.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| PHY2-12 | 465-522 | loop nesting / stale buffer | H/H | `ncvarput(fid, vid, start, count, val)` (whole-nbox record) is called inside `for b` → `for nf`, after only `val[b]` has been set. The last write for each fishery happens at b = nbox-1. At that point `val[0..nbox-2]` hold the values from the last flagged fishery computed for those boxes, not this fishery's. So every fishery except the last flagged one is written with another fishery's per-box catch/discards in all boxes except the last. Function: `writeBMAnnAgeCatData()`. | Restructure: for each propit/nf, loop b to fill `val`, then call ncvarput once after the box loop | [ ] |
| PHY2-13 | 109 & 341 vs 193 & 433 | loop bound mismatch | H/L | The Info routines define `numCohortsXnumGenes * numStocks * numAges` variables, but the Data routines loop only `numCohortsXnumGenes * numAges`. When numStocks > 1 the extra variables are defined but never written (left as fill values), and the names suggest age classes that don't exist. Function: `writeBMAnnAgeBioInfo()`, `writeBMAnnAgeCatInfo()`. | Drop `* numStocks` from the Info loops (or add a stock loop to the Data routines) | [ ] |
| PHY2-14 | 227-234 | wrong weighting | M/M | Weight at age is `sum_k N*w*boxPopRatio / sum_k N`. The numerator is weighted by this age class's population fraction but the denominator is not, so the reported individual weight is scaled down by that fraction and is not a true mean weight. Function: `writeBMAnnAgeBioData()`. | Use `tot += N * boxPopRatio[b][k][cohort][ai]` (or drop boxPopRatio from the numerator) | [ ] |
| PHY2-15 | 476, 504 | inconsistent check | M/L | In `writeBMAnnAgeCatData` the species filter lacks the `isFished \|\| isImpacted` test used in `writeBMAnnAgeCatInfo` (335). If `FisherySpeciesCatchFlags[sp][nf]` is TRUE for a species with neither flag set, `ncvarid` hits an undefined variable and is fatal under NC_FATAL. Function: `writeBMAnnAgeCatData()`. | Apply the same species filter in both routines | [ ] |


#### atphysics/attransport.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| PHY2-16 | 398-401 | leftover debug / out-of-bounds | H/M | This debug block is live: `if(bm->dayt > 1090.0 && strcmp(name,"salt")==0) fprintf(llogfp, ..., newwc[23][1][Salinity_i], ...)`. It hard-codes box 23 / layer 1, which reads out of bounds in models with fewer than 24 boxes or 2 layers. It also writes and flushes one log line per box-layer per transport step after day 1090. Function: `transportBM()`. | Delete or comment out the block, like the neighbouring debug blocks | [ ] |
| PHY2-17 | 207-216 | dead condition | M/L | `if ((bb < 0) \|\| (kk < 0))` sits inside `if (bm->hd.b[b][k][d] >= 0 ...)`, so `bb < 0` can never be true. Only invalid `kk` is caught. Destinations with `kk >= boxes[bb].nz` (valid < wcnz but below the box bottom) are not caught either (line 251 only checks `kk >= wcnz`). Function: `transportBM()`. | Check `kk >= bm->boxes[bb].nz` as well | [ ] |


#### atphysics/athydromod.c


No issues found. Functions checked: hydro_init, get_hydro, open_hydro, close_hydro, closeHydroFinal.


#### atphysics/atsummaryIO.c


No issues found. Functions checked: createBMSummaryDataFile, writeBMSummaryInfo, writeBMSummaryData, writeBMSummaryEpiData. (Only the `%d`-for-`sizeof` format nit at lines 310/408, covered in Lower-confidence notes.)


#### atphysics/atparamIO.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| PHY3-01 | 105 | buffer size / wrong sizeof | H/H | `snprintf(convertedXMLFileName, sizeof(convertedXMLFileName), "%s", biologyfile)`: `convertedXMLFileName` is a `char *` from malloc (line 89), so `sizeof` gives the pointer size (8). The name is cut to 7 characters, so a run file supplied directly as `.xml` cannot be opened. Function: `Util_Read_Run_XML()`. | Use `fileSize` (as on line 96) | [ ] |
| PHY3-02 | 87-98 | buffer overrun / NULL deref | H/M | `char biologyfile[120]; strcpy(biologyfile, bm->runprmIfname)`, but `runprmIfname` is `char[BMSLEN]` (512, atlantisboxmodel.h:4382), so a long path overruns the stack buffer. Line 97 `*(strstr(convertedXMLFileName, ".prm")) = '\0'` dereferences NULL if the name has neither ".xml" nor ".prm". `fileSize` comes from `fileName` while the copied string is `runprmIfname` (only equal because of how the caller calls it, atlantismain.c:1083). Function: `Util_Read_Run_XML()`. | Make the buffer BMSLEN, use `fileName`, check strstr for NULL | [ ] |
| PHY3-03 | 234-236 | uninitialised / missing read | M/M | `bm->track_contaminants` (read commented out at 234) and `bm->track_wind` (never read here) are only set inside the converter, ConvertAtlantis/atRunXML.c:102,127. If the run file is given as `.xml`, the converter is skipped, so contaminants and wind can never be switched on. Function: `Read_Run_Parameters()`. | Read `track_contaminants` and `trackWind` in Read_Run_Parameters | [ ] |
| PHY3-04 | 253-279 | memory leak | H/L | The first `values` array from `Util_XML_Read_Array_String` (names) is never freed (`c_free2d` is commented out at 266) before `values` is reused for the units. Function: `Read_Run_Parameters()`. | Uncomment `c_free2d(values)` at line 266 | [ ] |


#### atphysics/atepiIO.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| PHY3-05 | 224-227 vs 139,173-182 | inconsistent metadata | M/M | `writeBMEpiInfo` defines `is_boundary(t,b)` (NC_INT) with `bmtype="epibenthos"` but no `dtype`/`sumtype` attributes. If an output file is later read back (e.g. restart from output.nc), `readBMEpiInfo` picks it up through `ncvarfind(...,"bmtype","epibenthos")`. The `ncattinq(...,"dtype")` call then fails, and with NC_FATAL set the run exits. Even if it did not, the variable would be read as a double epi tracer. Functions: `writeBMEpiInfo()`, `readBMEpiInfo()`. | Give is_boundary a different bmtype (e.g. "phys") or add dtype/sumtype and skip it on read | [ ] |


#### atphysics/atbioirrig.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| PHY3-06 | 60,69,78,87 | hard-coded index / out of bounds | H/H | `if (!_finite(newwc[24][0][Salinity_i]))`: leftover debug code with box 24 hard-coded, run for every box. Any model with `nbox <= 24` reads past the `newwc` pointer array (UB / segfault). Otherwise it checks the wrong box. Function: `bioirrig()`. | Delete these checks or use `newwc[b]` | [ ] |
| PHY3-07 | 195-202 | unit mismatch | M/M | Oxygen case: `pvol = sm->porosity[k] * (oxvol / sm->volume[k]) + small_num` is a dimensionless ratio, not a pore-water volume in m3. It is then used as a volume in `ek = rate*(1/wvol+1/pvol)` and `ce = (...*wvol + ...*pvol)/(wvol+pvol)`. Oxygen exchange is therefore hugely over-damped and mass is not conserved. Compare the non-oxygen branch at line 235: `porosity*volume`. Function: `exchange()`. | `pvol = sm->porosity[k] * min(oxvol, sm->volume[k]) + small_num` | [ ] |


#### atphysics/atLandIO.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| PHY3-08 | 37-46 | NULL dereference | H/L | If `tracerName` does not contain "Land_", `name` stays NULL and is passed to `strcmp` (crash). If "Land_" appears but not at the start, the offset is wrong. The function is currently unused (latent). Function: `Find_WC_Tracer()`. | `name = (strncmp(tracerName,"Land_",5)==0) ? tracerName+5 : tracerName;` | [ ] |
| PHY3-09 | 276 | copy-paste log text | H/L | `"Entering writeBMEpiData\n"` printed in `writeBMLandData()`. | Change to writeBMLandData | [ ] |


#### atphysics/atindex.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| PHY3-10 | 72-76 | out of bounds / check order | H/M | `b = xytobox(...); kk = wczbtok(z, &this_bm->boxes[b]);` runs before the `b < 0` check. A point source outside all boxes gives `boxes[-1]`, and `gridz` is then dereferenced. Called via sourcesink (atsourcesink.c:546). Function: `ext_xyztoijk()`. | Return -1 if b<0 before calling wczbtok | [ ] |
| PHY3-11 | 246-259 | swapped return values | H/L | `wcsurface()` returns `gridz[0]` and `wcbottom()` returns `gridz[nz]`, but water-column `gridz[0] = botz` (atvertgeom.c:260), so the two are swapped. Currently unused (latent). | Swap the return values | [ ] |


#### atphysics/atdiagIO.c


No issues found. Functions checked: freeBMDiagInfo, writeBMDiagInfo, writeBMDiagData (readBMDiagInfo is commented out).


#### atphysics/atfishstatIO.c


No issues found. Functions checked: readBMFisheriesInfo, freeBMFisheriesInfo, writeBMFisheriesInfo, writeBMFisheriesData.


#### atphysics/atdietIO.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| PHY3-12 | 117,197,208,212,219 | cohort×genotype overrun | H/H | The loop runs `cohort < numCohortsXnumGenes * numStocks` and indexes `bm->totDiet[b][k][sp][cohort][prey]`. totDiet is allocated `alloc5d(K_num_tot_sp, K_num_max_cohort, ...)` (atlantismain.c:1176), and `K_num_max_cohort` is the max of `numCohorts` only (atUtilGroupIO.c:302). Any group with numGeneTypes>1 or numStocks>1 overruns the cohort dimension. ReInitDietData (atbiology.c:1477) has the same overrun. Functions: `writeBMDietInfo()`, `writeBMDietData()`. | Allocate the cohort dim as max(numCohortsXnumGenes*numStocks) | [ ] |


#### atphysics/atvmix.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| PHY3-13 | 135-139, 161-165 | mass conservation (updated value reused) | H/M | `c[k]` is overwritten first, then `c[k-1] = ((lvol-cvol)*c[k-1] + cvol*c[k])/lvol` uses the *new* `c[k]`. The total changes by `cvol*(c[k]'-c[k])`, so mass is not conserved in either branch. Function: `vertical_mixing()`. | Save `ck = c[k], ck1 = c[k-1]` before updating and use the originals | [ ] |
| PHY3-14 | 142-143,168-169 | accumulator inside tracer loop / sign | M/L | `bp->vflux[k] += cvol` sits inside the per-tracer loop, so the water-flux diagnostic grows with the number of dissolved tracers. The Oxygen branch also flips the sign (`vflux[k-1] += cvol`). vflux is written to output (atphysIO.c:824). Function: `vertical_mixing()`. | Accumulate vflux once per box/layer, outside the tracer loop, with a consistent sign | [ ] |


#### atphysics/atdeposition.c


No issues found. Functions checked: deposit. (See Lower-confidence notes on the maintain_sedlayer clamp.)


#### atphysics/atsettle.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| PHY3-15 | 92 | wrong condition (copy-paste) | H/H | `if( bm->boxes[b].type != BOUNDARY && bm->boxes[b].type)`: BOX_TYPE has `INTERNALBOX = 0, BOUNDARY, LAND` (atlantisboxmodel.h:3597). The test is true only for LAND boxes, so buoyant tracers (svel+xvel > 0) rise only in land boxes and never in normal water boxes. The sinking branch (line 112) correctly uses `!= LAND`. Function: `settleBMwc()`. | `... && bm->boxes[b].type != LAND` | [ ] |


#### atphysics/athdiff.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| PHY3-16 | 56-65 | out of bounds / misuse of diffusion1d | H/M | `diffusion1d(nnconn, hc, hx, hk, hx, ...)` needs `k` and `xk` arrays of n+1 values (sjwlib/diffusion.c:70-73). `hk`/`hx` are allocated `max_nconn` (atphysics.c:225-226), so when nconn==max_nconn they are read past the end. Also: (a) `k[0]` and `k[n]` are boundary *fluxes* but are set to `wc_kz` (atphysics.c:277), which injects spurious mass `dt*k[0]/dx`; (b) the same `hx` is passed as both cell and interface coordinates, and neighbour x-coords are not sorted, so dx can be negative; (c) box b itself is excluded and neighbours with `nz<=k` are read. Active when `horiz_diffusion==1`. Function: `hdiffBMwc()`. | Rewrite (e.g. reuse the filler_hdiffBMwc approach); at minimum size arrays n+1 and zero the end fluxes | [ ] |


#### atphysics/atbioturb.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| PHY3-17 | 102-119,150-166,196-213 | operation repeated per tracer | H/H | `transfer_sed(sm,k,dst,thick)` moves the whole sediment slab (dz and *all* tracers, atvertgeom.c:398-417), but it is called inside `for(n...)` over tracers. The bulk transfer therefore runs once for every qualifying particulate tracer, multiplying bioturbation by roughly the number of such tracers. Functions: `partic_diff()`, `expul()`, `mix_in()`. | Drop the tracer loop and call transfer_sed once per layer | [ ] |
| PHY3-18 | 98,143,189 | unit mismatch (s vs days) | H/H | `thick[k] = ... * bm->dt * 86400.0`: the comment assumes dt is in days, but `bm->dt` is in seconds (read via Util_XML_Get_Value_Time, atparamIO.c:178; bioirrig uses `bm->dt` directly). Transfer thicknesses are 86400x too large. Functions: `partic_diff()`, `expul()`, `mix_in()`. | Remove `* 86400.0` | [ ] |
| PHY3-19 | 59-65 (all routines) | changes lost (wrong storage) | H/M | `transfer_sed` changes `sm->tr` (= `bm->sedtr[b]`, atlantismain.c:1191), not the `newsed` working copy passed in. atlantismain.c:575 then copies `newsedtr` back over `sedtr`, so bioturbation's tracer redistribution is thrown away while the layer-thickness changes persist. Function: `bioturb()`. | Work on newsed (or sync newsed into sm->tr before and back after) | [ ] |
| PHY3-20 | 90-118 | off-by-one / missing init | M/M | `thick[k]` is only set for `k = tk+1..nz-1`. The loop at 114 uses `thick[tk]` (always 0) for the `tk -> tk+1` transfer, so the top layer never diffuses downward but does receive from below, giving net upward pumping. `if(k<sm->nz)` at 115 is always true. Function: `partic_diff()`. | Compute thick[tk] too, or use the interface value thick[k+1] for k->k+1 | [ ] |


#### atphysics/atsuspension.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| PHY3-21 | 153-156 | missing braces | H/H | `else if (insed && dissol && can_be_moved)` has no braces, so line 156 `newsed[b][tk][n] -= newsed[b][tk][n]*pwv/(...)` runs for **every** tracer. Particulates are depleted twice, and non-sediment and non-movable tracers lose mass that goes nowhere. Function: `resuspendBM()`. | Wrap lines 155-156 in braces | [ ] |
| PHY3-22 | 146,156,164-165 | erosion counted twice | M/H | Sediment tracers are per volume (summary output multiplies by sm.dz, atsummaryIO.c:346). After moving `c*ervol` to the water column, the code lowers the concentration (`c *= 1-erdz/dz`) **and** cuts the layer thickness by `erdz`. The remaining mass is `c(1-erdz/dz)(dz-erdz)A`, not `c(dz-erdz)A`, so mass is lost. Function: `resuspendBM()`. | Drop the concentration adjustment (thickness change already accounts for it) | [ ] |
| PHY3-23 | 66,100 | division by zero | M/M | `Shipping_stress_ratio = stress / init_stress`, and `init_stress` is set to `nxsbs` each step. If the stress was 0 in the previous step (stress file value 0, or `soft==1`) this gives inf/NaN. Function: `resuspendBM()`. | Guard: ratio=1 if init_stress<=0 | [ ] |
| PHY3-24 | 46,108-120 | logic: loop variable not set | M/M | `nxsbs` is only set when `dynamic_stress` is on, but `erosion_rate` uses the stored `boxes[b].stress`. With `dynamic_stress==0` the loop `while(nxsbs > 0.0 ...)` never runs, so there is no resuspension at all, even with valid stress values. Function: `resuspendBM()`. | Loop on `er_limited > 0` (or set nxsbs = stress) | [ ] |


#### atphysics/atsaturation.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| PHY3-25 | 44,73,89 | uninitialised variable | H/L | `do_break++` runs on an uninitialised `int do_break` (UB). The value is reset before use at line 98, so the result is harmless. Function: `Saturation_Check()`. | Initialise do_break=0 | [ ] |


#### atphysics/atswr.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| PHY3-26 | 56 | format/argument mismatch | H/L | `quit("%s in %d at time %e is not finite %e\n", b, bm->dayt, inputData->variableName, bp->swr)`: an int is passed for `%s` and a double for `%d`, so the error path crashes. Function: `swrForcingBM()`. | Reorder args: `inputData->variableName, b, bm->dayt, bp->swr` | [ ] |
| PHY3-27 | 80,86,96 | buffer overrun | M/L | `char buf[200]` is filled by `readkeyprm_s(fp,"Solar_radiation",buf)`, while the caller `readSolar` uses `buf[STRLEN]` (2000) for the same key. A long path overflows. Function: `init_SolarData()`. | Use `char buf[STRLEN]` | [ ] |


#### atphysics/atreadbm.c


No issues found that affect live code. Functions checked: readMSEBoxModel, freeMSEBoxModel (both unused; see notes).


#### atphysics/atgas.c


No issues found. Functions checked: gasExchange.


#### atphysics/atsedprops.c


No issues found. Functions checked: sedprops, calc_por, calc_css, calc_er.


#### atphysics/atvdiff.c


No issues found. Functions checked: vdiffBMwc.


#### atphysics/atboundary.c


No issues found. Functions checked: boundaries.


#### atphysics/atdecay.c


No issues found. Functions checked: decayBM.


#### atphysics/atprofile.c


No issues found. Functions checked: bioprofile.


#### atphysics/atPhysicsModule.c


No issues found. Functions checked: allocatePhysicsModule, closePhysicsFile, freePhysicsStruct.


#### atphysics/include/*.h


No issues found. Files checked: atPhysics.h, atPhysicsModule.h.


### B.2.8 atlantisUtil/

#### atlantisUtil/atUtilXML.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| UTIL-01 | 702 | wrong operator / always-false | H/M | `if ((*p)[i] < 0.0 && (*p)[i] > 1.0)`: a value can't be both <0 and >1, so the range check in `probability_check` never fires. Only the sum test runs, so negative entries that are offset by entries >1 still pass. The `proportion_check` case just above (line 692) correctly uses `\|\|`. Function: `Check_Input_Array()`. | Change `&&` to `\|\|`. | [ ] |
| UTIL-02 | 1217-1222 (with 1152-1163) | wrong return-value test | H/M | `Util_XML_Get_Node_Value_Double` returns -1 when the value attribute is missing and 1 on success. The caller tests `if(!Util_XML_Get_Node_Value_Double(...))`, but `!(-1)` is 0, so a missing value is never caught. `value` stays 0, `Check_Value` runs on 0, and the function returns TRUE even when `isRequired` is TRUE. Function: `Util_XML_Get_Value_Double()`. | Use `if (Util_XML_Get_Node_Value_Double(...) < 0)`. | [ ] |
| UTIL-03 | 1305-1310 | NULL dereference | H/M | When the value attribute is missing and `isRequired == FALSE`, `value` is NULL and execution falls through to `strcpy(returnValue, value)`, which crashes. Function: `Util_XML_Get_Value_String()`. | If `value == NULL` (not required), return FALSE before the strcpy. | [ ] |
| UTIL-04 | 1360-1362 (with 2184-2225) | wrong return-value test (dead check) | H/M | `skipToKeyEndNoRewind` returns 1 when it finds the key again (a duplicate) and 0 when it doesn't; it never returns a negative. So `if(skipToKeyEndNoRewind(infile,varstr) < 0)` is never true and duplicate parameters in the .prm file are never detected during conversion. Function: `Util_XML_Parse_Create_Node()`. | Change `< 0` to `> 0`. | [ ] |
| UTIL-05 | 1359, 1379-1383, 1391 | error handler silenced / NULL deref | H/M | `set_keyprm_errfn(quiet)` is set for the duplicate check but only restored at 1391, after the missing-value checks. `keyprm_errfn(...)` at 1380/1382 is therefore `quiet` and doesn't stop execution, so a missing value (`valuestr == NULL`) reaches `strlen(valuestr)` and then `Util_XML_Create_Node`, which crashes. Function: `Util_XML_Parse_Create_Node()`. | Call `set_keyprm_errfn(fn)` right after the duplicate check, or use `quit` for the missing-value errors. | [ ] |
| UTIL-06 | 1601-1603 | buffer overrun (heap) | H/H | `valueCopy = malloc(STRLEN)` (STRLEN=2000, atlantisboxmodel.h:331), then `strcpy(valueCopy, valueStr)` copies the full XML array string with no length check. Long arrays (per-box arrays in large models, long prey/diet vectors) easily go past 2000 chars and overflow the heap. The copy is only used in an error message. Function: `Parse_Into_Array()`. | Use `valueCopy = strdup(valueStr)` (or malloc `strlen(valueStr)+1`). | [ ] |
| UTIL-07 | 1470, 1489-1496 | buffer truncation | H/M | Each step rebuilds the whole growing output in `char tempStr[256]` with `snprintf(tempStr, sizeof(tempStr), "%s %s", returnString, ...)` and copies it back. Once the string passes 255 chars it is silently truncated, so the rewritten array loses its later entries and has the wrong length. `returnString` itself is `size*100` bytes. Function: `Replace_String_Entry()` (used by `Util_XML_Replace_Node_Value`). | Append directly into `returnString` (strcat, or track an offset) and drop `tempStr`. | [ ] |
| UTIL-08 | 1059-1062 | NULL dereference | H/L | `if (attributeNode == NULL) returnValue = NULL;` doesn't return, and the next line dereferences `attributeNode->nodesetval`. `Execute_Xpath_Expression` returns NULL on XPath failure. Callers at 1105 and 1127 (`Util_XML_Set_Node_Value`, `Util_XML_Replace_Node_Value`) also skip the NULL check. Function: `Util_XML_Get_Node()`. | `if (attributeNode == NULL) return NULL;`, and check `nodesetval != NULL`. | [ ] |
| UTIL-09 | 815-818 | wrong free | M/L | On context-creation failure, `Execute_Xpath_Expression` calls `xmlFreeDoc(doc)` on a document it doesn't own. The caller keeps using and later frees the doc, so this is a use-after-free or double free. Function: `Execute_Xpath_Expression()`. | Remove `xmlFreeDoc(doc)`. | [ ] |
| UTIL-10 | 2169 | inverted condition | H/L | `if(strcmp((char *)node->name, AtlantisXMLObjectNAMES[i][0]))` is true when the names differ, so the function returns the name property of the first node type that doesn't match (usually NULL), not the matching one. It's used for error-message node names in atManageParamIO.c/atassessParamIO.c. Function: `Util_Get_Node_Name()`. | Use `strcmp(...) == 0`. | [ ] |
| UTIL-11 | 1590 (with 1510) | swapped arguments | H/L | `Parse_Into_Int_Array(key, fileName, values, ...)`, but the signature is `(char *fileName, char *paramName, ...)`. Every error message therefore swaps the file name and the parameter name. Function: `Util_XML_Read_Array_Integer()`. | Call `Parse_Into_Int_Array(fileName, key, values, size, entry_type)`. | [ ] |
| UTIL-12 | 1547-1548, 1556 | format specifier mismatch | H/L | `%e` is given `values[i]`, which is an `int`. This is undefined behaviour and prints garbage in the error message. Function: `Parse_Into_Int_Array()`. | Use `%d`. | [ ] |
| UTIL-13 | 1713-1721 | NULL dereference | M/M | If the string has fewer than `size` tokens, `strtok` returns NULL and `strcpy((*p)[i], varStr)` crashes. Tokens longer than STRLEN would also overflow. Function: `Util_XML_Read_Array_String()`. | Check for NULL and quit with a clear message; use a bounded copy. | [ ] |
| UTIL-14 | 1433-1435 | use before NULL check | H/L | `strcpy(lengthStr, valuestr)` runs before `if (valuestr == NULL)`, so a missing length crashes instead of giving the error message. Also, the `fgets` return at 1442 isn't checked. Function: `Util_XML_Create_Node_Next_Line()`. | Move the strcpy after the NULL check, and check the fgets result. | [ ] |
| UTIL-15 | 1752-1758, 1763 | unreachable code / duplicated condition | H/L | The `else` branch is only reached when `result >= 0`, so `if(result == -1)` inside it can never be true. `!(_finite(value)) \|\| !(_finite(value))` tests the same thing twice (probably copied from an `isnan`/`_finite` pair). Neither changes behaviour. Function: `Util_XML_Get_Species_Node_Value()`. | Remove the dead test and the duplicate condition. | [ ] |
| UTIL-16 | 872, 878 | format mismatch / NULL deref | M/L | `quit("... %d", strlen(comment))` passes a `size_t` to `%d`. `strlen(units)` runs without a NULL check, while `comment` is NULL-checked. Function: `Add_Comment_Node()`. | Use `%zu`; add `units != NULL &&`. | [ ] |


#### atlantisUtil/atUtilGroupIO.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| UTIL-17 | 910-912 | cohort vs cohort x genotype | H/H | `contaminantTracers` and `contamPropTracers` are `Util_Alloc_Init_2D_Int(num_contaminants, numCohorts)`, which gives `[numCohorts][ncontam]`. `contaminantSpMort` is 1D with `numCohorts` entries. They are indexed with `cohort < numCohortsXnumGenes` in atecology/atContaminants.c:592-607 (`contaminantSpMort[cohort]`, `contaminantTracers[cohort][cIndex]`) and atmanage/atManage.c:2069-2070. That's an out-of-bounds read/write whenever `numGeneTypes > 1` with contaminants on. Function: `Util_Read_Functional_Group_XML()`. | Allocate with `numCohortsXnumGenes` (or bound the loops by `numCohorts`, depending on design). | [ ] |
| UTIL-18 | 894-905 | wrong dimension (habitat) and cohort vs cohort x genotype | H/H | The atomic-ratio arrays hard-code 3 habitats: `ratioLost = 3D(num_atomic_id, nCxG, 3)` gives `[3][nCxG][..]`, and `ratioLostGlobal = 4D(..., 3, 3)` gives `[3][3][nCxG][..]`. atecology/atecology.c:3899-3909 and 4036-4052 loop `hab < bm->num_active_habitats`, which is 5 when ice is on (attracerIO.c:239), so they write past the first dimension. `ratioLostPred`, `ratioLostPredGlobal`, `ratioGained` and `ratioGainedGlobal` are also sized `numCohorts` but looped with `cohort < numCohortsXnumGenes` in the same code, so they overflow when `numGeneTypes > 1`. Function: `Util_Read_Functional_Group_XML()`. | Use `bm->num_active_habitats` in place of 3 and `numCohortsXnumGenes` in place of `numCohorts` (fix the matching Init/free calls too). | [ ] |
| UTIL-19 | 1429-1435, 1440, 1502 | index beyond logical array size | H/M | `habitatCoeffs` is allocated with `bm->num_active_habitats` entries (3 = EPIFAUNA+1 without ice, 5 = ICE_BASED+1 with ice). The code then writes `[ICE_BASED]`(4), `[LAND_BASED]`(3) and `[MIXED]`(5). There's no heap corruption only because `Util_Alloc_Init_1D_Int` allocates double-sized storage. But the land flag (with terrestrial on and ice off) and the MIXED flag for ice groups fall outside every `hab < num_active_habitats` loop, so they're silently ignored. Function: `Set_Group_Habitats()`. | Allocate `nTotHabTypes` entries, or make `num_active_habitats` cover LAND_BASED/MIXED when those features are on. | [ ] |
| UTIL-20 | 397-399 | contradictory check / uninitialised member | H/M | `isAssessed` is read with `binary_check` (line 394), so a value >1 quits and `if(isAssessed > 1) isCloseKin = 1` can never fire. `FunctGroupArray` comes from `malloc` (line 252) and `isCloseKin` is never set otherwise, so it holds garbage, and atSS3Link/atSS3DataGen.c:85 tests it. Function: `Util_Read_Functional_Group_XML()`. | Initialise `isCloseKin = 0`, and read `IsAssessed` with `integer_check` if values >1 are meant to be allowed. | [ ] |
| UTIL-21 | 421-423 | uninitialised member | H/M | `isCollisionRisk` is only set when `bm->flagindustry_on`. Otherwise it's uninitialised malloc memory, read at atecology/atbiolmessage.c:2273/2303. Function: `Util_Read_Functional_Group_XML()`. | Add `else FunctGroupArray[groupIndex].isCollisionRisk = 0;`. | [ ] |
| UTIL-22 | 545-547 | unreachable condition | H/M | `if(groupType == TURF) bm->containsTurf = TRUE;` sits inside the `isVertebrate == TRUE` branch, but TURF is never a vertebrate, so `containsTurf` is always FALSE. atecology/atcoral.c:907/932 then forces `facilitate_space = 1.0` and skips the turf facilitation logic. Function: `Util_Read_Functional_Group_XML()`. | Move the TURF test into the invertebrate (`else`) branch or a separate loop. | [ ] |
| UTIL-23 | 244-252, 489-533 | NULL dereference | M/L | If `groupList` is NULL or empty, `FunctGroupArray` is never allocated and the count check is skipped. `Set_Group_Habitats` and the later loops then dereference NULL `FunctGroupArray`. Function: `Util_Read_Functional_Group_XML()`. | Quit with a clear message when no FunctionalGroup nodes are found. | [ ] |
| UTIL-24 | 1083-1085 | wrong loop variable | H/L | `TLorder2[eatIndex++] = i;` should be `guild`, since `i` is stale. This is inside `#ifdef ORIGINAL_GROUP_ORDER`, which is legacy code and not normally compiled. Function: `Util_Read_Functional_Group_XML()`. | Use `guild`. | [ ] |


#### atlantisUtil/atUtil.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| UTIL-25 | 841-845 | unit/reference mismatch (relative vs absolute day) | H/M | Existing entries store absolute days: `newArray[...][start_id] = start + bm->dayt` here and atharvest/atHarvest.c:1933. But the insertion point is chosen with `originalArray[i][start_id] > start`, using the relative `start`. Once `dayt > 0`, the new change goes in the wrong place, and `Util_Get_Change_Scale` (which assumes ascending start dates) picks the wrong active change. Callers: atHarvest.c:1943/1984, atbiolUtil.c:149. Function: `Util_Copy_Change_Values()`. | Compare against `start + bm->dayt`. | [ ] |
| UTIL-26 | 998-1009 | wrong algorithm | M/M | The comment says "random number from lognormal distribution", but the function draws `x_b ~ U(0,1)` and returns the lognormal density f(x_b), not a lognormal variate. The values are mostly not lognormally distributed, and `x_b = 0` gives division by zero / log(0). Used as a recruitment multiplier at atecology/atannualbiology.c:1116 and 1206. Function: `Util_Logx_Result()`. | Return `exp(mu + sigma * Util_Normx_Result(0,1))` (confirm intent with author). | [ ] |
| UTIL-27 | 809 | wrong index (copy-paste) | M/L | Inside the loop over `changeIndex` the guard tests `changeArray[0][start_id] != 0` rather than `changeArray[changeIndex][start_id]`. This was copied from `Util_Get_Change_Scale`, which has no loop. Function: `Util_Get_Accumulative_Change_Scale()`. | Use `changeArray[changeIndex][start_id]`, if the per-entry test was intended. | [ ] |
| UTIL-28 | 358 | wrong parameter key string | H/L | `paramStrings[RSstarve_id]` is set to `"RNstarve_"`, but the parameter is `RSstarve_` (regex `^RSstarve_`, atUtilXML.c:215). It's the same length, so ConvertAtlantis prefix stripping still works, but error messages (atCreateXML.c:326) name the wrong parameter. Similarly, `"_threshdepth_id"` and `"_depmum_scalar_id"` (462-463) have a stray `_id` compared with the real `_threshdepth` / `_depmum_scalar` keys. Function: `Util_Setup_Species_Param_Strings()`. | Use `"RSstarve_"`, `"_threshdepth"`, `"_depmum_scalar"`. | [ ] |
| UTIL-29 | 1185-1186, 1202-1203, 1197 | NULL dereference / wrong message | M/L | `*strstr(tempStr, ".nc") = '\0'` crashes if the file name has no ".nc". The stat-failure message prints `fileName` rather than the `fname` actually checked. Function: `Util_Check_NetCDF_Size()`. | Check strstr for NULL; print `fname`. | [ ] |
| UTIL-30 | 987, 1125 | log(0) | L/L | `log(step_a)` / `log(z2)` with a uniform draw that can be exactly 0 gives -inf/NaN. Functions: `Util_Normx_Result()`, `Util_xnorm()`. | Clamp the draw away from 0. | [ ] |


#### atlantisUtil/atUtilFisheryXML.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| UTL2-01 | 461-473 | buffer overrun | M/M | `counter = (int) bm->FISHERYprms[fisheryIndex][paramID]; ... array[fisheryIndex][b][index] = values[b];` for b < counter, no check against the allocated size. Callers size the array as 1 when the change flag is off: atharvest/atHarvestParamIO.c:860-864 `P_max_num_changes = 1; if (bm->flagchangep) ... = max+1; Pchange = Util_Alloc_Init_3D_Double(4, P_max_num_changes, K_num_fisheries)` (=> [nf][P_max][4]). The same applies to SWEPTchange at :873-880. If a fishery has `_p_changes`/`_swept_changes` >= 2 while flagchangep/flagchangeswept = 0, the writes go past `[nf][0]` and corrupt the heap. Function: `Util_XML_Read_Change_Values()`. | Pass the allocated max into the reader and quit if counter > max (or always size the array from the max param, whatever the flag). | [ ] |
| UTL2-02 | 504-521 | buffer overrun | M/M | Same pattern for groups: `for (b = 0; b < bm->SP_FISHERYprms[sp][i][paramID]; b++) array[sp][i][b][index] = values[b];`. Qchange is allocated with `Q_max_num_changes = 1` unless `bm->flagchangeq` (atharvest/atHarvestParamIO.c:905-908). If `_q_changes` > 1 while the flag is off, the write overflows the [sp][nf][1][3] allocation. TACchange (atimplementation/atImplementationParamIO.c:35) should be checked the same way. Function: `Util_XML_Read_Change_Fished_Groups()`. | Bound b by the allocated max_num_changes, or quit when the count is larger. | [ ] |
| UTL2-03 | 218-219 | wrong variable (log) | H/L | `printf("Fishery parameter: %s = %e\n", FisheryArray[fisheryIndex].fisheryCode, value);` prints the fishery code but not the parameter tag, so the verbose log can't tell which parameter was read. Function: `Get_Fisheries_Node_Value()`. | Print `paramStruct[paramID].tag` as well. | [ ] |


#### atlantisUtil/atUtilIO.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| UTL2-04 | 272 | wrong operator | H/M | `if ((*p)[i] < 0.0 && (*p)[i] > 1.0)` in the `probability_check` case is always false, so out-of-range entries are never rejected (only the sum is checked). This is a separate instance from the already-logged line 178, and the `proportion_check` case at line 262 correctly uses `\|\|`. Function: `readkeyprm_darray_checked()`. | Use `\|\|`. | [ ] |
| UTL2-05 | 189-191 | format mismatch | H/L | `quit("... current set to %d for %s ...", value, paramString);`: `value` is a double passed for `%d`, so the output is undefined (and `%s` may read a garbage pointer on some ABIs). The variable meant here is `checkType`. Function: `Check_Value()`. | Pass `checkType` instead of `value`. | [ ] |
| UTL2-06 | 317-323 | logic / unreachable | M/L | `if (sized != verifsize) quit(...); else if ((sized != 1) && (verifsize > -2) && (verifsize < 1)) warn(...)`. Unlike the darray version (line 237: `verifsize > 0 &&`), passing verifsize <= 0 to mean "no verification" always quits. The "no verification" warning can only run in the degenerate case sized == verifsize <= 0. Current callers (atphysics/atphysics.c:151-172) pass `ts.nv - 1`, which quits if nv == 1. Function: `readkeyprm_iarray_checked()`. | Mirror the darray guard: `if (verifsize > 0 && sized != verifsize)`. | [ ] |
| UTL2-07 | 92-103 | unchecked return / uninitialised | M/L | `Util_XML_Get_Value_String(...)`'s return is ignored. When `isRequired` is FALSE and the node is missing, `value` stays uninitialised and is passed to `strtosecs()`. The `localecotest` and `entry_type` parameters are also ignored. All current callers pass isRequired = 1, so this is latent. The error text names `Util_XML_Read_Time` instead of the function. Function: `Util_XML_Get_Value_Time()`. | Check the return and handle the missing, non-required case (e.g. set `*val = 0` and return FALSE). | [ ] |


#### atlantisUtil/atUtilArray.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| UTL2-08 | 142, 166 | prototype/definition type mismatch | H/M | The definitions are `Util_Alloc_Init_3D_Long_Double(int,int,int, double value)` and `Util_Alloc_Init_2D_Long_Double(int,int, double value)`, but atUtilLib.h:102-103 declares the last argument as `long double value`. atUtilArray.c doesn't include atUtilLib.h, so the compiler never catches this. Callers (e.g. atlantisUtil/atUtilGroupIO.c:821-822, 894-896, which include atUtilLib.h) pass a long double. On x86-64 Linux, long double goes on the stack and double in xmm0, so the callee reads a garbage init value and arrays such as `preyEaten` and `ratioLost` may not start at 0. On ARM64 macOS long double == double, so the problem stays hidden there. This is undefined behaviour. Functions: `Util_Alloc_Init_2D_Long_Double()`, `Util_Alloc_Init_3D_Long_Double()`. | Change both definitions to `long double value`, and include atUtilLib.h in atUtilArray.c so the compiler checks prototypes. | [ ] |
| UTL2-09 | 243 | prototype mismatch | H/L | The definition is `Util_Init_5D_Int(int *****array, ...)` but atUtilLib.h:112 declares `int ****array`. The pointer ABI is the same and there are no current callers, so this is latent; including atUtilLib.h (see UTL2-08) would turn it into a compile error. Function: `Util_Init_5D_Int()`. | Fix the header to `int *****array`. | [ ] |
| UTL2-10 | 396-403, 439-444 | buffer overrun (latent) | M/L | The copy runs if ANY new dim is larger, then copies the full old extent in every dimension: `for(i<olddim3) for(j<olddim2) for(k<olddim1) array[i][j][k] = oldarray[i][j][k]`. If one dim grows while another shrinks, it writes past the new allocation. It also quits when all dims are equal. Util_ReAlloc_2D_Int has the same issue. There are no current callers. Functions: `Util_ReAlloc_3D_Double()`, `Util_ReAlloc_2D_Int()`. | Loop to `min(old,new)` in each dimension, and only quit when no dimension grows. | [ ] |


#### atlantisUtil/atUtilhelp.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| UTL2-11 | 157-161 | off-by-one / buffer overrun | H/M | `istack = i_alloc1d(NSTACK)` gives valid indices 0..49. The check `if (jstack > NSTACK) quit` allows jstack == NSTACK, so `istack[jstack] = ...` writes istack[50], one past the end. This was carried over from Numerical Recipes' 1-based `ivector(1,NSTACK)` without adjusting. Index 0 is never used. Function: `Quicksort_Dir()`. | Allocate `NSTACK+1`, or test `jstack >= NSTACK`. | [ ] |
| UTL2-12 | 162-170 | copy error from NR | H/M | `if (ir - 1 + i >= j - 1)` should be NR's `if (ir - i + 1 >= j - l)` (letter l, not 1). As written it is always true (ir >= j, i >= 1), so the larger subarray is never pushed and the smaller is never processed first. Stack depth is then no longer bounded by log2(n), which makes the NSTACK quit/overrun in UTL2-11 reachable for large or adversarial inputs (atphysics/atgeomIO.c:879 sorts nbox entries; ateconomic/atquota.c:911, 1356). The dead else-branch is also wrong: `istack[jstack - 1] = i` should be `= l`. Function: `Quicksort_Dir()`. | Change the condition to `ir - i + 1 >= j - l` and the else-branch to `istack[jstack-1] = l;`. | [ ] |


#### atlantisUtil/atUtilFisheryIO.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| UTL2-13 | 106 | NULL dereference | L/L | `FisheryArray = malloc(...)` is never checked for NULL before use. Function: `Util_Read_Fisheries_XML()`. | Check for NULL and quit. | [ ] |
| UTL2-14 | 92, 124-125 | logic | L/M | `bm->K_max_num_subfleet = 5; // -1;` sets a floor of 5 on the max subfleet count, so arrays sized by it are larger than needed. It is probably deliberate (the -1 is commented out), but any loop over 0..K_max_num_subfleet will visit subfleets that don't exist for fisheries with fewer. Function: `Util_Read_Fisheries_XML()`. | Confirm intent. Otherwise initialise to 0 (or -1). | [ ] |


#### atlantisUtil/atUtilUnix.c


No issues found. Functions checked: _spawnlp.


#### atlantisUtil/include/*.h


The prototype/definition mismatches are logged above (UTL2-08 for atUtilLib.h:102-103, UTL2-09 for atUtilLib.h:112). No other issues found. Checked: the atUtilLib.h, atXMLUtil.h, atFisheryXMLUtil.h and atlantisutil.h prototypes against the definitions in the audited .c files, and the atlantisMem.h alloc/free macros. atFunctGroup.h, atTracer.h, atGroupTypes.h and atFisheryStruct.h were only skimmed (atFisheryStruct.h: `fisheryCode[20]`, `name[200]` are filled via strcpy in Util_XML_Get_Value_String with no length check, so an overlong code in the XML would overflow; see the note below).


### B.2.9 atlantismain/

#### atlantismain/atlantismain.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| MAIN-01 | 2293-2294 | NULL dereference / mismatched alloc-free | H/M | `free2d(dvol); free3d(dtr);` run every time, but `dvol`/`dtr` are only allocated inside `if (do_physics)` (atlantismain.c:1392-1398). `do_physics` is only set by the `-p` flag (line 869), so without `-p` they are NULL globals. `d_free2d` reads `pp[0]` before anything else (sjwlib/memory.c:648), and `d_free3d` reads `ppp[0][0]` (memory.c:721), so shutdown crashes. (If `flag_skip_phys` clears do_physics after allocation, the free is fine.) Function: `modelshutdown()`. | `if (dvol) free2d(dvol); if (dtr) free3d(dtr);` | [ ] |
| MAIN-02 | 994-1000 | NULL dereference | H/L | `pdest = strstr(bm->ncOfname, endname); totname = strlen(ncOfname) - strlen(pdest);` and then `*pdest = '\0'` on the second strstr. If the `-o` name does not contain ".nc", pdest is NULL and the program segfaults when it should print a usage error. Function: `setupMSEBoxModel()`. | Check `pdest == NULL` and call `quit("output file name must end in .nc")` | [ ] |
| MAIN-03 | 823-830 | buffer overrun | M/L | `strcat(commands, commandTempStr)` keeps appending every argv into `char commands[STRLEN]` (2000) with no bound check. Atlantis takes 16+ arguments, often long absolute paths, so the total can go past 2000 bytes and overflow the stack. Function: `setupMSEBoxModel()`. | Use `strncat(commands, commandTempStr, sizeof(commands)-strlen(commands)-1)` | [ ] |
| MAIN-04 | 1031 | always-false condition | H/L | `!bm->forceIfname \|\| !bm->fishprmIfname \|\| !bm->assessprmIfname \|\| !bm->econprmIfname` test char-array addresses, which are never NULL, so these parts of the "required arguments" check never fire. The other entries correctly test `[0]`. (forceIfname is already checked at line 986, so only the fish/assess/econ checks are dead. Those files are optional, so do not just add `[0]` to them.) Function: `setupMSEBoxModel()`. | Remove the dead terms, or use `[0]` only for files that really are required | [ ] |
| MAIN-05 | 1036 | wrong label / copy-paste (log text) | H/L | The `bm->params` string prints `-s %s -g %s -c %s -d %s` with the summary, PROD, TOTCATCH and CATCH output file names. These are old flag meanings: `-s` is now the functional-group file, `-c` is cabspot, `-d` is the destination folder, and `-g` is not a flag. The params string written into output metadata is therefore wrong and cannot be re-run. Function: `setupMSEBoxModel()`. | Rebuild params from the current flags (-s functionGroupIfname, -q fisheryIfname, -d destFolder, etc.) | [ ] |
| MAIN-06 | 2085-2090 | always-false condition | H/L | `not_tracking_flux = totfluxout; if (totfluxout != not_tracking_flux) quit(...)` assigns and then compares the same value, so the sanity check can never fire. Function: `AllocateArrayMemory()`. | Remove the check, or compare against the intended independent constant | [ ] |
| MAIN-07 | 842-942, 967 | missing string terminator | M/L | `strncpy(bm->X, *++argv, BMSLEN)` into BMSLEN-sized fields leaves no terminator when the argument is 512 characters or longer. The memset at 808 does not help because all 512 bytes get written. The `-d`/`-t` cases then `strcat(FOLDER_SEP)`, which can overflow. Function: `setupMSEBoxModel()`. | Copy BMSLEN-1 characters and terminate explicitly; check length before strcat | [ ] |
| MAIN-08 | 2250-2558 | memory leak | H/L | `bm->init_pH` (allocated at line 1179 with alloc2d) is never freed in `modelshutdown()`. Every other array in that allocation block is freed. Function: `modelshutdown()`. | Add `free2d(bm->init_pH);` | [ ] |


#### atlantismain/process.h


No issues found. Functions checked: `_spawnlp` prototype (fixed 3-arg declaration of a normally variadic Windows API; not called in the audited code).


#### atlantismain/include/atlantisboxmodel.h


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| MAIN-09 | 408-414 | copy-paste / wrong guard | H/L | `#ifndef min` / `#define min` is followed by `#if !defined(min)` / `#define max`. Because min was just defined, the max guard is always false, so this header never defines `max`. It only compiles because sjwlib/include/sjwlib.h:71 defines max. The code is latent and fragile, and with the `>=` version the two definitions also differ. | Change the guard to `#ifndef max` | [ ] |
| MAIN-10 | 333-340, 153 | preprocessor logic | M/L | `#ifndef SMALL_LOG_FILE_SIZE ... #elif BETH_LOG_FILE_SIZE ... #else`: the BETH option is only checked when SMALL_LOG_FILE_SIZE is defined, so defining BETH_LOG_FILE_SIZE alone has no effect. The revision note at line 153 also gives stale sizes (1.024e9/1.024e3, against the actual 2.024e8/1.024e7). | Restructure as `#if defined(BETH_LOG_FILE_SIZE) ... #elif defined(SMALL_LOG_FILE_SIZE) ... #else` | [ ] |
| MAIN-11 | 389-402 | conflicting enum/define values | L/M | `nlevel_id` and `growth_id` both equal `ICE_BASED+1` (5), which is also `MIXED`. `death_id` (6) equals `nTotHabTypes`. Arrays sized `nlevel_id` cannot take a `growth_id`/`death_id` index, and a `fluxnum` of MIXED cannot be told apart from growth_id (atecology/atfluxbreakdown.c:80, atecology.c:1350). There is no compile error only because no switch also has `case MIXED`. Arrays indexed by fluxnum were not traced. | Define growth_id/death_id as `nTotHabTypes`, `nTotHabTypes+1` and size the flux arrays to match | [ ] |

Prototype spot-checks (no mismatch found): createBMDataFile, readBMTracerInfo, boundaries, physics, External_Box_Ecology, Util_Check_NetCDF_Size, Ecology_Test_Fish_Total. The header's prototypes were not checked exhaustively.

Allocation and index checks (all consistent after applying the reversal convention): newwctr/newsedtr/newicetr/newlandtr, vvdistrib vs set_up_vvdistrib, stock_struct, v_stock_struct, coveramt, refuge_status, turbid_effect, FCtsCarryOver, Effort_hdistrib, EffortRecord, CatchRecord, totagepop, CumDisplaceEffort, TAC_trigger (triggered_scalar_id = 2 < 3).


### B.2.10 ConvertAtlantis/

#### ConvertAtlantis/atBioltoXML.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| CNV1-01 | 1780-1786 | buffer overrun / wrong variable | H/H | `tempStr[strlen(secondlastLetter) - strlen("popratioStock") - 2] = '\0';`: `secondlastLetter` points at the last 2 chars, so `strlen(secondlastLetter)` is 2. `2 - 13 - 2` is computed as size_t and wraps, so the write lands about 13 bytes *before* `tempStr` on the stack, and `tempStr` itself is never truncated. The single-digit branch at 1785 uses `strlen(tempStr)` correctly. This path runs whenever a stock index has 2 digits, i.e. `FBP1popratioStock10` (regex at 3278 allows `[1-9][0-9]{0,2}`) with K_num_stocks_per_sp >= 10. A 3-digit stock index is not handled either. Function: `PopRatioStockXMLFunction()`. | Use `tempStr[strlen(tempStr) - strlen("popratioStock") - 2] = '\0';`, or better, find `strstr(tempStr,"popratioStock")` and put the terminator there. | [ ] |
| CNV1-02 | 1159-1177 | wrong variable / logic | H/M | When `bm->flag_macro_model` is on, the `else { cohort = main_biomass_id; }` branch overwrites the `cohort = juv_id` / `adult_id` value set at 1149/1161 for every non-Epi/Root parameter, including non-seagrass groups. For AGE_STRUCTURED_BIOMASS inverts (e.g. CEP/PWN), both `C_CEP_T15` and `C_jCEP_T15` (and the `mum_` versions, via the call at 2977) then go to index 0 (main_biomass_id = 0) in `Util_XML_Replace_Node_Value(...,2,cohort)`. Whichever comes last wins, and the adult slot (index 1) stays at the "0.0" default (atUtilXML.c:1136-1142). Result: adult clearance/growth = 0 for those groups in macro-model runs. Function: `InvertebrateClearanceRateXMLFunction()`. | Do the macro-model Epi/Root/main override only when the group is SEAGRASS. Otherwise keep the juv/adult cohort. | [ ] |
| CNV1-03 | 948-985 | stale variable / not reset in loop | H/L | `groupNode` is set to NULL once, before the guild loop. For a guild that is `isOverWinter` but not mobile (or is EPIFAUNA), `if(groupNode == NULL)` is false because groupNode still holds the *previous* guild's node. The "overwinter" temporal node (plus juv/adult children) is therefore attached to the wrong group, and that group can end up with duplicate "overwinter" nodes. Function: `InitVertDistributionXML()`. | Reset `groupNode = NULL;` at the top of each guild iteration. | [ ] |
| CNV1-04 | 4078-4079 | format mismatch / wrong check | M/M | `quit("...(%d)... (value of %d)...", numGroups, convertedXMLFileName, bm->K_num_tot_sp)`: the 2nd `%d` gets a `char*` (UB on the error path). Separately, the check itself (`numGroups > K_num_tot_sp`) is wrong: each CSV row is one (group, stage, MigID) migration, so a valid file can have more rows than species (e.g. juv + adult rows for many groups), and this would quit spuriously. Function: `Util_Read_Migration_XML()`. | Remove or replace the check (e.g. validate per group against numStages*num_migs). Fix the format string to match its arguments. | [ ] |
| CNV1-05 | 4092-4128 | missing bounds check | M/M | `start_stage` and `migrationID` come straight from the CSV and are used unchecked as `MIGRATION[sp].X_Prm[start_stage][migrationID]`. Those arrays are `[numStages][num_migrate+1]` (atBiologyXMLParamIO.c:4331, 4402-4421) and Stagger_Prm is `[numStages][num_migs][2]`. A stage >= numStages, a negative value, or a MigID > num_migrate gives a silent heap write out of bounds. Function: `Util_Read_Migration_XML()`. | Add `if(start_stage<0 \|\| start_stage>=numStages \|\| migrationID<0 \|\| migrationID>num_migrate) quit(...)`. | [ ] |
| CNV1-06 | 636-680 | buffer overrun | M/H | `char sedimentValues[1000]; char finalValues[1000];` collect K_num_tot_sp tokens with `strcat`, and the input line can be up to STRLEN (2000) chars (Parse_File buf). Example: 90+ groups with values like `1.234e-05` (about 10 chars each) overflows the 1000-byte stack buffer. `spVERTEatXMLFunction` uses STRLEN buffers for the same data. `speciesStr[20]` filled by unbounded `sscanf("%s")` (643) is also latent. Function: `spInvertEatXMLFunction()`. | Size the buffers to STRLEN (or 2*STRLEN) and use bounded concatenation (`strncat`/`snprintf` with remaining length). | [ ] |
| CNV1-07 | 744-757, 762-767 | OOB index / wrong error text | L/M | `cohortNode = ...cohortStrings[i]` (753) runs *before* the `i == -1` check at 755. `cohortStrings` has only 2 entries (atUtil.c:175), and neither `i` (last char) nor `j` (leading digit) is range-checked. The error message at 756 names `spInvertEatXMLFunction` (copy-paste). `finalValues[STRLEN]` with `%.10e` (17 chars/value) also overflows for more than about 117 groups. Function: `spVERTEatXMLFunction()`. | Validate `0<=i<=1 && 0<=j<=1` before using them. Fix the message text. Bound the strcat. | [ ] |
| CNV1-08 | 94-107 | buffer overrun (edge) | M/M | `malloc(3*size)` followed by `strcpy(zeroString,"")`: with size==0 this is malloc(0) followed by a 1-byte write. Possible callers with size 0: `InitSuppDietXML`/`InitHabitatXML` (K_num_cover_types), `InitCatchPropGrazersXML` (K_num_fisheries). Also no NULL check on malloc. Function: `CreateZeroString()`. | Use `malloc(3*size + 1)` and check for NULL. | [ ] |
| CNV1-09 | 1115-1135 | hard-coded code length | M/M | `speciesStr = str + strlen(str) - 3;` assumes the predator code is exactly 3 characters (`p_BCFDB` gives prey BC, pred FDB). Atlantis codes of 2 or 4 characters give the wrong prey/pred split, and the node is created under a bogus group name with no validation (no FG lookup). Function: `AgeDietXMLFunction()`. | Look up the predator by trying FG codes that match the string suffix (or split using the known prey code), and validate both with `Util_Get_FG_Index_From_Token`. | [ ] |
| CNV1-10 | 1267-1271 | hard-coded code length | M/M | `if(strlen(str) == 9) speciesStr = str + 6; else speciesStr = str + 5;` for `mS_FDxxx`/`mS_SBxxx`. A 4-character group code (`mS_FDABCD`, length 9) is cut to `BCD`: wrong group or quit. The prm files seen use `mS_FDFBP` (length 8), so the +6 branch looks like it is only for an optional separator. Function: `ImplicitMortalityRateXMLFunction()`. | Use `str + 5` and skip one optional `_`. | [ ] |
| CNV1-11 | 2426-2427, 2433-2435, 2439-2440 | copy-paste (metadata) | H/L | `max_move_temp_id` is described as "Min tolerated temperatures" and `min_move_temp_id` as "Maximum tolerated temperatures". The salinity pair is swapped the same way. `min_O2_id` is described as "Maximum tolerated O2". `K_salt_const_id` and `K_o2_const_id` have units "deg C". Function: `createMovementXML()`. | Swap the descriptions back and fix the units. | [ ] |
| CNV1-12 | 2465, 2636, 2644, 3104, 3129, 3458, 3460 | copy-paste (metadata) | H/L | Copy-pasted descriptions or units: FLAG_LYSIS_RATE units "mg O2 m-3" (rate is d-1); `coral_compete_id` reuses "Rate macroalgae overgrow coral"; `HAB_mum_scalar_id` has units "W m-2" and range "3 - 65" (copied from KI); FLAG_SALT_SENSITIVE is described as "Temperature sensitivty"; FLAG_PH_CONSTC as "Coefficient B"; `mBleach_id` as "Slow recovery period", "days", "10 - 200"; `bleach_recovery_rate_id` (d-1) has range "10 - 200". Functions: `createMortalityXML()`, `createPrimaryProduceXML()`, `createQ10XML()`, `createCoralXML()`. | Correct each description/units/range. | [ ] |
| CNV1-13 | 2106-2110, 2172 | wrong XML type | H/L | `flag_refuge_model` is declared XML_TYPE_BOOLEAN but has a 3-value lookup (0/1/2, and 2 = rugosity_refuge_model is used at 2112). `K_max_num_DHW` is declared XML_TYPE_BOOLEAN with recommended value "14". The readers use no_checking/integer_check, so only metadata is wrong today, but any type-based validation/editor would reject valid values. Function: `createPhysicalLim_n_FLagsXML()`. | Use XML_TYPE_LOOKUP / XML_TYPE_INTEGER. | [ ] |
| CNV1-14 | 3760, 3764 | duplicate block | H/L | `Util_XML_Parse_Create_Node(..., "InvadersEntering", ...)` is called twice, so two identical attribute nodes are emitted in the XML. Function: `createInvaderXML()`. | Delete the first call (3760). | [ ] |
| CNV1-15 | 3953-4018 | NULL deref on malformed input | L/M | Each of the 17 `strtok(NULL, seps)` results is passed straight to `Util_XML_Create_Node` as a value. A data row with fewer columns than the header (only the header row is counted, 3928-3937) passes NULL. Also, `seps=","` keeps the trailing `\r\n` on the last field. Function: `Convert_Migration_To_XML()`. | Check each token for NULL and quit with row info. Add `\r\n` to seps. | [ ] |
| CNV1-16 | 1345-1379 | uninitialised variable | L/L | `char param[30]` is left uninitialised if none of the `strstr` branches match, then used as a node name. The function currently has no callers (dead code), so it is latent. Function: `MigrateReturnXMLFunction()`. | Add an `else quit(...)` branch, or remove the dead function. | [ ] |
| CNV1-17 | 3795-3798, 3910-3913 | resource leak | L/L | `if (doc == NULL) { ... return; }` returns without `fclose(fp)` / `fclose(inputFP)`. Functions: `Convert_Biol_To_XML()`, `Convert_Migration_To_XML()`. | fclose before returning (or quit). | [ ] |


#### ConvertAtlantis/atCreateXML.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| CNV1-18 | 33-45 | infinite loop | H/M | `while(1){ if(fgets(...) != NULL){...} }` has no else branch. At EOF fgets keeps returning NULL and the loop never ends. A parameter key on the last line with no value line, or followed only by comments/blank lines, makes the converter hang instead of printing the "No value found" error at 193-195 / 1490-1492. Function: `Get_Next_Line()`. | On a NULL fgets, set `buf[0]='\0'` and return. The callers already quit on `strlen(buf)==0`. | [ ] |
| CNV1-19 | 1836-1839 | inconsistent parsing / unchecked read | M/M | The value line is read with a raw `fgets(buf, buflen, inputFP)` (return value unchecked). Comment and blank lines between key and values are not skipped, the trailing `\n` is not trimmed, there is no numeric check, and the declared length is not checked. Every other array reader in this file uses `Get_Next_Line` + `trim` + `CheckIsDigits`. A comment line under a `sel_XXX`/`q_XXX` key is silently stored as the parameter value. Function: `Create_Harvest_Fishery_Group_ParamXML()`. | Use `Get_Next_Line(bm,inputFP,buf); trim(buf);` + `CheckIsDigits`, and validate the token count against K_num_fisheries. | [ ] |
| CNV1-20 | 1477-1478, 1629-1630 | NULL deref | H/L | `lengthString = strtok(NULL, seps); lengthValue = atoi(lengthString);` has no NULL check. A multi-stage key line with no length token crashes. `Parse_File` (179-181) checks the same case. Also, a length mismatch only produces a warning, and all stages are then marked present anyway (1483-1485 / 1635-1637). Functions: `Create_Species_Cohort_ParamXML()`, `Create_Species_Spawn_ParamXML()`. | Add `if(lengthString==NULL) quit(...)`. | [ ] |
| CNV1-21 | 297-303 | NULL deref | M/L | `valueStr = strtok(NULL, seps)` can be NULL (key with no value on the line) and is passed unchecked to `fp(...)` and from there to `Util_XML_Set_Node_Value`. Function: `Parse_File_Single_Line()`. | Quit with a message if `valueStr == NULL`. | [ ] |
| CNV1-22 | 903/1165-1283, 1729/1809-1823, 1899/1950-2034 | buffer overrun (latent) | M/M | `char strPtr[50]; strcpy(strPtr, varStr);` where varStr is an unbounded token from a STRLEN/5000-byte line. Long keys (e.g. `XXXX_coral_max_accel_trans_...`, long fishery codes + param name) overflow the stack buffer. `strstr`/`strchr` results are also dereferenced without NULL checks (e.g. 1166, 1173, 1199, 1206). Functions: `Create_Species_ParamXML()`, `Create_Harvest_Fishery_Group_ParamXML()`, `Create_Fishery_ParamXML()`. | Use `snprintf(strPtr, sizeof(strPtr), "%s", varStr)` (or a STRLEN buffer) and check the strstr/strchr results. | [ ] |
| CNV1-23 | 731-732 | OOB read (error path) | L/L | `cohortStrings[cohort]` has 2 entries (atUtil.c:175), but `Check_Species_Cohort_Param` loops `cohort < numStages`, which can be greater than 2 (seagrass/coral stages). Printing the error then reads out of bounds. Function: `Print_Species_Cohort_Param_Error()`. | Print `stage %d` when cohort > 1. | [ ] |
| CNV1-24 | 1320, 2376 | format/arg mismatch | L/L | `quit("...paramID not found = %d (%s) in switch statement. \n", paramID, tag, fileName)`: `fileName` is passed but has no `%s`, so the file is never reported (extra arg, harmless). Functions: `Create_Species_ParamXML()`, `Create_RBC_Species_ParamXML()`. | Add `in file %s` to the format. | [ ] |


#### ConvertAtlantis/atManagetoXML.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| CNV2-01 | 911 (with 176-211) | wrong regex / key mismatch | H/M | `"^[A-Z]{2,3}_co_sp_catch[1-9]{1,2}"` requires a trailing digit and exactly 2-3 uppercase letters for the group code. `Co_Sp_CatchXMLFunction()` handles keys with no digit ("no digits = companion 1", line 200-202), but the regex never passes those keys to it. Real prm files do use bare keys (e.g. `BB_co_sp_catch`, `BC_co_sp_catch` in /Users/ful083/runs). Four-character codes are dropped too. The `companionN` nodes then stay empty, and `Read_Fishery_Companion_Catch_XML()` (atManageParamIO.c:299) fails for them. Function: `TACXML()`. | Use regex `^[A-Za-z0-9]+_co_sp_catch[0-9]*$` | [ ] |
| CNV2-02 | 692-698 | parsing / wrong index | H/M | `PortXMLFunction()` treats the text after the last `_` as the fishery code. Fishery codes that contain `_` (e.g. `Astoria_Generic`, `Com_boat`, which exist in /Users/ful083/runs prm files) resolve to `Generic`/`boat`. `Util_Get_Fishery_Index()` then quits (atUtil.c:228), or maps to the wrong fishery if a fishery with that name exists. Function: `PortXMLFunction()`. | Strip the known `ports_` prefix (or match against FisheryArray codes) instead of taking the last `_` | [ ] |
| CNV2-03 | 46-56 | logic (latent) | H/L | `if (flag) strcat(newValueStr, " "); flag = FALSE;` adds a separator only after the first token, so later tokens run together (`"1 2345"`). It also hard-codes `45` as the not-fished id (the comment itself says this is worrying). The function is currently only referenced from commented-out code (line 905), so this is latent. Function: `Fix_Not_Fished_Species_Last_XMLFunction()`. | Always append `" "` between tokens and use the real `Not_fished_id`, or delete the function | [ ] |
| CNV2-04 | 112-127 | NULL deref / overflow | M/L | `strcpy(groupStr, strstr(indexStr, "_") + 1)` dereferences NULL+1 if the key has no `_` after the region number. It also copies into `groupStr[MAX_GROUPCODE_LENGTH]` without a bound. The error text says `FBoMonthlyXML`, and `index < 0` is checked after it has already been used. Function: `BiMonthlyXML()`. | Check the `strstr` result and use a bounded copy | [ ] |
| CNV2-05 | 507, 590, 742, 749, 788 | copy-paste metadata | H/L | Descriptions copied without editing. `flagcap_peak_id` uses the same text as `flagcap_id`. `flagchangeEFF_id` says "0=no, 0=yes". `targ_refE`/`forage_refE` say "tier4" (should be tier5). `Fref_High` uses the same text as `Fref`. This affects comments in the generated XML only. Functions: `ManamentFlagTimeXML()`, `ManagementSenarioFlagsXML()`, `TACXML()`. | Correct the description strings | [ ] |


#### ConvertAtlantis/atAssesstoXML.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| CNV2-06 | 689-690 | wrong check type | H/H | `Num_Sexes` is declared `XML_TYPE_BOOLEAN` and read with `binary_check`. `Check_Value` quits for any value other than 0 or 1 (atUtilIO.c:171-175). A normal two-sex setup (`Num_Sexes 2`; sexStrings has F/M) therefore aborts the conversion whenever `useRBCTiers` is on. The reader has the same bug (atassessParamIO.c:964). Function: `TierAssessmentXML()`. | Use `XML_TYPE_INTEGER` with `integer_check` here and in atassessParamIO.c:964 | [ ] |
| CNV2-07 | 549-554 | buffer overrun / NULL deref | M/L | `i = strcspn(speciesStr, "0-9"); strncpy(strPtr, speciesStr, i); strPtr[i] = '\0';` writes into `char strPtr[10]`. If a matching key has no digit before the parameter name, `i` is the full key length (overflow) and `sexnum` is NULL, so `atoi(NULL)` crashes. Group codes containing digits are also mis-split. Function: `AssessStockXMLFunction()`. | Bound `i` to `sizeof(strPtr)-1`, check `sexnum != NULL`, and split on `_` | [ ] |
| CNV2-08 | 975, 1037 | duplicate parameter | M/L | `trigger_threshold_id` is created twice from the same prm keys, once under `Tier6` and once under `TIERsigma`, with different descriptions. The reader uses only `TIERsigma` (atassessParamIO.c:1561), so the `Tier6` copy is dead and confusing. Function: `TierAssessmentXML()`. | Remove one copy | [ ] |
| CNV2-09 | 247, 471, 772, 981-985 | copy-paste metadata | H/L | `flagfishbiom` is described as "detritus". `NumRAssessScriptNames` is a count but typed `XML_TYPE_BOOLEAN`. For `UseCloseKin`, `nTotSamps`, `first_sy`, `last_sy` and `first_dy`, the description and units arguments are swapped (the description goes into units), and `UseCloseKin` has recommended value `"4000"`. Functions: `ErrorStructureXML()`, `RAssessFlagXML()`, `TierAssessmentXML()`. | Fix the argument order, types and texts | [ ] |


#### ConvertAtlantis/atHarvesttoXML.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| CNV2-10 | 47-54 | parsing / wrong index | H/M | `Fishery_Last_XMLFunction()` takes the text after the last `_` as the fishery code. Real models have fishery codes with underscores (`EffortChange_start_Astoria_Generic`, `..._Com_boat` in /Users/ful083/runs), which resolve to `Generic`/`boat`. `Util_Get_Fishery_Index()` then quits or picks the wrong fishery. This affects every caller: mEff_, habitat_, SELchange_*, EffortChange_*, Effort_mult_, among others. Function: `Fishery_Last_XMLFunction()`. | Strip the known parameter prefix (pass it in) instead of using the last `_` | [ ] |
| CNV2-11 | 330 | copy-paste | H/L | The lookup for option 4 ("Conditional on catch of other species") is created with value `"2"`, which duplicates the "Size based" entry. Function: `FisheryFlagXML()`. | Change `"2"` to `"4"` | [ ] |
| CNV2-12 | 312, 469 | wrong type metadata | H/L | `selcurve_id` is typed `XML_TYPE_BOOLEAN` but has 7 lookup values (0-6). `k_mismatch` is typed `XML_TYPE_BOOLEAN` but is a real number (recommended 0.25; the reader uses a double at atHarvestParamIO.c:602). Functions: `FisheryFlagXML()`, `FisheryScenarioFlags()`. | Use `XML_TYPE_LOOKUP` / `XML_TYPE_FLOAT` | [ ] |
| CNV2-13 | 167-185 | logic / dead code | M/L | `Incidental_MortalityXML()` builds `newValueStr` from the prm value and then ignores it, writing `"0.0"` for every group. The `IncidentalMortality` node is never read (no reader match), so the prm value is silently lost. | Use `valueStr` or remove the block | [ ] |
| CNV2-14 | 483, 653 | inconsistent length | M/L | `CatchAgeDistributions` expects length `K_num_max_cohort*K_num_max_genetypes`, `DiscardAgeDistributions` expects `K_num_max_cohort`, and the reader reads `numCohorts` values (atHarvestParamIO.c:148). When there is more than one genotype, Parse_File warns about a length mismatch for every catch key. Function: `FisheryScenarioFlags()`. | Use `K_num_max_cohort` for both | [ ] |


#### ConvertAtlantis/atEconomicXML.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| CNV2-15 | 33-35 | parsing / wrong index | H/M | `*strchr(str, '_') = '\0'` treats everything before the first `_` as the fishery code. Fishery codes containing `_` (e.g. `Com_boat`, `Astoria_Generic`) give `Com`/`Astoria`, and `Util_Get_Fishery_Index()` quits. This affects all subfleet parameters (`_nboat`, `_varcost`, ...). Function: `FisheryFirstXMLFunction()`. | Strip the known suffix (pass it in) instead of cutting at the first `_` | [ ] |
| CNV2-16 | 257, 259 | duplicate parameter | H/L | `UseEconQuotaDefine` is parsed twice in a row, giving two same-named nodes in `Economic_Flags` (the first without a description). The reader takes the first. Function: `economicFlagParamsXML()`. | Delete line 257 | [ ] |
| CNV2-17 | 253, 823 | copy-paste text | H/L | The `UseConstPrice` description is copied from `MultiPlanEffort`. The quit message names the "Chemical_Limitation attribute group" instead of Economic_Flags. Functions: `economicFlagParamsXML()`, `Convert_Economic_To_XML()`. | Fix the strings | [ ] |
| CNV2-18 | 69-71, 95-106, 117-118, 170-171, 603-604, 651-652 | parsing limitation | M/L | Market, subfleet and market-coefficient indices come from the last character only (`atoi(lastLetter)`). With 10 or more subfleets or markets, `sub10` maps to `sub0` / `Market_-1`. `createMonthlyMarketXMLNodes()` also hard-codes 2 markets, while the reader loops over `K_num_markets` (ateconParamIO.c:259). Functions: `MarketXML()`, `MonthlyMarketXML()`, `OwnQuotaXML()`, `TemporalBlackBookXML()`, `SpatialBlackBookXML()`, `createMonthlyMarketXMLNodes()`. | Parse the trailing digit run (`strspn` from the end) and loop over `K_num_markets` | [ ] |


#### ConvertAtlantis/atGroupsXML.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| CNV2-19 | 34-42 vs 261-264 | missing column count | H/H | `isCollisionRisk` is read as an extra column when `bm->flagindustry_on` is set, and the reader requires it (atUtilGroupIO.c:421-422). However, `numExpectedTokens` is only raised for `external_populations`, `terrestrial_on` and `flag_pollutant_impacts`. With industry on, a correct groups CSV has one more column than expected, and the converter quits with "does not have the correct number of columns". Function: `Convert_Groups_To_XML()`. | Add `if (bm->flagindustry_on) numExpectedTokens++;` | [ ] |


#### ConvertAtlantis/atRunXML.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| CNV2-20 | 49-73 | missing parameter | H/H | The reader requires `K_num_vessel_sizes` in `SubModelSetup` when `flagindustry_on == simple_industry_model (1)` (atparamIO.c:136-138, isRequired=1, which quits if absent at atUtilIO.c:71-72). The converter never writes it, so any `.prm` run file with industry on fails to load. Function: `Convert_Run_To_XML()`. | After `flagindustry_on`, read it back and, if it is 1, call `Util_XML_Parse_Create_Node(..., "K_num_vessel_sizes", ..., XML_TYPE_INTEGER, "1")` | [ ] |
| CNV2-21 | 126-129 (with atparamIO.c:234) | state only set by converter | H/M | `bm->track_contaminants` is set only as a side effect of conversion (line 127). In `Read_Run_Parameters()` the read is commented out (atparamIO.c:234). When the run file is supplied directly as `.xml`, `track_contaminants` keeps its default, so contaminants (and the num_contaminants/names block) are silently skipped. Function: `Convert_Run_To_XML()` / `Read_Run_Parameters()`. | Restore the `track_contaminants` read in atparamIO.c (ScenarioOptions group) | [ ] |
| CNV2-22 | 133-139 | metadata clobber | M/L | `Parse_File(..., initFP=NULL, ...)` uses the `ScenarioOptions` sub-group node itself as `newnode`. It therefore overwrites that group's `AttributeType`/`ArrayLength` properties with StringArray/"" (atCreateXML.c:144-153). The values are still created correctly. Function: `Convert_Run_To_XML()`. | Pass an initFP that creates a child node, or use `Util_XML_Create_Node_Next_Line` | [ ] |
| CNV2-23 | 126-127, 199 | wrong type metadata | H/L | `track_contaminants` is declared BOOLEAN but read back with `integer_check`. `debug` is declared `XML_TYPE_BOOLEAN` but has 25 lookup values (0-24). Function: `Convert_Run_To_XML()`. | Use `XML_TYPE_LOOKUP`/`XML_TYPE_INTEGER` | [ ] |


#### ConvertAtlantis/atFisheriesXML.c


No issues found. Functions checked: Convert_Fisheries_To_XML.


#### ConvertAtlantis/atImplementationtoXML.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| CNV2-24 | 29-33 | copy-paste metadata | H/L | The group `Fishing_Catchability_Change` has the descriptions "Changes in catchability through time" and "Number of changes of fishing catchability.", and passes `"catchability"` to `Create_Fished_Groups_ChangeXML`. All of this is copied from the Qchange block, but the content is TAC changes (`TAC_num_changes_id`, `TACchange`). The node name matches the reader (atImplementationParamIO.c:25), so this affects metadata only. Function: `ManagementImplementationChangeXML()`. | Fix the description strings | [ ] |


#### ConvertAtlantis/include/convertXML.h


No issues found. Functions checked: all prototypes against their definitions (Parse_File, Parse_File_Single_Line, Create_*_ParamXML, Convert_*_To_XML, Fishery_Last_XMLFunction, Species_NodeXML, createImpactedSpeciesParam etc.).


### B.2.11 atCLAMLink/

#### atCLAMLink/atCLAMComm.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| LINK-01 | 228 | NULL dereference | H/M | `*(strstr(valueStr, ".")) = '\0';` — crashes if the second token has no '.', and `valueStr` itself is NULL when a line has only one token (strtok returns NULL). Data comes from an external CLAM output file. Function: `Parse_CLAM_OutputFile()`. | Check `valueStr != NULL` and the strstr result before writing. | [ ] |
| LINK-02 | 245-254 | buffer overrun | H/H | `while ((valueStr = strtok(NULL, seps)) != NULL) { outcomeValueArray[...][i] = atof(valueStr); i++; }` — `i` has no upper bound, but the last dimension is MAXNUMOUTPUTSTATES (10) (atCLAMLinkSetup.c:68). A CLAM line with more than 10 values after the name writes past the end of the row (heap corruption). Function: `Parse_CLAM_OutputFile()`. | Stop the loop at `i < MAXNUMOUTPUTSTATES` (or at `numStates`) and warn. | [ ] |
| LINK-03 | 232, 247 | wrong index / overrun | M/M | `indicatorIndex = Get_CLAM_IndicatorID(...)` returns an index into `indicators[]` (0..numIndicators-1, read from XML), but it is used as the first index of `outcomeValueArray`, which has size `numClamIndicators` (7) (atCLAMLinkSetup.c:68). The same mix-up affects `expectedValue/variance/distance[..][..][indicatorIndex]` and `indicatorWeightings[indicatorIndex]` in atCLAMManage.c (all sized numClamIndicators). Overruns if the XML lists more than 7 indicators (for example two indicators mapped to the same CLAM output). Function: `Parse_CLAM_OutputFile()`. | Size these arrays by `numIndicators`, or index them by `indicators[i].CLAMOutputIndex`. | [ ] |
| LINK-04 | 146 | off-by-one | M/L | `if (optionSelected > maxOptionValue) quit(...)` — `optionSelected` is a 0-based strategy ID (it is written as `+1`, line 149) and `maxOptionValue` is the number of options (e.g. 4 for "Sea Level Rise", which has IDs 0..3), so the check lets ID == maxOptionValue through. Function: `Create_CLAM_Input_File()`. | Use `>=`. | [ ] |
| LINK-05 | 216 | buffer / truncation | L/L | `fgets(buf, 200, ...)`: lines longer than 199 chars are split, and the rest is parsed as a new line (its first token is skipped and its second is treated as a name). Function: `Parse_CLAM_OutputFile()`. | Use a larger buffer, or detect a missing '\n' and handle it. | [ ] |


#### atCLAMLink/atCLAMImplement.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| LINK-06 | 116-118 | wrong check / OOB write | H/M | `variableIndex = tsIndex(&ts, varName) - 1; if (variableIndex == -1) quit(...)` — `tsIndex()` (dfIndex, sjwlib/datafile.c:571) returns -1 when the name is not found, so variableIndex becomes -2 and the check misses it. Lines 124-127 then write to `[...][-2]` (heap corruption). Only a match on the time variable (index 0) is caught. Function: `Scale_PointSource()`. | Test `tsIndex(...) < 1` before subtracting 1. | [ ] |
| LINK-07 | 87, 95, 98 | missing bounds check | M/M | `pssIndex` comes straight from the XML lever "Location" (atCLAMLinkIO.c:178) and indexes `bm->pss[pssIndex]` without a range check against the number of point sources. Function: `Scale_PointSource()`. | Validate `0 <= pssIndex < bm->npss` (or equivalent) in Read_Lever_List. | [ ] |


#### atCLAMLink/atCLAMLink.c


No issues found. Not compiled: it is absent from Makefile.am `c_sources`, its header comment says "This page is not used", and it references the undefined `numOutputOptions`. Functions checked: none (data tables only).


#### atCLAMLink/atCLAMLinkIO.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| LINK-08 | 342, 542-544 | wrong variable | H/H | `indicators[indicatorID].scenarioLinks[linkID] = scenarioID;` — `linkID` is initialised to 0 and never incremented, while `numScenarioLinks++` counts up. Every link overwrites `scenarioLinks[0]`, and entries 1..n-1 stay uninitialised (malloc'd). Run_CLAM (atCLAMManage.c:241-245) then uses those garbage values as `CLAMScenarioOptions[scenarioID]` indices: out-of-bounds write and wrong scenarios run. There is also no check against MAXSCENARIOS. Function: `Read_CLAM_Link_Params()`. | Use `scenarioLinks[indicators[indicatorID].numScenarioLinks]` and quit if it reaches MAXSCENARIOS. | [ ] |
| LINK-09 | 396, 423-458 | uninitialised memory | H/H | `CLAMScenarioOptions = malloc(...)`: only the strategies present in the XML are filled, and `isLocked`/`timeLockOutStarted` are never initialised anywhere (grep shows them set only in atCLAMImplement.c:56-57 after a strategy is implemented). Run_CLAM tests `isLocked == TRUE/FALSE` (atCLAMManage.c:262, 311, 410) on garbage, so strategies can be silently skipped or treated as locked. Run_CLAM also loops `strategyIndex < maxOptionValue`, which can exceed the number of strategies read (garbage `isEnabled`, `numLevers`). Function: `Read_CLAM_Link_Params()`. | Use calloc, or explicitly set isLocked=FALSE, timeLockOutStarted=0 and isEnabled=0 for every strategy slot. | [ ] |
| LINK-10 | 413, 424, 147 | missing bounds check | H/H | `maxOptionValue`, `numStrategyOptions` and `numLevers` come from XML with no limit, but `strategies[]` is fixed at maxStrategyOptions (7) (atCLAMLinkPrivate.h:59) and `strategyLevers[]` at 20 (line 47). More than 7 strategies or 20 levers in the XML overruns the struct. Run_CLAM also indexes `strategies[strategyIndex]` up to maxOptionValue-1. Functions: `Read_CLAM_Link_Params()`, `Read_Lever_List()`. | Quit if numStrategyOptions or maxOptionValue > maxStrategyOptions, or numLevers > 20. | [ ] |
| LINK-11 | 470-471 | NULL dereference | H/M | `printf("...", indicatorList->nodesetval->nodeNr); if (indicatorList && ...)` — dereferenced before the NULL check. Function: `Read_CLAM_Link_Params()`. | Move the printf inside the `if`. | [ ] |
| LINK-12 | 344, 421, 565 | uninitialised / leak | M/M | `xmlXPathFreeObject(strategyList);` frees only the last per-scenario list (the others leak). If `scenarioList` is NULL or empty, `strategyList` is uninitialised and freeing it is UB. Function: `Read_CLAM_Link_Params()`. | Free strategyList inside the scenario loop, and initialise it to NULL. | [ ] |
| LINK-13 | 101 | format/arg mismatch | H/L | `quit("Lever name '%s' not recognised\n");` — `%s` with no argument (UB, garbage or crash in the error path). Function: `Get_Lever_Type()`. | Pass `name`. | [ ] |
| LINK-14 | 396, 476 | unchecked malloc | M/L | `malloc` results for CLAMScenarioOptions and indicators are not checked. Function: `Read_CLAM_Link_Params()`. | Check for NULL and quit. | [ ] |
| LINK-15 | 220-222 | uninitialised field | M/M | For Q_CHANGE levers, `groupCode = Util_Get_FG_Index(...)` is not checked for -1 (the LINEAR_MORT_CHANGE branch does check, lines 202-204). Q_CHANGE also never reads "Location", yet Implement_Strategy passes `lever->location` (uninitialised) to Harvest_Scale_Q (atCLAMImplement.c:40). Function: `Read_Lever_List()`. | Check groupCode == -1, and read "Location" for Q_CHANGE. | [ ] |


#### atCLAMLink/atCLAMLinkSetup.c


No issues found. Allocation orders were checked against the Init calls in atCLAMManage.c:199-203 and are consistent ([ind][scen][strat][state], [scen][strat][ind], [scen][strat]). Functions checked: CLAM_Init, Allocate_CLAM_Arrays, Free_CLAM_Arrays, CLAM_Free, Calculate_Max_Output_States.


#### atCLAMLink/atCLAMManage.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| LINK-16 | 440 | off-by-one / wrong condition | H/H | `if (bestScenarioIndex > 0 && bestStrategyIndex > 0) Implement_Strategy(...)` — indices are 0-based and -1 means "not found" (lines 405-406). A best result in scenario 0 or strategy 0 is never implemented. Function: `Run_CLAM()`. | Use `>= 0` on both. | [ ] |
| LINK-17 | 592-594 | copy-paste | H/M | `"Change in Seagrass Area"` returns `Assess_Get_Physical_Sampled_Value(bm, o2_wc_id, b)`, the dissolved oxygen tracer copied from the block above. The seagrass indicator is therefore driven by O2. Function: `Get_Box_Indicator_Value()`. | Return the seagrass cover/area value (or quit as "not implemented"). | [ ] |
| LINK-18 | 155-163, 616-617 | uninitialised / div-by-zero | M/M | `currentValue` is only set in the STANDARD_DEV branch (line 163), not for PERCENT indicators. The REACTIVE_MANAGER weighting divides by `indicators[i].currentValue` (uninitialised malloc memory, or 0). Functions: `CLAM_Check_Indicators()`, `Determine_Indicator_Weightings()`. | Set `currentValue = indicatorValue` in both branches, and guard against 0. | [ ] |
| LINK-19 | 114-119, 152, 298, 348 | stale value / div-by-zero | M/M | In the second loop every indicator gets `runScenarios = TRUE` (line 117; the comment above says "not running" — a copy-paste of line 74). `targetChange` is only updated when `requiredChange != 0`, so an indicator in range keeps a stale or uninitialised `targetChange`. Run_CLAM then divides by `fabs(targetChange)` (line 348), which gives inf/NaN when it is 0. Functions: `CLAM_Check_Indicators()`, `Run_CLAM()`. | Set targetChange = 0 for in-range indicators and skip the distance division when it is 0. | [ ] |
| LINK-20 | 157 | div-by-zero | M/M | `requiredChange = requiredChange / indicatorValue * 100;` — no guard if the sampled indicator value is 0 (e.g. algae or seagrass absent). Function: `CLAM_Check_Indicators()`. | Guard `indicatorValue != 0`. | [ ] |
| LINK-21 | 96, 138 | logic | L/M | `if (change > requiredChange \|\| requiredChange == 0)` selects the most positive change, not the largest magnitude, so when all changes are negative it keeps the smallest decrease. This contradicts the comment "Save if its the max required change". Function: `CLAM_Check_Indicators()`. | Compare `fabs(change) > fabs(requiredChange)`. | [ ] |
| LINK-22 | 335, 348-349 | div-by-zero | L/L | `normaliseVar = variance/varSum` (varSum can be 0; the value is only logged). The distance divides by `(1 - variance)`, which is 0 when variance == 1. Function: `Run_CLAM()`. | Guard the denominators. | [ ] |


#### atCLAMLink/include/atCLAMLink.h


No issues found. It defines (rather than declares) the module globals and initialised tables, which is acceptable because it is included only by atCLAMLinkIO.c. Functions checked: none (data definitions).


#### atCLAMLink/include/atCLAMLinkLib.h


No issues found. Functions checked: prototypes CLAM_Init, CLAM_Free, CLAM_Check_Indicators.


#### atCLAMLink/include/atCLAMLinkPrivate.h


No issues found beyond the fixed array limits (strategies[7], strategyLevers[20], scenarioLinks[MAXSCENARIOS]) that the IO code does not enforce (see LINK-08 and LINK-10). Functions checked: prototypes.


### B.2.12 atlink/

#### atlink/atComms.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| LINK-23 | 321-328 | buffer / sizeof(pointer) / overlap | H/H | `snprintf(str, sizeof(str), "%s,%e", str, ...)` — `str` is a `char *`, so `sizeof(str)` is 8 and the output is truncated to 7 characters. snprintf with the destination also used as a `%s` source is undefined behaviour (overlapping). The resulting data string sent to EwE is garbage. malloc is also unchecked. Function: `buildDataString()`. | Track the offset: `pos += snprintf(str+pos, cap-pos, pos?",%e":"%e", v)` with `cap = numRows*numColumns*50`. | [ ] |
| LINK-24 | 345, 353 | copy-paste / ignored param | H/M | `soap_ctx_new_with_method(EwEUrn, EwEGetBiomassMethod, ...)` and `soap_env_add_item(..., "name", "PL")` — the `method` and `species` parameters are ignored, so getProduction, getMortality, getNutrients and getProperties all request biomass for group "PL". The returned data is also never parsed into `dataArray`. Function: `getParameter()`. | Use `method` and `species`, and parse the response into dataArray. | [ ] |
| LINK-25 | 252 | copy-paste | M/M | `soap_client_invoke(..., EwEServerUrl, "http://tempuri.org/Init")` — the SOAPAction is hard-coded to Init for every method (LoadModel, RunEcospace, RunNextEcospaceTS, runModel). Function: `callBooleanServerFunction()`. | Build the SOAPAction from `method`. | [ ] |
| LINK-26 | 265-268 | leak | L/L | On `checkResponse()==FALSE` it returns without `endClient()`, leaking request, response and the client. checkResponse currently always returns TRUE. Function: `callBooleanServerFunction()`. | Call endClient before returning. | [ ] |


#### atlink/atlinkconversion.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| LINK-27 | 41 | wrong variable / NULL deref | H/H | `Util_Init_2D_Double(..., eweLinkModule->convertPRMS.io_nrow, ...)` uses the global `eweLinkModule` (declared in atLinkModule.h:49, never assigned) instead of `bm->eweLinkModule`, so this dereferences NULL. Function: `LinkToModel()`. | Use `bm->eweLinkModule->convertPRMS.io_nrow`. | [ ] |
| LINK-28 | 212, 223, 229, 247 | wrong variable | H/H | `nid = *PhysioChemArray[fgIndex].tracerIndex;` is computed but the nutrient and physchem blocks read `tr[k][bid]` and `sm.tr[k][bid]`. `bid` is the last biological group's tracer from the previous loop, or uninitialised if no group was exported. Function: `WeightedAvg()`. | Replace `bid` with `nid` in lines 223, 229 and 247. | [ ] |
| LINK-29 | 185 | wrong variable | H/M | Sediment P/B: `bm->boxes[b].sm.tr[k][pid] / (bm->boxes[b].tr[k][bid] + small_num)` divides sediment production by the water-column biomass (and indexes the WC layer with a sediment layer k). Compare lines 174 and 179. Function: `WeightedAvg()`. | Use `bm->boxes[b].sm.tr[k][bid]`. | [ ] |
| LINK-30 | 266 | div-by-zero | H/H | `values[r][c] /= ExportArray[l].area[r][c]; // area created on readin` — `area` is allocated with 0.0 (atlinkexport.c:53-54) and never set anywhere, so every exported value becomes inf/NaN. Function: `WeightedAvg()`. | Fill `area[][]` in LoadCoversionParameters and guard against 0. | [ ] |
| LINK-31 | 223, 229 | unit / placeholder | M/M | Nutrients are multiplied by `dz` = 1.0 ("//REMOVE") instead of `bm->boxes[b].dz[k]` / `sm.dz[k]`, unlike the biomass code (lines 141, 147). Function: `WeightedAvg()`. | Use the per-layer dz. | [ ] |
| LINK-32 | 40, 263 | wrong extent | M/M | Loops over `ExportArray` use `convertPRMS.ingroup`, but ExportArray is allocated with `outgroup` entries (atlinkexport.c:47), and `layer` is incremented once per exported group or tracer with no check against that size. Functions: `LinkToModel()`, `WeightedAvg()`. | Use outgroup for ExportArray and check layer < outgroup. | [ ] |


#### atlink/atlinkexport.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| LINK-33 | 38-55, 91-103 | uninitialised use | H/H | `initialiseLinkModule()` mallocs `eweLinkModule` and immediately uses `convertPRMS.ingroup`, `outgroup`, `io_ncol` and `io_nrow` (uninitialised) as allocation sizes. LoadCoversionParameters only sets these after calling it (lines 91 vs 100-103). Functions: `initialiseLinkModule()`, `LoadCoversionParameters()`. | Set the convertPRMS values before allocating the arrays. | [ ] |
| LINK-34 | 139-140 | copy-paste | H/H | `numrows = nr; numrows = nc;` — the second line should set `numcols`. `numcols` stays uninitialised, and all export/import loops (`nc < export->numcols`) run over garbage bounds. Function: `LoadCoversionParameters()`. | `bm->boxes[b].export->numcols = nc;` | [ ] |
| LINK-35 | 134, 152-154 | wrong index / OOB | H/H | `bcellarea[nr][nc] = area;` inside `for r, for c` — writes the out-of-bounds element [4][4] of an array allocated `alloc2d(nc,nr)` = [nr][nc] = [4][4]. The intended cells [r][c] stay uninitialised. Function: `LoadCoversionParameters()`. | `bcellarea[r][c] = area;` | [ ] |
| LINK-36 | 46-55 | swapped size | M/M | `ImportArray` is sized with `ingroup` and `ExportArray` with `outgroup`, but the init loop (and every use) iterates `ingroup` over both. If outgroup < ingroup this overruns ExportArray. Function: `initialiseLinkModule()`. | Allocate and loop each array by its own count. | [ ] |
| LINK-37 | 66-77 | wrong free / wrong variable | H/M | `free(values)` / `free(area)` on alloc2d arrays frees only the row pointers and leaks the data block (should be free2d). `free(eweLinkModule)` frees the never-assigned global, not `bm->eweLinkModule` (which leaks). Function: `freeLinkModule()`. | Use free2d for the 2D arrays and free(bm->eweLinkModule). | [ ] |
| LINK-38 | 208-225 | swapped args | H/M | `setBiomass(name, values, io_ncol, io_nrow)` passes (numRows=io_ncol, numColumns=io_nrow) to a function that indexes `dataArray[rows][columns]`, but `values` is `[io_nrow][io_ncol]` (alloc2d(io_ncol, io_nrow), line 51). Columns run to 71 when the row length is 60, so the read runs past the row and past the end of the last row. The same applies to all set* calls here and to all get* calls in atlinkimport.c:136-148. Function: `SendArrays()`. | Pass `io_nrow, io_ncol`. | [ ] |
| LINK-39 | 165, 208, 229 | uninitialised / copy-paste | M/M | `ExportArray[l].parameterName` is never assigned (an uninitialised pointer passed to SOAP). `dataType = -1` hits the `default: quit("WeightedAvg: ...")` branch if no group set it, and the quit message names the wrong function. Function: `SendArrays()`. | Set parameterName, and correct the message to "SendArrays". | [ ] |


#### atlink/atlinkimport.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| LINK-40 | 62, 71, 81 | wrong index / array | M/M | `area / ExportArray[layer].area[nr][nc]` indexes the EwE-grid area array with the box-local overlap indices `nr,nc` instead of the grid `row,col`, and uses ExportArray rather than ImportArray. That area array is also all zeros (see LINK-30), so this divides by 0. Function: `UnpackArrays()`. | Use `ImportArray[layer].area[row][col]` (once populated), with a zero guard. | [ ] |
| LINK-41 | 62, 71 | logic | M/M | Each overlapping EwE cell overwrites (`=`) the box value instead of accumulating its area-weighted contribution, so only the last cell counts. Function: `UnpackArrays()`. | Sum the cell contributions (zero first, then `+=`). | [ ] |
| LINK-42 | 71, 81 | div-by-zero | M/M | `(origval / totval)` — totval is 0 when the group has no biomass in the box, which gives NaN tracers. Function: `UnpackArrays()`. | If totval == 0, distribute by dz or assign to one layer. | [ ] |


#### atlink/include/atComms.h


No issues found (declares `sayHello`, which has no live definition; harmless unless called). Functions checked: prototypes.


#### atlink/include/atLinkModule.h


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| LINK-43 | 49 | global defined in header | M/L | `EwELinkStruct *eweLinkModule;` is a definition in a header included by atlantisboxmodel.h (line 301, under LINK_ENABLED), so it lands in every TU. Under -fno-common (GCC >= 10, Clang default) this gives duplicate-symbol link errors. It is also the stray global misused in LINK-27 and LINK-37. | Remove it (the struct member bm->eweLinkModule exists), or make it `extern`. | [ ] |


#### atlink/include/atlink.h


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| LINK-44 | 26-31 | global defined in header | M/L | `link_calc_types`, `link_data_types`, `link_data_entries` and `ExportName` are defined (not `extern`) in a header included by three .c files. This is a duplicate-symbol error under -fno-common. | Declare them `extern` in the header and define them once in a .c file, or delete them (unused). | [ ] |


### B.2.13 atbrokerlink/

#### atbrokerlink/atBrokerLinkInit.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| LINK-45 | 73-74 | missing bounds check | M/H | Polygon `id`s from the XML are stored in `polygonList` and later used directly as box indices `b` in Link_GetBiomass, Link_SetBiomass, Link_SetMortality and Link_SetDetritus with no `0 <= id < bm->nbox` check. Out-of-range ids read and write out of bounds. Function: `Linkage_Read_XML()`. | Validate each id against bm->nbox. | [ ] |
| LINK-46 | 55 | unchecked malloc | M/L | `bm->linkageInterface = malloc(...)` is not checked; numPolygons and other fields are uninitialised until set. Function: `Linkage_Read_XML()`. | Check for NULL, or use calloc. | [ ] |
| LINK-47 | 93-98 | infinite loop / protocol | M/M | When `Linkage_DeserialiseCommand` returns NULL (unknown request type, see LINK-50) no reply is sent. A ZMQ_REP socket must send before the next recv, so every following `zmq_recv` fails (EFSM). Network_Receive ignores the failure and returns an empty message, which makes `requests__request__unpack` fail and `exit(1)`. Function: `Linkage_Start()`. | Always send a (failure) response, and abort the loop on receive errors. | [ ] |


#### atbrokerlink/LinkageInterface.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| LINK-48 | 23-24, 82, 87 | unchecked malloc | M/L | `malloc` for the Network struct, polygonList and valueList is unchecked. Calling Linkage_SetNumPolygons a second time leaks the previous polygonList. Functions: `Linkage_Initialise()`, `Linkage_SetNumPolygons()`. | Check for NULL, and free the old polygonList. | [ ] |
| LINK-49 | 25 | type mismatch | L/L | `group = -1` is assigned to an `unsigned int group` field (atBrokerLink.h:21), so a "-1 unset" test can never be true. Function: `Linkage_Initialise()`. | Make the field `int`. | [ ] |


#### atbrokerlink/Deserialiser.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| LINK-50 | 57-153 | missing default / leak | M/M | The `switch (request->type)` has no `default`: an unrecognised type leaks the unpacked request (never `requests__request__free_unpacked`) and returns NULL, which causes the protocol failure in LINK-47. Function: `Deserialise_Message()`. | Add a default that frees the request and returns a FALSE boolean response. | [ ] |
| LINK-51 | 256-261, 280-293, 316-321, 339-351, 373-378, 396-408, 164-167 | NULL dereference | H/M | Every sub-message NULL check only prints, then dereferences anyway (`biomassRequest->groupid`, `request->n_values`, `requestInit->timestep`). `groupid` and `timestep` (char*) can also be NULL in protobuf-c. A malformed broker message crashes Atlantis. Functions: `Deserialiser_Init()`, `Deserialiser_GetBiomass()`, `Deserialiser_SetBiomass()`, `Deserialiser_GetMortality()`, `Deserialiser_SetMortality()`, `Deserialiser_GetDetritus()`, `Deserialiser_SetDetritus()`. | Return or exit after the error message, and check groupid/timestep for NULL. | [ ] |
| LINK-52 | 261, 293, 321, 351, 378, 408 | buffer overrun (network input) | H/H | `strcpy(interface->groupName, request->groupid)` copies an unbounded network string into `char groupName[1024]` (atBrokerLink.h:22): a remote input overflows a fixed buffer. Same six functions as LINK-51. | `snprintf(interface->groupName, sizeof(interface->groupName), "%s", groupid)` and reject over-long names. | [ ] |
| LINK-53 | 115-121 | copy-paste | M/M | The GETDETRITUS response calls `Link_GetBiomass(...)` rather than `Link_GetDetritus` (which happens to wrap GetBiomass now). GETMORTALITY returns dummy `data[i] = i` (lines 109-111). The broker gets fake mortality data with a TRUE response. Function: `Deserialise_Message()`. | Call Link_GetDetritus, and implement mortality or return a failure response. | [ ] |
| LINK-54 | 139-152 | missing call | M/M | SETMORTALITY and SETDETRITUS unpack values into `interface->valueList` but never call `Link_SetMortality` / `Link_SetDetritus` (the line is commented out). The broker data is silently discarded while TRUE is returned. Function: `Deserialise_Message()`. | Call Link_SetMortality / Link_SetDetritus with valueList. | [ ] |
| LINK-55 | 74-78 | ignored return | L/M | `halt = Link_RunNextTimeStep(bm);` — it is printed only, and the broker always receives TRUE even if the model signalled halt or failure. Function: `Deserialise_Message()`. | Return `!halt` (or the appropriate status) in the boolean response. | [ ] |
| LINK-56 | 288 | format specifier | M/L | `%ld` for a `size_t length` (and `%d` for an `unsigned int numPolygons`). This is wrong on LLP64 Windows. Function: `Deserialiser_SetBiomass()`. | Use `%zu` and `%u`. | [ ] |
| LINK-57 | 180-189, 199-211 | unchecked malloc | M/L | `malloc` results for message and bytes are not checked before `pack()` writes into them. `sizeof(void)` is a GNU extension (1). Functions: `Serialise_Boolean_Response()`, `Serialise_Data_Response()`. | Check for NULL, and use `malloc(message->size)`. | [ ] |


#### atbrokerlink/ImportExportData.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| LINK-58 | 312, 318 | wrong index | H/H | `linkageWCDetritusFlux[b][k][guild] = boxValues[b] / 100 * ...` — `boxValues` has `numBoxes` entries indexed by the polygon position `i` (compare `boxValues[i]` in Link_SetBiomass:126-135 and Link_SetMortality:401-410), but it is indexed by the box id `b`. This gives wrong values, or out-of-bounds reads when b >= numBoxes. Function: `Link_SetDetritus()`. | Use `boxValues[i]`. | [ ] |
| LINK-59 | 88, 126, 144 | accumulator not reset | H/H | `epi = 0` is only set in the first (fraction) loop. In the second loop `epi += boxValues[i]` keeps accumulating across boxes and cohorts, so box n is set to the sum of boxes 0..n. Function: `Link_SetBiomass()`. | Set `epi = boxValues[i];` (or reset epi = 0 at the start of each box iteration). | [ ] |
| LINK-60 | 56-57, 78-109, 130-135 | missing dimension | M/M | `columnFractions[b][k]` / `sedColumnFractions[b][k]` have no cohort dimension. They are computed inside the cohort loop and overwritten, so the second loop uses the last cohort's vertical distribution for every cohort. Function: `Link_SetBiomass()`. | Compute the fractions inside the second cohort loop, or add a cohort dimension. | [ ] |
| LINK-61 | 93-108, 130-135 vs 220-230 | unit mismatch | M/H | Link_GetBiomass returns `tr * mg_2_tonne * dz` (t/m2), but Link_SetBiomass distributes the incoming value directly as concentration: `tr[k] = boxValue * (tr[k]/sum(tr))`, with no tonne_2_mg and no /dz, and the fractions are computed from concentrations rather than mass. A get→set round trip changes biomass by about 1e9/dz. Functions: `Link_SetBiomass()`, `Link_GetBiomass()`. | Convert units symmetrically: fractions from tr*dz, then `tr[k] = val*tonne_2_mg*frac/dz[k]`. | [ ] |
| LINK-62 | 153-163 | wrong condition | M/M | The WC update loop runs for every non-EPIFAUNA group regardless of `habitatCoeffs[WC]`. For a sediment-only group, `tr[k]` is 0, which can zero or prune existing WC tracer values. Compare the SED block, which is guarded (line 165). Function: `Link_SetBiomass()`. | Wrap it in `if (habitatCoeffs[WC] > 0)`. | [ ] |
| LINK-63 | 51, 145-149 | logic | L/L | With `tolerance = 1.0`, `+= tolerance * difference` sets the value equal to the imported one, so the "pruning" branch does exactly what the else branch does and only adds a warning. Function: `Link_SetBiomass()`. | Use a tolerance < 1 or cap the step as intended. | [ ] |
| LINK-64 | 263, 390 | copy-paste message | H/L | Link_SetDetritus logs "Link_SetBiomass"; Link_SetMortality reports "Link_GetBiomass - group code not recognised". Functions: `Link_SetDetritus()`, `Link_SetMortality()`. | Fix the function names in the messages. | [ ] |


#### atbrokerlink/network.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| LINK-65 | 19-21 | unchecked return | H/M | The return values of `zmq_init`, `zmq_socket` and `zmq_bind` are ignored. A bind failure (port in use, bad URL) leaves Atlantis blocking forever in recv with no diagnostic. Function: `Network_Initialise()`. | Check each for NULL/-1, then printErrorMessage and quit. | [ ] |
| LINK-66 | 31-40 | error path continues | H/M | When `zmq_recv` returns -1 the code prints an error and then still builds a message from the uninitialised/empty msg (size 0, `malloc(0)` can be NULL), and closes it. The caller cannot tell a failure from a valid message. The malloc results are not checked. Function: `Network_Receive()`. | On error, close the msg and return NULL; check the mallocs. | [ ] |
| LINK-67 | 52-61 | leak / unchecked | M/L | `zmq_msg_init_size` result unchecked. On a send failure it returns without `zmq_msg_close(&response)`. Function: `Network_Send()`. | Check init and always close the msg. | [ ] |


#### atbrokerlink/NetworkError.c


No issues found. Functions checked: printErrorMessage.

#### atbrokerlink/include (Deserialiser.h, ImportExportData.h, LinkageInterface.h, Message.h, NetworkError.h, atBrokerLink.h, network.h)

| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| LINK-68 | Deserialiser.h:12 | prototype mismatch | M/L | `double Deserialiser_Init(void *bytes);` (external linkage, void*) conflicts with the `static double Deserialiser_Init(Requests__Request *)` in Deserialiser.c:25. It only compiles because Deserialiser.c does not include this header. LinkageInterface.h also declares functions that do not exist (Linkage_SetPolygons, Linkage_SerialiseCommand), and network.h declares Close_Message. | Remove the stale prototypes. | [ ] |
| LINK-69 | ImportExportData.h | missing prototype | L/L | `Link_SetDetritus` and `Link_GetMortality` are defined in ImportExportData.c but not declared, so any future caller gets an implicit declaration. | Add the prototypes. | [ ] |


### B.2.14 sjwlib/

#### sjwlib/memory.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SJW1-01 | 46, 51, 86, 150, 232, 336, 456, 574, 579, 616, 680, 762, 865, 985, 1104, 1145, 1210, 1291, 1384, 1426, 1490, 1564, 1606, 1669, 1751, 1845, 1885, 1948, 2024, 2110, 2149, 2212, 2288 | format specifier | H/L | `quit("...size %d, %d\n", n1, n2)` where `n1..n6` are `long int`, and `quit("...size = %d\n", size)` where `size` is `size_t`. `%d` with a 64-bit argument is undefined behaviour and prints the wrong value on LP64, so the error message reports the wrong size just when someone needs it to debug a bad allocation. Functions: every `*_alloc1d..6d` / `d_alloc*longd`. | Use `%ld` for the `long` dims and `%zu` for `size`. | [ ] |
| SJW1-02 | 118, 191-193, 283-287, 395-401, 523-531, 648, 721-723, 813-817, 924-930, 1052-1060, 1178, 1250-1252, 1342-1346, 1458, 1531-1533, 1637, 1710-1712, 1802-1806, 1916, 1987-1989, 2073-2077, 2180, 2251-2253, 2337-2341 | zero-size / free mismatch | M/M | The allocators accept a dimension of 0 (they only reject `n<0`). If an outer dimension is 0 (e.g. `d_alloc2d(n1,0)`, or `n2`/`n3` == 0 for 3d), the pointer block is `malloc(0)` and none of its entries are set. The matching free then reads `pp[0]` / `ppp[0][0]` / `pppp[0][0][0]` out of that zero-byte block and passes the garbage to `free()`: a heap over-read and an invalid free (crash at shutdown). On platforms where `malloc(0)` returns NULL the allocation instead quits with a false "Not enough memory". Functions: all `*_free2d..6d` paired with the `*_alloc2d..6d` routines. | Either reject zero dims in the allocators, or store the base pointers so the frees don't dereference empty blocks (e.g. always allocate `max(n,1)` pointer slots and set them). | [ ] |
| SJW1-03 | 153-160, 235-250, 339-362, 459-490, 619, 683-690, 765-780, 868-891, 988-1019, 1148, 1213-1220, 1294-1309, 1429, 1493-1500, 1609, 1672-1679, 1754-1769, 1888, 1951-1958, 2027-2042, 2152, 2215-2222, 2291-2306 | integer overflow | L/L | `size = (size_t)(n1 * n2 * n3) * sizeof(double)`: the product is formed in `long` before the cast. That is fine on LP64 (macOS/Linux), but on LLP64 Windows (the code has `WIN32` branches) `long` is 32-bit, so more than 2^31 elements overflow and under-allocate. The loop bound `i < n1*n2*n3` overflows the same way. Only `d_alloc2longd` (line 89) casts each factor correctly. Functions: all typed `*_alloc2d..6d`. | Cast each factor: `(size_t)n1 * (size_t)n2 * (size_t)n3`, and use a `size_t` loop index. | [ ] |
| SJW1-04 | 308, 1827, 2093 | copy-paste (message) | H/L | The 4d free routines report `"free3d: attempt to free NULL pointer"` for the main-storage check (copied from the 3d version). Functions: `d_free4longd()`, `i_free4d()`, `s_free4d()`. | Change the text to `free4d` / `free4longd`. | [ ] |


#### sjwlib/datafile.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SJW1-05 | 171-174 | wrong operator | H/M | `if ((varid < 0) && (varid >= df->nv))` can never be true, so an invalid `varid` is never caught. `rv = &df->variables[varid]` then indexes out of bounds and `rv->nd` / `rv->dimids` are read from garbage. Compare the correct check in `dfGetVariable()` line 591. Function: `dfSetRecord()`. | Use `\|\|` instead of `&&`. | [ ] |
| SJW1-06 | 317-321 | wrong formula (recycling) | H/H | When rewinding is on and `r` is past the last record: `leftover = floor(r / lengthdf); *newt = r - leftover * df->records[ilow];`. The code multiplies by the first record time instead of the period length `lengthdf`, and the floor ignores the record origin. If the series starts at t=0 (`records[0]==0`), `newt == r`, so nothing is recycled and `dfEval` clamps to the last record for the rest of the run. For any other start time the result is an arbitrary time. This is used for the rewound thermal and rate index series (`tsEvalR(..., bm->tindex_rewindid)` in atecology/atdemography.c:1503-1504 and atannualbiology.c:1241-1242) and in `tsEvalXYR`/`tsEvalXYZR`. Function: `dfFindTime()`. | `*newt = df->records[ilow] + fmod(r - df->records[ilow], lengthdf);` (guard `lengthdf > 0`). | [ ] |
| SJW1-07 | 214-231 (with 1013-1015) | out-of-bounds read | M/M | `df_netcdf_read` allocates `v->dimids = malloc(v->nd*sizeof(int))`, which is a zero-byte block for scalar (0-d) netCDF variables. `dfSetRecord` then tests `if (v->dimids[0] == recdimid)` for every variable without checking `v->nd > 0`. That reads uninitialised heap memory. If the garbage equals `recdimid` (often 0), the scalar gets `dim_as_record=1` and `--(v->nd)` makes `nd == -1`, so later reads quit with "Bad number of dimensions" or misbehave. Function: `dfSetRecord()`. | `if (v->nd > 0 && v->dimids[0] == recdimid)`. | [ ] |
| SJW1-08 | 1320 | type mismatch / memory overwrite | H/L | `nc_inq_att(..., &type, (size_t *) &a->n)`, but `Attribute.n` is declared `int` (datafile.h:138, the `size_t n` is commented out at :137). netCDF writes 8 bytes into a 4-byte field and overwrites the adjacent `a->type`. On little-endian hardware this happens to be harmless (high word 0, and `type` is reassigned afterwards except for unsupported types or `n==0`). On big-endian hardware `a->n` would always be 0. This is undefined behaviour either way. Function: `df_netcdf_read_attrib()`. | Read into a local `size_t len;` and then set `a->n = (int)len;`. | [ ] |
| SJW1-09 | 1507-1510 (with 371-385) | wrong bound check | M/M | `df_read_data` rejects any `r >= df->nrecords`, even for variables that do not use the record dimension. In a netCDF file with no record/time variable, `df->nrecords == 0`, so `dfReadRecord(df, v, 0)` on any static variable (e.g. coordinate reads in `df_set_2d_coords` line 1392, or `dfEval` on a file without time) quits with "Attempt to read an invalid record". `dfReadRecords` applies the same bound only when `v->dim_as_record` is set (line 372). Function: `df_read_data()`. | Apply the `r` range check only when `v->dim_as_record`; otherwise require `r == 0`. | [ ] |
| SJW1-10 | 1237-1246, 1447-1458 (via 508-509) | free of non-pointer | M/M | ASCII columns are stored as a flat `double[n]` with `nd = 1`, `nrecords = 1`, `dim_as_record = 0`. They only become `nd = 0` if `dfSetRecord` runs (i.e. a `t`/`time` column exists, timeseries.c:108-114). Without such a column, `dfFree` → `df_free_data_records` → `df_free_data_record` case 1 runs `free1d(VAR_1D(v)[0])`, which treats the bit pattern of the first data value as a pointer and frees it. That crashes at shutdown unless the value happens to be 0.0. Functions: `df_ascii_read()`, `df_free_data_record()`. | In `df_free_data_records` (or `dfFree`), just `free(v->data)` when `df->type == DFT_ASCII`. | [ ] |
| SJW1-11 | 1655-1661 | off-by-one / sign error | M/L | When clamping at the upper end: `nrecs -= ((int)df->nrecords - (nstart + v->nrecords));` adds the overshoot instead of subtracting it (the subtraction is reversed), and `nstart = df->nrecords - v->nrecords - 1;` is one short (it should be `df->nrecords - v->nrecords`). The function is currently not called anywhere (dead code), so this is latent. Function: `df_shift_data()`. | `nrecs -= (nstart + v->nrecords - df->nrecords); nstart = df->nrecords - v->nrecords;`, or delete the unused function. | [ ] |
| SJW1-12 | 1116, 1132-1136 | buffer underflow | M/L | The leading-trim loop skips `' '`, `'\t'` and `'\n'` but not `'\r'`, while the trailing loop also strips `'\r'`. A string of only `"\r"` characters (e.g. a CRLF `.ts` header line with an empty value) is not emptied by the first pass, so the trailing loop decrements `i` below 0 and reads `s[-1]`. Function: `trim()`. | Add `i >= 0 &&` to the trailing loop condition and `'\r'` to the leading set. | [ ] |
| SJW1-13 | 1379-1402, 1454-1475 | type mismatch (latent config) | M/L | `df_alloc_data_record` / `df_free_data_record` use the `alloc1d..4d` / `free1d..4d` macros. With `_ATLANTIS_LONG_DOUBLE` defined (atlantisMem.h:20-43), those become `d_alloc*longd`, which return `long double` storage. But `VAR_xD` casts to `doubleINPUT` (= `double`) and `nc_get_vara_double` writes `double`s, so the buffers would be misinterpreted. atlantisMem.h:69-80 provides `d_alloc*dInput` precisely for input buffers. This is latent because the macro is commented out. Functions: `df_alloc_data_record()`, `df_free_data_record()`. | Use `d_alloc1dInput..d_alloc4dInput` / `d_free*dInput`. | [ ] |


#### sjwlib/dfcoords.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SJW1-14 | 765-771 | missing braces / uninitialised | H/L | `if (cm->transform != NULL) cm->transform->inverse(..., tcoords, ttcoords); for (j...) tcoords[j] = ttcoords[j];`: the copy loop is outside the `if`. With no transform (the normal case) it overwrites the computed coordinates with uninitialised `ttcoords`. The forward path `dfCoordsToIndices` (lines 698-703) has the loop inside braces. Severity is low only because `dfIndicesToCoords` currently has no callers. Function: `dfIndicesToCoords()`. | Wrap the copy loop in braces with the `inverse` call. | [ ] |
| SJW1-15 | 1223-1227 | off-by-one OOB read | H/L | `if ((i < 0) \|\| (i > ihigh)) return 0; coords[0] = (indices[0]-i)*(data[i+1]-data[i]) + data[i];`. When `i == ihigh` (the last index, a valid input) this reads `data[ihigh+1]`, one past the end of the coordinate array. Reached only through `dfIndicesToCoords` (unused). Function: `cm_1d_itoc()`. | Return `data[ihigh]` when `i == ihigh` (or test `i >= ihigh` and handle it separately). | [ ] |
| SJW1-16 | 1285-1287 | always-true condition | H/L | `i = (i < 0) ? 0 : i; i = (i >= 0) ? dsize - 1 : i;`: after the first clamp `i >= 0` is always true, so every index maps to the last point `dsize-1`. The upper clamp should test `i >= dsize`. Reached only through `dfIndicesToCoords` (unused). Function: `cm_1d_multi_itoc()`. | `i = (i >= dsize) ? dsize - 1 : i;` | [ ] |
| SJW1-17 | 1460-1471 | copy-paste (wrong buffer) | H/L | `buf` is filled from `int_proj` once (line 1460) and `parseline` tokenises it in place for `int_mp`. The `ext_geotype == GT_PROJECTION` branch then calls `parseline(buf, ...)` again without first copying `ext_proj`, so `ext_mp` is built from the internal projection (or its already-split remains). Also, when `int_geotype == GT_GEOGRAPHIC`, `int_proj` is NULL and is passed to `%s`, which is undefined behaviour. Latent because `dfSetGeoType` / `dfSetDefaultGeoType` are never called, so `ext_geotype` stays `GT_NONE`. Function: `df_create_geo_transform()`. | Do `snprintf(buf, sizeof(buf), "%s", ext_proj);` before the ext parse, and only format `int_proj` when it is non-NULL. | [ ] |
| SJW1-18 | 533, 548 | ignored return value | M/L | The return value of `dfSetCoordSystem(df, v, ncm, cmaps)` is ignored. It can fail with `return 0` (duplicates, non-coordinate dims) and leave `v->csystem == NULL`, but `dfInferAndSetCoordSystem` still returns 1. Callers (`tsEvalXYR`/`tsEvalXYZR`) therefore skip their fallback path and later die in `dfCoordsToIndices` with "No coordinate system specified". On the `failed` branch, `cmaps[0..ncm-1].coordids/coordtypes` are also leaked. Function: `dfInferAndSetCoordSystem()`. | `int ok = dfSetCoordSystem(...); free cmaps...; return ok;` and free the cmaps on the failed path too. | [ ] |
| SJW1-19 | 945-971, 1546-1549 | memory leak | H/L | `df_free_coord_system` never frees `cmaps[i].transform`: neither the `CoordinateTransform`, nor its `private_data`, nor the `MapProjection`s. `geo_free()` frees the `GeoTransformData` but not `int_mp` / `ext_mp`. Functions: `df_free_coord_system()`, `geo_free()`. | Call `cm->transform->free(cm->transform->private_data); free(cm->transform);` and free the map projections inside `geo_free`. | [ ] |
| SJW1-20 | 123-124, 136-171, 181, 199-202 | memory leak | H/L | Each early `return 0` after line 124 leaks `coordids`/`coordtypes`, and the duplicate check's early returns also leak the `mp->dimids`/`depdimids` allocated by earlier calls. `v->ncdimids` is `malloc`ed on every call without freeing the previous allocation. Function: `dfSetCoordSystem()`. | Free `coordids`/`coordtypes` before every early return, and `free(v->ncdimids)` before reallocating it. | [ ] |
| SJW1-21 | 815; 374, 403-405 | buffer overrun | M/L | `strcpy(line, coords)` copies a netCDF `coordinates` attribute of arbitrary length into `char line[MAXLINELEN]` without a length check (`df_decode_coords()`). In `dfInferAndSetCoordSystem()`, `coordids[MAXNUMCOORDS]` (10) is filled with up to `v->nc` entries without a bound check, and `v->nc` can exceed 10 for a long coordinate list. | Use `snprintf`/`strncpy` with `sizeof(line)`, and quit or warn if `v->nc > MAXNUMCOORDS`. | [ ] |
| SJW1-22 | 1324-1345, 1348-1370 | memory leak | H/L | `rgi` / `pgi` are `malloc`ed before the `sscanf(...)==10` test. If the analytic attribute is malformed, the code falls through to the bilinear mapping without freeing them. Function: `df_set_2d_coords()`. | Free `rgi` / `pgi` when the `sscanf` check fails. | [ ] |


#### sjwlib/mapproj.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SJW2-01 | 298-347, 561-604 | invalid free / realloc of caller memory | H/H | `add_argument()` copies the caller's string pointers into the new array (`new_args[i+1] = args[i]`, line 587). Then `subst_argument_value("proj", ...)` calls `realloc(args[index], ...)` (line 599) on the caller's `proj=` string. The amg/mga branches also `free()` every `new_args[i]` (lines 303-307, 316-320), which includes the caller's original strings. Every caller builds `args` with `parseline()` (sjwlib/text_input.c:85), which returns pointers into a stack buffer (atphysics/atgeomIO.c:788-807, atecology/dump.c:143-152, sjwlib/dfcoords.c:1463-1470). So `proj=utm`, `proj=amg` or `proj=mga` calls realloc/free on non-heap memory, which is undefined behaviour and on macOS aborts with "pointer being realloc'd was not allocated". The amg/mga path also leaves dangling pointers in the caller's array. Function: `mpInitialise()` / `add_argument()` / `subst_argument_value()`. | In `add_argument` use `strdup(args[i])` (and honour `freeOld`), so that the code owns every string it reallocs or frees. | [ ] |
| SJW2-02 | 401-405 | wrong formula | H/M | `mp->ellip_flat = 1 / (1 - sqrt(1 - ecc * ecc));` In PROJ, `es` is the eccentricity squared, and flattening is `f = 1 - sqrt(1 - es)`. The code squares `es` again and takes the reciprocal. With `es=0` (sphere) this gives `1/0 = inf`; for real ellipsoids it gives about 300 instead of 0.0033. Every later `e2` in `merc_init`/`tcc_init`/`lcc_init` is then wrong. Function: `mpInitialise()`. | `mp->ellip_flat = 1 - sqrt(1 - atof(es));` | [ ] |
| SJW2-03 | 561-590, 1043-1051 | memory leak | H/L | `add_argument()` ignores `freeOld` (that code is commented out), so each chained call leaks the previous pointer array. The utm branch never frees `new_args` (lines 342-346 are commented out), and `serov_init` leaks too. The allocation `malloc(sizeof(char *)*strlen(arg)+1)` (line 583) over-allocates by 8x. Function: `add_argument()`. | Free the old array when `freeOld`, and allocate `strlen(arg)+1`. | [ ] |
| SJW2-04 | 548-556 | prefix match | M/L | `strncasecmp(tag, args[i], strlen(tag))` matches any argument that starts with the tag. So `get_argument_value("a", ...)`/`("b", ...)` (lines 382-383) match unrelated arguments such as `axis=...` or `b...`, and `"lat_0"`-style lookups also match longer keys. `atof` of the wrong value then silently sets the ellipsoid axes. Function: `find_argument()`. | Also require `args[i][strlen(tag)]` to be `'='` or `'\0'`. | [ ] |


#### sjwlib/polyline.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SJW2-05 | 481, 536-541 | memory leak | H/L | `ydd = alloc1d(pl->np)` is never freed (only x1, y1, d, x2 and y2 are). Function: `smoothpolyline()`. | Add `free1d(ydd);` | [ ] |
| SJW2-06 | 705-711, 714-720 | memory leak | H/L | `new = reverseline(...)` then `clearpolyline(new)` frees the points but not the `polyline` struct from `createpolyline()`, so it leaks on every reverse join. This runs in a loop from contour.c:125-127. Function: `joinsegments()`. | Use `freepolyline(new)` instead of `clearpolyline(new)`. | [ ] |
| SJW2-07 | 962-972 | algorithm assumption / leak | M/L | The entry/exit test casts a ray from `(0, y)` to the point (`pt.x = 0`). This is only valid if x=0 lies outside the polygon, so it gives wrong entry flags (union instead of intersection) for geometry that spans x<=0, e.g. negative longitudes. `left` is also malloc'd on every call and never freed. No current callers of `polyclip`, so the defect is latent. Function: `wa_test()`. | Start the ray at `min(bound.x) - 1` (or reuse `pointinpoly`), and `free(left)`. | [ ] |


#### sjwlib/nrcdflib.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SJW2-08 | 103 | wrong formula (copy-paste) | H/M | `return (0.5 * erfc(-0.7071... * (x - mu )/ sigma));` is the normal CDF formula. The lognormal CDF (NR3 Lognormaldist) uses `log(x) - mu`. Currently no callers, so the defect is latent. Function: `lognorm_cdf_values()`. | Use `(log(x) - mu)`. | [ ] |
| SJW2-09 | 141-143 | wrong formula (copy-paste) | H/M | The Cauchy CDF returns the lognormal formula `0.5*erfc(-0.707*(log(x)-mu)/sigma)` and the lognormal `x==0` guard. The Cauchy CDF is `0.5 + atan2(x-mu, sigma)/M_PI` (NR3 Cauchydist), defined for negative x too. It looks like the normal and lognormal bodies were pasted into the wrong functions. Currently no callers. Function: `cauchy_cdf_values()`. | `return 0.5 + 0.318309886183790671*atan2(x-mu, sigma);` and remove the x==0 guard. | [ ] |
| SJW2-10 | 309 | wrong formula | H/M | `1.0 - exp(pow((- x / beta), alpha))` should be `1 - exp(-pow(x/beta, alpha))`. `pow` of a negative base with non-integer alpha returns NaN. With alpha=2 it gives `1-exp(+(x/beta)^2)`, which is negative. It is only correct for alpha=1. Currently no callers. Function: `weibull_cdf_values()`. | `return 1.0 - exp(-pow(x / beta, alpha));` | [ ] |
| SJW2-11 | 192-194 | ignored parameters | M/L | The two-tailed branch uses the raw `t` and ignores `mu`/`sigma`, while the one-tailed branch standardises `(t-mu)/sigma`. Function: `studentt_cdf_values()`. | Use `step1 = (t-mu)/sigma` in the two-tailed branch as well. | [ ] |
| SJW2-12 | 645 | typo vs source | M/L | `if (a > 3000 && b > 300)`: NR3 uses SWITCH=3000 for both a and b. With b in (300, 3000] the code switches to the quadrature approximation outside its intended range, which reduces accuracy. This also affects F/beta/binomial/negbinomial CDFs, and negbinomial is live (atmanage/atManageSetup.c:1551). Function: `betai()`. | `b > 3000` | [ ] |


#### sjwlib/txt_param.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SJW2-13 | 670-677 (used throughout, e.g. 144-145, 166-167) | format string | M/L | `quit(msg)` / `warn(msg)` pass a message that contains the file's line text (`"...format bad on line>> %s"`) as the format string. A `%` in a parameter file line (common in comments and units) then produces garbage output or a crash inside the error path. Function: `param_error()` (and the direct `warn(msg)` calls in each `*param()`). | `quit("%s", msg)` / `warn("%s", msg)` | [ ] |


#### sjwlib/timeseries.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SJW2-14 | 337-361 | NULL dereference | M/M | `ATT_DOUBLE(x0,0)` etc. expand to `(a)->type` (datafile.h:126,131), but `dfGetGlobalAttribute` returns NULL when `xorigin`/`dx`/`rotation`/`arc`/`rmin` is missing, which segfaults. `str = ATT_TEXT(gt)` is NULL if `gridtype` is not text, and is then passed to `strcasecmp`. Function: `tsEvalXYR()`. | Check each attribute for NULL (and `str != NULL`) before use; quit with a clear message. | [ ] |
| SJW2-15 | 601-614 | stale copy | M/L | After converting units, `ts->t_units`, `df->rec_units` and `df->variables[ri].units` are updated, but the separate copy `ts->varunit[df->ri]` (strcpy'd in `tsRead`, lines 162-168) still holds the old units. Function: `tsNewTimeUnits()`. | Also update `ts->varunit[df->ri]`. | [ ] |
| SJW2-16 | 108-114, 117-139, 515-516, 563 | NULL dereference / uninitialised | L/L | If the file has no record variable and no `t`/`time` variable, `ts->t` stays NULL, and `tsMultiCheck`/`tsMultiEvalXYZ` dereference `ts->t[0]`. For NetCDF with `nd > 3`, `ts->type` is left unset (`default: break`). Function: `tsRead()`. | Quit if `ts->t == NULL`; set a type or quit in the default branch. | [ ] |


#### sjwlib/keyfile.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SJW2-17 | 393 | printf format/arg mismatch | H/L | `quit("readkeyprm_sarray: %s: Not enough values in array value %s\n", key);` has two `%s` but only one argument, which is undefined behaviour (likely crash or garbage) in the error path. The message also wrongly says "fill remainder" in the comment above. Function: `readkeyprm_sarray()`. | Drop the second `%s` or pass a value. | [ ] |
| SJW2-18 | 373, 377, 386 | buffer overrun / leak | M/L | `fscanf(fp,"%s",val)` into `char val[1024]` has no width limit. `*p` is always malloc'd, which ignores and leaks any existing array (unlike the d/i variants), and is leaked on the error returns. Function: `readkeyprm_sarray()`. | Use `"%1023s"`; free `*p` on error. | [ ] |
| SJW2-19 | 219-223, 232-235, 242-244 (and 295-319, 367-389) | wrong return value | M/L | The functions are documented to return non-zero on success, but return `-1` on error. When `keyprm_errfn` is `warn` or a quiet function (datafile.c:1194, atUtilXML.c:1359), callers testing `if(!readkeyprm_darray(...))` treat failure as success and use an unset or partially filled array. Function: `readkeyprm_darray()`, `readkeyprm_iarray()`, `readkeyprm_sarray()`. | Return 0 on error. | [ ] |


#### sjwlib/convertFileFormats.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SJW2-20 | 425-439 | wrong mode (copy-paste) | H/L | On `_WIN32`, `pFlag->FromToMode = FROMTO_DOS2UNIX` is set, but then `ConvertUnixToDos()` is called. That function's switch only handles `FROMTO_UNIX2DOS`/`FROMTO_UNIX2MAC` (lines 291-404), so it hits `default:` and writes nothing, leaving the temp file empty. The only caller (datafile.c:1736) is commented out, so this is latent. Function: `Convert_File_Format()`. | Set `FROMTO_UNIX2DOS` on Windows (or call the matching converter). | [ ] |
| SJW2-21 | 420-463 | unchecked fopen / leak / ignored error | M/L | The `outputFile = fopen(...)` result is not checked (NULL then goes to fputc/fclose). `pFlag` is never freed. The conversion return value is ignored, so a "binary symbol" abort leaves a silently truncated temp file while the function returns TRUE. Function: `Convert_File_Format()`. | Check fopen, free pFlag, and propagate the conversion's return value. | [ ] |


#### sjwlib/xytoij.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SJW2-22 | 296-309, 312-324 | off-by-one / out-of-bounds read | H/M | The trim uses `ival > nce1` / `jval > nce2`, so `ival == nce1` (or `jval == nce2`) passes, giving `i = nce1`. The code then reads `gx[j][i+1]` and `gx[j+1][i]`, but the grid has only `nce1+1` x `nce2+1` points (`xytoij_init` called with size-1, dfcoords.c:1394-1395). For the last row, `gx[nce2+1]` is past the row-pointer array. This is reached from `cm_bilinear_2d_itoc` (dfcoords.c:1659) for points on the upper grid edge. Function: `fijtoxy()`. | Use `>=` in both checks (clamp to `nce-EPS`), or clamp `i` to `nce1-1` and `j` to `nce2-1`. | [ ] |
| SJW2-23 | 241-246, 261-264 | dead cache / wrong guard | H/L | The "same cell as last time" fast path tests `partition->lasti > 0`, but `lasti`/`lastj` are never assigned (only `lastcell` is set), so the cache never fires and every lookup does a full tree search. If they were set, `> 0` would also exclude column 0. Function: `xytoij()`. | Set `partition->lasti = *ival; partition->lastj = *jval;` and test `>= 0` (with `lastcell != NULL`). | [ ] |


#### sjwlib/geodetic.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SJW2-24 | 199-212, 248-257 | division by zero | M/L | `c = cosbeta1*cosbeta2*sinomega/sinphi;` and the `phi*phi/sinphi`, `/tanphi` terms divide by `sinphi`, which is 0 when the two points coincide, so the result is NaN. This is used via the `GEODESIC` macro (gridmetric.c:31) and `GANGLE` (gridangle.c:31), where degenerate or duplicated grid corners give NaN metrics. Function: `inv_sodanos()`, `inv_sodanos_angles()`. | Return 0 (or a 0 angle) when `sinphi` is near 0. | [ ] |


#### sjwlib/time.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SJW2-25 | 160-164 | off-by-one day | H/M | When rounding pushes the time to 24:00, the code computes `j = tojul(y,mo,d,0,0,0) + 1`. `tojul` returns `julday - 0.5`, so `j = julday + 0.5` and `(long)j == julday`. `caldat` then returns the same date, and `h` is reset to 0, so the result is midnight at the start of the same day, 24 h too early. This happens whenever a float time sits just below midnight (e.g. 86399.9999 s), which is common for whole-day times, and affects `TimeToDate()` strings. Function: `todat()`. | `caldat((long)floor(tojul(*y,*mo,*d,0,0,0) + 1 + 0.5), mo, d, y);` (i.e. `julday(...)+1`). | [ ] |
| SJW2-26 | 373-377 | silent data loss | M/L | If the date has `HH:MM` without seconds, `sscanf` returns 2. The fallback then zeroes `h` and `mi` too, silently dropping the time of day from the epoch or date. Function: `DateToJulian()`. | Accept a return of >= 2 (set `s=0` when only 2 are read). | [ ] |


#### sjwlib/contour.c


No issues found. Functions checked: lencmp, contour, conbox, contri, con_between, labelcontour.


#### sjwlib/dfeval.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SJW3-01 | 65-77, 105-119 | wrong index / buffer overrun | H/M | `dfReadRecords(df, v, before, after - before + 1); ... v->data[before] ... v->data[after]` — for netCDF datafiles `dfReadRecords` (datafile.c:348-430) allocates a buffer of only `nrecs` (1 or 2) records starting at `v->start_record = before`, and `df_read_data` stores record r at `data[r - v->start_record]` (datafile.c:1505). Indexing with the absolute record number `before`/`after` therefore reads past the buffer for any record > 0 (and returns the wrong value). Compare `dfGetDataValue` (datafile.c:912) and `df_interp_1d_inv_weight` (line 227) which correctly subtract `v->start_record`. Only ASCII files work because `dfReadRecords` returns early and all records are in memory. Affects every 0-D netCDF time series evaluated via `tsEval`/`tsEvalR`/`tsEvalEx`. Functions: `dfEval()`, `dfEvalEx()`. | Use `v->data[before - v->start_record]` / `v->data[after - v->start_record]` (or `dfGetDataValue(df, v, before, NULL)`). | [ ] |
| SJW3-02 | 264-266, 294-296 | buffer overrun (latent) | M/L | `int coffsets[MAXNUMDIMS * MAXNUMDIMS - 1][MAXNUMDIMS]; ... ncorners = 1 << nd` — with MAXNUMDIMS=10 the array has 99 rows, but `ncorners` = 2^nd exceeds 99 for nd >= 7, overrunning the stack array. Should be sized `1 << MAXNUMDIMS`. Function: `df_interp_linear()`. | Declare `coffsets[1 << MAXNUMDIMS][MAXNUMDIMS]` or quit if nd > 6. | [ ] |


#### sjwlib/netcdf.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SJW3-03 | 277 | format specifier mismatch | H/L | `quit("ncvarread: %s has %d bytes per value in file, code expects %d\n",n,size);` — three specifiers but two args, and `%s` consumes the int `n` (crash/garbage on this error path). Function: `ncvarread()`. | `quit("... %s has %d ... %d\n", name, n, size);` | [ ] |
| SJW3-04 | 117, 138 | buffer overrun | M/L | `char aval[MAXLINELEN]; ... nc_get_att_text(fid,id,attr,aval)` — attribute length is never checked (no `nc_inq_attlen`), so a text attribute >= 4000 chars overflows the stack buffer; also a value of exactly MAXLINELEN chars is not NUL-terminated for the `strcmp`. Function: `ncvarfind()`. | Query `nc_inq_attlen` first and skip/limit if `len >= MAXLINELEN`. | [ ] |


#### sjwlib/ptrack.c


(Not called anywhere in the Atlantis tree — all latent.)

| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SJW3-05 | 57, 87 | type mismatch / stack corruption | H/L | `int n; ... nc_inq_dimlen(fid,p_did,(size_t *)&n);` writes an 8-byte `size_t` into a 4-byte `int` on 64-bit platforms, clobbering adjacent stack. Function: `readparticles()`. | Use a `size_t` temporary and cast to int afterwards. | [ ] |
| SJW3-06 | 154-158 | wrong logic / buffer overrun / NULL deref | H/L | `if( t ) nc_get_vara_double(..., count(=1), b); for(n = 0; n < rec; n++){ t[n] = (double)b[n]; }` — only one time value is read, but the loop writes `rec` values into `t` (a pointer to a single double per the API doc), reading uninitialised `b[1..]`; the loop is also outside the `if(t)`, so `t==NULL` with `rec>0` dereferences NULL, and for `rec==0` `*t` is never set. Function: `readparticles()`. | Replace loop with `if (t) *t = b[0];` inside the `if(t)` block. | [ ] |
| SJW3-07 | 100 | format specifier mismatch | M/L | `quit("... (only %d records)\n",rec,name,nrec)` — `nrec` is `size_t` passed to `%d`. Function: `readparticles()`. | Cast `(int)nrec`. | [ ] |


#### sjwlib/pointsourcesink.c


No issues found. Functions checked: readPointSourceSink, locatePointSourceSink, parse_location.


#### sjwlib/diffusion.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SJW3-08 | 204-213 | static state not reset / dangling pointer | H/L | `free_diffusion1d()` frees `Cm1, C, Cp1, rhs, ud` but does not set them to NULL nor reset `max_n = -1`. A later `diffusion1d()` call in the same process (e.g. a second run when Atlantis is embedded/re-invoked, or a call after `atphysics.c:113` teardown) would use freed memory if `n <= max_n`, or double-free them if `n > max_n`. Function: `free_diffusion1d()`. | After freeing, set all five pointers to NULL and `max_n = -1`. | [ ] |


#### sjwlib/drandom.c


No issues found. Functions checked: drandom, ran1, ran2, ran3.


#### sjwlib/waterprops.c


No issues found. Functions checked: water_vapourp (table has 101 entries, indices 0..100 valid).


#### sjwlib/colourtable.c


No issues found. Functions checked: readCT, writeCT, valcolour.


#### sjwlib/gridmetric.c


No issues found. Functions checked: gridmetric, geog_gridmetric.


#### sjwlib/gridangle.c


No issues found. Functions checked: gridangle, geog_gridangle.


#### sjwlib/spline.c


No issues found. Functions checked: dspline, dsplint.


#### sjwlib/text_input.c


No issues found. Functions checked: nextline, iscommentline, isblankline, parseline, flushline.


#### sjwlib/ellipt_coord.c


No issues found. Functions checked: ellipt_coord.


#### sjwlib/decay.c


No issues found. Functions checked: decay_forward, decay_centered, decay_backward, decay_exact.


#### sjwlib/weight_fn.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SJW3-09 | 66-67 | wrong formula (missing fabs) | H/M | `if( fabs(x) <= scale ) return(1.0 - x/scale);` — the bound uses `fabs(x)` but the value uses signed `x`, so for negative x the weight is `1 + |x|/scale` (rising to 2 at x=-scale, then dropping to 0), not the documented 1->0 decrease. Atlantis calls this via `bioprofile()` (atphysics/atprofile.c:48) with sediment depths `sm->gridz[k]`/`sm->cellz[k]`, which are <= 0 (atphysics/atvertgeom.c:289-292: `gridz[k+1] = gridz[k] - dz[k]`). So with `biosedprofile = 'l'` the bioirrigation/bioturbation profile increases with depth instead of decreasing (atbioirrig.c:114,162,278; atbioturb.c:92,143,189). Compare `w_parabolic`, which is symmetric (x*x). Function: `w_linear()`. | `return(1.0 - fabs(x)/scale);` | [ ] |


#### sjwlib/polar_coord.c


No issues found. Functions checked: polar_coord.


#### sjwlib/cfft.c


No issues found. Functions checked: cfft.


#### sjwlib/rect_coord.c


No issues found. Functions checked: rect_coord.


#### sjwlib/gridmisc.c


No issues found. Functions checked: CentreToCorner.


#### sjwlib/vgrid.c


(Standalone program, not built into libsjwlib.)

| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SJW3-10 | 41, 44 | scanf format mismatch | H/L | `scanf("%e",&c); ... scanf("%e",&d1);` — `c`, `d1` are `double` but `%e` expects `float*`, so only 4 bytes are written and the values are garbage. Also `n = nl-1` gives division by zero at line 51 when nl==1. Function: `main()`. | Use `%le`; guard `nl > 1`. | [ ] |


#### sjwlib/win32.c


(Windows/MSVC builds only.)

| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SJW3-11 | 50-58 | wrong logic / unsigned underflow | H/M | `if (n > (int) min(strlen(s1),strlen(s2)) ) return(-1); for (i=0;i<=(n-1);i++)` — (a) returns "not equal" whenever `n` exceeds either string length, so `strncasecmp("abc","ABC",10)` returns -1 instead of 0 (the common idiom of passing a buffer size breaks on Windows only); (b) with `n==0`, `n-1` underflows to SIZE_MAX (size_t) and the loop reads past both strings. Function: `strncasecmp()`. | Loop `for(i=0;i<n;i++){ if(tolower(s1[i])!=tolower(s2[i])) return diff; if(!s1[i]) return 0; } return 0;` | [ ] |
| SJW3-12 | 32-42 | non-standard return | M/L | `strcasecmp` returns -1 for any mismatch (never >0), so it is unusable for ordering (e.g. qsort/bsearch comparators) on Windows; equality tests are fine. Function: `strcasecmp()`. | Return `tolower(s1[i]) - tolower(s2[i])`. | [ ] |


#### sjwlib/ppbfetch.c


No issues found. Functions checked: ppbfetch (hard-coded path `/home/walker/...` but function is unused).


#### sjwlib/warn.c


No issues found. Functions checked: warn, quiet.


#### sjwlib/quit.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SJW3-13 | 41 | wrong return value | H/M | `quit()` is the fatal-error handler used throughout Atlantis but ends with `exit(0);`, i.e. reports SUCCESS to the shell. Batch scripts, R/Python wrappers, and HPC job managers cannot detect that a run aborted on an error (e.g. missing parameter, NaN trap). Function: `quit()`. | `exit(EXIT_FAILURE);` (or `exit(1)`). | [ ] |


#### sjwlib/erfc.c


| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SJW3-14 | 9-41 | precision / symbol override | M/L | File comment says it exists only because MSVC lacks erf/erfc, but it is compiled on all platforms (sjwlib/Makefile.am c_sources). Because libsjwlib.a is linked before libm, any Atlantis reference to `erf`/`erfc` resolves to these approximations (A&S 7.1.26, abs. error ~1.5e-7; NR erfcc ~1.2e-7 rel.) rather than the full-precision libm versions, giving platform-dependent results. Functions: `erf()`, `erfc()`. | Wrap the file in `#ifdef _MSC_VER` (and the nrcdflib.h prototypes likewise). | [ ] |

#### sjwlib/include/*.h (sjwlib.h, sjwalloc.h, datafile.h, colourtable.h, gridlib.h, mapproj.h, nrcdflib.h, pointsourcesink.h, polyline.h, ptrack.h, timeseries.h, txt_param.h, win32.h, xytoij.h, sjwconfig.h, common.h, dos2unix.h, unix2dos.h)

| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| SJW3-15 | sjwlib.h:80-82 | macro multiple evaluation / wrong rounding | H/M | `#if !defined(round)` / `#define round(x) ((x-floor(x))>0.5 ? ceil(x) : floor(x))` — `round` in <math.h> is a function, not a macro, so this macro is always defined and silently replaces C99 `round()` in every file that includes math.h then sjwlib.h. (a) `x` is evaluated 3 times: in atassess/atsurvey.c:640, :1260 and atassess/atattribute.c:858 the argument is `(chrt + drandom(0.0, 1.0)) * ageClassSize + 0.5`, so `drandom` is called three times and the comparison and the floor/ceil use three different random numbers — the assigned age `cage` is not a rounding of any single draw, distorting age-composition samples and potentially exceeding the intended age range. (b) Even with a pure argument it rounds halves down (`round(2.5)=2`, `round(-2.5)=-3`) unlike C `round`. (c) `x` is not parenthesised. | Remove the macro (C99 `round` is available on all supported compilers incl. MSVC 2013+), or make it a static inline function. In callers, compute the drandom expression into a temporary first. | [ ] |
| SJW3-16 | sjwlib.h:71-72 | macro multiple evaluation | L/L | `#define max(x,y) ((x)>(y) ? (x) : (y))` / `min` evaluate arguments twice; any caller passing a side-effecting argument (e.g. `drandom()`) gets inconsistent results. Not verified that any caller does. | Use inline functions. | [ ] |

No issues found in the remaining headers: struct/prototype declarations checked against the definitions in the assigned .c files (gridlib.h, ptrack.h, pointsourcesink.h, colourtable.h, polyline.h, timeseries.h, datafile.h, sjwalloc.h prototypes match); common.h/dos2unix.h/unix2dos.h are third-party dos2unix headers not used by the build.


## B.3 Lower-confidence notes (speculative — check before acting)

**From 01_DEMO.md:**

Lower-confidence notes:
- Queue indices: `Vertebrate_Reproduction` (302-309) and `Find_Final_Recruit_Distribution` (1841-1855) apply `qid++` without the `num_in_larval_queue - 1` clamp that `Ecology_Find_Embryoes` (1135) and `Get_Recruits` (1365) use, so they could overrun Larvae/num_recruits at the end of a run. In the invertebrate path, Larvae are written at `next_larvae` (4538) but read at `next_recruit` (4658). num_recruits is written at `next_spawn` (qid from 4313 passed to 4421) and read at `next_recruit` (4998). This is only correct while the three counters stay in step.
- `Get_Settlers()` 2353-2357: for external_recruit, `MIGRATION.recruit[ngene][migIDmatch]` is overwritten with `recruits_coming` in every box and layer. For external recruits num_recruits is generally not set (Find_Final writes MIGRATION.recruit directly), so this may reset to 0 the value set at 1931 before atmovement.c consumes it. The same migIDmatch default-0 / -1 concern as DEMO-15 applies.
- `Update_Invert_Aging()` 5022/5089/5118: settlers (`num_recruits[...][next_recruit]`) are added to cohort 0 on every call where `did_something` is true (spawn, recruit and aging days, and every timestep of those days). num_recruits is never decremented for invertebrates, so recruits could be added more than once.
- `Get_Recruits()` 1362 uses `qid++` when SpawnRecruitOverlap, without the CounterNotDone condition that `Ecology_Find_Embryoes` (1133) uses. The comment says this is deliberate, but the two can then address different Larvae slots.
- `Ecology_Find_Embryoes()` 1145: the larvae sanity check only inspects ngene = 0 (ngene is still 0 before the loop).
- `Larval_Mortality()` returns 0 (wiping out recruitment) for any `intersp_depend_recruit` value other than 1 or 2.
- `Vertebrate_Reproduction()` 492 checks `active_den[cohort]` for a single leftover cohort index only.

**From 02_MOVE.md:**

Lower-confidence notes:
- `n < age_mat`, `roc[ij][age_mat]`, `totroc[age_mat]` (1617, 1801-1805, 1862-1869, 2061) and `PartialMinAge/MaxAge` vs `n` (2626) compare an age-class/cohort number with the cohort×genotype index `n` (layout is `n = numGeneTypes*cohort + gene`, see 2314). These are wrong when numGeneTypes > 1. The same applies to `drandom(start_n=age_mat, end_n=numCohortsXnumGenes)` at 2065.
- In the flagenviro_displace branch (2017-2037), fish moved into a neighbour `chkbox > ij` are overwritten when that box is processed later (newden is assigned, not added to). The neighbour can also be a BOUNDARY/LAND box.
- Tidal channel code (2119-2145): the `<` test at 2119, the `>` at 2122 and the `>=` at 2131 are inconsistent. The `area*current_botz` weighting (2123) vs `area*dz[k]` (2133) depends on the sign of botz (negative botz makes nbox_spread <= 0). `tempdistrib[k][stage]` is left over from the last Get_Vertical_Distribution call, not from chkbox. The `else` branch zeroes neighbouring shallow boxes' densities rather than moving them.
- `Store_Min_Max_Avg()` line 763 returns unless *both* flagFonly and flag_systcap are set (`||` of negations). Check whether `&&` was intended.
- `avgsn/avgrn/finalmig` (2720-2722) are weighted by cumulative `totmig` (which includes earlier DEN) and pooled across cohorts. This only affects the box-0 diagnostic storage.
- `Get_Vertical_Distribution()` non-mobile branch with cohort == -1 uses cohorts 0 and 1 (`currentden[species][juv_id/adult_id]`) as the juvenile/adult profiles.
- For land boxes in the recruit_hdistrib update (2310-2328), the non-BOUNDARY layer loop also runs (its LAND exclusion is commented out), so land-box adults may be counted twice.
- In the invert enviro checks (3397-3463), only `newden[..][0][ij]` is used afterwards, so scalars applied for layers k > 0 have no effect. Only bottom-layer conditions matter (partly acknowledged by the TODO).
- `Get_Ice_Presence(..., ij, 0, ...)` at 3161/3446 uses layer 0 instead of k.
- Division risks: `mum_SP` = 0 (398), `MinYearsAway` = 0 (1214), `x*(1-IOBox)+y` (168).
- `preyamt` is allocated with only 2 stages (atbiolsetup.c:2919) and indexed by `preystage` up to numStages-1. That is fine while K_num_max_stages <= 2.

**From 03_BXML.md:**

Lower-confidence notes:
- `Read_Primary_Producer_Values()` (1220-1253): when `ice_on`, `attributeGroupNode` is reassigned to "IcePrimaryProduction", so the macro-model, coral, sponge and HAB species parameters that follow are read from the ice sub-group. `HAB_box` is read from `parent`, and the "HAB_Location" node that was just fetched is ignored. This matches what the converter writes (atBioltoXML.c:2618-2650), so it works today, but the structure looks accidental (the converter does the same reassignment). If either side is "fixed" alone, both will break.
- `Read_Vertebrate_Food_Avail()` (2127): under `flag_olddiet` the guard tests `pSPEat[0][b][WC]` but writes `pSPEat[cohort]...`. Once stage 0 is set, stage 1 is never filled. It may be deliberate, given the old-diet remapping comment in atmovement.c:286. Also, in the non-old branch `pSPEat[cohort][b]` is overwritten on every `preyCohort` pass, so only the adult-prey value survives.
- `Read_Reproduction_Values()` (3565-3569): `AquacultDistribution` is written into the same `recruit_hdistrib[0]` array as `RecruitDistribution`, so for cultured age-structured groups it overwrites the recruit distribution. This may be intended.
- `Read_Invert_Growth_Rates_XML()` and `Read_Invert_Clearance_Rates_XML()` set `mum_T15_per_day` and `C_T15_per_day`, while the vertebrate readers set `mum_per_day` and `SP_C_per_day`. A vertebrate with `numCohorts == 1` would go through the invertebrate growth reader and leave `mum_per_day` at 0, which is used as a divisor in atmovement.c:398. This is an edge case.
- `Read_Ice_Habitat_XML()` (1529-1538): the `!bm->ice_on` initialisation branch would dereference unallocated `ICE_HABITATlike` (only allocated when `ice_on`, atlantismain.c:1927). It is unreachable because the only caller (1699) is guarded by `ice_on`.

**From 04_ECOL.md:**

Lower-confidence notes:
- `Water_Column_Box()` 604-617: the dependent-discard reality check uses `initialBiomass` (WC tracers) as `this_bio` for every non-EPIFAUNA, non-vertebrate group, including SED groups. For a SED group this is ~0, so `realism_scalar` goes to ~0 and dependent discards of sediment groups caused by WC fleets vanish. `Epibenthic_Box()` then re-zeroes DependDiscardsTot for non-EPIFAUNA groups (2238), so those discards are never applied anywhere. Worth checking whether SED groups should be handled in Epibenthic instead.
- `Epibenthic_Box()` 2371: dependent discards go to `DetritusProd[SED][DCdet_id]`, while the WC box uses `[WC]` (628). DependDiscardsTot is per m3 of water, so adding it to sediment carrion production may need a wcLayerThick/smLayerThick conversion.
- `Epibenthic_Box()` 3009-3012: DiagN gain/loss is added to `localWCTracers` for any `habitat_check` with no `habitat_check` test. When habitat_check==SED it goes to the WC tracer, and for WC it may double count what the nested Water_Column_Box call already stored.
- `Sediment_Box()` 1781-1782: the debug loss term leaves out lysis and dead, which are included in the WC version (1008-1009). It also adds `linkageSEDDetritusFlux` to `loss_id` instead of `gain_id` (1805), but that branch is unreachable because detritus is skipped at 1723 (same unreachable detritus branch at 1030-1037).
- `Ice_Box()` 3519-3528: broker linkage mortality is applied to `localSEDFlux` rather than `localICEFlux` (only under BROKER_LINK_ENABLED).
- `Epibenthic_Box()` 2478-2480: `DIN = NO + NH + small_num` means the `if (!DIN)` reset can never fire (harmless).

**From 05_BSET.md:**

Lower-confidence notes: `Ecology_Read_Larval_Matrix()` reads into a double array with `ncvarget`, which assumes the netCDF variable is stored as double. It also looks for the variable name `"%s_Connnectivity"` (three n's); check that this matches the input-file convention. Line 3290 tests depth against `maxtotdepth_id` while the commented-out debug line refers to `maxdepth_id`, so it is worth checking which one is intended. `boxLayerInfo->localEPITracers` and `localLANDTracers` are sized `numwcvar` rather than `numepivar`/`numlandvar`, which is safe only while those counts are ≤ ntracer. Line 853 prints the same `MIGRATION[sp].Box` value `numStages` times (cosmetic). Allocation/use dimensions for `totdenCheck`, `DIET_check`, `nSTOCK`, `sumSTOCK`, `spSTOCKprop`, `step1distrib`, `tempdistrib`, `recVERTpopratio`, `numMortChanges`, `LinearMortChange`, `SUPPdistrib`, `pSTOCK`, `KDENR`, `recSTOCK`, `invader_spread_scalars`, `totrecruit`, `starve_check`, `AGE_stock_struct_prop`, `cysts`, `Vchange`, `recruit_vdistrib`, `boxPopRatio`, `tempPopRatio`, `HomeRangeTotal` and `rangenum` were checked against their uses elsewhere and are consistent once the reversed-dimension convention is applied.

**From 06_BMSG.md:**

Lower-confidence notes:
- atPhysChemIO.c 88-93: if `flagIsEstuary` and `flagpassivetracer` are both set, two PhysioChemArray entries named "SED" share `&SED_i`, so SED is printed and checked twice. The comment says the overload is intentional, but turning both on probably was not anticipated.
- atImposeRecruit.c 59-68 / 126-134: the column index `tsRecruitsid[sp]` is taken from `tsRecruits[0]`'s varnames and then used for every box's series. This silently assumes every MultRec box file has the same column order.
- atbiolmessage.c 1802/1825: `mort_scale = 365.0 / (365.0 - update_date)` divides by zero if `calcupdate_date` is 365 (Write_MortPerPred guards this case; Write_Mort does not).
- atbiolmessage.c 742-748, 793-799: the "biomass in inactive layer" warnings for biomass-pool groups multiply by `dz[k]`/`sm.dz[k]`, which are likely 0 for inactive layers, so these warnings may never fire.

**From 07_PROC.md:**

Lower-confidence notes:
- atprocess.c `Calculate_Catch()` 2042: divides by `propSediment`. All current callers pass 1.0 or smLayerThick, so this is not a live divide-by-zero.
- atGroupProcesses.c `Sediment_Bacterica_Process()` 1850-1857 passes `WC` as the globalHabitat argument to Gain_Element/Loose_Element for sediment bacteria, while Calculate_Element_Release uses `SED`. It looks copied from the pelagic version and only matters for isGlobal groups. `Labile_/Refractory_Detritus_Process()` also always pass `WC` as globalHabitat.
- `Pelagic_Bacteria_Process()` 1674 calls `Oxygen(2, ...)` (O2_IGBEM_id), which returns the constant `1/(KO2+1)`, so pelagic bacteria get no oxygen dependence. Possibly intended.
- `Encystment()` 1730: cysts hatch when `DIN < crit_nut`, i.e. under low nutrients. Check this is the intended direction (Hense & Beckmann).
- `Get_Extra_Feed()` 627-634: the supplementation spatial scalar `SUPPdistrib` is applied only to adults, while juveniles get scalar 1.0.
- `Update_Detritus()` 118 vs 138: the local WC DL production includes lysis but DetritusProdGlobal does not (invertebrate lysis is usually 0).
- `Sediment_Epi_Other_Process()` 2070: simple_ben_lim uses `biomass/max_id` with no area_hab and no small_num, unlike Epibenthic_Invert_Process 1923.
- `Create_Update_XRS()` 2517 uses `evolTraitOriginalValues` for RSmid when evolution is active. Check that the evolved, not the original, trait value is meant.
- `Print_Eat_Diagnostics(bm, logfp, ...)` (1618, 2028, 2151) uses the global `logfp` rather than `llogfp`.
- `UpdateTrackedMort()` 2279: `step1 / sizeScalar` can divide by zero (only under flag_replicated_old with !UseHardFeedingWindow, which deliberately replicates the old code).

**From 08_CONT.md:**

Lower-confidence notes: `Do_Vertebrate_Living()` (atvertprocesses.c:1059-1066) does not set `grow[cohort][SN_id]` in the shrinkfat branch, relying on `Reset_Arrays()` which only resets `grow` for the WC habitat (atecology.c:3984-3988), so the EPIFAUNA call may carry a stale SN growth value from the WC pass. `starve_check[guildcase][cohort][box]` is reset after the first layer processed on a new month (atvertprocesses.c:1112), so later layers in the same box see 0. `Free_Contaminants()` never frees the individual `bm->contaminantStructure[cIndex]` structs (leak, if they are separately allocated). `Calculate_Species_Contaminant_Decay()` divides by `this_half_life`, which is 0 if the species-based option is on but the parameter was left unset (instant full decay). In `Ecology_Get_Size()` with `flag_lengthSN`, `VERTinfo[species][cohort]` is read even for invertebrate groups (result later discarded since `li = 1.0`).

**From 09_BIOL.md:**

Lower-confidence notes:
- atmacrophytes.c l.148-152: root growth is `this_biom(roots) * growth[main] * ...`. `growth[main]` is already a flux (biomass × rate), so root growth has units of biomass² × rate. Check this against the Savina-Rolland formulation.
- atmacrophytes.c l.174-215: P and C uptake (`Calculate_Phosphorus_Uptake/Carbon_Uptake`) is only computed for the epiphyte cohort. For leaves and roots `uptakeP = uptakeC = 0` is passed to `PP_uptake()`, although growth is P-limited (`Nutrient_Lim(...P...)`), so N:P:C ratios of leaves/roots drift. In the caller (atGroupProcesses.c:738-739) P and C come from *sediment* tracers even for epiphytes, which draw uptake from the WC.
- atmacrophytes.c l.178-179: `/ DINs` has no `small_num` guard (the same pattern as atprocess.c:1517), so it gives NaN if sediment DIN is exactly 0.
- additionalTracer.c l.1224/1226: `Vertebrates_Transfer_To_Pred()` uses the global `GRAZEinfo`, while every other branch uses the `spGRAZEinfo` parameter. They are currently the same pointer (atecology.c:819), so this is harmless but fragile.
- additionalTracer.c l.1151/1229: `Gain_Element_Predation(..., prey, 0, amount)` always takes the prey ratio from cohort 0, even while looping over `kij`.
- atannualbiology.c l.499-520: the spawn queue is shifted in place inside the `nspawn` loop, so later iterations (`qid = nspawn`) read already-shifted entries. With numSpawns > 1 this may double-insert countback entries.
- atannualbiology.c `Get_Init_Embryos()` is called per box/layer/k. For `rand_recruit`/stochastic recruitment each call draws a new random number, so `Larvae[stock][gene][k]` ends up holding the last box's draw. The extra calls also change the RNG sequence.
- atbiology.c `UpdateMdt()`: for `isOncePerDt` groups after day 1, Mdt is set to `dtsz` rather than `bm->dt`. On `it_count==1`, `dtsz == tsz`, so this is likely harmless.

**From 10_NUTR.md:**

Lower-confidence notes:
- atNutrient.c `Carrion_ROC()` (291-313): the WC case subtracts no `DetritusLost[WC][DCdet_id]`, and the SED case neither adds `DetritusProd[SED][DCdet_id]` nor subtracts Lost. Refractory/labile handle both in all cases. This may be intentional, with EPIFAUNA (327-342) accumulating them, but it is worth checking.
- atNutrient.c `Micronutrient_ROC()` (742): the WC Fe gain is derived from `localWCFlux[NH3_i]`, which already includes atmospheric NH (and its multiple counting, NUTR-01), so Fe also picks up X_FeN × atmospheric NH.
- atbiophysics.c `Box_Light_Process()` (319-320, 387-402): `midwater_dz` is half the *top* layer thickness, not half the water depth. The "surface light" branches use `lighttop`, which at that point equals bottom-of-column light, not `lightsurf`. Both disagree with the comments.
- atbiophysics.c `Get_Enviro_Scalar()` (859-862): if trailing Xchange entries are zero-filled up to Xchange_max_num, `timeNow >= 0` makes `now_change` the last (empty) entry.
- atcoral.c `Do_Bleaching()` (420-424): once all shocks have left the DHW queue, `time_since_last_shock` returns to 0, so slow recovery (`> recovery_periodB`) only happens if recovery_periodB < K_max_num_DHW.
- atdiversity.c `Find_Evolution_Stats()` step 2 (185): the variance is taken around `evol_theoretmean_id` rather than `evol_mean_id`. This may be intentional.

**From 11_EINIT.md:**

Lower-confidence notes:
- atIceProcesses.c:36,51 `Box_Ice_Flux()`: `k_brine_diff *= 86400.0/bm->dt` is commented "per s to per day", but 86400/dt converts neither per s to per day (×86400) nor per s to per timestep (×dt). Also for `igr <= 0` the `else` branch at line 50 gives a negative diffusivity (only used when freezing, so possibly harmless). At line 62 the `wc_layer >= nz` check comes after the `pBox->tr[wc_layer]` pointer is formed (harmless since nothing is dereferenced first).
- atIceProcesses.c:205 `Box_Ice_Light_Level()`: `phytoBiomass` is seeded with `k_w_shallow`, a water attenuation coefficient, then multiplied by `ka_star`. This mixes an attenuation coefficient into a biomass sum.
- atmigration.c:243/324 use 364 where 365 is used everywhere else (lines 90, 101, 325, 386, 409) for unwinding the calendar.
- atmigration.c:195 vs 206: `ReprodAllowed` is set for juvenile cohorts and then immediately overwritten for all cohorts by line 206, so the stage-specific intent of 195 is lost.
- atmigration.c:759-761 `Init_Migration_Age_Check()`: with numGeneTypes>1, clamping `nextcid` to `numCohortsXnumGenes-1` moves the last cohort's genotype-0 individuals into genotype ngenes-1.
- atq10.c:469 `Apply_Q10_Corrections()`: `contamGrowthScalar = C_growth_corr[0]` is applied to every cohort. This may be intended, but looks like a cohort-0-only value.
- atcalibtools.c:403: `Spawn_Now[n][indx]` and `StartDay[n][next_recruit]` use `n == numGeneTypes` (cohort 1, gene 0) to describe larvae. This is in range but probably not the intended row.

**From 12_EMISC.md:**

Lower-confidence notes: `Util_Copy_Change_Values` (atlantisUtil/atUtil.c:841) compares stored `start_id` values, which already include `+bm->dayt`, against a raw `start`, so insertion order may be wrong after day 0 (not my file). `Ecology_Get_Linear_Mortality(bm, guild, cohort, cohort)` callers (atGroupProcesses.c:679, 1381; atIceProcesses.c:633) pass `cohort` as `isAdult` and index `cohortSpeciesParams[isAdult]`; this may go out of range for multi-cohort inverts (not my file). `current_SALT` is declared twice in the same global declaration (atbiology.h / atbiologyPrivate.h); this is harmless. `Calc_Scalar_Values` "interpolation" is an unweighted mean of two records, which may be by design.

**From 13_QUOT.md:**

Lower-confidence notes: In `Monthly_Effort_Schedule()` `this_profit` keeps only the last month's (month 11) value and applies it to every month (263/315). Lines 253 and 257 rescale using `oldtotpi` rather than `totppi`, which may be intended. In `Holland_Effort_Final_Allocation()` the effort-init block (960-981) sits inside the regid loop (redundant, harmless). `tot_marg_rent[nf][ns]` ends holding the last box's value (1241). The consolidation share uses all-fleet total catch in normal mode but only this fleet's catch in history mode (1426-1472), which is inconsistent. In `Get_Fish_Prices()` `deemprice_id` keeps only the last market's DV. Tax applies whenever tax_id/FixedMinTax>0 regardless of quota or TaxStart (238-239), which may be intended. `Quota_trade()` step 1/2 has no buyer==seller guard and its outer while loops rely on recon_buffer to terminate. In `Trade()` the seller's `permilease_id` is not rescaled after `leased_id` is reduced (1701, 1859).

**From 14_ECON.md:**

Lower-confidence notes:
- ateconomicsetup.c:225: when `!UseEconQuotaDefine`, `TACamt[sp][nf][now_id]` is added to owned quota even if it holds the `no_quota` sentinel (1e12), which would inflate owned quota.
- ateconresponse.c:1035: `CurrentEffort_id += Effort_hdistrib[b][nf][today_effort]` adds the whole fleet's effort to every subfleet, while catch is split by the SpatialBlackBook share. Subfleet CPUE (1041) may therefore be understated.
- ateconresponse.c:636-674: the "new subfleet opening up" branch sets `this_subfleet = 0` but never uses it, so no distribution is copied, as the comment says it should be. The `MofY > 10` block also runs for non-economic fisheries.
- ateconresponse.c:296-301: `log(resale/...)` and `log(crew_size)` give -inf if a subfleet has 0 resale value or crew.
- atEconomicAnnual.c:401-402: the insertion shuffle writes index `ntarg`, which can reach K_num_tot_sp (out of bounds) only if every group is fished and catchable. Unlikely.
- ateconomic.c:96: the RegCatch reset loops to `K_num_active_reg`, while Allocate_Catch can write any region below `K_num_reg`.

**From 15_ECIO.md:**

Lower-confidence notes:
- ateconindicator.c:529 (Holland branch) computes management cost as `gvp * (1 - mgmt_cost_id)`. If `mgmt_cost` is the cost-recovery fraction (as the comment "cost recovery costs only" suggests), the cost would be `gvp * mgmt_cost`. Check the parameter's definition.
- ateconindicator.c:439 divides spatial gvp by `nsubfleets` even though `ECONexprice`/`SpatialBlackBook` are already per subfleet. Possible double scaling, by design or not.
- atFishMIP_OSP_Bridge.c:67/141: `SimLength` is in days, but `recorded`/`effortFlat` are indexed by year. That is oversized (no bug), but the naming suggests a years-vs-days mix-up.
- atFishMIP_OSP_Bridge.c:443: when there is no prior TAC, MSY is split evenly across all fisheries, including ones that cannot catch the species.
- atFishMIP_OSP_PluginAdapter.cpp:140-142: if `sim.xsize == 0`, `t` becomes -1 and is cast to size_t.
- ateconomic/Makefile.am:21 lists `ateconomic.h ateconomicsetup.h` as sources, but the headers in include/ are named atEconomic.h and atEconomicSetup.h (outside the assigned files; noted only).

**From 16_HARV.md:**

Lower-confidence notes:
- atHarvest.c `Basic_Habitat_Overlap()` / `Habitat_Overlap()` loop `stage < numStages` into `bm->HABITATlike[sp][stage][...]`, which is allocated with only 2 stages (atlantismain.c:1926). Any impacted group with `numStages > 2` would overrun. The same root cause exists in the biology reader (atBiologyXMLParamIO.c:1486), so it should be fixed at allocation (`K_num_max_stages`).
- atHarvest.c:395-396 debug gate uses `which_check == not_tracking_flux`, while the two neighbouring gates use `>=`. This only affects debug output.
- atHarvest.c `Harvest_Skip_biology()`: the water-column `AGE_STRUCTURED_BIOMASS` branch reads `tr[k][totNTracers]` even for epifaunal groups, which are also handled in the epibenthic loop. `bm->Catch` is only reset for `isImpacted` groups but is accumulated for all groups. This is a dev/test path.
- atHarvestCatch.c `Get_addlsm_Scale()` returns a selectivity difference, which `Get_Selectivity()` then adds to a length-at-50% (`sel_lsm + addlsm`). This mixes units, but it may be intended as a rescaling heuristic.
- atHarvestCatch.c `Harvest_How_Much_Fishery_Access()` calls `Get_Catchability()`, which uses `bm->current_box` / `current_layer` for the stock id. These may be stale when it is called from output code (atagetracerIO.c).
- atHarvest.c `Harvest_Update_Habitat_Overlap()` stores into `p_fishi[sp][stage][nf]`, which has no box dimension, so it depends on being called immediately before each box's biology (it currently is: atlantismain.c:511).

**From 17_HRV2.md:**

Lower-confidence notes:
- atHarvestDiscards.c `Do_Dependent_Discards()` (338-341): in the non-impose branch, every genotype `bcohort` of species i gets the full `FCcocatch*base_catch_sp*age_scale` amount. With numGeneTypes>1 the dependent discards are multiplied by numGeneTypes, unless DependDiscardsTot is later divided by numGeneTypes (not checked; the consumer is outside the assigned files).
- atHarvestSetup.c `Calculate_Age_Vulnerability()` (634): the size used for selectivity comes from box 0 layer 0 initial tracers. If box 0 is a boundary or empty box, Wgt=0 and li is meaningless. Also, if no cohort reaches sel>0.5, Age50pcntV keeps whatever was read in (it is not set to numCohorts-1).
- atHarvestTS.c (202, 210, 247): `DiscardTS_agedistribOrig[..][ij + ncum]`. `ncum` is never reset after a non-depleted class, so the shift is the total number of depleted classes above, not the contiguous run. This stays in bounds; whether it is intended is unclear.
- atImplementationAnnual.c 560-565: with `flagchangeSEL == change_dynamic`, `q *= effort_scale` compounds every year. Given HRV2-20, effort_scale may be negative, which would make q negative.
- atImplementationAnnual.c 513-516, 532: season wrap arithmetic uses 364 rather than 365. It is used consistently, so it is left as is.

**From 18_TIER.md:**

Lower-confidence notes:
- 20:35:48 rule (983, 2314): `ftarg = F40*(dep/0.2 - 1)` between 0.2 and 0.35 gives 0.75·F40 at 0.35, then jumps to F48. It looks copied from a 20:40 rule. Check the intended breakpoint and scaling.
- `get_T5_F20_40_48()` hard-codes 0.48/0.40 rather than BrefA/BrefB (as used in getF20_40_48), and never sets F20 (it stays 0).
- `SurplusProduction()` / `TierFive_SP()` use `HistYrMin_id` directly as an array index into expectB/T5_removal/CatchData. This is only safe if HistYrMin is 0-based (not a calendar year).
- Several places index `year-4`/`year-3` without clamping at 0 (`TierThree_CC` 1092, `TierFour` 1981). The weighted discard average (744-758) excludes year 0 (`year-1 > 0` should probably be `>= 0`).
- `Check_Indicator_Links()`: `buffer = GradientBuffer * CPUEtrend` is relative to the trend itself, so with GradientBuffer<1 any non-zero trend is classified ±1. A buffer computed from a negative trend also makes `diff < buffer` impossible. `ref_sp`/`ref_spB` can stay -1 and then be used as an index in `CalculateNonIndicatorRBC()` (3806).
- `setRBC()` never sets `RBCest_id` (the RBC read is TODO, d0=0), so DoTierAssessment uses a stale RBCest for dynamic tier 4. The convergence threshold of 100.1 is inconsistent with the comment's 0.1.
- `getSlope()`, `DoCPUE()`, `regressionSlope()`, `TierThree_trueF()` (fmax) and `TierOneMimic()` (fmax at 947) have unguarded divisions that give NaN/inf when there is no data.
- `TierThree_CCsel()` and `EquilF_Funk()` read `AgeComp[...][a]` for a < Tier3_maxage, but AgeComp's age dimension is the user-set `OverallMaxAge`. Nothing checks that OverallMaxAge >= Tier3_maxage.

**From 19_MNG.md:**

Lower-confidence notes:
- atManage.c:2687 `tot_hold += hold_capacity` is not weighted by nboat while `prop_hold[ns]` is (2686), so prop_hold does not sum to 1; prop_hold appears unused at present.
- atManage.c:2987 `weighted_depth += dz[k] * catch_left` uses layer thickness rather than layer depth.
- atManage.c:245-258 (catch-only branch): `bm->Effort` is never reset there, so `Effort[ij] *= F_rescale; Effort[new] += F_displaced` compounds every day.
- atManage.c:976 vs 943-946: when `manage_reg`, overages are set over K_num_active_reg regions but the closure count uses numStocks.
- atManage.c:2464-2466 quarter boundaries `nd <= 91` / `nd > 90` are inconsistent (harmless).
- atSS3assess.c:127: RBC_by_year[year] is checked even when the assessment already failed (value may be stale from a prior run), only matters for the log message.

**From 20_MNGA.md:**

Lower-confidence notes:
- atManageSetup.c:329: in the fisheries-off branch, `Util_Alloc_Init_2D_Int(1, K_num_basket, K_num_tot_sp)` gives `[K_num_basket][1]`, unlike the normal `[K_num_tot_sp][K_num_basket]` (atManageParamIO.c:896). K_num_basket is not set yet on this path (Init_Manage_Flags is not called). Harmless only if sp_basket is never touched when fisheries are off.
- atManageSetup.c:1062-1065: for every fleet with `inQuota` set, BiTACamt is overwritten with `no_quota` right after being computed. This looks inverted (`!inQuota`?), but `Make_Mgmt_Decisions()` recomputes BiTACamt annually, so the impact is limited to year 0.
- atManageSetup.c:979: the loop variable `nreg` actually indexes the now/old slot (last dim, size 2). It works only because `totalreg_id == 2`. The region-1 term is used even when Kreg==1 (zero, so harmless).
- atManageAnnual.c:1807: `Get_Fishery_Group_Change_Scale(bm, nf, sp, mFC_num_changes_id, mFC_num_changes_id, ...)` uses the number of changes as the on/off flag. This works only when the count is non-zero, and harvest uses a different argument (atHarvestCatch.c:232, which itself passes `bm->flagchangeF` as a param index, which looks wrong too).
- atManageAnnual.c:1893: `Z_Est = 1 - end/start` is a proportion, not an instantaneous rate, but it is compared with and subtracted from instantaneous `Fcurr` (should be `-log(end/start)`).
- atManageAnnual.c:516 (and atManageSetup.c:485): the `"\n"` is printed outside the `if (flag_sp)` block, so blank log lines appear for unflagged species (cosmetic).
- atManageSetup.c:519-549: byproduct/bycatch tiers do not set `BrefE_id`, so tier14 has BrefE=0 for those species.

**From 21_MNGIO.md:**

Lower-confidence notes:
- atManageMPATS.c:263 `this_t = (nyr*365.0 + ij)*86400.0` ignores `bm->tstart`, so the MPA time series would be offset if tstart≠0. With `flagSimpleStartStopMPAs`, `startyr = bm->MPAstartyr` is not checked for <0 (line 107). The rolling loader (`Load_MPAlist_From_File`) also overwrites fixed-MPA fisheries' lists and ignores the start/stop window. All of this may be by design.
- atManageParamIO.c `readPopulationChangeXML()` calls `Util_XML_Read_Array_Double` with size `counter`, which may be 0. Other readers guard with `if (numChanges > 0)`.
- atManageParamIO.c:1195 `Read_Manage_Paramaters` does not check `inputDoc` for NULL (depends on whether xmlReadFileDestFolder calls quit itself).
- atRlinkRBC.c:260-261: `getline` returns ssize_t but is stored in size_t. The `line_size != 4` header test is fragile, and `line_buf` from getline is leaked (same at atRlinkRAssess.c:266).
- atRlinkRAssess.c:212 `swept_area = ... / (area * botz)`: botz is negative in Atlantis, so survey numbers would be negative unless the sign is handled elsewhere.
- atRlinkRedus.c:366 `REDUS_management` averages calcTrackedMort over `numCohorts` only, not numCohortsXnumGenes. It may be intentional.
- atRlink.c duplicates most of atRlinkRedus.c. It is not compiled now, but adding it to the build would cause duplicate-symbol link errors.

**From 22_SS3IO.md:**

Lower-confidence notes:
- Year convention: `WriteSSDat`/`WriteSSCtl` write `HistYrMin` as `#_StartYr` and the caller's `year` (a relative index, cf. `RBC_by_year[year]` in atSS3assess.c) as `#_EndYr`, and loop `t = HistYrMin .. maxyr-1` indexing the year dimension (size nYears) with `t`. This only works if `HistYrMin` is set to 0 in the XML; if it's an AD year (as the comment at line 635 says SS needs) no data rows are written at all. Hard-coded years (`2000 # F ballpark year` at 1980, `HistYrMin-50` at 1952) assume AD years. Worth agreeing one convention.
- `Read_SS3_Report_File()` stops (`break`) at the LAA block; if that section comes before `FORECAST:_With_F_to_match_adjusted_catch` in the SS3.30 Report.sso (and in 3.30 the forecast detail may be in Forecast-report.sso), RBC stays 0. `EstB0` is no longer computed here (commented out) but `SS330Assessment()` then recomputes `EstDepletion = EstBcurr / EstB0`, overwriting the depletion just read, using a stale EstB0.
- `WriteSSFor()` uses `CatchData[..][maxyr]` as "last year's catch", but `WriteSSDat()` only writes catches for `t < maxyr`; check that year `maxyr` is populated when the assessment runs.
- Age-comp terminator writes `2*MaxAge+10` zeros and length terminator `2*Nlen+5` regardless of `Nsexes`; for single-sex models SS row width is smaller, so check against SS 3.30 reading rules.
- Outside my file: `Run_SS3()` (atSS3assess.c:201) calls `free(ss3_abs)` where `ss3_abs` points at the stack buffer `buf` passed to `realpath()` — invalid free.

**From 23_SS3.md:**

Lower-confidence notes: In `GenData()` (line 85), `bm->CloseKinEst->UseCloseKin` reads element 0. That is fine if it is meant as a global flag, but it is inconsistent with the `[groupIndex]` usage. `WriteSSFiles()` hard-codes `2007 - 1915` as the year count. The comment marks it as sample/test code, so it is not logged. In `FindWhichData()`, `CPUEyears[f][r]` and `sumcatch[f]` rely on `SP_FISHERYprms[..][assess_nf_id] < Nfleets` without a check. `GetCatch_n_DiscardData` adds noise to the DiscType==2 fraction using `Util_xnorm(0, DiscCV*expdisc)`, which can give negative discard fractions.

**From 24_PHY1.md:**

Lower-confidence notes:
- aticeIO.c `Get_Ice()` (ts_file): inside the class loop, `ts_ice_dz_data[b]` is overwritten for every class, so the ice depth comes from the last class's depth series only.
- aticeIO.c `readBMIceInfo()` 346-352: for particulate ice-only tracers, `inice` (set to 1 "by definition") is overwritten from the file attribute. Combined with PHY1-04, a restart file could switch ice tracers off.
- aticeIO.c `Enter_IceBM()` 1267-1270: if `tot_depth > maxicedz*icenz`, `z` is set to `icenz-1` and layer `icenz-2` is then overwritten. This looks unintended, but the intent is unclear.
- aticeIO.c 903: the parameter key is spelled `"nInceFiles"`, so user files must use that typo. The same goes for `"temp_ampltiude"` at atphysics.c:376.
- aticeIO.c `open_iceprop()` 1173: buffer size is computed from `icenz`, but the buffers hold `K_num_ice_classes` values per box.
- attempsalt.c 1032/1186: 2-D (non-z) water-column forcing is written to `newwc[b][0]`, which is the bottom layer in Atlantis. If surface forcing was intended, it should be `nz-1`.

**From 25_PHY2.md:**

Lower-confidence notes:
- atgeomIO.c `parseProjectionString()` (791-807): `char str[100][10]` is cast to `char **` and parseline stores up to 20 pointers in it. That works only because 20*8 bytes fits in the 1000-byte block. It should be `char *str[20]`. `parseline` can also return `n+1` when more than 20 tokens are present.
- atgeomIO.c `freeMSEBoxModelGeom()` does not free `uniqueAdjBoxes`, `distID` or `is_boundary`. This leak happens only at shutdown.
- attracerIO.c `readBMTracerData()` matches species to tracers with `strstr(tracer, FunctGroupArray[sp].name)`, and the first substring hit wins. If one group name is a substring of another (e.g. "Cod" vs "RedCod"), the wrong `init_scalar` could be applied.
- attracerIO.c `readBMTracerInfo()`: `int d[3]` is filled by `ncvarinq`. A "tracer" variable with more than 3 dimensions would overflow it.
- atsourcesink.c `sourceSink()`: a water sink (wvol < 0) with |wvol| >= cell volume is not checked, so the volume can go negative (fw_area_input does check this). `read_bm_ts()` line 655 `ts = NULL;` assigns the local parameter only and has no effect.
- atsourcesink.c `ReadStress()` has no callers. If it were used, its `else` branch would overwrite stress values supplied in the initial conditions (`supplied_stress == 1`) with the mud relationship.
- athydromod.c `get_hydro()` (165-166) copies `nbuf` entries of `value` into `tbuf`, but only `count[0]` were read. Entries past `bufend` are never used, so this is harmless.

**From 26_PHY3.md:**

Lower-confidence notes:
- `quit(... "%d" ..., sizeof(doubleINPUT))` passes a size_t to `%d` (atsummaryIO.c:310,408; atepiIO.c:386; atLandIO.c:299; atdiagIO.c:283; atfishstatIO.c:268; atdietIO.c:227). This only matters on the error path (L).
- atdeposition.c:175-180 and atsuspension.c:167-172: the `maintain_sedlayer` clamp changes `sm->dz[tk]` without updating `sm->volume[tk]`, so dz and volume disagree, and material above maxdz is dropped from the thickness.
- atepiIO.c:303-316: the scaling match uses `strstr(name, groupName)`, so a short group code that is a substring of another (e.g. "BC" in "BCX_N") can pick the wrong init_scalar.
- atreadbm.c:60-67: `memset(bm,0,...)` clears `bm->inputFolder` before it is passed to `Util_ncopen`. The function is unused.
- atsummaryIO.c:250-252 writes dz/volume from the raw (unshifted) `bm->dz`/`bm->vol` and then overwrites them at 280-282. `nominal_dz` is written raw, so its layer ordering may differ from the shifted dz/volume.

**From 27_UTIL.md:**

Lower-confidence notes: `Util_Get_Current_Stock_Index()` (atUtil.c:904-907) still has a debug consistency check that calls `abort()`. In `Util_XML_Read_Species_Param()`, several cases (SP_SED_EP_FF, SP_BASAL, SP_POP_LIM, SP_IMPACTED, SP_OVERWINTER, SP_CORAL, SP_CULTURED, SP_RUGOSITY, SP_POLLUTE_IMPACTED) don't check `flag_id`, unlike the rest. Some of these are explicitly commented out, so it's probably deliberate, but turned-off groups still need values in the file. SP_MICROALGAE is identical to SP_PP, which could be a copy-paste if a narrower group set was intended. `Util_XML_Get_Node_Value_Int()` leaks the strings from `Get_Node_Property` in its error path (quits anyway). `Util_ncopen()` uses an unbounded `strcpy(fileName, name)` into BMSLEN (512).

**From 28_UTL2.md:**

Lower-confidence notes:
- atUtilFisheryXML.c:22/134: `numFisheryParams` = 79 but there are 78 initialisers, and `numGroupFisheryParams` = 44 but there are 42. The trailing entries are zero-filled (NULL tag, paramID 0, i.e. `sel_id` for the group enum). `Util_XML_Get_Fishery_Param_Index()` returns the first match, so it is harmless today, but any loop over the full array that dereferences `.tag`/`.regEx` would crash. A lookup of an unlisted enum value equal to 0 would return the empty entry instead of quitting.
- atUtilFisheryXML.c:513-520: every fishery i gets the same `values[0..n_i)` vector for a group (the read size is the max over fisheries), so per-fishery change schedules can't differ. This may be the intended file format.
- Regexes "tax" and "saleprice" (lines 194-195) are unanchored and match other parameter names (e.g. "maxsaleprice", "origtax"). They are only relevant to the text-to-XML converter.
- atUtilFisheryXML.c:269-331: error strings say "readFisheryGroupParamXML", and the `_Double` header comment says "integer data" (cosmetic).
- atUtilFisheryIO.c:60: `strstr(convertedXMLFileName, ".csv")` matches the first ".csv" anywhere in the path, not the extension. The overlong-string strcpy into `fisheryCode[20]` (Util_XML_Get_Value_String, atUtilXML.c:1310) is unbounded.
- `xmlCleanupParser()` at atUtilFisheryIO.c:134 is called mid-run, and libxml2 documents it as unsafe if the library is used afterwards. This is a common pattern in the codebase.

**From 29_MAIN.md:**

Lower-confidence notes:
- atlantismain.c:250. The general-output trigger uses `bm->t + dtscale * bm->dt / 2.0 > bm->tout`, where `dtscale = dt/86400` is in days, so the offset is dt²/172800 s. The fisheries output (line 300) and the increment loops (295, 329) use `dt/2`. The comment suggests this is a deliberate hack to keep output in daylight, but the units are mixed.
- atlantismain.c:581-583. The copy of newlandtr back to bm->landtr is commented out, while newlandtr is refreshed from landtr every step (343) and passed to boundaries(). Any land-tracer changes made in newlandtr are thrown away. This may be intentional if land biology writes bm->landtr directly.
- atlantismain.c:614. `bm->TofY % 5 == 0` makes checknetCDFFiles run on every timestep of such days when dt is less than one day. This costs time but does not give wrong results.
- set_up_vvdistrib(): for non-mobile groups, stages >= numStages are not zeroed (mobile groups are). This is only a problem if alloc4d does not zero memory and those slots are later read.

**From 30_CNV1.md:**

Lower-confidence notes:
- atBioltoXML.c `MigrateIOBoxXMLFunction()` (1499-1519): after truncating at "ad"/"juv", the migration index digit is only looked for at the end of the remaining species string. A name like `MigIOBox_CEPad2` (index after the age tag) would silently map to Migrate1. The prm files seen only use `MigIOBox_CEPad` / `MigIOBox_BML`, so I could not confirm the intended naming.
- Several regexes assume 2-3 uppercase-letter group codes: `^C_[A-Z]{2,3}$`, `^mum_[A-Z]{2,3}$`, `^KMIGa_[A-Z]{2,3}$`, `^trait_active_[A-Z]{2,3}$`, `^q10_([A-Z]{2,3})$`, `[A-Z]{2,3}_stock_struct$`. Groups with 4-character or digit-containing codes would be silently skipped (or quit as "not found" when required).
- atBioltoXML.c `InitSuppDietXML()` builds `zeroString` but never uses it (nodes get ""). Probably meant to be the default value.
- atBioltoXML.c `createPrimaryProduceXML()` (2646-2649) creates an empty `HAB_Location` subgroup but puts `HAB_box` under `groupingNode`. The reader (atBiologyXMLParamIO.c:1250-1251) also looks under the parent, so the two are consistent; only the empty subgroup is odd.
- atCreateXML.c `Check_Species_Param()` case `SP_MICROALGAE` (516-526) is identical to `SP_PP` (493-503). This may be intended to exclude macrophytes.
- atCreateXML.c `Create_Species_Spawn_ParamXML()` is called with `XML_TYPE_INTEGER` (atBioltoXML.c:3340) but writes array values and sets ArrayLength. The type metadata should probably be INTEGERARRAY.
- `commentString[200]` in `Create_Species_Cohort_ParamXML`/`Create_Species_Spawn_ParamXML` is built with strcpy/strcat from comment+units but never used. Current comments are short, so this is a latent overrun only.

**From 31_CNV2.md:**

Lower-confidence notes:
- These are outside my files but surfaced while checking readers. atManageParamIO.c:662 reads `CPUEshotavgdis` from key `"CPUEshotfdis"`, which is a copy-paste and gives the wrong value. In atUtilXML.c `Util_XML_Parse_Create_Node()` (1360) the duplicate-key test is `skipToKeyEndNoRewind(...) < 0`, while `Parse_File` uses `> 0` (atCreateXML.c:216). Also at atUtilXML.c:1373, `strcpy(valuestr, tempBuf)` writes the "value unit" string back into the middle of `buf`, which can overrun the line buffer.
- The atAssesstoXML.c Parse_File regexes `"li_a"`, `"samplesize"`, `"R_max"`, `"q_lsm"` and `"selcurve"` are unanchored. They could match unrelated keys that contain those substrings. I found no collisions in the sample prm files.
- `Group_Catch_Selectivity_XMLFunction()` (atHarvesttoXML.c:203-217) copies the key into `char tempStr[50]` without a bound. Also, `strstr(strPtr,"ad")` is unchecked; it is safe only because the regex `^sel_.*[dv]$` guarantees one of "juv"/"ad".

**From 32_LINK.md:**

Lower-confidence notes:
- atlink/Makefile.am lists `atlinkconversion.o` in `libatlink_a_SOURCES` instead of `atlinkconversion.c`. Not one of my assigned files, but it means atlinkconversion.c is probably not built from source.
- atbrokerlink/network.c uses the ZeroMQ 2.x API (`zmq_recv(sock, zmq_msg_t*, flags)`, `zmq_init`, `zmq_term`). Against ZeroMQ >= 3 these signatures differ (`zmq_recv` takes a buffer and length), so it will not compile or will misbehave. `zmq_msg_recv` / `zmq_msg_send` / `zmq_ctx_new` are the modern equivalents.
- atCLAMComm.c Start_CLAM (WIN32 only): `WaitForSingleObject(..., processWaitTime)` takes a DWORD but is passed a double, and the return is not checked. If the process times out, the code still looks for the finished file and does not terminate the process on the success path.
- atCLAMManage.c Get_Variance loops over MAXNUMOUTPUTSTATES while Get_Expected_Value loops over numStates. This is harmless because the unused OutputStates entries are zero-initialised, but it is inconsistent.
- atCLAMManage.c CLAM_Check_Indicators: SD is only set when the first or a larger change is found, and is logged for PERCENT indicators where it may be uninitialised (log only).
- Link_SetMortality uses `habitatCoeffs[EPIFAUNA] > 0` to choose the EPI branch, whereas Link_SetBiomass uses `habitatType == EPIFAUNA`. A group with an EPI coefficient that also lives in WC/SED will only get EPI mortality. This may be intentional.

**From 33_SJW1.md:**

Lower-confidence notes:
- memory.c: several pointer-block `sizeof`s name the wrong pointer level (e.g. `sizeof(long double *)` for the `long double ****` cube block at lines 258/362/370, and the equivalents in `d_alloc4d` 788, `d_alloc5d` 891/899, `f_alloc4d` 1317, `i_alloc4d` 1777, `s_alloc4d` 2050, `c_alloc4d` 2314, `d_alloc6*` 482/490/498/1011/1019/1027). All data-pointer types are the same size on supported platforms, so these are not functional bugs. The zero-initialisation, strides (`p + i*n1`, `pp + i*n2`, ...) and free order were checked for every typed variant and are correct. `checkMemory` is honoured inconsistently (`d_free1longd`, `*_free6*`, `s_free1d/3d/4d`, `c_free1d/3d/4d` always quit on NULL), but `checkMemory = 1` by default, so the behaviour is the same.
- datafile.c `dfReadRecords`: if the record variable itself were ever re-read with a different `nrecs` (as a coordinate in `v->csystem`), `df_free_data_records` would free `rv->data` and leave `df->records` dangling. Currently `dfInferAndSetCoordSystem` strips `df->ri` from coordinate lists (line 404), so I could not find a live path.
- datafile.c `df_netcdf_read` lines 1036-1040: `missing_value` and `_FillValue` are read with `ATT_DOUBLE`, which returns 0.0 unless the attribute is stored as NC_DOUBLE. Float or short missing values are silently treated as 0.
- dfcoords.c `df_decode_coord_type` lines 906-927: matching uses `strncasecmp(ctype, "time", strlen(ctype))`, so any prefix (and an empty string) matches. For example `""` maps to `VT_TIME` and `"l"` maps to `VT_LATITUDE`.
- The coordinate-mapping code (`dfIndicesToCoords`, `cm_*_itoc`, the geo transforms) is not reached from the Atlantis model code (`tsEvalXY*` is not called outside sjwlib), which is why SJW1-14..17 are rated low severity.

**From 34_SJW2.md:**

Lower-confidence notes:
- Outside the assigned files but found while checking callers: atmanage/atManageSetup.c:1552 computes the "top" skipper CDF with `speciesParams[cpue_cdf_poor_p_id]` instead of `cpue_cdf_top_p_id`, which is read in atManageParamIO.c:682 but never used. This looks like a high-confidence copy-paste bug.
- atphysics/atgeomIO.c:789-807 passes `char str[100][10]` cast to `char **` into `parseline`/`mpInitialise`. It works only by accident (the storage is reused as a pointer array), and the strings live in the stack `buf[200]`, which is the reason SJW2-01 crashes.
- `keyprm_errfn` is declared without an initialiser (sjwlib/warn.c:47). `skipToKeyStart`/`skipToKeyEnd` call `(*keyprm_errfn)` without a NULL check, so a missing key before any `set_keyprm_errfn()` call would jump through NULL.
- `gc_distance()` (geodetic.c:284-285) can pass an `acos` argument slightly greater than 1 because of rounding, which gives NaN for nearly identical points.
- `negbinomial_cdf_values()` computes `1 - betai(k+1, r, p)`. Whether this matches the intended (p = success vs failure) parameterisation could not be confirmed.
- In `mpInitialise` (amg/mga), the nested utm call reallocs the same `proj=` string again. The sizes are equal so realloc normally returns the same pointer, but if it moved, the outer free loop would double-free.

**From 35_SJW3.md:**

Lower-confidence notes:
- (Outside my file list, for the datafile.c reviewer) datafile.c:317-320 `dfFindTime()`: `leftover = floor(r / lengthdf); *newt = r - leftover * df->records[ilow];` — multiplies by the first record time instead of the cycle length `lengthdf`, so forcing-file recycling (`rwind`) produces a wrong time unless `records[0]` happens to equal the period; also does not account for a non-zero start time. Likely should be `*newt = df->records[ilow] + fmod(r - df->records[ilow], lengthdf)`.
- dfeval.c `dfEvalCoords()`: when `dfFindRecord` returns an out-of-range record (r0==r1, rfrac=0) the result is fine, but `recvals[1]` is used with weight `rfrac` even when only one record was read; harmless because rfrac==0 there.
- text_input.c `parseline()` returns `max+1` when more fields exist than `max`; callers that use the return value as a count into `str[]` (e.g. atgeomIO.c:805 with max 20) must not index `str[max]`. Appears to be intended as an "overflow" signal.
- decay.c `decay_exact()` caches `exp(-k*dt)` in statics — not thread-safe, but correct for single-threaded use.
- pointsourcesink.c: `pssN.rewind` is read with `readkeyprm_i` and its return value ignored; if missing, `rewindid` is left uninitialised (malloc'd struct) unless `readkeyprm_i` quits on missing keys (depends on keyprm_errfn setting).


## B.4 Cross-references, duplicates and fix-together groups

**ID prefixes:**

| Prefix | Files |
|---|---|
| DEMO | atdemography |
| MOVE | atmovement |
| BXML | atBiologyXMLParamIO |
| ECOL | atecology.c |
| BSET | atbiolsetup |
| BMSG | atbiolmessage, atfluxbreakdown, atLandProcess, atImposeRecruit, atPhysChemIO |
| PROC | atprocess, atGroupProcesses |
| CONT | atContaminants, atvertprocesses, atexternalpop |
| BIOL | atbiology, additionalTracer, atannualbiology, atmacrophytes |
| NUTR | atNutrient, atdiversity, atbiophysics, atcoral |
| EINIT | init, atmigration, atIceProcesses, atBuildTracer, atq10, atcalibtools |
| EMISC | other atecology files and headers |
| QUOT | atquota, ateconeffort, ateffortquota, ateconts |
| ECON | ateconomicsetup, ateconresponse, atEconomicAnnual, misc |
| ECIO | ateconio, ateconindicator, ateconParamIO, FishMIP bridge |
| HARV | atHarvest, atHarvestParamIO, atHarvestCatch |
| HRV2 | other atharvest files and atimplementation |
| TIER | atManageTier |
| MNG | atManage, atManageIndex, atSS3assess |
| MNGA | atManageAnnual, atManageSetup |
| MNGIO | atManageParamIO, atPGMSY, atRlink*, atManageIO, atManageMPATS |
| SS3IO | atSS3LinkIO |
| SS3 | other atSS3Link files |
| PHY1 | aticeIO, attempsalt, atphysics, atphysIO |
| PHY2 | geometry, tracer IO, source/sink, transport |
| PHY3 | other atphysics files |
| UTIL | atUtilXML, atUtilGroupIO, atUtil |
| UTL2 | other atlantisUtil files and headers |
| MAIN | atlantismain |
| CNV1 / CNV2 | ConvertAtlantis |
| LINK | atCLAMLink, atlink, atbrokerlink |
| SJW1 / SJW2 / SJW3 | sjwlib |

**Same bug reported twice. Fix once and tick both entries:**
- **EMISC-26 = CLS-17 (Part A):** `Ecology_Spawn` gets X_RS and FSPB in swapped order in atclassical.c.
- **UTIL-17 ↔ CONT-01:** contaminant arrays are allocated per `numCohorts` (atUtilGroupIO.c:910-914) but looped to `numCohortsXnumGenes` (atContaminants.c). Fix either the allocation or the loops.
- **API-06 (Part A) ↔ CNV2-06:** `Num_Sexes` is read and written as a binary flag, in both the reader and the converter.
- **SJW1-06:** the `dfFindTime` rewind formula. The SJW3 reviewer found the same bug independently.
- **BIOL-02 ↔ PHY3-12:** `totDiet` is sized by `K_num_max_cohort`, but both of these loop to `numCohortsXnumGenes*numStocks`.
- **PHY3-06 ↔ PHY1-20:** leftover debug checks hard-code box index 24 (`newwc[24]` / `newwctr[24]`).
- **SRV-08 / ATT-08 (Part A) ↔ SJW3-15:** the `round()` macro in sjwlib.h evaluates its argument three times. Where that argument contains `drandom()`, three different random numbers feed one age assignment, which makes the double-`+0.5` age bug worse.

**Bugs reviewers noticed in files outside their own assignment.** Check these against the owning file's entries:
- `atmanage/atManageSetup.c:1552`: `cpue_cdf_poor_p_id` is used where `cpue_cdf_top_p_id` is meant. Found by the SJW2 reviewer.
- `atmanage/atManageParamIO.c:662`: `CPUEshotavgdis` is read from the key `"CPUEshotfdis"`. Found by the CNV2 reviewer.
- **MOVE-04:** the fix also needs `int HowFar` changed to `double` in `atlantisUtil/include/atFunctGroup.h:94`.
- `atlink/Makefile.am` lists `atlinkconversion.o` instead of `.c`. Found by the LINK reviewer.

**Code that isn't compiled or isn't called (findings there are latent):**
- Not compiled: `atecology/dump.c`, `atmanage/atRlink.c`, `atCLAMLink/atCLAMLink.c`, `atSS3Link/atSS3Test.c`.
- Compiled but never called: the FishMIP bridge (`ateconomic/atFishMIP_OSP_*`), `atPGMSY.c`'s `Initialise_PGMSY` (so `Do_Atlantis_PGMSY` crashes if it is ever reached; see MNGIO-08), and `nrcdflib` lognormal/Cauchy/Weibull.
- `init.c` is fully commented out.
- The atlink EwE code only builds with LINK_ENABLED.

**Fix together:**
- **TIER-35, TIER-41, SS3-34:** several calls pass (year, species) in swapped order. Check every caller of `DoCPUE`, `DoPrager`, `Call_Tier3`, `TierFiveSAFE` and `writeCloseKinNcompsfiles`.
- **SS3-26 ✔:** every local close-kin array in atCloseKin.c is allocated in index order. The code's own TODO asks for this check, and the reviewer notes the same pattern at atAssessTierSetup.c:536-549.
- **SS3IO-14, SS3IO-16, SS3IO-17, SS3IO-20, SS3IO-21, SS3IO-25, SS3IO-28, SS3IO-31:** all corrupt the generated SS3 input files. Fix them as one pass, then check the result against an SS3.30 example file.

## B.5 Follow-up header review (2026-10-12)

A full check of the five headers that were only partly covered in B.2. All 145 prototypes in `atlantisboxmodel.h` were compared with their definitions: none has a wrong return type, parameter count, type or order. The four atlantisUtil headers were checked line by line. That adds 6 findings (BMH-01, BMH-02, UTH-01 to UTH-04): none high severity, 2 medium, 4 low.

### atlantismain/include/atlantisboxmodel.h

| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| BMH-01 | 6110, 6129, 6131, 6155, 6173, 6174, 6175, 6237 | dead declarations | H/L | Eight prototypes have no definition anywhere in the source tree (externalLibs/netcdf excluded) and no call sites: `alloc_names`, `readRunParams`, `readSourceSink`, `writeBMSummaryEpiInfo`, `setfisheryNameID`, `setfisheryParamNameID`, `setspNameID`, `setSPid`. The code says some were removed on purpose: atmanage/atManageSetup.c:157 "Deleted the setSPid function", and atphysics/atparamIO.c:32 says readRunParams was replaced (now `Util_Read_Run_XML`). They are harmless today, but if someone adds a call, the build fails at link time instead of at compile time. Functions: as listed. | Delete the 8 prototypes | [ ] |
| BMH-02 | 6280-6281 (defs atmanage/atRlink.c:298, 359; atmanage/atRlinkRedus.c:269, 323) | duplicate definition (latent) | H/L | `freeRedus()` and `REDUS_management()` are each defined twice, in atRlink.c and in atRlinkRedus.c. Both copies have identical signatures and both sit under `#ifdef RASSESS_LINK_ENABLED`. atRlink.c is not listed in atmanage/Makefile.am (line 11 lists only atRlinkRedus.c, atRlinkRBC.c and atRlinkRAssess.c) or in any other build file, so it is a stale copy that does not link today. If it is ever added back to the sources, the build gets duplicate-symbol link errors, and in the meantime the two copies can drift apart. Callers (atlantismain.c:751, atmanage/atManageAnnual.c:2176) bind to the atRlinkRedus.c versions. Functions: `freeRedus()`, `REDUS_management()`. | Delete atmanage/atRlink.c, or move it to an archive folder | [ ] |

Prototype check summary: all 145 prototypes in the header's prototype section (lines 6093-6307, including the 8 under `#ifdef RASSESS_LINK_ENABLED`) were checked against every definition in the repo's .c/.cpp files. There are no other function prototypes in the header, only the `void(*err)(...)` parameter types. The check was done with a helper script (scratchpad/bmh_protos.py, which strips comments and string literals), followed by a manual pass over its full signature dump (scratchpad/bmh_sigs.txt).
- 134 match exactly: same return type, same number of parameters, and the same parameter types in the same order. This includes `physics()`, whose definition in atphysics.c:666 also uses the reduced 4-argument form that matches the header comment, and the only call (atlantismain.c:550) follows it.
- No return-type, parameter-count, parameter-type or order-swap mismatches were found. Only 4 prototypes name their parameters differently from the definition, and none of these is a swap: readBMIceInfo `fielName`/`fileName` (a typo), Util_Read_Run_XML `name`/`fileName`, Determine_Stock_Prop `sp`/`species`, and Get_Length `i`/`sp`.
- 8 have no definition (BMH-01).
- 3 are defined in two places with identical signatures: freeRedus and REDUS_management (BMH-02), and `Quicksort_Dir` (ateconhelp.c:42 and atUtilhelp.c:43). Quicksort_Dir is already logged as ECIO-26 and is not logged again here.
- None of the prototype names is declared in another header with a different signature. The only other declaration is Quicksort_Dir in ateconomic/include/atEconomic.h:111, which is identical. No `static` definition or file-local redeclaration conflicts with any of these prototypes.

#define and enum value checks: I split the header's #defines into their comment- or blank-separated groups and its 23 enums into members. The enums have no duplicate values, and every count member is the last member. In the #define groups, the only duplicates are deliberate aliases:
- `fuel_intercept_id` and `market_intercept_id` are both 0 and index different arrays.
- `tier_orig` and `tier0` are both 0.
- The "changing conditions" group (1220-1231) overlays several change schemas. For example `end_id`, `period_id`, `selcurvechange_id` and `thresh_mult_id` are all 1, and `pulse_id` and `discard_mult_id` are both 3. Usage shows each array uses only one schema; the `end_id` and `period_id` overlap appears only in comments (atbiolUtil.c:124-125).
No `K_num_*`, `num_*` or `tot_*` count is smaller than or equal to an id in its own group. The `HABITAT_TYPES` versus `nlevel_id`/`growth_id`/`death_id` overlap is already logged as MAIN-11, and `within_id` aliasing into SUBFLEET_ECONprms is already logged as QUOT-13.

### atlantisUtil/include/atFunctGroup.h

| ID | Lines | Category | Conf/Sev | Description and reason | Suggested fix | Status |
|---|---|---|---|---|---|---|
| UTH-01 | 306 | fixed-size array vs unbounded index | M/M | `int spMinMax[4];` holds the min/max SN tracer id per stage, for juvenile and adult only. atbiolsetup.c:3600-3603 (`Set_spMinMax_Values()`) fills just [0..3]. It is read as `spMinMax[(preystage * 2)]` / `[(preystage * 2 + 1)]` with `preystage < FunctGroupArray[prey].numStages` (atmovement.c:283, 299-300), and the same with `predstage < numStages` (atmovement.c:472, 481-482). `numStages` is only checked to be >= 1 (atUtilGroupIO.c:294). An AGE_STRUCTURED group with 3+ stages therefore reads past the array into the next struct members (`AGEnewden` pointer bytes), and the garbage value is then used as a tracer index. This is the same root cause as BXML-05 (no `numStages <= 2` check), but that finding doesn't list `spMinMax`. Struct: `FunctionalGroupStruct`. | Add a group-load check that `numStages <= 2` for vertebrates (covers BXML-05 too), or make `spMinMax` `int *` sized `2*numStages` and fill per stage. | [ ] |
| UTH-02 | 351-353 | allocation size vs index (stage vs habitat) | M/M | The comment says `pSP_SG_eat` is "diet available for each cohort of seagrass". It is allocated with a fixed size of 3 (atBiologyXMLParamIO.c:833, `Util_Alloc_Init_1D_Double(3, 0.0)`). It is filled in `Read_Seagrass_Diet_Avail_XML()` (atBiologyXMLParamIO.c:2375-2390) as `pSP_SG_eat[cohort] = values[b]` with `cohort < FunctGroupArray[sp].numStages` of the seagrass group, but read in `Get_Prey_Avail` code as `pSP_SG_eat[habitat]` (atprocess.c:368). If the seagrass group has more than 3 stages, every predator's array is overrun. If it has fewer than 3, the epiphyte/root entries stay 0. Either way the fill index (stage) and the read index (habitat) are not the same thing. The in-code comment (line 2375) admits the loop "is actually habitats ... rather than cohorts". | Loop the read over the 3 seagrass components (or `FunctGroupArray[sp].numCohorts`, and allocate to match). Allocate `pSP_SG_eat` from the same constant used to read it, and fix the header comment. | [ ] |
| UTH-03 | 80-82 | buffer overrun (unbounded strcpy) | M/L | `char groupCode[20]`, `name[200]`, `fullName[200]` are filled by `Util_XML_Get_Value_String(..., "GroupCode"/"Name"/"LongName", FunctGroupArray[groupIndex].groupCode)` (atUtilGroupIO.c:264, 274-275). That function does `strcpy(returnValue, value)` with no length (atUtilXML.c:1310). `Is_Valid_Group_Code()` (2-3 letters) only runs after the copy (line 269). An over-long code or name in the groups XML/CSV therefore overflows into the following int fields before validation. The earlier UTL2 note only covered `FisheryStruct.fisheryCode[20]`/`name[200]` (same mechanism, atUtilFisheryIO.c). | Add a size argument to `Util_XML_Get_Value_String` and use a bounded copy (`snprintf(dst, size, "%s", value)`), or check `strlen(value)` before copying. | [ ] |
| UTH-04 | 36-72 | enum vs name table (converter lookup) | H/L | The GROUP_TYPES enum has 33 real values, and the runtime table in atGroupTypes.h covers all of them. The converter's `GroupType` Lookup list (ConvertAtlantis/atGroupsXML.c:181-206) is missing `BIRD`, `MICROPHTYBENTHOS`, `CORAL`, `SPONGE`, `ICE_DIATOMS`, `ICE_MIXOTROPHS` and `ICE_ZOOBIOTA`. The Lookup nodes are documentation only (nothing validates against them), so this doesn't change results, but the generated XML advertises an incomplete set of legal group types. | Add the missing `Util_XML_Create_Lookup_Value_Node` entries. | [ ] |

### atlantisUtil/include/atTracer.h

No issues found. Checked: every `extern int` tracer index against its definition in atecology/include/atnamelist.h:87-145 (only included by atbiolsetup.c:373, so there is no multiple definition) and against its registration in atPhysChemIO.c / atBuildTracer.c. Also checked that optional tracers (SED_i, Wind_i, Rugosity_i, AragoniteSat_i, Noise/Light_Pollution_i, P/C/TOP and the phosphorus set) are only used under the same flag that registers them (flagIsEstuary, track_wind, track_rugosity_arag, flag_pollutant_impacts / isNoiseEffected, track_atomic_ratio). pH_i is explicitly set to -1 when not tracked.

### atlantisUtil/include/atGroupTypes.h

No issues found. The `groupTypes[]` table has 33 entries covering every GROUP_TYPES value except the NUM_GROUP_TYPES sentinel. Its order differs from the enum (MICROPHTYBENTHOS sits before CORAL), but the table is only searched by string (atUtilGroupIO.c:345-347), so order doesn't matter. FISH_INVERT is remapped to FISH right afterwards (atUtilGroupIO.c:357-360). All GROUP_TYPES switches (atecology.c:301, atUtilGroupIO.c:1443, atassess/atindices.c, atnetwork.c, atdatareading.c) and the `spErrorStructure[NUM_GROUP_TYPES]` initialisation in atassess/atsamplesetup.c:733-777 cover every invertebrate type that can reach them.

### atlantisUtil/include/atFisheryStruct.h

No new issues. `fisheryCode[20]`/`name[200]` unbounded strcpy was already noted in 28_UTL2 (see UTH-03 for the same fix). `isRec`, `index`, `totalEffortTracer`, `averageCatchSizeTracer` are all set (atUtilFisheryIO.c:119-121, atHarvestSetup.c:391/412). The AssessProjectionStruct allocation, indexing and free are covered by MNGIO-08..17 (atmanage/atPGMSY.c).

Lower-confidence notes:
- Cross-checked and already logged elsewhere, so not re-logged here: HowFar int (MOVE-04); habitatCoeffs sizing (UTIL-19); contaminant arrays including `agingContam` sized numCohorts (UTIL-17/CONT-01); ratioLost*/ratioGained* habitat=3 and numCohorts (UTIL-18); isCloseKin/isCollisionRisk uninitialised (UTIL-20/21); containsTurf (UTIL-22); `cohortSpeciesParams` indexed by cohort (NUTR-26); `grow` reset loop bound (ECOL-16); `distrib_VERTICAL`/`distrib_OVERWINTER`/`distrib` stage dimension 2 (BXML-05); `co_sp`/`co_sp_catch` indexing (TIER-38, MNGA-02, MNGIO-05).
- Header comments that disagree with the allocations (cosmetic): `pSPEat` says `[cohort][prey][habitat]`, but the first dimension is `numStages` (atUtilGroupIO.c:832) and every caller loops stages, so it's correct. `ratioGainedPred` says "one value per cohort, element and habitat", but it is 2-D `[nCxG][num_atomic_id]` (atUtilGroupIO.c:896). The `FisheryStruct` doc comment says "each functional group", and the `AssessProjectionStruct` comment says "evolution relevant information" (copy-paste).
- atTracer.h declares `Diatom_S_i` and `MicroPB_S_i` extern, but they are never defined or used anywhere. Any future use would be a link error. atTracer.h also has no include guard, which is harmless for extern declarations.
- `isWCActiveOnly` (atFunctGroup.h:130) is set (atUtilGroupIO.c:1509-1512) but never read. `index` (line 84), `MaxNumMig`, `SpawnSizeTracer` and `tempINVpopratio` are likewise set/allocated but never used (dead members, no bug).
- atGroupTypes.h defines a non-static, initialised object (`GroupTypesStruct groupTypes[]`) in a header. It is only safe while exactly one translation unit includes it (currently atUtilGroupIO.c). Making it `static const` would prevent a future duplicate-symbol error.
