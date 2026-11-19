# atecology/atExternalScalar.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **7 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [EMISC-12](#emisc-12) | 231-235 | H/M | logic | `for (i = length-1; i >= 0; i--) if (isdigit(str[i])) index = i;` has no `break` on a non-digit, so it returns… |  | [ ] |
| [EMISC-13](#emisc-13) | 63, 271-277, 285-290 | H/M | buffer overrun | `Species_Cohort_Lookup[group][cohort][..] = variableIndex` checks only `cohort == -1`. The array is `[K_num_to… |  | [ ] |
| [EMISC-14](#emisc-14) | 380, 403-406 | M/L | off-by-one | `start[0] = max(tstepnum - 1, 0)`, but `bufstart = tstepnum`, `bufend = tstepnum + count - 1` and `nextrec += … |  | [ ] |
| [EMISC-15](#emisc-15) | 428-468 | H/M | stale index | After `Get_Scalar_Property()` reloads the buffer (line 431/440), `currentTimeIndex` is not reset, and the sear… |  | [ ] |
| [EMISC-16](#emisc-16) | 430-446 | M/M | logic | The rewind time offset (`+ nstep*rewindCount*dt`) is applied only to the chunk loaded right at rewind. Later c… |  | [ ] |
| [EMISC-17](#emisc-17) | 156 | H/L | format / uninitialised | The `quit("... %s dt must have positive value", propInput->variableName)` message passes `variableName`, which… |  | [ ] |
| [EMISC-18](#emisc-18) | 63 | M/M | cohort vs cohort x genotype | The lookup is sized `K_num_max_cohort`, but atq10.c:491-496 calls `Ecology_Get_Growth_Scalar(bm, sp, cohort)` … |  | [ ] |

---

## EMISC-12

**logic** · Conf H / Sev M  
**Lines (current code):** 231-235  
**Lines at review (Oct 2026):** 231-235

**Function:** `Get_Group_Cohort_Value()`

**Why it's thought to be a bug**

`for (i = length-1; i >= 0; i--) if (isdigit(str[i])) index = i;` has no `break` on a non-digit, so it returns the first digit anywhere in the string, not the start of the trailing number. A group code containing a digit (e.g. "ZM2", or "BB2" + cohort) is split at the wrong place: wrong cohort, or group = -1 and quit. Function: `Get_Group_Cohort_Value()`.

**Current code** (`atecology/atExternalScalar.c` lines 229–237)

```c
  229: 	*cohort = 0;
  230: 
  231: 	for (i = length - 1; i >= 0; i--) {
  232: 		if (isdigit(str[i])) {
  233: 			index = i;
  234: 		}
  235: 	}
  236: 
  237: 	if (index != -1) {
```

**Suggested fix (from review):** Scan back while `isdigit`, and stop at the first non-digit

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

## EMISC-13

**buffer overrun** · Conf H / Sev M  
**Lines (current code):** 63, 271-277, 285-290  
**Lines at review (Oct 2026):** 63, 271-277, 285-290

**Function:** `Match_Scalar_To_Groups()`

**Why it's thought to be a bug**

`Species_Cohort_Lookup[group][cohort][..] = variableIndex` checks only `cohort == -1`. The array is `[K_num_tot_sp][K_num_max_cohort][NUM_BIOLOGY_SCALAR]` (line 63, reversed-dims convention), so a variable like `XXX12mort_ff` for a 10-cohort group writes out of bounds. Function: `Match_Scalar_To_Groups()`.

**Current code** (`atecology/atExternalScalar.c` lines 61–65)

```c
   61: 
   62: 		/* Allocate memory for the species lookup data */
   63: 		bm->externalBiologyInput->Species_Cohort_Lookup = Util_Alloc_Init_3D_Int(NUM_BIOLOGY_SCALAR, bm->K_num_max_cohort, bm->K_num_tot_sp, -1);
   64: 
   65: 		/* Open the scalar input file and read all the attribute data */
```

**Suggested fix (from review):** Also quit if `cohort >= FunctGroupArray[group].numCohorts`

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

## EMISC-14

**off-by-one** · Conf M / Sev L  
**Lines (current code):** 380, 403-406  
**Lines at review (Oct 2026):** 380, 403-406

**Function:** `Get_Scalar_Property()`

**Why it's thought to be a bug**

`start[0] = max(tstepnum - 1, 0)`, but `bufstart = tstepnum`, `bufend = tstepnum + count - 1` and `nextrec += count`. Every chunk after the first is read one record early, so the final record `nstep-1` is never read, although `bufend` claims it is loaded. Function: `Get_Scalar_Property()`.

**Current code** (`atecology/atExternalScalar.c` lines 378–408)

```c
  378: 		 */
  379: 
  380: 		start[0] = max(tstepnum - 1, 0);
  381: 		start[1] = 0;
  382: 		start[2] = 0;
  383: 		count[0] = numTimeValues;
  384: 		count[1] = bm->nbox;
  385: 		count[2] = bm->wcnz;
  386: 
  387: 		ncvarget(propInput->fid, propInput->t_vid, start, count, value);
  388: 
  389: 		for (i = 0; i < numTimeValues; i++){
  390: 			propInput->timeDataBuffer[i] = (double) value[i];
  391: 		}
  392: 
  393: 		for (valueIndex = 0; valueIndex < propInput->nvariables; valueIndex++) {
  394: 			ncvarget(propInput->fid, propInput->prop_vid[valueIndex], start, count, array[0][0]);
  395: 			for (i = 0; i < count[0]; i++) {
  396: 				for (j = 0; j < count[1]; j++) {
  397: 					for (k = 0; k < count[2]; k++) {
  398: 						propInput->valuebuf[i][j][k][valueIndex] = (double) array[i][j][k];
  399: 					}
  400: 				}
  401: 			}
  402: 		}
  403: 		propInput->bufstart = tstepnum;
  404: 		propInput->bufend = tstepnum + count[0] - 1;
  405: 		/* Increment next record number */
  406: 		propInput->nextrec += count[0];
  407: 	}
  408: 
```

**Suggested fix (from review):** Make start/bufstart/bufend consistent (either no -1, or set bufstart=start)

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

## EMISC-15

**stale index** · Conf H / Sev M  
**Lines (current code):** 428-468  
**Lines at review (Oct 2026):** 428-468

**Function:** `Calc_Scalar_Values()`

**Why it's thought to be a bug**

After `Get_Scalar_Property()` reloads the buffer (line 431/440), `currentTimeIndex` is not reset, and the search loop starts from the old index. If that index is `>= current_nbuf` the loop never runs and a stale slot is used; otherwise earlier matching slots are skipped. Function: `Calc_Scalar_Values()`.

**Current code** (`atecology/atExternalScalar.c` lines 426–465 (truncated))

```c
  426: 
  427: 	/* Are we at the end of our data? */
  428: 	if(bm->t > propInput->timeDataBuffer[propInput->current_nbuf - 1]){
  429: 		/* Do we have more data to load? */
  430: 		if( propInput->bufend < (propInput->nstep - 1)){
  431: 			Get_Scalar_Property(bm, propInput);
  432: 		}else{
  433: 			/* Are we rewinding? */
  434: 			if(propInput->rewind == TRUE){
  435: 				/* Reset back to the start */
  436: 				propInput->bufstart = -1;
  437: 				propInput->bufend = -1;
  438: 				propInput->nextrec = 0;
  439: 
  440: 				Get_Scalar_Property(bm, propInput);
  441: 				propInput->rewindCount++;
  442: 
  443: 				for(index = 0; index< propInput->current_nbuf; index++){
  444: 					propInput->timeDataBuffer[index] = propInput->timeDataBuffer[index] + propInput->nstep * propInput->rewindCount * propInput->dt;
  445: 				}
  446: 			}
  447: 		}
  448: 	}
  449: 
  450: 	/* Work out what timestep we should be using. If there isn't a match then interpolate between two time steps */
  451: 	if (bm->t <= propInput->timeDataBuffer[0]) {
  452: 		bm->externalBiologyInput->currentTimeIndex = 0;
  453: 	} else if (bm->t >= propInput->timeDataBuffer[propInput->current_nbuf - 1]) {
  454: 		bm->externalBiologyInput->currentTimeIndex = propInput->current_nbuf - 1;
  455: 	} else {
  456: 		/* Else assume that the time is after the current time */
  457: 		for (index = bm->externalBiologyInput->currentTimeIndex; index < propInput->current_nbuf; index++) {
  458: 			if (bm->t == propInput->timeDataBuffer[index]) {
  459: 				bm->externalBiologyInput->currentTimeIndex = index;
  460: 				break;
  461: 			}
  462: 			if (bm->t <= propInput->timeDataBuffer[index]) {
  463: 				bm->externalBiologyInput->currentTimeIndex = index;
  464: 				prevIndex = TRUE;
  465: 				break;
```

**Suggested fix (from review):** Reset `currentTimeIndex = 0` whenever the buffer is reloaded

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

## EMISC-16

**logic** · Conf M / Sev M  
**Lines (current code):** 430-446  
**Lines at review (Oct 2026):** 430-446

**Function:** `Calc_Scalar_Values()`

**Why it's thought to be a bug**

The rewind time offset (`+ nstep*rewindCount*dt`) is applied only to the chunk loaded right at rewind. Later chunk loads (line 431) after a rewind keep raw file times, so `bm->t` is always past the buffer and the data jumps or reloads every step. This only matters for multi-chunk files with rewind on. Function: `Calc_Scalar_Values()`.

**Current code** (`atecology/atExternalScalar.c` lines 428–448)

```c
  428: 	if(bm->t > propInput->timeDataBuffer[propInput->current_nbuf - 1]){
  429: 		/* Do we have more data to load? */
  430: 		if( propInput->bufend < (propInput->nstep - 1)){
  431: 			Get_Scalar_Property(bm, propInput);
  432: 		}else{
  433: 			/* Are we rewinding? */
  434: 			if(propInput->rewind == TRUE){
  435: 				/* Reset back to the start */
  436: 				propInput->bufstart = -1;
  437: 				propInput->bufend = -1;
  438: 				propInput->nextrec = 0;
  439: 
  440: 				Get_Scalar_Property(bm, propInput);
  441: 				propInput->rewindCount++;
  442: 
  443: 				for(index = 0; index< propInput->current_nbuf; index++){
  444: 					propInput->timeDataBuffer[index] = propInput->timeDataBuffer[index] + propInput->nstep * propInput->rewindCount * propInput->dt;
  445: 				}
  446: 			}
  447: 		}
  448: 	}
```

**Suggested fix (from review):** Apply the offset inside `Get_Scalar_Property()` using `rewindCount`

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

## EMISC-17

**format / uninitialised** · Conf H / Sev L  
**Lines (current code):** 156  
**Lines at review (Oct 2026):** 156

**Function:** `Open_Scalar_Prop()`

**Why it's thought to be a bug**

The `quit("... %s dt must have positive value", propInput->variableName)` message passes `variableName`, which is `char**` and not allocated until line 165 (struct was malloc'ed). Function: `Open_Scalar_Prop()`.

**Current code** (`atecology/atExternalScalar.c` lines 154–158)

```c
  154: 	propInput->dt = (double) value;
  155: 	if (propInput->dt <= 0.0)
  156: 		quit("open_scalarprop: %s dt must have positive value\n", propInput->variableName);
  157: 
  158: 	/* Get other variable ids  - need to allow the user to specify as many tracers as they want in a single file.*/
```

**Suggested fix (from review):** Print `propInput->fname` instead

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

## EMISC-18

**cohort vs cohort x genotype** · Conf M / Sev M  
**Lines (current code):** 63  
**Lines at review (Oct 2026):** 63

**Function:** `Init_External_Scalars()`

**Why it's thought to be a bug**

The lookup is sized `K_num_max_cohort`, but atq10.c:493-498 calls `Ecology_Get_Growth_Scalar(bm, sp, cohort)` for `cohort < numCohortsXnumGenes`, which reads `Species_Cohort_Lookup[sp][cohort]` out of bounds when the genetics option makes numCohortsXnumGenes > K_num_max_cohort. Function: `Init_External_Scalars()` / `Ecology_Get_Growth_Scalar()`.

**Current code** (`atecology/atExternalScalar.c` lines 61–65)

```c
   61: 
   62: 		/* Allocate memory for the species lookup data */
   63: 		bm->externalBiologyInput->Species_Cohort_Lookup = Util_Alloc_Init_3D_Int(NUM_BIOLOGY_SCALAR, bm->K_num_max_cohort, bm->K_num_tot_sp, -1);
   64: 
   65: 		/* Open the scalar input file and read all the attribute data */
```

**Suggested fix (from review):** Map cohort to base cohort (cohort / numGenes) before lookup, or size by numCohortsXnumGenes

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
