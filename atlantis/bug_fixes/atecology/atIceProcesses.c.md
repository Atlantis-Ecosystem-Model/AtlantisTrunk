# atecology/atIceProcesses.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **6 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 2 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [EINIT-13](#einit-13) | 129 | M/M | sign / physical constant | `Y1[Temp_i] = 1.8;` sets every non-slush ice layer to +1.8 °C. Sea ice sits at or below the seawater freezing … |  | [ ] |
| [EINIT-14](#einit-14) | 181, 172, 264 | M/M | wrong loop direction | Ice layer 0 is the surface layer (atvertgeom.c:571-575 "Shrink into layer closer to the surface" k to k-1, ins… |  | [ ] |
| [EINIT-15](#einit-15) | 296 | H/M | uninitialised / stale value | `local_current_corr` is initialised to 1.0 and never derived from `current_temp`. For `base_q10_id` groups, `G… |  | [ ] |
| [EINIT-16](#einit-16) | 311 | M/M | wrong index (ice vs water layer) | `Get_Pollutant_Corrections(bm, sp, pBox->n, ice_layer)` passes an ice-layer index, and the callee reads `bm->b… |  | [ ] |
| [EINIT-17](#einit-17) | 605 | H/M | copy-paste / wrong group index | `int flag_sp = (int) (FunctGroupArray[pelagicBactIndex].speciesParams[flag_id]);` was copied from the pelagic … |  | [ ] |
| [EINIT-18](#einit-18) | 730 | H/M | swapped variables / unreachable branch | `BotDpth` = sum of dz above the current layer (i.e. the depth of its top), and `TopDpth = BotDpth + dz` (the d… |  | [ ] |

---

## EINIT-13

**sign / physical constant** · Conf M / Sev M  
**Found at (current code):** `atIceProcesses.c` 129
**Lines at review (Oct 2026):** 127-130

**Function:** `Box_Ice_Temperature_Related()`

**Why it's thought to be a bug**

`Y1[Temp_i] = 1.8;` sets every non-slush ice layer to +1.8 °C. Sea ice sits at or below the seawater freezing point (about -1.8 °C), and the slush layer is set to 0.0, which makes the solid ice warmer than the slush. This temperature drives `Get_Tcorr` for ice species in `Box_Ice_Q10`. Function: `Box_Ice_Temperature_Related()`.

**Code at issue** — `atIceProcesses.c` line 129  
Search for: `Y1[Temp_i] = 1.8`

```c
  127: 	for(i = 0; i < slush_layer; i++){
  128: 		Y1 = (double*) pBox->ice.tr[i];
  129→ 		Y1[Temp_i] = 1.8;
  130: 	}
  131: 	/* And the temperature in the slush layer */
```

**Suggested fix (from review):** `Y1[Temp_i] = -1.8;`

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

## EINIT-14

**wrong loop direction** · Conf M / Sev M  
**Found at (current code):** `atIceProcesses.c` 181, `atIceProcesses.c` 172, `atIceProcesses.c` 264
**Lines at review (Oct 2026):** 172-264

**Function:** `Box_Ice_Light_Level()`

**Why it's thought to be a bug**

Ice layer 0 is the surface layer (atvertgeom.c:571-575 "Shrink into layer closer to the surface" k to k-1, insert_ice_layer "Move deeper layers down"). `currentnz-1` is the bottom slush layer next to the water (used as the bottom in Box_Ice_Flux:39/57 and as slush in Box_Ice_Temperature_Related). The light loop `for(i = currentnz-1; i >= 0; i--)` starts with `lighttop = pBox->swr` at the bottom layer and attenuates upward, and `ice->bottom_light` ends up as the light below the top layer. Function: `Box_Ice_Light_Level()`.

**Code at issue** — `atIceProcesses.c` line 181  
Search for: `currentnz - 1`

```c
  179: 	/* Calculate irradiance in each layer */
  180: 	//icedepth = 0.0;
  181→ 	for(i = ice->currentnz - 1; i >= 0 ; i--){
  182: 		Y1 = (double*) pBox->ice.tr[i];
  183: 
```

**Code at issue** — `atIceProcesses.c` line 172  
Search for: `lighttop = pBox->swr`

```c
  170: 
  171: 	/* Calculate light at the top of the ice */
  172→ 	lighttop = pBox->swr;
  173: 
  174: 	ice_cover = pBox->tot_ice_prop;  // In case no layers below?
```

**Code at issue** — `atIceProcesses.c` line 264  
Search for: `ice->bottom_light`

```c
  262: 	}
  263: 
  264→ 	ice->bottom_light = lightbot;
  265: }
  266: 
```

**Suggested fix (from review):** Loop `for (i = 0; i < ice->currentnz; i++)`

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

## EINIT-15

**uninitialised / stale value** · Conf H / Sev M  
**Found at (current code):** `atIceProcesses.c` 296
**Lines at review (Oct 2026):** 285, 295

**Function:** `Box_Ice_Q10()`

**Why it's thought to be a bug**

`local_current_corr` is initialised to 1.0 and never derived from `current_temp`. For `base_q10_id` groups, `Get_Tcorr` returns `pow(q10, 1.0) = q10`, a constant regardless of ice temperature. After any humped-Griffith group, `Get_Tcorr` overwrites it with `T - Topt` (atq10.c:204) and later species reuse that value. It also drives the sign tests at 341/346. Function: `Box_Ice_Q10()`.

**Code at issue** — `atIceProcesses.c` line 296 (located by name only; check this is the right place)  
Search for: `FunctGroupArray[sp].Tcorr = Get_Tcorr(bm, sp, current_temp, &FunctGroupArray[sp].current_c`

```c
  294: 		if ((FunctGroupArray[sp].habitatType == ICE_BASED) && (FunctGroupArray[sp].speciesParams[flag_id] == TRUE)){
  295:             FunctGroupArray[sp].current_corr = local_current_corr;
  296→ 			FunctGroupArray[sp].Tcorr = Get_Tcorr(bm, sp, current_temp, &FunctGroupArray[sp].current_corr);
  297: 
  298: 			/* Include pH and salinity modifiers if desired
```

**Suggested fix (from review):** Inside the sp loop set `local_current_corr = (current_temp - bm->baseline_temp)/10.0;` before calling Get_Tcorr

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

## EINIT-16

**wrong index (ice vs water layer)** · Conf M / Sev M  
**Found at (current code):** `atIceProcesses.c` 311
**Lines at review (Oct 2026):** 310

**Function:** `Box_Ice_Q10()`

**Why it's thought to be a bug**

`Get_Pollutant_Corrections(bm, sp, pBox->n, ice_layer)` passes an ice-layer index, and the callee reads `bm->boxes[b].tr[clayer][Noise_Pollution_i]` (atq10.c:420), the water-column tracer array. Ice layer 0 is the top, but water layer 0 is the deepest layer. With fast ice (`nz == 0`, atvertgeom.c:612) this indexes past the water-column array. Function: `Box_Ice_Q10()`.

**Code at issue** — `atIceProcesses.c` line 311  
Search for: `Get_Pollutant_Corrections(bm, sp, pBox->n, ice_layer)`

```c
  309: 			FunctGroupArray[sp].pHcorr = 1.0;
  310: 
  311→             FunctGroupArray[sp].PolluteCorr = Get_Pollutant_Corrections(bm, sp, pBox->n, ice_layer);
  312:             
  313: 			/* Correct the rate parameters for primary producers */
```

**Suggested fix (from review):** Pass `ice->wc_layer_match[ice_layer]` (and guard nz>0), or use the ice tracers

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

## EINIT-17

**copy-paste / wrong group index** · Conf H / Sev M  
**Found at (current code):** `atIceProcesses.c` 605
**Lines at review (Oct 2026):** 604

**Function:** `Calculate_IceBact_Scale()`

**Why it's thought to be a bug**

`int flag_sp = (int) (FunctGroupArray[pelagicBactIndex].speciesParams[flag_id]);` was copied from the pelagic version (atprocess.c:2319). The sediment version correctly uses `SedBactIndex` (atprocess.c:2340). Ice bacteria partitioning is therefore switched on or off by the pelagic bacteria flag: if pelagic bacteria are off, ICEB_DL/DR are 0 even when ice bacteria are active. The header comment ("Sediment Bacteria partitioning") is also copy-pasted. Function: `Calculate_IceBact_Scale()`.

**Code at issue** — `atIceProcesses.c` line 605  
Search for: `int flag_sp = (int) (FunctGroupArray[pelagicBactIndex].speciesParams[flag_id])`

```c
  603: void Calculate_IceBact_Scale(MSEBoxModel *bm, HABITAT_TYPES habitatType, BoxLayerValues *boxLayerInfo) {
  604: 	double PB, DR, DL;
  605→ 	int flag_sp = (int) (FunctGroupArray[pelagicBactIndex].speciesParams[flag_id]);
  606: 
  607: 	DL = boxLayerInfo->localICETracers[FunctGroupArray[LabDetIndex].totNTracers[0]];
```

**Suggested fix (from review):** Use `IceBactIndex`

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

## EINIT-18

**swapped variables / unreachable branch** · Conf H / Sev M  
**Found at (current code):** `atIceProcesses.c` 730
**Lines at review (Oct 2026):** 720-742

**Function:** `Ice_Avail()`

**Why it's thought to be a bug**

`BotDpth` = sum of dz above the current layer (i.e. the depth of its top), and `TopDpth = BotDpth + dz` (the depth of its bottom). These are the reverse of the sediment `Avail()` in atprocess.c:2004-2005. As a result `if (BotDpth <= Depth)` gives full availability to any layer that merely starts above Depth, and the partial branch `else if (TopDpth < Depth)` can never be reached (TopDpth > BotDpth). `PercentDpth` is computed from the wrong boundary. Function: `Ice_Avail()`.

**Code at issue** — `atIceProcesses.c` line 730 (located by name only; check this is the right place)  
Search for: `PercentDpth = (Depth - NearDpth) / (ice->dz[bm->current_icelayer] + small_num);`

```c
  728: 	FarDpth = NearDpth + ice->dz[bm->current_icelayer];
  729: 
  730→ 	PercentDpth = (Depth - NearDpth) / (ice->dz[bm->current_icelayer] + small_num);
  731: 
  732: 	/* Layer completely within the depth the grazer can reach then all of it is available. */
```

**Suggested fix (from review):** `TopDpth = sum; BotDpth = TopDpth + dz[current_icelayer];`

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

atIceProcesses.c:36,51 `Box_Ice_Flux()`: `k_brine_diff *= 86400.0/bm->dt` is commented "per s to per day", but 86400/dt converts neither per s to per day (×86400) nor per s to per timestep (×dt). Also for `igr <= 0` the `else` branch at line 50 gives a negative diffusivity (only used when freezing, so possibly harmless). At line ≈62 the `wc_layer >= nz` check comes after the `pBox->tr[wc_layer]` pointer is formed (harmless since nothing is dereferenced first).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atIceProcesses.c:202 `Box_Ice_Light_Level()`: `phytoBiomass` is seeded with `k_w_shallow`, a water attenuation coefficient, then multiplied by `ka_star`. This mixes an attenuation coefficient into a biomass sum.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
