# atharvest/atHarvestCatch.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1), plus 2 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [HARV-11](#harv-11) ⭐ | 232, 226 | H/H | wrong parameter id | `Get_Fishery_Group_Change_Scale(bm, nf, sp, bm->flagchangeF, mFC_num_changes_id, mFCchange[sp])` passes the gl… |  | [ ] |
| [HARV-12](#harv-12) | 706 | M/L | copy-paste | The "baseline" selectivity `sel_temp` should be computed without the change offsets. For `q_bimodal_id`, line … |  | [ ] |
| [HARV-13](#harv-13) | 835 | M/M | cohort vs cohort x genotype | `Ecology_Get_Size(bm, species, Wgt, basechrt)` passes the age class. `Ecology_Get_Size` indexes `sn[]`, `X_RS[… |  | [ ] |

---

## HARV-11

**wrong parameter id** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atHarvestCatch.c` 232, `atHarvestCatch.c` 226
**Lines at review (Oct 2026):** 232

**Function:** `Get_Fishing_Mortality()`

**Why it's thought to be a bug**

`Get_Fishery_Group_Change_Scale(bm, nf, sp, bm->flagchangeF, mFC_num_changes_id, mFCchange[sp])` passes the global flag value (1) as the parameter index. It reads `SP_FISHERYprms[sp][nf][1]`, which is `flagQchange_id` (enum in atlantisboxmodel.h:950-952). So forced-F (mFC) change schedules are only applied when the Q-change flag is set for that group/fleet, and the per-group `flagFchange_id` (read at atHarvestParamIO.c:597) is ignored. Function: `Get_Fishing_Mortality()`.

**Code at issue** — `atHarvestCatch.c` line 232  
Search for: `Get_Fishery_Group_Change_Scale(bm, nf,sp, bm->flagchangeF, mFC_num_changes_id, mFCchange[sp])`

```c
  230: 	/* Get scenario scalars */
  231: 	if (bm->flagchangeF){
  232→ 		mFC_scale = Get_Fishery_Group_Change_Scale(bm, nf,sp, bm->flagchangeF, mFC_num_changes_id, mFCchange[sp]);
  233: 	}else{
  234: 		mFC_scale = 1.0;
```

**Code at issue** — `atHarvestCatch.c` line 226 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->dayt, FisheryArray[nf].fisheryCode, FunctGroupArray[sp].groupCode, bm->SP_FISHERYprms[`

```c
  224:     if(do_debug){
  225: 		fprintf(llogfp,"Get_Fishing_Mortality - Time: %e %s %s origmFC: %e, mFC_scale: %e, mFC: %e)\n",
  226→ 			bm->dayt, FisheryArray[nf].fisheryCode, FunctGroupArray[sp].groupCode, bm->SP_FISHERYprms[sp][nf][mFC_id] / 86400.0, bm->SP_FISHERYprms[sp][nf][mFC_scale_id], mFC);
  227: 	}
  228:     */
```

**Suggested fix (from review):** Pass `flagFchange_id` as the paramID.

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

## HARV-12

**copy-paste** · Conf M / Sev L  
**Found at (current code):** `atHarvestCatch.c` 706
**Lines at review (Oct 2026):** 709-718, 732-743

**Function:** `Get_addlsm_Scale()`

**Why it's thought to be a bug**

The "baseline" selectivity `sel_temp` should be computed without the change offsets. For `q_bimodal_id`, line 714 adds `addlsm` to `sel_bilsm2` for the baseline second mode. For `q_binormal_id`, lines 737-738 add both `addlsm` and `addsigma` to the baseline. So `sel - sel_temp` understates the change. Only affects invertebrates using bimodal curves with selectivity changes. Function: `Get_addlsm_Scale()`.

**Code at issue** — `atHarvestCatch.c` line 706  
Search for: `sel - sel_temp`

```c
  704: 				step2 = (sqrt(step1) - sel_lsm) / 2.0 + small_num;
  705: 				sel = pow((li / sel_lsm), (sel_lsm / step2)) * exp((sel_lsm - li) / step2);
  706→ 				returnValue = sel - sel_temp;
  707: 				break;
  708: 			case q_bimodal_id: /* Bimodal assymetric "normal" selectivity curve based on size */
```

**Suggested fix (from review):** Drop `+ addlsm` / `+ addsigma` in the baseline (`sel_temp2`) lines.

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

## HARV-13

**cohort vs cohort x genotype** · Conf M / Sev M  
**Found at (current code):** `atHarvestCatch.c` 835
**Lines at review (Oct 2026):** 829, 835

**Function:** `Harvest_How_Much_Fishery_Access()`

**Why it's thought to be a bug**

`Ecology_Get_Size(bm, species, Wgt, basechrt)` passes the age class. `Ecology_Get_Size` indexes `sn[]`, `X_RS[]` and `VERTinfo[][]` by cohort x genotype (atUtilGroupIO.c:770,842), and the main fishing path passes `chrt` (atHarvest.c:444). With `numGeneTypes > 1`, this gives the wrong genotype's length, so selectivity in the output access calculation is wrong. Function: `Harvest_How_Much_Fishery_Access()`.

**Code at issue** — `atHarvestCatch.c` line 835  
Search for: `Ecology_Get_Size(bm, species, Wgt, basechrt)`

```c
  833:     int flagimposecatch = (int) (bm->SP_FISHERYprms[species][nf][flagimposecatch_id]);
  834:     double gear_change_scale = 1.0;
  835→     double li = Ecology_Get_Size(bm, species, Wgt, basechrt);
  836:     double sel = Get_Catch_Selectivity(bm, species, stage, nf, li, &gear_change_scale, &sel_curve);
  837:     double q = Get_Catchability(bm, species, cohort, stage, nf);
```

**Suggested fix (from review):** Pass `cohort`, not `basechrt`, to `Ecology_Get_Size`.

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

atHarvestCatch.c `Get_addlsm_Scale()` returns a selectivity difference, which `Get_Selectivity()` then adds to a length-at-50% (`sel_lsm + addlsm`). This mixes units, but it may be intended as a rescaling heuristic.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atHarvestCatch.c `Harvest_How_Much_Fishery_Access()` calls `Get_Catchability()`, which uses `bm->current_box` / `current_layer` for the stock id. These may be stale when it is called from output code (atagetracerIO.c).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
