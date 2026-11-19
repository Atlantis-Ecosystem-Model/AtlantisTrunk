# atecology/atbiophysics.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 2 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [NUTR-17](#nutr-17) | 578 | M/M | stale globals | `H2Otemp`, `current_SALT` and `current_PH` are only assigned inside `if (!checked_already)`. When the box/laye… |  | [ ] |
| [NUTR-18](#nutr-18) | 647 | M/M | wrong variable | The depth correction uses the global `cell_depth` instead of the `cdepth` argument (which line 567 uses). Call… |  | [ ] |
| [NUTR-19](#nutr-19) | 165 | M/L | unit / double count | `intensity = 1.353 * (...)` is already a solar flux in kW m-2, because 1.353 is the solar constant. It is then… |  | [ ] |
| [NUTR-20](#nutr-20) | 764 | H/L | log flooding | An unconditional `fprintf(llogfp, ...)` runs on every pH_At_Depth call in the regression branch (every box/lay… |  | [ ] |

---

## NUTR-17

**stale globals** · Conf M / Sev M  
**Found at (current code):** `atbiophysics.c` 578
**Lines at review (Oct 2026):** 560, 578-660

**Function:** `Properties_At_Depth()`

**Why it's thought to be a bug**

`H2Otemp`, `current_SALT` and `current_PH` are only assigned inside `if (!checked_already)`. When the box/layer has already been checked since the last reset of `checkedalready` (attime.c:84 daily, attempsalt.c on forcing), these globals keep whatever box/layer was computed last. Parameter_Q10 (atq10.c:126-133) then computes Tcorr from that stale value. This hits repeat calls such as atmovement.c:904/3029 (box 1) and atexternalpop.c:42, and every timestep within a day when no forcing reset happens. Function: `Properties_At_Depth()`.

**Code at issue** — `atbiophysics.c` line 578  
Search for: `if (!checked_already)`

```c
  576: 	}
  577: 
  578→ 	if (!checked_already) { /* Only do calculations if haven't already */
  579: 
  580: 		/* if this is the sediment use the Tchange value in the first water column layer */
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
**Found at (current code):** `atbiophysics.c` 647
**Lines at review (Oct 2026):** 626

**Function:** `Properties_At_Depth()`

**Why it's thought to be a bug**

The depth correction uses the global `cell_depth` instead of the `cdepth` argument (which line 567 uses). Callers passing an explicit depth (atmovement.c:904 passes 0, atexternalpop.c:42 passes 0) get a correction based on whatever `cell_depth` was left by the last atbiology loop. pH_At_Depth (705) has the same problem. Function: `Properties_At_Depth()`, `pH_At_Depth()`.

**Code at issue** — `atbiophysics.c` line 647 (located by name only; check this is the right place)  
Search for: `base_pH = pH_At_Depth(bm, pBox, dayt, numwclayer, cdepth, clayer, midpoint, flagmodel, llo`

```c
  645:         if (bm->track_pH) {
  646:             /* Get pH values  - if required values are scaled in this function*/
  647→             base_pH = pH_At_Depth(bm, pBox, dayt, numwclayer, cdepth, clayer, midpoint, flagmodel, llogfp);
  648: 
  649:             Y1[pH_i] = base_pH;
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
**Found at (current code):** `atbiophysics.c` 165
**Lines at review (Oct 2026):** 161-172

**Function:** `Calculate_Box_Light()`

**Why it's thought to be a bug**

`intensity = 1.353 * (...)` is already a solar flux in kW m-2, because 1.353 is the solar constant. It is then multiplied by `Io`, which also contains the solar constant (`swr_const`, ~1367), so SWR is too high by about 1.35x. Also at elevation 0 (night) AM ≈ 38 still gives a nonzero intensity. Function: `Calculate_Box_Light()`.

**Code at issue** — `atbiophysics.c` line 165  
Search for: `intensity = 1.353 * (`

```c
  163: 	double step1 = pow(0.7,step0);
  164: 	double step2 = 0.14 * height;
  165→ 	double intensity = 1.353 * ((1.0 - step2) * step1 + step2); // Sunlight intensity at sea level
  166: 
  167: 	/* Exterterrestrial shortwave radiation levels */
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
**Found at (current code):** `atbiophysics.c` 764
**Lines at review (Oct 2026):** 764

**Function:** `pH_At_Depth()`

**Why it's thought to be a bug**

An unconditional `fprintf(llogfp, ...)` runs on every pH_At_Depth call in the regression branch (every box/layer/timestep), which produces very large log files. Function: `pH_At_Depth()`.

**Code at issue** — `atbiophysics.c` line 764  
Search for: `fprintf(llogfp,`

```c
  762: 
  763: 		ans_pH = ans_pH * pH_scale;
  764→ 		fprintf(llogfp, "Time: %e box%d-%d pH: %e CO2: %e\n", bm->dayt, bm->current_box, bm->current_layer, ans_pH, read_pCO2);
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
