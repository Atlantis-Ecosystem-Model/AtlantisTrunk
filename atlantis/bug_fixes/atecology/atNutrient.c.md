# atecology/atNutrient.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **7 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 2 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [NUTR-01](#nutr-01) | 498-519, 708-725, 808-817, 906-927, 972-981, 1033-1050, 1161-1170 | H/M | wrong condition / double counting | The atmospheric-input blocks are gated only by `bm->current_layer == nz-1`, not by `habitatType == WC`. Each R… |  | [ ] |
| [NUTR-02](#nutr-02) | 895-897 | H/M | copy-paste | `localICEFlux[Oxygen_i] = -bm->X_ON * (FunctGroupArray[LabDetIndex].solDON + ... + FunctGroupArray[IceBactInde… |  | [ ] |
| [NUTR-03](#nutr-03) | 1017-1020 | M/M | missing term | The EPIFAUNA case adds `NutsProd[WC][C_id]` and `NutsProd[SED][C_id]` but never subtracts `NutsLost[..][C_id]`… |  | [ ] |
| [NUTR-04](#nutr-04) | 1038-1044 | M/M | wrong tracer | Atmospheric CO2 is added to `*PhysioChemArray[index].tracerIndex`, which is `C_i` ("Carbon"). But the outgassi… |  | [ ] |
| [NUTR-05](#nutr-05) | 509, 719, 921, 1044 | H/L | wrong variable / sign | The outgassing debug loss uses `atmospheric_X` instead of `outgas_X` (NH 509, O2 921, CO2 1044). In Nitrate (7… |  | [ ] |
| [NUTR-06](#nutr-06) | 603-612 | M/L | cohort loop missing | The ICE case uses only `mortality[0]` and `lysis[0]` with no loop over `numCohortsXnumGenes`, unlike the WC/SE… |  | [ ] |
| [NUTR-07](#nutr-07) | 792-806 | L/L | incomplete reset | When `!bm->flagmicro`, WC and SED fluxes are zeroed but `localICEFlux[MicroNut]` is not, even though the ICE d… |  | [ ] |

---

## NUTR-01

**wrong condition / double counting** · Conf H / Sev M  
**Listed lines:** 498-519, 708-725, 808-817, 906-927, 972-981, 1033-1050, 1161-1170

**Why it's thought to be a bug**

The atmospheric-input blocks are gated only by `bm->current_layer == nz-1`, not by `habitatType == WC`. Each ROC function is called for WC, SED, EPIFAUNA and ICE_BASED (atecology.c:1082, 1847, 2811, 3548). In a box with one water-column layer (nz==1) the WC loop in atbiology.c:414 skips layer 0 (`ij > stopij`, stopij=0), and Epibenthic_Box (atecology.c:2132-2135 then its own ROC loop) runs WC, SED and EPIFAUNA all with `current_layer==0==nz-1`. `localWCFlux` is not reset between those calls (Reset_Arrays atecology.c:3970 does not touch it) and the EPIFAUNA cases use `+=`, so atmospheric NH, NO, Fe, O2, Si, CO2 and P inputs get added 3 times (2 times for Oxygen/Si/DON-like cases that `=` in WC). The SED call also adds to localWCFlux when a sediment layer index equals nz-1. Functions: `Ammonium_ROC()`, `Nitrate_ROC()`, `Micronutrient_ROC()`, `Oxygen_ROC()`, `Dissolved_Silica_ROC()`, `Carbon_ROC()`, `Phosphorus_ROC()`.

**Current code** (`atecology/atNutrient.c` lines 496–521)

```c
  496: 	}
  497:     
  498: 	if(bm->include_atmosphere){
  499: 		/** Get NH3 values from atmosphare on the surface cell **/
  500: 		if (bm->current_layer == (bm->boxes[bm->current_box].nz - 1)) {
  501: 			dzz = bm->boxes[bm->current_box].dz[bm->current_layer];
  502: 			if (dzz != 0.0) {
  503: 			   boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] += (bm->atmospheric_NH / dzz) / numsec;
  504: 			   boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][gain_id] += (bm->atmospheric_NH / dzz) / numsec;
  505:         
  506:                 if (bm->include_outgas) {
  507:                     if (boxLayerInfo->localWCTracers[NH3_i] > (bm->outgas_prop * 12500.0)) { // Where 12500.0 is the saturation point
  508:                         boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] -= (bm->outgas_NH / dzz) / numsec;
  509:                         boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][loss_id] += (bm->atmospheric_NH / dzz) / numsec;
  510:                         
  511:                         //if ((bm->current_box == 142) && bm->newmonth){
  512:                         if (bm->newmonth){
  513:                             fprintf(llogfp,"Time: %e box%d-%d outgassed\n", bm->dayt, bm->current_box, bm->current_layer);
  514:                         }
  515:                     }
  516:                 }
  517:             }
  518:         }
  519:     }
  520: }
  521: 
```

**Suggested fix (from review):** Wrap each atmosphere block in `if (habitatType == WC && ...)`.

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

## NUTR-02

**copy-paste** · Conf H / Sev M  
**Listed lines:** 895-897

**Function:** `Oxygen_ROC()`

**Why it's thought to be a bug**

`localICEFlux[Oxygen_i] = -bm->X_ON * (FunctGroupArray[LabDetIndex].solDON + ... + FunctGroupArray[IceBactIndex].prodnDON)` is the DON_ROC ICE expression (line 850), not the NH3+NO3 flux used in the WC/SED/EPIFAUNA cases (lines 874, 881, 887). Ice oxygen ends up driven by DON production. No debug info set either. Function: `Oxygen_ROC()`.

**Current code** (`atecology/atNutrient.c` lines 893–899)

```c
  893: 		break;
  894: 
  895: 	case ICE_BASED:
  896: 		boxLayerInfo->localICEFlux[Oxygen_i] = -bm->X_ON * (FunctGroupArray[LabDetIndex].solDON + FunctGroupArray[RefDetIndex].solDON - boxLayerInfo->DONremin + FunctGroupArray[IceBactIndex].prodnDON);
  897: 		break;
  898:     case MIXED:
  899:         quit("How did we get here as should come through a primary habitat\n");
```

**Suggested fix (from review):** `= -bm->X_ON * (boxLayerInfo->localICEFlux[NH3_i] + boxLayerInfo->localICEFlux[NO3_i]);`

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

## NUTR-03

**missing term** · Conf M / Sev M  
**Listed lines:** 1017-1020

**Function:** `Carbon_ROC()`

**Why it's thought to be a bug**

The EPIFAUNA case adds `NutsProd[WC][C_id]` and `NutsProd[SED][C_id]` but never subtracts `NutsLost[..][C_id]`. Compare Phosphorus_ROC EPIFAUNA (1145-1146), which uses Prod − Lost. Epibenthic carbon uptake gets dropped. Function: `Carbon_ROC()`.

**Current code** (`atecology/atNutrient.c` lines 1015–1022)

```c
 1015: 
 1016: 		break;
 1017: 	case EPIFAUNA:
 1018: 		boxLayerInfo->localWCFlux[C_i] += (double)boxLayerInfo->NutsProd[WC][C_id];
 1019: 		boxLayerInfo->localSEDFlux[C_i] += (double)boxLayerInfo->NutsProd[SED][C_id];
 1020: 		break;
 1021: 
 1022: 	case ICE_BASED:
```

**Suggested fix (from review):** Add `- (double)boxLayerInfo->NutsLost[WC][C_id]` (and the SED one).

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

## NUTR-04

**wrong tracer** · Conf M / Sev M  
**Listed lines:** 1038-1044

**Function:** `Carbon_ROC()`

**Why it's thought to be a bug**

Atmospheric CO2 is added to `*PhysioChemArray[index].tracerIndex`, which is `C_i` ("Carbon"). But the outgassing saturation test reads `localWCTracers[CO2_i]`, a separate tracer (atPhysChemIO.c:122-123). Either the input goes to the wrong tracer or the test reads the wrong one. Function: `Carbon_ROC()`.

**Current code** (`atecology/atNutrient.c` lines 1036–1046)

```c
 1036: 			dzz = bm->boxes[bm->current_box].dz[bm->current_layer];
 1037: 			if (dzz != 0.0) {
 1038: 				boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] += (bm->atmospheric_CO2 / dzz) / numsec;
 1039: 				boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][gain_id] += (bm->atmospheric_CO2 / dzz) / numsec;
 1040:                 
 1041:                 if (bm->include_outgas) { /* TODO: Check what saturation point of CO2 is */
 1042:                     if (boxLayerInfo->localWCTracers[CO2_i] > (bm->outgas_prop * 15000.0)) { // Where 15000.0 is the saturation point
 1043:                         boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] -= (bm->outgas_CO2 / dzz) / numsec;
 1044:                         boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][loss_id] += (bm->atmospheric_CO2 / dzz) / numsec;
 1045:                     }
 1046:                 }
```

**Suggested fix (from review):** Use one tracer consistently (probably CO2_i for both, or C_i for both).

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

## NUTR-05

**wrong variable / sign** · Conf H / Sev L  
**Listed lines:** 509, 719, 921, 1044

**Why it's thought to be a bug**

The outgassing debug loss uses `atmospheric_X` instead of `outgas_X` (NH 509, O2 921, CO2 1044). In Nitrate (719) it is also `-=` on `loss_id`, so the recorded loss goes negative. Only debug flux output is affected. Functions: `Ammonium_ROC()`, `Nitrate_ROC()`, `Oxygen_ROC()`, `Carbon_ROC()`.

**Current code** (`atecology/atNutrient.c` lines 507–511)

```c
  507:                     if (boxLayerInfo->localWCTracers[NH3_i] > (bm->outgas_prop * 12500.0)) { // Where 12500.0 is the saturation point
  508:                         boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] -= (bm->outgas_NH / dzz) / numsec;
  509:                         boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][loss_id] += (bm->atmospheric_NH / dzz) / numsec;
  510:                         
  511:                         //if ((bm->current_box == 142) && bm->newmonth){
```

**Suggested fix (from review):** `DebugFluxInfo[..][WC][loss_id] += (bm->outgas_X / dzz) / numsec;`

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

## NUTR-06

**cohort loop missing** · Conf M / Sev L  
**Listed lines:** 603-612

**Function:** `Detrital_Silica_ROC()`

**Why it's thought to be a bug**

The ICE case uses only `mortality[0]` and `lysis[0]` with no loop over `numCohortsXnumGenes`, unlike the WC/SED cases (540, 573). Si-dependent groups with genotypes (>1 cohort slot) lose all but slot 0. Function: `Detrital_Silica_ROC()`.

**Current code** (`atecology/atNutrient.c` lines 607–620) — code appears to have moved +6 lines since the review

```c
  607: 		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
  608: 			if (FunctGroupArray[guild].isSiliconDependant) {
  609: 				Si_Flux += (FunctGroupArray[guild].mortality[0] + FunctGroupArray[guild].lysis[0]);
  610: 						//+ FunctGroupArray[guild].preyEaten[0][habitatType]);
  611: 			}
  612: 		}
  613: 
  614: 		boxLayerInfo->localICEFlux[*PhysioChemArray[index].tracerIndex] = Si_Flux * bm->X_SiN - r_DSi * DSi;
  615: 		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][ICE_BASED][gain_id] = Si_Flux * bm->X_SiN;
  616: 		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][ICE_BASED][loss_id] = r_DSi * DSi;
  617: 		break;
  618:     case MIXED:
  619:         quit("How did we get here as should come through a primary habitat\n");
  620:         break;
```

**Suggested fix (from review):** Loop `cohort < numCohortsXnumGenes` as in WC case.

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

## NUTR-07

**incomplete reset** · Conf L / Sev L  
**Listed lines:** 792-806

**Function:** `Micronutrient_ROC()`

**Why it's thought to be a bug**

When `!bm->flagmicro`, WC and SED fluxes are zeroed but `localICEFlux[MicroNut]` is not, even though the ICE debug values are. Function: `Micronutrient_ROC()`.

**Current code** (`atecology/atNutrient.c` lines 790–808)

```c
  790:             break;
  791: 		}
  792: 	} else {
  793: 		/* Set all the tracers and debug values to 0 */
  794: 		boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] = 0.0;
  795: 		boxLayerInfo->localSEDFlux[*PhysioChemArray[index].tracerIndex] = 0.0;
  796: 
  797: 		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][gain_id] = 0.0;
  798: 		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][loss_id] = 0.0;
  799: 		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][SED][gain_id] = 0.0;
  800: 		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][SED][loss_id] = 0.0;
  801: 
  802: 		if(bm->ice_on) {
  803: 			boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][ICE_BASED][gain_id] = 0.0;
  804: 			boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][ICE_BASED][loss_id] = 0.0;
  805: 		}
  806: 	}
  807:     
  808: 	if(bm->include_atmosphere){
```

**Suggested fix (from review):** Also zero `localICEFlux[*tracerIndex]` when ice_on.

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

atNutrient.c `Carrion_ROC()` (291-313): the WC case subtracts no `DetritusLost[WC][DCdet_id]`, and the SED case neither adds `DetritusProd[SED][DCdet_id]` nor subtracts Lost. Refractory/labile handle both in all cases. This may be intentional, with EPIFAUNA (327-342) accumulating them, but it is worth checking.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atNutrient.c `Micronutrient_ROC()` (742): the WC Fe gain is derived from `localWCFlux[NH3_i]`, which already includes atmospheric NH (and its multiple counting, NUTR-01), so Fe also picks up X_FeN × atmospheric NH.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
