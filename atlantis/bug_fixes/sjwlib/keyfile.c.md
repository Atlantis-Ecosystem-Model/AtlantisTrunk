# sjwlib/keyfile.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW2-17](#sjw2-17) | 393 | H/L | printf format/arg mismatch | `quit("readkeyprm_sarray: %s: Not enough values in array value %s\n", key);` has two `%s` but only one argumen… |  | [ ] |
| [SJW2-18](#sjw2-18) | 373, 377, 386 | M/L | buffer overrun / leak | `fscanf(fp,"%s",val)` into `char val[1024]` has no width limit. `*p` is always malloc'd, which ignores and lea… |  | [ ] |
| [SJW2-19](#sjw2-19) | 219-223, 232-235, 242-244 (and 295-319, 367-389) | M/L | wrong return value | The functions are documented to return non-zero on success, but return `-1` on error. When `keyprm_errfn` is `… |  | [ ] |

---

## SJW2-17

**printf format/arg mismatch** · Conf H / Sev L  
**Lines (current code):** 393  
**Lines at review (Oct 2026):** 393

**Function:** `readkeyprm_sarray()`

**Why it's thought to be a bug**

`quit("readkeyprm_sarray: %s: Not enough values in array value %s\n", key);` has two `%s` but only one argument, which is undefined behaviour (likely crash or garbage) in the error path. The message also wrongly says "fill remainder" in the comment above. Function: `readkeyprm_sarray()`.

**Current code** (`sjwlib/keyfile.c` lines 391–395)

```c
  391: 				else {
  392: 					/* not enough values - fill remainder with last value */
  393: 					quit("readkeyprm_sarray: %s: Not enough values in array value %s\n", key);
  394: 				}
  395: 			}
```

**Suggested fix (from review):** Drop the second `%s` or pass a value.

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

## SJW2-18

**buffer overrun / leak** · Conf M / Sev L  
**Lines (current code):** 373, 377, 386  
**Lines at review (Oct 2026):** 373, 377, 386

**Function:** `readkeyprm_sarray()`

**Why it's thought to be a bug**

`fscanf(fp,"%s",val)` into `char val[1024]` has no width limit. `*p` is always malloc'd, which ignores and leaks any existing array (unlike the d/i variants), and is leaked on the error returns. Function: `readkeyprm_sarray()`.

**Current code** (`sjwlib/keyfile.c` lines 371–388)

```c
  371: 
  372:     if (*size > 0) {
  373:       char val[1024];
  374: 
  375: /* allocate memory for the array if required */
  376: 
  377:        if((*p=(char **)malloc((size_t)(*size)*sizeof(char *))) == NULL ) {
  378:            keyprm_errfn("readkeyprm_sarray: %s: Can't read array value\n",key);
  379:            return(-1);
  380:        }
  381: 
  382:        /**** Now populate the array, if we run out of values, then pad
  383:         **** to end with the last value
  384:         ****/
  385:        for (i=0; i<(*size); ++i) {
  386: 			if(fscanf(fp,"%s",val) != 1 ) {
  387: 				if ( i == 0 ) {
  388: 					keyprm_errfn("readkeyprm_sarray: %s: Can't read any array values\n", key);
```

**Suggested fix (from review):** Use `"%1023s"`; free `*p` on error.

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

## SJW2-19

**wrong return value** · Conf M / Sev L  
**Lines (current code):** 219-223, 232-235, 242-244 (and 295-319, 367-389)  
**Lines at review (Oct 2026):** 219-223, 232-235, 242-244 (and 295-319, 367-389)

**Function:** `readkeyprm_darray()`

**Why it's thought to be a bug**

The functions are documented to return non-zero on success, but return `-1` on error. When `keyprm_errfn` is `warn` or a quiet function (datafile.c:1194, atUtilXML.c:1412), callers testing `if(!readkeyprm_darray(...))` treat failure as success and use an unset or partially filled array. Function: `readkeyprm_darray()`, `readkeyprm_iarray()`, `readkeyprm_sarray()`.

**Current code** (`sjwlib/keyfile.c` lines 217–246)

```c
  217: 	return(0);
  218: 
  219:     if( fscanf(fp,"%d",size) != 1  || *size < 0) {
  220: 	keyprm_errfn("readkeyprm_darray: Parameter %s array size format bad\n",
  221: 			key);
  222: 	return -1;
  223: 
  224:     }
  225: 
  226:     if (*size > 0) {
  227:        double val = 0.0;
  228:        double lastval = 0.0;
  229: 
  230: /* allocate memory for the array if required */
  231: 
  232:        if( *p==NULL && (*p=(double *)malloc((size_t)(*size)*sizeof(double))) == NULL ) {
  233:            keyprm_errfn("readkeyprm_darray: %s: Can't read array value\n",key);
  234:            return(-1);
  235:        }
  236: 
  237: /**** Now populate the array, if we run out of values, then pad
  238:  **** to end with the last value
  239:  ****/
  240:        for (i=0; i<(*size); ++i) {
  241: 	   if(fscanf(fp,"%lf",&val) != 1 ) {
  242: 	      if ( i == 0 ) {
  243: 	         keyprm_errfn("readkeyprm_darray: %s: Can't read any array values\n", key);
  244: 		 return -1;
  245: 	      }
  246: 	      else {
```

**Suggested fix (from review):** Return 0 on error.

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

`keyprm_errfn` is declared without an initialiser (sjwlib/warn.c:47). `skipToKeyStart`/`skipToKeyEnd` call `(*keyprm_errfn)` without a NULL check, so a missing key before any `set_keyprm_errfn()` call would jump through NULL.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
