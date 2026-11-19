# atassess/atclassical.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **24 findings** (severity H: 8; confidence H and severity H: 7; on the priority list: 8).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [CLS-01](#cls-01) ⭐ | 217, 445 | H/H | off-by-one / algorithm | Starting simplex has only Ndim rows; row 0 is unscaled and the last parameter (Ndim-1) is never perturbed, so … |  | [ ] |
| [CLS-02](#cls-02) | 167 | H/M | uninitialised | With Pella-Tomlinson, Ndim=4 but only X[0..2] are reset; X[3] is stale from the previous fit. |  | [ ] |
| [CLS-03](#cls-03) | 274 | H/M | off-by-one | `ipnt = floor(icnt*U + 1.0)` gives 1..icnt, so BootResu[0] is never used and BootResu[icnt] is stale. |  | [ ] |
| [CLS-04](#cls-04) | 294 | H/M | off-by-one | Failure test reads `[nbs+1]` = `[bootstrap+1]` (never written). The sort takes slots 0..bootstrap-1, which inc… |  | [ ] |
| [CLS-05](#cls-05) ⭐ | 233 | M/H | see SOL-1 | Takes `P[Ilow]` which, because of the Amoeba swap bug, is not the best vertex. |  | [ ] |
| [CLS-06](#cls-06) | 231 | M/L | off-by-one | `nfunk <= Assess_Max_Int` treats hitting the iteration limit as success. |  | [ ] |
| [CLS-07](#cls-07) | 561 | H/M | misplaced block | `if (!assess_failed)` save sits inside the cohort loop, so partial sums are stored if a later cohort fails. |  | [ ] |
| [CLS-08](#cls-08) ✔ ⭐ | 668 | H/H | off-by-one | `z < nfzones+1` over `stockinfo`, `individVERTinfo` and `agelengthkey`, which all have zone dimension `nfzones… |  | [ ] |
| [CLS-09](#cls-09) | 723 | M/M | wrong variable | `numCohorts` used where tracer arrays need `numCohortsXnumGenes`. |  | [ ] |
| [CLS-10](#cls-10) | 855, 878 | M/M | unit mismatch | After line 855, `nagemat` is in years but is compared with cohort indices. |  | [ ] |
| [CLS-11](#cls-11) ⭐ | 1079 | H/H | wrong var / OOB | `nagemat` (years) used as a cohort index into `scaled_C`, `scaled_mum`, `biolVERTinfo`; e.g. 15 > 10 cohorts. |  | [ ] |
| [CLS-12](#cls-12) ⭐ | 1002 | H/H | accumulator reset | `catch_N = 0` inside the zone loop, while start_N/end_N sum across all zones. |  | [ ] |
| [CLS-13](#cls-13) | 893 | H/M | loop condition | `while (!end_N ...)`: end_N accumulates across zones, so later zones skip the search. |  | [ ] |
| [CLS-14](#cls-14) | 1177 | M/M | formula | `totDead = start-end` already includes catch, so `start-catch-totDead` double-counts catch (log of a negative … |  | [ ] |
| [CLS-15](#cls-15) | 826 | M/M | wrong variable | `b` is stale per chrt; calcE uses the running cross-zone calcB; predation uses the prey's catch and clearance … |  | [ ] |
| [CLS-16](#cls-16) ⭐ | 1187 | H/H | wrong var / OOB | `NumsTracers[na]` with na up to numCohorts×ageClassSize-1. |  | [ ] |
| [CLS-17](#cls-17) ✔ ⭐ | 1218 | H/H | swapped args | Signature is `Ecology_Spawn(..., X_RS, FSPB, log)` (atvertprocesses.c:342), but all four calls pass `scaled_FS… |  | [ ] |
| [CLS-18](#cls-18) ⭐ | 1254, 1243 | H/H | logic | BrefC branch sets `not_done = 0` and overrides BrefA's `not_done = 1`. |  | [ ] |
| [CLS-19](#cls-19) | 1608 | M/M | numerical | `diff = 1 - log(1/ratio)` goes negative (negative F) and is unguarded here, unlike line 1608; the NaN check co… |  | [ ] |
| [CLS-20](#cls-20) | 1224 | L/M | inconsistency | Beverton-Holt denominator uses `tbiom1` here but `spawn_biom1` at line 1116; cohort counts also differ. |  | [ ] |
| [CLS-21](#cls-21) | 1374 | H/M | sort algorithm | Rank sort gives ties the same slot, leaving stale entries; percentile index can equal nbs. |  | [ ] |
| [CLS-22](#cls-22) | 1479 | M/M | log domain | After the sanity resets, Z = log(start/small_num) is huge, or log of a negative. |  | [ ] |
| [CLS-23](#cls-23) | 1493 | L/L | other | Rescaled M is not written back to `NAssess[est_M_id]`. |  | [ ] |
| [CLS-24](#cls-24) | 1536 | H/L | log text | Message says 1.2 (code uses 1.25); `ageClassSize` is labelled nageclass; tempF is printed for est2. |  | [ ] |

## Reviewer notes for this file

Lower-confidence notes: `va95` is treated as a cohort at line 860 but as years at 926/1209. `CPUEtrend[sp][ny]` can overflow if pseudo-assessment runs from tstart. VPA "biomass" sums NEst, which may be numbers.

---

## CLS-01

**off-by-one / algorithm** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atclassical.c` 217, `atclassical.c` 445
**Lines at review (Oct 2026):** 213-219, 441-447

**Why it's thought to be a bug**

Starting simplex has only Ndim rows; row 0 is unscaled and the last parameter (Ndim-1) is never perturbed, so Amoeba can't move it. See SOL-2.

**Code at issue** — `atclassical.c` line 217 (the line the review pointed to, found by its text)  
Search for: `P[i][j] *= GRD;`

```c
  215: 				P[i][j] = X[j];
  216: 				if (i - 1 == j)
  217→ 					P[i][j] *= GRD;
  218: 			}
  219: 		}
```

**Code at issue** — `atclassical.c` line 445 (the line the review pointed to, found by its text)  
Search for: `P[i][j] *= GRD;`

```c
  443: 				P[i][j] = X[j];
  444: 				if (i - 1 == j)
  445→ 					P[i][j] *= GRD;
  446: 			}
  447: 		}
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
**Found at (current code):** `atclassical.c` 167
**Lines at review (Oct 2026):** 166-169, 207-209

**Why it's thought to be a bug**

With Pella-Tomlinson, Ndim=4 but only X[0..2] are reset; X[3] is stale from the previous fit.

**Code at issue** — `atclassical.c` line 167 (the line the review pointed to, found by its text)  
Search for: `Ndim = est_p_id + 1;`

```c
  165: 	/* Number of dimensions in data arrays to be solved */
  166: 	if ((int) (prm_sp)) {
  167→ 		Ndim = est_p_id + 1;
  168: 	} else
  169: 		Ndim = est_B0_id + 1;
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
**Found at (current code):** `atclassical.c` 274
**Lines at review (Oct 2026):** 274, 498

**Why it's thought to be a bug**

`ipnt = floor(icnt*U + 1.0)` gives 1..icnt, so BootResu[0] is never used and BootResu[icnt] is stale.

**Code at issue** — `atclassical.c` line 274 (closest match to the quoted code; wording differs slightly)  
Search for: `ipnt = (int) floor(ROUNDGUARD + (icnt * drandom(0.0, 1.0) + 1.0));`

```c
  272: 			ITData[Yr] = 0;
  273: 			yr_effort = bm->EffortRecord[Yr][sp][dataid];
  274→ 			ipnt = (int) floor(ROUNDGUARD + (icnt * drandom(0.0, 1.0) + 1.0));
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
**Found at (current code):** `atclassical.c` 294
**Lines at review (Oct 2026):** 294-297, 524-527

**Why it's thought to be a bug**

Failure test reads `[nbs+1]` = `[bootstrap+1]` (never written). The sort takes slots 0..bootstrap-1, which includes the real-data fit and excludes the last bootstrap. MAXDOUBLE failures are not excluded.

**Code at issue** — `atclassical.c` line 294  
Search for: `[nbs + 1]`

```c
  292: 		/* Bootstraps to chose between */
  293: 		for (i = 0; i < est_msy_id; i++) {
  294→ 			if ((NResult[YrMax][i][nbs + 1] < MAXDOUBLE) && ((i == est_r_id) || (i == est_K_id) || (i == est_B_id))) {
  295: 				for (j = 0; j < bootstrap; j++) {
  296: 					ResultToSort[j] = NResult[YrMax][i][j];
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
**Found at (current code):** `atclassical.c` 233
**Lines at review (Oct 2026):** 228-233, 461

**Why it's thought to be a bug**

Takes `P[Ilow]` which, because of the Amoeba swap bug, is not the best vertex.

**Code at issue** — `atclassical.c` line 233  
Search for: `P[Ilow]`

```c
  231: 		if (nfunk <= bm->Assess_Max_Int) {
  232: 			for (j = 0; j < Ndim; j++) {
  233→ 				X[j] = P[Ilow][j];
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
**Found at (current code):** `atclassical.c` 231
**Lines at review (Oct 2026):** 231, 459

**Why it's thought to be a bug**

`nfunk <= Assess_Max_Int` treats hitting the iteration limit as success.

**Code at issue** — `atclassical.c` line 231  
Search for: `nfunk <= bm->Assess_Max_Int`

```c
  229: 				prm_sp, &nfunk, &Ilow, ofp, NULL);
  230: 
  231→ 		if (nfunk <= bm->Assess_Max_Int) {
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
**Found at (current code):** `atclassical.c` 561
**Lines at review (Oct 2026):** 521-574

**Why it's thought to be a bug**

`if (!assess_failed)` save sits inside the cohort loop, so partial sums are stored if a later cohort fails.

**Code at issue** — `atclassical.c` line 561  
Search for: `if (!assess_failed)`

```c
  559: 
  560: 			/* If assessment valid save biomasses */
  561→ 			if (!assess_failed) {
  562: 				/* Median */
  563: 				bm->NAssess[sp][est_med_stock_id] = med_biom;
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
**Found at (current code):** `atclassical.c` 668
**Lines at review (Oct 2026):** 668-680

**Why it's thought to be a bug**

`z < nfzones+1` over `stockinfo`, `individVERTinfo` and `agelengthkey`, which all have zone dimension `nfzones` (atsamplesetup.c:213, 223, 226).

**Code at issue** — `atclassical.c` line 668  
Search for: `z < bm->nfzones + 1`

```c
  666:     
  667:     /** Find new recruitment and biomass estimates **/
  668→ 	for (z = 0; z < bm->nfzones + 1; z++) {
  669:         
  670: 		/** Get biomass estimate **/
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
**Found at (current code):** `atclassical.c` 723
**Lines at review (Oct 2026):** 723, 738-741, 793

**Why it's thought to be a bug**

`numCohorts` used where tracer arrays need `numCohortsXnumGenes`.

**Code at issue** — `atclassical.c` line 723 (the line the review pointed to, found by its text)  
Search for: `int Ncohorts = FunctGroupArray[sp].numCohorts;`

```c
  721: 	int b, nc, k, sn, rn, den;
  722: 	double med_recruit, bot_recruit, top_recruit, med_biom, bot_biom, top_biom, biom_yoy, counter, calcF, calcM;
  723→     int Ncohorts = FunctGroupArray[sp].numCohorts;
  724:     int maxstock = FunctGroupArray[sp].numStocks;
  725:     
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
**Found at (current code):** `atclassical.c` 855, `atclassical.c` 878
**Lines at review (Oct 2026):** 855, 878, 895, 980, 992, 1045

**Why it's thought to be a bug**

After line 855, `nagemat` is in years but is compared with cohort indices.

**Code at issue** — `atclassical.c` line 855 (the line the review pointed to, found by its text)  
Search for: `nagemat *= sp_AgeClassSize;`

```c
  853:     */
  854:     
  855→ 	nagemat *= sp_AgeClassSize;
  856: 
  857: 	/** Natural mortality (M) and current fishing mortality (Fcurr) estimation **/
```

**Code at issue** — `atclassical.c` line 878 (the line the review pointed to, found by its text)  
Search for: `if (nc < nagemat)`

```c
  876: 	max_yrs_included = 0;
  877: 	for (z = 0; z < bm->nfzones; z++) {
  878→ 		if (nc < nagemat)
  879: 			stage = 0;
  880: 		else
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
**Found at (current code):** `atclassical.c` 1079
**Lines at review (Oct 2026):** 1042, 1050-1051, 1079-1080

**Why it's thought to be a bug**

`nagemat` (years) used as a cohort index into `scaled_C`, `scaled_mum`, `biolVERTinfo`; e.g. 15 > 10 cohorts.

**Code at issue** — `atclassical.c` line 1079 (the line the review pointed to, found by its text)  
Search for: `clear = FunctGroupArray[sp].scaled_C[nagemat] * numsec / (1.0 + step1 * E1_sp * FunctGroup`

```c
 1077: 						//	clear = bm->C_spVERT[sp][nagemat] / (1.0 + step1 * E1_sp * bm->C_spVERT[sp][nagemat] / bm->mum_spVERT[sp][nagemat]);
 1078: 						/* Don't worry about the numsec in the later part of the equation as they will cancel out */
 1079→ 						clear = FunctGroupArray[sp].scaled_C[nagemat] * numsec / (1.0 + step1 * E1_sp * FunctGroupArray[sp].scaled_C[nagemat]
 1080: 								/ FunctGroupArray[sp].scaled_mum[nagemat]);
 1081: 						calcE += clear * step1;
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
**Found at (current code):** `atclassical.c` 1002
**Lines at review (Oct 2026):** 972, 1002

**Why it's thought to be a bug**

`catch_N = 0` inside the zone loop, while start_N/end_N sum across all zones.

**Code at issue** — `atclassical.c` line 1002  
Search for: `catch_N = 0`

```c
 1000: 				Sort_Length_Weight(bm, 0, 1, sizestocknums_id, z, sp, nc_fishery, lngth, 0, wgt, cmaxsize, sample_id, &chrt_size, ofp);
 1001: 				end_N += individVERTinfo[istocknums_id][sp][chrt_size][z][sample_id] * stockinfo[sstocknums_id][sp][z][sample_id] * popratio;
 1002→ 				catch_N = 0;
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
**Found at (current code):** `atclassical.c` 893
**Lines at review (Oct 2026):** 893

**Why it's thought to be a bug**

`while (!end_N ...)`: end_N accumulates across zones, so later zones skip the search.

**Code at issue** — `atclassical.c` line 893 (the line the review pointed to, found by its text)  
Search for: `while ((!end_N) && (nc_end > nc)) {`

```c
  891: 		nc_remain_end = sp_AgeClassSize - 1;
  892: 		nc_end = (FunctGroupArray[sp].numCohortsXnumGenes) - 1;
  893→ 		while ((!end_N) && (nc_end > nc)) {
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
**Found at (current code):** `atclassical.c` 1177
**Lines at review (Oct 2026):** 949-951, 1025-1027

**Why it's thought to be a bug**

`totDead = start-end` already includes catch, so `start-catch-totDead` double-counts catch (log of a negative is possible), and the "M" formula is actually Z.

**Code at issue** — `atclassical.c` line 1177 (located by name only; check this is the right place)  
Search for: `end up with desired Fref points */`

```c
 1175: 	while (not_done) {
 1176: 		/* Run model forward to see what biomass is reached with current F and M values and then scale F until
 1177→ 		 end up with desired Fref points */
 1178: 
 1179:         if(!typecall) {
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
**Found at (current code):** `atclassical.c` 826
**Lines at review (Oct 2026):** 918-925, 1068, 1076-1079

**Why it's thought to be a bug**

`b` is stale per chrt; calcE uses the running cross-zone calcB; predation uses the prey's catch and clearance instead of the predator's.

**Code at issue** — `atclassical.c` line 826 (located by name only; check this is the right place)  
Search for: `popratio, totDead_N, calcE, calcB, calcC, step1, lngth_end, max_yrs_included, wgt, wgt_end`

```c
  824:     double calcM, calcF, calcZ, spawn_biom1, weight, M, num_rec1, calcB1, minend_N, maxend_N, lngth,
  825:     clear, this_N, E1_sp = 0, start_N, end_N, catch_N,
  826→     popratio, totDead_N, calcE, calcB, calcC, step1, lngth_end, max_yrs_included, wgt, wgt_end, yrs_included;
  827: 	int chrt, z, ai, na, nc, nyr, nc_remain, minnc, min_remain, maxnc, max_remain, thisai, oldest_age,
  828:     b, youngest_age, testai, all_done, pred, nc_end, chrt_size, end_nc, nc_remain_end, size_nc, size_nc_end, size_nc_start, stage;
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
**Found at (current code):** `atclassical.c` 1187
**Lines at review (Oct 2026):** 1145, 1186-1187

**Why it's thought to be a bug**

`NumsTracers[na]` with na up to numCohorts×ageClassSize-1.

**Code at issue** — `atclassical.c` line 1187  
Search for: `NumsTracers[na]`

```c
 1185: 		} else {
 1186:             for (na = 0; na < nageclass; na++) {
 1187→                 den = FunctGroupArray[sp].NumsTracers[na];
 1188:                 this_num = 0;
 1189:                 
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
**Found at (current code):** `atclassical.c` 1218
**Lines at review (Oct 2026):** 1218-1219, 1555-1556

**Why it's thought to be a bug**

Signature is `Ecology_Spawn(..., X_RS, FSPB, log)` (atvertprocesses.c:342), but all four calls pass `scaled_FSPB[nc], X_RS[nc]`.

**Code at issue** — `atclassical.c` line 1218  
Search for: `Ecology_Spawn(`

```c
 1216:                 
 1217:                 if (na > nagemat - 1) {
 1218→                     spawn_biom1 += num_nyr[na][est1_id] * FunctGroupArray[sp].scaled_FSPB[nc] * Ecology_Spawn(sp, KSPA_sp, FSP_sp, sp_SN, sp_RN, FunctGroupArray[sp].scaled_FSPB[nc], FunctGroupArray[sp].X_RS[nc], ofp);
 1219:                     spawn_biom2 += num_nyr[na][est2_id] * FunctGroupArray[sp].scaled_FSPB[nc] * Ecology_Spawn(sp, KSPA_sp, FSP_sp, sp_SN, sp_RN, FunctGroupArray[sp].scaled_FSPB[nc], FunctGroupArray[sp].X_RS[nc], ofp);
 1220:                 }
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
**Found at (current code):** `atclassical.c` 1254, `atclassical.c` 1243
**Lines at review (Oct 2026):** 1242-1279, 1596-1660

**Why it's thought to be a bug**

BrefC branch sets `not_done = 0` and overrides BrefA's `not_done = 1`.

**Code at issue** — `atclassical.c` line 1254  
Search for: `not_done = 0`

```c
 1252: 			}
 1253: 		} else {
 1254→ 			not_done = 0;
 1255: 			diff = 1;
 1256: 		}
```

**Code at issue** — `atclassical.c` line 1243  
Search for: `not_done = 1`

```c
 1241: 		ratiodiff = calcB1 / (BrefA + small_num);
 1242: 		if (reldiff > bm->Assess_Btol) {
 1243→ 			not_done = 1;
 1244: 			if (!tempF && (ratiodiff > 1.0)) {
 1245: 				tempF = M;
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
**Found at (current code):** `atclassical.c` 1608
**Lines at review (Oct 2026):** 1250-1251, 1273-1274, 1285-1293

**Why it's thought to be a bug**

`diff = 1 - log(1/ratio)` goes negative (negative F) and is unguarded here, unlike line 1608; the NaN check covers tempF only.

**Code at issue** — `atclassical.c` line 1608 (located by name only; check this is the right place)  
Search for: `diff = max(small_num,1.0 - log(1 / ratiodiff));`

```c
 1606: 					}
 1607: 				} else {
 1608→ 					diff = max(small_num,1.0 - log(1 / ratiodiff));
 1609: 					if ((diff == small_num) && (prevR > 1.0) && (ratiodiff < 1.0)) {
 1610: 						avgF = (prevF / prevR + tempF / (ratiodiff + small_num)) / 2.0;
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
**Found at (current code):** `atclassical.c` 1224
**Lines at review (Oct 2026):** 1224-1225 vs 1116

**Why it's thought to be a bug**

Beverton-Holt denominator uses `tbiom1` here but `spawn_biom1` at line 1116; cohort counts also differ.

**Code at issue** — `atclassical.c` line 1224 (the line the review pointed to, found by its text)  
Search for: `num_rec1 = (0.5 * spawn_biom1 * Balpha / (Bbeta + 0.5 * tbiom1)) / (KWRR_sp + KWSR_sp);`

```c
 1222: 				tbiom2 += num_nyr[na][est2_id] * weight;
 1223: 			}
 1224→ 			num_rec1 = (0.5 * spawn_biom1 * Balpha / (Bbeta + 0.5 * tbiom1)) / (KWRR_sp + KWSR_sp);
 1225: 			num_rec2 = (0.5 * spawn_biom2 * Balpha / (Bbeta + 0.5 * tbiom2)) / (KWRR_sp + KWSR_sp);
 1226: 			num_nyr[0][est1_id] = num_rec1;
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
**Found at (current code):** `atclassical.c` 1374
**Lines at review (Oct 2026):** 1374-1386; 287-289, 510-512

**Why it's thought to be a bug**

Rank sort gives ties the same slot, leaving stale entries; percentile index can equal nbs.

**Code at issue** — `atclassical.c` line 1374 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `for (j = 0; j < nbs; j++) {`

```c
 1372: 
 1373: 	/* Do the sort */
 1374→ 	for (j = 0; j < nbs; j++) {
 1375: 		rank = 0;
 1376: 		for (i = 0; i < nbs; i++) {
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
**Found at (current code):** `atclassical.c` 1479
**Lines at review (Oct 2026):** 1465-1479

**Why it's thought to be a bug**

After the sanity resets, Z = log(start/small_num) is huge, or log of a negative.

**Code at issue** — `atclassical.c` line 1479 (located by name only; check this is the right place)  
Search for: `calcZ = (log(start_N / (start_N - catch_N - natDead_N + small_num)));`

```c
 1477: 	}
 1478: 
 1479→ 	calcZ = (log(start_N / (start_N - catch_N - natDead_N + small_num)));
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
**Found at (current code):** `atclassical.c` 1493
**Lines at review (Oct 2026):** 1493-1509

**Why it's thought to be a bug**

Rescaled M is not written back to `NAssess[est_M_id]`.

**Code at issue** — `atclassical.c` line 1493 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->NAssess[sp][est_M_id] = calcM;`

```c
 1491: 	}
 1492: 
 1493→ 	bm->NAssess[sp][est_M_id] = calcM;
 1494: 	bm->NAssess[sp][est_Fcurr_id] = calcF;
 1495: 	Fcurr = calcF;
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
**Found at (current code):** `atclassical.c` 1536
**Lines at review (Oct 2026):** 1476, 1535-1536, 1565-1566

**Why it's thought to be a bug**

Message says 1.2 (code uses 1.25); `ageClassSize` is labelled nageclass; tempF is printed for est2.

**Code at issue** — `atclassical.c` line 1536 (the line the review pointed to, found by its text)  
Search for: `FunctGroupArray[sp].ageClassSize, num_nyr[na][est2_id]);`

```c
 1534: 				if (do_debug) {
 1535: 					fprintf(ofp, "Time: %e %s-%d (nc: %d, nageclass: %d) initDEN: %e\n", bm->dayt, FunctGroupArray[sp].groupCode, na, nc,
 1536→ 							FunctGroupArray[sp].ageClassSize, num_nyr[na][est2_id]);
 1537: 				}
 1538: 
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
