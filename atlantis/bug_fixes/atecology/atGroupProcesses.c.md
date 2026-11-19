# atecology/atGroupProcesses.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **10 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 4 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PROC-19](#proc-19) | 943, 1096, 1216 | H/M | wrong semantics | `spawn_now = (int)EMBRYO[guild].Spawn_Now[cohort][next_spawn]` and `if ((int)(Spawn_Now[...] == TRUE))`. Spawn… |  | [ ] |
| [PROC-20](#proc-20) | 832, 838 | H/M | copy-paste / wrong habitat | Algal epibenthic branch: detritus goes to `DetritusProd[WC][DLdet_id]` (832), but `Group_Transfer_Contaminant(… |  | [ ] |
| [PROC-21](#proc-21) | 1437, 1526, 1562 | H/M | double counting | With track_atomic_ratio, `Gain_Element(... LabDetIndex ..., lysis[0] ...)` is called at 1437 using the Primary… |  | [ ] |
| [PROC-22](#proc-22) | 1533-1550 | M/M | wrong habitat index | Dinoflag_Process handles `case WC: case ICE_BASED:` together, but all flux updates are hard-coded to `[WC]` (N… |  | [ ] |
| [PROC-23](#proc-23) | 604 vs 616/620; 1550 vs 1562/1571 | M/M | inconsistent fluxes | `DetritusProd[..][DLdet_id] += lysis + mortality[0]`, but the element gain (Gain_Element), and the DebugInfo D… |  | [ ] |
| [PROC-24](#proc-24) | 714 | M/M | unit mismatch | `cysts[...][cystenter_id] = (1.0 - (mS_sp + mL_sp + mE_sp)) * cysts[...]`. The mortality rates are per second … |  | [ ] |
| [PROC-25](#proc-25) | 299-303, 338 | M/M | inverted condition / double count | In the EPIFAUNA case, for SED_EP groups: `if (diagTol==2 && it_count==1) NutsProd[SED] += releaseNH; else Nuts… |  | [ ] |
| [PROC-26](#proc-26) | 2157-2166 | M/M | inconsistent reset | After temporarily overwriting phytoplankton PREYinfo with sediment availability, the reset sets `PREYinfo[sp][… |  | [ ] |
| [PROC-27](#proc-27) | 1679-1680, 1811-1812 | M/M | division by zero | With flagkdrop: `pow(PB_DL/(XPB_DL*DL), 3.0)` and `pow(BB_DL/(XBB_DL*DL), 3.0)` have no small_num. When DL (or… |  | [ ] |
| [PROC-28](#proc-28) | 317-322, 346-352 | M/L | ungated debug | `if((bm->dayt > bm->checkstart) && (bm->checkbox == bm->current_box)) fprintf(bm->logFile, ...)` is not gated … |  | [ ] |

---

## PROC-19

**wrong semantics** · Conf H / Sev M  
**Listed lines:** 943, 1096, 1216

**Why it's thought to be a bug**

`spawn_now = (int)EMBRYO[guild].Spawn_Now[cohort][next_spawn]` and `if ((int)(Spawn_Now[...] == TRUE))`. Spawn_Now stores spawn day-of-year (atannualbiology.c:248, 386-390), and atvertprocesses.c:1232 and atdemography.c:894 compare it with the day. So spawn_now is non-zero almost always, and `mat_pcnt` is only set when the spawn day equals 1. The feed-while-spawning reduction in Eat is effectively never applied correctly for AGE_STRUCTURED_BIOMASS inverts. Functions: `Invert_Consumers_Process()`, `Coral_Process()`.

**Current code** (`atecology/atGroupProcesses.c` lines 947–951) — code appears to have moved +6 lines since the review

```c
  947: 		spawn_now = 0;
  948: 	else
  949: 		spawn_now = (int) (EMBRYO[guild].Spawn_Now[cohort][EMBRYO[guild].next_spawn]);
  950: 
  951: 	switch(habitatType){
```

**Suggested fix (from review):** Compute spawning-now as in atvertprocesses.c (compare spawn_date and spawn_period with the current day)

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

## PROC-20

**copy-paste / wrong habitat** · Conf H / Sev M  
**Listed lines:** 832, 838

**Function:** `Phytoplankton_Process()`

**Why it's thought to be a bug**

Algal epibenthic branch: detritus goes to `DetritusProd[WC][DLdet_id]` (832), but `Group_Transfer_Contaminant(bm, boxLayerInfo, EPIFAUNA, SED, LabDetIndex, ...)` moves the contaminant into the SED habitat (the 4th arg is the habitat, atContaminants.c:940). This was copied from the seagrass branch (784), which does use SED detritus. Compare Update_Detritus 382-383 (`EPIFAUNA, WC`). Function: `Phytoplankton_Process()`.

**Current code** (`atecology/atGroupProcesses.c` lines 836–846) — code appears to have moved +6 lines since the review

```c
  836: 				}
  837: 
  838: 				boxLayerInfo->DetritusProd[WC][DLdet_id] += FunctGroupArray[guild].mortality[cohort] / wcLayerThick;
  839: 
  840: 				/* Contaminants transfer into detritus */
  841: 				if(bm->track_contaminants){
  842:                     //fprintf(bm->logFile,"Calling Group_Transfer_Contaminant from Update_Detritus EPIFAUNA SED - %s-%d with initialBiomass: %e mort: %e\n", FunctGroupArray[guild].groupCode, cohort, initialBiomass, FunctGroupArray[guild].mortality[cohort] / wcLayerThick);
  843:                     
  844:                     Group_Transfer_Contaminant(bm, boxLayerInfo, EPIFAUNA, SED, LabDetIndex, 0, guild, cohort,  FunctGroupArray[guild].mortality[cohort] / wcLayerThick, 0, initialBiomass, bm->dtsz_stored, 1, 25);   // Was case 0 for need_prop
  845: 				}
  846: 
```

**Suggested fix (from review):** Change to `EPIFAUNA, WC`

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

## PROC-21

**double counting** · Conf H / Sev M  
**Listed lines:** 1437, 1526, 1562

**Function:** `Dinoflag_Process()`

**Why it's thought to be a bug**

With track_atomic_ratio, `Gain_Element(... LabDetIndex ..., lysis[0] ...)` is called at 1437 using the Primary_Production lysis. Lysis is then recomputed at 1526, and Gain_Element for DL is called again at 1562. DL elements (P/C) are gained twice per step. Function: `Dinoflag_Process()`.

**Current code** (`atecology/atGroupProcesses.c` lines 1446–1450) — code appears to have moved +11 lines since the review

```c
 1446: 
 1447: 			if(bm->track_atomic_ratio == TRUE){
 1448: 				/* Change in Biomass in phyto due to lysis handled in final flux calc so just need to account for gain in DL here */
 1449: 				Gain_Element(bm, boxLayerInfo, habitatType, LabDetIndex, 0, guild, cohort, FunctGroupArray[guild].lysis[0], WC, isGlobal);
 1450: 
```

**Suggested fix (from review):** Remove the call at 1437

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

## PROC-22

**wrong habitat index** · Conf M / Sev M  
**Listed lines:** 1533-1550

**Function:** `Dinoflag_Process()`

**Why it's thought to be a bug**

Dinoflag_Process handles `case WC: case ICE_BASED:` together, but all flux updates are hard-coded to `[WC]` (NutsLost[WC], DetritusLost[WC], DetritusProd[WC], `GRAZEinfo[RefDetIndex][0][WC]`). For ice dinoflagellates the uptake and detritus fluxes go to the water column, and the ice detritus grazing is not removed. Phytoplankton_Process (865-870) correctly uses `habitatType`. Function: `Dinoflag_Process()`.

**Current code** (`atecology/atGroupProcesses.c` lines 1558–1579) — code appears to have moved +27 lines since the review

```c
 1558: 			boxLayerInfo->NutsLost[WC][Fe_id] += uptakeFe;
 1559: 
 1560: 			boxLayerInfo->DetritusLost[WC][DRdet_id] += GRAZEinfo[RefDetIndex][0][WC];
 1561: 			boxLayerInfo->DetritusLost[WC][DLdet_id] += GRAZEinfo[LabDetIndex][0][WC];
 1562: 
 1563: 			boxLayerInfo->DetritusProd[WC][DLdet_id] += FunctGroupArray[guild].lysis[cohort] + FunctGroupArray[guild].mortality[0];
 1564: 
 1565: 			/* Contaminants transfer */
 1566: 			if(bm->track_contaminants){
 1567: 				/* Gains in DL due to mortality */
 1568:                 //fprintf(bm->logFile,"Calling Group_Transfer_Contaminant from Update_Detritus WC WC - %s-%d with initialBiomass: %e lysis: %e\n", FunctGroupArray[guild].groupCode, cohort, initialBiomass, FunctGroupArray[guild].lysis[cohort]);
 1569:                 
 1570:                 Group_Transfer_Contaminant(bm, boxLayerInfo, WC, WC, LabDetIndex, 0, guild, cohort, (FunctGroupArray[guild].lysis[cohort] + FunctGroupArray[guild].mortality[0]), 0, initialBiomass, bm->dtsz_stored, 1, 27);   // Was case 0 for need_prop
 1571: 			}
 1572: 
 1573: 			if(bm->track_atomic_ratio == TRUE){
 1574: 				/* Change in bysisiomass due to lysys handled in final flux calc so just need to account for gain in DL here */
 1575: 				Gain_Element(bm, boxLayerInfo, WC, LabDetIndex, 0, guild, cohort,FunctGroupArray[guild].lysis[cohort], WC, isGlobal);
 1576: 				boxLayerInfo->NutsLost[WC][P_id] += uptakeP;
 1577: 				boxLayerInfo->NutsLost[WC][C_id] += uptakeC;
 1578: 			}
 1579: 
```

**Suggested fix (from review):** Use `habitatType` in place of `WC`

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

## PROC-23

**inconsistent fluxes** · Conf M / Sev M  
**Listed lines:** 604 vs 616/620; 1550 vs 1562/1571

**Why it's thought to be a bug**

`DetritusProd[..][DLdet_id] += lysis + mortality[0]`, but the element gain (Gain_Element), and the DebugInfo DL, only use `lysis`. With flag_replicated_old_PPmort off, mortality[0] is non-zero, so the P/C ratios of DL drift (N added without the matching elements). Functions: `Phytoplankton_Process()`, `Dinoflag_Process()`.

**Current code** (`atecology/atGroupProcesses.c` lines 608–612) — code appears to have moved +6 lines since the review

```c
  608: 			boxLayerInfo->NutsLost[habitatType][C_id] += uptakeC;
  609: 
  610: 			boxLayerInfo->DetritusProd[habitatType][DLdet_id] += FunctGroupArray[guild].lysis[cohort] + FunctGroupArray[guild].mortality[0];
  611: 
  612: 			/* Contaminants transfer into detritus */
```

**Suggested fix (from review):** Include mortality[0] in Gain_Element and DebugInfo

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

## PROC-24

**unit mismatch** · Conf M / Sev M  
**Listed lines:** 714

**Function:** `Phytoplankton_Process()`

**Why it's thought to be a bug**

`cysts[...][cystenter_id] = (1.0 - (mS_sp + mL_sp + mE_sp)) * cysts[...]`. The mortality rates are per second (used as `rate * biomass` fluxes elsewhere), so this kills a negligible per-second fraction per call, with no dt, independent of timestep. Function: `Phytoplankton_Process()`.

**Current code** (`atecology/atGroupProcesses.c` lines 717–721) — code appears to have moved +5 lines since the review

```c
  717: 					boxLayerInfo->DebugInfo[guild][SED][DiagnostDLsed_id] = FunctGroupArray[guild].mortality[cohort];
  718: 
  719: 					/* Kill off cysts too -TODO should we actually used the SED pool as the cyst storage facility rather than a cysts array? */
  720: 					cysts[guild][bm->current_box][cystenter_id] = (1.0 - (mS_sp + mL_sp + mE_sp)) * cysts[guild][bm->current_box][cystenter_id];
  721: 				}
```

**Suggested fix (from review):** Use `(1.0 - (mS+mL+mE)*bm->dt)` (or exp(-rate*dt))

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

## PROC-25

**inverted condition / double count** · Conf M / Sev M  
**Listed lines:** 299-303, 338

**Function:** `Update_Detritus()`

**Why it's thought to be a bug**

In the EPIFAUNA case, for SED_EP groups: `if (diagTol==2 && it_count==1) NutsProd[SED] += releaseNH; else NutsProdGlobal[EPIFAUNA][SED] += releaseNH;`. Every other case updates the local flux always and the global flux only when diagTol==2 && it_count==1. Here the non-global groups never get a local NH flux, and in the global case NutsProdGlobal[EPIFAUNA][SED] is incremented again at 338. Function: `Update_Detritus()`.

**Current code** (`atecology/atGroupProcesses.c` lines 297–336 (truncated))

```c
  297: 			boxLayerInfo->DetritusProd[SED][DRdet_id] += FunctGroupArray[guild].prodnDR[cohort];
  298: 
  299: 
  300: 			if(bm->track_atomic_ratio == TRUE){
  301: 				Gain_Element(bm, boxLayerInfo, SED, RefDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDR[cohort], EPIFAUNA, isGlobal);
  302: 				Gain_Element(bm, boxLayerInfo, SED, LabDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDL[cohort], EPIFAUNA, isGlobal);
  303: 			}
  304: 
  305: 			if (FunctGroupArray[guild].diagTol == 2 && it_count == 1){
  306: 				boxLayerInfo->NutsProd[SED][NH_id] += FunctGroupArray[guild].releaseNH[cohort];
  307: 			} else {
  308: 				boxLayerInfo->NutsProdGlobal[EPIFAUNA][SED][NH_id] += FunctGroupArray[guild].releaseNH[cohort]; // Was boxLayerInfo->NutsProd[WC][NH_id]
  309: 			}
  310: 
  311: 		} else {
  312: 			boxLayerInfo->DetritusProd[WC][DLdet_id] += FunctGroupArray[guild].prodnDL[cohort];
  313: 			boxLayerInfo->DetritusProd[WC][DRdet_id] += FunctGroupArray[guild].prodnDR[cohort];
  314: 			boxLayerInfo->NutsProd[WC][NH_id] += FunctGroupArray[guild].releaseNH[cohort];
  315: 
  316: 
  317: 			if(bm->track_atomic_ratio == TRUE){
  318: 				Gain_Element(bm, boxLayerInfo, WC, RefDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDR[cohort], EPIFAUNA, isGlobal);
  319: 				Gain_Element(bm, boxLayerInfo, WC, LabDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDL[cohort], EPIFAUNA, isGlobal);
  320: 			}
  321: 		}
  322:             
  323:         if((bm->dayt > bm->checkstart) && (bm->checkbox == bm->current_box)) {
  324:             fprintf(bm->logFile, "Time: %e box%d-%d %s epiDLprod: %Lf (%Lf) DLlost: %Lf (%Lf) graze %e (%e) prodnDet: %e\n",
  325:                 bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, boxLayerInfo->DetritusProd[WC][DLdet_id], boxLayerInfo->DetritusProd[SED][DLdet_id],
  326:                 boxLayerInfo->DetritusLost[WC][DLdet_id], boxLayerInfo->DetritusLost[SED][DLdet_id],
  327:                 GRAZEinfo[LabDetIndex][0][WC], GRAZEinfo[LabDetIndex][0][SED], FunctGroupArray[guild].prodnDL[cohort]);
  328:         }
  329: 
  330: 		/* Update global fluxes */
  331: 		if (FunctGroupArray[guild].diagTol == 2 && it_count == 1) {
  332: 			FunctGroupArray[pelagicBactIndex].preyEatenGlobal[0][EPIFAUNA][WC] += GRAZEinfo[pelagicBactIndex][0][WC];
  333: 			FunctGroupArray[SedBactIndex].preyEatenGlobal[0][EPIFAUNA][SED] += GRAZEinfo[SedBactIndex][0][SED];
  334: 			boxLayerInfo->DetritusLostGlobal[EPIFAUNA][WC][DRdet_id] += GRAZEinfo[RefDetIndex][0][WC];
  335: 			boxLayerInfo->DetritusLostGlobal[EPIFAUNA][WC][DLdet_id] += GRAZEinfo[LabDetIndex][0][WC];
  336: 			boxLayerInfo->DetritusLostGlobal[EPIFAUNA][SED][DRdet_id] += GRAZEinfo[RefDetIndex][0][SED];
```

**Suggested fix (from review):** Always update NutsProd[SED] and leave the global flux to the block at 325-344

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

## PROC-26

**inconsistent reset** · Conf M / Sev M  
**Listed lines:** 2157-2166

**Function:** `Sediment_Epi_Other_Process()`

**Why it's thought to be a bug**

After temporarily overwriting phytoplankton PREYinfo with sediment availability, the reset sets `PREYinfo[sp][chrt][WC] = localWCTracers[...]` without the `habitatCoeffs[WC]` (and ice MIXED) scaling used by Construct_Prey_Info (atprocess.c:2404, 2423), and leaves `PREYinfo[...][SED] = 0`. Predators processed later in the same cell see different phytoplankton availability. Function: `Sediment_Epi_Other_Process()`.

**Current code** (`atecology/atGroupProcesses.c` lines 2169–2182) — code appears to have moved +14 lines since the review

```c
 2169: 		if (bm->debug == debug_prey_biology_process && bm->dayt >= bm->checkstart && bm->dayt < bm->checkstop) {
 2170: 			Print_Eat_Diagnostics(bm, logfp, guild, habitatType, 1);
 2171: 			fprintf(llogfp, "Sediment_Epi_Other_Process outcome %s DetritusProd[SED][DLdet_id = %.20Le, NutsProd[SED][NH_id] = %.20Le\n",
 2172: 					FunctGroupArray[guild].groupCode, boxLayerInfo->DetritusProd[SED][DLdet_id], boxLayerInfo->NutsProd[SED][NH_id]);
 2173: 		}
 2174: 	}
 2175: 
 2176: 	/* reset planktonic groups to the values set in Epibenthic_Box() for the other predators -
 2177: 	 * water column value scaled by habitat coefficient, phytoplankton are not put in the sediment slot there */
 2178: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 2179: 		if(FunctGroupArray[sp].speciesParams[flag_id] == TRUE){
 2180: 			if (FunctGroupArray[sp].groupType == LG_PHY || FunctGroupArray[sp].groupType == DINOFLAG || FunctGroupArray[sp].groupType == SM_PHY) {
 2181: 				for (prey_chrt = 0; prey_chrt < FunctGroupArray[sp].numCohortsXnumGenes; prey_chrt++) {
 2182: 					PREYinfo[sp][prey_chrt][WC] = boxLayerInfo->localWCTracers[FunctGroupArray[sp].totNTracers[prey_chrt]] * FunctGroupArray[sp].habitatCoeffs[WC];
```

**Suggested fix (from review):** Save and restore the original values (or reapply the habitatCoeffs)

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

## PROC-27

**division by zero** · Conf M / Sev M  
**Listed lines:** 1679-1680, 1811-1812

**Why it's thought to be a bug**

With flagkdrop: `pow(PB_DL/(XPB_DL*DL), 3.0)` and `pow(BB_DL/(XBB_DL*DL), 3.0)` have no small_num. When DL (or DR) is 0, the numerator is also 0, giving 0/0 = NaN growth (`max(0.0, NaN)` is unreliable). Functions: `Pelagic_Bacteria_Process()`, `Sediment_Bacterica_Process()`.

**Current code** (`atecology/atGroupProcesses.c` lines 1680–1685) — code appears to have moved +3 lines since the review

```c
 1680: 			fprintf(
 1681: 					llogfp,
 1682: 					"Pelagic_Bacteria_Process parameters: mL_sp = %e, FunctGroupArray[guild].speciesParams[mO_id] = %e, FunctGroupArray[guild].speciesParams[E3_id] = %e, FunctGroupArray[guild].speciesParams[FDMort_id] = %e\n",
 1683: 					mL_sp, FunctGroupArray[guild].speciesParams[mO_id], FunctGroupArray[guild].speciesParams[E3_id],
 1684: 					FunctGroupArray[guild].speciesParams[FDMort_id]);
 1685: 		}
```

**Suggested fix (from review):** Add `+ small_num` to the denominators

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

## PROC-28

**ungated debug** · Conf M / Sev L  
**Listed lines:** 317-322, 346-352

**Function:** `Update_Detritus()`

**Why it's thought to be a bug**

`if((bm->dayt > bm->checkstart) && (bm->checkbox == bm->current_box)) fprintf(bm->logFile, ...)` is not gated on bm->debug or which_check. It prints two lines for every epibenthic group, every iteration, in the check box once past checkstart, which can flood the log. Function: `Update_Detritus()`.

**Current code** (`atecology/atGroupProcesses.c` lines 321–360) — code appears to have moved +6 lines since the review

```c
  321: 		}
  322:             
  323:         if((bm->dayt > bm->checkstart) && (bm->checkbox == bm->current_box)) {
  324:             fprintf(bm->logFile, "Time: %e box%d-%d %s epiDLprod: %Lf (%Lf) DLlost: %Lf (%Lf) graze %e (%e) prodnDet: %e\n",
  325:                 bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, boxLayerInfo->DetritusProd[WC][DLdet_id], boxLayerInfo->DetritusProd[SED][DLdet_id],
  326:                 boxLayerInfo->DetritusLost[WC][DLdet_id], boxLayerInfo->DetritusLost[SED][DLdet_id],
  327:                 GRAZEinfo[LabDetIndex][0][WC], GRAZEinfo[LabDetIndex][0][SED], FunctGroupArray[guild].prodnDL[cohort]);
  328:         }
  329: 
  330: 		/* Update global fluxes */
  331: 		if (FunctGroupArray[guild].diagTol == 2 && it_count == 1) {
  332: 			FunctGroupArray[pelagicBactIndex].preyEatenGlobal[0][EPIFAUNA][WC] += GRAZEinfo[pelagicBactIndex][0][WC];
  333: 			FunctGroupArray[SedBactIndex].preyEatenGlobal[0][EPIFAUNA][SED] += GRAZEinfo[SedBactIndex][0][SED];
  334: 			boxLayerInfo->DetritusLostGlobal[EPIFAUNA][WC][DRdet_id] += GRAZEinfo[RefDetIndex][0][WC];
  335: 			boxLayerInfo->DetritusLostGlobal[EPIFAUNA][WC][DLdet_id] += GRAZEinfo[LabDetIndex][0][WC];
  336: 			boxLayerInfo->DetritusLostGlobal[EPIFAUNA][SED][DRdet_id] += GRAZEinfo[RefDetIndex][0][SED];
  337: 			boxLayerInfo->DetritusLostGlobal[EPIFAUNA][SED][DLdet_id] += GRAZEinfo[LabDetIndex][0][SED];
  338: 
  339: 
  340: 			if (FunctGroupArray[processGuild].groupType == SED_EP_OTHER || FunctGroupArray[processGuild].groupType == SED_EP_FF
  341: 					|| FunctGroupArray[processGuild].groupType == MOB_EP_OTHER) {
  342: 				boxLayerInfo->DetritusProdGlobal[EPIFAUNA][SED][DLdet_id] += FunctGroupArray[guild].prodnDL[cohort];
  343: 				boxLayerInfo->DetritusProdGlobal[EPIFAUNA][SED][DRdet_id] += FunctGroupArray[guild].prodnDR[cohort];
  344: 				boxLayerInfo->NutsProdGlobal[EPIFAUNA][SED][NH_id] += FunctGroupArray[guild].releaseNH[cohort];
  345: 
  346: 			} else {
  347: 				boxLayerInfo->DetritusProdGlobal[EPIFAUNA][WC][DLdet_id] += FunctGroupArray[guild].prodnDL[cohort];
  348: 				boxLayerInfo->DetritusProdGlobal[EPIFAUNA][WC][DRdet_id] += FunctGroupArray[guild].prodnDR[cohort];
  349: 				boxLayerInfo->NutsProdGlobal[EPIFAUNA][WC][NH_id] += FunctGroupArray[guild].releaseNH[cohort];
  350: 			}
  351:             
  352:             if((bm->dayt > bm->checkstart) && (bm->checkbox == bm->current_box)) {
  353:                 fprintf(bm->logFile, "Time: %e box%d-%d %s epiDLprodGLOBAL: %Lf (%Lf) DLlostGLOBAL: %Lf (%Lf) grazeGLOBAL %e (%e) prodnGLOBAL: %e\n",
  354:                     bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode,
  355:                     boxLayerInfo->DetritusProdGlobal[EPIFAUNA][WC][DLdet_id], boxLayerInfo->DetritusProdGlobal[EPIFAUNA][SED][DLdet_id],
  356:                     boxLayerInfo->DetritusLostGlobal[EPIFAUNA][WC][DLdet_id], boxLayerInfo->DetritusLostGlobal[EPIFAUNA][SED][DLdet_id],
  357:                     GRAZEinfo[LabDetIndex][0][WC], GRAZEinfo[LabDetIndex][0][SED], FunctGroupArray[guild].prodnDL[cohort]);
  358:             }
  359:             
  360: 		}
```

**Suggested fix (from review):** Gate on `bm->debug == debug_biology_process` / which_check, or comment out

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

atGroupProcesses.c `Sediment_Bacterica_Process()` 1850-1857 passes `WC` as the globalHabitat argument to Gain_Element/Loose_Element for sediment bacteria, while Calculate_Element_Release uses `SED`. It looks copied from the pelagic version and only matters for isGlobal groups. `Labile_/Refractory_Detritus_Process()` also always pass `WC` as globalHabitat.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

`Pelagic_Bacteria_Process()` 1674 calls `Oxygen(2, ...)` (O2_IGBEM_id), which returns the constant `1/(KO2+1)`, so pelagic bacteria get no oxygen dependence. Possibly intended.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-3

`Update_Detritus()` 118 vs 138: the local WC DL production includes lysis but DetritusProdGlobal does not (invertebrate lysis is usually 0).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-4

`Sediment_Epi_Other_Process()` 2070: simple_ben_lim uses `biomass/max_id` with no area_hab and no small_num, unlike Epibenthic_Invert_Process 1923.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
