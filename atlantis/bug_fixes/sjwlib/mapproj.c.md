# sjwlib/mapproj.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW2-01](#sjw2-01) ⭐ | 298-347, 561-604 | H/H | invalid free / realloc of caller memory | `add_argument()` copies the caller's string pointers into the new array (`new_args[i+1] = args[i]`, line 587).… |  | [ ] |
| [SJW2-02](#sjw2-02) | 401-405 | H/M | wrong formula | `mp->ellip_flat = 1 / (1 - sqrt(1 - ecc * ecc));` In PROJ, `es` is the eccentricity squared, and flattening is… |  | [ ] |
| [SJW2-03](#sjw2-03) | 561-590, 1043-1051 | H/L | memory leak | `add_argument()` ignores `freeOld` (that code is commented out), so each chained call leaks the previous point… |  | [ ] |
| [SJW2-04](#sjw2-04) | 548-556 | M/L | prefix match | `strncasecmp(tag, args[i], strlen(tag))` matches any argument that starts with the tag. So `get_argument_value… |  | [ ] |

---

## SJW2-01

**invalid free / realloc of caller memory** · Conf H / Sev H ⭐ priority  
**Listed lines:** 298-347, 561-604

**Function:** `mpInitialise()`

**Why it's thought to be a bug**

`add_argument()` copies the caller's string pointers into the new array (`new_args[i+1] = args[i]`, line 587). Then `subst_argument_value("proj", ...)` calls `realloc(args[index], ...)` (line 599) on the caller's `proj=` string. The amg/mga branches also `free()` every `new_args[i]` (lines 303-307, 316-320), which includes the caller's original strings. Every caller builds `args` with `parseline()` (sjwlib/text_input.c:85), which returns pointers into a stack buffer (atphysics/atgeomIO.c:788-807, atecology/dump.c:143-152, sjwlib/dfcoords.c:1463-1470). So `proj=utm`, `proj=amg` or `proj=mga` calls realloc/free on non-heap memory, which is undefined behaviour and on macOS aborts with "pointer being realloc'd was not allocated". The amg/mga path also leaves dangling pointers in the caller's array. Function: `mpInitialise()` / `add_argument()` / `subst_argument_value()`.

**Current code** (`sjwlib/mapproj.c` lines 296–335 (truncated))

```c
  296: 
  297: 	/* Australian Map Grid */
  298: 	if (strcasecmp(projname, "amg") == 0) {
  299: 		new_args = add_argument("ellps=agd66", nargs, args, &new_nargs, FALSE);
  300: 		new_args = add_argument("south", new_nargs, new_args, &new_nargs, TRUE);
  301: 		subst_argument_value("proj", "utm", new_nargs, new_args);
  302: 		mp = mpInitialise(new_nargs, new_args);
  303: 		for (i = 0; i < new_nargs; i++) {
  304: 			if (new_args[i])
  305: 				free(new_args[i]);
  306: 		}
  307: 		free(new_args);
  308: 	}
  309: 
  310: 	/* Map Grid of Australia */
  311: 	else if (strcasecmp(projname, "mga") == 0) {
  312: 		new_args = add_argument("ellps=gda94", nargs, args, &new_nargs, FALSE);
  313: 		new_args = add_argument("south", new_nargs, new_args, &new_nargs, TRUE);
  314: 		subst_argument_value("proj", "utm", new_nargs, new_args);
  315: 		mp = mpInitialise(new_nargs, new_args);
  316: 		for (i = 0; i < new_nargs; i++) {
  317: 			if (new_args[i])
  318: 				free(new_args[i]);
  319: 		}
  320: 		free(new_args);
  321: 	}
  322: 
  323: 	/* Universal Transverse Mercator */
  324: 	else if (strcasecmp(projname, "utm") == 0) {
  325: 		char buf[256];
  326: 		int is_north = (find_argument("south", nargs, args) < 0);
  327: 
  328: 		new_args = add_argument("x_0=500000.0", nargs, args, &new_nargs, FALSE);
  329: 
  330: 		if (is_north)
  331: 			new_args = add_argument("y_0=0.0", new_nargs, new_args, &new_nargs, TRUE);
  332: 		else
  333: 			new_args = add_argument("y_0=10000000.0", new_nargs, new_args, &new_nargs, TRUE);
  334: 
  335: 		snprintf(buf, sizeof(buf), "lon_0=%f", UTM_ZONE(new_nargs, new_args) * 6 - 183);
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
**Listed lines:** 401-405

**Function:** `mpInitialise()`

**Why it's thought to be a bug**

`mp->ellip_flat = 1 / (1 - sqrt(1 - ecc * ecc));` In PROJ, `es` is the eccentricity squared, and flattening is `f = 1 - sqrt(1 - es)`. The code squares `es` again and takes the reciprocal. With `es=0` (sphere) this gives `1/0 = inf`; for real ellipsoids it gives about 300 instead of 0.0033. Every later `e2` in `merc_init`/`tcc_init`/`lcc_init` is then wrong. Function: `mpInitialise()`.

**Current code** (`sjwlib/mapproj.c` lines 399–407)

```c
  399: 				mp->ellip_flat = 1 / atof(rf);
  400: 
  401: 			/* Get eccentricity */
  402: 			if (es != NULL) {
  403: 				double ecc = atof(es);
  404: 				mp->ellip_flat = 1 / (1 - sqrt(1 - ecc * ecc));
  405: 			}
  406: 		}
  407: 
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
**Listed lines:** 561-590, 1043-1051

**Function:** `add_argument()`

**Why it's thought to be a bug**

`add_argument()` ignores `freeOld` (that code is commented out), so each chained call leaks the previous pointer array. The utm branch never frees `new_args` (lines 342-346 are commented out), and `serov_init` leaks too. The allocation `malloc(sizeof(char *)*strlen(arg)+1)` (line 583) over-allocates by 8x. Function: `add_argument()`.

**Current code** (`sjwlib/mapproj.c` lines 559–592)

```c
  559: /* Add a new argument to the list.
  560:  */
  561: static char** add_argument(char *arg, int nargs, char *args[], int *new_nargs, int freeOld) {
  562: 	char **new_args;
  563: 	int i;
  564: 
  565: 	*new_nargs = nargs + 1;
  566: 	//new_args = c_alloc2d((*new_nargs), STRLEN);
  567: //	new_args = (char**) malloc(sizeof(char*) * (*new_nargs));
  568: //	new_args[0] = (char *) malloc(sizeof(char *) * strlen(arg) + 1);
  569: //	strcpy(new_args[0], arg);
  570: //
  571: //	for (i = 0; i < nargs; ++i)
  572: //		new_args[i + 1] = strdup(args[i]);
  573: //
  574: //	if (freeOld) {
  575: //		for (i = 0; i < nargs; ++i) {
  576: //			if (args[i])
  577: //				free(args[i]);
  578: //		}
  579: //		free(args);
  580: //	}
  581: 
  582:     new_args = (char**)malloc(sizeof(char*)*(size_t)(*new_nargs));
  583:     new_args[0] = (char *)malloc(sizeof(char *)*strlen(arg)+1);
  584:     strcpy(new_args[0], arg);
  585: 
  586:     for (i=0; i<nargs; ++i)
  587:        new_args[i+1] = args[i];
  588: 
  589: 	return new_args;
  590: }
  591: 
  592: /* Subsitute an argument.
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
**Listed lines:** 548-556

**Function:** `find_argument()`

**Why it's thought to be a bug**

`strncasecmp(tag, args[i], strlen(tag))` matches any argument that starts with the tag. So `get_argument_value("a", ...)`/`("b", ...)` (lines 382-383) match unrelated arguments such as `axis=...` or `b...`, and `"lat_0"`-style lookups also match longer keys. `atof` of the wrong value then silently sets the ellipsoid axes. Function: `find_argument()`.

**Current code** (`sjwlib/mapproj.c` lines 546–558)

```c
  546:  * @return index of tag in list or -1.
  547:  */
  548: static int find_argument(char *tag, int nargs, char *args[]) {
  549: 	int i;
  550: 
  551: 	for (i = 0; i < nargs; ++i) {
  552: 		if (strncasecmp(tag, args[i], strlen(tag)) == 0)
  553: 			return i;
  554: 	}
  555: 
  556: 	return -1;
  557: }
  558: 
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
