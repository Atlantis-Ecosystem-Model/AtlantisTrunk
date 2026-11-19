# atecology/atexternalpop.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [CONT-25](#cont-25) | 91, 104, 108, 192 | M/M | cohort vs cohort×genotype | `age = ageclasssize * cohort ...` and `if (cohort < age_mat)` use the cohort×genotype loop index (line 71 loop… |  | [ ] |
| [CONT-26](#cont-26) | 147-153 | M/M | unit mismatch | `Fish_Respiration()` returns a per-second rate (`/86400.0`, atvertprocesses.c:1532) while `Growth_increment` i… |  | [ ] |
| [CONT-27](#cont-27) | 97-100, 180-181 | M/M | unit mismatch | Marie's option computes a *daily* increment (`lengthgrowth = Kbert*(linf-Length)/365`) and adds it to SN/RN on… |  | [ ] |
| [CONT-28](#cont-28) | 199-201 | L/M | missing floor | `MIGRATION[species].DEN[cohort][mid] -= (mort * dt);` with no lower bound; large mL/mP or dt can drive density… |  | [ ] |

---

## CONT-25

**cohort vs cohort×genotype** · Conf M / Sev M  
**Listed lines:** 91, 104, 108, 192

**Function:** `External_Box_Ecology()`

**Why it's thought to be a bug**

`age = ageclasssize * cohort ...` and `if (cohort < age_mat)` use the cohort×genotype loop index (line 71 loops `numCohortsXnumGenes`) as if it were the age class; with genotypes ages/maturity are wrong. Function: `External_Box_Ecology()`.

**Current code** (`atecology/atexternalpop.c` lines 89–110)

```c
   89:                                         if ( !bm->flag_extpop_growth_option ) {  // Marie's equations
   90:                                             //			calculation of age
   91:                                             age = ageclasssize * cohort + ageclasssize / 2.0;
   92: 
   93:                                             //			calculation of length
   94:                                             Length = linf - (linf * exp(-Kbert * (age - tzero)));
   95:                                 
   96:                                             //			daily growth in length (cm)
   97:                                             lengthgrowth = Kbert * (linf - Length) / 365.0;
   98:                                             //			daily growth in weight (mg N)
   99:                                             Growth = li_a * pow(lengthgrowth,li_b) * 1000.0 / (bm->k_wetdry * bm->X_CN);
  100:                                             Growth_increment = Growth;
  101:                                         } else { // Isaac's equations
  102:                                         // Alternative - get length at age and resulting mass and then difference that to get growth increment
  103:                                             //            calculation of age
  104:                                             old_age = ageclasssize * cohort + prop_year_elapsed - dt_step;
  105:                                             if(old_age < 0.0)
  106:                                                 old_age = 0.0;
  107:                                             
  108:                                             age = ageclasssize * cohort + prop_year_elapsed;
  109:                                             
  110:                                             //            calculation of length (in cm)
```

**Suggested fix (from review):** Use `cohort / numGeneTypes` for age

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

## CONT-26

**unit mismatch** · Conf M / Sev M  
**Listed lines:** 147-153

**Function:** `External_Box_Ecology()`

**Why it's thought to be a bug**

`Fish_Respiration()` returns a per-second rate (`/86400.0`, atvertprocesses.c:1532) while `Growth_increment` is a per-step (per-dt or per-day) mass increment; `avail_intake = Growth_increment - Resp` mixes units, feeding `Partition_Weight_Dynamic()`. Function: `External_Box_Ecology()`.

**Current code** (`atecology/atexternalpop.c` lines 145–155)

```c
  145:                                                 break;
  146:                                             case base_resp:
  147:                                                 Fish_Respiration(bm, species, SN, RN, X_RS, Density, &Resp);
  148:                                                 break;
  149:                                             case deb_resp: // DEB model for respitaiton
  150:                                                 Fish_Respiration_DEB(bm, species, SN, RN, X_RS, Density, &Resp);
  151:                                                 break;
  152:                                         }
  153:                                         avail_intake = Growth_increment - Resp;
  154:                                         
  155:                                         /*
```

**Suggested fix (from review):** Multiply Resp by `bm->dt` (and check Growth units)

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

## CONT-27

**unit mismatch** · Conf M / Sev M  
**Listed lines:** 97-100, 180-181

**Function:** `External_Box_Ecology()`

**Why it's thought to be a bug**

Marie's option computes a *daily* increment (`lengthgrowth = Kbert*(linf-Length)/365`) and adds it to SN/RN once per call with no dt scaling (comment says dt not needed as increment already per step, which is only true for the Isaac option). If dt ≠ 1 day growth is mis-scaled. Also `li_a*pow(lengthgrowth, li_b)` converts a length *increment* through the length-weight relation, which is not a weight increment. Function: `External_Box_Ecology()`.

**Current code** (`atecology/atexternalpop.c` lines 95–102)

```c
   95:                                 
   96:                                             //			daily growth in length (cm)
   97:                                             lengthgrowth = Kbert * (linf - Length) / 365.0;
   98:                                             //			daily growth in weight (mg N)
   99:                                             Growth = li_a * pow(lengthgrowth,li_b) * 1000.0 / (bm->k_wetdry * bm->X_CN);
  100:                                             Growth_increment = Growth;
  101:                                         } else { // Isaac's equations
  102:                                         // Alternative - get length at age and resulting mass and then difference that to get growth increment
```

**Suggested fix (from review):** Scale by `dt/86400` and use W(L+dL)-W(L)

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

## CONT-28

**missing floor** · Conf L / Sev M  
**Listed lines:** 199-201

**Function:** `External_Box_Ecology()`

**Why it's thought to be a bug**

`MIGRATION[species].DEN[cohort][mid] -= (mort * dt);` with no lower bound; large mL/mP or dt can drive density negative. Function: `External_Box_Ecology()`.

**Current code** (`atecology/atexternalpop.c` lines 197–203)

```c
  197:                                             mPext = FunctGroupArray[species].cohortSpeciesParams[adult_id][mPext_id];
  198:                                         }
  199:                                         mort = (mLext + mPext) * nums;
  200:                             
  201:                                         MIGRATION[species].DEN[cohort][mid] -= (mort * dt);
  202:                             
  203:                             
```

**Suggested fix (from review):** Clamp to ≥ 0

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
