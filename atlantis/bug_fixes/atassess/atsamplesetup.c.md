# atassess/atsamplesetup.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **7 findings** (severity H: 1; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SSU-1](#ssu-1) | 468 | M/H | missing branch | `flagrandom=1, flaggen=0` leaves `rand`/`tassPatchy` at 0, so diet sampling never advances and assessment samp… |  | [ ] |
| [SSU-2](#ssu-2) | 472 | L/M | formula | `tassPatchy = tassessinc / freq` is used as a multiplier, giving tassessinc²/freq. |  | [ ] |
| [SSU-3](#ssu-3) | 383 | H/L | units | `nkk = tstop/365` but tstop is in seconds, so allocation is 86400× too big. |  | [ ] |
| [SSU-4](#ssu-4) | 398 | M/M | alloc vs use | `NResult` middle dimension = cohorts only; it is indexed up to `num_est_prm` (NEst uses max). |  | [ ] |
| [SSU-5](#ssu-5) | 433 | M/L | always false | `numsteps < 0` can never be true; probably meant `< 1`. |  | [ ] |
| [SSU-6](#ssu-6) | 51 | H/L | NULL deref | `*(strstr(name, ".prm")) = 0` with no NULL check. |  | [ ] |
| [SSU-7](#ssu-7) | 748 | L/L | copy-paste | DINOFLAG and ICE_MIXOTROPHS use the detritus flag; SED_EP_OTHER mixes epi1 and epi2 errors. |  | [ ] |

---

## SSU-1

**missing branch** · Conf M / Sev H  
**Found at (current code):** `atsamplesetup.c` 468
**Lines at review (Oct 2026):** 467-500

**Why it's thought to be a bug**

`flagrandom=1, flaggen=0` leaves `rand`/`tassPatchy` at 0, so diet sampling never advances and assessment sampling runs daily.

**Code at issue** — `atsamplesetup.c` line 468 (the line the review pointed to, found by its text)  
Search for: `if (bm->flagrandom == 1) {`

```c
  466: 	/* Handle random number generation */
  467: 
  468→ 	if (bm->flagrandom == 1) {
  469: 		if (bm->flaggen == 1) {
  470: 			/* Generate and record random numbers */
```

**Suggested fix (from review):** read stored values, or quit

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

## SSU-2

**formula** · Conf L / Sev M  
**Found at (current code):** `atsamplesetup.c` 472
**Lines at review (Oct 2026):** 471-472

**Why it's thought to be a bug**

`tassPatchy = tassessinc / freq` is used as a multiplier, giving tassessinc²/freq.

**Code at issue** — `atsamplesetup.c` line 472 (the line the review pointed to, found by its text)  
Search for: `bm->tassPatchy[b][tass_id] = bm->tassessinc / drandom(bm->minfreq, bm->maxfreq);`

```c
  470: 			/* Generate and record random numbers */
  471: 			for (b = 0; b < numsamples; b++) {
  472→ 				bm->tassPatchy[b][tass_id] = bm->tassessinc / drandom(bm->minfreq, bm->maxfreq);
  473: 				bm->tassPatchy[b][tasseat_id] = bm->teatassessinc / drandom(bm->minfreq, bm->maxfreq);
  474: 			}
```

**Suggested fix (from review):** `freq / tassessinc`

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

## SSU-3

**units** · Conf H / Sev L  
**Found at (current code):** `atsamplesetup.c` 383
**Lines at review (Oct 2026):** 382-386

**Why it's thought to be a bug**

`nkk = tstop/365` but tstop is in seconds, so allocation is 86400× too big.

**Code at issue** — `atsamplesetup.c` line 383 (closest match to the quoted code; wording differs slightly)  
Search for: `nkk = (int) floor(ROUNDGUARD + (floor(bm->tstop / 365.0)));`

```c
  381: 	PP = (double **) alloc2d(2, 3);   // Nov 2026: simplex for 2 parameter linear regression (was never allocated)
  382: 
  383→ 	nkk = (int) floor(ROUNDGUARD + (floor(bm->tstop / 365.0)));
  384: 	if (nkk < 3)
  385: 		nkk = 3;   // Nov 2026: YY also holds the 3 simplex values for the 2 parameter linear regression (was 1)
```

**Suggested fix (from review):** divide by 86400*365

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

## SSU-4

**alloc vs use** · Conf M / Sev M  
**Found at (current code):** `atsamplesetup.c` 398
**Lines at review (Oct 2026):** 397

**Why it's thought to be a bug**

`NResult` middle dimension = cohorts only; it is indexed up to `num_est_prm` (NEst uses max).

**Code at issue** — `atsamplesetup.c` line 398 (the line the review pointed to, found by its text)  
Search for: `NResult = (double ***) alloc3d(nk, bm->K_num_max_cohort * bm->K_num_max_genetypes, nyr);`

```c
  396: 
  397: 	nk = max_nbs + 2; // One spare in case of overlaps
  398→ 	NResult = (double ***) alloc3d(nk, bm->K_num_max_cohort * bm->K_num_max_genetypes, nyr);
  399: 	ResultToSort = (double *) alloc1d(nk);
  400: 	ResultSorted = (double *) alloc1d(nk);
```

**Suggested fix (from review):** max(cohorts, num_est_prm)

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

## SSU-5

**always false** · Conf M / Sev L  
**Found at (current code):** `atsamplesetup.c` 433
**Lines at review (Oct 2026):** 432-435

**Why it's thought to be a bug**

`numsteps < 0` can never be true; probably meant `< 1`.

**Code at issue** — `atsamplesetup.c` line 433  
Search for: `numsteps < 0`

```c
  431: 	//fprintf(llogfp, "tassess: %e, numsteps: %d, tassessinc: %e, ratio: %e\n", bm->tassess, numsteps, bm->tassessinc, 364.0 / bm->tassessinc);
  432: 
  433→ 	if (numsteps < 0) {
  434: 		numsteps_orig = 0;
  435: 		numsteps = 1;
```

**Suggested fix (from review):** `< 1`

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

## SSU-6

**NULL deref** · Conf H / Sev L  
**Found at (current code):** `atsamplesetup.c` 51
**Lines at review (Oct 2026):** 129-131

**Why it's thought to be a bug**

`*(strstr(name, ".prm")) = 0` with no NULL check.

**Code at issue** — `atsamplesetup.c` line 51 (located by name only; check this is the right place)  
Search for: `Changed the assessment xml file to use the management input file name as a guide.`

```c
   49: 
   50:  02-11-2009 Bec Gorton
   51→  Changed the assessment xml file to use the management input file name as a guide.
   52: 
   53:  09-03-2010 Bec Gorton
```

**Suggested fix (from review):** check

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

## SSU-7

**copy-paste** · Conf L / Sev L  
**Found at (current code):** `atsamplesetup.c` 748
**Lines at review (Oct 2026):** 738, 746, 768

**Why it's thought to be a bug**

DINOFLAG and ICE_MIXOTROPHS use the detritus flag; SED_EP_OTHER mixes epi1 and epi2 errors.

**Code at issue** — `atsamplesetup.c` line 748 (located by name only; check this is the right place)  
Search for: `Set_Species_Biased_Value(spErrorStructure, SED_EP_OTHER, flagepibiom, k_avgepif1, k_varepi`

```c
  746: 
  747: 	Set_Species_Biased_Value(spErrorStructure, SED_EP_FF, flagepibiom, k_avgepif1, k_varepif1, 0, 0, k_avgepi1p, k_varepi1p, k_avgepi1eat, k_varepi1eat);
  748→ 	Set_Species_Biased_Value(spErrorStructure, SED_EP_OTHER, flagepibiom, k_avgepif1, k_varepif1, 0, 0, k_avgepi2p, k_varepi2p, k_avgepi2eat, k_varepi2eat);
  749: 	Set_Species_Biased_Value(spErrorStructure, CORAL, flagepibiom, k_avgepif1, k_varepif1, 0, 0, k_avgepi1p, k_varepi1p, k_avgepi1eat, k_varepi1eat);
  750:     Set_Species_Biased_Value(spErrorStructure, SPONGE, flagepibiom, k_avgepif1, k_varepif1, 0, 0, k_avgepi1p, k_varepi1p, k_avgepi1eat, k_varepi1eat);
```

**Suggested fix (from review):** confirm

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
