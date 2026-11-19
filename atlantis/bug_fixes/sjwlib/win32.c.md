# sjwlib/win32.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW3-11](#sjw3-11) | 50-58 | H/M | wrong logic / unsigned underflow | `if (n > (int) min(strlen(s1),strlen(s2)) ) return(-1); for (i=0;i<=(n-1);i++)` — (a) returns "not equal" when… |  | [ ] |
| [SJW3-12](#sjw3-12) | 32-42 | M/L | non-standard return | `strcasecmp` returns -1 for any mismatch (never >0), so it is unusable for ordering (e.g. qsort/bsearch compar… |  | [ ] |

## Reviewer notes for this file

(Windows/MSVC builds only.)

---

## SJW3-11

**wrong logic / unsigned underflow** · Conf H / Sev M  
**Lines (current code):** 50-58  
**Lines at review (Oct 2026):** 50-58

**Function:** `strncasecmp()`

**Why it's thought to be a bug**

`if (n > (int) min(strlen(s1),strlen(s2)) ) return(-1); for (i=0;i<=(n-1);i++)` — (a) returns "not equal" whenever `n` exceeds either string length, so `strncasecmp("abc","ABC",10)` returns -1 instead of 0 (the common idiom of passing a buffer size breaks on Windows only); (b) with `n==0`, `n-1` underflows to SIZE_MAX (size_t) and the loop reads past both strings. Function: `strncasecmp()`.

**Current code** (`sjwlib/win32.c` lines 48–60)

```c
   48: 
   49: 
   50: 	if (n > (int) min(strlen(s1),strlen(s2)) ) return(-1);
   51: 
   52: 	for (i=0;i<=(n-1);i++) {
   53: 		if (tolower(s1[i]) == tolower(s2[i])) {
   54: 		}
   55: 		else {
   56: 			return(-1);
   57: 		}
   58: 	}
   59: 	return(0);
   60: }
```

**Suggested fix (from review):** Loop `for(i=0;i<n;i++){ if(tolower(s1[i])!=tolower(s2[i])) return diff; if(!s1[i]) return 0; } return 0;`

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

## SJW3-12

**non-standard return** · Conf M / Sev L  
**Lines (current code):** 32-42  
**Lines at review (Oct 2026):** 32-42

**Function:** `strcasecmp()`

**Why it's thought to be a bug**

`strcasecmp` returns -1 for any mismatch (never >0), so it is unusable for ordering (e.g. qsort/bsearch comparators) on Windows; equality tests are fine. Function: `strcasecmp()`.

**Current code** (`sjwlib/win32.c` lines 30–44)

```c
   30: 	n2 = (int)strlen(s2);
   31: 
   32: 	if (n1 != n2) return(-1);
   33: 
   34: 	for (i=0;(s1[i] != '\0');i++) {
   35: 		if (tolower(s1[i]) == tolower(s2[i])) {
   36: 		}
   37: 		else {
   38: 			return(-1);
   39: 		}
   40: 	}
   41: 
   42: 	return(0);
   43: }
   44: 
```

**Suggested fix (from review):** Return `tolower(s1[i]) - tolower(s2[i])`.

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
