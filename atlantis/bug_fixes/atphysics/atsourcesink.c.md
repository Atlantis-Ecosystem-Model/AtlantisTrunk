# atphysics/atsourcesink.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1), plus 2 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY2-07](#phy2-07) ⭐ | 201-225 | H/H | duplicated logic / double application | In the no-water branch, `nut_scale = Get_PSS_Scalar(bm, s, variableIndex);` is followed by a copy of Get_PSS_S… |  | [ ] |
| [PHY2-08](#phy2-08) | 209, 508 | M/M | wrong index | `strcasecmp(pss->ts.varname[variableIndex], "Si")` uses `variableIndex`, which counts only valid non-time, non… |  | [ ] |
| [PHY2-09](#phy2-09) | 706 | H/L | format specifier | `quit("read_bm_ts: Can't open %%s\n", bm->inputFolder, bm->forceIfname)`: the `%%s` prints a literal "%s" and … |  | [ ] |

---

## PHY2-07

**duplicated logic / double application** · Conf H / Sev H ⭐ priority  
**Listed lines:** 201-225

**Function:** `sourceSink()`

**Why it's thought to be a bug**

In the no-water branch, `nut_scale = Get_PSS_Scalar(bm, s, variableIndex);` is followed by a copy of Get_PSS_Scalar's own loop, which multiplies `nut_scale` again by the same `pssMult` factors. The scaling is applied twice (mult² after the change period, and the ramp is squared). The water branch (line 159) only calls Get_PSS_Scalar. The copied block also tests `bm->t == pssStart` (days) without `*86400` (line 218). Function: `sourceSink()`.

**Current code** (`atphysics/atsourcesink.c` lines 199–227)

```c
  199: 					nut_scale = 1.0;
  200: 
  201: 					if (bm->nutrientChange) {
  202:                         nut_scale = Get_PSS_Scalar(bm, s, variableIndex);
  203: 						/* Loop over each possible change */
  204: 						for (index = 0; index < bm->pss[s].numPssChanges; index++) {
  205: 							if (bm->pss[s].pssChange[index][variableIndex] && (bm->t >= (bm->pss[s].pssStart[index][variableIndex] * 86400.0))) {
  206: 								/* Must correct nutrient change period into seconds as that's what bm works in */
  207: 								end_date = (bm->pss[s].pssPeriod[index][variableIndex] + bm->pss[s].pssStart[index][variableIndex]) * 86400.0;
  208: 
  209: 								if ((strcasecmp(pss->ts.varname[variableIndex], "Si") == 0) || (bm->pss[s].pssMult[index][variableIndex] == 1.0)) {
  210: 									nut_scale = nut_scale * 1.0;
  211: 								} else if (end_date < bm->t) {
  212: 									if (bm->pulsechange) {
  213: 										// Nothing to do as simply * 1.0
  214: 									} else {
  215: 										nut_scale = nut_scale * bm->pss[s].pssMult[index][variableIndex];
  216: 									}
  217: 								} else {
  218:                                     if (bm->t == (bm->pss[s].pssStart[index][variableIndex])) {
  219:                                         // Nothing to do as simply * 1.0
  220:                                     } else {
  221:                                         nut_scale = nut_scale * (bm->pss[s].pssMult[index][variableIndex] * ((bm->t - (bm->pss[s].pssStart[index][variableIndex] * 86400.0)) / (bm->pss[s].pssPeriod[index][variableIndex] * 86400.0)));
  222:                                     }
  223: 								}
  224: 							}
  225: 						}
  226: 					}
  227: 					if (verbose > 1) {
```

**Suggested fix (from review):** Delete the loop at lines 203-225 and keep only the `Get_PSS_Scalar` call

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

## PHY2-08

**wrong index** · Conf M / Sev M  
**Listed lines:** 209, 508

**Function:** `sourceSink()`

**Why it's thought to be a bug**

`strcasecmp(pss->ts.varname[variableIndex], "Si")` uses `variableIndex`, which counts only valid non-time, non-water tracer columns. `ts.varname` is indexed by the raw time-series column `i` and includes the time column (sjwlib/timeseries.c:146-156, pointsourcesink.c:165). So the Si exemption checks the wrong column, usually shifted by one because time is column 0, and Si can get scaled while the column before it is exempted. Function: `sourceSink()`, `Get_PSS_Scalar()`.

**Current code** (`atphysics/atsourcesink.c` lines 207–211)

```c
  207: 								end_date = (bm->pss[s].pssPeriod[index][variableIndex] + bm->pss[s].pssStart[index][variableIndex]) * 86400.0;
  208: 
  209: 								if ((strcasecmp(pss->ts.varname[variableIndex], "Si") == 0) || (bm->pss[s].pssMult[index][variableIndex] == 1.0)) {
  210: 									nut_scale = nut_scale * 1.0;
  211: 								} else if (end_date < bm->t) {
```

**Suggested fix (from review):** Pass the ts column index `i` to the Si test (e.g. add a parameter to Get_PSS_Scalar)

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

## PHY2-09

**format specifier** · Conf H / Sev L  
**Listed lines:** 706

**Function:** `read_bm_ts_rewind()`

**Why it's thought to be a bug**

`quit("read_bm_ts: Can't open %%s\n", bm->inputFolder, bm->forceIfname)`: the `%%s` prints a literal "%s" and both arguments are ignored. The function name in the message is also wrong. Function: `read_bm_ts_rewind()`.

**Current code** (`atphysics/atsourcesink.c` lines 704–708)

```c
  704: 	/* Open parameter file */
  705: 	if ((fp = Open_Input_File(bm->inputFolder,bm->forceIfname, "r")) == NULL)
  706: 		quit("read_bm_ts: Can't open %%s\n", bm->inputFolder, bm->forceIfname);
  707: 
  708: 	/* Find time series name in parameter file */
```

**Suggested fix (from review):** `quit("read_bm_ts_rewind: Can't open %s%s\n", ...)`

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

atsourcesink.c `sourceSink()`: a water sink (wvol < 0) with |wvol| >= cell volume is not checked, so the volume can go negative (fw_area_input does check this). `read_bm_ts()` line 655 `ts = NULL;` assigns the local parameter only and has no effect.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atsourcesink.c `ReadStress()` has no callers. If it were used, its `else` branch would overwrite stress values supplied in the initial conditions (`supplied_stress == 1`) with the mud relationship.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
