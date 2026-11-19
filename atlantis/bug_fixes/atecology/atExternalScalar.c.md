# atecology/atExternalScalar.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **7 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [EMISC-12](#emisc-12) | 232 | H/M | logic | `for (i = length-1; i >= 0; i--) if (isdigit(str[i])) index = i;` has no `break` on a non-digit, so it returns… |  | [ ] |
| [EMISC-13](#emisc-13) | 277, 273 | H/M | buffer overrun | `Species_Cohort_Lookup[group][cohort][..] = variableIndex` checks only `cohort == -1`. The array is `[K_num_to… |  | [ ] |
| [EMISC-14](#emisc-14) | 380, 403, 406 | M/L | off-by-one | `start[0] = max(tstepnum - 1, 0)`, but `bufstart = tstepnum`, `bufend = tstepnum + count - 1` and `nextrec += … |  | [ ] |
| [EMISC-15](#emisc-15) | 454 | H/M | stale index | After `Get_Scalar_Property()` reloads the buffer (line 431/440), `currentTimeIndex` is not reset, and the sear… |  | [ ] |
| [EMISC-16](#emisc-16) | 444 | M/M | logic | The rewind time offset (`+ nstep*rewindCount*dt`) is applied only to the chunk loaded right at rewind. Later c… |  | [ ] |
| [EMISC-17](#emisc-17) | 156 | H/L | format / uninitialised | The `quit("... %s dt must have positive value", propInput->variableName)` message passes `variableName`, which… |  | [ ] |
| [EMISC-18](#emisc-18) | 63 | M/M | cohort vs cohort x genotype | The lookup is sized `K_num_max_cohort`, but atq10.c:491-496 calls `Ecology_Get_Growth_Scalar(bm, sp, cohort)` … |  | [ ] |

---

## EMISC-12

**logic** · Conf H / Sev M  
**Found at (current code):** `atExternalScalar.c` 232
**Lines at review (Oct 2026):** 231-235

**Function:** `Get_Group_Cohort_Value()`

**Why it's thought to be a bug**

`for (i = length-1; i >= 0; i--) if (isdigit(str[i])) index = i;` has no `break` on a non-digit, so it returns the first digit anywhere in the string, not the start of the trailing number. A group code containing a digit (e.g. "ZM2", or "BB2" + cohort) is split at the wrong place: wrong cohort, or group = -1 and quit. Function: `Get_Group_Cohort_Value()`.

**Code at issue** — `atExternalScalar.c` line 232 (the line the review pointed to, found by its text)  
Search for: `if (isdigit(str[i])) {`

```c
  230: 
  231: 	for (i = length - 1; i >= 0; i--) {
  232→ 		if (isdigit(str[i])) {
  233: 			index = i;
  234: 		}
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
**Found at (current code):** `atExternalScalar.c` 277, `atExternalScalar.c` 273
**Lines at review (Oct 2026):** 63, 271-277, 285-290

**Function:** `Match_Scalar_To_Groups()`

**Why it's thought to be a bug**

`Species_Cohort_Lookup[group][cohort][..] = variableIndex` checks only `cohort == -1`. The array is `[K_num_tot_sp][K_num_max_cohort][NUM_BIOLOGY_SCALAR]` (line 63, reversed-dims convention), so a variable like `XXX12mort_ff` for a 10-cohort group writes out of bounds. Function: `Match_Scalar_To_Groups()`.

**Code at issue** — `atExternalScalar.c` line 277 (closest match to the quoted code; wording differs slightly)  
Search for: `propInput->Species_Cohort_Lookup[group][cohort][MORT_INDEX_ID] = variableIndex;`

```c
  275: 			}
  276: 
  277→ 			propInput->Species_Cohort_Lookup[group][cohort][MORT_INDEX_ID] = variableIndex;
  278: 		}
  279: 		if (strstr(str, "growth_ff") != NULL) {
```

**Code at issue** — `atExternalScalar.c` line 273  
Search for: `cohort == -1`

```c
  271: 			Get_Group_Cohort_Value(bm, str, &group, &cohort);
  272: 
  273→ 			if(group == -1 || cohort == -1){
  274: 				quit("Match_Scalar_To_Groups: Variable (%s) in file is unrecognised \n", propInput->variableName[variableIndex]);
  275: 			}
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
**Found at (current code):** `atExternalScalar.c` 380, `atExternalScalar.c` 403, `atExternalScalar.c` 406
**Lines at review (Oct 2026):** 380, 403-406

**Function:** `Get_Scalar_Property()`

**Why it's thought to be a bug**

`start[0] = max(tstepnum - 1, 0)`, but `bufstart = tstepnum`, `bufend = tstepnum + count - 1` and `nextrec += count`. Every chunk after the first is read one record early, so the final record `nstep-1` is never read, although `bufend` claims it is loaded. Function: `Get_Scalar_Property()`.

**Code at issue** — `atExternalScalar.c` line 380  
Search for: `start[0] = max(tstepnum - 1, 0)`

```c
  378: 		 */
  379: 
  380→ 		start[0] = max(tstepnum - 1, 0);
  381: 		start[1] = 0;
  382: 		start[2] = 0;
```

**Code at issue** — `atExternalScalar.c` line 403  
Search for: `bufstart = tstepnum`

```c
  401: 			}
  402: 		}
  403→ 		propInput->bufstart = tstepnum;
  404: 		propInput->bufend = tstepnum + count[0] - 1;
  405: 		/* Increment next record number */
```

**Code at issue** — `atExternalScalar.c` line 406  
Search for: `nextrec += count`

```c
  404: 		propInput->bufend = tstepnum + count[0] - 1;
  405: 		/* Increment next record number */
  406→ 		propInput->nextrec += count[0];
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
**Found at (current code):** `atExternalScalar.c` 454
**Lines at review (Oct 2026):** 428-468

**Function:** `Calc_Scalar_Values()`

**Why it's thought to be a bug**

After `Get_Scalar_Property()` reloads the buffer (line 431/440), `currentTimeIndex` is not reset, and the search loop starts from the old index. If that index is `>= current_nbuf` the loop never runs and a stale slot is used; otherwise earlier matching slots are skipped. Function: `Calc_Scalar_Values()`.

**Code at issue** — `atExternalScalar.c` line 454 (located by name only; check this is the right place)  
Search for: `bm->externalBiologyInput->currentTimeIndex = propInput->current_nbuf - 1;`

```c
  452: 		bm->externalBiologyInput->currentTimeIndex = 0;
  453: 	} else if (bm->t >= propInput->timeDataBuffer[propInput->current_nbuf - 1]) {
  454→ 		bm->externalBiologyInput->currentTimeIndex = propInput->current_nbuf - 1;
  455: 	} else {
  456: 		/* Else assume that the time is after the current time */
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
**Found at (current code):** `atExternalScalar.c` 444
**Lines at review (Oct 2026):** 430-446

**Function:** `Calc_Scalar_Values()`

**Why it's thought to be a bug**

The rewind time offset (`+ nstep*rewindCount*dt`) is applied only to the chunk loaded right at rewind. Later chunk loads (line 431) after a rewind keep raw file times, so `bm->t` is always past the buffer and the data jumps or reloads every step. This only matters for multi-chunk files with rewind on. Function: `Calc_Scalar_Values()`.

**Code at issue** — `atExternalScalar.c` line 444 (closest match to the quoted code; wording differs slightly)  
Search for: `propInput->timeDataBuffer[index] = propInput->timeDataBuffer[index] + propInput->nstep * p`

```c
  442: 
  443: 				for(index = 0; index< propInput->current_nbuf; index++){
  444→ 					propInput->timeDataBuffer[index] = propInput->timeDataBuffer[index] + propInput->nstep * propInput->rewindCount * propInput->dt;
  445: 				}
  446: 			}
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
**Found at (current code):** `atExternalScalar.c` 156
**Lines at review (Oct 2026):** 156

**Function:** `Open_Scalar_Prop()`

**Why it's thought to be a bug**

The `quit("... %s dt must have positive value", propInput->variableName)` message passes `variableName`, which is `char**` and not allocated until line 165 (struct was malloc'ed). Function: `Open_Scalar_Prop()`.

**Code at issue** — `atExternalScalar.c` line 156  
Search for: `quit("`

```c
  154: 	propInput->dt = (double) value;
  155: 	if (propInput->dt <= 0.0)
  156→ 		quit("open_scalarprop: %s dt must have positive value\n", propInput->variableName);
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
**Found at (current code):** `atExternalScalar.c` 63
**Lines at review (Oct 2026):** 63

**Function:** `Init_External_Scalars()`

**Why it's thought to be a bug**

The lookup is sized `K_num_max_cohort`, but atq10.c:493-498 calls `Ecology_Get_Growth_Scalar(bm, sp, cohort)` for `cohort < numCohortsXnumGenes`, which reads `Species_Cohort_Lookup[sp][cohort]` out of bounds when the genetics option makes numCohortsXnumGenes > K_num_max_cohort. Function: `Init_External_Scalars()` / `Ecology_Get_Growth_Scalar()`.

**Code at issue** — `atExternalScalar.c` line 63 (the line the review pointed to, found by its text)  
Search for: `bm->externalBiologyInput->Species_Cohort_Lookup = Util_Alloc_Init_3D_Int(NUM_BIOLOGY_SCALA`

```c
   61: 
   62: 		/* Allocate memory for the species lookup data */
   63→ 		bm->externalBiologyInput->Species_Cohort_Lookup = Util_Alloc_Init_3D_Int(NUM_BIOLOGY_SCALAR, bm->K_num_max_cohort, bm->K_num_tot_sp, -1);
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
