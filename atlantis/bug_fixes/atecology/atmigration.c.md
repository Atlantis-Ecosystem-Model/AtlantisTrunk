# atecology/atmigration.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **12 findings** (severity H: 2; confidence H and severity H: 2; on the priority list: 2), plus 3 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [EINIT-01](#einit-01) ⭐ | 204-207 | H/H | wrong variable / copy-paste | `MIGRATION[sp].all_go[cohort][currentID] = MIGRATION[sp].all_go[cohort][migrationID];` copies `all_go` from it… |  | [ ] |
| [EINIT-02](#einit-02) | 92-150 | H/M | logic / always-true condition | The pairwise check loops over all ordered pairs (migrationID, migID) and does not skip empty (0/0) slots. (a) … |  | [ ] |
| [EINIT-03](#einit-03) ⭐ | 158-421 | H/H | buffer overrun (queue size) | For multi-year migrators the year-0 "start with return" branch writes `this_currentID = currentID + counter*nu… |  | [ ] |
| [EINIT-04](#einit-04) | 191-202 | M/M | cohort vs cohort×genotype | `nagemat` (age_mat, in cohort units) is used directly as a bound and start index in cohort×genotype space: `fo… |  | [ ] |
| [EINIT-05](#einit-05) | 164-166 | L/M | logic (break vs continue) | `if(!StartDay_Prm[stage][migrationID] && !EndDay_Prm[stage][migrationID]) break;` stops processing all later m… |  | [ ] |
| [EINIT-06](#einit-06) | 585-596 | M/M | logic (max overwritten) | In the loop computing the maximum years away, the `else` branch sets `max_years = 1;` whenever an entry is ann… |  | [ ] |
| [EINIT-07](#einit-07) | 600 | M/L | integer division / cohort vs genotype | `num_cohorts_away = ceil(b + (max_years / FunctGroupArray[sp].ageClassSize));` divides two ints (`ageClassSize… |  | [ ] |
| [EINIT-08](#einit-08) | 630-631 | H/M | unit / genotype split | `SN[b][counter] = InitSN[b][0] / ngenes; RN[b][counter] = InitRN[b][0] / ngenes;` SN and RN are per-individual… |  | [ ] |
| [EINIT-09](#einit-09) | 692-694 | H/L | wrong variable in message | `quit("... StaggerYears cannot be set to %d ...", (int)(FunctGroupArray[sp].speciesParams[stagger_years_id]), … |  | [ ] |
| [EINIT-10](#einit-10) | 740-753 | M/M | loop logic / overwritten result | `end_pt[qid] = cohort + yrs_to_age_pre_model[qid]` is assigned inside a `cohort` loop running from top to 0, s… |  | [ ] |
| [EINIT-11](#einit-11) | 743 | H/M | integer division | `diff_nc = (int)ceil((Return_Now[qid] - Leave_Now[qid]) / 365);` Both operands are int, so the division trunca… |  | [ ] |
| [EINIT-12](#einit-12) | 175, 304 | H/L | log noise | `printf("Doing migration entry %d ...")` and `printf("Step1 Doing currentID ...")` are unconditional and run f… |  | [ ] |

---

## EINIT-01

**wrong variable / copy-paste** · Conf H / Sev H ⭐ priority  
**Listed lines:** 204-207

**Function:** `Init_Migration()`

**Why it's thought to be a bug**

`MIGRATION[sp].all_go[cohort][currentID] = MIGRATION[sp].all_go[cohort][migrationID];` copies `all_go` from itself. `all_go` is allocated zeroed (atBiologyXMLParamIO.c:4353), and the parameter values are only ever written to `all_go_Prm[stage][migrate]` (atBiologyXMLParamIO.c:4121, 4143). This line is the only writer of `all_go` anywhere in the repo, so `all_go` stays 0 for every queue entry. The "whole cohort leaves" branches in atdemography.c:3062/3086 (`if (MIGRATION[species].all_go[cohort][qid] && ...)`) therefore never fire. Function: `Init_Migration()`.

**Current code** (`atecology/atmigration.c` lines 202–209)

```c
  202:                 }
  203:                 
  204:                 for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
  205:                     MIGRATION[sp].all_go[cohort][currentID] = MIGRATION[sp].all_go[cohort][migrationID];
  206:                     MIGRATION[sp].ReprodAllowed[cohort][currentID] = MIGRATION[sp].ReprodAllowedPrm[stage][migrationID];
  207:                 }
  208:                 
  209:                 for(ij=0; ij<bm->nbox; ij++) {
```

**Suggested fix (from review):** `MIGRATION[sp].all_go[cohort][currentID] = MIGRATION[sp].all_go_Prm[stage][migrationID];` (`all_go_Prm` is filled by stage index)

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

## EINIT-02

**logic / always-true condition** · Conf H / Sev M  
**Listed lines:** 92-150

**Function:** `Init_Migration()`

**Why it's thought to be a bug**

The pairwise check loops over all ordered pairs (migrationID, migID) and does not skip empty (0/0) slots. (a) The quit test `(sp_Migrate_Time_orig > sp_Migrate_Return_chk) && (sp_Migrate_Time > sp_Migrate_Return_chk)` repeats the same condition (Time is never modified after `_orig` is set). It fires whenever entry migrationID starts after migID returns. So two correctly ordered, non-overlapping migrations (e.g. 10-50 and 100-150) quit when evaluated as (1,0), and so does any real entry compared against an unused 0/0 slot of the same stage. The message also says "happens completely before", which is the opposite of what is tested. (b) The overlap tests at 126 (`Return > Time_chk`) and 134 (`Time < Return_chk`) are each only half of an interval-overlap test, so nearly every species with num_migrate>1 gets `isComplexMigrator = TRUE`. Line 116 is unreachable after `quit`. Function: `Init_Migration()`.

**Current code** (`atecology/atmigration.c` lines 90–129 (truncated))

```c
   90:                 sp_Migrate_Return += 365;
   91:             
   92:             for(migID = 0; migID < FunctGroupArray[sp].num_migrate; migID++){
   93:                 if (migrationID == migID)
   94:                     continue;  // Do't check against same ID entry
   95:                 
   96:                 sp_Migrate_Time_chk = (int) (MIGRATION[sp].StartDay_Prm[stage][migID]);
   97:                 sp_Migrate_Return_chk = (int) (MIGRATION[sp].EndDay_Prm[stage][migID]);
   98:                 
   99:                 // Get it so all in linear time so if Return < Start due to looping calendar unwind
  100:                 if (sp_Migrate_Time_chk > sp_Migrate_Return_chk)
  101:                     sp_Migrate_Return_chk += 365;
  102:                 
  103:                 //fprintf(llogfp, "%s stage %d migrationID %d sp_Migrate_Time %d sp_Migrate_Return %d (orig %d) num_mig: %d migID: %d sp_Migrate_Time_chk: %d sp_Migrate_Return_chk: %d\n", FunctGroupArray[sp].groupCode, stage, migrationID, sp_Migrate_Time, sp_Migrate_Return, (int)(MIGRATION[sp].EndDay_Prm[stage][migrationID]), FunctGroupArray[sp].num_migrate, migID, sp_Migrate_Time_chk, sp_Migrate_Return_chk);
  104:                 
  105:                 
  106:                 // Now check each case
  107:                 if ((sp_Migrate_Time_orig > sp_Migrate_Return_chk) && (sp_Migrate_Time > sp_Migrate_Return_chk)) {
  108:                     
  109:                     /* This is case where the second entry happens before first entry
  110:                      1.               |-----|
  111:                      2.    |-----|
  112:                      This is more sensibly handled by reversing the order in the parameter file - will run faster and less chance of a cock up
  113:                      */
  114:                     quit("The migration entry %d (starting %d) for %s-%d happens completely before migration entry %d (return starting %d EndDay %d). Please fix that in the parameter file as it will help the code run faster and there is less chance something will go wrong\n", migrationID, sp_Migrate_Time, FunctGroupArray[sp].groupCode, stage, migID, sp_Migrate_Return_chk, MIGRATION[sp].EndDay_Prm[stage][migID]);
  115:                     
  116:                     FunctGroupArray[sp].isComplexMigrator = TRUE;
  117:                 }
  118:                 if ((sp_Migrate_Time < sp_Migrate_Time_chk) && (sp_Migrate_Return > sp_Migrate_Return_chk)) {
  119:                     
  120:                     /* This is case where the second entry happens within the time period of the first entry
  121:                      1.     |---------------|
  122:                      2.         |-----|
  123:                      */
  124:                     FunctGroupArray[sp].isComplexMigrator = TRUE;
  125:                 }
  126:                 if (sp_Migrate_Return > sp_Migrate_Time_chk) {
  127:                     
  128:                     /* This is case where the first entry end overlaps with the start time period of the second  entry
  129:                      1.     |-----|
```

**Suggested fix (from review):** Only test `migID > migrationID`, skip 0/0 slots (as line 164 does), and use `(Time < Return_chk && Return > Time_chk)` for overlap

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

## EINIT-03

**buffer overrun (queue size)** · Conf H / Sev H ⭐ priority  
**Listed lines:** 158-421

**Function:** `Init_Migration()`

**Why it's thought to be a bug**

For multi-year migrators the year-0 "start with return" branch writes `this_currentID = currentID + counter*num_migrate` for `counter < MinYearsAway`, then `last_currentID` jumps to that value, so every (stage, migration) at yr 0 uses `(Y-1)*nm+1` slots. Total slots = `S*nm*((Y-1)*nm + nyears)`, but the arrays hold `maxnum = nyears*S*(nm+1)` (atBiologyXMLParamIO.c:4334). This overflows when `nm^2*(Y-1) > nyears`, e.g. nm=1, Y=5 on a run of 3 years or less, or nm=2, Y=3 on a run under 8 years. Nothing bounds `currentID`/`this_currentID` against `num_in_queue`, so Leave_Now, Return_Now, all_go, Box, Stagger, etc. are written past their allocation (heap corruption). Function: `Init_Migration()`.

**Current code** (`atecology/atmigration.c` lines 180–214 (range truncated)) — code appears to have moved +24 lines since the review

```c
  180:                 MIGRATION[sp].Return_Period[currentID] = MIGRATION[sp].Return_Period_Prm[stage][migrationID];
  181:                 MIGRATION[sp].MinYearsAway[currentID] = MIGRATION[sp].MinYearsAway_Prm[stage][migrationID];
  182:                 MIGRATION[sp].MaxYearsAway[currentID] = MIGRATION[sp].MaxYearsAway_Prm[stage][migrationID];
  183:                 MIGRATION[sp].survival[currentID] = MIGRATION[sp].survival_Prm[stage][migrationID];
  184:                 MIGRATION[sp].growth[currentID] = MIGRATION[sp].growth_Prm[stage][migrationID];
  185:                 MIGRATION[sp].returnstock[currentID] = MIGRATION[sp].returnstock_Prm[stage][migrationID];
  186:                 
  187:                 MIGRATION[sp].IsPartialMigration[currentID] = MIGRATION[sp].IsPartialMigration_Prm[stage][migrationID];
  188:                 MIGRATION[sp].PartialMinAge[currentID] = MIGRATION[sp].PartialMigration_MinPrm[stage][migrationID];
  189:                 MIGRATION[sp].PartialMaxAge[currentID] = MIGRATION[sp].PartialMigration_MaxPrm[stage][migrationID];
  190:                 
  191:                 if(!stage) {
  192:                     MIGRATION[sp].start_cohort[currentID] = 0;
  193:                     for(cohort = 0; cohort < nagemat; cohort++) {
  194:                         MIGRATION[sp].cohort_migrating[cohort][currentID] = 1; // Really only used in Leave_Now so ok if don't set to 1 for age classes might age into while away
  195:                         MIGRATION[sp].ReprodAllowed[cohort][currentID] = MIGRATION[sp].ReprodAllowedPrm[stage][migrationID];
  196:                     }
  197:                 } else {
  198:                     MIGRATION[sp].start_cohort[currentID] = nagemat;
  199:                     for(cohort = nagemat; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++) {
  200:                         MIGRATION[sp].cohort_migrating[cohort][currentID] = 1;
  201:                     }
  202:                 }
  203:                 
  204:                 for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
  205:                     MIGRATION[sp].all_go[cohort][currentID] = MIGRATION[sp].all_go[cohort][migrationID];
  206:                     MIGRATION[sp].ReprodAllowed[cohort][currentID] = MIGRATION[sp].ReprodAllowedPrm[stage][migrationID];
  207:                 }
  208:                 
  209:                 for(ij=0; ij<bm->nbox; ij++) {
  210:                     MIGRATION[sp].Box[ij][currentID] = MIGRATION[sp].Box_Prm[ij][stage][migrationID];
  211:                 }
  212:                 
  213:                 MIGRATION[sp].Stagger[currentID][stagger_years_id] = MIGRATION[sp].Stagger_Prm[stage][migrationID][stagger_years_id];
  214:                 MIGRATION[sp].Stagger[currentID][return_stagger_id] = MIGRATION[sp].Stagger_Prm[stage][migrationID][return_stagger_id];
```

**Suggested fix (from review):** Add `if (this_currentID >= MIGRATION[sp].num_in_queue) quit(...)`, and size maxnum (and the Check_Migration:550 num_in_queue) to include `S*nm*nm*(MaxYearsAway-1)`

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

## EINIT-04

**cohort vs cohort×genotype** · Conf M / Sev M  
**Listed lines:** 191-202

**Function:** `Init_Migration()`

**Why it's thought to be a bug**

`nagemat` (age_mat, in cohort units) is used directly as a bound and start index in cohort×genotype space: `for(cohort = 0; cohort < nagemat; ...)` and `for(cohort = nagemat; cohort < numCohortsXnumGenes; ...)`, plus `start_cohort = nagemat`. When numGeneTypes>1, juveniles are flagged only for the first `nagemat` cohort×gene slots, and adult flags and `start_cohort` begin at the wrong slot. This also assumes exactly two stages (`!stage` vs else). The same `start_cohort` feeds `EMBRYO.Age_Now[cohort]` and the aging loops in Init_Migration_Age_Check. Function: `Init_Migration()`.

**Current code** (`atecology/atmigration.c` lines 189–204)

```c
  189:                 MIGRATION[sp].PartialMaxAge[currentID] = MIGRATION[sp].PartialMigration_MaxPrm[stage][migrationID];
  190:                 
  191:                 if(!stage) {
  192:                     MIGRATION[sp].start_cohort[currentID] = 0;
  193:                     for(cohort = 0; cohort < nagemat; cohort++) {
  194:                         MIGRATION[sp].cohort_migrating[cohort][currentID] = 1; // Really only used in Leave_Now so ok if don't set to 1 for age classes might age into while away
  195:                         MIGRATION[sp].ReprodAllowed[cohort][currentID] = MIGRATION[sp].ReprodAllowedPrm[stage][migrationID];
  196:                     }
  197:                 } else {
  198:                     MIGRATION[sp].start_cohort[currentID] = nagemat;
  199:                     for(cohort = nagemat; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++) {
  200:                         MIGRATION[sp].cohort_migrating[cohort][currentID] = 1;
  201:                     }
  202:                 }
  203:                 
  204:                 for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
```

**Suggested fix (from review):** Use `nagemat * FunctGroupArray[sp].numGeneTypes` for the bounds and `start_cohort`

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

## EINIT-05

**logic (break vs continue)** · Conf L / Sev M  
**Listed lines:** 164-166

**Function:** `Init_Migration()`

**Why it's thought to be a bug**

`if(!StartDay_Prm[stage][migrationID] && !EndDay_Prm[stage][migrationID]) break;` stops processing all later migration entries for this stage. If entry 0 of a stage is unused but entry 1 is real, the real migration is silently dropped. Function: `Init_Migration()`.

**Current code** (`atecology/atmigration.c` lines 162–168)

```c
  162:             for(migrationID = 0; migrationID < FunctGroupArray[sp].num_migrate; migrationID++){
  163:                 /* First check if even for real */
  164:                 if(!MIGRATION[sp].StartDay_Prm[stage][migrationID] && !MIGRATION[sp].EndDay_Prm[stage][migrationID]){
  165:                     break;
  166:                 }
  167: 
  168:                 // If yes then we can continue
```

**Suggested fix (from review):** Use `continue` unless entries are guaranteed to be packed

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

## EINIT-06

**logic (max overwritten)** · Conf M / Sev M  
**Listed lines:** 585-596

**Function:** `Check_Migration()`

**Why it's thought to be a bug**

In the loop computing the maximum years away, the `else` branch sets `max_years = 1;` whenever an entry is annual. A larger MaxYearsAway found in an earlier entry is discarded, so `num_away`, `num_cohorts_away` and `needed_for_age_away` are under-computed. Function: `Check_Migration()`.

**Current code** (`atecology/atmigration.c` lines 583–598)

```c
  583:             if (FunctGroupArray[sp].multiyr_mig){
  584:                 max_years = -MAXINT;
  585:                 for(qid = 0; qid < num_mig; qid++){
  586:                     if (MIGRATION[sp].IsAnnualMigration_Prm[stage][qid] < 1){
  587:                         if (MIGRATION[sp].MaxYearsAway_Prm[stage][qid] > max_years) {
  588:                             max_years = MIGRATION[sp].MaxYearsAway_Prm[stage][qid];
  589:                         }
  590:                     } else {
  591:                         max_years = 1;
  592:                     }
  593:                 }
  594:                 if (max_years < 0) {
  595:                     max_years = 1;
  596:                 }
  597:                 num_away *= max_years;
  598:                 
```

**Suggested fix (from review):** `else max_years = max(max_years, 1);`

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

## EINIT-07

**integer division / cohort vs genotype** · Conf M / Sev L  
**Listed lines:** 600

**Function:** `Check_Migration()`

**Why it's thought to be a bug**

`num_cohorts_away = ceil(b + (max_years / FunctGroupArray[sp].ageClassSize));` divides two ints (`ageClassSize` is int, atFunctGroup.h:90), so the `ceil` does nothing and any max_years < ageClassSize adds 0 cohorts. The cohort offset is also added to a cohort×gene index `b` without multiplying by numGeneTypes. Function: `Check_Migration()`.

**Current code** (`atecology/atmigration.c` lines 598–602)

```c
  598:                 
  599:                 // Check if needed due to aging in multiyear migration
  600:                 num_cohorts_away = ceil(b + (max_years / FunctGroupArray[sp].ageClassSize));
  601:                 if (num_cohorts_away > FunctGroupArray[sp].numCohortsXnumGenes) {
  602:                     num_cohorts_away = FunctGroupArray[sp].numCohortsXnumGenes;
```

**Suggested fix (from review):** `b + numGeneTypes * (int)ceil((double)max_years / ageClassSize)`

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

## EINIT-08

**unit / genotype split** · Conf H / Sev M  
**Listed lines:** 630-631

**Function:** `Check_Migration()`

**Why it's thought to be a bug**

`SN[b][counter] = InitSN[b][0] / ngenes; RN[b][counter] = InitRN[b][0] / ngenes;` SN and RN are per-individual sizes (mg N per fish). The KMIG reader already copies the same per-individual value to every genotype (atBiologyXMLParamIO.c:4188-4194, `values[basechrt]`). Dividing the size by ngenes makes returning migrants 1/ngenes the correct weight whenever numGeneTypes>1. Only DEN (numbers) should be split. Function: `Check_Migration()`.

**Current code** (`atecology/atmigration.c` lines 628–633)

```c
  628:                     
  629:                     // TODO: Think if this is the best way of handling all the migration options - all away or only a cohort at a time
  630:                     MIGRATION[sp].SN[b][counter] = MIGRATION[sp].InitSN[b][0] / ngenes;   // As all KMIG saved Init [0] entry on read in
  631:                     MIGRATION[sp].RN[b][counter] = MIGRATION[sp].InitRN[b][0] / ngenes;
  632:                     MIGRATION[sp].DEN[b][counter] = MIGRATION[sp].InitDEN[b][0] / ngenes;
  633: 
```

**Suggested fix (from review):** Remove `/ ngenes` from the SN and RN lines and keep it on DEN

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

## EINIT-09

**wrong variable in message** · Conf H / Sev L  
**Listed lines:** 692-694

**Function:** `Init_Migration_Age_Check()`

**Why it's thought to be a bug**

`quit("... StaggerYears cannot be set to %d ...", (int)(FunctGroupArray[sp].speciesParams[stagger_years_id]), ...)`. `stagger_years_id` (=1, atlantisboxmodel.h:753) is a column index into `MIGRATION.Stagger`, not a speciesParams id, so the message prints an unrelated species parameter. Function: `Init_Migration_Age_Check()`.

**Current code** (`atecology/atmigration.c` lines 690–696)

```c
  690:     // For each migration check how many aging events occur while away
  691:     for (qid = 0; qid < MIGRATION[sp].num_in_queue; qid++) {
  692:         if (MIGRATION[sp].Stagger[qid][return_stagger_id] && !MIGRATION[sp].Stagger[qid][stagger_years_id]) {
  693:                 quit("For %s StaggerYears cannot be set to %d if stagger_return is set to %d, please fix the parametersation\n", FunctGroupArray[sp].groupCode, (int)(FunctGroupArray[sp].speciesParams[stagger_years_id]), MIGRATION[sp].Stagger[qid][return_stagger_id]);
  694:         }
  695:         
  696:         age_away = 0;
```

**Suggested fix (from review):** Print `MIGRATION[sp].Stagger[qid][stagger_years_id]`

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

## EINIT-10

**loop logic / overwritten result** · Conf M / Sev M  
**Listed lines:** 740-753

**Function:** `Init_Migration_Age_Check()`

**Why it's thought to be a bug**

`end_pt[qid] = cohort + yrs_to_age_pre_model[qid]` is assigned inside a `cohort` loop running from top to 0, so only the cohort 0 value survives and `end_pt = yrs_to_age_pre_model`. The aging loop at 757 (`cohort = end_pt-1 .. start_cohort`) then does nothing for adult-stage entries (start_cohort = nagemat > end_pt-1). For juvenile entries it only ages the lowest few slots, so pre-model aging of migrants is mostly skipped. Function: `Init_Migration_Age_Check()`.

**Current code** (`atecology/atmigration.c` lines 738–755)

```c
  738:         
  739:     // Sort the aging that happens before the run starts for all those starting outside the model
  740:     for (qid = 0; qid < MIGRATION[sp].num_in_queue; qid++) {
  741:         for (cohort = (int)floor((FunctGroupArray[sp].numCohortsXnumGenes) - 1); cohort >= 0; cohort--) {
  742:             if (MIGRATION[sp].Leave_Now[qid] < 0) {
  743:                 diff_nc = (int)ceil((MIGRATION[sp].Return_Now[qid] - MIGRATION[sp].Leave_Now[qid]) / 365);
  744:                 sp_Migrate_Years = max(1,((int)(MIGRATION[sp].MinYearsAway[qid])));
  745:                 MIGRATION[sp].yrs_to_age_pre_model[qid] = sp_Migrate_Years - diff_nc;
  746:                 MIGRATION[sp].end_pt[qid] = cohort + MIGRATION[sp].yrs_to_age_pre_model[qid];
  747:                 
  748:                 if (MIGRATION[sp].end_pt[qid] > (FunctGroupArray[sp].numCohortsXnumGenes - 1)) {
  749:                     MIGRATION[sp].end_pt[qid] = (FunctGroupArray[sp].numCohortsXnumGenes - 1);
  750:                 }
  751:             }
  752:         }
  753:     }
  754:                                 
  755:     for (qid = 0; qid < MIGRATION[sp].num_in_queue; qid++) {
```

**Suggested fix (from review):** Drop the cohort loop and set end_pt = min(numCohortsXnumGenes-1, top cohort of the migrating stage + yrs_to_age) or similar intended value

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

## EINIT-11

**integer division** · Conf H / Sev M  
**Listed lines:** 743

**Function:** `Init_Migration_Age_Check()`

**Why it's thought to be a bug**

`diff_nc = (int)ceil((Return_Now[qid] - Leave_Now[qid]) / 365);` Both operands are int, so the division truncates before `ceil` (400 days gives 1, not 2). This feeds `yrs_to_age_pre_model = sp_Migrate_Years - diff_nc`. Function: `Init_Migration_Age_Check()`.

**Current code** (`atecology/atmigration.c` lines 741–745)

```c
  741:         for (cohort = (int)floor((FunctGroupArray[sp].numCohortsXnumGenes) - 1); cohort >= 0; cohort--) {
  742:             if (MIGRATION[sp].Leave_Now[qid] < 0) {
  743:                 diff_nc = (int)ceil((MIGRATION[sp].Return_Now[qid] - MIGRATION[sp].Leave_Now[qid]) / 365);
  744:                 sp_Migrate_Years = max(1,((int)(MIGRATION[sp].MinYearsAway[qid])));
  745:                 MIGRATION[sp].yrs_to_age_pre_model[qid] = sp_Migrate_Years - diff_nc;
```

**Suggested fix (from review):** `/ 365.0`

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

## EINIT-12

**log noise** · Conf H / Sev L  
**Listed lines:** 175, 304

**Function:** `Init_Migration()`

**Why it's thought to be a bug**

`printf("Doing migration entry %d ...")` and `printf("Step1 Doing currentID ...")` are unconditional and run for every year × stage × migration of every species. Function: `Init_Migration()`.

**Current code** (`atecology/atmigration.c` lines 173–177)

```c
  173:                 }
  174:                 
  175:                 printf("Doing migration entry %d for %s\n", currentID, FunctGroupArray[sp].groupCode);
  176:                 
  177:                 // Set parameters
```

**Suggested fix (from review):** Guard with `if (verbose > 1)`

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

atmigration.c:243/324 use 364 where 365 is used everywhere else (lines 90, 101, 325, 386, 409) for unwinding the calendar.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atmigration.c:195 vs 206: `ReprodAllowed` is set for juvenile cohorts and then immediately overwritten for all cohorts by line 206, so the stage-specific intent of 195 is lost.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-3

atmigration.c:759-761 `Init_Migration_Age_Check()`: with numGeneTypes>1, clamping `nextcid` to `numCohortsXnumGenes-1` moves the last cohort's genotype-0 individuals into genotype ngenes-1.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
