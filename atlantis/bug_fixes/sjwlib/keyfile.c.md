# sjwlib/keyfile.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW2-17](#sjw2-17) | 393 | H/L | printf format/arg mismatch | `quit("readkeyprm_sarray: %s: Not enough values in array value %s\n", key);` has two `%s` but only one argumen… |  | [ ] |
| [SJW2-18](#sjw2-18) | 386, 373 | M/L | buffer overrun / leak | `fscanf(fp,"%s",val)` into `char val[1024]` has no width limit. `*p` is always malloc'd, which ignores and lea… |  | [ ] |
| [SJW2-19](#sjw2-19) | 296 | M/L | wrong return value | The functions are documented to return non-zero on success, but return `-1` on error. When `keyprm_errfn` is `… |  | [ ] |

---

## SJW2-17

**printf format/arg mismatch** · Conf H / Sev L  
**Found at (current code):** `keyfile.c` 393
**Lines at review (Oct 2026):** 393

**Function:** `readkeyprm_sarray()`

**Why it's thought to be a bug**

`quit("readkeyprm_sarray: %s: Not enough values in array value %s\n", key);` has two `%s` but only one argument, which is undefined behaviour (likely crash or garbage) in the error path. The message also wrongly says "fill remainder" in the comment above. Function: `readkeyprm_sarray()`.

**Code at issue** — `keyfile.c` line 393  
Search for: `quit("readkeyprm_sarray: %s: Not enough values in array value %s\n", key)`

```c
  391: 				else {
  392: 					/* not enough values - fill remainder with last value */
  393→ 					quit("readkeyprm_sarray: %s: Not enough values in array value %s\n", key);
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
**Found at (current code):** `keyfile.c` 386, `keyfile.c` 373
**Lines at review (Oct 2026):** 373, 377, 386

**Function:** `readkeyprm_sarray()`

**Why it's thought to be a bug**

`fscanf(fp,"%s",val)` into `char val[1024]` has no width limit. `*p` is always malloc'd, which ignores and leaks any existing array (unlike the d/i variants), and is leaked on the error returns. Function: `readkeyprm_sarray()`.

**Code at issue** — `keyfile.c` line 386  
Search for: `fscanf(fp,"%s",val)`

```c
  384:         ****/
  385:        for (i=0; i<(*size); ++i) {
  386→ 			if(fscanf(fp,"%s",val) != 1 ) {
  387: 				if ( i == 0 ) {
  388: 					keyprm_errfn("readkeyprm_sarray: %s: Can't read any array values\n", key);
```

**Code at issue** — `keyfile.c` line 373  
Search for: `char val[1024]`

```c
  371: 
  372:     if (*size > 0) {
  373→       char val[1024];
  374: 
  375: /* allocate memory for the array if required */
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
**Found at (current code):** `keyfile.c` 296
**Lines at review (Oct 2026):** 219-223, 232-235, 242-244 (and 295-319, 367-389)

**Function:** `readkeyprm_darray()`

**Why it's thought to be a bug**

The functions are documented to return non-zero on success, but return `-1` on error. When `keyprm_errfn` is `warn` or a quiet function (datafile.c:1194, atUtilXML.c:1412), callers testing `if(!readkeyprm_darray(...))` treat failure as success and use an unset or partially filled array. Function: `readkeyprm_darray()`, `readkeyprm_iarray()`, `readkeyprm_sarray()`.

**Code at issue** — `keyfile.c` line 296 (located by name only; check this is the right place)  
Search for: `keyprm_errfn("readkeyprm_iarray: Parameter %s array size format bad\n",`

```c
  294: 
  295:     if( fscanf(fp,"%d",size) != 1  || *size < 0) {
  296→ 	keyprm_errfn("readkeyprm_iarray: Parameter %s array size format bad\n",
  297: 			key);
  298: 	return -1;
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
