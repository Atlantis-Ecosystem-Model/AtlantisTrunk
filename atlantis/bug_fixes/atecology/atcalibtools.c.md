# atecology/atcalibtools.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [EINIT-25](#einit-25) | 382, 440, 374 | M/M | off-by-one / OOB read | `qid = EMBRYO[sp].next_larvae; if (SpawnRecruitOverlap) qid++;` is then used in `Larvae[k][n][qid]` and `Start… |  | [ ] |
| [EINIT-26](#einit-26) | 506 | M/M | buffer overrun | `strcat(keywithin/keymore/keyless, key)` appends about 25-30 chars per prey group (e.g. `" CODE-12 (>SN 1.2345… |  | [ ] |

---

## EINIT-25

**off-by-one / OOB read** · Conf M / Sev M  
**Found at (current code):** `atcalibtools.c` 382, `atcalibtools.c` 440, `atcalibtools.c` 374
**Lines at review (Oct 2026):** 364-375, 382-389, 434-440

**Function:** `Check_Gape()`

**Why it's thought to be a bug**

`qid = EMBRYO[sp].next_larvae; if (SpawnRecruitOverlap) qid++;` is then used in `Larvae[k][n][qid]` and `StartDay[n][qid]`. Both arrays have queue length `max_num = num_in_spawn_queue` (atbiolsetup.c:4035/3816). The guard `if (next_larvae > num_in_spawn_queue)` permits `next_larvae == num_in_spawn_queue`, and the `++` can add one more, so the reads can go past the end, most likely at `coming_to_end`. Function: `Check_Gape()`.

**Code at issue** — `atcalibtools.c` line 382  
Search for: `Larvae[k][n][qid]`

```c
  380:                         if (n < FunctGroupArray[sp].numGeneTypes) {
  381: 							for (k = 0; k < FunctGroupArray[sp].numStocks; k++) {
  382→ 								totrec += EMBRYO[sp].Larvae[k][n][qid];
  383: 								totrecold += EMBRYO[sp].Larvae[k][n][qidi];
  384: 
```

**Code at issue** — `atcalibtools.c` line 440  
Search for: `StartDay[n][qid]`

```c
  438:                         // TODO: Fx this for new queues
  439: 						for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
  440→ 							recdate = EMBRYO[sp].StartDay[n][qid];
  441:                             
  442: 							fprintf(llogfp, "d: %d, AgeDate: %d, SpawnDate: %d, RecruitDate:%d ageclass_size: %e\n", bm->TofY,
```

**Code at issue** — `atcalibtools.c` line 374 (closest match to the quoted code; wording differs slightly)  
Search for: `if (EMBRYO[sp].next_larvae > EMBRYO[sp].num_in_spawn_queue)`

```c
  372:                     //printf("Last step Check_Gape W-%d %s qid: %d (SpawnRecruitOverlap: %d, next_larvae: %d)\n", sp, FunctGroupArray[sp].groupCode, qid, EMBRYO[sp].SpawnRecruitOverlap, EMBRYO[sp].next_larvae);
  373:                     
  374→                     if (EMBRYO[sp].next_larvae > EMBRYO[sp].num_in_spawn_queue)
  375:                         quit("EMBRYO[%s].next_larvae (%d) > EMBRYO[].num_in_spawn_queue %d)\n", FunctGroupArray[sp].groupCode, EMBRYO[sp].next_larvae, EMBRYO[sp].num_in_spawn_queue);
  376: 
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
**Found at (current code):** `atcalibtools.c` 506
**Lines at review (Oct 2026):** 486-538

**Function:** `Check_Gape()`

**Why it's thought to be a bug**

`strcat(keywithin/keymore/keyless, key)` appends about 25-30 chars per prey group (e.g. `" CODE-12 (>SN 1.234567e+00)"`) into `char[BMSLEN*2]` (1024?) buffers with no length check. A predator stage with more than about 35-40 linked vertebrate prey in one category overflows the stack buffer. Function: `Check_Gape()`.

**Code at issue** — `atcalibtools.c` line 506 (located by name only; check this is the right place)  
Search for: `strcat(keywithin, key);`

```c
  504: 												//snprintf(keywithin,sizeof(keywithin), "%s", key);
  505: 												snprintf(key, sizeof(key), " %s-%d", FunctGroupArray[prey].groupCode, preychrt);
  506→ 												strcat(keywithin, key);
  507: 											} else {
  508: 												/* Above upper gape */
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
