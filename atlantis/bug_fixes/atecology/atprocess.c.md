# atecology/atprocess.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **18 findings** (severity H: 3; confidence H and severity H: 2; on the priority list: 2), plus 5 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PROC-01](#proc-01) ⭐ | 420, 800, 1096 | H/H | wrong index / buffer overrun | `bm->pSPFCeat[sp_id][preyID][cohort][kij]` (and `[predatorID][preyID][cohort][prey_chrt]` at 420). pSPFCeat is… |  | [ ] |
| [PROC-02](#proc-02) | 795-801, 1094 | H/M | accumulator not reset | `bm->groupTotCatch[preyID][kij] += FCcaughttemp*pFLEET` and then `*= pSPFCeat`. groupTotCatch is only allocate… |  | [ ] |
| [PROC-03](#proc-03) ⭐ | 1074-1076 | H/H | wrong variable / buffer overrun | `thisID = (int)speciesParams[coralID_id]; ... FunctGroupArray[thisID].cohortSpeciesParams[kij][rugosity_dec_id… |  | [ ] |
| [PROC-04](#proc-04) | 1005-1006, 1013-1014, 1162, 1165 | H/M | double division | For the std_holling/ratio/HV/CM cases, Eat sets `scalar = pHscalar / tot_prey; tprey = tot_prey;`. Do_Extra_Fe… |  | [ ] |
| [PROC-05](#proc-05) | 646-652 vs 1143-1165; 1244 | H/M | wrong index | Get_Extra_Feed stores feed availability in `EATINGinfo/FEEDinfo[AquacultFeedIndex][cohort][WC]`, but Do_Extra_… |  | [ ] |
| [PROC-06](#proc-06) | 462-466, 483, 504-509 | H/M | variable overwritten in loop | `pHscalar` is set before the habitat loop (1/pHcorr for the predation-availability effect), but for plant prey… |  | [ ] |
| [PROC-07](#proc-07) | 981-990 | H/M | division by zero / wrong loop bound | eat_minmax: `rel_scalar = EATINGinfo/(EATINGinfo + catch_addition)` is 0/0 = NaN for any prey/habitat with zer… |  | [ ] |
| [PROC-08](#proc-08) | 1585, 1618 | H/M | double-applied scalar | `mortality[0]` already includes `mortality_scalar` (1585), but calcTrackedMort adds `mortality_scalar * mortal… |  | [ ] |
| [PROC-09](#proc-09) | 1643-1657 | H/L | leftover debug | `if((bm->dayt > 3633) && (groupType == TURF) && (current_box == 24)) fprintf(llogfp, ...)` is live code (the s… |  | [ ] |
| [PROC-10](#proc-10) | 1685-1689, 1711-1716 | H/M | logic / copy-paste | Wrap handling: when `encyst_TofY + encyst_period > 365`, it sets `encyst_TofY = 0; encyst_period = rollover`, … |  | [ ] |
| [PROC-11](#proc-11) | 1401-1407 | M/M | truncation / missing state | `overStartTemp = (int)(speciesParams[overwinterStartTemp_id])` truncates fractional temperature thresholds (2.… |  | [ ] |
| [PROC-12](#proc-12) | 2179-2193 | M/M | formula inconsistent | `BB_DL = BB*Bact_stim*XBB_DL*hO_BB/(Bact_stim*XBB_DL*DLsed*hO_BB + ...)`: the numerator is missing `DLsed` (sa… |  | [ ] |
| [PROC-13](#proc-13) | 1996, 2041-2042 | M/M | double counting | For invertebrates, `biomassToDL` is added to `DetritusProd[WC][DCdet_id]` (1996) and then again to `DetritusPr… |  | [ ] |
| [PROC-14](#proc-14) | 2459-2465 | M/H | unit mismatch | McElhany-Bush: `annualSurvivalNoAcid = exp(-pHmortB)` is annual, so `-log(1-annualSurvivalDecrement)` is an an… |  | [ ] |
| [PROC-15](#proc-15) | 1535-1554 | M/M | no-op logic | `scale_uptake_NO = max(scale_uptake, NO/uptakeNO); scale_uptake = min(scale_uptake, scale_uptake_NO);`. The ma… |  | [ ] |
| [PROC-16](#proc-16) | 1517-1520 | M/M | division by zero | `uptakeNH = sp_grow*(NH/(KN+NH))*((KN+DIN)/DIN)`, and uptakeNO also divides by DIN. When DIN == 0, sp_grow is … |  | [ ] |
| [PROC-17](#proc-17) | 560-568 | H/L | wrong variable / index (latent) | Unreachable for now (Eat quits before calling it), but: the habitat coefficients use `FunctGroupArray[sp_id]` … |  | [ ] |
| [PROC-18](#proc-18) | 316-323, 1476 | M/L | division by zero | Primary_Production (and atcoral/atmacrophytes) pass `PBmax_D = 0.0`. If bm->flaglight is irr_ice_v1, `step2 = … |  | [ ] |

---

## PROC-01

**wrong index / buffer overrun** · Conf H / Sev H ⭐ priority  
**Listed lines:** 420, 800, 1096

**Why it's thought to be a bug**

`bm->pSPFCeat[sp_id][preyID][cohort][kij]` (and `[predatorID][preyID][cohort][prey_chrt]` at 420). pSPFCeat is allocated `Util_Alloc_Init_4D_Double(K_num_max_stages, K_num_max_stages, K_num_tot_sp, K_num_tot_sp)` => `[sp][sp][stage][stage]` (atlantismain/atlantismain.c:1914), and it is indexed by stage in atvertprocesses.c:729/738 (`[chrtstage][preyage]`). Here it uses the cohort x genotype indices `cohort`/`kij`, which go past K_num_max_stages (normally 2), so the read is out of bounds and returns the wrong preference. Live whenever catcheater && flag_fisheries_on && !flag_olddiet. Functions: `Get_Catch_Prey()`, `Eat()`.

**Current code** (`atecology/atprocess.c` lines 418–422)

```c
  418: 
  419: 		if(!bm->flag_olddiet)
  420: 			step1 *= bm->pSPFCeat[predatorID][preyID][cohort][prey_chrt];
  421: 
  422: 		prey_avail += step1;
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
**Listed lines:** 795-801, 1094

**Function:** `Eat()`

**Why it's thought to be a bug**

`bm->groupTotCatch[preyID][kij] += FCcaughttemp*pFLEET` and then `*= pSPFCeat`. groupTotCatch is only allocated (atbiolsetup.c:2901) and freed. A repo-wide grep finds no reset, so it keeps growing across predators, cells and timesteps, and the `*=` scales the whole running total, not just this predator's share. The `ratio` denominator at 1094 is therefore far too big, so too little catch is taken out of FCcaughttemp/Catch. Function: `Eat()`.

**Current code** (`atecology/atprocess.c` lines 888–898) — code appears to have moved +95 lines since the review

```c
  888: 			if (catcheater && bm->flag_fisheries_on){
  889: 				for (fleet = 0; fleet < bm->K_num_fisheries; fleet++) {
  890: 					bm->groupTotCatch[preyID][kij] += bm->FCcaughttemp[preyID][fleet][kij] * bm->pFLEET[sp_id][fleet];
  891: 				}
  892: 				if(!bm->flag_olddiet)
  893: 					bm->groupTotCatch[preyID][kij] *= bm->pSPFCeat[sp_id][preyID][cohort][kij];
  894: 			}
  895: 
  896: 			if(flagcase == eat_minmax){
  897: 				max_hab = bm->num_active_habitats - 1;
  898: 				if(FunctGroupArray[preyID].isVertebrate == TRUE)
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
**Listed lines:** 1074-1076

**Function:** `Eat()`

**Why it's thought to be a bug**

`thisID = (int)speciesParams[coralID_id]; ... FunctGroupArray[thisID].cohortSpeciesParams[kij][rugosity_dec_id]`. coralID_id holds the CORALREEF index 0..ncoral-1 (atbiolsetup.c:3908), not a guild, so this reads the parameters of an unrelated group (e.g. guild 0). cohortSpeciesParams is also sized `[numStages]` (atUtilGroupIO.c:830), but here it is indexed by `kij` (cohort x genotype), which can overrun it. Live with flag_adv_habitat whenever coral or sponge is eaten. Function: `Eat()`.

**Current code** (`atecology/atprocess.c` lines 1072–1078)

```c
 1072: 
 1073: 					rel_scalar = 0.0;
 1074: 					for (habitat = EPIFAUNA; habitat >= 0; habitat--) {
 1075: 						if(habitat == WC){
 1076: 							catch_addition = CATCHEATINGinfo[preyID][kij];
 1077: 						} else {
 1078: 							catch_addition = 0.0;
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
**Listed lines:** 1005-1006, 1013-1014, 1162, 1165

**Why it's thought to be a bug**

For the std_holling/ratio/HV/CM cases, Eat sets `scalar = pHscalar / tot_prey; tprey = tot_prey;`. Do_Extra_Feed then computes `EATINGinfo*CLEAR*scalar/(tprey+small_num)`, so aquaculture feed consumption is divided by tot_prey twice (tot_prey_sq twice for holling3). `scalar` also carries the pH correction of whichever prey group was processed last, which has nothing to do with the feed. Functions: `Eat()`, `Do_Extra_Feed()`.

**Current code** (`atecology/atprocess.c` lines 1003–1008)

```c
 1003: 			} else {
 1004: 				pHscalar = 1.0;
 1005: 			}
 1006: 			scalar = pHscalar;
 1007:             scaled_clear = pHscalar * CLEAR;
 1008: 
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
**Listed lines:** 646-652 vs 1143-1165; 1244

**Why it's thought to be a bug**

Get_Extra_Feed stores feed availability in `EATINGinfo/FEEDinfo[AquacultFeedIndex][cohort][WC]`, but Do_Extra_Feed always reads `[AquacultFeedIndex][0][WC]` and writes spGRAZEinfo `[...][0]`. Invert_Activities (1244) also reads `GRAZEinfo[AquacultFeedIndex][0][WC]`. For any cohort > 0, a cultured or supplemented group eats feed sized from cohort 0's (stale) availability. Functions: `Get_Extra_Feed()`, `Do_Extra_Feed()`.

**Current code** (`atecology/atprocess.c` lines 639–649) — code appears to have moved -5 lines since the review

```c
  639:  *
  640:  */
  641: void Get_Extra_Feed(MSEBoxModel *bm, FILE *llogfp, int sp_id, int cohort, double sp_Biomass, int flagcase, double KL_sp, long double *living_prey,
  642:                     long double *living_prey_sq, double *denom_step) {
  643:     int do_adult = 0;
  644:     int do_juv = 0;
  645:     int do_feeding = 0;
  646:     int stage;
  647:     double scalar = 1.0;
  648:     
  649:     if (FunctGroupArray[sp_id].isSupplemented > 0) {
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
**Listed lines:** 462-466, 483, 504-509

**Function:** `Calculate_PreyAvail()`

**Why it's thought to be a bug**

`pHscalar` is set before the habitat loop (1/pHcorr for the predation-availability effect), but for plant prey it is overwritten inside the loop (`pHscalar = pHcorr` or 1.0). From the 2nd habitat on, `prey_eat = prey_avail * pHscalar` uses the nutritional-value scalar instead of the predation scalar, and the 1/pHcorr effect is lost or inverted for SED/EPIFAUNA plant prey. Function: `Calculate_PreyAvail()`.

**Current code** (`atecology/atprocess.c` lines 460–499 (truncated))

```c
  460: 	 */
  461: 	pHsensitive_sp = (int) (FunctGroupArray[preyGuildID].speciesParams[flagpredavaileffect_id]);
  462: 	if(pHsensitive_sp){
  463: 		pHscalar = (1.0 / FunctGroupArray[preyGuildID].pHcorr);
  464: 	} else {
  465: 		pHscalar = 1.0;
  466: 	}
  467: 
  468: 	/* Zero diet link: the availability below is exactly 0 in every habitat, so the
  469: 	 * prey totals would only gain +0. pSPVERTeat is fixed once the parameters are read. */
  470: 	if (!bm->flag_olddiet && !bm->debug
  471: 			&& !(bm->flag_macro_model && (FunctGroupArray[preyGuildID].groupType == SEAGRASS))
  472: 			&& !(catcheater && bm->flag_fisheries_on)
  473: 			&& _finite((double) pHscalar) && _finite(FunctGroupArray[preyGuildID].pHcorr)
  474: 			&& (bm->pSPVERTeat[predatorGuildID][preyGuildID][chrtstage][FunctGroupArray[preyGuildID].cohort_stage[prey_chrt]] == 0.0)) {
  475: 		for (habitat = WC; habitat <= max_hab; habitat++) {
  476: 			EATINGinfo[preyGuildID][prey_chrt][habitat] = 0.0;
  477: 		}
  478: 		return;
  479: 	}
  480: 
  481: 	/* Piggy back on the habitat loops for the seagrass cohorts */
  482: 	for (habitat = WC; habitat <= max_hab; habitat++) {
  483: 
  484: 
  485: 		/* Get the prey availability for the pre calculated array */
  486: 		prey_avail = Get_Gape_Lim_Prey(bm, llogfp, predatorGuildID, cohort, chrtstage, preyGuildID, prey_chrt, habitat, spPREYinfo);
  487: 
  488: 
  489: 		if (habitat == WC && catcheater && bm->flag_fisheries_on){
  490: 			catch_avail = Get_Catch_Prey(bm, llogfp, predatorGuildID, cohort, chrtstage, preyGuildID, prey_chrt, habitat);
  491: 			catch_eat = catch_avail * pHscalar;
  492: 			CATCHEATINGinfo[preyGuildID][prey_chrt] = (double)catch_eat;
  493: 		}
  494: 
  495: 		/* determine biomass available */
  496: 		prey_eat =  prey_avail * pHscalar;
  497: 
  498: 		/* PREYinfo holds each habitat in its own units: WC mg N m-3 water, SED mg N m-3 sediment,
  499: 		 * EPIFAUNA mg N m-2, ICE_BASED mg N m-3 ice - so put the prey on a consistent basis for the predator.
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
**Listed lines:** 981-990

**Function:** `Eat()`

**Why it's thought to be a bug**

eat_minmax: `rel_scalar = EATINGinfo/(EATINGinfo + catch_addition)` is 0/0 = NaN for any prey/habitat with zero availability, which makes spGRAZEinfo NaN and triggers the `_finite` quit at 1083. The loop also runs `EPIFAUNA..0` regardless of `max_hab` (WC only for vertebrate prey, where EATINGinfo[SED/EPI] is never set this call), so stale values get used. Function: `Eat()`.

**Current code** (`atecology/atprocess.c` lines 904–917) — code appears to have moved -75 lines since the review

```c
  904: 						catch_addition = 0.0;
  905: 					}
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
  917: 
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
**Listed lines:** 1585, 1618

**Function:** `Primary_Production()`

**Why it's thought to be a bug**

`mortality[0]` already includes `mortality_scalar` (1585), but calcTrackedMort adds `mortality_scalar * mortality[0] * Mdt`, which applies the scalar twice (including the HAB scalar). Invert_Activities line 1269 correctly omits it. Function: `Primary_Production()`.

**Current code** (`atecology/atprocess.c` lines 1606–1643) — code appears to have moved +23 lines since the review

```c
 1606: 	 *
 1607: 	 * Cysts "hatching" = add to growth
 1608: 	 * Cysts forming and sinking = local additional mortality with cysts being moved into a global to settle on the bottom
 1609: 	 *
 1610: 	 */
 1611: 	FunctGroupArray[sp_id].speciesParams[overwintering_id] = Check_OverWintering(bm, llogfp, sp_id);
 1612: 	if (FunctGroupArray[sp_id].speciesParams[overwintering_id] > 0){
 1613: 		Encystment(bm, llogfp, sp_id, sp_biom, DIN, &sp_grow, &cyst_transfer);
 1614: 	}
 1615: 
 1616: 	FunctGroupArray[sp_id].growth[0] = sp_grow;
 1617: 
 1618: 	/* End loop over strata */
 1619: 
 1620: 	/* Calculate nutrient uptake */
 1621: 	uptakeNH = sp_grow * (NH / (FunctGroupArray[sp_id].speciesParams[KN_id] + NH)) * ((FunctGroupArray[sp_id].speciesParams[KN_id] + DIN) / DIN);
 1622: 	uptakeNO = sp_grow * (NO / (FunctGroupArray[sp_id].speciesParams[KN_id] + DIN))
 1623: 			* (FunctGroupArray[sp_id].speciesParams[KN_id]	/ (FunctGroupArray[sp_id].speciesParams[KN_id] + NH))
 1624: 			* (1.0 + FunctGroupArray[sp_id].speciesParams[KN_id] / DIN);
 1625: 
 1626:     if (lim_case != one_nut_lim) {
 1627:         uptakeSi = sp_grow * bm->X_SiN;
 1628:     } else {
 1629:         uptakeSi = 0.0;
 1630:     }
 1631: 
 1632:     if (micro_case) {
 1633:         uptakeFe = sp_grow * bm->X_FeN;
 1634:     } else {
 1635:         uptakeFe = 0.0;
 1636:     }
 1637: 
 1638: 	/* Cap nutrient uptake to what is available - this intentionally a max as it is rescaling the NH uptake based on which is the least limiting of the limiting nutients */
 1639: 	scale_uptake = 1.0;
 1640:     if(uptakeNH > 0) {
 1641:         if (uptakeNH > NH) {
 1642:             scale_uptake_NH = NH / uptakeNH;
 1643:         }
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
**Listed lines:** 1643-1657

**Function:** `Primary_Production()`

**Why it's thought to be a bug**

`if((bm->dayt > 3633) && (groupType == TURF) && (current_box == 24)) fprintf(llogfp, ...)` is live code (the surrounding `/** ... **/` closes at 1642). Any model with a TURF group and a box 24 writes two long lines per call after day 3633, which bloats the log. Function: `Primary_Production()`.

**Current code** (`atecology/atprocess.c` lines 1667–1685) — code appears to have moved +26 lines since the review

```c
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
 1677: 		mortality_scalar = Ecology_Get_Mortality_Scalar(bm, sp_id, 0);
 1678: 	}else{
 1679: 		mortality_scalar = 1.0;
 1680: 	}
 1681:     
 1682:     if(bm->flagbayHAB == TRUE){
 1683:         if(HAB_boxi[bm->current_box] > 0) {
 1684:             mortality_scalar *= FunctGroupArray[sp_id].speciesParams[HAB_mort_scalar_id];
 1685:         }
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
**Listed lines:** 1685-1689, 1711-1716

**Function:** `Encystment()`

**Why it's thought to be a bug**

Wrap handling: when `encyst_TofY + encyst_period > 365`, it sets `encyst_TofY = 0; encyst_period = rollover`, which drops the days from encyst_TofY to 365 from the window entirely. The hatch block then reuses the already-modified `encyst_period`, so the hatch window length is wrong whenever the encyst window wrapped. Function: `Encystment()`.

**Current code** (`atecology/atprocess.c` lines 1771–1806) — code appears to have moved +88 lines since the review

```c
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
 1781: 	int encyst_TofY = (int)(FunctGroupArray[sp_id].speciesParams[overwinterStartTofY_id]);
 1782: 	int encyst_period = (int)(FunctGroupArray[sp_id].speciesParams[encyst_period_id]);
 1783: 	int flagencyst = (int)(FunctGroupArray[sp_id].speciesParams[flagencyst_id]);
 1784: 
 1785: 	double this_cyst_hatch = 0.0;
 1786: 	double this_encyst = 0.0;
 1787: 
 1788: 	/* Deal with wrapping event */
 1789: 	if((encyst_TofY + encyst_period) > 365) {
 1790: 		encyst_rollover = (encyst_TofY + encyst_period) - 365;
 1791: 		encyst_TofY = 0;
 1792: 		encyst_period = encyst_rollover;
 1793: 	}
 1794: 
 1795: 	/* Form cysts */
 1796: 	switch(flagencyst){
 1797: 	case no_overwintering: // Do nothing
 1798: 		break;
 1799: 	case date_overwintering:
 1800: 		if((encyst_TofY <= bm->TofY) && (bm->TofY <= (encyst_TofY + encyst_period))) {
 1801: 			this_encyst = encyst_rate * sp_biom;
 1802: 			cysts[sp_id][bm->current_box][cystenter_id] += this_encyst;
 1803: 		}
 1804: 		break;
 1805: 	case enviro_overwintering:
 1806: 		if(this_grow < crit_growth){
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
**Listed lines:** 1401-1407

**Function:** `Check_OverWintering()`

**Why it's thought to be a bug**

`overStartTemp = (int)(speciesParams[overwinterStartTemp_id])` truncates fractional temperature thresholds (2.5 becomes 2). `ans` is a local starting at 0, so when start < temp < end it returns 0: there is no hysteresis, even though the start/end pair implies one. Function: `Check_OverWintering()`.

**Current code** (`atecology/atprocess.c` lines 1477–1487) — code appears to have moved +78 lines since the review

```c
 1477: 	int overStartDay = 0;
 1478: 	int overEndDay = 0;
 1479: 	double overStartTemp = 0.0;
 1480: 	double overEndTemp = 0.0;
 1481: 
 1482: 	switch(FunctGroupArray[sp_id].isOverWinter) {
 1483: 	case no_overwintering:
 1484: 		ans = 0;
 1485: 		break;
 1486: 	case date_overwintering:
 1487: 		overStartDay = (int)(FunctGroupArray[sp_id].speciesParams[overwinterStartTofY_id]);
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
**Listed lines:** 2179-2193

**Function:** `Calculate_SedBact_Scale()`

**Why it's thought to be a bug**

`BB_DL = BB*Bact_stim*XBB_DL*hO_BB/(Bact_stim*XBB_DL*DLsed*hO_BB + ...)`: the numerator is missing `DLsed` (same for BB_DR/DRsed), and for habitatType==SED it hard-codes `DLsed = DRsed = 1.0`. Compare Calculate_PelagicBact_Scale (2165, `PB*XPB_DL*DL/(...)`) and Sediment_Bacterica_Process (atGroupProcesses.c:1807, numerator includes DL). Callers then divide by DLsed again (Invert_Activities 1238), so sediment bacteria grazing is mis-scaled. Function: `Calculate_SedBact_Scale()`.

**Current code** (`atecology/atprocess.c` lines 2297–2315) — code appears to have moved +120 lines since the review

```c
 2297: 
 2298: 		hO_BB = Oxygen(1, O2, 0.0, 0.0, FunctGroupArray[SedBactIndex].speciesParams[mD_id]);
 2299: 		boxLayerInfo->BB_DL = BB * Bact_stim * XBB_DL * hO_BB / (Bact_stim * XBB_DL * DLsed * hO_BB + Bact_stim * XBB_DR * DRsed * hO_BB + small_num);
 2300: 		boxLayerInfo->BB_DR = BB * Bact_stim * XBB_DR * hO_BB / (Bact_stim * XBB_DL * DLsed * hO_BB + Bact_stim * XBB_DR * DRsed * hO_BB + small_num);
 2301: 	} else {
 2302: 		boxLayerInfo->BB_DL = 0.0;
 2303: 		boxLayerInfo->BB_DR = 0.0;
 2304: 	}
 2305: }
 2306: 
 2307: /**
 2308:  * \brief Update the tracked mortality record after Eat()
 2309:  *
 2310:  *
 2311:  */
 2312: void UpdateTrackedMort(MSEBoxModel *bm, FILE *llogfp, int guildcase, int cohort, HABITAT_TYPES level_id, HABITAT_TYPES habitat, int prey, int prey_chrt,
 2313: 		BoxLayerValues *boxLayerInfo, double scalar, int global_contrib) {
 2314: 	double biomass_correction, step1;
 2315:     int preystock, stock_id, this_layer;
```

**Suggested fix (from review):** Use the actual sediment DL/DR and include them in the numerator, as at atGroupProcesses.c:1807

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
**Listed lines:** 1996, 2041-2042

**Function:** `Calculate_Catch()`

**Why it's thought to be a bug**

For invertebrates, `biomassToDL` is added to `DetritusProd[WC][DCdet_id]` (1996) and then again to `DetritusProdGlobal[WC][WC][DLdet_id]` (divided by propSediment) at 2042, so the discard waste nitrogen is created twice. Function: `Calculate_Catch()`.

**Current code** (`atecology/atprocess.c` lines 2101–2140 (truncated)) — code appears to have moved +107 lines since the review

```c
 2101: 		}
 2102: 		
 2103: 		boxLayerInfo->DetritusProd[WC][DCdet_id] += biomassToDL;   // This differs to the original code, which added it with each fishery but that was a bug
 2104: 
 2105: 		stock_id = bm->group_stock[guild][bm->current_box][bm->current_layer];
 2106: 
 2107: 		bm->calcTrackedMort[guild][cohort][stock_id][ongoingF_id] += (numDead * bm->dt);
 2108: 
 2109: 		for (nf = 0; nf < bm->K_num_fisheries; nf++) {
 2110: 			if (!FisheryArray[nf].isRec) {
 2111: 				bm->Catch[bm->current_box][guild][nf][bm->current_layer] += bm->FishingResults[nf][Catch_result_id];
 2112: 				bm->FCcaughttemp[guild][nf][cohort] += bm->FishingResults[nf][CatchAtAge_result_id];
 2113: 				if (FunctGroupArray[guild].isVertebrate == TRUE) {
 2114: 					FunctGroupArray[guild].SizeNumCaught[cohort][nf][bm->current_box] += bm->FishingResults[nf][CatchAtAge_result_id];
 2115: 					FunctGroupArray[guild].SizeNumDiscard[cohort][nf][bm->current_box] += bm->FishingResults[nf][DiscardsAtAge_result_id];
 2116: 				}
 2117: 			} else {
 2118: 				bm->RecCatch[bm->current_box][guild][nf] += bm->FishingResults[nf][Catch_result_id];
 2119: 			}
 2120: 
 2121: 			if (FunctGroupArray[guild].isVertebrate == TRUE) {
 2122: 				bm->FCdiscard[guild][cohort] += bm->FishingResults[nf][DiscardsAtAge_result_id];
 2123: 			} else {
 2124: 				FunctGroupArray[guild].dead[cohort] += (bm->FishingResults[nf][Catch_result_id] * propWater / (bm->cell_vol * bm->dt));
 2125: 				FunctGroupArray[guild].dead[cohort] += (bm->FishingResults[nf][Discards_result_id] * propWater / (bm->cell_vol * bm->dt));
 2126: 				numInvDead = (bm->FishingResults[nf][Catch_result_id] * propWater / (bm->cell_vol * bm->dt)) + (bm->FishingResults[nf][Discards_result_id] * propWater / (bm->cell_vol * bm->dt));
 2127: 				bm->calcTrackedMort[guild][cohort][stock_id][ongoingF_id] += (numInvDead * bm->dt);
 2128: 
 2129: 				//if(bm->which_check == guild)
 2130: 				//	fprintf(llogfp,"%s-%d dead: %e due to catch\n", FunctGroupArray[guild].groupCode, cohort, FunctGroupArray[guild].dead[cohort]);
 2131: 
 2132: 				if ((FunctGroupArray[guild].groupType == CORAL) || (FunctGroupArray[guild].groupType == SPONGE)) {
 2133: 					Destroy_Rugosity(bm, boxLayerInfo, llogfp, guild, cohort, nf, numInvDead);
 2134: 				}
 2135: 
 2136: 			}
 2137: 
 2138: 			/**/
 2139: 			if ((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)) {
 2140: 				if(bm->FishingResults[nf][Catch_result_id] > 0){
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
**Listed lines:** 2459-2465

**Function:** `Acidif_Mort()`

**Why it's thought to be a bug**

McElhany-Bush: `annualSurvivalNoAcid = exp(-pHmortB)` is annual, so `-log(1-annualSurvivalDecrement)` is an annual instantaneous rate. Dividing by 86400 alone (named `mortDueToAcidPerDay`) treats it as a daily rate, which makes acidification mortality about 365x too high. Function: `Acidif_Mort()`.

**Current code** (`atecology/atprocess.c` lines 2547–2557) — code appears to have moved +90 lines since the review

```c
 2547:     double sp_pHmortstart = FunctGroupArray[species].speciesParams[pHmortstart_id];
 2548:     double sp_pHmortA = FunctGroupArray[species].speciesParams[pHmortA_id];
 2549:     double sp_pHmortB = FunctGroupArray[species].speciesParams[pHmortB_id];
 2550:     double sp_pHmortmid = FunctGroupArray[species].speciesParams[pHmortmid_id];
 2551:     double annualSurvivalNoAcid, scalarOfSurvival, annualSurvivalScaled, annualSurvivalDecrement, mortDueToAcidPerDay;
 2552:     int pHmortcase = bm->flagPHmortcase;
 2553: 
 2554:     if (current_PH < sp_pHmortstart ) {
 2555:         switch (pHmortcase) {
 2556:             case ph_no_mort:
 2557:                 ans = 0.0;
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
**Listed lines:** 1535-1554

**Function:** `Primary_Production()`

**Why it's thought to be a bug**

`scale_uptake_NO = max(scale_uptake, NO/uptakeNO); scale_uptake = min(scale_uptake, scale_uptake_NO);`. The max guarantees scale_uptake_NO >= scale_uptake, so the min never changes scale_uptake. The NO, Si and Fe caps therefore never take effect, and uptake of those nutrients can exceed what is available. Only the NH cap is active. Function: `Primary_Production()`.

**Current code** (`atecology/atprocess.c` lines 1530–1553) — code appears to have moved -3 lines since the review

```c
 1530: 		double *spUptakeSi, double *spUptakeFe, double *spUptakeP,  double *spUptakeC, double *sphN) {
 1531: 
 1532: 	double hN_sp, hI_sp, sp_grow, scale_uptake, uptakeNH, uptakeNO, uptakeSi, uptakeFe, uptakeP = 0, uptakeC = 0;
 1533: 	double mL_sp = Ecology_Get_Linear_Mortality(bm, sp_id, 0, 0) + mL_other;
 1534: 	double mE_sp = FunctGroupArray[sp_id].cohortSpeciesParams[0][mE_id];
 1535:     double mS_sp = Acidif_Mort(bm, sp_id);
 1536: 	double cyst_transfer = 0.0;
 1537: 	double prop_daylight;
 1538: 	double pH_lim;
 1539: 	double P0 = FunctGroupArray[sp_id].speciesParams[P_min_internal_id];
 1540: 	double P1 = FunctGroupArray[sp_id].speciesParams[P_max_internal_id];
 1541: 	double mortality_scalar;
 1542:     double scale_uptake_NO = 1.0, scale_uptake_Si = 1.0, scale_uptake_Fe = 1.0, scale_uptake_NH = 1.0;
 1543:     
 1544:     /**
 1545: 	//if ((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)) {
 1546:     if((bm->dayt > 3633) && (FunctGroupArray[sp_id].groupType == TURF) && (bm->current_box == 24)) {
 1547: 		fprintf(
 1548: 			llogfp,
 1549: 			"Primary_Production parameters: sp_id = %d, micro_case = %d, lim_case = %d, macro_producer = %d, sp = %.20e, DIN = %.20e, NH = %.20e, NO = %.20e, Si = %.20e, Fe = %.20e, KN_sp = %.20e, KS_sp = %.20e, KF_sp = %.20e, KI_sp = %.20e, IRR = %.20e, OPT_IRR_sp = %.20e, mum_sp = %.20e, E_sp = %.20e, KLYS_sp = %.20e, mL_other = %.20e, SPmax = %.20e, area_hab = %.20e, mL_sp = %.20e\n",
 1550: 			sp_id, micro_case, lim_case, macro_producer, sp_biom, DIN, NH, NO, Si, Fe, FunctGroupArray[sp_id].speciesParams[KN_id],
 1551: 			FunctGroupArray[sp_id].speciesParams[KS_id], FunctGroupArray[sp_id].speciesParams[KF_id], FunctGroupArray[sp_id].speciesParams[KI_id], IRR,
 1552: 			FunctGroupArray[sp_id].SP_IRR, mum, E_sp, FunctGroupArray[sp_id].speciesParams[KLYS_id],
 1553: 			mL_other, SPmax, area_hab, mL_sp);
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
**Listed lines:** 1517-1520

**Function:** `Primary_Production()`

**Why it's thought to be a bug**

`uptakeNH = sp_grow*(NH/(KN+NH))*((KN+DIN)/DIN)`, and uptakeNO also divides by DIN. When DIN == 0, sp_grow is 0, so this is 0*inf = NaN. NaN fails both `>0` and `<0`, so it is stored in uptakeNH and NutsLost. Function: `Primary_Production()`.

**Current code** (`atecology/atprocess.c` lines 1530–1537) — code appears to have moved +15 lines since the review

```c
 1530: 		double *spUptakeSi, double *spUptakeFe, double *spUptakeP,  double *spUptakeC, double *sphN) {
 1531: 
 1532: 	double hN_sp, hI_sp, sp_grow, scale_uptake, uptakeNH, uptakeNO, uptakeSi, uptakeFe, uptakeP = 0, uptakeC = 0;
 1533: 	double mL_sp = Ecology_Get_Linear_Mortality(bm, sp_id, 0, 0) + mL_other;
 1534: 	double mE_sp = FunctGroupArray[sp_id].cohortSpeciesParams[0][mE_id];
 1535:     double mS_sp = Acidif_Mort(bm, sp_id);
 1536: 	double cyst_transfer = 0.0;
 1537: 	double prop_daylight;
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
**Listed lines:** 560-568

**Function:** `Get_Predator_Competition()`

**Why it's thought to be a bug**

Unreachable for now (Eat quits before calling it), but: the habitat coefficients use `FunctGroupArray[sp_id]` rather than `[predID]`; the else branch indexes `sm.tr[bm->current_layer]` with a water-column layer index (can exceed sediment nz); the epi biomass is indexed by `totNTracers` regardless of tracer type; and the loop includes non-predator groups. Function: `Get_Predator_Competition()`.

**Current code** (`atecology/atprocess.c` lines 591–603) — code appears to have moved +33 lines since the review

```c
  591:     double ans_tot_pred = 0.0;
  592:     double ans_tot_pred_comp = 0.0;
  593:     int predID, pid, predCohort;
  594:     
  595:     //TODO: FIX the dim_corrections
  596:     double epi_dim_correction = 1.0, sm_dim_correction = 1.0, wc_dim_correction = 1.0;
  597:     
  598:     if (!bm->need_predcomp_params ) { // Just need total predation but not competition
  599:         for (predID = 0; predID < bm->K_num_tot_sp; predID++) {
  600:             for (predCohort = 0; predCohort < FunctGroupArray[predID].numCohortsXnumGenes; predCohort++) {
  601:                 pid = FunctGroupArray[predID].totNTracers[predCohort];
  602:             
  603:                 if ( !bm->current_layer ) { // Need sediment based predators
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
**Listed lines:** 316-323, 1476

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
