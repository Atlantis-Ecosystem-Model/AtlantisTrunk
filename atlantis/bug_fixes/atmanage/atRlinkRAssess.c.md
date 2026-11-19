# atmanage/atRlinkRAssess.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **5 findings** (severity H: 2; confidence H and severity H: 2; on the priority list: 2), plus 1 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MNGIO-35](#mngio-35) ⭐ | 180, 202-207 | H/H | uninitialised variable / accumulator not reset | `rawwgt` is never initialised and never reset per cohort/box before `if (this_wgt > rawwgt) rawwgt = this_wgt;… |  | [ ] |
| [MNGIO-36](#mngio-36) ⭐ | 213 | H/H | out-of-bounds layer | `rawn = bm->boxes[b].tr[k][den];` runs after the `for (k...)` loop, so k == nz, one layer past the last water … |  | [ ] |
| [MNGIO-37](#mngio-37) | 326-352 | H/M | PROTECT imbalance | Four objects are PROTECTed (infile, outfile, outdir, r_call), but only `UNPROTECT(3)` is called, so R's protec… |  | [ ] |
| [MNGIO-38](#mngio-38) | 118-119 | M/M | cohort vs age | `MeanWtAge[istock][isex][cohort][iyr]` and `MeanLenAge[istock][isex][cohort]` are age-indexed arrays (OverallM… |  | [ ] |
| [MNGIO-39](#mngio-39) | 282-287 | M/L | string compare / NULL | `strcmp(buffer, "TAC")` never matches because fgets keeps the "\n". The header is parsed as `atof("TAC")=0`, w… |  | [ ] |

---

## MNGIO-35

**uninitialised variable / accumulator not reset** · Conf H / Sev H ⭐ priority  
**Listed lines:** 180, 202-207

**Function:** `RAssessSurvey()`

**Why it's thought to be a bug**

`rawwgt` is never initialised and never reset per cohort/box before `if (this_wgt > rawwgt) rawwgt = this_wgt;`. The first comparison uses garbage, and later boxes and cohorts inherit the previous maximum. Function: `RAssessSurvey()`.

**Current code** (`atmanage/atRlinkRAssess.c` lines 178–209)

```c
  178:     int springSurvey = 0;
  179:     int sp, cohort, bb, b, den, sn, rn, k;
  180:     double sample_num, survey_num, q, avail, swept_area, rawwgt, rawlngth, rawn, this_wgt;
  181:     int fishery_id = bm->RAssessFisheryID;
  182: 
  183:     if (bm->TofY == bm->RAssessSpringSurveyDay) {
  184:         springSurvey = 1;
  185:     }
  186:     
  187:     // Survey the species
  188:     for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  189:         if (FunctGroupArray[sp].isTAC > 1) {
  190:             
  191:             printf("Doing RAssess Survey for %s\n", FunctGroupArray[sp].groupCode);
  192:             
  193:             for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++) {
  194:                 for (bb = 0; bb < bm->nsbox; bb++) {
  195:                     b = bm->nsboxes[bb];
  196: 
  197:                     den = FunctGroupArray[sp].NumsTracers[cohort];
  198:                     sn = FunctGroupArray[sp].structNTracers[cohort];
  199:                     rn = FunctGroupArray[sp].resNTracers[cohort];
  200:                 
  201:                     // Just use max size as do mixing in movement so undless sedentary then all same size anyways
  202:                     for (k = 0; k < bm->boxes[b].nz; k++){
  203:                         this_wgt = (bm->boxes[b].tr[k][sn] + bm->boxes[b].tr[k][rn]);
  204:                         if (this_wgt > rawwgt) {
  205:                             rawwgt = this_wgt;
  206:                         }
  207:                     }
  208:                 
  209:                     rawlngth = Get_Length(bm, rawwgt, sp);
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
**Listed lines:** 213

**Function:** `RAssessSurvey()`

**Why it's thought to be a bug**

`rawn = bm->boxes[b].tr[k][den];` runs after the `for (k...)` loop, so k == nz, one layer past the last water layer. Function: `RAssessSurvey()`.

**Current code** (`atmanage/atRlinkRAssess.c` lines 211–215)

```c
  211:                     q = Selectivity(bm, rawlngth, fishery_id, avail, 0, bm->logFile);  // Also accounts for availability
  212:                     swept_area = bm->FISHERYprms[fishery_id][swept_area_id] / (bm->boxes[b].area * bm->boxes[b].botz);
  213:                     rawn = bm->boxes[b].tr[k][den];
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
**Listed lines:** 326-352

**Function:** `Run_RAssess()`

**Why it's thought to be a bug**

Four objects are PROTECTed (infile, outfile, outdir, r_call), but only `UNPROTECT(3)` is called, so R's protect stack leaks on every call and eventually overflows. Function: `Run_RAssess()`.

**Current code** (`atmanage/atRlinkRAssess.c` lines 312–342) — code appears to have moved -12 lines since the review

```c
  312:     
  313:     if(idR < 0) {
  314:         quit("whichRAssess is negative for %s so can't complete Run_RAssess\n", FunctGroupArray[species].groupCode);
  315:     }
  316:     
  317:     if (bm->RAssessRuseScript) {
  318:         // Using a system call
  319:         snprintf(R_ScriptName, sizeof(R_ScriptName), "Rscript %s_%s", FunctGroupArray[species].groupCode, bm->RAssessRscriptName[idR]);
  320:         system(R_ScriptName);
  321:     } else {
  322:         // Using embedded R call
  323:         snprintf(inScriptName, sizeof(inScriptName), "%s_%s", FunctGroupArray[species].groupCode, bm->RAssessRscriptName[0]);
  324:         snprintf(outScriptName, sizeof(outScriptName), "%s_%s", FunctGroupArray[species].groupCode, bm->RAssessRoutName);
  325:         
  326:         SEXP infile = PROTECT(allocVector(STRSXP, 1));
  327:         SET_STRING_ELT(infile, 0, mkChar(inScriptName));
  328: 
  329:         SEXP outfile = PROTECT(allocVector(STRSXP, 1));
  330:         SET_STRING_ELT(outfile, 0, mkChar(outScriptName));
  331:         
  332:         SEXP outdir = PROTECT(allocVector(STRSXP, 1));
  333:         SET_STRING_ELT(outdir, 0, mkChar(bm->destFolder));
  334: 
  335:         // Setup a call to the R RAssess function, will need other function calls embedded in that?
  336:         SEXP r_call;
  337:         PROTECT(r_call = lang4(install("doRAssess"), infile, outfile, outdir));
  338: 
  339:         // Execute the function
  340:         SEXP ret = R_tryEval(r_call, R_GlobalEnv, &errorOccurred);
  341: 
  342:         if (!errorOccurred) {
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
**Listed lines:** 118-119

**Function:** `Populate_RAssessFile()`

**Why it's thought to be a bug**

`MeanWtAge[istock][isex][cohort][iyr]` and `MeanLenAge[istock][isex][cohort]` are age-indexed arrays (OverallMaxAge dim, atAssessTierSetup.c:100-101) but are indexed with the cohort number instead of `this_age`, which is wrong when ageClassSize>1. Function: `Populate_RAssessFile()`.

**Current code** (`atmanage/atRlinkRAssess.c` lines 116–121)

```c
  116:         for (istock = 0; istock < FunctGroupArray[groupIndex].numStocks; istock++){
  117:             for (isex = 0; isex < bm->K_num_sexes; isex++){
  118:                 this_size += bm->RBCestimation.RBCspeciesArray[groupIndex].MeanWtAge[istock][isex][cohort][iyr] * 1000.0; // convert weight to g from kg
  119:                 this_length += bm->RBCestimation.RBCspeciesArray[groupIndex].MeanLenAge[istock][isex][cohort];
  120:                 counter1 += 1.0;
  121:             }
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
**Listed lines:** 282-287

**Function:** `Read_RAssess_output()`

**Why it's thought to be a bug**

`strcmp(buffer, "TAC")` never matches because fgets keeps the "\n". The header is parsed as `atof("TAC")=0`, which is only fixed by the next line. `strtok` can return NULL (for example, a whitespace-only line), and then `strlen(valueStr)` crashes. Function: `Read_RAssess_output()`.

**Current code** (`atmanage/atRlinkRAssess.c` lines 280–289)

```c
  280:             continue;
  281: 
  282:         if (strcmp(buffer, "TAC") == 0){
  283:             // Do nothing as header row
  284:         } else {
  285:             valueStr = strtok(buffer, seps);
  286:             if(strlen(valueStr) != 0) {
  287:                 bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = atof(valueStr);
  288:             }
  289:         }
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
