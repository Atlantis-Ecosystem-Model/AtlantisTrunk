# atecology/include/*.h — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [EMISC-26](#emisc-26) ⭐ | 1218 | H/H | swapped arguments | The prototype and definition (atvertprocesses.c:342) are `Ecology_Spawn(sp, KSPA, FSP, SN, RN, X_RS, FSPB, fp)… |  | [ ] |
| [EMISC-27](#emisc-27) | 19 | H/L | stale prototypes | These prototypes have no definition anywhere and no callers: `Apply_External_Biology_Scalars`, `Get_Aquacult_F… |  | [ ] |

## Reviewer notes for this file

Header checks done: all 257 other prototypes in atecology/include/*.h were compared against their definitions across the repo (return type, argument count, types, order) with no mismatches found. The `extern` declarations in atbiology.h match the definitions in atbiologyPrivate.h (`cohortParamStrings`/`spawnParamStrings` are defined in atlantisUtil/atUtil.c). There are no conflicting #defines or enums in these headers.

---

## EMISC-26

**swapped arguments** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atclassical.c` 1218
**Lines at review (Oct 2026):** atEcologyLib.h:68; atassess/atclassical.c:1218-1219, 1555-1556

**Why it's thought to be a bug**

The prototype and definition (atvertprocesses.c:342) are `Ecology_Spawn(sp, KSPA, FSP, SN, RN, X_RS, FSPB, fp)`, but the atclassical.c callers pass `..., scaled_FSPB[nc], X_RS[nc], ofp`, so X_RS and FSPB are swapped. That gives wrong `WforS = (1+X_RS)*SN` and a wrong FSPB check, so spawning biomass in the classical assessment is wrong. Function: callers in atclassical.c.

**Code at issue** — `atclassical.c` line 1218 (closest match to the quoted code; wording differs slightly)  
Search for: `spawn_biom1 += num_nyr[na][est1_id] * FunctGroupArray[sp].scaled_FSPB[nc] * Ecology_Spawn(`

```c
 1216:                 
 1217:                 if (na > nagemat - 1) {
 1218→                     spawn_biom1 += num_nyr[na][est1_id] * FunctGroupArray[sp].scaled_FSPB[nc] * Ecology_Spawn(sp, KSPA_sp, FSP_sp, sp_SN, sp_RN, FunctGroupArray[sp].scaled_FSPB[nc], FunctGroupArray[sp].X_RS[nc], ofp);
 1219:                     spawn_biom2 += num_nyr[na][est2_id] * FunctGroupArray[sp].scaled_FSPB[nc] * Ecology_Spawn(sp, KSPA_sp, FSP_sp, sp_SN, sp_RN, FunctGroupArray[sp].scaled_FSPB[nc], FunctGroupArray[sp].X_RS[nc], ofp);
 1220:                 }
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
**Found at (current code):** `atExternalScalar.h` 19
**Lines at review (Oct 2026):** atExternalScalar.h:19; atecology.h:367, 369, 439, 485; atContaminants.h:39

**Why it's thought to be a bug**

These prototypes have no definition anywhere and no callers: `Apply_External_Biology_Scalars`, `Get_Aquacult_Feed`, `Update_GrazeInfo`, `Setup_Size_Change_Indices`, `Land_Biology_Process`, `Loose_Contaminant`. They are harmless now but would give link errors if called.

**Code at issue** — `atExternalScalar.h` line 19 (located by name only; check this is the right place)  
Search for: `void Apply_External_Biology_Scalars(MSEBoxModel *bm);`

```c
   17: void Calc_Scalar_Values(MSEBoxModel *bm, EcologyScalarData *propInput);
   18: void Free_Scalar_Prop(MSEBoxModel *bm, EcologyScalarData *propInput);
   19→ void Apply_External_Biology_Scalars(MSEBoxModel *bm);
   20: double Get_Group_Scalar(MSEBoxModel *bm, EcologyScalarData *propInput, int scalarIndex, int boxIndex, int layerIndex);
   21: 
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
