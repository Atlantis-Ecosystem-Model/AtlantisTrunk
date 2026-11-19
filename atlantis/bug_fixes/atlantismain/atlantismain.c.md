# atlantismain/atlantismain.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **8 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 3 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MAIN-01](#main-01) | 2440-2441 | H/M | NULL dereference / mismatched alloc-free | `free2d(dvol); free3d(dtr);` run every time, but `dvol`/`dtr` are only allocated inside `if (do_physics)` (atl… |  | [ ] |
| [MAIN-02](#main-02) | 1136-1142 | H/L | NULL dereference | `pdest = strstr(bm->ncOfname, endname); totname = strlen(ncOfname) - strlen(pdest);` and then `*pdest = '\0'` … |  | [ ] |
| [MAIN-03](#main-03) | 962-969 | M/L | buffer overrun | `strcat(commands, commandTempStr)` keeps appending every argv into `char commands[STRLEN]` (2000) with no boun… |  | [ ] |
| [MAIN-04](#main-04) | 1031 | H/L | always-false condition | `!bm->forceIfname \|\| !bm->fishprmIfname \|\| !bm->assessprmIfname \|\| !bm->econprmIfname` test char-array address… |  | [ ] |
| [MAIN-05](#main-05) | 1178 | H/L | wrong label / copy-paste (log text) | The `bm->params` string prints `-s %s -g %s -c %s -d %s` with the summary, PROD, TOTCATCH and CATCH output fil… |  | [ ] |
| [MAIN-06](#main-06) | 2232-2237 | H/L | always-false condition | `not_tracking_flux = totfluxout; if (totfluxout != not_tracking_flux) quit(...)` assigns and then compares the… |  | [ ] |
| [MAIN-07](#main-07) | 981-1084, 1109 | M/L | missing string terminator | `strncpy(bm->X, *++argv, BMSLEN)` into BMSLEN-sized fields leaves no terminator when the argument is 512 chara… |  | [ ] |
| [MAIN-08](#main-08) | 2397-2705 | H/L | memory leak | `bm->init_pH` (allocated at line 1179 with alloc2d) is never freed in `modelshutdown()`. Every other array in … |  | [ ] |

---

## MAIN-01

**NULL dereference / mismatched alloc-free** · Conf H / Sev M  
**Lines (current code):** 2440-2441  
**Lines at review (Oct 2026):** 2293-2294

**Function:** `modelshutdown()`

**Why it's thought to be a bug**

`free2d(dvol); free3d(dtr);` run every time, but `dvol`/`dtr` are only allocated inside `if (do_physics)` (atlantismain.c:1534-1540). `do_physics` is only set by the `-p` flag (line 1008), so without `-p` they are NULL globals. `d_free2d` reads `pp[0]` before anything else (sjwlib/memory.c:648), and `d_free3d` reads `ppp[0][0]` (memory.c:721), so shutdown crashes. (If `flag_skip_phys` clears do_physics after allocation, the free is fine.) Function: `modelshutdown()`.

**Current code** (`atlantismain/atlantismain.c` lines 2438–2443)

```c
 2438: 	}
 2439: 
 2440: 	free1d(bm->init_scalar);
 2441: 	if(bm->estBo)
 2442: 		free1d(bm->estBo);
 2443: 	free2d(bm->portindx);
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
**Lines (current code):** 1136-1142  
**Lines at review (Oct 2026):** 994-1000

**Function:** `setupMSEBoxModel()`

**Why it's thought to be a bug**

`pdest = strstr(bm->ncOfname, endname); totname = strlen(ncOfname) - strlen(pdest);` and then `*pdest = '\0'` on the second strstr. If the `-o` name does not contain ".nc", pdest is NULL and the program segfaults when it should print a usage error. Function: `setupMSEBoxModel()`.

**Current code** (`atlantismain/atlantismain.c` lines 1134–1144)

```c
 1134: 	totname = (int) strlen(bm->ncOfname) - (int) strlen(pdest);
 1135: 
 1136: 	// Create a string that stores the filename without the .nc part.
 1137: 	strcpy(tempStr, bm->ncOfname);
 1138: 	pdest = strstr(tempStr, endname);
 1139: 	*pdest = '\0';
 1140: 
 1141: 	strncpy(bm->ncOSumfname, bm->ncOfname, (size_t)totname);
 1142: 	snprintf(bm->startfname, BMSLEN, "%s", bm->ncOSumfname);
 1143: 	//snprintf(bm->ncOSumfname, BMSLEN, "%sTOT.nc", bm->ncOSumfname);
 1144: 	strcat(bm->ncOSumfname, "TOT.nc");
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
**Lines (current code):** 962-969  
**Lines at review (Oct 2026):** 823-830

**Function:** `setupMSEBoxModel()`

**Why it's thought to be a bug**

`strcat(commands, commandTempStr)` keeps appending every argv into `char commands[STRLEN]` (2147) with no bound check. Atlantis takes 16+ arguments, often long absolute paths, so the total can go past 2000 bytes and overflow the stack. Function: `setupMSEBoxModel()`.

**Current code** (`atlantismain/atlantismain.c` lines 960–971)

```c
  960: 	for (i = 1; i < argc-1; i++) {
  961: 		snprintf(commandTempStr, sizeof(commandTempStr), " %s", argv[i]);
  962: 		strcat(commands, commandTempStr);
  963: 		//(void) snprintf(commands, sizeof(commands), "%s %s", commands,
  964: 	}
  965: 	snprintf(commandTempStr, sizeof(commandTempStr), " %s", argv[argc-1]);
  966: 	strcat(commands, commandTempStr);
  967: 
  968: 	//(void) snprintf(commands, sizeof(commands), "%s %s",commands, argv[argc-1]);
  969: 
  970: 	strcpy(bm->destFolder, "");
  971: 	strcpy(bm->inputFolder, "");
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
**Lines (current code):** 1031  
**Lines at review (Oct 2026):** 1031

**Function:** `setupMSEBoxModel()`

**Why it's thought to be a bug**

`!bm->forceIfname || !bm->fishprmIfname || !bm->assessprmIfname || !bm->econprmIfname` test char-array addresses, which are never NULL, so these parts of the "required arguments" check never fire. The other entries correctly test `[0]`. (forceIfname is already checked at line 986, so only the fish/assess/econ checks are dead. Those files are optional, so do not just add `[0]` to them.) Function: `setupMSEBoxModel()`.

**Current code** (`atlantismain/atlantismain.c` lines 1029–1033)

```c
 1029: 			case 'e': // Economics parameter file
 1030: 				if (argc < 2)
 1031: 					Util_Usage(1);
 1032: 				strncpy(bm->econprmIfname, *++argv, BMSLEN);
 1033: 				argc--;
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
**Lines (current code):** 1178  
**Lines at review (Oct 2026):** 1036

**Function:** `setupMSEBoxModel()`

**Why it's thought to be a bug**

The `bm->params` string prints `-s %s -g %s -c %s -d %s` with the summary, PROD, TOTCATCH and CATCH output file names. These are old flag meanings: `-s` is now the functional-group file, `-c` is cabspot, `-d` is the destination folder, and `-g` is not a flag. The params string written into output metadata is therefore wrong and cannot be re-run. Function: `setupMSEBoxModel()`.

**Current code** (`atlantismain/atlantismain.c` lines 1176–1180)

```c
 1176:     
 1177: 	if (do_biology) {
 1178: 		char s[STRLEN];
 1179: 		snprintf(s, sizeof(s), " -b %s", bm->bioprmIfname);
 1180: 		strcat(bm->params, s);
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
**Lines (current code):** 2232-2237  
**Lines at review (Oct 2026):** 2085-2090

**Function:** `AllocateArrayMemory()`

**Why it's thought to be a bug**

`not_tracking_flux = totfluxout; if (totfluxout != not_tracking_flux) quit(...)` assigns and then compares the same value, so the sanity check can never fire. Function: `AllocateArrayMemory()`.

**Current code** (`atlantismain/atlantismain.c` lines 2230–2239) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
 2230: 
 2231: 	/* Set up performance measure arrays */
 2232: 	bm->ecolindx = Util_Alloc_Init_1D_Double(K_num_ecol_indx, 0.0);
 2233: 	bm->econindx = Util_Alloc_Init_2D_Double(K_num_econ_indx, bm->K_num_fisheries, 0.0);
 2234: 	bm->portindx = Util_Alloc_Init_2D_Double(K_num_port_indx, bm->K_num_ports, 0.0);
 2235: 
 2236: 	/* Set up performance measure name arrays - the 20 is the length of the string and would
 2237: 	 need to be increased if the names of the code ids increased in size */
 2238: 	bm->ecolindxNAME = (char **) c_alloc2d(20, K_num_ecol_indx);
 2239: 	bm->econindxNAME = (char **) c_alloc2d(20, K_num_econ_indx);
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
**Lines (current code):** 981-1084, 1109  
**Lines at review (Oct 2026):** 842-942, 967

**Function:** `setupMSEBoxModel()`

**Why it's thought to be a bug**

`strncpy(bm->X, *++argv, BMSLEN)` into BMSLEN-sized fields leaves no terminator when the argument is 512 characters or longer. The memset at 808 does not help because all 512 bytes get written. The `-d`/`-t` cases then `strcat(FOLDER_SEP)`, which can overflow. Function: `setupMSEBoxModel()`.

**Current code** (`atlantismain/atlantismain.c` lines 979–1018 (truncated))

```c
  979: 				bm->ncIfdump = atoi(*++argv);
  980: 				argc -= 2;
  981: 				break;
  982: 			case 'o': // Output name
  983: 				if (argc < 2)
  984: 					Util_Usage(1);
  985: 				strncpy(bm->ncOfname, *++argv, BMSLEN);
  986: 				argc--;
  987: 				break;
  988: 			case 'r': // Run parameter file
  989: 				if (argc < 2)
  990: 					Util_Usage(1);
  991: 				strncpy(bm->runprmIfname, *++argv, BMSLEN);
  992: 				argc--;
  993: 				break;
  994: 			case 'f': // Forcing parameter file
  995: 				if (argc < 2)
  996: 					Util_Usage(1);
  997: 				strncpy(bm->forceIfname, *++argv, BMSLEN);
  998: 				argc--;
  999: 				break;
 1000: 			case 'p': // Physics parameter file
 1001: 				if (argc < 2)
 1002: 					Util_Usage(1);
 1003: 				strncpy(bm->physprmIfname, *++argv, BMSLEN);
 1004: 				argc--;
 1005: 				do_physics = 1;
 1006: 				break;
 1007: 			case 'b': // Biology parameter file
 1008: 				if (argc < 2)
 1009: 					Util_Usage(1);
 1010: 				strncpy(bm->bioprmIfname, *++argv, BMSLEN);
 1011: 				argc--;
 1012: 				do_biology = 1;
 1013: 				break;
 1014: 			case 'h': // Harvest parameter file
 1015: 				if (argc < 2)
 1016: 					Util_Usage(1);
 1017: 				strncpy(bm->fishprmIfname, *++argv, BMSLEN);
 1018: 				argc--;
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
**Lines (current code):** 2397-2705  
**Lines at review (Oct 2026):** 2250-2558

**Function:** `modelshutdown()`

**Why it's thought to be a bug**

`bm->init_pH` (allocated at line 1321 with alloc2d) is never freed in `modelshutdown()`. Every other array in that allocation block is freed. Function: `modelshutdown()`.

**Current code** (`atlantismain/atlantismain.c` lines 2395–2434 (truncated))

```c
 2395:         free1d(bm->boxes[b].flush_in);
 2396:         free1d(bm->boxes[b].flush_out);
 2397: 
 2398:         if(bm->boxes[b].uniqueAdjBoxes){
 2399:         	free(bm->boxes[b].uniqueAdjBoxes);
 2400:         }
 2401: 
 2402:         free(bm->boxes[b].distID);
 2403:     }
 2404: 
 2405: 	/* Free Memory for top level boxmodel arrays */
 2406:     i_free1d((int*) bm->is_boundary);
 2407: 	free2d((double**) bm->nom_dz);
 2408: 	free2d((double**) bm->dz);
 2409: 	free3d((double***) bm->wctr);
 2410: 	free3d((double***) bm->sedtr);
 2411: 	free2d((double**) bm->epi);
 2412: 	free2d((double**) bm->vol);
 2413: 	free2d((double**) bm->por);
 2414: 	free2d((double**) bm->hddrain);
 2415: 	free2d((double**) bm->hdpool);
 2416: 	free2d((double**) bm->efluxes);
 2417: 	free2d((double**) bm->vfluxes);
 2418: 	free4d((double****) bm->vvdistrib);
 2419: 	free3d((double***) newwctr);
 2420: 	free3d((double***) newsedtr);
 2421: 	if(bm->ice_on){
 2422: 		free3d((double***) newicetr);
 2423: 		free3d((double***) bm->icetr);
 2424: 		free2d((double**) bm->icedz);
 2425: 	}
 2426: 	if(bm->terrestrial_on){
 2427: 		free2d((double**) newlandtr);
 2428: 		free2d((double**) bm->landtr);
 2429: 	}
 2430: 
 2431: 	i_free2d(bm->checkedalready);
 2432: 	free2d(dvol);
 2433: 	free3d(dtr);
 2434: 
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
