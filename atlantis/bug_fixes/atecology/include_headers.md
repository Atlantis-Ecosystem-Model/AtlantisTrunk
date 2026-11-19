# atecology/include/*.h — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [EMISC-26](#emisc-26) ⭐ | atEcologyLib.h:68; atassess/atclassical.c:1218-1219, 1555-1556 | H/H | swapped arguments | The prototype and definition (atvertprocesses.c:342) are `Ecology_Spawn(sp, KSPA, FSP, SN, RN, X_RS, FSPB, fp)… |  | [ ] |
| [EMISC-27](#emisc-27) | atExternalScalar.h:19; atecology.h:383, 385, 455, 504; atContaminants.h:39 | H/L | stale prototypes | These prototypes have no definition anywhere and no callers: `Apply_External_Biology_Scalars`, `Get_Aquacult_F… |  | [ ] |

## Reviewer notes for this file

Header checks done: all 257 other prototypes in atecology/include/*.h were compared against their definitions across the repo (return type, argument count, types, order) with no mismatches found. The `extern` declarations in atbiology.h match the definitions in atbiologyPrivate.h (`cohortParamStrings`/`spawnParamStrings` are defined in atlantisUtil/atUtil.c). There are no conflicting #defines or enums in these headers.

---

## EMISC-26

**swapped arguments** · Conf H / Sev H ⭐ priority  
**Lines (current code):** atEcologyLib.h:68; atassess/atclassical.c:1218-1219, 1555-1556  
**Lines at review (Oct 2026):** atEcologyLib.h:68; atassess/atclassical.c:1218-1219, 1555-1556

**Why it's thought to be a bug**

The prototype and definition (atvertprocesses.c:342) are `Ecology_Spawn(sp, KSPA, FSP, SN, RN, X_RS, FSPB, fp)`, but the atclassical.c callers pass `..., scaled_FSPB[nc], X_RS[nc], ofp`, so X_RS and FSPB are swapped. That gives wrong `WforS = (1+X_RS)*SN` and a wrong FSPB check, so spawning biomass in the classical assessment is wrong. Function: callers in atclassical.c.

**Current code** (`atecology/include/atEcologyLib.h` lines 66–70)

```c
   66: void Ecology_Test_Fish_Total(MSEBoxModel *bm, double ***valtr, double **landtr, int calltype, char *spotcall, FILE *llogfp);
   67: void Ecology_Starve_Notice(MSEBoxModel *bm, FILE *llogfp);
   68: void Ecology_Check_VertAbund(MSEBoxModel *bm, double ***valtr, FILE *llogfp, int instance);
   69: 
   70: double Ecology_Spawn(int species, double KSPA_SP, double FSP_SP, double SN, double RN, double X_RS, double FSPB_SP, FILE *llogfp);
```

**Current code** (`atassess/atclassical.c` lines 1216–1221)

```c
 1216:                 
 1217:                 if (na > nagemat - 1) {
 1218:                     spawn_biom1 += num_nyr[na][est1_id] * FunctGroupArray[sp].scaled_FSPB[nc] * Ecology_Spawn(sp, KSPA_sp, FSP_sp, sp_SN, sp_RN, FunctGroupArray[sp].scaled_FSPB[nc], FunctGroupArray[sp].X_RS[nc], ofp);
 1219:                     spawn_biom2 += num_nyr[na][est2_id] * FunctGroupArray[sp].scaled_FSPB[nc] * Ecology_Spawn(sp, KSPA_sp, FSP_sp, sp_SN, sp_RN, FunctGroupArray[sp].scaled_FSPB[nc], FunctGroupArray[sp].X_RS[nc], ofp);
 1220:                 }
 1221: 				tbiom1 += num_nyr[na][est1_id] * weight;
```

**Suggested fix (from review):** Swap to `..., X_RS[nc], scaled_FSPB[nc], ofp`

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

## EMISC-27

**stale prototypes** · Conf H / Sev L  
**Lines (current code):** atExternalScalar.h:19; atecology.h:383, 385, 455, 504; atContaminants.h:39  
**Lines at review (Oct 2026):** atExternalScalar.h:19; atecology.h:367, 369, 439, 485; atContaminants.h:39

**Why it's thought to be a bug**

These prototypes have no definition anywhere and no callers: `Apply_External_Biology_Scalars`, `Get_Aquacult_Feed`, `Update_GrazeInfo`, `Setup_Size_Change_Indices`, `Land_Biology_Process`, `Loose_Contaminant`. They are harmless now but would give link errors if called.

**Current code** (`atecology/include/atExternalScalar.h` lines 17–21)

```c
   17: void Calc_Scalar_Values(MSEBoxModel *bm, EcologyScalarData *propInput);
   18: void Free_Scalar_Prop(MSEBoxModel *bm, EcologyScalarData *propInput);
   19: void Apply_External_Biology_Scalars(MSEBoxModel *bm);
   20: double Get_Group_Scalar(MSEBoxModel *bm, EcologyScalarData *propInput, int scalarIndex, int boxIndex, int layerIndex);
   21: 
```

**Current code** (`atecology/include/atecology.h` lines 381–387)

```c
  381: void Get_Aquacult_Feed(MSEBoxModel *bm, FILE *llogfp, int sp_id, double sp_Biomass, int flagcase, double KL_sp, long double *living_prey,
  382: 		long double *living_prey_sq, double *denom_step);
  383: void Update_GrazeInfo(MSEBoxModel *bm, FILE *llogfp, int flagcase, int sp_id, int cohort, int preyID, int kij, int catcheater,
  384: 		long double *graze_live, double ***spPREYinfo, double ***spGRAZEinfo, double **spCATCHGRAZEinfo);
  385: void UpdateTrackedMort(MSEBoxModel *bm, FILE *llogfp, int guildcase, int cohort, HABITAT_TYPES level_id, HABITAT_TYPES habitat, int prey, int prey_chrt,
  386: 		BoxLayerValues *boxLayerInfo, double scalar, int global_contrib);
  387: 
```

**Suggested fix (from review):** Remove

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
