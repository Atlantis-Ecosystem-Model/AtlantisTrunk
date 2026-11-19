# atimplementation/atImplementationAnnual.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1), plus 2 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [HRV2-20](#hrv2-20) ⭐ | 147, 143, 171 | H/H | logic / wrong condition | `high_trigger_tripped` starts at 1 and is only cleared when a species falls between the low and high threshold… |  | [ ] |
| [HRV2-21](#hrv2-21) | 198, 164, 220 | H/M | misplaced else / wrong branch | In the endangered-species loop, the "Use assessment model output" `else` (221) belongs to `if (speciesParams[s… |  | [ ] |
| [HRV2-22](#hrv2-22) | 351 | M/M | dead code / feature not applied | In the "Disable TAC check" block, `TAC_scale` is computed via Util_Get_Change_Scale but never used. Nothing is… |  | [ ] |
| [HRV2-23](#hrv2-23) | 380 | M/M | accumulator not reset / loop placement | For non-vertebrate groups, `RegionalData[sp][nreg][reg_tac_id] += stock_struct_prop[..]/n` is never reset firs… |  | [ ] |

---

## HRV2-20

**logic / wrong condition** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atImplementationAnnual.c` 147, `atImplementationAnnual.c` 143, `atImplementationAnnual.c` 171
**Lines at review (Oct 2026):** 81, 95-103, 164-168

**Function:** `Apply_Annual_Fisheries_Mgmt()`

**Why it's thought to be a bug**

`high_trigger_tripped` starts at 1 and is only cleared when a species falls between the low and high thresholds. (1) When the low trigger trips (95-100), the loop `break`s with high_trigger_tripped still 1, so the reduction computed at 143-147 is always overwritten by the "recovery" formula at 166-167. (2) The recovery block runs even when `flagADJUSTmanage==0` (it is outside that `if`). (3) `EFF_scale = 1.0 - step1*(1+FC_restrict)` with `step1=(dayt+1-target_day)/FC_period`. target_day is reset to 0 at line 159, so step1 ~ dayt/FC_period and EFF_scale goes strongly negative. This value is stored in `effort_scale[..][target_tac_id]` and used as TAC_scale (319) and mgmt_scale (atHarvestImposedCatch.c:317). The endangered block 264-268 has the same pattern. Function: `Apply_Annual_Fisheries_Mgmt()`.

**Code at issue** — `atImplementationAnnual.c` line 147 (closest match to the quoted code; wording differs slightly)  
Search for: `EFF_scale = 1.0 - (step1 * (1 - FC_restrict));`

```c
  145: 					else
  146: 						step1 = 1.0;
  147→ 					EFF_scale = 1.0 - (step1 * (1 - FC_restrict));
  148: 				} else
  149: 					/* Use assessment model output */
```

**Code at issue** — `atImplementationAnnual.c` line 143 (closest match to the quoted code; wording differs slightly)  
Search for: `if (bm->dayt < (bm->TAC_trigger[fishery_id][target_day_id] + FC_period))`

```c
  141: 				if (!bm->do_assessing) {
  142: 					/* Using original simple adaptive management with perfect knowledge model */
  143→ 					if (bm->dayt < (bm->TAC_trigger[fishery_id][target_day_id] + FC_period))
  144: 						step1 = (bm->dayt + 1.0 - bm->TAC_trigger[fishery_id][target_day_id]) / FC_period;
  145: 					else
```

**Code at issue** — `atImplementationAnnual.c` line 171 (closest match to the quoted code; wording differs slightly)  
Search for: `Note that it is stored in effort_scale[fishery_id][target_tac_id] in all cases (whether ad`

```c
  169: 
  170: 		/* If have stock-based management (flagADJUSTmanage > 0) then this EFF_scale may have a value other than 1.0
  171→ 		 Note that it is stored in effort_scale[fishery_id][target_tac_id] in all cases (whether adjusting
  172: 		 gear, season, spatial management, effort or TACs). For gear and season this is applied below. For
  173: 		 spatial management it is done in Check_For_Active_MPA. For effort it is applied in Effort_Restrict_check().
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
**Found at (current code):** `atImplementationAnnual.c` 198, `atImplementationAnnual.c` 164, `atImplementationAnnual.c` 220
**Lines at review (Oct 2026):** 197-237, 248-257

**Function:** `Apply_Annual_Fisheries_Mgmt()`

**Why it's thought to be a bug**

In the endangered-species loop, the "Use assessment model output" `else` (221) belongs to `if (speciesParams[sp_concern_id])`, not to a `!bm->do_assessing` test like the target-species loop (94/109). Non-concern species are judged on assessment output (TAC_trigger) even when no assessment runs, and concern species always use perfect knowledge. Also, `pseudo_survey_trip=1` (220) is only reached for species above the upper threshold. When the trigger trips via the perfect-knowledge `break` (209), pseudo_survey_trip is 0, so line 257 uses `TAC_trigger[..][triggered_scalar_id]` instead of the ramp. Line 228 prints `fishnowpop/startpop`, which may be uninitialised in that branch. Function: `Apply_Annual_Fisheries_Mgmt()`.

**Code at issue** — `atImplementationAnnual.c` line 198 (closest match to the quoted code; wording differs slightly)  
Search for: `if (FunctGroupArray[sp].speciesParams[sp_concern_id]) {`

```c
  196: 			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  197: 				if ((FunctGroupArray[sp].isImpacted == TRUE) || ((FunctGroupArray[sp].isImpacted == FALSE) && (FunctGroupArray[sp].speciesParams[assess_flag_id] < 0))){
  198→ 					if (FunctGroupArray[sp].speciesParams[sp_concern_id]) {
  199: 						FC_thresh = bm->SP_FISHERYprms[sp][fishery_id][FC_thresh_id];
  200: 						FC_high_thresh = bm->SP_FISHERYprms[sp][fishery_id][FC_high_thresh_id];
```

**Code at issue** — `atImplementationAnnual.c` line 164  
Search for: `!bm->do_assessing`

```c
  162: 		}
  163: 
  164→ 		if ((high_trigger_tripped) && (!bm->do_assessing)) {
  165: 			/* If in recovery mode let it rebuild */
  166: 			step1 = (bm->dayt + 1.0 - bm->TAC_trigger[fishery_id][target_day_id]) / FC_period;
```

**Code at issue** — `atImplementationAnnual.c` line 220  
Search for: `pseudo_survey_trip = 1`

```c
  218:                                         bm->dayt, FunctGroupArray[sp].groupCode, FisheryArray[fishery_id].fisheryCode, fishnowpop / startpop);
  219:                         }
  220→                         pseudo_survey_trip = 1;
  221:                     } else {
  222:                         /* Use assessment model output */
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
**Found at (current code):** `atImplementationAnnual.c` 351
**Lines at review (Oct 2026):** 357-366

**Function:** `Apply_Annual_Fisheries_Mgmt()`

**Why it's thought to be a bug**

In the "Disable TAC check" block, `TAC_scale` is computed via Util_Get_Change_Scale but never used. Nothing is assigned to TACamt, so disabling TACs has no effect. Function: `Apply_Annual_Fisheries_Mgmt()`.

**Code at issue** — `atImplementationAnnual.c` line 351 (located by name only; check this is the right place)  
Search for: `bm->TACamt[sp][fishery_id][now_id], TACorig, TAC_scale);`

```c
  349: 
  350:                         fprintf(llogfp, "%e, %s TAC set to %e (TACorig: %e, TAC_scale: %e\n", bm->dayt, FunctGroupArray[sp].groupCode,
  351→ 							bm->TACamt[sp][fishery_id][now_id], TACorig, TAC_scale);
  352:                     }
  353: 				}
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
**Found at (current code):** `atImplementationAnnual.c` 380
**Lines at review (Oct 2026):** 372-501, 491-498

**Function:** `Apply_Annual_Fisheries_Mgmt()`

**Why it's thought to be a bug**

For non-vertebrate groups, `RegionalData[sp][nreg][reg_tac_id] += stock_struct_prop[..]/n` is never reset first. The whole regional block also sits inside the `fishery_id` loop, so the value grows by about 1 per managed fishery per year and stops being a proportion. The vertebrate branch also recomputes `initreg_prop` for every guild, for every sp and every fishery: wasteful, but the result is correct. Function: `Apply_Annual_Fisheries_Mgmt()`.

**Code at issue** — `atImplementationAnnual.c` line 380 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->lastreg_prop[sp][nreg] = bm->RegionalData[sp][nreg][reg_tac_id];`

```c
  378: 
  379: 						for (nreg = 0; nreg < bm->K_num_active_reg; nreg++) {
  380→ 							bm->lastreg_prop[sp][nreg] = bm->RegionalData[sp][nreg][reg_tac_id];
  381: 							bm->initreg_prop[sp][nreg] = 0.0;
  382: 						}
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
