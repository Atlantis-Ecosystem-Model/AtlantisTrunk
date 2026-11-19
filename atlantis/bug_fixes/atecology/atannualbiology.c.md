# atecology/atannualbiology.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **9 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 2 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [BIOL-16](#biol-16) | 603, 619 | H/M | off-by-one | `layerk = k + diffdeep - 1;` / `layerk = j + diffdeep - 1;` — this is the same vertical rescaling as `Get_Vert… |  | [ ] |
| [BIOL-17](#biol-17) | 1162-1164 | M/M | wrong box index | `boxkey_id = bm->BoxKeyMap[bm->current_box][rects_key_id];` — `Get_Init_Embryos()` is called from inside the b… |  | [ ] |
| [BIOL-18](#biol-18) | 1103-1186 vs 1243 | M/M | missing switch case | The SR switch handles `case ice_wimd_based`, but the first (larvae) switch has no such case, so the run reache… |  | [ ] |
| [BIOL-19](#biol-19) | 471, 593-669 | M/M | accumulator / loop nesting | The larval-queue-extension block (recruit setup for k < larval_queue_extension) sits inside the `for nspawn < … |  | [ ] |
| [BIOL-20](#biol-20) | 1111-1113 | H/L | debug output left live | `if(species == 9) { fprintf(bm->logFile,"Init for %s qid: ..."); }` — a hard-coded species index debug print i… |  | [ ] |
| [BIOL-21](#biol-21) | 924-934 | M/L | debug output left live | The `do_debug` guard is commented out, so `PrintDates()` writes a line for every species × queue slot × cohort… |  | [ ] |
| [BIOL-22](#biol-22) | 493-494 vs 518 | L/L | inconsistent constant | Countback uses `temp_val -= 364` (and l.526-527 for recruits), but insertion subtracts `365 * i` (l.518, 560-5… |  | [ ] |
| [BIOL-23](#biol-23) | 752-755 | L/L | unit mismatch in check | `next_spawn_any_age > ((365.0 * bm->tstop) / 86400.0)` — `tstop/86400` is already days, and multiplying by 365… |  | [ ] |
| [BIOL-24](#biol-24) | 703-710 | L/L | wrong formula (latent) | `num_steps = ceil(364.0 / AgeClassSize_sp)` should be `ceil(1.0 / AgeClassSize_sp)` (steps per year). As writt… |  | [ ] |

---

## BIOL-16

**off-by-one** · Conf H / Sev M  
**Lines (current code):** 603, 619  
**Lines at review (Oct 2026):** 603, 619

**Function:** `Init_Spawning()`

**Why it's thought to be a bug**

`layerk = k + diffdeep - 1;` / `layerk = j + diffdeep - 1;` — this is the same vertical rescaling as `Get_Vertical_Recruit_Position()` (atdemography.c:2029-2046), which uses `k + diffdeep` (also atmovement.c:4224, atManageSetup.c:1155). With `-1`, shallow boxes map onto layers shifted down by one and never use the top layer `nlayers-1`, so the initial pre-model recruits get the wrong vertical distribution. Function: `Init_Spawning()`.

**Current code** (`atecology/atannualbiology.c` lines 601–621)

```c
  601:                             sumnzj2 = 0.0;
  602:                             for (k = 0; k < bm->boxes[i].nz; k++) {
  603:                                 layerk = k + diffdeep - 1;
  604:                                 sumnzj1 += recruit_vdistrib[layerk][sp];
  605:                                 sumnzj2 += recruit_vdistrib[k][sp];
  606:                             }
  607:                             if (sumnzj1 < sumnzj2) {
  608:                                 vertdistrib = recruit_vdistrib[j][sp] / sumnzj2;
  609:                             } else if (!sumnzj1) {
  610:                                 /* If demersal and on the bottom */
  611:                                 if (flagdem && (j == 0)) {
  612:                                     vertdistrib = 1;
  613:                                     /* If pelagic and at the surface */
  614:                                 } else if (!flagdem && (j == bm->boxes[i].nz - 1)) {
  615:                                     vertdistrib = 1;
  616:                                 } else
  617:                                     vertdistrib = 0;
  618:                             } else {
  619:                                 layerk = j + diffdeep - 1;
  620:                                 vertdistrib = recruit_vdistrib[layerk][sp] / sumnzj1;
  621:                             }
```

**Suggested fix (from review):** Use `k + diffdeep` / `j + diffdeep`, or simply call `Get_Vertical_Recruit_Position(sp, j, nlayers, bm->boxes[i].nz)`

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

## BIOL-17

**wrong box index** · Conf M / Sev M  
**Lines (current code):** 1162-1164  
**Lines at review (Oct 2026):** 1157-1159

**Function:** `Get_Init_Embryos()`

**Why it's thought to be a bug**

`boxkey_id = bm->BoxKeyMap[bm->current_box][rects_key_id];` — `Get_Init_Embryos()` is called from inside the box loop `i` of `Init_Spawning()`, but it has no box argument, so it uses the stale `bm->current_box` (whatever was last processed) for every box. Function: `Get_Init_Embryos()`.

**Current code** (`atecology/atannualbiology.c` lines 1160–1166)

```c
 1160:             break;
 1161:         case multiple_ts_recruit: /* Read in timeseries of recruitment - assume all genotypes get the same number of recruits */
 1162:             boxkey_id = bm->BoxKeyMap[bm->current_box][rects_key_id];
 1163:             EMBRYO[species].Larvae[stock_id][ngene][qid] +=  Get_Imposed_MultRecruits(bm, species, do_debug, boxkey_id, bm->logFile);
 1164:             break;
 1165:         case jackknife_recruit: /* Jackknife spawning function - sum up spawning stock biomass */
 1166:             for(bcohort = 0; bcohort < FunctGroupArray[species].numCohorts; bcohort++){
```

**Suggested fix (from review):** Pass box `i` into `Get_Init_Embryos()` and use it here

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

## BIOL-18

**missing switch case** · Conf M / Sev M  
**Lines (current code):** 1103-1186 vs 1243  
**Lines at review (Oct 2026):** 1098-1181 vs 1238

**Function:** `Get_Init_Embryos()`

**Why it's thought to be a bug**

The SR switch handles `case ice_wimd_based`, but the first (larvae) switch has no such case, so the run reaches `default: quit("No such flagrecruit defined...")` whenever `flag_old_embryo_init` is off and a larval-queue extension exists. The same omission appears in atdemography.c's first switch. Function: `Get_Init_Embryos()`.

**Current code** (`atecology/atannualbiology.c` lines 1101–1140 (truncated))

```c
 1101:     /* First get larvae */
 1102:     switch (recruit_sp) {
 1103:         case no_recruit:
 1104:             quit("No such flagrecruit defined (i.e. value must be > 0)\n");
 1105:             break;
 1106:         case const_recruit: /* Fixed set of constants */
 1107:             if (EMBRYO[species].readytospawn[stock_id] == 1)
 1108:                 EMBRYO[species].Larvae[stock_id][ngene][qid] = KDENR_sp / ng;
 1109:             
 1110:             if(species == 9) {
 1111:                 fprintf(bm->logFile,"Init for %s qid: %d ngene: %d Larvae: %e KDENR_sp: %e ng: %e\n", FunctGroupArray[species].groupCode, qid, ngene,  EMBRYO[species].Larvae[stock_id][ngene][qid], KDENR_sp, ng);
 1112:             }
 1113:             
 1114:             break;
 1115:         case chl_recruit: /* Proportional to primary productivity */
 1116:             EMBRYO[species].Larvae[stock_id][ngene][qid] += recSTOCK[species][stock_id] * PP_sp * bm->avg_cell_vol * (CHLa / bm->ref_chl);
 1117:             break;
 1118:         case rand_recruit: /* Random - follows lognormal */
 1119:             if (EMBRYO[species].readytospawn[stock_id] == 1)
 1120:                 EMBRYO[species].Larvae[stock_id][ngene][qid] = recSTOCK[species][stock_id] * sp_log_mult * Util_Logx_Result(-lognorm_mu, lognorm_sigma);
 1121:             break;
 1122:         case plank_recruit: /* Spawn is based on plankton levels (not just CHLa) */
 1123:             EMBRYO[species].Larvae[stock_id][ngene][qid] += recSTOCK[species][stock_id] * PP_sp * plankton / bm->ref_chl;
 1124:             break;
 1125:         case BevHolt_recruit: /* basical Atlantis Beverton-Holt stock-recruit relationship - so uses spawn calculated above */
 1126:         case BevHolt_rand_recruit: /* Spawn is based on Beverton Holt with lognormal variation and dependence on plankton levels  - so uses spawn calculated above */
 1127:         case BevHolt_num_recruit: /* Beverton-Holt stock-recruit relationship using numbers only - uses spawn calculated above */
 1128:         case recover_recruit: /* Beverton-Holt stock-recruit relationship - so uses spawn calculated above */
 1129:         case force_recover_recruit: /* Beverton-Holt stock-recruit relationship - so uses spawn calculated above */
 1130:         case Ricker_recruit: /* Ricker - so uses spawn calculated above */
 1131:         case baltic_ricker: /* Baltic version of the ricker */
 1132:         case SSB_ricker: /* SSB based ricker version - for senescent and short lived species */
 1133:         case SSB_BevHolt_recruit:/* Standard Beverton Holt */
 1134:         case linear_recruit:/* Pupping or calving linearly dependent on maternal condition */
 1135:             
 1136:             // Evolution only works for these spawning cases for now - TODO: Generalise so evolution works in all cases
 1137:             EMBRYO[species].Larvae[stock_id][ngene][qid] += EMBRYO[species].TotSpawn[ngene];
 1138:             
 1139:             /**
 1140:             //if (do_debug && (bm->which_check == species)) {
```

**Suggested fix (from review):** Add `case ice_wimd_based:` alongside the BevHolt/Ricker cases (Larvae += TotSpawn)

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

## BIOL-19

**accumulator / loop nesting** · Conf M / Sev M  
**Lines (current code):** 471, 593-669  
**Lines at review (Oct 2026):** 471, 593-669

**Function:** `Init_Spawning()`

**Why it's thought to be a bug**

The larval-queue-extension block (recruit setup for k < larval_queue_extension) sits inside the `for nspawn < numSpawns` loop. With `flag_old_embryo_init`, `EMBRYO[sp].Larvae[stock_id][ngene][k] += num_recruits` (l.649) is accumulated again for each spawn event, so it counts recruits numSpawns times. The `num_recruits[..][k]` entries are also overwritten per nspawn, whatever spawn they belong to. Function: `Init_Spawning()`.

**Current code** (`atecology/atannualbiology.c` lines 469–473)

```c
  469:     
  470:     // Now shuffle based on needing to insert any from the spawns prior to the start of the model
  471:     for( nspawn = 0; nspawn < FunctGroupArray[sp].numSpawns; nspawn++){  // As all queues rhe same length
  472:         // Assume the genetypes can go at different times but that all cohorts per genetype go at the same time
  473:         // This is different to the old code where it was the cohort not the genetype that was being set
```

**Suggested fix (from review):** Index by the spawn's queue slot, or move the block outside the nspawn loop and reset Larvae first

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

## BIOL-20

**debug output left live** · Conf H / Sev L  
**Lines (current code):** 1111-1113  
**Lines at review (Oct 2026):** 1106-1108

**Function:** `Get_Init_Embryos()`

**Why it's thought to be a bug**

`if(species == 9) { fprintf(bm->logFile,"Init for %s qid: ..."); }` — a hard-coded species index debug print in live code. Function: `Get_Init_Embryos()`.

**Current code** (`atecology/atannualbiology.c` lines 1109–1115)

```c
 1109:             
 1110:             if(species == 9) {
 1111:                 fprintf(bm->logFile,"Init for %s qid: %d ngene: %d Larvae: %e KDENR_sp: %e ng: %e\n", FunctGroupArray[species].groupCode, qid, ngene,  EMBRYO[species].Larvae[stock_id][ngene][qid], KDENR_sp, ng);
 1112:             }
 1113:             
 1114:             break;
 1115:         case chl_recruit: /* Proportional to primary productivity */
```

**Suggested fix (from review):** Delete it

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

## BIOL-21

**debug output left live** · Conf M / Sev L  
**Lines (current code):** 924-934  
**Lines at review (Oct 2026):** 924-934

**Function:** `PrintDates()`

**Why it's thought to be a bug**

The `do_debug` guard is commented out, so `PrintDates()` writes a line for every species × queue slot × cohort×genotype (called from l.718). The queue length is numSpawns*(years+…), so this can be thousands of lines per species at init. Function: `PrintDates()`.

**Current code** (`atecology/atannualbiology.c` lines 922–936)

```c
  922:     //do_debug = 1;
  923:     
  924: 	//if (do_debug && (bm->which_check == sp)) {
  925:         fprintf(llogfp, "%s-%d SpawnDay: %d AgeDay: %d StartDay: %d EndDay: %d ",
  926:             FunctGroupArray[sp].groupCode, cohort, EMBRYO[sp].Spawn_Now[cohort][qid], EMBRYO[sp].Age_Now[cohort][aid], EMBRYO[sp].StartDay[cohort][lid], EMBRYO[sp].EndDay[cohort][lid]);
  927:             
  928:         for (k = 0; k < FunctGroupArray[sp].numStocks; k++) {
  929:             fprintf(llogfp, " VERTembryo-%d %e", k, EMBRYO[sp].Larvae[k][ngene][lid]);
  930:         }
  931: 
  932:         fprintf(llogfp, "\n");
  933: 
  934: 	//}
  935: 
  936: 	return;
```

**Suggested fix (from review):** Restore the `if (do_debug && ...)` guard

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

## BIOL-22

**inconsistent constant** · Conf L / Sev L  
**Lines (current code):** 493-494 vs 518  
**Lines at review (Oct 2026):** 493-494 vs 518

**Function:** `Init_Spawning()`

**Why it's thought to be a bug**

Countback uses `temp_val -= 364` (and l.526-527 for recruits), but insertion subtracts `365 * i` (l.518, 560-561). For dates of about 729-730 days, countback gives 2 years while the insertion assumes 365-day years. Function: `Init_Spawning()`.

**Current code** (`atecology/atannualbiology.c` lines 491–520)

```c
  491:                 }
  492:                 
  493:                 while ( temp_val > 364 ) {
  494:                     temp_val -= 364;
  495:                     countback_spawn++;
  496:                 }
  497:             
  498:                 // Create the inserted countback cases
  499:                 if (countback_spawn) {
  500:                     temp_val_start = EMBRYO[sp].Spawn_Now[cohort][qid];
  501:                     start_i = 0;
  502:                     // Start by finding the insertion point
  503:                     for(i = 0; i < FunctGroupArray[sp].numSpawns; i++){
  504:                         if(temp_val_start > EMBRYO[sp].Spawn_Now[cohort][i]) {
  505:                             start_i = i;
  506:                             break;
  507:                         }
  508:                     }
  509:                 
  510:                     // Shuffle entries out of the way - including qid as will need that to lead into next year
  511:                     for(i=EMBRYO[sp].num_in_spawn_queue - countback_spawn - 1; i >= start_i; i--){
  512:                         EMBRYO[sp].Spawn_Now[cohort][i + countback_spawn] = EMBRYO[sp].Spawn_Now[cohort][i];
  513:                     }
  514:                 
  515:                     // Now insert the new values after countback
  516:                     j = 0;
  517:                     for(i = countback_spawn; i >= 1; i--){
  518:                         EMBRYO[sp].Spawn_Now[cohort][start_i + j] = temp_val_start - 365 * i;
  519:                         j++;
  520:                     }
```

**Suggested fix (from review):** Use 365 consistently

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

## BIOL-23

**unit mismatch in check** · Conf L / Sev L  
**Lines (current code):** 752-755  
**Lines at review (Oct 2026):** 752-755

**Function:** `Init_Spawning()`

**Why it's thought to be a bug**

`next_spawn_any_age > ((365.0 * bm->tstop) / 86400.0)` — `tstop/86400` is already days, and multiplying by 365 makes the upper bound meaningless (365x too lax). Function: `Init_Spawning()`.

**Current code** (`atecology/atannualbiology.c` lines 750–757)

```c
  750:     
  751:     /* Sanity checks */
  752:     if ((EMBRYO[sp].next_spawn_any_age < 0) || (EMBRYO[sp].next_spawn_any_age > ((365.0 * bm->tstop) / 86400.0)))
  753:         quit("First spawning date for %s (%d) makes no sense - note testing against 0 and %e as tstop is : %e\n", FunctGroupArray[sp].groupCode, EMBRYO[sp].next_spawn_any_age, ((365.0 * bm->tstop) / 86400.0), bm->tstop);
  754:     if ((EMBRYO[sp].next_age_any_age < 0) || (EMBRYO[sp].next_age_any_age > ((365.0 * bm->tstop) / 86400.0)))
  755:         quit("First aging date for %s (%d) makes no sense - note testing against 0 and %e as tstop is : %e\n", FunctGroupArray[sp].groupCode, EMBRYO[sp].next_age_any_age, ((365.0 * bm->tstop) / 86400.0), bm->tstop);
  756:     
  757: 	return;
```

**Suggested fix (from review):** Compare against `bm->tstop / 86400.0`

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

## BIOL-24

**wrong formula (latent)** · Conf L / Sev L  
**Lines (current code):** 703-710  
**Lines at review (Oct 2026):** 703-710

**Function:** `Init_Spawning()`

**Why it's thought to be a bug**

`num_steps = ceil(364.0 / AgeClassSize_sp)` should be `ceil(1.0 / AgeClassSize_sp)` (steps per year). As written, `stid` reaches far beyond the `Age_Now` queue (`[cohort][max_num]`, atbiolsetup.c:4032). This is currently unreachable because `ageClassSize` is an `int` (atFunctGroup.h:90), so it is always ≥1. Function: `Init_Spawning()`.

**Current code** (`atecology/atannualbiology.c` lines 701–712)

```c
  701:                      */
  702: 
  703:                     stepsize = (int) floor(ROUNDGUARD + (364.0 * AgeClassSize_sp));
  704:                     num_steps = (int) ceil(364.0 / AgeClassSize_sp);
  705: 
  706:                     /* TODO: May need to put in a check that not accidentally wiping valid dates created in earlier years */
  707:                     for (stid = aid; stid < aid + num_steps; stid++) {
  708:                         // So start at spawning day and step forward through the year
  709:                         EMBRYO[sp].Age_Now[cohort][stid] = EMBRYO[sp].StartDay[cohort][lid] - 1 + (stid - aid) * stepsize;
  710:                     }
  711:                 }
  712: 
```

**Suggested fix (from review):** `num_steps = (int)ceil(1.0 / AgeClassSize_sp)` and bound by `num_in_aging_queue`

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

atannualbiology.c l.499-520: the spawn queue is shifted in place inside the `nspawn` loop, so later iterations (`qid = nspawn`) read already-shifted entries. With numSpawns > 1 this may double-insert countback entries.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atannualbiology.c `Get_Init_Embryos()` is called per box/layer/k. For `rand_recruit`/stochastic recruitment each call draws a new random number, so `Larvae[stock][gene][k]` ends up holding the last box's draw. The extra calls also change the RNG sequence.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
