# sjwlib/datafile.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **9 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1), plus 2 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW1-05](#sjw1-05) | 171, 174, 177 | H/M | wrong operator | `if ((varid < 0) && (varid >= df->nv))` can never be true, so an invalid `varid` is never caught. `rv = &df->v… |  | [ ] |
| [SJW1-06](#sjw1-06) ⭐ | 318 | H/H | wrong formula (recycling) | When rewinding is on and `r` is past the last record: `leftover = floor(r / lengthdf); *newt = r - leftover * … |  | [ ] |
| [SJW1-07](#sjw1-07) | 1014, 216, 219 | M/M | out-of-bounds read | `df_netcdf_read` allocates `v->dimids = malloc(v->nd*sizeof(int))`, which is a zero-byte block for scalar (0-d… |  | [ ] |
| [SJW1-08](#sjw1-08) | 1320, 1325 | H/L | type mismatch / memory overwrite | `nc_inq_att(..., &type, (size_t *) &a->n)`, but `Attribute.n` is declared `int` (datafile.h:138, the `size_t n… |  | [ ] |
| [SJW1-09](#sjw1-09) | 1507, 550, 1512 | M/M | wrong bound check | `df_read_data` rejects any `r >= df->nrecords`, even for variables that do not use the record dimension. In a … |  | [ ] |
| [SJW1-10](#sjw1-10) | 1242, 1456 | M/M | free of non-pointer | ASCII columns are stored as a flat `double[n]` with `nd = 1`, `nrecords = 1`, `dim_as_record = 0`. They only b… |  | [ ] |
| [SJW1-11](#sjw1-11) | 1659 | M/L | off-by-one / sign error | When clamping at the upper end: `nrecs -= ((int)df->nrecords - (nstart + v->nrecords));` adds the overshoot in… |  | [ ] |
| [SJW1-12](#sjw1-12) | 1116 | M/L | buffer underflow | The leading-trim loop skips `' '`, `'\t'` and `'\n'` but not `'\r'`, while the trailing loop also strips `'\r'… |  | [ ] |
| [SJW1-13](#sjw1-13) | 1381, 1456 | M/L | type mismatch (latent config) | `df_alloc_data_record` / `df_free_data_record` use the `alloc1d..4d` / `free1d..4d` macros. With `_ATLANTIS_LO… |  | [ ] |

---

## SJW1-05

**wrong operator** · Conf H / Sev M  
**Found at (current code):** `datafile.c` 171, `datafile.c` 174, `datafile.c` 177
**Lines at review (Oct 2026):** 171-174

**Function:** `dfSetRecord()`

**Why it's thought to be a bug**

`if ((varid < 0) && (varid >= df->nv))` can never be true, so an invalid `varid` is never caught. `rv = &df->variables[varid]` then indexes out of bounds and `rv->nd` / `rv->dimids` are read from garbage. Compare the correct check in `dfGetVariable()` line 591. Function: `dfSetRecord()`.

**Code at issue** — `datafile.c` line 171  
Search for: `if ((varid < 0) && (varid >= df->nv))`

```c
  169: 
  170: 	/* Check if this is a valid variable */
  171→ 	if ((varid < 0) && (varid >= df->nv))
  172: 		quit("dfSetRecord: Invalid variable identifier.\n");
  173: 	else
```

**Code at issue** — `datafile.c` line 174  
Search for: `rv = &df->variables[varid]`

```c
  172: 		quit("dfSetRecord: Invalid variable identifier.\n");
  173: 	else
  174→ 		rv = &df->variables[varid];
  175: 
  176: 	/* Confirm that the variable contains only one dimension. */
```

**Code at issue** — `datafile.c` line 177 (closest match to the quoted code; wording differs slightly)  
Search for: `if ((rv->nd == 1) && (df->dimensions[rv->dimids[0]].size > 0))`

```c
  175: 
  176: 	/* Confirm that the variable contains only one dimension. */
  177→ 	if ((rv->nd == 1) && (df->dimensions[rv->dimids[0]].size > 0))
  178: 		recdimid = rv->dimids[0];
  179: 	else
```

**Suggested fix (from review):** Use `||` instead of `&&`.

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

## SJW1-06

**wrong formula (recycling)** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `datafile.c` 318
**Lines at review (Oct 2026):** 317-321

**Function:** `dfFindTime()`

**Why it's thought to be a bug**

When rewinding is on and `r` is past the last record: `leftover = floor(r / lengthdf); *newt = r - leftover * df->records[ilow];`. The code multiplies by the first record time instead of the period length `lengthdf`, and the floor ignores the record origin. If the series starts at t=0 (`records[0]==0`), `newt == r`, so nothing is recycled and `dfEval` clamps to the last record for the rest of the run. For any other start time the result is an arbitrary time. This is used for the rewound thermal and rate index series (`tsEvalR(..., bm->tindex_rewindid)` in atecology/atdemography.c:1510-1511 and atannualbiology.c:1246-1247) and in `tsEvalXYR`/`tsEvalXYZR`. Function: `dfFindTime()`.

**Code at issue** — `datafile.c` line 318 (the line the review pointed to, found by its text)  
Search for: `double leftover = floor(r / lengthdf);`

```c
  316: 			/* If not in range and use cycling of forcing then recycle */
  317: 			double lengthdf = df->records[ihigh] - df->records[ilow];
  318→ 			double leftover = floor(r / lengthdf);
  319: 
  320: 			*newt = r - leftover * df->records[ilow];
```

**Suggested fix (from review):** `*newt = df->records[ilow] + fmod(r - df->records[ilow], lengthdf);` (guard `lengthdf > 0`).

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

## SJW1-07

**out-of-bounds read** · Conf M / Sev M  
**Found at (current code):** `datafile.c` 1014, `datafile.c` 216, `datafile.c` 219
**Lines at review (Oct 2026):** 214-231 (with 1013-1015)

**Function:** `dfSetRecord()`

**Why it's thought to be a bug**

`df_netcdf_read` allocates `v->dimids = malloc(v->nd*sizeof(int))`, which is a zero-byte block for scalar (0-d) netCDF variables. `dfSetRecord` then tests `if (v->dimids[0] == recdimid)` for every variable without checking `v->nd > 0`. That reads uninitialised heap memory. If the garbage equals `recdimid` (often 0), the scalar gets `dim_as_record=1` and `--(v->nd)` makes `nd == -1`, so later reads quit with "Bad number of dimensions" or misbehave. Function: `dfSetRecord()`.

**Code at issue** — `datafile.c` line 1014 (closest match to the quoted code; wording differs slightly)  
Search for: `v->dimids = (int *) malloc((size_t)v->nd * sizeof(int));`

```c
 1012: 			/* Extract the dimension information for each variable */
 1013: 			nc_inq_varndims(df->ncid, v->varid, &v->nd);
 1014→ 			v->dimids = (int *) malloc((size_t)v->nd * sizeof(int));
 1015: 			nc_inq_vardimid(df->ncid, v->varid, v->dimids);
 1016: 
```

**Code at issue** — `datafile.c` line 216  
Search for: `if (v->dimids[0] == recdimid)`

```c
  214: 	for (i = 0; i < df->nv; ++i) {
  215: 		Variable *v = &df->variables[i];
  216→ 		if (v->dimids[0] == recdimid) {
  217: 			int i;
  218: 
```

**Code at issue** — `datafile.c` line 219  
Search for: `dim_as_record = 1`

```c
  217: 			int i;
  218: 
  219→ 			v->dim_as_record = 1;
  220: 			if (v->data != NULL) {
  221: 				v->start_record = 0;
```

**Suggested fix (from review):** `if (v->nd > 0 && v->dimids[0] == recdimid)`.

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

## SJW1-08

**type mismatch / memory overwrite** · Conf H / Sev L  
**Found at (current code):** `datafile.c` 1320, `datafile.c` 1325
**Lines at review (Oct 2026):** 1320

**Function:** `df_netcdf_read_attrib()`

**Why it's thought to be a bug**

`nc_inq_att(..., &type, (size_t *) &a->n)`, but `Attribute.n` is declared `int` (datafile.h:138, the `size_t n` is commented out at :137). netCDF writes 8 bytes into a 4-byte field and overwrites the adjacent `a->type`. On little-endian hardware this happens to be harmless (high word 0, and `type` is reassigned afterwards except for unsupported types or `n==0`). On big-endian hardware `a->n` would always be 0. This is undefined behaviour either way. Function: `df_netcdf_read_attrib()`.

**Code at issue** — `datafile.c` line 1320  
Search for: `nc_inq_att(`

```c
 1318: 
 1319: 	/* Get the attribute type and length */
 1320→ 	nc_inq_att(df->ncid, varid, a->name, &type, (size_t *) &a->n);
 1321: 	if (type == NC_CHAR) {
 1322: 		a->value = (void*) malloc(sizeof(char) * (size_t)(a->n + 1));
```

**Code at issue** — `datafile.c` line 1325  
Search for: `a->type`

```c
 1323: 		memset(a->value, 0, sizeof(char) * (size_t)(a->n + 1));
 1324: 		nc_get_att_text(df->ncid, varid, a->name, (char*) a->value);
 1325→ 		a->type = AT_TEXT;
 1326: 	}
 1327: 
```

**Suggested fix (from review):** Read into a local `size_t len;` and then set `a->n = (int)len;`.

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

## SJW1-09

**wrong bound check** · Conf M / Sev M  
**Found at (current code):** `datafile.c` 1507, `datafile.c` 550, `datafile.c` 1512
**Lines at review (Oct 2026):** 1507-1510 (with 371-385)

**Function:** `df_read_data()`

**Why it's thought to be a bug**

`df_read_data` rejects any `r >= df->nrecords`, even for variables that do not use the record dimension. In a netCDF file with no record/time variable, `df->nrecords == 0`, so `dfReadRecord(df, v, 0)` on any static variable (e.g. coordinate reads in `df_set_2d_coords` line 1392, or `dfEval` on a file without time) quits with "Attempt to read an invalid record". `dfReadRecords` applies the same bound only when `v->dim_as_record` is set (line 372). Function: `df_read_data()`.

**Code at issue** — `datafile.c` line 1507  
Search for: `r >= df->nrecords`

```c
 1505: 	int index = r - v->start_record;
 1506: 
 1507→ 	if ((r < 0) || (r >= df->nrecords))
 1508: 		quit(
 1509: 				"df_read_data: Attempt to read an invalid record for variable '%s'.\n",
```

**Code at issue** — `datafile.c` line 550 (closest match to the quoted code; wording differs slightly)  
Search for: `void dfReadRecord(Datafile *df, Variable *v, int rec) {`

```c
  548:  * @see dfReadRecords
  549:  */
  550→ void dfReadRecord(Datafile *df, Variable *v, int rec) {
  551: 	dfReadRecords(df, v, rec, 1);
  552: }
```

**Code at issue** — `datafile.c` line 1512  
Search for: `v->dim_as_record`

```c
 1510: 				v->name);
 1511: 
 1512→ 	if (v->dim_as_record) {
 1513: 		start[i] = (size_t)r;
 1514: 		count[i++] = 1;
```

**Suggested fix (from review):** Apply the `r` range check only when `v->dim_as_record`; otherwise require `r == 0`.

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

## SJW1-10

**free of non-pointer** · Conf M / Sev M  
**Found at (current code):** `datafile.c` 1242, `datafile.c` 1456
**Lines at review (Oct 2026):** 1237-1246, 1447-1458 (via 508-509)

**Why it's thought to be a bug**

ASCII columns are stored as a flat `double[n]` with `nd = 1`, `nrecords = 1`, `dim_as_record = 0`. They only become `nd = 0` if `dfSetRecord` runs (i.e. a `t`/`time` column exists, timeseries.c:108-114). Without such a column, `dfFree` → `df_free_data_records` → `df_free_data_record` case 1 runs `free1d(VAR_1D(v)[0])`, which treats the bit pattern of the first data value as a pointer and frees it. That crashes at shutdown unless the value happens to be 0.0. Functions: `df_ascii_read()`, `df_free_data_record()`.

**Code at issue** — `datafile.c` line 1242  
Search for: `nrecords = 1`

```c
 1240: 		df->variables[i].dimids[0] = 0;
 1241: 		df->variables[i].start_record = 0;
 1242→ 		df->variables[i].nrecords = 1;
 1243: 		df->variables[i].dim_as_record = 0;
 1244: 		df->variables[i].data = (double*) malloc(df->dimensions[0].size
```

**Code at issue** — `datafile.c` line 1456 (closest match to the quoted code; wording differs slightly)  
Search for: `free1d(VAR_1D(v)[r]);`

```c
 1454: 		case 1:
 1455: 			if (VAR_1D(v)[r] != NULL)
 1456→ 				free1d(VAR_1D(v)[r]);
 1457: 			VAR_1D(v)[r] = NULL;
 1458: 			break;
```

**Suggested fix (from review):** In `df_free_data_records` (or `dfFree`), just `free(v->data)` when `df->type == DFT_ASCII`.

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

## SJW1-11

**off-by-one / sign error** · Conf M / Sev L  
**Found at (current code):** `datafile.c` 1659
**Lines at review (Oct 2026):** 1655-1661

**Function:** `df_shift_data()`

**Why it's thought to be a bug**

When clamping at the upper end: `nrecs -= ((int)df->nrecords - (nstart + v->nrecords));` adds the overshoot instead of subtracting it (the subtraction is reversed), and `nstart = df->nrecords - v->nrecords - 1;` is one short (it should be `df->nrecords - v->nrecords`). The function is currently not called anywhere (dead code), so this is latent. Function: `df_shift_data()`.

**Code at issue** — `datafile.c` line 1659 (closest match to the quoted code; wording differs slightly)  
Search for: `nrecs -= ((int)df->nrecords - (nstart + (int)v->nrecords));`

```c
 1657: 		nstart = 0;
 1658: 	} else if ((nstart + v->nrecords) > df->nrecords) {
 1659→ 		nrecs -= ((int)df->nrecords - (nstart + (int)v->nrecords));
 1660: 		nstart = (int)df->nrecords - (int)v->nrecords - 1;
 1661: 	}
```

**Suggested fix (from review):** `nrecs -= (nstart + v->nrecords - df->nrecords); nstart = df->nrecords - v->nrecords;`, or delete the unused function.

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

## SJW1-12

**buffer underflow** · Conf M / Sev L  
**Found at (current code):** `datafile.c` 1116
**Lines at review (Oct 2026):** 1116, 1132-1136

**Function:** `trim()`

**Why it's thought to be a bug**

The leading-trim loop skips `' '`, `'\t'` and `'\n'` but not `'\r'`, while the trailing loop also strips `'\r'`. A string of only `"\r"` characters (e.g. a CRLF `.ts` header line with an empty value) is not emptied by the first pass, so the trailing loop decrements `i` below 0 and reads `s[-1]`. Function: `trim()`.

**Code at issue** — `datafile.c` line 1116 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `while((s[i]==' ')||(s[i]=='\t') ||(s[i]=='\n')) {`

```c
 1114: 		return;
 1115: 
 1116→ 	while((s[i]==' ')||(s[i]=='\t') ||(s[i]=='\n')) {
 1117: 		i++;
 1118: 	}
```

**Suggested fix (from review):** Add `i >= 0 &&` to the trailing loop condition and `'\r'` to the leading set.

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

## SJW1-13

**type mismatch (latent config)** · Conf M / Sev L  
**Found at (current code):** `datafile.c` 1381, `datafile.c` 1456
**Lines at review (Oct 2026):** 1379-1402, 1454-1475

**Why it's thought to be a bug**

`df_alloc_data_record` / `df_free_data_record` use the `alloc1d..4d` / `free1d..4d` macros. With `_ATLANTIS_LONG_DOUBLE` defined (atlantisMem.h:20-43), those become `d_alloc*longd`, which return `long double` storage. But `VAR_xD` casts to `doubleINPUT` (= `double`) and `nc_get_vara_double` writes `double`s, so the buffers would be misinterpreted. atlantisMem.h:69-80 provides `d_alloc*dInput` precisely for input buffers. This is latent because the macro is commented out. Functions: `df_alloc_data_record()`, `df_free_data_record()`.

**Code at issue** — `datafile.c` line 1381 (the line the review pointed to, found by its text)  
Search for: `VAR_1D(v)[r] = alloc1d(e1);`

```c
 1379: 	case 1:
 1380: 		e1 = (int)df->dimensions[v->dimids[0]].size;
 1381→ 		VAR_1D(v)[r] = alloc1d(e1);
 1382: 		break;
 1383: 
```

**Code at issue** — `datafile.c` line 1456 (the line the review pointed to, found by its text)  
Search for: `free1d(VAR_1D(v)[r]);`

```c
 1454: 		case 1:
 1455: 			if (VAR_1D(v)[r] != NULL)
 1456→ 				free1d(VAR_1D(v)[r]);
 1457: 			VAR_1D(v)[r] = NULL;
 1458: 			break;
```

**Suggested fix (from review):** Use `d_alloc1dInput..d_alloc4dInput` / `d_free*dInput`.

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

datafile.c `dfReadRecords`: if the record variable itself were ever re-read with a different `nrecs` (as a coordinate in `v->csystem`), `df_free_data_records` would free `rv->data` and leave `df->records` dangling. Currently `dfInferAndSetCoordSystem` strips `df->ri` from coordinate lists (line 404), so I could not find a live path.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

datafile.c `df_netcdf_read` lines 1036-1040: `missing_value` and `_FillValue` are read with `ATT_DOUBLE`, which returns 0.0 unless the attribute is stored as NC_DOUBLE. Float or short missing values are silently treated as 0.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
