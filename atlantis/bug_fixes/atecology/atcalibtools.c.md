# atecology/atcalibtools.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [EINIT-25](#einit-25) | 364-375, 382-389, 434-440 | M/M | off-by-one / OOB read | `qid = EMBRYO[sp].next_larvae; if (SpawnRecruitOverlap) qid++;` is then used in `Larvae[k][n][qid]` and `Start… |  | [ ] |
| [EINIT-26](#einit-26) | 486-538 | M/M | buffer overrun | `strcat(keywithin/keymore/keyless, key)` appends about 25-30 chars per prey group (e.g. `" CODE-12 (>SN 1.2345… |  | [ ] |

---

## EINIT-25

**off-by-one / OOB read** · Conf M / Sev M  
**Lines (current code):** 364-375, 382-389, 434-440  
**Lines at review (Oct 2026):** 364-375, 382-389, 434-440

**Function:** `Check_Gape()`

**Why it's thought to be a bug**

`qid = EMBRYO[sp].next_larvae; if (SpawnRecruitOverlap) qid++;` is then used in `Larvae[k][n][qid]` and `StartDay[n][qid]`. Both arrays have queue length `max_num = num_in_spawn_queue` (atbiolsetup.c:4035/3816). The guard `if (next_larvae > num_in_spawn_queue)` permits `next_larvae == num_in_spawn_queue`, and the `++` can add one more, so the reads can go past the end, most likely at `coming_to_end`. Function: `Check_Gape()`.

**Current code** (`atecology/atcalibtools.c` lines 362–391)

```c
  362: 					totrec = 0;
  363: 					totrecold = 0;
  364: 					qid = EMBRYO[sp].next_larvae;
  365:                     
  366:                     if(EMBRYO[sp].SpawnRecruitOverlap)
  367:                         qid++;
  368:                     qidi = qid - 1;
  369:                     if(qidi < 0)
  370:                         qidi = 0;
  371: 
  372:                     //printf("Last step Check_Gape W-%d %s qid: %d (SpawnRecruitOverlap: %d, next_larvae: %d)\n", sp, FunctGroupArray[sp].groupCode, qid, EMBRYO[sp].SpawnRecruitOverlap, EMBRYO[sp].next_larvae);
  373:                     
  374:                     if (EMBRYO[sp].next_larvae > EMBRYO[sp].num_in_spawn_queue)
  375:                         quit("EMBRYO[%s].next_larvae (%d) > EMBRYO[].num_in_spawn_queue %d)\n", FunctGroupArray[sp].groupCode, EMBRYO[sp].next_larvae, EMBRYO[sp].num_in_spawn_queue);
  376: 
  377:                     
  378:                     for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
  379:                         
  380:                         if (n < FunctGroupArray[sp].numGeneTypes) {
  381: 							for (k = 0; k < FunctGroupArray[sp].numStocks; k++) {
  382: 								totrec += EMBRYO[sp].Larvae[k][n][qid];
  383: 								totrecold += EMBRYO[sp].Larvae[k][n][qidi];
  384: 
  385:                                 //printf("Last step Check_Gape W-%d-%d %s k: %d qid: %d qidi: %d\n", sp, n, FunctGroupArray[sp].groupCode, k, qid, qidi);
  386:                                 
  387: 								if(EMBRYO[sp].Larvae[k][n][qid] || EMBRYO[sp].Larvae[k][n][expect_id])
  388: 									fprintf(llogfp,"%s stock %d, gene: %d, VERTembryo: %e, VERTembryoOld: %e\n",
  389: 											FunctGroupArray[sp].groupCode, k, n, EMBRYO[sp].Larvae[k][n][qid], EMBRYO[sp].Larvae[k][n][qidi]);
  390:                                 
  391:                                 
```

**Suggested fix (from review):** Guard with `if (qid >= num_in_spawn_queue) qid = num_in_spawn_queue - 1;` (and use `>=` in the check)

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

## EINIT-26

**buffer overrun** · Conf M / Sev M  
**Lines (current code):** 486-538  
**Lines at review (Oct 2026):** 486-538

**Function:** `Check_Gape()`

**Why it's thought to be a bug**

`strcat(keywithin/keymore/keyless, key)` appends about 25-30 chars per prey group (e.g. `" CODE-12 (>SN 1.234567e+00)"`) into `char[BMSLEN*2]` (1024?) buffers with no length check. A predator stage with more than about 35-40 linked vertebrate prey in one category overflows the stack buffer. Function: `Check_Gape()`.

**Current code** (`atecology/atcalibtools.c` lines 484–523 (truncated))

```c
  484: 							//snprintf(keymore, sizeof(keymore), " ");
  485: 							//snprintf(keyless, sizeof(keyless)," ");
  486: 							strcpy(keywithin, " ");
  487: 							strcpy(keymore, " ");
  488: 							strcpy(keyless, " ");
  489: 							for (prey = 0; prey < bm->K_num_tot_sp; prey++) { // Prey
  490: 								if (FunctGroupArray[prey].isVertebrate == TRUE && preychrt < FunctGroupArray[prey].numCohortsXnumGenes) {
  491: 									preyage = FunctGroupArray[prey].cohort_stage[preychrt];
  492: 
  493: 									/* Check to see if groups are active and if link is non-zero */
  494: 									if (FunctGroupArray[sp].speciesParams[flag_id] && FunctGroupArray[prey].speciesParams[flag_id]
  495: 											&& (bm->pSPVERTeat[sp][prey][chrtstage][preyage] > 0.0)) {
  496: 
  497: 										/* Check to see if fit in gape */
  498: 										if (VERTinfo[prey][preychrt][SN_id] >= FunctGroupArray[sp].speciesParams[KLP_id] * VERTinfo[sp][n][SN_id]) {
  499: 											/* Above lower gape */
  500: 											if (VERTinfo[prey][preychrt][SN_id] <= FunctGroupArray[sp].speciesParams[KUP_id] * VERTinfo[sp][n][SN_id]) {
  501: 												/* Less than upper gape */
  502: 
  503: 												//snprintf(key, sizeof(key), "%s %s-%d", keywithin, FunctGroupArray[prey].groupCode, preychrt);
  504: 												//snprintf(keywithin,sizeof(keywithin), "%s", key);
  505: 												snprintf(key, sizeof(key), " %s-%d", FunctGroupArray[prey].groupCode, preychrt);
  506: 												strcat(keywithin, key);
  507: 											} else {
  508: 												/* Above upper gape */
  509: 												if (VERTinfo[prey][preychrt][SN_id] > VERTinfo[sp][n][SN_id]) {
  510: 													//snprintf(key, sizeof(key), "%s %s-%d (>SN %e)", keymore, FunctGroupArray[prey].groupCode, preychrt,
  511: 													//		VERTinfo[prey][preychrt][SN_id] / VERTinfo[sp][n][SN_id]);
  512: 													//snprintf(keymore, sizeof(keymore), "%s", key);
  513: 
  514: 													snprintf(key, sizeof(key), " %s-%d (>SN %e)", FunctGroupArray[prey].groupCode, preychrt,
  515: 															VERTinfo[prey][preychrt][SN_id] / VERTinfo[sp][n][SN_id]);
  516: 
  517: 													strcat(keymore, key);
  518: 
  519: 												} else {
  520: 													//snprintf(key, sizeof(key), "%s %s-%d (%e)", keymore, FunctGroupArray[prey].groupCode, preychrt,
  521: 													//		VERTinfo[prey][preychrt][SN_id] / VERTinfo[sp][n][SN_id]);
  522: 													//snprintf(keymore, sizeof(keymore), "%s", key);
  523: 
```

**Suggested fix (from review):** Track remaining space and use `strncat`, or `snprintf` at an offset with a size check

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

atcalibtools.c:403: `Spawn_Now[n][indx]` and `StartDay[n][next_recruit]` use `n == numGeneTypes` (cohort 1, gene 0) to describe larvae. This is in range but probably not the intended row.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
