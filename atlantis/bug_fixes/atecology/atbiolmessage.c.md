# atecology/atbiolmessage.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **11 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 2 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [BMSG-01](#bmsg-01) | 966, 987-994 | H/M | accumulator not reset | `coverindx = 0;` is set once before the box loop (966), but inside the loop `coverindx += epi[CoverTracer]` ke… |  | [ ] |
| [BMSG-02](#bmsg-02) | 2736-2778 | H/M | double counting | For vertebrates, the first `n` loop adds `MIGRATION[sp].DEN/SN/RN[n][i]` into `juv_mig_sum`/`ad_mig_sum`. Then… |  | [ ] |
| [BMSG-03](#bmsg-03) | 2697-2703 vs 2790-2808 | M/M | header/data mismatch | For invertebrate migrators the header writes 3 columns for each `n` and for each `i < num_migrate`. The writer… |  | [ ] |
| [BMSG-04](#bmsg-04) | 435, 530-574 | M/M | wrong scope | The ice-layer block (`if(bm->ice_on)`) sits inside `if (bm->boxes[ij].botz >= bm->maxwcbotz)`, which is meant … |  | [ ] |
| [BMSG-05](#bmsg-05) | 457-458, 504-505, 552-553 | M/M | missing update | `bm->boxBiomass[ij][sp]` is incremented for water-column groups (281, 360) and land (601), but not in the sedi… |  | [ ] |
| [BMSG-06](#bmsg-06) | 2735, 2743, 2763 | M/M | cohort vs cohort x genotype | `n` runs over `numCohortsXnumGenes` but is compared directly with `age_mat` (an age-class index). Elsewhere th… |  | [ ] |
| [BMSG-07](#bmsg-07) | 266-268, 310 | M/M | cohort vs cohort x genotype | `fished_chrt` comes from `Age95pcntV_id` (a cohort index), and its floor is set from `numCohortsXnumGenes/2`. … |  | [ ] |
| [BMSG-08](#bmsg-08) | 1077-1095 | M/L | wrong condition | `if ((flagstockout == 1) \|\| call_type)` also holds for diagnostic box calls (`call_type == 1`, from atecology.… |  | [ ] |
| [BMSG-09](#bmsg-09) | 1026-1036 | M/L | out-of-bounds read | The BSS slope is computed only when `smallID != -1 && largeID != -1`, but the debug fprintf right after it use… |  | [ ] |
| [BMSG-10](#bmsg-10) | 694, 726 | M/L | cohort vs cohort x genotype | The initial-condition checks loop `n < numCohorts`, while the tracer arrays (`structNTracers`, `totNTracers`) … |  | [ ] |
| [BMSG-11](#bmsg-11) | 411-414 | L/L | wrong variable | `bm->diagnosticBiom[sp] += ...NO3...` uses the loop variable `sp`, left over after the species loop, while the… |  | [ ] |

---

## BMSG-01

**accumulator not reset** · Conf H / Sev M  
**Listed lines:** 966, 987-994

**Function:** `Ecology_Calculate_Total_Abundance()`

**Why it's thought to be a bug**

`coverindx = 0;` is set once before the box loop (966), but inside the loop `coverindx += epi[CoverTracer]` keeps adding and then `ecolindx[coverindx_id] += coverindx / (nbox-nboundary-nland)` runs for every box. So box 1's cover gets counted once per box, box 2's once per remaining box, and so on. The cover indicator is a weighted cumulative sum, not a mean. Function: `Ecology_Calculate_Total_Abundance()`.

**Current code** (`atecology/atbiolmessage.c` lines 964–996)

```c
  964: 		bm->ecolindx[InfEpi_id] = 0;
  965: 		bm->ecolindx[coverindx_id] = 0;
  966: 		coverindx = 0;
  967: 
  968: 		for (ij = 0; ij < ncells; ij++) {
  969: 			nreg = bm->regID[ij];
  970: 
  971: 			if ((!call_type && bm->boxes[ij].type != BOUNDARY) || (call_type && ij == bm->current_box)) {
  972:                 
  973: 				/* Calculate the biomass of each functional group in this box */
  974: 				Calculate_Box_Biomass(bm, llogfp, ij, nreg, FALSE, isDiagnostic);
  975: 
  976: 				if (call_type) {
  977: 					break;
  978: 				}
  979: 
  980: 				//	fprintf(llogfp, "bm->diagnost[ij][DivCount_i] = %.20e\n", bm->diagnost[ij][DivCount_i]);
  981: 				/* Ecological indicators */
  982: 				bm->ecolindx[PDratio_id] += bm->diagnost[ij][PelDem_ratio_i] / (bm->nbox - bm->nboundary - bm->nland);
  983: 				bm->ecolindx[FPFVratio_id] += bm->diagnost[ij][PiscPlank_ratio_i] / (bm->nbox - bm->nboundary - bm->nland);
  984: 				bm->ecolindx[DivCount_id] += bm->diagnost[ij][DivCount_i] / (bm->nbox - bm->nboundary - bm->nland);
  985: 				//fprintf(llogfp, "bm->ecolindx[DivCount_id] = %.20e\n", bm->ecolindx[DivCount_id]);
  986: 				bm->ecolindx[InfEpi_id] += bm->diagnost[ij][InfEpi_ratio_i] / (bm->nbox - bm->nboundary - bm->nland);
  987: 				for (k = 0; k < bm->K_num_tot_sp; k++) {
  988: 					if(FunctGroupArray[k].speciesParams[flag_id] == TRUE){
  989: 						if (FunctGroupArray[k].CoverTracer != -1) {
  990: 							coverindx += bm->boxes[ij].epi[FunctGroupArray[k].CoverTracer];
  991: 						}
  992: 					}
  993: 				}
  994: 				bm->ecolindx[coverindx_id] += coverindx / (bm->nbox - bm->nboundary - bm->nland);
  995: 
  996: 				/*
```

**Suggested fix (from review):** Move `coverindx = 0;` inside the `for (ij...)` loop, before the `k` loop (or add straight into `ecolindx`).

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

## BMSG-02

**double counting** · Conf H / Sev M  
**Listed lines:** 2736-2778

**Function:** `Write_Migration_File()`

**Why it's thought to be a bug**

For vertebrates, the first `n` loop adds `MIGRATION[sp].DEN/SN/RN[n][i]` into `juv_mig_sum`/`ad_mig_sum`. Then the `j` loop uses `index = j*num_migrate + i`, which at `j=0` is `i` again, so queue slot `i` is added twice. Migrating biomass and Prop_Migrating in Migration.txt are too high. Function: `Write_Migration_File()`.

**Current code** (`atecology/atbiolmessage.c` lines 2734–2773 (truncated))

```c
 2734: 
 2735:            		age_mat = (int) (FunctGroupArray[sp].speciesParams[age_mat_id]);
 2736:            		for(i = 0; i < FunctGroupArray[sp].num_migrate; i++){
 2737:            			juv_mig_sum = juv_sum = ad_mig_sum = ad_sum = 0;
 2738: 					for(n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++){
 2739: 
 2740: 						dynsn = MIGRATION[sp].SN[n][i];
 2741: 						dynrn = MIGRATION[sp].RN[n][i];
 2742: 
 2743: 						if (n < age_mat){
 2744: 							juv_mig_sum += (MIGRATION[sp].DEN[n][i] * (dynsn + dynrn));
 2745: 							juv_sum += bm->totagepop[sp][n];
 2746: 						}
 2747: 						else{
 2748: 							ad_mig_sum += (MIGRATION[sp].DEN[n][i] * (dynsn + dynrn));
 2749: 							ad_sum += bm->totagepop[sp][n];
 2750: 						}
 2751:                         
 2752:                         
 2753: 					}
 2754:                     
 2755:                     for (j = 0; j < numValues; j++){
 2756:                         
 2757:                         index = (j * FunctGroupArray[sp].num_migrate) + i;
 2758:                         for(n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++){
 2759:                             
 2760:                             dynsn = MIGRATION[sp].SN[n][index];
 2761:                             dynrn = MIGRATION[sp].RN[n][index];
 2762:                             
 2763:                             if (n < age_mat){
 2764:                                 juv_mig_sum += (MIGRATION[sp].DEN[n][index] * (dynsn + dynrn));
 2765:                                 //juv_sum += bm->totagepop[sp][n];
 2766:                             }
 2767:                             else{
 2768:                                 ad_mig_sum += (MIGRATION[sp].DEN[n][index] * (dynsn + dynrn));
 2769:                                 //ad_sum += bm->totagepop[sp][n];
 2770:                             }
 2771:                             
 2772:                             
 2773: //                            fprintf(bm->logFile, "Write_Migration_File - day = %f, - MIGRATION[sp = %s].DEN[n = %d][qid = %d] = %e, spmigrate= %d\n",
```

**Suggested fix (from review):** Delete the migrating-sum lines from the first loop (keep only `juv_sum`/`ad_sum` there), or start `j` at 1.

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

## BMSG-03

**header/data mismatch** · Conf M / Sev M  
**Listed lines:** 2697-2703 vs 2790-2808

**Function:** `Init_Migration_File()`

**Why it's thought to be a bug**

For invertebrate migrators the header writes 3 columns for each `n` and for each `i < num_migrate`. The writer writes 3 values for each `n` and each `i` in `[num_in_queue_done, num_in_queue)`, which is a different, time-varying count. Migration.txt columns are misaligned and the row width changes from row to row. Function: `Init_Migration_File()` / `Write_Migration_File()`.

**Current code** (`atecology/atbiolmessage.c` lines 2673–2683) — code appears to have moved -22 lines since the review

```c
 2673:     /** Create file **/
 2674:     if ( (fid=Util_fopen(bm, fname, "w")) == NULL )
 2675:         quit("Init_Migration_File: Can't open %s\n",fname);
 2676: 
 2677:     /** Column definitions **/
 2678:     fprintf(fid,"Time");
 2679: 
 2680:     /* Absolute values */
 2681:     for (sp=0; sp<bm->K_num_tot_sp; sp++) {
 2682: 
 2683:         if ( (int)(FunctGroupArray[sp].speciesParams[flag_id])){
```

**Suggested fix (from review):** Use the same loop bound in both, e.g. aggregate over the queue per `i < num_migrate` as the vertebrate branch does.

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

## BMSG-04

**wrong scope** · Conf M / Sev M  
**Listed lines:** 435, 530-574

**Function:** `Calculate_Box_Biomass()`

**Why it's thought to be a bug**

The ice-layer block (`if(bm->ice_on)`) sits inside `if (bm->boxes[ij].botz >= bm->maxwcbotz)`, which is meant to leave out the non-dynamic sediments and epibenthos of oceanic cells. Sea ice over deep oceanic boxes is dynamic, but it is left out of totbiom, reg_prop and targetspbiom. Function: `Calculate_Box_Biomass()`.

**Current code** (`atecology/atbiolmessage.c` lines 433–437)

```c
  433: 	/* Only include benthos and epibenthics if not oceanic cell - oceanic cells should not be counted as their sediments are not dynamic */
  434: 	//if (isInitPops == TRUE || bm->boxes[ij].botz >= bm->maxwcbotz) {
  435: 	if (bm->boxes[ij].botz >= bm->maxwcbotz) {
  436: 		for (k = 0; k < bm->boxes[ij].sm.nz; k++) { // Loop through each sediment layer
  437: 			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
```

**Suggested fix (from review):** Move the ice block outside the `botz >= maxwcbotz` if-block.

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

## BMSG-05

**missing update** · Conf M / Sev M  
**Listed lines:** 457-458, 504-505, 552-553

**Function:** `Calculate_Box_Biomass()`

**Why it's thought to be a bug**

`bm->boxBiomass[ij][sp]` is incremented for water-column groups (281, 360) and land (601), but not in the sediment, epibenthic or ice branches. BoxBiomass.txt (`Write_BoxBiomass`) therefore leaves out all infauna, epibenthos and ice biomass and only shows any WC fraction. Function: `Calculate_Box_Biomass()`.

**Current code** (`atecology/atbiolmessage.c` lines 455–460)

```c
  455: 									}else{
  456: 
  457: 										bm->totbiom[sp] += biomass;
  458: 										bm->totagepop[sp][n] += biomass;
  459: 
  460: 										if (isInitPops == TRUE) {
```

**Suggested fix (from review):** Add `bm->boxBiomass[ij][sp] += biomass;` in the sed, epi and ice branches (non-diagnostic path).

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

## BMSG-06

**cohort vs cohort x genotype** · Conf M / Sev M  
**Listed lines:** 2735, 2743, 2763

**Function:** `Write_Migration_File()`

**Why it's thought to be a bug**

`n` runs over `numCohortsXnumGenes` but is compared directly with `age_mat` (an age-class index). Elsewhere the convention is `basecohort = n / numGeneTypes` (atdemography.c:373, 500). With `numGeneTypes > 1` the juvenile/adult split is wrong. Function: `Write_Migration_File()`.

**Current code** (`atecology/atbiolmessage.c` lines 2733–2765)

```c
 2733:            	if(FunctGroupArray[sp].isVertebrate == TRUE){
 2734: 
 2735:            		age_mat = (int) (FunctGroupArray[sp].speciesParams[age_mat_id]);
 2736:            		for(i = 0; i < FunctGroupArray[sp].num_migrate; i++){
 2737:            			juv_mig_sum = juv_sum = ad_mig_sum = ad_sum = 0;
 2738: 					for(n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++){
 2739: 
 2740: 						dynsn = MIGRATION[sp].SN[n][i];
 2741: 						dynrn = MIGRATION[sp].RN[n][i];
 2742: 
 2743: 						if (n < age_mat){
 2744: 							juv_mig_sum += (MIGRATION[sp].DEN[n][i] * (dynsn + dynrn));
 2745: 							juv_sum += bm->totagepop[sp][n];
 2746: 						}
 2747: 						else{
 2748: 							ad_mig_sum += (MIGRATION[sp].DEN[n][i] * (dynsn + dynrn));
 2749: 							ad_sum += bm->totagepop[sp][n];
 2750: 						}
 2751:                         
 2752:                         
 2753: 					}
 2754:                     
 2755:                     for (j = 0; j < numValues; j++){
 2756:                         
 2757:                         index = (j * FunctGroupArray[sp].num_migrate) + i;
 2758:                         for(n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++){
 2759:                             
 2760:                             dynsn = MIGRATION[sp].SN[n][index];
 2761:                             dynrn = MIGRATION[sp].RN[n][index];
 2762:                             
 2763:                             if (n < age_mat){
 2764:                                 juv_mig_sum += (MIGRATION[sp].DEN[n][index] * (dynsn + dynrn));
 2765:                                 //juv_sum += bm->totagepop[sp][n];
```

**Suggested fix (from review):** Compare `(n / FunctGroupArray[sp].numGeneTypes) < age_mat`.

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

## BMSG-07

**cohort vs cohort x genotype** · Conf M / Sev M  
**Listed lines:** 266-268, 310

**Function:** `Calculate_Box_Biomass()`

**Why it's thought to be a bug**

`fished_chrt` comes from `Age95pcntV_id` (a cohort index), and its floor is set from `numCohortsXnumGenes/2`. It is then compared with `n`, the cohort x genotype index. With genotypes, `calcNstart[sp][hist_id]` counts the wrong (single genotype) slot. Function: `Calculate_Box_Biomass()`.

**Current code** (`atecology/atbiolmessage.c` lines 264–303 (truncated))

```c
  264: 				switch (FunctGroupArray[sp].groupAgeType) {
  265: 				case AGE_STRUCTURED:
  266: 					fished_chrt = (int) (FunctGroupArray[sp].speciesParams[Age95pcntV_id]);
  267: 					if (fished_chrt < ((FunctGroupArray[sp].numCohortsXnumGenes) / 2))
  268: 						fished_chrt = ((FunctGroupArray[sp].numCohortsXnumGenes) / 2);
  269: 
  270: 					for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
  271: 						sn = FunctGroupArray[sp].structNTracers[n];
  272: 						rn = FunctGroupArray[sp].resNTracers[n];
  273: 						den = FunctGroupArray[sp].NumsTracers[n];
  274: 						biomass = (bm->boxes[ij].tr[k][sn] + bm->boxes[ij].tr[k][rn]) * bm->boxes[ij].tr[k][den] * FunctGroupArray[sp].habitatCoeffs[WC];
  275: 
  276: 						if(isDiagnostic == TRUE){
  277: 							bm->diagnosticBiom[sp] += biomass;
  278: 						}else{
  279: 							bm->totbiom[sp] += biomass;
  280: 							bm->totagepop[sp][n] += biomass;
  281: 							bm->boxBiomass[ij][sp] += biomass;
  282: 
  283: 							/**
  284:                             if((sp == bm->which_check) && (bm->checkstart <= bm->dayt) && ((bm->checkbox == ij) || (bm->checkbox > bm->nbox))){
  285: 								fprintf(llogfp, "TIme: %e in calc_box_biom %s-%d in box %d-%d totbiom[sp]: %.20e, sn = %.20e, rn = %.20e, den = %.20e\n",
  286: 									bm->dayt, FunctGroupArray[sp].groupCode, n, ij, k, bm->totbiom[sp], bm->boxes[ij].tr[k][sn],
  287: 									bm->boxes[ij].tr[k][rn], bm->boxes[ij].tr[k][den]);
  288: 							}
  289: 							**/
  290: 
  291:                             if(bm->totbiom[sp] < 0){
  292:                                 printf("sn = %e, rn = %e, den = %e\n", bm->boxes[ij].tr[k][sn], bm->boxes[ij].tr[k][rn], bm->boxes[ij].tr[k][den]);
  293:                                 quit("Biomass of %s:%d is less than zero after adding biomass from box%d:%d\n", FunctGroupArray[sp].groupCode, n, ij, k);
  294:                             }
  295: 
  296: 							/* If this is the initPopulations call then do the extra steps */
  297: 							if (isInitPops == FALSE) {
  298: 								bm->reg_prop[sp][nreg] += biomass;
  299: 							} else {
  300: 								bm->initreg_prop[sp][nreg] += biomass;
  301: 
  302: 								/* Need to store target species biomass here so can be used in calculations for Dan Holland's
  303: 								 economically based effort model (movement code not executed before Economics done, so need
```

**Suggested fix (from review):** Use `(n / numGeneTypes) == fished_chrt`, with the floor based on `numCohorts/2`.

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

## BMSG-08

**wrong condition** · Conf M / Sev L  
**Listed lines:** 1077-1095

**Function:** `Ecology_Calculate_Total_Abundance()`

**Why it's thought to be a bug**

`if ((flagstockout == 1) || call_type)` also holds for diagnostic box calls (`call_type == 1`, from atecology.c:1253, 1959, 3020, 3616). So `Ecology_Output_Mort_Per_Pred_Estimates()` writes extra MortPerPred/SpecificPredMort rows for every debugged box/layer call. Function: `Ecology_Calculate_Total_Abundance()`.

**Current code** (`atecology/atbiolmessage.c` lines 1075–1097)

```c
 1075: 
 1076: 		/* If non-annual call proceed with standard log.txt output */
 1077: 		if ((flagstockout == 1) || call_type) {
 1078: 
 1079: 
 1080: 			//fprintf(llogfp, "\n");
 1081: 
 1082: 			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 1083: 				if (FunctGroupArray[sp].isDetritus == TRUE || (FunctGroupArray[sp].isDetritus == FALSE
 1084: 						&& (int) (FunctGroupArray[sp].speciesParams[flag_id]))) { /* Allow for DIN */
 1085: 
 1086: 					Print_Biomass_Value(bm, call_type, llogfp, sp);
 1087: 				}
 1088: 			}
 1089:             
 1090: 			/* Now print out DIN value */
 1091: 			Print_Biomass_Value(bm, call_type, llogfp, bm->K_num_tot_sp);
 1092: 
 1093: 			if (!bm->flagannual_Mest) {
 1094: 				Ecology_Output_Mort_Per_Pred_Estimates(bm, llogfp);
 1095: 			}
 1096:             
 1097: 		}
```

**Suggested fix (from review):** Wrap the Mort_Per_Pred call in `if (!call_type && !bm->flagannual_Mest)`.

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

## BMSG-09

**out-of-bounds read** · Conf M / Sev L  
**Listed lines:** 1026-1036

**Function:** `Ecology_Calculate_Total_Abundance()`

**Why it's thought to be a bug**

The BSS slope is computed only when `smallID != -1 && largeID != -1`, but the debug fprintf right after it uses `FunctGroupArray[bm->smallID].groupCode` and `totfishpop[bm->smallID]` without that guard, so it reads index -1. Function: `Ecology_Calculate_Total_Abundance()`.

**Current code** (`atecology/atbiolmessage.c` lines 1024–1038)

```c
 1024: 				}
 1025: 			}
 1026: 			if(bm->smallID != -1 && bm->largeID != -1){
 1027: 				/* Find BSS - numerator and denominator to be all in kg in calculation */
 1028: 				bm->ecolindx[BSSslope_id] = ((bm->totfishpop[bm->smallID] - bm->totfishpop[bm->largeID]) * bm->X_CN * mg_2_tonne) / (log(bm->smallsize) - log(
 1029: 						bm->largesize) + small_num);
 1030: 			}
 1031: 			if ((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)) {
 1032: 				fprintf(llogfp,
 1033: 						"BSSslope: %e, fishpop[small-%s]: %e, fishpop[large-%s]: %e, log(smallsize): %e, smallsize: %e, log(largesize): %e, largesize: %e\n",
 1034: 						bm->ecolindx[BSSslope_id], FunctGroupArray[bm->smallID].groupCode, bm->totfishpop[bm->smallID], FunctGroupArray[bm->largeID].groupCode,
 1035: 						bm->totfishpop[bm->largeID], (double)log(bm->smallsize), bm->smallsize, (double)log(bm->largesize), bm->largesize);
 1036: 			}
 1037:             
 1038:             /* If not an annual call then do all other necessary checks */
```

**Suggested fix (from review):** Move the debug print inside the `smallID/largeID != -1` block.

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

## BMSG-10

**cohort vs cohort x genotype** · Conf M / Sev L  
**Listed lines:** 694, 726

**Function:** `Check_Layer_Initial_Biomass()`

**Why it's thought to be a bug**

The initial-condition checks loop `n < numCohorts`, while the tracer arrays (`structNTracers`, `totNTracers`) run over `numCohortsXnumGenes`. Genotypes beyond the first are never checked for negative or misplaced biomass. Function: `Check_Layer_Initial_Biomass()`.

**Current code** (`atecology/atbiolmessage.c` lines 692–728)

```c
  692: 			case AGE_STRUCTURED:
  693: 
  694: 				for (n = 0; n < FunctGroupArray[sp].numCohorts; n++) {
  695: 					sn = FunctGroupArray[sp].structNTracers[n];
  696: 					rn = FunctGroupArray[sp].resNTracers[n];
  697: 					den = FunctGroupArray[sp].NumsTracers[n];
  698: 
  699: 					for (b = 0; b < bm->nbox; b++) {
  700: 
  701: 						/* Check to see if we have negative biomass */
  702: 						for (k = 0; k < bm->boxes[b].nz; k++){
  703: 							biomass = (bm->boxes[b].tr[k][sn] + bm->boxes[b].tr[k][rn]) * bm->boxes[b].tr[k][den] * FunctGroupArray[sp].habitatCoeffs[WC];
  704: 
  705: 							if(biomass < 0){
  706: 								fprintf(stderr, "ERROR: Data for group %s, cohort %d in box %d, layer %d in your initial conditions file. This biomass is NEGATIVE! = %e. Please update your initial conditions and try again.\n\n",
  707: 									FunctGroupArray[sp].groupCode, n, b, k, biomass);
  708: 								quit("");
  709: 							}
  710: 
  711: 						}
  712: 						/* Check to see if we have biomass in layers that are not active */
  713: 						for (k = bm->boxes[b].nz; k < bm->wcnz; k++) {
  714: 							biomass = (bm->boxes[b].tr[k][sn] + bm->boxes[b].tr[k][rn]) * bm->boxes[b].tr[k][den] * FunctGroupArray[sp].habitatCoeffs[WC];
  715: 
  716: 							if(biomass != 0){
  717: 								fprintf(stderr, "WARN: Data for group %s, cohort %d in box %d, layer %d in your initial conditions file. There are only %d layers in this box. Please check and updated your initial conditions file.\n\n",
  718: 									FunctGroupArray[sp].groupCode, n, b, k, bm->boxes[b].nz);
  719: 							}
  720: 						}
  721: 					}
  722: 				}
  723: 				break;
  724: 			case AGE_STRUCTURED_BIOMASS:
  725: 				if (FunctGroupArray[sp].groupType != REF_DET && FunctGroupArray[sp].groupType != LAB_DET) {
  726: 					for (n = 0; n < FunctGroupArray[sp].numCohorts; n++) {
  727: 
  728: 						pid = FunctGroupArray[sp].totNTracers[n];
```

**Suggested fix (from review):** Loop to `numCohortsXnumGenes`.

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

## BMSG-11

**wrong variable** · Conf L / Sev L  
**Listed lines:** 411-414

**Function:** `Calculate_Box_Biomass()`

**Why it's thought to be a bug**

`bm->diagnosticBiom[sp] += ...NO3...` uses the loop variable `sp`, left over after the species loop, while the NH3 block (395) uses `nid` (`DIN_id`). It works only because `sp == K_num_tot_sp == DIN_id` (atparamIO.c:367). Function: `Calculate_Box_Biomass()`.

**Current code** (`atecology/atbiolmessage.c` lines 409–416)

```c
  409: 		}
  410: 
  411: 		pid = NO3_i;
  412: 		nid = bm->DIN_id;
  413: 		if(isDiagnostic == TRUE){
  414: 			bm->diagnosticBiom[sp] += bm->boxes[ij].tr[k][pid] * bm->boxes[ij].dz[k] * bm->boxes[ij].area; // Intentional overload storing all nutrients in final slot
  415: 		}else{
  416: 			bm->totbiom[nid] += bm->boxes[ij].tr[k][pid] * bm->boxes[ij].dz[k] * bm->boxes[ij].area;
```

**Suggested fix (from review):** Use `bm->diagnosticBiom[nid]`.

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

## Lower-confidence notes (speculative — check before acting)

These came from section B.3 of the list. They were not raised as findings. Tick if you want one followed up.

### LC-1

atbiolmessage.c 1802/1825: `mort_scale = 365.0 / (365.0 - update_date)` divides by zero if `calcupdate_date` is 365 (Write_MortPerPred guards this case; Write_Mort does not).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atbiolmessage.c 742-748, 793-799: the "biomass in inactive layer" warnings for biomass-pool groups multiply by `dz[k]`/`sm.dz[k]`, which are likely 0 for inactive layers, so these warnings may never fire.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
