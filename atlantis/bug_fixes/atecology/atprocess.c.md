# atecology/atprocess.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **18 findings** (severity H: 3; confidence H and severity H: 2; on the priority list: 2), plus 5 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PROC-01](#proc-01) ⭐ | 439, 913, 1219 | H/H | wrong index / buffer overrun | `bm->pSPFCeat[sp_id][preyID][cohort][kij]` (and `[predatorID][preyID][cohort][prey_chrt]` at 420). pSPFCeat is… |  | [ ] |
| [PROC-02](#proc-02) | 908-914, 1217 | H/M | accumulator not reset | `bm->groupTotCatch[preyID][kij] += FCcaughttemp*pFLEET` and then `*= pSPFCeat`. groupTotCatch is only allocate… |  | [ ] |
| [PROC-03](#proc-03) ⭐ | 1197-1199 | H/H | wrong variable / buffer overrun | `thisID = (int)speciesParams[coralID_id]; ... FunctGroupArray[thisID].cohortSpeciesParams[kij][rugosity_dec_id… |  | [ ] |
| [PROC-04](#proc-04) | 1005-1006, 1013-1014, 1162, 1165 | H/M | double division | For the std_holling/ratio/HV/CM cases, Eat sets `scalar = pHscalar / tot_prey; tprey = tot_prey;`. Do_Extra_Fe… |  | [ ] |
| [PROC-05](#proc-05) | 759-765 vs 1266-1288; 1368 | H/M | wrong index | Get_Extra_Feed stores feed availability in `EATINGinfo/FEEDinfo[AquacultFeedIndex][cohort][WC]`, but Do_Extra_… |  | [ ] |
| [PROC-06](#proc-06) | 494-528, 545, 566-571 | H/M | variable overwritten in loop | `pHscalar` is set before the habitat loop (1/pHcorr for the predation-availability effect), but for plant prey… |  | [ ] |
| [PROC-07](#proc-07) | 1094-1103 | H/M | division by zero / wrong loop bound | eat_minmax: `rel_scalar = EATINGinfo/(EATINGinfo + catch_addition)` is 0/0 = NaN for any prey/habitat with zer… |  | [ ] |
| [PROC-08](#proc-08) | 1745, 1778 | H/M | double-applied scalar | `mortality[0]` already includes `mortality_scalar` (1585), but calcTrackedMort adds `mortality_scalar * mortal… |  | [ ] |
| [PROC-09](#proc-09) | 1803-1817 | H/L | leftover debug | `if((bm->dayt > 3633) && (groupType == TURF) && (current_box == 24)) fprintf(llogfp, ...)` is live code (the s… |  | [ ] |
| [PROC-10](#proc-10) | 1845-1849, 1873-1878 | H/M | logic / copy-paste | Wrap handling: when `encyst_TofY + encyst_period > 365`, it sets `encyst_TofY = 0; encyst_period = rollover`, … |  | [ ] |
| [PROC-11](#proc-11) | 1525-1531 | M/M | truncation / missing state | `overStartTemp = (int)(speciesParams[overwinterStartTemp_id])` truncates fractional temperature thresholds (2.… |  | [ ] |
| [PROC-12](#proc-12) | 2342-2356 | M/M | formula inconsistent | `BB_DL = BB*Bact_stim*XBB_DL*hO_BB/(Bact_stim*XBB_DL*DLsed*hO_BB + ...)`: the numerator is missing `DLsed` (sa… |  | [ ] |
| [PROC-13](#proc-13) | 2159, 2204-2205 | M/M | double counting | For invertebrates, `biomassToDL` is added to `DetritusProd[WC][DCdet_id]` (1996) and then again to `DetritusPr… |  | [ ] |
| [PROC-14](#proc-14) | 2619-2625 | M/H | unit mismatch | McElhany-Bush: `annualSurvivalNoAcid = exp(-pHmortB)` is annual, so `-log(1-annualSurvivalDecrement)` is an an… |  | [ ] |
| [PROC-15](#proc-15) | 1695-1714 | M/M | no-op logic | `scale_uptake_NO = max(scale_uptake, NO/uptakeNO); scale_uptake = min(scale_uptake, scale_uptake_NO);`. The ma… |  | [ ] |
| [PROC-16](#proc-16) | 1664-1674 | M/M | division by zero | `uptakeNH = sp_grow*(NH/(KN+NH))*((KN+DIN)/DIN)`, and uptakeNO also divides by DIN. When DIN == 0, sp_grow is … |  | [ ] |
| [PROC-17](#proc-17) | ≈622-630 | H/L | wrong variable / index (latent) | Unreachable for now (Eat quits before calling it), but: the habitat coefficients use `FunctGroupArray[sp_id]` … |  | [ ] |
| [PROC-18](#proc-18) | 316-323, 1476 | M/L | division by zero | Primary_Production (and atcoral/atmacrophytes) pass `PBmax_D = 0.0`. If bm->flaglight is irr_ice_v1, `step2 = … |  | [ ] |

---

## PROC-01

**wrong index / buffer overrun** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 439, 913, 1219  
**Lines at review (Oct 2026):** 420, 800, 1096

**Why it's thought to be a bug**

`bm->pSPFCeat[sp_id][preyID][cohort][kij]` (and `[predatorID][preyID][cohort][prey_chrt]` at 420). pSPFCeat is allocated `Util_Alloc_Init_4D_Double(K_num_max_stages, K_num_max_stages, K_num_tot_sp, K_num_tot_sp)` => `[sp][sp][stage][stage]` (atlantismain/atlantismain.c:2061), and it is indexed by stage in atvertprocesses.c:729/738 (`[chrtstage][preyage]`). Here it uses the cohort x genotype indices `cohort`/`kij`, which go past K_num_max_stages (normally 2), so the read is out of bounds and returns the wrong preference. Live whenever catcheater && flag_fisheries_on && !flag_olddiet. Functions: `Get_Catch_Prey()`, `Eat()`.

**Current code** (`atecology/atprocess.c` lines 437–441)

```c
  437: 	int catcheater = (int) (FunctGroupArray[predatorGuildID].speciesParams[catcheater_id]);
  438: 	//int bcohort = floor(cohort / FunctGroupArray[predatorGuildID].numGeneTypes);
  439: 	// bcohort was used in place of cohort in spPreyAvail before moved to full gene expression (to allow for evolving diets and ontogeny)
  440: 
  441: 	CATCHEATINGinfo[preyGuildID][prey_chrt] = 0; // incase never called again
```

**Suggested fix (from review):** Index with `[chrtstage][FunctGroupArray[preyID].cohort_stage[kij]]`

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

## PROC-02

**accumulator not reset** · Conf H / Sev M  
**Lines (current code):** 908-914, 1217  
**Lines at review (Oct 2026):** 795-801, 1094

**Function:** `Eat()`

**Why it's thought to be a bug**

`bm->groupTotCatch[preyID][kij] += FCcaughttemp*pFLEET` and then `*= pSPFCeat`. groupTotCatch is only allocated (atbiolsetup.c:2897) and freed. A repo-wide grep finds no reset, so it keeps growing across predators, cells and timesteps, and the `*=` scales the whole running total, not just this predator's share. The `ratio` denominator at 1094 is therefore far too big, so too little catch is taken out of FCcaughttemp/Catch. Function: `Eat()`.

**Current code** (`atecology/atprocess.c` lines 906–916)

```c
  906: 					eat_amt = (long double) EATINGinfo[preyID][kij][habitat] + catch_addition;
  907: 					FEEDinfo[preyID][kij][habitat] += ((double)eat_amt) * Util_Mich_Ment((double)eat_amt, KL_sp);
  908: 					denom_step += FEEDinfo[preyID][kij][habitat];
  909: 				}
  910: 			}
  911: 		}
  912: 	}
  913: 
  914: 	if(FunctGroupArray[sp_id].isCultured || FunctGroupArray[sp_id].isSupplemented) {
  915: 		Get_Extra_Feed(bm, llogfp, sp_id, cohort, sp_Biomass, flagcase, KL_sp, &living_prey, &living_prey_sq, &denom_step);
  916: 	}
```

**Suggested fix (from review):** Before the fleet loop, reset with `groupTotCatch[preyID][kij] = 0` (or use a local), then apply pSPFCeat

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

## PROC-03

**wrong variable / buffer overrun** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 1197-1199  
**Lines at review (Oct 2026):** 1074-1076

**Function:** `Eat()`

**Why it's thought to be a bug**

`thisID = (int)speciesParams[coralID_id]; ... FunctGroupArray[thisID].cohortSpeciesParams[kij][rugosity_dec_id]`. coralID_id holds the CORALREEF index 0..ncoral-1 (atbiolsetup.c:4136), not a guild, so this reads the parameters of an unrelated group (e.g. guild 0). cohortSpeciesParams is also sized `[numStages]` (atUtilGroupIO.c:829), but here it is indexed by `kij` (cohort x genotype), which can overrun it. Live with flag_adv_habitat whenever coral or sponge is eaten. Function: `Eat()`.

**Current code** (`atecology/atprocess.c` lines 1195–1201)

```c
 1195: 				if (catcheater && bm->flag_fisheries_on && (spCATCHGRAZEinfo[preyID][kij])){
 1196: 					for (fleet = 0; fleet < bm->K_num_fisheries; fleet++) {
 1197: 						ratio = bm->FCcaughttemp[preyID][fleet][kij] * bm->pFLEET[sp_id][fleet] / (bm->groupTotCatch[preyID][kij] + small_num);
 1198: 						if(!bm->flag_olddiet)
 1199: 							ratio *= bm->pSPFCeat[sp_id][preyID][cohort][kij];
 1200:                         if(FunctGroupArray[preyID].isVertebrate == TRUE) {
 1201: 							bm->FCcaughttemp[preyID][fleet][kij] -= spCATCHGRAZEinfo[preyID][kij] * (double)ratio * bm->cell_vol / (VERTinfo[preyID][kij][SN_id] + VERTinfo[preyID][kij][RN_id]);  // as divide by bm->cell_vol in calculating CATCHEATINFO which SPCATCHGRAZEinfo comes from
```

**Suggested fix (from review):** Use `FunctGroupArray[preyID].cohortSpeciesParams[FunctGroupArray[preyID].cohort_stage[kij]][rugosity_dec_id]`

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

## PROC-04

**double division** · Conf H / Sev M  
**Lines (current code):** 1005-1006, 1013-1014, 1162, 1165  
**Lines at review (Oct 2026):** 1005-1006, 1013-1014, 1162, 1165

**Why it's thought to be a bug**

For the std_holling/ratio/HV/CM cases, Eat sets `scalar = pHscalar / tot_prey; tprey = tot_prey;`. Do_Extra_Feed then computes `EATINGinfo*CLEAR*scalar/(tprey+small_num)`, so aquaculture feed consumption is divided by tot_prey twice (tot_prey_sq twice for holling3). `scalar` also carries the pH correction of whichever prey group was processed last, which has nothing to do with the feed. Functions: `Eat()`, `Do_Extra_Feed()`.

**Current code** (`atecology/atprocess.c` lines 1003–1016)

```c
 1003: 			} else {
 1004: 				pHscalar = 1.0;
 1005: 			}
 1006: 			scalar = pHscalar;
 1007:             scaled_clear = pHscalar * CLEAR;
 1008: 
 1009: 			max_hab = bm->num_active_habitats - 1;
 1010: 			if(FunctGroupArray[preyID].isVertebrate == TRUE)
 1011: 				max_hab = WC;
 1012: 
 1013:             tprey = 1.0;
 1014: 			for (kij = 0; kij < FunctGroupArray[preyID].numCohortsXnumGenes; kij++) {
 1015:                 
 1016: 				switch (flagcase) {  /* calculate the biomass actually eaten of each prey group */
```

**Suggested fix (from review):** Pass `scalar=1.0` (or the feed-only scalar) and divide once by tprey

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

## PROC-05

**wrong index** · Conf H / Sev M  
**Lines (current code):** 759-765 vs 1266-1288; 1368  
**Lines at review (Oct 2026):** 646-652 vs 1143-1165; 1244

**Why it's thought to be a bug**

Get_Extra_Feed stores feed availability in `EATINGinfo/FEEDinfo[AquacultFeedIndex][cohort][WC]`, but Do_Extra_Feed always reads `[AquacultFeedIndex][0][WC]` and writes spGRAZEinfo `[...][0]`. Invert_Activities (1368) also reads `GRAZEinfo[AquacultFeedIndex][0][WC]`. For any cohort > 0, a cultured or supplemented group eats feed sized from cohort 0's (stale) availability. Functions: `Get_Extra_Feed()`, `Do_Extra_Feed()`.

**Current code** (`atecology/atprocess.c` lines 757–767)

```c
  757:  * mg N s-1. This change per second is later calculated by the size of the timestep
  758:  * to get the total change in mg N.
  759:  *
  760:  */
  761: void Eat(MSEBoxModel *bm, FILE *llogfp, int flagcase, int sp_id, int cohort, double sp, double C_sp, double mum_sp, double KL_sp, double KU_sp, double vl_sp, double ht_sp, int flagsat_sp, double E1_sp, double E2_sp, double EDL_sp, double EDR_sp, int sp_feed_while_spawn, int sp_spawn_now, double chrt_mat, double ***spPREYinfo, double ***spGRAZEinfo, double **spCATCHGRAZEinfo, double sp_Biomass) {  // sp_Biomass is only used for aquaculture, sp is used for everything else
  762: 	int preyID, max_hab, prey_active;
  763: 	int kij = 0, fleet = 0;
  764: 	int habitat, thisID;
  765: 	long double living_prey, tot_prey, tot_prey_sq, tprey = 0, living_prey_sq,  labdet, refdet, CLEAR, step1, plant_prey, graze_live, pHscalar;
  766: 	long double rel_scalar, catch_addition, ratio, eat_amt;
  767: 	int ncohort, chrtstage;
```

**Suggested fix (from review):** Use a consistent index (store at [0], or pass the cohort through)

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

## PROC-06

**variable overwritten in loop** · Conf H / Sev M  
**Lines (current code):** 494-528, 545, 566-571  
**Lines at review (Oct 2026):** 462-466, 483, 504-509

**Function:** `Calculate_PreyAvail()`

**Why it's thought to be a bug**

`pHscalar` is set before the habitat loop (1/pHcorr for the predation-availability effect), but for plant prey it is overwritten inside the loop (`pHscalar = pHcorr` or 1.0). From the 2nd habitat on, `prey_eat = prey_avail * pHscalar` uses the nutritional-value scalar instead of the predation scalar, and the 1/pHcorr effect is lost or inverted for SED/EPIFAUNA plant prey. Function: `Calculate_PreyAvail()`.

**Current code** (`atecology/atprocess.c` lines 492–531 (truncated))

```c
  492: 			CATCHEATINGinfo[preyGuildID][prey_chrt] = (double)catch_eat;
  493: 		}
  494: 
  495: 		/* determine biomass available */
  496: 		prey_eat =  prey_avail * pHscalar;
  497: 
  498: 		/* PREYinfo holds each habitat in its own units: WC mg N m-3 water, SED mg N m-3 sediment,
  499: 		 * EPIFAUNA mg N m-2, ICE_BASED mg N m-3 ice - so put the prey on a consistent basis for the predator.
  500: 		 *
  501: 		 * Water column predators (mg N m-3): spread the other habitats over the water cell the predator is in.
  502: 		 * Converted back in Eat() so the amount removed from the prey stays in the prey's own units.
  503: 		 *
  504: 		 * Epibenthic predators (mg N m-2): prey are used as concentrations so the amount eaten is per m-2.
  505: 		 * The water column, sediment and ice prey are already concentrations, the epibenthic prey are
  506: 		 * converted to a concentration over smLayerThick (the epibenthic m-2 / smLayerThick convention).
  507: 		 */
  508: 		if (FunctGroupArray[predatorGuildID].habitatType == EPIFAUNA) {
  509: 			if (habitat == EPIFAUNA)
  510: 				prey_eat /= smLayerThick;
  511: 		} else if (FunctGroupArray[predatorGuildID].habitatType == WC) {
  512: 			switch (habitat) {
  513: 			case EPIFAUNA:
  514: 				prey_eat /= wcLayerThick;
  515: 				break;
  516: 			case SED:
  517: 				prey_eat *= smLayerThick / wcLayerThick;
  518: 				break;
  519: 			case ICE_BASED:
  520: 				prey_eat *= iceLayerThick / wcLayerThick;
  521: 				break;
  522: 			default:
  523: 				break;
  524: 			}
  525: 		}
  526: 
  527: 		EATINGinfo[preyGuildID][prey_chrt][habitat] = (double)prey_eat;
  528: 
  529: 		/**
  530: 		if ((predatorGuildID == bm->which_check) || (preyGuildID == bm->which_check)){
  531: 			fprintf(llogfp,"Time: %e hab: %d, pred %s-%d on %s-%d has EATINGinfo: %.20e, CATCHEATINGinfo: %.20e, prey_avail: %.20e, pHscalar: %.20e\n",
```

**Suggested fix (from review):** Use a separate local (e.g. `nutScalar`) for the plant nutritional-value correction

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

## PROC-07

**division by zero / wrong loop bound** · Conf H / Sev M  
**Lines (current code):** 1094-1103  
**Lines at review (Oct 2026):** 981-990

**Function:** `Eat()`

**Why it's thought to be a bug**

eat_minmax: `rel_scalar = EATINGinfo/(EATINGinfo + catch_addition)` is 0/0 = NaN for any prey/habitat with zero availability, which makes spGRAZEinfo NaN and triggers the `_finite` quit at 1083. The loop also runs `EPIFAUNA..0` regardless of `max_hab` (WC only for vertebrate prey, where EATINGinfo[SED/EPI] is never set this call), so stale values get used. Function: `Eat()`.

**Current code** (`atecology/atprocess.c` lines 1092–1105)

```c
 1092: 					/* Calculate the amount actually eaten of each prey group - divide by tot_prey and then multiply with individual prey group biomasses */
 1093: 
 1094: 					for (habitat = WC; habitat <= max_hab; habitat++) {
 1095: 						spGRAZEinfo[preyID][kij][habitat] = EATINGinfo[preyID][kij][habitat] * (double)(scaled_clear / tot_prey);
 1096: 					}
 1097: 					spCATCHGRAZEinfo[preyID][kij] = CATCHEATINGinfo[preyID][kij] * (double)(scaled_clear / tot_prey);
 1098: 					scalar = pHscalar / tot_prey;
 1099:                     tprey = tot_prey;
 1100: 					break;
 1101:                 case eat_std_holling3:
 1102:                         for (habitat = WC; habitat <= max_hab; habitat++) {
 1103:                             spGRAZEinfo[preyID][kij][habitat] = EATINGinfo[preyID][kij][habitat] * EATINGinfo[preyID][kij][habitat] * (double)(scaled_clear / tot_prey_sq);
 1104:                         }
 1105:                         spCATCHGRAZEinfo[preyID][kij] = CATCHEATINGinfo[preyID][kij] * CATCHEATINGinfo[preyID][kij] * (double)(scaled_clear / tot_prey_sq);
```

**Suggested fix (from review):** Add `+ small_num` to the denominator and loop `habitat = max_hab..WC`

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

## PROC-08

**double-applied scalar** · Conf H / Sev M  
**Lines (current code):** 1745, 1778  
**Lines at review (Oct 2026):** 1585, 1618

**Function:** `Primary_Production()`

**Why it's thought to be a bug**

`mortality[0]` already includes `mortality_scalar` (1745), but calcTrackedMort adds `mortality_scalar * mortality[0] * Mdt`, which applies the scalar twice (including the HAB scalar). Invert_Activities line 1393 correctly omits it. Function: `Primary_Production()`.

**Current code** (`atecology/atprocess.c` lines 1743–1780)

```c
 1743: 	 fprintf(llogfp,"uptakeFE: %e, growth: %e, mort: %e, lysis: %e, scale_uptake: %e\n",
 1744: 	 *sp_uptakeFe, *sp_growth, *sp_mortality, *sp_lysis, scale_uptake);
 1745: 	 }
 1746: 	*/
 1747:      //if (((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop))){
 1748:     //if ((bm->current_box == bm->checkbox) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop) && (bm->which_check == sp_id)){
 1749:      if((bm->dayt > 3633) && (FunctGroupArray[sp_id].groupType == TURF) && (bm->current_box == 24)) {
 1750: 		fprintf(
 1751: 			llogfp,
 1752: 			"Time: %e Primary_Production outcomes: sp_id = %s, box%d-%d, growth = %.20e, uptakeNH = %.20e, uptakeNO = %.20e, uptakeSi = %.20e, uptakeFe = %.20e, mortality = %.20e, lysis = %.20e, maxPhagotrophy = %.20e, hN_sp = %.20e\n",
 1753: 			bm->dayt, FunctGroupArray[sp_id].groupCode, bm->current_box, bm->current_layer, FunctGroupArray[sp_id].growth[0], FunctGroupArray[sp_id].uptakeNH[0], *spUptakeNO, *spUptakeSi, *spUptakeFe,
 1754: 			FunctGroupArray[sp_id].mortality[0], FunctGroupArray[sp_id].lysis[0], FunctGroupArray[sp_id].maxPhagotrophy,
 1755: 			*sphN);
 1756: 
 1757: 		fprintf(llogfp,"Time: %e box%d-%d, %s sp_grow: %e, sp_biom: %e, mum: %e, hN_sp: %e, hI_sp: %e, eddy_strength: %e, eddy_scale: %e, min: %e, (SPmax = %e, area_hab = %e)\n",
 1758: 				bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[sp_id].groupCode, sp_grow, sp_biom, mum, hN_sp,
 1759: 					hI_sp, eddy_strength, bm->eddy_scale, min(1.0,max(0.0,(1.0 - sp_biom / (SPmax * area_hab + small_num)))), SPmax, area_hab);
 1760: 
 1761: 	}
 1762:     /**/
 1763:     
 1764: 	return;
 1765: }
 1766: 
 1767: /**
 1768:  *	\brief Encystment of primary producers - based on model by Hense and Beckmann (2010) Ecological Modelling 221:2330�2338
 1769:  *
 1770:  */
 1771: void Encystment(MSEBoxModel *bm, FILE *llogfp, int sp_id, double sp_biom, double DIN, double *sp_grow, double *cyst_transfer)
 1772: {
 1773: 	int encyst_rollover;
 1774: 	double this_grow = *sp_grow;
 1775: 	double encyst_rate = FunctGroupArray[sp_id].speciesParams[encyst_rate_id];
 1776: 	double hatch_rate = FunctGroupArray[sp_id].speciesParams[hatch_rate_id];
 1777: 	double crit_growth = FunctGroupArray[sp_id].speciesParams[crit_mum_id];
 1778: 	double crit_nut = FunctGroupArray[sp_id].speciesParams[crit_nut_id];
 1779: 	double crit_temp = FunctGroupArray[sp_id].speciesParams[crit_temp_id];
 1780: 	int hatch_TofY = (int)(FunctGroupArray[sp_id].speciesParams[overwinterEndTofY_id]);
```

**Suggested fix (from review):** Drop the outer `mortality_scalar *` at 1618

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

## PROC-09

**leftover debug** · Conf H / Sev L  
**Lines (current code):** 1803-1817  
**Lines at review (Oct 2026):** 1643-1657

**Function:** `Primary_Production()`

**Why it's thought to be a bug**

`if((bm->dayt > 3633) && (groupType == TURF) && (current_box == 24)) fprintf(llogfp, ...)` is live code (the surrounding `/** ... **/` closes at 1642). Any model with a TURF group and a box 24 writes two long lines per call after day 3633, which bloats the log. Function: `Primary_Production()`.

**Current code** (`atecology/atprocess.c` lines 1801–1819)

```c
 1801: 			this_encyst = encyst_rate * sp_biom;
 1802: 			cysts[sp_id][bm->current_box][cystenter_id] += this_encyst;
 1803: 		}
 1804: 		break;
 1805: 	case enviro_overwintering:
 1806: 		if(this_grow < crit_growth){
 1807: 			this_encyst = encyst_rate * sp_biom;
 1808: 			cysts[sp_id][bm->current_box][cystenter_id] += this_encyst;
 1809: 		}
 1810: 		break;
 1811: 	default: // Do nothing
 1812: 		break;
 1813: 	}
 1814: 
 1815: 	/* Deal with wrapping event */
 1816: 	if((hatch_TofY + encyst_period) > 365) {
 1817: 		encyst_rollover = (hatch_TofY + encyst_period) - 365;
 1818: 		hatch_TofY = 0;
 1819: 		encyst_period = encyst_rollover;
```

**Suggested fix (from review):** Comment out, or gate on bm->debug/which_check/checkbox

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

## PROC-10

**logic / copy-paste** · Conf H / Sev M  
**Lines (current code):** 1845-1849, 1873-1878  
**Lines at review (Oct 2026):** 1685-1689, 1711-1716

**Function:** `Encystment()`

**Why it's thought to be a bug**

Wrap handling: when `encyst_TofY + encyst_period > 365`, it sets `encyst_TofY = 0; encyst_period = rollover`, which drops the days from encyst_TofY to 365 from the window entirely. The hatch block then reuses the already-modified `encyst_period`, so the hatch window length is wrong whenever the encyst window wrapped. Function: `Encystment()`.

**Current code** (`atecology/atprocess.c` lines 1843–1880)

```c
 1843: 
 1844: 	// Sanity checks
 1845: 	if(cysts[sp_id][bm->current_box][cystenter_id] < 0.0)
 1846: 		cysts[sp_id][bm->current_box][cystenter_id] = 0.0;
 1847: 	if(cysts[sp_id][bm->current_box][cystexit_id] < 0.0)
 1848: 		cysts[sp_id][bm->current_box][cystexit_id] = 0.0;
 1849: 
 1850: 	if(bm->current_layer == (bm->boxes[bm->current_box].nz-1)){
 1851: 		// Add hatched cysts to population growth and ten zero out the cyst pool
 1852: 		*sp_grow += cysts[sp_id][bm->current_box][cystexit_id];
 1853: 		cysts[sp_id][bm->current_box][cystexit_id] = 0.0;
 1854: 	}
 1855: 
 1856: 	*cyst_transfer = this_encyst; // To be applied as mortality back in the calling routines routine
 1857: 
 1858: 	return;
 1859: }
 1860: 
 1861: /**
 1862:  *	\brief Availability of sediment fauna to epibenthos and pelagos
 1863:  *
 1864:  *	Note this writes predator specific (KDEP dependent) values into the shared spPREYinfo, so every predator
 1865:  *	eating sediment prey must call it before Eat() (likewise Calculate_Ice_Prey_Avail() for ice prey).
 1866:  */
 1867: void Calculate_Sediment_Prey_Avail(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, int guild, double ***spPREYinfo,
 1868: 		double *avail_BB) {
 1869: 	double depth_scalar = 1.0;
 1870: 	double biomass, avail, corrected_avail;
 1871: 	int prey, prey_chrt;
 1872: 	double KDEP_sp = FunctGroupArray[guild].speciesParams[KDEP_id];
 1873: 
 1874: 	*avail_BB = 0.0;
 1875: 	if (KDEP_sp < O2depth)
 1876: 		depth_scalar = (O2depth - KDEP_sp) / (O2depth + small_num);
 1877: 	else
 1878: 		depth_scalar = 1.0;
 1879: 
 1880: 	//	if ((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)) {
```

**Suggested fix (from review):** Test `TofY >= start || TofY <= rollover` for wrapping windows and use separate period variables

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

## PROC-11

**truncation / missing state** · Conf M / Sev M  
**Lines (current code):** 1525-1531  
**Lines at review (Oct 2026):** 1401-1407

**Function:** `Check_OverWintering()`

**Why it's thought to be a bug**

`overStartTemp = (int)(speciesParams[overwinterStartTemp_id])` truncates fractional temperature thresholds (2.5 becomes 2). `ans` is a local starting at 0, so when start < temp < end it returns 0: there is no hysteresis, even though the start/end pair implies one. Function: `Check_OverWintering()`.

**Current code** (`atecology/atprocess.c` lines 1523–1533) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
 1523:  *	\brief Primary productivity
 1524:  *
 1525:  */
 1526: void Primary_Production(MSEBoxModel *bm, FILE *llogfp, int sp_id, int micro_case, int lim_case, int macro_producer, double sp_biom,
 1527: 		double DIN, double NH, double NO, double Si, double Fe, double P, double PRatio, double C, double CRatio,
 1528: 		double IRR, double mum, double E_sp, double mL_other, double SPmax,
 1529: 		double area_hab, double *spUptakeNO,
 1530: 		double *spUptakeSi, double *spUptakeFe, double *spUptakeP,  double *spUptakeC, double *sphN) {
 1531: 
 1532: 	double hN_sp, hI_sp, sp_grow, scale_uptake, uptakeNH, uptakeNO, uptakeSi, uptakeFe, uptakeP = 0, uptakeC = 0;
 1533: 	double mL_sp = Ecology_Get_Linear_Mortality(bm, sp_id, 0, 0) + mL_other;
```

**Suggested fix (from review):** Remove the `(int)` casts; return the previous overwintering state when between the thresholds

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

## PROC-12

**formula inconsistent** · Conf M / Sev M  
**Lines (current code):** 2342-2356  
**Lines at review (Oct 2026):** 2179-2193

**Function:** `Calculate_SedBact_Scale()`

**Why it's thought to be a bug**

`BB_DL = BB*Bact_stim*XBB_DL*hO_BB/(Bact_stim*XBB_DL*DLsed*hO_BB + ...)`: the numerator is missing `DLsed` (same for BB_DR/DRsed), and for habitatType==SED it hard-codes `DLsed = DRsed = 1.0`. Compare Calculate_PelagicBact_Scale (2165, `PB*XPB_DL*DL/(...)`) and Sediment_Bacterica_Process (atGroupProcesses.c:1827, numerator includes DL). Callers then divide by DLsed again (Invert_Activities 1238), so sediment bacteria grazing is mis-scaled. Function: `Calculate_SedBact_Scale()`.

**Current code** (`atecology/atprocess.c` lines 2340–2358) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
 2340: 
 2341: 	preystock = bm->group_stock[prey][bm->current_box][bm->current_layer];
 2342: 	
 2343: 	// Need this for the diet data
 2344: 
 2345: 	stock_id = bm->group_stock[guildcase][bm->current_box][bm->current_layer];
 2346: 	
 2347: 	step1 = 0;
 2348: 	if(FunctGroupArray[prey].groupAgeType == AGE_STRUCTURED){
 2349: 		/* Sort out numbers eaten for age structured (e.g. vertebrate) groups */
 2350: 		step1 = GRAZEinfo[prey][prey_chrt][habitat] * bm->cell_vol / (VERTinfo[prey][prey_chrt][SN_id] + VERTinfo[prey][prey_chrt][RN_id] + small_num);
 2351: 
 2352: /*
 2353:  * This very ugly code has been added to help people that were using the old bec_dev code to migrate to the new trunk code.
 2354:  * Turning this on will hopefully result in very similar results.
 2355:  */
 2356: 		if(bm->flag_replicated_old == TRUE){
 2357: 
 2358: 			/**
```

**Suggested fix (from review):** Use the actual sediment DL/DR and include them in the numerator, as at atGroupProcesses.c:1827

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

## PROC-13

**double counting** · Conf M / Sev M  
**Lines (current code):** 2159, 2204-2205  
**Lines at review (Oct 2026):** 1996, 2041-2042

**Function:** `Calculate_Catch()`

**Why it's thought to be a bug**

For invertebrates, `biomassToDL` is added to `DetritusProd[WC][DCdet_id]` (2159) and then again to `DetritusProdGlobal[WC][WC][DLdet_id]` (divided by propSediment) at 2042, so the discard waste nitrogen is created twice. Function: `Calculate_Catch()`.

**Current code** (`atecology/atprocess.c` lines 2157–2161) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
 2157:  * \brief Get the area habitat of the given species/cohort.
 2158:  *
 2159:  *
 2160:  */
 2161: double Get_Species_Area_Hab(MSEBoxModel *bm, int guild, int cohort, BoxLayerValues *boxLayerInfo) {
```

**Suggested fix (from review):** Add biomassToDL to only one pool

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

## PROC-14

**unit mismatch** · Conf M / Sev H  
**Lines (current code):** 2619-2625  
**Lines at review (Oct 2026):** 2459-2465

**Function:** `Acidif_Mort()`

**Why it's thought to be a bug**

McElhany-Bush: `annualSurvivalNoAcid = exp(-pHmortB)` is annual, so `-log(1-annualSurvivalDecrement)` is an annual instantaneous rate. Dividing by 86400 alone (named `mortDueToAcidPerDay`) treats it as a daily rate, which makes acidification mortality about 365x too high. Function: `Acidif_Mort()`.

**Current code** (`atecology/atprocess.c` lines 2617–2627) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
 2617:                 }
 2618:                 
 2619:                 //tmp = FunctGroupArray[sp].speciesParams[RSslope_id] * (this_age - FunctGroupArray[sp].speciesParams[RSmid_id]);
 2620:                 if (bm->flag_do_evolution && DNA[sp].trait_active[rsmid_trait_id]) {
 2621:                     tmp = FunctGroupArray[sp].speciesParams[RSslope_id] * (this_age - bm->evolTraitOriginalValues[sp][chrt][rsmid_trait_id]);
 2622:                 } else {
 2623:                     tmp = FunctGroupArray[sp].speciesParams[RSslope_id] * (this_age - FunctGroupArray[sp].speciesParams[RSmid_id]);
 2624:                 }
 2625:                 if (tmp < (-1.0 * bm->XRS_cap)) 	// Added bounds to prevent numerical issues
 2626:                     tmp = (-1.0 * bm->XRS_cap);
 2627:                 
```

**Suggested fix (from review):** Divide by `365.0*86400.0` (or document pHmortB as a daily rate)

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

## PROC-15

**no-op logic** · Conf M / Sev M  
**Lines (current code):** 1695-1714  
**Lines at review (Oct 2026):** 1535-1554

**Function:** `Primary_Production()`

**Why it's thought to be a bug**

`scale_uptake_NO = max(scale_uptake, NO/uptakeNO); scale_uptake = min(scale_uptake, scale_uptake_NO);`. The max guarantees scale_uptake_NO >= scale_uptake, so the min never changes scale_uptake. The NO, Si and Fe caps therefore never take effect, and uptake of those nutrients can exceed what is available. Only the NH cap is active. Function: `Primary_Production()`.

**Current code** (`atecology/atprocess.c` lines 1693–1716)

```c
 1693: 
 1694: 		/* Calculate P uptake */
 1695: 		uptakeP = Calculate_Phosphorus_Uptake(bm, sp_id, sp_grow, PRatio);
 1696: 
 1697: 		/* Now do carbon */
 1698: 		uptakeC = Calculate_Carbon_Uptake(bm, sp_id, sp_grow, CRatio);
 1699: 	}
 1700: 
 1701: 	if(bm->ecotest > 1){
 1702: 		if(!_finite(FunctGroupArray[sp_id].uptakeNH[0])){
 1703: 			printf("group %s - uptakeNH = %e, scale_uptake = %e, DIN = %e, NH= %e, sp_grow= %e\n",
 1704: 					FunctGroupArray[sp_id].groupCode, uptakeNH, scale_uptake, DIN, NH, sp_grow);
 1705: 			quit("ERROR: uptakeNH is infinite.\n");
 1706: 		}
 1707: 	}
 1708: 	*spUptakeNO = uptakeNO * scale_uptake;
 1709: 	*spUptakeSi = uptakeSi * scale_uptake;
 1710: 	*spUptakeFe = uptakeFe * scale_uptake;
 1711: 	*spUptakeP = uptakeP * scale_uptake;
 1712: 	*spUptakeC = uptakeC * scale_uptake;
 1713: 
 1714: 	if(bm->track_atomic_ratio == TRUE){
 1715: 		PP_uptake(bm, sp_id, FunctGroupArray[sp_id].uptakeNH[0] + *spUptakeNO, *spUptakeP , *spUptakeC);
 1716: 	}
```

**Suggested fix (from review):** Use `min(1.0, NO/uptakeNO)` for each nutrient's own scale, then take the overall min

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

## PROC-16

**division by zero** · Conf M / Sev M  
**Lines (current code):** 1664-1674  
**Lines at review (Oct 2026):** 1517-1520

**Function:** `Primary_Production()`

**Why it's thought to be a bug**

`uptakeNH = sp_grow*(NH/(KN+NH))*((KN+DIN)/DIN)`, and uptakeNO also divides by DIN. When DIN == 0, sp_grow is 0, so this is 0*inf = NaN. NaN fails both `>0` and `<0`, so it is stored in uptakeNH and NutsLost. Function: `Primary_Production()`.

**Current code** (`atecology/atprocess.c` lines 1662–1676)

```c
 1662:         }
 1663:     }
 1664: 
 1665: 	/* Assign uptake */
 1666: 	FunctGroupArray[sp_id].uptakeNH[0] = uptakeNH * scale_uptake;
 1667:     
 1668:     /*
 1669:     //if ((bm->dayt >= bm->checkstart) && (bm->checkbox == bm->current_box)) {
 1670:     if (bm->dayt >= bm->checkstart) {
 1671:         fprintf(llogfp, "Time %e box%d-%d %s sp_uptakeNH: %e uptakeNH: %e scale_uptake: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[sp_id].groupCode, FunctGroupArray[sp_id].uptakeNH[0], uptakeNH, scale_uptake);
 1672:     }
 1673:     */
 1674: 
 1675: 	/* If there is a scalar to apply then grab it now */
 1676: 	if(bm->scale_all_mortality == TRUE){
```

**Suggested fix (from review):** Guard with `DIN > 0` (else uptake = 0)

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

## PROC-17

**wrong variable / index (latent)** · Conf H / Sev L  
**Lines (current code):** ≈622-630  
**Lines at review (Oct 2026):** 560-568

**Function:** `Get_Predator_Competition()`

**Why it's thought to be a bug**

Unreachable for now (Eat quits before calling it), but: the habitat coefficients use `FunctGroupArray[sp_id]` rather than `[predID]`; the else branch indexes `sm.tr[bm->current_layer]` with a water-column layer index (can exceed sediment nz); the epi biomass is indexed by `totNTracers` regardless of tracer type; and the loop includes non-predator groups. Function: `Get_Predator_Competition()`.

**Current code** (`atecology/atprocess.c` lines 620–632) — ≈ the code here has changed since the review; lines are approximate

```c
  620:             if (current_layer_sed > -1) { // Need sediment based predators
  621:                     
  622:             } else if (current_layer_sed > 0) {
  623:                     
  624:             } else {
  625:                     
  626:             }
  627:         }
  628:     }
  629:     
  630:     *tot_pred_comp = ans_tot_pred_comp;
  631:     *tot_pred = ans_tot_pred;
  632:     return;
```

**Suggested fix (from review):** Fix before enabling the ratio-dependent/HV/CM cases

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

## PROC-18

**division by zero** · Conf M / Sev L  
**Lines (current code):** 316-323, 1476  
**Lines at review (Oct 2026):** 316-323, 1476

**Function:** `Light_Lim()`

**Why it's thought to be a bug**

Primary_Production (and atcoral/atmacrophytes) pass `PBmax_D = 0.0`. If bm->flaglight is irr_ice_v1, `step2 = 0`, so `step1*amt_irr/step2` is a division by zero (and irr_ice_v2 gives `tanh(x/0)`). Function: `Light_Lim()`.

**Current code** (`atecology/atprocess.c` lines 314–325)

```c
  314: 
  315: 		*/
  316: 		step1 = -1.0 * KI_sp / bm->X_CN; 	// C to N
  317: 		step2 = PBmax_D / bm->X_CN;
  318: 
  319: 		// Light limitation with photoadaptation
  320: 		hI = (1.0 - exp(step1 * amt_irr / step2)) * exp(-1.0 * opt_irr * amt_irr / step2);
  321: 		break;
  322:     case irr_ice_v2:
  323:         hI = tanh(this_a * amt_irr / PBmax_D);
  324:         break;
  325: 	default:
```

**Suggested fix (from review):** Reject the ice light cases for flaglight, or pass the real PBmax

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

atprocess.c `Calculate_Catch()` 2042: divides by `propSediment`. All current callers pass 1.0 or smLayerThick, so this is not a live divide-by-zero.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

`Encystment()` 1730: cysts hatch when `DIN < crit_nut`, i.e. under low nutrients. Check this is the intended direction (Hense & Beckmann).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-3

`Get_Extra_Feed()` 627-634: the supplementation spatial scalar `SUPPdistrib` is applied only to adults, while juveniles get scalar 1.0.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-4

`Create_Update_XRS()` 2517 uses `evolTraitOriginalValues` for RSmid when evolution is active. Check that the evolved, not the original, trait value is meant.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-5

`UpdateTrackedMort()` 2279: `step1 / sizeScalar` can divide by zero (only under flag_replicated_old with !UseHardFeedingWindow, which deliberately replicates the old code).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
