# atecology/atForcedMovement.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **5 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [EMISC-01](#emisc-01) 🔀 | 307, 69, 46 | H/M | uninitialised variable | `if(propInput->curFile + 1 >= propInput->nFiles)` — `forceMoveEntryInput` is `malloc`ed (line 63) and `nFiles`… |  | [ ] |
| [EMISC-02](#emisc-02) | 81, 117 | H/M | NULL/invalid index | `tracerID = -1` stays -1 when no `<code>_stage_<n>` name matches, then `FunctGroupArray[speciesIndex].moveEntr… |  | [ ] |
| [EMISC-03](#emisc-03) | 468, 514 | M/M | float equality / logic | `if (bm->dayt != ...movetstart)` uses exact double equality. If `<code>_tstart` is not exactly on a model step… |  | [ ] |
| [EMISC-04](#emisc-04) | 227 | M/L | wrong order of checks | The `nctypelen(daty)` type check runs before `status` is checked. When `missing_value`/`valid_max`/`valid_min`… |  | [ ] |
| [EMISC-05](#emisc-05) | 108 | H/L | leak | `close_move_prop()` is never called (grep), so netCDF ids, `tbuf`/`valuebuf`, `fname`, `variableName` and `use… |  | [ ] |

---

## EMISC-01

**uninitialised variable** · Conf H / Sev M  
**Found at (current code):** `atForcedMovement.c` 307, `atForcedMovement.c` 69, `atForcedMovement.c` 46
**Lines at review (Oct 2026):** 63, 306

**Function:** `get_move_property()`

**Why it's thought to be a bug**

`if(propInput->curFile + 1 >= propInput->nFiles)` — `forceMoveEntryInput` is `malloc`ed (line ≈69) and `nFiles` is never assigned (grep: no `forceMoveEntryInput[..].nFiles` anywhere; local `nFiles=1` is never stored). The end-of-file test reads garbage: if garbage > 1, `atEnd` is never set, and the read goes on past `nstep` with `count[0] = nstep - tstepnum <= 0` (line 322/334), so a zero/negative alloc and a fatal ncvarget. Function: `get_move_property()` (set in `init_forceMoveEntries()`).

> 🔀 **Changed by the merge of main (Nov 2026):** forceMoveEntryInput malloc -> calloc. Check whether this finding is now fixed before acting.

**Code at issue** — `atForcedMovement.c` line 307  
Search for: `if(propInput->curFile + 1 >= propInput->nFiles)`

```c
  305:     if (tstepnum >= propInput->nstep) {
  306:         /* Are we at the end of the files  - assumes never rewind */
  307→         if(propInput->curFile + 1 >= propInput->nFiles){
  308:             propInput->atEnd = TRUE;
  309:             return;
```

**Code at issue** — `atForcedMovement.c` line 69 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->forceMoveEntryInput[tracerIndex].fname = c_alloc2d(BMSLEN, nFiles);`

```c
   67:         for (tracerIndex = 0; tracerIndex < bm->numForceMoveEntries; tracerIndex++) {
   68:             
   69→             bm->forceMoveEntryInput[tracerIndex].fname = c_alloc2d(BMSLEN, nFiles);
   70:             bm->forceMoveEntryInput[tracerIndex].variableName = c_alloc1d(BMSLEN);
   71:             bm->forceMoveEntryInput[tracerIndex].isEpi = FALSE;    /* NA */
```

**Code at issue** — `atForcedMovement.c` line 46  
Search for: `nFiles = 1`

```c
   44:     char **buf;
   45:     int size, speciesIndex = 0;
   46→     int nFiles = 1; //Too hard to do current distribution and next distribution if split over multiple files
   47: 
   48:     /* Read the list of files */
```

**Suggested fix (from review):** Add `bm->forceMoveEntryInput[tracerIndex].nFiles = nFiles;` in init (and init `fid=-1`)

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

## EMISC-02

**NULL/invalid index** · Conf H / Sev M  
**Found at (current code):** `atForcedMovement.c` 81, `atForcedMovement.c` 117
**Lines at review (Oct 2026):** 80-92, 115-117

**Function:** `init_forceMoveEntries()`

**Why it's thought to be a bug**

`tracerID = -1` stays -1 when no `<code>_stage_<n>` name matches, then `FunctGroupArray[speciesIndex].moveEntryIndex = 0` writes `FunctGroupArray[-1]`; `stageID` is also left uninitialised (malloc). There is no check, so a typo in `MoveGroupCodes` corrupts memory. Function: `init_forceMoveEntries()`.

**Code at issue** — `atForcedMovement.c` line 81  
Search for: `tracerID = -1`

```c
   79:             strcpy(bm->forceMoveEntryInput[tracerIndex].variableName, buf[tracerIndex]);
   80:             
   81→             bm->forceMoveEntryInput[tracerIndex].tracerID = -1;
   82:             stageID = 0;
   83:             for (i = 0; i < bm->K_num_tot_sp; i++) {
```

**Code at issue** — `atForcedMovement.c` line 117  
Search for: `FunctGroupArray[speciesIndex].moveEntryIndex = 0`

```c
  115:             /* Setting move index */
  116:             speciesIndex = bm->forceMoveEntryInput[tracerIndex].tracerID;
  117→             FunctGroupArray[speciesIndex].moveEntryIndex = 0;
  118:             FunctGroupArray[speciesIndex].next_moveEntryIndex = 1;
  119:             
```

**Suggested fix (from review):** `if (tracerID < 0) quit("...%s not recognised...")`

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

## EMISC-03

**float equality / logic** · Conf M / Sev M  
**Found at (current code):** `atForcedMovement.c` 468, `atForcedMovement.c` 514
**Lines at review (Oct 2026):** 467, 482

**Function:** `Ecology_Update_Move_Entry()`

**Why it's thought to be a bug**

`if (bm->dayt != ...movetstart)` uses exact double equality. If `<code>_tstart` is not exactly on a model step, `Init_full_move_array` never runs. Line 483 (`dayt > movetstart`) still starts reading records into `distrib[..][next_qrt]`, so the initial `qrt` slot is never filled from the file. Function: `Ecology_Update_Move_Entry()`.

**Code at issue** — `atForcedMovement.c` line 468  
Search for: `if (bm->dayt !=`

```c
  466:         for (tracerIndex = 0; tracerIndex < bm->numForceMoveEntries; tracerIndex++) {
  467:             if (!bm->forceMoveEntryInput[tracerIndex].moveInitDone) {
  468→                 if (bm->dayt != bm->forceMoveEntryInput[tracerIndex].movetstart) {
  469:                     // Nothing to do as too early
  470:                 } else {
```

**Code at issue** — `atForcedMovement.c` line 514 (closest match to the quoted code; wording differs slightly)  
Search for: `distrib_next_qrt = FunctGroupArray[speciesIndex].distrib[b][next_qrt][stageIndex];`

```c
  512:                     distrib_last_qrt = FunctGroupArray[speciesIndex].distrib[b][last_qrt][stageIndex];
  513:                     distrib_qrt = FunctGroupArray[speciesIndex].distrib[b][qrt][stageIndex];
  514→                     distrib_next_qrt = FunctGroupArray[speciesIndex].distrib[b][next_qrt][stageIndex];
  515:                     
  516:                     fprintf(llogfp, "Ecology_Update_Move_Entry Time: %e, %s-%d box: %d last_qrt: %d, qrt: %d next_qrt: %d distrib_last_qrt: %e distrib_qrt: %e orig distrib_next_qrt: %e ", bm->dayt, FunctGroupArray[speciesIndex].groupCode, stageIndex, b, last_qrt, qrt, next_qrt, distrib_last_qrt, distrib_qrt, distrib_next_qrt);
```

**Suggested fix (from review):** Use `bm->dayt >= movetstart && !moveInitDone`

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

## EMISC-04

**wrong order of checks** · Conf M / Sev L  
**Found at (current code):** `atForcedMovement.c` 227
**Lines at review (Oct 2026):** 225-255

**Function:** `open_move_prop()`

**Why it's thought to be a bug**

The `nctypelen(daty)` type check runs before `status` is checked. When `missing_value`/`valid_max`/`valid_min` is absent, `daty` holds whatever was left from the previous call, so the type test is meaningless (it could spuriously quit). Function: `open_move_prop()`.

**Code at issue** — `atForcedMovement.c` line 227  
Search for: `nctypelen(daty)`

```c
  225:     /* Get the missing value */
  226:     status = ncattinq(propInput->fid, propInput->prop_vid, "missing_value", &daty, &len);
  227→     if (nctypelen(daty) != sizeof(doubleINPUT)){
  228:         quit("open_move_prop: %s missing_value attribute wrong type\n", propInput->variableName);
  229:     }
```

**Suggested fix (from review):** Test `status >= 0` first, then the type

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

## EMISC-05

**leak** · Conf H / Sev L  
**Found at (current code):** `atForcedMovement.c` 108
**Lines at review (Oct 2026):** 268-281

**Function:** `close_move_prop()`

**Why it's thought to be a bug**

`close_move_prop()` is never called (grep), so netCDF ids, `tbuf`/`valuebuf`, `fname`, `variableName` and `use_resets` are never freed. Function: `close_move_prop()`.

**Code at issue** — `atForcedMovement.c` line 108 (located by name only; check this is the right place)  
Search for: `bm->forceMoveEntryInput[tracerIndex].use_resets[0] = 0;`

```c
  106:             
  107:             bm->forceMoveEntryInput[tracerIndex].use_resets = (int *)i_alloc1d(nFiles);
  108→             bm->forceMoveEntryInput[tracerIndex].use_resets[0] = 0;
  109:             
  110:             
```

**Suggested fix (from review):** Call it on shutdown / at atEnd

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
