# atmanage/atManage.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **29 findings** (severity H: 8; confidence H and severity H: 6; on the priority list: 6), plus 5 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MNG-01](#mng-01) ⭐ | 161-164 | H/H | swapped indices | `bm->TAC_over[ij][sp][k] = 0` with `ij` looping over `K_num_fisheries` — TAC_over is allocated `Util_Alloc_Ini… |  | [ ] |
| [MNG-02](#mng-02) ⭐ | 971, 990, 1123, 1145, 2420 | H/H | buffer overrun | `bm->TAC_over[sp][fishery_id][nreg]` — third dimension is allocated with `K_num_basket` (atManageSetup.c:1260;… |  | [ ] |
| [MNG-03](#mng-03) 🔀 | 392-422 | H/M | uninitialised variable | `} else if (flagmanage > stock_adapt_mgmt)` — `flagmanage` is only assigned inside the `if (flagspeffortmodel … |  | [ ] |
| [MNG-04](#mng-04) 🔀 | ≈503 | H/M | wrong index | `mpa_scale = bm->MPA[bm->current_box][fishery_id];` inside `for (ij...)` loop — `current_box` is the ecology l… |  | [ ] |
| [MNG-05](#mng-05) | 566-568 | M/M | always-false condition | Outer `if (flagspeffortmodel == wgt_effort \|\| == dist_effort)` then inner `if (== array_effort \|\| == dist_effo… |  | [ ] |
| [MNG-06](#mng-06) 🔀 | 116-149 | M/M | ordering | `bm->CumDisplaceEffort[ij][nf] += bm->Effort[ij][nf];` runs after `bm->Effort[ij][nf] = 0.0` at line 123 (when… |  | [ ] |
| [MNG-07](#mng-07) 🔀 ⭐ | 245-257 | H/H | uninitialised variable | In the `!bm->EffortModelsActive` branch `Effort_Displacement(bm, ..., orig_FCpressure, &FCpressure, ...)` is c… |  | [ ] |
| [MNG-08](#mng-08) | 1141-1149 | H/M | wrong variable / copy-paste | `if (regtaccount == nstock)` in the common-pool regional closure check — `nstock` is stale (last species' valu… |  | [ ] |
| [MNG-09](#mng-09) | 960, 1099 | L/M | wrong operator | `if (!in_quota \|\| (!sp_reg && (nstock < 1)))` — comment says "Only include those groups under regional TACs wi… |  | [ ] |
| [MNG-10](#mng-10) | 1278-1285 | H/M | always-true condition | `if (flagpulsefish && (end_date < bm->dayt + 1.0))` inside `if (end_date < bm->dayt)` — always true, so the 1/… |  | [ ] |
| [MNG-11](#mng-11) | 1751-1752 | M/M | off-by-one / missing wrap | `bm->Effort_hdistrib[ij][fishery_id][bm->QofY + 1]` without the `QofY < 3` guard used in array_effort/rec_effo… |  | [ ] |
| [MNG-12](#mng-12) 🔀 ⭐ | ≈1922-1925 | H/H | inverted logic | `MPA_check = 1.0 - bm->MPA[nb][fishery_id]; fishthere *= MPA_check;` — bm->MPA is an access multiplier (1 = op… |  | [ ] |
| [MNG-13](#mng-13) ⭐ | 2082-2104 | H/H | buffer overrun | "Break loop" is done with `sp = bm->K_num_tot_sp;` but the inner `cohort`/`cIndex` loops keep running and imme… |  | [ ] |
| [MNG-14](#mng-14) | 2081-2085 | H/M | wrong box | `tracerArray = boxLayerInfo->localWCTracers;` — this is the tracer set of whatever box/layer the ecology loop … |  | [ ] |
| [MNG-15](#mng-15) | 2083 | M/H | cohort vs cohort×genotype | `for (cohort = 0; cohort < numCohortsXnumGenes; ...) contaminantTracers[cohort][cIndex]` — contaminantTracers … |  | [ ] |
| [MNG-16](#mng-16) | 2132-2158 | H/M | state not reset | `CAP_scale = 1.0;` is set once before the fishery loop; a fishery with `flagchangeP_id == 0` inherits the CAP_… |  | [ ] |
| [MNG-17](#mng-17) | 2225-2238 | H/M | state not reset | `double POP_scale = 1.0` initialised once; ports without `popchange_id` get the previous port's change scale a… |  | [ ] |
| [MNG-18](#mng-18) | 2532-2590 | M/M | logic / stale variable | Rescaling of BiTACamt is inside the per-companion loop, so it is applied once per OK companion (compounding `B… |  | [ ] |
| [MNG-19](#mng-19) | 2394 | L/L | wrong message argument | `quit("...more than 12 months in a year - %d\n", bm->BiM)` while switching on `bm->MofY`. Function: `Manage_Vi… |  | [ ] |
| [MNG-20](#mng-20) | 2630-2635 | L/L | off-by-one (doc vs code) | `if (val > maxValue) maxValue = val + 1;` — later fisheries are compared against the +1'd value, so e.g. value… |  | [ ] |
| [MNG-21](#mng-21) ⭐ | 2733-2748 | H/H | infinite loop | `while (this_shot < nshot) { step1 = drandom(0, max_prob); ... if (step1 <= sum_prob) {...this_shot++;} }` — i… |  | [ ] |
| [MNG-22](#mng-22) | 2703-2719, 2742, 2757 | M/H | buffer overrun | `nshot` is derived from boats × down_time × FishableLength × hr_per_dt/ShotLength and is not clamped to `max_p… |  | [ ] |
| [MNG-23](#mng-23) | 2867 | H/M | wrong operator | `bm->alloced_discard[sp][ij] -= final_shot_discards;` — alloced_discard is subtracted from available discards … |  | [ ] |
| [MNG-24](#mng-24) | 2870-2889, 2999-3001 | H/M | accumulator / allocation logic | `sum_prob` is reset only once before the while loop and the k loop has no `break`, so after the first pass eve… |  | [ ] |
| [MNG-25](#mng-25) | 3007-3011 | H/M | wrong array / copy-paste | `for (ij = 0; ij < bm->nbox; ij++) bm->prop_effort[ij] /= (new_sum + small_num);` — after reducing `prop_depth… |  | [ ] |
| [MNG-26](#mng-26) | 2951 | M/M | unit mismatch | Left-over effort uses `Effort_hdistrib[..][today_effort] / 24.0` whereas shot effort uses `* 24.0` ("originall… |  | [ ] |
| [MNG-27](#mng-27) | 2952-2956 | H/M | accumulator in wrong loop | `ShotData[...][tdiscard_id] += Discards[ij][sp][nf]*... - alloced_discard[sp][ij]` is inside the `k` (layer) l… |  | [ ] |
| [MNG-28](#mng-28) | 2953, 2958 | M/L | wrong formula | Left-over `wdepth_id += catch` then `/= tcatch` gives ~1, not a catch-weighted depth (compare 2987 which multi… |  | [ ] |
| [MNG-29](#mng-29) | 2832-2845 | M/L | logic | If `step1 >= cpue_cdf[sp][nc][gun]` for all bins (cdf not reaching 1), `this_catchbin` stays 0 giving zero cat… |  | [ ] |

---

## MNG-01

**swapped indices** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 161-164  
**Lines at review (Oct 2026):** 160-163

**Function:** `Manage_Calculate_Total_Effort()`

**Why it's thought to be a bug**

`bm->TAC_over[ij][sp][k] = 0` with `ij` looping over `K_num_fisheries` — TAC_over is allocated `Util_Alloc_Init_3D_Int(K_num_basket, K_num_fisheries, K_num_tot_sp)` (atManageSetup.c:1260) => `[sp][nf][basket]`, and every other use is `TAC_over[sp][nf][..]` (lines 830, 871, 971, atHarvestDiscards.c:248). Here sp indexes the fishery dimension (size K_num_fisheries) -> out-of-bounds writes whenever K_num_tot_sp > K_num_fisheries, and the real flags are never reset so TAC-over discarding/closures are sticky. Function: `Manage_Calculate_Total_Effort()`.

**Current code** (`atmanage/atManage.c` lines 159–166)

```c
  159:     for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  160:         if (FunctGroupArray[sp].isTAC) {
  161:             for (ij = 0; ij < bm->K_num_fisheries; ij++) {
  162:                 for (k = 0; k < bm->K_num_basket; k++) {
  163: 					bm->TAC_over[ij][sp][k] = 0;
  164:                 }
  165:             }
  166:         }
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
**Lines (current code):** 971, 990, 1123, 1145, 2420  
**Lines at review (Oct 2026):** 959, 978, 1111, 1133, 2405

**Why it's thought to be a bug**

`bm->TAC_over[sp][fishery_id][nreg]` — third dimension is allocated with `K_num_basket` (atManageSetup.c:1260; K_num_basket = max basket size, usually 1, atManageSetup.c:1291-1297) but is indexed by region/stock `nreg` up to `K_num_active_reg` / `numStocks`. Any regional-TAC config with >1 region/stock writes/reads past the end (also atHarvestDiscards.c:248 reads `[this_stock]`). Functions: `TAC_Check()`, `Manage_Visit_Council()`.

**Current code** (`atmanage/atManage.c` lines 969–992)

```c
  969: 						TAC = (regTAC_scale * bm->TACamt[sp][fishery_id][now_id] * kg_2_mg) / bm->X_CN;
  970: 						if(((totCumCatch + bm->TotOldCumCatch[sp][fishery_id]) * regC_scale) > TAC){
  971: 							bm->TAC_over[sp][fishery_id][nreg] = 1;
  972: 
  973:                             /*
  974: 							if (do_debug) {
  975: 								fprintf(llogfp, "time: %e, Cumulative regional catch of %s (%e) is over TAC (%e) for fishery %s in region %d\n", bm->dayt,
  976: 										FunctGroupArray[sp].groupCode, ((totCumCatch + bm->TotOldCumCatch[sp][fishery_id]) * regC_scale), TAC, FisheryArray[fishery_id].fisheryCode, nreg);
  977: 
  978: 							}
  979:                             */
  980: 						}
  981: 					}
  982: 				}
  983: 			}
  984: 			/* Check for complete fishery closure everywhere */
  985: 			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  986: 				if (FunctGroupArray[sp].isTAC) {
  987: 					regtaccount = 0;
  988: 					nstock = FunctGroupArray[sp].numStocks;
  989: 					for (nreg = 0; nreg < nstock; nreg++) {
  990: 						if (bm->TAC_over[sp][fishery_id][nreg])
  991: 							regtaccount++;
  992: 					}
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
**Lines (current code):** 392-422  
**Lines at review (Oct 2026):** 387-417

**Function:** `Manage_Calculate_Total_Effort()`

**Why it's thought to be a bug**

`} else if (flagmanage > stock_adapt_mgmt)` — `flagmanage` is only assigned inside the `if (flagspeffortmodel != readts_effort)` branch (line 395); for read-ts effort fisheries it is uninitialised (first fishery) or stale from a previous fishery, so TAC_Check (and TAC_over discards) runs/does not run for the wrong fisheries. Function: `Manage_Calculate_Total_Effort()`.

> 🔀 **Changed by the merge of main (Nov 2026):** code shown above was changed by the merge. Check whether this finding is now fixed before acting.

**Current code** (`atmanage/atManage.c` lines 390–424)

```c
  390:             active_scale = 1.0;
  391:         
  392:         if (flagspeffortmodel != readts_effort) {
  393: 
  394:             /* Effort management */
  395: 			flagmanage = (int) (bm->FISHERYprms[fishery_id][flagmanage_id]);
  396: 			EFF_scale0 = Effort_Restrict_Check(bm, fishery_id, flagmanage, &trigger_tripped, llogfp);
  397:             
  398:             /* TAC-based adjustment to effort */
  399: 			EFF_scale1 = TAC_Check(bm, fishery_id, flagmanage, llogfp);
  400:             
  401:             /* Protected-endangered-threatened species management */
  402: 			EFF_scale2 = Endangered_Check(bm, fishery_id, EFF_scale1, &end_trigger_tripped, llogfp);
  403:             
  404:             /* If general change in fishing pressure through time (gradual change) calculate
  405: 			 new levels here. */
  406: 			EFF_scale3 = Forced_Effort_Change(bm, fishery_id, flagspeffortmodel, llogfp);
  407:             
  408:             /* If recreational fishery determine human population actually fishing */
  409:             if(FisheryArray[fishery_id].isRec) {
  410: 				prop_pop_fish = k_proprecfish;
  411:             } else {
  412: 				prop_pop_fish = 1.0;
  413:             }
  414: 
  415:             /* Before do spatially explicit management identify active MPAs - moved earlier so available even if not using dynamic effort
  416: 			if (bm->dayt != bm->predayt)
  417: 				Check_For_Active_MPA(bm, fishery_id);
  418:              */
  419:         } else if (flagmanage > stock_adapt_mgmt){
  420:             /* TAC-check needed so discard if over TAC amount - dummy not used but TAC_over calculated within the routine is */
  421:             dummy = TAC_Check(bm, fishery_id, flagmanage, llogfp);
  422:         }
  423: 
  424:         /* Check port status and contribution */
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
**Lines (current code):** ≈503  
**Lines at review (Oct 2026):** 498

**Function:** `Manage_Calculate_Total_Effort()`

**Why it's thought to be a bug**

`mpa_scale = bm->MPA[bm->current_box][fishery_id];` inside `for (ij...)` loop — `current_box` is the ecology loop's stale global box, not `ij`; every box gets the MPA scalar of one arbitrary box. Compare line 459 `bm->MPA[ij][fishery_id]`. Function: `Manage_Calculate_Total_Effort()`.

> 🔀 **Changed by the merge of main (Nov 2026):** MPA[bm->current_box] -> MPA[ij] (JMK fix). Check whether this finding is now fixed before acting.

**Current code** (`atmanage/atManage.c` lines 501–505) — ≈ the code here has changed since the review; lines are approximate

```c
  501:                     flagfcmpa = (int) (bm->FISHERYprms[fishery_id][flagmpa_id]);
  502:                     if (flagfcmpa) {
  503:                     	mpa_scale = bm->MPA[ij][fishery_id]; /*jmk fix, from mpa_scale = bm->MPA[bm->current_box][fishery_id];*/
  504:                         
  505:                         /* Allow for infringement */
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
**Lines (current code):** 566-568  
**Lines at review (Oct 2026):** 561-563

**Function:** `Manage_Calculate_Total_Effort()`

**Why it's thought to be a bug**

Outer `if (flagspeffortmodel == wgt_effort || == dist_effort)` then inner `if (== array_effort || == dist_effort)` — array_effort can never be true inside, and wgt_effort never passes the inner test, so effort buffering is never applied to wgt_effort (and array_effort is never reached). One of the two lists is a copy-paste error. Function: `Manage_Calculate_Total_Effort()`.

**Current code** (`atmanage/atManage.c` lines 564–570)

```c
  564:         
  565: 		flagspeffortmodel = (int) (bm->FISHERYprms[fishery_id][flageffortmodel_id]);
  566: 		if (flagspeffortmodel == wgt_effort || flagspeffortmodel == dist_effort) {
  567: 			if (bm->flagbuffereffort) {
  568: 				if (flagspeffortmodel == array_effort || flagspeffortmodel == dist_effort) {
  569: 					if (scale_effort[fishery_id] > 1.0) {
  570: 						if (bm->totNewEffort[fishery_id] > scale_effort[fishery_id] * bm->totOldEffort[fishery_id]) {
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
**Lines (current code):** 116-149  
**Lines at review (Oct 2026):** 116-148

**Function:** `Manage_Calculate_Total_Effort()`

**Why it's thought to be a bug**

`bm->CumDisplaceEffort[ij][nf] += bm->Effort[ij][nf];` runs after `bm->Effort[ij][nf] = 0.0` at line 123 (when EffortModelsActive), so the displaced-effort record (written out in atHarvestIO.c:570) only ever accumulates zeros (or the 1.0 reset value). Function: `Manage_Calculate_Total_Effort()`.

> 🔀 **Changed by the merge of main (Nov 2026):** CumDisplaceEffort accumulation moved. Check whether this finding is now fixed before acting.

**Current code** (`atmanage/atManage.c` lines 114–151)

```c
  114:     
  115:     /* Initialise local arrays */
  116:     if(bm->EffortModelsActive) {
  117:         for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  118:             Harvest_Set_Harvest_Index(bm, nf, checkdone_id, 0);
  119:             for (ij = 0; ij < ncells; ij++) {
  120:                 bm->CumEffort[nf][ij] += bm->Effort[ij][nf];
  121:                 bm->OldEffort[ij][nf] = bm->Effort[ij][nf];
  122:                 bm->totOldEffort[nf] += bm->OldEffort[ij][nf];
  123:                 bm->Effort[ij][nf] = 0.0;
  124:                 bm->TempCPUE[ij][nf] = 0.0;
  125:                 
  126:                 /*
  127:                  if (do_debug && (nf == bm->which_fleet)) {
  128:                  fprintf(llogfp, "Time: %e, %s-%d, effort: %e (oldeffort: %e with totOldEffort: %e)\n", bm->dayt, FisheryArray[nf].fisheryCode, ij,
  129:                  bm->Effort[ij][nf], bm->OldEffort[ij][nf], bm->totOldEffort[nf]);
  130:                  }
  131:                  */
  132:                 
  133:                 
  134:             }
  135:             scale_effort[nf] = 0;
  136:             bm->totCPUE[nf] = 0;
  137:         }
  138:     }
  139:     
  140:     // If displaceing effort need to clean up the record keeping
  141:     if (bm->flagdisplace) {
  142:         for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  143:             for (ij = 0; ij < ncells; ij++) {
  144:                 if(bm->newmonth && bm->flagday) {
  145:                     bm->CumDisplaceEffort[ij][nf] = 0.0;
  146: 	    	        }
  147: 	        	    if(!bm->Effort[ij][nf] && (bm->FISHERYprms[nf][flagMFCdisplace_id])){ 
  148: 	            	    bm->Effort[ij][nf] = 1.0;
  149:                 }
  150:                 if(!bm->Effort[ij][nf] && (bm->FISHERYprms[nf][flagMFCdisplace_id])){
  151:                     bm->Effort[ij][nf] = 1.0;  // As used a scalar in mFC fisheries
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
**Lines (current code):** 245-257  
**Lines at review (Oct 2026):** 245-257

**Function:** `Manage_Calculate_Total_Effort()`

**Why it's thought to be a bug**

In the `!bm->EffortModelsActive` branch `Effort_Displacement(bm, ..., orig_FCpressure, &FCpressure, ...)` is called with `FCpressure` never initialised (declared line 86); Effort_Displacement reads `*FCpressure` (line 1937-1938) and sets `*FCdisplaced = orig_FCpressure - *FCpressure`, so F_rescale/F_displaced applied to `bm->Effort` are garbage. Function: `Manage_Calculate_Total_Effort()`.

> 🔀 **Changed by the merge of main (Nov 2026):** orig_FCpressure now set before the branch. Check whether this finding is now fixed before acting.

**Current code** (`atmanage/atManage.c` lines 243–259)

```c
  243:                 Check_For_Active_MPA(bm, fishery_id);
  244:                 if (bm->flagdisplace) {
  245:                     for (ij = 0; ij < bm->nbox; ij++) {
  246:                         orig_FCpressure = 1.0;
  247:                         Effort_Displacement(bm, fishery_id, ij, orig_FCpressure, &FCpressure, &FCdisplaced, &new_fish_loc, llogfp);
  248:                         if (FCdisplaced < 0.0)
  249:                             FCdisplaced = 0.0;
  250:                         
  251:                         /* Find the proportion retained and displaced */
  252:                         F_rescale = 1.0 - (FCdisplaced / orig_FCpressure);
  253:                         F_displaced = FCdisplaced / orig_FCpressure;
  254:                         
  255:                         /* Updating scalars - overload via the use of effort array */
  256:                         bm->Effort[ij][fishery_id] *= F_rescale; /* this is so that accumulate additional fishing mortakity pressure due to displaced effort */
  257:                         bm->Effort[new_fish_loc][fishery_id] += F_displaced;
  258:                     }
  259: 
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
**Lines (current code):** 1141-1149  
**Lines at review (Oct 2026):** 1129-1137

**Function:** `TAC_Check()`

**Why it's thought to be a bug**

`if (regtaccount == nstock)` in the common-pool regional closure check — `nstock` is stale (last species' value from the loop at 1077-1127, possibly K_num_active_reg) while the count loop runs to `FunctGroupArray[sp].numStocks`. The non-common-pool version (988-993) correctly resets `nstock = numStocks`. Fishery closure decision is wrong when species have different stock numbers. Function: `TAC_Check()`.

**Current code** (`atmanage/atManage.c` lines 1139–1151)

```c
 1139: 			}
 1140: 			/* Check for complete fishery closure everywhere */
 1141: 			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 1142: 				if (FunctGroupArray[sp].isTAC) {
 1143: 					regtaccount = 0;
 1144: 					for (nreg = 0; nreg < FunctGroupArray[sp].numStocks; nreg++) {
 1145: 						if (bm->TAC_over[sp][fishery_id][nreg])
 1146: 							regtaccount++;
 1147: 					}
 1148: 					if (regtaccount == nstock) {
 1149: 						taccount++;
 1150: 					}
 1151: 				}
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
**Lines (current code):** 960, 1099  
**Lines at review (Oct 2026):** 948, 1087

**Function:** `TAC_Check()`

**Why it's thought to be a bug**

`if (!in_quota || (!sp_reg && (nstock < 1)))` — comment says "Only include those groups under regional TACs with separate stocks", but with `&&` non-regional species (sp_reg = 0) are still processed whenever nstock >= 1 (always), so speciesParams[regionalSP_id] has no effect. Function: `TAC_Check()`.

**Current code** (`atmanage/atManage.c` lines 958–962)

```c
  958: 						nstock = FunctGroupArray[sp].numStocks;
  959: 
  960: 					if (!in_quota || (!sp_reg && (nstock < 1))) /* Only include those groups under regional TACs with seperate stocks */
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
**Lines (current code):** 1278-1285  
**Lines at review (Oct 2026):** 1266-1273

**Function:** `Forced_Effort_Change()`

**Why it's thought to be a bug**

`if (flagpulsefish && (end_date < bm->dayt + 1.0))` inside `if (end_date < bm->dayt)` — always true, so the 1/EFFmultA "undo pulse" correction (meant only for the first day after the pulse) is applied every timestep forever, compounding on previous-effort-dependent models. Function: `Forced_Effort_Change()`.

**Current code** (`atmanage/atManage.c` lines 1276–1287)

```c
 1276: 		 each little change in effort is cumulative */
 1277: 		if (flag_use_preveffortmodel) {
 1278: 			if (end_date < bm->dayt) {
 1279: 				/* Change over */
 1280: 				flagpulsefish = (int) (EFFORTchange[fishery_id][now_change][pulse_id]);
 1281: 				if (flagpulsefish && (end_date < bm->dayt + 1.0)) {
 1282: 					/* If only pulse of change, divide by level of change to return to
 1283: 					 normal for fisheries dependent on previous effort levels - correction only needed for the first day after the end of the pulse */
 1284: 					EFF_scale3 = 1.0 / EFFmultA;
 1285: 				} else {
 1286: 					/* Otherwise change is complete no more scaling necessary */
 1287: 					EFF_scale3 = 1.0;
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
**Lines (current code):** 1751-1752  
**Lines at review (Oct 2026):** 1739-1740

**Function:** `Allocate_Immediate_Effort()`

**Why it's thought to be a bug**

`bm->Effort_hdistrib[ij][fishery_id][bm->QofY + 1]` without the `QofY < 3` guard used in array_effort/rec_effort (1463-1468, 1548-1553); in Q4 it reads entry 4 (unused quarterly slot, normally 0, or a monthly econ value) instead of wrapping to 0. Function: `Allocate_Immediate_Effort()` (ideal_distrib_effort).

**Current code** (`atmanage/atManage.c` lines 1749–1754)

```c
 1749: 			/* If no target left then may be targeting by-product so distrbute based on perscribed distributions */
 1750: 			if (!totFCweight) {
 1751: 				FCweight = (bm->HowFar * (bm->Effort_hdistrib[ij][fishery_id][bm->QofY + 1] - bm->Effort_hdistrib[ij][fishery_id][bm->QofY])
 1752: 						+ bm->Effort_hdistrib[ij][fishery_id][bm->QofY]);
 1753: 			}
 1754: 
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
**Lines (current code):** ≈1922-1925  
**Lines at review (Oct 2026):** 1894-1897

**Function:** `Effort_Displacement()`

**Why it's thought to be a bug**

`MPA_check = 1.0 - bm->MPA[nb][fishery_id]; fishthere *= MPA_check;` — bm->MPA is an access multiplier (1 = open, applied as `FCpressure * MPA` at 454; default 1.0, atManageSetup.c:1219). Using 1-MPA zeroes open neighbours and gives full weight to closed ones, so displaced effort is sent INTO closed areas. Function: `Effort_Displacement()`.

> 🔀 **Changed by the merge of main (Nov 2026):** MPA_check no longer inverted. Check whether this finding is now fixed before acting.

**Current code** (`atmanage/atManage.c` lines 1920–1927) — ≈ the code here has changed since the review; lines are approximate

```c
 1920: 			if(debug){
 1921: 				fprintf(bm->logFile, "Effort_Displacement fishery %s, box %d, maxfishthere = %e, fishthere = %e, FCpressure = %e, new_fish_loc = %d, mEff_thresh = %e\n",
 1922: 						FisheryArray[fishery_id].fisheryCode, ij, maxfishthere, fishthere, *FCpressure, *new_fish_loc, mEff_thresh);
 1923: 
 1924: 			}
 1925: 			*/
 1926: 			/* If stock in an adjacent box is better than in the current box displace
 1927: 			 some effort to that location */
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
**Lines (current code):** 2082-2104  
**Lines at review (Oct 2026):** 2067-2089

**Function:** `Check_For_Active_MPA()`

**Why it's thought to be a bug**

"Break loop" is done with `sp = bm->K_num_tot_sp;` but the inner `cohort`/`cIndex` loops keep running and immediately evaluate `FunctGroupArray[sp].numCohortsXnumGenes` / `FunctGroupArray[sp].contaminantTracers[cohort][cIndex]` with sp == K_num_tot_sp — read past FunctGroupArray. Function: `Check_For_Active_MPA()`.

**Current code** (`atmanage/atManage.c` lines 2080–2106)

```c
 2080:                 case conc_based: // Based on concentration in species
 2081:                     tracerArray = boxLayerInfo->localWCTracers;
 2082:                     for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 2083:                         for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
 2084:                             for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
 2085:                                 cGroupLevel = tracerArray[FunctGroupArray[sp].contaminantTracers[cohort][cIndex]];
 2086:                                 
 2087:                                 thresh_level = bm->contaminantStructure[cIndex]->fishery_thresh_level;
 2088:                                 if (cGroupLevel > thresh_level) {
 2089:                                     if (!bm->contam_fishery_closure_option) {
 2090:                                         bm->MPA[ij][fishery_id] = 0.0;  // Close only the immediate box
 2091:                                     } else { // Close immediate box and adjactent boxes
 2092:                                         bm->MPA[ij][fishery_id] = 0.0;
 2093:                                         for (ad = 0; ad < bm->boxes[ij].nconn; ad++) {
 2094:                                             adbox = bm->boxes[ij].ibox[ad];
 2095:                                             bm->MPA[adbox][fishery_id] = 0.0;
 2096:                                         }
 2097:                                     }
 2098:                                     
 2099:                                     // Break loop as already closed now
 2100:                                     sp = bm->K_num_tot_sp;
 2101:                                 }
 2102:                             }
 2103:                         }
 2104:                     }
 2105:                     break;
 2106:                 default:
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
**Lines (current code):** 2081-2085  
**Lines at review (Oct 2026):** 2066-2070

**Function:** `Check_For_Active_MPA()`

**Why it's thought to be a bug**

`tracerArray = boxLayerInfo->localWCTracers;` — this is the tracer set of whatever box/layer the ecology loop last processed, not box `ij`; every box's closure is decided on one box's concentrations (and epibenthic groups' contaminantTracers index the epi tracer list, not WC). Function: `Check_For_Active_MPA()`.

**Current code** (`atmanage/atManage.c` lines 2079–2087)

```c
 2079:                     break;
 2080:                 case conc_based: // Based on concentration in species
 2081:                     tracerArray = boxLayerInfo->localWCTracers;
 2082:                     for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 2083:                         for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
 2084:                             for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
 2085:                                 cGroupLevel = tracerArray[FunctGroupArray[sp].contaminantTracers[cohort][cIndex]];
 2086:                                 
 2087:                                 thresh_level = bm->contaminantStructure[cIndex]->fishery_thresh_level;
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
**Lines (current code):** 2083  
**Lines at review (Oct 2026):** 2068

**Function:** `Check_For_Active_MPA()`

**Why it's thought to be a bug**

`for (cohort = 0; cohort < numCohortsXnumGenes; ...) contaminantTracers[cohort][cIndex]` — contaminantTracers is allocated `[numCohorts][num_contaminants]` (atUtilGroupIO.c:909); with numGenes > 1 this reads past the array. Function: `Check_For_Active_MPA()`.

**Current code** (`atmanage/atManage.c` lines 2081–2085)

```c
 2081:                     tracerArray = boxLayerInfo->localWCTracers;
 2082:                     for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 2083:                         for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
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
**Lines (current code):** 2132-2158  
**Lines at review (Oct 2026):** 2117-2143

**Function:** `Check_CAP()`

**Why it's thought to be a bug**

`CAP_scale = 1.0;` is set once before the fishery loop; a fishery with `flagchangeP_id == 0` inherits the CAP_scale computed for an earlier fishery. Function: `Check_CAP()`.

**Current code** (`atmanage/atManage.c` lines 2130–2160)

```c
 2130:     */
 2131: 
 2132: 	CAP_scale = 1.0;
 2133: 	for (fishery_id = 0; fishery_id < bm->K_num_fisheries; fishery_id++) {
 2134: 
 2135: 		/*if (do_debug_base && ((bm->which_fleet == fishery_id) || (bm->which_fleet >= bm->K_num_fisheries))) {
 2136: 			do_debug = 1;
 2137: 		} else {
 2138: 			do_debug = 0;
 2139:          }
 2140:         */
 2141: 
 2142: 		cap = bm->FISHERYprms[fishery_id][cap_id];
 2143: 		flagcap = (int) (bm->FISHERYprms[fishery_id][flagcap_id]);
 2144: 		flagspeffortmodel = (int) (bm->FISHERYprms[fishery_id][flageffortmodel_id]);
 2145: 
 2146: 		/* Only finish the check if caps in place */
 2147: 		if (!flagcap)
 2148: 			continue;
 2149: 
 2150: 		flagchangeCAP = (int) (bm->FISHERYprms[fishery_id][flagchangeP_id]);
 2151: 		CAP_num_changes = (int) (bm->FISHERYprms[fishery_id][P_num_changes_id]);
 2152: 
 2153: 		if (flagchangeCAP) {
 2154: 			CAP_scale = Util_Get_Change_Scale(bm, CAP_num_changes, CAPchange[fishery_id]);
 2155: 
 2156: 		}
 2157: 
 2158: 		cap *= CAP_scale;
 2159: 
 2160: 		/* Total shots to date */
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
**Lines (current code):** 2225-2238  
**Lines at review (Oct 2026):** 2210-2223

**Function:** `Update_Port_Population()`

**Why it's thought to be a bug**

`double POP_scale = 1.0` initialised once; ports without `popchange_id` get the previous port's change scale applied to their population. Function: `Update_Port_Population()`.

**Current code** (`atmanage/atManage.c` lines 2223–2240)

```c
 2223: void Update_Port_Population(MSEBoxModel *bm, FILE *llogfp) {
 2224: 	int porti, flagchangePOP, POP_num_changes;
 2225: 	double POP_scale = 1.0, start_pop;
 2226: 
 2227: 	for (porti = 0; porti < bm->K_num_ports; porti++) {
 2228: 		/* No check if active here as needn't be a commerical port to have recreational fishing pressure */
 2229: 
 2230: 		/* Get population of port fishing and scale it for growth if necessary */
 2231: 		start_pop = bm->Port_info[porti][population_id];
 2232: 		flagchangePOP = (int) (bm->Port_info[porti][popchange_id]);
 2233: 		POP_num_changes = (int) (bm->Port_info[porti][POP_num_changes_id]);
 2234: 
 2235: 		if (flagchangePOP) {
 2236: 			POP_scale = Util_Get_Change_Scale(bm, POP_num_changes, bm->POPchange[porti]);
 2237: 		}
 2238: 		bm->Port_info[porti][popactive_id] = start_pop * POP_scale;
 2239: 	}
 2240: 
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
**Lines (current code):** 2532-2590  
**Lines at review (Oct 2026):** 2517-2575

**Function:** `Manage_Visit_Council()`

**Why it's thought to be a bug**

Rescaling of BiTACamt is inside the per-companion loop, so it is applied once per OK companion (compounding `BiTACamt *= 1/catch_ratio` each time) and never if the species has no companions; also `prop_yr_changed = 1 - implement_BiM/6.0` (2554) uses implement_BiM from a previous species/fishery since it is only set at 2557. Comment says rescale "only if companion species would be worse off, so check companions" — i.e. check all then rescale once. Function: `Manage_Visit_Council()`.

**Current code** (`atmanage/atManage.c` lines 2530–2569 (truncated))

```c
 2530: 						/* Rescaling necessary - but only if companion species would be worse off, so check companions.*/
 2531:                         
 2532:                         for (co_sp = 0; co_sp < FunctGroupArray[sp].speciesParams[max_co_sp_id]; co_sp++ ) {
 2533:                             co_sp2 = FunctGroupArray[sp].co_sp[co_sp];
 2534:                             
 2535:                             if ((co_sp2 < 0) || (co_sp2 > bm->K_max_impacted_sp) || (FunctGroupArray[co_sp2].isFished == FALSE)) {
 2536:                                 continue;
 2537:                             }
 2538: 
 2539:                             regTAC_scale_cosp = bm->RegionalData[co_sp2][nreg][reg_tac_id];
 2540: 							regC_scale_cosp = bm->RegionalData[co_sp2][nreg][reg_catch_id];
 2541: 							TAC_cosp = 0;
 2542:                             CumCatch_cosp = 0.0;
 2543:                             companion_ok = 1;
 2544:                             
 2545: 							for (nf = 0; nf < bm->K_num_fisheries; nf++) {
 2546: 								flagTACparticipate = (int)(bm->FISHERYprms[nf][flagTACpartipcate_id]);
 2547: 								if (flagTACparticipate) {
 2548: 									/* Convert from tonnes wet weight to mg N */
 2549: 									TAC_cosp += (regTAC_scale_cosp * bm->TACamt[co_sp2][nf][now_id] * kg_2_mg) / bm->X_CN;
 2550:                                     CumCatch_cosp += (Harvest_Get_TotCumCatch(co_sp2, nf, bm->thisyear)  + bm->TotOldCumCatch[co_sp2][nf]) * regC_scale_cosp;
 2551: 								}
 2552: 							}
 2553: 							/* Check if scaled up CumCatch for companions would exceed TAC */
 2554: 							prop_yr_changed = (1.0 - implement_BiM / 6.0);
 2555: 							CumCatch_cosp *= 1.0 + ((1 / (catch_ratio + small_num)) * prop_yr_changed);
 2556: 							catch_ratio_cosp = CumCatch_cosp / (TAC_cosp + small_num);
 2557: 
 2558:                             if (catch_ratio_cosp > 1.0) {
 2559:                                 companion_ok = 0;
 2560:                             }
 2561: 
 2562: 
 2563:                             /* Do rescaling */
 2564:                             if (companion_ok) {
 2565:                                 min_delay = MAXINT;
 2566:                                 for (nf = 0; nf < bm->K_num_fisheries; nf++) {
 2567:                                     /* Political process delay vs bimonthly council metting steps _ to see when action implemented */
 2568:                                     FCperiod = bm->FISHERYprms[nf][FC_period_id];
 2569:                                     delay = (int) (ceil(FCperiod / 60.0));
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
**Lines (current code):** 2394  
**Lines at review (Oct 2026):** 2379

**Function:** `Manage_Visit_Council()`

**Why it's thought to be a bug**

`quit("...more than 12 months in a year - %d\n", bm->BiM)` while switching on `bm->MofY`. Function: `Manage_Visit_Council()`.

**Current code** (`atmanage/atManage.c` lines 2392–2396)

```c
 2392: 			break;
 2393: 		default:
 2394: 			quit("How the hell did you get more than 12 months in a year - %d\n", bm->BiM);
 2395: 			break;
 2396: 		}
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
**Lines (current code):** 2630-2635  
**Lines at review (Oct 2026):** 2615-2620

**Function:** `Manage_Get_Max_Fishery_Param()`

**Why it's thought to be a bug**

`if (val > maxValue) maxValue = val + 1;` — later fisheries are compared against the +1'd value, so e.g. values {3,4} return 4 not 5 as documented. Callers add +1 so no overrun found, but the function does not return what it claims; Species version (2644) never adds +1. Function: `Manage_Get_Max_Fishery_Param()`.

**Current code** (`atmanage/atManage.c` lines 2628–2637)

```c
 2628: int Manage_Get_Max_Fishery_Param(MSEBoxModel *bm, int paramIndex) {
 2629: 	int i;
 2630: 	int maxValue = 1;
 2631: 
 2632: 	for (i = 0; i < bm->K_num_fisheries; i++) {
 2633: 		if (bm->FISHERYprms[i][paramIndex] > maxValue)
 2634: 			maxValue = (int) (bm->FISHERYprms[i][paramIndex] + 1);
 2635: 	}
 2636: 	return maxValue;
 2637: 
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
**Lines (current code):** 2733-2748  
**Lines at review (Oct 2026):** 2718-2733

**Function:** `GenerateCPUE()`

**Why it's thought to be a bug**

`while (this_shot < nshot) { step1 = drandom(0, max_prob); ... if (step1 <= sum_prob) {...this_shot++;} }` — if `Effort_hdistrib[..][today_effort]` is zero in all boxes (fleet idle/closed that day) then prop_effort is all 0, sum_prob stays 0 and no shot is ever placed, while nshot > 0 (it is computed from boat numbers, not effort) -> hang. Function: `GenerateCPUE()`.

**Current code** (`atmanage/atManage.c` lines 2731–2750)

```c
 2731:                 }
 2732:                 
 2733:                 this_shot = 0;
 2734:                 max_prob = 1.0;
 2735:                 Util_Init_1D_Int(bm->shot_loc, max_poss_shots, 0);
 2736:                 while (this_shot < nshot) {
 2737:                     step1 = drandom(0.0, max_prob);
 2738:                     sum_prob = 0.0;
 2739:                     for (ij = 0; ij < bm->nbox; ij++) {
 2740:                         sum_prob += bm->prop_effort[ij];
 2741:                         if (step1 <= sum_prob) {
 2742:                             bm->shot_loc[this_shot] = ij;
 2743:                             bm->box_done[ij] = 1;
 2744:                             this_shot++;
 2745:                             break;
 2746:                         }
 2747:                     }
 2748:                 }
 2749:                 // Make sure all the boxes with catch are included - if not pad out the list now
 2750:                 for (ij = 0; ij < bm->nbox; ij++) {
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
**Lines (current code):** 2703-2719, 2742, 2757  
**Lines at review (Oct 2026):** 2688-2704, 2727, 2742

**Function:** `GenerateCPUE()`

**Why it's thought to be a bug**

`nshot` is derived from boats × down_time × FishableLength × hr_per_dt/ShotLength and is not clamped to `max_poss_shots` (= K_max_num_subfleet × K_max_num_boats × ceil(24/K_min_shotlength) + nbox + 1, set at setup atManageSetup.c:1591); boat numbers can also grow under the econ model. `shot_loc[this_shot]` / `ShotData[this_shot]` can overrun. Function: `GenerateCPUE()`.

**Current code** (`atmanage/atManage.c` lines 2701–2740 (truncated))

```c
 2701:                     bm->prop_hold[ns] += bm->SUBFLEET_ECONprms[nf][ns][hold_capacity_id] * bm->SUBFLEET_ECONprms[nf][ns][nboat_id];
 2702:                     tot_hold += bm->SUBFLEET_ECONprms[nf][ns][hold_capacity_id];
 2703:                     min_shots += bm->SUBFLEET_ECONprms[nf][ns][nboat_id] * bm->SUBFLEET_ECONprms[nf][ns][down_time_id] * bm->SUBFLEET_ECONprms[nf][ns][FishableLength_id] ;  // So a minimum of one shot each for active vessels
 2704:                     max_shots += bm->SUBFLEET_ECONprms[nf][ns][nboat_id] * bm->SUBFLEET_ECONprms[nf][ns][down_time_id] * bm->SUBFLEET_ECONprms[nf][ns][FishableLength_id] * (hr_per_dt / (bm->SUBFLEET_ECONprms[nf][ns][ShotLength_id] + small_num));
 2705:                 }
 2706:                 for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
 2707:                     bm->prop_hold[ns] /= (tot_hold + small_num);
 2708:                 }
 2709:                 
 2710:                 // Find number of shots to take
 2711:                 if (min_shots > max_shots)
 2712:                     max_shots = min_shots;
 2713:                 step1 = drandom(0.0, min_shots);  // As getting too many shots so reset starting point a bit lower
 2714:                 min_shots = step1;
 2715:                 shotcount = ceil(max_shots - min_shots);
 2716:                 if(shotcount)
 2717:                     step1 = drandom(0.0, shotcount);
 2718:                 step1 += min_shots;
 2719:                 nshot = (int)ceil(step1);
 2720:                 
 2721:                 // Find the locaiton of the shots
 2722:                 tot_effort = 0.0;
 2723:                 Util_Init_1D_Double(bm->prop_effort, bm->nbox, 0.0);
 2724:                 Util_Init_1D_Int(bm->shot_loc, max_poss_shots, 0);
 2725:                 for (ij = 0; ij < bm->nbox; ij++) {
 2726:                     bm->prop_effort[ij] += bm->Effort_hdistrib[ij][nf][today_effort];
 2727:                     tot_effort += bm->Effort_hdistrib[ij][nf][today_effort];
 2728:                 }
 2729:                 for (ij = 0; ij < bm->nbox; ij++) {
 2730:                     bm->prop_effort[ij] /= (tot_effort + small_num);
 2731:                 }
 2732:                 
 2733:                 this_shot = 0;
 2734:                 max_prob = 1.0;
 2735:                 Util_Init_1D_Int(bm->shot_loc, max_poss_shots, 0);
 2736:                 while (this_shot < nshot) {
 2737:                     step1 = drandom(0.0, max_prob);
 2738:                     sum_prob = 0.0;
 2739:                     for (ij = 0; ij < bm->nbox; ij++) {
 2740:                         sum_prob += bm->prop_effort[ij];
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
**Lines (current code):** 2867  
**Lines at review (Oct 2026):** 2852

**Function:** `GenerateCPUE()`

**Why it's thought to be a bug**

`bm->alloced_discard[sp][ij] -= final_shot_discards;` — alloced_discard is subtracted from available discards at 2808/2940, so it must increase as discards are allocated; decrementing makes available discards grow with each shot. Function: `GenerateCPUE()`.

**Current code** (`atmanage/atManage.c` lines 2865–2869)

```c
 2865:                         
 2866:                         // Allocated catch and discards per depth so don't get double counting - also gives average depth for reporting
 2867:                         bm->alloced_discard[sp][ij] -= final_shot_discards;
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
**Lines (current code):** 2870-2889, 2999-3001  
**Lines at review (Oct 2026):** 2855-2874, 2984-2986

**Why it's thought to be a bug**

`sum_prob` is reset only once before the while loop and the k loop has no `break`, so after the first pass every layer past the threshold (and on later passes all layers) is allocated; and `allocate_catch()` allocates the whole `catch_left` of a layer rather than `min(catch_left, *catch_unalloced)`, so a single small shot consumes all catch in a layer and catch_unalloced goes negative. Shot catches/depths and tot_alloced are wrong. Functions: `GenerateCPUE()`, `allocate_catch()`.

**Current code** (`atmanage/atManage.c` lines 2868–2891)

```c
 2868:                         weighted_depth = 0.0;
 2869:                         catch_unalloced = final_shot_catch;
 2870:                         sum_prob = 0.0;
 2871:                         
 2872:                         // Start with random allocation
 2873:                         numstep = 0;
 2874:                         while ((catch_unalloced > 0.0) && (numstep < step_tolerance)) {
 2875:                             step1 = drandom(0.0, 1.0);
 2876:                             for (k = 0; k < bm->boxes[ij].nz; k++) {
 2877:                                 sum_prob += bm->prop_depth[k];
 2878:                                 if (step1 < sum_prob) {
 2879:                                     allocate_catch(bm, sp, nf, ij, k, &catch_unalloced, &weighted_depth, this_catch, llogfp);
 2880:                                 }
 2881:                             }
 2882:                             numstep++;
 2883:                         }
 2884:                         // Now iteratively allocate any remaining
 2885:                         if (catch_unalloced > 0.0) {
 2886:                             for (k = 0; k < bm->boxes[ij].nz; k++) {
 2887:                                 allocate_catch(bm, sp, nf, ij, k, &catch_unalloced, &weighted_depth, this_catch, llogfp);
 2888:                             }
 2889:                         }
 2890:                         
 2891:                         weighted_depth /= (final_shot_catch + small_num);
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
**Lines (current code):** 3007-3011  
**Lines at review (Oct 2026):** 2992-2996

**Function:** `allocate_catch()`

**Why it's thought to be a bug**

`for (ij = 0; ij < bm->nbox; ij++) bm->prop_effort[ij] /= (new_sum + small_num);` — after reducing `prop_depth[k]` the code renormalises the box effort distribution instead of the depth distribution (and reuses parameter `ij` as loop counter). prop_effort is repeatedly inflated; prop_depth never renormalised. Function: `allocate_catch()`.

**Current code** (`atmanage/atManage.c` lines 3005–3013)

```c
 3005:         if (bm->prop_depth[k] < 0.0)
 3006:             bm->prop_depth[k] = 0.0;
 3007:         new_sum = 1.0 - prop_used;
 3008:         if (new_sum > 0.0) {
 3009:             for (ij = 0; ij < bm->nbox; ij++) {
 3010:                 bm->prop_effort[ij] /= (new_sum + small_num);
 3011:             }
 3012:         } else {
 3013:             *catch_unalloced = 0.0;
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
**Lines (current code):** 2951  
**Lines at review (Oct 2026):** 2936

**Function:** `GenerateCPUE()`

**Why it's thought to be a bug**

Left-over effort uses `Effort_hdistrib[..][today_effort] / 24.0` whereas shot effort uses `* 24.0` ("originally in days and want hours", 2897); also `alloced_effort` is never incremented, so the residual effort is the full effort, not the left-over. Function: `GenerateCPUE()`.

**Current code** (`atmanage/atManage.c` lines 2949–2953)

```c
 2949:                         for (ij=0; ij < bm->nbox; ij++) {
 2950:                             bm->ShotData[this_shot][sp][tloc_id] = ij;
 2951:                             bm->ShotData[this_shot][sp][teffort_id] += ((bm->Effort_hdistrib[ij][nf][today_effort] / 24.0 ) - bm->alloced_effort[ij]);
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
**Lines (current code):** 2952-2956  
**Lines at review (Oct 2026):** 2937-2941

**Function:** `GenerateCPUE()`

**Why it's thought to be a bug**

`ShotData[...][tdiscard_id] += Discards[ij][sp][nf]*... - alloced_discard[sp][ij]` is inside the `k` (layer) loop, but Discards/alloced_discard are per box — discards are added nz times. Function: `GenerateCPUE()`.

**Current code** (`atmanage/atManage.c` lines 2950–2958)

```c
 2950:                             bm->ShotData[this_shot][sp][tloc_id] = ij;
 2951:                             bm->ShotData[this_shot][sp][teffort_id] += ((bm->Effort_hdistrib[ij][nf][today_effort] / 24.0 ) - bm->alloced_effort[ij]);
 2952:                             for (k = 0; k < bm->boxes[ij].nz; k++) {
 2953:                                 bm->ShotData[this_shot][sp][wdepth_id] += (bm->Catch[ij][sp][nf][k] * bm->X_CN * mg_2_kg) - bm->alloced_catch[sp][ij][k];
 2954:                                 bm->ShotData[this_shot][sp][tcatch_id] += (bm->Catch[ij][sp][nf][k] * bm->X_CN * mg_2_kg) - bm->alloced_catch[sp][ij][k];
 2955:                                 bm->ShotData[this_shot][sp][tdiscard_id] += (bm->Discards[ij][sp][nf] * bm->X_CN * mg_2_kg) - bm->alloced_discard[sp][ij];
 2956:                             }
 2957:                         }
 2958:                         bm->ShotData[this_shot][sp][wdepth_id] /= (bm->ShotData[this_shot][sp][tcatch_id] + small_num);
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
**Lines (current code):** 2953, 2958  
**Lines at review (Oct 2026):** 2938, 2943

**Function:** `GenerateCPUE()`

**Why it's thought to be a bug**

Left-over `wdepth_id += catch` then `/= tcatch` gives ~1, not a catch-weighted depth (compare 2987 which multiplies by a depth term). Function: `GenerateCPUE()`.

**Current code** (`atmanage/atManage.c` lines 2951–2960)

```c
 2951:                             bm->ShotData[this_shot][sp][teffort_id] += ((bm->Effort_hdistrib[ij][nf][today_effort] / 24.0 ) - bm->alloced_effort[ij]);
 2952:                             for (k = 0; k < bm->boxes[ij].nz; k++) {
 2953:                                 bm->ShotData[this_shot][sp][wdepth_id] += (bm->Catch[ij][sp][nf][k] * bm->X_CN * mg_2_kg) - bm->alloced_catch[sp][ij][k];
 2954:                                 bm->ShotData[this_shot][sp][tcatch_id] += (bm->Catch[ij][sp][nf][k] * bm->X_CN * mg_2_kg) - bm->alloced_catch[sp][ij][k];
 2955:                                 bm->ShotData[this_shot][sp][tdiscard_id] += (bm->Discards[ij][sp][nf] * bm->X_CN * mg_2_kg) - bm->alloced_discard[sp][ij];
 2956:                             }
 2957:                         }
 2958:                         bm->ShotData[this_shot][sp][wdepth_id] /= (bm->ShotData[this_shot][sp][tcatch_id] + small_num);
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
**Lines (current code):** 2832-2845  
**Lines at review (Oct 2026):** 2817-2830

**Function:** `GenerateCPUE()`

**Why it's thought to be a bug**

If `step1 >= cpue_cdf[sp][nc][gun]` for all bins (cdf not reaching 1), `this_catchbin` stays 0 giving zero catch instead of the top bin; bin 0 always yields `max_catch = 0`. Function: `GenerateCPUE()`.

**Current code** (`atmanage/atManage.c` lines 2830–2847)

```c
 2830:                         sum_prob = 0.0;
 2831:                         this_catchbin = 0;
 2832:                         for (nc = 0; nc < bm->K_num_catchbin; nc++){
 2833:                             sum_prob = cpue_cdf[sp][nc][this_gun];   // Could use a calculated negative binomal cdf, but reading it in from pre-generated set for now (as cpue_cdf is cumualtive then set sum_prob = not += )
 2834:                             
 2835:                             //fprintf(llogfp,"gun %d step1: %e nc: %d cpue_cdf: %e sum_prob %e\n", this_gun, step1, nc, cpue_cdf[nc][this_gun], sum_prob);
 2836:                             
 2837:                             if (step1 < sum_prob) {
 2838:                                 this_catchbin = nc;
 2839:                                 break;
 2840:                             }
 2841:                         }
 2842:                         min_catch = (this_catchbin - 1) * bm->size_catchbin;  // in kg
 2843:                         if(min_catch < 0.0)
 2844:                             min_catch = 0.0;
 2845:                         max_catch = this_catchbin * bm->size_catchbin;
 2846:                         diff_Catch = max_catch - min_catch;
 2847:                         step1 = drandom(0.0, diff_Catch);
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
