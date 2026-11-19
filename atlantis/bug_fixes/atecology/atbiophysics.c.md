# atecology/atbiophysics.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 2 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [NUTR-17](#nutr-17) | 560, 578-660 | M/M | stale globals | `H2Otemp`, `current_SALT` and `current_PH` are only assigned inside `if (!checked_already)`. When the box/laye… |  | [ ] |
| [NUTR-18](#nutr-18) | 626 | M/M | wrong variable | The depth correction uses the global `cell_depth` instead of the `cdepth` argument (which line 567 uses). Call… |  | [ ] |
| [NUTR-19](#nutr-19) | 161-172 | M/L | unit / double count | `intensity = 1.353 * (...)` is already a solar flux in kW m-2, because 1.353 is the solar constant. It is then… |  | [ ] |
| [NUTR-20](#nutr-20) | 764 | H/L | log flooding | An unconditional `fprintf(llogfp, ...)` runs on every pH_At_Depth call in the regression branch (every box/lay… |  | [ ] |

---

## NUTR-17

**stale globals** · Conf M / Sev M  
**Lines (current code):** 560, 578-660  
**Lines at review (Oct 2026):** 560, 578-660

**Function:** `Properties_At_Depth()`

**Why it's thought to be a bug**

`H2Otemp`, `current_SALT` and `current_PH` are only assigned inside `if (!checked_already)`. When the box/layer has already been checked since the last reset of `checkedalready` (attime.c:84 daily, attempsalt.c on forcing), these globals keep whatever box/layer was computed last. Parameter_Q10 (atq10.c:126-133) then computes Tcorr from that stale value. This hits repeat calls such as atmovement.c:904/3029 (box 1) and atexternalpop.c:42, and every timestep within a day when no forcing reset happens. Function: `Properties_At_Depth()`.

**Current code** (`atecology/atbiophysics.c` lines 558–597 (truncated))

```c
  558: 
  559: 
  560: 	checked_already = bm->checkedalready[pBox->n][clayer + flagmodel * bm->wcnz];
  561:     
  562: 	/* Regression and southern hemisphere */
  563: 	if ((bm->flagmodeltemp == 1) && (!bm->flaghemisphere)) {
  564: 		/* Determine sea surface temperature using linear regression
  565: 		 equations determined from CARS data. Assumes midpoint is
  566: 		 the value in metres of the latitude (-ve metres as -ve latitude) */
  567: 		if (cdepth < -250.0) {
  568: 			basetemp = 0.000005 * midpoint + 38.158;
  569: 		} else {
  570: 			basetemp = 0.000006 * midpoint + 38.365;
  571: 		}
  572: 	} else if (bm->flagmodeltemp == 2) {
  573: 		basetemp = bm->baseline_temp;
  574: 	} else {
  575: 		basetemp = Y1[Temp_i];
  576: 	}
  577: 
  578: 	if (!checked_already) { /* Only do calculations if haven't already */
  579: 
  580: 		/* if this is the sediment use the Tchange value in the first water column layer */
  581: 		if(flagmodel == SED){
  582: 			T_scale = bm->TempLayerScalar[0];
  583: 		}else{
  584: 			T_scale = bm->TempLayerScalar[clayer];
  585: 		}
  586: 
  587: 		//fprintf(bm->logFile, "Time %e, box %d, layer %d, T_scale = %e\n", bm->dayt, pBox->n, clayer, T_scale);
  588: 
  589: 		/* If appropriate use a regression to define the seasonal signal
  590: 		 Note that would normally convert degrees to radians as 2pi/360, but as got 365 days
  591: 		 in a revolution (one turn through the calendar) then divide by 365 instead. Also offset
  592: 		 the peak value by 31 days as don't peak summer heat in the water column until after mid summer
  593: 		 (so in February not January).
  594: 		 */
  595: 		if (bm->flagseason && bm->flagmodeltemp > 0) {
  596: 			if (!bm->flaghemisphere) // Southern hemisphere
  597: 				surfH2Otemp = (bm->temp_amplitude * cos(2.0 * 3.1415926 * (dayt - 31.0) / 365.0) + basetemp);
```

**Suggested fix (from review):** In the `else` branch reload `H2Otemp = Y1[Temp_i]; current_SALT = Y1[Salinity_i]; current_PH = track_pH ? Y1[pH_i] : 8.0;`.

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

## NUTR-18

**wrong variable** · Conf M / Sev M  
**Lines (current code):** 626  
**Lines at review (Oct 2026):** 626

**Function:** `Properties_At_Depth()`

**Why it's thought to be a bug**

The depth correction uses the global `cell_depth` instead of the `cdepth` argument (which line 567 uses). Callers passing an explicit depth (atmovement.c:904 passes 0, atexternalpop.c:42 passes 0) get a correction based on whatever `cell_depth` was left by the last atbiology loop. pH_At_Depth (705) has the same problem. Function: `Properties_At_Depth()`, `pH_At_Depth()`.

**Current code** (`atecology/atbiophysics.c` lines 624–628)

```c
  624: 			 to data from Bax and Williams 2000 SE ecosystem report and Parlsow et al 1996
  625: 			 Tasmanian slope trophodynamics temperature-at-depth data (r-squared = 0.98) */
  626: 			deepH2Otemp = surfH2Otemp * exp(0.0011 * cell_depth);
  627: 
  628: 			H2Otemp = deepH2Otemp;
```

**Suggested fix (from review):** Use `cdepth` (and make callers pass the correct depth).

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

## NUTR-19

**unit / double count** · Conf M / Sev L  
**Lines (current code):** 161-172  
**Lines at review (Oct 2026):** 161-172

**Function:** `Calculate_Box_Light()`

**Why it's thought to be a bug**

`intensity = 1.353 * (...)` is already a solar flux in kW m-2, because 1.353 is the solar constant. It is then multiplied by `Io`, which also contains the solar constant (`swr_const`, ~1367), so SWR is too high by about 1.35x. Also at elevation 0 (night) AM ≈ 38 still gives a nonzero intensity. Function: `Calculate_Box_Light()`.

**Current code** (`atecology/atbiophysics.c` lines 159–174)

```c
  159: 	/* Intensity - currently assumed to be at sea level */
  160: 	double height = 0.0;   // Height above sea level TODO: Actually have this as real height eventually
  161: 	double AM = 1.0 / (cos(zenith * deg2rad) + 0.50572 * pow((96.07995 - zenith),(-1.6364)));  // Air mass including curvature of the earth
  162: 	double step0 = pow(AM,0.678);
  163: 	double step1 = pow(0.7,step0);
  164: 	double step2 = 0.14 * height;
  165: 	double intensity = 1.353 * ((1.0 - step2) * step1 + step2); // Sunlight intensity at sea level
  166: 
  167: 	/* Exterterrestrial shortwave radiation levels */
  168: 	//double Io = 1367.0 * (1.0 + 0.034 * cos((360.0 * dn / 365.0) * deg2rad)) * bm->swr_scalar;
  169:     double Io = bm->swr_const * (1.0 + bm->swr_cos_coefft * cos((360.0 * dn / 365.0 - bm->swr_cos_offset) * deg2rad)) * bm->swr_scalar;
  170: 
  171: 	/* Available irradiance (PAR) - level available from Griffiths et al ice algal model */
  172: 	pBox->swr = intensity * Io;
  173: 
  174: 	if (verbose > 0)
```

**Suggested fix (from review):** Use `(intensity / 1.353) * Io` (or drop Io's constant).

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

## NUTR-20

**log flooding** · Conf H / Sev L  
**Lines (current code):** 764  
**Lines at review (Oct 2026):** 764

**Function:** `pH_At_Depth()`

**Why it's thought to be a bug**

An unconditional `fprintf(llogfp, ...)` runs on every pH_At_Depth call in the regression branch (every box/layer/timestep), which produces very large log files. Function: `pH_At_Depth()`.

**Current code** (`atecology/atbiophysics.c` lines 762–766)

```c
  762: 
  763: 		ans_pH = ans_pH * pH_scale;
  764: 		fprintf(llogfp, "Time: %e box%d-%d pH: %e CO2: %e\n", bm->dayt, bm->current_box, bm->current_layer, ans_pH, read_pCO2);
  765: 
  766: 	} else if (bm->flagmodelpH == ph_data) {
```

**Suggested fix (from review):** Guard with debug flag or remove.

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

atbiophysics.c `Box_Light_Process()` (319-320, 387-402): `midwater_dz` is half the *top* layer thickness, not half the water depth. The "surface light" branches use `lighttop`, which at that point equals bottom-of-column light, not `lightsurf`. Both disagree with the comments.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atbiophysics.c `Get_Enviro_Scalar()` (857-860): if trailing Xchange entries are zero-filled up to Xchange_max_num, `timeNow >= 0` makes `now_change` the last (empty) entry.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
