# sjwlib/txt_param.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW2-13](#sjw2-13) | 670-677 (used throughout, e.g. 144-145, 166-167) | M/L | format string | `quit(msg)` / `warn(msg)` pass a message that contains the file's line text (`"...format bad on line>> %s"`) a… |  | [ ] |

---

## SJW2-13

**format string** · Conf M / Sev L  
**Lines (current code):** 670-677 (used throughout, e.g. 144-145, 166-167)  
**Lines at review (Oct 2026):** 670-677 (used throughout, e.g. 144-145, 166-167)

**Function:** `param_error()`

**Why it's thought to be a bug**

`quit(msg)` / `warn(msg)` pass a message that contains the file's line text (`"...format bad on line>> %s"`) as the format string. A `%` in a parameter file line (common in comments and units) then produces garbage output or a crash inside the error path. Function: `param_error()` (and the direct `warn(msg)` calls in each `*param()`).

**Current code** (`sjwlib/txt_param.c` lines 668–678)

```c
  668: }
  669: 
  670: void
  671: param_error(char *msg)
  672: {
  673:     if( prm_fatal )
  674: 	quit(msg);
  675:     else
  676: 	warn(msg);
  677: }
  678: 
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
