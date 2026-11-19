# sjwlib/dfcoords.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **9 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW1-14](#sjw1-14) | 767, 770 | H/L | missing braces / uninitialised | `if (cm->transform != NULL) cm->transform->inverse(..., tcoords, ttcoords); for (j...) tcoords[j] = ttcoords[j… |  | [ ] |
| [SJW1-15](#sjw1-15) | 1227, 1202 | H/L | off-by-one OOB read | `if ((i < 0) \|\| (i > ihigh)) return 0; coords[0] = (indices[0]-i)*(data[i+1]-data[i]) + data[i];`. When `i == … |  | [ ] |
| [SJW1-16](#sjw1-16) | 1286 | H/L | always-true condition | `i = (i < 0) ? 0 : i; i = (i >= 0) ? dsize - 1 : i;`: after the first clamp `i >= 0` is always true, so every … |  | [ ] |
| [SJW1-17](#sjw1-17) | 1452, 1463, 1449 | H/L | copy-paste (wrong buffer) | `buf` is filled from `int_proj` once (line 1460) and `parseline` tokenises it in place for `int_mp`. The `ext_… |  | [ ] |
| [SJW1-18](#sjw1-18) | 533 | M/L | ignored return value | The return value of `dfSetCoordSystem(df, v, ncm, cmaps)` is ignored. It can fail with `return 0` (duplicates,… |  | [ ] |
| [SJW1-19](#sjw1-19) | 768 | H/L | memory leak | `df_free_coord_system` never frees `cmaps[i].transform`: neither the `CoordinateTransform`, nor its `private_d… |  | [ ] |
| [SJW1-20](#sjw1-20) | 235, 181 | H/L | memory leak | Each early `return 0` after line 124 leaks `coordids`/`coordtypes`, and the duplicate check's early returns al… |  | [ ] |
| [SJW1-21](#sjw1-21) | 815, 805, 374 | M/L | buffer overrun | `strcpy(line, coords)` copies a netCDF `coordinates` attribute of arbitrary length into `char line[MAXLINELEN]… |  | [ ] |
| [SJW1-22](#sjw1-22) | 1326 | H/L | memory leak | `rgi` / `pgi` are `malloc`ed before the `sscanf(...)==10` test. If the analytic attribute is malformed, the co… |  | [ ] |

---

## SJW1-14

**missing braces / uninitialised** · Conf H / Sev L  
**Found at (current code):** `dfcoords.c` 767, `dfcoords.c` 770
**Lines at review (Oct 2026):** 765-771

**Function:** `dfIndicesToCoords()`

**Why it's thought to be a bug**

`if (cm->transform != NULL) cm->transform->inverse(..., tcoords, ttcoords); for (j...) tcoords[j] = ttcoords[j];`: the copy loop is outside the `if`. With no transform (the normal case) it overwrites the computed coordinates with uninitialised `ttcoords`. The forward path `dfCoordsToIndices` (lines 698-703) has the loop inside braces. Severity is low only because `dfIndicesToCoords` currently has no callers. Function: `dfIndicesToCoords()`.

**Code at issue** — `dfcoords.c` line 767  
Search for: `if (cm->transform != NULL)`

```c
  765: 		if (cm->indices_to_coords(df, cm, depindices, tindices, tcoords)) {
  766: 			/* Perform an inverse coordinate transform (to user) if required. */
  767→ 			if (cm->transform != NULL)
  768: 				cm->transform->inverse(cm->transform->private_data, cm->nc,
  769: 						tcoords, ttcoords);
```

**Code at issue** — `dfcoords.c` line 770  
Search for: `for (j = 0; j < cm->nc; ++j)`

```c
  768: 				cm->transform->inverse(cm->transform->private_data, cm->nc,
  769: 						tcoords, ttcoords);
  770→ 			for (j = 0; j < cm->nc; ++j)
  771: 				tcoords[j] = ttcoords[j];
  772: 
```

**Suggested fix (from review):** Wrap the copy loop in braces with the `inverse` call.

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

## SJW1-15

**off-by-one OOB read** · Conf H / Sev L  
**Found at (current code):** `dfcoords.c` 1227, `dfcoords.c` 1202
**Lines at review (Oct 2026):** 1223-1227

**Function:** `cm_1d_itoc()`

**Why it's thought to be a bug**

`if ((i < 0) || (i > ihigh)) return 0; coords[0] = (indices[0]-i)*(data[i+1]-data[i]) + data[i];`. When `i == ihigh` (the last index, a valid input) this reads `data[ihigh+1]`, one past the end of the coordinate array. Reached only through `dfIndicesToCoords` (unused). Function: `cm_1d_itoc()`.

**Code at issue** — `dfcoords.c` line 1227 (closest match to the quoted code; wording differs slightly)  
Search for: `coords[0] = (indices[0] - i) * (data[i + 1] - data[i]) + data[i];`

```c
 1225: 		return 0;
 1226: 
 1227→ 	coords[0] = (indices[0] - i) * (data[i + 1] - data[i]) + data[i];
 1228: 
 1229: 	return 1;
```

**Code at issue** — `dfcoords.c` line 1202 (closest match to the quoted code; wording differs slightly)  
Search for: `indices[0] = ilow + (coords[0] - data[ilow]) / (data[ihigh]`

```c
 1200: 		indices[0] = ilow;
 1201: 	else
 1202→ 		indices[0] = ilow + (coords[0] - data[ilow]) / (data[ihigh]
 1203: 				- data[ilow]);
 1204: 
```

**Suggested fix (from review):** Return `data[ihigh]` when `i == ihigh` (or test `i >= ihigh` and handle it separately).

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

## SJW1-16

**always-true condition** · Conf H / Sev L  
**Found at (current code):** `dfcoords.c` 1286
**Lines at review (Oct 2026):** 1285-1287

**Function:** `cm_1d_multi_itoc()`

**Why it's thought to be a bug**

`i = (i < 0) ? 0 : i; i = (i >= 0) ? dsize - 1 : i;`: after the first clamp `i >= 0` is always true, so every index maps to the last point `dsize-1`. The upper clamp should test `i >= dsize`. Reached only through `dfIndicesToCoords` (unused). Function: `cm_1d_multi_itoc()`.

**Code at issue** — `dfcoords.c` line 1286  
Search for: `i = (i < 0) ? 0 : i;`

```c
 1284: 
 1285: 	i = (int) (indices[0] + 0.5);
 1286→ 	i = (i < 0) ? 0 : i;
 1287: 	i = (i >= 0) ? dsize - 1 : i;
 1288: 
```

**Suggested fix (from review):** `i = (i >= dsize) ? dsize - 1 : i;`

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

## SJW1-17

**copy-paste (wrong buffer)** · Conf H / Sev L  
**Found at (current code):** `dfcoords.c` 1452, `dfcoords.c` 1463, `dfcoords.c` 1449
**Lines at review (Oct 2026):** 1460-1471

**Function:** `df_create_geo_transform()`

**Why it's thought to be a bug**

`buf` is filled from `int_proj` once (line 1460) and `parseline` tokenises it in place for `int_mp`. The `ext_geotype == GT_PROJECTION` branch then calls `parseline(buf, ...)` again without first copying `ext_proj`, so `ext_mp` is built from the internal projection (or its already-split remains). Also, when `int_geotype == GT_GEOGRAPHIC`, `int_proj` is NULL and is passed to `%s`, which is undefined behaviour. Latent because `dfSetGeoType` / `dfSetDefaultGeoType` are never called, so `ext_geotype` stays `GT_NONE`. Function: `df_create_geo_transform()`.

**Code at issue** — `dfcoords.c` line 1452  
Search for: `ext_geotype == GT_PROJECTION`

```c
 1450: 		return NULL;
 1451: 
 1452→ 	if ((int_geotype == GT_PROJECTION) && (ext_geotype == GT_PROJECTION)
 1453: 			&& (strcasecmp(int_proj, ext_proj) == 0))
 1454: 		return NULL;
```

**Code at issue** — `dfcoords.c` line 1463  
Search for: `parseline(buf,`

```c
 1461: 	data->int_geotype = int_geotype;
 1462: 	if (int_geotype == GT_PROJECTION) {
 1463→ 		int nargs = parseline(buf, args, 256);
 1464: 		data->int_mp = mpInitialise(nargs, args);
 1465: 	}
```

**Code at issue** — `dfcoords.c` line 1449  
Search for: `int_geotype == GT_GEOGRAPHIC`

```c
 1447: 		return NULL;
 1448: 
 1449→ 	if ((int_geotype == GT_GEOGRAPHIC) && (ext_geotype == GT_GEOGRAPHIC))
 1450: 		return NULL;
 1451: 
```

**Suggested fix (from review):** Do `snprintf(buf, sizeof(buf), "%s", ext_proj);` before the ext parse, and only format `int_proj` when it is non-NULL.

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

## SJW1-18

**ignored return value** · Conf M / Sev L  
**Found at (current code):** `dfcoords.c` 533
**Lines at review (Oct 2026):** 533, 548

**Function:** `dfInferAndSetCoordSystem()`

**Why it's thought to be a bug**

The return value of `dfSetCoordSystem(df, v, ncm, cmaps)` is ignored. It can fail with `return 0` (duplicates, non-coordinate dims) and leave `v->csystem == NULL`, but `dfInferAndSetCoordSystem` still returns 1. Callers (`tsEvalXYR`/`tsEvalXYZR`) therefore skip their fallback path and later die in `dfCoordsToIndices` with "No coordinate system specified". On the `failed` branch, `cmaps[0..ncm-1].coordids/coordtypes` are also leaked. Function: `dfInferAndSetCoordSystem()`.

**Code at issue** — `dfcoords.c` line 533  
Search for: `dfSetCoordSystem(df, v, ncm, cmaps)`

```c
  531: 	if (!failed) {
  532: 		/* Now set the coordinate system */
  533→ 		dfSetCoordSystem(df, v, ncm, cmaps);
  534: 
  535: 		/* Free memory associated with coordinate mapping */
```

**Suggested fix (from review):** `int ok = dfSetCoordSystem(...); free cmaps...; return ok;` and free the cmaps on the failed path too.

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

## SJW1-19

**memory leak** · Conf H / Sev L  
**Found at (current code):** `dfcoords.c` 768
**Lines at review (Oct 2026):** 945-971, 1546-1549

**Why it's thought to be a bug**

`df_free_coord_system` never frees `cmaps[i].transform`: neither the `CoordinateTransform`, nor its `private_data`, nor the `MapProjection`s. `geo_free()` frees the `GeoTransformData` but not `int_mp` / `ext_mp`. Functions: `df_free_coord_system()`, `geo_free()`.

**Code at issue** — `dfcoords.c` line 768 (located by name only; check this is the right place)  
Search for: `cm->transform->inverse(cm->transform->private_data, cm->nc,`

```c
  766: 			/* Perform an inverse coordinate transform (to user) if required. */
  767: 			if (cm->transform != NULL)
  768→ 				cm->transform->inverse(cm->transform->private_data, cm->nc,
  769: 						tcoords, ttcoords);
  770: 			for (j = 0; j < cm->nc; ++j)
```

**Suggested fix (from review):** Call `cm->transform->free(cm->transform->private_data); free(cm->transform);` and free the map projections inside `geo_free`.

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

## SJW1-20

**memory leak** · Conf H / Sev L  
**Found at (current code):** `dfcoords.c` 235, `dfcoords.c` 181
**Lines at review (Oct 2026):** 123-124, 136-171, 181, 199-202

**Function:** `dfSetCoordSystem()`

**Why it's thought to be a bug**

Each early `return 0` after line 124 leaks `coordids`/`coordtypes`, and the duplicate check's early returns also leak the `mp->dimids`/`depdimids` allocated by earlier calls. `v->ncdimids` is `malloc`ed on every call without freeing the previous allocation. Function: `dfSetCoordSystem()`.

**Code at issue** — `dfcoords.c` line 235  
Search for: `mp->dimids`

```c
  233: 		Variable *fv = &df->variables[mp->coordids[0]];
  234: 		mp->nd = 0;
  235→ 		mp->dimids = (int *) malloc(sizeof(int) * (size_t)fv->nd);
  236: 		mp->ndepd = 0;
  237: 		mp->depdimids = (int *) malloc(sizeof(int) * (size_t)fv->nd);
```

**Code at issue** — `dfcoords.c` line 181  
Search for: `v->ncdimids`

```c
  179: 	 */
  180: 	v->nncd = 0;
  181→ 	v->ncdimids = (int *) malloc((size_t)v->nd * sizeof(int));
  182: 
  183: 	for (i = 0; i < v->nd; ++i) {
```

**Suggested fix (from review):** Free `coordids`/`coordtypes` before every early return, and `free(v->ncdimids)` before reallocating it.

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

## SJW1-21

**buffer overrun** · Conf M / Sev L  
**Found at (current code):** `dfcoords.c` 815, `dfcoords.c` 805, `dfcoords.c` 374
**Lines at review (Oct 2026):** 815; 374, 403-405

**Why it's thought to be a bug**

`strcpy(line, coords)` copies a netCDF `coordinates` attribute of arbitrary length into `char line[MAXLINELEN]` without a length check (`df_decode_coords()`). In `dfInferAndSetCoordSystem()`, `coordids[MAXNUMCOORDS]` (10) is filled with up to `v->nc` entries without a bound check, and `v->nc` can exceed 10 for a long coordinate list.

**Code at issue** — `dfcoords.c` line 815  
Search for: `strcpy(line, coords)`

```c
  813: 	if (coords != NULL) {
  814: 		length = (int)strlen(coords);
  815→ 		strcpy(line, coords);
  816: 		for (i = 0; i < length; ++i) {
  817: 			/* If space,tab or comma then delimits a variable */
```

**Code at issue** — `dfcoords.c` line 805  
Search for: `char line[MAXLINELEN]`

```c
  803: void df_decode_coords(Datafile *df, Variable *v, char *coords) {
  804: 	int i, j, k;
  805→ 	char line[MAXLINELEN];
  806: 	int maxnc = v->nd + 1; /* Maximum number of coordinates */
  807: 	int *coordids;
```

**Code at issue** — `dfcoords.c` line 374  
Search for: `coordids[MAXNUMCOORDS]`

```c
  372: 	CoordinateMapping cmaps[MAXNUMCMAPS];
  373: 	CoordinateMapping *cm;
  374→ 	int coordids[MAXNUMCOORDS];
  375: 
  376: 	/* No coordinate system required if 0 dimensional */
```

**Suggested fix (from review):** Use `snprintf`/`strncpy` with `sizeof(line)`, and quit or warn if `v->nc > MAXNUMCOORDS`.

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

## SJW1-22

**memory leak** · Conf H / Sev L  
**Found at (current code):** `dfcoords.c` 1326
**Lines at review (Oct 2026):** 1324-1345, 1348-1370

**Function:** `df_set_2d_coords()`

**Why it's thought to be a bug**

`rgi` / `pgi` are `malloc`ed before the `sscanf(...)==10` test. If the analytic attribute is malformed, the code falls through to the bilinear mapping without freeing them. Function: `df_set_2d_coords()`.

**Code at issue** — `dfcoords.c` line 1326  
Search for: `sscanf(`

```c
 1324: 					RectGridInfo *rgi = (RectGridInfo *) malloc(
 1325: 							sizeof(RectGridInfo));
 1326→ 					if (sscanf(xa, "%s %lf %lf %d %d %lf %lf %lf %lf %lf",
 1327: 							atype, &rgi->ioffset, &rgi->joffset, &rgi->ni,
 1328: 							&rgi->nj, &rgi->x0, &rgi->y0, &rgi->dx, &rgi->dy,
```

**Suggested fix (from review):** Free `rgi` / `pgi` when the `sscanf` check fails.

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

dfcoords.c `df_decode_coord_type` lines 906-927: matching uses `strncasecmp(ctype, "time", strlen(ctype))`, so any prefix (and an empty string) matches. For example `""` maps to `VT_TIME` and `"l"` maps to `VT_LATITUDE`.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
