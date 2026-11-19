# atecology/atNutrient.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **7 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 2 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [NUTR-01](#nutr-01) | 500 | H/M | wrong condition / double counting | The atmospheric-input blocks are gated only by `bm->current_layer == nz-1`, not by `habitatType == WC`. Each R… |  | [ ] |
| [NUTR-02](#nutr-02) | 896 | H/M | copy-paste | `localICEFlux[Oxygen_i] = -bm->X_ON * (FunctGroupArray[LabDetIndex].solDON + ... + FunctGroupArray[IceBactInde… |  | [ ] |
| [NUTR-03](#nutr-03) | 1018, 1011 | M/M | missing term | The EPIFAUNA case adds `NutsProd[WC][C_id]` and `NutsProd[SED][C_id]` but never subtracts `NutsLost[..][C_id]`… |  | [ ] |
| [NUTR-04](#nutr-04) | 1038, 1042 | M/M | wrong tracer | Atmospheric CO2 is added to `*PhysioChemArray[index].tracerIndex`, which is `C_i` ("Carbon"). But the outgassi… |  | [ ] |
| [NUTR-05](#nutr-05) | 631 | H/L | wrong variable / sign | The outgassing debug loss uses `atmospheric_X` instead of `outgas_X` (NH 509, O2 921, CO2 1044). In Nitrate (7… |  | [ ] |
| [NUTR-06](#nutr-06) | 609 | M/L | cohort loop missing | The ICE case uses only `mortality[0]` and `lysis[0]` with no loop over `numCohortsXnumGenes`, unlike the WC/SE… |  | [ ] |
| [NUTR-07](#nutr-07) | 739 | L/L | incomplete reset | When `!bm->flagmicro`, WC and SED fluxes are zeroed but `localICEFlux[MicroNut]` is not, even though the ICE d… |  | [ ] |

---

## NUTR-01

**wrong condition / double counting** · Conf H / Sev M  
**Found at (current code):** `atNutrient.c` 500
**Lines at review (Oct 2026):** 498-519, 708-725, 808-817, 906-927, 972-981, 1033-1050, 1161-1170

**Why it's thought to be a bug**

The atmospheric-input blocks are gated only by `bm->current_layer == nz-1`, not by `habitatType == WC`. Each ROC function is called for WC, SED, EPIFAUNA and ICE_BASED (atecology.c:1081, 1849, 2813, 3553). In a box with one water-column layer (nz==1) the WC loop in atbiology.c:449 skips layer 0 (`ij > stopij`, stopij=0), and Epibenthic_Box (atecology.c:2134-2137 then its own ROC loop) runs WC, SED and EPIFAUNA all with `current_layer==0==nz-1`. `localWCFlux` is not reset between those calls (Reset_Arrays atecology.c:3975 does not touch it) and the EPIFAUNA cases use `+=`, so atmospheric NH, NO, Fe, O2, Si, CO2 and P inputs get added 3 times (2 times for Oxygen/Si/DON-like cases that `=` in WC). The SED call also adds to localWCFlux when a sediment layer index equals nz-1. Functions: `Ammonium_ROC()`, `Nitrate_ROC()`, `Micronutrient_ROC()`, `Oxygen_ROC()`, `Dissolved_Silica_ROC()`, `Carbon_ROC()`, `Phosphorus_ROC()`.

**Code at issue** — `atNutrient.c` line 500 (closest match to the quoted code; wording differs slightly)  
Search for: `if (bm->current_layer == (bm->boxes[bm->current_box].nz - 1)) {`

```c
  498: 	if(bm->include_atmosphere){
  499: 		/** Get NH3 values from atmosphare on the surface cell **/
  500→ 		if (bm->current_layer == (bm->boxes[bm->current_box].nz - 1)) {
  501: 			dzz = bm->boxes[bm->current_box].dz[bm->current_layer];
  502: 			if (dzz != 0.0) {
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
**Found at (current code):** `atNutrient.c` 896
**Lines at review (Oct 2026):** 895-897

**Function:** `Oxygen_ROC()`

**Why it's thought to be a bug**

`localICEFlux[Oxygen_i] = -bm->X_ON * (FunctGroupArray[LabDetIndex].solDON + ... + FunctGroupArray[IceBactIndex].prodnDON)` is the DON_ROC ICE expression (line 851), not the NH3+NO3 flux used in the WC/SED/EPIFAUNA cases (lines 874, 881, 887). Ice oxygen ends up driven by DON production. No debug info set either. Function: `Oxygen_ROC()`.

**Code at issue** — `atNutrient.c` line 896  
Search for: `localICEFlux[Oxygen_i] = -bm->X_ON * (FunctGroupArray[LabDetIndex].solDON +`

```c
  894: 
  895: 	case ICE_BASED:
  896→ 		boxLayerInfo->localICEFlux[Oxygen_i] = -bm->X_ON * (FunctGroupArray[LabDetIndex].solDON + FunctGroupArray[RefDetIndex].solDON - boxLayerInfo->DONremin + FunctGroupArray[IceBactIndex].prodnDON);
  897: 		break;
  898:     case MIXED:
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
**Found at (current code):** `atNutrient.c` 1018, `atNutrient.c` 1011
**Lines at review (Oct 2026):** 1017-1020

**Function:** `Carbon_ROC()`

**Why it's thought to be a bug**

The EPIFAUNA case adds `NutsProd[WC][C_id]` and `NutsProd[SED][C_id]` but never subtracts `NutsLost[..][C_id]`. Compare Phosphorus_ROC EPIFAUNA (1141-1142), which uses Prod − Lost. Epibenthic carbon uptake gets dropped. Function: `Carbon_ROC()`.

**Code at issue** — `atNutrient.c` line 1018  
Search for: `NutsProd[WC][C_id]`

```c
 1016: 		break;
 1017: 	case EPIFAUNA:
 1018→ 		boxLayerInfo->localWCFlux[C_i] += (double)boxLayerInfo->NutsProd[WC][C_id];
 1019: 		boxLayerInfo->localSEDFlux[C_i] += (double)boxLayerInfo->NutsProd[SED][C_id];
 1020: 		break;
```

**Code at issue** — `atNutrient.c` line 1011 (closest match to the quoted code; wording differs slightly)  
Search for: `- (double)boxLayerInfo->NutsLost[habitatType][C_id]`

```c
 1009: 					+ bm->atomicRatioInfo->C_decomp
 1010: 					+ bm->atomicRatioInfo->C_trans
 1011→ 					- (double)boxLayerInfo->NutsLost[habitatType][C_id]
 1012: 					+ (double)boxLayerInfo->NutsProd[habitatType][C_id];
 1013: 
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
**Found at (current code):** `atNutrient.c` 1038, `atNutrient.c` 1042
**Lines at review (Oct 2026):** 1038-1044

**Function:** `Carbon_ROC()`

**Why it's thought to be a bug**

Atmospheric CO2 is added to `*PhysioChemArray[index].tracerIndex`, which is `C_i` ("Carbon"). But the outgassing saturation test reads `localWCTracers[CO2_i]`, a separate tracer (atPhysChemIO.c:122-123). Either the input goes to the wrong tracer or the test reads the wrong one. Function: `Carbon_ROC()`.

**Code at issue** — `atNutrient.c` line 1038  
Search for: `*PhysioChemArray[index].tracerIndex`

```c
 1036: 			dzz = bm->boxes[bm->current_box].dz[bm->current_layer];
 1037: 			if (dzz != 0.0) {
 1038→ 				boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] += (bm->atmospheric_CO2 / dzz) / numsec;
 1039: 				boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][gain_id] += (bm->atmospheric_CO2 / dzz) / numsec;
 1040:                 
```

**Code at issue** — `atNutrient.c` line 1042  
Search for: `localWCTracers[CO2_i]`

```c
 1040:                 
 1041:                 if (bm->include_outgas) { /* TODO: Check what saturation point of CO2 is */
 1042→                     if (boxLayerInfo->localWCTracers[CO2_i] > (bm->outgas_prop * 15000.0)) { // Where 15000.0 is the saturation point
 1043:                         boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] -= (bm->outgas_CO2 / dzz) / numsec;
 1044:                         boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][loss_id] += (bm->atmospheric_CO2 / dzz) / numsec;
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
**Found at (current code):** `atNutrient.c` 631
**Lines at review (Oct 2026):** 509, 719, 921, 1044

**Why it's thought to be a bug**

The outgassing debug loss uses `atmospheric_X` instead of `outgas_X` (NH 509, O2 921, CO2 1044). In Nitrate (721) it is also `-=` on `loss_id`, so the recorded loss goes negative. Only debug flux output is affected. Functions: `Ammonium_ROC()`, `Nitrate_ROC()`, `Oxygen_ROC()`, `Carbon_ROC()`.

**Code at issue** — `atNutrient.c` line 631 (located by name only; check this is the right place)  
Search for: `void Nitrate_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int index, BoxL`

```c
  629:  *
  630:  */
  631→ void Nitrate_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int index, BoxLayerValues *boxLayerInfo) {
  632: 
  633:     double dzz, numsec = 86400.0;
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
**Found at (current code):** `atNutrient.c` 609
**Lines at review (Oct 2026):** 603-612

**Function:** `Detrital_Silica_ROC()`

**Why it's thought to be a bug**

The ICE case uses only `mortality[0]` and `lysis[0]` with no loop over `numCohortsXnumGenes`, unlike the WC/SED cases (542, 575). Si-dependent groups with genotypes (>1 cohort slot) lose all but slot 0. Function: `Detrital_Silica_ROC()`.

**Code at issue** — `atNutrient.c` line 609  
Search for: `mortality[0]`

```c
  607: 		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
  608: 			if (FunctGroupArray[guild].isSiliconDependant) {
  609→ 				Si_Flux += (FunctGroupArray[guild].mortality[0] + FunctGroupArray[guild].lysis[0]);
  610: 						//+ FunctGroupArray[guild].preyEaten[0][habitatType]);
  611: 			}
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
**Found at (current code):** `atNutrient.c` 739
**Lines at review (Oct 2026):** 792-806

**Function:** `Micronutrient_ROC()`

**Why it's thought to be a bug**

When `!bm->flagmicro`, WC and SED fluxes are zeroed but `localICEFlux[MicroNut]` is not, even though the ICE debug values are. Function: `Micronutrient_ROC()`.

**Code at issue** — `atNutrient.c` line 739 (located by name only; check this is the right place)  
Search for: `if (bm->flagmicro) {`

```c
  737:     double dzz, numsec = 86400.0;
  738: 
  739→ 	if (bm->flagmicro) {
  740: 		switch (habitatType) {
  741: 		case WC:
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

atNutrient.c `Micronutrient_ROC()` (744): the WC Fe gain is derived from `localWCFlux[NH3_i]`, which already includes atmospheric NH (and its multiple counting, NUTR-01), so Fe also picks up X_FeN × atmospheric NH.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
