# atimplementation/atImplementationAnnual.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1), plus 2 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [HRV2-20](#hrv2-20) ⭐ | 81, 95-103, 164-168 | H/H | logic / wrong condition | `high_trigger_tripped` starts at 1 and is only cleared when a species falls between the low and high threshold… |  | [ ] |
| [HRV2-21](#hrv2-21) | 197-237, 248-257 | H/M | misplaced else / wrong branch | In the endangered-species loop, the "Use assessment model output" `else` (221) belongs to `if (speciesParams[s… |  | [ ] |
| [HRV2-22](#hrv2-22) | 357-366 | M/M | dead code / feature not applied | In the "Disable TAC check" block, `TAC_scale` is computed via Util_Get_Change_Scale but never used. Nothing is… |  | [ ] |
| [HRV2-23](#hrv2-23) | 372-501, 491-498 | M/M | accumulator not reset / loop placement | For non-vertebrate groups, `RegionalData[sp][nreg][reg_tac_id] += stock_struct_prop[..]/n` is never reset firs… |  | [ ] |

---

## HRV2-20

**logic / wrong condition** · Conf H / Sev H ⭐ priority  
**Listed lines:** 81, 95-103, 164-168

**Function:** `Apply_Annual_Fisheries_Mgmt()`

**Why it's thought to be a bug**

`high_trigger_tripped` starts at 1 and is only cleared when a species falls between the low and high thresholds. (1) When the low trigger trips (95-100), the loop `break`s with high_trigger_tripped still 1, so the reduction computed at 143-147 is always overwritten by the "recovery" formula at 166-167. (2) The recovery block runs even when `flagADJUSTmanage==0` (it is outside that `if`). (3) `EFF_scale = 1.0 - step1*(1+FC_restrict)` with `step1=(dayt+1-target_day)/FC_period`. target_day is reset to 0 at line 159, so step1 ~ dayt/FC_period and EFF_scale goes strongly negative. This value is stored in `effort_scale[..][target_tac_id]` and used as TAC_scale (319) and mgmt_scale (atHarvestImposedCatch.c:317). The endangered block 264-268 has the same pattern. Function: `Apply_Annual_Fisheries_Mgmt()`.

**Current code** (`atimplementation/atImplementationAnnual.c` lines 79–83)

```c
   79: 		FC_period2 = bm->FISHERYprms[fishery_id][FC_period2_id];
   80: 		EFF_scale = 1.0;
   81: 		high_trigger_tripped = 1;
   82: 		if (flagADJUSTmanage > 0) {
   83: 			trigger_tripped = 0;
```

**Suggested fix (from review):** Run the recovery only when `!trigger_tripped && high_trigger_tripped && flagADJUSTmanage>0`, and use a bounded ramp, e.g. `FC_restrict + min(1,step1)*(1-FC_restrict)`

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

## HRV2-21

**misplaced else / wrong branch** · Conf H / Sev M  
**Listed lines:** 197-237, 248-257

**Function:** `Apply_Annual_Fisheries_Mgmt()`

**Why it's thought to be a bug**

In the endangered-species loop, the "Use assessment model output" `else` (221) belongs to `if (speciesParams[sp_concern_id])`, not to a `!bm->do_assessing` test like the target-species loop (94/109). Non-concern species are judged on assessment output (TAC_trigger) even when no assessment runs, and concern species always use perfect knowledge. Also, `pseudo_survey_trip=1` (220) is only reached for species above the upper threshold. When the trigger trips via the perfect-knowledge `break` (209), pseudo_survey_trip is 0, so line 257 uses `TAC_trigger[..][triggered_scalar_id]` instead of the ramp. Line 228 prints `fishnowpop/startpop`, which may be uninitialised in that branch. Function: `Apply_Annual_Fisheries_Mgmt()`.

**Current code** (`atimplementation/atImplementationAnnual.c` lines 195–234 (truncated))

```c
  195: 			end_trigger_tripped = 0;
  196: 			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  197: 				if ((FunctGroupArray[sp].isImpacted == TRUE) || ((FunctGroupArray[sp].isImpacted == FALSE) && (FunctGroupArray[sp].speciesParams[assess_flag_id] < 0))){
  198: 					if (FunctGroupArray[sp].speciesParams[sp_concern_id]) {
  199: 						FC_thresh = bm->SP_FISHERYprms[sp][fishery_id][FC_thresh_id];
  200: 						FC_high_thresh = bm->SP_FISHERYprms[sp][fishery_id][FC_high_thresh_id];
  201: 						startpop = bm->totinitpop[sp];
  202: 						fishnowpop = bm->totfishpop[sp];
  203: 
  204: 						/* Using original simple adaptive management with perfect knowledge model */
  205:                         if (fishnowpop < FC_thresh * startpop) {
  206:                             end_trigger_tripped = 1;
  207:                             fprintf(llogfp, "Time: %e, endangered species %s (impacted by fishery %s) is at critical population levels (%.2f)\n", bm->dayt,
  208:                                     FunctGroupArray[sp].groupCode, FisheryArray[fishery_id].fisheryCode, fishnowpop / startpop);
  209:                             break;
  210:                         } else if (fishnowpop < FC_high_thresh * startpop) {
  211:                             high_end_trigger_tripped = 0;
  212:                             break;
  213:                         } else {
  214:                             if (verbose > 1)
  215:                                 fprintf(
  216:                                         llogfp,
  217:                                         "Time: %e, endangered species %s population (impacted by fishery %s) is larger than upper reference level (%.2f)\n",
  218:                                         bm->dayt, FunctGroupArray[sp].groupCode, FisheryArray[fishery_id].fisheryCode, fishnowpop / startpop);
  219:                         }
  220:                         pseudo_survey_trip = 1;
  221:                     } else {
  222:                         /* Use assessment model output */
  223:                         if (bm->TAC_trigger[fishery_id][triggered_scalar_id] < 1.0 && bm->TAC_trigger[fishery_id][endanger_is_trigger_id]) {
  224:                             end_trigger_tripped = 1;
  225:                             fprintf(
  226:                                     llogfp,
  227:                                     "Time: %e, endangered species %s (impacted by fishery %s) is at critical population levels so fishery effort impacted (%.2f)\n",
  228:                                     bm->dayt, FunctGroupArray[sp].groupCode, FisheryArray[fishery_id].fisheryCode, fishnowpop / startpop);
  229:                             break;
  230:                         } else if (bm->TAC_trigger[fishery_id][triggered_scalar_id] > 1.0 && bm->TAC_trigger[fishery_id][endanger_is_trigger_id]) {
  231:                             if (verbose > 1)
  232:                                 fprintf(llogfp, "Time: %e, endangered species %s population (impacted by fishery %s) is not endangered any more (%.2f)\n",
  233:                                         bm->dayt, FunctGroupArray[sp].groupCode, FisheryArray[fishery_id].fisheryCode, fishnowpop / startpop);
  234:                         } else {
```

**Suggested fix (from review):** Restructure as `if (sp_concern) { if (!bm->do_assessing) {...; pseudo_survey_trip=1;} else {...} }`

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

## HRV2-22

**dead code / feature not applied** · Conf M / Sev M  
**Listed lines:** 357-366

**Function:** `Apply_Annual_Fisheries_Mgmt()`

**Why it's thought to be a bug**

In the "Disable TAC check" block, `TAC_scale` is computed via Util_Get_Change_Scale but never used. Nothing is assigned to TACamt, so disabling TACs has no effect. Function: `Apply_Annual_Fisheries_Mgmt()`.

**Current code** (`atimplementation/atImplementationAnnual.c` lines 360–373) — code appears to have moved +5 lines since the review

```c
  360:                         if (flagTACchange == disable_tac ) {
  361:                             TAC_num_changes = (int) (bm->SP_FISHERYprms[sp][fishery_id][TAC_num_changes_id]);
  362:                             TAC_scale = Util_Get_Change_Scale(bm, TAC_num_changes, TACchange[sp][fishery_id]);
  363:                         }
  364:                     }
  365:                 }
  366:             }
  367: 		}
  368: 
  369: 		/* Set regional TAC - can't have regional TACs or stocks for invertebrates as yet
  370: 		 FIX - allow multiple stocks and regional TACs for all groups
  371: 		 */
  372:         if (!bm->flagecon_on || bm->MultiPlanEffort) {
  373:             if (bm->manage_reg) {
```

**Suggested fix (from review):** Apply the scale (e.g. set `TACamt[sp][fishery_id][now_id]`) or remove

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

## HRV2-23

**accumulator not reset / loop placement** · Conf M / Sev M  
**Listed lines:** 372-501, 491-498

**Function:** `Apply_Annual_Fisheries_Mgmt()`

**Why it's thought to be a bug**

For non-vertebrate groups, `RegionalData[sp][nreg][reg_tac_id] += stock_struct_prop[..]/n` is never reset first. The whole regional block also sits inside the `fishery_id` loop, so the value grows by about 1 per managed fishery per year and stops being a proportion. The vertebrate branch also recomputes `initreg_prop` for every guild, for every sp and every fishery: wasteful, but the result is correct. Function: `Apply_Annual_Fisheries_Mgmt()`.

**Current code** (`atimplementation/atImplementationAnnual.c` lines 378–412 (range truncated)) — code appears to have moved +8 lines since the review

```c
  378: 
  379: 						for (nreg = 0; nreg < bm->K_num_active_reg; nreg++) {
  380: 							bm->lastreg_prop[sp][nreg] = bm->RegionalData[sp][nreg][reg_tac_id];
  381: 							bm->initreg_prop[sp][nreg] = 0.0;
  382: 						}
  383: 
  384: 						for (ij = 0; ij < bm->nbox; ij++) {
  385: 							nreg = bm->regID[ij];
  386: 							if (bm->boxes[ij].type != BOUNDARY) {
  387: 
  388: 								for (k = 0; k < bm->boxes[ij].nz; k++) { // loop through each layer in this box.
  389: 									for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
  390: 										switch (FunctGroupArray[guild].groupAgeType) {
  391: 										case AGE_STRUCTURED:
  392: 											for (n = 0; n < FunctGroupArray[guild].numCohortsXnumGenes; n++) {
  393: 												sn = FunctGroupArray[guild].structNTracers[n];
  394: 												rn = FunctGroupArray[guild].resNTracers[n];
  395: 												den = FunctGroupArray[guild].NumsTracers[n];
  396: 												biomass = (bm->boxes[ij].tr[k][sn] + bm->boxes[ij].tr[k][rn]) * bm->boxes[ij].tr[k][den]
  397: 														* FunctGroupArray[guild].habitatCoeffs[WC];
  398: 
  399: 												bm->initreg_prop[guild][nreg] += biomass;
  400: 
  401: 											}
  402: 											break;
  403: 										case AGE_STRUCTURED_BIOMASS:
  404: 										case BIOMASS:
  405: 											if (FunctGroupArray[guild].groupType != REF_DET && FunctGroupArray[guild].groupType != LAB_DET) {
  406: 												for (n = 0; n < FunctGroupArray[guild].numCohortsXnumGenes; n++) {
  407: 													pid = FunctGroupArray[guild].totNTracers[n];
  408: 													biomass = bm->boxes[ij].tr[k][pid] * bm->boxes[ij].dz[k] * bm->boxes[ij].area
  409: 															* FunctGroupArray[guild].habitatCoeffs[WC];
  410: 
  411: 													bm->initreg_prop[guild][nreg] += biomass;
  412: 												}
```

**Suggested fix (from review):** Set RegionalData to 0 before the += loop, and move the regional block outside the fishery loop

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

atImplementationAnnual.c 560-565: with `flagchangeSEL == change_dynamic`, `q *= effort_scale` compounds every year. Given HRV2-20, effort_scale may be negative, which would make q negative.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atImplementationAnnual.c 513-516, 532: season wrap arithmetic uses 364 rather than 365. It is used consistently, so it is left as is.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
