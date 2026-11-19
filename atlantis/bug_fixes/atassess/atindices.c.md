# atassess/atindices.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **22 findings** (severity H: 9; confidence H and severity H: 8; on the priority list: 8).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [IND-01](#ind-01) | 240 | M/M | inconsistency | The denominator includes epi/infauna; methods 2/3 exclude them from the numerator. |  | [ ] |
| [IND-02](#ind-02) ⭐ | 321 | H/H | off-by-one | `if (j > K_max_agekey) j = K_max_agekey` lets the index equal the dimension size (`agelengthkey` age dimension… |  | [ ] |
| [IND-03](#ind-03) | 347 | M/M | formula | Bird size-at-maturity sums 3 lengths instead of averaging them. |  | [ ] |
| [IND-04](#ind-04) | 644 | H/M | wrong variable | Clamp tests `startbinbig` instead of `bincount`. |  | [ ] |
| [IND-05](#ind-05) | 604 | M/M | index arithmetic | `+4` plankton offset added to an already-absolute bin; line 653 uses absolute `sizeclass`. |  | [ ] |
| [IND-06](#ind-06) | 417 | M/M | stale accumulator | `nbinleft`/`startbinbig` carry over between species; `prop_bin` sums to >1. |  | [ ] |
| [IND-07](#ind-07) ⭐ | 714 | H/H | copy-paste | Vertebrates are excluded from ranking but are written, so all land in `endnums[0]`. |  | [ ] |
| [IND-08](#ind-08) ✔ ⭐ | 826 | H/H | copy-paste | Inner rank loop is unfiltered, so rank > counter-1 and `numsort` < 0, writing `endnumsbig[negative]`. |  | [ ] |
| [IND-09](#ind-09) | 714 | M/M | ranking | Ties collide; `endnums` is never cleared, so stale slots remain. |  | [ ] |
| [IND-10](#ind-10) ⭐ | 844, 869 | H/H | wrong variable | Loops over the trait index (0=nums, 1=biom) with `numStages` bound, so single-stage groups lose biomass and >2… |  | [ ] |
| [IND-11](#ind-11) | 906 | M/L | mislabel | `abundindex_id=1`/`bioindex_id=0`, but the stored data are 0=nums, 1=biomass. |  | [ ] |
| [IND-12](#ind-12) ✔ ⭐ | 1003 | H/H | swapped index | `agelengthkey[k][sp][jj]`; layout is `[size][age][sp]`, as line 1004 shows. `scalarsum` is unused. |  | [ ] |
| [IND-13](#ind-13) | 1018 | L/M | truncation | No ROUNDGUARD; can produce -1. |  | [ ] |
| [IND-14](#ind-14) | 1103 | M/M | inverted ratio | PS:ZP computed as planktivore/piscivore, opposite of the doc; pd[0] includes birds and mammals. |  | [ ] |
| [IND-15](#ind-15) | 1255 | M/M | div-by-zero | No zero-count guard; sqrt of a slightly negative value gives NaN. |  | [ ] |
| [IND-16](#ind-16) ⭐ | 1250 | H/H | formula | Variance = (Σx² − **mean²**)/(n−1); should be `n·mean²`. Compare Calculate_Phys_SD line 1255. |  | [ ] |
| [IND-17](#ind-17) ⭐ | 1316, 1360 | H/H | off-by-one | `avgtl` is `[8][nfzones+2]` but reset only to `z < nfzones+1`, so `[nfzones+1]` accumulates forever. |  | [ ] |
| [IND-18](#ind-18) | 1362 | M/H | wrong variable | Whole-area columns are weighted by `samplearea` (sums to 1 per zone), giving ≈ nfzones × TL. |  | [ ] |
| [IND-19](#ind-19) | 1346 | M/M | units | Vertebrate catch is divided by zonearea; invertebrate catch is not. |  | [ ] |
| [IND-20](#ind-20) ⭐ | 1547 | H/H | copy-paste | "Static" block uses `tl[a]` (dynamic); the invertebrate equivalent uses `tl[a+3]`. |  | [ ] |
| [IND-21](#ind-21) | 1501 | M/M | missing weight | Zone-level catch is added once per box. |  | [ ] |
| [IND-22](#ind-22) | 1651 | H/M | logic | Whole-area functional diversity omits the species count included per zone. |  | [ ] |

---

## IND-01

**inconsistency** · Conf M / Sev M  
**Found at (current code):** `atindices.c` 240
**Lines at review (Oct 2026):** 240, 287-288

**Why it's thought to be a bug**

The denominator includes epi/infauna; methods 2/3 exclude them from the numerator.

**Code at issue** — `atindices.c` line 240 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `if (FunctGroupArray[i].isVertebrate == FALSE && FunctGroupArray[i].isFished == TRUE) {`

```c
  238: 
  239: 	for (i = 0; i < bm->K_num_tot_sp; i++) { // Targeted invertebrates only
  240→ 		if (FunctGroupArray[i].isVertebrate == FALSE && FunctGroupArray[i].isFished == TRUE) {
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
**Found at (current code):** `atindices.c` 321
**Lines at review (Oct 2026):** 321-322, 1019-1020

**Why it's thought to be a bug**

`if (j > K_max_agekey) j = K_max_agekey` lets the index equal the dimension size (`agelengthkey` age dimension = K_max_agekey).

**Code at issue** — `atindices.c` line 321  
Search for: `if (j > bm->K_max_agekey)`

```c
  319: 			/* Note: Using year classes so use agemat * age classsize */
  320: 			j = (int) (FunctGroupArray[indx].speciesParams[age_mat_id] * FunctGroupArray[indx].ageClassSize);
  321→ 			if (j > bm->K_max_agekey)
  322: 				j = bm->K_max_agekey;
  323: 			step1 = 0.0;
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
**Found at (current code):** `atindices.c` 347
**Lines at review (Oct 2026):** 347-353

**Why it's thought to be a bug**

Bird size-at-maturity sums 3 lengths instead of averaging them.

**Code at issue** — `atindices.c` line 347 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `for (k = 0; k < 3; k++) {`

```c
  345: 				}
  346: 			} else {
  347→ 				for (k = 0; k < 3; k++) {
  348: 					/* Size mature */
  349: 					wgt = individVERTinfo[iweight_id][indx][k][z][id];
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
**Found at (current code):** `atindices.c` 644
**Lines at review (Oct 2026):** 643-645

**Why it's thought to be a bug**

Clamp tests `startbinbig` instead of `bincount`.

**Code at issue** — `atindices.c` line 644 (the line the review pointed to, found by its text)  
Search for: `if (startbinbig > pelbinbird - 1)`

```c
  642: 					for (k = 0; k < binbita; k++) {
  643: 						bincount = startbinbig + k;
  644→ 						if (startbinbig > pelbinbird - 1)
  645: 							bincount = pelbinbird - 1;
  646: 						pelbin[bincount][bioindex_id][z] += prop_bin * individVERTinfo[ibiomass_id][sp][sizeclasstransit][z][id];
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
**Found at (current code):** `atindices.c` 604
**Lines at review (Oct 2026):** 563/578, 649/653

**Why it's thought to be a bug**

`+4` plankton offset added to an already-absolute bin; line 653 uses absolute `sizeclass`.

**Code at issue** — `atindices.c` line 604 (located by name only; check this is the right place)  
Search for: `sizeclasstransit = sizeclass;`

```c
  602: 						nextbinB = (int) floor(ROUNDGUARD + (4 + startbin + (sizeclass + 1) * numbin));
  603: 						if (nextbin <= pelbinbig && nextbinB > pelbinbig) {
  604→ 							sizeclasstransit = sizeclass;
  605: 							break;
  606: 						}
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
**Found at (current code):** `atindices.c` 417
**Lines at review (Oct 2026):** 422-425, 625-649

**Why it's thought to be a bug**

`nbinleft`/`startbinbig` carry over between species; `prop_bin` sums to >1.

**Code at issue** — `atindices.c` line 417 (located by name only; check this is the right place)  
Search for: `int j, k, bin, sizeclass, nextbin, bincount, startbin, sp, nbin, nbinbig, nbinleft, staysm`

```c
  415:  */
  416: void Calc_Size_Spectra(MSEBoxModel *bm, int z, int id) {
  417→ 	int j, k, bin, sizeclass, nextbin, bincount, startbin, sp, nbin, nbinbig, nbinleft, staysmall, staybig, sizeclasstransit, nextbinB, startbinbig;
  418: 	double wgt, prop_bin, binbita, numbin, numbinB, prop_binB, biK_num_sizeleft, binsze;
  419: 	int pelbinbird = bm->K_num_pelbin - 4;
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
**Found at (current code):** `atindices.c` 714
**Lines at review (Oct 2026):** 691, 699, 702 vs 713

**Why it's thought to be a bug**

Vertebrates are excluded from ranking but are written, so all land in `endnums[0]`.

**Code at issue** — `atindices.c` line 714 (located by name only; check this is the right place)  
Search for: `endnums[numsort[sp]][index][z] = nums[sp][index];`

```c
  712: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  713: 		if (FunctGroupArray[sp].isVertebrate == TRUE || FunctGroupArray[sp].isMacroFauna == TRUE || FunctGroupArray[sp].isMicroFauna == TRUE) {
  714→ 			endnums[numsort[sp]][index][z] = nums[sp][index];
  715: 		}
  716: 	}
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
**Found at (current code):** `atindices.c` 826
**Lines at review (Oct 2026):** 814-819

**Why it's thought to be a bug**

Inner rank loop is unfiltered, so rank > counter-1 and `numsort` < 0, writing `endnumsbig[negative]`.

**Code at issue** — `atindices.c` line 826 (located by name only; check this is the right place)  
Search for: `endnumsbig[numsort[sp]][index][z] = globalnums[sp][index][typeIndex];`

```c
  824: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  825: 		if (FunctGroupArray[sp].isVertebrate == TRUE || FunctGroupArray[sp].isMacroFauna == TRUE) {
  826→ 			endnumsbig[numsort[sp]][index][z] = globalnums[sp][index][typeIndex];
  827: 		}
  828: 	}
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
**Found at (current code):** `atindices.c` 714
**Lines at review (Oct 2026):** 703-707, 741-746, 777-782, 815-819

**Why it's thought to be a bug**

Ties collide; `endnums` is never cleared, so stale slots remain.

**Code at issue** — `atindices.c` line 714 (located by name only; check this is the right place)  
Search for: `endnums[numsort[sp]][index][z] = nums[sp][index];`

```c
  712: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  713: 		if (FunctGroupArray[sp].isVertebrate == TRUE || FunctGroupArray[sp].isMacroFauna == TRUE || FunctGroupArray[sp].isMicroFauna == TRUE) {
  714→ 			endnums[numsort[sp]][index][z] = nums[sp][index];
  715: 		}
  716: 	}
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
**Found at (current code):** `atindices.c` 844, `atindices.c` 869
**Lines at review (Oct 2026):** 844-845, 869-870, 881-882, 900-901

**Why it's thought to be a bug**

Loops over the trait index (0=nums, 1=biom) with `numStages` bound, so single-stage groups lose biomass and >2 stages go out of bounds.

**Code at issue** — `atindices.c` line 844 (the line the review pointed to, found by its text)  
Search for: `for (chrt = 0; chrt < FunctGroupArray[sp].numStages; chrt++)`

```c
  842: 
  843: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  844→ 		for (chrt = 0; chrt < FunctGroupArray[sp].numStages; chrt++)
  845: 			nums[sp][chrt] = 0;
  846: 	}
```

**Code at issue** — `atindices.c` line 869 (the line the review pointed to, found by its text)  
Search for: `for (chrt = 0; chrt < FunctGroupArray[sp].numStages; chrt++)`

```c
  867: 	/* Don't bother checking groups as we don't care about adding 0 values */
  868: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  869→ 		for (chrt = 0; chrt < FunctGroupArray[sp].numStages; chrt++)
  870: 			globalnums[sp][chrt][0] += nums[sp][chrt];
  871: 	}
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
**Found at (current code):** `atindices.c` 906
**Lines at review (Oct 2026):** 874-925

**Why it's thought to be a bug**

`abundindex_id=1`/`bioindex_id=0`, but the stored data are 0=nums, 1=biomass.

**Code at issue** — `atindices.c` line 906 (located by name only; check this is the right place)  
Search for: `sort_values_big(bm, abundindex_id, z);`

```c
  904: 
  905: 	/* First for numbers */
  906→ 	sort_values_big(bm, abundindex_id, z);
  907: 
  908: 	/* Now for biomass */
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
**Found at (current code):** `atindices.c` 1003
**Lines at review (Oct 2026):** 1003-1004

**Why it's thought to be a bug**

`agelengthkey[k][sp][jj]`; layout is `[size][age][sp]`, as line 1004 shows. `scalarsum` is unused.

**Code at issue** — `atindices.c` line 1003  
Search for: `agelengthkey[k][sp][jj]`

```c
 1001: 							scalarsum = 0;
 1002: 							for (k = 0; k < bm->K_num_size; k++) {
 1003→ 								newnums = stockinfo[sstocknums_id][sp][z][id] * individVERTinfo[istocknums_id][sp][k][z][id] * agelengthkey[k][sp][jj][z][id];
 1004: 								scalarsum += individVERTinfo[istocknums_id][sp][k][z][id] * agelengthkey[k][jj][sp][z][id];
 1005: 								step1 += newnums;
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
**Found at (current code):** `atindices.c` 1018
**Lines at review (Oct 2026):** 1018

**Why it's thought to be a bug**

No ROUNDGUARD; can produce -1.

**Code at issue** — `atindices.c` line 1018 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `chrt = (int) (FunctGroupArray[sp].speciesParams[age_mat_id] * FunctGroupArray[sp].ageClass`

```c
 1016: 				} else {
 1017: 					step2 = 0.0;
 1018→ 					chrt = (int) (FunctGroupArray[sp].speciesParams[age_mat_id] * FunctGroupArray[sp].ageClassSize - 1);
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
**Found at (current code):** `atindices.c` 1103
**Lines at review (Oct 2026):** 1103-1130

**Why it's thought to be a bug**

PS:ZP computed as planktivore/piscivore, opposite of the doc; pd[0] includes birds and mammals.

**Code at issue** — `atindices.c` line 1103 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `if(FunctGroupArray[sp].speciesParams[flagplankfish_id])`

```c
 1101: 		if (FunctGroupArray[sp].isVertebrate == TRUE) {
 1102: 			if (FunctGroupArray[sp].speciesParams[flagdem_id] < 1) {
 1103→ 				if(FunctGroupArray[sp].speciesParams[flagplankfish_id])
 1104: 					/* Planktivorous surface fish biomass */
 1105: 					step1 += stockinfo[sbiomass_id][sp][z][id];
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
**Found at (current code):** `atindices.c` 1255
**Lines at review (Oct 2026):** 1255-1257

**Why it's thought to be a bug**

No zero-count guard; sqrt of a slightly negative value gives NaN.

**Code at issue** — `atindices.c` line 1255 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `step1 = cvphys[1][i][z] - ((cvphys[0][i][z] * cvphys[0][i][z]) / cvphys[2][i][z]);`

```c
 1253: 
 1254: 	for (i = 0; i < num_sampled_phy_id; i++) {
 1255→ 		step1 = cvphys[1][i][z] - ((cvphys[0][i][z] * cvphys[0][i][z]) / cvphys[2][i][z]);
 1256: 		step2 = step1 / (cvphys[2][i][z] - 1.0 + TINY);
 1257: 		physicalSigma[i][z] = sqrt(step2);
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
**Found at (current code):** `atindices.c` 1250
**Lines at review (Oct 2026):** 1275-1277

**Why it's thought to be a bug**

Variance = (Σx² − **mean²**)/(n−1); should be `n·mean²`. Compare Calculate_Phys_SD line 1255.

**Code at issue** — `atindices.c` line 1250 (located by name only; check this is the right place)  
Search for: `void Calculate_Phys_SD(MSEBoxModel *bm, int z) {`

```c
 1248:  *
 1249:  */
 1250→ void Calculate_Phys_SD(MSEBoxModel *bm, int z) {
 1251: 	int i;
 1252: 	double step1, step2;
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
**Found at (current code):** `atindices.c` 1316, `atindices.c` 1360
**Lines at review (Oct 2026):** 1316-1318

**Why it's thought to be a bug**

`avgtl` is `[8][nfzones+2]` but reset only to `z < nfzones+1`, so `[nfzones+1]` accumulates forever.

**Code at issue** — `atindices.c` line 1316  
Search for: `z < bm->nfzones + 1`

```c
 1314: 	}
 1315: 
 1316→ 	for (z = 0; z < bm->nfzones + 1; z++)
 1317: 		for (bb = 0; bb < 8; bb++)
 1318: 			avgtl[bb][z] = 0.0;
```

**Code at issue** — `atindices.c` line 1360  
Search for: `[bm->nfzones + 1]`

```c
 1358: 		avgtl[4][z] += catches * samplearea[bb] / (totcatch + TINY);
 1359: 		avgtl[4][bm->nfzones] += catches * totalarea[bb] / (totcatch + TINY);
 1360→ 		avgtl[4][bm->nfzones + 1] += totcatchA * samplearea[bb] / (totcatch + TINY);
 1361: 		avgtl[5][z] += catchesST * samplearea[bb] / (totcatch + TINY);
 1362: 		avgtl[5][bm->nfzones] += catchesST * totalarea[bb] / (totcatch + TINY);
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
**Found at (current code):** `atindices.c` 1362
**Lines at review (Oct 2026):** 1360-1455

**Why it's thought to be a bug**

Whole-area columns are weighted by `samplearea` (sums to 1 per zone), giving ≈ nfzones × TL.

**Code at issue** — `atindices.c` line 1362 (the line the review pointed to, found by its text)  
Search for: `avgtl[5][bm->nfzones] += catchesST * totalarea[bb] / (totcatch + TINY);`

```c
 1360: 		avgtl[4][bm->nfzones + 1] += totcatchA * samplearea[bb] / (totcatch + TINY);
 1361: 		avgtl[5][z] += catchesST * samplearea[bb] / (totcatch + TINY);
 1362→ 		avgtl[5][bm->nfzones] += catchesST * totalarea[bb] / (totcatch + TINY);
 1363: 		avgtl[5][bm->nfzones + 1] += totcatchAST * samplearea[bb] / (totcatch + TINY);
 1364: 
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
**Found at (current code):** `atindices.c` 1346
**Lines at review (Oct 2026):** 1346-1348 vs 1370-1376

**Why it's thought to be a bug**

Vertebrate catch is divided by zonearea; invertebrate catch is not.

**Code at issue** — `atindices.c` line 1346 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `catches += stockinfo[stotcatch_id][sp][z][id] * sample * tl[a][sp][stage] / (zonearea[z] +`

```c
 1344: 						sample = pop_fraction[1][sp][z];
 1345: 
 1346→ 					catches += stockinfo[stotcatch_id][sp][z][id] * sample * tl[a][sp][stage] / (zonearea[z] + TINY);
 1347: 					catchesST += stockinfo[stotcatch_id][sp][z][id] * sample * tl[a + 3][sp][stage] / (zonearea[z] + TINY);
 1348: 					totcatch += stockinfo[stotcatch_id][sp][z][id] * sample / (zonearea[z] + TINY);
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
**Found at (current code):** `atindices.c` 1547
**Lines at review (Oct 2026):** 1504-1505

**Why it's thought to be a bug**

"Static" block uses `tl[a]` (dynamic); the invertebrate equivalent uses `tl[a+3]`.

**Code at issue** — `atindices.c` line 1547  
Search for: `tl[a + 3]`

```c
 1545: 
 1546: 					/* Zone specific and total, static trophic level through time */
 1547→ 					tlindx = (int) (floor((tl[a + 3][sp][stage] - 1.0) / 0.5));
 1548: 					if (tlindx < 0)
 1549: 						tlindx = 0;
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
**Found at (current code):** `atindices.c` 1501
**Lines at review (Oct 2026):** 1501, 1512, 1543, 1552

**Why it's thought to be a bug**

Zone-level catch is added once per box.

**Code at issue** — `atindices.c` line 1501 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `trophspect[tlindx][0][z] += stockinfo[stotcatch_id][sp][z][id] * sample;`

```c
 1499: 						if (tlindx >= bm->K_num_trophbin)
 1500: 							tlindx = bm->K_num_trophbin - 1;
 1501→ 						trophspect[tlindx][0][z] += stockinfo[stotcatch_id][sp][z][id] * sample;
 1502: 						trophspect[tlindx][0][bm->nfzones] += stockinfo[stotcatch_id][sp][z][id] * sample * samplearea[bb];
 1503: 
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
**Found at (current code):** `atindices.c` 1651
**Lines at review (Oct 2026):** 1638-1665

**Why it's thought to be a bug**

Whole-area functional diversity omits the species count included per zone.

**Code at issue** — `atindices.c` line 1651 (located by name only; check this is the right place)  
Search for: `divindx[0][bm->nfzones]++;`

```c
 1649: 		for (z = 0; z < bm->nfzones; z++) {
 1650: 			if (divsp[j][z] > 0) {
 1651→ 				divindx[0][bm->nfzones]++;
 1652: 				break;
 1653: 			}
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
