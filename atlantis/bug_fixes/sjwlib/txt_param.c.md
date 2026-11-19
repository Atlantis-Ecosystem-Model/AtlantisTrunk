# sjwlib/txt_param.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW2-13](#sjw2-13) | 674, 600, 92 | M/L | format string | `quit(msg)` / `warn(msg)` pass a message that contains the file's line text (`"...format bad on line>> %s"`) a… |  | [ ] |

---

## SJW2-13

**format string** · Conf M / Sev L  
**Found at (current code):** `txt_param.c` 674, `txt_param.c` 600, `txt_param.c` 92
**Lines at review (Oct 2026):** 670-677 (used throughout, e.g. 144-145, 166-167)

**Function:** `param_error()`

**Why it's thought to be a bug**

`quit(msg)` / `warn(msg)` pass a message that contains the file's line text (`"...format bad on line>> %s"`) as the format string. A `%` in a parameter file line (common in comments and units) then produces garbage output or a crash inside the error path. Function: `param_error()` (and the direct `warn(msg)` calls in each `*param()`).

**Code at issue** — `txt_param.c` line 674  
Search for: `quit(msg)`

```c
  672: {
  673:     if( prm_fatal )
  674→ 	quit(msg);
  675:     else
  676: 	warn(msg);
```

**Code at issue** — `txt_param.c` line 600  
Search for: `format bad on line>> %s"`

```c
  598: 	if( strncmp(label,line,(size_t)ll)==0 && isspace(line[ll]) ) {
  599: 	    if( sscanf(line+ll,"%ld",size) != 1 || *size < 0L ) {
  600→ 		snprintf(msg, sizeof(msg),"dparamarray: Parameter %s array size format bad on line>> %s",label,line);
  601: 		param_error(msg);
  602: 		return(-1);
```

**Code at issue** — `txt_param.c` line 92  
Search for: `param_error()`

```c
   90: 		    memory if *p == NULL
   91: 
   92→ 		    28 Oct 93: Added param_error() and
   93: 		    param_fatal() code.
   94: 
```

**Suggested fix (from review):** `quit("%s", msg)` / `warn("%s", msg)`

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
