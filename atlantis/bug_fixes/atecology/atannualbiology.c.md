# atecology/atannualbiology.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **9 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 2 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [BIOL-16](#biol-16) | 603, 619 | H/M | off-by-one | `layerk = k + diffdeep - 1;` / `layerk = j + diffdeep - 1;` — this is the same vertical rescaling as `Get_Vert… |  | [ ] |
| [BIOL-17](#biol-17) | 1162, 653 | M/M | wrong box index | `boxkey_id = bm->BoxKeyMap[bm->current_box][rects_key_id];` — `Get_Init_Embryos()` is called from inside the b… |  | [ ] |
| [BIOL-18](#biol-18) | 1182, 653 | M/M | missing switch case | The SR switch handles `case ice_wimd_based`, but the first (larvae) switch has no such case, so the run reache… |  | [ ] |
| [BIOL-19](#biol-19) | 471, 649 | M/M | accumulator / loop nesting | The larval-queue-extension block (recruit setup for k < larval_queue_extension) sits inside the `for nspawn < … |  | [ ] |
| [BIOL-20](#biol-20) | 1110, 653 | H/L | debug output left live | `if(species == 9) { fprintf(bm->logFile,"Init for %s qid: ..."); }` — a hard-coded species index debug print i… |  | [ ] |
| [BIOL-21](#biol-21) | 798 | M/L | debug output left live | The `do_debug` guard is commented out, so `PrintDates()` writes a line for every species × queue slot × cohort… |  | [ ] |
| [BIOL-22](#biol-22) | 494 | L/L | inconsistent constant | Countback uses `temp_val -= 364` (and l.526-527 for recruits), but insertion subtracts `365 * i` (l.518, 560-5… |  | [ ] |
| [BIOL-23](#biol-23) | 752 | L/L | unit mismatch in check | `next_spawn_any_age > ((365.0 * bm->tstop) / 86400.0)` — `tstop/86400` is already days, and multiplying by 365… |  | [ ] |
| [BIOL-24](#biol-24) | 704 | L/L | wrong formula (latent) | `num_steps = ceil(364.0 / AgeClassSize_sp)` should be `ceil(1.0 / AgeClassSize_sp)` (steps per year). As writt… |  | [ ] |

---

## BIOL-16

**off-by-one** · Conf H / Sev M  
**Found at (current code):** `atannualbiology.c` 603, `atannualbiology.c` 619
**Lines at review (Oct 2026):** 603, 619

**Function:** `Init_Spawning()`

**Why it's thought to be a bug**

`layerk = k + diffdeep - 1;` / `layerk = j + diffdeep - 1;` — this is the same vertical rescaling as `Get_Vertical_Recruit_Position()` (atdemography.c:2029-2046), which uses `k + diffdeep` (also atmovement.c:4224, atManageSetup.c:1155). With `-1`, shallow boxes map onto layers shifted down by one and never use the top layer `nlayers-1`, so the initial pre-model recruits get the wrong vertical distribution. Function: `Init_Spawning()`.

**Code at issue** — `atannualbiology.c` line 603  
Search for: `layerk = k + diffdeep - 1`

```c
  601:                             sumnzj2 = 0.0;
  602:                             for (k = 0; k < bm->boxes[i].nz; k++) {
  603→                                 layerk = k + diffdeep - 1;
  604:                                 sumnzj1 += recruit_vdistrib[layerk][sp];
  605:                                 sumnzj2 += recruit_vdistrib[k][sp];
```

**Code at issue** — `atannualbiology.c` line 619  
Search for: `layerk = j + diffdeep - 1`

```c
  617:                                     vertdistrib = 0;
  618:                             } else {
  619→                                 layerk = j + diffdeep - 1;
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
**Found at (current code):** `atannualbiology.c` 1162, `atannualbiology.c` 653
**Lines at review (Oct 2026):** 1157-1159

**Function:** `Get_Init_Embryos()`

**Why it's thought to be a bug**

`boxkey_id = bm->BoxKeyMap[bm->current_box][rects_key_id];` — `Get_Init_Embryos()` is called from inside the box loop `i` of `Init_Spawning()`, but it has no box argument, so it uses the stale `bm->current_box` (whatever was last processed) for every box. Function: `Get_Init_Embryos()`.

**Code at issue** — `atannualbiology.c` line 1162  
Search for: `boxkey_id = bm->BoxKeyMap[bm->current_box][rects_key_id]`

```c
 1160:             break;
 1161:         case multiple_ts_recruit: /* Read in timeseries of recruitment - assume all genotypes get the same number of recruits */
 1162→             boxkey_id = bm->BoxKeyMap[bm->current_box][rects_key_id];
 1163:             EMBRYO[species].Larvae[stock_id][ngene][qid] +=  Get_Imposed_MultRecruits(bm, species, do_debug, boxkey_id, bm->logFile);
 1164:             break;
```

**Code at issue** — `atannualbiology.c` line 653  
Search for: `Get_Init_Embryos()`

```c
  651:                             } else {
  652:                                 tot_embryo = Get_Init_Embryos(bm, sp, ngene, stock_id, k, first_spawn, do_debug);
  653→                                 EMBRYO[sp].num_recruits[i][j][ngene][k] = tot_embryo * vertdistrib * bm->recruit_hdistrib[ngene][i][sp] / ngenes;  // Don't store the summation for new code as calculate Larvae in Get_Init_Embryos()
  654:                                 EMBRYO[sp].num_recruits_updating[i][j][ngene][k] = EMBRYO[sp].num_recruits[i][j][ngene][k];
  655:                             }
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
**Found at (current code):** `atannualbiology.c` 1182, `atannualbiology.c` 653
**Lines at review (Oct 2026):** 1098-1181 vs 1238

**Function:** `Get_Init_Embryos()`

**Why it's thought to be a bug**

The SR switch handles `case ice_wimd_based`, but the first (larvae) switch has no such case, so the run reaches `default: quit("No such flagrecruit defined...")` whenever `flag_old_embryo_init` is off and a larval-queue extension exists. The same omission appears in atdemography.c's first switch. Function: `Get_Init_Embryos()`.

**Code at issue** — `atannualbiology.c` line 1182  
Search for: `default:`

```c
 1180:             }
 1181:             break;
 1182→         default:
 1183:             quit("No such flagrecruit defined for vertebrates (i.e. value must be between 1 ands 10 at present)\n");
 1184:             break;
```

**Code at issue** — `atannualbiology.c` line 653  
Search for: `Get_Init_Embryos()`

```c
  651:                             } else {
  652:                                 tot_embryo = Get_Init_Embryos(bm, sp, ngene, stock_id, k, first_spawn, do_debug);
  653→                                 EMBRYO[sp].num_recruits[i][j][ngene][k] = tot_embryo * vertdistrib * bm->recruit_hdistrib[ngene][i][sp] / ngenes;  // Don't store the summation for new code as calculate Larvae in Get_Init_Embryos()
  654:                                 EMBRYO[sp].num_recruits_updating[i][j][ngene][k] = EMBRYO[sp].num_recruits[i][j][ngene][k];
  655:                             }
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
**Found at (current code):** `atannualbiology.c` 471, `atannualbiology.c` 649
**Lines at review (Oct 2026):** 471, 593-669

**Function:** `Init_Spawning()`

**Why it's thought to be a bug**

The larval-queue-extension block (recruit setup for k < larval_queue_extension) sits inside the `for nspawn < numSpawns` loop. With `flag_old_embryo_init`, `EMBRYO[sp].Larvae[stock_id][ngene][k] += num_recruits` (l.649) is accumulated again for each spawn event, so it counts recruits numSpawns times. The `num_recruits[..][k]` entries are also overwritten per nspawn, whatever spawn they belong to. Function: `Init_Spawning()`.

**Code at issue** — `atannualbiology.c` line 471 (closest match to the quoted code; wording differs slightly)  
Search for: `for( nspawn = 0; nspawn < FunctGroupArray[sp].numSpawns; nspawn++){  // As all queues rhe `

```c
  469:     
  470:     // Now shuffle based on needing to insert any from the spawns prior to the start of the model
  471→     for( nspawn = 0; nspawn < FunctGroupArray[sp].numSpawns; nspawn++){  // As all queues rhe same length
  472:         // Assume the genetypes can go at different times but that all cohorts per genetype go at the same time
  473:         // This is different to the old code where it was the cohort not the genetype that was being set
```

**Code at issue** — `atannualbiology.c` line 649 (closest match to the quoted code; wording differs slightly)  
Search for: `EMBRYO[sp].Larvae[stock_id][ngene][k] += EMBRYO[sp].num_recruits[i][j][ngene][k];`

```c
  647:                                 tot_embryo = KDENR[sp][stock_id];
  648:                                 EMBRYO[sp].num_recruits[i][j][ngene][k] = tot_embryo * prod_scalar * vertdistrib * bm->recruit_hdistrib[ngene][i][sp] / ngenes;
  649→                                 EMBRYO[sp].Larvae[stock_id][ngene][k] += EMBRYO[sp].num_recruits[i][j][ngene][k];
  650:                                 EMBRYO[sp].num_recruits_updating[i][j][ngene][k] = EMBRYO[sp].num_recruits[i][j][ngene][k];
  651:                             } else {
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
**Found at (current code):** `atannualbiology.c` 1110, `atannualbiology.c` 653
**Lines at review (Oct 2026):** 1106-1108

**Function:** `Get_Init_Embryos()`

**Why it's thought to be a bug**

`if(species == 9) { fprintf(bm->logFile,"Init for %s qid: ..."); }` — a hard-coded species index debug print in live code. Function: `Get_Init_Embryos()`.

**Code at issue** — `atannualbiology.c` line 1110  
Search for: `if(species == 9) {`

```c
 1108:                 EMBRYO[species].Larvae[stock_id][ngene][qid] = KDENR_sp / ng;
 1109:             
 1110→             if(species == 9) {
 1111:                 fprintf(bm->logFile,"Init for %s qid: %d ngene: %d Larvae: %e KDENR_sp: %e ng: %e\n", FunctGroupArray[species].groupCode, qid, ngene,  EMBRYO[species].Larvae[stock_id][ngene][qid], KDENR_sp, ng);
 1112:             }
```

**Code at issue** — `atannualbiology.c` line 653  
Search for: `Get_Init_Embryos()`

```c
  651:                             } else {
  652:                                 tot_embryo = Get_Init_Embryos(bm, sp, ngene, stock_id, k, first_spawn, do_debug);
  653→                                 EMBRYO[sp].num_recruits[i][j][ngene][k] = tot_embryo * vertdistrib * bm->recruit_hdistrib[ngene][i][sp] / ngenes;  // Don't store the summation for new code as calculate Larvae in Get_Init_Embryos()
  654:                                 EMBRYO[sp].num_recruits_updating[i][j][ngene][k] = EMBRYO[sp].num_recruits[i][j][ngene][k];
  655:                             }
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
**Found at (current code):** `atannualbiology.c` 798
**Lines at review (Oct 2026):** 924-934

**Function:** `PrintDates()`

**Why it's thought to be a bug**

The `do_debug` guard is commented out, so `PrintDates()` writes a line for every species × queue slot × cohort×genotype (called from l.718). The queue length is numSpawns*(years+…), so this can be thousands of lines per species at init. Function: `PrintDates()`.

**Code at issue** — `atannualbiology.c` line 798 (located by name only; check this is the right place)  
Search for: `stepsize = (int)(365 / FunctGroupArray[sp].numSpawns);`

```c
  796: 
  797:         // Now deal with oddness
  798→         stepsize = (int)(365 / FunctGroupArray[sp].numSpawns);
  799:         for (i = (maxage_id + 1); i < EMBRYO[sp].num_in_aging_queue; i++) {
  800:             step = i - maxage_id;
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
**Found at (current code):** `atannualbiology.c` 494
**Lines at review (Oct 2026):** 493-494 vs 518

**Function:** `Init_Spawning()`

**Why it's thought to be a bug**

Countback uses `temp_val -= 364` (and l.526-527 for recruits), but insertion subtracts `365 * i` (l.518, 560-561). For dates of about 729-730 days, countback gives 2 years while the insertion assumes 365-day years. Function: `Init_Spawning()`.

**Code at issue** — `atannualbiology.c` line 494  
Search for: `temp_val -= 364`

```c
  492:                 
  493:                 while ( temp_val > 364 ) {
  494→                     temp_val -= 364;
  495:                     countback_spawn++;
  496:                 }
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
**Found at (current code):** `atannualbiology.c` 752
**Lines at review (Oct 2026):** 752-755

**Function:** `Init_Spawning()`

**Why it's thought to be a bug**

`next_spawn_any_age > ((365.0 * bm->tstop) / 86400.0)` — `tstop/86400` is already days, and multiplying by 365 makes the upper bound meaningless (365x too lax). Function: `Init_Spawning()`.

**Code at issue** — `atannualbiology.c` line 752  
Search for: `next_spawn_any_age > ((365.0 * bm->tstop) / 86400.0)`

```c
  750:     
  751:     /* Sanity checks */
  752→     if ((EMBRYO[sp].next_spawn_any_age < 0) || (EMBRYO[sp].next_spawn_any_age > ((365.0 * bm->tstop) / 86400.0)))
  753:         quit("First spawning date for %s (%d) makes no sense - note testing against 0 and %e as tstop is : %e\n", FunctGroupArray[sp].groupCode, EMBRYO[sp].next_spawn_any_age, ((365.0 * bm->tstop) / 86400.0), bm->tstop);
  754:     if ((EMBRYO[sp].next_age_any_age < 0) || (EMBRYO[sp].next_age_any_age > ((365.0 * bm->tstop) / 86400.0)))
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
**Found at (current code):** `atannualbiology.c` 704
**Lines at review (Oct 2026):** 703-710

**Function:** `Init_Spawning()`

**Why it's thought to be a bug**

`num_steps = ceil(364.0 / AgeClassSize_sp)` should be `ceil(1.0 / AgeClassSize_sp)` (steps per year). As written, `stid` reaches far beyond the `Age_Now` queue (`[cohort][max_num]`, atbiolsetup.c:4032). This is currently unreachable because `ageClassSize` is an `int` (atFunctGroup.h:90), so it is always ≥1. Function: `Init_Spawning()`.

**Code at issue** — `atannualbiology.c` line 704 (closest match to the quoted code; wording differs slightly)  
Search for: `num_steps = (int) ceil(364.0 / AgeClassSize_sp);`

```c
  702: 
  703:                     stepsize = (int) floor(ROUNDGUARD + (364.0 * AgeClassSize_sp));
  704→                     num_steps = (int) ceil(364.0 / AgeClassSize_sp);
  705: 
  706:                     /* TODO: May need to put in a check that not accidentally wiping valid dates created in earlier years */
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
