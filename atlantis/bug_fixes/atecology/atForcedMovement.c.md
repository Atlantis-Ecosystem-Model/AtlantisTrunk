# atecology/atForcedMovement.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **5 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [EMISC-01](#emisc-01) | 63, 306 | H/M | uninitialised variable | `if(propInput->curFile + 1 >= propInput->nFiles)` — `forceMoveEntryInput` is `malloc`ed (line 63) and `nFiles`… |  | [ ] |
| [EMISC-02](#emisc-02) | 80-92, 115-117 | H/M | NULL/invalid index | `tracerID = -1` stays -1 when no `<code>_stage_<n>` name matches, then `FunctGroupArray[speciesIndex].moveEntr… |  | [ ] |
| [EMISC-03](#emisc-03) | 467, 482 | M/M | float equality / logic | `if (bm->dayt != ...movetstart)` uses exact double equality. If `<code>_tstart` is not exactly on a model step… |  | [ ] |
| [EMISC-04](#emisc-04) | 225-255 | M/L | wrong order of checks | The `nctypelen(daty)` type check runs before `status` is checked. When `missing_value`/`valid_max`/`valid_min`… |  | [ ] |
| [EMISC-05](#emisc-05) | 268-281 | H/L | leak | `close_move_prop()` is never called (grep), so netCDF ids, `tbuf`/`valuebuf`, `fname`, `variableName` and `use… |  | [ ] |

---

## EMISC-01

**uninitialised variable** · Conf H / Sev M  
**Listed lines:** 63, 306

**Function:** `get_move_property()`

**Why it's thought to be a bug**

`if(propInput->curFile + 1 >= propInput->nFiles)` — `forceMoveEntryInput` is `malloc`ed (line 63) and `nFiles` is never assigned (grep: no `forceMoveEntryInput[..].nFiles` anywhere; local `nFiles=1` is never stored). The end-of-file test reads garbage: if garbage > 1, `atEnd` is never set, and the read goes on past `nstep` with `count[0] = nstep - tstepnum <= 0` (line 320/332), so a zero/negative alloc and a fatal ncvarget. Function: `get_move_property()` (set in `init_forceMoveEntries()`).

**Current code** (`atecology/atForcedMovement.c` lines 61–65)

```c
   61:         /* Read in the names of the tracers */
   62:         readkeyprm_sarray(fp, "MoveGroupCodes", &buf, &size);
   63:         bm->forceMoveEntryInput = (PhyPropertyData *) malloc(sizeof(PhyPropertyData) * (long unsigned int)bm->numForceMoveEntries);
   64:         
   65:         /* Now read in the files for each tracer */
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
**Listed lines:** 80-92, 115-117

**Function:** `init_forceMoveEntries()`

**Why it's thought to be a bug**

`tracerID = -1` stays -1 when no `<code>_stage_<n>` name matches, then `FunctGroupArray[speciesIndex].moveEntryIndex = 0` writes `FunctGroupArray[-1]`; `stageID` is also left uninitialised (malloc). There is no check, so a typo in `MoveGroupCodes` corrupts memory. Function: `init_forceMoveEntries()`.

**Current code** (`atecology/atForcedMovement.c` lines 78–117 (truncated))

```c
   78:             strcpy(bm->forceMoveEntryInput[tracerIndex].variableName, buf[tracerIndex]);
   79:             
   80:             bm->forceMoveEntryInput[tracerIndex].tracerID = -1;
   81:             stageID = 0;
   82:             for (i = 0; i < bm->K_num_tot_sp; i++) {
   83:                 for (stage = 0; stage < FunctGroupArray[i].numStages; stage++) {
   84:                     stageID = stage;
   85:                     snprintf(checkname, sizeof(checkname), "%s_stage_%d", FunctGroupArray[i].groupCode, stageID);
   86:                     if(strcmp(checkname, bm->forceMoveEntryInput[tracerIndex].variableName) == 0){
   87:                         bm->forceMoveEntryInput[tracerIndex].tracerID = i;
   88:                         bm->forceMoveEntryInput[tracerIndex].stageID = stage;
   89:                         break;
   90:                     }
   91:                 }
   92:             }
   93: 
   94:             snprintf(key, sizeof(key), "%s_File.name", bm->forceMoveEntryInput[tracerIndex].variableName);
   95:             readkeyprm_s(fp, key, bm->forceMoveEntryInput[tracerIndex].fname[0]);
   96:             
   97:             snprintf(key, sizeof(key), "%s_tstart", bm->forceMoveEntryInput[tracerIndex].variableName);
   98:             readkeyprm_d(fp, key, &bm->forceMoveEntryInput[tracerIndex].movetstart);
   99:             
  100:             bm->forceMoveEntryInput[tracerIndex].rewind = 0;
  101:             bm->forceMoveEntryInput[tracerIndex].wgt_coefft = 1.0;
  102:             bm->forceMoveEntryInput[tracerIndex].curFile = 0;
  103:             bm->forceMoveEntryInput[tracerIndex].ResetTol = 0.0;
  104:             bm->forceMoveEntryInput[tracerIndex].total_input = 0.0;
  105:             
  106:             bm->forceMoveEntryInput[tracerIndex].use_resets = (int *)i_alloc1d(nFiles);
  107:             bm->forceMoveEntryInput[tracerIndex].use_resets[0] = 0;
  108:             
  109:             
  110:             /* Set the min and max values */
  111:             bm->forceMoveEntryInput[tracerIndex].min_value = 0;
  112:             bm->forceMoveEntryInput[tracerIndex].max_value = 1.0;
  113:             
  114:             /* Setting move index */
  115:             speciesIndex = bm->forceMoveEntryInput[tracerIndex].tracerID;
  116:             FunctGroupArray[speciesIndex].moveEntryIndex = 0;
  117:             FunctGroupArray[speciesIndex].next_moveEntryIndex = 1;
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
**Listed lines:** 467, 482

**Function:** `Ecology_Update_Move_Entry()`

**Why it's thought to be a bug**

`if (bm->dayt != ...movetstart)` uses exact double equality. If `<code>_tstart` is not exactly on a model step, `Init_full_move_array` never runs. Line 482 (`dayt > movetstart`) still starts reading records into `distrib[..][next_qrt]`, so the initial `qrt` slot is never filled from the file. Function: `Ecology_Update_Move_Entry()`.

**Current code** (`atecology/atForcedMovement.c` lines 465–484)

```c
  465:         for (tracerIndex = 0; tracerIndex < bm->numForceMoveEntries; tracerIndex++) {
  466:             if (!bm->forceMoveEntryInput[tracerIndex].moveInitDone) {
  467:                 if (bm->dayt != bm->forceMoveEntryInput[tracerIndex].movetstart) {
  468:                     // Nothing to do as too early
  469:                 } else {
  470:                     Init_full_move_array(bm, &(bm->forceMoveEntryInput[tracerIndex]), tracerIndex);
  471:                     bm->forceMoveEntryInput[tracerIndex].moveInitDone = 1;
  472:                     speciesIndex = bm->forceMoveEntryInput[tracerIndex].tracerID;
  473:                     for (stageIndex = 0; stageIndex < FunctGroupArray[speciesIndex].numStages; stageIndex++) {
  474:                         FunctGroupArray[speciesIndex].NeedMoveUpdate[stageIndex] = 0;
  475:                     }
  476:                 }
  477:             }
  478:         }
  479:         
  480:         /* Move read-in value to the movement arrays */
  481:         for (tracerIndex = 0; tracerIndex < bm->numForceMoveEntries; tracerIndex++) {
  482:             if(bm->dayt > bm->forceMoveEntryInput[tracerIndex].movetstart) {
  483:                 speciesIndex = bm->forceMoveEntryInput[tracerIndex].tracerID;
  484:                 stageIndex = bm->forceMoveEntryInput[tracerIndex].stageID;
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
**Listed lines:** 225-255

**Function:** `open_move_prop()`

**Why it's thought to be a bug**

The `nctypelen(daty)` type check runs before `status` is checked. When `missing_value`/`valid_max`/`valid_min` is absent, `daty` holds whatever was left from the previous call, so the type test is meaningless (it could spuriously quit). Function: `open_move_prop()`.

**Current code** (`atecology/atForcedMovement.c` lines 223–257)

```c
  223: 
  224:     /* Get the missing value */
  225:     status = ncattinq(propInput->fid, propInput->prop_vid, "missing_value", &daty, &len);
  226:     if (nctypelen(daty) != sizeof(doubleINPUT)){
  227:         quit("open_move_prop: %s missing_value attribute wrong type\n", propInput->variableName);
  228:     }
  229:     if (status >= 0){
  230:         /* go ahead and grab the value */
  231:         status = ncattget(propInput->fid, propInput->prop_vid, "missing_value",&propInput->missing_value);
  232:         propInput->missing_value_set = 1;
  233:     } else {
  234:         propInput->missing_value_set = 0;
  235:         propInput->missing_value = -1; /* Set so we are not referencing invalid memory */
  236:     }
  237: 
  238:     status = ncattinq(propInput->fid, propInput->prop_vid, "valid_max", &daty, &len);
  239:     if (nctypelen(daty) != sizeof(doubleINPUT)){
  240:         quit("open_move_prop: %s valid_max attribute wrong type\n", propInput->variableName);
  241:     }
  242:     if (status >= 0){
  243:         /* go ahead and grab the value */
  244:         status = ncattget(propInput->fid, propInput->prop_vid, "valid_max", &propInput->max_value);
  245:     }
  246: 
  247: 
  248:     status = ncattinq(propInput->fid, propInput->prop_vid, "valid_min", &daty, &len);
  249:     if (nctypelen(daty) != sizeof(doubleINPUT)){
  250:         quit("open_move_prop: %s valid_min attribute wrong type\n", propInput->variableName);
  251:     }
  252:     if (status >= 0){
  253:         /* go ahead and grab the value */
  254:         status = ncattget(propInput->fid, propInput->prop_vid, "valid_min", &propInput->min_value);
  255:     }
  256: 
  257:     /* Grab the units of the tracer and check that its the same as the units of the tracer in the model */
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
**Listed lines:** 268-281

**Function:** `close_move_prop()`

**Why it's thought to be a bug**

`close_move_prop()` is never called (grep), so netCDF ids, `tbuf`/`valuebuf`, `fname`, `variableName` and `use_resets` are never freed. Function: `close_move_prop()`.

**Current code** (`atecology/atForcedMovement.c` lines 266–283)

```c
  266: 
  267: /* Routine to close an already open forced movement input file */
  268: void close_move_prop(MSEBoxModel *bm, PhyPropertyData *propInput) {
  269: 
  270:     /* Close movement property file */
  271:     if (propInput->fid >= 0)
  272:         ncclose(propInput->fid);
  273:     propInput->fid = -1;
  274: 
  275:     /* Free buffers */
  276:     free1d(propInput->tbuf);
  277:     free3d((double ***) propInput->valuebuf);
  278: 
  279:     propInput->tbuf = NULL;
  280:     propInput->valuebuf = NULL;
  281: }
  282: 
  283: /**
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
