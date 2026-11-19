# atmanage/atRlinkRedus.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MNGIO-40](#mngio-40) | 39-47 | H/M | buffer overrun / leak | Same bug as MNGIO-26, in compiled code: `malloc(sizeof(char*) * bm->K_num_tot_sp + 1)` leaves `names[K]` out o… |  | [ ] |
| [MNGIO-41](#mngio-41) | 61, 87-93 | H/L | uninitialised variable | `int error;` is returned uninitialised if the parsed expression has length 0, because R_tryEval is never calle… |  | [ ] |

---

## MNGIO-40

**buffer overrun / leak** · Conf H / Sev M  
**Lines (current code):** 39-47  
**Lines at review (Oct 2026):** 39-47

**Function:** `Redus_Linkage_Start()`

**Why it's thought to be a bug**

Same bug as MNGIO-26, in compiled code: `malloc(sizeof(char*) * bm->K_num_tot_sp + 1)` leaves `names[K]` out of bounds, `malloc(5)` with `strcpy(groupCode)` overflows for codes over 4 characters, and names is never freed. Function: `Redus_Linkage_Start()`.

**Current code** (`atmanage/atRlinkRedus.c` lines 37–49)

```c
   37:     printf("Redus_Linkage_Start: Preparing R data structure for %d species\n", bm->K_num_tot_sp);
   38:     numyr = (int)floor(ROUNDGUARD + ((bm->tstop - bm->tstart)/(86400.0*365.0)));
   39:     names = (char**) malloc(sizeof(char*) * bm->K_num_tot_sp + 1);
   40: 
   41:     for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
   42:         names[sp] = (char*) malloc(sizeof(char) * 5);
   43:         strcpy(names[sp], FunctGroupArray[sp].groupCode);
   44:     }
   45: 
   46:     names[bm->K_num_tot_sp] = (char*) malloc(sizeof(char) * 5);
   47:     strcpy(names[sp], "");
   48: 
   49:     printf("Redus_Linkage_Start: Now creating R objects\n");
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
**Lines (current code):** 61, 87-93  
**Lines at review (Oct 2026):** 61, 87-93

**Function:** `exec_r()`

**Why it's thought to be a bug**

`int error;` is returned uninitialised if the parsed expression has length 0, because R_tryEval is never called. Function: `exec_r()`.

**Current code** (`atmanage/atRlinkRedus.c` lines 59–95)

```c
   59: int exec_r(const char* str)
   60: {
   61:   int error;
   62:   SEXP code, expr = R_NilValue;
   63:   ParseStatus status;
   64:  
   65:   PROTECT(code = Rf_allocVector(STRSXP, 1));
   66:   SET_STRING_ELT(code, 0, Rf_mkChar(str));
   67:  
   68:   expr = PROTECT(R_ParseVector(code, -1, &status, R_NilValue));
   69:  
   70:   switch (status) {
   71:   case PARSE_OK:
   72:     break;
   73:   case PARSE_INCOMPLETE:
   74:     fprintf(stderr, "Expression incomplete! Missing brackets?\n");
   75:     UNPROTECT(2);
   76:     return -1;
   77:   case PARSE_ERROR:
   78:     fprintf(stderr, "Syntax error!\n");
   79:     UNPROTECT(2);
   80:     return -1;
   81:   default:
   82:     UNPROTECT(2);
   83:     return -1;
   84:   }
   85:    
   86:   int i;
   87:   for (i=0; i<LENGTH(expr); i++) {
   88:     R_tryEval(VECTOR_ELT(expr, i), R_GlobalEnv, &error);
   89:   }
   90:  
   91:   UNPROTECT(2);
   92:  
   93:   return error;
   94: }
   95: 
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
