# atmanage/atRlinkRAssess.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **5 findings** (severity H: 2; confidence H and severity H: 2; on the priority list: 2), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MNGIO-35](#mngio-35) ⭐ | 180, 204 | H/H | uninitialised variable / accumulator not reset | `rawwgt` is never initialised and never reset per cohort/box before `if (this_wgt > rawwgt) rawwgt = this_wgt;… |  | [ ] |
| [MNGIO-36](#mngio-36) ⭐ | 213 | H/H | out-of-bounds layer | `rawn = bm->boxes[b].tr[k][den];` runs after the `for (k...)` loop, so k == nz, one layer past the last water … |  | [ ] |
| [MNGIO-37](#mngio-37) | 352 | H/M | PROTECT imbalance | Four objects are PROTECTed (infile, outfile, outdir, r_call), but only `UNPROTECT(3)` is called, so R's protec… |  | [ ] |
| [MNGIO-38](#mngio-38) | 118 | M/M | cohort vs age | `MeanWtAge[istock][isex][cohort][iyr]` and `MeanLenAge[istock][isex][cohort]` are age-indexed arrays (OverallM… |  | [ ] |
| [MNGIO-39](#mngio-39) | 282, 286 | M/L | string compare / NULL | `strcmp(buffer, "TAC")` never matches because fgets keeps the "\n". The header is parsed as `atof("TAC")=0`, w… |  | [ ] |

---

## MNGIO-35

**uninitialised variable / accumulator not reset** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atRlinkRAssess.c` 180, `atRlinkRAssess.c` 204
**Lines at review (Oct 2026):** 180, 202-207

**Function:** `RAssessSurvey()`

**Why it's thought to be a bug**

`rawwgt` is never initialised and never reset per cohort/box before `if (this_wgt > rawwgt) rawwgt = this_wgt;`. The first comparison uses garbage, and later boxes and cohorts inherit the previous maximum. Function: `RAssessSurvey()`.

**Code at issue** — `atRlinkRAssess.c` line 180 (the line the review pointed to, found by its text)  
Search for: `double sample_num, survey_num, q, avail, swept_area, rawwgt, rawlngth, rawn, this_wgt;`

```c
  178:     int springSurvey = 0;
  179:     int sp, cohort, bb, b, den, sn, rn, k;
  180→     double sample_num, survey_num, q, avail, swept_area, rawwgt, rawlngth, rawn, this_wgt;
  181:     int fishery_id = bm->RAssessFisheryID;
  182: 
```

**Code at issue** — `atRlinkRAssess.c` line 204 (the line the review pointed to, found by its text)  
Search for: `if (this_wgt > rawwgt) {`

```c
  202:                     for (k = 0; k < bm->boxes[b].nz; k++){
  203:                         this_wgt = (bm->boxes[b].tr[k][sn] + bm->boxes[b].tr[k][rn]);
  204→                         if (this_wgt > rawwgt) {
  205:                             rawwgt = this_wgt;
  206:                         }
```

**Suggested fix (from review):** Set `rawwgt = 0.0` before the k loop.

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

## MNGIO-36

**out-of-bounds layer** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atRlinkRAssess.c` 213
**Lines at review (Oct 2026):** 213

**Function:** `RAssessSurvey()`

**Why it's thought to be a bug**

`rawn = bm->boxes[b].tr[k][den];` runs after the `for (k...)` loop, so k == nz, one layer past the last water layer. Function: `RAssessSurvey()`.

**Code at issue** — `atRlinkRAssess.c` line 213  
Search for: `rawn = bm->boxes[b].tr[k][den]`

```c
  211:                     q = Selectivity(bm, rawlngth, fishery_id, avail, 0, bm->logFile);  // Also accounts for availability
  212:                     swept_area = bm->FISHERYprms[fishery_id][swept_area_id] / (bm->boxes[b].area * bm->boxes[b].botz);
  213→                     rawn = bm->boxes[b].tr[k][den];
  214:                     sample_num = Assess_Add_Error(bm, flagcount, rawn, k_avgcount, k_varcount);
  215:                     survey_num = sample_num * q * swept_area;
```

**Suggested fix (from review):** Sum `tr[k][den]` over k inside the loop (or use the intended layer).

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

## MNGIO-37

**PROTECT imbalance** · Conf H / Sev M  
**Found at (current code):** `atRlinkRAssess.c` 352
**Lines at review (Oct 2026):** 326-352

**Function:** `Run_RAssess()`

**Why it's thought to be a bug**

Four objects are PROTECTed (infile, outfile, outdir, r_call), but only `UNPROTECT(3)` is called, so R's protect stack leaks on every call and eventually overflows. Function: `Run_RAssess()`.

**Code at issue** — `atRlinkRAssess.c` line 352  
Search for: `UNPROTECT(3)`

```c
  350:         }
  351: 
  352→         UNPROTECT(3);
  353: 
  354:         // Check interrupts
```

**Suggested fix (from review):** `UNPROTECT(4)`.

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

## MNGIO-38

**cohort vs age** · Conf M / Sev M  
**Found at (current code):** `atRlinkRAssess.c` 118
**Lines at review (Oct 2026):** 118-119

**Function:** `Populate_RAssessFile()`

**Why it's thought to be a bug**

`MeanWtAge[istock][isex][cohort][iyr]` and `MeanLenAge[istock][isex][cohort]` are age-indexed arrays (OverallMaxAge dim, atAssessTierSetup.c:100-101) but are indexed with the cohort number instead of `this_age`, which is wrong when ageClassSize>1. Function: `Populate_RAssessFile()`.

**Code at issue** — `atRlinkRAssess.c` line 118  
Search for: `MeanWtAge[istock][isex][cohort][iyr]`

```c
  116:         for (istock = 0; istock < FunctGroupArray[groupIndex].numStocks; istock++){
  117:             for (isex = 0; isex < bm->K_num_sexes; isex++){
  118→                 this_size += bm->RBCestimation.RBCspeciesArray[groupIndex].MeanWtAge[istock][isex][cohort][iyr] * 1000.0; // convert weight to g from kg
  119:                 this_length += bm->RBCestimation.RBCspeciesArray[groupIndex].MeanLenAge[istock][isex][cohort];
  120:                 counter1 += 1.0;
```

**Suggested fix (from review):** Index with `this_age`.

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

## MNGIO-39

**string compare / NULL** · Conf M / Sev L  
**Found at (current code):** `atRlinkRAssess.c` 282, `atRlinkRAssess.c` 286
**Lines at review (Oct 2026):** 282-287

**Function:** `Read_RAssess_output()`

**Why it's thought to be a bug**

`strcmp(buffer, "TAC")` never matches because fgets keeps the "\n". The header is parsed as `atof("TAC")=0`, which is only fixed by the next line. `strtok` can return NULL (for example, a whitespace-only line), and then `strlen(valueStr)` crashes. Function: `Read_RAssess_output()`.

**Code at issue** — `atRlinkRAssess.c` line 282  
Search for: `strcmp(buffer, "TAC")`

```c
  280:             continue;
  281: 
  282→         if (strcmp(buffer, "TAC") == 0){
  283:             // Do nothing as header row
  284:         } else {
```

**Code at issue** — `atRlinkRAssess.c` line 286  
Search for: `strlen(valueStr)`

```c
  284:         } else {
  285:             valueStr = strtok(buffer, seps);
  286→             if(strlen(valueStr) != 0) {
  287:                 bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = atof(valueStr);
  288:             }
```

**Suggested fix (from review):** Strip the newline before comparing and check valueStr != NULL.

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

atRlinkRAssess.c:212 `swept_area = ... / (area * botz)`: botz is negative in Atlantis, so survey numbers would be negative unless the sign is handled elsewhere.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
