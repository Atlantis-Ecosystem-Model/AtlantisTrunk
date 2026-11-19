# atecology/atprocess.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **18 findings** (severity H: 3; confidence H and severity H: 2; on the priority list: 2), plus 5 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PROC-01](#proc-01) ⭐ | 893, 420 | H/H | wrong index / buffer overrun | `bm->pSPFCeat[sp_id][preyID][cohort][kij]` (and `[predatorID][preyID][cohort][prey_chrt]` at 420). pSPFCeat is… |  | [ ] |
| [PROC-02](#proc-02) | 890, 893 | H/M | accumulator not reset | `bm->groupTotCatch[preyID][kij] += FCcaughttemp*pFLEET` and then `*= pSPFCeat`. groupTotCatch is only allocate… |  | [ ] |
| [PROC-03](#proc-03) ⭐ | 1177 | H/H | wrong variable / buffer overrun | `thisID = (int)speciesParams[coralID_id]; ... FunctGroupArray[thisID].cohortSpeciesParams[kij][rugosity_dec_id… |  | [ ] |
| [PROC-04](#proc-04) | 1098, 1265 | H/M | double division | For the std_holling/ratio/HV/CM cases, Eat sets `scalar = pHscalar / tot_prey; tprey = tot_prey;`. Do_Extra_Fe… |  | [ ] |
| [PROC-05](#proc-05) | 694, 1228 | H/M | wrong index | Get_Extra_Feed stores feed availability in `EATINGinfo/FEEDinfo[AquacultFeedIndex][cohort][WC]`, but Do_Extra_… |  | [ ] |
| [PROC-06](#proc-06) | 473, 496, 709 | H/M | variable overwritten in loop | `pHscalar` is set before the habitat loop (1/pHcorr for the predation-availability effect), but for plant prey… |  | [ ] |
| [PROC-07](#proc-07) | 1080 | H/M | division by zero / wrong loop bound | eat_minmax: `rel_scalar = EATINGinfo/(EATINGinfo + catch_addition)` is 0/0 = NaN for any prey/habitat with zer… |  | [ ] |
| [PROC-08](#proc-08) | 1754, 1722, 45 | H/M | double-applied scalar | `mortality[0]` already includes `mortality_scalar` (1585), but calcTrackedMort adds `mortality_scalar * mortal… |  | [ ] |
| [PROC-09](#proc-09) | 45 | H/L | leftover debug | `if((bm->dayt > 3633) && (groupType == TURF) && (current_box == 24)) fprintf(llogfp, ...)` is live code (the s… |  | [ ] |
| [PROC-10](#proc-10) | 1800 | H/M | logic / copy-paste | Wrap handling: when `encyst_TofY + encyst_period > 365`, it sets `encyst_TofY = 0; encyst_period = rollover`, … |  | [ ] |
| [PROC-11](#proc-11) | 1505 | M/M | truncation / missing state | `overStartTemp = (int)(speciesParams[overwinterStartTemp_id])` truncates fractional temperature thresholds (2.… |  | [ ] |
| [PROC-12](#proc-12) | 2299, 2272 | M/M | formula inconsistent | `BB_DL = BB*Bact_stim*XBB_DL*hO_BB/(Bact_stim*XBB_DL*DLsed*hO_BB + ...)`: the numerator is missing `DLsed` (sa… |  | [ ] |
| [PROC-13](#proc-13) | 2103, 2149 | M/M | double counting | For invertebrates, `biomassToDL` is added to `DetritusProd[WC][DCdet_id]` (1996) and then again to `DetritusPr… |  | [ ] |
| [PROC-14](#proc-14) | 2568 | M/H | unit mismatch | McElhany-Bush: `annualSurvivalNoAcid = exp(-pHmortB)` is annual, so `-log(1-annualSurvivalDecrement)` is an an… |  | [ ] |
| [PROC-15](#proc-15) | 45 | M/M | no-op logic | `scale_uptake_NO = max(scale_uptake, NO/uptakeNO); scale_uptake = min(scale_uptake, scale_uptake_NO);`. The ma… |  | [ ] |
| [PROC-16](#proc-16) | 1621, 45 | M/M | division by zero | `uptakeNH = sp_grow*(NH/(KN+NH))*((KN+DIN)/DIN)`, and uptakeNO also divides by DIN. When DIN == 0, sp_grow is … |  | [ ] |
| [PROC-17](#proc-17) | 610, 604 | H/L | wrong variable / index (latent) | Unreachable for now (Eat quits before calling it), but: the habitat coefficients use `FunctGroupArray[sp_id]` … |  | [ ] |
| [PROC-18](#proc-18) | 296, 320 | M/L | division by zero | Primary_Production (and atcoral/atmacrophytes) pass `PBmax_D = 0.0`. If bm->flaglight is irr_ice_v1, `step2 = … |  | [ ] |

---

## PROC-01

**wrong index / buffer overrun** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atprocess.c` 893, `atprocess.c` 420
**Lines at review (Oct 2026):** 420, 800, 1096

**Why it's thought to be a bug**

`bm->pSPFCeat[sp_id][preyID][cohort][kij]` (and `[predatorID][preyID][cohort][prey_chrt]` at 420). pSPFCeat is allocated `Util_Alloc_Init_4D_Double(K_num_max_stages, K_num_max_stages, K_num_tot_sp, K_num_tot_sp)` => `[sp][sp][stage][stage]` (atlantismain/atlantismain.c:2061), and it is indexed by stage in atvertprocesses.c:729/738 (`[chrtstage][preyage]`). Here it uses the cohort x genotype indices `cohort`/`kij`, which go past K_num_max_stages (normally 2), so the read is out of bounds and returns the wrong preference. Live whenever catcheater && flag_fisheries_on && !flag_olddiet. Functions: `Get_Catch_Prey()`, `Eat()`.

**Code at issue** — `atprocess.c` line 893  
Search for: `pSPFCeat[sp_id][preyID][cohort][kij]`

```c
  891: 				}
  892: 				if(!bm->flag_olddiet)
  893→ 					bm->groupTotCatch[preyID][kij] *= bm->pSPFCeat[sp_id][preyID][cohort][kij];
  894: 			}
  895: 
```

**Code at issue** — `atprocess.c` line 420  
Search for: `[predatorID][preyID][cohort][prey_chrt]`

```c
  418: 
  419: 		if(!bm->flag_olddiet)
  420→ 			step1 *= bm->pSPFCeat[predatorID][preyID][cohort][prey_chrt];
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
**Found at (current code):** `atprocess.c` 890, `atprocess.c` 893
**Lines at review (Oct 2026):** 795-801, 1094

**Function:** `Eat()`

**Why it's thought to be a bug**

`bm->groupTotCatch[preyID][kij] += FCcaughttemp*pFLEET` and then `*= pSPFCeat`. groupTotCatch is only allocated (atbiolsetup.c:2897) and freed. A repo-wide grep finds no reset, so it keeps growing across predators, cells and timesteps, and the `*=` scales the whole running total, not just this predator's share. The `ratio` denominator at 1094 is therefore far too big, so too little catch is taken out of FCcaughttemp/Catch. Function: `Eat()`.

**Code at issue** — `atprocess.c` line 890 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->groupTotCatch[preyID][kij] += bm->FCcaughttemp[preyID][fleet][kij] * bm->pFLEET[sp_id]`

```c
  888: 			if (catcheater && bm->flag_fisheries_on){
  889: 				for (fleet = 0; fleet < bm->K_num_fisheries; fleet++) {
  890→ 					bm->groupTotCatch[preyID][kij] += bm->FCcaughttemp[preyID][fleet][kij] * bm->pFLEET[sp_id][fleet];
  891: 				}
  892: 				if(!bm->flag_olddiet)
```

**Code at issue** — `atprocess.c` line 893  
Search for: `*= bm->pSPFCeat`

```c
  891: 				}
  892: 				if(!bm->flag_olddiet)
  893→ 					bm->groupTotCatch[preyID][kij] *= bm->pSPFCeat[sp_id][preyID][cohort][kij];
  894: 			}
  895: 
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
**Found at (current code):** `atprocess.c` 1177
**Lines at review (Oct 2026):** 1074-1076

**Function:** `Eat()`

**Why it's thought to be a bug**

`thisID = (int)speciesParams[coralID_id]; ... FunctGroupArray[thisID].cohortSpeciesParams[kij][rugosity_dec_id]`. coralID_id holds the CORALREEF index 0..ncoral-1 (atbiolsetup.c:4136), not a guild, so this reads the parameters of an unrelated group (e.g. guild 0). cohortSpeciesParams is also sized `[numStages]` (atUtilGroupIO.c:829), but here it is indexed by `kij` (cohort x genotype), which can overrun it. Live with flag_adv_habitat whenever coral or sponge is eaten. Function: `Eat()`.

**Code at issue** — `atprocess.c` line 1177 (closest match to the quoted code; wording differs slightly)  
Search for: `thisID = (int) (FunctGroupArray[preyID].speciesParams[coralID_id]);`

```c
 1175:                 		if (spGRAZEinfo[preyID][kij][habitat] > 0) {
 1176: 							if ((FunctGroupArray[preyID].groupType == CORAL) || (FunctGroupArray[preyID].groupType == SPONGE)) {
 1177→                                 thisID = (int) (FunctGroupArray[preyID].speciesParams[coralID_id]);
 1178:                                 
 1179:                                 CORALREEF[thisID].RugosityEaten[kij][bm->current_box] += FunctGroupArray[thisID].cohortSpeciesParams[kij][rugosity_dec_id] * spGRAZEinfo[preyID][kij][habitat] * FunctGroupArray[sp_id].speciesParams[rugFeedScalar_id];
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
**Found at (current code):** `atprocess.c` 1098, `atprocess.c` 1265
**Lines at review (Oct 2026):** 1005-1006, 1013-1014, 1162, 1165

**Why it's thought to be a bug**

For the std_holling/ratio/HV/CM cases, Eat sets `scalar = pHscalar / tot_prey; tprey = tot_prey;`. Do_Extra_Feed then computes `EATINGinfo*CLEAR*scalar/(tprey+small_num)`, so aquaculture feed consumption is divided by tot_prey twice (tot_prey_sq twice for holling3). `scalar` also carries the pH correction of whichever prey group was processed last, which has nothing to do with the feed. Functions: `Eat()`, `Do_Extra_Feed()`.

**Code at issue** — `atprocess.c` line 1098  
Search for: `scalar = pHscalar / tot_prey;`

```c
 1096: 					}
 1097: 					spCATCHGRAZEinfo[preyID][kij] = CATCHEATINGinfo[preyID][kij] * (double)(scaled_clear / tot_prey);
 1098→ 					scalar = pHscalar / tot_prey;
 1099:                     tprey = tot_prey;
 1100: 					break;
```

**Code at issue** — `atprocess.c` line 1265 (closest match to the quoted code; wording differs slightly)  
Search for: `spGRAZEinfo[AquacultFeedIndex][0][WC] = EATINGinfo[AquacultFeedIndex][0][WC] * CLEAR * sca`

```c
 1263:         case eat_hassel_varley:
 1264:         case eat_crowley_martin:
 1265→             spGRAZEinfo[AquacultFeedIndex][0][WC] = EATINGinfo[AquacultFeedIndex][0][WC] * CLEAR * scalar / (tprey + small_num);
 1266:             break;
 1267:         case eat_std_holling3:
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
**Found at (current code):** `atprocess.c` 694, `atprocess.c` 1228
**Lines at review (Oct 2026):** 646-652 vs 1143-1165; 1244

**Why it's thought to be a bug**

Get_Extra_Feed stores feed availability in `EATINGinfo/FEEDinfo[AquacultFeedIndex][cohort][WC]`, but Do_Extra_Feed always reads `[AquacultFeedIndex][0][WC]` and writes spGRAZEinfo `[...][0]`. Invert_Activities (1368) also reads `GRAZEinfo[AquacultFeedIndex][0][WC]`. For any cohort > 0, a cultured or supplemented group eats feed sized from cohort 0's (stale) availability. Functions: `Get_Extra_Feed()`, `Do_Extra_Feed()`.

**Code at issue** — `atprocess.c` line 694 (closest match to the quoted code; wording differs slightly)  
Search for: `FEEDinfo[AquacultFeedIndex][cohort][WC] += EATINGinfo[AquacultFeedIndex][cohort][WC] * Uti`

```c
  692:         
  693:         if(flagcase == eat_minmax){
  694→             FEEDinfo[AquacultFeedIndex][cohort][WC] += EATINGinfo[AquacultFeedIndex][cohort][WC] * Util_Mich_Ment(EATINGinfo[AquacultFeedIndex][cohort][WC], KL_sp);
  695:             *denom_step += FEEDinfo[AquacultFeedIndex][cohort][WC];
  696:         }
```

**Code at issue** — `atprocess.c` line 1228  
Search for: `[AquacultFeedIndex][0][WC]`

```c
 1226:         Do_Extra_Feed(bm, llogfp, sp_id, flagcase, (double)CLEAR, mum_sp, E1_sp, (double)scalar, tprey, spGRAZEinfo);
 1227:     else
 1228→         spGRAZEinfo[AquacultFeedIndex][0][WC] = 0.0;
 1229: 
 1230: //	if (((bm->debug == debug_biology_process) && (bm->which_check == sp_id) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop))) {
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
**Found at (current code):** `atprocess.c` 473, `atprocess.c` 496, `atprocess.c` 709
**Lines at review (Oct 2026):** 462-466, 483, 504-509

**Function:** `Calculate_PreyAvail()`

**Why it's thought to be a bug**

`pHscalar` is set before the habitat loop (1/pHcorr for the predation-availability effect), but for plant prey it is overwritten inside the loop (`pHscalar = pHcorr` or 1.0). From the 2nd habitat on, `prey_eat = prey_avail * pHscalar` uses the nutritional-value scalar instead of the predation scalar, and the 1/pHcorr effect is lost or inverted for SED/EPIFAUNA plant prey. Function: `Calculate_PreyAvail()`.

**Code at issue** — `atprocess.c` line 473 (closest match to the quoted code; wording differs slightly)  
Search for: `&& _finite((double) pHscalar) && _finite(FunctGroupArray[preyGuildID].pHcorr)`

```c
  471: 			&& !(bm->flag_macro_model && (FunctGroupArray[preyGuildID].groupType == SEAGRASS))
  472: 			&& !(catcheater && bm->flag_fisheries_on)
  473→ 			&& _finite((double) pHscalar) && _finite(FunctGroupArray[preyGuildID].pHcorr)
  474: 			&& (bm->pSPVERTeat[predatorGuildID][preyGuildID][chrtstage][FunctGroupArray[preyGuildID].cohort_stage[prey_chrt]] == 0.0)) {
  475: 		for (habitat = WC; habitat <= max_hab; habitat++) {
```

**Code at issue** — `atprocess.c` line 496  
Search for: `prey_eat =  prey_avail * pHscalar`

```c
  494: 
  495: 		/* determine biomass available */
  496→ 		prey_eat =  prey_avail * pHscalar;
  497: 
  498: 		/* PREYinfo holds each habitat in its own units: WC mg N m-3 water, SED mg N m-3 sediment,
```

**Code at issue** — `atprocess.c` line 709  
Search for: `Calculate_PreyAvail()`

```c
  707:  * removed directly from the prey: WC mg N m-3 water, SED mg N m-3 sediment, EPIFAUNA mg N m-2, ICE_BASED mg N m-3 ice.
  708:  * Water column predators grow per m-3 of the water cell they are in. Epibenthic predators eat per m-2 (prey used as
  709→  * concentrations in Calculate_PreyAvail()) and their growth uses m-2 / smLayerThick, which is set back to m-2 when
  710:  * their growth is multiplied by smLayerThick. Sediment and ice predators only eat in their own habitat so need no conversion.
  711:  */
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
**Found at (current code):** `atprocess.c` 1080
**Lines at review (Oct 2026):** 981-990

**Function:** `Eat()`

**Why it's thought to be a bug**

eat_minmax: `rel_scalar = EATINGinfo/(EATINGinfo + catch_addition)` is 0/0 = NaN for any prey/habitat with zero availability, which makes spGRAZEinfo NaN and triggers the `_finite` quit at 1083. The loop also runs `EPIFAUNA..0` regardless of `max_hab` (WC only for vertebrate prey, where EATINGinfo[SED/EPI] is never set this call), so stale values get used. Function: `Eat()`.

**Code at issue** — `atprocess.c` line 1080 (closest match to the quoted code; wording differs slightly)  
Search for: `rel_scalar = EATINGinfo[preyID][kij][habitat] / (EATINGinfo[preyID][kij][habitat] + catch_`

```c
 1078: 							catch_addition = 0.0;
 1079: 						}
 1080→ 						rel_scalar = EATINGinfo[preyID][kij][habitat] / (EATINGinfo[preyID][kij][habitat] + catch_addition);
 1081: 						spGRAZEinfo[preyID][kij][habitat] = EATINGinfo[preyID][kij][habitat] * (double)scaled_clear * mum_sp * FEEDinfo[preyID][kij][habitat] * (double)rel_scalar;
 1082: 					}
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
**Found at (current code):** `atprocess.c` 1754, `atprocess.c` 1722, `atprocess.c` 45
**Lines at review (Oct 2026):** 1585, 1618

**Function:** `Primary_Production()`

**Why it's thought to be a bug**

`mortality[0]` already includes `mortality_scalar` (1745), but calcTrackedMort adds `mortality_scalar * mortality[0] * Mdt`, which applies the scalar twice (including the HAB scalar). Invert_Activities line 1393 correctly omits it. Function: `Primary_Production()`.

**Code at issue** — `atprocess.c` line 1754  
Search for: `mortality[0]`

```c
 1752: 			"Time: %e Primary_Production outcomes: sp_id = %s, box%d-%d, growth = %.20e, uptakeNH = %.20e, uptakeNO = %.20e, uptakeSi = %.20e, uptakeFe = %.20e, mortality = %.20e, lysis = %.20e, maxPhagotrophy = %.20e, hN_sp = %.20e\n",
 1753: 			bm->dayt, FunctGroupArray[sp_id].groupCode, bm->current_box, bm->current_layer, FunctGroupArray[sp_id].growth[0], FunctGroupArray[sp_id].uptakeNH[0], *spUptakeNO, *spUptakeSi, *spUptakeFe,
 1754→ 			FunctGroupArray[sp_id].mortality[0], FunctGroupArray[sp_id].lysis[0], FunctGroupArray[sp_id].maxPhagotrophy,
 1755: 			*sphN);
 1756: 
```

**Code at issue** — `atprocess.c` line 1722 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->calcTrackedMort[sp_id][0][0][ongoingM1_id] += (mortality_scalar * (FunctGroupArray[sp_`

```c
 1720: 	//bm->calcMQuadMort[sp_id][current_id] += (mQ_sp * sp_biom * sp_biom);
 1721: 
 1722→ 	bm->calcTrackedMort[sp_id][0][0][ongoingM1_id] += (mortality_scalar * (FunctGroupArray[sp_id].mortality[0] * FunctGroupArray[sp_id].speciesParams[Mdt_id]));
 1723: 
 1724: 	if(bm->track_contaminants){
```

**Code at issue** — `atprocess.c` line 45  
Search for: `Primary_Production()`

```c
   43:  straight from parameter array not feed through routine call
   44:  (so can allow for separate juvenile and adult mortality in
   45→  vertebrates). Similarly modified Added Primary_Production()
   46: 
   47:  18/07/2008 Bec Gorton
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
**Found at (current code):** `atprocess.c` 45
**Lines at review (Oct 2026):** 1643-1657

**Function:** `Primary_Production()`

**Why it's thought to be a bug**

`if((bm->dayt > 3633) && (groupType == TURF) && (current_box == 24)) fprintf(llogfp, ...)` is live code (the surrounding `/** ... **/` closes at 1642). Any model with a TURF group and a box 24 writes two long lines per call after day 3633, which bloats the log. Function: `Primary_Production()`.

**Code at issue** — `atprocess.c` line 45  
Search for: `Primary_Production()`

```c
   43:  straight from parameter array not feed through routine call
   44:  (so can allow for separate juvenile and adult mortality in
   45→  vertebrates). Similarly modified Added Primary_Production()
   46: 
   47:  18/07/2008 Bec Gorton
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
**Found at (current code):** `atprocess.c` 1800
**Lines at review (Oct 2026):** 1685-1689, 1711-1716

**Function:** `Encystment()`

**Why it's thought to be a bug**

Wrap handling: when `encyst_TofY + encyst_period > 365`, it sets `encyst_TofY = 0; encyst_period = rollover`, which drops the days from encyst_TofY to 365 from the window entirely. The hatch block then reuses the already-modified `encyst_period`, so the hatch window length is wrong whenever the encyst window wrapped. Function: `Encystment()`.

**Code at issue** — `atprocess.c` line 1800 (closest match to the quoted code; wording differs slightly)  
Search for: `if((encyst_TofY <= bm->TofY) && (bm->TofY <= (encyst_TofY + encyst_period))) {`

```c
 1798: 		break;
 1799: 	case date_overwintering:
 1800→ 		if((encyst_TofY <= bm->TofY) && (bm->TofY <= (encyst_TofY + encyst_period))) {
 1801: 			this_encyst = encyst_rate * sp_biom;
 1802: 			cysts[sp_id][bm->current_box][cystenter_id] += this_encyst;
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
**Found at (current code):** `atprocess.c` 1505
**Lines at review (Oct 2026):** 1401-1407

**Function:** `Check_OverWintering()`

**Why it's thought to be a bug**

`overStartTemp = (int)(speciesParams[overwinterStartTemp_id])` truncates fractional temperature thresholds (2.5 becomes 2). `ans` is a local starting at 0, so when start < temp < end it returns 0: there is no hysteresis, even though the start/end pair implies one. Function: `Check_OverWintering()`.

**Code at issue** — `atprocess.c` line 1505 (closest match to the quoted code; wording differs slightly)  
Search for: `overStartTemp = (int)(FunctGroupArray[sp_id].speciesParams[overwinterStartTemp_id]);`

```c
 1503: 		break;
 1504: 	case enviro_overwintering:
 1505→ 		overStartTemp = (int)(FunctGroupArray[sp_id].speciesParams[overwinterStartTemp_id]);
 1506: 		overEndTemp = (int)(FunctGroupArray[sp_id].speciesParams[overwinterEndTemp_id]);
 1507: 
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
**Found at (current code):** `atprocess.c` 2299, `atprocess.c` 2272
**Lines at review (Oct 2026):** 2179-2193

**Function:** `Calculate_SedBact_Scale()`

**Why it's thought to be a bug**

`BB_DL = BB*Bact_stim*XBB_DL*hO_BB/(Bact_stim*XBB_DL*DLsed*hO_BB + ...)`: the numerator is missing `DLsed` (same for BB_DR/DRsed), and for habitatType==SED it hard-codes `DLsed = DRsed = 1.0`. Compare Calculate_PelagicBact_Scale (2165, `PB*XPB_DL*DL/(...)`) and Sediment_Bacterica_Process (atGroupProcesses.c:1827, numerator includes DL). Callers then divide by DLsed again (Invert_Activities 1238), so sediment bacteria grazing is mis-scaled. Function: `Calculate_SedBact_Scale()`.

**Code at issue** — `atprocess.c` line 2299  
Search for: `BB_DL = BB * Bact_stim * XBB_DL * hO_BB / (Bact_stim * XBB_DL * DLsed * hO_BB +`

```c
 2297: 
 2298: 		hO_BB = Oxygen(1, O2, 0.0, 0.0, FunctGroupArray[SedBactIndex].speciesParams[mD_id]);
 2299→ 		boxLayerInfo->BB_DL = BB * Bact_stim * XBB_DL * hO_BB / (Bact_stim * XBB_DL * DLsed * hO_BB + Bact_stim * XBB_DR * DRsed * hO_BB + small_num);
 2300: 		boxLayerInfo->BB_DR = BB * Bact_stim * XBB_DR * hO_BB / (Bact_stim * XBB_DL * DLsed * hO_BB + Bact_stim * XBB_DR * DRsed * hO_BB + small_num);
 2301: 	} else {
```

**Code at issue** — `atprocess.c` line 2272  
Search for: `PB * XPB_DL * DL / (`

```c
 2270: 		PB = boxLayerInfo->localWCTracers[FunctGroupArray[pelagicBactIndex].totNTracers[0]];
 2271: 
 2272→ 		boxLayerInfo->PB_DL = PB * XPB_DL * DL / (XPB_DL * DL + XPB_DR * DR + small_num);
 2273: 		boxLayerInfo->PB_DR = PB * XPB_DR * DR / (XPB_DL * DL + XPB_DR * DR + small_num);
 2274: 	} else {
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
**Found at (current code):** `atprocess.c` 2103, `atprocess.c` 2149
**Lines at review (Oct 2026):** 1996, 2041-2042

**Function:** `Calculate_Catch()`

**Why it's thought to be a bug**

For invertebrates, `biomassToDL` is added to `DetritusProd[WC][DCdet_id]` (2159) and then again to `DetritusProdGlobal[WC][WC][DLdet_id]` (divided by propSediment) at 2042, so the discard waste nitrogen is created twice. Function: `Calculate_Catch()`.

**Code at issue** — `atprocess.c` line 2103  
Search for: `DetritusProd[WC][DCdet_id]`

```c
 2101: 		}
 2102: 		
 2103→ 		boxLayerInfo->DetritusProd[WC][DCdet_id] += biomassToDL;   // This differs to the original code, which added it with each fishery but that was a bug
 2104: 
 2105: 		stock_id = bm->group_stock[guild][bm->current_box][bm->current_layer];
```

**Code at issue** — `atprocess.c` line 2149  
Search for: `DetritusProdGlobal[WC][WC][DLdet_id]`

```c
 2147: 	}
 2148: 	if (FunctGroupArray[guild].isVertebrate == FALSE) {
 2149→ 		boxLayerInfo->DetritusProdGlobal[WC][WC][DLdet_id] += biomassToDL / propSediment;
 2150: 
 2151: 		if (FunctGroupArray[guild].groupAgeType == BIOMASS)
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
**Found at (current code):** `atprocess.c` 2568
**Lines at review (Oct 2026):** 2459-2465

**Function:** `Acidif_Mort()`

**Why it's thought to be a bug**

McElhany-Bush: `annualSurvivalNoAcid = exp(-pHmortB)` is annual, so `-log(1-annualSurvivalDecrement)` is an annual instantaneous rate. Dividing by 86400 alone (named `mortDueToAcidPerDay`) treats it as a daily rate, which makes acidification mortality about 365x too high. Function: `Acidif_Mort()`.

**Code at issue** — `atprocess.c` line 2568 (closest match to the quoted code; wording differs slightly)  
Search for: `mortDueToAcidPerDay = -1.0 * (log(1.0 - annualSurvivalDecrement)) / 86400.0;  // convert t`

```c
 2566:                 annualSurvivalScaled = annualSurvivalNoAcid * scalarOfSurvival;     // get new annual survival rate under acidification
 2567:                 annualSurvivalDecrement =  (annualSurvivalNoAcid - annualSurvivalScaled);   // will be some small positive value, annual decline in survival.
 2568→                 mortDueToAcidPerDay = -1.0 * (log(1.0 - annualSurvivalDecrement)) / 86400.0;  // convert to per second rate. Note log is natural log.
 2569:                 ans = mortDueToAcidPerDay;
 2570:                 break;
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
**Found at (current code):** `atprocess.c` 45
**Lines at review (Oct 2026):** 1535-1554

**Function:** `Primary_Production()`

**Why it's thought to be a bug**

`scale_uptake_NO = max(scale_uptake, NO/uptakeNO); scale_uptake = min(scale_uptake, scale_uptake_NO);`. The max guarantees scale_uptake_NO >= scale_uptake, so the min never changes scale_uptake. The NO, Si and Fe caps therefore never take effect, and uptake of those nutrients can exceed what is available. Only the NH cap is active. Function: `Primary_Production()`.

**Code at issue** — `atprocess.c` line 45  
Search for: `Primary_Production()`

```c
   43:  straight from parameter array not feed through routine call
   44:  (so can allow for separate juvenile and adult mortality in
   45→  vertebrates). Similarly modified Added Primary_Production()
   46: 
   47:  18/07/2008 Bec Gorton
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
**Found at (current code):** `atprocess.c` 1621, `atprocess.c` 45
**Lines at review (Oct 2026):** 1517-1520

**Function:** `Primary_Production()`

**Why it's thought to be a bug**

`uptakeNH = sp_grow*(NH/(KN+NH))*((KN+DIN)/DIN)`, and uptakeNO also divides by DIN. When DIN == 0, sp_grow is 0, so this is 0*inf = NaN. NaN fails both `>0` and `<0`, so it is stored in uptakeNH and NutsLost. Function: `Primary_Production()`.

**Code at issue** — `atprocess.c` line 1621 (closest match to the quoted code; wording differs slightly)  
Search for: `uptakeNH = sp_grow * (NH / (FunctGroupArray[sp_id].speciesParams[KN_id] + NH)) * ((FunctGr`

```c
 1619: 
 1620: 	/* Calculate nutrient uptake */
 1621→ 	uptakeNH = sp_grow * (NH / (FunctGroupArray[sp_id].speciesParams[KN_id] + NH)) * ((FunctGroupArray[sp_id].speciesParams[KN_id] + DIN) / DIN);
 1622: 	uptakeNO = sp_grow * (NO / (FunctGroupArray[sp_id].speciesParams[KN_id] + DIN))
 1623: 			* (FunctGroupArray[sp_id].speciesParams[KN_id]	/ (FunctGroupArray[sp_id].speciesParams[KN_id] + NH))
```

**Code at issue** — `atprocess.c` line 45  
Search for: `Primary_Production()`

```c
   43:  straight from parameter array not feed through routine call
   44:  (so can allow for separate juvenile and adult mortality in
   45→  vertebrates). Similarly modified Added Primary_Production()
   46: 
   47:  18/07/2008 Bec Gorton
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
**Found at (current code):** `atprocess.c` 610, `atprocess.c` 604
**Lines at review (Oct 2026):** 560-568

**Function:** `Get_Predator_Competition()`

**Why it's thought to be a bug**

Unreachable for now (Eat quits before calling it), but: the habitat coefficients use `FunctGroupArray[sp_id]` rather than `[predID]`; the else branch indexes `sm.tr[bm->current_layer]` with a water-column layer index (can exceed sediment nz); the epi biomass is indexed by `totNTracers` regardless of tracer type; and the loop includes non-predator groups. Function: `Get_Predator_Competition()`.

**Code at issue** — `atprocess.c` line 610  
Search for: `FunctGroupArray[sp_id]`

```c
  608:                 } else {
  609:                     ans_tot_pred += bm->boxes[bm->current_box].sm.tr[bm->current_layer][pid] * FunctGroupArray[sp_id].habitatCoeffs[SED] * sm_dim_correction;
  610→                     ans_tot_pred += bm->boxes[bm->current_box].tr[bm->current_layer][pid] * FunctGroupArray[sp_id].habitatCoeffs[WC] * wc_dim_correction;
  611:                 }
  612:             }
```

**Code at issue** — `atprocess.c` line 604  
Search for: `[predID]`

```c
  602:             
  603:                 if ( !bm->current_layer ) { // Need sediment based predators
  604→                     if ( FunctGroupArray[predID].isEpiFauna )
  605:                         ans_tot_pred += bm->boxes[bm->current_box].epi[pid] * FunctGroupArray[sp_id].habitatCoeffs[EPIFAUNA] * epi_dim_correction;
  606:                     ans_tot_pred += bm->boxes[bm->current_box].sm.tr[bm->current_layer][pid] * FunctGroupArray[sp_id].habitatCoeffs[SED] * sm_dim_correction;
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
**Found at (current code):** `atprocess.c` 296, `atprocess.c` 320
**Lines at review (Oct 2026):** 316-323, 1476

**Function:** `Light_Lim()`

**Why it's thought to be a bug**

Primary_Production (and atcoral/atmacrophytes) pass `PBmax_D = 0.0`. If bm->flaglight is irr_ice_v1, `step2 = 0`, so `step1*amt_irr/step2` is a division by zero (and irr_ice_v2 gives `tanh(x/0)`). Function: `Light_Lim()`.

**Code at issue** — `atprocess.c` line 296  
Search for: `step2 = 0`

```c
  294: 	double hI = 0;
  295: 	double step1 = 0.0;
  296→ 	double step2 = 0.0;
  297: 
  298: 
```

**Code at issue** — `atprocess.c` line 320  
Search for: `step1 * amt_irr / step2`

```c
  318: 
  319: 		// Light limitation with photoadaptation
  320→ 		hI = (1.0 - exp(step1 * amt_irr / step2)) * exp(-1.0 * opt_irr * amt_irr / step2);
  321: 		break;
  322:     case irr_ice_v2:
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
