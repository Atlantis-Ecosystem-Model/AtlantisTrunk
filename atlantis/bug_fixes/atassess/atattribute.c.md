# atassess/atattribute.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **16 findings** (severity H: 2; confidence H and severity H: 0; on the priority list: 1).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [ATT-01](#att-01) | 166, 188, 390, 1011-1013 | M/M | averaging | `oldbaby` is assigned with `=` each step but divided by numsteps; birds are double-added at line 390. |  | [ ] |
| [ATT-02](#att-02) | 153, 220 | H/M | wrong variable | Time column `nt = dt/86400` (step length) instead of year (`dayt/365`, as in atsample.c:59). |  | [ ] |
| [ATT-03](#att-03) | 245-289 | M/H | missing normalisation | Invertebrate true values are never divided by numsteps (survey divides them). |  | [ ] |
| [ATT-04](#att-04) | 246-249 | M/L | range | Loop `light_wc_id..<don_wc_id` skips DON, DIN and Temp. |  | [ ] |
| [ATT-05](#att-05) | 268-270 | M/M | wrong slot | Reads `biolbiom[..][WC]` only; sediment groups are stored in SED (see DAT-1). |  | [ ] |
| [ATT-06](#att-06) | 394-395 | M/M | wrong index | Bird catch numbers always go into bin 0, not `lbin`. |  | [ ] |
| [ATT-07](#att-07) | 370-377, 469, 858 | M/M | wrong variable | Stage switch and age use `chrt` (cohort×gene). |  | [ ] |
| [ATT-08](#att-08) | 469, 858 | H/M | off-by-one | `round(x+0.5)` shifts age up by 1 (same as SRV-08). |  | [ ] |
| [ATT-09](#att-09) | 460, 852 | L/L | off-by-one | `ceil(ageClassSize+0.5)` gives one too many sub-ages; the remainder is dropped. |  | [ ] |
| [ATT-10](#att-10) | 383-402, 418-435 | M/M | accumulator mixing | Bird raw counts are added onto already-normalised fractions in later rounds. |  | [ ] |
| [ATT-11](#att-11) | 490, 505, 869 (+atsurvey 535/699) | M/M | formula | Production multiplied by stock numbers again (already included in atdatareading.c:536). |  | [ ] |
| [ATT-12](#att-12) | 500-512 vs 485, 866 | L/M | inconsistency | Bird prod/eat are not gated by `sample_now`. |  | [ ] |
| [ATT-13](#att-13) ⭐ | 705 (+atsurvey.c:600) | M/H | units | `bdiscards` holds numbers (atdatareading.c:575) but is divided by weight, so it is ≈0. |  | [ ] |
| [ATT-14](#att-14) | 756-765 (+429) | M/M | double count | Bird catch numbers are added twice. |  | [ ] |
| [ATT-15](#att-15) | 727, 756 | M/L | condition | `else` also catches invertebrates. |  | [ ] |
| [ATT-16](#att-16) | 934-935 | M/M | formula | Biomass per bin = total biomass × individual weight (mass²). |  | [ ] |

---

## ATT-01

**averaging** · Conf M / Sev M  
**Lines (current code):** 166, 188, 390, 1011-1013  
**Lines at review (Oct 2026):** 166, 188, 390, 1011-1013

**Why it's thought to be a bug**

`oldbaby` is assigned with `=` each step but divided by numsteps; birds are double-added at line 390.

**Current code** (`atassess/atattribute.c` lines 164–190)

```c
  164: 				if (FunctGroupArray[sp].groupType == BIRD) {
  165: 					/* Seabirds and penguins */
  166: 					oldbaby[sp][b][attrib_id] = stockinfo[sstocknums_id][sp][b][attrib_id] * individVERTinfo[istocknums_id][sp][0][b][attrib_id];
  167: 				}
  168: 				if (FunctGroupArray[sp].groupType == MAMMAL) {
  169: 					/* Mammals */
  170: 					step1 = minpool;
  171: 					/* Note: Using year classes so use agemat * age classsize */
  172: 					matage = (int) (FunctGroupArray[sp].speciesParams[age_mat_id] * FunctGroupArray[sp].ageClassSize);
  173: 					if (matage > bm->K_max_agekey)
  174: 						matage = bm->K_max_agekey;
  175: 
  176: 					if (matage > FunctGroupArray[sp].numCohorts)
  177: 						matage = FunctGroupArray[sp].numCohorts;
  178: 
  179: 					for (jj = 0; jj < matage; jj++) {
  180: 						scalarsum = 0;
  181: 						for (k = 0; k < bm->K_num_size; k++) {
  182: 							newnums = stockinfo[sstocknums_id][sp][b][attrib_id] * individVERTinfo[istocknums_id][sp][k][b][attrib_id]
  183: 									* agelengthkey[k][jj][sp][b][attrib_id];
  184: 							scalarsum += individVERTinfo[istocknums_id][sp][k][b][attrib_id] * agelengthkey[k][jj][sp][b][attrib_id];
  185: 							step1 += newnums;
  186: 						}
  187: 					}
  188: 					oldbaby[sp][b][attrib_id] = step1;
  189: 				}
  190: 			}
```

**Suggested fix (from review):** no division / accumulate

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

## ATT-02

**wrong variable** · Conf H / Sev M  
**Lines (current code):** 153, 220  
**Lines at review (Oct 2026):** 153, 220

**Why it's thought to be a bug**

Time column `nt = dt/86400` (step length) instead of year (`dayt/365`, as in atsample.c:59).

**Current code** (`atassess/atattribute.c` lines 151–155) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
  151: 	double step1, newnums, scalarsum;
  152: 
  153: 	nt = (int) (bm->dt / 86400.0);
  154: 
  155: 	if (verbose)
```

**Suggested fix (from review):** `floor(ROUNDGUARD + dayt/365)`

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

## ATT-03

**missing normalisation** · Conf M / Sev H  
**Lines (current code):** 245-289  
**Lines at review (Oct 2026):** 245-289

**Why it's thought to be a bug**

Invertebrate true values are never divided by numsteps (survey divides them).

**Current code** (`atassess/atattribute.c` lines 243–282 (truncated))

```c
  243: 		/* Physical characteristics */
  244: 		sample = phys[salinity_id][b];
  245: 		samplephys[0][z][attrib_id] += sample * samplearea[bb];
  246: 		for (i = light_wc_id; i < don_wc_id; i++) {
  247: 			sample = phys[i][b];
  248: 			samplephys[i - 2][z][attrib_id] += sample * samplearea[bb];
  249: 		}
  250: 		/* Processes */
  251: 		for (i = 0; i < 3; i++) {
  252: 			sample = physprocess[i][b];
  253: 			sampleprocess[i][z][attrib_id] += sample * samplearea[bb];
  254: 		}
  255: 		/* Biomass */
  256: 		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  257: 			if (FunctGroupArray[sp].isVertebrate == FALSE) {
  258: 
  259: 				if (FunctGroupArray[sp].isDetritus == TRUE) {
  260: 					detIndex = FunctGroupArray[sp].detIndex;
  261: 
  262: 					sample = biolbiom[sp][b][WC];
  263: 					sampledetbiom[detIndex][WC][z][attrib_id] += sample * samplearea[bb];
  264: 
  265: 					sample = biolbiom[sp][b][SED];
  266: 					sampledetbiom[detIndex][SED][z][attrib_id] += sample * samplearea[bb];
  267: 
  268: 				} else {
  269: 					sample = biolbiom[sp][b][WC];
  270: 					samplebiom[sp][z][attrib_id] += sample * samplearea[bb];
  271: 				}
  272: 			}
  273: 		}
  274: 
  275: 		/* Production */
  276: 		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  277: 			if (FunctGroupArray[sp].isVertebrate == FALSE && FunctGroupArray[sp].isDetritus == FALSE) {
  278: 				sample = biolprod[sp][b];
  279: 				sampleprod[sp][z][attrib_id] += sample * samplearea[bb];
  280: 			}
  281: 		}
  282: 
```

**Suggested fix (from review):** add the year-end division

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

## ATT-04

**range** · Conf M / Sev L  
**Lines (current code):** 246-249  
**Lines at review (Oct 2026):** 246-249

**Why it's thought to be a bug**

Loop `light_wc_id..<don_wc_id` skips DON, DIN and Temp.

**Current code** (`atassess/atattribute.c` lines 244–251)

```c
  244: 		sample = phys[salinity_id][b];
  245: 		samplephys[0][z][attrib_id] += sample * samplearea[bb];
  246: 		for (i = light_wc_id; i < don_wc_id; i++) {
  247: 			sample = phys[i][b];
  248: 			samplephys[i - 2][z][attrib_id] += sample * samplearea[bb];
  249: 		}
  250: 		/* Processes */
  251: 		for (i = 0; i < 3; i++) {
```

**Suggested fix (from review):** include them

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

## ATT-05

**wrong slot** · Conf M / Sev M  
**Lines (current code):** 268-270  
**Lines at review (Oct 2026):** 268-270

**Why it's thought to be a bug**

Reads `biolbiom[..][WC]` only; sediment groups are stored in SED (see DAT-1).

**Current code** (`atassess/atattribute.c` lines 266–272)

```c
  266: 					sampledetbiom[detIndex][SED][z][attrib_id] += sample * samplearea[bb];
  267: 
  268: 				} else {
  269: 					sample = biolbiom[sp][b][WC];
  270: 					samplebiom[sp][z][attrib_id] += sample * samplearea[bb];
  271: 				}
  272: 			}
```

**Suggested fix (from review):** sum WC+SED

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

## ATT-06

**wrong index** · Conf M / Sev M  
**Lines (current code):** 394-395  
**Lines at review (Oct 2026):** 394-395

**Why it's thought to be a bug**

Bird catch numbers always go into bin 0, not `lbin`.

**Current code** (`atassess/atattribute.c` lines 392–397) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
  392: 
  393: 						/* Numbers in catch*/
  394: 						raw = biolVERTinfo[bcatchnums_id][sp][chrt][b];
  395: 						individVERTinfo[icatchnums_id][sp][0][z][attrib_id] += raw;
  396: 
  397: 						/* Biomass */
```

**Suggested fix (from review):** `[lbin]`

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

## ATT-07

**wrong variable** · Conf M / Sev M  
**Lines (current code):** 370-377, 469, 858  
**Lines at review (Oct 2026):** 370-377, 469, 858

**Why it's thought to be a bug**

Stage switch and age use `chrt` (cohort×gene).

**Current code** (`atassess/atattribute.c` lines 368–379)

```c
  368: 					for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
  369: 						/* Get information for nestlings, newly fledged and adult seabirds */
  370: 						switch (chrt) {
  371: 						case 0: /* Nestlings */
  372: 						case 1: /* Fledglings */
  373: 							lbin = chrt;
  374: 							break;
  375: 						default: /* All adults together */
  376: 							lbin = 2;
  377: 							break;
  378: 						}
  379: 						/* Get juvenile and newly fledged bird information */
```

**Suggested fix (from review):** base cohort

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

## ATT-08

**off-by-one** · Conf H / Sev M  
**Lines (current code):** 469, 858  
**Lines at review (Oct 2026):** 469, 858

**Why it's thought to be a bug**

`round(x+0.5)` shifts age up by 1 (same as SRV-08).

**Current code** (`atassess/atattribute.c` lines 467–471)

```c
  467: 
  468: 						//TODO Check with beth about changing to a round instead of a floor.
  469: 						cage = (int) (round((chrt + v) * FunctGroupArray[sp].ageClassSize + 0.5));
  470: 
  471: 						ninsubage = nraw / nsubages;
```

**Suggested fix (from review):** `round(x)`

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

## ATT-09

**off-by-one** · Conf L / Sev L  
**Lines (current code):** 460, 852  
**Lines at review (Oct 2026):** 460, 852

**Why it's thought to be a bug**

`ceil(ageClassSize+0.5)` gives one too many sub-ages; the remainder is dropped.

**Current code** (`atassess/atattribute.c` lines 458–462)

```c
  458: 					Sort_Length_Weight(bm, 0, 1, sizestocknums_id, z, indx, fishery_id, lngth, nraw, wgt, cmaxsize, attrib_id, &ltc, ofp);
  459: 
  460: 					nsubages = (int) (ceil(FunctGroupArray[sp].ageClassSize + 0.5));
  461:                     if (nsubages < 1)
  462:                         nsubages = 1;
```

**Suggested fix (from review):** `(int)(acs+0.5)`

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

## ATT-10

**accumulator mixing** · Conf M / Sev M  
**Lines (current code):** 383-402, 418-435  
**Lines at review (Oct 2026):** 383-402, 418-435

**Why it's thought to be a bug**

Bird raw counts are added onto already-normalised fractions in later rounds.

**Current code** (`atassess/atattribute.c` lines 381–420 (truncated))

```c
  381: 						raw = biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b];
  382: 						rawW = raw;
  383: 						individVERTinfo[iweight_id][sp][lbin][z][attrib_id] += raw;
  384: 
  385: 						/* Numbers in stock */
  386: 						raw = biolVERTinfo[bstocknums_id][sp][chrt][b];
  387: 						rawN = raw;
  388: 						individVERTinfo[istocknums_id][sp][lbin][z][attrib_id] += raw;
  389:                         if (chrt < FunctGroupArray[sp].numGeneTypes) {
  390:                             oldbaby[sp][z][attrib_id] += raw;
  391:                         }
  392: 
  393: 						/* Numbers in catch*/
  394: 						raw = biolVERTinfo[bcatchnums_id][sp][chrt][b];
  395: 						individVERTinfo[icatchnums_id][sp][0][z][attrib_id] += raw;
  396: 
  397: 						/* Biomass */
  398: 						individVERTinfo[ibiomass_id][sp][lbin][z][attrib_id] += rawW * rawN / (bm->boxes[b].area + TINY);
  399: 
  400: 						/* Discards */
  401: 						raw = biolVERTinfo[bdiscards_id][sp][chrt][b];
  402: 						individVERTinfo[idiscards_id][sp][lbin][z][attrib_id] += raw;
  403: 
  404: 					}
  405: 				}
  406: 			}
  407: 			/* Total mammal stock numbers */
  408: 			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  409: 				if (FunctGroupArray[sp].groupType == MAMMAL) {
  410: 					for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
  411: 						raw = biolVERTinfo[bstocknums_id][sp][chrt][b];
  412: 						stockinfo[sstocknums_id][sp][z][attrib_id] += raw;
  413: 					}
  414: 				}
  415: 			}
  416: 		}
  417: 	}
  418: 	for (z = 0; z < bm->nfzones; z++) {
  419: 		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  420: 			if(FunctGroupArray[sp].groupType == BIRD){
```

**Suggested fix (from review):** per-round locals

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

## ATT-11

**formula** · Conf M / Sev M  
**Lines (current code):** 490, 505, 869 (+atsurvey 535/699)  
**Lines at review (Oct 2026):** 490, 505, 869 (+atsurvey 535/699)

**Why it's thought to be a bug**

Production multiplied by stock numbers again (already included in atdatareading.c:536).

**Current code** (`atassess/atattribute.c` lines 488–507)

```c
  488: 						raw = biolVERTinfo[bprod_id][sp][chrt][b];
  489: 
  490: 						stockinfo[sprod_id][sp][z][attrib_id] += raw * samplearea[bb] * ceil(biolVERTinfo[bstocknums_id][sp][chrt][b]) / (vol + TINY);
  491: 
  492: 						/* Mammal Consumption */
  493: 						raw = biolVERTinfo[beat_id][sp][chrt][b];
  494: 						stockinfo[seat_id][sp][z][attrib_id] += raw * samplearea[bb];
  495: 					}
  496: 				}
  497: 			}
  498: 		}
  499: 
  500: 		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  501: 			if(FunctGroupArray[sp].groupType == BIRD){
  502: 				for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
  503: 					/* Bird production */
  504: 					raw = biolVERTinfo[bprod_id][sp][chrt][b];
  505: 					stockinfo[sprod_id][sp][z][attrib_id] += raw * samplearea[bb] * ceil(biolVERTinfo[bstocknums_id][sp][chrt][b]) / (vol + TINY);
  506: 
  507: 					/* Bird consumption */
```

**Suggested fix (from review):** drop the factor

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

## ATT-12

**inconsistency** · Conf L / Sev M  
**Lines (current code):** 500-512 vs 485, 866  
**Lines at review (Oct 2026):** 500-512 vs 485, 866

**Why it's thought to be a bug**

Bird prod/eat are not gated by `sample_now`.

**Current code** (`atassess/atattribute.c` lines 498–514) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
  498: 		}
  499: 
  500: 		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  501: 			if(FunctGroupArray[sp].groupType == BIRD){
  502: 				for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
  503: 					/* Bird production */
  504: 					raw = biolVERTinfo[bprod_id][sp][chrt][b];
  505: 					stockinfo[sprod_id][sp][z][attrib_id] += raw * samplearea[bb] * ceil(biolVERTinfo[bstocknums_id][sp][chrt][b]) / (vol + TINY);
  506: 
  507: 					/* Bird consumption */
  508: 					raw = biolVERTinfo[beat_id][sp][chrt][b];
  509: 					stockinfo[seat_id][sp][z][attrib_id] += raw * samplearea[bb];
  510: 				}
  511: 			}
  512: 		}
  513: 	}
  514: 
```

**Suggested fix (from review):** gate

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

## ATT-13

**units** · Conf M / Sev H ⭐ priority  
**Lines (current code):** 705 (+atsurvey.c:600)  
**Lines at review (Oct 2026):** 705 (+atsurvey.c:600)

**Why it's thought to be a bug**

`bdiscards` holds numbers (atdatareading.c:575) but is divided by weight, so it is ≈0.

**Current code** (`atassess/atattribute.c` lines 703–707)

```c
  703: 					Sort_Length_Weight(bm, 1, 1, sizecatchnums_id, z, sp, fishery_id, lngth, nraw, wgt, cmaxsize, attrib_id, &ltc, ofp);
  704: 
  705: 					raw = biolVERTinfo[bdiscards_id][sp][chrt][b] / (wgt + TINY);
  706: 					nraw = (int) (ceil(raw));
  707: 					Sort_Length_Weight(bm, 1, 1, sizediscardnums_id, z, sp, fishery_id, lngth, nraw, wgt, cmaxsize, attrib_id, &ltc, ofp);
```

**Suggested fix (from review):** drop `/ wgt`

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

## ATT-14

**double count** · Conf M / Sev M  
**Lines (current code):** 756-765 (+429)  
**Lines at review (Oct 2026):** 756-765 (+429)

**Why it's thought to be a bug**

Bird catch numbers are added twice.

**Current code** (`atassess/atattribute.c` lines 754–767)

```c
  754: 						}
  755: 					}
  756: 				} else {
  757: 					for (i = 0; i < 3; i++) {
  758: 						/* Total number of seabirds caught */
  759: 						stockinfo[scatchnums_id][sp][z][attrib_id] += stockinfo[stotcatch_id][sp][z][attrib_id]
  760: 								* individVERTinfo[icatchnums_id][sp][i][z][attrib_id] / (individVERTinfo[iweight_id][sp][i][z][attrib_id] + TINY);
  761: 
  762: 						/* Total number of seabirds in discards */
  763: 						stockinfo[sdiscardnums_id][sp][z][attrib_id] += stockinfo[sdiscards_id][sp][z][attrib_id]
  764: 								* individVERTinfo[idiscards_id][sp][i][z][attrib_id] / (individVERTinfo[iweight_id][sp][i][z][attrib_id] + TINY);
  765: 					}
  766: 				}
  767: 			}
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

## ATT-15

**condition** · Conf M / Sev L  
**Lines (current code):** 727, 756  
**Lines at review (Oct 2026):** 727, 756

**Why it's thought to be a bug**

`else` also catches invertebrates.

**Current code** (`atassess/atattribute.c` lines 725–758)

```c
  725: 			}
  726: 			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  727: 				if (FunctGroupArray[sp].isVertebrate == TRUE && FunctGroupArray[sp].groupType != BIRD) {
  728: 					for (i = 0; i < bm->K_num_size; i++) {
  729: 						if(FunctGroupArray[sp].groupType != MAMMAL){
  730: 							/* Store fish info */
  731: 							/* In case never get a fisheries independent estimate for this use caught and
  732: 							 discarded fish to give an average weight of individual in that size bin,
  733: 							 this value will be replaced by a fisheries independent one if it is collected */
  734: 
  735: 							individVERTinfo[iweight_id][sp][i][z][attrib_id] = sizebins[sizeweight_id][sp][z][i][1][attrib_id]
  736: 									/ (sizebins[sizecatchnums_id][sp][z][i][1][attrib_id] + sizebins[sizediscardnums_id][sp][z][i][1][attrib_id] + TINY);
  737: 
  738: 							/* Size distribution of catch */
  739: 							individVERTinfo[icatchnums_id][sp][i][z][attrib_id] = sizebins[sizecatchnums_id][sp][z][i][1][attrib_id]
  740: 									/ (totnums[totcatchnums_id][sp][z] + TINY);
  741: 
  742: 							/* Size distribution of discards */
  743: 							individVERTinfo[idiscards_id][sp][i][z][attrib_id] = sizebins[sizediscardnums_id][sp][z][i][1][attrib_id]
  744: 									/ (totnums[totdiscardnums_id][sp][z] + TINY);
  745: 						} else {
  746: 							/* Store mammal mammal info */
  747: 							/* Total number of mammals caught */
  748: 							stockinfo[scatchnums_id][sp][z][attrib_id] += stockinfo[stotcatch_id][sp][z][attrib_id]
  749: 									* individVERTinfo[icatchnums_id][sp][i][z][attrib_id] / (individVERTinfo[iweight_id][sp][i][z][attrib_id] + TINY);
  750: 
  751: 							/* Total number of mammals in discards */
  752: 							stockinfo[sdiscardnums_id][sp][z][attrib_id] += stockinfo[sdiscards_id][sp][z][attrib_id]
  753: 									* individVERTinfo[idiscards_id][sp][i][z][attrib_id] / (individVERTinfo[iweight_id][sp][i][z][attrib_id] + TINY);
  754: 						}
  755: 					}
  756: 				} else {
  757: 					for (i = 0; i < 3; i++) {
  758: 						/* Total number of seabirds caught */
```

**Suggested fix (from review):** `else if (BIRD)`

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

## ATT-16

**formula** · Conf M / Sev M  
**Lines (current code):** 934-935  
**Lines at review (Oct 2026):** 934-935

**Why it's thought to be a bug**

Biomass per bin = total biomass × individual weight (mass²).

**Current code** (`atassess/atattribute.c` lines 932–937) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
  932: 						/* Do stuff that was dependent on rescaled size distribution */
  933: 						/* Biomass per size bin - corrected for selectivity */
  934: 						individVERTinfo[ibiomass_id][sp][i][z][attrib_id] = stockinfo[sbiomass_id][sp][z][attrib_id]
  935: 								* individVERTinfo[iweight_id][sp][i][z][attrib_id];
  936: 
  937: 						/* Numbers in the stock */
```

**Suggested fix (from review):** × `istocknums` fraction

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
