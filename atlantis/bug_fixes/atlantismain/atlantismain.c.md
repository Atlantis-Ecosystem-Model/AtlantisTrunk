# atlantismain/atlantismain.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **8 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 3 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MAIN-01](#main-01) | 2432, 1531 | H/M | NULL dereference / mismatched alloc-free | `free2d(dvol); free3d(dtr);` run every time, but `dvol`/`dtr` are only allocated inside `if (do_physics)` (atl… |  | [ ] |
| [MAIN-02](#main-02) | 1139 | H/L | NULL dereference | `pdest = strstr(bm->ncOfname, endname); totname = strlen(ncOfname) - strlen(pdest);` and then `*pdest = '\0'` … |  | [ ] |
| [MAIN-03](#main-03) | 962, 930 | M/L | buffer overrun | `strcat(commands, commandTempStr)` keeps appending every argv into `char commands[STRLEN]` (2000) with no boun… |  | [ ] |
| [MAIN-04](#main-04) | 1170 | H/L | always-false condition | `!bm->forceIfname \|\| !bm->fishprmIfname \|\| !bm->assessprmIfname \|\| !bm->econprmIfname` test char-array address… |  | [ ] |
| [MAIN-05](#main-05) | 1180 | H/L | wrong label / copy-paste (log text) | The `bm->params` string prints `-s %s -g %s -c %s -d %s` with the summary, PROD, TOTCATCH and CATCH output fil… |  | [ ] |
| [MAIN-06](#main-06) | 101 | H/L | always-false condition | `not_tracking_flux = totfluxout; if (totfluxout != not_tracking_flux) quit(...)` assigns and then compares the… |  | [ ] |
| [MAIN-07](#main-07) | 978, 1083 | M/L | missing string terminator | `strncpy(bm->X, *++argv, BMSLEN)` into BMSLEN-sized fields leaves no terminator when the argument is 512 chara… |  | [ ] |
| [MAIN-08](#main-08) | 1318 | H/L | memory leak | `bm->init_pH` (allocated at line 1179 with alloc2d) is never freed in `modelshutdown()`. Every other array in … |  | [ ] |

---

## MAIN-01

**NULL dereference / mismatched alloc-free** · Conf H / Sev M  
**Found at (current code):** `atlantismain.c` 2432, `atlantismain.c` 1531
**Lines at review (Oct 2026):** 2293-2294

**Function:** `modelshutdown()`

**Why it's thought to be a bug**

`free2d(dvol); free3d(dtr);` run every time, but `dvol`/`dtr` are only allocated inside `if (do_physics)` (atlantismain.c:1534-1540). `do_physics` is only set by the `-p` flag (line 1008), so without `-p` they are NULL globals. `d_free2d` reads `pp[0]` before anything else (sjwlib/memory.c:648), and `d_free3d` reads `ppp[0][0]` (memory.c:721), so shutdown crashes. (If `flag_skip_phys` clears do_physics after allocation, the free is fine.) Function: `modelshutdown()`.

**Code at issue** — `atlantismain.c` line 2432  
Search for: `free2d(dvol);`

```c
 2430: 
 2431: 	i_free2d(bm->checkedalready);
 2432→ 	free2d(dvol);
 2433: 	free3d(dtr);
 2434: 
```

**Code at issue** — `atlantismain.c` line 1531  
Search for: `if (do_physics)`

```c
 1529: 	bm->QofY = 0;
 1530: 
 1531→ 	if (do_physics) {
 1532: 		initPhysics(bm);
 1533: 
```

**Suggested fix (from review):** `if (dvol) free2d(dvol); if (dtr) free3d(dtr);`

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

## MAIN-02

**NULL dereference** · Conf H / Sev L  
**Found at (current code):** `atlantismain.c` 1139
**Lines at review (Oct 2026):** 994-1000

**Function:** `setupMSEBoxModel()`

**Why it's thought to be a bug**

`pdest = strstr(bm->ncOfname, endname); totname = strlen(ncOfname) - strlen(pdest);` and then `*pdest = '\0'` on the second strstr. If the `-o` name does not contain ".nc", pdest is NULL and the program segfaults when it should print a usage error. Function: `setupMSEBoxModel()`.

**Code at issue** — `atlantismain.c` line 1139  
Search for: `*pdest = '\0'`

```c
 1137: 	strcpy(tempStr, bm->ncOfname);
 1138: 	pdest = strstr(tempStr, endname);
 1139→ 	*pdest = '\0';
 1140: 
 1141: 	strncpy(bm->ncOSumfname, bm->ncOfname, (size_t)totname);
```

**Suggested fix (from review):** Check `pdest == NULL` and call `quit("output file name must end in .nc")`

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

## MAIN-03

**buffer overrun** · Conf M / Sev L  
**Found at (current code):** `atlantismain.c` 962, `atlantismain.c` 930
**Lines at review (Oct 2026):** 823-830

**Function:** `setupMSEBoxModel()`

**Why it's thought to be a bug**

`strcat(commands, commandTempStr)` keeps appending every argv into `char commands[STRLEN]` (2147) with no bound check. Atlantis takes 16+ arguments, often long absolute paths, so the total can go past 2000 bytes and overflow the stack. Function: `setupMSEBoxModel()`.

**Code at issue** — `atlantismain.c` line 962  
Search for: `strcat(commands, commandTempStr)`

```c
  960: 	for (i = 1; i < argc-1; i++) {
  961: 		snprintf(commandTempStr, sizeof(commandTempStr), " %s", argv[i]);
  962→ 		strcat(commands, commandTempStr);
  963: 		//(void) snprintf(commands, sizeof(commands), "%s %s", commands,
  964: 	}
```

**Code at issue** — `atlantismain.c` line 930  
Search for: `char commands[STRLEN]`

```c
  928: 	char *pdest;
  929: 	char tempStr[STRLEN];
  930→ 	char commands[STRLEN];
  931: 	char commandTempStr[STRLEN];
  932: 
```

**Suggested fix (from review):** Use `strncat(commands, commandTempStr, sizeof(commands)-strlen(commands)-1)`

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

## MAIN-04

**always-false condition** · Conf H / Sev L  
**Found at (current code):** `atlantismain.c` 1170
**Lines at review (Oct 2026):** 1031

**Function:** `setupMSEBoxModel()`

**Why it's thought to be a bug**

`!bm->forceIfname || !bm->fishprmIfname || !bm->assessprmIfname || !bm->econprmIfname` test char-array addresses, which are never NULL, so these parts of the "required arguments" check never fire. The other entries correctly test `[0]`. (forceIfname is already checked at line 986, so only the fish/assess/econ checks are dead. Those files are optional, so do not just add `[0]` to them.) Function: `setupMSEBoxModel()`.

**Code at issue** — `atlantismain.c` line 1170  
Search for: `!bm->forceIfname || !bm->fishprmIfname || !bm->assessprmIfname || !bm->econprmIfname`

```c
 1168: 
 1169: 	/* Check that all required arguments were present */
 1170→ 	if (!bm->ncIfname[0] || !bm->ncOfname[0] || !bm->runprmIfname[0] || !bm->forceIfname || !bm->fishprmIfname || !bm->assessprmIfname || !bm->econprmIfname
 1171: 			|| !bm->ncODetFishfname[0] || !bm->ncOFishfname[0] || !bm->ncOSumfname[0] || !bm->ncOPCfname[0] || bm->ncIfdump < 0)
 1172: 		Util_Usage(1);
```

**Suggested fix (from review):** Remove the dead terms, or use `[0]` only for files that really are required

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

## MAIN-05

**wrong label / copy-paste (log text)** · Conf H / Sev L  
**Found at (current code):** `atlantismain.c` 1180
**Lines at review (Oct 2026):** 1036

**Function:** `setupMSEBoxModel()`

**Why it's thought to be a bug**

The `bm->params` string prints `-s %s -g %s -c %s -d %s` with the summary, PROD, TOTCATCH and CATCH output file names. These are old flag meanings: `-s` is now the functional-group file, `-c` is cabspot, `-d` is the destination folder, and `-g` is not a flag. The params string written into output metadata is therefore wrong and cannot be re-run. Function: `setupMSEBoxModel()`.

**Code at issue** — `atlantismain.c` line 1180  
Search for: `params`

```c
 1178: 		char s[STRLEN];
 1179: 		snprintf(s, sizeof(s), " -b %s", bm->bioprmIfname);
 1180→ 		strcat(bm->params, s);
 1181: 		snprintf(s, sizeof(s), " -h %s", bm->fishprmIfname);
 1182: 		strcat(bm->params, s);
```

**Suggested fix (from review):** Rebuild params from the current flags (-s functionGroupIfname, -q fisheryIfname, -d destFolder, etc.)

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

## MAIN-06

**always-false condition** · Conf H / Sev L  
**Found at (current code):** `atlantismain.c` 101
**Lines at review (Oct 2026):** 2085-2090

**Function:** `AllocateArrayMemory()`

**Why it's thought to be a bug**

`not_tracking_flux = totfluxout; if (totfluxout != not_tracking_flux) quit(...)` assigns and then compares the same value, so the sanity check can never fire. Function: `AllocateArrayMemory()`.

**Code at issue** — `atlantismain.c` line 101  
Search for: `AllocateArrayMemory()`

```c
   99: 
  100:  01-03-2010 Bec Gorton
  101→  Added code in AllocateArrayMemory() to allocate memory for the bm->connectivityMatrix and
  102:  bm->larvaldistrib arrays.
  103: 
```

**Suggested fix (from review):** Remove the check, or compare against the intended independent constant

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

## MAIN-07

**missing string terminator** · Conf M / Sev L  
**Found at (current code):** `atlantismain.c` 978, `atlantismain.c` 1083
**Lines at review (Oct 2026):** 842-942, 967

**Function:** `setupMSEBoxModel()`

**Why it's thought to be a bug**

`strncpy(bm->X, *++argv, BMSLEN)` into BMSLEN-sized fields leaves no terminator when the argument is 512 characters or longer. The memset at 808 does not help because all 512 bytes get written. The `-d`/`-t` cases then `strcat(FOLDER_SEP)`, which can overflow. Function: `setupMSEBoxModel()`.

**Code at issue** — `atlantismain.c` line 978 (closest match to the quoted code; wording differs slightly)  
Search for: `strncpy(bm->ncIfname, *++argv, BMSLEN);`

```c
  976: 				if (argc < 3)
  977: 					Util_Usage(1);
  978→ 				strncpy(bm->ncIfname, *++argv, BMSLEN);
  979: 				bm->ncIfdump = atoi(*++argv);
  980: 				argc -= 2;
```

**Code at issue** — `atlantismain.c` line 1083 (closest match to the quoted code; wording differs slightly)  
Search for: `strcat(bm->destFolder, FOLDER_SEP);`

```c
 1081: 				strncpy(bm->destFolder, *++argv,BMSLEN);
 1082: 				trim(bm->destFolder);
 1083→ 				strcat(bm->destFolder, FOLDER_SEP);
 1084: 
 1085: 				/* Make the output folder */
```

**Suggested fix (from review):** Copy BMSLEN-1 characters and terminate explicitly; check length before strcat

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

## MAIN-08

**memory leak** · Conf H / Sev L  
**Found at (current code):** `atlantismain.c` 1318
**Lines at review (Oct 2026):** 2250-2558

**Function:** `modelshutdown()`

**Why it's thought to be a bug**

`bm->init_pH` (allocated at line 1321 with alloc2d) is never freed in `modelshutdown()`. Every other array in that allocation block is freed. Function: `modelshutdown()`.

**Code at issue** — `atlantismain.c` line 1318  
Search for: `init_pH`

```c
 1316:     }
 1317: 
 1318→     bm->init_pH = (double **) alloc2d(bm->wcnz + bm->sednz, bm->nbox);
 1319: 	bm->vol = (double **) alloc2d(bm->wcnz + bm->sednz, bm->nbox);
 1320: 	bm->por = (double **) alloc2d(bm->wcnz + bm->sednz, bm->nbox);
```

**Suggested fix (from review):** Add `free2d(bm->init_pH);`

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

atlantismain.c:316. The general-output trigger uses `bm->t + dtscale * bm->dt / 2.0 > bm->tout`, where `dtscale = dt/86400` is in days, so the offset is dt²/172800 s. The fisheries output (line 366) and the increment loops (361, 395) use `dt/2`. The comment suggests this is a deliberate hack to keep output in daylight, but the units are mixed.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atlantismain.c:703-705. The copy of newlandtr back to bm->landtr is commented out, while newlandtr is refreshed from landtr every step (409) and passed to boundaries(). Any land-tracer changes made in newlandtr are thrown away. This may be intentional if land biology writes bm->landtr directly.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-3

atlantismain.c:737. `bm->TofY % 5 == 0` makes checknetCDFFiles run on every timestep of such days when dt is less than one day. This costs time but does not give wrong results.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
