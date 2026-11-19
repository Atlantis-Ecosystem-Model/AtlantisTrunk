# atmanage/atRlinkRedus.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MNGIO-40](#mngio-40) | 39, 43 | H/M | buffer overrun / leak | Same bug as MNGIO-26, in compiled code: `malloc(sizeof(char*) * bm->K_num_tot_sp + 1)` leaves `names[K]` out o… |  | [ ] |
| [MNGIO-41](#mngio-41) | 88 | H/L | uninitialised variable | `int error;` is returned uninitialised if the parsed expression has length 0, because R_tryEval is never calle… |  | [ ] |

---

## MNGIO-40

**buffer overrun / leak** · Conf H / Sev M  
**Found at (current code):** `atRlinkRedus.c` 39, `atRlinkRedus.c` 43
**Lines at review (Oct 2026):** 39-47

**Function:** `Redus_Linkage_Start()`

**Why it's thought to be a bug**

Same bug as MNGIO-26, in compiled code: `malloc(sizeof(char*) * bm->K_num_tot_sp + 1)` leaves `names[K]` out of bounds, `malloc(5)` with `strcpy(groupCode)` overflows for codes over 4 characters, and names is never freed. Function: `Redus_Linkage_Start()`.

**Code at issue** — `atRlinkRedus.c` line 39  
Search for: `malloc(sizeof(char*) * bm->K_num_tot_sp + 1)`

```c
   37:     printf("Redus_Linkage_Start: Preparing R data structure for %d species\n", bm->K_num_tot_sp);
   38:     numyr = (int)floor(ROUNDGUARD + ((bm->tstop - bm->tstart)/(86400.0*365.0)));
   39→     names = (char**) malloc(sizeof(char*) * bm->K_num_tot_sp + 1);
   40: 
   41:     for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
```

**Code at issue** — `atRlinkRedus.c` line 43 (closest match to the quoted code; wording differs slightly)  
Search for: `strcpy(names[sp], FunctGroupArray[sp].groupCode);`

```c
   41:     for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
   42:         names[sp] = (char*) malloc(sizeof(char) * 5);
   43→         strcpy(names[sp], FunctGroupArray[sp].groupCode);
   44:     }
   45: 
```

**Suggested fix (from review):** `malloc(sizeof(char*)*(K+1))`, `strdup`, then free after use.

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

## MNGIO-41

**uninitialised variable** · Conf H / Sev L  
**Found at (current code):** `atRlinkRedus.c` 88
**Lines at review (Oct 2026):** 61, 87-93

**Function:** `exec_r()`

**Why it's thought to be a bug**

`int error;` is returned uninitialised if the parsed expression has length 0, because R_tryEval is never called. Function: `exec_r()`.

**Code at issue** — `atRlinkRedus.c` line 88 (the line the review pointed to, found by its text)  
Search for: `R_tryEval(VECTOR_ELT(expr, i), R_GlobalEnv, &error);`

```c
   86:   int i;
   87:   for (i=0; i<LENGTH(expr); i++) {
   88→     R_tryEval(VECTOR_ELT(expr, i), R_GlobalEnv, &error);
   89:   }
   90:  
```

**Suggested fix (from review):** `int error = 0;`.

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

atRlinkRedus.c:366 `REDUS_management` averages calcTrackedMort over `numCohorts` only, not numCohortsXnumGenes. It may be intentional.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
