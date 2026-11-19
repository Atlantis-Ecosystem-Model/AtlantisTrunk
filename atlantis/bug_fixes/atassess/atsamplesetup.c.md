# atassess/atsamplesetup.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **7 findings** (severity H: 1; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SSU-1](#ssu-1) | 468-501 | M/H | missing branch | `flagrandom=1, flaggen=0` leaves `rand`/`tassPatchy` at 0, so diet sampling never advances and assessment samp… |  | [ ] |
| [SSU-2](#ssu-2) | 472-473 | L/M | formula | `tassPatchy = tassessinc / freq` is used as a multiplier, giving tassessinc²/freq. |  | [ ] |
| [SSU-3](#ssu-3) | 383-387 | H/L | units | `nkk = tstop/365` but tstop is in seconds, so allocation is 86400× too big. |  | [ ] |
| [SSU-4](#ssu-4) | 398 | M/M | alloc vs use | `NResult` middle dimension = cohorts only; it is indexed up to `num_est_prm` (NEst uses max). |  | [ ] |
| [SSU-5](#ssu-5) | 433-436 | M/L | always false | `numsteps < 0` can never be true; probably meant `< 1`. |  | [ ] |
| [SSU-6](#ssu-6) | 129-131 | H/L | NULL deref | `*(strstr(name, ".prm")) = 0` with no NULL check. |  | [ ] |
| [SSU-7](#ssu-7) | 740, 748, 770 | L/L | copy-paste | DINOFLAG and ICE_MIXOTROPHS use the detritus flag; SED_EP_OTHER mixes epi1 and epi2 errors. |  | [ ] |

---

## SSU-1

**missing branch** · Conf M / Sev H  
**Lines (current code):** 468-501  
**Lines at review (Oct 2026):** 467-500

**Why it's thought to be a bug**

`flagrandom=1, flaggen=0` leaves `rand`/`tassPatchy` at 0, so diet sampling never advances and assessment sampling runs daily.

**Current code** (`atassess/atsamplesetup.c` lines 466–503)

```c
  466: 	/* Handle random number generation */
  467: 
  468: 	if (bm->flagrandom == 1) {
  469: 		if (bm->flaggen == 1) {
  470: 			/* Generate and record random numbers */
  471: 			for (b = 0; b < numsamples; b++) {
  472: 				bm->tassPatchy[b][tass_id] = bm->tassessinc / drandom(bm->minfreq, bm->maxfreq);
  473: 				bm->tassPatchy[b][tasseat_id] = bm->teatassessinc / drandom(bm->minfreq, bm->maxfreq);
  474: 			}
  475: 			for (i = 0; i < bm->K_num_tot_sp; i++) {
  476: 				if (FunctGroupArray[i].isFished == TRUE) {
  477: 					for (b = 0; b < numyears; b++) {
  478: 						bm->rand[i][startN_id][b] = Assess_Add_Error(bm, flagprms, 1.0, k_avgprms, k_varprms);
  479: 						bm->rand[i][calcF_id][b] = Assess_Add_Error(bm, flagprms, 1.0, k_avgprms, k_varprms);
  480: 						bm->rand[i][calcM_id][b] = Assess_Add_Error(bm, flagprms, 1.0, k_avgprms, k_varprms);
  481: 					}
  482: 				}
  483: 			}
  484: 			Write_Random_Numbers(bm, numyears, numsamples);
  485: 		}
  486: 	} else {
  487: 		/* No randomisation so make all scalars 1.0 */
  488: 		for (b = 0; b < numsamples; b++) {
  489: 			bm->tassPatchy[b][tass_id] = 1.0;
  490: 			bm->tassPatchy[b][tasseat_id] = 1.0;
  491: 		}
  492: 		for (i = 0; i < bm->K_num_tot_sp; i++) {
  493: 			if (FunctGroupArray[i].isFished == TRUE) {
  494: 				for (b = 0; b < numyears; b++) {
  495: 					bm->rand[i][startN_id][b] = 1.0;
  496: 					bm->rand[i][calcF_id][b] = 1.0;
  497: 					bm->rand[i][calcM_id][b] = 1.0;
  498: 				}
  499: 			}
  500: 		}
  501: 	}
  502: 
  503: 	/* No patch sampling so set scalar to 1.0 */
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
**Lines (current code):** 472-473  
**Lines at review (Oct 2026):** 471-472

**Why it's thought to be a bug**

`tassPatchy = tassessinc / freq` is used as a multiplier, giving tassessinc²/freq.

**Current code** (`atassess/atsamplesetup.c` lines 470–475)

```c
  470: 			/* Generate and record random numbers */
  471: 			for (b = 0; b < numsamples; b++) {
  472: 				bm->tassPatchy[b][tass_id] = bm->tassessinc / drandom(bm->minfreq, bm->maxfreq);
  473: 				bm->tassPatchy[b][tasseat_id] = bm->teatassessinc / drandom(bm->minfreq, bm->maxfreq);
  474: 			}
  475: 			for (i = 0; i < bm->K_num_tot_sp; i++) {
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
**Lines (current code):** 383-387  
**Lines at review (Oct 2026):** 382-386

**Why it's thought to be a bug**

`nkk = tstop/365` but tstop is in seconds, so allocation is 86400× too big.

**Current code** (`atassess/atsamplesetup.c` lines 381–389)

```c
  381: 	PP = (double **) alloc2d(2, 3);   // Nov 2026: simplex for 2 parameter linear regression (was never allocated)
  382: 
  383: 	nkk = (int) floor(ROUNDGUARD + (floor(bm->tstop / 365.0)));
  384: 	if (nkk < 3)
  385: 		nkk = 3;   // Nov 2026: YY also holds the 3 simplex values for the 2 parameter linear regression (was 1)
  386: 	XX = (double *) alloc1d(nkk);
  387: 	YY = (double *) alloc1d(nkk);
  388: 
  389: 	max_nbs = 0;
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
**Lines (current code):** 398  
**Lines at review (Oct 2026):** 397

**Why it's thought to be a bug**

`NResult` middle dimension = cohorts only; it is indexed up to `num_est_prm` (NEst uses max).

**Current code** (`atassess/atsamplesetup.c` lines 396–400)

```c
  396: 
  397: 	nk = max_nbs + 2; // One spare in case of overlaps
  398: 	NResult = (double ***) alloc3d(nk, bm->K_num_max_cohort * bm->K_num_max_genetypes, nyr);
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
**Lines (current code):** 433-436  
**Lines at review (Oct 2026):** 432-435

**Why it's thought to be a bug**

`numsteps < 0` can never be true; probably meant `< 1`.

**Current code** (`atassess/atsamplesetup.c` lines 431–438)

```c
  431: 	//fprintf(llogfp, "tassess: %e, numsteps: %d, tassessinc: %e, ratio: %e\n", bm->tassess, numsteps, bm->tassessinc, 364.0 / bm->tassessinc);
  432: 
  433: 	if (numsteps < 0) {
  434: 		numsteps_orig = 0;
  435: 		numsteps = 1;
  436: 	}
  437: 
  438: 	if (bm->flagcont && (numsteps > 0) && ((numsteps * bm->tassessinc) > 364)) {
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
**Lines (current code):** 129-131  
**Lines at review (Oct 2026):** 129-131

**Why it's thought to be a bug**

`*(strstr(name, ".prm")) = 0` with no NULL check.

**Current code** (`atassess/atsamplesetup.c` lines 127–133)

```c
  127: 
  128: 	/* If the input file is a prm file convert it to the XML format */
  129: 	if(strstr(bm->assessprmIfname, ".xml") == NULL){
  130: 		/* Build the converted filename */
  131: 		*(strstr(convertedXMLFileName, ".prm")) = '\0';
  132: 		strcat(convertedXMLFileName, ".xml");
  133: 
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
**Lines (current code):** 740, 748, 770  
**Lines at review (Oct 2026):** 738, 746, 768

**Why it's thought to be a bug**

DINOFLAG and ICE_MIXOTROPHS use the detritus flag; SED_EP_OTHER mixes epi1 and epi2 errors.

**Current code** (`atassess/atsamplesetup.c` lines 738–772)

```c
  738: 	Set_Species_Biased_Value(spErrorStructure, SM_PHY, flaginvpbiom, k_avgphyts, k_varphyts, 0, 0, k_avgpelpp, k_varpelpp, 0, 0);
  739: 	// TODO: no values read in for DF and its not handled in code. Need to check with Beth about this.
  740: 	Set_Species_Biased_Value(spErrorStructure, DINOFLAG, flagdetbiom, k_avgphyts, k_varphyts, 0, 0, k_avgpelpp, k_varpelpp, 0, 0);
  741: 
  742: 	Set_Species_Biased_Value(spErrorStructure, SM_ZOO, flaginvpbiom, k_avgzoops, k_varzoops, 0, 0, k_avgzoopp, k_varzoopp, k_avgzoopeat, k_varzoopeat);
  743: 	Set_Species_Biased_Value(spErrorStructure, MED_ZOO, flaginvpbiom, k_avgzoopl, k_varzoopl, 0, 0, k_avgzoopp, k_varzoopp, k_avgzoopeat, k_varzoopeat);
  744: 	Set_Species_Biased_Value(spErrorStructure, LG_ZOO, flaginvpbiom, k_avgzoopl, k_varzoopl, 0, 0, k_avgzoopp, k_varzoopp, k_avgzoopeat, k_varzoopeat);
  745: 	Set_Species_Biased_Value(spErrorStructure, CEP, flaginvpbiom, k_avgcep, k_varcep, 0, 0, k_avgcepp, k_varcepp, k_avgcepeat, k_varcepeat);
  746: 
  747: 	Set_Species_Biased_Value(spErrorStructure, SED_EP_FF, flagepibiom, k_avgepif1, k_varepif1, 0, 0, k_avgepi1p, k_varepi1p, k_avgepi1eat, k_varepi1eat);
  748: 	Set_Species_Biased_Value(spErrorStructure, SED_EP_OTHER, flagepibiom, k_avgepif1, k_varepif1, 0, 0, k_avgepi2p, k_varepi2p, k_avgepi2eat, k_varepi2eat);
  749: 	Set_Species_Biased_Value(spErrorStructure, CORAL, flagepibiom, k_avgepif1, k_varepif1, 0, 0, k_avgepi1p, k_varepi1p, k_avgepi1eat, k_varepi1eat);
  750:     Set_Species_Biased_Value(spErrorStructure, SPONGE, flagepibiom, k_avgepif1, k_varepif1, 0, 0, k_avgepi1p, k_varepi1p, k_avgepi1eat, k_varepi1eat);
  751:     
  752: 	Set_Species_Biased_Value(spErrorStructure, PWN, flagepibiom, k_avgepif2, k_varepif2, 0, 0, k_avgepi2p, k_varepi2p, k_avgepi2eat, k_varepi2eat);
  753: 	Set_Species_Biased_Value(spErrorStructure, MOB_EP_OTHER, flagepibiom, k_avgepif2, k_varepif2, 0, 0, k_avgepi2p, k_varepi2p, k_avgepi2eat, k_varepi2eat);
  754: 
  755: 	Set_Species_Biased_Value(spErrorStructure, PL_BACT, flaginvpbiom, k_avgpbact, k_varpbact, 0, 0, k_avgpbactp, k_varpbactp, k_avgpbacteat, k_varpbacteat);
  756: 	Set_Species_Biased_Value(spErrorStructure, SED_BACT, flaginvpbiom, k_avgsbact, k_varsbact, 0, 0, k_avgsbactp, k_varsbactp, k_avgsbacteat, k_varsbacteat);
  757: 	Set_Species_Biased_Value(spErrorStructure, ICE_BACT, flaginvpbiom, k_avgpbact, k_varpbact, 0, 0, k_avgpbactp, k_varpbactp, k_avgpbacteat, k_varpbacteat);
  758: 	// Treat ice bacteria like pelagic bacteria for now
  759: 
  760: 	Set_Species_Biased_Value(spErrorStructure, SM_INF, flaginfbiom, k_avginfs, k_varinfs, 0, 0, k_avginfsp, k_varinfsp, k_avginfseat, k_varinfseat);
  761: 	Set_Species_Biased_Value(spErrorStructure, LG_INF, flaginfbiom, k_avginfl, k_varinfl, 0, 0, k_avginflp, k_varinflp, k_avginfleat, k_varinfleat);
  762: 
  763: 	Set_Species_Biased_Value(spErrorStructure, PHYTOBEN, flagepibiom, k_avgppben, k_varppben, 0, 0, k_avgbenpp, k_varbenpp, 0, 0);
  764: 	Set_Species_Biased_Value(spErrorStructure, MICROPHTYBENTHOS, flagepibiom, k_avgppben, k_varppben, 0, 0, k_avgbenpp, k_varbenpp, 0, 0);
  765: 	Set_Species_Biased_Value(spErrorStructure, SEAGRASS, flagepibiom, k_avgppben, k_varppben, 0, 0, k_avgbenpp, k_varbenpp, 0, 0);
  766: 	Set_Species_Biased_Value(spErrorStructure, TURF, flagepibiom, k_avgppben, k_varppben, 0, 0, k_avgbenpp, k_varbenpp, 0, 0);
  767: 
  768: 
  769: 	Set_Species_Biased_Value(spErrorStructure, ICE_DIATOMS, flaginvpbiom, k_avgphytl, k_varphytl, 0, 0, k_avgpelpp, k_varpelpp, 0, 0);
  770: 	Set_Species_Biased_Value(spErrorStructure, ICE_MIXOTROPHS, flagdetbiom, k_avgphyts, k_varphyts, 0, 0, k_avgpelpp, k_varpelpp, 0, 0);
  771: 	Set_Species_Biased_Value(spErrorStructure, ICE_ZOOBIOTA, flaginvpbiom, k_avgzoops, k_varzoops, 0, 0, k_avgzoopp, k_varzoopp, k_avgzoopeat, k_varzoopeat);
  772: 
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
