# atphysics/atdietIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY3-12](#phy3-12) ⭐ | 117,197,208,212,219 | H/H | cohort×genotype overrun | The loop runs `cohort < numCohortsXnumGenes * numStocks` and indexes `bm->totDiet[b][k][sp][cohort][prey]`. to… |  | [ ] |

---

## PHY3-12

**cohort×genotype overrun** · Conf H / Sev H ⭐ priority  
**Listed lines:** 117,197,208,212,219

**Why it's thought to be a bug**

The loop runs `cohort < numCohortsXnumGenes * numStocks` and indexes `bm->totDiet[b][k][sp][cohort][prey]`. totDiet is allocated `alloc5d(K_num_tot_sp, K_num_max_cohort, ...)` (atlantismain.c:1176), and `K_num_max_cohort` is the max of `numCohorts` only (atUtilGroupIO.c:302). Any group with numGeneTypes>1 or numStocks>1 overruns the cohort dimension. ReInitDietData (atbiology.c:1477) has the same overrun. Functions: `writeBMDietInfo()`, `writeBMDietData()`.

**Current code** (`atphysics/atdietIO.c` lines 115–119)

```c
  115:     for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  116:         if (sp == bm->which_check) {
  117:             for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes * FunctGroupArray[sp].numStocks; cohort++) {
  118:                 for (prey=0; prey < bm->K_num_tot_sp; prey++) {
  119:                     snprintf(ann_name, sizeof(ann_name), "%s%d_Diet_%s", FunctGroupArray[sp].groupCode, cohort+1, FunctGroupArray[prey].groupCode);
```

**Suggested fix (from review):** Allocate the cohort dim as max(numCohortsXnumGenes*numStocks)

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
