# atmanage/atRlinkRBC.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **8 findings** (severity H: 5; confidence H and severity H: 5; on the priority list: 5), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MNGIO-27](#mngio-27) ⭐ | 386, 381 | H/H | use after close | `Util_Close_Output_File(bm->metierFile)` closes the file but cannot NULL the pointer (atUtil.c:200). In the ne… |  | [ ] |
| [MNGIO-28](#mngio-28) ⭐ | 273 | H/H | uninitialised variable | `if (line_size == 1) { ThisSpID = atoi(line_buf); spID = ...}`. getline counts the newline, so a species-ID li… |  | [ ] |
| [MNGIO-29](#mngio-29) ⭐ | 285, 303 | H/H | wrong strtok delimiter | `tok = strtok(NULL, "\n")` uses "\n" instead of `seps` for later tokens, so the rest of the line becomes one t… |  | [ ] |
| [MNGIO-30](#mngio-30) ⭐ | 280 | H/H | loop condition order | `while (fgets(buffer,...) && (i < nrow))` reads a line before checking i<nrow. After nrow rows it consumes and… |  | [ ] |
| [MNGIO-31](#mngio-31) ⭐ | 268, 295, 298 | H/H | shadowed variable / wrong index | `for (int sp = 0; sp < MULTISPestNspecies; ...)` shadows the outer `sp`. `RBC_by_year[year]` is written to `RB… |  | [ ] |
| [MNGIO-32](#mngio-32) | 318 | H/M | off-by-one | The average loop runs `yr < mgt_rbcYear + nRBCaverage - 1`, which sums nRBCaverage-1 values, but divides by nR… |  | [ ] |
| [MNGIO-33](#mngio-33) | 202 | M/L | output formatting | The newline test sits inside the `nreg` loop, so the last species/metier prints Nregions newlines per year row… |  | [ ] |
| [MNGIO-34](#mngio-34) | 389 | M/L | wrong directory | `cleanPGMSYFiles()` removes "Metier.dat" and "MULTRBC.RBC" relative to the current working directory (input fo… |  | [ ] |

---

## MNGIO-27

**use after close** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atRlinkRBC.c` 386, `atRlinkRBC.c` 381
**Lines at review (Oct 2026):** 371-376

**Function:** `Do_PGMSY_R()`

**Why it's thought to be a bug**

`Util_Close_Output_File(bm->metierFile)` closes the file but cannot NULL the pointer (atUtil.c:200). In the next assessment year `if(!bm->metierFile)` is false, so writeMetierFile writes to a closed FILE* (undefined behaviour). Function: `Do_PGMSY_R()`.

**Code at issue** — `atRlinkRBC.c` line 386  
Search for: `Util_Close_Output_File(bm->metierFile)`

```c
  384:     }
  385:     writeMetierFile(bm, year, bm->metierFile);
  386→     Util_Close_Output_File(bm->metierFile);
  387: 
  388:     /* Change directory into the output directory to run PGMSY*/
```

**Code at issue** — `atRlinkRBC.c` line 381  
Search for: `if(!bm->metierFile)`

```c
  379:     
  380:     /* multiSpeciesAssess_R.bat */
  381→     if(!bm->metierFile) {
  382:         printf("Attempting to create MetierFile\n");
  383:         bm->metierFile = initMetierFile(bm);
```

**Suggested fix (from review):** Set `bm->metierFile = NULL` after closing.

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

## MNGIO-28

**uninitialised variable** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atRlinkRBC.c` 273
**Lines at review (Oct 2026):** 270-274

**Function:** `Read_PGMSY_output()`

**Why it's thought to be a bug**

`if (line_size == 1) { ThisSpID = atoi(line_buf); spID = ...}`. getline counts the newline, so a species-ID line like "5\n" has length 2. The branch only runs for empty lines, so `spID` stays uninitialised and indexes RBCspeciesParam and Catch_by_Metier. Function: `Read_PGMSY_output()`.

**Code at issue** — `atRlinkRBC.c` line 273 (the line the review pointed to, found by its text)  
Search for: `ThisSpID = atoi(line_buf);`

```c
  271:         line_size = getline(&line_buf, &line_buf_size, fp);
  272:         if(line_size == 1) {
  273→             ThisSpID = atoi(line_buf);
  274:             spID = bm->RBCestimation.MULTISPestIDs[ThisSpID];
  275:         }
```

**Suggested fix (from review):** Test `line_size > 1` (or parse it), and quit on a bad line.

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

## MNGIO-29

**wrong strtok delimiter** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atRlinkRBC.c` 285, `atRlinkRBC.c` 303
**Lines at review (Oct 2026):** 284

**Function:** `Read_PGMSY_output()`

**Why it's thought to be a bug**

`tok = strtok(NULL, "\n")` uses "\n" instead of `seps` for later tokens, so the rest of the line becomes one token. Only values[0] and values[1] are set, and `values[2+m]` (line 303) reads uninitialised malloc'd memory. Function: `Read_PGMSY_output()`.

**Code at issue** — `atRlinkRBC.c` line 285  
Search for: `tok = strtok(NULL, "\n")`

```c
  283: 
  284:             // Assign values
  285→             for (tok = strtok(buffer, seps); tok && *tok; j++, tok = strtok(NULL, "\n")) {
  286:                 values[j] = atof(tok);
  287:                 //fprintf(bm->logFile, "%f\t", values[j]);
```

**Code at issue** — `atRlinkRBC.c` line 303  
Search for: `values[2+m]`

```c
  301:             for (m = 0; m < bm->RBCestimation.RBCspeciesParam[spID][NumFisheries_id]; m++) {
  302:                 nf = bm->RBCestimation.speciesRPFleetToMetier[m][spID];
  303→                 bm->RBCestimation.Catch_by_Metier[spID][year][i][nf] = values[2+m];
  304:             }
  305:             
```

**Suggested fix (from review):** `strtok(NULL, seps)` (with "\n" added to seps), and bound j < size.

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

## MNGIO-30

**loop condition order** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atRlinkRBC.c` 280
**Lines at review (Oct 2026):** 279

**Function:** `Read_PGMSY_output()`

**Why it's thought to be a bug**

`while (fgets(buffer,...) && (i < nrow))` reads a line before checking i<nrow. After nrow rows it consumes and drops the next species' ID line, so the following species is misparsed. Function: `Read_PGMSY_output()`.

**Code at issue** — `atRlinkRBC.c` line 280  
Search for: `while (fgets(buffer,`

```c
  278:         // TODO: Will need to fix this
  279:         i = 0;
  280→         while (fgets(buffer, sizeof buffer, fp) && (i < nrow)) {
  281:             const char* tok;
  282:             j = 0;
```

**Suggested fix (from review):** `while (i < nrow && fgets(...))`.

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

## MNGIO-31

**shadowed variable / wrong index** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atRlinkRBC.c` 268, `atRlinkRBC.c` 295, `atRlinkRBC.c` 298
**Lines at review (Oct 2026):** 267, 294

**Function:** `Read_PGMSY_output()`

**Why it's thought to be a bug**

`for (int sp = 0; sp < MULTISPestNspecies; ...)` shadows the outer `sp`. `RBC_by_year[year]` is written to `RBCspeciesArray[sp]` (loop counter) instead of `[spID]`, and every projection row overwrites the same `[year]` slot although the comment says all years are needed (for the average at 316-317). Function: `Read_PGMSY_output()`.

**Code at issue** — `atRlinkRBC.c` line 268 (closest match to the quoted code; wording differs slightly)  
Search for: `for (int sp = 0; sp < bm->RBCestimation.MULTISPestNspecies; sp++) {`

```c
  266:     // Loop through the file
  267:     
  268→     for (int sp = 0; sp < bm->RBCestimation.MULTISPestNspecies; sp++) {
  269: 
  270:         // Read in species of interest
```

**Code at issue** — `atRlinkRBC.c` line 295  
Search for: `RBC_by_year[year]`

```c
  293:             
  294:             // Assign projection RBC
  295→             bm->RBCestimation.RBCspeciesArray[sp].RBC_by_year[year] = values[1];  // Nead all years (i) as used in calculating Average RBC below
  296:             if (year == values[0]) {
  297:                 // Store current RBC
```

**Code at issue** — `atRlinkRBC.c` line 298  
Search for: `[spID]`

```c
  296:             if (year == values[0]) {
  297:                 // Store current RBC
  298→                 bm->RBCestimation.RBCspeciesParam[spID][RBCest_id] = values[1];
  299:             }
  300:             // Assign metier catches
```

**Suggested fix (from review):** Use `RBCspeciesArray[spID].RBC_by_year[(int)values[0]]` (or year+i).

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

## MNGIO-32

**off-by-one** · Conf H / Sev M  
**Found at (current code):** `atRlinkRBC.c` 318
**Lines at review (Oct 2026):** 316-319

**Function:** `Read_PGMSY_output()`

**Why it's thought to be a bug**

The average loop runs `yr < mgt_rbcYear + nRBCaverage - 1`, which sums nRBCaverage-1 values, but divides by nRBCaverage. Function: `Read_PGMSY_output()`.

**Code at issue** — `atRlinkRBC.c` line 318 (the line the review pointed to, found by its text)  
Search for: `(previously summed nRBCaverage-1 values starting at mgt_rbcYear and divided by nRBCaverage`

```c
  316:             // Calculate the average RBC
  317:             /* Nov 2026: average the most recent nRBCaverage RBCs ending with this year, as in Averaged_RBC
  318→                (previously summed nRBCaverage-1 values starting at mgt_rbcYear and divided by nRBCaverage) */
  319:             AveRBC = 0;
  320:             startyr = year - bm->RBCestimation.nRBCaverage + 1;
```

**Suggested fix (from review):** `yr < mgt_rbcYear + nRBCaverage`.

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

## MNGIO-33

**output formatting** · Conf M / Sev L  
**Found at (current code):** `atRlinkRBC.c` 202
**Lines at review (Oct 2026):** 202-209, 97-98

**Function:** `writeMetierFile()`

**Why it's thought to be a bug**

The newline test sits inside the `nreg` loop, so the last species/metier prints Nregions newlines per year row when Nregions>1. Metier codes are printed as `"%s "` plus another `" "`, giving a double space. Function: `writeMetierFile()`.

**Code at issue** — `atRlinkRBC.c` line 202 (the line the review pointed to, found by its text)  
Search for: `for (nreg = 0; nreg < Nregions ; nreg++){`

```c
  200:                 Nregions = (int)(bm->RBCestimation.RBCspeciesParam[sp][NumRegions_id]);
  201:                 for (m = 0; m < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; m++) {
  202→                     for (nreg = 0; nreg < Nregions ; nreg++){
  203:                         nf = bm->RBCestimation.speciesMetierToRPFleet[sp][m];
  204:                         fprintf(fid, "%e", bm->RBCestimation.RBCspeciesArray[sp].CatchData[nf][nreg][yr]);
```

**Suggested fix (from review):** Move the end-of-row test outside the region loop and check nreg==Nregions-1.

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

## MNGIO-34

**wrong directory** · Conf M / Sev L  
**Found at (current code):** `atRlinkRBC.c` 389
**Lines at review (Oct 2026):** 359, 40-43

**Function:** `Do_PGMSY_R()`

**Why it's thought to be a bug**

`cleanPGMSYFiles()` removes "Metier.dat" and "MULTRBC.RBC" relative to the current working directory (input folder) before `chdir(destFolder)`, but the files are created and read in destFolder, so stale files are not removed. Function: `Do_PGMSY_R()`.

**Code at issue** — `atRlinkRBC.c` line 389  
Search for: `chdir(bm->destFolder)`

```c
  387: 
  388:     /* Change directory into the output directory to run PGMSY*/
  389→     ret = chdir(bm->destFolder);
  390:     
  391:     // if the change of directory was successful it will print successful otherwise it will print not successful
```

**Suggested fix (from review):** Build the paths with bm->destFolder.

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

atRlinkRBC.c:261-262: `getline` returns ssize_t but is stored in size_t. The `line_size != 4` header test is fragile, and `line_buf` from getline is leaked (same at atRlinkRAssess.c:266).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
