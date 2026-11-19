# atharvest/atHarvestCatch.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1), plus 2 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [HARV-11](#harv-11) ⭐ | 232 | H/H | wrong parameter id | `Get_Fishery_Group_Change_Scale(bm, nf, sp, bm->flagchangeF, mFC_num_changes_id, mFCchange[sp])` passes the gl… |  | [ ] |
| [HARV-12](#harv-12) | 709-718, 732-743 | M/L | copy-paste | The "baseline" selectivity `sel_temp` should be computed without the change offsets. For `q_bimodal_id`, line … |  | [ ] |
| [HARV-13](#harv-13) | 829, 835 | M/M | cohort vs cohort x genotype | `Ecology_Get_Size(bm, species, Wgt, basechrt)` passes the age class. `Ecology_Get_Size` indexes `sn[]`, `X_RS[… |  | [ ] |

---

## HARV-11

**wrong parameter id** · Conf H / Sev H ⭐ priority  
**Listed lines:** 232

**Function:** `Get_Fishing_Mortality()`

**Why it's thought to be a bug**

`Get_Fishery_Group_Change_Scale(bm, nf, sp, bm->flagchangeF, mFC_num_changes_id, mFCchange[sp])` passes the global flag value (1) as the parameter index. It reads `SP_FISHERYprms[sp][nf][1]`, which is `flagQchange_id` (enum in atlantisboxmodel.h:950-952). So forced-F (mFC) change schedules are only applied when the Q-change flag is set for that group/fleet, and the per-group `flagFchange_id` (read at atHarvestParamIO.c:597) is ignored. Function: `Get_Fishing_Mortality()`.

**Current code** (`atharvest/atHarvestCatch.c` lines 230–234)

```c
  230: 	/* Get scenario scalars */
  231: 	if (bm->flagchangeF){
  232: 		mFC_scale = Get_Fishery_Group_Change_Scale(bm, nf,sp, bm->flagchangeF, mFC_num_changes_id, mFCchange[sp]);
  233: 	}else{
  234: 		mFC_scale = 1.0;
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
**Listed lines:** 709-718, 732-743

**Function:** `Get_addlsm_Scale()`

**Why it's thought to be a bug**

The "baseline" selectivity `sel_temp` should be computed without the change offsets. For `q_bimodal_id`, line 714 adds `addlsm` to `sel_bilsm2` for the baseline second mode. For `q_binormal_id`, lines 737-738 add both `addlsm` and `addsigma` to the baseline. So `sel - sel_temp` understates the change. Only affects invertebrates using bimodal curves with selectivity changes. Function: `Get_addlsm_Scale()`.

**Current code** (`atharvest/atHarvestCatch.c` lines 706–744)

```c
  706: 				returnValue = sel - sel_temp;
  707: 				break;
  708: 			case q_bimodal_id: /* Bimodal assymetric "normal" selectivity curve based on size */
  709: 				sel_lsm = bm->FISHERYprms[nf][sel_bilsm1_id];
  710: 				sel_sigma = bm->FISHERYprms[nf][sel_bisigma_id];
  711: 				step1 = li - sel_lsm;
  712: 				step2 = -(step1 * step1) / (2.0 * sel_sigma * sel_sigma + small_num);
  713: 				sel_temp1 = exp(step2);
  714: 				sel_lsm = bm->FISHERYprms[nf][sel_bilsm2_id] + addlsm;
  715: 				step1 = li - sel_lsm;
  716: 				step2 = -(step1 * step1) / (2.0 * sel_sigma * sel_sigma + small_num);
  717: 				sel_temp2 = exp(step2);
  718: 				sel_temp = max(sel_temp1, sel_temp2);
  719: 				sel_lsm = bm->FISHERYprms[nf][sel_bilsm1_id] + addlsm;
  720: 				sel_sigma = bm->FISHERYprms[nf][sel_bisigma_id] + addsigma;
  721: 				step1 = li - sel_lsm;
  722: 				step2 = -(step1 * step1) / (2.0 * sel_sigma * sel_sigma + small_num);
  723: 				sel1 = exp(step2);
  724: 				sel_lsm = bm->FISHERYprms[nf][sel_bilsm2_id] + addlsm;
  725: 				step1 = li - sel_lsm;
  726: 				step2 = -(step1 * step1) / (2.0 * sel_sigma * sel_sigma + small_num);
  727: 				sel2 = exp(step2);
  728: 				sel = max(sel1, sel2);
  729: 				returnValue = sel - sel_temp;
  730:                 break;
  731:             case q_binormal_id: /* Bimodal assymetric "normal" selectivity curve based on size */
  732:                 sel_lsm = bm->FISHERYprms[nf][sel_bilsm1_id];
  733:                 sel_sigma = bm->FISHERYprms[nf][sel_bisigma_id];
  734:                 step1 = li - sel_lsm;
  735:                 step2 = -(step1 * step1) / (2.0 * sel_sigma * sel_sigma + small_num);
  736:                 sel_temp1 = exp(step2);
  737:                 sel_lsm = bm->FISHERYprms[nf][sel_bilsm2_id] + addlsm;
  738:                 sel_sigma = bm->FISHERYprms[nf][sel_bisigma2_id] + addsigma;
  739:                 sel_ampli = bm->FISHERYprms[nf][sel_ampli_id];
  740:                 step1 = li - sel_lsm;
  741:                 step2 = -(step1 * step1) / (2.0 * sel_sigma * sel_sigma + small_num);
  742:                 sel_temp2 = sel_ampli * exp(step2);
  743:                 sel_temp = max(sel_temp1, sel_temp2);
  744:                 sel_lsm = bm->FISHERYprms[nf][sel_bilsm1_id] + addlsm;
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
**Listed lines:** 829, 835

**Function:** `Harvest_How_Much_Fishery_Access()`

**Why it's thought to be a bug**

`Ecology_Get_Size(bm, species, Wgt, basechrt)` passes the age class. `Ecology_Get_Size` indexes `sn[]`, `X_RS[]` and `VERTinfo[][]` by cohort x genotype (atUtilGroupIO.c:771,843), and the main fishing path passes `chrt` (atHarvest.c:444). With `numGeneTypes > 1`, this gives the wrong genotype's length, so selectivity in the output access calculation is wrong. Function: `Harvest_How_Much_Fishery_Access()`.

**Current code** (`atharvest/atHarvestCatch.c` lines 827–837)

```c
  827: double Harvest_How_Much_Fishery_Access(MSEBoxModel *bm, int species, int cohort, int nf, double Wgt) {
  828:     int do_debug = 0;
  829:     int basechrt = (int)floor(cohort / FunctGroupArray[species].numGeneTypes);
  830:     int stage = FunctGroupArray[species].cohort_stage[cohort];
  831:     int sel_curve = (int) (bm->FISHERYprms[nf][selcurve_id]);
  832:     int flagF = (int) (bm->SP_FISHERYprms[species][nf][flagF_id]);
  833:     int flagimposecatch = (int) (bm->SP_FISHERYprms[species][nf][flagimposecatch_id]);
  834:     double gear_change_scale = 1.0;
  835:     double li = Ecology_Get_Size(bm, species, Wgt, basechrt);
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
