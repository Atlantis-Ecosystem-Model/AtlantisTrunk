# atassess/atclassical.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **24 findings** (severity H: 8; confidence H and severity H: 7; on the priority list: 8).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [CLS-01](#cls-01) ⭐ | ≈213-219, ≈441-447 | H/H | off-by-one / algorithm | Starting simplex has only Ndim rows; row 0 is unscaled and the last parameter (Ndim-1) is never perturbed, so … |  | [ ] |
| [CLS-02](#cls-02) | 166-169, 207-209 | H/M | uninitialised | With Pella-Tomlinson, Ndim=4 but only X[0..2] are reset; X[3] is stale from the previous fit. |  | [ ] |
| [CLS-03](#cls-03) | 274, 498 | H/M | off-by-one | `ipnt = floor(icnt*U + 1.0)` gives 1..icnt, so BootResu[0] is never used and BootResu[icnt] is stale. |  | [ ] |
| [CLS-04](#cls-04) | 294-297, 524-527 | H/M | off-by-one | Failure test reads `[nbs+1]` = `[bootstrap+1]` (never written). The sort takes slots 0..bootstrap-1, which inc… |  | [ ] |
| [CLS-05](#cls-05) ⭐ | 228-233, 461 | M/H | see SOL-1 | Takes `P[Ilow]` which, because of the Amoeba swap bug, is not the best vertex. |  | [ ] |
| [CLS-06](#cls-06) | 231, 459 | M/L | off-by-one | `nfunk <= Assess_Max_Int` treats hitting the iteration limit as success. |  | [ ] |
| [CLS-07](#cls-07) | 521-574 | H/M | misplaced block | `if (!assess_failed)` save sits inside the cohort loop, so partial sums are stored if a later cohort fails. |  | [ ] |
| [CLS-08](#cls-08) ✔ ⭐ | 668-680 | H/H | off-by-one | `z < nfzones+1` over `stockinfo`, `individVERTinfo` and `agelengthkey`, which all have zone dimension `nfzones… |  | [ ] |
| [CLS-09](#cls-09) | 723, 738-741, 793 | M/M | wrong variable | `numCohorts` used where tracer arrays need `numCohortsXnumGenes`. |  | [ ] |
| [CLS-10](#cls-10) | 855, 878, 895, 980, 992, 1045 | M/M | unit mismatch | After line 855, `nagemat` is in years but is compared with cohort indices. |  | [ ] |
| [CLS-11](#cls-11) ⭐ | 1042, 1050-1051, 1079-1080 | H/H | wrong var / OOB | `nagemat` (years) used as a cohort index into `scaled_C`, `scaled_mum`, `biolVERTinfo`; e.g. 15 > 10 cohorts. |  | [ ] |
| [CLS-12](#cls-12) ⭐ | 972, 1002 | H/H | accumulator reset | `catch_N = 0` inside the zone loop, while start_N/end_N sum across all zones. |  | [ ] |
| [CLS-13](#cls-13) | 893 | H/M | loop condition | `while (!end_N ...)`: end_N accumulates across zones, so later zones skip the search. |  | [ ] |
| [CLS-14](#cls-14) | 949-951, 1025-1027 | M/M | formula | `totDead = start-end` already includes catch, so `start-catch-totDead` double-counts catch (log of a negative … |  | [ ] |
| [CLS-15](#cls-15) | 918-925, 1068, 1076-1079 | M/M | wrong variable | `b` is stale per chrt; calcE uses the running cross-zone calcB; predation uses the prey's catch and clearance … |  | [ ] |
| [CLS-16](#cls-16) ⭐ | 1145, 1186-1187 | H/H | wrong var / OOB | `NumsTracers[na]` with na up to numCohorts×ageClassSize-1. |  | [ ] |
| [CLS-17](#cls-17) ✔ ⭐ | 1218-1219, 1555-1556 | H/H | swapped args | Signature is `Ecology_Spawn(..., X_RS, FSPB, log)` (atvertprocesses.c:342), but all four calls pass `scaled_FS… |  | [ ] |
| [CLS-18](#cls-18) ⭐ | 1242-1279, 1596-1660 | H/H | logic | BrefC branch sets `not_done = 0` and overrides BrefA's `not_done = 1`. |  | [ ] |
| [CLS-19](#cls-19) | 1250-1251, 1273-1274, 1285-1293 | M/M | numerical | `diff = 1 - log(1/ratio)` goes negative (negative F) and is unguarded here, unlike line 1608; the NaN check co… |  | [ ] |
| [CLS-20](#cls-20) | 1224-1225 vs 1116 | L/M | inconsistency | Beverton-Holt denominator uses `tbiom1` here but `spawn_biom1` at line 1116; cohort counts also differ. |  | [ ] |
| [CLS-21](#cls-21) | 1374-1386; 287-289, 510-512 | H/M | sort algorithm | Rank sort gives ties the same slot, leaving stale entries; percentile index can equal nbs. |  | [ ] |
| [CLS-22](#cls-22) | 1465-1479 | M/M | log domain | After the sanity resets, Z = log(start/small_num) is huge, or log of a negative. |  | [ ] |
| [CLS-23](#cls-23) | 1493-1509 | L/L | other | Rescaled M is not written back to `NAssess[est_M_id]`. |  | [ ] |
| [CLS-24](#cls-24) | 1476, 1535-1536, 1565-1566 | H/L | log text | Message says 1.2 (code uses 1.25); `ageClassSize` is labelled nageclass; tempF is printed for est2. |  | [ ] |

## Reviewer notes for this file

Lower-confidence notes: `va95` is treated as a cohort at line 860 but as years at 926/1209. `CPUEtrend[sp][ny]` can overflow if pseudo-assessment runs from tstart. VPA "biomass" sums NEst, which may be numbers.

---

## CLS-01

**off-by-one / algorithm** · Conf H / Sev H ⭐ priority  
**Lines (current code):** ≈213-219, ≈441-447  
**Lines at review (Oct 2026):** 213-219, 441-447

**Why it's thought to be a bug**

Starting simplex has only Ndim rows; row 0 is unscaled and the last parameter (Ndim-1) is never perturbed, so Amoeba can't move it. See SOL-2.

**Current code** (`atassess/atclassical.c` lines 211–221) — ≈ the code here has changed since the review; lines are approximate

```c
  211: 		/** Fit the model using Amoeba **/
  212: 		/* Set up tolerances and gridding - Nov 2026: Ndim+1 vertices (was Ndim, so last parameter never perturbed) */
  213: 		for (i = 0; i < Ndim + 1; i++) {
  214: 			for (j = 0; j < Ndim; j++) {
  215: 				P[i][j] = X[j];
  216: 				if (i - 1 == j)
  217: 					P[i][j] *= GRD;
  218: 			}
  219: 		}
  220: 
  221: 		for (i = 0; i < Ndim + 1; i++) {
```

**Suggested fix (from review):** build Ndim+1 rows with `P[i+1][i] *= GRD`; allocate P/Y with nk+1 rows

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

## CLS-02

**uninitialised** · Conf H / Sev M  
**Lines (current code):** 166-169, 207-209  
**Lines at review (Oct 2026):** 166-169, 207-209

**Why it's thought to be a bug**

With Pella-Tomlinson, Ndim=4 but only X[0..2] are reset; X[3] is stale from the previous fit.

**Current code** (`atassess/atclassical.c` lines 164–203 (truncated))

```c
  164: 
  165: 	/* Number of dimensions in data arrays to be solved */
  166: 	if ((int) (prm_sp)) {
  167: 		Ndim = est_p_id + 1;
  168: 	} else
  169: 		Ndim = est_B0_id + 1;
  170: 
  171: 	/* Specifiy data type to use */
  172: 	if ((FunctGroupArray[sp].groupType == BIRD) || (FunctGroupArray[sp].groupType == MAMMAL)) {
  173: 		/* FIX - Current code only allows for survey data of birds and mammals, this may need to change if have whaling etc */
  174: 		dataid = survey_id;
  175: 	}
  176: 
  177: 	/* Data is loaded in the sampling routines FisheriesRecords() and FishEstimates() */
  178: 	/* Set up base data for the optimisation
  179: 	 FIX -- At present uses commerical cpue at age, when should really use survey
  180: 	 cpue at age, but how to define survey effort so can get cpue?? */
  181: 	for (Yr = 0; Yr < YrMax + 1; Yr++) {
  182: 		CTData[Yr] = 0;
  183: 		ITData[Yr] = 0;
  184: 		yr_effort = bm->EffortRecord[Yr][sp][dataid];
  185: 		for (age = 0; age < FunctGroupArray[sp].numCohortsXnumGenes; age++) {
  186: 			CTData[Yr] += bm->CatchRecord[Yr][sp][age][dataid];
  187: 			ITData[Yr] += bm->CatchRecord[Yr][sp][age][dataid] / (yr_effort + small_num);
  188: 
  189: 			fprintf(ofp, "Time: %e, Yr: %d, CTData: %e, ITData: %e, Catch-%s-%d: %e, yr_effort: %e\n", bm->dayt, Yr, CTData[Yr], ITData[Yr],
  190: 					FunctGroupArray[sp].groupCode, age, bm->CatchRecord[Yr][sp][age][dataid], yr_effort);
  191: 		}
  192: 	}
  193: 
  194: 	/* Bootstrap loop - nbs set to -1 so get one assessment loop even when not
  195: 	 bootstrapping */
  196: 	for (nbs = -1; nbs < bootstrap; nbs++) {
  197: 		/* Do projections and minimise sums of squares using amoeba
  198: 		 Assumes
  199: 		 X[est_r_id] = r
  200: 		 X[est_K_id] = K
  201: 		 X[est_B0_id] = B0
  202: 		 X[est_p_id] = p
  203: 
```

**Suggested fix (from review):** loop `i < Ndim`

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

## CLS-03

**off-by-one** · Conf H / Sev M  
**Lines (current code):** 274, 498  
**Lines at review (Oct 2026):** 274, 498

**Why it's thought to be a bug**

`ipnt = floor(icnt*U + 1.0)` gives 1..icnt, so BootResu[0] is never used and BootResu[icnt] is stale.

**Current code** (`atassess/atclassical.c` lines 272–276)

```c
  272: 			ITData[Yr] = 0;
  273: 			yr_effort = bm->EffortRecord[Yr][sp][dataid];
  274: 			ipnt = (int) floor(ROUNDGUARD + (icnt * drandom(0.0, 1.0) + 1.0));
  275: 			for (age = 0; age < FunctGroupArray[sp].numCohortsXnumGenes; age++) {
  276: 				ITData[Yr] += (bm->CatchRecord[Yr][sp][age][dataid] / (yr_effort + small_num)) * exp(BootResu[ipnt]);
```

**Suggested fix (from review):** drop `+1.0`, clamp

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

## CLS-04

**off-by-one** · Conf H / Sev M  
**Lines (current code):** 294-297, 524-527  
**Lines at review (Oct 2026):** 294-297, 524-527

**Why it's thought to be a bug**

Failure test reads `[nbs+1]` = `[bootstrap+1]` (never written). The sort takes slots 0..bootstrap-1, which includes the real-data fit and excludes the last bootstrap. MAXDOUBLE failures are not excluded.

**Current code** (`atassess/atclassical.c` lines 292–299)

```c
  292: 		/* Bootstraps to chose between */
  293: 		for (i = 0; i < est_msy_id; i++) {
  294: 			if ((NResult[YrMax][i][nbs + 1] < MAXDOUBLE) && ((i == est_r_id) || (i == est_K_id) || (i == est_B_id))) {
  295: 				for (j = 0; j < bootstrap; j++) {
  296: 					ResultToSort[j] = NResult[YrMax][i][j];
  297: 				}
  298: 				Calc_Percentile(ResultToSort, ResultSorted, nbsSort, nbs, (FunctGroupArray[sp].numCohortsXnumGenes), YrMax, ofp);
  299: 
```

**Suggested fix (from review):** sort `[j+1]`, skip MAXDOUBLE

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

## CLS-05

**see SOL-1** · Conf M / Sev H ⭐ priority  
**Lines (current code):** 228-233, 461  
**Lines at review (Oct 2026):** 228-233, 461

**Why it's thought to be a bug**

Takes `P[Ilow]` which, because of the Amoeba swap bug, is not the best vertex.

**Current code** (`atassess/atclassical.c` lines 226–235)

```c
  226: 		}
  227: 
  228: 		Amoeba(bm, 1, sp, bm->dayt, FunctGroupArray[sp].groupCode, P, Y, Ndim, bm->Assess_Tol, bm->Assess_Max_Int, funkflag, (FunctGroupArray[sp].numCohortsXnumGenes), YrMax,
  229: 				prm_sp, &nfunk, &Ilow, ofp, NULL);
  230: 
  231: 		if (nfunk <= bm->Assess_Max_Int) {
  232: 			for (j = 0; j < Ndim; j++) {
  233: 				X[j] = P[Ilow][j];
  234: 			}
  235: 			SS = Funk(bm, funkflag, sp, (FunctGroupArray[sp].numCohortsXnumGenes), Ndim, YrMax, prm_sp, X, ofp, NULL);
```

**Suggested fix (from review):** fix in atsolve.c

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

## CLS-06

**off-by-one** · Conf M / Sev L  
**Lines (current code):** 231, 459  
**Lines at review (Oct 2026):** 231, 459

**Why it's thought to be a bug**

`nfunk <= Assess_Max_Int` treats hitting the iteration limit as success.

**Current code** (`atassess/atclassical.c` lines 229–233)

```c
  229: 				prm_sp, &nfunk, &Ilow, ofp, NULL);
  230: 
  231: 		if (nfunk <= bm->Assess_Max_Int) {
  232: 			for (j = 0; j < Ndim; j++) {
  233: 				X[j] = P[Ilow][j];
```

**Suggested fix (from review):** `<`, or an explicit flag

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

## CLS-07

**misplaced block** · Conf H / Sev M  
**Lines (current code):** 521-574  
**Lines at review (Oct 2026):** 521-574

**Why it's thought to be a bug**

`if (!assess_failed)` save sits inside the cohort loop, so partial sums are stored if a later cohort fails.

**Current code** (`atassess/atclassical.c` lines 519–558 (truncated))

```c
  519: 		med_biom = 0;
  520: 		assess_failed = 0;
  521: 		for (i = 0; i < FunctGroupArray[sp].numCohortsXnumGenes; i++) {
  522: 
  523: 			/* FIX -- Only want latest estimates or over the entire assessment period ?*/
  524: 			if (NResult[YrMax][i][nbs + 1] < MAXDOUBLE) {
  525: 				for (j = 0; j < bootstrap; j++) {
  526: 					ResultToSort[j] = NResult[YrMax][i][j];
  527: 				}
  528: 
  529: 				Calc_Percentile(ResultToSort, ResultSorted, nbsSort, nbs, (FunctGroupArray[sp].numCohortsXnumGenes), YrMax, ofp);
  530: 
  531: 				/* Save the estimates for r, K, stock biomass estimate */
  532: 				if ((i == 0) || (i == pchrt)) {
  533: 					/* Get index */
  534: 					if (i == 0) { /* Recruits */
  535: 						indxm = est_med_recruit_id;
  536: 						indxt = est_top_recruit_id;
  537: 						indxb = est_bot_recruit_id;
  538: 					} else if (i == pchrt) { /* Plus Group */
  539: 						indxm = est_med_prm2_id;
  540: 						indxt = est_top_prm2_id;
  541: 						indxb = est_bot_prm2_id;
  542: 					}
  543: 
  544: 					/* Median */
  545: 					bm->NAssess[sp][indxm] = ResultSorted[med_p];
  546: 
  547: 					/* Lower x percentile */
  548: 					bm->NAssess[sp][indxb] = ResultSorted[low_p];
  549: 
  550: 					/* Upper x percentile */
  551: 					bm->NAssess[sp][indxt] = ResultSorted[high_p];
  552: 				}
  553: 
  554: 				top_biom += ResultSorted[high_p];
  555: 				med_biom += ResultSorted[med_p];
  556: 				bot_biom += ResultSorted[low_p];
  557: 			} else
  558: 				assess_failed = 1;
```

**Suggested fix (from review):** move after the loop

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

## CLS-08

**off-by-one** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Lines (current code):** 668-680  
**Lines at review (Oct 2026):** 668-680

**Why it's thought to be a bug**

`z < nfzones+1` over `stockinfo`, `individVERTinfo` and `agelengthkey`, which all have zone dimension `nfzones` (atsamplesetup.c:213, 223, 226).

**Current code** (`atassess/atclassical.c` lines 666–682)

```c
  666:     
  667:     /** Find new recruitment and biomass estimates **/
  668: 	for (z = 0; z < bm->nfzones + 1; z++) {
  669:         
  670: 		/** Get biomass estimate **/
  671: 		med_biom += stockinfo[sbiomass_id][sp][z][sample_id];
  672: 		bot_biom += (bot_pcnt / 100.0) * stockinfo[sbiomass_id][sp][z][sample_id];
  673: 		top_biom += (top_pcnt / 100.0) * stockinfo[sbiomass_id][sp][z][sample_id];
  674:         
  675: 		/** Get recruitment estimate **/
  676: 		/* Get Biomass of young-of-year */
  677: 		biom_yoy = 0;
  678: 		for (i = 0; i < bm->K_num_size; i++) {
  679: 			biom_yoy += individVERTinfo[ibiomass_id][sp][i][z][sample_id] * agelengthkey[i][0][sp][z][sample_id];
  680: 		}
  681: 		med_recruit += biom_yoy;
  682: 		bot_recruit += (bot_pcnt / 100.0) * biom_yoy;
```

**Suggested fix (from review):** `z < nfzones`

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

## CLS-09

**wrong variable** · Conf M / Sev M  
**Lines (current code):** 723, 738-741, 793  
**Lines at review (Oct 2026):** 723, 738-741, 793

**Why it's thought to be a bug**

`numCohorts` used where tracer arrays need `numCohortsXnumGenes`.

**Current code** (`atassess/atclassical.c` lines 721–743)

```c
  721: 	int b, nc, k, sn, rn, den;
  722: 	double med_recruit, bot_recruit, top_recruit, med_biom, bot_biom, top_biom, biom_yoy, counter, calcF, calcM;
  723:     int Ncohorts = FunctGroupArray[sp].numCohorts;
  724:     int maxstock = FunctGroupArray[sp].numStocks;
  725:     
  726: 	/** Initialise estimates **/
  727: 	med_recruit = 0;
  728: 	bot_recruit = 0;
  729: 	top_recruit = 0;
  730: 	med_biom = 0;
  731: 	bot_biom = 0;
  732: 	top_biom = 0;
  733: 	biom_yoy = 0;
  734:     
  735: 	/** Find new recruitment and biomass estimates **/
  736: 	for (b = 0; b < bm->nbox; b++) {
  737:         if (bm->boxes[b].type != BOUNDARY) {
  738:             for (nc = 0; nc < Ncohorts; nc++) {
  739:                 sn = FunctGroupArray[sp].structNTracers[nc];
  740:                 rn = FunctGroupArray[sp].resNTracers[nc];
  741:                 den = FunctGroupArray[sp].NumsTracers[nc];
  742:                 for (k=0; k < bm->boxes[b].nz; k++) {
  743:                     med_biom += (bm->boxes[b].tr[k][sn] + bm->boxes[b].tr[k][rn]) * bm->boxes[b].tr[k][den] * FunctGroupArray[sp].habitatCoeffs[WC] * bm->X_CN * mg_2_kg;
```

**Suggested fix (from review):** use XnumGenes

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

## CLS-10

**unit mismatch** · Conf M / Sev M  
**Lines (current code):** 855, 878, 895, 980, 992, 1045  
**Lines at review (Oct 2026):** 855, 878, 895, 980, 992, 1045

**Why it's thought to be a bug**

After line 855, `nagemat` is in years but is compared with cohort indices.

**Current code** (`atassess/atclassical.c` lines 853–892 (truncated))

```c
  853:     */
  854:     
  855: 	nagemat *= sp_AgeClassSize;
  856: 
  857: 	/** Natural mortality (M) and current fishing mortality (Fcurr) estimation **/
  858: 
  859: 	/* start_N = numbers in first cohort available to the fishery (use age given by va95) */
  860: 	nc = va95 * FunctGroupArray[sp].numGeneTypes;
  861: 	nc_remain = 0;
  862: 	start_N = 0;
  863: 	end_N = 0;
  864: 	catch_N = 0;
  865: 	minend_N = MAXDOUBLE;
  866: 	minnc = 0;
  867: 	min_remain = 0;
  868: 	maxend_N = 0;
  869: 	maxnc = 0;
  870: 	max_remain = 0;
  871: 
  872: 	/* TODO: Check with Beth about these values - don't think they are supposed to be 0.*/
  873: 	size_nc = 0;
  874: 	end_nc = 0;
  875: 
  876: 	max_yrs_included = 0;
  877: 	for (z = 0; z < bm->nfzones; z++) {
  878: 		if (nc < nagemat)
  879: 			stage = 0;
  880: 		else
  881: 			stage = 1;
  882: 		b = bm->rep_box_of_zone[sp][z][stage];
  883: 		/* Determine size of first capture (equivalent to youngest) */
  884: 		wgt = biolVERTinfo[bstruct_id][sp][nc][b] + biolVERTinfo[bres_id][sp][nc][b];
  885: 		lngth = Get_Length(bm, wgt, sp);
  886: 		Sort_Length_Weight(bm, 0, 1, sizestocknums_id, z, sp, nc_fishery, lngth, 0, wgt, cmaxsize, sample_id, &size_nc_start, ofp);
  887: 		popratio = zoneVERTpopratio[sp][nc][nc_remain][z];
  888: 		start_N += individVERTinfo[istocknums_id][sp][size_nc_start][z][sample_id] * stockinfo[sstocknums_id][sp][z][sample_id] * popratio;
  889: 
  890: 		/* Find oldest fish - Iterate through the size classes if top one empty */
  891: 		nc_remain_end = sp_AgeClassSize - 1;
  892: 		nc_end = (FunctGroupArray[sp].numCohortsXnumGenes) - 1;
```

**Suggested fix (from review):** keep a separate cohort-level variable

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

## CLS-11

**wrong var / OOB** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 1042, 1050-1051, 1079-1080  
**Lines at review (Oct 2026):** 1042, 1050-1051, 1079-1080

**Why it's thought to be a bug**

`nagemat` (years) used as a cohort index into `scaled_C`, `scaled_mum`, `biolVERTinfo`; e.g. 15 > 10 cohorts.

**Current code** (`atassess/atclassical.c` lines 1040–1079 (truncated))

```c
 1040: 			calcC = 0;
 1041: 			calcB = 0;
 1042: 			nc = nagemat * FunctGroupArray[sp].numGeneTypes;
 1043: 			nc_remain = 0;
 1044: 			for (z = 0; z < bm->nfzones; z++) {
 1045: 				if (nc < nagemat)
 1046: 					stage = 0;
 1047: 				else
 1048: 					stage = 1;
 1049: 				b = bm->rep_box_of_zone[sp][z][stage];
 1050: 				popratio = zoneVERTpopratio[sp][nc][nc_remain][z];
 1051: 				wgt = biolVERTinfo[bstruct_id][sp][nc][b] + biolVERTinfo[bres_id][sp][nc][b];
 1052: 				lngth = Get_Length(bm, wgt, sp);
 1053: 				Sort_Length_Weight(bm, 0, 1, sizestocknums_id, z, sp, nc_fishery, lngth, 0, wgt, cmaxsize, sample_id, &chrt_size, ofp);
 1054: 				calcB += individVERTinfo[istocknums_id][sp][chrt_size][z][sample_id] * stockinfo[sstocknums_id][sp][z][sample_id] * popratio;
 1055: 				calcC += individVERTinfo[icatchnums_id][sp][chrt_size][z][sample_id] * stockinfo[scatchnums_id][sp][z][sample_id] * popratio;
 1056: 
 1057: 				fprintf( bm->logFile, "Time: %e %s b: %d, z: %d, chrt_size: %d, lngth: %e, wgt: %e (nc: %d, SN: %e, RN: %e), individVERTinfo: %e, stockinfo: %e, individCatch: %e, stockcatch: %e, popratio: %e, \n",
 1058: 						bm->dayt, FunctGroupArray[sp].groupCode, b, z, chrt_size, lngth, wgt, nc, biolVERTinfo[bstruct_id][sp][nc][b],
 1059: 						biolVERTinfo[bres_id][sp][nc][b], individVERTinfo[istocknums_id][sp][chrt_size][z][sample_id],
 1060: 						stockinfo[sstocknums_id][sp][z][sample_id], individVERTinfo[icatchnums_id][sp][chrt_size][z][sample_id],
 1061: 						stockinfo[scatchnums_id][sp][z][sample_id], popratio);
 1062: 
 1063: 				//step1 = (mL + mQ * calcB) * calcB;
 1064: 				//step2 = calcB * calcB / (step1 + small_num);
 1065: 				//calcE += (mL + mQ * step2) * step2;	
 1066: 				/* Other mortality */
 1067: 
 1068: 				calcE += (mL + mQ * calcB) * calcB;
 1069: 				for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
 1070: 					E1_sp= 0;
 1071: 					step1 = 0;
 1072: 					clear = 0;
 1073: 					if (FunctGroupArray[pred].isVertebrate == TRUE) {
 1074: 						/* Predation mortality */
 1075: 						E1_sp = FunctGroupArray[pred].speciesParams[E1_id];
 1076: 						step1 = bm->pSPVERTeat[pred][sp][adult_id][adult_id] * stockinfo[scatchnums_id][sp][z][sample_id] * popratio;
 1077: 						//	clear = bm->C_spVERT[sp][nagemat] / (1.0 + step1 * E1_sp * bm->C_spVERT[sp][nagemat] / bm->mum_spVERT[sp][nagemat]);
 1078: 						/* Don't worry about the numsec in the later part of the equation as they will cancel out */
 1079: 						clear = FunctGroupArray[sp].scaled_C[nagemat] * numsec / (1.0 + step1 * E1_sp * FunctGroupArray[sp].scaled_C[nagemat]
```

**Suggested fix (from review):** use cohort-level age_mat

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

## CLS-12

**accumulator reset** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 972, 1002  
**Lines at review (Oct 2026):** 972, 1002

**Why it's thought to be a bug**

`catch_N = 0` inside the zone loop, while start_N/end_N sum across all zones.

**Current code** (`atassess/atclassical.c` lines 970–1004)

```c
  970: 			start_N = 0;
  971: 			end_N = 0;
  972: 			for (z = 0; z < bm->nfzones; z++) {
  973: 
  974: 				/*if(sp == FPO_id){
  975: 				 fprintf(ofp, "individVERTinfo[istocknums_id][sp][maxnc][z][sample_id] = %e\n",individVERTinfo[istocknums_id][sp][maxnc][z][sample_id]);
  976: 				 fprintf(ofp, "stockinfo[sstocknums_id][sp][z][sample_id] = %e\n",stockinfo[sstocknums_id][sp][z][sample_id]);
  977: 				 fprintf(ofp, "popratio = %e\n", popratio);
  978: 				 }
  979: 				 */
  980: 				if (maxnc < nagemat)
  981: 					stage = 0;
  982: 				else
  983: 					stage = 1;
  984: 				b = bm->rep_box_of_zone[sp][z][stage];
  985: 				popratio = zoneVERTpopratio[sp][maxnc][max_remain][z];
  986: 
  987: 				wgt = biolVERTinfo[bstruct_id][sp][maxnc][b] + biolVERTinfo[bres_id][sp][maxnc][b];
  988: 				lngth = Get_Length(bm, wgt, sp);
  989: 				Sort_Length_Weight(bm, 0, 1, sizestocknums_id, z, sp, nc_fishery, lngth, 0, wgt, cmaxsize, sample_id, &chrt_size, ofp);
  990: 				start_N += individVERTinfo[istocknums_id][sp][chrt_size][z][sample_id] * stockinfo[sstocknums_id][sp][z][sample_id] * popratio;
  991: 
  992: 				if (minnc < nagemat)
  993: 					stage = 0;
  994: 				else
  995: 					stage = 1;
  996: 				b = bm->rep_box_of_zone[sp][z][stage];
  997: 				popratio = zoneVERTpopratio[sp][minnc][min_remain][z];
  998: 				wgt = biolVERTinfo[bstruct_id][sp][minnc][b] + biolVERTinfo[bres_id][sp][minnc][b];
  999: 				lngth = Get_Length(bm, wgt, sp);
 1000: 				Sort_Length_Weight(bm, 0, 1, sizestocknums_id, z, sp, nc_fishery, lngth, 0, wgt, cmaxsize, sample_id, &chrt_size, ofp);
 1001: 				end_N += individVERTinfo[istocknums_id][sp][chrt_size][z][sample_id] * stockinfo[sstocknums_id][sp][z][sample_id] * popratio;
 1002: 				catch_N = 0;
 1003: 				all_done = 0;
 1004: 				for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
```

**Suggested fix (from review):** move the reset before the zone loop

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

## CLS-13

**loop condition** · Conf H / Sev M  
**Lines (current code):** 893  
**Lines at review (Oct 2026):** 893

**Why it's thought to be a bug**

`while (!end_N ...)`: end_N accumulates across zones, so later zones skip the search.

**Current code** (`atassess/atclassical.c` lines 891–895)

```c
  891: 		nc_remain_end = sp_AgeClassSize - 1;
  892: 		nc_end = (FunctGroupArray[sp].numCohortsXnumGenes) - 1;
  893: 		while ((!end_N) && (nc_end > nc)) {
  894: 			/* Determine maxsize (equivalent to oldest) */
  895: 			if (nc_end < nagemat)
```

**Suggested fix (from review):** per-zone variable

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

## CLS-14

**formula** · Conf M / Sev M  
**Lines (current code):** 949-951, 1025-1027  
**Lines at review (Oct 2026):** 949-951, 1025-1027

**Why it's thought to be a bug**

`totDead = start-end` already includes catch, so `start-catch-totDead` double-counts catch (log of a negative is possible), and the "M" formula is actually Z.

**Current code** (`atassess/atclassical.c` lines 947–953)

```c
  947: 		/* TODO: Check with Beth about this */
  948: 		yrs_included = 1.0;
  949: 		totDead_N = start_N - end_N;
  950: 		calcZ = (1.0 / yrs_included) * (log(start_N / (start_N - catch_N - totDead_N + small_num)));
  951: 		calcM = (1.0 / yrs_included) * (log(start_N / (start_N - totDead_N + small_num)));
  952: 		calcF = calcZ - calcM;
  953:         
```

**Suggested fix (from review):** natural deaths = start-end-catch

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

## CLS-15

**wrong variable** · Conf M / Sev M  
**Lines (current code):** 918-925, 1068, 1076-1079  
**Lines at review (Oct 2026):** 918-925, 1068, 1076-1079

**Why it's thought to be a bug**

`b` is stale per chrt; calcE uses the running cross-zone calcB; predation uses the prey's catch and clearance instead of the predator's.

**Current code** (`atassess/atclassical.c` lines 916–927)

```c
  916: 		}
  917: 
  918: 		for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
  919: 			for (ai = 0; ai < sp_AgeClassSize; ai++) {
  920: 				thisai = chrt * sp_AgeClassSize + ai;
  921: 				popratio = zoneVERTpopratio[sp][chrt][ai][z];
  922: 
  923: 				wgt = biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b];
  924: 				lngth = Get_Length(bm, wgt, sp);
  925: 				Sort_Length_Weight(bm, 0, 1, sizestocknums_id, z, sp, nc_fishery, lngth, 0, wgt, cmaxsize, sample_id, &chrt_size, ofp);
  926: 				if (thisai >= va95) {
  927: 					catch_N += individVERTinfo[icatchnums_id][sp][chrt_size][z][sample_id] * stockinfo[scatchnums_id][sp][z][sample_id] * popratio;
```

**Suggested fix (from review):** fix variables

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

## CLS-16

**wrong var / OOB** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 1145, 1186-1187  
**Lines at review (Oct 2026):** 1145, 1186-1187

**Why it's thought to be a bug**

`NumsTracers[na]` with na up to numCohorts×ageClassSize-1.

**Current code** (`atassess/atclassical.c` lines 1143–1147) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
 1143:     int va50 = (int) (FunctGroupArray[sp].speciesParams[Age50pcntV_id]);
 1144: 	int va95 = (int) (FunctGroupArray[sp].speciesParams[Age95pcntV_id]);
 1145: 	int nageclass = (int) (FunctGroupArray[sp].numCohorts * FunctGroupArray[sp].ageClassSize);
 1146: 	int nagemat = (int) (FunctGroupArray[sp].speciesParams[age_mat_id]);
 1147:     
```

**Suggested fix (from review):** `nc = floor(na/nasize)`

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

## CLS-17

**swapped args** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Lines (current code):** 1218-1219, 1555-1556  
**Lines at review (Oct 2026):** 1218-1219, 1555-1556

**Why it's thought to be a bug**

Signature is `Ecology_Spawn(..., X_RS, FSPB, log)` (atvertprocesses.c:342), but all four calls pass `scaled_FSPB[nc], X_RS[nc]`.

**Current code** (`atassess/atclassical.c` lines 1216–1221)

```c
 1216:                 
 1217:                 if (na > nagemat - 1) {
 1218:                     spawn_biom1 += num_nyr[na][est1_id] * FunctGroupArray[sp].scaled_FSPB[nc] * Ecology_Spawn(sp, KSPA_sp, FSP_sp, sp_SN, sp_RN, FunctGroupArray[sp].scaled_FSPB[nc], FunctGroupArray[sp].X_RS[nc], ofp);
 1219:                     spawn_biom2 += num_nyr[na][est2_id] * FunctGroupArray[sp].scaled_FSPB[nc] * Ecology_Spawn(sp, KSPA_sp, FSP_sp, sp_SN, sp_RN, FunctGroupArray[sp].scaled_FSPB[nc], FunctGroupArray[sp].X_RS[nc], ofp);
 1220:                 }
 1221: 				tbiom1 += num_nyr[na][est1_id] * weight;
```

**Suggested fix (from review):** swap the two arguments

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

## CLS-18

**logic** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 1242-1279, 1596-1660  
**Lines at review (Oct 2026):** 1242-1279, 1596-1660

**Why it's thought to be a bug**

BrefC branch sets `not_done = 0` and overrides BrefA's `not_done = 1`.

**Current code** (`atassess/atclassical.c` lines 1240–1279 (truncated))

```c
 1240: 		reldiff = fabs(calcB1 - BrefA) / (BrefA + small_num);
 1241: 		ratiodiff = calcB1 / (BrefA + small_num);
 1242: 		if (reldiff > bm->Assess_Btol) {
 1243: 			not_done = 1;
 1244: 			if (!tempF && (ratiodiff > 1.0)) {
 1245: 				tempF = M;
 1246: 				if (do_debug) {
 1247: 					fprintf(bm->logFile, "Time: %e %s reset tempF = M (%e)\n", bm->dayt, FunctGroupArray[sp].groupCode, M);
 1248: 				}
 1249: 			} else {
 1250: 				diff = 1.0 - log(1.0 / ratiodiff);
 1251: 				tempF *= diff;
 1252: 			}
 1253: 		} else {
 1254: 			not_done = 0;
 1255: 			diff = 1;
 1256: 		}
 1257:         
 1258: 		if (do_debug) {
 1259: 			fprintf(bm->logFile, "Time: %e %s calcB1: %e, BrefA: %e, reldiff: %e, ratiodiff: %e, diff: %e, nageclass: %d\n", bm->dayt, FunctGroupArray[sp].groupCode, calcB1, BrefA, reldiff, ratiodiff, diff, nageclass);
 1260: 		}
 1261:         
 1262: 		/* Check against desired reference level - BrefC */
 1263: 		reldiff = fabs(calcB2 - BrefC) / (BrefC + small_num);
 1264: 		ratiodiff = calcB2 / (BrefC + small_num);
 1265: 		if (reldiff > bm->Assess_Btol) {
 1266: 			not_done = 1;
 1267: 			if (!tempF2 && (ratiodiff > 1.0)) {
 1268: 				tempF2 = M;
 1269: 				if (do_debug) {
 1270: 					fprintf(bm->logFile, "Time: %e %s reset tempF2 = M (%e)\n", bm->dayt, FunctGroupArray[sp].groupCode, M);
 1271: 				}
 1272: 			} else {
 1273: 				diff = 1.0 - log(1.0 / ratiodiff);
 1274: 				tempF2 *= diff;
 1275: 			}
 1276: 		} else {
 1277: 			not_done = 0;
 1278: 			diff = 1;
 1279: 		}
```

**Suggested fix (from review):** separate doneA/doneC flags

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

## CLS-19

**numerical** · Conf M / Sev M  
**Lines (current code):** 1250-1251, 1273-1274, 1285-1293  
**Lines at review (Oct 2026):** 1250-1251, 1273-1274, 1285-1293

**Why it's thought to be a bug**

`diff = 1 - log(1/ratio)` goes negative (negative F) and is unguarded here, unlike line 1608; the NaN check covers tempF only.

**Current code** (`atassess/atclassical.c` lines 1248–1287 (truncated))

```c
 1248: 				}
 1249: 			} else {
 1250: 				diff = 1.0 - log(1.0 / ratiodiff);
 1251: 				tempF *= diff;
 1252: 			}
 1253: 		} else {
 1254: 			not_done = 0;
 1255: 			diff = 1;
 1256: 		}
 1257:         
 1258: 		if (do_debug) {
 1259: 			fprintf(bm->logFile, "Time: %e %s calcB1: %e, BrefA: %e, reldiff: %e, ratiodiff: %e, diff: %e, nageclass: %d\n", bm->dayt, FunctGroupArray[sp].groupCode, calcB1, BrefA, reldiff, ratiodiff, diff, nageclass);
 1260: 		}
 1261:         
 1262: 		/* Check against desired reference level - BrefC */
 1263: 		reldiff = fabs(calcB2 - BrefC) / (BrefC + small_num);
 1264: 		ratiodiff = calcB2 / (BrefC + small_num);
 1265: 		if (reldiff > bm->Assess_Btol) {
 1266: 			not_done = 1;
 1267: 			if (!tempF2 && (ratiodiff > 1.0)) {
 1268: 				tempF2 = M;
 1269: 				if (do_debug) {
 1270: 					fprintf(bm->logFile, "Time: %e %s reset tempF2 = M (%e)\n", bm->dayt, FunctGroupArray[sp].groupCode, M);
 1271: 				}
 1272: 			} else {
 1273: 				diff = 1.0 - log(1.0 / ratiodiff);
 1274: 				tempF2 *= diff;
 1275: 			}
 1276: 		} else {
 1277: 			not_done = 0;
 1278: 			diff = 1;
 1279: 		}
 1280:         
 1281: 		if (do_debug) {
 1282: 			fprintf(bm->logFile, "Time: %e %s calcB2: %e, BrefC: %e, reldiff: %e, ratiodiff: %e, diff: %e, nageclass: %d\n", bm->dayt, FunctGroupArray[sp].groupCode, calcB2, BrefC, reldiff, ratiodiff, diff, nageclass);
 1283: 		}
 1284:         
 1285: 		if (_isnan(tempF)) {
 1286: 			if (_isnan(tempF2)) {
 1287: 				tempF = Fcurr;
```

**Suggested fix (from review):** `max(small_num, …)`; check tempF2

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

## CLS-20

**inconsistency** · Conf L / Sev M  
**Lines (current code):** 1224-1225 vs 1116  
**Lines at review (Oct 2026):** 1224-1225 vs 1116

**Why it's thought to be a bug**

Beverton-Holt denominator uses `tbiom1` here but `spawn_biom1` at line 1116; cohort counts also differ.

**Current code** (`atassess/atclassical.c` lines 1222–1227)

```c
 1222: 				tbiom2 += num_nyr[na][est2_id] * weight;
 1223: 			}
 1224: 			num_rec1 = (0.5 * spawn_biom1 * Balpha / (Bbeta + 0.5 * tbiom1)) / (KWRR_sp + KWSR_sp);
 1225: 			num_rec2 = (0.5 * spawn_biom2 * Balpha / (Bbeta + 0.5 * tbiom2)) / (KWRR_sp + KWSR_sp);
 1226: 			num_nyr[0][est1_id] = num_rec1;
 1227: 			num_nyr[0][est2_id] = num_rec2;
```

**Suggested fix (from review):** unify

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

## CLS-21

**sort algorithm** · Conf H / Sev M  
**Lines (current code):** 1374-1386; 287-289, 510-512  
**Lines at review (Oct 2026):** 1374-1386; 287-289, 510-512

**Why it's thought to be a bug**

Rank sort gives ties the same slot, leaving stale entries; percentile index can equal nbs.

**Current code** (`atassess/atclassical.c` lines 1372–1388)

```c
 1372: 
 1373: 	/* Do the sort */
 1374: 	for (j = 0; j < nbs; j++) {
 1375: 		rank = 0;
 1376: 		for (i = 0; i < nbs; i++) {
 1377: 			if (EstToSort[j] > EstToSort[i])
 1378: 				rank++;
 1379: 		}
 1380: 		SortRank[j] = nbs - 1 - rank;
 1381: 	}
 1382: 
 1383: 	/* Save sorted results */
 1384: 	for (j = 0; j < nbs; j++) {
 1385: 		indx = SortRank[j];
 1386: 		SortedResults[indx] = EstToSort[j];
 1387: 	}
 1388: 
```

**Suggested fix (from review):** tie-break / qsort; clamp

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

## CLS-22

**log domain** · Conf M / Sev M  
**Lines (current code):** 1465-1479  
**Lines at review (Oct 2026):** 1465-1479

**Why it's thought to be a bug**

After the sanity resets, Z = log(start/small_num) is huge, or log of a negative.

**Current code** (`atassess/atclassical.c` lines 1463–1481)

```c
 1463: 		natDead_N = 0;
 1464: 
 1465: 	if (catch_N > start_N) {
 1466: 		/* Sanity check so can not record catching more than started with
 1467: 		 - do pro-rated correction and down scaling */
 1468: 		temp = catch_N;
 1469: 		catch_N = start_N * temp / (temp + natDead_N);
 1470: 		natDead_N = start_N - catch_N;
 1471: 	}
 1472: 
 1473: 	/* This should get used rarely - if not rethink this as basically capping M at about 0.8 */
 1474: 	if (start_N < natDead_N) {
 1475: 		start_N = 1.25 * natDead_N;
 1476: 		fprintf(ofp, "Time: %e had to reset start_N to 1.2 * natDead_N (so now %e)\n", bm->dayt, start_N);
 1477: 	}
 1478: 
 1479: 	calcZ = (log(start_N / (start_N - catch_N - natDead_N + small_num)));
 1480: 	calcM = (log(start_N / (start_N - natDead_N + small_num)));
 1481: 	calcF = calcZ - calcM;
```

**Suggested fix (from review):** clamp denominator

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

## CLS-23

**other** · Conf L / Sev L  
**Lines (current code):** 1493-1509  
**Lines at review (Oct 2026):** 1493-1509

**Why it's thought to be a bug**

Rescaled M is not written back to `NAssess[est_M_id]`.

**Current code** (`atassess/atclassical.c` lines 1491–1511)

```c
 1491: 	}
 1492: 
 1493: 	bm->NAssess[sp][est_M_id] = calcM;
 1494: 	bm->NAssess[sp][est_Fcurr_id] = calcF;
 1495: 	Fcurr = calcF;
 1496: 	M = calcM;
 1497: 
 1498: 	/* Sanity check on M */
 1499: 	if (M < 0.001) {
 1500: 		M *= 100.0;
 1501: 		if (M > 0.3)
 1502: 			M *= 0.5;
 1503: 		fprintf(ofp, "Time: %e rescaled %s M est to %e\n", bm->dayt, FunctGroupArray[sp].groupCode, M);
 1504: 	} else if (M < 0.01) {
 1505: 		M *= 10.0;
 1506: 		if (M > 0.3)
 1507: 			M *= 0.5;
 1508: 		fprintf(ofp, "Time: %e rescaled %s M est to %e\n", bm->dayt, FunctGroupArray[sp].groupCode, M);
 1509: 	}
 1510: 
 1511: 	/** Fishing mortality parameters **/
```

**Suggested fix (from review):** confirm intent

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

## CLS-24

**log text** · Conf H / Sev L  
**Lines (current code):** 1476, 1535-1536, 1565-1566  
**Lines at review (Oct 2026):** 1476, 1535-1536, 1565-1566

**Why it's thought to be a bug**

Message says 1.2 (code uses 1.25); `ageClassSize` is labelled nageclass; tempF is printed for est2.

**Current code** (`atassess/atclassical.c` lines 1474–1478) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
 1474: 	if (start_N < natDead_N) {
 1475: 		start_N = 1.25 * natDead_N;
 1476: 		fprintf(ofp, "Time: %e had to reset start_N to 1.2 * natDead_N (so now %e)\n", bm->dayt, start_N);
 1477: 	}
 1478: 
```

**Suggested fix (from review):** fix text

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
