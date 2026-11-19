# atSS3Link/atSS3LinkIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **33 findings** (severity H: 19; confidence H and severity H: 17; on the priority list: 17), plus 5 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SS3IO-01](#ss3io-01) ⭐ | 437, 92, 46 | H/H | NULL dereference | `if ((fid = fopen(...)) == NULL) { ... AssessFail_id = 1; }` then falls through to `while (fgets(buf, buflen, … |  | [ ] |
| [SS3IO-02](#ss3io-02) ⭐ | 54, 374 | H/H | wrong path / logic | Both readers open bare `"Report.sso"` / `"ss3.par"` in the current working dir and ignore `folderName`. In the… |  | [ ] |
| [SS3IO-03](#ss3io-03) | 107, 112 | M/L | NULL dereference | `varStr = strtok(NULL, seps); n++; if (n == EstBcurrIndex) ... atof(varStr)` — the NULL check is done before t… |  | [ ] |
| [SS3IO-04](#ss3io-04) ⭐ | 214, 225, 59 | H/H | uninitialised variable / logic | In the `Biology_at_age_in_endyr_with_CV=f(LAA)` block no lines are ever read: `strtok(buffer, seps)` tokenises… |  | [ ] |
| [SS3IO-05](#ss3io-05) ⭐ | 218, 237 | H/H | buffer overrun (index) | `for (age = 0; age < RBCspeciesParam[AccumAge_id]; age++) ... MeanLenAgeM[0][sex][age][thisID]` — MeanLenAgeM/… |  | [ ] |
| [SS3IO-06](#ss3io-06) | 255 | H/L | logic (log text) | "Something has gone wrong reading in the SS3 report file" is printed/logged unconditionally after the loop, in… |  | [ ] |
| [SS3IO-07](#ss3io-07) | 271 | H/M | resource leak | `fid` is never `fclose`d in `Read_SS3_Report_File()`, `Read_SS3_Report_File_EstBo_Only()` or `Read_SS3_Par_Fil… |  | [ ] |
| [SS3IO-08](#ss3io-08) | 397 | M/H | logic / parsing | Searches for `MGparm[16]`, then "ignore the next line" and reads the line after. In an ADMB `.par` file each `… |  | [ ] |
| [SS3IO-09](#ss3io-09) ⭐ | 448 | H/H | wrong variable | Starter file writes `"%s%s.dat", groupCode, fileName` where `fileName` is the starter's own path (`"<dir>//sta… |  | [ ] |
| [SS3IO-10](#ss3io-10) ⭐ | 471 | H/H | copy-paste | `"1 # min age for calc of summary biomass"` is written twice. SS3.30 starter expects a single value here, so e… |  | [ ] |
| [SS3IO-11](#ss3io-11) | 497 | H/L | buffer size | `snprintf(fileName, sizeof(fileName), ...)` where `fileName` is a `char *` parameter -> `sizeof` is 8, path tr… |  | [ ] |
| [SS3IO-12](#ss3io-12) | 632 | M/L | division by zero / fleet index | `catch = CatchData[f][sumregion][maxyr] * TAC_old / last_catch` with no guard for `last_catch == 0`; fleets pr… |  | [ ] |
| [SS3IO-13](#ss3io-13) | 764 | H/L | copy-paste / wrong variable | Several errors: 780 `for (f = 0; fleetIndex < NumFisheries; f++)` tests the stale `fleetIndex` (== NumFisherie… |  | [ ] |
| [SS3IO-14](#ss3io-14) ⭐ | 1181, 1230 | H/H | double free (fclose) | `WriteSSCtl()` ends with `fclose(fid)` and `WriteSS330Files()` then calls `fclose(fidctl)` on the same FILE*; … |  | [ ] |
| [SS3IO-15](#ss3io-15) ⭐ | 1296 | H/H | data file format | Fleet definition line `" 1 -1 1 1 0 FLEET%d in metier %s\n"` has no `#`, so SS reads fleetname=`FLEET0` and th… |  | [ ] |
| [SS3IO-16](#ss3io-16) ⭐ | 2814 | H/H | off-by-one (fleet numbering) | All SS fleet numbers are written 0-based (`f`/`i`): catch lines `-999 1 %d`, CPUE setup `%d 1 0 0`, CPUE/disca… |  | [ ] |
| [SS3IO-17](#ss3io-17) ⭐ | 1324, 1316 | H/H | data file format | `fprintf(fid, "%d 1 %d", t, f)` has no trailing space and the next `fprintf(fid, "%e ", catch)` has no leading… |  | [ ] |
| [SS3IO-18](#ss3io-18) | 1354 | H/M | data file format | CPUE setup lines for non-metier fleets and all surveys (`" %d 1 0 0 # SURVEY%d"`) have no `\n`; the next fleet… |  | [ ] |
| [SS3IO-19](#ss3io-19) | 1292, 1365, 1488 | M/H | buffer overrun (index) | Loops run `f < NumFisheries + NumSurvey` and index arrays sized `K_num_fisheries` in the fleet dimension: `CPU… |  | [ ] |
| [SS3IO-20](#ss3io-20) ⭐ | 1678 | H/H | off-by-one | Writes `MaxAge+1` as `#_N_age_bins` but then only `MaxAge` bin labels (`a < MaxAge`) and `MaxAge` values per a… |  | [ ] |
| [SS3IO-21](#ss3io-21) ⭐ | 1567, 1571 | H/H | off-by-one | Ageing-error definition rows must have Nages+1 entries (ages 0..accumulator age); loops write `a < AccumAge` =… |  | [ ] |
| [SS3IO-22](#ss3io-22) ⭐ | 1486 | H/H | copy-paste / uninitialised | Age-comp partition uses `it==1 -> 2, it==2 -> 0, it==3 -> 1` but `it` runs 0..2: `it==0` (retained) leaves `pa… |  | [ ] |
| [SS3IO-23](#ss3io-23) | 1754 | H/M | wrong index (region) | `WriteSSCtl()` and `WriteSSFor()` set `allregion = 0`, but the sum over regions is stored at index `NumRegions… |  | [ ] |
| [SS3IO-24](#ss3io-24) | 1780, 1786 | H/M | off-by-one / dead data | Writes `NblockperPatt[i]` blocks per pattern but loops `j < NblockperPatt[i]-1` with `Blocks[i][j*2+1], Blocks… |  | [ ] |
| [SS3IO-25](#ss3io-25) ⭐ | 1875 | H/H | data file format | `Mat_slope_Fem` line `" -3 3 %f %f %f 0.8 0 -3 0 0 0 0 0 0 0"` has 15 fields (three `%f` plus 0.8), whereas ea… |  | [ ] |
| [SS3IO-26](#ss3io-26) | 1926 | H/M | wrong operator/precedence | `vbk = log(VBk[0][1])/VBk[0][0];` — the male K offset should be `log(VBk[0][1]/VBk[0][0])` (cf. 1863, 1874). W… |  | [ ] |
| [SS3IO-27](#ss3io-27) | 2091, 1589 | H/M | wrong index / overrun | `speciesRPFleetToMetier[f][sp+1]` — array is `[K_num_fisheries][K_num_tot_sp]` (atAssessTierSetup.c:53) and ev… |  | [ ] |
| [SS3IO-28](#ss3io-28) ⭐ | 2189 | H/H | ctl file format | Age-selex pattern lines for fishing fleets (`"%d 0 0 0 # FLEET %d in metier %s"`) have no `\n`, so the next fl… |  | [ ] |
| [SS3IO-29](#ss3io-29) | 2162 | M/L | ctl file format | Mirrored size-selex line `"%d %d 0 %d "` has no `\n` (no `#` so tokens still parse, but the next fleet's line … |  | [ ] |
| [SS3IO-30](#ss3io-30) ⭐ | 2447 | H/H | uninitialised field / division by zero | `cat[f] = CatchData[f][allregion][maxyr] * RBCspeciesArray[sp].mgtTAC_old / last_catch;` — `mgtTAC_old` (atlan… |  | [ ] |
| [SS3IO-31](#ss3io-31) ⭐ | 2448 | H/H | ctl file format | `fprintf(fid, "%d 1 %d %f\n,", ...)` — stray comma after the newline, so each following line starts with `,` (… |  | [ ] |
| [SS3IO-32](#ss3io-32) | 2585, 2551, 2834 | H/L | copy-paste / wrong index | (Dead; `Write_SS_Control_File_Orig` is a verbatim copy.) Male parameters are all computed from FEMALE values (… |  | [ ] |
| [SS3IO-33](#ss3io-33) | 3056, 3448, 3101 | H/L | logic / overrun | (Dead.) `s` loops `0..Nsex_samp-1` with `Nsex_samp = 1` but vectors are only written for `s == 1` / `s == 2` (… |  | [ ] |

## Reviewer notes for this file

Context: the live path is `TierOne()` -> `SS330Assessment()` (atmanage/atSS3assess.c) -> `WriteSS330Files()` (-> `WriteSSDat`, `WriteSSCtl`, `WriteSSFor`, `Create_Starter_File`), then `Run_SS3()`, `Read_SS3_Par_File()`, `Read_SS3_Report_File()`. `WriteSSFiles()` (atSS3Link.c) is commented out in `TierOne()`, so `Write_Forecast_File`, `Write_SS_Data_File`, `Write_SS_Control_File`, the `*_Orig` functions and `Read_SS3_Report_File_EstBo_Only` are currently unreachable (dead). Findings in dead code are rated Sev L.
Functions checked: Read_SS3_Report_File, Read_SS3_Report_File_EstBo_Only, Read_SS3_Par_File, Create_Starter_File, Create_Starter_File_Orig, Write_Forecast_File, Write_SS_Data_File, WriteSS330Files, WriteSSDat, WriteSSCtl, WriteSSFor, Write_SS_Control_File, Write_SS_Data_File_Orig, Write_SS_Control_File_Orig.

---

## SS3IO-01

**NULL dereference** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atSS3LinkIO.c` 437, `atSS3LinkIO.c` 92, `atSS3LinkIO.c` 46
**Lines at review (Oct 2026):** 61-72 (also 219-230)

**Function:** `Read_SS3_Report_File()`

**Why it's thought to be a bug**

`if ((fid = fopen(...)) == NULL) { ... AssessFail_id = 1; }` then falls through to `while (fgets(buf, buflen, fid) ...)` with `fid == NULL` -> crash whenever SS3 fails to produce Report.sso (the very case the warning is for). Function: `Read_SS3_Report_File()` (same in `Read_SS3_Report_File_EstBo_Only()`). `Read_SS3_Par_File()` correctly returns.

**Code at issue** — `atSS3LinkIO.c` line 437  
Search for: `if ((fid = fopen(`

```c
  435: 	printf("fileName = %s\n", fileName);
  436: 
  437→ 	if ((fid = fopen(fileName, "w")) == NULL)
  438: 		quit("Create_Starter_File: Can't open %s\n", fileName);
  439: 
```

**Code at issue** — `atSS3LinkIO.c` line 92  
Search for: `while (fgets(buf, buflen, fid)`

```c
   90: 
   91: 	/* Find line that starts with 'FORECAST:_With_F_to_match_adjusted_catch' */
   92→ 	while (fgets(buf, buflen, fid) != NULL) {
   93: 
   94: 		if (strstr(buf, "FORECAST:_With_F_to_match_adjusted_catch") != NULL) {
```

**Code at issue** — `atSS3LinkIO.c` line 46  
Search for: `fid == NULL`

```c
   44: 		fid = fopen(fullName, "r");
   45: 	}
   46→ 	if (fid == NULL)
   47: 		fid = fopen(fileName, "r");
   48: 
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
**Found at (current code):** `atSS3LinkIO.c` 54, `atSS3LinkIO.c` 374
**Lines at review (Oct 2026):** 36, 61; 302, 315

**Why it's thought to be a bug**

Both readers open bare `"Report.sso"` / `"ss3.par"` in the current working dir and ignore `folderName`. In the live path `Run_SS3()` (atSS3assess.c:≈190) does `chdir(run_dir)` and then `chdir(cwd)` back before `Read_SS3_Par_File`/`Read_SS3_Report_File` are called (atSS3assess.c:118-124), so the files SS3 just wrote in `<sp>_SS3_sim_..._year_...` are never read (either "can't open" -> SS3IO-01 crash / par=1000 -> assessment always failed, or a stale file from cwd is read). The old `WriteSSFiles()` path did chdir before reading, the new one doesn't. Functions: `Read_SS3_Report_File()`, `Read_SS3_Par_File()`.

**Code at issue** — `atSS3LinkIO.c` line 54  
Search for: `"Report.sso"`

```c
   52: double Read_SS3_Report_File(MSEBoxModel *bm, int species, int year, char *folderName) {
   53: 
   54→ 	char outputFileName[] = "Report.sso";
   55: 
   56: 	FILE *fid;
```

**Code at issue** — `atSS3LinkIO.c` line 374  
Search for: `"ss3.par"`

```c
  372: double Read_SS3_Par_File(MSEBoxModel *bm, int sp, int year, char *folderName) {
  373:     
  374→ 	char outputFileName[] = "ss3.par";
  375:     
  376: 	FILE *fid;
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
**Found at (current code):** `atSS3LinkIO.c` 107, `atSS3LinkIO.c` 112
**Lines at review (Oct 2026):** 85-104 (also 243-256)

**Function:** `Read_SS3_Report_File()`

**Why it's thought to be a bug**

`varStr = strtok(NULL, seps); n++; if (n == EstBcurrIndex) ... atof(varStr)` — the NULL check is done before the strtok, so on a short/blank line `atof(NULL)` is called. Function: `Read_SS3_Report_File()`.

**Code at issue** — `atSS3LinkIO.c` line 107 (closest match to the quoted code; wording differs slightly)  
Search for: `varStr = strtok(NULL, seps);`

```c
  105: 			while (varStr != NULL) {
  106: 
  107→ 				varStr = strtok(NULL, seps);
  108: 				n++;
  109:                 
```

**Code at issue** — `atSS3LinkIO.c` line 112  
Search for: `atof(varStr)`

```c
  110:                 /*
  111:                 if ( n == year) {
  112→                     yy = atof(varStr);
  113:                 }
  114:                  */
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
**Found at (current code):** `atSS3LinkIO.c` 214, `atSS3LinkIO.c` 225, `atSS3LinkIO.c` 59
**Lines at review (Oct 2026):** 41, 146-160

**Function:** `Read_SS3_Report_File()`

**Why it's thought to be a bug**

In the `Biology_at_age_in_endyr_with_CV=f(LAA)` block no lines are ever read: `strtok(buffer, seps)` tokenises the never-initialised local `char buffer[1000]`, so `values[12..16]` are garbage (or uninitialised malloc memory). Also the continuation separator is `"\n"` not `seps`, so after the first token the rest of the line would be a single token; and `j` is unbounded against `values[100]`. Function: `Read_SS3_Report_File()`.

**Code at issue** — `atSS3LinkIO.c` line 214  
Search for: `Biology_at_age_in_endyr_with_CV=f(LAA)`

```c
  212:         
  213:         // Check on fit
  214→         if (strstr(buf, "Biology_at_age_in_endyr_with_CV=f(LAA)") != NULL) {
  215:             //found_LAA = 1;
  216:             
```

**Code at issue** — `atSS3LinkIO.c` line 225  
Search for: `strtok(buffer, seps)`

```c
  223: 
  224:                     // Assign values
  225→                     for (tok = strtok(buffer, seps); tok && *tok; j++, tok = strtok(NULL, "\n")) {
  226:                         values[j] = atof(tok);
  227:                         //fprintf(bm->logFile, "%f\t", values[j]);
```

**Code at issue** — `atSS3LinkIO.c` line 59  
Search for: `char buffer[1000]`

```c
   57: 	int buflen = STRLEN;
   58: 	char buf[STRLEN];
   59→     char buffer[1000]; // TODO: Set this to sensible value
   60: 	char seps[] = " ";
   61:     int n, j, thisID;
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
**Found at (current code):** `atSS3LinkIO.c` 218, `atSS3LinkIO.c` 237
**Lines at review (Oct 2026):** 149-153, 169-179

**Function:** `Read_SS3_Report_File()`

**Why it's thought to be a bug**

`for (age = 0; age < RBCspeciesParam[AccumAge_id]; age++) ... MeanLenAgeM[0][sex][age][thisID]` — MeanLenAgeM/SigmaLenAgeM/MeanWtAgeS/MeanWtAgeM are allocated `Util_Alloc_Init_4D_Double(nYears, MaxAge, Nsexes, stocks)` => `[stocks][Nsexes][MaxAge][nYears]` (atAssessTierSetup.c:281-285), and AccumAge = MaxAge + ceil(MaxAge/4) (atAssessTierSetup.c:86) > MaxAge, so `age` overruns the age dimension. Function: `Read_SS3_Report_File()`.

**Code at issue** — `atSS3LinkIO.c` line 218 (closest match to the quoted code; wording differs slightly)  
Search for: `for (int age = 0; age < bm->RBCestimation.RBCspeciesParam[species][AccumAge_id]; age++) {`

```c
  216:             
  217:             for (int sex = 0; sex < bm->RBCestimation.RBCspeciesParam[species][Nsexes_id]; sex++) {
  218→                 for (int age = 0; age < bm->RBCestimation.RBCspeciesParam[species][AccumAge_id]; age++) {
  219:                 
  220:                     const char* tok;
```

**Code at issue** — `atSS3LinkIO.c` line 237  
Search for: `MeanLenAgeM[0][sex][age][thisID]`

```c
  235:                     fprintf(bm->logFile,"Time: %e %s sex %d age %d Len_mid: %e Len_SD: %e wt_beg: %e  wt_mid: %e\n",bm->dayt, FunctGroupArray[species].groupCode, sex, age, Len_mid, Len_SD, wt_beg, wt_mid);
  236:                     
  237→                     if ((fabs(bm->RBCestimation.RBCspeciesArray[species].MeanLenAgeM[0][sex][age][thisID] - Len_mid) / Len_mid) > bm->RBCestimation.ConvergeThresh) {
  238:                         fprintf(bm->logFile,"Time: %e %s sex %d age %d - problem with  Len_mid: %e vs MeanLenAge: %e\n", bm->dayt, FunctGroupArray[species].groupCode, sex, age, Len_mid, bm->RBCestimation.RBCspeciesArray[species].MeanLenAgeM[0][sex][age][thisID]);
  239:                     }
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
**Found at (current code):** `atSS3LinkIO.c` 255
**Lines at review (Oct 2026):** 187-188

**Function:** `Read_SS3_Report_File()`

**Why it's thought to be a bug**

"Something has gone wrong reading in the SS3 report file" is printed/logged unconditionally after the loop, including on success. Function: `Read_SS3_Report_File()`.

**Code at issue** — `atSS3LinkIO.c` line 255 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `printf("%s Something has gone wrong reading in the SS3 report file. Look at the Report.sso`

```c
  253: 	}
  254:     
  255→     printf("%s Something has gone wrong reading in the SS3 report file. Look at the Report.sso: Biology_at_age_in_endyr_with_CV=f(LAA)\n", FunctGroupArray[species].groupCode);
  256:     fprintf(bm->logFile,"Time: %e %s Something has gone wrong reading in the SS3 report file. Look at the Report.sso: Biology_at_age_in_endyr_with_CV=f(LAA)\n", bm->dayt, FunctGroupArray[species].groupCode);
  257: 
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
**Found at (current code):** `atSS3LinkIO.c` 271
**Lines at review (Oct 2026):** 72-194; 230-292; 326-345

**Why it's thought to be a bug**

`fid` is never `fclose`d in `Read_SS3_Report_File()`, `Read_SS3_Report_File_EstBo_Only()` or `Read_SS3_Par_File()`. Two descriptors leak per species per assessment year; on long multi-species runs this can hit the per-process FD limit, after which fopen fails (-> SS3IO-01 crash).

**Code at issue** — `atSS3LinkIO.c` line 271 (located by name only; check this is the right place)  
Search for: `double Read_SS3_Report_File_EstBo_Only(MSEBoxModel *bm, int species, int year, char *folde`

```c
  269:  *
  270:  */
  271→ double Read_SS3_Report_File_EstBo_Only(MSEBoxModel *bm, int species, int year, char *folderName) {
  272: 
  273:     char outputFileName[] = "Report.sso";
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
**Found at (current code):** `atSS3LinkIO.c` 397
**Lines at review (Oct 2026):** 323-338

**Function:** `Read_SS3_Par_File()`

**Why it's thought to be a bug**

Searches for `MGparm[16]`, then "ignore the next line" and reads the line after. In an ADMB `.par` file each `# MGparm[16]:` header is followed directly by its value line, so the code skips the value and reads the next `# MGparm[17]:` header -> `atof("#") = 0` -> `MaxConvergCrit` is always 0 and the convergence test in `SS330Assessment()` always passes. Also MGparm[16] is a biology parameter, not a convergence measure (the max gradient is on the `.par` header line "Maximum gradient component = ..."). Function: `Read_SS3_Par_File()`.

**Code at issue** — `atSS3LinkIO.c` line 397  
Search for: `MGparm[16]`

```c
  395:     snprintf(str, sizeof(str), "MGparm[%d]", maxConvergCritIndex);
  396:     
  397→ 	/* Find line that starts with 'MGparm[16]' - as that marks the convergence result */
  398: 	while (fgets(buf, buflen, fid) != NULL) {
  399:         
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
**Found at (current code):** `atSS3LinkIO.c` 448
**Lines at review (Oct 2026):** 359, 374-375

**Function:** `Create_Starter_File()`

**Why it's thought to be a bug**

Starter file writes `"%s%s.dat", groupCode, fileName` where `fileName` is the starter's own path (`"<dir>//starter.ss"`) -> e.g. `FISdir//starter.ss.dat`. The real files written by `WriteSS330Files()` are `<dir>/<groupCode>WriteSS330Files.dat/.ctl` (lines 1216, 1225), and SS3 runs inside `<dir>`, so SS3 cannot find its data/control files. Function: `Create_Starter_File()`.

**Code at issue** — `atSS3LinkIO.c` line 448 (closest match to the quoted code; wording differs slightly)  
Search for: `fprintf(fid, "%s%s.dat\n", FunctGroupArray[groupIndex].groupCode, fileName);`

```c
  446: 	//fprintf(fid, "atlantisSS3_%d.ctl\n", versionID);
  447: 
  448→ 	fprintf(fid, "%s%s.dat\n", FunctGroupArray[groupIndex].groupCode, fileName);
  449: 	fprintf(fid, "%s%s.ctl\n", FunctGroupArray[groupIndex].groupCode, fileName);
  450: 
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
**Found at (current code):** `atSS3LinkIO.c` 471
**Lines at review (Oct 2026):** 397-398

**Function:** `Create_Starter_File()`

**Why it's thought to be a bug**

`"1 # min age for calc of summary biomass"` is written twice. SS3.30 starter expects a single value here, so every subsequent entry (Depletion basis, fraction X, SPR basis, F units, ..., check value 3.30) is shifted by one and the starter fails its end-of-file check / is misread. Function: `Create_Starter_File()`.

**Code at issue** — `atSS3LinkIO.c` line 471 (closest match to the quoted code; wording differs slightly)  
Search for: `fprintf(fid, "1 # min age for calc of summary biomass\n");`

```c
  469:     fprintf(fid, "%e # final convergence criteria (e.g. 1.0e-04)\n", bm->RBCestimation.SSTol);  // 0.0001
  470:     fprintf(fid, "0 # retrospective year relative to end year (e.g. -4)\n");
  471→     fprintf(fid, "1 # min age for calc of summary biomass\n");
  472:     fprintf(fid, "1 # min age for calc of summary biomass\n");
  473:     fprintf(fid, "%d # Depletion basis:  denom is: 0=skip; 1=rel X*B0; 2=rel X*Bmsy; 3=rel X*B_styr\n", bm->RBCestimation.SSDepletionBasis); //1
```

**Suggested fix (from review):** Delete line 472

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
**Found at (current code):** `atSS3LinkIO.c` 497
**Lines at review (Oct 2026):** 419, 423

**Function:** `Create_Starter_File_Orig()`

**Why it's thought to be a bug**

`snprintf(fileName, sizeof(fileName), ...)` where `fileName` is a `char *` parameter -> `sizeof` is 8, path truncated to 7 chars. Function: `Create_Starter_File_Orig()` (dead).

**Code at issue** — `atSS3LinkIO.c` line 497  
Search for: `snprintf(fileName, sizeof(fileName),`

```c
  495:     FILE *fid;
  496: 
  497→     snprintf(fileName, sizeof(fileName), "%s/%sstarter.ss", dirName, FOLDER_SEP);
  498: 
  499:     printf("fileName = %s\n", fileName);
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
**Found at (current code):** `atSS3LinkIO.c` 632
**Lines at review (Oct 2026):** 552-561

**Function:** `Write_Forecast_File()`

**Why it's thought to be a bug**

`catch = CatchData[f][sumregion][maxyr] * TAC_old / last_catch` with no guard for `last_catch == 0`; fleets printed 0-based (`f`) where SS needs 1..Nfleets. Function: `Write_Forecast_File()` (dead).

**Code at issue** — `atSS3LinkIO.c` line 632 (closest match to the quoted code; wording differs slightly)  
Search for: `catch = bm->RBCestimation.RBCspeciesArray[groupIndex].CatchData[f][sumregion][maxyr] * bm-`

```c
  630: 
  631: 	for (f = 0; f < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; f++) {
  632→ 		catch = bm->RBCestimation.RBCspeciesArray[groupIndex].CatchData[f][sumregion][maxyr] * bm->RBCestimation.RBCspeciesParam[groupIndex][TAC_old_id]
  633: 				/ last_catch;
  634: 		fprintf(fid, " %d 1 %d %f\n", (int) bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id] + maxyr + 1, f, catch);
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
**Found at (current code):** `atSS3LinkIO.c` 764
**Lines at review (Oct 2026):** 733, 748, 774, 780, 942

**Function:** `Write_SS_Data_File()`

**Why it's thought to be a bug**

Several errors: 780 `for (f = 0; fleetIndex < NumFisheries; f++)` tests the stale `fleetIndex` (== NumFisheries after the loop at 690) so discard fleet specs are never written; 733 writes a stray line `"%e %e"` of NumFisheries/NumSurvey into the CPUE block; 748 CPUE obs `" %d 7 %e %e"` omits the fleet field; 774 writes NumFisheries as N_fleets_with_discard instead of `nfltdisc` (fixed in `WriteSSDat` 1326); 942 no newline after age bins. Function: `Write_SS_Data_File()` (dead).

**Code at issue** — `atSS3LinkIO.c` line 764 (closest match to the quoted code; wording differs slightly)  
Search for: `for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheri`

```c
  762: 
  763:     // loop over the fleets only as catch data is ignored for survey fleets
  764→ 	for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
  765: 
  766:         fprintf(fid, "-999 1 %d %e 0.01\n", fleetIndex, bm->RBCestimation.RBCspeciesArray[groupIndex].initialEquilCatch[fleetIndex]);
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
**Found at (current code):** `atSS3LinkIO.c` 1181, `atSS3LinkIO.c` 1230
**Lines at review (Oct 2026):** 1148/1155-1156, 2281; 1164-1165, 2380

**Why it's thought to be a bug**

`WriteSSCtl()` ends with `fclose(fid)` and `WriteSS330Files()` then calls `fclose(fidctl)` on the same FILE*; likewise `WriteSSFor()` closes and the caller closes `fidss` again. Double `fclose` is undefined behaviour (heap corruption / abort on glibc). `WriteSSDat()` correctly leaves closing to the caller. Functions: `WriteSSCtl()`, `WriteSSFor()`, `WriteSS330Files()`.

**Code at issue** — `atSS3LinkIO.c` line 1181  
Search for: `fclose(fid)`

```c
 1179:     fprintf(fid, "\n");
 1180: 
 1181→ 	fclose(fid);
 1182:     
 1183:     free(emptySex);
```

**Code at issue** — `atSS3LinkIO.c` line 1230  
Search for: `fclose(fidctl)`

```c
 1228:     }
 1229:     WriteSSCtl(bm, fidctl, sp, maxyr);
 1230→     fclose(fidctl);
 1231: 
 1232:     //  SS forecast file
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
**Found at (current code):** `atSS3LinkIO.c` 1296
**Lines at review (Oct 2026):** 1221-1222

**Function:** `WriteSSDat()`

**Why it's thought to be a bug**

Fleet definition line `" 1  -1  1  1  0  FLEET%d in metier %s\n"` has no `#`, so SS reads fleetname=`FLEET0` and then `in`, `metier`, `<code>` as the start of the next fleet's definition -> whole data file misparsed. Because `speciesRPFleetToMetier` is never populated (all 0, so `> -1` is always true) this branch is always taken. The earlier `Write_SS_Data_File()` (line 740) has `# %d in metier`. Function: `WriteSSDat()`.

**Code at issue** — `atSS3LinkIO.c` line 1296  
Search for: `" 1  -1  1  1  0  FLEET%d in metier %s\n"`

```c
 1294:         if(i < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]){
 1295:             if (bm->RBCestimation.speciesRPFleetToMetier[i][sp] > -1) {
 1296→                 fprintf(fid, " 1  -1  1  1  0  FLEET%d in metier %s\n", i, FisheryArray[i].fisheryCode);
 1297:             } else {
 1298:                 fprintf(fid, " 1  -1  1  1  0  FLEET%d\n", i);
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
**Found at (current code):** `atSS3LinkIO.c` 2814
**Lines at review (Oct 2026):** 1222-1300, 1333, 1346, 1446, 1574, 2018-2023, 2037-2039, 2240-2255

**Why it's thought to be a bug**

All SS fleet numbers are written 0-based (`f`/`i`): catch lines `-999 1 %d`, CPUE setup `%d 1 0 0`, CPUE/discard/length/age observations, Q setup, Q parms, variance adjustments. SS3.30 numbers fleets 1..Nfleets, so fleet 0 is invalid and every observation is attributed to the previous fleet (the last survey is never referenced). The `*_Orig` writers used `fleetIndex + 1` (e.g. 3087-3110, 3269). Functions: `WriteSSDat()`, `WriteSSCtl()`.

**Code at issue** — `atSS3LinkIO.c` line 2814  
Search for: `fleetIndex + 1`

```c
 2812: 
 2813: 	for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
 2814→ 		fprintf(fid, " 0 0 0 0 # 1 FISHERY%d\n", fleetIndex + 1);
 2815: 	}
 2816: 
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
**Found at (current code):** `atSS3LinkIO.c` 1324, `atSS3LinkIO.c` 1316
**Lines at review (Oct 2026):** 1250-1255

**Function:** `WriteSSDat()`

**Why it's thought to be a bug**

`fprintf(fid, "%d 1 %d", t, f)` has no trailing space and the next `fprintf(fid, "%e ", catch)` has no leading space, so fleet and catch merge (e.g. `1980 1 01.234e+02  0.01`) -> 4 tokens instead of 5, and the whole catch block is misread. `Write_SS_Data_File()` (line 770) has `"%d 1 %d "`. Function: `WriteSSDat()`.

**Code at issue** — `atSS3LinkIO.c` line 1324  
Search for: `fprintf(fid, "%d 1 %d", t, f)`

```c
 1322:         // t is the year, f is fleet
 1323:         for (t = bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]; t < maxyr; t++){
 1324→             fprintf(fid, "%d 1 %d", t, f);
 1325:             // Apply proportion catch rule for closed area scenarios
 1326:             if ((bm->RBCestimation.RBCspeciesArray[sp].mgt_Retro_catch && (bm->RBCestimation.RBCspeciesParam[sp][PropClosed_id] > 0)) && (t < bm->RBCestimation.RBCspeciesParam[sp][HistYrMax_id])){   // allocate historic catches to open area by proportion open
```

**Code at issue** — `atSS3LinkIO.c` line 1316 (closest match to the quoted code; wording differs slightly)  
Search for: `fprintf(fid, "#_NOTE:  catch data is ignored for survey fleets\n");`

```c
 1314:     fprintf(fid, "#_Catch data: yr, seas, fleet, catch, catch_se\n");
 1315:     fprintf(fid, "#_catch_se:  standard error of log(catch)\n");
 1316→     fprintf(fid, "#_NOTE:  catch data is ignored for survey fleets\n");
 1317: 
 1318:     // loop over the fleets only as catch data is ignored for survey fleets
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
**Found at (current code):** `atSS3LinkIO.c` 1354
**Lines at review (Oct 2026):** 1278, 1280 (also 1300, 1346)

**Function:** `WriteSSDat()`

**Why it's thought to be a bug**

CPUE setup lines for non-metier fleets and all surveys (`" %d 1  0  0  # SURVEY%d"`) have no `\n`; the next fleet's setup then sits after the `#` on the same line and is ignored by SS, so with 2+ surveys the specs are missing and the data file is misread. (1300 CPUE obs and 1346 discard obs also lack `\n`; harmless token-wise but they merge with the following line.) Function: `WriteSSDat()`.

**Code at issue** — `atSS3LinkIO.c` line 1354  
Search for: `" %d 1  0  0  # SURVEY%d"`

```c
 1352:                 fprintf(fid, " %d 1  0  0  # FLEET%d", i, i);
 1353:         } else {
 1354→             fprintf(fid, " %d 1  0  0  # SURVEY%d", i, i - (int)(bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]));
 1355:         }
 1356:     }
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
**Found at (current code):** `atSS3LinkIO.c` 1292, `atSS3LinkIO.c` 1365, `atSS3LinkIO.c` 1488
**Lines at review (Oct 2026):** 1218-1528, 2012-2255

**Why it's thought to be a bug**

Loops run `f < NumFisheries + NumSurvey` and index arrays sized `K_num_fisheries` in the fleet dimension: `CPUEgen[f]`, `CPUEcv[f]`, `LengthFltYr[it][f]`, `LFss[f]`, `LenComp[f]` (atAssessTierSetup.c:149-170, all `[..K_num_fisheries..]`), and in the ctl `CPUEqmu[f]`, `Sel_Pattern[f]`, `mirrored_fleet[f]`, `Start_Sel[f]`, `Sel_Phase[f]`, `SelBlock[f]`, `Varadj_*[f]` (allocated K_num_fisheries / `[K_num_fisheries][K_num_tot_sp]`). NumFisheries is capped near K_num_fisheries (atAssessTierSetup.c:1478-1482) so any surveys push `f` past the end. Functions: `WriteSSDat()`, `WriteSSCtl()`.

**Code at issue** — `atSS3LinkIO.c` line 1292 (closest match to the quoted code; wording differs slightly)  
Search for: `for (i = 0; i < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] + bm->RBCestimation`

```c
 1290:     // loop over fleets and surveys
 1291:     // We assume that fleets always come before surveys
 1292→     for (i = 0; i < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]; i++){
 1293:         //add the fleet type
 1294:         if(i < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]){
```

**Code at issue** — `atSS3LinkIO.c` line 1365  
Search for: `CPUEgen[f]`

```c
 1363:     for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]; f++) {
 1364:         for (t = bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]; t < maxyr; t++) {
 1365→             if (bm->RBCestimation.RBCspeciesArray[sp].CPUEgen[f][allregion][t] > 0.0) {
 1366:                 if (!bm->RBCestimation.RBCspeciesArray[sp].CPUEcv[f]) {   // generate cpue with no error, but still want cv in SS
 1367:                     cpcv = 0.2;
```

**Code at issue** — `atSS3LinkIO.c` line 1488  
Search for: `LengthFltYr[it][f]`

```c
 1486:             for (it = 0; it < 3; it++) {
 1487:                 for (s = 0; s < bm->RBCestimation.RBCspeciesParam[sp][Nsex_samp_id]; s++) {
 1488→                     if ((bm->RBCestimation.RBCspeciesArray[sp].LengthFltYr[it][f][t] > 0)) {
 1489:                         if (bm->RBCestimation.RBCspeciesArray[sp].LFss[f][s][t][it] > bm->RBCestimation.RBCspeciesParam[sp][LFSSlim_id]) {
 1490:                             num++;
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
**Found at (current code):** `atSS3LinkIO.c` 1678
**Lines at review (Oct 2026):** 1486-1491, 1577-1590

**Function:** `WriteSSDat()`

**Why it's thought to be a bug**

Writes `MaxAge+1` as `#_N_age_bins` but then only `MaxAge` bin labels (`a < MaxAge`) and `MaxAge` values per age-comp vector. SS reads MaxAge+1 bin edges (consuming the `1` of `#_N_ageerror_definitions`) and every age-comp row is one value short -> file misparsed. Function: `WriteSSDat()` (same pattern in `Write_SS_Data_File()` 936-940, 1032-1045).

**Code at issue** — `atSS3LinkIO.c` line 1678  
Search for: `MaxAge+1`

```c
 1676:     // terminator for the age data
 1677:     fprintf(fid, "-9999 ");
 1678→     // loop to create 2*n_age_bins +9 zeroes, where n_age_bins=MaxAge+1 **update the equation**
 1679:     for (i = 0; i < 2*bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]+10; i++) {
 1680:         fprintf(fid, "0 ");
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
**Found at (current code):** `atSS3LinkIO.c` 1567, `atSS3LinkIO.c` 1571
**Lines at review (Oct 2026):** 1493-1499 (also 943-949, 3323-3334)

**Function:** `WriteSSDat()`

**Why it's thought to be a bug**

Ageing-error definition rows must have Nages+1 entries (ages 0..accumulator age); loops write `a < AccumAge` = AccumAge entries. `Ageing_error` is allocated `AccumAge+1` "dimensions 0..AccumAge (SS data file)" (atAssessTierSetup.c:175), and the comment on 1493 says "accumulator age + 1". Each row is one short so the following data is shifted. Function: `WriteSSDat()`.

**Code at issue** — `atSS3LinkIO.c` line 1567 (the line the review pointed to, found by its text)  
Search for: `for (a = 0; a < bm->RBCestimation.RBCspeciesParam[sp][AccumAge_id]; a++) {     // bm->RBCe`

```c
 1565:     fprintf(fid, "\n");
 1566:     fprintf(fid, "1   #_N_ageerror_definitions\n");
 1567→     for (a = 0; a < bm->RBCestimation.RBCspeciesParam[sp][AccumAge_id]; a++) {     // bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id] +6 is accumulator age + 1
 1568:         fprintf(fid, "%f ", a+0.5);
 1569:     }
```

**Code at issue** — `atSS3LinkIO.c` line 1571 (the line the review pointed to, found by its text)  
Search for: `for (a = 0; a < bm->RBCestimation.RBCspeciesParam[sp][AccumAge_id]; a++) {`

```c
 1569:     }
 1570:     fprintf(fid, "\n");
 1571→     for (a = 0; a < bm->RBCestimation.RBCspeciesParam[sp][AccumAge_id]; a++) {
 1572:         fprintf(fid, "%f ", bm->RBCestimation.RBCspeciesArray[sp].Ageing_error[a]);
 1573:     }
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
**Found at (current code):** `atSS3LinkIO.c` 1486
**Lines at review (Oct 2026):** 1187, 1561-1566

**Function:** `WriteSSDat()`

**Why it's thought to be a bug**

Age-comp partition uses `it==1 -> 2, it==2 -> 0, it==3 -> 1` but `it` runs 0..2: `it==0` (retained) leaves `part` unset (uninitialised in this function — declared without init at 1187) or stale, `it==1` (whole) is labelled retained, `it==2` (discard) labelled whole. The length block a few lines up (1507-1513) uses the correct `!it -> 2, 1 -> 0, else 1`. Function: `WriteSSDat()`.

**Code at issue** — `atSS3LinkIO.c` line 1486 (closest match to the quoted code; wording differs slightly)  
Search for: `for (it = 0; it < 3; it++) {`

```c
 1484:     for (t = bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]; t < maxyr; t++) {
 1485:         for (f = 0; f < (bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]); f++) {
 1486→             for (it = 0; it < 3; it++) {
 1487:                 for (s = 0; s < bm->RBCestimation.RBCspeciesParam[sp][Nsex_samp_id]; s++) {
 1488:                     if ((bm->RBCestimation.RBCspeciesArray[sp].LengthFltYr[it][f][t] > 0)) {
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
**Found at (current code):** `atSS3LinkIO.c` 1754
**Lines at review (Oct 2026):** 1680, 2065-2071; 2365-2373

**Why it's thought to be a bug**

`WriteSSCtl()` and `WriteSSFor()` set `allregion = 0`, but the sum over regions is stored at index `NumRegions` (`CatchData`/`DiscData` allocated `K_num_reg+1` "so have sum over all regions"; `WriteSSDat()` 1192 and atManageTier.c:700 use `Nregions`). So retention parameters (`dflt`) are decided from region 0 only while the dat file's discard fleets come from the all-region sum (mismatched parameter count -> SS error), and forecast catch splits use region 0 only.

**Code at issue** — `atSS3LinkIO.c` line 1754  
Search for: `allregion = 0`

```c
 1752:     double sv, mval, lo, hi;
 1753:     int *dflt;
 1754→     int allregion = 0;    // uses weighted sum over regions, assume assessment doesn't know about regions
 1755:     
 1756:     // initial header block
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
**Found at (current code):** `atSS3LinkIO.c` 1780, `atSS3LinkIO.c` 1786
**Lines at review (Oct 2026):** 1705-1713

**Function:** `WriteSSCtl()`

**Why it's thought to be a bug**

Writes `NblockperPatt[i]` blocks per pattern but loops `j < NblockperPatt[i]-1` with `Blocks[i][j*2+1], Blocks[i][j*2+2]` (1-based leftover) -> one begin/end pair too few, so the ctl is misread when `NblockPattern > 0`. Also `Blocks` and `NblockperPatt` are allocated (atAssessTierSetup.c:287-288) but never filled anywhere in the repo. Function: `WriteSSCtl()`.

**Code at issue** — `atSS3LinkIO.c` line 1780  
Search for: `NblockperPatt[i]`

```c
 1778:         fprintf(fid, "%d #_Nblock_Patterns\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][NblockPattern_id]));
 1779:         for (i = 0; i < bm->RBCestimation.RBCspeciesParam[sp][NblockPattern_id]; i++) {
 1780→             fprintf(fid, "%d  ", bm->RBCestimation.RBCspeciesArray[sp].NblockperPatt[i]);
 1781:         }
 1782:         fprintf(fid, "        # Blocks per pattern\n");
```

**Code at issue** — `atSS3LinkIO.c` line 1786 (closest match to the quoted code; wording differs slightly)  
Search for: `fprintf(fid, "%f %f\n", bm->RBCestimation.RBCspeciesArray[sp].Blocks[i][j*2+1], bm->RBCest`

```c
 1784:         for (i=0; i < bm->RBCestimation.RBCspeciesParam[sp][NblockPattern_id]; i++) {
 1785:             for (j=0; j< bm->RBCestimation.RBCspeciesArray[sp].NblockperPatt[i]-1; j++) {
 1786→                 fprintf(fid, "%f %f\n", bm->RBCestimation.RBCspeciesArray[sp].Blocks[i][j*2+1], bm->RBCestimation.RBCspeciesArray[sp].Blocks[i][j*2+2]);
 1787:             }
 1788:         }
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
**Found at (current code):** `atSS3LinkIO.c` 1875
**Lines at review (Oct 2026):** 1801

**Function:** `WriteSSCtl()`

**Why it's thought to be a bug**

`Mat_slope_Fem` line `"    -3     3 %f %f %f     0.8    0   -3  0 0 0 0 0 0 0"` has 15 fields (three `%f` plus 0.8), whereas each MG parameter line takes 14 (cf. Mat50 line 1874). Every later ctl entry is shifted by one token. Function: `WriteSSCtl()`.

**Code at issue** — `atSS3LinkIO.c` line 1875 (the line the review pointed to, found by its text)  
Search for: `fprintf(fid, "    -3     3 %f %f %f     0.8    0   -3       0       0         0         0 `

```c
 1873:     fprintf(fid, "# Sex: 1  BioPattern: 1  Maturity&Fecundity\n");
 1874:     fprintf(fid, "%f %f %f %f     10    0   -3       0       0         0         0        0     0     0     # Mat50_Fem\n", bm->RBCestimation.RBCspeciesParam[sp][Maturity_Inflect_id]/2.0, bm->RBCestimation.RBCspeciesParam[sp][Maturity_Inflect_id]*2.0, bm->RBCestimation.RBCspeciesParam[sp][Maturity_Inflect_id], bm->RBCestimation.RBCspeciesParam[sp][Maturity_Inflect_id]);
 1875→     fprintf(fid, "    -3     3 %f %f %f     0.8    0   -3       0       0         0         0        0     0     0     # Mat_slope_Fem\n", bm->RBCestimation.RBCspeciesParam[sp][Maturity_Slope_id], bm->RBCestimation.RBCspeciesParam[sp][Maturity_Slope_id], bm->RBCestimation.RBCspeciesParam[sp][Maturity_Slope_id]);
 1876:     //* the next two parameters were labeled egg/gm in the 3.24 version, howeverit doesn't look like they're doing anything important
 1877:      fprintf(fid, "    -3     3        1        1       0.8   0   -3       0       0         0         0        0     0     0     #   Eggs/kg_inter_Fem\n");
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
**Found at (current code):** `atSS3LinkIO.c` 1926
**Lines at review (Oct 2026):** 1852

**Function:** `WriteSSCtl()`

**Why it's thought to be a bug**

`vbk = log(VBk[0][1])/VBk[0][0];` — the male K offset should be `log(VBk[0][1]/VBk[0][0])` (cf. 1863, 1874). Wrong male K whenever Nsexes>1 and MG_offset>2. Function: `WriteSSCtl()`.

**Code at issue** — `atSS3LinkIO.c` line 1926 (closest match to the quoted code; wording differs slightly)  
Search for: `vbk = log(bm->RBCestimation.RBCspeciesArray[sp].VBk[0][1])/bm->RBCestimation.RBCspeciesArr`

```c
 1924:         fprintf(fid, "%f %f %f %f      10   0   -3       0       0         0         0        0     0     0     # L_at_Amax_Mal_GP_1\n", lo, hi, lmaxm, lmaxm);
 1925:         if (bm->RBCestimation.RBCspeciesParam[sp][MG_offset_id] > 2) {
 1926→             vbk = log(bm->RBCestimation.RBCspeciesArray[sp].VBk[0][1])/bm->RBCestimation.RBCspeciesArray[sp].VBk[0][0];
 1927:             lo = -1.0;
 1928:             hi = 1.0;
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
**Found at (current code):** `atSS3LinkIO.c` 2091, `atSS3LinkIO.c` 1589
**Lines at review (Oct 2026):** 2017, 2114

**Function:** `WriteSSCtl()`

**Why it's thought to be a bug**

`speciesRPFleetToMetier[f][sp+1]` — array is `[K_num_fisheries][K_num_tot_sp]` (atAssessTierSetup.c:53) and every other use in this file is `[i][sp]` (1295, 1349, 1459, 1589). `sp+1` reads the next species' column and overruns the row when `sp == K_num_tot_sp-1`. Function: `WriteSSCtl()`.

**Code at issue** — `atSS3LinkIO.c` line 2091  
Search for: `speciesRPFleetToMetier[f][sp+1]`

```c
 2089:     
 2090:         if( f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]){
 2091→             if (bm->RBCestimation.speciesRPFleetToMetier[f][sp+1] > -1) {
 2092:                 fprintf(fid,  "%d   1   0   0   0   0  # FLEET %d in metier %s\n", f, f, FisheryArray[f].fisheryCode);
 2093:             } else {
```

**Code at issue** — `atSS3LinkIO.c` line 1589  
Search for: `[i][sp]`

```c
 1587:     for (i = 0; i < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]; i++){
 1588:         if(i < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]){
 1589→             if(bm->RBCestimation.speciesRPFleetToMetier[i][sp] > -1) {
 1590:                 fprintf(fid, "0 1e-007 1 0 0 0 1  # FLEET%d in metier %s\n", i, FisheryArray[i].fisheryCode);
 1591:             } else {
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
**Found at (current code):** `atSS3LinkIO.c` 2189
**Lines at review (Oct 2026):** 2115, 2117

**Function:** `WriteSSCtl()`

**Why it's thought to be a bug**

Age-selex pattern lines for fishing fleets (`"%d  0  0  0  # FLEET %d in metier %s"`) have no `\n`, so the next fleet's/survey's `pattern 0 0 0` line lands after the `#` and is discarded by SS -> only one age-selex line is read for all fleets and the rest of the ctl is shifted. The survey branch (2194) has `\n`. Function: `WriteSSCtl()`.

**Code at issue** — `atSS3LinkIO.c` line 2189  
Search for: `"%d  0  0  0  # FLEET %d in metier %s"`

```c
 2187:         if (i < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]) {
 2188:             if (bm->RBCestimation.speciesRPFleetToMetier[i][sp+1] > -1) {
 2189→                 fprintf(fid, "%d  0  0  0  # FLEET %d in metier %s", (int)(bm->RBCestimation.RBCspeciesParam[sp][Agesel_Pattern_id]), i, FisheryArray[i].fisheryCode);
 2190:             } else {
 2191:                 fprintf(fid, "%d  0  0  0  # FLEET %d in metier", (int)(bm->RBCestimation.RBCspeciesParam[sp][Agesel_Pattern_id]), i);
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
**Found at (current code):** `atSS3LinkIO.c` 2162
**Lines at review (Oct 2026):** 2088

**Function:** `WriteSSCtl()`

**Why it's thought to be a bug**

Mirrored size-selex line `"%d %d 0 %d "` has no `\n` (no `#` so tokens still parse, but the next fleet's line is appended). Function: `WriteSSCtl()`.

**Code at issue** — `atSS3LinkIO.c` line 2162  
Search for: `"%d %d 0 %d "`

```c
 2160:                 quit(" --- Error --- \n  Len Selectivity pattern 5 specified for fleet %d\n but no fleet specified to mirror, \n i.e. bm->RBCestimation.RBCspeciesArray[sp].mirrored_fleet[f].\n ---------------\n", f);
 2161:             } else {
 2162→                 fprintf(fid, "%d %d 0 %d ", bm->RBCestimation.RBCspeciesArray[sp].Sel_Pattern[f], disc, bm->RBCestimation.RBCspeciesArray[sp].mirrored_fleet[f]);
 2163:             }
 2164:         }
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
**Found at (current code):** `atSS3LinkIO.c` 2447
**Lines at review (Oct 2026):** 2368-2373

**Function:** `WriteSSFor()`

**Why it's thought to be a bug**

`cat[f] = CatchData[f][allregion][maxyr] * RBCspeciesArray[sp].mgtTAC_old / last_catch;` — `mgtTAC_old` (atlantisboxmodel.h:3165) is never assigned anywhere in the repo, so forecast catches are 0 (or garbage); the TAC is kept in `RBCspeciesParam[sp][TAC_old_id]` (set atManageTier.c:147, 246; used by `Write_Forecast_File()` 558). Also no guard for `last_catch == 0` (NaN/inf written to forecast.ss). Function: `WriteSSFor()`.

**Code at issue** — `atSS3LinkIO.c` line 2447 (closest match to the quoted code; wording differs slightly)  
Search for: `cat[f] = bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][allregion][maxyr] * bm->RBCest`

```c
 2445:     }
 2446:     for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
 2447→         cat[f] = bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][allregion][maxyr] * bm->RBCestimation.RBCspeciesArray[sp].mgtTAC_old / last_catch;
 2448:          fprintf(fid, "%d 1 %d %f\n,", maxyr+1, f, cat[f]);
 2449:     }
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
**Found at (current code):** `atSS3LinkIO.c` 2448
**Lines at review (Oct 2026):** 2374

**Function:** `WriteSSFor()`

**Why it's thought to be a bug**

`fprintf(fid, "%d 1 %d %f\n,", ...)` — stray comma after the newline, so each following line starts with `,` (e.g. `,-9999 1 1 0`), corrupting the forecast catch list and its terminator. Function: `WriteSSFor()`.

**Code at issue** — `atSS3LinkIO.c` line 2448  
Search for: `fprintf(fid, "%d 1 %d %f\n,",`

```c
 2446:     for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
 2447:         cat[f] = bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][allregion][maxyr] * bm->RBCestimation.RBCspeciesArray[sp].mgtTAC_old / last_catch;
 2448→          fprintf(fid, "%d 1 %d %f\n,", maxyr+1, f, cat[f]);
 2449:     }
 2450:     fprintf(fid, "-9999 1 1 0  # terminator_for_Fcast_catches\n");
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
**Found at (current code):** `atSS3LinkIO.c` 2585, `atSS3LinkIO.c` 2551, `atSS3LinkIO.c` 2834
**Lines at review (Oct 2026):** 2383-2948, 3444-3981

**Why it's thought to be a bug**

(Dead; `Write_SS_Control_File_Orig` is a verbatim copy.) Male parameters are all computed from FEMALE values (2511 `log(SSMort[0][FEMALE]/SSMort[0][FEMALE])` = 0; 2523-2548, 2563, 2587/2591 use `[0][0]` for male CV_old); `Wtlen_2_Fem` written twice (2680, 2683) and male wt-len lines use FEMALE and are labelled `_Fem` (2697-2701); `bm->RBCestimation.RBCspeciesArray->VBt0` (2551, 2559) = species 0, not `groupIndex`; `dflt` loop (2834-2839) overwrites per year so only the last year counts and runs `yearIndex <= maxyr`; 2675-2679, 2798-2846 missing `\n` before `#` comments. Functions: `Write_SS_Control_File()`, `Write_SS_Control_File_Orig()`.

**Code at issue** — `atSS3LinkIO.c` line 2585 (closest match to the quoted code; wording differs slightly)  
Search for: `mval = log(fabs(bm->RBCestimation.RBCspeciesArray[groupIndex].SSMort[0][FEMALE]) / fabs(bm`

```c
 2583: 	if (Nsexes == 2) {
 2584: 		if (bm->RBCestimation.RBCspeciesParam[groupIndex][MG_offset_id] == 3) {
 2585→ 			mval = log(fabs(bm->RBCestimation.RBCspeciesArray[groupIndex].SSMort[0][FEMALE]) / fabs(bm->RBCestimation.RBCspeciesArray[groupIndex].SSMort[0][FEMALE]));
 2586: 			lo = -1.0;
 2587: 			hi = 1.0;
```

**Code at issue** — `atSS3LinkIO.c` line 2551  
Search for: `RBCestimation.RBCspeciesArray->VBt0`

```c
 2549:                 * (1.0 - exp( -1.0 * bm->RBCestimation.RBCspeciesArray[groupIndex].VBk[0][FEMALE]
 2550:                                 * (bm->RBCestimation.RBCspeciesParam[groupIndex][Growthage_L1_id]
 2551→                                 - bm->RBCestimation.RBCspeciesArray->VBt0[0][FEMALE])));
 2552: 
 2553: 	fprintf(fid, " %f %f %f %f 0 10 -3 0 0 0 0 0.5 0 0 # L_at_Amin_Fem_GP_1\n", lminf / 2.0, lminf * 2.0, lminf, lminf);
```

**Code at issue** — `atSS3LinkIO.c` line 2834  
Search for: `yearIndex <= maxyr`

```c
 2832: 	fprintf(fid, "#_Pattern Discard Male Special\n");
 2833: 
 2834→ 	for (yearIndex = HistYrMin; yearIndex <= maxyr; yearIndex++)
 2835: 		for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++)
 2836: 			if (bm->RBCestimation.RBCspeciesArray[groupIndex].DiscData[fleetIndex][allregion][yearIndex] > 0)
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
**Found at (current code):** `atSS3LinkIO.c` 3056, `atSS3LinkIO.c` 3448, `atSS3LinkIO.c` 3101
**Lines at review (Oct 2026):** 2966-3442

**Function:** `Write_SS_Data_File_Orig()`

**Why it's thought to be a bug**

(Dead.) `s` loops `0..Nsex_samp-1` with `Nsex_samp = 1` but vectors are only written for `s == 1` / `s == 2` (3346, 3353, 3446, 3453) so no comp data are ever written; `ageIndex <= MaxAge` (3448, 3458) reads `AgeComp[..][MaxAge]`, past the `OverallMaxAge` dimension when MaxAge == OverallMaxAge; `num` not reset before the length/age obs counts (3317, 3416) so counts include discards; 3134 prints stale `fleetIndex+1` instead of `nf+1`; environmental-variable header written twice (3493-3505). Function: `Write_SS_Data_File_Orig()`.

**Code at issue** — `atSS3LinkIO.c` line 3056  
Search for: `Nsex_samp = 1`

```c
 3054:     double cpcv, disccv;
 3055:     char *emptySex;
 3056→     double Nsex_samp = 1;
 3057:     int emptySexSize;
 3058: 
```

**Code at issue** — `atSS3LinkIO.c` line 3448 (closest match to the quoted code; wording differs slightly)  
Search for: `for (ageIndex = 0; ageIndex <= bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id]; a`

```c
 3446:                         if (s == 1) {    // combined or females - write female LFs then male (ignored)
 3447: 
 3448→                             for (ageIndex = 0; ageIndex <= bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id]; ageIndex++)
 3449:                                 fprintf(fid, " %d", bm->RBCestimation.RBCspeciesArray[groupIndex].AgeComp[fleetIndex][s][yearIndex][it][ageIndex]);
 3450: 
```

**Code at issue** — `atSS3LinkIO.c` line 3101  
Search for: `fleetIndex + 1`

```c
 3099: 
 3100:     for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
 3101→         snprintf(tempStr, sizeof(tempStr), "FISHERY%d", fleetIndex + 1);
 3102:         strcat(line1, tempStr);
 3103:         //snprintf(line1, sizeof(line1), "%sFISHERY%d", line1, fleetIndex + 1);
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

Year convention: `WriteSSDat`/`WriteSSCtl` write `HistYrMin` as `#_StartYr` and the caller's `year` (a relative index, cf. `RBC_by_year[year]` in atSS3assess.c) as `#_EndYr`, and loop `t = HistYrMin .. maxyr-1` indexing the year dimension (size nYears) with `t`. This only works if `HistYrMin` is set to 0 in the XML; if it's an AD year (as the comment at line 709 says SS needs) no data rows are written at all. Hard-coded years (`2000 # F ballpark year` at 1980, `HistYrMin-50` at 1952) assume AD years. Worth agreeing one convention.

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
