# atassess/atsurvey.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **18 findings** (severity H: 6; confidence H and severity H: 4; on the priority list: 5).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SRV-01](#srv-01) ⭐ | 291 | H/H | misplaced block | Per-zone arrays are divided by numsteps once per box, so they are divided N times and results depend on box or… |  | [ ] |
| [SRV-02](#srv-02) | 192 | L/M | scaling | samplephys may be divided by numsteps twice. |  | [ ] |
| [SRV-03](#srv-03) ⭐ | 620, 646 | H/H | reused temp | `raw` is overwritten with reserve weight (line 646) and then used for catch/discard weights. |  | [ ] |
| [SRV-04](#srv-04) | 635 | H/M | off-by-one | `while (nsample && i <= nsample)` from i=0 gives nsample+1 draws. |  | [ ] |
| [SRV-05](#srv-05) | 420 | M/H | misplaced brace | All bird sampling is inside `if (annual_sample == 1)` (meant only for the bin init), yet the result is still d… |  | [ ] |
| [SRV-06](#srv-06) | 503 | M/M | loop bound | Birds fill bins 0-2 but the sum loops to `numCohortsXnumGenes` (can exceed K_num_size). |  | [ ] |
| [SRV-07](#srv-07) | 560 | M/M | wrong variable | Age and stage computed from `chrt` (cohort×gene) rather than the base cohort. |  | [ ] |
| [SRV-08](#srv-08) | 640, 1260 | M/M | off-by-one | `round(x + 0.5)` = floor(x)+1, so age 0 is never populated. |  | [ ] |
| [SRV-09](#srv-09) | 737 | M/M | denominator | Per-bin mean weight divided by total numbers over all bins. |  | [ ] |
| [SRV-10](#srv-10) ⭐ | 969 | H/H | reused temp | Same as SRV-03, for fish discards. |  | [ ] |
| [SRV-11](#srv-11) | 1072 | M/M | double count | Bird catch numbers are added twice. |  | [ ] |
| [SRV-12](#srv-12) | 836 | L/M | condition | Reset is gated on isImpacted, accumulation on isFished. |  | [ ] |
| [SRV-13](#srv-13) ✔ ⭐ | 1313 | H/H | off-by-one | `sizebins` reset `j<2` of 5 characteristics; `sizeselectnums_id` (4) grows every year and inflates catchabilit… |  | [ ] |
| [SRV-14](#srv-14) | 1210 | M/M | loop bound | Uses global max cohorts, so additive noise creates phantom fish. |  | [ ] |
| [SRV-15](#srv-15) | 1286 | H/L | unguarded logging | Debug fprintf calls are not behind `verbose`, so log files bloat. |  | [ ] |
| [SRV-16](#srv-16) | 1396 | L/L | units | matage (years) is capped at numCohorts. |  | [ ] |
| [SRV-17](#srv-17) ⭐ | 1574 | M/H | random range | Upper bound is clamped but not lower, so a negative `samplec` writes out of bounds in `agebins`. |  | [ ] |
| [SRV-18](#srv-18) | 1680 | L/L | log(0) | Box-Muller with U=0 is possible. |  | [ ] |

## Reviewer notes for this file

Lower-confidence notes: the survey never fills `oldbaby[..][sample_id]`, so the sampled young-of-year index is always 0. Prod/eat gated on `sample_now` are still divided by numsteps.

---

## SRV-01

**misplaced block** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atsurvey.c` 291
**Lines at review (Oct 2026):** 290-325 (inside bb loop 185-359)

**Why it's thought to be a bug**

Per-zone arrays are divided by numsteps once per box, so they are divided N times and results depend on box order.

**Code at issue** — `atsurvey.c` line 291 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `if (bm->annual_sample == numsteps) {`

```c
  289: 
  290: 		/* Correct average back to per year rather than multiple times per year */
  291→ 		if (bm->annual_sample == numsteps) {
  292: 			correct_size = numsteps;
  293: 
```

**Suggested fix (from review):** move after the box loop

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

## SRV-02

**scaling** · Conf L / Sev M  
**Found at (current code):** `atsurvey.c` 192
**Lines at review (Oct 2026):** 192-203, 295-296 (+ atsample.c:238)

**Why it's thought to be a bug**

samplephys may be divided by numsteps twice.

**Code at issue** — `atsurvey.c` line 192 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `for (sampleIndex = 0; sampleIndex < phys_samplingsize; sampleIndex++) {`

```c
  190: 			/* Physical characteristics */
  191: 
  192→ 			for (sampleIndex = 0; sampleIndex < phys_samplingsize; sampleIndex++) {
  193: 
  194: 				SamplePhysicalProperty(bm, salinity_id, bb, b, z, k_avgsalt, k_varsalt);
```

**Suggested fix (from review):** normalise once

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

## SRV-03

**reused temp** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atsurvey.c` 620, `atsurvey.c` 646
**Lines at review (Oct 2026):** 620, 646, 668, 688

**Why it's thought to be a bug**

`raw` is overwritten with reserve weight (line 646) and then used for catch/discard weights.

**Code at issue** — `atsurvey.c` line 620 (the line the review pointed to, found by its text)  
Search for: `raw = biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b];`

```c
  618:                          */
  619: 						/* Individual size */
  620→ 						raw = biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b];
  621: 
  622: 						/* Stock*/
```

**Code at issue** — `atsurvey.c` line 646 (the line the review pointed to, found by its text)  
Search for: `raw = biolVERTinfo[bres_id][sp][chrt][b];`

```c
  644: 							/* Condition */
  645: 							if (bm->sample_now) {
  646→ 								raw = biolVERTinfo[bres_id][sp][chrt][b];
  647: 								sample = Assess_Add_Error(bm, flagverts, raw, k_avgweight, k_varweight);
  648: 								individVERTinfo[icondn_id][sp][ltc][z][sample_id] += (sample / (wgt + TINY)) * samplearea[bb] / (nsample + TINY);
```

**Suggested fix (from review):** separate variable

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

## SRV-04

**off-by-one** · Conf H / Sev M  
**Found at (current code):** `atsurvey.c` 635
**Lines at review (Oct 2026):** 635, 667, 687, 983, 1010, 1253

**Why it's thought to be a bug**

`while (nsample && i <= nsample)` from i=0 gives nsample+1 draws.

**Code at issue** — `atsurvey.c` line 635  
Search for: `while (nsample && i <= nsample)`

```c
  633: 							nsample = samplen2;
  634: 						i = 0;
  635→ 						while (nsample && i <= nsample) {
  636: 							wgt = Assess_Add_Error(bm, flagverts, raw, k_avgweight, k_varweight);
  637: 							lngth = Get_Length(bm, wgt, sp);
```

**Suggested fix (from review):** `i < nsample`

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

## SRV-05

**misplaced brace** · Conf M / Sev H  
**Found at (current code):** `atsurvey.c` 420
**Lines at review (Oct 2026):** 420-546, 1467-1473

**Why it's thought to be a bug**

All bird sampling is inside `if (annual_sample == 1)` (meant only for the bin init), yet the result is still divided by numsteps.

**Code at issue** — `atsurvey.c` line 420  
Search for: `if (bm->annual_sample == 1)`

```c
  418: 
  419: 	/* Initialise the bins */
  420→ 	if (bm->annual_sample == 1) {
  421: 		for (i = 0; i < bm->K_num_size; i++)
  422: 			for (b = 0; b < bm->nfzones; b++)
```

**Suggested fix (from review):** close the block after line 437

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

## SRV-06

**loop bound** · Conf M / Sev M  
**Found at (current code):** `atsurvey.c` 503
**Lines at review (Oct 2026):** 503-517

**Why it's thought to be a bug**

Birds fill bins 0-2 but the sum loops to `numCohortsXnumGenes` (can exceed K_num_size).

**Code at issue** — `atsurvey.c` line 503 (the line the review pointed to, found by its text)  
Search for: `for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {`

```c
  501: 				if (FunctGroupArray[sp].isVertebrate == TRUE) {
  502: 					if(FunctGroupArray[sp].groupType == BIRD){
  503→ 						for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
  504: 							/* Bird stock biomass */
  505: 							stockinfo[sbiomass_id][sp][z][sample_id] += individVERTinfo[ibiomass_id][sp][chrt][z][sample_id];
```

**Suggested fix (from review):** loop 0..2

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

## SRV-07

**wrong variable** · Conf M / Sev M  
**Found at (current code):** `atsurvey.c` 560
**Lines at review (Oct 2026):** 454-461, 640, 1260

**Why it's thought to be a bug**

Age and stage computed from `chrt` (cohort×gene) rather than the base cohort.

**Code at issue** — `atsurvey.c` line 560 (located by name only; check this is the right place)  
Search for: `for (chrt = 0; chrt < FunctGroupArray[j].numCohorts * FunctGroupArray[j].numGeneTypes; chr`

```c
  558: 				if (FunctGroupArray[j].isVertebrate == TRUE) {
  559: 					if (FunctGroupArray[j].groupType == MAMMAL) {
  560→ 						for (chrt = 0; chrt < FunctGroupArray[j].numCohorts * FunctGroupArray[j].numGeneTypes; chrt++) {
  561: 							raw = biolVERTinfo[bstocknums_id][j][chrt][b];
  562: 							sample = Assess_Add_Error(bm, flagcount, raw, k_avgcount, k_varcount);
```

**Suggested fix (from review):** `chrt/numGeneTypes`

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

## SRV-08

**off-by-one** · Conf M / Sev M  
**Found at (current code):** `atsurvey.c` 640, `atsurvey.c` 1260
**Lines at review (Oct 2026):** 640, 1260

**Why it's thought to be a bug**

`round(x + 0.5)` = floor(x)+1, so age 0 is never populated.

**Code at issue** — `atsurvey.c` line 640 (the line the review pointed to, found by its text)  
Search for: `cage = (int)(round((chrt + drandom(0.0, 1.0)) * FunctGroupArray[sp].ageClassSize + 0.5));`

```c
  638: 							Sort_Length_Weight(bm, 0, 1, sizestocknums_id, z, sp, fishery_id, lngth, 1, wgt, cmaxsize, sample_id, &ltc, ofp);
  639: 							//TODO Check with beth about changing to a round instead of a floor.
  640→ 							cage = (int)(round((chrt + drandom(0.0, 1.0)) * FunctGroupArray[sp].ageClassSize + 0.5));
  641: 
  642: 							Length_Age_Key(bm, z, sp, cmaxsize, lngth, cage, cmaxage, 1, sample_id);
```

**Code at issue** — `atsurvey.c` line 1260 (the line the review pointed to, found by its text)  
Search for: `cage = (int) (round((chrt +  drandom(0.0, 1.0)) * FunctGroupArray[sp].ageClassSize + 0.5))`

```c
 1258: 
 1259: 							//TODO Check with beth about changing to a round instead of a floor.
 1260→ 							cage = (int) (round((chrt +  drandom(0.0, 1.0)) * FunctGroupArray[sp].ageClassSize + 0.5));
 1261: 
 1262: 							Length_Age_Key(bm, z, sp, cmaxsize, lngth, cage, cmaxage, 1, sample_id);
```

**Suggested fix (from review):** `floor(x)`

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

## SRV-09

**denominator** · Conf M / Sev M  
**Found at (current code):** `atsurvey.c` 737
**Lines at review (Oct 2026):** 737-738

**Why it's thought to be a bug**

Per-bin mean weight divided by total numbers over all bins.

**Code at issue** — `atsurvey.c` line 737 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `individVERTinfo[iweight_id][sp][i][z][sample_id] = sizebins[sizeweight_id][sp][z][i][0][sa`

```c
  735: 
  736: 							/* Average weight of individual in that size bin */
  737→ 							individVERTinfo[iweight_id][sp][i][z][sample_id] = sizebins[sizeweight_id][sp][z][i][0][sample_id]
  738: 									/ (totnums[totstocknums_id][sp][z] + totnums[totcatchnums_id][sp][z] + totnums[totdiscardnums_id][sp][z] + TINY);
  739: 
```

**Suggested fix (from review):** per-bin count

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

## SRV-10

**reused temp** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atsurvey.c` 969
**Lines at review (Oct 2026):** 969, 990, 1011

**Why it's thought to be a bug**

Same as SRV-03, for fish discards.

**Code at issue** — `atsurvey.c` line 969 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `rawwgt = biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b];`

```c
  967:                          */
  968:                         
  969→ 						rawwgt = biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b];
  970: 						sample = samplenums[chrt][samplecatch_id] * totn[samplecatch_id];
  971: 						samplen1 = (int) (ceil(sample));
```

**Suggested fix (from review):** separate variable

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

## SRV-11

**double count** · Conf M / Sev M  
**Found at (current code):** `atsurvey.c` 1072
**Lines at review (Oct 2026):** 1072-1080 (+511)

**Why it's thought to be a bug**

Bird catch numbers are added twice.

**Code at issue** — `atsurvey.c` line 1072 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `for (i = 0; i < 3; i++) {`

```c
 1070: 						}
 1071: 					} else {
 1072→ 						for (i = 0; i < 3; i++) {
 1073: 							/* Total number of seabirds caught */
 1074: 							stockinfo[scatchnums_id][sp][z][sample_id] += stockinfo[stotcatch_id][sp][z][sample_id]
```

**Suggested fix (from review):** one source

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

## SRV-12

**condition** · Conf L / Sev M  
**Found at (current code):** `atsurvey.c` 836
**Lines at review (Oct 2026):** 794-797 vs 819-829, 876

**Why it's thought to be a bug**

Reset is gated on isImpacted, accumulation on isFished.

**Code at issue** — `atsurvey.c` line 836 (located by name only; check this is the right place)  
Search for: `if (FunctGroupArray[sp].isFished == FALSE && FunctGroupArray[sp].isImpacted == TRUE) {`

```c
  834: 
  835: 				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  836→ 					if (FunctGroupArray[sp].isFished == FALSE && FunctGroupArray[sp].isImpacted == TRUE) {
  837: 
  838: 						//TODO: check with Beth if this should only collect states for commercial fisheries.
```

**Suggested fix (from review):** unify conditions

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

## SRV-13

**off-by-one** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Found at (current code):** `atsurvey.c` 1313
**Lines at review (Oct 2026):** 1123-1124

**Why it's thought to be a bug**

`sizebins` reset `j<2` of 5 characteristics; `sizeselectnums_id` (4) grows every year and inflates catchability. atattribute.c:807 uses `j<5`.

**Code at issue** — `atsurvey.c` line 1313 (located by name only; check this is the right place)  
Search for: `individVERTinfo[iq_id][sp][i][z][sample_id] = sizebins[sizeselectnums_id][sp][z][i][0][sam`

```c
 1311: 							fprintf(bm->logFile, "nsq[sp][z][i][sample_id] = %d\n", nsq[sp][z][i][sample_id]);
 1312: 							/* Catchability */
 1313→ 							individVERTinfo[iq_id][sp][i][z][sample_id] = sizebins[sizeselectnums_id][sp][z][i][0][sample_id] / (nsq[sp][z][i][sample_id]
 1314: 									+ TINY);
 1315: 
```

**Suggested fix (from review):** `j < 5`

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

## SRV-14

**loop bound** · Conf M / Sev M  
**Found at (current code):** `atsurvey.c` 1210
**Lines at review (Oct 2026):** 1210, 1220, 1223

**Why it's thought to be a bug**

Uses global max cohorts, so additive noise creates phantom fish.

**Code at issue** — `atsurvey.c` line 1210 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `for (chrt = 0; chrt < bm->K_num_max_cohort * bm->K_num_max_genetypes; chrt++) {`

```c
 1208: 						totn[chrt] = 0.0;
 1209: 
 1210→ 					for (chrt = 0; chrt < bm->K_num_max_cohort * bm->K_num_max_genetypes; chrt++) {
 1211: 						rawn = biolVERTinfo[bstocknums_id][sp][chrt][b];
 1212: 						sample = Assess_Add_Error(bm, flagcount, rawn, k_avgcount, k_varcount);
```

**Suggested fix (from review):** per-species count

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

## SRV-15

**unguarded logging** · Conf H / Sev L  
**Found at (current code):** `atsurvey.c` 1286
**Lines at review (Oct 2026):** 1311, 1320-1323, 1360-1361, 1372-1374

**Why it's thought to be a bug**

Debug fprintf calls are not behind `verbose`, so log files bloat.

**Code at issue** — `atsurvey.c` line 1286 (located by name only; check this is the right place)  
Search for: `if (verbose)`

```c
 1284: 	}
 1285: 
 1286→ 	if (verbose)
 1287: 		fprintf(stderr, "Update fish stock and individ arrays\n");
 1288: 
```

**Suggested fix (from review):** wrap in `verbose`

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

## SRV-16

**units** · Conf L / Sev L  
**Found at (current code):** `atsurvey.c` 1396
**Lines at review (Oct 2026):** 1393-1397

**Why it's thought to be a bug**

matage (years) is capped at numCohorts.

**Code at issue** — `atsurvey.c` line 1396 (located by name only; check this is the right place)  
Search for: `if (matage > FunctGroupArray[sp].numCohorts)`

```c
 1394: 						if (matage > bm->K_max_agekey)
 1395: 							matage = bm->K_max_agekey;
 1396→ 						if (matage > FunctGroupArray[sp].numCohorts)
 1397: 							matage = FunctGroupArray[sp].numCohorts;
 1398: 
```

**Suggested fix (from review):** cap at numCohorts×ageClassSize

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

## SRV-17

**random range** · Conf M / Sev H ⭐ priority  
**Found at (current code):** `atsurvey.c` 1574
**Lines at review (Oct 2026):** 1559-1568, 1574

**Why it's thought to be a bug**

Upper bound is clamped but not lower, so a negative `samplec` writes out of bounds in `agebins`.

**Code at issue** — `atsurvey.c` line 1574 (the line the review pointed to, found by its text)  
Search for: `agebins[i][z][samplec][ni][id] += n;`

```c
 1572: 	}
 1573: 	if (chk == 0) {
 1574→ 		agebins[i][z][samplec][ni][id] += n;
 1575: 	}
 1576: }
```

**Suggested fix (from review):** `if (samplec < 0) samplec = 0;`

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

## SRV-18

**log(0)** · Conf L / Sev L  
**Found at (current code):** `atsurvey.c` 1680
**Lines at review (Oct 2026):** 1680-1689

**Why it's thought to be a bug**

Box-Muller with U=0 is possible.

**Code at issue** — `atsurvey.c` line 1680 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `step1 = drandom(0.0, 1.0);`

```c
 1678: 		break;
 1679: 	case normal_err: /* Normal */
 1680→ 		step1 = drandom(0.0, 1.0);
 1681: 		step2 = drandom(0.0, 1.0);
 1682: 		step3 = sqrt(-2.0 * log(step1)) * cos(2.0 * 3.1415926 * step2);
```

**Suggested fix (from review):** redraw / drandom(TINY,1)

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
