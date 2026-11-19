# atecology/additionalTracer.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **11 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1), plus 2 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [BIOL-05](#biol-05) | 827 | H/M | off-by-one / wrong bound | `for (tracerIndex = 0; tracerIndex <= p_id; tracerIndex++)` — `p_id = 0`, `c_id = 1` (atlantisboxmodel.h:463-4… |  | [ ] |
| [BIOL-06](#biol-06) | 363 | H/M | off-by-one / always-false | `if (bm->current_layer == bm->boxes[bm->current_box].nz)` — layers are 0..nz-1, so this is never true. The air… |  | [ ] |
| [BIOL-07](#biol-07) | 325 | H/M | unit mismatch | `RTOP = bm->atomicRatioInfo->RTOP / (24.0 * 86400.0); /* divide to get rate per second */` — RTOP is documente… |  | [ ] |
| [BIOL-08](#biol-08) ⭐ | 1311-1346 | H/H | unit mismatch / integration bypass | `tracerArray[DIP_i] += net_desorp/porosity; tracerArray[PIP_i] -= ...` — `net_desorp = Pads_r * (...)` is a ra… |  | [ ] |
| [BIOL-09](#biol-09) | 1296-1309 | H/M | dead code / missing init | `p_adsorption_setup()` is never called anywhere in the repo (grep finds only its definition, and it has no pro… |  | [ ] |
| [BIOL-10](#biol-10) | 1308, 1346 | M/M | accumulator | `tracerArray[TIP_i] += PIP + PIPI + PIP_Dust;` — total inorganic P should be the sum (`=`), but it is accumula… |  | [ ] |
| [BIOL-11](#biol-11) | 457 | H/M | debug output left live | An unconditional `fprintf(bm->logFile, "%s-%d Box %d-%d Density: ...")` runs for every age-structured cohort, … |  | [ ] |
| [BIOL-12](#biol-12) | 655 | H/M | debug output left live | An unconditional `fprintf(bm->logFile, "Group %s Box %d layer %d newN: ...")` runs for every primary-producer … |  | [ ] |
| [BIOL-13](#biol-13) | 1111-1118 | H/M | debug output left live | `/**/ if(amount > 0 && toGuild == 0){ fprintf(bm->logFile, ...) } /**/` — the empty `/**/` markers leave this … |  | [ ] |
| [BIOL-14](#biol-14) | 755-759 | M/L | uninitialised variable / stdout spam | Inside the `flagratio_warn` block, the `/**/ ... /**/` markers leave a `printf` to stdout plus an `fprintf` li… |  | [ ] |
| [BIOL-15](#biol-15) | 450, 624 | M/M | cohort vs cohort×genotype | Both ratio loops use `cohort < FunctGroupArray[sp].numCohorts`. `addRatioTracers`/`ratioLost`/`ratioGainedPred… |  | [ ] |

---

## BIOL-05

**off-by-one / wrong bound** · Conf H / Sev M  
**Listed lines:** 827

**Function:** `Calculate_Element_Release()`

**Why it's thought to be a bug**

`for (tracerIndex = 0; tracerIndex <= p_id; tracerIndex++)` — `p_id = 0`, `c_id = 1` (atlantisboxmodel.h:463-468), so the loop runs only for P. The `case c_id:` branch can never run, carbon released by breakdown/excretion is never added to `NutsProd[..][C_id]`, and `Calculate_Carbon_Decomposition()` (l.299) always sees only other contributions. Function: `Calculate_Element_Release()`.

**Current code** (`atecology/additionalTracer.c` lines 825–829)

```c
  825: 	double *tracerArray = getTracerArray(boxLayerInfo, habitat);
  826: 
  827: 	for (tracerIndex = 0; tracerIndex <= p_id; tracerIndex++) {
  828: 
  829: 		tracerValue = tracerArray[FunctGroupArray[guild].addRatioTracers[0][tracerIndex]];
```

**Suggested fix (from review):** `tracerIndex < num_atomic_id`

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

## BIOL-06

**off-by-one / always-false** · Conf H / Sev M  
**Listed lines:** 363

**Function:** `Calculate_Carbon_Transformation()`

**Why it's thought to be a bug**

`if (bm->current_layer == bm->boxes[bm->current_box].nz)` — layers are 0..nz-1, so this is never true. The air-sea CO2 exchange `*trans` is never computed and `C_trans` stays 0 for the whole run. When it is called for SED/EPIFAUNA, `current_layer` is not a WC index anyway. Function: `Calculate_Carbon_Transformation()`.

**Current code** (`atecology/additionalTracer.c` lines 361–365)

```c
  361: 
  362: 	/* If this is the top layer */
  363: 	if (bm->current_layer == bm->boxes[bm->current_box].nz) {
  364: 		T = tracerArray[Temp_i];
  365: 		P_a = 1.0;
```

**Suggested fix (from review):** `if (habitatType == WC && bm->current_layer == nz - 1)`, else `*trans = 0`

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

## BIOL-07

**unit mismatch** · Conf H / Sev M  
**Listed lines:** 325

**Function:** `Calculate_Phosphorus_Transformation()`

**Why it's thought to be a bug**

`RTOP = bm->atomicRatioInfo->RTOP / (24.0 * 86400.0); /* divide to get rate per second */` — RTOP is documented as days^-1 (atBioltoXML.c:2377, atNutrient.c:1083), and day^-1 to s^-1 is /86400. The extra /24 makes P transformation 24x too slow. Function: `Calculate_Phosphorus_Transformation()`.

**Current code** (`atecology/additionalTracer.c` lines 323–327)

```c
  323: 	TOP = tracerArray[TOP_i];
  324: 
  325: 	RTOP = bm->atomicRatioInfo->RTOP / (24.0 * 86400.0); /* divide to get rate per second */
  326: 
  327: //	if(bm->current_box == 1 && bm->current_layer == 3){
```

**Suggested fix (from review):** `/ 86400.0`

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

## BIOL-08

**unit mismatch / integration bypass** · Conf H / Sev H ⭐ priority  
**Listed lines:** 1311-1346

**Function:** `p_adsorption_calc()`

**Why it's thought to be a bug**

`tracerArray[DIP_i] += net_desorp/porosity; tracerArray[PIP_i] -= ...` — `net_desorp = Pads_r * (...)` is a rate (EMS source writes it to derivatives), but here it is added straight to the local *state* with no `dt`. It is also applied on every adaptive sub-step through `Calculate_Phosphorus()`, so the result depends on the number of sub-steps and the units are wrong. Runs when `flag_adv_P >= 1`. Function: `p_adsorption_calc()`.

**Current code** (`atecology/additionalTracer.c` lines 1309–1338)

```c
 1309:     double Dust = tracerArray[Dust_i];
 1310: 
 1311:     if(habitatType != SED) {
 1312:         porosity = 1.0;
 1313:     } else {
 1314:         porosity = bm->boxes[bm->current_box].sm.porosity[bm->current_layer];
 1315:     }
 1316:         
 1317:     EFI = max(bm->min_pool, (EFI - Dust));
 1318:     Dust = max(bm->min_pool, Dust);
 1319: 
 1320:     net_desorp = Pads_r * (PIP / (EFI * bm->Pads_K) -  DIP * (Oxygen / (bm->Pads_KO + Oxygen)));
 1321:     PIP_immob = r_immob_PIP * PIP;
 1322:     
 1323:     tracerArray[DIP_i] += (net_desorp) / porosity;
 1324:     tracerArray[PIP_i] -= (net_desorp + PIP_immob);
 1325:     tracerArray[PIPI_i] += PIP_immob;
 1326: 
 1327:     net_desorp_Dust = Pads_r * ( PIP_Dust / (Dust * bm->Pads_K) - DIP * (Oxygen / (bm->Pads_KO + Oxygen)));
 1328:     PIP_immob_Dust = r_immob_PIP * PIP_Dust;
 1329:     
 1330:     tracerArray[DIP_i] += (net_desorp_Dust) / porosity;
 1331:     tracerArray[PIP_Dust_i] -= (net_desorp_Dust + PIP_immob_Dust);
 1332:     tracerArray[PIPI_i] += PIP_immob_Dust;
 1333:     tracerArray[TIP_i] += tracerArray[PIP_i] + tracerArray[PIPI_i] + tracerArray[PIP_Dust_i];
 1334:     
 1335:     return;
 1336: 
 1337: }
 1338: 
```

**Suggested fix (from review):** Add the terms to the flux array (`localWCFlux`/`localSEDFlux`) instead of the tracers, or multiply by `dtsz`

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

## BIOL-09

**dead code / missing init** · Conf H / Sev M  
**Listed lines:** 1296-1309

**Function:** `p_adsorption_setup()`

**Why it's thought to be a bug**

`p_adsorption_setup()` is never called anywhere in the repo (grep finds only its definition, and it has no prototype). So `Pads_r_i`, `r_immob_PIP_i` and `Tfactor_i` are never set from `bm->Pads_r_t0`/`r_immob_PIP_t0`, and `p_adsorption_calc()` uses whatever value the tracer has in the initial file (usually 0, which turns adsorption off). Function: `p_adsorption_setup()`.

**Current code** (`atecology/additionalTracer.c` lines 1294–1311)

```c
 1294:     tracerArray[r_immob_PIP_i] = bm->r_immob_PIP_t0 * Tfactor;
 1295:     tracerArray[TIP_i] += tracerArray[PIP_i] + tracerArray[PIPI_i] + tracerArray[PIP_Dust_i];
 1296: }
 1297: 
 1298: void p_adsorption_calc(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitatType)
 1299: {
 1300:     double *tracerArray = getTracerArray(boxLayerInfo, habitatType);
 1301:     double porosity, net_desorp, PIP_immob, net_desorp_Dust, PIP_immob_Dust;
 1302:     double Pads_r = tracerArray[Pads_r_i];
 1303:     double r_immob_PIP = tracerArray[r_immob_PIP_i];
 1304:     double Oxygen = tracerArray[Oxygen_i];
 1305:     double DIP = tracerArray[DIP_i];
 1306:     double EFI = tracerArray[EFI_i];
 1307:     double PIP = tracerArray[PIP_i];
 1308:     double PIP_Dust = tracerArray[PIP_Dust_i];
 1309:     double Dust = tracerArray[Dust_i];
 1310: 
 1311:     if(habitatType != SED) {
```

**Suggested fix (from review):** Call it before `p_adsorption_calc()` in `Calculate_Phosphorus()`

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

## BIOL-10

**accumulator** · Conf M / Sev M  
**Listed lines:** 1308, 1346

**Why it's thought to be a bug**

`tracerArray[TIP_i] += PIP + PIPI + PIP_Dust;` — total inorganic P should be the sum (`=`), but it is accumulated on every call/sub-step, so TIP grows without bound. The TIP tracer is registered with flux flag 0 (atPhysChemIO.c:132), so it is not reset each step. Functions: `p_adsorption_setup()`, `p_adsorption_calc()`.

**Current code** (`atecology/additionalTracer.c` lines 1306–1338)

```c
 1306:     double EFI = tracerArray[EFI_i];
 1307:     double PIP = tracerArray[PIP_i];
 1308:     double PIP_Dust = tracerArray[PIP_Dust_i];
 1309:     double Dust = tracerArray[Dust_i];
 1310: 
 1311:     if(habitatType != SED) {
 1312:         porosity = 1.0;
 1313:     } else {
 1314:         porosity = bm->boxes[bm->current_box].sm.porosity[bm->current_layer];
 1315:     }
 1316:         
 1317:     EFI = max(bm->min_pool, (EFI - Dust));
 1318:     Dust = max(bm->min_pool, Dust);
 1319: 
 1320:     net_desorp = Pads_r * (PIP / (EFI * bm->Pads_K) -  DIP * (Oxygen / (bm->Pads_KO + Oxygen)));
 1321:     PIP_immob = r_immob_PIP * PIP;
 1322:     
 1323:     tracerArray[DIP_i] += (net_desorp) / porosity;
 1324:     tracerArray[PIP_i] -= (net_desorp + PIP_immob);
 1325:     tracerArray[PIPI_i] += PIP_immob;
 1326: 
 1327:     net_desorp_Dust = Pads_r * ( PIP_Dust / (Dust * bm->Pads_K) - DIP * (Oxygen / (bm->Pads_KO + Oxygen)));
 1328:     PIP_immob_Dust = r_immob_PIP * PIP_Dust;
 1329:     
 1330:     tracerArray[DIP_i] += (net_desorp_Dust) / porosity;
 1331:     tracerArray[PIP_Dust_i] -= (net_desorp_Dust + PIP_immob_Dust);
 1332:     tracerArray[PIPI_i] += PIP_immob_Dust;
 1333:     tracerArray[TIP_i] += tracerArray[PIP_i] + tracerArray[PIPI_i] + tracerArray[PIP_Dust_i];
 1334:     
 1335:     return;
 1336: 
 1337: }
 1338: 
```

**Suggested fix (from review):** `tracerArray[TIP_i] = ...`

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

## BIOL-11

**debug output left live** · Conf H / Sev M  
**Listed lines:** 457

**Function:** `Calculate_Element_Flux()`

**Why it's thought to be a bug**

An unconditional `fprintf(bm->logFile, "%s-%d Box %d-%d Density: ...")` runs for every age-structured cohort, every atomic tracer, every layer and every adaptive sub-step whenever `track_atomic_ratio` is on. It can create very large log files, the same problem as the printf just removed from atContaminants. Function: `Calculate_Element_Flux()`.

**Current code** (`atecology/additionalTracer.c` lines 455–459)

```c
  455: 								elementflux = ((double)FunctGroupArray[sp].ratioGainedPred[cohort][tracerIndex]/(Density + small_num)) - (double)FunctGroupArray[sp].ratioLost[habitat][cohort][tracerIndex];
  456: 
  457:                                 fprintf(bm->logFile, "%s-%d Box %d-%d Density: %e Nums: %e cell_vol: %e ratioGainedPred: %e ratioLost: %e habitat: %d\n", FunctGroupArray[sp].groupCode, cohort, bm->current_box, bm->current_layer, Density, tracerArray[FunctGroupArray[sp].NumsTracers[cohort]], bm->cell_vol, ((double)FunctGroupArray[sp].ratioGainedPred[cohort][tracerIndex]), ((double)FunctGroupArray[sp].ratioLost[habitat][cohort][tracerIndex]), habitat);
  458:                                     
  459: 								break;
```

**Suggested fix (from review):** Delete it or guard with a debug flag

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

## BIOL-12

**debug output left live** · Conf H / Sev M  
**Listed lines:** 655

**Function:** `Integrate_Ratio_Variables()`

**Why it's thought to be a bug**

An unconditional `fprintf(bm->logFile, "Group %s Box %d layer %d newN: ...")` runs for every primary-producer cohort/element on every sub-step. Function: `Integrate_Ratio_Variables()`.

**Current code** (`atecology/additionalTracer.c` lines 653–657)

```c
  653:                                 }
  654: 
  655: 								fprintf(bm->logFile, "Group %s Box %d layer %d newN: %e oldB: %e newValue: %e it_count: %d ratio: %e B: %e habitat: %d timeValue: %e oldRatio: %e Bflux: %e oldN: %e elementValue: %e\n", FunctGroupArray[sp].groupCode, bm->current_box, bm->current_layer, newN, oldB, newValue, it_count, ratio, B, habitat, timeValue, oldRatio, Bflux, FunctGroupArray[sp].addRatioFluxes[cohort][num_atomic_id], elementValue);
  656: 
  657: 
```

**Suggested fix (from review):** Delete it or guard with a debug flag

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

## BIOL-13

**debug output left live** · Conf H / Sev M  
**Listed lines:** 1111-1118

**Function:** `Gain_Element_Predation()`

**Why it's thought to be a bug**

`/**/ if(amount > 0 && toGuild == 0){ fprintf(bm->logFile, ...) } /**/` — the empty `/**/` markers leave this debug print active. It logs every predation transfer into group index 0. Function: `Gain_Element_Predation()`.

**Current code** (`atecology/additionalTracer.c` lines 1109–1120)

```c
 1109: 		FunctGroupArray[toGuild].ratioGainedPred[toCohort][tracerIndex] += pLost;
 1110: 
 1111: 		/**/
 1112: 		//if(amount > 0 && tracerIndex == c_id && toGuild == 5 && toCohort == 1 && bm->current_box == 15 && bm->current_layer == 3){
 1113:         if(amount > 0 && toGuild == 0){
 1114: 			fprintf(bm->logFile, "habitatType = %d, after %s, amount = %e, ratio = %e, FunctGroupArray[toGuild].ratioGainedPred[toCohort][tracerIndex] = %Le\n",
 1115: 					habitatType, FunctGroupArray[fromGuild].groupCode, amount, pRatio, FunctGroupArray[toGuild].ratioGainedPred[toCohort][tracerIndex]);
 1116: 
 1117: 		}
 1118:         /**/
 1119: 	}
 1120: }
```

**Suggested fix (from review):** Comment it out properly or delete it

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

## BIOL-14

**uninitialised variable / stdout spam** · Conf M / Sev L  
**Listed lines:** 755-759

**Function:** `Integrate_Ratio_Variables()`

**Why it's thought to be a bug**

Inside the `flagratio_warn` block, the `/**/ ... /**/` markers leave a `printf` to stdout plus an `fprintf` live on every ratio update. The fprintf prints `newN`, which is only set in the primary-producer branch (l.643), so it can be uninitialised for non-PP groups. Function: `Integrate_Ratio_Variables()`.

**Current code** (`atecology/additionalTracer.c` lines 753–761)

```c
  753: 
  754: 
  755:                                         /**/
  756:                                         //if(fabs(ratio - oldRatio) > buffer_rounding){
  757:                                             printf("Group %s-%d Box %d layer %d oldB: %e newValue: %e elementValue: %e elementflux: %e it_count: %d ratio: %e, B: %e, Bflux: %e habitat= %d\n", FunctGroupArray[sp].groupCode, cohort, bm->current_box, bm->current_layer, oldB, newValue, elementValue, elementflux, it_count, ratio, B, Bflux, habitat);
  758: 
  759:                                             fprintf(bm->logFile, "Group %s-%d Box %d layer %d newN: %e oldB: %e newValue: %e it_count: %d ratio: %e B: %e habitat: %d timeValue: %e oldRatio: %e Bflux: %e oldN: %e elementValue: %e elementflux: %e\n", FunctGroupArray[sp].groupCode, cohort,  bm->current_box, bm->current_layer, newN, oldB, newValue, it_count, ratio, B, habitat, timeValue, oldRatio, Bflux, FunctGroupArray[sp].addRatioFluxes[cohort][num_atomic_id], elementValue, elementflux);
  760: 
  761:                                             //quit("\n");
```

**Suggested fix (from review):** Remove the prints or initialise `newN = 0`

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

## BIOL-15

**cohort vs cohort×genotype** · Conf M / Sev M  
**Listed lines:** 450, 624

**Why it's thought to be a bug**

Both ratio loops use `cohort < FunctGroupArray[sp].numCohorts`. `addRatioTracers`/`ratioLost`/`ratioGainedPred` are sized `numCohortsXnumGenes` (atUtilGroupIO.c:891-896) and cohorts are gene-interleaved (`numGeneTypes*bcohort+ngene`). With >1 genotype, half of the ratio tracers are never integrated, and the cohorts that are covered are the wrong ones. `ratioGained`/`ratioLostPred` are sized only `numCohorts` (l.901,904), so the sizes are inconsistent too. Functions: `Calculate_Element_Flux()`, `Integrate_Ratio_Variables()`.

**Current code** (`atecology/additionalTracer.c` lines 448–452)

```c
  448: 
  449: 
  450: 						for (cohort = 0; cohort < FunctGroupArray[sp].numCohorts; cohort++) {
  451: 
  452: 							switch(FunctGroupArray[sp].groupAgeType){
```

**Suggested fix (from review):** Loop over `numCohortsXnumGenes` and size `ratioGained`/`ratioLostPred` to match

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

additionalTracer.c l.1224/1226: `Vertebrates_Transfer_To_Pred()` uses the global `GRAZEinfo`, while every other branch uses the `spGRAZEinfo` parameter. They are currently the same pointer (atecology.c:819), so this is harmless but fragile.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

additionalTracer.c l.1151/1229: `Gain_Element_Predation(..., prey, 0, amount)` always takes the prey ratio from cohort 0, even while looping over `kij`.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
