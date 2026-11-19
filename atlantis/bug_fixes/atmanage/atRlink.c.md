# atmanage/atRlink.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MNGIO-26](#mngio-26) | 85, 173 | H/L | buffer overrun / leak | `malloc(sizeof(char*) * K + 1)` allocates K pointers plus 1 byte, so `names[K]` is out of bounds (operator pre… |  | [ ] |

---

## MNGIO-26

**buffer overrun / leak** · Conf H / Sev L  
**Found at (current code):** `atRlink.c` 85, `atRlink.c` 173
**Lines at review (Oct 2026):** 81-89

**Function:** `R_Linkage_Start()`

**Why it's thought to be a bug**

`malloc(sizeof(char*) * K + 1)` allocates K pointers plus 1 byte, so `names[K]` is out of bounds (operator precedence). `malloc(5)` followed by `strcpy(..., groupCode)` overflows for codes over 4 characters (groupCode is char[20], atFunctGroup.h:80). names is never freed. This is latent: atmanage/Makefile.am:11 lists atRlink.h, not atRlink.c, so this file is not compiled. Its live duplicate is MNGIO-40. Function: `R_Linkage_Start()`.

**Code at issue** — `atRlink.c` line 85  
Search for: `strcpy(`

```c
   83:     for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
   84:         names[sp] = (char*) malloc(sizeof(char) * 5);
   85→         strcpy(names[sp], FunctGroupArray[sp].groupCode);
   86:     }
   87: 
```

**Code at issue** — `atRlink.c` line 173  
Search for: `, groupCode)`

```c
  171: 	exec_r(test);
  172: 
  173→ 	//snprintf(test, sizeof(test), "print(biomassIndex[[\"%s\"]])", groupCode);
  174: 	//printf("%s\n", test);
  175: 	//exec_r(test);
```

**Suggested fix (from review):** `malloc(sizeof(char*)*(K+1))`, `strdup(groupCode)`, then free. Or delete the dead file.

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

atRlink.c duplicates most of atRlinkRedus.c. It is not compiled now, but adding it to the build would cause duplicate-symbol link errors.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
