# sjwlib/quit.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW3-13](#sjw3-13) | 41 | H/M | wrong return value | `quit()` is the fatal-error handler used throughout Atlantis but ends with `exit(0);`, i.e. reports SUCCESS to… |  | [ ] |

---

## SJW3-13

**wrong return value** · Conf H / Sev M  
**Found at (current code):** `quit.c` 41
**Lines at review (Oct 2026):** 41

**Function:** `quit()`

**Why it's thought to be a bug**

`quit()` is the fatal-error handler used throughout Atlantis but ends with `exit(0);`, i.e. reports SUCCESS to the shell. Batch scripts, R/Python wrappers, and HPC job managers cannot detect that a run aborted on an error (e.g. missing parameter, NaN trap). Function: `quit()`.

**Code at issue** — `quit.c` line 41  
Search for: `exit(0)`

```c
   39:     va_end(args);
   40:     fprintf(stderr,"\n\n");
   41→ 	exit(0);
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
