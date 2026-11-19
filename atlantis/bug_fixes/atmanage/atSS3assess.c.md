# atmanage/atSS3assess.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1), plus 1 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MNG-31](#mng-31) ⭐ | 176, 200 | H/H | invalid free | `ss3_abs = realpath(bm->ss3Name, buf); ... free(ss3_abs);` — realpath with a non-NULL buffer returns `buf`, a … |  | [ ] |
| [MNG-32](#mng-32) | 165-167, 176 | M/M | buffer overrun | `char buf[1024]` passed to `realpath()`, which may write up to PATH_MAX bytes (4096 on Linux). Long paths over… |  | [ ] |
| [MNG-33](#mng-33) | 190, 199 | M/L | unchecked return | `chdir(run_dir)` return not checked — if it fails ss3 runs in the model directory, overwriting/reading the wro… |  | [ ] |

---

## MNG-31

**invalid free** · Conf H / Sev H ⭐ priority  
**Listed lines:** 176, 200

**Function:** `Run_SS3()`

**Why it's thought to be a bug**

`ss3_abs = realpath(bm->ss3Name, buf); ... free(ss3_abs);` — realpath with a non-NULL buffer returns `buf`, a stack array, so free() of a stack pointer aborts ("pointer being freed was not allocated") on every SS3 run. Function: `Run_SS3()`.

**Current code** (`atmanage/atSS3assess.c` lines 174–202)

```c
  174: 
  175:     // resolve link or relative path to an absolute path
  176:     ss3_abs = realpath(bm->ss3Name, buf);
  177:     if (ss3_abs) {
  178:         // Real path found
  179:     } else {
  180:         // print explanatory error message
  181:         int errc = errno;
  182:         char *errStr = strerror(errno);
  183:         quit("Failed to resolve: %s - Error: %d %s\n", bm->ss3Name, errc, errStr);
  184:     }
  185: 
  186:     // build command string to pass to system
  187:     snprintf(ss3_cmd, sizeof(ss3_cmd), "%s -nohess > mse.junk", ss3_abs);
  188: 
  189:     // change to rundir and execute the ss3 binary
  190:     chdir(run_dir);
  191:     printf("Running %s in dir %s\n", ss3_cmd, run_dir);
  192:     int ret = system(ss3_cmd);
  193: 
  194:     if (ret != 0) {
  195:         printf("ss3 return code: %d - possibly an error?\n", ret);
  196:     }
  197: 
  198:     // go back to previous directory
  199:     chdir(cwd);
  200:     // Nov 2026: ss3_abs points to the local buffer buf (realpath was given buf) so must not be freed
  201:     //free(cwd);  // TODO: This correct or cause dump?
  202: 
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
**Listed lines:** 165-167, 176

**Function:** `Run_SS3()`

**Why it's thought to be a bug**

`char buf[1024]` passed to `realpath()`, which may write up to PATH_MAX bytes (4096 on Linux). Long paths overflow the stack buffer. Function: `Run_SS3()`.

**Current code** (`atmanage/atSS3assess.c` lines 174–189) — code appears to have moved +11 lines since the review

```c
  174: 
  175:     // resolve link or relative path to an absolute path
  176:     ss3_abs = realpath(bm->ss3Name, buf);
  177:     if (ss3_abs) {
  178:         // Real path found
  179:     } else {
  180:         // print explanatory error message
  181:         int errc = errno;
  182:         char *errStr = strerror(errno);
  183:         quit("Failed to resolve: %s - Error: %d %s\n", bm->ss3Name, errc, errStr);
  184:     }
  185: 
  186:     // build command string to pass to system
  187:     snprintf(ss3_cmd, sizeof(ss3_cmd), "%s -nohess > mse.junk", ss3_abs);
  188: 
  189:     // change to rundir and execute the ss3 binary
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
**Listed lines:** 190, 199

**Function:** `Run_SS3()`

**Why it's thought to be a bug**

`chdir(run_dir)` return not checked — if it fails ss3 runs in the model directory, overwriting/reading the wrong files, and results are then read from run_dir. Function: `Run_SS3()`.

**Current code** (`atmanage/atSS3assess.c` lines 188–201)

```c
  188: 
  189:     // change to rundir and execute the ss3 binary
  190:     chdir(run_dir);
  191:     printf("Running %s in dir %s\n", ss3_cmd, run_dir);
  192:     int ret = system(ss3_cmd);
  193: 
  194:     if (ret != 0) {
  195:         printf("ss3 return code: %d - possibly an error?\n", ret);
  196:     }
  197: 
  198:     // go back to previous directory
  199:     chdir(cwd);
  200:     // Nov 2026: ss3_abs points to the local buffer buf (realpath was given buf) so must not be freed
  201:     //free(cwd);  // TODO: This correct or cause dump?
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
