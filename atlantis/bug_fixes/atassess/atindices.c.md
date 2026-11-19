# atassess/atindices.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **22 findings** (severity H: 9; confidence H and severity H: 8; on the priority list: 8).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [IND-01](#ind-01) | 240, 287-288 | M/M | inconsistency | The denominator includes epi/infauna; methods 2/3 exclude them from the numerator. |  | [ ] |
| [IND-02](#ind-02) ⭐ | 321-322, 1019-1020 | H/H | off-by-one | `if (j > K_max_agekey) j = K_max_agekey` lets the index equal the dimension size (`agelengthkey` age dimension… |  | [ ] |
| [IND-03](#ind-03) | 347-353 | M/M | formula | Bird size-at-maturity sums 3 lengths instead of averaging them. |  | [ ] |
| [IND-04](#ind-04) | 643-645 | H/M | wrong variable | Clamp tests `startbinbig` instead of `bincount`. |  | [ ] |
| [IND-05](#ind-05) | 563/578, 649/653 | M/M | index arithmetic | `+4` plankton offset added to an already-absolute bin; line 653 uses absolute `sizeclass`. |  | [ ] |
| [IND-06](#ind-06) | 422-425, 625-649 | M/M | stale accumulator | `nbinleft`/`startbinbig` carry over between species; `prop_bin` sums to >1. |  | [ ] |
| [IND-07](#ind-07) ⭐ | 691, 699, 702 vs 713 | H/H | copy-paste | Vertebrates are excluded from ranking but are written, so all land in `endnums[0]`. |  | [ ] |
| [IND-08](#ind-08) ✔ ⭐ | 814-819 | H/H | copy-paste | Inner rank loop is unfiltered, so rank > counter-1 and `numsort` < 0, writing `endnumsbig[negative]`. |  | [ ] |
| [IND-09](#ind-09) | 703-707, 741-746, 777-782, 815-819 | M/M | ranking | Ties collide; `endnums` is never cleared, so stale slots remain. |  | [ ] |
| [IND-10](#ind-10) ⭐ | 844-845, 869-870, 881-882, 900-901 | H/H | wrong variable | Loops over the trait index (0=nums, 1=biom) with `numStages` bound, so single-stage groups lose biomass and >2… |  | [ ] |
| [IND-11](#ind-11) | 874-925 | M/L | mislabel | `abundindex_id=1`/`bioindex_id=0`, but the stored data are 0=nums, 1=biomass. |  | [ ] |
| [IND-12](#ind-12) ✔ ⭐ | 1003-1004 | H/H | swapped index | `agelengthkey[k][sp][jj]`; layout is `[size][age][sp]`, as line 1004 shows. `scalarsum` is unused. |  | [ ] |
| [IND-13](#ind-13) | 1018 | L/M | truncation | No ROUNDGUARD; can produce -1. |  | [ ] |
| [IND-14](#ind-14) | 1103-1130 | M/M | inverted ratio | PS:ZP computed as planktivore/piscivore, opposite of the doc; pd[0] includes birds and mammals. |  | [ ] |
| [IND-15](#ind-15) | 1255-1257 | M/M | div-by-zero | No zero-count guard; sqrt of a slightly negative value gives NaN. |  | [ ] |
| [IND-16](#ind-16) ⭐ | 1275-1277 | H/H | formula | Variance = (Σx² − **mean²**)/(n−1); should be `n·mean²`. Compare Calculate_Phys_SD line 1255. |  | [ ] |
| [IND-17](#ind-17) ⭐ | 1316-1318 | H/H | off-by-one | `avgtl` is `[8][nfzones+2]` but reset only to `z < nfzones+1`, so `[nfzones+1]` accumulates forever. |  | [ ] |
| [IND-18](#ind-18) | 1360-1455 | M/H | wrong variable | Whole-area columns are weighted by `samplearea` (sums to 1 per zone), giving ≈ nfzones × TL. |  | [ ] |
| [IND-19](#ind-19) | 1346-1348 vs 1370-1376 | M/M | units | Vertebrate catch is divided by zonearea; invertebrate catch is not. |  | [ ] |
| [IND-20](#ind-20) ⭐ | 1504-1505 | H/H | copy-paste | "Static" block uses `tl[a]` (dynamic); the invertebrate equivalent uses `tl[a+3]`. |  | [ ] |
| [IND-21](#ind-21) | 1501, 1512, 1543, 1552 | M/M | missing weight | Zone-level catch is added once per box. |  | [ ] |
| [IND-22](#ind-22) | 1638-1665 | H/M | logic | Whole-area functional diversity omits the species count included per zone. |  | [ ] |

---

## IND-01

**inconsistency** · Conf M / Sev M  
**Lines (current code):** 240, 287-288  
**Lines at review (Oct 2026):** 240, 287-288

**Why it's thought to be a bug**

The denominator includes epi/infauna; methods 2/3 exclude them from the numerator.

**Current code** (`atassess/atindices.c` lines 238–242)

```c
  238: 
  239: 	for (i = 0; i < bm->K_num_tot_sp; i++) { // Targeted invertebrates only
  240: 		if (FunctGroupArray[i].isVertebrate == FALSE && FunctGroupArray[i].isFished == TRUE) {
  241: 			totcatch += invstockinfo[fcatch_id][i][z][id] / (zonearea[z] + TINY);
  242: 		}
```

**Suggested fix (from review):** same filter

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

## IND-02

**off-by-one** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 321-322, 1019-1020  
**Lines at review (Oct 2026):** 321-322, 1019-1020

**Why it's thought to be a bug**

`if (j > K_max_agekey) j = K_max_agekey` lets the index equal the dimension size (`agelengthkey` age dimension = K_max_agekey).

**Current code** (`atassess/atindices.c` lines 319–324)

```c
  319: 			/* Note: Using year classes so use agemat * age classsize */
  320: 			j = (int) (FunctGroupArray[indx].speciesParams[age_mat_id] * FunctGroupArray[indx].ageClassSize);
  321: 			if (j > bm->K_max_agekey)
  322: 				j = bm->K_max_agekey;
  323: 			step1 = 0.0;
  324: 			step2 = 0.0;
```

**Suggested fix (from review):** `>=` … `K_max_agekey-1`

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

## IND-03

**formula** · Conf M / Sev M  
**Lines (current code):** 347-353  
**Lines at review (Oct 2026):** 347-353

**Why it's thought to be a bug**

Bird size-at-maturity sums 3 lengths instead of averaging them.

**Current code** (`atassess/atindices.c` lines 345–355)

```c
  345: 				}
  346: 			} else {
  347: 				for (k = 0; k < 3; k++) {
  348: 					/* Size mature */
  349: 					wgt = individVERTinfo[iweight_id][indx][k][z][id];
  350: 					lngth = Get_Length(bm, wgt, indx);
  351: 
  352: 
  353: 					step2 += lngth;
  354: 					/* Condition factor */
  355: 					stepA += individVERTinfo[icondn_id][indx][k][z][id] * individVERTinfo[istocknums_id][indx][k][z][id];
```

**Suggested fix (from review):** weight / divide

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

## IND-04

**wrong variable** · Conf H / Sev M  
**Lines (current code):** 643-645  
**Lines at review (Oct 2026):** 643-645

**Why it's thought to be a bug**

Clamp tests `startbinbig` instead of `bincount`.

**Current code** (`atassess/atindices.c` lines 641–647)

```c
  641: 
  642: 					for (k = 0; k < binbita; k++) {
  643: 						bincount = startbinbig + k;
  644: 						if (startbinbig > pelbinbird - 1)
  645: 							bincount = pelbinbird - 1;
  646: 						pelbin[bincount][bioindex_id][z] += prop_bin * individVERTinfo[ibiomass_id][sp][sizeclasstransit][z][id];
  647: 						pelbin[bincount][abundindex_id][z] += prop_bin * individVERTinfo[istocknums_id][sp][sizeclasstransit][z][id] * stockinfo[sstocknums_id][sp][z][id];
```

**Suggested fix (from review):** `if (bincount > pelbinbird-1)`

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

## IND-05

**index arithmetic** · Conf M / Sev M  
**Lines (current code):** 563/578, 649/653  
**Lines at review (Oct 2026):** 563/578, 649/653

**Why it's thought to be a bug**

`+4` plankton offset added to an already-absolute bin; line 653 uses absolute `sizeclass`.

**Current code** (`atassess/atindices.c` lines 561–580)

```c
  561: 						staysmall = 0;
  562: 				} else {
  563: 					startbin = (int) (floor((FunctGroupArray[sp].speciesParams[allometic_bin_start_id] - 200.0) / 50.0)) + pelbinbig;
  564: 					numbin = FunctGroupArray[sp].speciesParams[allometic_bin_size_id] / 50.0;
  565: 					staybig = 1;
  566: 				}
  567: 
  568: 				if (numbin > 1)
  569: 					prop_bin = 1.0 / numbin;
  570: 				else {
  571: 					prop_bin = 1.0;
  572: 				}
  573: 				nbin = (int) (numbin);
  574: 
  575: 				/* If all within either the 10cm or 50cm bins without overlapping the transition */
  576: 				if (staysmall || staybig) {
  577: 					for (sizeclass = 0; sizeclass < bm->K_num_size; sizeclass++) {
  578: 						nextbin = (int) floor(ROUNDGUARD + (4 + startbin + sizeclass * numbin)); // 4 is the number of plankton bins
  579: 						if (nextbin > pelbinbird - 1)
  580: 							nextbin = pelbinbird - 1;
```

**Suggested fix (from review):** drop `+4`; use `sizeclass - sizeclasstransit`

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

## IND-06

**stale accumulator** · Conf M / Sev M  
**Lines (current code):** 422-425, 625-649  
**Lines at review (Oct 2026):** 422-425, 625-649

**Why it's thought to be a bug**

`nbinleft`/`startbinbig` carry over between species; `prop_bin` sums to >1.

**Current code** (`atassess/atindices.c` lines 420–427)

```c
  420: 	int pelbinbig = bm->K_num_pelbin - 11;
  421: 
  422: 	bincount = 0;
  423: 	startbinbig = 0;
  424: 	/* TODO: Check with Beth about  this */
  425: 	nbinleft = 0;
  426: 
  427: 	for (j = 0; j < bm->K_num_pelbin; j++)
```

**Suggested fix (from review):** reset per species

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

## IND-07

**copy-paste** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 691, 699, 702 vs 713  
**Lines at review (Oct 2026):** 691, 699, 702 vs 713

**Why it's thought to be a bug**

Vertebrates are excluded from ranking but are written, so all land in `endnums[0]`.

**Current code** (`atassess/atindices.c` lines 689–704) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
  689: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  690: 		numsort[sp] = 0;
  691: 		if (FunctGroupArray[sp].isMacroFauna == TRUE || FunctGroupArray[sp].isMicroFauna == TRUE) {
  692: 			counter++;
  693: 		}
  694: 	}
  695: 
  696: 	/* First for numbers */
  697: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  698: 		/* Skip det and sediment bacteria */
  699: 		if (FunctGroupArray[sp].isMacroFauna == TRUE || FunctGroupArray[sp].isMicroFauna == TRUE) {
  700: 			rank = 0;
  701: 			for (chrt = 0; chrt < bm->K_num_tot_sp; chrt++) {
  702: 				if (FunctGroupArray[chrt].isMacroFauna == TRUE || FunctGroupArray[chrt].isMicroFauna == TRUE) {
  703: 					if (nums[sp][index] > nums[chrt][index])
  704: 						rank++;
```

**Suggested fix (from review):** add `isVertebrate`

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

## IND-08

**copy-paste** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Lines (current code):** 814-819  
**Lines at review (Oct 2026):** 814-819

**Why it's thought to be a bug**

Inner rank loop is unfiltered, so rank > counter-1 and `numsort` < 0, writing `endnumsbig[negative]`.

**Current code** (`atassess/atindices.c` lines 812–821)

```c
  812: 		if (FunctGroupArray[sp].isVertebrate == TRUE || FunctGroupArray[sp].isMacroFauna == TRUE) {
  813: 			rank = 0;
  814: 			for (chrt = 0; chrt < bm->K_num_tot_sp; chrt++) {
  815:                 if (globalnums[sp][index][typeIndex] > globalnums[chrt][index][typeIndex]) {
  816:                     rank++;
  817:                 }
  818: 			}
  819: 			numsort[sp] = counter - 1 - rank;
  820: 		}
  821: 	}
```

**Suggested fix (from review):** filter `chrt` like the sibling functions

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

## IND-09

**ranking** · Conf M / Sev M  
**Lines (current code):** 703-707, 741-746, 777-782, 815-819  
**Lines at review (Oct 2026):** 703-707, 741-746, 777-782, 815-819

**Why it's thought to be a bug**

Ties collide; `endnums` is never cleared, so stale slots remain.

**Current code** (`atassess/atindices.c` lines 701–740 (truncated))

```c
  701: 			for (chrt = 0; chrt < bm->K_num_tot_sp; chrt++) {
  702: 				if (FunctGroupArray[chrt].isMacroFauna == TRUE || FunctGroupArray[chrt].isMicroFauna == TRUE) {
  703: 					if (nums[sp][index] > nums[chrt][index])
  704: 						rank++;
  705: 				}
  706: 			}
  707: 			numsort[sp] = counter - 1 - rank;
  708: 		}
  709: 	}
  710: 
  711: 	/* Update final ranked values */
  712: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  713: 		if (FunctGroupArray[sp].isVertebrate == TRUE || FunctGroupArray[sp].isMacroFauna == TRUE || FunctGroupArray[sp].isMicroFauna == TRUE) {
  714: 			endnums[numsort[sp]][index][z] = nums[sp][index];
  715: 		}
  716: 	}
  717: }
  718: /**
  719:  * \brief Sort the nums values - this function will not include the microfauna.
  720:  *
  721:  *
  722:  */
  723: static void sort_values_big(MSEBoxModel *bm, int index, int z) {
  724: 	int sp, chrt;
  725: 	int rank;
  726: 	int counter = 0;
  727: 
  728: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  729: 		numsort[sp] = 0;
  730: 		if (FunctGroupArray[sp].isMacroFauna == TRUE) {
  731: 			counter++;
  732: 		}
  733: 	}
  734: 	/* First for numbers */
  735: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  736: 		/* Skip det and sediment bacteria */
  737: 		if (FunctGroupArray[sp].isMacroFauna == TRUE) {
  738: 			rank = 0;
  739: 			for (chrt = 0; chrt < bm->K_num_tot_sp; chrt++) {
  740: 				if (FunctGroupArray[chrt].isMacroFauna == TRUE) {
```

**Suggested fix (from review):** tie-break and clear

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

## IND-10

**wrong variable** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 844-845, 869-870, 881-882, 900-901  
**Lines at review (Oct 2026):** 844-845, 869-870, 881-882, 900-901

**Why it's thought to be a bug**

Loops over the trait index (0=nums, 1=biom) with `numStages` bound, so single-stage groups lose biomass and >2 stages go out of bounds.

**Current code** (`atassess/atindices.c` lines 842–881 (truncated))

```c
  842: 
  843: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  844: 		for (chrt = 0; chrt < FunctGroupArray[sp].numStages; chrt++)
  845: 			nums[sp][chrt] = 0;
  846: 	}
  847: 
  848: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  849: 		/* Skip det and sediment bacteria */
  850: 		if (FunctGroupArray[sp].isDetritus == TRUE || FunctGroupArray[sp].isBacteria == TRUE)
  851: 			continue;
  852: 
  853: 		if (FunctGroupArray[sp].isVertebrate == TRUE) {
  854: 			nums[sp][0] = stockinfo[sstocknums_id][sp][z][id] / (zonearea[z] + TINY);
  855: 			nums[sp][1] = stockinfo[sbiomass_id][sp][z][id] / (zonearea[z] + TINY);
  856: 
  857: 		} else if (FunctGroupArray[sp].isMacroFauna == TRUE || FunctGroupArray[sp].isMicroFauna == TRUE) {
  858: 			nums[sp][0] = samplebiom[sp][z][id] / (FunctGroupArray[sp].speciesParams[avg_inv_size_id] + TINY);
  859: 			nums[sp][1] = samplebiom[sp][z][id];
  860: 
  861: //			fprintf(bm->logFile, "nums[%s][0] = %e, samplebiom[sp][z][id] = %e, FunctGroupArray[sp].speciesParams[avg_inv_size_id] = %e\n",FunctGroupArray[sp].groupCode, nums[sp][0], samplebiom[sp][z][id], FunctGroupArray[sp].speciesParams[avg_inv_size_id]);
  862: //			fprintf(bm->logFile, "nums[%s][1] = %e\n",FunctGroupArray[sp].groupCode, nums[sp][1]);
  863: 
  864: 		}
  865: 	}
  866: 
  867: 	/* Don't bother checking groups as we don't care about adding 0 values */
  868: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  869: 		for (chrt = 0; chrt < FunctGroupArray[sp].numStages; chrt++)
  870: 			globalnums[sp][chrt][0] += nums[sp][chrt];
  871: 	}
  872: 
  873: 	/* First for numbers */
  874: 	sort_values(bm, 0, z);
  875: 
  876: 	/* Now for biomass */
  877: 	sort_values(bm, 1, z);
  878: 
  879: 	/* Repeat without microfauna */
  880: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  881:         for (chrt = 0; chrt < FunctGroupArray[sp].numStages; chrt++) {
```

**Suggested fix (from review):** `chrt < 2`

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

## IND-11

**mislabel** · Conf M / Sev L  
**Lines (current code):** 874-925  
**Lines at review (Oct 2026):** 874-925

**Why it's thought to be a bug**

`abundindex_id=1`/`bioindex_id=0`, but the stored data are 0=nums, 1=biomass.

**Current code** (`atassess/atindices.c` lines 872–911 (truncated))

```c
  872: 
  873: 	/* First for numbers */
  874: 	sort_values(bm, 0, z);
  875: 
  876: 	/* Now for biomass */
  877: 	sort_values(bm, 1, z);
  878: 
  879: 	/* Repeat without microfauna */
  880: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  881:         for (chrt = 0; chrt < FunctGroupArray[sp].numStages; chrt++) {
  882:             nums[sp][chrt] = 0;
  883:         }
  884: 	}
  885: 
  886: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  887: 		/* Skip det and sediment bacteria */
  888: 		if (FunctGroupArray[sp].isMacroFauna == TRUE) {
  889: 			nums[sp][0] = samplebiom[sp][z][id] / (FunctGroupArray[sp].speciesParams[avg_inv_size_id] + TINY);
  890: 			nums[sp][1] = samplebiom[sp][z][id];
  891: 
  892: 		}
  893: 		if (FunctGroupArray[sp].isVertebrate == TRUE) {
  894: 			nums[sp][0] = stockinfo[sstocknums_id][sp][z][id] / (zonearea[z] + TINY);
  895: 			nums[sp][1] = stockinfo[sbiomass_id][sp][z][id] / (zonearea[z] + TINY);
  896: 		}
  897: 	}
  898: 
  899: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  900: 		for (chrt = 0; chrt < FunctGroupArray[sp].numStages; chrt++) {
  901: 			globalnums[sp][chrt][1] += nums[sp][chrt];
  902: 		}
  903: 	}
  904: 
  905: 	/* First for numbers */
  906: 	sort_values_big(bm, abundindex_id, z);
  907: 
  908: 	/* Now for biomass */
  909: 	sort_values_big(bm, bioindex_id, z);
  910: 
  911: 	/* For bay as a whole */
```

**Suggested fix (from review):** align constants

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

## IND-12

**swapped index** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Lines (current code):** 1003-1004  
**Lines at review (Oct 2026):** 1003-1004

**Why it's thought to be a bug**

`agelengthkey[k][sp][jj]`; layout is `[size][age][sp]`, as line 1004 shows. `scalarsum` is unused.

**Current code** (`atassess/atindices.c` lines 1001–1006)

```c
 1001: 							scalarsum = 0;
 1002: 							for (k = 0; k < bm->K_num_size; k++) {
 1003: 								newnums = stockinfo[sstocknums_id][sp][z][id] * individVERTinfo[istocknums_id][sp][k][z][id] * agelengthkey[k][sp][jj][z][id];
 1004: 								scalarsum += individVERTinfo[istocknums_id][sp][k][z][id] * agelengthkey[k][jj][sp][z][id];
 1005: 								step1 += newnums;
 1006: 							}
```

**Suggested fix (from review):** `[k][jj][sp]`

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

## IND-13

**truncation** · Conf L / Sev M  
**Lines (current code):** 1018  
**Lines at review (Oct 2026):** 1018

**Why it's thought to be a bug**

No ROUNDGUARD; can produce -1.

**Current code** (`atassess/atindices.c` lines 1016–1020)

```c
 1016: 				} else {
 1017: 					step2 = 0.0;
 1018: 					chrt = (int) (FunctGroupArray[sp].speciesParams[age_mat_id] * FunctGroupArray[sp].ageClassSize - 1);
 1019: 					if (chrt > bm->K_max_agekey)
 1020: 						chrt = bm->K_max_agekey;
```

**Suggested fix (from review):** floor + guard + clamp

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

## IND-14

**inverted ratio** · Conf M / Sev M  
**Lines (current code):** 1103-1130  
**Lines at review (Oct 2026):** 1103-1130

**Why it's thought to be a bug**

PS:ZP computed as planktivore/piscivore, opposite of the doc; pd[0] includes birds and mammals.

**Current code** (`atassess/atindices.c` lines 1101–1132)

```c
 1101: 		if (FunctGroupArray[sp].isVertebrate == TRUE) {
 1102: 			if (FunctGroupArray[sp].speciesParams[flagdem_id] < 1) {
 1103: 				if(FunctGroupArray[sp].speciesParams[flagplankfish_id])
 1104: 					/* Planktivorous surface fish biomass */
 1105: 					step1 += stockinfo[sbiomass_id][sp][z][id];
 1106: 				else if (FunctGroupArray[sp].groupType == FISH)
 1107: 					/* Pisivorous non-demersal fish biomass */
 1108: 					step2 += stockinfo[sbiomass_id][sp][z][id];
 1109: 			}
 1110: 		}
 1111: 	}
 1112: 
 1113: 	pd[1][z] = step1 / (step2 + TINY);
 1114: 	globalpd[0][1] += step1;
 1115: 	globalpd[1][1] += step2;
 1116: 
 1117: 	step1 = 0.0;
 1118: 	step2 = 0.0;
 1119: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 1120: 		if (FunctGroupArray[sp].isVertebrate == TRUE) {
 1121: 			if(FunctGroupArray[sp].speciesParams[flagplankfish_id])
 1122: 				/* Planktivorous fish biomass */
 1123: 				step1 += stockinfo[sbiomass_id][sp][z][id];
 1124: 			else if (FunctGroupArray[sp].groupType == FISH)
 1125: 				/* Piscivorous fish biomass */
 1126: 				step2 += stockinfo[sbiomass_id][sp][z][id];
 1127: 		}
 1128: 	}
 1129: 
 1130: 	pd[2][z] = step1 / (step2 + TINY);
 1131: 	globalpd[0][2] += step1;
 1132: 	globalpd[1][2] += step2;
```

**Suggested fix (from review):** swap / fix doc

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

## IND-15

**div-by-zero** · Conf M / Sev M  
**Lines (current code):** 1255-1257  
**Lines at review (Oct 2026):** 1255-1257

**Why it's thought to be a bug**

No zero-count guard; sqrt of a slightly negative value gives NaN.

**Current code** (`atassess/atindices.c` lines 1253–1259)

```c
 1253: 
 1254: 	for (i = 0; i < num_sampled_phy_id; i++) {
 1255: 		step1 = cvphys[1][i][z] - ((cvphys[0][i][z] * cvphys[0][i][z]) / cvphys[2][i][z]);
 1256: 		step2 = step1 / (cvphys[2][i][z] - 1.0 + TINY);
 1257: 		physicalSigma[i][z] = sqrt(step2);
 1258: 	}
 1259: }
```

**Suggested fix (from review):** +TINY, clamp ≥0

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

## IND-16

**formula** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 1275-1277  
**Lines at review (Oct 2026):** 1275-1277

**Why it's thought to be a bug**

Variance = (Σx² − **mean²**)/(n−1); should be `n·mean²`. Compare Calculate_Phys_SD line 1255.

**Current code** (`atassess/atindices.c` lines 1273–1279)

```c
 1273: 	for (j = 0; j < bm->K_num_tot_sp; j++) {
 1274: 		if (FunctGroupArray[j].isDetritus == FALSE) {
 1275: 			mean = cvsample[0][j][z] / (cvsample[2][j][z] + TINY);
 1276: 			step1 = cvsample[1][j][z] - (mean * mean);
 1277: 			step2 = step1 / (cvsample[2][j][z] - 1.0 + TINY);
 1278: 			sigma = sqrt(step2);
 1279: 
```

**Suggested fix (from review):** `- n*mean*mean`

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

## IND-17

**off-by-one** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 1316-1318  
**Lines at review (Oct 2026):** 1316-1318

**Why it's thought to be a bug**

`avgtl` is `[8][nfzones+2]` but reset only to `z < nfzones+1`, so `[nfzones+1]` accumulates forever.

**Current code** (`atassess/atindices.c` lines 1314–1320)

```c
 1314: 	}
 1315: 
 1316: 	for (z = 0; z < bm->nfzones + 1; z++)
 1317: 		for (bb = 0; bb < 8; bb++)
 1318: 			avgtl[bb][z] = 0.0;
 1319: 
 1320: 	for (bb = 0; bb < bm->nsbox; bb++) {
```

**Suggested fix (from review):** `z < nfzones+2`

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

## IND-18

**wrong variable** · Conf M / Sev H  
**Lines (current code):** 1360-1455  
**Lines at review (Oct 2026):** 1360-1455

**Why it's thought to be a bug**

Whole-area columns are weighted by `samplearea` (sums to 1 per zone), giving ≈ nfzones × TL.

**Current code** (`atassess/atindices.c` lines 1358–1397 (truncated))

```c
 1358: 		avgtl[4][z] += catches * samplearea[bb] / (totcatch + TINY);
 1359: 		avgtl[4][bm->nfzones] += catches * totalarea[bb] / (totcatch + TINY);
 1360: 		avgtl[4][bm->nfzones + 1] += totcatchA * samplearea[bb] / (totcatch + TINY);
 1361: 		avgtl[5][z] += catchesST * samplearea[bb] / (totcatch + TINY);
 1362: 		avgtl[5][bm->nfzones] += catchesST * totalarea[bb] / (totcatch + TINY);
 1363: 		avgtl[5][bm->nfzones + 1] += totcatchAST * samplearea[bb] / (totcatch + TINY);
 1364: 
 1365: 		/* Invertebrates */
 1366: 		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 1367: 			if (FunctGroupArray[sp].isVertebrate == FALSE && (FunctGroupArray[sp].isFished == TRUE || FunctGroupArray[sp].isImpacted == TRUE)) {
 1368: 
 1369: 				/* Assume the the fished invertebrates don't include detritus */
 1370: 				catches += invstockinfo[fcatch_id][sp][z][id] * tl[a][sp][0];
 1371: 				catchesST += invstockinfo[fcatch_id][sp][z][id] * tl[a + 3][sp][0];
 1372: 				totcatch += invstockinfo[fcatch_id][sp][z][id];
 1373: 
 1374: 				/* Overall case */
 1375: 				totcatchA += invstockinfo[fcatch_id][sp][z][id] * tl[2][sp][0];
 1376: 				totcatchAST += invstockinfo[fcatch_id][sp][z][id] * tl[5][sp][0];
 1377: 			}
 1378: 		}
 1379: 
 1380: 		avgtl[0][z] += catches * samplearea[bb] / (totcatch + TINY);
 1381: 		avgtl[0][bm->nfzones] += catches * totalarea[bb] / (totcatch + TINY);
 1382: 		avgtl[0][bm->nfzones + 1] += totcatchA * samplearea[bb] / (totcatch + TINY);
 1383: 		avgtl[2][z] += catchesST * samplearea[bb] / (totcatch + TINY);
 1384: 		avgtl[2][bm->nfzones] += catchesST * totalarea[bb] / (totcatch + TINY);
 1385: 		avgtl[2][bm->nfzones + 1] += totcatchAST * samplearea[bb] / (totcatch + TINY);
 1386: 
 1387: 		/*** Overall ***/
 1388: 		/* Fish */
 1389: 		biom = 0.0;
 1390: 		biomST = 0.0;
 1391: 		totbiom = 0.0;
 1392: 		biomA = 0.0;
 1393: 		biomAST = 0.0;
 1394: 
 1395: 		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 1396: 			if (FunctGroupArray[sp].isVertebrate == TRUE) {
 1397: 				for (stage = 0; stage < K_num_invageclass; stage++) {
```

**Suggested fix (from review):** use `totalarea[bb]`

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

## IND-19

**units** · Conf M / Sev M  
**Lines (current code):** 1346-1348 vs 1370-1376  
**Lines at review (Oct 2026):** 1346-1348 vs 1370-1376

**Why it's thought to be a bug**

Vertebrate catch is divided by zonearea; invertebrate catch is not.

**Current code** (`atassess/atindices.c` lines 1344–1378)

```c
 1344: 						sample = pop_fraction[1][sp][z];
 1345: 
 1346: 					catches += stockinfo[stotcatch_id][sp][z][id] * sample * tl[a][sp][stage] / (zonearea[z] + TINY);
 1347: 					catchesST += stockinfo[stotcatch_id][sp][z][id] * sample * tl[a + 3][sp][stage] / (zonearea[z] + TINY);
 1348: 					totcatch += stockinfo[stotcatch_id][sp][z][id] * sample / (zonearea[z] + TINY);
 1349: 
 1350: 					/* Overall case */
 1351: 					totcatchA += stockinfo[stotcatch_id][sp][z][id] * sample * tl[2][sp][stage] / (zonearea[z] + TINY);
 1352: 					totcatchAST += stockinfo[stotcatch_id][sp][z][id] * sample * tl[5][sp][stage] / (zonearea[z] + TINY);
 1353: 
 1354: 				}
 1355: 			}
 1356: 		}
 1357: 
 1358: 		avgtl[4][z] += catches * samplearea[bb] / (totcatch + TINY);
 1359: 		avgtl[4][bm->nfzones] += catches * totalarea[bb] / (totcatch + TINY);
 1360: 		avgtl[4][bm->nfzones + 1] += totcatchA * samplearea[bb] / (totcatch + TINY);
 1361: 		avgtl[5][z] += catchesST * samplearea[bb] / (totcatch + TINY);
 1362: 		avgtl[5][bm->nfzones] += catchesST * totalarea[bb] / (totcatch + TINY);
 1363: 		avgtl[5][bm->nfzones + 1] += totcatchAST * samplearea[bb] / (totcatch + TINY);
 1364: 
 1365: 		/* Invertebrates */
 1366: 		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 1367: 			if (FunctGroupArray[sp].isVertebrate == FALSE && (FunctGroupArray[sp].isFished == TRUE || FunctGroupArray[sp].isImpacted == TRUE)) {
 1368: 
 1369: 				/* Assume the the fished invertebrates don't include detritus */
 1370: 				catches += invstockinfo[fcatch_id][sp][z][id] * tl[a][sp][0];
 1371: 				catchesST += invstockinfo[fcatch_id][sp][z][id] * tl[a + 3][sp][0];
 1372: 				totcatch += invstockinfo[fcatch_id][sp][z][id];
 1373: 
 1374: 				/* Overall case */
 1375: 				totcatchA += invstockinfo[fcatch_id][sp][z][id] * tl[2][sp][0];
 1376: 				totcatchAST += invstockinfo[fcatch_id][sp][z][id] * tl[5][sp][0];
 1377: 			}
 1378: 		}
```

**Suggested fix (from review):** divide inverts too

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

## IND-20

**copy-paste** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 1504-1505  
**Lines at review (Oct 2026):** 1504-1505

**Why it's thought to be a bug**

"Static" block uses `tl[a]` (dynamic); the invertebrate equivalent uses `tl[a+3]`.

**Current code** (`atassess/atindices.c` lines 1502–1507)

```c
 1502: 						trophspect[tlindx][0][bm->nfzones] += stockinfo[stotcatch_id][sp][z][id] * sample * samplearea[bb];
 1503: 
 1504: 						/* Zone specific and total, static trophic level through time */
 1505: 						tlindx = (int) (floor((tl[a][sp][stage] - 1.0) / 0.5));
 1506:                         if (tlindx < 0) {
 1507:                             tlindx = 0;
```

**Suggested fix (from review):** `tl[a+3]`

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

## IND-21

**missing weight** · Conf M / Sev M  
**Lines (current code):** 1501, 1512, 1543, 1552  
**Lines at review (Oct 2026):** 1501, 1512, 1543, 1552

**Why it's thought to be a bug**

Zone-level catch is added once per box.

**Current code** (`atassess/atindices.c` lines 1499–1538 (truncated))

```c
 1499: 						if (tlindx >= bm->K_num_trophbin)
 1500: 							tlindx = bm->K_num_trophbin - 1;
 1501: 						trophspect[tlindx][0][z] += stockinfo[stotcatch_id][sp][z][id] * sample;
 1502: 						trophspect[tlindx][0][bm->nfzones] += stockinfo[stotcatch_id][sp][z][id] * sample * samplearea[bb];
 1503: 
 1504: 						/* Zone specific and total, static trophic level through time */
 1505: 						tlindx = (int) (floor((tl[a][sp][stage] - 1.0) / 0.5));
 1506:                         if (tlindx < 0) {
 1507:                             tlindx = 0;
 1508:                         }
 1509:                         if (tlindx >= bm->K_num_trophbin) {
 1510:                             tlindx = bm->K_num_trophbin - 1;
 1511:                         }
 1512: 						trophspect[tlindx][1][z] += stockinfo[stotcatch_id][sp][z][id] * sample;
 1513: 						trophspect[tlindx][1][bm->nfzones] += stockinfo[stotcatch_id][sp][z][id] * sample * samplearea[bb];
 1514: 
 1515: 						/* Overall area, dynamic trophic level through time */
 1516: 						tlindx = (int) (floor((tl[2][sp][stage] - 1.0) / 0.5));
 1517: 						if (tlindx < 0)
 1518: 							tlindx = 0;
 1519: 						if (tlindx >= bm->K_num_trophbin)
 1520: 							tlindx = bm->K_num_trophbin - 1;
 1521: 						trophspect[tlindx][0][bm->nfzones + 1] += stockinfo[stotcatch_id][sp][z][id] * sample * samplearea[bb];
 1522: 
 1523: 						/* Overall area, static trophic level through time */
 1524: 						tlindx = (int) (floor((tl[5][sp][stage] - 1.0) / 0.5));
 1525: 						if (tlindx < 0)
 1526: 							tlindx = 0;
 1527: 						if (tlindx >= bm->K_num_trophbin)
 1528: 							tlindx = bm->K_num_trophbin - 1;
 1529: 						trophspect[tlindx][1][bm->nfzones + 1] += stockinfo[stotcatch_id][sp][z][id] * sample * samplearea[bb];
 1530: 					}
 1531: 				}
 1532: 
 1533: 				else {
 1534: 					sample = 1.0;
 1535: 
 1536: 					stage = 0;
 1537: 					/* Zone specific and total, dynamic trophic level through time */
 1538: 					tlindx = (int) (floor((tl[a][sp][stage] - 1.0) / 0.5));
```

**Suggested fix (from review):** × samplearea or move out

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

## IND-22

**logic** · Conf H / Sev M  
**Lines (current code):** 1638-1665  
**Lines at review (Oct 2026):** 1638-1665

**Why it's thought to be a bug**

Whole-area functional diversity omits the species count included per zone.

**Current code** (`atassess/atindices.c` lines 1636–1667)

```c
 1636: 				rank++;
 1637: 		}
 1638: 		divindx[0][z] = rank;
 1639: 		for (j = 0; j < bm->K_num_tot_sp; j++) {
 1640: 			if (FunctGroupArray[j].isVertebrate == TRUE) {
 1641: 				if (divfn[j][z] > 0)
 1642: 					rank++;
 1643: 			}
 1644: 		}
 1645: 		divindx[1][z] = rank;
 1646: 	}
 1647: 
 1648: 	for (j = 0; j < bm->K_num_tot_sp; j++) {
 1649: 		for (z = 0; z < bm->nfzones; z++) {
 1650: 			if (divsp[j][z] > 0) {
 1651: 				divindx[0][bm->nfzones]++;
 1652: 				break;
 1653: 			}
 1654: 		}
 1655: 	}
 1656: 	for (j = 0; j < bm->K_num_tot_sp; j++) {
 1657: 		if (FunctGroupArray[j].isVertebrate == TRUE) {
 1658: 			for (z = 0; z < bm->nfzones; z++) {
 1659: 				if (divfn[j][z] > 0) {
 1660: 					divindx[1][bm->nfzones]++;
 1661: 					break;
 1662: 				}
 1663: 			}
 1664: 		}
 1665: 	}
 1666: 
 1667: 	return;
```

**Suggested fix (from review):** start from `divindx[0][nfzones]`

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
