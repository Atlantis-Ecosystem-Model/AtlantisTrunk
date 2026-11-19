# atecology/atGroupProcesses.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **10 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 4 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PROC-19](#proc-19) | 949 | H/M | wrong semantics | `spawn_now = (int)EMBRYO[guild].Spawn_Now[cohort][next_spawn]` and `if ((int)(Spawn_Now[...] == TRUE))`. Spawn… |  | [ ] |
| [PROC-20](#proc-20) | 838, 844 | H/M | copy-paste / wrong habitat | Algal epibenthic branch: detritus goes to `DetritusProd[WC][DLdet_id]` (832), but `Group_Transfer_Contaminant(… |  | [ ] |
| [PROC-21](#proc-21) | 1452 | H/M | double counting | With track_atomic_ratio, `Gain_Element(... LabDetIndex ..., lysis[0] ...)` is called at 1437 using the Primary… |  | [ ] |
| [PROC-22](#proc-22) | 1560 | M/M | wrong habitat index | Dinoflag_Process handles `case WC: case ICE_BASED:` together, but all flux updates are hard-coded to `[WC]` (N… |  | [ ] |
| [PROC-23](#proc-23) | 610 | M/M | inconsistent fluxes | `DetritusProd[..][DLdet_id] += lysis + mortality[0]`, but the element gain (Gain_Element), and the DebugInfo D… |  | [ ] |
| [PROC-24](#proc-24) | 720 | M/M | unit mismatch | `cysts[...][cystenter_id] = (1.0 - (mS_sp + mL_sp + mE_sp)) * cysts[...]`. The mortality rates are per second … |  | [ ] |
| [PROC-25](#proc-25) 🔀 | 308 | M/M | inverted condition / double count | In the EPIFAUNA case, for SED_EP groups: `if (diagTol==2 && it_count==1) NutsProd[SED] += releaseNH; else Nuts… |  | [ ] |
| [PROC-26](#proc-26) | 2182 | M/M | inconsistent reset | After temporarily overwriting phytoplankton PREYinfo with sediment availability, the reset sets `PREYinfo[sp][… |  | [ ] |
| [PROC-27](#proc-27) | 1692, 1824 | M/M | division by zero | With flagkdrop: `pow(PB_DL/(XPB_DL*DL), 3.0)` and `pow(BB_DL/(XBB_DL*DL), 3.0)` have no small_num. When DL (or… |  | [ ] |
| [PROC-28](#proc-28) | 323, 352 | M/L | ungated debug | `if((bm->dayt > bm->checkstart) && (bm->checkbox == bm->current_box)) fprintf(bm->logFile, ...)` is not gated … |  | [ ] |

---

## PROC-19

**wrong semantics** · Conf H / Sev M  
**Found at (current code):** `atGroupProcesses.c` 949
**Lines at review (Oct 2026):** 943, 1096, 1216

**Why it's thought to be a bug**

`spawn_now = (int)EMBRYO[guild].Spawn_Now[cohort][next_spawn]` and `if ((int)(Spawn_Now[...] == TRUE))`. Spawn_Now stores spawn day-of-year (atannualbiology.c:248, 386-390), and atvertprocesses.c:1232 and atdemography.c:895 compare it with the day. So spawn_now is non-zero almost always, and `mat_pcnt` is only set when the spawn day equals 1. The feed-while-spawning reduction in Eat is effectively never applied correctly for AGE_STRUCTURED_BIOMASS inverts. Functions: `Invert_Consumers_Process()`, `Coral_Process()`.

**Code at issue** — `atGroupProcesses.c` line 949 (closest match to the quoted code; wording differs slightly)  
Search for: `spawn_now = (int) (EMBRYO[guild].Spawn_Now[cohort][EMBRYO[guild].next_spawn]);`

```c
  947: 		spawn_now = 0;
  948: 	else
  949→ 		spawn_now = (int) (EMBRYO[guild].Spawn_Now[cohort][EMBRYO[guild].next_spawn]);
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
**Found at (current code):** `atGroupProcesses.c` 838, `atGroupProcesses.c` 844
**Lines at review (Oct 2026):** 832, 838

**Function:** `Phytoplankton_Process()`

**Why it's thought to be a bug**

Algal epibenthic branch: detritus goes to `DetritusProd[WC][DLdet_id]` (845), but `Group_Transfer_Contaminant(bm, boxLayerInfo, EPIFAUNA, SED, LabDetIndex, ...)` moves the contaminant into the SED habitat (the 4th arg is the habitat, atContaminants.c:960). This was copied from the seagrass branch (797), which does use SED detritus. Compare Update_Detritus 382-383 (`EPIFAUNA, WC`). Function: `Phytoplankton_Process()`.

**Code at issue** — `atGroupProcesses.c` line 838  
Search for: `DetritusProd[WC][DLdet_id]`

```c
  836: 				}
  837: 
  838→ 				boxLayerInfo->DetritusProd[WC][DLdet_id] += FunctGroupArray[guild].mortality[cohort] / wcLayerThick;
  839: 
  840: 				/* Contaminants transfer into detritus */
```

**Code at issue** — `atGroupProcesses.c` line 844  
Search for: `Group_Transfer_Contaminant(bm, boxLayerInfo, EPIFAUNA, SED, LabDetIndex,`

```c
  842:                     //fprintf(bm->logFile,"Calling Group_Transfer_Contaminant from Update_Detritus EPIFAUNA SED - %s-%d with initialBiomass: %e mort: %e\n", FunctGroupArray[guild].groupCode, cohort, initialBiomass, FunctGroupArray[guild].mortality[cohort] / wcLayerThick);
  843:                     
  844→                     Group_Transfer_Contaminant(bm, boxLayerInfo, EPIFAUNA, SED, LabDetIndex, 0, guild, cohort,  FunctGroupArray[guild].mortality[cohort] / wcLayerThick, 0, initialBiomass, bm->dtsz_stored, 1, 25);   // Was case 0 for need_prop
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
**Found at (current code):** `atGroupProcesses.c` 1452
**Lines at review (Oct 2026):** 1437, 1526, 1562

**Function:** `Dinoflag_Process()`

**Why it's thought to be a bug**

With track_atomic_ratio, `Gain_Element(... LabDetIndex ..., lysis[0] ...)` is called at 1437 using the Primary_Production lysis. Lysis is then recomputed at 1526, and Gain_Element for DL is called again at 1562. DL elements (P/C) are gained twice per step. Function: `Dinoflag_Process()`.

**Code at issue** — `atGroupProcesses.c` line 1452  
Search for: `Gain_Element(`

```c
 1450: 
 1451: 				/* Gain due to growth */
 1452→ 				Gain_Element(bm, boxLayerInfo, habitatType, guild, 0, guild, 0, FunctGroupArray[guild].growth[0],  WC, isGlobal);
 1453: 			}
 1454: 
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
**Found at (current code):** `atGroupProcesses.c` 1560
**Lines at review (Oct 2026):** 1533-1550

**Function:** `Dinoflag_Process()`

**Why it's thought to be a bug**

Dinoflag_Process handles `case WC: case ICE_BASED:` together, but all flux updates are hard-coded to `[WC]` (NutsLost[WC], DetritusLost[WC], DetritusProd[WC], `GRAZEinfo[RefDetIndex][0][WC]`). For ice dinoflagellates the uptake and detritus fluxes go to the water column, and the ice detritus grazing is not removed. Phytoplankton_Process (878-883) correctly uses `habitatType`. Function: `Dinoflag_Process()`.

**Code at issue** — `atGroupProcesses.c` line 1560  
Search for: `GRAZEinfo[RefDetIndex][0][WC]`

```c
 1558: 			boxLayerInfo->NutsLost[WC][Fe_id] += uptakeFe;
 1559: 
 1560→ 			boxLayerInfo->DetritusLost[WC][DRdet_id] += GRAZEinfo[RefDetIndex][0][WC];
 1561: 			boxLayerInfo->DetritusLost[WC][DLdet_id] += GRAZEinfo[LabDetIndex][0][WC];
 1562: 
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
**Found at (current code):** `atGroupProcesses.c` 610
**Lines at review (Oct 2026):** 604 vs 616/620; 1550 vs 1562/1571

**Why it's thought to be a bug**

`DetritusProd[..][DLdet_id] += lysis + mortality[0]`, but the element gain (Gain_Element), and the DebugInfo DL, only use `lysis`. With flag_replicated_old_PPmort off, mortality[0] is non-zero, so the P/C ratios of DL drift (N added without the matching elements). Functions: `Phytoplankton_Process()`, `Dinoflag_Process()`.

**Code at issue** — `atGroupProcesses.c` line 610 (closest match to the quoted code; wording differs slightly)  
Search for: `boxLayerInfo->DetritusProd[habitatType][DLdet_id] += FunctGroupArray[guild].lysis[cohort] `

```c
  608: 			boxLayerInfo->NutsLost[habitatType][C_id] += uptakeC;
  609: 
  610→ 			boxLayerInfo->DetritusProd[habitatType][DLdet_id] += FunctGroupArray[guild].lysis[cohort] + FunctGroupArray[guild].mortality[0];
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
**Found at (current code):** `atGroupProcesses.c` 720
**Lines at review (Oct 2026):** 714

**Function:** `Phytoplankton_Process()`

**Why it's thought to be a bug**

`cysts[...][cystenter_id] = (1.0 - (mS_sp + mL_sp + mE_sp)) * cysts[...]`. The mortality rates are per second (used as `rate * biomass` fluxes elsewhere), so this kills a negligible per-second fraction per call, with no dt, independent of timestep. Function: `Phytoplankton_Process()`.

**Code at issue** — `atGroupProcesses.c` line 720  
Search for: `cysts[`

```c
  718: 
  719: 					/* Kill off cysts too -TODO should we actually used the SED pool as the cyst storage facility rather than a cysts array? */
  720→ 					cysts[guild][bm->current_box][cystenter_id] = (1.0 - (mS_sp + mL_sp + mE_sp)) * cysts[guild][bm->current_box][cystenter_id];
  721: 				}
  722: 
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
**Found at (current code):** `atGroupProcesses.c` 308
**Lines at review (Oct 2026):** 299-303, 338

**Function:** `Update_Detritus()`

**Why it's thought to be a bug**

In the EPIFAUNA case, for SED_EP groups: `if (diagTol==2 && it_count==1) NutsProd[SED] += releaseNH; else NutsProdGlobal[EPIFAUNA][SED] += releaseNH;`. Every other case updates the local flux always and the global flux only when diagTol==2 && it_count==1. Here the non-global groups never get a local NH flux, and in the global case NutsProdGlobal[EPIFAUNA][SED] is incremented again at 338. Function: `Update_Detritus()`.

> 🔀 **Changed by the merge of main (Nov 2026):** diagTol for CARRION/PWN changed. Check whether this finding is now fixed before acting.

**Code at issue** — `atGroupProcesses.c` line 308 (the line the review pointed to, found by its text)  
Search for: `boxLayerInfo->NutsProdGlobal[EPIFAUNA][SED][NH_id] += FunctGroupArray[guild].releaseNH[coh`

```c
  306: 				boxLayerInfo->NutsProd[SED][NH_id] += FunctGroupArray[guild].releaseNH[cohort];
  307: 			} else {
  308→ 				boxLayerInfo->NutsProdGlobal[EPIFAUNA][SED][NH_id] += FunctGroupArray[guild].releaseNH[cohort]; // Was boxLayerInfo->NutsProd[WC][NH_id]
  309: 			}
  310: 
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
**Found at (current code):** `atGroupProcesses.c` 2182
**Lines at review (Oct 2026):** 2157-2166

**Function:** `Sediment_Epi_Other_Process()`

**Why it's thought to be a bug**

After temporarily overwriting phytoplankton PREYinfo with sediment availability, the reset sets `PREYinfo[sp][chrt][WC] = localWCTracers[...]` without the `habitatCoeffs[WC]` (and ice MIXED) scaling used by Construct_Prey_Info (atprocess.c:2404, 2423), and leaves `PREYinfo[...][SED] = 0`. Predators processed later in the same cell see different phytoplankton availability. Function: `Sediment_Epi_Other_Process()`.

**Code at issue** — `atGroupProcesses.c` line 2182 (closest match to the quoted code; wording differs slightly)  
Search for: `PREYinfo[sp][prey_chrt][WC] = boxLayerInfo->localWCTracers[FunctGroupArray[sp].totNTracers`

```c
 2180: 			if (FunctGroupArray[sp].groupType == LG_PHY || FunctGroupArray[sp].groupType == DINOFLAG || FunctGroupArray[sp].groupType == SM_PHY) {
 2181: 				for (prey_chrt = 0; prey_chrt < FunctGroupArray[sp].numCohortsXnumGenes; prey_chrt++) {
 2182→ 					PREYinfo[sp][prey_chrt][WC] = boxLayerInfo->localWCTracers[FunctGroupArray[sp].totNTracers[prey_chrt]] * FunctGroupArray[sp].habitatCoeffs[WC];
 2183: 					PREYinfo[sp][prey_chrt][SED] = 0.0;
 2184: 				}
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
**Found at (current code):** `atGroupProcesses.c` 1692, `atGroupProcesses.c` 1824
**Lines at review (Oct 2026):** 1679-1680, 1811-1812

**Why it's thought to be a bug**

With flagkdrop: `pow(PB_DL/(XPB_DL*DL), 3.0)` and `pow(BB_DL/(XBB_DL*DL), 3.0)` have no small_num. When DL (or DR) is 0, the numerator is also 0, giving 0/0 = NaN growth (`max(0.0, NaN)` is unreliable). Functions: `Pelagic_Bacteria_Process()`, `Sediment_Bacterica_Process()`.

**Code at issue** — `atGroupProcesses.c` line 1692 (closest match to the quoted code; wording differs slightly)  
Search for: `potential_PB = FunctGroupArray[guild].scaled_mum[cohort] * (boxLayerInfo->PB_DL * (1.0 - p`

```c
 1690: 
 1691: 		if (flagkdrop)
 1692→ 			potential_PB = FunctGroupArray[guild].scaled_mum[cohort] * (boxLayerInfo->PB_DL * (1.0 - pow(boxLayerInfo->PB_DL / (XPB_DL * DL), 3.0))
 1693: 					+ boxLayerInfo->PB_DR * (1.0 - pow(boxLayerInfo->PB_DR / (XPB_DR * DR), 3.0)));
 1694: 		else {
```

**Code at issue** — `atGroupProcesses.c` line 1824  
Search for: `pow(BB_DL / (XBB_DL * DL), 3.0)`

```c
 1822: 
 1823: 		if (flagkdrop) {
 1824→ 			potential_BB = FunctGroupArray[guild].scaled_mum[cohort] * (BB_DL * (1.0 - pow(BB_DL / (XBB_DL * DL), 3.0)) + BB_DR * (1.0 - pow(BB_DR
 1825: 					/ (XBB_DR * DR), 3.0)));
 1826: 		} else {/* Gain due to growth */
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
**Found at (current code):** `atGroupProcesses.c` 323, `atGroupProcesses.c` 352
**Lines at review (Oct 2026):** 317-322, 346-352

**Function:** `Update_Detritus()`

**Why it's thought to be a bug**

`if((bm->dayt > bm->checkstart) && (bm->checkbox == bm->current_box)) fprintf(bm->logFile, ...)` is not gated on bm->debug or which_check. It prints two lines for every epibenthic group, every iteration, in the check box once past checkstart, which can flood the log. Function: `Update_Detritus()`.

**Code at issue** — `atGroupProcesses.c` line 323 (the line the review pointed to, found by its text)  
Search for: `if((bm->dayt > bm->checkstart) && (bm->checkbox == bm->current_box)) {`

```c
  321: 		}
  322:             
  323→         if((bm->dayt > bm->checkstart) && (bm->checkbox == bm->current_box)) {
  324:             fprintf(bm->logFile, "Time: %e box%d-%d %s epiDLprod: %Lf (%Lf) DLlost: %Lf (%Lf) graze %e (%e) prodnDet: %e\n",
  325:                 bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, boxLayerInfo->DetritusProd[WC][DLdet_id], boxLayerInfo->DetritusProd[SED][DLdet_id],
```

**Code at issue** — `atGroupProcesses.c` line 352 (the line the review pointed to, found by its text)  
Search for: `if((bm->dayt > bm->checkstart) && (bm->checkbox == bm->current_box)) {`

```c
  350: 			}
  351:             
  352→             if((bm->dayt > bm->checkstart) && (bm->checkbox == bm->current_box)) {
  353:                 fprintf(bm->logFile, "Time: %e box%d-%d %s epiDLprodGLOBAL: %Lf (%Lf) DLlostGLOBAL: %Lf (%Lf) grazeGLOBAL %e (%e) prodnGLOBAL: %e\n",
  354:                     bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode,
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
