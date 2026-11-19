# atassess/atsurvey.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **18 findings** (severity H: 6; confidence H and severity H: 4; on the priority list: 5).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SRV-01](#srv-01) ⭐ | 290-325 (inside bb loop 185-359) | H/H | misplaced block | Per-zone arrays are divided by numsteps once per box, so they are divided N times and results depend on box or… |  | [ ] |
| [SRV-02](#srv-02) | 192-203, 295-296 (+ atsample.c:238) | L/M | scaling | samplephys may be divided by numsteps twice. |  | [ ] |
| [SRV-03](#srv-03) ⭐ | 620, 646, 668, 688 | H/H | reused temp | `raw` is overwritten with reserve weight (line 646) and then used for catch/discard weights. |  | [ ] |
| [SRV-04](#srv-04) | 635, 667, 687, 983, 1010, 1253 | H/M | off-by-one | `while (nsample && i <= nsample)` from i=0 gives nsample+1 draws. |  | [ ] |
| [SRV-05](#srv-05) | 420-546, 1467-1473 | M/H | misplaced brace | All bird sampling is inside `if (annual_sample == 1)` (meant only for the bin init), yet the result is still d… |  | [ ] |
| [SRV-06](#srv-06) | 503-517 | M/M | loop bound | Birds fill bins 0-2 but the sum loops to `numCohortsXnumGenes` (can exceed K_num_size). |  | [ ] |
| [SRV-07](#srv-07) | 454-461, 640, 1260 | M/M | wrong variable | Age and stage computed from `chrt` (cohort×gene) rather than the base cohort. |  | [ ] |
| [SRV-08](#srv-08) | 640, 1260 | M/M | off-by-one | `round(x + 0.5)` = floor(x)+1, so age 0 is never populated. |  | [ ] |
| [SRV-09](#srv-09) | 737-738 | M/M | denominator | Per-bin mean weight divided by total numbers over all bins. |  | [ ] |
| [SRV-10](#srv-10) ⭐ | 969, 990, 1011 | H/H | reused temp | Same as SRV-03, for fish discards. |  | [ ] |
| [SRV-11](#srv-11) | 1072-1080 (+511) | M/M | double count | Bird catch numbers are added twice. |  | [ ] |
| [SRV-12](#srv-12) | 794-797 vs 819-829, 876 | L/M | condition | Reset is gated on isImpacted, accumulation on isFished. |  | [ ] |
| [SRV-13](#srv-13) ✔ ⭐ | 1123-1124 | H/H | off-by-one | `sizebins` reset `j<2` of 5 characteristics; `sizeselectnums_id` (4) grows every year and inflates catchabilit… |  | [ ] |
| [SRV-14](#srv-14) | 1210, 1220, 1223 | M/M | loop bound | Uses global max cohorts, so additive noise creates phantom fish. |  | [ ] |
| [SRV-15](#srv-15) | 1311, 1320-1323, 1360-1361, 1372-1374 | H/L | unguarded logging | Debug fprintf calls are not behind `verbose`, so log files bloat. |  | [ ] |
| [SRV-16](#srv-16) | 1393-1397 | L/L | units | matage (years) is capped at numCohorts. |  | [ ] |
| [SRV-17](#srv-17) ⭐ | 1559-1568, 1574 | M/H | random range | Upper bound is clamped but not lower, so a negative `samplec` writes out of bounds in `agebins`. |  | [ ] |
| [SRV-18](#srv-18) | 1680-1689 | L/L | log(0) | Box-Muller with U=0 is possible. |  | [ ] |

## Reviewer notes for this file

Lower-confidence notes: the survey never fills `oldbaby[..][sample_id]`, so the sampled young-of-year index is always 0. Prod/eat gated on `sample_now` are still divided by numsteps.

---

## SRV-01

**misplaced block** · Conf H / Sev H ⭐ priority  
**Listed lines:** 290-325 (inside bb loop 185-359)

**Why it's thought to be a bug**

Per-zone arrays are divided by numsteps once per box, so they are divided N times and results depend on box order.

**Current code** (`atassess/atsurvey.c` lines 288–327)

```c
  288: 		}
  289: 
  290: 		/* Correct average back to per year rather than multiple times per year */
  291: 		if (bm->annual_sample == numsteps) {
  292: 			correct_size = numsteps;
  293: 
  294: 			/* Physical characteristics */
  295: 			for (i = 0; i < num_sampled_phy_id - 2; i++)
  296: 				samplephys[i][z][sample_id] /= correct_size;
  297: 			/* Processes */
  298: 			for (i = 0; i < 3; i++)
  299: 				sampleprocess[i][z][sample_id] /= correct_size;
  300: 			/* Biomass */
  301: 			for (i = 0; i < bm->K_num_tot_sp; i++) {
  302: 				if (FunctGroupArray[i].isVertebrate == FALSE) {
  303: 					if (FunctGroupArray[i].isDetritus == TRUE) {
  304: 						detIndex = FunctGroupArray[i].detIndex;
  305: 						sampledetbiom[detIndex][WC][z][sample_id] /= correct_size;
  306: 						sampledetbiom[detIndex][SED][z][sample_id] /= correct_size;
  307: 					} else {
  308: 						samplebiom[i][z][sample_id] /= correct_size;
  309: 					}
  310: 				}
  311: 			}
  312: 			/* Production */
  313: 			for (i = 0; i < bm->K_num_tot_sp; i++) {
  314: 				if (FunctGroupArray[i].isVertebrate == FALSE && FunctGroupArray[i].isDetritus == FALSE) {
  315: 					sampleprod[i][z][sample_id] /= correct_size;
  316: 				}
  317: 			}
  318: 
  319: 			/* Consumption */
  320: 			for (i = 0; i < bm->K_num_tot_sp; i++) {
  321: 				if (FunctGroupArray[i].isVertebrate == FALSE && FunctGroupArray[i].isGrazer == TRUE) {
  322: 					sampleeat[i][z][sample_id] /= correct_size;
  323: 				}
  324: 			}
  325: 		}
  326: 		if (verbose > 2 && bm->flagecosysassess) {
  327: 			for (i = 0; i < num_sampled_phy_id - 2; i++)
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
**Listed lines:** 192-203, 295-296 (+ atsample.c:238)

**Why it's thought to be a bug**

samplephys may be divided by numsteps twice.

**Current code** (`atassess/atsurvey.c` lines 190–205)

```c
  190: 			/* Physical characteristics */
  191: 
  192: 			for (sampleIndex = 0; sampleIndex < phys_samplingsize; sampleIndex++) {
  193: 
  194: 				SamplePhysicalProperty(bm, salinity_id, bb, b, z, k_avgsalt, k_varsalt);
  195: 				SamplePhysicalProperty(bm, temperature_id, bb, b, z, k_avgsalt, k_varsalt); // Intentioanl reulst of salt properties here as treating as well known physical properties
  196: 
  197: 				for (i = light_wc_id; i < don_wc_id; i++) {
  198: 					SamplePhysicalProperty(bm, i, bb, b, z, k_avgphys, k_varphys);
  199: 				}
  200: 				for (i = don_wc_id; i < vol_id; i++) {
  201: 					SamplePhysicalProperty(bm, i, bb, b, z, k_avgnuts, k_varnuts);
  202: 				}
  203: 			}
  204: 			/* Processes */
  205: 			for (i = 0; i < 3; i++) {
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
**Listed lines:** 620, 646, 668, 688

**Why it's thought to be a bug**

`raw` is overwritten with reserve weight (line 646) and then used for catch/discard weights.

**Current code** (`atassess/atsurvey.c` lines 618–657 (truncated))

```c
  618:                          */
  619: 						/* Individual size */
  620: 						raw = biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b];
  621: 
  622: 						/* Stock*/
  623: 						sample = samplenums[chrt][samplestock_id] * totn[samplestock_id];
  624: 						samplen1 = (int) (ceil(sample));
  625: 						samplen2 = (int) floor(ROUNDGUARD + (FunctGroupArray[sp].speciesParams[samplesize_id] * samplenums[chrt][samplestock_id]));
  626: 
  627: 						if (!samplen2 && (samplen1 > 0))
  628: 							samplen2 = 1;
  629: 
  630: 						if (samplen1 < samplen2)
  631: 							nsample = samplen1;
  632: 						else
  633: 							nsample = samplen2;
  634: 						i = 0;
  635: 						while (nsample && i <= nsample) {
  636: 							wgt = Assess_Add_Error(bm, flagverts, raw, k_avgweight, k_varweight);
  637: 							lngth = Get_Length(bm, wgt, sp);
  638: 							Sort_Length_Weight(bm, 0, 1, sizestocknums_id, z, sp, fishery_id, lngth, 1, wgt, cmaxsize, sample_id, &ltc, ofp);
  639: 							//TODO Check with beth about changing to a round instead of a floor.
  640: 							cage = (int)(round((chrt + drandom(0.0, 1.0)) * FunctGroupArray[sp].ageClassSize + 0.5));
  641: 
  642: 							Length_Age_Key(bm, z, sp, cmaxsize, lngth, cage, cmaxage, 1, sample_id);
  643: 
  644: 							/* Condition */
  645: 							if (bm->sample_now) {
  646: 								raw = biolVERTinfo[bres_id][sp][chrt][b];
  647: 								sample = Assess_Add_Error(bm, flagverts, raw, k_avgweight, k_varweight);
  648: 								individVERTinfo[icondn_id][sp][ltc][z][sample_id] += (sample / (wgt + TINY)) * samplearea[bb] / (nsample + TINY);
  649: 							}
  650: 
  651: 							i++;
  652: 						}
  653: 
  654: 						/* Catch */
  655: 						sample = samplenums[chrt][samplecatch_id] * totn[samplecatch_id];
  656: 						samplen1 = (int) (ceil(sample));
  657: 						samplen2 = (int) floor(ROUNDGUARD + (FunctGroupArray[sp].speciesParams[samplesize_id] * samplenums[chrt][samplecatch_id]));
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
**Listed lines:** 635, 667, 687, 983, 1010, 1253

**Why it's thought to be a bug**

`while (nsample && i <= nsample)` from i=0 gives nsample+1 draws.

**Current code** (`atassess/atsurvey.c` lines 633–672 (truncated))

```c
  633: 							nsample = samplen2;
  634: 						i = 0;
  635: 						while (nsample && i <= nsample) {
  636: 							wgt = Assess_Add_Error(bm, flagverts, raw, k_avgweight, k_varweight);
  637: 							lngth = Get_Length(bm, wgt, sp);
  638: 							Sort_Length_Weight(bm, 0, 1, sizestocknums_id, z, sp, fishery_id, lngth, 1, wgt, cmaxsize, sample_id, &ltc, ofp);
  639: 							//TODO Check with beth about changing to a round instead of a floor.
  640: 							cage = (int)(round((chrt + drandom(0.0, 1.0)) * FunctGroupArray[sp].ageClassSize + 0.5));
  641: 
  642: 							Length_Age_Key(bm, z, sp, cmaxsize, lngth, cage, cmaxage, 1, sample_id);
  643: 
  644: 							/* Condition */
  645: 							if (bm->sample_now) {
  646: 								raw = biolVERTinfo[bres_id][sp][chrt][b];
  647: 								sample = Assess_Add_Error(bm, flagverts, raw, k_avgweight, k_varweight);
  648: 								individVERTinfo[icondn_id][sp][ltc][z][sample_id] += (sample / (wgt + TINY)) * samplearea[bb] / (nsample + TINY);
  649: 							}
  650: 
  651: 							i++;
  652: 						}
  653: 
  654: 						/* Catch */
  655: 						sample = samplenums[chrt][samplecatch_id] * totn[samplecatch_id];
  656: 						samplen1 = (int) (ceil(sample));
  657: 						samplen2 = (int) floor(ROUNDGUARD + (FunctGroupArray[sp].speciesParams[samplesize_id] * samplenums[chrt][samplecatch_id]));
  658: 
  659: 						if (!samplen2 && (samplen1 > 0))
  660: 							samplen2 = 1;
  661: 
  662: 						if (samplen1 < samplen2)
  663: 							nsample = samplen1;
  664: 						else
  665: 							nsample = samplen2;
  666: 						i = 0;
  667: 						while (nsample && i <= nsample) {
  668: 							wgt = Assess_Add_Error(bm, flagverts, raw, k_avgweight, k_varweight);
  669: 							lngth = Get_Length(bm, wgt, sp);
  670: 							Sort_Length_Weight(bm, 0, 1, sizecatchnums_id, z, sp, fishery_id, lngth, 1, wgt, cmaxsize, sample_id, &ltc, ofp);
  671: 							i++;
  672: 						}
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
**Listed lines:** 420-546, 1467-1473

**Why it's thought to be a bug**

All bird sampling is inside `if (annual_sample == 1)` (meant only for the bin init), yet the result is still divided by numsteps.

**Current code** (`atassess/atsurvey.c` lines 418–452 (range truncated))

```c
  418: 
  419: 	/* Initialise the bins */
  420: 	if (bm->annual_sample == 1) {
  421: 		for (i = 0; i < bm->K_num_size; i++)
  422: 			for (b = 0; b < bm->nfzones; b++)
  423: 				for (k = 0; k < bm->K_num_tot_sp; k++) {
  424: 					if ((FunctGroupArray[k].groupType == BIRD) || (FunctGroupArray[k].groupType == MAMMAL)) {
  425: 						for (j = 0; j < 5; j++)
  426: 							sizebins[j][k][b][i][0][sample_id] = 0.0;
  427: 					}
  428: 				}
  429: 
  430: 		for (k = 0; k < bm->K_num_tot_sp; k++) {
  431: 			if ((FunctGroupArray[k].groupType == BIRD) || (FunctGroupArray[k].groupType == MAMMAL)) {
  432: 				for (b = 0; b < bm->nfzones; b++)
  433: 					for (j = 0; j < bm->K_max_agekey; j++)
  434: 						for (i = 0; i < bm->K_num_size; i++)
  435: 							agebins[k][b][j][i][sample_id] = 0.0;
  436: 			}
  437: 		}
  438: 
  439: 		/* Bird information */
  440: 		if (verbose)
  441: 			fprintf(stderr, "Sampling birds\n");
  442: 
  443: 		for (b = 0; b < bm->nbox; b++) {
  444: 			if (bm->boxes[b].type != BOUNDARY) {
  445: 				z = bm->boxes[b].zone - 1;
  446: 				vol = phys[vol_id][b];
  447: 
  448: 				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  449: 					if (FunctGroupArray[sp].isVertebrate == TRUE) {
  450: 						if(FunctGroupArray[sp].groupType == BIRD){
  451: 							for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
  452: 
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
**Listed lines:** 503-517

**Why it's thought to be a bug**

Birds fill bins 0-2 but the sum loops to `numCohortsXnumGenes` (can exceed K_num_size).

**Current code** (`atassess/atsurvey.c` lines 501–519)

```c
  501: 				if (FunctGroupArray[sp].isVertebrate == TRUE) {
  502: 					if(FunctGroupArray[sp].groupType == BIRD){
  503: 						for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
  504: 							/* Bird stock biomass */
  505: 							stockinfo[sbiomass_id][sp][z][sample_id] += individVERTinfo[ibiomass_id][sp][chrt][z][sample_id];
  506: 
  507: 							/* Bird stock numbers */
  508: 							stockinfo[sstocknums_id][sp][z][sample_id] += individVERTinfo[istocknums_id][sp][chrt][z][sample_id];
  509: 
  510: 							/* Bird catch numbers */
  511: 							stockinfo[scatchnums_id][sp][z][sample_id] += individVERTinfo[icatchnums_id][sp][chrt][z][sample_id];
  512: 						}
  513: 						for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
  514: 							/* Normalise individual bird numbers */
  515: 							individVERTinfo[istocknums_id][sp][chrt][z][sample_id] /= (stockinfo[sstocknums_id][sp][z][sample_id] + TINY);
  516: 							individVERTinfo[icatchnums_id][sp][chrt][z][sample_id] /= (stockinfo[scatchnums_id][sp][z][sample_id] + TINY);
  517: 						}
  518: 					}
  519: 				}
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
**Listed lines:** 454-461, 640, 1260

**Why it's thought to be a bug**

Age and stage computed from `chrt` (cohort×gene) rather than the base cohort.

**Current code** (`atassess/atsurvey.c` lines 452–463)

```c
  452: 
  453: 								/* Get information for nestlings, newly fledged and adult seabirds */
  454: 								switch (chrt) {
  455: 								case 0: /* Nestlings */
  456: 								case 1: /* Fledglings */
  457: 									lbin = chrt;
  458: 									break;
  459: 								default: /* All adults together */
  460: 									lbin = 2;
  461: 									break;
  462: 								}
  463: 
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
**Listed lines:** 640, 1260

**Why it's thought to be a bug**

`round(x + 0.5)` = floor(x)+1, so age 0 is never populated.

**Current code** (`atassess/atsurvey.c` lines 638–642)

```c
  638: 							Sort_Length_Weight(bm, 0, 1, sizestocknums_id, z, sp, fishery_id, lngth, 1, wgt, cmaxsize, sample_id, &ltc, ofp);
  639: 							//TODO Check with beth about changing to a round instead of a floor.
  640: 							cage = (int)(round((chrt + drandom(0.0, 1.0)) * FunctGroupArray[sp].ageClassSize + 0.5));
  641: 
  642: 							Length_Age_Key(bm, z, sp, cmaxsize, lngth, cage, cmaxage, 1, sample_id);
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
**Listed lines:** 737-738

**Why it's thought to be a bug**

Per-bin mean weight divided by total numbers over all bins.

**Current code** (`atassess/atsurvey.c` lines 735–740)

```c
  735: 
  736: 							/* Average weight of individual in that size bin */
  737: 							individVERTinfo[iweight_id][sp][i][z][sample_id] = sizebins[sizeweight_id][sp][z][i][0][sample_id]
  738: 									/ (totnums[totstocknums_id][sp][z] + totnums[totcatchnums_id][sp][z] + totnums[totdiscardnums_id][sp][z] + TINY);
  739: 
  740: 							/* Biomass */
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
**Listed lines:** 969, 990, 1011

**Why it's thought to be a bug**

Same as SRV-03, for fish discards.

**Current code** (`atassess/atsurvey.c` lines 967–1006 (truncated))

```c
  967:                          */
  968:                         
  969: 						rawwgt = biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b];
  970: 						sample = samplenums[chrt][samplecatch_id] * totn[samplecatch_id];
  971: 						samplen1 = (int) (ceil(sample));
  972: 						samplen2 = (int) floor(ROUNDGUARD + (FunctGroupArray[sp].speciesParams[samplesize_id] * samplenums[chrt][samplecatch_id]));
  973: 
  974: 						if (!samplen2 && (samplen1 > 0))
  975: 							samplen2 = 1;
  976: 
  977: 						if (samplen1 < samplen2)
  978: 							nsample = samplen1;
  979: 						else
  980: 							nsample = samplen2;
  981: 						i = 0;
  982: 
  983: 						while (nsample && i <= nsample) {
  984: 							wgt = Assess_Add_Error(bm, flagverts, rawwgt, k_avgweight, k_varweight);
  985: 							lngth = Get_Length(bm, wgt, sp);
  986: 							Sort_Length_Weight(bm, 1, 1, sizecatchnums_id, z, sp, fishery_id, lngth, 1, wgt, cmaxsize, sample_id, &ltc, ofp);
  987: 
  988: 							/* Condition */
  989: 							if (bm->sample_now) {
  990: 								rawwgt = biolVERTinfo[bres_id][sp][chrt][b];
  991: 								sample = Assess_Add_Error(bm, flagverts, rawwgt, k_avgweight, k_varweight);
  992: 								individVERTinfo[icondn_id][sp][ltc][z][sample_id] += (sample / (wgt + TINY)) * samplearea[bb] / (nsample + TINY);
  993: 							}
  994: 
  995: 							i++;
  996: 						}
  997: 
  998: 						sample = samplenums[chrt][samplediscard_id] * totn[samplediscard_id];
  999: 						samplen1 = (int) (ceil(sample));
 1000: 						samplen2 = (int) floor(ROUNDGUARD + (FunctGroupArray[sp].speciesParams[samplesize_id] * samplenums[chrt][samplediscard_id]));
 1001: 
 1002: 						if (!samplen2 && (samplen1 > 0))
 1003: 							samplen2 = 1;
 1004: 
 1005: 						if (samplen1 < samplen2)
 1006: 							nsample = samplen1;
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
**Listed lines:** 1072-1080 (+511)

**Why it's thought to be a bug**

Bird catch numbers are added twice.

**Current code** (`atassess/atsurvey.c` lines 1070–1082)

```c
 1070: 						}
 1071: 					} else {
 1072: 						for (i = 0; i < 3; i++) {
 1073: 							/* Total number of seabirds caught */
 1074: 							stockinfo[scatchnums_id][sp][z][sample_id] += stockinfo[stotcatch_id][sp][z][sample_id]
 1075: 									* individVERTinfo[icatchnums_id][sp][i][z][sample_id] / (individVERTinfo[iweight_id][sp][i][z][sample_id] + TINY);
 1076: 
 1077: 							/* Total number of seabirds in discards */
 1078: 							stockinfo[sdiscardnums_id][sp][z][sample_id] += stockinfo[sdiscards_id][sp][z][sample_id]
 1079: 									* individVERTinfo[idiscards_id][sp][i][z][sample_id] / (individVERTinfo[iweight_id][sp][i][z][sample_id] + TINY);
 1080: 						}
 1081: 					}
 1082: 				}
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
**Listed lines:** 794-797 vs 819-829, 876

**Why it's thought to be a bug**

Reset is gated on isImpacted, accumulation on isFished.

**Current code** (`atassess/atsurvey.c` lines 792–799)

```c
  792: 		for (b = 0; b < bm->nbox; b++) {
  793: 			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  794: 				if (FunctGroupArray[sp].isImpacted == TRUE) {
  795: 					for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  796: 						for (k = 0; k < 3; k++)
  797: 							fishery[k][nf][sp][b][sample_id] = 0.0;
  798: 					}
  799: 				}
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
**Listed lines:** 1123-1124

**Why it's thought to be a bug**

`sizebins` reset `j<2` of 5 characteristics; `sizeselectnums_id` (4) grows every year and inflates catchability. atattribute.c:807 uses `j<5`.

**Current code** (`atassess/atsurvey.c` lines 1121–1126)

```c
 1121: 					if (FunctGroupArray[k].isVertebrate == TRUE) {
 1122: 						if ((FunctGroupArray[k].groupType != BIRD) && (FunctGroupArray[k].groupType != MAMMAL)) {
 1123: 							for (j = 0; j < 2; j++)
 1124: 								sizebins[j][k][b][i][0][sample_id] = 0.0;
 1125: 						}
 1126: 					}
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
**Listed lines:** 1210, 1220, 1223

**Why it's thought to be a bug**

Uses global max cohorts, so additive noise creates phantom fish.

**Current code** (`atassess/atsurvey.c` lines 1208–1225)

```c
 1208: 						totn[chrt] = 0.0;
 1209: 
 1210: 					for (chrt = 0; chrt < bm->K_num_max_cohort * bm->K_num_max_genetypes; chrt++) {
 1211: 						rawn = biolVERTinfo[bstocknums_id][sp][chrt][b];
 1212: 						sample = Assess_Add_Error(bm, flagcount, rawn, k_avgcount, k_varcount);
 1213: 						samplenums[chrt][samplestock_id] = ceil(sample);
 1214: 						totn[samplestock_id] += samplenums[chrt][samplestock_id];
 1215: 
 1216: 						/* Collect assessment data */
 1217: 						bm->CatchRecord[Yr][sp][chrt][survey_id] += sample;
 1218: 
 1219: 					}
 1220: 					for (chrt = 0; chrt < bm->K_num_max_cohort * bm->K_num_max_genetypes; chrt++)
 1221: 						samplenums[chrt][samplestock_id] /= (totn[samplestock_id] + TINY);
 1222: 
 1223: 					for (chrt = 0; chrt < bm->K_num_max_cohort * bm->K_num_max_genetypes; chrt++) {
 1224:                         
 1225:                         /*
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
**Listed lines:** 1311, 1320-1323, 1360-1361, 1372-1374

**Why it's thought to be a bug**

Debug fprintf calls are not behind `verbose`, so log files bloat.

**Current code** (`atassess/atsurvey.c` lines 1284–1323 (truncated)) — code appears to have moved -25 lines since the review

```c
 1284: 	}
 1285: 
 1286: 	if (verbose)
 1287: 		fprintf(stderr, "Update fish stock and individ arrays\n");
 1288: 
 1289: 	/* Scaling size structure per zone so it sums to one - only do this annually (i.e. once sampling complete) */
 1290: 	if (bm->annual_sample == numsteps) {
 1291: 		for (z = 0; z < bm->nfzones; z++) {
 1292: 			for (b = 0; b < bm->K_num_tot_sp; b++) {
 1293: 				if (FunctGroupArray[b].isVertebrate == TRUE) {
 1294: 					if ((FunctGroupArray[b].groupType != BIRD) && (FunctGroupArray[b].groupType != MAMMAL)) {
 1295: 						totnums[totstocknums_id][b][z] = 0.0;
 1296: 						for (i = 0; i < bm->K_num_size; i++) {
 1297: 							totnums[totstocknums_id][b][z] += sizebins[sizestocknums_id][b][z][i][0][sample_id];
 1298: 						}
 1299: 					}
 1300: 				}
 1301: 			}
 1302: 			/* Assign final values */
 1303: 			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 1304: 				if (FunctGroupArray[sp].isVertebrate == TRUE) {
 1305: 					if ((FunctGroupArray[sp].groupType != BIRD) && (FunctGroupArray[sp].groupType != MAMMAL)) {
 1306: 						/* Stock biomass */
 1307: 						stockinfo[sbiomass_id][sp][z][sample_id] = biom[sp][z][sample_id] / 2.0;
 1308: 
 1309: 						for (i = 0; i < bm->K_num_size; i++) {
 1310: 
 1311: 							fprintf(bm->logFile, "nsq[sp][z][i][sample_id] = %d\n", nsq[sp][z][i][sample_id]);
 1312: 							/* Catchability */
 1313: 							individVERTinfo[iq_id][sp][i][z][sample_id] = sizebins[sizeselectnums_id][sp][z][i][0][sample_id] / (nsq[sp][z][i][sample_id]
 1314: 									+ TINY);
 1315: 
 1316: 							/* Size distribution of stock - correcting for selectivity */
 1317: 							individVERTinfo[istocknums_id][sp][i][z][sample_id] = (sizebins[sizestocknums_id][sp][z][i][0][sample_id]
 1318: 									/ (totnums[totstocknums_id][sp][z] + TINY)) / (individVERTinfo[iq_id][sp][i][z][sample_id] + TINY);
 1319: 
 1320: 							fprintf(ofp, "Time: %e, %s-%d, z: %d, individVERTinfo: %e, sizebin; %e, totnums: %e, iq: %e\n", bm->dayt,
 1321: 									FunctGroupArray[sp].groupCode, i, z, individVERTinfo[istocknums_id][sp][i][z][sample_id],
 1322: 									sizebins[sizestocknums_id][sp][z][i][0][sample_id], totnums[totstocknums_id][sp][z],
 1323: 									individVERTinfo[iq_id][sp][i][z][sample_id]);
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
**Listed lines:** 1393-1397

**Why it's thought to be a bug**

matage (years) is capped at numCohorts.

**Current code** (`atassess/atsurvey.c` lines 1391–1399)

```c
 1391: 						/* Get data */
 1392: 						/* Note: Using year classes so use agemat * age classsize */
 1393: 						matage = (int) (FunctGroupArray[sp].speciesParams[age_mat_id] * FunctGroupArray[sp].ageClassSize);
 1394: 						if (matage > bm->K_max_agekey)
 1395: 							matage = bm->K_max_agekey;
 1396: 						if (matage > FunctGroupArray[sp].numCohorts)
 1397: 							matage = FunctGroupArray[sp].numCohorts;
 1398: 
 1399: 						for (chrt = 0; chrt < matage; chrt++) {
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
**Listed lines:** 1559-1568, 1574

**Why it's thought to be a bug**

Upper bound is clamped but not lower, so a negative `samplec` writes out of bounds in `agebins`.

**Current code** (`atassess/atsurvey.c` lines 1557–1576)

```c
 1557: 	double val, sample;
 1558: 
 1559: 	sample = Assess_Add_Error(bm, flagage, c, k_avgage, k_varage);
 1560: 	samplec = (int) (sample);
 1561: 	if (samplec > cmax)
 1562: 		samplec = cmax;
 1563: 
 1564: 	k = 0;
 1565: 	while ((chk == 0) && (k < ni)) {
 1566: 		val = FunctGroupArray[i].speciesParams[allometic_bin_start_id] + (FunctGroupArray[i].speciesParams[allometic_bin_size_id] * (k + 1));
 1567: 		if (l <= val) {
 1568: 			agebins[i][z][samplec][k][id] += n;
 1569: 			chk = 1;
 1570: 		}
 1571: 		k++;
 1572: 	}
 1573: 	if (chk == 0) {
 1574: 		agebins[i][z][samplec][ni][id] += n;
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
**Listed lines:** 1680-1689

**Why it's thought to be a bug**

Box-Muller with U=0 is possible.

**Current code** (`atassess/atsurvey.c` lines 1678–1691)

```c
 1678: 		break;
 1679: 	case normal_err: /* Normal */
 1680: 		step1 = drandom(0.0, 1.0);
 1681: 		step2 = drandom(0.0, 1.0);
 1682: 		step3 = sqrt(-2.0 * log(step1)) * cos(2.0 * 3.1415926 * step2);
 1683: 		ans = step3 * sqrt(v) + true_val * a;
 1684: 		break;
 1685: 	case lognorm_err: /* Lognormal - note - v/2 is the bias correction term
 1686: 	 needed when generating lognormal by exponentiating a standard normal */
 1687: 		step1 = drandom(0.0, 1.0);
 1688: 		step2 = drandom(0.0, 1.0);
 1689: 		step3 = sqrt(-2.0 * log(step1)) * cos(2.0 * 3.1415926 * step2);
 1690: 		ans = exp(step3 * sqrt(v) - v / 2.0) * (true_val * a);
 1691: 		break;
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
