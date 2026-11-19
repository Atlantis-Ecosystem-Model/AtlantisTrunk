# sjwlib/dfcoords.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **9 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW1-14](#sjw1-14) | 765-771 | H/L | missing braces / uninitialised | `if (cm->transform != NULL) cm->transform->inverse(..., tcoords, ttcoords); for (j...) tcoords[j] = ttcoords[j… |  | [ ] |
| [SJW1-15](#sjw1-15) | 1223-1227 | H/L | off-by-one OOB read | `if ((i < 0) \|\| (i > ihigh)) return 0; coords[0] = (indices[0]-i)*(data[i+1]-data[i]) + data[i];`. When `i == … |  | [ ] |
| [SJW1-16](#sjw1-16) | 1285-1287 | H/L | always-true condition | `i = (i < 0) ? 0 : i; i = (i >= 0) ? dsize - 1 : i;`: after the first clamp `i >= 0` is always true, so every … |  | [ ] |
| [SJW1-17](#sjw1-17) | 1460-1471 | H/L | copy-paste (wrong buffer) | `buf` is filled from `int_proj` once (line 1460) and `parseline` tokenises it in place for `int_mp`. The `ext_… |  | [ ] |
| [SJW1-18](#sjw1-18) | 533, 548 | M/L | ignored return value | The return value of `dfSetCoordSystem(df, v, ncm, cmaps)` is ignored. It can fail with `return 0` (duplicates,… |  | [ ] |
| [SJW1-19](#sjw1-19) | 945-971, 1546-1549 | H/L | memory leak | `df_free_coord_system` never frees `cmaps[i].transform`: neither the `CoordinateTransform`, nor its `private_d… |  | [ ] |
| [SJW1-20](#sjw1-20) | 123-124, 136-171, 181, 199-202 | H/L | memory leak | Each early `return 0` after line 124 leaks `coordids`/`coordtypes`, and the duplicate check's early returns al… |  | [ ] |
| [SJW1-21](#sjw1-21) | 815; 374, 403-405 | M/L | buffer overrun | `strcpy(line, coords)` copies a netCDF `coordinates` attribute of arbitrary length into `char line[MAXLINELEN]… |  | [ ] |
| [SJW1-22](#sjw1-22) | 1324-1345, 1348-1370 | H/L | memory leak | `rgi` / `pgi` are `malloc`ed before the `sscanf(...)==10` test. If the analytic attribute is malformed, the co… |  | [ ] |

---

## SJW1-14

**missing braces / uninitialised** · Conf H / Sev L  
**Listed lines:** 765-771

**Function:** `dfIndicesToCoords()`

**Why it's thought to be a bug**

`if (cm->transform != NULL) cm->transform->inverse(..., tcoords, ttcoords); for (j...) tcoords[j] = ttcoords[j];`: the copy loop is outside the `if`. With no transform (the normal case) it overwrites the computed coordinates with uninitialised `ttcoords`. The forward path `dfCoordsToIndices` (lines 698-703) has the loop inside braces. Severity is low only because `dfIndicesToCoords` currently has no callers. Function: `dfIndicesToCoords()`.

**Current code** (`sjwlib/dfcoords.c` lines 763–773)

```c
  763: 		}
  764: 
  765: 		if (cm->indices_to_coords(df, cm, depindices, tindices, tcoords)) {
  766: 			/* Perform an inverse coordinate transform (to user) if required. */
  767: 			if (cm->transform != NULL)
  768: 				cm->transform->inverse(cm->transform->private_data, cm->nc,
  769: 						tcoords, ttcoords);
  770: 			for (j = 0; j < cm->nc; ++j)
  771: 				tcoords[j] = ttcoords[j];
  772: 
  773: 			/* Copy the coordinate system coords into the correct positions
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
**Listed lines:** 1223-1227

**Function:** `cm_1d_itoc()`

**Why it's thought to be a bug**

`if ((i < 0) || (i > ihigh)) return 0; coords[0] = (indices[0]-i)*(data[i+1]-data[i]) + data[i];`. When `i == ihigh` (the last index, a valid input) this reads `data[ihigh+1]`, one past the end of the coordinate array. Reached only through `dfIndicesToCoords` (unused). Function: `cm_1d_itoc()`.

**Current code** (`sjwlib/dfcoords.c` lines 1221–1229)

```c
 1221: 	}
 1222: 
 1223: 	i = (int) indices[0];
 1224: 	if ((i < 0) || (i > ihigh))
 1225: 		return 0;
 1226: 
 1227: 	coords[0] = (indices[0] - i) * (data[i + 1] - data[i]) + data[i];
 1228: 
 1229: 	return 1;
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
**Listed lines:** 1285-1287

**Function:** `cm_1d_multi_itoc()`

**Why it's thought to be a bug**

`i = (i < 0) ? 0 : i; i = (i >= 0) ? dsize - 1 : i;`: after the first clamp `i >= 0` is always true, so every index maps to the last point `dsize-1`. The upper clamp should test `i >= dsize`. Reached only through `dfIndicesToCoords` (unused). Function: `cm_1d_multi_itoc()`.

**Current code** (`sjwlib/dfcoords.c` lines 1283–1289)

```c
 1283: 	int i, j;
 1284: 
 1285: 	i = (int) (indices[0] + 0.5);
 1286: 	i = (i < 0) ? 0 : i;
 1287: 	i = (i >= 0) ? dsize - 1 : i;
 1288: 
 1289: 	for (j = 0; j < cm->nc; ++j)
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
**Listed lines:** 1460-1471

**Function:** `df_create_geo_transform()`

**Why it's thought to be a bug**

`buf` is filled from `int_proj` once (line 1460) and `parseline` tokenises it in place for `int_mp`. The `ext_geotype == GT_PROJECTION` branch then calls `parseline(buf, ...)` again without first copying `ext_proj`, so `ext_mp` is built from the internal projection (or its already-split remains). Also, when `int_geotype == GT_GEOGRAPHIC`, `int_proj` is NULL and is passed to `%s`, which is undefined behaviour. Latent because `dfSetGeoType` / `dfSetDefaultGeoType` are never called, so `ext_geotype` stays `GT_NONE`. Function: `df_create_geo_transform()`.

**Current code** (`sjwlib/dfcoords.c` lines 1458–1473)

```c
 1458: 	memset(data, 0, sizeof(struct GeoTransformData));
 1459: 
 1460: 	snprintf(buf, sizeof(buf), "%s", int_proj);
 1461: 	data->int_geotype = int_geotype;
 1462: 	if (int_geotype == GT_PROJECTION) {
 1463: 		int nargs = parseline(buf, args, 256);
 1464: 		data->int_mp = mpInitialise(nargs, args);
 1465: 	}
 1466: 
 1467: 	data->ext_geotype = ext_geotype;
 1468: 	if (ext_geotype == GT_PROJECTION) {
 1469: 		int nargs = parseline(buf, args, 256);
 1470: 		data->ext_mp = mpInitialise(nargs, args);
 1471: 	}
 1472: 
 1473: 	/* Build the transform */
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
**Listed lines:** 533, 548

**Function:** `dfInferAndSetCoordSystem()`

**Why it's thought to be a bug**

The return value of `dfSetCoordSystem(df, v, ncm, cmaps)` is ignored. It can fail with `return 0` (duplicates, non-coordinate dims) and leave `v->csystem == NULL`, but `dfInferAndSetCoordSystem` still returns 1. Callers (`tsEvalXYR`/`tsEvalXYZR`) therefore skip their fallback path and later die in `dfCoordsToIndices` with "No coordinate system specified". On the `failed` branch, `cmaps[0..ncm-1].coordids/coordtypes` are also leaked. Function: `dfInferAndSetCoordSystem()`.

**Current code** (`sjwlib/dfcoords.c` lines 531–550)

```c
  531: 	if (!failed) {
  532: 		/* Now set the coordinate system */
  533: 		dfSetCoordSystem(df, v, ncm, cmaps);
  534: 
  535: 		/* Free memory associated with coordinate mapping */
  536: 		for (i = 0; i < ncm; ++i) {
  537: 			free(cmaps[i].coordtypes);
  538: 			free(cmaps[i].coordids);
  539: 		}
  540: 	}
  541: 
  542: 	/* Unsupported */
  543: 	else {
  544: 		warn("dfInferCoordSystem: The coordinate system is unsupported.\n");
  545: 		return 0;
  546: 	}
  547: 
  548: 	return 1;
  549: }
  550: 
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
**Listed lines:** 945-971, 1546-1549

**Why it's thought to be a bug**

`df_free_coord_system` never frees `cmaps[i].transform`: neither the `CoordinateTransform`, nor its `private_data`, nor the `MapProjection`s. `geo_free()` frees the `GeoTransformData` but not `int_mp` / `ext_mp`. Functions: `df_free_coord_system()`, `geo_free()`.

**Current code** (`sjwlib/dfcoords.c` lines 943–973)

```c
  943:  * @param csystem pointer to a coordinate system.
  944:  */
  945: void df_free_coord_system(Datafile *df, CoordinateSystem *csystem) {
  946: 	if (csystem != NULL) {
  947: 		int i;
  948: 		for (i = 0; i < csystem->ncm; ++i) {
  949: 			if (csystem->cmaps[i].free != NULL)
  950: 				csystem->cmaps[i].free(df, &csystem->cmaps[i]);
  951: 			if (csystem->cmaps[i].depdimids != NULL)
  952: 				free(csystem->cmaps[i].depdimids);
  953: 			if (csystem->cmaps[i].dimids != NULL)
  954: 				free(csystem->cmaps[i].dimids);
  955: 			if (csystem->cmaps[i].coordids != NULL)
  956: 				free(csystem->cmaps[i].coordids);
  957: 			if (csystem->cmaps[i].coordtypes != NULL)
  958: 				free(csystem->cmaps[i].coordtypes);
  959: 
  960: 		}
  961: 
  962: 		if (csystem->ncm > 0) {
  963: 			free(csystem->cmaps);
  964: 			free(csystem->cmorder);
  965: 			free(csystem->coordids);
  966: 			free(csystem->coordtypes);
  967: 
  968: 		}
  969: 
  970: 		free(csystem);
  971: 	}
  972: }
  973: 
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
**Listed lines:** 123-124, 136-171, 181, 199-202

**Function:** `dfSetCoordSystem()`

**Why it's thought to be a bug**

Each early `return 0` after line 124 leaks `coordids`/`coordtypes`, and the duplicate check's early returns also leak the `mp->dimids`/`depdimids` allocated by earlier calls. `v->ncdimids` is `malloc`ed on every call without freeing the previous allocation. Function: `dfSetCoordSystem()`.

**Current code** (`sjwlib/dfcoords.c` lines 121–160 (truncated))

```c
  121: 	 * This makes it easier to check for duplicates, check for
  122: 	 * existance of variables, and defines the order */
  123: 	coordids = (int *) malloc(sizeof(int) * (size_t)nc);
  124: 	coordtypes = (int *) malloc(sizeof(int) * (size_t)nc);
  125: 	for (i = 0, k = 0; i < ncm; ++i) {
  126: 		for (j = 0; j < cmaps[i].nc; ++j, ++k) {
  127: 			coordids[k] = cmaps[i].coordids[j];
  128: 			coordtypes[k] = cmaps[i].coordtypes[j];
  129: 		}
  130: 	}
  131: 
  132: 	/* Check for duplicate coordinates */
  133: 	for (i = 0; i < nc; ++i) {
  134: 		for (j = i + 1; j < nc; ++j) {
  135: 			if (coordids[i] == coordids[j]) {
  136: 				warn(
  137: 						"dfSetCoordSystem: Duplicate coordinate variables specified.\n");
  138: 				return 0;
  139: 			}
  140: 
  141: 			if (coordtypes[i] == coordtypes[j]) {
  142: 				warn(
  143: 						"dfSetCoordSystem: Duplicate coordinate types specified.\n");
  144: 				return 0;
  145: 			}
  146: 		}
  147: 	}
  148: 
  149: 	/* Verify that the coord variable exists, and that it's
  150: 	 * dimensions are a subset of the data variable coordinates.
  151: 	 * All remaining dimensions are non-coordinate dimensions. */
  152: 	for (i = 0; i < nc; ++i) {
  153: 
  154: 		/* Range check */
  155: 		if ((coordids[i] < 0) || (coordids[i] >= df->nv))
  156: 			quit("dfSetCoordSystem: Invalid coordinate id.\n");
  157: 		cv = &df->variables[coordids[i]];
  158: 
  159: 		/* Check the dimensions */
  160: 		for (j = 0; j < cv->nd; ++j) {
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
**Listed lines:** 815; 374, 403-405

**Why it's thought to be a bug**

`strcpy(line, coords)` copies a netCDF `coordinates` attribute of arbitrary length into `char line[MAXLINELEN]` without a length check (`df_decode_coords()`). In `dfInferAndSetCoordSystem()`, `coordids[MAXNUMCOORDS]` (10) is filled with up to `v->nc` entries without a bound check, and `v->nc` can exceed 10 for a long coordinate list.

**Current code** (`sjwlib/dfcoords.c` lines 813–817)

```c
  813: 	if (coords != NULL) {
  814: 		length = (int)strlen(coords);
  815: 		strcpy(line, coords);
  816: 		for (i = 0; i < length; ++i) {
  817: 			/* If space,tab or comma then delimits a variable */
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
**Listed lines:** 1324-1345, 1348-1370

**Function:** `df_set_2d_coords()`

**Why it's thought to be a bug**

`rgi` / `pgi` are `malloc`ed before the `sscanf(...)==10` test. If the analytic attribute is malformed, the code falls through to the bilinear mapping without freeing them. Function: `df_set_2d_coords()`.

**Current code** (`sjwlib/dfcoords.c` lines 1322–1361 (truncated))

```c
 1322: 				 * polar. */
 1323: 				if (strcasecmp(atype, "rectangular") == 0) {
 1324: 					RectGridInfo *rgi = (RectGridInfo *) malloc(
 1325: 							sizeof(RectGridInfo));
 1326: 					if (sscanf(xa, "%s %lf %lf %d %d %lf %lf %lf %lf %lf",
 1327: 							atype, &rgi->ioffset, &rgi->joffset, &rgi->ni,
 1328: 							&rgi->nj, &rgi->x0, &rgi->y0, &rgi->dx, &rgi->dy,
 1329: 							&rgi->th) == 10) {
 1330: 						rgi->sinth = sin(rgi->th * M_PI / 180.0);
 1331: 						rgi->costh = cos(rgi->th * M_PI / 180.0);
 1332: 
 1333: 						/* Shift origin to account for i/j offset */
 1334: 						rgi->x0 = rgi->ioffset * rgi->dx * rgi->costh
 1335: 								- rgi->joffset * rgi->dy * rgi->sinth + rgi->x0;
 1336: 						rgi->y0 = rgi->ioffset * rgi->dx * rgi->sinth
 1337: 								+ rgi->joffset * rgi->dy * rgi->costh + rgi->y0;
 1338: 						cm->free = cm_rect_2free;
 1339: 						cm->coords_to_indices = cm_rect_2d_ctoi;
 1340: 						cm->indices_to_coords = cm_rect_2d_itoc;
 1341: 						cm->special_data = rgi;
 1342: 						df_set_geo_transform(df, cm);
 1343: 						return;
 1344: 					}
 1345: 				}
 1346: 
 1347: 				else if (strcasecmp(atype, "polar") == 0) {
 1348: 					PolarGridInfo *pgi = (PolarGridInfo *) malloc(
 1349: 							sizeof(PolarGridInfo));
 1350: 					if (sscanf(xa, "%s %lf %lf %d %d %lf %lf %lf %lf %lf",
 1351: 							atype, &pgi->ioffset, &pgi->joffset, &pgi->ni,
 1352: 							&pgi->nj, &pgi->x0, &pgi->y0, &pgi->arc,
 1353: 							&pgi->rmin, &pgi->rotation) == 10) {
 1354: 
 1355: 						pgi->rotation = pgi->rotation * DEG2RAD;
 1356: 						if (pgi->rotation >= 2.0 * M_PI)
 1357: 							pgi->rotation -= 2.0 * M_PI;
 1358: 						if (pgi->rotation < 0.0)
 1359: 							pgi->rotation += 2.0 * M_PI;
 1360: 						pgi->dth = (pgi->arc / (pgi->ni - 1)) * DEG2RAD;
 1361: 						pgi->logrmin = log(pgi->rmin);
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
