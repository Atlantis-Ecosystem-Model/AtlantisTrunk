# atSS3Link/atSS3LinkIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **33 findings** (severity H: 19; confidence H and severity H: 17; on the priority list: 17), plus 5 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SS3IO-01](#ss3io-01) ⭐ | ≈92 (also ≈300) | H/H | NULL dereference | `if ((fid = fopen(...)) == NULL) { ... AssessFail_id = 1; }` then falls through to `while (fgets(buf, buflen, … |  | [ ] |
| [SS3IO-02](#ss3io-02) ⭐ | 54, ≈80; 374, ≈388 | H/H | wrong path / logic | Both readers open bare `"Report.sso"` / `"ss3.par"` in the current working dir and ignore `folderName`. In the… |  | [ ] |
| [SS3IO-03](#ss3io-03) | 105-124 (also 313-326) | M/L | NULL dereference | `varStr = strtok(NULL, seps); n++; if (n == EstBcurrIndex) ... atof(varStr)` — the NULL check is done before t… |  | [ ] |
| [SS3IO-04](#ss3io-04) ⭐ | 59, 214-228 | H/H | uninitialised variable / logic | In the `Biology_at_age_in_endyr_with_CV=f(LAA)` block no lines are ever read: `strtok(buffer, seps)` tokenises… |  | [ ] |
| [SS3IO-05](#ss3io-05) ⭐ | 217-221, 237-247 | H/H | buffer overrun (index) | `for (age = 0; age < RBCspeciesParam[AccumAge_id]; age++) ... MeanLenAgeM[0][sex][age][thisID]` — MeanLenAgeM/… |  | [ ] |
| [SS3IO-06](#ss3io-06) | 255-256 | H/L | logic (log text) | "Something has gone wrong reading in the SS3 report file" is printed/logged unconditionally after the loop, in… |  | [ ] |
| [SS3IO-07](#ss3io-07) | 72-194; 230-292; 326-345 | H/M | resource leak | `fid` is never `fclose`d in `Read_SS3_Report_File()`, `Read_SS3_Report_File_EstBo_Only()` or `Read_SS3_Par_Fil… |  | [ ] |
| [SS3IO-08](#ss3io-08) | 395-410 | M/H | logic / parsing | Searches for `MGparm[16]`, then "ignore the next line" and reads the line after. In an ADMB `.par` file each `… |  | [ ] |
| [SS3IO-09](#ss3io-09) ⭐ | 433, 448-449 | H/H | wrong variable | Starter file writes `"%s%s.dat", groupCode, fileName` where `fileName` is the starter's own path (`"<dir>//sta… |  | [ ] |
| [SS3IO-10](#ss3io-10) ⭐ | 471-472 | H/H | copy-paste | `"1 # min age for calc of summary biomass"` is written twice. SS3.30 starter expects a single value here, so e… |  | [ ] |
| [SS3IO-11](#ss3io-11) | 493, 497 | H/L | buffer size | `snprintf(fileName, sizeof(fileName), ...)` where `fileName` is a `char *` parameter -> `sizeof` is 8, path tr… |  | [ ] |
| [SS3IO-12](#ss3io-12) | 626-635 | M/L | division by zero / fleet index | `catch = CatchData[f][sumregion][maxyr] * TAC_old / last_catch` with no guard for `last_catch == 0`; fleets pr… |  | [ ] |
| [SS3IO-13](#ss3io-13) | 807, 822, 848, 854, 1016 | H/L | copy-paste / wrong variable | Several errors: 780 `for (f = 0; fleetIndex < NumFisheries; f++)` tests the stale `fleetIndex` (== NumFisherie… |  | [ ] |
| [SS3IO-14](#ss3io-14) ⭐ | 1222/1229-1230, 2355; 1238-1239, 2454 | H/H | double free (fclose) | `WriteSSCtl()` ends with `fclose(fid)` and `WriteSS330Files()` then calls `fclose(fidctl)` on the same FILE*; … |  | [ ] |
| [SS3IO-15](#ss3io-15) ⭐ | 1295-1296 | H/H | data file format | Fleet definition line `" 1 -1 1 1 0 FLEET%d in metier %s\n"` has no `#`, so SS reads fleetname=`FLEET0` and th… |  | [ ] |
| [SS3IO-16](#ss3io-16) ⭐ | 1222-1300, 1333, 1346, 1446, 1574, 2018-2023, 2037-2039, 2240-2255 | H/H | off-by-one (fleet numbering) | All SS fleet numbers are written 0-based (`f`/`i`): catch lines `-999 1 %d`, CPUE setup `%d 1 0 0`, CPUE/disca… |  | [ ] |
| [SS3IO-17](#ss3io-17) ⭐ | 1324-1329 | H/H | data file format | `fprintf(fid, "%d 1 %d", t, f)` has no trailing space and the next `fprintf(fid, "%e ", catch)` has no leading… |  | [ ] |
| [SS3IO-18](#ss3io-18) | 1352, 1354 (also 1374, 1420) | H/M | data file format | CPUE setup lines for non-metier fleets and all surveys (`" %d 1 0 0 # SURVEY%d"`) have no `\n`; the next fleet… |  | [ ] |
| [SS3IO-19](#ss3io-19) | 1292-1602, 2086-2329 | M/H | buffer overrun (index) | Loops run `f < NumFisheries + NumSurvey` and index arrays sized `K_num_fisheries` in the fleet dimension: `CPU… |  | [ ] |
| [SS3IO-20](#ss3io-20) ⭐ | 1560-1565, 1651-1664 | H/H | off-by-one | Writes `MaxAge+1` as `#_N_age_bins` but then only `MaxAge` bin labels (`a < MaxAge`) and `MaxAge` values per a… |  | [ ] |
| [SS3IO-21](#ss3io-21) ⭐ | 1567-1573 (also 1017-1023, 3397-3408) | H/H | off-by-one | Ageing-error definition rows must have Nages+1 entries (ages 0..accumulator age); loops write `a < AccumAge` =… |  | [ ] |
| [SS3IO-22](#ss3io-22) ⭐ | 1261, 1635-1640 | H/H | copy-paste / uninitialised | Age-comp partition uses `it==1 -> 2, it==2 -> 0, it==3 -> 1` but `it` runs 0..2: `it==0` (retained) leaves `pa… |  | [ ] |
| [SS3IO-23](#ss3io-23) | 1754, 2139-2145; 2439-2447 | H/M | wrong index (region) | `WriteSSCtl()` and `WriteSSFor()` set `allregion = 0`, but the sum over regions is stored at index `NumRegions… |  | [ ] |
| [SS3IO-24](#ss3io-24) | 1779-1787 | H/M | off-by-one / dead data | Writes `NblockperPatt[i]` blocks per pattern but loops `j < NblockperPatt[i]-1` with `Blocks[i][j*2+1], Blocks… |  | [ ] |
| [SS3IO-25](#ss3io-25) ⭐ | 1875 | H/H | data file format | `Mat_slope_Fem` line `" -3 3 %f %f %f 0.8 0 -3 0 0 0 0 0 0 0"` has 15 fields (three `%f` plus 0.8), whereas ea… |  | [ ] |
| [SS3IO-26](#ss3io-26) | 1926 | H/M | wrong operator/precedence | `vbk = log(VBk[0][1])/VBk[0][0];` — the male K offset should be `log(VBk[0][1]/VBk[0][0])` (cf. 1863, 1874). W… |  | [ ] |
| [SS3IO-27](#ss3io-27) | 2091, 2188 | H/M | wrong index / overrun | `speciesRPFleetToMetier[f][sp+1]` — array is `[K_num_fisheries][K_num_tot_sp]` (atAssessTierSetup.c:53) and ev… |  | [ ] |
| [SS3IO-28](#ss3io-28) ⭐ | 2189, 2191 | H/H | ctl file format | Age-selex pattern lines for fishing fleets (`"%d 0 0 0 # FLEET %d in metier %s"`) have no `\n`, so the next fl… |  | [ ] |
| [SS3IO-29](#ss3io-29) | 2162 | M/L | ctl file format | Mirrored size-selex line `"%d %d 0 %d "` has no `\n` (no `#` so tokens still parse, but the next fleet's line … |  | [ ] |
| [SS3IO-30](#ss3io-30) ⭐ | 2442-2447 | H/H | uninitialised field / division by zero | `cat[f] = CatchData[f][allregion][maxyr] * RBCspeciesArray[sp].mgtTAC_old / last_catch;` — `mgtTAC_old` (atlan… |  | [ ] |
| [SS3IO-31](#ss3io-31) ⭐ | 2448 | H/H | ctl file format | `fprintf(fid, "%d 1 %d %f\n,", ...)` — stray comma after the newline, so each following line starts with `,` (… |  | [ ] |
| [SS3IO-32](#ss3io-32) | 2457-3022, 3518-4055 | H/L | copy-paste / wrong index | (Dead; `Write_SS_Control_File_Orig` is a verbatim copy.) Male parameters are all computed from FEMALE values (… |  | [ ] |
| [SS3IO-33](#ss3io-33) | 3040-3516 | H/L | logic / overrun | (Dead.) `s` loops `0..Nsex_samp-1` with `Nsex_samp = 1` but vectors are only written for `s == 1` / `s == 2` (… |  | [ ] |

## Reviewer notes for this file

Context: the live path is `TierOne()` -> `SS330Assessment()` (atmanage/atSS3assess.c) -> `WriteSS330Files()` (-> `WriteSSDat`, `WriteSSCtl`, `WriteSSFor`, `Create_Starter_File`), then `Run_SS3()`, `Read_SS3_Par_File()`, `Read_SS3_Report_File()`. `WriteSSFiles()` (atSS3Link.c) is commented out in `TierOne()`, so `Write_Forecast_File`, `Write_SS_Data_File`, `Write_SS_Control_File`, the `*_Orig` functions and `Read_SS3_Report_File_EstBo_Only` are currently unreachable (dead). Findings in dead code are rated Sev L.
Functions checked: Read_SS3_Report_File, Read_SS3_Report_File_EstBo_Only, Read_SS3_Par_File, Create_Starter_File, Create_Starter_File_Orig, Write_Forecast_File, Write_SS_Data_File, WriteSS330Files, WriteSSDat, WriteSSCtl, WriteSSFor, Write_SS_Control_File, Write_SS_Data_File_Orig, Write_SS_Control_File_Orig.

---

## SS3IO-01

**NULL dereference** · Conf H / Sev H ⭐ priority  
**Lines (current code):** ≈92 (also ≈300)  
**Lines at review (Oct 2026):** 61-72 (also 219-230)

**Function:** `Read_SS3_Report_File()`

**Why it's thought to be a bug**

`if ((fid = fopen(...)) == NULL) { ... AssessFail_id = 1; }` then falls through to `while (fgets(buf, buflen, fid) ...)` with `fid == NULL` -> crash whenever SS3 fails to produce Report.sso (the very case the warning is for). Function: `Read_SS3_Report_File()` (same in `Read_SS3_Report_File_EstBo_Only()`). `Read_SS3_Par_File()` correctly returns.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 90–94) — ≈ the code here has changed since the review; lines are approximate

```c
   90: 
   91: 	/* Find line that starts with 'FORECAST:_With_F_to_match_adjusted_catch' */
   92: 	while (fgets(buf, buflen, fid) != NULL) {
   93: 
   94: 		if (strstr(buf, "FORECAST:_With_F_to_match_adjusted_catch") != NULL) {
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
**Lines (current code):** 54, ≈80; 374, ≈388  
**Lines at review (Oct 2026):** 36, 61; 302, 315

**Why it's thought to be a bug**

Both readers open bare `"Report.sso"` / `"ss3.par"` in the current working dir and ignore `folderName`. In the live path `Run_SS3()` (atSS3assess.c:≈190) does `chdir(run_dir)` and then `chdir(cwd)` back before `Read_SS3_Par_File`/`Read_SS3_Report_File` are called (atSS3assess.c:118-124), so the files SS3 just wrote in `<sp>_SS3_sim_..._year_...` are never read (either "can't open" -> SS3IO-01 crash / par=1000 -> assessment always failed, or a stale file from cwd is read). The old `WriteSSFiles()` path did chdir before reading, the new one doesn't. Functions: `Read_SS3_Report_File()`, `Read_SS3_Par_File()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 52–82) — ≈ the code here has changed since the review; lines are approximate

```c
   52: double Read_SS3_Report_File(MSEBoxModel *bm, int species, int year, char *folderName) {
   53: 
   54: 	char outputFileName[] = "Report.sso";
   55: 
   56: 	FILE *fid;
   57: 	int buflen = STRLEN;
   58: 	char buf[STRLEN];
   59:     char buffer[1000]; // TODO: Set this to sensible value
   60: 	char seps[] = " ";
   61:     int n, j, thisID;
   62:     //int yy;
   63: 	char *varStr;
   64: 	int fleetIndex;
   65: 	double catch;
   66: 	double RBC;
   67:     //char *line_buf = NULL;
   68:     //size_t line_buf_size = 0;
   69:     //size_t line_size;
   70:     //int found_LAA = 0;
   71:     int size = 100;  // TODO: Set this to sensible value
   72:     double Len_mid, Len_SD, wt_beg, wt_mid;
   73: 
   74:     double *values = (double *) malloc((size_t)size * sizeof(double));
   75: 
   76: 	_getcwd(buf, (int)STRLEN);
   77: 	printf("getcwd = %s\n", buf);
   78: 
   79:     if ((fid = Open_SS3_Output_File(folderName, outputFileName)) == NULL) {
   80: 		fprintf(bm->logFile,"Read_SS3_Report_File: Can't open %s/%s\n", folderName, outputFileName);
   81:         fprintf(bm->logFile, "yr: %d WARNING - Read_SS3_Report_File - SS3 failed to generate a Report.sso output file for %s\n", year, FunctGroupArray[species].groupCode);
   82:         printf("yr: %d WARNING - Read_SS3_Report_File - SS3 failed to generate a Report.sso output file for %s\n", year, FunctGroupArray[species].groupCode);
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
**Lines (current code):** 105-124 (also 313-326)  
**Lines at review (Oct 2026):** 85-104 (also 243-256)

**Function:** `Read_SS3_Report_File()`

**Why it's thought to be a bug**

`varStr = strtok(NULL, seps); n++; if (n == EstBcurrIndex) ... atof(varStr)` — the NULL check is done before the strtok, so on a short/blank line `atof(NULL)` is called. Function: `Read_SS3_Report_File()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 103–126)

```c
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
  115:                 
  116: 				/* The 'bio-all' column */
  117: 				if (n == EstBcurrIndex) {
  118: 					bm->RBCestimation.RBCspeciesParam[species][EstBcurr_id] = atof(varStr);
  119: 				}
  120: 
  121: 				/* The 'Depletion' column */
  122: 				if (n == depletionIndex) {
  123: 					bm->RBCestimation.RBCspeciesParam[species][EstDepletion_id] = atof(varStr);
  124: 				}
  125: 
  126: 				if (n >= startOfFleetData)
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
**Lines (current code):** 59, 214-228  
**Lines at review (Oct 2026):** 41, 146-160

**Function:** `Read_SS3_Report_File()`

**Why it's thought to be a bug**

In the `Biology_at_age_in_endyr_with_CV=f(LAA)` block no lines are ever read: `strtok(buffer, seps)` tokenises the never-initialised local `char buffer[1000]`, so `values[12..16]` are garbage (or uninitialised malloc memory). Also the continuation separator is `"\n"` not `seps`, so after the first token the rest of the line would be a single token; and `j` is unbounded against `values[100]`. Function: `Read_SS3_Report_File()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 57–61)

```c
   57: 	int buflen = STRLEN;
   58: 	char buf[STRLEN];
   59:     char buffer[1000]; // TODO: Set this to sensible value
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
**Lines (current code):** 217-221, 237-247  
**Lines at review (Oct 2026):** 149-153, 169-179

**Function:** `Read_SS3_Report_File()`

**Why it's thought to be a bug**

`for (age = 0; age < RBCspeciesParam[AccumAge_id]; age++) ... MeanLenAgeM[0][sex][age][thisID]` — MeanLenAgeM/SigmaLenAgeM/MeanWtAgeS/MeanWtAgeM are allocated `Util_Alloc_Init_4D_Double(nYears, MaxAge, Nsexes, stocks)` => `[stocks][Nsexes][MaxAge][nYears]` (atAssessTierSetup.c:281-285), and AccumAge = MaxAge + ceil(MaxAge/4) (atAssessTierSetup.c:86) > MaxAge, so `age` overruns the age dimension. Function: `Read_SS3_Report_File()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 215–249)

```c
  215:             //found_LAA = 1;
  216:             
  217:             for (int sex = 0; sex < bm->RBCestimation.RBCspeciesParam[species][Nsexes_id]; sex++) {
  218:                 for (int age = 0; age < bm->RBCestimation.RBCspeciesParam[species][AccumAge_id]; age++) {
  219:                 
  220:                     const char* tok;
  221:                     thisID = (int)(bm->RBCestimation.RBCspeciesParam[species][HistYrMin_id]);
  222:                     j = 0;
  223: 
  224:                     // Assign values
  225:                     for (tok = strtok(buffer, seps); tok && *tok; j++, tok = strtok(NULL, "\n")) {
  226:                         values[j] = atof(tok);
  227:                         //fprintf(bm->logFile, "%f\t", values[j]);
  228:                     }
  229: 
  230:                     Len_mid = values[12];
  231:                     Len_SD = values[14];
  232:                     wt_beg = values[15];
  233:                     wt_mid = values[16];
  234:                     
  235:                     fprintf(bm->logFile,"Time: %e %s sex %d age %d Len_mid: %e Len_SD: %e wt_beg: %e  wt_mid: %e\n",bm->dayt, FunctGroupArray[species].groupCode, sex, age, Len_mid, Len_SD, wt_beg, wt_mid);
  236:                     
  237:                     if ((fabs(bm->RBCestimation.RBCspeciesArray[species].MeanLenAgeM[0][sex][age][thisID] - Len_mid) / Len_mid) > bm->RBCestimation.ConvergeThresh) {
  238:                         fprintf(bm->logFile,"Time: %e %s sex %d age %d - problem with  Len_mid: %e vs MeanLenAge: %e\n", bm->dayt, FunctGroupArray[species].groupCode, sex, age, Len_mid, bm->RBCestimation.RBCspeciesArray[species].MeanLenAgeM[0][sex][age][thisID]);
  239:                     }
  240:                     if ((fabs(bm->RBCestimation.RBCspeciesArray[species].SigmaLenAgeM[0][sex][age][thisID] - Len_SD) / Len_SD) > bm->RBCestimation.ConvergeThresh) {
  241:                         fprintf(bm->logFile,"Time: %e %s sex %d age %d - problem with  Len_SD: %e vs SigmaLenAgeM: %e\n", bm->dayt, FunctGroupArray[species].groupCode, sex, age, Len_SD, bm->RBCestimation.RBCspeciesArray[species].SigmaLenAgeM[0][sex][age][thisID]);
  242:                     }
  243:                     if ((fabs(bm->RBCestimation.RBCspeciesArray[species].MeanWtAgeS[0][sex][age][thisID] - wt_beg) / wt_beg) > bm->RBCestimation.ConvergeThresh) {
  244:                         fprintf(bm->logFile,"Time: %e %s sex %d age %d - problem with  wt_beg: %e vs MeanWtAgeS: %e\n", bm->dayt, FunctGroupArray[species].groupCode, sex, age, wt_beg, bm->RBCestimation.RBCspeciesArray[species].MeanWtAgeS[0][sex][age][thisID]);
  245:                     }
  246:                     if ((fabs(bm->RBCestimation.RBCspeciesArray[species].MeanWtAgeM[0][sex][age][thisID] - wt_mid) / wt_mid) > bm->RBCestimation.ConvergeThresh) {
  247:                         fprintf(bm->logFile,"Time: %e %s sex %d age %d - problem with  wt_mid: %e vs MeanWtAgeM: %e\n", bm->dayt, FunctGroupArray[species].groupCode, sex, age, wt_mid, bm->RBCestimation.RBCspeciesArray[species].MeanWtAgeM[0][sex][age][thisID]);
  248:                     }
  249:                 }
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
**Lines (current code):** 255-256  
**Lines at review (Oct 2026):** 187-188

**Function:** `Read_SS3_Report_File()`

**Why it's thought to be a bug**

"Something has gone wrong reading in the SS3 report file" is printed/logged unconditionally after the loop, including on success. Function: `Read_SS3_Report_File()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 253–258)

```c
  253: 	}
  254:     
  255:     printf("%s Something has gone wrong reading in the SS3 report file. Look at the Report.sso: Biology_at_age_in_endyr_with_CV=f(LAA)\n", FunctGroupArray[species].groupCode);
  256:     fprintf(bm->logFile,"Time: %e %s Something has gone wrong reading in the SS3 report file. Look at the Report.sso: Biology_at_age_in_endyr_with_CV=f(LAA)\n", bm->dayt, FunctGroupArray[species].groupCode);
  257: 
  258: 	fprintf(bm->logFile,"Time: %e %s EstB0 = %e, RBC = %e\n", bm->dayt, FunctGroupArray[species].groupCode, bm->RBCestimation.RBCspeciesParam[species][EstB0_id], RBC);
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
**Lines (current code):** 72-194; 230-292; 326-345  
**Lines at review (Oct 2026):** 72-194; 230-292; 326-345

**Why it's thought to be a bug**

`fid` is never `fclose`d in `Read_SS3_Report_File()`, `Read_SS3_Report_File_EstBo_Only()` or `Read_SS3_Par_File()`. Two descriptors leak per species per assessment year; on long multi-species runs this can hit the per-process FD limit, after which fopen fails (-> SS3IO-01 crash).

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 70–109 (truncated))

```c
   70:     //int found_LAA = 0;
   71:     int size = 100;  // TODO: Set this to sensible value
   72:     double Len_mid, Len_SD, wt_beg, wt_mid;
   73: 
   74:     double *values = (double *) malloc((size_t)size * sizeof(double));
   75: 
   76: 	_getcwd(buf, (int)STRLEN);
   77: 	printf("getcwd = %s\n", buf);
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
**Lines (current code):** 395-410  
**Lines at review (Oct 2026):** 323-338

**Function:** `Read_SS3_Par_File()`

**Why it's thought to be a bug**

Searches for `MGparm[16]`, then "ignore the next line" and reads the line after. In an ADMB `.par` file each `# MGparm[16]:` header is followed directly by its value line, so the code skips the value and reads the next `# MGparm[17]:` header -> `atof("#") = 0` -> `MaxConvergCrit` is always 0 and the convergence test in `SS330Assessment()` always passes. Also MGparm[16] is a biology parameter, not a convergence measure (the max gradient is on the `.par` header line "Maximum gradient component = ..."). Function: `Read_SS3_Par_File()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 393–412)

```c
  393:     }
  394:     
  395:     snprintf(str, sizeof(str), "MGparm[%d]", maxConvergCritIndex);
  396:     
  397: 	/* Find line that starts with 'MGparm[16]' - as that marks the convergence result */
  398: 	while (fgets(buf, buflen, fid) != NULL) {
  399:         
  400: 		if (strstr(buf, str) != NULL) {
  401:             
  402: 			/* ignore the next line */
  403: 			fgets(buf, buflen, fid);
  404:             
  405: 			/* grab the data from the next line */
  406: 			fgets(buf, buflen, fid);
  407:             
  408: 			varStr = strtok(buf, seps);
  409: 			ans = atof(varStr);
  410: 		}
  411: 	}
  412:     
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
**Lines (current code):** 433, 448-449  
**Lines at review (Oct 2026):** 359, 374-375

**Function:** `Create_Starter_File()`

**Why it's thought to be a bug**

Starter file writes `"%s%s.dat", groupCode, fileName` where `fileName` is the starter's own path (`"<dir>//starter.ss"`) -> e.g. `FISdir//starter.ss.dat`. The real files written by `WriteSS330Files()` are `<dir>/<groupCode>WriteSS330Files.dat/.ctl` (lines 1216, 1225), and SS3 runs inside `<dir>`, so SS3 cannot find its data/control files. Function: `Create_Starter_File()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 431–451)

```c
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
  442: 
  443: 	fprintf(fid, "# SS starter file for %s\n", FunctGroupArray[groupIndex].groupCode);
  444:     fprintf(fid, "#C starter comment here\n");
  445: 	//fprintf(fid, "atlantisSS3_%d.dat\n", versionID);
  446: 	//fprintf(fid, "atlantisSS3_%d.ctl\n", versionID);
  447: 
  448: 	fprintf(fid, "%s%s.dat\n", FunctGroupArray[groupIndex].groupCode, fileName);
  449: 	fprintf(fid, "%s%s.ctl\n", FunctGroupArray[groupIndex].groupCode, fileName);
  450: 
  451: 	fprintf(fid, "0 # 0=use init values in control file; 1=use ss3.par\n");
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
**Lines (current code):** 471-472  
**Lines at review (Oct 2026):** 397-398

**Function:** `Create_Starter_File()`

**Why it's thought to be a bug**

`"1 # min age for calc of summary biomass"` is written twice. SS3.30 starter expects a single value here, so every subsequent entry (Depletion basis, fraction X, SPR basis, F units, ..., check value 3.30) is shifted by one and the starter fails its end-of-file check / is misread. Function: `Create_Starter_File()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 469–474)

```c
  469:     fprintf(fid, "%e # final convergence criteria (e.g. 1.0e-04)\n", bm->RBCestimation.SSTol);  // 0.0001
  470:     fprintf(fid, "0 # retrospective year relative to end year (e.g. -4)\n");
  471:     fprintf(fid, "1 # min age for calc of summary biomass\n");
  472:     fprintf(fid, "1 # min age for calc of summary biomass\n");
  473:     fprintf(fid, "%d # Depletion basis:  denom is: 0=skip; 1=rel X*B0; 2=rel X*Bmsy; 3=rel X*B_styr\n", bm->RBCestimation.SSDepletionBasis); //1
  474:     fprintf(fid, "%e # Fraction (X) for Depletion denominator (e.g. 0.4)\n", bm->RBCestimation.SSFractX); // 1.0  neil had 0.41 here
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
**Lines (current code):** 493, 497  
**Lines at review (Oct 2026):** 419, 423

**Function:** `Create_Starter_File_Orig()`

**Why it's thought to be a bug**

`snprintf(fileName, sizeof(fileName), ...)` where `fileName` is a `char *` parameter -> `sizeof` is 8, path truncated to 7 chars. Function: `Create_Starter_File_Orig()` (dead).

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 491–499)

```c
  491:  */
  492: 
  493: void Create_Starter_File_Orig(MSEBoxModel *bm, char *dirName, int groupIndex, char *fileName, int versionID) {
  494: 
  495:     FILE *fid;
  496: 
  497:     snprintf(fileName, sizeof(fileName), "%s/%sstarter.ss", dirName, FOLDER_SEP);
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
**Lines (current code):** 626-635  
**Lines at review (Oct 2026):** 552-561

**Function:** `Write_Forecast_File()`

**Why it's thought to be a bug**

`catch = CatchData[f][sumregion][maxyr] * TAC_old / last_catch` with no guard for `last_catch == 0`; fleets printed 0-based (`f`) where SS needs 1..Nfleets. Function: `Write_Forecast_File()` (dead).

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 624–637)

```c
  624: 	fprintf(fid, "#Year Seas Fleet Catch(or_F) \n");
  625: 
  626: 	last_catch = 0;
  627:     for (f = 0; f < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; f++) {
  628: 		last_catch = last_catch + bm->RBCestimation.RBCspeciesArray[groupIndex].CatchData[f][sumregion][maxyr];
  629:     }
  630: 
  631: 	for (f = 0; f < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; f++) {
  632: 		catch = bm->RBCestimation.RBCspeciesArray[groupIndex].CatchData[f][sumregion][maxyr] * bm->RBCestimation.RBCspeciesParam[groupIndex][TAC_old_id]
  633: 				/ last_catch;
  634: 		fprintf(fid, " %d 1 %d %f\n", (int) bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id] + maxyr + 1, f, catch);
  635: 	}
  636: 
  637: 	fprintf(fid, "\n#\n");
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
**Lines (current code):** 807, 822, 848, 854, 1016  
**Lines at review (Oct 2026):** 733, 748, 774, 780, 942

**Function:** `Write_SS_Data_File()`

**Why it's thought to be a bug**

Several errors: 780 `for (f = 0; fleetIndex < NumFisheries; f++)` tests the stale `fleetIndex` (== NumFisheries after the loop at 690) so discard fleet specs are never written; 733 writes a stray line `"%e %e"` of NumFisheries/NumSurvey into the CPUE block; 748 CPUE obs `" %d 7 %e %e"` omits the fleet field; 774 writes NumFisheries as N_fleets_with_discard instead of `nfltdisc` (fixed in `WriteSSDat` 1326); 942 no newline after age bins. Function: `Write_SS_Data_File()` (dead).

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 805–844 (truncated))

```c
  805:     //* could add fleet names as comment at the end of each row
  806:     fprintf(fid, "#_yr month fleet obs stderr\n");
  807:     fprintf(fid, "%e %e\n", bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id], bm->RBCestimation.RBCspeciesParam[groupIndex][NumSurvey_id]);
  808: 
  809:     for (f = 0; f < (bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[groupIndex][NumSurvey_id]); f++) {
  810:         for (t = bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id]; t < maxyr; t++) {
  811:             if (bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEgen[f][allregion][t] > 0) {
  812:                 if (!bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEcv[f]) {   // generate cpue with no error, but still want cv in SS
  813:                     cpcv = 0.2;
  814:                 } else {
  815:                     cpcv = bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEcv[f];
  816:                 }
  817: 
  818:                 if (bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEgen[f][allregion][t] < 0.01) {
  819:                     bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEgen[f][allregion][t] = 0.01; // this is because SS fails if abund index<=0
  820:                 }
  821:                 
  822:                 fprintf(fid, " %d 7 %e %e\n", t, bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEgen[f][allregion][t], cpcv);
  823:             }
  824:         }
  825:     }
  826:     fprintf(fid, "-9999 1 1 1 1 # terminator for CPUE and survey observations\n");
  827:     fprintf(fid, "#\n");
  828: 
  829:     // setup for discards
  830:     //* check this works for an assessment with discards
  831:     //* Jemery suggested the flathead and school whiting examples
  832:     Util_Init_1D_Int(bm->RBCestimation.RBCspeciesArray[groupIndex].discfleet, bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id], 0.0);
  833:     
  834:     nfltdisc = 0;
  835:     num = 0;
  836:     for (t = bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id]; t < maxyr; t++) {
  837:         for (f = 0; f < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; f++) {
  838:             if (bm->RBCestimation.RBCspeciesArray[groupIndex].DiscData[f][allregion][t] > 0) {
  839:                 num++;
  840:                 bm->RBCestimation.RBCspeciesArray[groupIndex].discfleet[f] = 1;
  841:             }
  842:         }
  843:     }
  844:     for (f = 0; f < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; f++){
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
**Lines (current code):** 1222/1229-1230, 2355; 1238-1239, 2454  
**Lines at review (Oct 2026):** 1148/1155-1156, 2281; 1164-1165, 2380

**Why it's thought to be a bug**

`WriteSSCtl()` ends with `fclose(fid)` and `WriteSS330Files()` then calls `fclose(fidctl)` on the same FILE*; likewise `WriteSSFor()` closes and the caller closes `fidss` again. Double `fclose` is undefined behaviour (heap corruption / abort on glibc). `WriteSSDat()` correctly leaves closing to the caller. Functions: `WriteSSCtl()`, `WriteSSFor()`, `WriteSS330Files()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1220–1232)

```c
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
**Lines (current code):** 1295-1296  
**Lines at review (Oct 2026):** 1221-1222

**Function:** `WriteSSDat()`

**Why it's thought to be a bug**

Fleet definition line `" 1  -1  1  1  0  FLEET%d in metier %s\n"` has no `#`, so SS reads fleetname=`FLEET0` and then `in`, `metier`, `<code>` as the start of the next fleet's definition -> whole data file misparsed. Because `speciesRPFleetToMetier` is never populated (all 0, so `> -1` is always true) this branch is always taken. The earlier `Write_SS_Data_File()` (line 740) has `# %d in metier`. Function: `WriteSSDat()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1293–1298)

```c
 1293:         //add the fleet type
 1294:         if(i < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]){
 1295:             if (bm->RBCestimation.speciesRPFleetToMetier[i][sp] > -1) {
 1296:                 fprintf(fid, " 1  -1  1  1  0  FLEET%d in metier %s\n", i, FisheryArray[i].fisheryCode);
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
**Lines (current code):** 1222-1300, 1333, 1346, 1446, 1574, 2018-2023, 2037-2039, 2240-2255  
**Lines at review (Oct 2026):** 1222-1300, 1333, 1346, 1446, 1574, 2018-2023, 2037-2039, 2240-2255

**Why it's thought to be a bug**

All SS fleet numbers are written 0-based (`f`/`i`): catch lines `-999 1 %d`, CPUE setup `%d 1 0 0`, CPUE/discard/length/age observations, Q setup, Q parms, variance adjustments. SS3.30 numbers fleets 1..Nfleets, so fleet 0 is invalid and every observation is attributed to the previous fleet (the last survey is never referenced). The `*_Orig` writers used `fleetIndex + 1` (e.g. 3087-3110, 3269). Functions: `WriteSSDat()`, `WriteSSCtl()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1220–1259 (truncated))

```c
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
 1254: // calls :
 1255: // created  : Nov 2007 Sally
 1256: // updated:   Feb 2011 Sally
 1257: // updated  : Nov 2012 Sally for SS v3.24
 1258: //
 1259: //******************************************************************************
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
**Lines (current code):** 1324-1329  
**Lines at review (Oct 2026):** 1250-1255

**Function:** `WriteSSDat()`

**Why it's thought to be a bug**

`fprintf(fid, "%d 1 %d", t, f)` has no trailing space and the next `fprintf(fid, "%e ", catch)` has no leading space, so fleet and catch merge (e.g. `1980 1 01.234e+02  0.01`) -> 4 tokens instead of 5, and the whole catch block is misread. `Write_SS_Data_File()` (line 770) has `"%d 1 %d "`. Function: `WriteSSDat()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1322–1331)

```c
 1322:         // t is the year, f is fleet
 1323:         for (t = bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]; t < maxyr; t++){
 1324:             fprintf(fid, "%d 1 %d", t, f);
 1325:             // Apply proportion catch rule for closed area scenarios
 1326:             if ((bm->RBCestimation.RBCspeciesArray[sp].mgt_Retro_catch && (bm->RBCestimation.RBCspeciesParam[sp][PropClosed_id] > 0)) && (t < bm->RBCestimation.RBCspeciesParam[sp][HistYrMax_id])){   // allocate historic catches to open area by proportion open
 1327:                 fprintf(fid, "%e ", (1.0 - bm->RBCestimation.RBCspeciesArray[sp].PropClosed) * bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][allregion][t]);
 1328:             } else {
 1329:                 fprintf(fid, "%e ", bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][allregion][t]);   // region=0 is sum over regions
 1330:             }
 1331:          fprintf(fid, " 0.01\n");
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
**Lines (current code):** 1352, 1354 (also 1374, 1420)  
**Lines at review (Oct 2026):** 1278, 1280 (also 1300, 1346)

**Function:** `WriteSSDat()`

**Why it's thought to be a bug**

CPUE setup lines for non-metier fleets and all surveys (`" %d 1  0  0  # SURVEY%d"`) have no `\n`; the next fleet's setup then sits after the `#` on the same line and is ignored by SS, so with 2+ surveys the specs are missing and the data file is misread. (1300 CPUE obs and 1346 discard obs also lack `\n`; harmless token-wise but they merge with the following line.) Function: `WriteSSDat()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1350–1376)

```c
 1350:                 fprintf(fid, " %d 1  0  0  # FLEET%d in metier %s\n", i, i, FisheryArray[i].fisheryCode);
 1351:             else
 1352:                 fprintf(fid, " %d 1  0  0  # FLEET%d", i, i);
 1353:         } else {
 1354:             fprintf(fid, " %d 1  0  0  # SURVEY%d", i, i - (int)(bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]));
 1355:         }
 1356:     }
 1357:     // abundance indices
 1358:     //* could add fleet names as comment at the end of each row
 1359:     fprintf(fid, "#_yr month fleet obs stderr\n");
 1360:                         
 1361:     //fprintf(bm->logFile, "Time %e %s %d %d\n", bm->dayt, FunctGroupArray[sp].groupCode, bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id], bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]);
 1362: 
 1363:     for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]; f++) {
 1364:         for (t = bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]; t < maxyr; t++) {
 1365:             if (bm->RBCestimation.RBCspeciesArray[sp].CPUEgen[f][allregion][t] > 0.0) {
 1366:                 if (!bm->RBCestimation.RBCspeciesArray[sp].CPUEcv[f]) {   // generate cpue with no error, but still want cv in SS
 1367:                     cpcv = 0.2;
 1368:                 } else {
 1369:                     cpcv = bm->RBCestimation.RBCspeciesArray[sp].CPUEcv[f];
 1370:                 }
 1371:             
 1372:                 if (bm->RBCestimation.RBCspeciesArray[sp].CPUEgen[f][allregion][t] < 0.01) { bm->RBCestimation.RBCspeciesArray[sp].CPUEgen[f][allregion][t] = 0.01; // this is because SS fails if abund index<=0
 1373:                 }
 1374:                 fprintf(fid, "%d 7 %d %e %e", t, f, bm->RBCestimation.RBCspeciesArray[sp].CPUEgen[f][allregion][t], cpcv);
 1375:             }
 1376:         }
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
**Lines (current code):** 1292-1602, 2086-2329  
**Lines at review (Oct 2026):** 1218-1528, 2012-2255

**Why it's thought to be a bug**

Loops run `f < NumFisheries + NumSurvey` and index arrays sized `K_num_fisheries` in the fleet dimension: `CPUEgen[f]`, `CPUEcv[f]`, `LengthFltYr[it][f]`, `LFss[f]`, `LenComp[f]` (atAssessTierSetup.c:149-170, all `[..K_num_fisheries..]`), and in the ctl `CPUEqmu[f]`, `Sel_Pattern[f]`, `mirrored_fleet[f]`, `Start_Sel[f]`, `Sel_Phase[f]`, `SelBlock[f]`, `Varadj_*[f]` (allocated K_num_fisheries / `[K_num_fisheries][K_num_tot_sp]`). NumFisheries is capped near K_num_fisheries (atAssessTierSetup.c:1478-1482) so any surveys push `f` past the end. Functions: `WriteSSDat()`, `WriteSSCtl()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1290–1329 (truncated))

```c
 1290:     // loop over fleets and surveys
 1291:     // We assume that fleets always come before surveys
 1292:     for (i = 0; i < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]; i++){
 1293:         //add the fleet type
 1294:         if(i < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]){
 1295:             if (bm->RBCestimation.speciesRPFleetToMetier[i][sp] > -1) {
 1296:                 fprintf(fid, " 1  -1  1  1  0  FLEET%d in metier %s\n", i, FisheryArray[i].fisheryCode);
 1297:             } else {
 1298:                 fprintf(fid, " 1  -1  1  1  0  FLEET%d\n", i);
 1299:             }
 1300:         } else {
 1301:             fprintf(fid, " 3   1  1  2  0  SURVEY%d # \n", (i - (int)(bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id])));
 1302:         }
 1303:     }
 1304:     // Bycatch fleet not currently used
 1305:     fprintf(fid, "#Bycatch_fleet_input_goes_next\n");
 1306:     fprintf(fid, "#a:  fleet index\n");
 1307:     fprintf(fid, "#b:  1=include dead bycatch in total dead catch for F0.1 and MSY optimizations and forecast ABC; 2=omit from total catch for these purposes (but still include the mortality)\n");
 1308:     fprintf(fid, "#c:  1=Fmult scales with other fleets; 2=bycatch F constant at input value; 3=bycatch F from range of years\n");
 1309:     fprintf(fid, "#d:  F or first year of range\n");
 1310:     fprintf(fid, "#e:  last year of range\n");
 1311:     fprintf(fid, "#f:  not used\n");
 1312:     fprintf(fid, "# a   b   c   d   e   f \n");
 1313:     // Header for the catch data
 1314:     fprintf(fid, "#_Catch data: yr, seas, fleet, catch, catch_se\n");
 1315:     fprintf(fid, "#_catch_se:  standard error of log(catch)\n");
 1316:     fprintf(fid, "#_NOTE:  catch data is ignored for survey fleets\n");
 1317: 
 1318:     // loop over the fleets only as catch data is ignored for survey fleets
 1319:     for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++){
 1320:         fprintf(fid, "-999 1 %d %e 0.01\n", f, bm->RBCestimation.RBCspeciesArray[sp].initialEquilCatch[f]);
 1321:         //next is a loop over the years
 1322:         // t is the year, f is fleet
 1323:         for (t = bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]; t < maxyr; t++){
 1324:             fprintf(fid, "%d 1 %d", t, f);
 1325:             // Apply proportion catch rule for closed area scenarios
 1326:             if ((bm->RBCestimation.RBCspeciesArray[sp].mgt_Retro_catch && (bm->RBCestimation.RBCspeciesParam[sp][PropClosed_id] > 0)) && (t < bm->RBCestimation.RBCspeciesParam[sp][HistYrMax_id])){   // allocate historic catches to open area by proportion open
 1327:                 fprintf(fid, "%e ", (1.0 - bm->RBCestimation.RBCspeciesArray[sp].PropClosed) * bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][allregion][t]);
 1328:             } else {
 1329:                 fprintf(fid, "%e ", bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][allregion][t]);   // region=0 is sum over regions
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
**Lines (current code):** 1560-1565, 1651-1664  
**Lines at review (Oct 2026):** 1486-1491, 1577-1590

**Function:** `WriteSSDat()`

**Why it's thought to be a bug**

Writes `MaxAge+1` as `#_N_age_bins` but then only `MaxAge` bin labels (`a < MaxAge`) and `MaxAge` values per age-comp vector. SS reads MaxAge+1 bin edges (consuming the `1` of `#_N_ageerror_definitions`) and every age-comp row is one value short -> file misparsed. Function: `WriteSSDat()` (same pattern in `Write_SS_Data_File()` 936-940, 1032-1045).

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1558–1567)

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
**Lines (current code):** 1567-1573 (also 1017-1023, 3397-3408)  
**Lines at review (Oct 2026):** 1493-1499 (also 943-949, 3323-3334)

**Function:** `WriteSSDat()`

**Why it's thought to be a bug**

Ageing-error definition rows must have Nages+1 entries (ages 0..accumulator age); loops write `a < AccumAge` = AccumAge entries. `Ageing_error` is allocated `AccumAge+1` "dimensions 0..AccumAge (SS data file)" (atAssessTierSetup.c:175), and the comment on 1493 says "accumulator age + 1". Each row is one short so the following data is shifted. Function: `WriteSSDat()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1565–1575)

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
**Lines (current code):** 1261, 1635-1640  
**Lines at review (Oct 2026):** 1187, 1561-1566

**Function:** `WriteSSDat()`

**Why it's thought to be a bug**

Age-comp partition uses `it==1 -> 2, it==2 -> 0, it==3 -> 1` but `it` runs 0..2: `it==0` (retained) leaves `part` unset (uninitialised in this function — declared without init at 1187) or stale, `it==1` (whole) is labelled retained, `it==2` (discard) labelled whole. The length block a few lines up (1507-1513) uses the correct `!it -> 2, 1 -> 0, else 1`. Function: `WriteSSDat()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1259–1263)

```c
 1259: //******************************************************************************
 1260: void WriteSSDat(MSEBoxModel *bm, FILE *fid, int sp, int maxyr) {
 1261:     int t, f, s, a, l, i, num, part, it, allregion, gender, nfltdisc, iy;
 1262:     double *emptySex;
 1263:     double cpcv, disccv;
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
**Lines (current code):** 1754, 2139-2145; 2439-2447  
**Lines at review (Oct 2026):** 1680, 2065-2071; 2365-2373

**Why it's thought to be a bug**

`WriteSSCtl()` and `WriteSSFor()` set `allregion = 0`, but the sum over regions is stored at index `NumRegions` (`CatchData`/`DiscData` allocated `K_num_reg+1` "so have sum over all regions"; `WriteSSDat()` 1192 and atManageTier.c:700 use `Nregions`). So retention parameters (`dflt`) are decided from region 0 only while the dat file's discard fleets come from the all-region sum (mismatched parameter count -> SS error), and forecast catch splits use region 0 only.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1752–1756)

```c
 1752:     double sv, mval, lo, hi;
 1753:     int *dflt;
 1754:     int allregion = 0;    // uses weighted sum over regions, assume assessment doesn't know about regions
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
**Lines (current code):** 1779-1787  
**Lines at review (Oct 2026):** 1705-1713

**Function:** `WriteSSCtl()`

**Why it's thought to be a bug**

Writes `NblockperPatt[i]` blocks per pattern but loops `j < NblockperPatt[i]-1` with `Blocks[i][j*2+1], Blocks[i][j*2+2]` (1-based leftover) -> one begin/end pair too few, so the ctl is misread when `NblockPattern > 0`. Also `Blocks` and `NblockperPatt` are allocated (atAssessTierSetup.c:287-288) but never filled anywhere in the repo. Function: `WriteSSCtl()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1777–1789)

```c
 1777:     if (bm->RBCestimation.RBCspeciesParam[sp][NblockPattern_id] > 0){
 1778:         fprintf(fid, "%d #_Nblock_Patterns\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][NblockPattern_id]));
 1779:         for (i = 0; i < bm->RBCestimation.RBCspeciesParam[sp][NblockPattern_id]; i++) {
 1780:             fprintf(fid, "%d  ", bm->RBCestimation.RBCspeciesArray[sp].NblockperPatt[i]);
 1781:         }
 1782:         fprintf(fid, "        # Blocks per pattern\n");
 1783:         fprintf(fid, "# begin and end years of Blocks\n");
 1784:         for (i=0; i < bm->RBCestimation.RBCspeciesParam[sp][NblockPattern_id]; i++) {
 1785:             for (j=0; j< bm->RBCestimation.RBCspeciesArray[sp].NblockperPatt[i]-1; j++) {
 1786:                 fprintf(fid, "%f %f\n", bm->RBCestimation.RBCspeciesArray[sp].Blocks[i][j*2+1], bm->RBCestimation.RBCspeciesArray[sp].Blocks[i][j*2+2]);
 1787:             }
 1788:         }
 1789:     } else {
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
**Lines (current code):** 1875  
**Lines at review (Oct 2026):** 1801

**Function:** `WriteSSCtl()`

**Why it's thought to be a bug**

`Mat_slope_Fem` line `"    -3     3 %f %f %f     0.8    0   -3  0 0 0 0 0 0 0"` has 15 fields (three `%f` plus 0.8), whereas each MG parameter line takes 14 (cf. Mat50 line 1874). Every later ctl entry is shifted by one token. Function: `WriteSSCtl()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1873–1877)

```c
 1873:     fprintf(fid, "# Sex: 1  BioPattern: 1  Maturity&Fecundity\n");
 1874:     fprintf(fid, "%f %f %f %f     10    0   -3       0       0         0         0        0     0     0     # Mat50_Fem\n", bm->RBCestimation.RBCspeciesParam[sp][Maturity_Inflect_id]/2.0, bm->RBCestimation.RBCspeciesParam[sp][Maturity_Inflect_id]*2.0, bm->RBCestimation.RBCspeciesParam[sp][Maturity_Inflect_id], bm->RBCestimation.RBCspeciesParam[sp][Maturity_Inflect_id]);
 1875:     fprintf(fid, "    -3     3 %f %f %f     0.8    0   -3       0       0         0         0        0     0     0     # Mat_slope_Fem\n", bm->RBCestimation.RBCspeciesParam[sp][Maturity_Slope_id], bm->RBCestimation.RBCspeciesParam[sp][Maturity_Slope_id], bm->RBCestimation.RBCspeciesParam[sp][Maturity_Slope_id]);
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
**Lines (current code):** 1926  
**Lines at review (Oct 2026):** 1852

**Function:** `WriteSSCtl()`

**Why it's thought to be a bug**

`vbk = log(VBk[0][1])/VBk[0][0];` — the male K offset should be `log(VBk[0][1]/VBk[0][0])` (cf. 1863, 1874). Wrong male K whenever Nsexes>1 and MG_offset>2. Function: `WriteSSCtl()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 1924–1928)

```c
 1924:         fprintf(fid, "%f %f %f %f      10   0   -3       0       0         0         0        0     0     0     # L_at_Amax_Mal_GP_1\n", lo, hi, lmaxm, lmaxm);
 1925:         if (bm->RBCestimation.RBCspeciesParam[sp][MG_offset_id] > 2) {
 1926:             vbk = log(bm->RBCestimation.RBCspeciesArray[sp].VBk[0][1])/bm->RBCestimation.RBCspeciesArray[sp].VBk[0][0];
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
**Lines (current code):** 2091, 2188  
**Lines at review (Oct 2026):** 2017, 2114

**Function:** `WriteSSCtl()`

**Why it's thought to be a bug**

`speciesRPFleetToMetier[f][sp+1]` — array is `[K_num_fisheries][K_num_tot_sp]` (atAssessTierSetup.c:53) and every other use in this file is `[i][sp]` (1295, 1349, 1459, 1589). `sp+1` reads the next species' column and overruns the row when `sp == K_num_tot_sp-1`. Function: `WriteSSCtl()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 2089–2093)

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
**Lines (current code):** 2189, 2191  
**Lines at review (Oct 2026):** 2115, 2117

**Function:** `WriteSSCtl()`

**Why it's thought to be a bug**

Age-selex pattern lines for fishing fleets (`"%d  0  0  0  # FLEET %d in metier %s"`) have no `\n`, so the next fleet's/survey's `pattern 0 0 0` line lands after the `#` and is discarded by SS -> only one age-selex line is read for all fleets and the rest of the ctl is shifted. The survey branch (2194) has `\n`. Function: `WriteSSCtl()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 2187–2193)

```c
 2187:         if (i < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]) {
 2188:             if (bm->RBCestimation.speciesRPFleetToMetier[i][sp+1] > -1) {
 2189:                 fprintf(fid, "%d  0  0  0  # FLEET %d in metier %s", (int)(bm->RBCestimation.RBCspeciesParam[sp][Agesel_Pattern_id]), i, FisheryArray[i].fisheryCode);
 2190:             } else {
 2191:                 fprintf(fid, "%d  0  0  0  # FLEET %d in metier", (int)(bm->RBCestimation.RBCspeciesParam[sp][Agesel_Pattern_id]), i);
 2192:             }
 2193:         } else {
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
**Lines (current code):** 2162  
**Lines at review (Oct 2026):** 2088

**Function:** `WriteSSCtl()`

**Why it's thought to be a bug**

Mirrored size-selex line `"%d %d 0 %d "` has no `\n` (no `#` so tokens still parse, but the next fleet's line is appended). Function: `WriteSSCtl()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 2160–2164)

```c
 2160:                 quit(" --- Error --- \n  Len Selectivity pattern 5 specified for fleet %d\n but no fleet specified to mirror, \n i.e. bm->RBCestimation.RBCspeciesArray[sp].mirrored_fleet[f].\n ---------------\n", f);
 2161:             } else {
 2162:                 fprintf(fid, "%d %d 0 %d ", bm->RBCestimation.RBCspeciesArray[sp].Sel_Pattern[f], disc, bm->RBCestimation.RBCspeciesArray[sp].mirrored_fleet[f]);
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
**Lines (current code):** 2442-2447  
**Lines at review (Oct 2026):** 2368-2373

**Function:** `WriteSSFor()`

**Why it's thought to be a bug**

`cat[f] = CatchData[f][allregion][maxyr] * RBCspeciesArray[sp].mgtTAC_old / last_catch;` — `mgtTAC_old` (atlantisboxmodel.h:3165) is never assigned anywhere in the repo, so forecast catches are 0 (or garbage); the TAC is kept in `RBCspeciesParam[sp][TAC_old_id]` (set atManageTier.c:147, 246; used by `Write_Forecast_File()` 558). Also no guard for `last_catch == 0` (NaN/inf written to forecast.ss). Function: `WriteSSFor()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 2440–2449)

```c
 2440:     
 2441:     cat = Util_Alloc_Init_1D_Double(bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id], 0.0);
 2442:     last_catch = 0;
 2443:     for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
 2444:         last_catch += bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][allregion][maxyr];
 2445:     }
 2446:     for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
 2447:         cat[f] = bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][allregion][maxyr] * bm->RBCestimation.RBCspeciesArray[sp].mgtTAC_old / last_catch;
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
**Lines (current code):** 2448  
**Lines at review (Oct 2026):** 2374

**Function:** `WriteSSFor()`

**Why it's thought to be a bug**

`fprintf(fid, "%d 1 %d %f\n,", ...)` — stray comma after the newline, so each following line starts with `,` (e.g. `,-9999 1 1 0`), corrupting the forecast catch list and its terminator. Function: `WriteSSFor()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 2446–2450)

```c
 2446:     for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
 2447:         cat[f] = bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][allregion][maxyr] * bm->RBCestimation.RBCspeciesArray[sp].mgtTAC_old / last_catch;
 2448:          fprintf(fid, "%d 1 %d %f\n,", maxyr+1, f, cat[f]);
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
**Lines (current code):** 2457-3022, 3518-4055  
**Lines at review (Oct 2026):** 2383-2948, 3444-3981

**Why it's thought to be a bug**

(Dead; `Write_SS_Control_File_Orig` is a verbatim copy.) Male parameters are all computed from FEMALE values (2511 `log(SSMort[0][FEMALE]/SSMort[0][FEMALE])` = 0; 2523-2548, 2563, 2587/2591 use `[0][0]` for male CV_old); `Wtlen_2_Fem` written twice (2680, 2683) and male wt-len lines use FEMALE and are labelled `_Fem` (2697-2701); `bm->RBCestimation.RBCspeciesArray->VBt0` (2551, 2559) = species 0, not `groupIndex`; `dflt` loop (2834-2839) overwrites per year so only the last year counts and runs `yearIndex <= maxyr`; 2675-2679, 2798-2846 missing `\n` before `#` comments. Functions: `Write_SS_Control_File()`, `Write_SS_Control_File_Orig()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 2455–2494 (truncated))

```c
 2455: }
 2456: 
 2457: void Write_SS_Control_File(MSEBoxModel *bm, char *dirName, char *fileName, int maxyr, int groupIndex, int versionID) {
 2458: 
 2459: 	FILE *fid;
 2460: 	int disc, phase;
 2461: 	int _Nblock_Patterns = 0;
 2462: 	int fleetIndex, yearIndex;
 2463: 	double mortality, lminf, lmaxf, vbk, cvest, hi, lo, mval, lminm, lmaxm, sv;
 2464: 	char str[STRLEN];
 2465: 	int startYear, fisheryIndex;
 2466: 	int HistYrMin;
 2467:     //int Growthage_L1 = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][Growthage_L1_id]);
 2468: 	double Nsexes = (double) (bm->K_num_sexes);
 2469: 	int *dflt = malloc(sizeof(int) * (size_t)bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]);
 2470: 	int allregion = 0;    // uses weighted sum over regions, assume assessment doesn't know about regions
 2471: 	int recruit_sp;
 2472: 
 2473: 	if (bm->RBCestimation.RBCspeciesArray[groupIndex].CurrentYear == (bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMax_id] - 1))
 2474: 		HistYrMin = (int)bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id];
 2475:         //HistYrMin = 0;
 2476: 	else
 2477: 		HistYrMin = (int)bm->RBCestimation.RBCspeciesArray[groupIndex].CurrentYear + 1 - (int)bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id];
 2478: 
 2479: 	sscanf(bm->t_units, "seconds since %d-%s", &startYear, str);
 2480: 
 2481: 	recruit_sp = (int) (FunctGroupArray[groupIndex].speciesParams[flagrecruit_id]);
 2482: 
 2483: 	snprintf(str, sizeof(str), "%s%s%s", dirName, FOLDER_SEP, fileName);
 2484: 
 2485: 	printf("fileName = %s\n", str);
 2486: 
 2487: 	if ((fid = fopen(str, "w")) == NULL)
 2488: 		quit("Create_Control_File: Can't open %s\n", str);
 2489: 
 2490: 	printf("versionID = %d\n", versionID);
 2491: 
 2492: 	fprintf(fid, "#V3.24f\n");
 2493: 	fprintf(fid, "#C growth parameters are estimated\n");
 2494: 	fprintf(fid, "#C spawner-recruitment bias adjustment Not tuned For optimality\n");
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
**Lines (current code):** 3040-3516  
**Lines at review (Oct 2026):** 2966-3442

**Function:** `Write_SS_Data_File_Orig()`

**Why it's thought to be a bug**

(Dead.) `s` loops `0..Nsex_samp-1` with `Nsex_samp = 1` but vectors are only written for `s == 1` / `s == 2` (3346, 3353, 3446, 3453) so no comp data are ever written; `ageIndex <= MaxAge` (3448, 3458) reads `AgeComp[..][MaxAge]`, past the `OverallMaxAge` dimension when MaxAge == OverallMaxAge; `num` not reset before the length/age obs counts (3317, 3416) so counts include discards; 3134 prints stale `fleetIndex+1` instead of `nf+1`; environmental-variable header written twice (3493-3505). Function: `Write_SS_Data_File_Orig()`.

**Current code** (`atSS3Link/atSS3LinkIO.c` lines 3038–3077 (truncated))

```c
 3038: //
 3039: //******************************************************************************
 3040: void Write_SS_Data_File_Orig(MSEBoxModel *bm, char *dirName, char *fileName, int maxyr, int groupIndex, int versionID) {
 3041:     FILE *fid;
 3042:     char str[STRLEN];
 3043:     char tempStr[STRLEN];
 3044:     char line1[STRLEN], line2[STRLEN], line3[STRLEN];
 3045:     int seasonIndex, yearIndex, fleetIndex;
 3046:     int index;
 3047:     double value;
 3048:     int nseas = 1;
 3049:     int nAgeBins;
 3050:     int Nsexes = bm->K_num_sexes;
 3051:     int AccumAge = (int)FunctGroupArray[groupIndex].speciesParams[AccumAge_id];  // TODO: Or should this be bm->RBCestimation.RBCspeciesParam[groupIndex][AccumAge_id] ?
 3052:     int HistYrMin;
 3053:     int numYears, allregion, nf, num, it, s, part = 0, gender, l, ageIndex;
 3054:     double cpcv, disccv;
 3055:     char *emptySex;
 3056:     double Nsex_samp = 1;
 3057:     int emptySexSize;
 3058: 
 3059:     if (bm->RBCestimation.RBCspeciesArray[groupIndex].CurrentYear == (bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMax_id] - 1))
 3060:         HistYrMin = (int)bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id];
 3061:         //HistYrMin = 0;
 3062:     else
 3063:         HistYrMin = (int)bm->RBCestimation.RBCspeciesArray[groupIndex].CurrentYear + 1 - (int)bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id];
 3064: 
 3065:     printf("maxyr = %d\n", maxyr);
 3066:     numYears = maxyr + 1;    // - HistYrMin + 1;
 3067: 
 3068:     printf("bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMax_id] = %d\n", (int) bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMax_id]);
 3069:     printf("bm->RBCestimation.RBCspeciesArray[groupIndex].CurrentYear = %d\n", bm->RBCestimation.RBCspeciesArray[groupIndex].CurrentYear);
 3070:     printf("numYears = %d\n", numYears);
 3071: 
 3072:     //sscanf(bm->t_units, "seconds since %d-%s", &startYear, str);
 3073: 
 3074:     snprintf(str, sizeof(str), "%s%s%s", dirName, FOLDER_SEP, fileName);
 3075: 
 3076:     if ((fid = fopen(str, "w")) == NULL)
 3077:         quit("Write_SS_Data_File_Orig: Can't open %s\n", str);
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
