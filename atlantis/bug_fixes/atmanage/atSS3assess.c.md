# atmanage/atSS3assess.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MNG-31](#mng-31) ⭐ | 176 | H/H | invalid free | `ss3_abs = realpath(bm->ss3Name, buf); ... free(ss3_abs);` — realpath with a non-NULL buffer returns `buf`, a … |  | [ ] |
| [MNG-32](#mng-32) | 176 | M/M | buffer overrun | `char buf[1024]` passed to `realpath()`, which may write up to PATH_MAX bytes (4096 on Linux). Long paths over… |  | [ ] |
| [MNG-33](#mng-33) | 190 | M/L | unchecked return | `chdir(run_dir)` return not checked — if it fails ss3 runs in the model directory, overwriting/reading the wro… |  | [ ] |

---

## MNG-31

**invalid free** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atSS3assess.c` 176
**Lines at review (Oct 2026):** 176, 200

**Function:** `Run_SS3()`

**Why it's thought to be a bug**

`ss3_abs = realpath(bm->ss3Name, buf); ... free(ss3_abs);` — realpath with a non-NULL buffer returns `buf`, a stack array, so free() of a stack pointer aborts ("pointer being freed was not allocated") on every SS3 run. Function: `Run_SS3()`.

**Code at issue** — `atSS3assess.c` line 176  
Search for: `ss3_abs = realpath(bm->ss3Name, buf)`

```c
  174: 
  175:     // resolve link or relative path to an absolute path
  176→     ss3_abs = realpath(bm->ss3Name, buf);
  177:     if (ss3_abs) {
  178:         // Real path found
```

**Suggested fix (from review):** Remove `free(ss3_abs)` (or call `realpath(..., NULL)` and free that).

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

## MNG-32

**buffer overrun** · Conf M / Sev M  
**Found at (current code):** `atSS3assess.c` 176
**Lines at review (Oct 2026):** 165-167, 176

**Function:** `Run_SS3()`

**Why it's thought to be a bug**

`char buf[1024]` passed to `realpath()`, which may write up to PATH_MAX bytes (4096 on Linux). Long paths overflow the stack buffer. Function: `Run_SS3()`.

**Code at issue** — `atSS3assess.c` line 176 (the line the review pointed to, found by its text)  
Search for: `ss3_abs = realpath(bm->ss3Name, buf);`

```c
  174: 
  175:     // resolve link or relative path to an absolute path
  176→     ss3_abs = realpath(bm->ss3Name, buf);
  177:     if (ss3_abs) {
  178:         // Real path found
```

**Suggested fix (from review):** Use `realpath(bm->ss3Name, NULL)` and free the result.

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

## MNG-33

**unchecked return** · Conf M / Sev L  
**Found at (current code):** `atSS3assess.c` 190
**Lines at review (Oct 2026):** 190, 199

**Function:** `Run_SS3()`

**Why it's thought to be a bug**

`chdir(run_dir)` return not checked — if it fails ss3 runs in the model directory, overwriting/reading the wrong files, and results are then read from run_dir. Function: `Run_SS3()`.

**Code at issue** — `atSS3assess.c` line 190  
Search for: `chdir(run_dir)`

```c
  188: 
  189:     // change to rundir and execute the ss3 binary
  190→     chdir(run_dir);
  191:     printf("Running %s in dir %s\n", ss3_cmd, run_dir);
  192:     int ret = system(ss3_cmd);
```

**Suggested fix (from review):** Check return and quit on failure.

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

atSS3assess.c:127: RBC_by_year[year] is checked even when the assessment already failed (value may be stale from a prior run), only matters for the log message.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
