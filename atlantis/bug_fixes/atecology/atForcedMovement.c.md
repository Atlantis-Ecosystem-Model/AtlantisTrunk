# atecology/atForcedMovement.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **5 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [EMISC-01](#emisc-01) 🔀 | ≈69, 308 | H/M | uninitialised variable | `if(propInput->curFile + 1 >= propInput->nFiles)` — `forceMoveEntryInput` is `malloc`ed (line 63) and `nFiles`… |  | [ ] |
| [EMISC-02](#emisc-02) | 82-94, 117-119 | H/M | NULL/invalid index | `tracerID = -1` stays -1 when no `<code>_stage_<n>` name matches, then `FunctGroupArray[speciesIndex].moveEntr… |  | [ ] |
| [EMISC-03](#emisc-03) | 468, 483 | M/M | float equality / logic | `if (bm->dayt != ...movetstart)` uses exact double equality. If `<code>_tstart` is not exactly on a model step… |  | [ ] |
| [EMISC-04](#emisc-04) | 227-257 | M/L | wrong order of checks | The `nctypelen(daty)` type check runs before `status` is checked. When `missing_value`/`valid_max`/`valid_min`… |  | [ ] |
| [EMISC-05](#emisc-05) | 270-283 | H/L | leak | `close_move_prop()` is never called (grep), so netCDF ids, `tbuf`/`valuebuf`, `fname`, `variableName` and `use… |  | [ ] |

---

## EMISC-01

**uninitialised variable** · Conf H / Sev M  
**Lines (current code):** ≈69, 308  
**Lines at review (Oct 2026):** 63, 306

**Function:** `get_move_property()`

**Why it's thought to be a bug**

`if(propInput->curFile + 1 >= propInput->nFiles)` — `forceMoveEntryInput` is `malloc`ed (line ≈69) and `nFiles` is never assigned (grep: no `forceMoveEntryInput[..].nFiles` anywhere; local `nFiles=1` is never stored). The end-of-file test reads garbage: if garbage > 1, `atEnd` is never set, and the read goes on past `nstep` with `count[0] = nstep - tstepnum <= 0` (line 322/334), so a zero/negative alloc and a fatal ncvarget. Function: `get_move_property()` (set in `init_forceMoveEntries()`).

> 🔀 **Changed by the merge of main (Nov 2026):** forceMoveEntryInput malloc -> calloc. Check whether this finding is now fixed before acting.

**Current code** (`atecology/atForcedMovement.c` lines 67–71) — ≈ the code here has changed since the review; lines are approximate

```c
   67:         for (tracerIndex = 0; tracerIndex < bm->numForceMoveEntries; tracerIndex++) {
   68:             
   69:             bm->forceMoveEntryInput[tracerIndex].fname = c_alloc2d(BMSLEN, nFiles);
   70:             bm->forceMoveEntryInput[tracerIndex].variableName = c_alloc1d(BMSLEN);
   71:             bm->forceMoveEntryInput[tracerIndex].isEpi = FALSE;    /* NA */
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
**Lines (current code):** 82-94, 117-119  
**Lines at review (Oct 2026):** 80-92, 115-117

**Function:** `init_forceMoveEntries()`

**Why it's thought to be a bug**

`tracerID = -1` stays -1 when no `<code>_stage_<n>` name matches, then `FunctGroupArray[speciesIndex].moveEntryIndex = 0` writes `FunctGroupArray[-1]`; `stageID` is also left uninitialised (malloc). There is no check, so a typo in `MoveGroupCodes` corrupts memory. Function: `init_forceMoveEntries()`.

**Current code** (`atecology/atForcedMovement.c` lines 80–119 (truncated))

```c
   80:             
   81:             bm->forceMoveEntryInput[tracerIndex].tracerID = -1;
   82:             stageID = 0;
   83:             for (i = 0; i < bm->K_num_tot_sp; i++) {
   84:                 for (stage = 0; stage < FunctGroupArray[i].numStages; stage++) {
   85:                     stageID = stage;
   86:                     snprintf(checkname, sizeof(checkname), "%s_stage_%d", FunctGroupArray[i].groupCode, stageID);
   87:                     if(strcmp(checkname, bm->forceMoveEntryInput[tracerIndex].variableName) == 0){
   88:                         bm->forceMoveEntryInput[tracerIndex].tracerID = i;
   89:                         bm->forceMoveEntryInput[tracerIndex].stageID = stage;
   90:                         break;
   91:                     }
   92:                 }
   93:             }
   94: 
   95:             snprintf(key, sizeof(key), "%s_File.name", bm->forceMoveEntryInput[tracerIndex].variableName);
   96:             readkeyprm_s(fp, key, bm->forceMoveEntryInput[tracerIndex].fname[0]);
   97:             
   98:             snprintf(key, sizeof(key), "%s_tstart", bm->forceMoveEntryInput[tracerIndex].variableName);
   99:             readkeyprm_d(fp, key, &bm->forceMoveEntryInput[tracerIndex].movetstart);
  100:             
  101:             bm->forceMoveEntryInput[tracerIndex].rewind = 0;
  102:             bm->forceMoveEntryInput[tracerIndex].wgt_coefft = 1.0;
  103:             bm->forceMoveEntryInput[tracerIndex].curFile = 0;
  104:             bm->forceMoveEntryInput[tracerIndex].ResetTol = 0.0;
  105:             bm->forceMoveEntryInput[tracerIndex].total_input = 0.0;
  106:             
  107:             bm->forceMoveEntryInput[tracerIndex].use_resets = (int *)i_alloc1d(nFiles);
  108:             bm->forceMoveEntryInput[tracerIndex].use_resets[0] = 0;
  109:             
  110:             
  111:             /* Set the min and max values */
  112:             bm->forceMoveEntryInput[tracerIndex].min_value = 0;
  113:             bm->forceMoveEntryInput[tracerIndex].max_value = 1.0;
  114:             
  115:             /* Setting move index */
  116:             speciesIndex = bm->forceMoveEntryInput[tracerIndex].tracerID;
  117:             FunctGroupArray[speciesIndex].moveEntryIndex = 0;
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
**Lines (current code):** 468, 483  
**Lines at review (Oct 2026):** 467, 482

**Function:** `Ecology_Update_Move_Entry()`

**Why it's thought to be a bug**

`if (bm->dayt != ...movetstart)` uses exact double equality. If `<code>_tstart` is not exactly on a model step, `Init_full_move_array` never runs. Line 483 (`dayt > movetstart`) still starts reading records into `distrib[..][next_qrt]`, so the initial `qrt` slot is never filled from the file. Function: `Ecology_Update_Move_Entry()`.

**Current code** (`atecology/atForcedMovement.c` lines 466–485)

```c
  466:         for (tracerIndex = 0; tracerIndex < bm->numForceMoveEntries; tracerIndex++) {
  467:             if (!bm->forceMoveEntryInput[tracerIndex].moveInitDone) {
  468:                 if (bm->dayt != bm->forceMoveEntryInput[tracerIndex].movetstart) {
  469:                     // Nothing to do as too early
  470:                 } else {
  471:                     Init_full_move_array(bm, &(bm->forceMoveEntryInput[tracerIndex]), tracerIndex);
  472:                     bm->forceMoveEntryInput[tracerIndex].moveInitDone = 1;
  473:                     speciesIndex = bm->forceMoveEntryInput[tracerIndex].tracerID;
  474:                     for (stageIndex = 0; stageIndex < FunctGroupArray[speciesIndex].numStages; stageIndex++) {
  475:                         FunctGroupArray[speciesIndex].NeedMoveUpdate[stageIndex] = 0;
  476:                     }
  477:                 }
  478:             }
  479:         }
  480:         
  481:         /* Move read-in value to the movement arrays */
  482:         for (tracerIndex = 0; tracerIndex < bm->numForceMoveEntries; tracerIndex++) {
  483:             if(bm->dayt > bm->forceMoveEntryInput[tracerIndex].movetstart) {
  484:                 speciesIndex = bm->forceMoveEntryInput[tracerIndex].tracerID;
  485:                 stageIndex = bm->forceMoveEntryInput[tracerIndex].stageID;
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
**Lines (current code):** 227-257  
**Lines at review (Oct 2026):** 225-255

**Function:** `open_move_prop()`

**Why it's thought to be a bug**

The `nctypelen(daty)` type check runs before `status` is checked. When `missing_value`/`valid_max`/`valid_min` is absent, `daty` holds whatever was left from the previous call, so the type test is meaningless (it could spuriously quit). Function: `open_move_prop()`.

**Current code** (`atecology/atForcedMovement.c` lines 225–259)

```c
  225:     /* Get the missing value */
  226:     status = ncattinq(propInput->fid, propInput->prop_vid, "missing_value", &daty, &len);
  227:     if (nctypelen(daty) != sizeof(doubleINPUT)){
  228:         quit("open_move_prop: %s missing_value attribute wrong type\n", propInput->variableName);
  229:     }
  230:     if (status >= 0){
  231:         /* go ahead and grab the value */
  232:         status = ncattget(propInput->fid, propInput->prop_vid, "missing_value",&propInput->missing_value);
  233:         propInput->missing_value_set = 1;
  234:     } else {
  235:         propInput->missing_value_set = 0;
  236:         propInput->missing_value = -1; /* Set so we are not referencing invalid memory */
  237:     }
  238: 
  239:     status = ncattinq(propInput->fid, propInput->prop_vid, "valid_max", &daty, &len);
  240:     if (nctypelen(daty) != sizeof(doubleINPUT)){
  241:         quit("open_move_prop: %s valid_max attribute wrong type\n", propInput->variableName);
  242:     }
  243:     if (status >= 0){
  244:         /* go ahead and grab the value */
  245:         status = ncattget(propInput->fid, propInput->prop_vid, "valid_max", &propInput->max_value);
  246:     }
  247: 
  248: 
  249:     status = ncattinq(propInput->fid, propInput->prop_vid, "valid_min", &daty, &len);
  250:     if (nctypelen(daty) != sizeof(doubleINPUT)){
  251:         quit("open_move_prop: %s valid_min attribute wrong type\n", propInput->variableName);
  252:     }
  253:     if (status >= 0){
  254:         /* go ahead and grab the value */
  255:         status = ncattget(propInput->fid, propInput->prop_vid, "valid_min", &propInput->min_value);
  256:     }
  257: 
  258:     /* Grab the units of the tracer and check that its the same as the units of the tracer in the model */
  259:     memset(propInput->tracer_units, 0, BMSLEN);
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
**Lines (current code):** 270-283  
**Lines at review (Oct 2026):** 268-281

**Function:** `close_move_prop()`

**Why it's thought to be a bug**

`close_move_prop()` is never called (grep), so netCDF ids, `tbuf`/`valuebuf`, `fname`, `variableName` and `use_resets` are never freed. Function: `close_move_prop()`.

**Current code** (`atecology/atForcedMovement.c` lines 268–285)

```c
  268: /* Routine to close an already open forced movement input file */
  269: void close_move_prop(MSEBoxModel *bm, PhyPropertyData *propInput) {
  270: 
  271:     /* Close movement property file */
  272:     if (propInput->fid >= 0)
  273:         ncclose(propInput->fid);
  274:     propInput->fid = -1;
  275: 
  276:     /* Free buffers */
  277:     free1d(propInput->tbuf);
  278:     free3d((double ***) propInput->valuebuf);
  279: 
  280:     propInput->tbuf = NULL;
  281:     propInput->valuebuf = NULL;
  282: }
  283: 
  284: /**
  285:  * \brief Read in a new chunk of the movement entry forcing file - get the movement data for this time period and next.
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
