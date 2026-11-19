# atlantismain/atlantismain.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **8 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 3 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MAIN-01](#main-01) | 2293-2294 | H/M | NULL dereference / mismatched alloc-free | `free2d(dvol); free3d(dtr);` run every time, but `dvol`/`dtr` are only allocated inside `if (do_physics)` (atl… |  | [ ] |
| [MAIN-02](#main-02) | 994-1000 | H/L | NULL dereference | `pdest = strstr(bm->ncOfname, endname); totname = strlen(ncOfname) - strlen(pdest);` and then `*pdest = '\0'` … |  | [ ] |
| [MAIN-03](#main-03) | 823-830 | M/L | buffer overrun | `strcat(commands, commandTempStr)` keeps appending every argv into `char commands[STRLEN]` (2000) with no boun… |  | [ ] |
| [MAIN-04](#main-04) | 1031 | H/L | always-false condition | `!bm->forceIfname \|\| !bm->fishprmIfname \|\| !bm->assessprmIfname \|\| !bm->econprmIfname` test char-array address… |  | [ ] |
| [MAIN-05](#main-05) | 1036 | H/L | wrong label / copy-paste (log text) | The `bm->params` string prints `-s %s -g %s -c %s -d %s` with the summary, PROD, TOTCATCH and CATCH output fil… |  | [ ] |
| [MAIN-06](#main-06) | 2085-2090 | H/L | always-false condition | `not_tracking_flux = totfluxout; if (totfluxout != not_tracking_flux) quit(...)` assigns and then compares the… |  | [ ] |
| [MAIN-07](#main-07) | 842-942, 967 | M/L | missing string terminator | `strncpy(bm->X, *++argv, BMSLEN)` into BMSLEN-sized fields leaves no terminator when the argument is 512 chara… |  | [ ] |
| [MAIN-08](#main-08) | 2250-2558 | H/L | memory leak | `bm->init_pH` (allocated at line 1179 with alloc2d) is never freed in `modelshutdown()`. Every other array in … |  | [ ] |

---

## MAIN-01

**NULL dereference / mismatched alloc-free** · Conf H / Sev M  
**Listed lines:** 2293-2294

**Function:** `modelshutdown()`

**Why it's thought to be a bug**

`free2d(dvol); free3d(dtr);` run every time, but `dvol`/`dtr` are only allocated inside `if (do_physics)` (atlantismain.c:1392-1398). `do_physics` is only set by the `-p` flag (line 869), so without `-p` they are NULL globals. `d_free2d` reads `pp[0]` before anything else (sjwlib/memory.c:648), and `d_free3d` reads `ppp[0][0]` (memory.c:721), so shutdown crashes. (If `flag_skip_phys` clears do_physics after allocation, the free is fine.) Function: `modelshutdown()`.

**Current code** (`atlantismain/atlantismain.c` lines 2387–2392) — code appears to have moved +96 lines since the review

```c
 2387:  *
 2388:  */
 2389: void modelshutdown(MSEBoxModel *bm) {
 2390:     int b = 0;
 2391: 	printf("Freeing general box arrays\n");
 2392: 
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
**Listed lines:** 994-1000

**Function:** `setupMSEBoxModel()`

**Why it's thought to be a bug**

`pdest = strstr(bm->ncOfname, endname); totname = strlen(ncOfname) - strlen(pdest);` and then `*pdest = '\0'` on the second strstr. If the `-o` name does not contain ".nc", pdest is NULL and the program segfaults when it should print a usage error. Function: `setupMSEBoxModel()`.

**Current code** (`atlantismain/atlantismain.c` lines 992–1002)

```c
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
**Listed lines:** 823-830

**Function:** `setupMSEBoxModel()`

**Why it's thought to be a bug**

`strcat(commands, commandTempStr)` keeps appending every argv into `char commands[STRLEN]` (2000) with no bound check. Atlantis takes 16+ arguments, often long absolute paths, so the total can go past 2000 bytes and overflow the stack. Function: `setupMSEBoxModel()`.

**Current code** (`atlantismain/atlantismain.c` lines 928–939) — code appears to have moved +107 lines since the review

```c
  928: 	char *pdest;
  929: 	char tempStr[STRLEN];
  930: 	char commands[STRLEN];
  931: 	char commandTempStr[STRLEN];
  932: 
  933: 	logfp = NULL;
  934: 	haltfp = NULL;
  935: 
  936: #ifdef _STUDIO
  937: 	_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF);
  938: #endif
  939: 
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
**Listed lines:** 1031

**Function:** `setupMSEBoxModel()`

**Why it's thought to be a bug**

`!bm->forceIfname || !bm->fishprmIfname || !bm->assessprmIfname || !bm->econprmIfname` test char-array addresses, which are never NULL, so these parts of the "required arguments" check never fire. The other entries correctly test `[0]`. (forceIfname is already checked at line 986, so only the fish/assess/econ checks are dead. Those files are optional, so do not just add `[0]` to them.) Function: `setupMSEBoxModel()`.

**Current code** (`atlantismain/atlantismain.c` lines 1030–1034)

```c
 1030: 				if (argc < 2)
 1031: 					Util_Usage(1);
 1032: 				strncpy(bm->econprmIfname, *++argv, BMSLEN);
 1033: 				argc--;
 1034: 				do_economics = 1;
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
**Listed lines:** 1036

**Function:** `setupMSEBoxModel()`

**Why it's thought to be a bug**

The `bm->params` string prints `-s %s -g %s -c %s -d %s` with the summary, PROD, TOTCATCH and CATCH output file names. These are old flag meanings: `-s` is now the functional-group file, `-c` is cabspot, `-d` is the destination folder, and `-g` is not a flag. The params string written into output metadata is therefore wrong and cannot be re-run. Function: `setupMSEBoxModel()`.

**Current code** (`atlantismain/atlantismain.c` lines 918–922) — code appears to have moved -116 lines since the review

```c
  918:  Basic model initialisation
  919:  */
  920: void setupMSEBoxModel(int argc, char *argv[], MSEBoxModel *bm) {
  921: 	double cellvol, ask_t;
  922: 	int ij, traceid, den, rn, sn, fgIndex, i;
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
**Listed lines:** 2085-2090

**Function:** `AllocateArrayMemory()`

**Why it's thought to be a bug**

`not_tracking_flux = totfluxout; if (totfluxout != not_tracking_flux) quit(...)` assigns and then compares the same value, so the sanity check can never fire. Function: `AllocateArrayMemory()`.

**Current code** (`atlantismain/atlantismain.c` lines 2042–2051) — code appears to have moved -41 lines since the review

```c
 2042:  */
 2043: void AllocateArrayMemory(MSEBoxModel *bm, FILE *llogfp) {
 2044: 	int i, b, nyr, nid, totfluxout, nreg;
 2045: 	//int numRunYears = ceil(bm->tstop / 86400) / 365.0 + 1;
 2046: 
 2047: 	printf("flag_fine_ontogenetic_diets: %d initarray\n", bm->flag_fine_ontogenetic_diets);
 2048: 
 2049: 	/* Initialise MSEBoxModel arrays */
 2050:     bm->BoxKeyMap = Util_Alloc_Init_2D_Int(3, bm->nbox, 0);
 2051: 
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
**Listed lines:** 842-942, 967

**Function:** `setupMSEBoxModel()`

**Why it's thought to be a bug**

`strncpy(bm->X, *++argv, BMSLEN)` into BMSLEN-sized fields leaves no terminator when the argument is 512 characters or longer. The memset at 808 does not help because all 512 bytes get written. The `-d`/`-t` cases then `strcat(FOLDER_SEP)`, which can overflow. Function: `setupMSEBoxModel()`.

**Current code** (`atlantismain/atlantismain.c` lines 960–994 (range truncated)) — code appears to have moved +120 lines since the review

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
  972: 	while (--argc > 0) {
  973: 		if ((*++argv)[0] == '-') {
  974: 			switch ((*argv)[1]) {
  975: 			case 'i': // Input name
  976: 				if (argc < 3)
  977: 					Util_Usage(1);
  978: 				strncpy(bm->ncIfname, *++argv, BMSLEN);
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
**Listed lines:** 2250-2558

**Function:** `modelshutdown()`

**Why it's thought to be a bug**

`bm->init_pH` (allocated at line 1179 with alloc2d) is never freed in `modelshutdown()`. Every other array in that allocation block is freed. Function: `modelshutdown()`.

**Current code** (`atlantismain/atlantismain.c` lines 2387–2421 (range truncated)) — code appears to have moved +139 lines since the review

```c
 2387:  *
 2388:  */
 2389: void modelshutdown(MSEBoxModel *bm) {
 2390:     int b = 0;
 2391: 	printf("Freeing general box arrays\n");
 2392: 
 2393:     /* Free flux trackers */
 2394:     for (b = 0; b < bm->nbox; b++) {
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

atlantismain.c:250. The general-output trigger uses `bm->t + dtscale * bm->dt / 2.0 > bm->tout`, where `dtscale = dt/86400` is in days, so the offset is dt²/172800 s. The fisheries output (line 300) and the increment loops (295, 329) use `dt/2`. The comment suggests this is a deliberate hack to keep output in daylight, but the units are mixed.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atlantismain.c:581-583. The copy of newlandtr back to bm->landtr is commented out, while newlandtr is refreshed from landtr every step (343) and passed to boundaries(). Any land-tracer changes made in newlandtr are thrown away. This may be intentional if land biology writes bm->landtr directly.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-3

atlantismain.c:614. `bm->TofY % 5 == 0` makes checknetCDFFiles run on every timestep of such days when dt is less than one day. This costs time but does not give wrong results.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
