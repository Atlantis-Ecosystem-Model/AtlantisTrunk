# atSS3Link/atSS3LinkIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **33 findings** (severity H: 19; confidence H and severity H: 17; on the priority list: 17), plus 5 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SS3IO-01](#ss3io-01) ⭐ | 61-72 (also 219-230) | H/H | NULL dereference | `if ((fid = fopen(...)) == NULL) { ... AssessFail_id = 1; }` then falls through to `while (fgets(buf, buflen, … |  | [ ] |
| [SS3IO-02](#ss3io-02) ⭐ | 36, 61; 302, 315 | H/H | wrong path / logic | Both readers open bare `"Report.sso"` / `"ss3.par"` in the current working dir and ignore `folderName`. In the… |  | [ ] |
| [SS3IO-03](#ss3io-03) | 85-104 (also 243-256) | M/L | NULL dereference | `varStr = strtok(NULL, seps); n++; if (n == EstBcurrIndex) ... atof(varStr)` — the NULL check is done before t… |  | [ ] |
| [SS3IO-04](#ss3io-04) ⭐ | 41, 146-160 | H/H | uninitialised variable / logic | In the `Biology_at_age_in_endyr_with_CV=f(LAA)` block no lines are ever read: `strtok(buffer, seps)` tokenises… |  | [ ] |
| [SS3IO-05](#ss3io-05) ⭐ | 149-153, 169-179 | H/H | buffer overrun (index) | `for (age = 0; age < RBCspeciesParam[AccumAge_id]; age++) ... MeanLenAgeM[0][sex][age][thisID]` — MeanLenAgeM/… |  | [ ] |
| [SS3IO-06](#ss3io-06) | 187-188 | H/L | logic (log text) | "Something has gone wrong reading in the SS3 report file" is printed/logged unconditionally after the loop, in… |  | [ ] |
| [SS3IO-07](#ss3io-07) | 72-194; 230-292; 326-345 | H/M | resource leak | `fid` is never `fclose`d in `Read_SS3_Report_File()`, `Read_SS3_Report_File_EstBo_Only()` or `Read_SS3_Par_Fil… |  | [ ] |
| [SS3IO-08](#ss3io-08) | 323-338 | M/H | logic / parsing | Searches for `MGparm[16]`, then "ignore the next line" and reads the line after. In an ADMB `.par` file each `… |  | [ ] |
| [SS3IO-09](#ss3io-09) ⭐ | 359, 374-375 | H/H | wrong variable | Starter file writes `"%s%s.dat", groupCode, fileName` where `fileName` is the starter's own path (`"<dir>//sta… |  | [ ] |
| [SS3IO-10](#ss3io-10) ⭐ | 397-398 | H/H | copy-paste | `"1 # min age for calc of summary biomass"` is written twice. SS3.30 starter expects a single value here, so e… |  | [ ] |
| [SS3IO-11](#ss3io-11) | 419, 423 | H/L | buffer size | `snprintf(fileName, sizeof(fileName), ...)` where `fileName` is a `char *` parameter -> `sizeof` is 8, path tr… |  | [ ] |
| [SS3IO-12](#ss3io-12) | 552-561 | M/L | division by zero / fleet index | `catch = CatchData[f][sumregion][maxyr] * TAC_old / last_catch` with no guard for `last_catch == 0`; fleets pr… |  | [ ] |
| [SS3IO-13](#ss3io-13) | 733, 748, 774, 780, 942 | H/L | copy-paste / wrong variable | Several errors: 780 `for (f = 0; fleetIndex < NumFisheries; f++)` tests the stale `fleetIndex` (== NumFisherie… |  | [ ] |
| [SS3IO-14](#ss3io-14) ⭐ | 1148/1155-1156, 2281; 1164-1165, 2380 | H/H | double free (fclose) | `WriteSSCtl()` ends with `fclose(fid)` and `WriteSS330Files()` then calls `fclose(fidctl)` on the same FILE*; … |  | [ ] |
| [SS3IO-15](#ss3io-15) ⭐ | 1221-1222 | H/H | data file format | Fleet definition line `" 1 -1 1 1 0 FLEET%d in metier %s\n"` has no `#`, so SS reads fleetname=`FLEET0` and th… |  | [ ] |
| [SS3IO-16](#ss3io-16) ⭐ | 1222-1300, 1333, 1346, 1446, 1574, 2018-2023, 2037-2039, 2240-2255 | H/H | off-by-one (fleet numbering) | All SS fleet numbers are written 0-based (`f`/`i`): catch lines `-999 1 %d`, CPUE setup `%d 1 0 0`, CPUE/disca… |  | [ ] |
| [SS3IO-17](#ss3io-17) ⭐ | 1250-1255 | H/H | data file format | `fprintf(fid, "%d 1 %d", t, f)` has no trailing space and the next `fprintf(fid, "%e ", catch)` has no leading… |  | [ ] |
| [SS3IO-18](#ss3io-18) | 1278, 1280 (also 1300, 1346) | H/M | data file format | CPUE setup lines for non-metier fleets and all surveys (`" %d 1 0 0 # SURVEY%d"`) have no `\n`; the next fleet… |  | [ ] |
| [SS3IO-19](#ss3io-19) | 1218-1528, 2012-2255 | M/H | buffer overrun (index) | Loops run `f < NumFisheries + NumSurvey` and index arrays sized `K_num_fisheries` in the fleet dimension: `CPU… |  | [ ] |
| [SS3IO-20](#ss3io-20) ⭐ | 1486-1491, 1577-1590 | H/H | off-by-one | Writes `MaxAge+1` as `#_N_age_bins` but then only `MaxAge` bin labels (`a < MaxAge`) and `MaxAge` values per a… |  | [ ] |
| [SS3IO-21](#ss3io-21) ⭐ | 1493-1499 (also 943-949, 3323-3334) | H/H | off-by-one | Ageing-error definition rows must have Nages+1 entries (ages 0..accumulator age); loops write `a < AccumAge` =… |  | [ ] |
| [SS3IO-22](#ss3io-22) ⭐ | 1187, 1561-1566 | H/H | copy-paste / uninitialised | Age-comp partition uses `it==1 -> 2, it==2 -> 0, it==3 -> 1` but `it` runs 0..2: `it==0` (retained) leaves `pa… |  | [ ] |
| [SS3IO-23](#ss3io-23) | 1680, 2065-2071; 2365-2373 | H/M | wrong index (region) | `WriteSSCtl()` and `WriteSSFor()` set `allregion = 0`, but the sum over regions is stored at index `NumRegions… |  | [ ] |
| [SS3IO-24](#ss3io-24) | 1705-1713 | H/M | off-by-one / dead data | Writes `NblockperPatt[i]` blocks per pattern but loops `j < NblockperPatt[i]-1` with `Blocks[i][j*2+1], Blocks… |  | [ ] |
| [SS3IO-25](#ss3io-25) ⭐ | 1801 | H/H | data file format | `Mat_slope_Fem` line `" -3 3 %f %f %f 0.8 0 -3 0 0 0 0 0 0 0"` has 15 fields (three `%f` plus 0.8), whereas ea… |  | [ ] |
| [SS3IO-26](#ss3io-26) | 1852 | H/M | wrong operator/precedence | `vbk = log(VBk[0][1])/VBk[0][0];` — the male K offset should be `log(VBk[0][1]/VBk[0][0])` (cf. 1863, 1874). W… |  | [ ] |
| [SS3IO-27](#ss3io-27) | 2017, 2114 | H/M | wrong index / overrun | `speciesRPFleetToMetier[f][sp+1]` — array is `[K_num_fisheries][K_num_tot_sp]` (atAssessTierSetup.c:53) and ev… |  | [ ] |
| [SS3IO-28](#ss3io-28) ⭐ | 2115, 2117 | H/H | ctl file format | Age-selex pattern lines for fishing fleets (`"%d 0 0 0 # FLEET %d in metier %s"`) have no `\n`, so the next fl… |  | [ ] |
| [SS3IO-29](#ss3io-29) | 2088 | M/L | ctl file format | Mirrored size-selex line `"%d %d 0 %d "` has no `\n` (no `#` so tokens still parse, but the next fleet's line … |  | [ ] |
| [SS3IO-30](#ss3io-30) ⭐ | 2368-2373 | H/H | uninitialised field / division by zero | `cat[f] = CatchData[f][allregion][maxyr] * RBCspeciesArray[sp].mgtTAC_old / last_catch;` — `mgtTAC_old` (atlan… |  | [ ] |
| [SS3IO-31](#ss3io-31) ⭐ | 2374 | H/H | ctl file format | `fprintf(fid, "%d 1 %d %f\n,", ...)` — stray comma after the newline, so each following line starts with `,` (… |  | [ ] |
| [SS3IO-32](#ss3io-32) | 2383-2948, 3444-3981 | H/L | copy-paste / wrong index | (Dead; `Write_SS_Control_File_Orig` is a verbatim copy.) Male parameters are all computed from FEMALE values (… |  | [ ] |
| [SS3IO-33](#ss3io-33) | 2966-3442 | H/L | logic / overrun | (Dead.) `s` loops `0..Nsex_samp-1` with `Nsex_samp = 1` but vectors are only written for `s == 1` / `s == 2` (… |  | [ ] |

## Reviewer notes for this file

Context: the live path is `TierOne()` -> `SS330Assessment()` (atmanage/atSS3assess.c) -> `WriteSS330Files()` (-> `WriteSSDat`, `WriteSSCtl`, `WriteSSFor`, `Create_Starter_File`), then `Run_SS3()`, `Read_SS3_Par_File()`, `Read_SS3_Report_File()`. `WriteSSFiles()` (atSS3Link.c) is commented out in `TierOne()`, so `Write_Forecast_File`, `Write_SS_Data_File`, `Write_SS_Control_File`, the `*_Orig` functions and `Read_SS3_Report_File_EstBo_Only` are currently unreachable (dead). Findings in dead code are rated Sev L.
Functions checked: Read_SS3_Report_File, Read_SS3_Report_File_EstBo_Only, Read_SS3_Par_File, Create_Starter_File, Create_Starter_File_Orig, Write_Forecast_File, Write_SS_Data_File, WriteSS330Files, WriteSSDat, WriteSSCtl, WriteSSFor, Write_SS_Control_File, Write_SS_Data_File_Orig, Write_SS_Control_File_Orig.

---

## SS3IO-01

**NULL dereference** · Conf H / Sev H ⭐ priority  
**Listed lines:** 61-72 (also 219-230)

**Function:** `Read_SS3_Report_File()`

**Why it's thought to be a bug**

`if ((fid = fopen(...)) == NULL) { ... AssessFail_id = 1; }` then falls through to `while (fgets(buf, buflen, fid) ...)` with `fid == NULL` -> crash whenever SS3 fails to produce Report.sso (the very case the warning is for). Function: `Read_SS3_Report_File()` (same in `Read_SS3_Report_File_EstBo_Only()`). `Read_SS3_Par_File()` correctly returns.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 81–96) — code appears to have moved +22 lines since the review

```c
   81:         fprintf(bm->logFile, "yr: %d WARNING - Read_SS3_Report_File - SS3 failed to generate a Report.sso output file for %s\n", year, FunctGroupArray[species].groupCode);
   82:         printf("yr: %d WARNING - Read_SS3_Report_File - SS3 failed to generate a Report.sso output file for %s\n", year, FunctGroupArray[species].groupCode);
   83:         bm->RBCestimation.RBCspeciesParam[species][AssessFail_id] = 1;
   84:         free1d(values);
   85:         return 0.0;   // Nov 2026: previously carried on and read from a NULL file pointer
   86:     }
   87: 
   88: 	/* Need to read in the RBC and total catch*/
   89: 	RBC = 0;
   90: 
   91: 	/* Find line that starts with 'FORECAST:_With_F_to_match_adjusted_catch' */
   92: 	while (fgets(buf, buflen, fid) != NULL) {
   93: 
   94: 		if (strstr(buf, "FORECAST:_With_F_to_match_adjusted_catch") != NULL) {
   95: 
   96: 			/* ignore the next line */
```

**Suggested fix (from review):** `free(values); return 0.0;` inside the NULL branch

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

## SS3IO-02

**wrong path / logic** · Conf H / Sev H ⭐ priority  
**Listed lines:** 36, 61; 302, 315

**Why it's thought to be a bug**

Both readers open bare `"Report.sso"` / `"ss3.par"` in the current working dir and ignore `folderName`. In the live path `Run_SS3()` (atSS3assess.c:190-200) does `chdir(run_dir)` and then `chdir(cwd)` back before `Read_SS3_Par_File`/`Read_SS3_Report_File` are called (atSS3assess.c:118-124), so the files SS3 just wrote in `<sp>_SS3_sim_..._year_...` are never read (either "can't open" -> SS3IO-01 crash / par=1000 -> assessment always failed, or a stale file from cwd is read). The old `WriteSSFiles()` path did chdir before reading, the new one doesn't. Functions: `Read_SS3_Report_File()`, `Read_SS3_Par_File()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 34–38)

```c
   34: /**
   35:  * Nov 2026: open an SS3 output file from the run folder (folderName/fileName). Falls back to the
   36:  * bare file name for callers that have already changed into the run folder (e.g. legacy WriteSSFiles).
   37:  */
   38: static FILE *Open_SS3_Output_File(char *folderName, char *fileName) {
```

**Suggested fix (from review):** Build path `snprintf(path, ..., "%s%s%s", folderName, FOLDER_SEP, outputFileName)` and fopen that

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

## SS3IO-03

**NULL dereference** · Conf M / Sev L  
**Listed lines:** 85-104 (also 243-256)

**Function:** `Read_SS3_Report_File()`

**Why it's thought to be a bug**

`varStr = strtok(NULL, seps); n++; if (n == EstBcurrIndex) ... atof(varStr)` — the NULL check is done before the strtok, so on a short/blank line `atof(NULL)` is called. Function: `Read_SS3_Report_File()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 80–103) — code appears to have moved -3 lines since the review

```c
   80: 		fprintf(bm->logFile,"Read_SS3_Report_File: Can't open %s/%s\n", folderName, outputFileName);
   81:         fprintf(bm->logFile, "yr: %d WARNING - Read_SS3_Report_File - SS3 failed to generate a Report.sso output file for %s\n", year, FunctGroupArray[species].groupCode);
   82:         printf("yr: %d WARNING - Read_SS3_Report_File - SS3 failed to generate a Report.sso output file for %s\n", year, FunctGroupArray[species].groupCode);
   83:         bm->RBCestimation.RBCspeciesParam[species][AssessFail_id] = 1;
   84:         free1d(values);
   85:         return 0.0;   // Nov 2026: previously carried on and read from a NULL file pointer
   86:     }
   87: 
   88: 	/* Need to read in the RBC and total catch*/
   89: 	RBC = 0;
   90: 
   91: 	/* Find line that starts with 'FORECAST:_With_F_to_match_adjusted_catch' */
   92: 	while (fgets(buf, buflen, fid) != NULL) {
   93: 
   94: 		if (strstr(buf, "FORECAST:_With_F_to_match_adjusted_catch") != NULL) {
   95: 
   96: 			/* ignore the next line */
   97: 			fgets(buf, buflen, fid);
   98: 
   99: 			/* grab the data from the next line */
  100: 			fgets(buf, buflen, fid);
  101: 
  102: 			varStr = strtok(buf, seps);
  103: 			n = 0;
```

**Suggested fix (from review):** Test `varStr != NULL` after each strtok before use

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

## SS3IO-04

**uninitialised variable / logic** · Conf H / Sev H ⭐ priority  
**Listed lines:** 41, 146-160

**Function:** `Read_SS3_Report_File()`

**Why it's thought to be a bug**

In the `Biology_at_age_in_endyr_with_CV=f(LAA)` block no lines are ever read: `strtok(buffer, seps)` tokenises the never-initialised local `char buffer[1000]`, so `values[12..16]` are garbage (or uninitialised malloc memory). Also the continuation separator is `"\n"` not `seps`, so after the first token the rest of the line would be a single token; and `j` is unbounded against `values[100]`. Function: `Read_SS3_Report_File()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 50–54) — code appears to have moved +11 lines since the review

```c
   50: }
   51: 
   52: double Read_SS3_Report_File(MSEBoxModel *bm, int species, int year, char *folderName) {
   53: 
   54: 	char outputFileName[] = "Report.sso";
```

**Suggested fix (from review):** `fgets(buffer, sizeof buffer, fid)` per sex/age row (after skipping header), use `seps` for every strtok, bound `j < size`

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

## SS3IO-05

**buffer overrun (index)** · Conf H / Sev H ⭐ priority  
**Listed lines:** 149-153, 169-179

**Function:** `Read_SS3_Report_File()`

**Why it's thought to be a bug**

`for (age = 0; age < RBCspeciesParam[AccumAge_id]; age++) ... MeanLenAgeM[0][sex][age][thisID]` — MeanLenAgeM/SigmaLenAgeM/MeanWtAgeS/MeanWtAgeM are allocated `Util_Alloc_Init_4D_Double(nYears, MaxAge, Nsexes, stocks)` => `[stocks][Nsexes][MaxAge][nYears]` (atAssessTierSetup.c:281-285), and AccumAge = MaxAge + ceil(MaxAge/4) (atAssessTierSetup.c:86) > MaxAge, so `age` overruns the age dimension. Function: `Read_SS3_Report_File()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 80–114) — code appears to have moved -67 lines since the review

```c
   80: 		fprintf(bm->logFile,"Read_SS3_Report_File: Can't open %s/%s\n", folderName, outputFileName);
   81:         fprintf(bm->logFile, "yr: %d WARNING - Read_SS3_Report_File - SS3 failed to generate a Report.sso output file for %s\n", year, FunctGroupArray[species].groupCode);
   82:         printf("yr: %d WARNING - Read_SS3_Report_File - SS3 failed to generate a Report.sso output file for %s\n", year, FunctGroupArray[species].groupCode);
   83:         bm->RBCestimation.RBCspeciesParam[species][AssessFail_id] = 1;
   84:         free1d(values);
   85:         return 0.0;   // Nov 2026: previously carried on and read from a NULL file pointer
   86:     }
   87: 
   88: 	/* Need to read in the RBC and total catch*/
   89: 	RBC = 0;
   90: 
   91: 	/* Find line that starts with 'FORECAST:_With_F_to_match_adjusted_catch' */
   92: 	while (fgets(buf, buflen, fid) != NULL) {
   93: 
   94: 		if (strstr(buf, "FORECAST:_With_F_to_match_adjusted_catch") != NULL) {
   95: 
   96: 			/* ignore the next line */
   97: 			fgets(buf, buflen, fid);
   98: 
   99: 			/* grab the data from the next line */
  100: 			fgets(buf, buflen, fid);
  101: 
  102: 			varStr = strtok(buf, seps);
  103: 			n = 0;
  104: 
  105: 			while (varStr != NULL) {
  106: 
  107: 				varStr = strtok(NULL, seps);
  108: 				n++;
  109:                 
  110:                 /*
  111:                 if ( n == year) {
  112:                     yy = atof(varStr);
  113:                 }
  114:                  */
```

**Suggested fix (from review):** Loop `age < MaxAge_id` (or allocate with AccumAge+1)

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

## SS3IO-06

**logic (log text)** · Conf H / Sev L  
**Listed lines:** 187-188

**Function:** `Read_SS3_Report_File()`

**Why it's thought to be a bug**

"Something has gone wrong reading in the SS3 report file" is printed/logged unconditionally after the loop, including on success. Function: `Read_SS3_Report_File()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 269–274) — code appears to have moved +84 lines since the review

```c
  269:  *
  270:  */
  271: double Read_SS3_Report_File_EstBo_Only(MSEBoxModel *bm, int species, int year, char *folderName) {
  272: 
  273:     char outputFileName[] = "Report.sso";
  274: 
```

**Suggested fix (from review):** Only print when the LAA block was not found

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

## SS3IO-07

**resource leak** · Conf H / Sev M  
**Listed lines:** 72-194; 230-292; 326-345

**Why it's thought to be a bug**

`fid` is never `fclose`d in `Read_SS3_Report_File()`, `Read_SS3_Report_File_EstBo_Only()` or `Read_SS3_Par_File()`. Two descriptors leak per species per assessment year; on long multi-species runs this can hit the per-process FD limit, after which fopen fails (-> SS3IO-01 crash).

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 78–112 (range truncated)) — code appears to have moved +8 lines since the review

```c
   78: 
   79:     if ((fid = Open_SS3_Output_File(folderName, outputFileName)) == NULL) {
   80: 		fprintf(bm->logFile,"Read_SS3_Report_File: Can't open %s/%s\n", folderName, outputFileName);
   81:         fprintf(bm->logFile, "yr: %d WARNING - Read_SS3_Report_File - SS3 failed to generate a Report.sso output file for %s\n", year, FunctGroupArray[species].groupCode);
   82:         printf("yr: %d WARNING - Read_SS3_Report_File - SS3 failed to generate a Report.sso output file for %s\n", year, FunctGroupArray[species].groupCode);
   83:         bm->RBCestimation.RBCspeciesParam[species][AssessFail_id] = 1;
   84:         free1d(values);
   85:         return 0.0;   // Nov 2026: previously carried on and read from a NULL file pointer
   86:     }
   87: 
   88: 	/* Need to read in the RBC and total catch*/
   89: 	RBC = 0;
   90: 
   91: 	/* Find line that starts with 'FORECAST:_With_F_to_match_adjusted_catch' */
   92: 	while (fgets(buf, buflen, fid) != NULL) {
   93: 
   94: 		if (strstr(buf, "FORECAST:_With_F_to_match_adjusted_catch") != NULL) {
   95: 
   96: 			/* ignore the next line */
   97: 			fgets(buf, buflen, fid);
   98: 
   99: 			/* grab the data from the next line */
  100: 			fgets(buf, buflen, fid);
  101: 
  102: 			varStr = strtok(buf, seps);
  103: 			n = 0;
  104: 
  105: 			while (varStr != NULL) {
  106: 
  107: 				varStr = strtok(NULL, seps);
  108: 				n++;
  109:                 
  110:                 /*
  111:                 if ( n == year) {
  112:                     yy = atof(varStr);
```

**Suggested fix (from review):** `fclose(fid)` before each return

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

## SS3IO-08

**logic / parsing** · Conf M / Sev H  
**Listed lines:** 323-338

**Function:** `Read_SS3_Par_File()`

**Why it's thought to be a bug**

Searches for `MGparm[16]`, then "ignore the next line" and reads the line after. In an ADMB `.par` file each `# MGparm[16]:` header is followed directly by its value line, so the code skips the value and reads the next `# MGparm[17]:` header -> `atof("#") = 0` -> `MaxConvergCrit` is always 0 and the convergence test in `SS330Assessment()` always passes. Also MGparm[16] is a biology parameter, not a convergence measure (the max gradient is on the `.par` header line "Maximum gradient component = ..."). Function: `Read_SS3_Par_File()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 321–340)

```c
  321:                 }
  322: 
  323:                 /* The 'Depletion' column */
  324:                 if (n == depletionIndex) {
  325:                     bm->RBCestimation.RBCspeciesParam[species][EstDepletion_id] = atof(varStr);
  326:                 }
  327: 
  328:                 if (n >= startOfFleetData)
  329:                     break;
  330:             }
  331: 
  332:             bm->RBCestimation.RBCspeciesParam[species][EstB0_id] = bm->RBCestimation.RBCspeciesParam[species][EstBcurr_id] / bm->RBCestimation.RBCspeciesParam[species][EstDepletion_id];
  333: 
  334: 
  335:             fleetIndex = 0;
  336:             n = 0;
  337:             RBC = 0;
  338:             /* Now deal with the fleets */
  339:             while (varStr != NULL) {
  340: 
```

**Suggested fix (from review):** Parse "Maximum gradient component" from the first line of ss3.par (or read the value line directly after the header)

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

## SS3IO-09

**wrong variable** · Conf H / Sev H ⭐ priority  
**Listed lines:** 359, 374-375

**Function:** `Create_Starter_File()`

**Why it's thought to be a bug**

Starter file writes `"%s%s.dat", groupCode, fileName` where `fileName` is the starter's own path (`"<dir>//starter.ss"`) -> e.g. `FISdir//starter.ss.dat`. The real files written by `WriteSS330Files()` are `<dir>/<groupCode>WriteSS330Files.dat/.ctl` (lines 1142, 1151), and SS3 runs inside `<dir>`, so SS3 cannot find its data/control files. Function: `Create_Starter_File()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 421–441) — code appears to have moved +64 lines since the review

```c
  421: 
  422: /**
  423:  *	Create the SS3 run. starter file
  424:  *
  425:  *
  426:  */
  427: 
  428: void Create_Starter_File(MSEBoxModel *bm, char *dirName, int groupIndex, int versionID) {
  429: 
  430: 	FILE *fid;
  431:     char fileName[STRLEN];
  432:     
  433: 	snprintf(fileName, sizeof(fileName), "%s/%sstarter.ss", dirName, FOLDER_SEP);
  434: 
  435: 	printf("fileName = %s\n", fileName);
  436: 
  437: 	if ((fid = fopen(fileName, "w")) == NULL)
  438: 		quit("Create_Starter_File: Can't open %s\n", fileName);
  439: 
  440: 	printf("versionID = %d\n", versionID);
  441: 	/* Now start writing out the data */
```

**Suggested fix (from review):** Pass the dat/ctl base name into `Create_Starter_File` and write `"%s%s.dat"` with groupCode + that base name (no directory)

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

## SS3IO-10

**copy-paste** · Conf H / Sev H ⭐ priority  
**Listed lines:** 397-398

**Function:** `Create_Starter_File()`

**Why it's thought to be a bug**

`"1 # min age for calc of summary biomass"` is written twice. SS3.30 starter expects a single value here, so every subsequent entry (Depletion basis, fraction X, SPR basis, F units, ..., check value 3.30) is shifted by one and the starter fails its end-of-file check / is misread. Function: `Create_Starter_File()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 426–431) — code appears to have moved +31 lines since the review

```c
  426:  */
  427: 
  428: void Create_Starter_File(MSEBoxModel *bm, char *dirName, int groupIndex, int versionID) {
  429: 
  430: 	FILE *fid;
  431:     char fileName[STRLEN];
```

**Suggested fix (from review):** Delete line 398

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

## SS3IO-11

**buffer size** · Conf H / Sev L  
**Listed lines:** 419, 423

**Function:** `Create_Starter_File_Orig()`

**Why it's thought to be a bug**

`snprintf(fileName, sizeof(fileName), ...)` where `fileName` is a `char *` parameter -> `sizeof` is 8, path truncated to 7 chars. Function: `Create_Starter_File_Orig()` (dead).

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 429–437) — code appears to have moved +12 lines since the review

```c
  429: 
  430: 	FILE *fid;
  431:     char fileName[STRLEN];
  432:     
  433: 	snprintf(fileName, sizeof(fileName), "%s/%sstarter.ss", dirName, FOLDER_SEP);
  434: 
  435: 	printf("fileName = %s\n", fileName);
  436: 
  437: 	if ((fid = fopen(fileName, "w")) == NULL)
```

**Suggested fix (from review):** Pass the buffer length or use STRLEN

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

## SS3IO-12

**division by zero / fleet index** · Conf M / Sev L  
**Listed lines:** 552-561

**Function:** `Write_Forecast_File()`

**Why it's thought to be a bug**

`catch = CatchData[f][sumregion][maxyr] * TAC_old / last_catch` with no guard for `last_catch == 0`; fleets printed 0-based (`f`) where SS needs 1..Nfleets. Function: `Write_Forecast_File()` (dead).

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 550–563)

```c
  550: 	char fileName[STRLEN];
  551: 	double catch, last_catch;
  552: 	int f;
  553: 	//int sumregion = 0; /* This isn't going to work - needs to be numRegions + 1 */
  554:     int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumRegions_id]);
  555:     int sumregion = Nregions;  // sum over regions
  556: 
  557: 	snprintf(fileName, sizeof(fileName), "%s%sforecast.ss", dirName, FOLDER_SEP);
  558: 
  559: 	printf("fileName = %s\n", fileName);
  560: 
  561: 	if ((fid = fopen(fileName, "w")) == NULL)
  562: 		quit("Write_Forecast_File: Can't open %s\n", fileName);
  563: 
```

**Suggested fix (from review):** Guard `last_catch > 0`; print `f+1`

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

## SS3IO-13

**copy-paste / wrong variable** · Conf H / Sev L  
**Listed lines:** 733, 748, 774, 780, 942

**Function:** `Write_SS_Data_File()`

**Why it's thought to be a bug**

Several errors: 780 `for (f = 0; fleetIndex < NumFisheries; f++)` tests the stale `fleetIndex` (== NumFisheries after the loop at 690) so discard fleet specs are never written; 733 writes a stray line `"%e %e"` of NumFisheries/NumSurvey into the CPUE block; 748 CPUE obs `" %d 7 %e %e"` omits the fleet field; 774 writes NumFisheries as N_fleets_with_discard instead of `nfltdisc` (fixed in `WriteSSDat` 1326); 942 no newline after age bins. Function: `Write_SS_Data_File()` (dead).

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 731–770 (truncated))

```c
  731:     fprintf(fid, "#_fleet_type timing area units need_catch_mult fleetname\n");
  732:     
  733:     // loop over fleets and surveys
  734:     // We assume that fleets always come before surveys
  735: 	for (fleetIndex = 0; fleetIndex < (bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[groupIndex][NumSurvey_id]); fleetIndex++) {
  736:         
  737:         //add the fleet type
  738:         if (fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]) {
  739:             if (bm->RBCestimation.speciesRPFleetToMetier[fleetIndex][groupIndex] > -1) {
  740:                 fprintf(fid, " 1  -1  1  1  0  FLEET%d # %d in metier %s\n", fleetIndex, fleetIndex, FisheryArray[fleetIndex].fisheryCode);
  741:             } else {
  742:                 fprintf(fid, " 1  -1  1  1  0  FLEET%d # %d\n", fleetIndex, fleetIndex);
  743:             }
  744:         } else {
  745:             fprintf(fid, " 3   1  1  2  0  SURVEY%d # %d\n", fleetIndex, fleetIndex - (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]));
  746:         }
  747: 	}
  748: 
  749:     // Bycatch fleet not currently used
  750:     fprintf(fid, "#Bycatch_fleet_input_goes_next\n");
  751:     fprintf(fid, "#a:  fleet index\n");
  752:     fprintf(fid, "#b:  1=include dead bycatch in total dead catch for F0.1 and MSY optimizations and forecast ABC; 2=omit from total catch for these purposes (but still include the mortality)\n");
  753:     fprintf(fid, "#c:  1=Fmult scales with other fleets; 2=bycatch F constant at input value; 3=bycatch F from range of years\n");
  754:     fprintf(fid, "#d:  F or first year of range\n");
  755:     fprintf(fid, "#e:  last year of range\n");
  756:     fprintf(fid, "#f:  not used\n");
  757:     fprintf(fid, "# a   b   c   d   e   f \n");
  758:     // Header for the catch data
  759:     fprintf(fid, "#_Catch data: yr, seas, fleet, catch, catch_se\n");
  760:     fprintf(fid, "#_catch_se:  standard error of log(catch)\n");
  761:     fprintf(fid, "#_NOTE:  catch data is ignored for survey fleets\n");
  762: 
  763:     // loop over the fleets only as catch data is ignored for survey fleets
  764: 	for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
  765: 
  766:         fprintf(fid, "-999 1 %d %e 0.01\n", fleetIndex, bm->RBCestimation.RBCspeciesArray[groupIndex].initialEquilCatch[fleetIndex]);
  767:        //next is a loop over the years
  768:        // t is the year, f is fleet
  769:        for (t = bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id]; t < maxyr; t++) {
  770:          fprintf(fid, "%d 1 %d ", t, fleetIndex);
```

**Suggested fix (from review):** Use `f` in loop test; remove 733; add fleet to 748; use `nfltdisc`

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

## SS3IO-14

**double free (fclose)** · Conf H / Sev H ⭐ priority  
**Listed lines:** 1148/1155-1156, 2281; 1164-1165, 2380

**Why it's thought to be a bug**

`WriteSSCtl()` ends with `fclose(fid)` and `WriteSS330Files()` then calls `fclose(fidctl)` on the same FILE*; likewise `WriteSSFor()` closes and the caller closes `fidss` again. Double `fclose` is undefined behaviour (heap corruption / abort on glibc). `WriteSSDat()` correctly leaves closing to the caller. Functions: `WriteSSCtl()`, `WriteSSFor()`, `WriteSS330Files()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1188–1192) — code appears to have moved +42 lines since the review

```c
 1188: //******************************************************************************
 1189: //
 1190: // Name:  WriteSS330Files
 1191: // Description: write the files for input to SS v3.30.15.03
 1192: //
```

**Suggested fix (from review):** Remove `fclose(fid)` at 2281 and 2380

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

## SS3IO-15

**data file format** · Conf H / Sev H ⭐ priority  
**Listed lines:** 1221-1222

**Function:** `WriteSSDat()`

**Why it's thought to be a bug**

Fleet definition line `" 1  -1  1  1  0  FLEET%d in metier %s\n"` has no `#`, so SS reads fleetname=`FLEET0` and then `in`, `metier`, `<code>` as the start of the next fleet's definition -> whole data file misparsed. Because `speciesRPFleetToMetier` is never populated (all 0, so `> -1` is always true) this branch is always taken. The earlier `Write_SS_Data_File()` (line 666) has `# %d in metier`. Function: `WriteSSDat()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1219–1224)

```c
 1219:     }
 1220: 
 1221:     WriteSSDat(bm, fiddat, sp, maxyr);
 1222:     fclose(fiddat);
 1223: 
 1224:     // SS control file
```

**Suggested fix (from review):** `" 1 -1 1 1 0 FLEET%d # in metier %s\n"`

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

## SS3IO-16

**off-by-one (fleet numbering)** · Conf H / Sev H ⭐ priority  
**Listed lines:** 1222-1300, 1333, 1346, 1446, 1574, 2018-2023, 2037-2039, 2240-2255

**Why it's thought to be a bug**

All SS fleet numbers are written 0-based (`f`/`i`): catch lines `-999 1 %d`, CPUE setup `%d 1 0 0`, CPUE/discard/length/age observations, Q setup, Q parms, variance adjustments. SS3.30 numbers fleets 1..Nfleets, so fleet 0 is invalid and every observation is attributed to the previous fleet (the last survey is never referenced). The `*_Orig` writers used `fleetIndex + 1` (e.g. 3087-3110, 3269). Functions: `WriteSSDat()`, `WriteSSCtl()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1219–1253 (range truncated))

```c
 1219:     }
 1220: 
 1221:     WriteSSDat(bm, fiddat, sp, maxyr);
 1222:     fclose(fiddat);
 1223: 
 1224:     // SS control file
 1225:     snprintf(outputFileName, sizeof(outputFileName), "%s/%s%s.ctl", baseFolder,FunctGroupArray[sp].groupCode, fileName);
 1226:     if ((fidctl = fopen(outputFileName, "w")) == NULL) {
 1227:         quit("Error opening ctl control file for  %s\n", FunctGroupArray[sp].groupCode);
 1228:     }
 1229:     WriteSSCtl(bm, fidctl, sp, maxyr);
 1230:     fclose(fidctl);
 1231: 
 1232:     //  SS forecast file
 1233:     snprintf(outputFileName, sizeof(outputFileName), "%s/forecast.ss", baseFolder);
 1234: 
 1235:     if ((fidss = fopen(outputFileName, "w")) == NULL) {
 1236:         quit("Error opening forecast file for  %s\n", FunctGroupArray[sp].groupCode);
 1237:     }
 1238:     WriteSSFor(bm, fidss, sp, maxyr);
 1239:     fclose(fidss);
 1240: 
 1241:     // SS starter file
 1242:     Create_Starter_File(bm, baseFolder, sp, versionID);
 1243:     
 1244:     return;
 1245: }
 1246: 
 1247: //******************************************************************************
 1248: //
 1249: // Name:  WriteSSDat
 1250: // Description: write the files for input to SS v3.30.15.03
 1251: //
 1252: //
 1253: // called by : WriteSSFiles
```

**Suggested fix (from review):** Write `f+1` / `i+1` wherever an SS fleet number is output

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

## SS3IO-17

**data file format** · Conf H / Sev H ⭐ priority  
**Listed lines:** 1250-1255

**Function:** `WriteSSDat()`

**Why it's thought to be a bug**

`fprintf(fid, "%d 1 %d", t, f)` has no trailing space and the next `fprintf(fid, "%e ", catch)` has no leading space, so fleet and catch merge (e.g. `1980 1 01.234e+02  0.01`) -> 4 tokens instead of 5, and the whole catch block is misread. `Write_SS_Data_File()` (line 696) has `"%d 1 %d "`. Function: `WriteSSDat()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1247–1256)

```c
 1247: //******************************************************************************
 1248: //
 1249: // Name:  WriteSSDat
 1250: // Description: write the files for input to SS v3.30.15.03
 1251: //
 1252: //
 1253: // called by : WriteSSFiles
 1254: // calls :
 1255: // created  : Nov 2007 Sally
 1256: // updated:   Feb 2011 Sally
```

**Suggested fix (from review):** `"%d 1 %d "`

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

## SS3IO-18

**data file format** · Conf H / Sev M  
**Listed lines:** 1278, 1280 (also 1300, 1346)

**Function:** `WriteSSDat()`

**Why it's thought to be a bug**

CPUE setup lines for non-metier fleets and all surveys (`" %d 1  0  0  # SURVEY%d"`) have no `\n`; the next fleet's setup then sits after the `#` on the same line and is ignored by SS, so with 2+ surveys the specs are missing and the data file is misread. (1300 CPUE obs and 1346 discard obs also lack `\n`; harmless token-wise but they merge with the following line.) Function: `WriteSSDat()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1258–1264) — code appears to have moved -18 lines since the review

```c
 1258: //
 1259: //******************************************************************************
 1260: void WriteSSDat(MSEBoxModel *bm, FILE *fid, int sp, int maxyr) {
 1261:     int t, f, s, a, l, i, num, part, it, allregion, gender, nfltdisc, iy;
 1262:     double *emptySex;
 1263:     double cpcv, disccv;
 1264: 
```

**Suggested fix (from review):** Add `\n` to all four formats

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

## SS3IO-19

**buffer overrun (index)** · Conf M / Sev H  
**Listed lines:** 1218-1528, 2012-2255

**Why it's thought to be a bug**

Loops run `f < NumFisheries + NumSurvey` and index arrays sized `K_num_fisheries` in the fleet dimension: `CPUEgen[f]`, `CPUEcv[f]`, `LengthFltYr[it][f]`, `LFss[f]`, `LenComp[f]` (atAssessTierSetup.c:149-170, all `[..K_num_fisheries..]`), and in the ctl `CPUEqmu[f]`, `Sel_Pattern[f]`, `mirrored_fleet[f]`, `Start_Sel[f]`, `Sel_Phase[f]`, `SelBlock[f]`, `Varadj_*[f]` (allocated K_num_fisheries / `[K_num_fisheries][K_num_tot_sp]`). NumFisheries is capped near K_num_fisheries (atAssessTierSetup.c:1453-1457) so any surveys push `f` past the end. Functions: `WriteSSDat()`, `WriteSSCtl()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1227–1261 (range truncated)) — code appears to have moved +11 lines since the review

```c
 1227:         quit("Error opening ctl control file for  %s\n", FunctGroupArray[sp].groupCode);
 1228:     }
 1229:     WriteSSCtl(bm, fidctl, sp, maxyr);
 1230:     fclose(fidctl);
 1231: 
 1232:     //  SS forecast file
 1233:     snprintf(outputFileName, sizeof(outputFileName), "%s/forecast.ss", baseFolder);
 1234: 
 1235:     if ((fidss = fopen(outputFileName, "w")) == NULL) {
 1236:         quit("Error opening forecast file for  %s\n", FunctGroupArray[sp].groupCode);
 1237:     }
 1238:     WriteSSFor(bm, fidss, sp, maxyr);
 1239:     fclose(fidss);
 1240: 
 1241:     // SS starter file
 1242:     Create_Starter_File(bm, baseFolder, sp, versionID);
 1243:     
 1244:     return;
 1245: }
 1246: 
 1247: //******************************************************************************
 1248: //
 1249: // Name:  WriteSSDat
 1250: // Description: write the files for input to SS v3.30.15.03
 1251: //
 1252: //
 1253: // called by : WriteSSFiles
 1254: // calls :
 1255: // created  : Nov 2007 Sally
 1256: // updated:   Feb 2011 Sally
 1257: // updated  : Nov 2012 Sally for SS v3.24
 1258: //
 1259: //******************************************************************************
 1260: void WriteSSDat(MSEBoxModel *bm, FILE *fid, int sp, int maxyr) {
 1261:     int t, f, s, a, l, i, num, part, it, allregion, gender, nfltdisc, iy;
```

**Suggested fix (from review):** Allocate fleet dimension as K_num_fisheries + max surveys, or guard `f < K_num_fisheries`

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

## SS3IO-20

**off-by-one** · Conf H / Sev H ⭐ priority  
**Listed lines:** 1486-1491, 1577-1590

**Function:** `WriteSSDat()`

**Why it's thought to be a bug**

Writes `MaxAge+1` as `#_N_age_bins` but then only `MaxAge` bin labels (`a < MaxAge`) and `MaxAge` values per age-comp vector. SS reads MaxAge+1 bin edges (consuming the `1` of `#_N_ageerror_definitions`) and every age-comp row is one value short -> file misparsed. Function: `WriteSSDat()` (same pattern in `Write_SS_Data_File()` 936-940, 1032-1045).

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1558–1567) — code appears to have moved +74 lines since the review

```c
 1558:     // Initialise - TODO: Is emptysex dealt with properly?
 1559: 
 1560:     fprintf(fid, "%d   #_N_age_bins\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]+1));
 1561:     // Note: 3.30.15 requires ages to start from 1 - but note from RL suggest this no longer true as of July 2021
 1562:     for (a = 0; a < bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]; a++){
 1563:         fprintf(fid, "%d ", a);
 1564:     }
 1565:     fprintf(fid, "\n");
 1566:     fprintf(fid, "1   #_N_ageerror_definitions\n");
 1567:     for (a = 0; a < bm->RBCestimation.RBCspeciesParam[sp][AccumAge_id]; a++) {     // bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id] +6 is accumulator age + 1
```

**Suggested fix (from review):** Make the count and the loops agree (e.g. print `MaxAge` bins, or loop `a <= MaxAge` everywhere with AgeComp sized accordingly)

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

## SS3IO-21

**off-by-one** · Conf H / Sev H ⭐ priority  
**Listed lines:** 1493-1499 (also 943-949, 3323-3334)

**Function:** `WriteSSDat()`

**Why it's thought to be a bug**

Ageing-error definition rows must have Nages+1 entries (ages 0..accumulator age); loops write `a < AccumAge` = AccumAge entries. `Ageing_error` is allocated `AccumAge+1` "dimensions 0..AccumAge (SS data file)" (atAssessTierSetup.c:175), and the comment on 1493 says "accumulator age + 1". Each row is one short so the following data is shifted. Function: `WriteSSDat()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1565–1575) — code appears to have moved +74 lines since the review

```c
 1565:     fprintf(fid, "\n");
 1566:     fprintf(fid, "1   #_N_ageerror_definitions\n");
 1567:     for (a = 0; a < bm->RBCestimation.RBCspeciesParam[sp][AccumAge_id]; a++) {     // bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id] +6 is accumulator age + 1
 1568:         fprintf(fid, "%f ", a+0.5);
 1569:     }
 1570:     fprintf(fid, "\n");
 1571:     for (a = 0; a < bm->RBCestimation.RBCspeciesParam[sp][AccumAge_id]; a++) {
 1572:         fprintf(fid, "%f ", bm->RBCestimation.RBCspeciesArray[sp].Ageing_error[a]);
 1573:     }
 1574:     fprintf(fid, "\n");
 1575:     num = 0;
```

**Suggested fix (from review):** `a <= AccumAge` in both loops

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

## SS3IO-22

**copy-paste / uninitialised** · Conf H / Sev H ⭐ priority  
**Listed lines:** 1187, 1561-1566

**Function:** `WriteSSDat()`

**Why it's thought to be a bug**

Age-comp partition uses `it==1 -> 2, it==2 -> 0, it==3 -> 1` but `it` runs 0..2: `it==0` (retained) leaves `part` unset (uninitialised in this function — declared without init at 1187) or stale, `it==1` (whole) is labelled retained, `it==2` (discard) labelled whole. The length block a few lines up (1433-1439) uses the correct `!it -> 2, 1 -> 0, else 1`. Function: `WriteSSDat()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1193–1197) — code appears to have moved +8 lines since the review

```c
 1193: //
 1194: // called by : DoAssessment
 1195: // calls : WriteSSDat, WriteSSCtl, WriteSSFor
 1196: // created  :
 1197: // updated  : Oct 2020 Maciej Golebiewski
```

**Suggested fix (from review):** Copy the mapping from 1433-1439

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

## SS3IO-23

**wrong index (region)** · Conf H / Sev M  
**Listed lines:** 1680, 2065-2071; 2365-2373

**Why it's thought to be a bug**

`WriteSSCtl()` and `WriteSSFor()` set `allregion = 0`, but the sum over regions is stored at index `NumRegions` (`CatchData`/`DiscData` allocated `K_num_reg+1` "so have sum over all regions"; `WriteSSDat()` 1192 and atManageTier.c:598 use `Nregions`). So retention parameters (`dflt`) are decided from region 0 only while the dat file's discard fleets come from the all-region sum (mismatched parameter count -> SS error), and forecast catch splits use region 0 only.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1264–1268) — code appears to have moved -414 lines since the review

```c
 1264: 
 1265:     //allregion = 0;    // uses weighted sum over regions, assume assessment doesn't know about regions
 1266:     allregion = (int)(bm->RBCestimation.RBCspeciesParam[sp][NumRegions_id]);
 1267: 
 1268:     //  ------  Dimensions
```

**Suggested fix (from review):** `allregion = (int)RBCspeciesParam[sp][NumRegions_id];` in both

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

## SS3IO-24

**off-by-one / dead data** · Conf H / Sev M  
**Listed lines:** 1705-1713

**Function:** `WriteSSCtl()`

**Why it's thought to be a bug**

Writes `NblockperPatt[i]` blocks per pattern but loops `j < NblockperPatt[i]-1` with `Blocks[i][j*2+1], Blocks[i][j*2+2]` (1-based leftover) -> one begin/end pair too few, so the ctl is misread when `NblockPattern > 0`. Also `Blocks` and `NblockperPatt` are allocated (atAssessTierSetup.c:287-288) but never filled anywhere in the repo. Function: `WriteSSCtl()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1732–1744) — code appears to have moved +29 lines since the review

```c
 1732: //******************************************************************************
 1733: //
 1734: // Name:  WriteSSCtl
 1735: // Description: write the files for input to SS v3.30.15.03
 1736: //
 1737: //
 1738: // called by : _WriteSSFiles
 1739: // calls :
 1740: // created  : Nov 2007 Sally
 1741: // updated  : Feb 2011 to write SSv3.1
 1742: // updated  : Nov 2012 Sally for SS v3.24
 1743: // updated  : FEb 2020 PB for SS v3.30
 1744: //
```

**Suggested fix (from review):** `for (j = 0; j < NblockperPatt[i]; j++) ... Blocks[i][2*j], Blocks[i][2*j+1]` and populate the arrays

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

## SS3IO-25

**data file format** · Conf H / Sev H ⭐ priority  
**Listed lines:** 1801

**Function:** `WriteSSCtl()`

**Why it's thought to be a bug**

`Mat_slope_Fem` line `"    -3     3 %f %f %f     0.8    0   -3  0 0 0 0 0 0 0"` has 15 fields (three `%f` plus 0.8), whereas each MG parameter line takes 14 (cf. Mat50 line 1800). Every later ctl entry is shifted by one token. Function: `WriteSSCtl()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1744–1748) — code appears to have moved -55 lines since the review

```c
 1744: //
 1745: //******************************************************************************
 1746: void WriteSSCtl(MSEBoxModel *bm, FILE *fid, int sp, int maxyr) {
 1747:     int f, disc, t, i, j, phase, nsp, block_fxn, ageindex;
 1748:     int yindex = (int)(bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]);
```

**Suggested fix (from review):** Drop one `%f` (and its argument)

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

## SS3IO-26

**wrong operator/precedence** · Conf H / Sev M  
**Listed lines:** 1852

**Function:** `WriteSSCtl()`

**Why it's thought to be a bug**

`vbk = log(VBk[0][1])/VBk[0][0];` — the male K offset should be `log(VBk[0][1]/VBk[0][0])` (cf. 1863, 1874). Wrong male K whenever Nsexes>1 and MG_offset>2. Function: `WriteSSCtl()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1744–1748) — code appears to have moved -106 lines since the review

```c
 1744: //
 1745: //******************************************************************************
 1746: void WriteSSCtl(MSEBoxModel *bm, FILE *fid, int sp, int maxyr) {
 1747:     int f, disc, t, i, j, phase, nsp, block_fxn, ageindex;
 1748:     int yindex = (int)(bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]);
```

**Suggested fix (from review):** `log(VBk[0][1] / VBk[0][0])`

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

## SS3IO-27

**wrong index / overrun** · Conf H / Sev M  
**Listed lines:** 2017, 2114

**Function:** `WriteSSCtl()`

**Why it's thought to be a bug**

`speciesRPFleetToMetier[f][sp+1]` — array is `[K_num_fisheries][K_num_tot_sp]` (atAssessTierSetup.c:53) and every other use in this file is `[i][sp]` (1221, 1275, 1385, 1515). `sp+1` reads the next species' column and overruns the row when `sp == K_num_tot_sp-1`. Function: `WriteSSCtl()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 2089–2093) — code appears to have moved +74 lines since the review

```c
 2089:     
 2090:         if( f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]){
 2091:             if (bm->RBCestimation.speciesRPFleetToMetier[f][sp+1] > -1) {
 2092:                 fprintf(fid,  "%d   1   0   0   0   0  # FLEET %d in metier %s\n", f, f, FisheryArray[f].fisheryCode);
 2093:             } else {
```

**Suggested fix (from review):** `[f][sp]` / `[i][sp]`

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

## SS3IO-28

**ctl file format** · Conf H / Sev H ⭐ priority  
**Listed lines:** 2115, 2117

**Function:** `WriteSSCtl()`

**Why it's thought to be a bug**

Age-selex pattern lines for fishing fleets (`"%d  0  0  0  # FLEET %d in metier %s"`) have no `\n`, so the next fleet's/survey's `pattern 0 0 0` line lands after the `#` and is discarded by SS -> only one age-selex line is read for all fleets and the rest of the ctl is shifted. The survey branch (2120) has `\n`. Function: `WriteSSCtl()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 2113–2119)

```c
 2113:             fprintf(fid,  "  -15             15  %f 0             1             0          1          0          0          0          0          0          0          0    # LnQ_base_SURVEY %d (%d)\n", log(bm->RBCestimation.RBCspeciesArray[sp].CPUEqmu[f]), f, (int)(f - bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]));
 2114:         }
 2115:     }
 2116:     fprintf(fid, "# \n");
 2117:     fprintf(fid, "#_size_selex_patterns\n");
 2118:     fprintf(fid, "#Pattern:_0; parm=0; selex=1.0 for all sizes\n");
 2119:     fprintf(fid, "#Pattern:_1; parm=2; logistic; with 95 percent width specification\n");
```

**Suggested fix (from review):** Add `\n` to both formats

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

## SS3IO-29

**ctl file format** · Conf M / Sev L  
**Listed lines:** 2088

**Function:** `WriteSSCtl()`

**Why it's thought to be a bug**

Mirrored size-selex line `"%d %d 0 %d "` has no `\n` (no `#` so tokens still parse, but the next fleet's line is appended). Function: `WriteSSCtl()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1744–1748) — code appears to have moved -342 lines since the review

```c
 1744: //
 1745: //******************************************************************************
 1746: void WriteSSCtl(MSEBoxModel *bm, FILE *fid, int sp, int maxyr) {
 1747:     int f, disc, t, i, j, phase, nsp, block_fxn, ageindex;
 1748:     int yindex = (int)(bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]);
```

**Suggested fix (from review):** Add `\n`

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

## SS3IO-30

**uninitialised field / division by zero** · Conf H / Sev H ⭐ priority  
**Listed lines:** 2368-2373

**Function:** `WriteSSFor()`

**Why it's thought to be a bug**

`cat[f] = CatchData[f][allregion][maxyr] * RBCspeciesArray[sp].mgtTAC_old / last_catch;` — `mgtTAC_old` (atlantisboxmodel.h:2999) is never assigned anywhere in the repo, so forecast catches are 0 (or garbage); the TAC is kept in `RBCspeciesParam[sp][TAC_old_id]` (set atManageTier.c:140, 238; used by `Write_Forecast_File()` 558). Also no guard for `last_catch == 0` (NaN/inf written to forecast.ss). Function: `WriteSSFor()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 2445–2454) — code appears to have moved +79 lines since the review

```c
 2445:     }
 2446:     for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
 2447:         cat[f] = bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][allregion][maxyr] * bm->RBCestimation.RBCspeciesArray[sp].mgtTAC_old / last_catch;
 2448:          fprintf(fid, "%d 1 %d %f\n,", maxyr+1, f, cat[f]);
 2449:     }
 2450:     fprintf(fid, "-9999 1 1 0  # terminator_for_Fcast_catches\n");
 2451:     fprintf(fid, "#\n");
 2452:     fprintf(fid, "999     #  terminator_for_the_for_file \n");
 2453: 
 2454:     fclose(fid);
```

**Suggested fix (from review):** Use `RBCspeciesParam[sp][TAC_old_id]` and guard `last_catch > 0`

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

## SS3IO-31

**ctl file format** · Conf H / Sev H ⭐ priority  
**Listed lines:** 2374

**Function:** `WriteSSFor()`

**Why it's thought to be a bug**

`fprintf(fid, "%d 1 %d %f\n,", ...)` — stray comma after the newline, so each following line starts with `,` (e.g. `,-9999 1 1 0`), corrupting the forecast catch list and its terminator. Function: `WriteSSFor()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 2370–2374)

```c
 2370: //
 2371: //******************************************************************************
 2372: void WriteSSFor(MSEBoxModel *bm, FILE *fid, int sp, int maxyr) {
 2373:     int f,allregion;
 2374:     double last_catch;
```

**Suggested fix (from review):** `"%d 1 %d %f\n"`

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

## SS3IO-32

**copy-paste / wrong index** · Conf H / Sev L  
**Listed lines:** 2383-2948, 3444-3981

**Why it's thought to be a bug**

(Dead; `Write_SS_Control_File_Orig` is a verbatim copy.) Male parameters are all computed from FEMALE values (2511 `log(SSMort[0][FEMALE]/SSMort[0][FEMALE])` = 0; 2523-2548, 2563, 2587/2591 use `[0][0]` for male CV_old); `Wtlen_2_Fem` written twice (2606, 2609) and male wt-len lines use FEMALE and are labelled `_Fem` (2623-2627); `bm->RBCestimation.RBCspeciesArray->VBt0` (2477, 2485) = species 0, not `groupIndex`; `dflt` loop (2760-2765) overwrites per year so only the last year counts and runs `yearIndex <= maxyr`; 2675-2679, 2798-2846 missing `\n` before `#` comments. Functions: `Write_SS_Control_File()`, `Write_SS_Control_File_Orig()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 2381–2415 (range truncated))

```c
 2381:     fprintf(fid, "3 # MSY: 1= set to F(SPR); 2=calc F(MSY); 3=set to F(Btgt) or F0.1; 4=set to F(endyr) \n");
 2382:     fprintf(fid, "%f    # SPR target\n", bm->RBCestimation.RBCspeciesParam[sp][mgt_HCRtarget_id]);
 2383:     fprintf(fid, "%f    # Biomass target\n", bm->RBCestimation.RBCspeciesParam[sp][mgt_HCRtarget_id]);
 2384:     //* the values below may need to be the appropriate years rather than zeroes. The manual isn't informative but provides an example of 10 zeroes
 2385:     fprintf(fid, "#   Bmark_years: beg_bio, end_bio, beg_selex, end_selex, beg_relF, end_relF, beg_recr_dist, end_recr_dist, beg_SRparm, end_SRparm (enter actual year, or values of 0 or -integer to be rel. endyr)\n");
 2386:     fprintf(fid, " 0 0 0 0 0 0 0 0 0 0\n");
 2387:     fprintf(fid, "1 # Bmark_relF_Basis: 1 = use year range; 2 = set relF same as forecast below\n");
 2388:     fprintf(fid, "#\n");
 2389:     fprintf(fid, "3 # Forecast: -1=none; 0=simple_1yr; 1=F(SPR); 2=F(MSY) 3=F(Btgt) or F0.1; 4=Ave F (uses first-last relF yrs); 5=input annual F scalar\n");
 2390:     fprintf(fid, "# where none and simple require no input after this line; simple sets forecast F same as end year F \n");
 2391:     fprintf(fid, "51 # N forecast years\n");
 2392:     fprintf(fid, "0 # Fmult (only used for Do_Forecast==5) such that apical_F(f)=Fmult*relF(f)\n");
 2393:     fprintf(fid, "#_Fcast_years:  beg_selex, end_selex, beg_relF, end_relF, beg_mean recruits, end_recruits  (enter actual year, or values of 0 or -integer to be rel. endyr)\n");
 2394:     //* the example in the manual is 6 zeroes, the values below were used in 2019 SESSF assessments
 2395:     fprintf(fid, "0 0 0 0 -999 0\n");
 2396:     fprintf(fid, "0 # Forecast_selectivity (0=fcast selex is mean from year range; 1=fcast selectivity from annual time-vary parms)\n");
 2397:     fprintf(fid, "2 # Control rule method (1: ramp does catch=f(SSB), buffer on F; 2: ramp does F=f(SSB), buffer on F; 3: ramp does catch=f(SSB), buffer on catch; 4: ramp does F=f(SSB), buffer on catch) \n");
 2398:     fprintf(fid, "%f # Control rule Biomass level for constant F (as frac of Bzero, e.g. 0.40)\n", bm->RBCestimation.RBCspeciesParam[sp][mgt_HCRbreak_id]);
 2399:     /*
 2400:     if(0) {
 2401:         fprintf(bm->logFile," ------------------------------\n limit reference point  = 0.01 \n------------------------------\n");
 2402:         fprintf(fid, "0.01   # Control_rule_Biomass_level_for_no_F (as frac of Bzero, e.g. 0.10)\n");
 2403:     } else {
 2404:     */
 2405:     fprintf(fid, "%f # Control_rule_Biomass_level_for_no_F (as frac of Bzero, e.g. 0.10)\n", bm->RBCestimation.RBCspeciesParam[sp][mgt_HCRlimit_id]);
 2406:     //}
 2407:     fprintf(fid, "1 # Buffer:  enter Control rule target as fraction of Flimit (e.g. 0.75), negative value invokes list of [year, scalar] with filling from year to YrMax \n");
 2408:     fprintf(fid, "3 # N_forecast_loops (1=OFL only; 2=ABC; 3=get F from forecast ABC catch with allocations applied)\n");
 2409:     fprintf(fid, "3 # First_forecast_loop_with_stochastic_recruitment\n");
 2410:     fprintf(fid, "0 #_Forecast recruitment:  0= spawn_recr (<=SS3.30.10); 1=value*spawn_recr_fxn; 2=value*VirginRecr; 3=recent mean from yr range above (need to set phase to -1 in control to get constant recruitment in MCMC)\n");
 2411:     fprintf(fid, "1 # value is multiplier of SRR (if 1 or 2 selected above then this is a scalar applied to recruitment. If 3 = number of years to average recruitment. If 0 then ignored.\n");
 2412:     fprintf(fid, "0 #_Forecast loop control #5 (reserved for future bells&whistles) \n");
 2413:     fprintf(fid, "%d # FirstYear_for_caps_and_allocations (should be after years with fixed inputs) \n", maxyr+2);
 2414:     fprintf(fid, "0 # stddev_of_log (realized F/target F) in forecast (set value>0.0 to cause active impl_error)\n");
 2415:     fprintf(fid, "0 # Do_West_Coast_gfish_rebuilder output (0/1)\n");
```

**Suggested fix (from review):** Use MALE indices/`[groupIndex]`, set `dflt` only to 1 inside loop after zero-init

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

## SS3IO-33

**logic / overrun** · Conf H / Sev L  
**Listed lines:** 2966-3442

**Function:** `Write_SS_Data_File_Orig()`

**Why it's thought to be a bug**

(Dead.) `s` loops `0..Nsex_samp-1` with `Nsex_samp = 1` but vectors are only written for `s == 1` / `s == 2` (3272, 3279, 3372, 3379) so no comp data are ever written; `ageIndex <= MaxAge` (3374, 3384) reads `AgeComp[..][MaxAge]`, past the `OverallMaxAge` dimension when MaxAge == OverallMaxAge; `num` not reset before the length/age obs counts (3243, 3342) so counts include discards; 3134 prints stale `fleetIndex+1` instead of `nf+1`; environmental-variable header written twice (3419-3431). Function: `Write_SS_Data_File_Orig()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 2964–2998 (range truncated))

```c
 2964: 	for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
 2965: 		fprintf(fid, " %f", bm->RBCestimation.RBCspeciesArray[groupIndex].Varadj_length[fleetIndex]);
 2966: 	}
 2967: 	fprintf(fid, "      # mult by length comp\n");
 2968: 	for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
 2969: 		fprintf(fid, " %f", bm->RBCestimation.RBCspeciesArray[groupIndex].Varadj_age[fleetIndex]);
 2970: 	}
 2971: 	fprintf(fid, "      # mult by age comp\n");
 2972: 	for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
 2973: 		fprintf(fid, " 1");
 2974: 	}
 2975: 	fprintf(fid, "     # mult by mean size at age\n");
 2976: 
 2977: 	fprintf(fid, "#\n");
 2978: 	fprintf(fid, "4 #_maxlambdaphase\n");
 2979: 	fprintf(fid, "1 #_sd_offset\n");
 2980: 	fprintf(fid, "#\n");
 2981: 
 2982:    if (bm->RBCestimation.RBCspeciesParam[groupIndex][NumChangeLambda_id] > 0) {
 2983: 
 2984: 		fprintf(fid, "#Lambdas\n");
 2985: 		fprintf(fid, "%d      #  number of changes to make to default lambdas\n", (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumChangeLambda_id]));
 2986:         fprintf(fid,"# Like_comp codes:  1=surv; 2=disc; 3=mnwt; 4=length; 5=age; 6=SizeFreq; 7=sizeage; 8=catch;\n");
 2987:         fprintf(fid, "# 9=init_equ_catch; 10=recrdev; 11=parm_prior; 12=parm_dev; 13=CrashPen; 14=Morphcomp; 15=Tag-comp; 16=Tag-negbin\n");
 2988: 
 2989: 		fprintf(fid, "#component  fleet phase lambda sizefreq_meth\n");
 2990: 		for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
 2991: 			fprintf(fid, " 4 %d 1 0.1 1\n", fleetIndex + 1);
 2992: 		}
 2993: 		for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
 2994: 			fprintf(fid, " 5 %d 1 0.1 1\n", fleetIndex + 1);
 2995: 		}
 2996: 	} else {
 2997: 		fprintf(fid, "0   #  number of changes to make to default lambdas\n");
 2998: 	}
```

**Suggested fix (from review):** Fix sex tests to `s == 0`/`s == 1`, `<` MaxAge, reset `num`

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

Year convention: `WriteSSDat`/`WriteSSCtl` write `HistYrMin` as `#_StartYr` and the caller's `year` (a relative index, cf. `RBC_by_year[year]` in atSS3assess.c) as `#_EndYr`, and loop `t = HistYrMin .. maxyr-1` indexing the year dimension (size nYears) with `t`. This only works if `HistYrMin` is set to 0 in the XML; if it's an AD year (as the comment at line 635 says SS needs) no data rows are written at all. Hard-coded years (`2000 # F ballpark year` at 1980, `HistYrMin-50` at 1952) assume AD years. Worth agreeing one convention.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

`Read_SS3_Report_File()` stops (`break`) at the LAA block; if that section comes before `FORECAST:_With_F_to_match_adjusted_catch` in the SS3.30 Report.sso (and in 3.30 the forecast detail may be in Forecast-report.sso), RBC stays 0. `EstB0` is no longer computed here (commented out) but `SS330Assessment()` then recomputes `EstDepletion = EstBcurr / EstB0`, overwriting the depletion just read, using a stale EstB0.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-3

`WriteSSFor()` uses `CatchData[..][maxyr]` as "last year's catch", but `WriteSSDat()` only writes catches for `t < maxyr`; check that year `maxyr` is populated when the assessment runs.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-4

Age-comp terminator writes `2*MaxAge+10` zeros and length terminator `2*Nlen+5` regardless of `Nsexes`; for single-sex models SS row width is smaller, so check against SS 3.30 reading rules.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-5

Outside my file: `Run_SS3()` (atSS3assess.c:201) calls `free(ss3_abs)` where `ss3_abs` points at the stack buffer `buf` passed to `realpath()` — invalid free.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
