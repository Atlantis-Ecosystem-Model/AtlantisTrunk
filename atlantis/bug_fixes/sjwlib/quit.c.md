# sjwlib/quit.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW3-13](#sjw3-13) | 41 | H/M | wrong return value | `quit()` is the fatal-error handler used throughout Atlantis but ends with `exit(0);`, i.e. reports SUCCESS to… |  | [ ] |

---

## SJW3-13

**wrong return value** · Conf H / Sev M  
**Lines (current code):** 41  
**Lines at review (Oct 2026):** 41

**Function:** `quit()`

**Why it's thought to be a bug**

`quit()` is the fatal-error handler used throughout Atlantis but ends with `exit(0);`, i.e. reports SUCCESS to the shell. Batch scripts, R/Python wrappers, and HPC job managers cannot detect that a run aborted on an error (e.g. missing parameter, NaN trap). Function: `quit()`.

**Current code** (`sjwlib/quit.c` lines 39–43)

```c
   39:     va_end(args);
   40:     fprintf(stderr,"\n\n");
   41: 	exit(0);
   42: }
   43: 
```

**Suggested fix (from review):** `exit(EXIT_FAILURE);` (or `exit(1)`).

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
