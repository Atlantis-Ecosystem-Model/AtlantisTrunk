# atassess/atattribute.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **16 findings** (severity H: 2; confidence H and severity H: 0; on the priority list: 1).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [ATT-01](#att-01) | 166, 188 | M/M | averaging | `oldbaby` is assigned with `=` each step but divided by numsteps; birds are double-added at line 390. |  | [ ] |
| [ATT-02](#att-02) | 153 | H/M | wrong variable | Time column `nt = dt/86400` (step length) instead of year (`dayt/365`, as in atsample.c:59). |  | [ ] |
| [ATT-03](#att-03) | 245 | M/H | missing normalisation | Invertebrate true values are never divided by numsteps (survey divides them). |  | [ ] |
| [ATT-04](#att-04) | 246 | M/L | range | Loop `light_wc_id..<don_wc_id` skips DON, DIN and Temp. |  | [ ] |
| [ATT-05](#att-05) | 269 | M/M | wrong slot | Reads `biolbiom[..][WC]` only; sediment groups are stored in SED (see DAT-1). |  | [ ] |
| [ATT-06](#att-06) | 398 | M/M | wrong index | Bird catch numbers always go into bin 0, not `lbin`. |  | [ ] |
| [ATT-07](#att-07) | 370, 469 | M/M | wrong variable | Stage switch and age use `chrt` (cohort×gene). |  | [ ] |
| [ATT-08](#att-08) | 469, 858 | H/M | off-by-one | `round(x+0.5)` shifts age up by 1 (same as SRV-08). |  | [ ] |
| [ATT-09](#att-09) | 460 | L/L | off-by-one | `ceil(ageClassSize+0.5)` gives one too many sub-ages; the remainder is dropped. |  | [ ] |
| [ATT-10](#att-10) | 383 | M/M | accumulator mixing | Bird raw counts are added onto already-normalised fractions in later rounds. |  | [ ] |
| [ATT-11](#att-11) | 490 | M/M | formula | Production multiplied by stock numbers again (already included in atdatareading.c:536). |  | [ ] |
| [ATT-12](#att-12) | 485 | L/M | inconsistency | Bird prod/eat are not gated by `sample_now`. |  | [ ] |
| [ATT-13](#att-13) ⭐ | 705 | M/H | units | `bdiscards` holds numbers (atdatareading.c:575) but is divided by weight, so it is ≈0. |  | [ ] |
| [ATT-14](#att-14) | 757 | M/M | double count | Bird catch numbers are added twice. |  | [ ] |
| [ATT-15](#att-15) | 727 | M/L | condition | `else` also catches invertebrates. |  | [ ] |
| [ATT-16](#att-16) | 929 | M/M | formula | Biomass per bin = total biomass × individual weight (mass²). |  | [ ] |

---

## ATT-01

**averaging** · Conf M / Sev M  
**Found at (current code):** `atattribute.c` 166, `atattribute.c` 188
**Lines at review (Oct 2026):** 166, 188, 390, 1011-1013

**Why it's thought to be a bug**

`oldbaby` is assigned with `=` each step but divided by numsteps; birds are double-added at line 390.

**Code at issue** — `atattribute.c` line 166 (the line the review pointed to, found by its text)  
Search for: `oldbaby[sp][b][attrib_id] = stockinfo[sstocknums_id][sp][b][attrib_id] * individVERTinfo[i`

```c
  164: 				if (FunctGroupArray[sp].groupType == BIRD) {
  165: 					/* Seabirds and penguins */
  166→ 					oldbaby[sp][b][attrib_id] = stockinfo[sstocknums_id][sp][b][attrib_id] * individVERTinfo[istocknums_id][sp][0][b][attrib_id];
  167: 				}
  168: 				if (FunctGroupArray[sp].groupType == MAMMAL) {
```

**Code at issue** — `atattribute.c` line 188 (the line the review pointed to, found by its text)  
Search for: `oldbaby[sp][b][attrib_id] = step1;`

```c
  186: 						}
  187: 					}
  188→ 					oldbaby[sp][b][attrib_id] = step1;
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
**Found at (current code):** `atattribute.c` 153
**Lines at review (Oct 2026):** 153, 220

**Why it's thought to be a bug**

Time column `nt = dt/86400` (step length) instead of year (`dayt/365`, as in atsample.c:59).

**Code at issue** — `atattribute.c` line 153 (closest match to the quoted code; wording differs slightly)  
Search for: `nt = (int) (bm->dt / 86400.0);`

```c
  151: 	double step1, newnums, scalarsum;
  152: 
  153→ 	nt = (int) (bm->dt / 86400.0);
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
**Found at (current code):** `atattribute.c` 245
**Lines at review (Oct 2026):** 245-289

**Why it's thought to be a bug**

Invertebrate true values are never divided by numsteps (survey divides them).

**Code at issue** — `atattribute.c` line 245 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `samplephys[0][z][attrib_id] += sample * samplearea[bb];`

```c
  243: 		/* Physical characteristics */
  244: 		sample = phys[salinity_id][b];
  245→ 		samplephys[0][z][attrib_id] += sample * samplearea[bb];
  246: 		for (i = light_wc_id; i < don_wc_id; i++) {
  247: 			sample = phys[i][b];
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
**Found at (current code):** `atattribute.c` 246
**Lines at review (Oct 2026):** 246-249

**Why it's thought to be a bug**

Loop `light_wc_id..<don_wc_id` skips DON, DIN and Temp.

**Code at issue** — `atattribute.c` line 246 (closest match to the quoted code; wording differs slightly)  
Search for: `for (i = light_wc_id; i < don_wc_id; i++) {`

```c
  244: 		sample = phys[salinity_id][b];
  245: 		samplephys[0][z][attrib_id] += sample * samplearea[bb];
  246→ 		for (i = light_wc_id; i < don_wc_id; i++) {
  247: 			sample = phys[i][b];
  248: 			samplephys[i - 2][z][attrib_id] += sample * samplearea[bb];
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
**Found at (current code):** `atattribute.c` 269
**Lines at review (Oct 2026):** 268-270

**Why it's thought to be a bug**

Reads `biolbiom[..][WC]` only; sediment groups are stored in SED (see DAT-1).

**Code at issue** — `atattribute.c` line 269 (closest match to the quoted code; wording differs slightly)  
Search for: `sample = biolbiom[sp][b][WC];`

```c
  267: 
  268: 				} else {
  269→ 					sample = biolbiom[sp][b][WC];
  270: 					samplebiom[sp][z][attrib_id] += sample * samplearea[bb];
  271: 				}
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
**Found at (current code):** `atattribute.c` 398
**Lines at review (Oct 2026):** 394-395

**Why it's thought to be a bug**

Bird catch numbers always go into bin 0, not `lbin`.

**Code at issue** — `atattribute.c` line 398 (located by name only; check this is the right place)  
Search for: `individVERTinfo[ibiomass_id][sp][lbin][z][attrib_id] += rawW * rawN / (bm->boxes[b].area +`

```c
  396: 
  397: 						/* Biomass */
  398→ 						individVERTinfo[ibiomass_id][sp][lbin][z][attrib_id] += rawW * rawN / (bm->boxes[b].area + TINY);
  399: 
  400: 						/* Discards */
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
**Found at (current code):** `atattribute.c` 370, `atattribute.c` 469
**Lines at review (Oct 2026):** 370-377, 469, 858

**Why it's thought to be a bug**

Stage switch and age use `chrt` (cohort×gene).

**Code at issue** — `atattribute.c` line 370 (the line the review pointed to, found by its text)  
Search for: `switch (chrt) {`

```c
  368: 					for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
  369: 						/* Get information for nestlings, newly fledged and adult seabirds */
  370→ 						switch (chrt) {
  371: 						case 0: /* Nestlings */
  372: 						case 1: /* Fledglings */
```

**Code at issue** — `atattribute.c` line 469 (the line the review pointed to, found by its text)  
Search for: `cage = (int) (round((chrt + v) * FunctGroupArray[sp].ageClassSize + 0.5));`

```c
  467: 
  468: 						//TODO Check with beth about changing to a round instead of a floor.
  469→ 						cage = (int) (round((chrt + v) * FunctGroupArray[sp].ageClassSize + 0.5));
  470: 
  471: 						ninsubage = nraw / nsubages;
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
**Found at (current code):** `atattribute.c` 469, `atattribute.c` 858
**Lines at review (Oct 2026):** 469, 858

**Why it's thought to be a bug**

`round(x+0.5)` shifts age up by 1 (same as SRV-08).

**Code at issue** — `atattribute.c` line 469 (the line the review pointed to, found by its text)  
Search for: `cage = (int) (round((chrt + v) * FunctGroupArray[sp].ageClassSize + 0.5));`

```c
  467: 
  468: 						//TODO Check with beth about changing to a round instead of a floor.
  469→ 						cage = (int) (round((chrt + v) * FunctGroupArray[sp].ageClassSize + 0.5));
  470: 
  471: 						ninsubage = nraw / nsubages;
```

**Code at issue** — `atattribute.c` line 858 (the line the review pointed to, found by its text)  
Search for: `cage = (int) (round((chrt + drandom(0.0, 1.0)) * FunctGroupArray[sp].ageClassSize + 0.5));`

```c
  856: 						/* Assume animal uniformly distributed in age class - may not be right! */
  857: 						//TODO Check with beth about changing to a round instead of a floor.
  858→ 						cage = (int) (round((chrt + drandom(0.0, 1.0)) * FunctGroupArray[sp].ageClassSize + 0.5));
  859: 						ninsubage = nraw / nsubages;
  860: 						Length_Age_Key(bm, z, indx, cmaxsize, lngth, cage, cmaxage, ninsubage, attrib_id);
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
**Found at (current code):** `atattribute.c` 460
**Lines at review (Oct 2026):** 460, 852

**Why it's thought to be a bug**

`ceil(ageClassSize+0.5)` gives one too many sub-ages; the remainder is dropped.

**Code at issue** — `atattribute.c` line 460 (closest match to the quoted code; wording differs slightly)  
Search for: `nsubages = (int) (ceil(FunctGroupArray[sp].ageClassSize + 0.5));`

```c
  458: 					Sort_Length_Weight(bm, 0, 1, sizestocknums_id, z, indx, fishery_id, lngth, nraw, wgt, cmaxsize, attrib_id, &ltc, ofp);
  459: 
  460→ 					nsubages = (int) (ceil(FunctGroupArray[sp].ageClassSize + 0.5));
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
**Found at (current code):** `atattribute.c` 383
**Lines at review (Oct 2026):** 383-402, 418-435

**Why it's thought to be a bug**

Bird raw counts are added onto already-normalised fractions in later rounds.

**Code at issue** — `atattribute.c` line 383 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `individVERTinfo[iweight_id][sp][lbin][z][attrib_id] += raw;`

```c
  381: 						raw = biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b];
  382: 						rawW = raw;
  383→ 						individVERTinfo[iweight_id][sp][lbin][z][attrib_id] += raw;
  384: 
  385: 						/* Numbers in stock */
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
**Found at (current code):** `atattribute.c` 490
**Lines at review (Oct 2026):** 490, 505, 869 (+atsurvey 535/699)

**Why it's thought to be a bug**

Production multiplied by stock numbers again (already included in atdatareading.c:536).

**Code at issue** — `atattribute.c` line 490 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `stockinfo[sprod_id][sp][z][attrib_id] += raw * samplearea[bb] * ceil(biolVERTinfo[bstocknu`

```c
  488: 						raw = biolVERTinfo[bprod_id][sp][chrt][b];
  489: 
  490→ 						stockinfo[sprod_id][sp][z][attrib_id] += raw * samplearea[bb] * ceil(biolVERTinfo[bstocknums_id][sp][chrt][b]) / (vol + TINY);
  491: 
  492: 						/* Mammal Consumption */
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
**Found at (current code):** `atattribute.c` 485
**Lines at review (Oct 2026):** 500-512 vs 485, 866

**Why it's thought to be a bug**

Bird prod/eat are not gated by `sample_now`.

**Code at issue** — `atattribute.c` line 485 (located by name only; check this is the right place)  
Search for: `if (bm->sample_now) {`

```c
  483: 					Sort_Length_Weight(bm, 0, 1, sizediscardnums_id, z, indx, fishery_id, lngth, nraw, wgt, cmaxsize, attrib_id, &ltc, ofp);
  484: 
  485→ 					if (bm->sample_now) {
  486: 
  487: 						/* Mammal Production */
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
**Found at (current code):** `atattribute.c` 705
**Lines at review (Oct 2026):** 705 (+atsurvey.c:600)

**Why it's thought to be a bug**

`bdiscards` holds numbers (atdatareading.c:575) but is divided by weight, so it is ≈0.

**Code at issue** — `atattribute.c` line 705 (the line the review pointed to, found by its text)  
Search for: `raw = biolVERTinfo[bdiscards_id][sp][chrt][b] / (wgt + TINY);`

```c
  703: 					Sort_Length_Weight(bm, 1, 1, sizecatchnums_id, z, sp, fishery_id, lngth, nraw, wgt, cmaxsize, attrib_id, &ltc, ofp);
  704: 
  705→ 					raw = biolVERTinfo[bdiscards_id][sp][chrt][b] / (wgt + TINY);
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
**Found at (current code):** `atattribute.c` 757
**Lines at review (Oct 2026):** 756-765 (+429)

**Why it's thought to be a bug**

Bird catch numbers are added twice.

**Code at issue** — `atattribute.c` line 757 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `for (i = 0; i < 3; i++) {`

```c
  755: 					}
  756: 				} else {
  757→ 					for (i = 0; i < 3; i++) {
  758: 						/* Total number of seabirds caught */
  759: 						stockinfo[scatchnums_id][sp][z][attrib_id] += stockinfo[stotcatch_id][sp][z][attrib_id]
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
**Found at (current code):** `atattribute.c` 727
**Lines at review (Oct 2026):** 727, 756

**Why it's thought to be a bug**

`else` also catches invertebrates.

**Code at issue** — `atattribute.c` line 727 (the line the review pointed to, found by its text)  
Search for: `if (FunctGroupArray[sp].isVertebrate == TRUE && FunctGroupArray[sp].groupType != BIRD) {`

```c
  725: 			}
  726: 			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  727→ 				if (FunctGroupArray[sp].isVertebrate == TRUE && FunctGroupArray[sp].groupType != BIRD) {
  728: 					for (i = 0; i < bm->K_num_size; i++) {
  729: 						if(FunctGroupArray[sp].groupType != MAMMAL){
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
**Found at (current code):** `atattribute.c` 929
**Lines at review (Oct 2026):** 934-935

**Why it's thought to be a bug**

Biomass per bin = total biomass × individual weight (mass²).

**Code at issue** — `atattribute.c` line 929 (located by name only; check this is the right place)  
Search for: `individVERTinfo[istocknums_id][sp][i][z][attrib_id] = individVERTinfo[istocknums_id][sp][i`

```c
  927: 					/* Rescale size distribution so it sums to one */
  928: 					for (i = 0; i < bm->K_num_size; i++) {
  929→ 						individVERTinfo[istocknums_id][sp][i][z][attrib_id] = individVERTinfo[istocknums_id][sp][i][z][attrib_id]
  930: 								/ (totnums[totstocknums_id][sp][z] + TINY);
  931: 
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
