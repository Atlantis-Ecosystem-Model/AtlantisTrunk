# atmanage/atManage.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **29 findings** (severity H: 8; confidence H and severity H: 6; on the priority list: 6), plus 5 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MNG-01](#mng-01) ⭐ | 163, 1052 | H/H | swapped indices | `bm->TAC_over[ij][sp][k] = 0` with `ij` looping over `K_num_fisheries` — TAC_over is allocated `Util_Alloc_Ini… |  | [ ] |
| [MNG-02](#mng-02) ⭐ | 971, 2010 | H/H | buffer overrun | `bm->TAC_over[sp][fishery_id][nreg]` — third dimension is allocated with `K_num_basket` (atManageSetup.c:1260;… |  | [ ] |
| [MNG-03](#mng-03) 🔀 | 419, 392 | H/M | uninitialised variable | `} else if (flagmanage > stock_adapt_mgmt)` — `flagmanage` is only assigned inside the `if (flagspeffortmodel … |  | [ ] |
| [MNG-04](#mng-04) 🔀 | 503, 561 | H/M | wrong index | `mpa_scale = bm->MPA[bm->current_box][fishery_id];` inside `for (ij...)` loop — `current_box` is the ecology l… |  | [ ] |
| [MNG-05](#mng-05) | 566 | M/M | always-false condition | Outer `if (flagspeffortmodel == wgt_effort \|\| == dist_effort)` then inner `if (== array_effort \|\| == dist_effo… |  | [ ] |
| [MNG-06](#mng-06) 🔀 | 121 | M/M | ordering | `bm->CumDisplaceEffort[ij][nf] += bm->Effort[ij][nf];` runs after `bm->Effort[ij][nf] = 0.0` at line 123 (when… |  | [ ] |
| [MNG-07](#mng-07) 🔀 ⭐ | 239, 247, 441 | H/H | uninitialised variable | In the `!bm->EffortModelsActive` branch `Effort_Displacement(bm, ..., orig_FCpressure, &FCpressure, ...)` is c… |  | [ ] |
| [MNG-08](#mng-08) | 1148, 1144, 1096 | H/M | wrong variable / copy-paste | `if (regtaccount == nstock)` in the common-pool regional closure check — `nstock` is stale (last species' valu… |  | [ ] |
| [MNG-09](#mng-09) | 960 | L/M | wrong operator | `if (!in_quota \|\| (!sp_reg && (nstock < 1)))` — comment says "Only include those groups under regional TACs wi… |  | [ ] |
| [MNG-10](#mng-10) | 1281, 1278 | H/M | always-true condition | `if (flagpulsefish && (end_date < bm->dayt + 1.0))` inside `if (end_date < bm->dayt)` — always true, so the 1/… |  | [ ] |
| [MNG-11](#mng-11) | 1751, 1548 | M/M | off-by-one / missing wrap | `bm->Effort_hdistrib[ij][fishery_id][bm->QofY + 1]` without the `QofY < 3` guard used in array_effort/rec_effo… |  | [ ] |
| [MNG-12](#mng-12) 🔀 ⭐ | 459 | H/H | inverted logic | `MPA_check = 1.0 - bm->MPA[nb][fishery_id]; fishthere *= MPA_check;` — bm->MPA is an access multiplier (1 = op… |  | [ ] |
| [MNG-13](#mng-13) ⭐ | 2100, 2083 | H/H | buffer overrun | "Break loop" is done with `sp = bm->K_num_tot_sp;` but the inner `cohort`/`cIndex` loops keep running and imme… |  | [ ] |
| [MNG-14](#mng-14) | 2081 | H/M | wrong box | `tracerArray = boxLayerInfo->localWCTracers;` — this is the tracer set of whatever box/layer the ecology loop … |  | [ ] |
| [MNG-15](#mng-15) | 2083 | M/H | cohort vs cohort×genotype | `for (cohort = 0; cohort < numCohortsXnumGenes; ...) contaminantTracers[cohort][cIndex]` — contaminantTracers … |  | [ ] |
| [MNG-16](#mng-16) | 2132 | H/M | state not reset | `CAP_scale = 1.0;` is set once before the fishery loop; a fishery with `flagchangeP_id == 0` inherits the CAP_… |  | [ ] |
| [MNG-17](#mng-17) | 2225 | H/M | state not reset | `double POP_scale = 1.0` initialised once; ports without `popchange_id` get the previous port's change scale a… |  | [ ] |
| [MNG-18](#mng-18) | 2581, 2554, 2012 | M/M | logic / stale variable | Rescaling of BiTACamt is inside the per-companion loop, so it is applied once per OK companion (compounding `B… |  | [ ] |
| [MNG-19](#mng-19) | 2394, 2012 | L/L | wrong message argument | `quit("...more than 12 months in a year - %d\n", bm->BiM)` while switching on `bm->MofY`. Function: `Manage_Vi… |  | [ ] |
| [MNG-20](#mng-20) | 2650 | L/L | off-by-one (doc vs code) | `if (val > maxValue) maxValue = val + 1;` — later fisheries are compared against the +1'd value, so e.g. value… |  | [ ] |
| [MNG-21](#mng-21) ⭐ | 2741, 2727 | H/H | infinite loop | `while (this_shot < nshot) { step1 = drandom(0, max_prob); ... if (step1 <= sum_prob) {...this_shot++;} }` — i… |  | [ ] |
| [MNG-22](#mng-22) | 2742, 2894 | M/H | buffer overrun | `nshot` is derived from boats × down_time × FishableLength × hr_per_dt/ShotLength and is not clamped to `max_p… |  | [ ] |
| [MNG-23](#mng-23) | 2867 | H/M | wrong operator | `bm->alloced_discard[sp][ij] -= final_shot_discards;` — alloced_discard is subtracted from available discards … |  | [ ] |
| [MNG-24](#mng-24) | 2879 | H/M | accumulator / allocation logic | `sum_prob` is reset only once before the while loop and the k loop has no `break`, so after the first pass eve… |  | [ ] |
| [MNG-25](#mng-25) | 3006 | H/M | wrong array / copy-paste | `for (ij = 0; ij < bm->nbox; ij++) bm->prop_effort[ij] /= (new_sum + small_num);` — after reducing `prop_depth… |  | [ ] |
| [MNG-26](#mng-26) | 2951 | M/M | unit mismatch | Left-over effort uses `Effort_hdistrib[..][today_effort] / 24.0` whereas shot effort uses `* 24.0` ("originall… |  | [ ] |
| [MNG-27](#mng-27) | 2951, 2955 | H/M | accumulator in wrong loop | `ShotData[...][tdiscard_id] += Discards[ij][sp][nf]*... - alloced_discard[sp][ij]` is inside the `k` (layer) l… |  | [ ] |
| [MNG-28](#mng-28) | 2958 | M/L | wrong formula | Left-over `wdepth_id += catch` then `/= tcatch` gives ~1, not a catch-weighted depth (compare 2987 which multi… |  | [ ] |
| [MNG-29](#mng-29) | 2832 | M/L | logic | If `step1 >= cpue_cdf[sp][nc][gun]` for all bins (cdf not reaching 1), `this_catchbin` stays 0 giving zero cat… |  | [ ] |

---

## MNG-01

**swapped indices** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManage.c` 163, `atManage.c` 1052
**Lines at review (Oct 2026):** 160-163

**Function:** `Manage_Calculate_Total_Effort()`

**Why it's thought to be a bug**

`bm->TAC_over[ij][sp][k] = 0` with `ij` looping over `K_num_fisheries` — TAC_over is allocated `Util_Alloc_Init_3D_Int(K_num_basket, K_num_fisheries, K_num_tot_sp)` (atManageSetup.c:1260) => `[sp][nf][basket]`, and every other use is `TAC_over[sp][nf][..]` (lines 830, 871, 971, atHarvestDiscards.c:248). Here sp indexes the fishery dimension (size K_num_fisheries) -> out-of-bounds writes whenever K_num_tot_sp > K_num_fisheries, and the real flags are never reset so TAC-over discarding/closures are sticky. Function: `Manage_Calculate_Total_Effort()`.

**Code at issue** — `atManage.c` line 163  
Search for: `TAC_over[ij][sp][k] = 0`

```c
  161:             for (ij = 0; ij < bm->K_num_fisheries; ij++) {
  162:                 for (k = 0; k < bm->K_num_basket; k++) {
  163→ 					bm->TAC_over[ij][sp][k] = 0;
  164:                 }
  165:             }
```

**Code at issue** — `atManage.c` line 1052 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->TAC_over[sp][nf][0] = 1;`

```c
 1050: 							flagTACparticipate = (int) (bm->FISHERYprms[nf][flagTACpartipcate_id]);
 1051: 							if (flagTACparticipate) {
 1052→ 								bm->TAC_over[sp][nf][0] = 1;
 1053: 								//fprintf(llogfp,"time: %e, Cumulative catch of %s (%e) is over common pool TAC (%e) for fisheries including %s\n", bm->dayt, FunctGroupArray[sp].groupCode, bm->TotCumCatch[sp][fishery_id][bm->thisyear], TAC, FisheryArray[fishery_id].fisheryCode);
 1054: 								taccount++;
```

**Suggested fix (from review):** `bm->TAC_over[sp][ij][k] = 0;`

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

## MNG-02

**buffer overrun** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManage.c` 971, `atManage.c` 2010
**Lines at review (Oct 2026):** 959, 978, 1111, 1133, 2405

**Why it's thought to be a bug**

`bm->TAC_over[sp][fishery_id][nreg]` — third dimension is allocated with `K_num_basket` (atManageSetup.c:1260; K_num_basket = max basket size, usually 1, atManageSetup.c:1291-1297) but is indexed by region/stock `nreg` up to `K_num_active_reg` / `numStocks`. Any regional-TAC config with >1 region/stock writes/reads past the end (also atHarvestDiscards.c:248 reads `[this_stock]`). Functions: `TAC_Check()`, `Manage_Visit_Council()`.

**Code at issue** — `atManage.c` line 971  
Search for: `TAC_over[sp][fishery_id][nreg]`

```c
  969: 						TAC = (regTAC_scale * bm->TACamt[sp][fishery_id][now_id] * kg_2_mg) / bm->X_CN;
  970: 						if(((totCumCatch + bm->TotOldCumCatch[sp][fishery_id]) * regC_scale) > TAC){
  971→ 							bm->TAC_over[sp][fishery_id][nreg] = 1;
  972: 
  973:                             /*
```

**Code at issue** — `atManage.c` line 2010  
Search for: `Manage_Visit_Council()`

```c
 2008: 		case mix_f_r_spawn_mpa: /* Mix of fixed, rolling and seasonal spawning closures used in management */
 2009: 		case depth_stock_mpa: /* US-like depth closures triggered when TAC exceeded (if list needed updating that
 2010→ 		 has been done in the Manage_Visit_Council() routine) */
 2011: 		case council_stock_mpa: /* US-like closures (but based on biom distrib of groups) triggered when TAC exceeded
 2012: 		 (if list needed updating that has been done in the Manage_Visit_Council() routine) */
```

**Suggested fix (from review):** Allocate third dim as max(K_num_basket, nreg, K_num_stocks_per_sp) (and reset that full extent in MNG-01).

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

## MNG-03

**uninitialised variable** · Conf H / Sev M  
**Found at (current code):** `atManage.c` 419, `atManage.c` 392
**Lines at review (Oct 2026):** 387-417

**Function:** `Manage_Calculate_Total_Effort()`

**Why it's thought to be a bug**

`} else if (flagmanage > stock_adapt_mgmt)` — `flagmanage` is only assigned inside the `if (flagspeffortmodel != readts_effort)` branch (line 395); for read-ts effort fisheries it is uninitialised (first fishery) or stale from a previous fishery, so TAC_Check (and TAC_over discards) runs/does not run for the wrong fisheries. Function: `Manage_Calculate_Total_Effort()`.

> 🔀 **Changed by the merge of main (Nov 2026):** the code for this finding was changed by the merge. Check whether this finding is now fixed before acting.

**Code at issue** — `atManage.c` line 419  
Search for: `} else if (flagmanage > stock_adapt_mgmt)`

```c
  417: 				Check_For_Active_MPA(bm, fishery_id);
  418:              */
  419→         } else if (flagmanage > stock_adapt_mgmt){
  420:             /* TAC-check needed so discard if over TAC amount - dummy not used but TAC_over calculated within the routine is */
  421:             dummy = TAC_Check(bm, fishery_id, flagmanage, llogfp);
```

**Code at issue** — `atManage.c` line 392  
Search for: `if (flagspeffortmodel != readts_effort)`

```c
  390:             active_scale = 1.0;
  391:         
  392→         if (flagspeffortmodel != readts_effort) {
  393: 
  394:             /* Effort management */
```

**Suggested fix (from review):** Move `flagmanage = (int) bm->FISHERYprms[fishery_id][flagmanage_id];` before the if.

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

## MNG-04

**wrong index** · Conf H / Sev M  
**Found at (current code):** `atManage.c` 503, `atManage.c` 561
**Lines at review (Oct 2026):** 498

**Function:** `Manage_Calculate_Total_Effort()`

**Why it's thought to be a bug**

`mpa_scale = bm->MPA[bm->current_box][fishery_id];` inside `for (ij...)` loop — `current_box` is the ecology loop's stale global box, not `ij`; every box gets the MPA scalar of one arbitrary box. Compare line 459 `bm->MPA[ij][fishery_id]`. Function: `Manage_Calculate_Total_Effort()`.

> 🔀 **Changed by the merge of main (Nov 2026):** MPA[bm->current_box] -> MPA[ij] (JMK fix). Check whether this finding is now fixed before acting.

**Code at issue** — `atManage.c` line 503  
Search for: `mpa_scale = bm->MPA[bm->current_box][fishery_id]`

```c
  501:                     flagfcmpa = (int) (bm->FISHERYprms[fishery_id][flagmpa_id]);
  502:                     if (flagfcmpa) {
  503→                     	mpa_scale = bm->MPA[ij][fishery_id]; /*jmk fix, from mpa_scale = bm->MPA[bm->current_box][fishery_id];*/
  504:                         
  505:                         /* Allow for infringement */
```

**Code at issue** — `atManage.c` line 561  
Search for: `for (ij`

```c
  559: 	for (fishery_id = 0; fishery_id < bm->K_num_fisheries; fishery_id++) {
  560:         
  561→ 		for (ij = 0; ij < ncells; ij++) {
  562: 			bm->totNewEffort[fishery_id] += bm->Effort[ij][fishery_id];
  563: 		}
```

**Suggested fix (from review):** `bm->MPA[ij][fishery_id]`

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

## MNG-05

**always-false condition** · Conf M / Sev M  
**Found at (current code):** `atManage.c` 566
**Lines at review (Oct 2026):** 561-563

**Function:** `Manage_Calculate_Total_Effort()`

**Why it's thought to be a bug**

Outer `if (flagspeffortmodel == wgt_effort || == dist_effort)` then inner `if (== array_effort || == dist_effort)` — array_effort can never be true inside, and wgt_effort never passes the inner test, so effort buffering is never applied to wgt_effort (and array_effort is never reached). One of the two lists is a copy-paste error. Function: `Manage_Calculate_Total_Effort()`.

**Code at issue** — `atManage.c` line 566 (closest match to the quoted code; wording differs slightly)  
Search for: `if (flagspeffortmodel == wgt_effort || flagspeffortmodel == dist_effort) {`

```c
  564:         
  565: 		flagspeffortmodel = (int) (bm->FISHERYprms[fishery_id][flageffortmodel_id]);
  566→ 		if (flagspeffortmodel == wgt_effort || flagspeffortmodel == dist_effort) {
  567: 			if (bm->flagbuffereffort) {
  568: 				if (flagspeffortmodel == array_effort || flagspeffortmodel == dist_effort) {
```

**Suggested fix (from review):** Make the two model lists consistent (decide whether array_effort or wgt_effort is intended).

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

## MNG-06

**ordering** · Conf M / Sev M  
**Found at (current code):** `atManage.c` 121
**Lines at review (Oct 2026):** 116-148

**Function:** `Manage_Calculate_Total_Effort()`

**Why it's thought to be a bug**

`bm->CumDisplaceEffort[ij][nf] += bm->Effort[ij][nf];` runs after `bm->Effort[ij][nf] = 0.0` at line 123 (when EffortModelsActive), so the displaced-effort record (written out in atHarvestIO.c:570) only ever accumulates zeros (or the 1.0 reset value). Function: `Manage_Calculate_Total_Effort()`.

> 🔀 **Changed by the merge of main (Nov 2026):** CumDisplaceEffort accumulation moved. Check whether this finding is now fixed before acting.

**Code at issue** — `atManage.c` line 121 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->OldEffort[ij][nf] = bm->Effort[ij][nf];`

```c
  119:             for (ij = 0; ij < ncells; ij++) {
  120:                 bm->CumEffort[nf][ij] += bm->Effort[ij][nf];
  121→                 bm->OldEffort[ij][nf] = bm->Effort[ij][nf];
  122:                 bm->totOldEffort[nf] += bm->OldEffort[ij][nf];
  123:                 bm->Effort[ij][nf] = 0.0;
```

**Suggested fix (from review):** Accumulate CumDisplaceEffort before Effort is zeroed (e.g. use OldEffort).

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

## MNG-07

**uninitialised variable** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManage.c` 239, `atManage.c` 247, `atManage.c` 441
**Lines at review (Oct 2026):** 245-257

**Function:** `Manage_Calculate_Total_Effort()`

**Why it's thought to be a bug**

In the `!bm->EffortModelsActive` branch `Effort_Displacement(bm, ..., orig_FCpressure, &FCpressure, ...)` is called with `FCpressure` never initialised (declared line 86); Effort_Displacement reads `*FCpressure` (line 1937-1938) and sets `*FCdisplaced = orig_FCpressure - *FCpressure`, so F_rescale/F_displaced applied to `bm->Effort` are garbage. Function: `Manage_Calculate_Total_Effort()`.

> 🔀 **Changed by the merge of main (Nov 2026):** orig_FCpressure now set before the branch. Check whether this finding is now fixed before acting.

**Code at issue** — `atManage.c` line 239  
Search for: `!bm->EffortModelsActive`

```c
  237:     }
  238:      
  239→     if(!bm->EffortModelsActive) {
  240:         // Check MPAs active or not then jump out - need MPA check as used by some catch only models
  241:         if (bm->dayt != bm->predayt) {
```

**Code at issue** — `atManage.c` line 247  
Search for: `Effort_Displacement(bm,`

```c
  245:                     for (ij = 0; ij < bm->nbox; ij++) {
  246:                         orig_FCpressure = 1.0;
  247→                         Effort_Displacement(bm, fishery_id, ij, orig_FCpressure, &FCpressure, &FCdisplaced, &new_fish_loc, llogfp);
  248:                         if (FCdisplaced < 0.0)
  249:                             FCdisplaced = 0.0;
```

**Code at issue** — `atManage.c` line 441  
Search for: `* FCpressure`

```c
  439: 
  440: 					/* Adjust based on spatially independent effort management (or changes) above */
  441→ 					FCpressure = EFF_scale0 * EFF_scale1 * EFF_scale2 * EFF_scale3 * EFF_scale4 * FCpressure;
  442: 
  443: 					/* Set the flag allowing an effort drop to on if management
```

**Suggested fix (from review):** Set `FCpressure = orig_FCpressure;` (scaled by MPA if intended) before the call.

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

## MNG-08

**wrong variable / copy-paste** · Conf H / Sev M  
**Found at (current code):** `atManage.c` 1148, `atManage.c` 1144, `atManage.c` 1096
**Lines at review (Oct 2026):** 1129-1137

**Function:** `TAC_Check()`

**Why it's thought to be a bug**

`if (regtaccount == nstock)` in the common-pool regional closure check — `nstock` is stale (last species' value from the loop at 1077-1127, possibly K_num_active_reg) while the count loop runs to `FunctGroupArray[sp].numStocks`. The non-common-pool version (988-993) correctly resets `nstock = numStocks`. Fishery closure decision is wrong when species have different stock numbers. Function: `TAC_Check()`.

**Code at issue** — `atManage.c` line 1148  
Search for: `if (regtaccount == nstock)`

```c
 1146: 							regtaccount++;
 1147: 					}
 1148→ 					if (regtaccount == nstock) {
 1149: 						taccount++;
 1150: 					}
```

**Code at issue** — `atManage.c` line 1144  
Search for: `FunctGroupArray[sp].numStocks`

```c
 1142: 				if (FunctGroupArray[sp].isTAC) {
 1143: 					regtaccount = 0;
 1144→ 					for (nreg = 0; nreg < FunctGroupArray[sp].numStocks; nreg++) {
 1145: 						if (bm->TAC_over[sp][fishery_id][nreg])
 1146: 							regtaccount++;
```

**Code at issue** — `atManage.c` line 1096 (closest match to the quoted code; wording differs slightly)  
Search for: `nstock = FunctGroupArray[sp].numStocks;`

```c
 1094: 						nstock = bm->K_num_active_reg;
 1095:                     } else {
 1096→ 						nstock = FunctGroupArray[sp].numStocks;
 1097:                     }
 1098: 
```

**Suggested fix (from review):** Add `nstock = FunctGroupArray[sp].numStocks;` before the inner loop.

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

## MNG-09

**wrong operator** · Conf L / Sev M  
**Found at (current code):** `atManage.c` 960
**Lines at review (Oct 2026):** 948, 1087

**Function:** `TAC_Check()`

**Why it's thought to be a bug**

`if (!in_quota || (!sp_reg && (nstock < 1)))` — comment says "Only include those groups under regional TACs with separate stocks", but with `&&` non-regional species (sp_reg = 0) are still processed whenever nstock >= 1 (always), so speciesParams[regionalSP_id] has no effect. Function: `TAC_Check()`.

**Code at issue** — `atManage.c` line 960  
Search for: `if (!in_quota || (!sp_reg && (nstock < 1)))`

```c
  958: 						nstock = FunctGroupArray[sp].numStocks;
  959: 
  960→ 					if (!in_quota || (!sp_reg && (nstock < 1))) /* Only include those groups under regional TACs with seperate stocks */
  961: 						continue;
  962: 
```

**Suggested fix (from review):** Likely `(!sp_reg || (nstock < 1))`.

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

## MNG-10

**always-true condition** · Conf H / Sev M  
**Found at (current code):** `atManage.c` 1281, `atManage.c` 1278
**Lines at review (Oct 2026):** 1266-1273

**Function:** `Forced_Effort_Change()`

**Why it's thought to be a bug**

`if (flagpulsefish && (end_date < bm->dayt + 1.0))` inside `if (end_date < bm->dayt)` — always true, so the 1/EFFmultA "undo pulse" correction (meant only for the first day after the pulse) is applied every timestep forever, compounding on previous-effort-dependent models. Function: `Forced_Effort_Change()`.

**Code at issue** — `atManage.c` line 1281  
Search for: `if (flagpulsefish && (end_date < bm->dayt + 1.0))`

```c
 1279: 				/* Change over */
 1280: 				flagpulsefish = (int) (EFFORTchange[fishery_id][now_change][pulse_id]);
 1281→ 				if (flagpulsefish && (end_date < bm->dayt + 1.0)) {
 1282: 					/* If only pulse of change, divide by level of change to return to
 1283: 					 normal for fisheries dependent on previous effort levels - correction only needed for the first day after the end of the pulse */
```

**Code at issue** — `atManage.c` line 1278  
Search for: `if (end_date < bm->dayt)`

```c
 1276: 		 each little change in effort is cumulative */
 1277: 		if (flag_use_preveffortmodel) {
 1278→ 			if (end_date < bm->dayt) {
 1279: 				/* Change over */
 1280: 				flagpulsefish = (int) (EFFORTchange[fishery_id][now_change][pulse_id]);
```

**Suggested fix (from review):** `if (flagpulsefish && (bm->dayt - end_date) <= bm->dt / 86400.0)` (or track that the reset has been done).

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

## MNG-11

**off-by-one / missing wrap** · Conf M / Sev M  
**Found at (current code):** `atManage.c` 1751, `atManage.c` 1548
**Lines at review (Oct 2026):** 1739-1740

**Function:** `Allocate_Immediate_Effort()`

**Why it's thought to be a bug**

`bm->Effort_hdistrib[ij][fishery_id][bm->QofY + 1]` without the `QofY < 3` guard used in array_effort/rec_effort (1463-1468, 1548-1553); in Q4 it reads entry 4 (unused quarterly slot, normally 0, or a monthly econ value) instead of wrapping to 0. Function: `Allocate_Immediate_Effort()` (ideal_distrib_effort).

**Code at issue** — `atManage.c` line 1751  
Search for: `Effort_hdistrib[ij][fishery_id][bm->QofY + 1]`

```c
 1749: 			/* If no target left then may be targeting by-product so distrbute based on perscribed distributions */
 1750: 			if (!totFCweight) {
 1751→ 				FCweight = (bm->HowFar * (bm->Effort_hdistrib[ij][fishery_id][bm->QofY + 1] - bm->Effort_hdistrib[ij][fishery_id][bm->QofY])
 1752: 						+ bm->Effort_hdistrib[ij][fishery_id][bm->QofY]);
 1753: 			}
```

**Code at issue** — `atManage.c` line 1548  
Search for: `QofY < 3`

```c
 1546: 		}
 1547: 		/* Use population-based effort weight to scale effort in coastal boxes */
 1548→ 		if (bm->QofY < 3)
 1549: 			FCpressure = FCweight * (bm->HowFar * (bm->Effort_hdistrib[ij][fishery_id][bm->QofY + 1] - bm->Effort_hdistrib[ij][fishery_id][bm->QofY])
 1550: 					+ bm->Effort_hdistrib[ij][fishery_id][bm->QofY]);
```

**Suggested fix (from review):** Use `(bm->QofY < 3) ? QofY+1 : 0`.

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

## MNG-12

**inverted logic** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManage.c` 459
**Lines at review (Oct 2026):** 1894-1897

**Function:** `Effort_Displacement()`

**Why it's thought to be a bug**

`MPA_check = 1.0 - bm->MPA[nb][fishery_id]; fishthere *= MPA_check;` — bm->MPA is an access multiplier (1 = open, applied as `FCpressure * MPA` at 454; default 1.0, atManageSetup.c:1219). Using 1-MPA zeroes open neighbours and gives full weight to closed ones, so displaced effort is sent INTO closed areas. Function: `Effort_Displacement()`.

> 🔀 **Changed by the merge of main (Nov 2026):** MPA_check no longer inverted. Check whether this finding is now fixed before acting.

**Code at issue** — `atManage.c` line 459  
Search for: `FCpressure * bm->MPA`

```c
  457: 					/* Spatial management */
  458: 					orig_FCpressure = FCpressure;
  459→ 					FCpressure = FCpressure * bm->MPA[ij][fishery_id];
  460:                     
  461:                     /**
```

**Suggested fix (from review):** `MPA_check = bm->MPA[nb][fishery_id];`

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

## MNG-13

**buffer overrun** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManage.c` 2100, `atManage.c` 2083
**Lines at review (Oct 2026):** 2067-2089

**Function:** `Check_For_Active_MPA()`

**Why it's thought to be a bug**

"Break loop" is done with `sp = bm->K_num_tot_sp;` but the inner `cohort`/`cIndex` loops keep running and immediately evaluate `FunctGroupArray[sp].numCohortsXnumGenes` / `FunctGroupArray[sp].contaminantTracers[cohort][cIndex]` with sp == K_num_tot_sp — read past FunctGroupArray. Function: `Check_For_Active_MPA()`.

**Code at issue** — `atManage.c` line 2100  
Search for: `sp = bm->K_num_tot_sp`

```c
 2098:                                     
 2099:                                     // Break loop as already closed now
 2100→                                     sp = bm->K_num_tot_sp;
 2101:                                 }
 2102:                             }
```

**Code at issue** — `atManage.c` line 2083  
Search for: `FunctGroupArray[sp].numCohortsXnumGenes`

```c
 2081:                     tracerArray = boxLayerInfo->localWCTracers;
 2082:                     for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 2083→                         for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
 2084:                             for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
 2085:                                 cGroupLevel = tracerArray[FunctGroupArray[sp].contaminantTracers[cohort][cIndex]];
```

**Suggested fix (from review):** Use a flag/goto to exit all three loops (or `break` out of each level).

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

## MNG-14

**wrong box** · Conf H / Sev M  
**Found at (current code):** `atManage.c` 2081
**Lines at review (Oct 2026):** 2066-2070

**Function:** `Check_For_Active_MPA()`

**Why it's thought to be a bug**

`tracerArray = boxLayerInfo->localWCTracers;` — this is the tracer set of whatever box/layer the ecology loop last processed, not box `ij`; every box's closure is decided on one box's concentrations (and epibenthic groups' contaminantTracers index the epi tracer list, not WC). Function: `Check_For_Active_MPA()`.

**Code at issue** — `atManage.c` line 2081  
Search for: `tracerArray = boxLayerInfo->localWCTracers`

```c
 2079:                     break;
 2080:                 case conc_based: // Based on concentration in species
 2081→                     tracerArray = boxLayerInfo->localWCTracers;
 2082:                     for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 2083:                         for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
```

**Suggested fix (from review):** Read concentrations for box ij (e.g. from `bm->boxes[ij].tr[k][...]` / epi arrays).

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

## MNG-15

**cohort vs cohort×genotype** · Conf M / Sev H  
**Found at (current code):** `atManage.c` 2083
**Lines at review (Oct 2026):** 2068

**Function:** `Check_For_Active_MPA()`

**Why it's thought to be a bug**

`for (cohort = 0; cohort < numCohortsXnumGenes; ...) contaminantTracers[cohort][cIndex]` — contaminantTracers is allocated `[numCohorts][num_contaminants]` (atUtilGroupIO.c:909); with numGenes > 1 this reads past the array. Function: `Check_For_Active_MPA()`.

**Code at issue** — `atManage.c` line 2083 (closest match to the quoted code; wording differs slightly)  
Search for: `for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){`

```c
 2081:                     tracerArray = boxLayerInfo->localWCTracers;
 2082:                     for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 2083→                         for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
 2084:                             for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
 2085:                                 cGroupLevel = tracerArray[FunctGroupArray[sp].contaminantTracers[cohort][cIndex]];
```

**Suggested fix (from review):** Loop to `numCohorts`.

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

## MNG-16

**state not reset** · Conf H / Sev M  
**Found at (current code):** `atManage.c` 2132
**Lines at review (Oct 2026):** 2117-2143

**Function:** `Check_CAP()`

**Why it's thought to be a bug**

`CAP_scale = 1.0;` is set once before the fishery loop; a fishery with `flagchangeP_id == 0` inherits the CAP_scale computed for an earlier fishery. Function: `Check_CAP()`.

**Code at issue** — `atManage.c` line 2132  
Search for: `CAP_scale = 1.0`

```c
 2130:     */
 2131: 
 2132→ 	CAP_scale = 1.0;
 2133: 	for (fishery_id = 0; fishery_id < bm->K_num_fisheries; fishery_id++) {
 2134: 
```

**Suggested fix (from review):** Set `CAP_scale = 1.0;` at the top of the fishery loop.

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

## MNG-17

**state not reset** · Conf H / Sev M  
**Found at (current code):** `atManage.c` 2225
**Lines at review (Oct 2026):** 2210-2223

**Function:** `Update_Port_Population()`

**Why it's thought to be a bug**

`double POP_scale = 1.0` initialised once; ports without `popchange_id` get the previous port's change scale applied to their population. Function: `Update_Port_Population()`.

**Code at issue** — `atManage.c` line 2225  
Search for: `double POP_scale = 1.0`

```c
 2223: void Update_Port_Population(MSEBoxModel *bm, FILE *llogfp) {
 2224: 	int porti, flagchangePOP, POP_num_changes;
 2225→ 	double POP_scale = 1.0, start_pop;
 2226: 
 2227: 	for (porti = 0; porti < bm->K_num_ports; porti++) {
```

**Suggested fix (from review):** Reset `POP_scale = 1.0;` per port.

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

## MNG-18

**logic / stale variable** · Conf M / Sev M  
**Found at (current code):** `atManage.c` 2581, `atManage.c` 2554, `atManage.c` 2012
**Lines at review (Oct 2026):** 2517-2575

**Function:** `Manage_Visit_Council()`

**Why it's thought to be a bug**

Rescaling of BiTACamt is inside the per-companion loop, so it is applied once per OK companion (compounding `BiTACamt *= 1/catch_ratio` each time) and never if the species has no companions; also `prop_yr_changed = 1 - implement_BiM/6.0` (2554) uses implement_BiM from a previous species/fishery since it is only set at 2557. Comment says rescale "only if companion species would be worse off, so check companions" — i.e. check all then rescale once. Function: `Manage_Visit_Council()`.

**Code at issue** — `atManage.c` line 2581 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->BiTAC_sp[bim][nreg][sp][now_id] = regTAC_scale * bm->BiTACamt[bim][sp][nf][now_id] * (`

```c
 2579:                                          */
 2580:                                         for (bim = implement_BiM; bim < 6; bim++) {
 2581→                                             bm->BiTAC_sp[bim][nreg][sp][now_id] = regTAC_scale * bm->BiTACamt[bim][sp][nf][now_id] * (1 / (catch_ratio + small_num));
 2582:                                             other_biTAC_reg = (1.0 - regTAC_scale) * bm->BiTACamt[bim][sp][nf][now_id];
 2583:                                             bm->BiTACamt[bim][sp][nf][now_id] = bm->BiTAC_sp[bim][nreg][sp][now_id] + other_biTAC_reg;
```

**Code at issue** — `atManage.c` line 2554 (closest match to the quoted code; wording differs slightly)  
Search for: `prop_yr_changed = (1.0 - implement_BiM / 6.0);`

```c
 2552: 							}
 2553: 							/* Check if scaled up CumCatch for companions would exceed TAC */
 2554→ 							prop_yr_changed = (1.0 - implement_BiM / 6.0);
 2555: 							CumCatch_cosp *= 1.0 + ((1 / (catch_ratio + small_num)) * prop_yr_changed);
 2556: 							catch_ratio_cosp = CumCatch_cosp / (TAC_cosp + small_num);
```

**Code at issue** — `atManage.c` line 2012  
Search for: `Manage_Visit_Council()`

```c
 2010: 		 has been done in the Manage_Visit_Council() routine) */
 2011: 		case council_stock_mpa: /* US-like closures (but based on biom distrib of groups) triggered when TAC exceeded
 2012→ 		 (if list needed updating that has been done in the Manage_Visit_Council() routine) */
 2013: 			bm->MPA[ij][fishery_id] = bm->MPAlist[bm->thisyear][bm->TofY][ij][fishery_id];
 2014: 			break;
```

**Suggested fix (from review):** Compute companion_ok over all companions first (AND), then do the nf/bim rescale once outside that loop; compute implement_BiM before use.

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

## MNG-19

**wrong message argument** · Conf L / Sev L  
**Found at (current code):** `atManage.c` 2394, `atManage.c` 2012
**Lines at review (Oct 2026):** 2379

**Function:** `Manage_Visit_Council()`

**Why it's thought to be a bug**

`quit("...more than 12 months in a year - %d\n", bm->BiM)` while switching on `bm->MofY`. Function: `Manage_Visit_Council()`.

**Code at issue** — `atManage.c` line 2394  
Search for: `quit("`

```c
 2392: 			break;
 2393: 		default:
 2394→ 			quit("How the hell did you get more than 12 months in a year - %d\n", bm->BiM);
 2395: 			break;
 2396: 		}
```

**Code at issue** — `atManage.c` line 2012  
Search for: `Manage_Visit_Council()`

```c
 2010: 		 has been done in the Manage_Visit_Council() routine) */
 2011: 		case council_stock_mpa: /* US-like closures (but based on biom distrib of groups) triggered when TAC exceeded
 2012→ 		 (if list needed updating that has been done in the Manage_Visit_Council() routine) */
 2013: 			bm->MPA[ij][fishery_id] = bm->MPAlist[bm->thisyear][bm->TofY][ij][fishery_id];
 2014: 			break;
```

**Suggested fix (from review):** Print `bm->MofY`.

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

## MNG-20

**off-by-one (doc vs code)** · Conf L / Sev L  
**Found at (current code):** `atManage.c` 2650
**Lines at review (Oct 2026):** 2615-2620

**Function:** `Manage_Get_Max_Fishery_Param()`

**Why it's thought to be a bug**

`if (val > maxValue) maxValue = val + 1;` — later fisheries are compared against the +1'd value, so e.g. values {3,4} return 4 not 5 as documented. Callers add +1 so no overrun found, but the function does not return what it claims; Species version (2644) never adds +1. Function: `Manage_Get_Max_Fishery_Param()`.

**Code at issue** — `atManage.c` line 2650 (located by name only; check this is the right place)  
Search for: `maxValue = max ( maxValue, ( int ) bm->SP_FISHERYprms[sp][i][paramIndex] );`

```c
 2648: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 2649: 		for (i = 0; i < bm->K_num_fisheries; i++) {
 2650→ 			maxValue = max ( maxValue, ( int ) bm->SP_FISHERYprms[sp][i][paramIndex] );
 2651: 		}
 2652: 	}
```

**Suggested fix (from review):** Track the raw max then return `max(1, raw_max + 1)`.

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

## MNG-21

**infinite loop** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManage.c` 2741, `atManage.c` 2727
**Lines at review (Oct 2026):** 2718-2733

**Function:** `GenerateCPUE()`

**Why it's thought to be a bug**

`while (this_shot < nshot) { step1 = drandom(0, max_prob); ... if (step1 <= sum_prob) {...this_shot++;} }` — if `Effort_hdistrib[..][today_effort]` is zero in all boxes (fleet idle/closed that day) then prop_effort is all 0, sum_prob stays 0 and no shot is ever placed, while nshot > 0 (it is computed from boat numbers, not effort) -> hang. Function: `GenerateCPUE()`.

**Code at issue** — `atManage.c` line 2741  
Search for: `if (step1 <= sum_prob) {`

```c
 2739:                     for (ij = 0; ij < bm->nbox; ij++) {
 2740:                         sum_prob += bm->prop_effort[ij];
 2741→                         if (step1 <= sum_prob) {
 2742:                             bm->shot_loc[this_shot] = ij;
 2743:                             bm->box_done[ij] = 1;
```

**Code at issue** — `atManage.c` line 2727 (closest match to the quoted code; wording differs slightly)  
Search for: `tot_effort += bm->Effort_hdistrib[ij][nf][today_effort];`

```c
 2725:                 for (ij = 0; ij < bm->nbox; ij++) {
 2726:                     bm->prop_effort[ij] += bm->Effort_hdistrib[ij][nf][today_effort];
 2727→                     tot_effort += bm->Effort_hdistrib[ij][nf][today_effort];
 2728:                 }
 2729:                 for (ij = 0; ij < bm->nbox; ij++) {
```

**Suggested fix (from review):** Skip shot generation when tot_effort == 0 (and guard against round-off by assigning the last box with effort).

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

## MNG-22

**buffer overrun** · Conf M / Sev H  
**Found at (current code):** `atManage.c` 2742, `atManage.c` 2894
**Lines at review (Oct 2026):** 2688-2704, 2727, 2742

**Function:** `GenerateCPUE()`

**Why it's thought to be a bug**

`nshot` is derived from boats × down_time × FishableLength × hr_per_dt/ShotLength and is not clamped to `max_poss_shots` (= K_max_num_subfleet × K_max_num_boats × ceil(24/K_min_shotlength) + nbox + 1, set at setup atManageSetup.c:1591); boat numbers can also grow under the econ model. `shot_loc[this_shot]` / `ShotData[this_shot]` can overrun. Function: `GenerateCPUE()`.

**Code at issue** — `atManage.c` line 2742  
Search for: `shot_loc[this_shot]`

```c
 2740:                         sum_prob += bm->prop_effort[ij];
 2741:                         if (step1 <= sum_prob) {
 2742→                             bm->shot_loc[this_shot] = ij;
 2743:                             bm->box_done[ij] = 1;
 2744:                             this_shot++;
```

**Code at issue** — `atManage.c` line 2894  
Search for: `ShotData[this_shot]`

```c
 2892:                         
 2893:                         // Store results
 2894→                         bm->ShotData[this_shot][sp][wdepth_id] = weighted_depth;
 2895:                         bm->ShotData[this_shot][sp][tdiscard_id] = final_shot_discards;
 2896:                         bm->ShotData[this_shot][sp][tcatch_id] = final_shot_catch;
```

**Suggested fix (from review):** Clamp `nshot` to `max_poss_shots - bm->nbox - 1`.

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

## MNG-23

**wrong operator** · Conf H / Sev M  
**Found at (current code):** `atManage.c` 2867
**Lines at review (Oct 2026):** 2852

**Function:** `GenerateCPUE()`

**Why it's thought to be a bug**

`bm->alloced_discard[sp][ij] -= final_shot_discards;` — alloced_discard is subtracted from available discards at 2808/2940, so it must increase as discards are allocated; decrementing makes available discards grow with each shot. Function: `GenerateCPUE()`.

**Code at issue** — `atManage.c` line 2867  
Search for: `alloced_discard[sp][ij] -= final_shot_discards`

```c
 2865:                         
 2866:                         // Allocated catch and discards per depth so don't get double counting - also gives average depth for reporting
 2867→                         bm->alloced_discard[sp][ij] -= final_shot_discards;
 2868:                         weighted_depth = 0.0;
 2869:                         catch_unalloced = final_shot_catch;
```

**Suggested fix (from review):** `+=`

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

## MNG-24

**accumulator / allocation logic** · Conf H / Sev M  
**Found at (current code):** `atManage.c` 2879
**Lines at review (Oct 2026):** 2855-2874, 2984-2986

**Why it's thought to be a bug**

`sum_prob` is reset only once before the while loop and the k loop has no `break`, so after the first pass every layer past the threshold (and on later passes all layers) is allocated; and `allocate_catch()` allocates the whole `catch_left` of a layer rather than `min(catch_left, *catch_unalloced)`, so a single small shot consumes all catch in a layer and catch_unalloced goes negative. Shot catches/depths and tot_alloced are wrong. Functions: `GenerateCPUE()`, `allocate_catch()`.

**Code at issue** — `atManage.c` line 2879 (located by name only; check this is the right place)  
Search for: `allocate_catch(bm, sp, nf, ij, k, &catch_unalloced, &weighted_depth, this_catch, llogfp);`

```c
 2877:                                 sum_prob += bm->prop_depth[k];
 2878:                                 if (step1 < sum_prob) {
 2879→                                     allocate_catch(bm, sp, nf, ij, k, &catch_unalloced, &weighted_depth, this_catch, llogfp);
 2880:                                 }
 2881:                             }
```

**Suggested fix (from review):** Reset sum_prob each iteration, break after allocating, and allocate `min(catch_left, *catch_unalloced)`.

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

## MNG-25

**wrong array / copy-paste** · Conf H / Sev M  
**Found at (current code):** `atManage.c` 3006
**Lines at review (Oct 2026):** 2992-2996

**Function:** `allocate_catch()`

**Why it's thought to be a bug**

`for (ij = 0; ij < bm->nbox; ij++) bm->prop_effort[ij] /= (new_sum + small_num);` — after reducing `prop_depth[k]` the code renormalises the box effort distribution instead of the depth distribution (and reuses parameter `ij` as loop counter). prop_effort is repeatedly inflated; prop_depth never renormalised. Function: `allocate_catch()`.

**Code at issue** — `atManage.c` line 3006  
Search for: `prop_depth[k]`

```c
 3004:         bm->prop_depth[k] -= prop_used;
 3005:         if (bm->prop_depth[k] < 0.0)
 3006→             bm->prop_depth[k] = 0.0;
 3007:         new_sum = 1.0 - prop_used;
 3008:         if (new_sum > 0.0) {
```

**Suggested fix (from review):** Loop `for (kk = 0; kk < bm->boxes[ij].nz; kk++) bm->prop_depth[kk] /= new_sum;`

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

## MNG-26

**unit mismatch** · Conf M / Sev M  
**Found at (current code):** `atManage.c` 2951
**Lines at review (Oct 2026):** 2936

**Function:** `GenerateCPUE()`

**Why it's thought to be a bug**

Left-over effort uses `Effort_hdistrib[..][today_effort] / 24.0` whereas shot effort uses `* 24.0` ("originally in days and want hours", 2897); also `alloced_effort` is never incremented, so the residual effort is the full effort, not the left-over. Function: `GenerateCPUE()`.

**Code at issue** — `atManage.c` line 2951 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->ShotData[this_shot][sp][teffort_id] += ((bm->Effort_hdistrib[ij][nf][today_effort] / 2`

```c
 2949:                         for (ij=0; ij < bm->nbox; ij++) {
 2950:                             bm->ShotData[this_shot][sp][tloc_id] = ij;
 2951→                             bm->ShotData[this_shot][sp][teffort_id] += ((bm->Effort_hdistrib[ij][nf][today_effort] / 24.0 ) - bm->alloced_effort[ij]);
 2952:                             for (k = 0; k < bm->boxes[ij].nz; k++) {
 2953:                                 bm->ShotData[this_shot][sp][wdepth_id] += (bm->Catch[ij][sp][nf][k] * bm->X_CN * mg_2_kg) - bm->alloced_catch[sp][ij][k];
```

**Suggested fix (from review):** Use `* 24.0` and update alloced_effort with final_shot_effort per box.

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

## MNG-27

**accumulator in wrong loop** · Conf H / Sev M  
**Found at (current code):** `atManage.c` 2951, `atManage.c` 2955
**Lines at review (Oct 2026):** 2937-2941

**Function:** `GenerateCPUE()`

**Why it's thought to be a bug**

`ShotData[...][tdiscard_id] += Discards[ij][sp][nf]*... - alloced_discard[sp][ij]` is inside the `k` (layer) loop, but Discards/alloced_discard are per box — discards are added nz times. Function: `GenerateCPUE()`.

**Code at issue** — `atManage.c` line 2951  
Search for: `ShotData[`

```c
 2949:                         for (ij=0; ij < bm->nbox; ij++) {
 2950:                             bm->ShotData[this_shot][sp][tloc_id] = ij;
 2951→                             bm->ShotData[this_shot][sp][teffort_id] += ((bm->Effort_hdistrib[ij][nf][today_effort] / 24.0 ) - bm->alloced_effort[ij]);
 2952:                             for (k = 0; k < bm->boxes[ij].nz; k++) {
 2953:                                 bm->ShotData[this_shot][sp][wdepth_id] += (bm->Catch[ij][sp][nf][k] * bm->X_CN * mg_2_kg) - bm->alloced_catch[sp][ij][k];
```

**Code at issue** — `atManage.c` line 2955 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->ShotData[this_shot][sp][tdiscard_id] += (bm->Discards[ij][sp][nf] * bm->X_CN * mg_2_kg`

```c
 2953:                                 bm->ShotData[this_shot][sp][wdepth_id] += (bm->Catch[ij][sp][nf][k] * bm->X_CN * mg_2_kg) - bm->alloced_catch[sp][ij][k];
 2954:                                 bm->ShotData[this_shot][sp][tcatch_id] += (bm->Catch[ij][sp][nf][k] * bm->X_CN * mg_2_kg) - bm->alloced_catch[sp][ij][k];
 2955→                                 bm->ShotData[this_shot][sp][tdiscard_id] += (bm->Discards[ij][sp][nf] * bm->X_CN * mg_2_kg) - bm->alloced_discard[sp][ij];
 2956:                             }
 2957:                         }
```

**Suggested fix (from review):** Move the tdiscard line outside the k loop.

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

## MNG-28

**wrong formula** · Conf M / Sev L  
**Found at (current code):** `atManage.c` 2958
**Lines at review (Oct 2026):** 2938, 2943

**Function:** `GenerateCPUE()`

**Why it's thought to be a bug**

Left-over `wdepth_id += catch` then `/= tcatch` gives ~1, not a catch-weighted depth (compare 2987 which multiplies by a depth term). Function: `GenerateCPUE()`.

**Code at issue** — `atManage.c` line 2958 (the line the review pointed to, found by its text)  
Search for: `bm->ShotData[this_shot][sp][wdepth_id] /= (bm->ShotData[this_shot][sp][tcatch_id] + small_`

```c
 2956:                             }
 2957:                         }
 2958→                         bm->ShotData[this_shot][sp][wdepth_id] /= (bm->ShotData[this_shot][sp][tcatch_id] + small_num);
 2959:                     }
 2960:                 }
```

**Suggested fix (from review):** Accumulate `depth(k) * catch_left`.

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

## MNG-29

**logic** · Conf M / Sev L  
**Found at (current code):** `atManage.c` 2832
**Lines at review (Oct 2026):** 2817-2830

**Function:** `GenerateCPUE()`

**Why it's thought to be a bug**

If `step1 >= cpue_cdf[sp][nc][gun]` for all bins (cdf not reaching 1), `this_catchbin` stays 0 giving zero catch instead of the top bin; bin 0 always yields `max_catch = 0`. Function: `GenerateCPUE()`.

**Code at issue** — `atManage.c` line 2832 (the line the review pointed to, found by its text)  
Search for: `for (nc = 0; nc < bm->K_num_catchbin; nc++){`

```c
 2830:                         sum_prob = 0.0;
 2831:                         this_catchbin = 0;
 2832→                         for (nc = 0; nc < bm->K_num_catchbin; nc++){
 2833:                             sum_prob = cpue_cdf[sp][nc][this_gun];   // Could use a calculated negative binomal cdf, but reading it in from pre-generated set for now (as cpue_cdf is cumualtive then set sum_prob = not += )
 2834:                             
```

**Suggested fix (from review):** Initialise `this_catchbin = K_num_catchbin - 1`.

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

atManage.c:2702 `tot_hold += hold_capacity` is not weighted by nboat while `prop_hold[ns]` is (2701), so prop_hold does not sum to 1; prop_hold appears unused at present.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atManage.c:3002 `weighted_depth += dz[k] * catch_left` uses layer thickness rather than layer depth.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-3

atManage.c:245-258 (catch-only branch): `bm->Effort` is never reset there, so `Effort[ij] *= F_rescale; Effort[new] += F_displaced` compounds every day.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-4

atManage.c:988 vs 943-946: when `manage_reg`, overages are set over K_num_active_reg regions but the closure count uses numStocks.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-5

atManage.c:2479-2481 quarter boundaries `nd <= 91` / `nd > 90` are inconsistent (harmless).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
