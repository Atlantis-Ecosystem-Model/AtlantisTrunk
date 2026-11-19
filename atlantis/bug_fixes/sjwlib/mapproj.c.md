# sjwlib/mapproj.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW2-01](#sjw2-01) ⭐ | 587, 301, 599 | H/H | invalid free / realloc of caller memory | `add_argument()` copies the caller's string pointers into the new array (`new_args[i+1] = args[i]`, line 587).… |  | [ ] |
| [SJW2-02](#sjw2-02) | 404 | H/M | wrong formula | `mp->ellip_flat = 1 / (1 - sqrt(1 - ecc * ecc));` In PROJ, `es` is the eccentricity squared, and flattening is… |  | [ ] |
| [SJW2-03](#sjw2-03) | 568 | H/L | memory leak | `add_argument()` ignores `freeOld` (that code is commented out), so each chained call leaks the previous point… |  | [ ] |
| [SJW2-04](#sjw2-04) | 552, 382, 948 | M/L | prefix match | `strncasecmp(tag, args[i], strlen(tag))` matches any argument that starts with the tag. So `get_argument_value… |  | [ ] |

---

## SJW2-01

**invalid free / realloc of caller memory** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `mapproj.c` 587, `mapproj.c` 301, `mapproj.c` 599
**Lines at review (Oct 2026):** 298-347, 561-604

**Function:** `mpInitialise()`

**Why it's thought to be a bug**

`add_argument()` copies the caller's string pointers into the new array (`new_args[i+1] = args[i]`, line 587). Then `subst_argument_value("proj", ...)` calls `realloc(args[index], ...)` (line 599) on the caller's `proj=` string. The amg/mga branches also `free()` every `new_args[i]` (lines 303-307, 316-320), which includes the caller's original strings. Every caller builds `args` with `parseline()` (sjwlib/text_input.c:85), which returns pointers into a stack buffer (atphysics/atgeomIO.c:788-807, atecology/dump.c:143-152, sjwlib/dfcoords.c:1463-1470). So `proj=utm`, `proj=amg` or `proj=mga` calls realloc/free on non-heap memory, which is undefined behaviour and on macOS aborts with "pointer being realloc'd was not allocated". The amg/mga path also leaves dangling pointers in the caller's array. Function: `mpInitialise()` / `add_argument()` / `subst_argument_value()`.

**Code at issue** — `mapproj.c` line 587  
Search for: `new_args[i+1] = args[i]`

```c
  585: 
  586:     for (i=0; i<nargs; ++i)
  587→        new_args[i+1] = args[i];
  588: 
  589: 	return new_args;
```

**Code at issue** — `mapproj.c` line 301  
Search for: `subst_argument_value("proj",`

```c
  299: 		new_args = add_argument("ellps=agd66", nargs, args, &new_nargs, FALSE);
  300: 		new_args = add_argument("south", new_nargs, new_args, &new_nargs, TRUE);
  301→ 		subst_argument_value("proj", "utm", new_nargs, new_args);
  302: 		mp = mpInitialise(new_nargs, new_args);
  303: 		for (i = 0; i < new_nargs; i++) {
```

**Code at issue** — `mapproj.c` line 599  
Search for: `realloc(args[index],`

```c
  597:     if (index >= 0){
  598:         size_t needed = strlen(arg) + 1 + strlen(value) + 1;  // arg + '=' + value + '\0'
  599→         args[index] = (char *)realloc(args[index], needed);  // So can have the size "needed" for the snprintf call
  600:         snprintf(args[index], needed, "%s=%s", arg, value);
  601:         
```

**Suggested fix (from review):** In `add_argument` use `strdup(args[i])` (and honour `freeOld`), so that the code owns every string it reallocs or frees.

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

## SJW2-02

**wrong formula** · Conf H / Sev M  
**Found at (current code):** `mapproj.c` 404
**Lines at review (Oct 2026):** 401-405

**Function:** `mpInitialise()`

**Why it's thought to be a bug**

`mp->ellip_flat = 1 / (1 - sqrt(1 - ecc * ecc));` In PROJ, `es` is the eccentricity squared, and flattening is `f = 1 - sqrt(1 - es)`. The code squares `es` again and takes the reciprocal. With `es=0` (sphere) this gives `1/0 = inf`; for real ellipsoids it gives about 300 instead of 0.0033. Every later `e2` in `merc_init`/`tcc_init`/`lcc_init` is then wrong. Function: `mpInitialise()`.

**Code at issue** — `mapproj.c` line 404  
Search for: `mp->ellip_flat = 1 / (1 - sqrt(1 - ecc * ecc))`

```c
  402: 			if (es != NULL) {
  403: 				double ecc = atof(es);
  404→ 				mp->ellip_flat = 1 / (1 - sqrt(1 - ecc * ecc));
  405: 			}
  406: 		}
```

**Suggested fix (from review):** `mp->ellip_flat = 1 - sqrt(1 - atof(es));`

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

## SJW2-03

**memory leak** · Conf H / Sev L  
**Found at (current code):** `mapproj.c` 568
**Lines at review (Oct 2026):** 561-590, 1043-1051

**Function:** `add_argument()`

**Why it's thought to be a bug**

`add_argument()` ignores `freeOld` (that code is commented out), so each chained call leaks the previous pointer array. The utm branch never frees `new_args` (lines 342-346 are commented out), and `serov_init` leaks too. The allocation `malloc(sizeof(char *)*strlen(arg)+1)` (line 583) over-allocates by 8x. Function: `add_argument()`.

**Code at issue** — `mapproj.c` line 568  
Search for: `malloc(sizeof(char *) * strlen(arg) + 1)`

```c
  566: 	//new_args = c_alloc2d((*new_nargs), STRLEN);
  567: //	new_args = (char**) malloc(sizeof(char*) * (*new_nargs));
  568→ //	new_args[0] = (char *) malloc(sizeof(char *) * strlen(arg) + 1);
  569: //	strcpy(new_args[0], arg);
  570: //
```

**Suggested fix (from review):** Free the old array when `freeOld`, and allocate `strlen(arg)+1`.

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

## SJW2-04

**prefix match** · Conf M / Sev L  
**Found at (current code):** `mapproj.c` 552, `mapproj.c` 382, `mapproj.c` 948
**Lines at review (Oct 2026):** 548-556

**Function:** `find_argument()`

**Why it's thought to be a bug**

`strncasecmp(tag, args[i], strlen(tag))` matches any argument that starts with the tag. So `get_argument_value("a", ...)`/`("b", ...)` (lines 382-383) match unrelated arguments such as `axis=...` or `b...`, and `"lat_0"`-style lookups also match longer keys. `atof` of the wrong value then silently sets the ellipsoid axes. Function: `find_argument()`.

**Code at issue** — `mapproj.c` line 552  
Search for: `strncasecmp(tag, args[i], strlen(tag))`

```c
  550: 
  551: 	for (i = 0; i < nargs; ++i) {
  552→ 		if (strncasecmp(tag, args[i], strlen(tag)) == 0)
  553: 			return i;
  554: 	}
```

**Code at issue** — `mapproj.c` line 382  
Search for: `get_argument_value("a",`

```c
  380: 
  381: 		if (!got_ellip_data) {
  382→ 			char *major = get_argument_value("a", nargs, args, 0);
  383: 			char *minor = get_argument_value("b", nargs, args, 0);
  384: 			char *rf = get_argument_value("rf", nargs, args, 0);
```

**Code at issue** — `mapproj.c` line 948  
Search for: `"lat_0"`

```c
  946: 
  947: 	/* If lat_0 is not provided, then compute it. */
  948→ 	if (find_argument("lat_0", nargs, args) < 0) {
  949: 		data->sinphi0 = data->nn;
  950: 		data->phi0 = asin(data->sinphi0);
```

**Suggested fix (from review):** Also require `args[i][strlen(tag)]` to be `'='` or `'\0'`.

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
